#include "queue.h"
#include "errno.h"
#include "mem_manager.h"
#include "sched.h"
#include "mcu.h"

uint16_t queue_length( queue_t *queue )
{
	return fifo_length( queue->fifo );
}

int8_t queue_is_full( queue_t *queue )
{
	return fifo_is_full( queue->fifo );
}

int8_t queue_is_empty( queue_t *queue )
{
	return fifo_is_empty(  queue->fifo );
}

queue_t *queue_create( uint16_t size, uint16_t item_size )
{
	queue_t* queue;
	fifo_t *fifo;

	queue = (queue_t *)mem_alloc( sizeof(queue_t) );  
	if( !queue )
		return NULL;
	
	fifo = fifo_alloc( size, item_size );
	if( !fifo )
	{
		mem_free( (uint8 *) queue);
		return NULL;
	}

	list_init( &(queue->list_wait_recv) );
	list_init( &(queue->list_wait_send) );
	queue->fifo = fifo;
	return queue;
	
}

int8_t queue_delete( queue_t *queue )
{
	int8_t ret = FALSE;
	if( queue != NULL)
	{
		fifo_free( queue->fifo );
		mem_free( (uint8 *)queue );
		ret = TRUE;	
	} 
	ret = EINVALID_PARAMETERS;
	return ret;
}

/*
 *  tick_to_wait : 0, 不阻塞; PORT_MAX_DELAY, 挂起; tick_to_wait, 最多等待时间 
 */
int8_t queue_send(queue_t *queue,  void * const item_value, tick_type_t tick_to_wait )
{
	//int8_t ret = FALSE;

	if( queue == NULL || queue->fifo == NULL)
		return EINVALID_PARAMETERS;
	
	while(1)
	{
		MCU_ENTER_CRITICAL();
		{
			if( queue_is_full(queue) == FALSE )
			{/*队列是否满*/	
				
				/*(1)数据入队列*/
				if( fifo_put( queue->fifo, (uint8_t *)item_value, 1) > 0 )
				{	
					/*(2)查看接收队列是否有任务, 有则唤醒最早进入的任务*/
					if( list_is_empty( &(queue->list_wait_recv) ) == FALSE )
					{
						get_scheduler()->task_pop_queue( &(queue->list_wait_recv) );
					}
				}
				MCU_EXIT_CRITICAL();
				return TRUE;
			}
			else
			{
				if( tick_to_wait == 0 )
				{
					/*(1), 不阻塞, 直接反回*/
					MCU_EXIT_CRITICAL();
					return EQUEUE_FULL;
				}
				else if( tick_to_wait == MCU_MAX_DELAY )
				{
                    /*(2), 挂起, 等待*/
					get_scheduler()->task_push_queue( &(queue->list_wait_send) );
				}
				/*(3), 等待一段时间, 进入延迟列表*/
			}
		}
		MCU_EXIT_CRITICAL();
	}
	
}

int8_t queue_send_from_isr(queue_t *queue, void * const item_value )
{
	//int8_t ret = FALSE;

	if( queue == NULL || queue->fifo == NULL)
		return EINVALID_PARAMETERS;
	
	uint32_t pir = MCU_SET_INTERRUPT_MASK_FROM_ISR();
	{
		if( queue_is_full(queue) == FALSE )
		{/*队列是否满*/	
			
			/*(1)数据入队列*/
			if(fifo_put( queue->fifo, (uint8_t *)item_value, 1) > 0)
			{	
				/*(2)查看接收队列是否有任务, 有则唤醒最早进入的任务*/
				if( list_is_empty( &(queue->list_wait_recv) ) == FALSE )
				{
					get_scheduler()->task_pop_queue( &(queue->list_wait_recv) );
				}
			}
			MCU_CLEAR_INTERRUPT_MASK_FROM_ISR(pir);
			return TRUE;
		}

	}
	MCU_CLEAR_INTERRUPT_MASK_FROM_ISR(pir);
	return EQUEUE_FULL;
}


int8_t queue_recv( queue_t *queue, void * const item_value, tick_type_t tick_to_wait )
{
	if( queue == NULL || queue->fifo == NULL )
		return EINVALID_PARAMETERS;
	while(1)
	{

		MCU_ENTER_CRITICAL();
		{
			if( queue_is_empty( queue ) ==  FALSE )
			{
				if( fifo_get( queue->fifo, (uint8_t *)item_value, 1) > 0 )
				{
					if( list_is_empty( &(queue->list_wait_send) ) == FALSE )
					{
						get_scheduler()->task_pop_queue( &(queue->list_wait_send) );
					}
				}
				MCU_EXIT_CRITICAL();
				return TRUE;
			}
			else
			{
				if( tick_to_wait == 0 )
				{
					MCU_EXIT_CRITICAL();
					return EQUEUE_EMPTY;
				}
				else if ( tick_to_wait == MCU_MAX_DELAY )
				{
					get_scheduler()->task_push_queue( &(queue->list_wait_recv) );
				}

			}
		}
		MCU_EXIT_CRITICAL();
    }
}


int8_t queue_recv_from_isr( queue_t *queue, void * const item_value )
{
	if( queue == NULL || queue->fifo == NULL )
		return EINVALID_PARAMETERS;
	uint32_t pir = MCU_SET_INTERRUPT_MASK_FROM_ISR();
	{
		if( queue_is_empty( queue ) ==  FALSE )
		{
			if( fifo_get( queue->fifo, (uint8_t *)item_value, 1) > 0 )
			{
				if( list_is_empty( &(queue->list_wait_send) ) == FALSE )
				{
					get_scheduler()->task_pop_queue( &(queue->list_wait_send) );
				}
			}
			MCU_CLEAR_INTERRUPT_MASK_FROM_ISR( pir );
			return TRUE;
		}
	}
	MCU_CLEAR_INTERRUPT_MASK_FROM_ISR( pir );
	return EQUEUE_EMPTY;
}    

int8_t queue_qeek( queue_t *queue, void * const item_value, tick_type_t tick_to_wait )
{
	if( queue == NULL || queue->fifo == NULL)
		return EINVALID_PARAMETERS;
	while(1)
	{

		MCU_ENTER_CRITICAL();
		{
			if( queue_is_empty( queue ) ==  FALSE )
			{
				fifo_peek( queue->fifo, (uint8_t *)item_value, 1 );
				MCU_EXIT_CRITICAL();
				return TRUE;
			}
			else
			{
				if( tick_to_wait == 0 )
				{
					MCU_EXIT_CRITICAL();
					return EQUEUE_EMPTY;
				}
				else if ( tick_to_wait == MCU_MAX_DELAY )
				{
					get_scheduler()->task_push_queue( &(queue->list_wait_recv) );
				}
			}
		}
		MCU_EXIT_CRITICAL();
    }

}

int8_t queue_qeek_from_isr( queue_t *queue, void * const item_value )
{
	if( queue == NULL || queue->fifo == NULL )
		return EINVALID_PARAMETERS;
	uint32_t pir = MCU_SET_INTERRUPT_MASK_FROM_ISR();
	{
		if( queue_is_empty( queue ) ==  FALSE )
		{
			fifo_get( queue->fifo, (uint8_t *)item_value, 1 );
			MCU_CLEAR_INTERRUPT_MASK_FROM_ISR(pir);
			return TRUE;
		}
	}
	MCU_CLEAR_INTERRUPT_MASK_FROM_ISR( pir );
	return EQUEUE_EMPTY;

}


