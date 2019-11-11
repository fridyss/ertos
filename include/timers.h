#ifndef __TIEMRS_H
#define __TIEMRS_H

#include "type.h"
#include "list.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef int8_t (*timer_start_event_t)( task_handle_t handle, uint32_t event,
																	  uint8_t reload_flag, 
																	  tick_type_t tick
																	);
typedef int8_t (*timer_stop_event_t)(task_handle_t handle, uint32_t event);
typedef tick_type_t (*timer_caculate_t)();
typedef int8_t(*timer_run_t)();

typedef struct __timer_node
{
	list_head_t list;
	uint32_t wake_ticks;
	uint32_t start_tick;
	uint8_t reload_flag;	/*0，单次，1 ，周期*/
	task_handle_t task_handle;
	uint32_t task_event;
}timer_node_t;

typedef struct __timer
{
	timer_node_t node;
	timer_start_event_t start_event;		/*开启定时任务*/
	timer_stop_event_t stop_event;			/*暂定定时任务*/
	timer_caculate_t caculate;				/*时间获取*/
	timer_run_t run;						/*定时器运行*/
}timer_t;

extern timer_t * get_timer(void);

#ifdef __cplusplus
}
#endif


#endif /*__TIEMRS_H*/


