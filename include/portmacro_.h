#ifndef __PORTMACRO_H
#define __PORTMACRO_H

#include "type.h"
#include "mcu.h"
#include "ertos_config.h"
#include "sched.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t tick_type_t;
#define PORT_MAX_DELAY 									( tick_type_t ) 0xffffffffUL


#define PORT_SY_FULL_READ_WRITE							( 15 )

#define PORT_NVIC_INT_CTRL_REG							( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define PORT_NVIC_PENDSVSET_BIT							( 1UL << 28UL )
#define	PORT_END_SWITCHING_ISR( xSwitchRequired ) 		
#define PORT_YIELD_FROM_ISR( x ) 						

#define PORT_BYTE_ALIGNMENT								(8)
#define PORT_BYTE_ALIGNMENT_MASK 						(0x0007)


#define PORT_YIELD()	 \
{ \
	PORT_NVIC_INT_CTRL_REG = PORT_NVIC_PENDSVSET_BIT;	\
	__dsb( PORT_SY_FULL_READ_WRITE );		\
	__isb( PORT_SY_FULL_READ_WRITE );		\
}


#define PORT_DISABLE_INTERRUPTS()						raise_basepri()
#define PORT_ENABLE_INTERRUPTS()						set_basepri(0)
#define PORT_ENTER_CRITICAL()							mcu_enter_critical()
#define PORT_EXIT_CRITICAL()							mcu_exit_critical()
#define PORT_SET_INTERRUPT_MASK_FROM_ISR()				set_interrput_mask_from_irs()
#define PORT_CLEAR_INTERRUPT_MASK_FROM_ISR( x )			set_basepri(x)

#define PORT_SET_READY_PRIORITY( pri, ready_pri )		( ready_pri ) |= ( 1ul << (pri) )
#define PORT_RESET_READY_PRIORITY( pri, ready_pri )		( ready_pri ) &= ~( 1ul << (pri) )

#define OPEN_PRIORITY_DEBUG

#ifndef OPEN_PRIORITY_DEBUGx
/*__clz, 硬件指令计算前导0个数*/
#define PORT_GET_HIGHEST_PRIORITY( top_pri, ready_pri ) \
					top_pri = ( 31UL - ( uint32_t ) __clz( ( ready_pri ) ) ) 
#else
#define PORT_GET_HIGHEST_PRIORITY( top_pri, ready_pri ) \
					top_pri = ( 31UL - ( uint32_t ) __clz( ( ready_pri ) ) );printf("top_pri :%u,%s,%u\r\n", top_pri,__func__, __LINE__)

#endif
extern stack_type_t *mcu_stack_init( stack_type_t *top_stack, task_fun_t task_fun, void *parameters );
#define PORT_STACK_INIT( top_stack, task_fun, parameters )	\
					mcu_stack_init( top_stack, task_fun, parameters )

#define PORT_START_SCHEDULER()	\
					mcu_start_scheduler()

static __forceinline void set_basepri( uint32_t lbasepri )
{
	__asm
	{
		msr basepri, lbasepri
	}
}

static __forceinline void raise_basepri( void )
{
	uint32_t lbasepri = ERTOS_MAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		msr basepri, lbasepri
		dsb
		isb
	}
}

static __forceinline uint32_t set_interrput_mask_from_irs( void )
{
    uint32_t ret, lbasepri = ERTOS_MAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		mrs ret, basepri
		msr basepri, lbasepri
		dsb
		isb
	}

	return ret;
}

static __forceinline void clear_basepri_from_isr( void )
{
	__asm
	{
		msr basepri, #0
	}
}



#ifdef __cplusplus
}
#endif


#endif /*__MCU_MACRO_H*/
