#include "sem.h"
#include "sched.h"

int8_t sem_create( sem_t *sem, sem_type_t type, uint16_t value )
{
	int8_t ret = FALSE; 

	if( sem == NULL )
		return ret;
	
	SCHED_ENTER_CRITICAL();
	{
		sem->type = type;
		sem->value = value;
		sem->state = SEM_STATE_NONE;
		list_init( &sem->list );
	}
	SCHED_EXIT_CRITICAL();
	return ret;
}

int8_t sem_delete( sem_t *sem )
{
	//int8_t ret = FALSE;
	SCHED_ENTER_CRITICAL();
	{	
		/*设置信号标记位, 并弹起挂起任务列表*/
		sem->state = SEM_STATE_DELETE;
		list_head_t *pos1, *pos2;
		list_for_each_safe( pos1, pos2, &sem->list )
		{
			get_scheduler()->task_pop_queue( &(sem->list) );
		}
	}
	SCHED_EXIT_CRITICAL();
	return TRUE;
}

int8_t sem_pend( sem_t *sem, tick_type_t time_out )
{
	//int8_t ret = FALSE;

	if( sem == NULL)
		return FALSE;
	while( sem->state != SEM_STATE_DELETE )
	{
		SCHED_ENTER_CRITICAL();
		{
			if( sem->value > 0 )
			{
				--sem->value;
				SCHED_EXIT_CRITICAL();
				return TRUE;
			}

			if( time_out == 0 )
			{
				SCHED_EXIT_CRITICAL();
				return FALSE;
			}

			if( time_out == MCU_MAX_DELAY )
			{	
				get_scheduler()->task_push_queue( &(sem->list) );
			}
		}
		SCHED_EXIT_CRITICAL();
	}
	return FALSE;
}

int8_t sem_post( sem_t *sem )
{
	if( sem == NULL)
		return FALSE;
	
	SCHED_ENTER_CRITICAL();
	{
		++sem->value;
		get_scheduler()->task_pop_queue( &(sem->list) );
	}
	SCHED_EXIT_CRITICAL();

	return FALSE;

}

