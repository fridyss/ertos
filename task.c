#include "task.h"
#include "errno.h"
#include "sched.h"

/*有序添加，小值有前*/
int8_t task_list_add(list_head_t *head, tcb_t *tcb)
{
	int8_t ret = FALSE;
	if(head == NULL || tcb == NULL)
		return ret;
	if(list_is_empty( head ) == FALSE)
	{
		list_head_t *pos1, *pos2;
		tick_type_t cur_tick = get_scheduler()->tick_count;
		list_for_each_safe(pos1, pos2, head)
		{
			tcb_t *ltcb = list_entry(pos1, tcb_t, list);
			tick_type_t tick = cur_tick - ltcb->suspend_time_tick;
			if( tcb->wake_tick <= ( ltcb->wake_tick - tick ) )
			{
				list_node_add( &(tcb->list), pos1 );
				ret = TRUE;
				//printf("%s \r\n", __func__);
				break;
			}
		}
	}

	if(ret == FALSE)
	{	
		/*(1),MAX值,加入链表尾部*/
		/*(2),空链表 加入链表尾部;*/
		list_add_tail(&(tcb->list), head);
		ret = TRUE;
	}
	

	return ret;
}

/*从任务链表中删除*/
int8_t task_list_del(tcb_t *tcb)
{
	int8_t ret = FALSE;
	if(tcb == NULL)
		return ret;
	
	list_node_del( &(tcb->list) );
	ret = TRUE;
	return ret;
}


uint32_t task_set_event( task_handle_t handle, uint32_t event )
{
	if( handle == NULL)
		return 0;
	tcb_t * tcb = (tcb_t *) handle;
	tcb->task_event |= event;
	
	return tcb->task_event;
}

uint32_t task_clear_event( task_handle_t handle, uint32_t event )
{
	
	if( handle == NULL)
		return 0;
	tcb_t * tcb = (tcb_t *) handle;
	tcb->task_event &= ~event;

	return tcb->task_event;

}

