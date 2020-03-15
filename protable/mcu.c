#include "mcu.h"
#include "ertos_config.h"
#include "sched.h"
#include "stm32f4xx.h"

/*临界访问变量*/
static uint32_t critical_nesting = 0;

static uint8_t max_syscall_priority = 0;
static uint32_t max_prigroup_value = 0;

//static const volatile uint8_t * const ptr_interrupt_priority_registers = ( uint8_t * ) MCU_NVIC_IP_REGISTERS_OFFSET_16;

#define MCU_NVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define MCU_NVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define MCU_NVIC_SYSTICK_CURRENT_VALUE_REG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )
#define MCU_NVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )

/* ...then bits in the registers. */
#define MCU_NVIC_SYSTICK_INT_BIT			( 1UL << 1UL )
#define MCU_NVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )
#define MCU_NVIC_SYSTICK_COUNT_FLAG_BIT		( 1UL << 16UL )
#define MCU_NVIC_PENDSVCLEAR_BIT 			( 1UL << 27UL )
#define MCU_NVIC_PEND_SYSTICK_CLEAR_BIT		( 1UL << 25UL )


#ifndef MCU_SYSTICK_CLOCK_HZ
	#define MCU_SYSTICK_CLOCK_HZ 			( ERTOS_CPU_CLOCK_HZ )
	#define MCU_TICK_RATE_HZ 				( ERTOS_TICK_RATE_HZ )
	/* Ensure the SysTick is clocked at the same frequency as the core. */
	#define MCU_NVIC_SYSTICK_CLK_BIT		( 1UL << 2UL )
#endif


/*
 *  systick clok ,为是时钟频180Mhz, 180Mhz/1000ms 则为每一ms寄存器增加的计数, 
 *  公式，180Mhz/1000ms = x/tick_hz, tick_hz, 为tick 时钟中断频率, x为寄存器重装寄存器数值。
 */
/* 每1ms 寄存器计数值*/
#define MCU_SYSTICK_PER_MS_VALUE        	( MCU_SYSTICK_CLOCK_HZ / 1000 )
#define MCU_SYSTICK_RELAODE_VALUE			( MCU_SYSTICK_PER_MS_VALUE * ERTOS_TICK_RATE_HZ ) 

/* 最max睡眠次数 */
#define MCU_SLEEP_MAX_TICK					( (  MAX_24_BIT_NUMBER / MCU_SYSTICK_RELAODE_VALUE ) )

/*停止运行时间补尝*/
#define MCU_STOP_TIME_COMPENSAIIOCN 		( 45ul )


#define MCU_FPCCR							( ( volatile uint32_t * ) 0xe000ef34 ) /* Floating point context control register. */
#define MCU_ASPEN_AND_LSPEN_BITS			( 0x3UL << 30UL )

#define MCU_NVIC_PENDSV_PRI					( ( ( uint32_t ) ERTOS_KERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define MCU_NVIC_SYSTICK_PRI				( ( ( uint32_t ) ERTOS_KERNEL_INTERRUPT_PRIORITY ) << 24UL )


#define MCU_INITIAL_XPSR					( 0x01000000 )
#define MCU_INITIAL_EXEC_RETURN				( 0xfffffffd )

#define MCU_START_ADDRESS_MASK				( ( stack_type_t ) 0xfffffffeUL )

#define MCU_FIRST_USER_INTERRUPT_NUMBER		( 16 )
#define MCU_NVIC_IP_REGISTERS_OFFSET_16 	( 0xE000E3F0 )
#define MCU_MAX_8_BIT_VALUE					( ( uint8_t ) 0xff )

#define MCU_TOP_BIT_OF_BYTE					( ( uint8_t ) 0x80 )
#define MCU_MAX_PRIGROUP_BITS			    ( ( uint8_t ) 7 )
#define MCU_PRIORITY_GROUP_MASK				( 0x07UL << 8UL )
#define MCU_PRIGROUP_SHIFT					( 8UL )


void handr_fault_c(unsigned int * svc_args)
{
	//printf("HardFault\r\n");
    while(1);
}

__asm void HardFault_Handler(void)
{
	TST LR, #4
	ITE EQ
	MRSEQ R0, MSP
	MRSNE R0, PSP
	B __cpp(handr_fault_c)
}


void SysTick_Handler(void)
{

	raise_basepri();
	{

		//printf("%s \r\n", __func__);
		if( get_scheduler()->task_tick_inc() != FALSE )
		{
			MCU_NVIC_INT_CTRL_REG = MCU_NVIC_PENDSVSET_BIT;
		}
	}
	clear_basepri_from_isr();
   
}

__asm void SVC_Handler( void )
{
    PRESERVE8
    extern current_tcb;
    /* Get the location of the current TCB. */
    ldr	r3, =current_tcb
    ldr r1, [r3]
    ldr r0, [r1]
    /* Pop the core registers. */
    ldmia r0!, {r4-r11, r14}
    msr psp, r0
    isb
    mov r0, #0
    msr	basepri, r0
    bx r14
    NOP /*对齐*/
}


__asm void PendSV_Handler( void )
{
	PRESERVE8
    
 	extern critical_nesting;
	extern current_tcb;
	//extern scheduler_task_switch_context;
    extern scheduler_task_switch_context;   
    
	mrs r0, psp /*传送psp寄存器到r0*/
	isb
	/* Get the location of the current TCB. */
	ldr	r3, =current_tcb /*同时也是栈顶指针*/
	ldr	r2, [r3]         /*将栈顶值传送到r2*/

	/* Is the task using the FPU context?  If so, push high vfp registers. */
	tst r14, #0x10      /*比较连接寄存器值, 判断是否使用了浮点运算*/
	it eq
	vstmdbeq r0!, {s16-s31} /*如果使用了浮点运算, 则在当堆栈保存s16 - s31 寄存器值*/

	/* Save the core registers. */
	stmdb r0!, {r4-r11, r14} /*保存r4-r11 , r14 寄存器到*/

	/* Save the new top of stack into the first member of the TCB. */
	str r0, [r2]       /*更新栈顶, tcb 的值*/

	stmdb sp!, {r3}
	mov r0, #ERTOS_MAX_SYSCALL_INTERRUPT_PRIORITY
	msr basepri, r0
	dsb
	isb
	bl scheduler_task_switch_context
	mov r0, #0
	msr basepri, r0
	ldmia sp!, {r3}  /*r3  为更新后tcb值*/

	/* The first item in pxCurrentTCB is the task top of stack. */
	ldr r1, [r3]
	ldr r0, [r1]

	/* Pop the core registers. */
	ldmia r0!, {r4-r11, r14}

	/* Is the task using the FPU context?  If so, pop the high vfp registers
	too. */
	tst r14, #0x10
	it eq
	vldmiaeq r0!, {s16-s31}

	msr psp, r0
	isb

	bx r14
}


__asm void mcu_enable_vfp( void )
{
	PRESERVE8

	/* The FPU enable bits are in the CPACR. */
	ldr.w r0, =0xE000ED88
	ldr	r1, [r0]

	/* Enable CP10 and CP11 coprocessors, then save back. */
	orr	r1, r1, #( 0xf << 20 )
	str r1, [r0]
	bx	r14
	nop
}


__asm void mcu_start_first_task( void )
{
	PRESERVE8

	/* Use the NVIC offset register to locate the stack. */
	ldr r0, =0xE000ED08    /*VTOR, 第一字，msp, 第二字， 复位向量*/
	ldr r0, [r0]
	ldr r0, [r0]
	/* Set the msp back to the start of the stack. */
	msr msp, r0
	/* Globally enable interrupts. */
	cpsie i
	cpsie f
	dsb
	isb
	/* Call SVC to start the first task. */
	svc 0
	nop
	nop
}

static void mcu_task_exit_err( void )
{
	//DISABLE_INTERRUPTS();
    //printf("%s \r\n", __FUNCTION__);
	for( ;; );
}


void mcu_exit_critical( void )
{
	critical_nesting--;
	if( critical_nesting == 0 )
	{
		MCU_ENABLE_INTERRUPTS();
	}
}

void mcu_enter_critical( void )
{
	MCU_DISABLE_INTERRUPTS();
	critical_nesting++;
}


stack_type_t *mcu_stack_init( stack_type_t *top_stack, mcu_fun_t task_fun, void *parameters )
{

	top_stack--;

	*top_stack = MCU_INITIAL_XPSR;	/* xPSR */
	top_stack--;
	*top_stack = ( ( stack_type_t ) task_fun ) & MCU_START_ADDRESS_MASK;	/* PC */
	top_stack--;
	*top_stack = ( stack_type_t ) mcu_task_exit_err;	/* LR */

	/* Save code space by skipping register initialisation. */
	top_stack -= 5;	/* R12, R3, R2 and R1. */
	*top_stack = ( stack_type_t ) parameters;	/* R0 */


	top_stack--;
	*top_stack = MCU_INITIAL_EXEC_RETURN;

	top_stack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */

	return top_stack;

}


void setup_timer_interrupt( void )
{
	SysTick->LOAD  = MCU_SYSTICK_RELAODE_VALUE - 1UL; 
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;    
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; 
}

int32_t mcu_start_scheduler(void)
{
	int32_t ret = 0;
	volatile uint32_t original_pri;
	volatile uint8_t *ptr_first_user_pri_register = (uint8_t *)( MCU_NVIC_IP_REGISTERS_OFFSET_16 + MCU_FIRST_USER_INTERRUPT_NUMBER);
	volatile uint8_t max_pri_value;
    
	original_pri = *ptr_first_user_pri_register;
	*ptr_first_user_pri_register = MCU_MAX_8_BIT_VALUE;
	max_pri_value = *ptr_first_user_pri_register;

	if( (max_pri_value == ( ERTOS_KERNEL_INTERRUPT_PRIORITY & max_pri_value )) == 0 )
	{
		ret = -1;
	}
	
	max_syscall_priority = ERTOS_MAX_SYSCALL_INTERRUPT_PRIORITY & max_pri_value;
	max_prigroup_value = MCU_MAX_PRIGROUP_BITS;

	while( ( max_pri_value & MCU_TOP_BIT_OF_BYTE ) == MCU_TOP_BIT_OF_BYTE )
	{
		max_prigroup_value--;
		max_pri_value <<= ( uint8_t ) 0x01;
	}

	/* Shift the priority group value back to its position within the AIRCR
	register. */
	max_prigroup_value <<= MCU_PRIGROUP_SHIFT;
	max_prigroup_value &= MCU_PRIORITY_GROUP_MASK;
	*ptr_first_user_pri_register = original_pri;
	
	MCU_NVIC_SYSPRI2_REG |= MCU_NVIC_PENDSV_PRI;
	MCU_NVIC_SYSPRI2_REG |= MCU_NVIC_SYSTICK_PRI;
    
	setup_timer_interrupt();
	
	critical_nesting = 0;
	mcu_enable_vfp();
	/* Lazy save always. */
	*( MCU_FPCCR ) |= MCU_ASPEN_AND_LSPEN_BITS;

	/* Start the first task. */
	mcu_start_first_task();
	return ret;
}

/*
 *  睡眠前处理函数
 */
void mcu_per_sleep_processing( tick_type_t tick_time )
{
	//printf( "%s , %d \r\n", __func__ , tick_time );
	return ;
}

/*
 *  睡眠退出处理函数
 */
void mcu_post_sleep_processing( tick_type_t tick_time )
{
	//printf( "%s , %d\r\n", __func__ , tick_time );
	return ;
}

void mcu_sleep( tick_type_t tick_time )
{
	uint32_t reload_value, complete_tick_periods, completed_systick_dec, systick_ctrl;
	
	if( tick_time > MCU_SLEEP_MAX_TICK )
	{
		tick_time = MCU_SLEEP_MAX_TICK;
	}
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk; 
	reload_value = SysTick->VAL + MCU_SYSTICK_RELAODE_VALUE * ( tick_time - 1 );
		
	if( reload_value > MCU_STOP_TIME_COMPENSAIIOCN )
	{
		reload_value -= MCU_STOP_TIME_COMPENSAIIOCN;
	}
	__disable_irq();
	__dsb( MCU_SY_FULL_READ_WRITE );
	__isb( MCU_SY_FULL_READ_WRITE );
	
	SysTick->LOAD = reload_value;
	SysTick->VAL = 0UL;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	mcu_per_sleep_processing( tick_time );

	__dsb( MCU_SY_FULL_READ_WRITE );
	__wfi(); /*进入睡眠*/
	__isb( MCU_SY_FULL_READ_WRITE );

	mcu_post_sleep_processing( tick_time );
	
	systick_ctrl = SysTick->CTRL;
	SysTick->CTRL = ( systick_ctrl & ~SysTick_CTRL_ENABLE_Msk );
	__enable_irq();

	/*
	 *  唤醒后进行时间补尝
	 */
	if( ( systick_ctrl & SysTick_CTRL_ENABLE_Msk ) != 0 )
	{
		/*systick interrput*/
		uint32_t cal_load_value;
		cal_load_value = ( MCU_SYSTICK_RELAODE_VALUE - 1UL ) - ( reload_value - SysTick->VAL );
		if( ( cal_load_value < MCU_STOP_TIME_COMPENSAIIOCN ) || ( cal_load_value > MCU_SYSTICK_RELAODE_VALUE ) )
		{
			cal_load_value = ( MCU_SYSTICK_RELAODE_VALUE - 1UL );
		}

		SysTick->LOAD = cal_load_value;
		complete_tick_periods = tick_time - 1UL;
	}
	else
	{
		completed_systick_dec = ( tick_time * MCU_SYSTICK_RELAODE_VALUE ) - SysTick->VAL;
		complete_tick_periods = completed_systick_dec / MCU_SYSTICK_RELAODE_VALUE;
		SysTick->LOAD = ( ( complete_tick_periods + 1UL ) * MCU_SYSTICK_RELAODE_VALUE ) - completed_systick_dec;
	}

	SysTick->VAL = 0UL;
	MCU_ENTER_CRITICAL();
	{
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
		get_scheduler()->tick_count += complete_tick_periods; /*补尝时间*/
		SysTick->LOAD = MCU_SYSTICK_RELAODE_VALUE - 1UL;
	}
	MCU_EXIT_CRITICAL();
	
}

