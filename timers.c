#include "timers.h"
#include "mcu.h"
#include "mem_manager.h"
#include "errno.h"
#include "sched.h"

int8_t timer_start_event(			task_handle_t handle, 
						 			uint32_t event,
						 			uint8_t reload_flag, 
									tick_type_t tick
							 	 );

int8_t timer_stop_event( task_handle_t handle, uint32_t event );
tick_type_t timer_caculate( void );
int8_t timer_run( void );




timer_t g_timer = 
{
	.node	= {
				.list = { &g_timer.node.list, &g_timer.node.list },
	},
	.start_event = 	timer_start_event,	
	.stop_event =	timer_stop_event,	
	.caculate = timer_caculate,				
	.run = timer_run,
};


/*
 * 有序添加
 */
static int8_t timer_node_add( timer_node_t *node )
{
	int8_t ret = FALSE;

	if( node == NULL)
		return ret;
	if(list_is_empty(  &(g_timer.node.list) ) == FALSE )
	{
		list_head_t *pos1, *pos2;
		tick_type_t cur_tick = timer_caculate();
		list_for_each_safe( pos1, pos2, &(g_timer.node.list) )
		{
			timer_node_t *lnode = list_entry( pos1, timer_node_t, list );
			tick_type_t tick = cur_tick - lnode->start_tick; /*已经走过的tick*/
			if( node->wake_ticks <= (lnode->wake_ticks - tick) )
			{
				list_node_add( &(node->list), pos1 );
				ret = TRUE;
				//printf("%s head \r\n", __func__);
				break;
			}
		}
	
	}

	if(ret == FALSE)
	{	
		/*(1),MAX值,加入链表尾部*/
		/*(2),空链表 加入链表尾部;*/
		list_add_tail( &(node->list), &(g_timer.node.list) );
		//printf("%s tail \r\n", __func__);
		ret = TRUE;
	}
	return ret;
}

/* 删除*/
static int8_t timer_node_del( timer_node_t *node )
{
	if( node == NULL )
		return FALSE;
	list_node_del( &(node->list) );
	return TRUE;

}

int8_t timer_start_event(  		task_handle_t handle, 
								 	uint32_t event,
								 	uint8_t reload_flag, 
									tick_type_t tick
							 	 )
{
	int8_t ret = FALSE;
	MCU_ENTER_CRITICAL();
	{
		list_head_t *pos1,*pos2;
		list_for_each_safe( pos1, pos2, &(g_timer.node.list) )
		{
			timer_node_t *node = list_entry( pos1, timer_node_t, list );
			if( handle != NULL && \
					node->task_handle == handle && \
					node->task_event == event \
			  )
			{
				node->reload_flag = reload_flag;
				node->wake_ticks = tick;
				node->start_tick = timer_caculate();
				list_node_del( pos1 ); /*重新连接*/
				timer_node_add( node );
				MCU_EXIT_CRITICAL();
				return TRUE;
			}
		}

		/*没有找到, 则需要新建*/
		timer_node_t *node = (timer_node_t *)mem_alloc( sizeof(timer_node_t) );
		if(node == NULL)
			ret = ECOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
		else
		{
			//list_add_tail( &(node->list), &(g_timer.node.list));
			node->reload_flag = reload_flag;
			node->start_tick = timer_caculate();
			node->wake_ticks = tick;
			node->task_handle = handle;
			node->task_event = event;
            timer_node_add( node );
			ret = TRUE;
		}
	}
	MCU_EXIT_CRITICAL();
	return ret;
}

int8_t timer_stop_event(task_handle_t handle, uint32_t event)
{
    int8_t ret = FALSE;
	MCU_ENTER_CRITICAL();
	{
		list_head_t *pos1,*pos2;
		list_for_each_safe( pos1, pos2, &(g_timer.node.list) )
		{
			timer_node_t *node = list_entry( pos1, timer_node_t, list);
			if( handle != NULL && \
					node->task_handle == handle && \
					node->task_event == event \
			  )
			{
				//list_node_del( pos1 );
				timer_node_del( node );
				mem_free( (uint8_t *)node );
				MCU_EXIT_CRITICAL();
				return TRUE;
			}
		}

	}
	MCU_EXIT_CRITICAL();
	return ret;
}

tick_type_t timer_caculate()
{
	return get_scheduler()->tick_count;
}

int8_t timer_run()
{
	
	MCU_ENTER_CRITICAL();
	{
		tick_type_t cur_tick = timer_caculate();
		if( list_is_empty( &(g_timer.node.list ) ) == FALSE )
		{
			list_head_t *pos1, *pos2;
			list_for_each_safe( pos1, pos2, &(g_timer.node.list) )
			{
				timer_node_t *node = list_entry( pos1, timer_node_t, list );
				tick_type_t tick = cur_tick - node->start_tick;
				if( tick >= node->wake_ticks )
				{
					/*已经超时, 
					 * (1), 设置相应事件 
					 * (2), 判断是否周期, 
					 *      	是，则重新开启定时事件
					 *			否，则进行删除
					 * (3),
					 */
					 get_scheduler()->set_event( node->task_handle, node->task_event );
					 if( node->reload_flag )
					 {
						timer_start_event( node->task_handle, node->task_event, 1, node->wake_ticks );
					 }
					 else
					 {
						timer_node_del( node );
					 }
				}
				else
					break;
			}
		}
	}
	MCU_EXIT_CRITICAL();

    return 0;
}


timer_t * get_timer()
{
	return &g_timer;
}


