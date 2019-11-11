#ifndef _SEM_H
#define _SEM_H

#include "type.h"
#include "condef.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef enum {
	SEM_MUTEX,		/*互斥量*/
	SEM_COUNTER,	/*计数量*/
}sem_type_t;

typedef enum {
	SEM_STATE_NONE,
	SEM_STATE_DELETE,
}sem_state_t;

typedef struct _sem
{
	list_head_t list;
	sem_state_t state;  /*任务状态*/
	sem_type_t type;
	uint16_t value;
	
}sem_t;


extern int8_t sem_create( sem_t *sem, sem_type_t type, uint16_t value );
#define sem_metux_create( sem ) \
				sem_create( (sem), SEM_MUTEX, 1 )
#define sem_counter_create( sem, value) \
				sem_create( (sem), SEM_COUNTER, (value) )

extern int8_t sem_delete( sem_t *sem );
extern int8_t sem_pend( sem_t *sem, tick_type_t time_out );
extern int8_t sem_post( sem_t *sem );

#ifdef __cplusplus
}
#endif 

#endif /*_SEM_H*/

