#ifndef __MCU_H
#define __MCU_H

#include "type.h"
#include "condef.h"
#include "ertos_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MCU_MAX_DELAY 									( tick_type_t ) 0xffffffffUL
#define MCU_SY_FULL_READ_WRITE							( 15 )

#define MCU_NVIC_INT_CTRL_REG							( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define MCU_NVIC_PENDSVSET_BIT							( 1UL << 28UL )

#define	MCU_END_SWITCHING_ISR( x ) 		
#define MCU_YIELD_FROM_ISR( x ) 						

#define MCU_BYTE_ALIGNMENT								( 8 )
#define MCU_BYTE_ALIGNMENT_MASK 						( 0x0007 )


#define MCU_YIELD()	 \
{ \
	MCU_NVIC_INT_CTRL_REG = MCU_NVIC_PENDSVSET_BIT;	\
	__dsb( MCU_SY_FULL_READ_WRITE );		\
	__isb( MCU_SY_FULL_READ_WRITE );		\
}


#define MCU_DISABLE_INTERRUPTS()						raise_basepri()
#define MCU_ENABLE_INTERRUPTS()							set_basepri(0)
#define MCU_ENTER_CRITICAL()							mcu_enter_critical()
#define MCU_EXIT_CRITICAL()								mcu_exit_critical()
#define MCU_SET_INTERRUPT_MASK_FROM_ISR()				set_interrput_mask_from_irs()
#define MCU_CLEAR_INTERRUPT_MASK_FROM_ISR( x )			set_basepri(x)

#define	MCU_SET_READY_PRIORITY( pri, ready_pri )		( ready_pri ) |= ( 1ul << (pri) )
#define MCU_RESET_READY_PRIORITY( pri, ready_pri )		( ready_pri ) &= ~( 1ul << (pri) )

#define OPEN_PRIORITY_DEBUGX

#ifndef OPEN_PRIORITY_DEBUG
/*__clz, 硬件指令计算前导0个数*/
#define MCU_GET_HIGHEST_PRIORITY( top_pri, ready_pri ) \
					top_pri = ( 31UL - ( uint32_t ) __clz( ( ready_pri ) ) ) 
#else
#define MCU_GET_HIGHEST_PRIORITY( top_pri, ready_pri ) \
					top_pri = ( 31UL - ( uint32_t ) __clz( ( ready_pri ) ) );\
					printf("top_pri :%u,%s,%u\r\n", top_pri,__func__, __LINE__)

#endif

typedef void(*mcu_fun_t)(void *params);

//extern stack_type_t *mcu_stack_init( stack_type_t *top_stack, task_fun_t task_fun, void *parameters );
#define MCU_STACK_INIT( top_stack, task_fun, parameters )	\
					mcu_stack_init( top_stack, task_fun, parameters )

#define MCU_START_SCHEDULER()	\
					mcu_start_scheduler()

#define MCU_SLEEP() 	mcu_sleep()
					
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

extern void mcu_exit_critical( void );
extern void mcu_enter_critical( void );
extern stack_type_t *mcu_stack_init( stack_type_t *top_stack, mcu_fun_t task_fun, void *parameters );
extern int32_t mcu_start_scheduler(void);
extern void mcu_sleep( tick_type_t tick_time );

#ifdef __cplusplus
}
#endif


#endif /*__MCU_H*/
