#ifndef __TASK_H
#define __TASK_H

#include "type.h"
#include "condef.h"
#include "list.h"
#include "ertos_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	TASK_STOP,
	TASK_RUNNING,
	TASK_READY,
	TASK_BLOCK,
	TASK_DELAY,
}task_state_t;

typedef void* task_handle_t;

typedef void (*task_event_fun_t)( task_handle_t handle, uint32_t event );
typedef void (*task_event_add_fun_t)();
typedef void (*task_event_del_fun_t)();

typedef struct _task_control_block
{
	
	stack_type_t *stack_top;						/*任务栈顶*/
	stack_type_t *stack_base;						/*任务基栈*/
	stack_type_t *stack_buffer;						/*任务栈缓存*/
	list_head_t list;								/*任务列表*/
	list_head_t event_list;							/*事件列表*/
	uint32_t priority;								/*任务优先级*/
	char name[ERTOS_MAX_TASK_NAME_LEN];				/*任务名*/
	task_state_t state;								/*任务状态*/
	tick_type_t counter;							/*运行时间*/
	tick_type_t suspend_time_tick;					/*任务挂起时间点*/
	tick_type_t wake_tick;							/*任务唤醒时间段*/	
	uint32_t task_event;							/*任务事件*/
	task_event_fun_t event_fun;						/*任务事件回调函数*/
	task_event_add_fun_t event_add_fun;				/*任务事件添加回调函数*/
	task_event_del_fun_t event_del_fun;				/*任务事件删除回调函数*/
}tcb_t;


extern int8_t task_list_add(list_head_t *head, tcb_t *tcb);
extern int8_t task_list_del(tcb_t *tcb);
extern uint32_t task_set_event( task_handle_t handle, uint32_t event );
extern uint32_t task_clear_event( task_handle_t handle, uint32_t event );


#ifdef __cplusplus
}
#endif


#endif /*__TASK_H*/

