#ifndef __ETROS_CONFIG_H
#define __ETROS_CONFIG_H

#include "type.h"
#include "stm32f4xx.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif


#define OPEN_SHCDE_DEBUG


#ifdef __NVIC_PRIO_BITS
	#define ERTOS_PRIO_BITS       		__NVIC_PRIO_BITS
#else
	#define PRIO_BITS       			4                  
#endif

#define ERTOS_ASSERT(x)  				ASSERT(x)
 

#define ERTOS_MAX_TASK_NAME_LEN								(16)                   
#define ERTOS_MAX_PRIORITIES								(32)

#define ERTOS_MINIMAL_STACK_SIZE				    		(130)   

#define ERTOS_LIBRARY_LOWEST_INTERRUPT_PRIORITY				(15)
#define ERTOS_LIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY		(5 )                      
#define ERTOS_KERNEL_INTERRUPT_PRIORITY 					( ERTOS_LIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - ERTOS_PRIO_BITS) )
#define ERTOS_MAX_SYSCALL_INTERRUPT_PRIORITY 				( ERTOS_LIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - ERTOS_PRIO_BITS) )


#define ERTOS_CPU_CLOCK_HZ									(SystemCoreClock)       
#define ERTOS_TICK_RATE_HZ									(10)  /*10ms每一个时钟片断*/                

#define ERTOS_OPEN_MEM_INFO 

#ifdef __cplusplus
}
#endif


#endif /*__ETROS_CONFIG_H*/
