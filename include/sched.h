#ifndef __SCHED_H
#define __SCHED_H

#include "type.h"
#include "task.h"
#include "timers.h"
//#include "portmacro.h"
#include "mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef void (*task_fun_t)( void *parameters );
typedef mcu_fun_t task_fun_t; 
#define TASK_IDLE_PRIORITY    				( 0 )

/*进入临界区*/
#define SCHED_ENTER_CRITICAL()				MCU_ENTER_CRITICAL()
#define SCHED_ENTER_CRITICAL_FROM_ISR()

/*退出临界区*/
#define SCHED_EXIT_CRITICAL()				MCU_EXIT_CRITICAL()
#define SCHED_EXIT_CRITICAL_FROM_ISR()

#define SCHED_YIELD()						MCU_YIELD()
#define SCHED_START()						MCU_START_SCHEDULER()

#define SCHED_STACK_INIT()					MCU_STACK_INIT()				
#define SCHED_MCU_SLEEP()					MCU_SLEEP();

#define SCHED_GET_HIHGEST_PRIORITY( top_pri, ready_pri )		\
							MCU_GET_HIGHEST_PRIORITY( (top_pri), (ready_pri))

typedef int8_t(*scheduler_task_create_t)( 
												   task_handle_t *task_handle, 
												   task_fun_t task_fun, 
												   const char * const name,
												   const uint16_t stack_depth,
												   void *const parameters,
												   uint32_t priority
												   );
typedef int8_t(*scheduler_task_delete_t)( task_handle_t task_handle );


typedef int8_t(*scheduler_task_event_create_t)( 
												   task_handle_t *task_handle, 
												   task_event_fun_t task_event_fun, 
												   const char * const name,
												   const uint16_t stack_depth,
												   uint32_t priority,
												   task_event_add_fun_t add,
												   task_event_del_fun_t del
												   );
typedef int8_t(*scheduler_task_event_delete_t)( task_handle_t task_handle );

typedef uint32_t(*scheduler_task_set_event_t)( task_handle_t handle, uint32_t event );
typedef uint32_t(*scheduler_task_clear_event_t)( task_handle_t handle, uint32_t event );

typedef void(*scheduler_task_delay_t)(        	const tick_type_t time_tick	);
typedef void(*scheduler_task_delay_until_t)( tick_type_t * const per_wake_time, const tick_type_t time_tick );

typedef int8_t(*scheduler_task_schde_t)();
typedef int8_t(*scheduler_init_t)();
typedef int8_t(*scheduler_task_tick_inc_t)();
typedef tcb_t*(*scheduler_get_current_tcb_t)();
typedef void(*scheduler_task_switch_context_t)();
typedef void(*scheduler_suspend_t)();
typedef void(*scheduler_resume_t)();
typedef void(*scheduler_task_suspend_t)(task_handle_t handle);
typedef void(*scheduler_task_resume_t)(task_handle_t handle);

#ifdef OPEN_SHCDE_DEBUG
typedef void(*scheduler_task_info_show_t)();	
#endif

typedef int8_t(*scheduler_task_push_msg_queue_t)( list_head_t *head );
typedef int8_t(*scheduler_task_pop_msg_quue_t)( list_head_t *head );

//typedef stack_type_t * (*scheduler_task_stack_init_t)( stack_type_t *stack, task_fun_t task_fun, void *params);
//typedef int8_t(*scheduler_task_register_stack_init_fun_t)( scheduler_task_stack_init_t task_init );


typedef struct _scheduler
{
	
	tcb_t *idle_tcb;												/*任务控制块*/

	uint32_t bmp_priority;											/*任务优先级位图*/							
	tick_type_t tick_count;											/*系统运行tick计数*/
								
	list_head_t task_list_ready[ ERTOS_MAX_PRIORITIES ];			/*就绪任务列表*/
	list_head_t task_list_delay;									/*延时任务链表*/
	list_head_t task_list_suspend;									/*挂起任务链表*/
	list_head_t task_list_waiting_termination;						/*等待终止链表*/
	
	scheduler_get_current_tcb_t	get_current_tcb;
	scheduler_task_switch_context_t task_switch_context;			/*上下文切换*/
	scheduler_task_delay_t task_delay;								/*任务相对延迟*/
	scheduler_task_delay_until_t task_delay_until;					/*任务绝对延迟*/
	//scheduler_task_register_stack_init_fun_t register_stack_init_fun;	/*注册任务栈初始化函数*/
	scheduler_task_push_msg_queue_t  task_push_queue;     			/*进入队列, 并添加到挂起任务链表*/
	scheduler_task_pop_msg_quue_t	task_pop_queue;					/*从队列中删除,并添加到就绪任务链表*/
	scheduler_task_suspend_t task_suspend;							/*任务暂定*/
	scheduler_task_resume_t task_resume;							/*任务恢复*/
	scheduler_task_tick_inc_t task_tick_inc;						/*增加时钟tick*/

	scheduler_task_set_event_t set_event;							/*设置任务标志位*/
	scheduler_task_clear_event_t clear_event;						/*清除任务标志位*/
	scheduler_task_event_create_t task_event_create;				/*事件任务创建*/
	scheduler_task_event_delete_t task_event_delete;				/*事件任务删除*/
	
	scheduler_task_create_t task_create;							/*任务创建*/
	scheduler_task_delete_t task_delete;							/*任务删除*/
	scheduler_task_schde_t schde;									/*任务调度*/
	scheduler_init_t init;											/*初始化*/
	
#ifdef OPEN_SHCDE_DEBUG
	scheduler_task_info_show_t task_info_show;						/*任务信息*/
#endif
	timer_t *timer;													/*事件定时器*/
}scheduler_t;

extern void scheduler_task_switch_context(void);
extern scheduler_t * get_scheduler(void);

#ifdef __cplusplus
}
#endif


#endif /*__SCHED_H*/

