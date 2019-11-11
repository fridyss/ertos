#ifndef __QUEUE_H
#define __QUEUE_H

#include "type.h"
#include "list.h"
#include "condef.h"
#include "fifo.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _queue
{
	list_head_t list_wait_send;
	list_head_t list_wait_recv;
	fifo_t *fifo;
}queue_t;


extern queue_t *queue_create( uint16_t size, uint16_t item_size );
extern int8_t queue_delete( queue_t *queue );
extern int8_t queue_send(queue_t *queue, void * const item_value, tick_type_t tick_to_wait );
extern int8_t queue_send_from_isr(queue_t *queue, void * const item_value );
extern int8_t queue_recv( queue_t *queue, void * const item_value, tick_type_t tick_to_wait );
extern int8_t queue_recv_from_isr( queue_t *queue,  void * const item_value );
extern int8_t queue_peek( queue_t *queue, void * const item_value, tick_type_t tick_to_wait );
extern int8_t queue_peek_from_isr( queue_t *queue, void * const item_value );



#ifdef __cplusplus
}
#endif


#endif /*__QUEUE_H*/

