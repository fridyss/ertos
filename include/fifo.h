#ifndef _FIFO_H
#define _FIFO_H

#include "type.h"
#include "log2.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef min
#define	min(x, y)	(((x) < (y)) ? (x) : (y))
#endif


typedef struct _fifo
{
	uint16_t item_nr;			/*数据条目个数*/
	uint16_t item_size;			/*数据条目size*/
	uint16_t in;				/*写数据位置*/
	uint16_t out;				/*读数据位置*/
	//uint8_t buffer[0];		/*没有分配内存*/
}fifo_t;




extern fifo_t *fifo_alloc( uint16 item_nr, uint16_t item_size );
extern void fifo_free( fifo_t *fifo );
extern uint16_t fifo_put( fifo_t *fifo, uint8_t *buffer, uint16_t item_nr );
extern uint16_t fifo_get( fifo_t *fifo, uint8_t *buffer, uint16_t item_nr );
extern uint16_t fifo_peek( fifo_t *fifo, uint8_t *buffer, uint16_t item_nr );
extern uint16_t fifo_length( fifo_t *fifo );
extern int8_t fifo_is_full( fifo_t *fifo );
extern int8_t fifo_is_empty( fifo_t *fifo );


#ifdef __cplusplus
}
#endif


#endif /*_FIFO_H*/

