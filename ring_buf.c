#include "ring_buf.h"
#include "mcu.h"
#include "mem_menager.h"


int8 ring_buf_is_full( ring_buf_t *ring_buf );

ring_buf_t * ring_buf_alloc( uint16_t max_size )
{
	ring_buf_t *ring_buf = (ring_buf_t)mem_alloc( sizeof(ring_buf_t) );
	ring_buf->max_size = max_size;
	ring_buf->seek = 0;
	list_init( &ring_buf->node_list );
	return ring_buf;
}

err_t ring_buf_push(     ring_buf_t *ring_buf, ring_buf_node_t *node )
{
	if( ring_buf == NULL || node == NULL )
		return -EERR;
	mcu_enter_critical();
	{
		if( !ring_buf_is_full(ring_buf) )
		{
			ring_buf->seek++;
			list_add_tail( &(node->list) , &(ring_buf->node_list) );
		}
		else
		{
			/*已满, 释放节点内存*/
			mem_free( (uint8*)node->buf );
			mem_free( (uint8*)node );
		}
	}
	mcu_exit_critical();
}

ring_buf_node_t *ring_buf_pop(      ring_buf_t *ring_buf )
{
	ring_buf_node_t *node = NULL;
	if(ring_buf == NULL)
		return NULL;
	
	if( !ring_buf_is_empty(ring_buf) )
	{
		mcu_enter_critical();
		{
			node = list_entry( ring_buf->node_list.next, ring_buf_node_t, list);
			list_node_del( node );
			ring_buf->seek--;
		}
		mcu_exit_critical();
	}
	return node;
}

int8 ring_buf_is_empty( ring_buf_t *ring_buf )
{
	if( ring_buf == NULL )
		return ring_buf->seek;
}

int8 ring_buf_is_full( ring_buf_t *ring_buf )
{
	if( ring_buf == NULL)
		return -EERR;
	return ring_buf->max_size == ring_buf->seek;
}


int16 ring_buf_num( ring_buf_t *ring_buf )
{

}





#endif /*__RING_BUF_H*/
