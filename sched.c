#include "sched.h"
#include "mcu.h"
#include "mem_manager.h"
#include "errno.h"
#include "usart.h"


#define OPEN_SCHED_PRIORITY_DEBUGx

#ifndef OPEN_SCHED_PRIORITY_DEBUG
#define SCHED_SET_PRIORITY(x, y)			( (y) |= (1ul << (x)) )
#define SCHED_RESET_PRIORITY(x, y)			( (y) &= ~(1ul << (x)) )
#else
#define PRIORITY_PRINTF(x)	printf("%s, %d, %u \r\n", __func__, __LINE__, (x) )
#define SCHED_SET_PRIORITY(x, y)			( (y) |= (1ul << (x)) );PRIORITY_PRINTF(y)
#define SCHED_RESET_PRIORITY(x, y)			( (y) &= ~(1ul << (x)) );PRIORITY_PRINTF(y)
#endif


#ifdef OPEN_SHCDE_DEBUG
void scheduler_task_info_show( void );	
#endif


static void scheduler_task_init(
									task_handle_t * const handler,
									task_fun_t task_fun,
									const char * const name,
									const uint32_t stack_depth,
									void * const parameters,
									uint32_t priority,
									tcb_t *tcb
								);

int8_t scheduler_task_create( 
									   task_handle_t *task_handle, 
									   task_fun_t task_fun, 
									   const char * const name,
									   const uint16_t stack_depth,
									   void *const parameters,
									   uint32_t priority
									);
int8_t scheduler_task_delete( task_handle_t task_handle );


int8_t scheduler_task_event_create( 
									   task_handle_t *task_handle, 
									   task_event_fun_t task_event_fun, 
									   const char * const name,
									   const uint16_t stack_depth,
									   uint32_t priority,
									   task_event_add_fun_t add,
									   task_event_del_fun_t del
									   );
int8_t scheduler_task_event_delete( task_handle_t task_handle );

uint32_t scheduler_task_set_event( task_handle_t handle, uint32_t event );
uint32_t scheduler_task_clear_event( task_handle_t handle, uint32_t event );


void scheduler_task_delay(        	const tick_type_t time_tick	);
void scheduler_task_delay_until( tick_type_t * const per_wake_time, const tick_type_t time_tick );
int8_t scheduler_task_schde(void);
int8_t scheduler_init( void );
int8_t scheduler_task_tick_inc( void );
tcb_t* scheduler_get_current_tcb( void );
void scheduler_task_switch_context( void );
void scheduler_task_suspend( task_handle_t handle );
void scheduler_task_resume( task_handle_t handle );
int8_t scheduler_task_pop_queue( list_head_t *head );
int8_t scheduler_task_push_queue( list_head_t *head );

static void scheduler_task_idel( void * parameters );
static void scheduler_task_add_to_ready_list( tcb_t *tcb );
static void scheduler_suspend( void );
static void scheduler_resume( void );        
static void scheduler_task_event_process( void *pamas );


//static scheduler_task_stack_init_t stack_init = NULL;

static scheduler_t g_scheduler =
{
	.idle_tcb = NULL,
	//.current = NULL, 												
	.bmp_priority = 0,
	.tick_count = 0,
	//.task_list_block = { &(g_scheduler.task_list_block), &(g_scheduler.task_list_block) },		
	.task_list_delay = { &(g_scheduler.task_list_delay), &(g_scheduler.task_list_delay) },	
	.task_list_suspend = { &(g_scheduler.task_list_suspend), &(g_scheduler.task_list_suspend) },	
	.task_list_waiting_termination = { &(g_scheduler.task_list_waiting_termination), \
											&(g_scheduler.task_list_waiting_termination) },
	.task_delay = scheduler_task_delay,								
	.task_delay_until = scheduler_task_delay_until,					

	.task_pop_queue = scheduler_task_pop_queue,
	.task_push_queue = scheduler_task_push_queue,

	.set_event	= scheduler_task_set_event,			
	.clear_event = scheduler_task_clear_event,		
	.task_event_create = scheduler_task_event_create,
	.task_event_delete = scheduler_task_event_delete,
	
	.task_create = scheduler_task_create,							
	.task_delete = scheduler_task_delete,
	.schde = scheduler_task_schde,									
	.init =scheduler_init,
	.task_tick_inc = scheduler_task_tick_inc,
	.task_switch_context = scheduler_task_switch_context,
#ifdef OPEN_SHCDE_DEBUG
	.task_info_show = scheduler_task_info_show,
#endif	
	.task_suspend = scheduler_task_suspend,
	.task_resume = scheduler_task_resume,
	.timer = NULL,
};

/*调度器运行状态*/
static volatile uint8_t g_scheduler_running = FALSE;

/*调度器挂起状态*/
static volatile uint8_t g_scheduler_suspended = TRUE;

/*当前运行任务*/
tcb_t *current_tcb = NULL;


static void scheduler_suspend()
{
	g_scheduler_suspended++;
}

static void scheduler_resume()
{
   g_scheduler_suspended--;
}


void scheduler_task_delay( const tick_type_t time_tick	)
{
	if( time_tick > 0)
	{
		scheduler_suspend();
		//SCHED_ENTER_CRITICAL();
		{
			tcb_t *tcb = current_tcb;
			//list_node_del( &(tcb->list) );
			
			tcb->wake_tick = time_tick;
			tcb->suspend_time_tick = g_scheduler.tick_count;
			//tcb->state = TASK_BLOCK;
			tcb->state = TASK_DELAY;

			/*添加到延时链表*/
			task_list_add( &(g_scheduler.task_list_delay) , tcb );
			if( list_is_empty( &(g_scheduler.task_list_ready[tcb->priority]) ) == TRUE )
			{
				/*该优先没有任务, 更新优先级位图*/
				//printf("%s, %s, update bmp_priority :%d \r\n", __func__, tcb->name, tcb->priority);
				SCHED_RESET_PRIORITY( tcb->priority, g_scheduler.bmp_priority );
			}
		}
        scheduler_resume();
		//SCHED_ENTER_CRITICAL();
		SCHED_YIELD();
	}
	
}


void scheduler_task_delay_until( tick_type_t * const per_wake_time, const tick_type_t time_tick )
{

}

/*
 *  
 */
int8_t scheduler_task_push_queue( list_head_t *head )
{
	if( head == NULL )
		return EINVALID_PARAMETERS;
	{
		tcb_t *tcb = current_tcb;
		list_add_tail( &(tcb->event_list), head );
		get_scheduler()->task_suspend( tcb );
	}
    return TRUE;
}

int8_t scheduler_task_pop_queue( list_head_t *head )
{
	if( head == NULL )
		return EINVALID_PARAMETERS;
	else
	{
		/*删除首个队列元素, 并将其添加到就绪链表*/
		if( list_is_empty( head ) == FALSE )
		{
			list_head_t *first_list = head->next ;
			list_node_del( first_list );
			tcb_t *tcb = list_entry( first_list, tcb_t, event_list );
			
			if( tcb->state == TASK_BLOCK )
				get_scheduler()->task_resume( tcb );
			else if( tcb->state == TASK_DELAY )
			{
				scheduler_task_add_to_ready_list( tcb );
			}
		}

		return TRUE;
	}
}

static void scheduler_task_add_to_ready_list( tcb_t *tcb )
{
	//uint32_t pri = 0;
	SCHED_ENTER_CRITICAL();
	{
		
		if(	current_tcb == NULL	)
		{
			current_tcb = tcb;
		}
		else
		{
			if(g_scheduler_running == FALSE )
			{
				
				if( current_tcb->priority <= tcb->priority )
				{
					current_tcb = tcb;	
				}
			}
		}

		SCHED_SET_PRIORITY( tcb->priority, g_scheduler.bmp_priority );

		/*将就绪状态任务 ，添加到任务链表*/
		//if(tcb->state == TASK_READY)
		tcb->state = TASK_READY;
		list_add_tail( &(tcb->list) , &(g_scheduler.task_list_ready[tcb->priority]) );

	}
	SCHED_EXIT_CRITICAL();

	if(	g_scheduler_running == TRUE && \
			current_tcb->priority < tcb->priority )
	{
		SCHED_YIELD();
	}
}

int8_t scheduler_task_tick_inc( void )
{
	int8_t ret = FALSE;
	//printf("%s , g_scheduler_suspended :%d \r\n", __func__ , g_scheduler_suspended);
	
	if( g_scheduler_suspended == FALSE )
	{
		//printf("%s \r\n", __func__ );
		g_scheduler.tick_count++;
		if( !list_is_empty( &(g_scheduler.task_list_delay) ) ) 
		{
			list_head_t *pos1, *pos2;
			list_for_each_safe( pos1, pos2, &(g_scheduler.task_list_delay) )
			{
				tcb_t * ltcb = list_entry( pos1, tcb_t, list );

				//偏移时间, 满足溢出情况
				tick_type_t tick = g_scheduler.tick_count - ltcb->suspend_time_tick;
				
				if(  tick >= ltcb->wake_tick )
				{
					//printf("wake up task \r\n");
					ltcb->suspend_time_tick = 0;
					ltcb->wake_tick = 0;
					list_node_del( &(ltcb->list) );
					scheduler_task_add_to_ready_list( ltcb );
				}
				else
					break;
			}	
		}
		/*重新调度*/
		list_head_t  *task_list = &( g_scheduler.task_list_ready[ current_tcb->priority ] );
		if(list_length( task_list ) > 1)
		{
			ret = TRUE;
		}

		if( get_scheduler()->timer != NULL )
			get_scheduler()->timer->run();
	}

	return ret;
}

/*
 *	(1), 先将当前任务,添加到对应优先级的就绪任务链表尾部。
 *  (2), 从最高优先级的就绪链表中,寻找最当前第一个任务, 
 * 		 并将其值赋给current_tcb全局变量, 而且从就绪链表删除。
 */
void scheduler_task_switch_context( void )
{
	//printf("%s \r\n", __func__ );
	if(g_scheduler_suspended == FALSE)
	{
		uint32_t pri ;
		/*获取当前最高优先级*/
        if( current_tcb->state == TASK_RUNNING )
        {
            list_add_tail( &(current_tcb->list), &(g_scheduler.task_list_ready[current_tcb->priority]) );
            current_tcb->state = TASK_READY;	
        }
		
        SCHED_GET_HIHGEST_PRIORITY( pri, g_scheduler.bmp_priority );
		if( !list_is_empty( &(g_scheduler.task_list_ready[pri]) ) ) 
		{
			/*取将要运行的任务链表*/
			
			list_head_t *head = &(g_scheduler.task_list_ready[pri]);
			list_head_t *llist = head->next;
			if( llist != head )
			{
				tcb_t *tcb = list_entry( llist, tcb_t, list );
				tcb->state = TASK_RUNNING;
				current_tcb = tcb;
				list_node_del( llist );
			}
		}
	}
}

tcb_t* scheduler_get_current_tcb()
{
	return current_tcb;
}

/*
 *	(1), handler : 任务句柄; task_fun : 任务函数; stack_depth : 栈深度,单位字;
 *		 parameters : 运行任务时传递的参数指针; priority : 任务优先级;   	 		 
 *		 tcb : 初始化任务结构体对象;
 *  (2), 该函数主要是初始化任务, 并将tcb的值作为句柄, 赋值给handler;
 */
static void scheduler_task_init(
									task_handle_t * const handler,
									task_fun_t task_fun,
									const char * const name,
									const uint32_t stack_depth,
									void * const parameters,
									uint32_t priority,
									tcb_t *tcb
								)
{
	stack_type_t *stack_top;

	/*初始化任务栈*/
	stack_top = tcb->stack_buffer + (stack_depth - 1);
	/*字节对齐*/
	stack_top = (stack_type_t *)( (stack_type_t)stack_top & (~( (stack_type_t) MCU_BYTE_ALIGNMENT_MASK) ) );

	/*初始化名字*/
	if(name != NULL)
	{
		for(uint8_t i = 0; i < ERTOS_MAX_TASK_NAME_LEN; i++)
		{
			tcb->name[i] = name[i];
			if(name[i] == '\0')
		 		break;
		}
		tcb->name[ERTOS_MAX_TASK_NAME_LEN - 1] = '\0';
	}
	
	/*初始优先级*/
	tcb->priority = (priority >= ERTOS_MAX_PRIORITIES) ? \
					(ERTOS_MAX_PRIORITIES - 1) : \
					(priority);

	/*初始化状态*/
	tcb->state = TASK_READY;

	tcb->stack_top = mcu_stack_init( stack_top, task_fun, parameters );						

	
	tcb->suspend_time_tick = 0;
	tcb->wake_tick = 0;
	tcb->task_event = 0;
	tcb->event_fun = NULL;
	tcb->event_add_fun = NULL;
	tcb->event_del_fun = NULL;
	
	if( handler != NULL )
	{
		*handler = tcb;
	}
	
}

/*
 *  任务挂起, handle : 任务句柄
 */
void scheduler_task_suspend( task_handle_t handle )
{

	tcb_t *tcb = NULL;
	
	SCHED_ENTER_CRITICAL();
	{

		tcb = ( handle == NULL )?( current_tcb ):( (tcb_t *) handle );
		if( tcb->state != TASK_RUNNING )
		{/*不为运行状态，则需从链表中删除*/
			list_node_del( &(tcb->list) );
		}
		
		tcb->state = TASK_BLOCK;
		list_add_tail( &(tcb->list), &(g_scheduler.task_list_suspend) );

		if( list_is_empty( &(g_scheduler.task_list_ready[tcb->priority])) )
		{
			SCHED_RESET_PRIORITY( tcb->priority, g_scheduler.bmp_priority );
		}
	}
	SCHED_EXIT_CRITICAL();

	if( tcb == current_tcb )
	{
		if( g_scheduler_running == TRUE 
				&& g_scheduler_suspended == FALSE)
		{
			SCHED_YIELD();
		}		
	}
	

}

/*
 * 任务恢复, handle : 任务句柄
 */
void scheduler_task_resume( task_handle_t handle )
{
	tcb_t *tcb = (tcb_t *)handle;

	if( tcb != NULL && tcb != current_tcb && tcb->state == TASK_BLOCK )
	{
		SCHED_ENTER_CRITICAL();
		{
			if( !list_is_empty( &(g_scheduler.task_list_suspend) ) )
			{/*如果挂起链表不为空*/
				list_node_del( &(tcb->list) );

				tcb->state = TASK_READY;
				tcb->suspend_time_tick = 0;
				tcb->wake_tick = 0;
				
				list_add_tail( &(tcb->list), &(g_scheduler.task_list_ready[tcb->priority]) );
				SCHED_SET_PRIORITY( tcb->priority, g_scheduler.bmp_priority );

				if( tcb->priority >= current_tcb->priority ) 
				{
					SCHED_YIELD();
				}
			}

		}
		SCHED_EXIT_CRITICAL();
	}

}

/*
 *  (1), 任务创建
 *  (2), task_handle : 返回句柄; task_fun : 任务函数; name : 任务名字
 *   	 stack_depth : 任务栈, 单位字; parameters : 任务传递参数;
 *       priority : 任务优先级;
 */
int8_t scheduler_task_create ( 
										   task_handle_t *task_handle, 
										   task_fun_t task_fun, 
										   const char * const name,
										   const uint16_t stack_depth,
										   void *const parameters,
										   uint32_t priority
									 )
{
	tcb_t *tcb = NULL;
	int8_t ret = FALSE;
	stack_type_t * stack = NULL;

	SCHED_ENTER_CRITICAL();
	
	stack = (stack_type_t *)mem_alloc( stack_depth * sizeof(stack_type_t) );
	ERTOS_ASSERT(stack);
	if( stack != NULL )
	{
		tcb = (tcb_t *)mem_alloc( sizeof(tcb_t) );
		if(tcb)
			tcb->stack_buffer = stack;
		else
			mem_free( (uint8_t *)stack );
	}

	if(tcb != NULL)
	{
		scheduler_task_init( task_handle , task_fun, name, \
							 stack_depth, parameters , priority, \
							 tcb 
						   );
		scheduler_task_add_to_ready_list( tcb );
		ret = SUCCESS;
	}
	else
	{
		ret = ECOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
	}

	SCHED_EXIT_CRITICAL();
	
	return ret;

}

/*
 *  任务删除, task_handle : 任务句柄
 */
 int8_t scheduler_task_delete( task_handle_t task_handle )
 {
	int8_t ret = FALSE;
	tcb_t *tcb = ( task_handle == NULL )?( current_tcb ):( (tcb_t *) task_handle );

	if( tcb->state != TASK_STOP )
	{
		//printf("%s \r\n", __func__);
		SCHED_ENTER_CRITICAL();
		
		if( tcb !=  current_tcb )
		{
			if( list_is_empty( &(g_scheduler.task_list_ready[tcb->priority]) ) == TRUE )
			{
				SCHED_RESET_PRIORITY( tcb->priority, g_scheduler.bmp_priority );
			}
			
			list_node_del( &(tcb->list) );
			list_node_del( &(tcb->event_list) );
			mem_free( (uint8_t *)tcb->stack_buffer );
			mem_free( (uint8_t *)tcb );
		}
		else
		{/*删除正在运行的任务, 将其移到待终止链表*/
			tcb->state = TASK_STOP;
			list_add_tail( &(tcb->list), &(g_scheduler.task_list_waiting_termination) );
			if( list_is_empty( &(g_scheduler.task_list_ready[tcb->priority]) ) )
			{/*为空，则清位优先级位图变量*/
				//printf("%s, name :%s, pri :%u \r\n", tcb->name, tcb->priority );
				SCHED_RESET_PRIORITY( tcb->priority, g_scheduler.bmp_priority );
			}
		}	 
		SCHED_EXIT_CRITICAL();
		
	}	
	 
	 if(g_scheduler_running == TRUE \
	 		&& tcb == current_tcb)
	 {
		/*重新调度*/
	 	SCHED_YIELD();
	 }
	 ret = TRUE;
	 return ret;
 }


/*
 *	(1), 任务事件创建;
 *  (2), task_handle : 任务句柄; even_fun : 任务函数; name : 任务事件名字;
 *  	 statck_depth : 栈深度; priority : 任务优先级;
 		 add : 任务事件添加回调函数; del : 任务事件删除回调函数;
 */
 int8_t scheduler_task_event_create( 
										task_handle_t *task_handle, 
										task_event_fun_t task_event_fun, 
										const char * const name,
										const uint16_t stack_depth,
										uint32_t priority,
										task_event_add_fun_t add,
										task_event_del_fun_t del
										)
{
	tcb_t *tcb = NULL;
	SCHED_ENTER_CRITICAL();
	{
		get_scheduler()->task_create( task_handle, scheduler_task_event_process, \
								  	  name, \
								      stack_depth, \
								  	  NULL, priority
								 	);
		tcb = (tcb_t *)(*task_handle);
		tcb->event_fun = task_event_fun;
		tcb->event_add_fun = add;
		tcb->event_del_fun = del;
	}
	SCHED_EXIT_CRITICAL();
    return TRUE;
}

/*
 *  任务事件删除, task_handle : 任务句柄;
 */
int8_t scheduler_task_event_delete( task_handle_t task_handle )
{
	tcb_t *tcb = NULL;
	SCHED_ENTER_CRITICAL();
	{
		tcb = (tcb_t *)(task_handle);
		if( tcb != NULL )
		{
			/*执行删除回调函数*/
			if( tcb->event_del_fun != NULL )
				tcb->event_del_fun();
			
			tcb->event_add_fun = NULL;
			tcb->event_del_fun = NULL;
			tcb->event_fun = NULL;
			tcb->task_event = 0;
			get_scheduler()->task_delete( task_handle );
		}
	}
	SCHED_EXIT_CRITICAL();
	return TRUE;
}

/*
 *  返回非event事件,则表示不成功0;
 */
uint32_t scheduler_task_set_event( task_handle_t handle, uint32_t event )
{
	if( handle == NULL)
		return 0;

	SCHED_ENTER_CRITICAL();
	{
		task_set_event( handle, event );
		if(event)
			get_scheduler()->task_resume( (tcb_t *)handle );
	}
	SCHED_EXIT_CRITICAL();
    return ((tcb_t *)handle)->task_event;  
}

/*
 *  任务事件标记位清除
 */
uint32_t scheduler_task_clear_event( task_handle_t handle, uint32_t event )
{
	if( handle == NULL)
		return 0;
	SCHED_ENTER_CRITICAL();
	{
		task_clear_event( handle, event );
	}
	SCHED_EXIT_CRITICAL();
	return ((tcb_t *)handle)->task_event;   
}

/*
 *  空闲任务函数, 主要是判断当前是最早唤醒的任务或定时事件，设置
 *  进低功耗, 处理被删除的任务。
 */
static void scheduler_task_idel(void * parameters)
{
	(void) parameters;
	while(1)
	{
		//printf("%s \r\n", __func__);
		//scheduler_suspend();
		while( !list_is_empty( &(g_scheduler.task_list_waiting_termination) ) )
		{/*删除终止任务*/
			SCHED_ENTER_CRITICAL();
			list_head_t *next = g_scheduler.task_list_waiting_termination.next;
			//if( next != g_scheduler.task_list_waiting_termination )
			{
               
				list_node_del( next );
				tcb_t *tcb = list_entry( next, tcb_t, list);
				list_node_del( &(tcb->event_list) );
                printf("%s ,free :%s\r\n", __func__, tcb->name);
				mem_free( (uint8_t *)tcb->stack_buffer );
				mem_free( (uint8_t *)tcb );
				
			}
			SCHED_EXIT_CRITICAL();
		}		

		{ /*计算睡眠时间*/
			tick_type_t task_sleep_tick = 0;
			tick_type_t cur_tick ;
			cur_tick = get_scheduler()->tick_count;
			if( list_is_empty( &(g_scheduler.task_list_delay) ) == FALSE )
			{
				list_head_t *first_head = NULL;
				tcb_t *tcb = NULL;

				first_head = g_scheduler.task_list_delay.next;
				tcb = list_entry( first_head, tcb_t, list);
				
				task_sleep_tick = tcb->wake_tick - ( cur_tick - tcb->suspend_time_tick );
			}
			
			tick_type_t timer_sleep_tick = 0;
			timer_node_t *node = &((get_scheduler()->timer)->node);
			if( list_is_empty( &(node->list))  == FALSE )
			{
				list_head_t *first_head = NULL;
				timer_node_t *lnode = NULL;

				first_head = (node->list).next;
				lnode = list_entry( first_head, timer_node_t, list );
				timer_sleep_tick = lnode->wake_ticks - (cur_tick - lnode->start_tick );
			}

			tick_type_t tick = ( task_sleep_tick <= timer_sleep_tick ) ? \
			                   ( task_sleep_tick ) : \
			                   ( timer_sleep_tick );
			if( tick > 2)
				mcu_sleep( tick );
		}
		//scheduler_resume();
		
	}
}

#ifdef OPEN_SHCDE_DEBUG
/*
 *  调试函数
 */
void scheduler_task_info_show(void)
{
	list_head_t *pos1, *pos2;
	scheduler_t *sched = &g_scheduler; 
	tcb_t *tcb = NULL;
	
	printf("**************SCHEDULER*********************\r\n");
	tcb = current_tcb;
	printf("bmp priority :%u\r\n", sched->bmp_priority );
	printf("curent task name :%s\r\n", tcb->name);
	printf("curent task priority :%u\r\n", tcb->priority);
	printf("curent task state :%u\r\n", tcb->state);
	printf("suspend time :%u , wake time :%u\r\n", tcb->suspend_time_tick, tcb->wake_tick);
	printf("suspend :%d, running :%d\r\n", g_scheduler_suspended, g_scheduler_running);
	printf("********************************************\r\n\r\n");	

	for(int8_t i = 0; i < ERTOS_MAX_PRIORITIES; i++)
	{
		list_for_each_safe( pos1, pos2, &(g_scheduler.task_list_ready[i]) )
		{
			tcb = list_entry( pos1, tcb_t, list );
			printf("**************TASK LIST READY***************\r\n");
			printf("name :%s\r\n", tcb->name );
			printf("priority :%u\r\n", tcb->priority );
			printf("state :%u\r\n", tcb->state);
			printf("suspend time :%u , wake time :%u\r\n", tcb->suspend_time_tick, tcb->wake_tick);
			printf("********************************************\r\n\r\n");
		}
	}
	list_for_each_safe( pos1, pos2, &(g_scheduler.task_list_delay) )
	{
		tcb = list_entry( pos1, tcb_t, list );
		printf("**************TASK LIST DELAY***************\r\n");
		printf("name :%s\r\n", tcb->name );
		printf("priority :%u\r\n", tcb->priority );
		printf("state :%u\r\n", tcb->state);
		printf("suspend time :%u , wake time :%u\r\n", tcb->suspend_time_tick, tcb->wake_tick);
		printf("********************************************\r\n\r\n");
	}

	
}
#endif

/*
 *  任务事件处理函数, 清除和传递事件标记位
 */
static void scheduler_task_event_process( void *pamas )
{
	
	tcb_t *tcb = NULL;
	uint32_t event = 0;
	printf("%s \r\n", __func__ );

	tcb = current_tcb;
	if( tcb != NULL && tcb->event_add_fun != NULL )
	{
		/*执行添加回调函数*/
		tcb->event_add_fun();
	} 	
		
	while(1)
	{
		//tcb = current_tcb;
		if( g_scheduler_running == TRUE )
		{
			if(tcb != NULL && tcb->task_event != 0)
			{
				SCHED_ENTER_CRITICAL();
				{
					event = tcb->task_event;
					tcb->task_event = 0;
				}
				SCHED_EXIT_CRITICAL();
				tcb->event_fun( (task_handle_t )tcb,  event );
			}
			else
			{
				get_scheduler()->task_suspend(tcb);
			}
		}
	}
}

/*
 *  调度器初始化函数
 */
int8_t scheduler_init(void)
{
	int8_t ret = FALSE;
    mem_init();

	g_scheduler.timer = get_timer();
	
	/*初始化就绪任务链表*/
	for( int8_t i = 0; i < ERTOS_MAX_PRIORITIES; i++ )
	{
		list_init( &(g_scheduler.task_list_ready[i]) );
	}

	ret = TRUE;
	return ret;
}

/*
 *  开始调度函数
 */
int8_t scheduler_task_schde()
{
	int8_t ret = FALSE;
	

	/*初始化空闲任务链表*/
	ret = scheduler_task_create(	NULL,  scheduler_task_idel, "idle_task", \
									ERTOS_MINIMAL_STACK_SIZE, NULL, TASK_IDLE_PRIORITY );

	/*第一个运行任务移出就绪链表*/
	current_tcb->state = TASK_RUNNING;
	if( !list_is_empty( &(current_tcb->list) ) )
		list_node_del( &(current_tcb->list) );
		
	g_scheduler_running = TRUE;
	g_scheduler_suspended = FALSE;
	
	SCHED_START();
	
	return ret;
}

scheduler_t * get_scheduler(void)
{
	return &g_scheduler;
}




