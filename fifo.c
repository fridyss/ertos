#include "fifo.h"
#include "mem_manager.h"
#include "log2.h"
#include "debug.h"
#include "string.h"

fifo_t *fifo_alloc( uint16 item_nr, uint16_t item_size )
{
	fifo_t *fifo;
	uint16_t size = 0;
	
	if( (item_nr & (item_nr - 1)) )
	{
		uint8 bits = sizeof(item_nr)*8;
		ASSERT( item_nr < (1 << (bits - 1)) );
		item_nr = roundup_pow_of_two(item_nr);
	}

	size = sizeof(fifo_t) + item_nr*item_size;
	fifo = ( fifo_t *)mem_alloc( size ); 

	if(!fifo)
	{
		ASSERT(fifo);
		return NULL;
	}

	fifo->in = fifo->out = 0;
	fifo->item_nr = item_nr;
	fifo->item_size = item_size;

	return fifo;	
}

void fifo_free( fifo_t *fifo )
{
	if(!fifo)
	{
		ASSERT(fifo);
		return ;
	}

	mem_free( (uint8_t *)fifo );
	return ;
}

uint16_t fifo_put( fifo_t *fifo, uint8_t *buffer, uint16_t item_nr )
{
	uint16_t nr;
	uint8_t *fb;

	fb = (uint8_t *)(fifo + 1);
	//fb = fifo->buffer;

	/*最max 可以分配个数*/
	item_nr = min( item_nr , fifo->item_nr - (fifo->in - fifo->out) );

	/*分段写入*/
	nr = min( item_nr , fifo->item_nr - ( fifo->in & (fifo->item_nr - 1) ));
	memcpy( fb + (fifo->in & (fifo->item_nr - 1))*fifo->item_size , buffer, nr*fifo->item_size );
	memcpy( fb, buffer + nr*fifo->item_size, (item_nr - nr)* fifo->item_size );
	
	fifo->in += item_nr;
	return item_nr;
}

uint16_t fifo_get( fifo_t *fifo, uint8_t *buffer, uint16_t item_nr )
{
	uint16_t nr;
	uint8_t *fb;
	
	fb = (uint8_t *)(fifo + 1);
	//fb = fifo->buffer;
	item_nr = min( item_nr , (fifo->in - fifo->out) );	
	nr = min( item_nr , fifo->item_nr - (fifo->in & fifo->item_nr - 1) );
	memcpy( buffer, fb + (fifo->out & fifo->item_nr - 1)*fifo->item_size, nr*fifo->item_size );
	memcpy( buffer + nr*fifo->item_size, fb, (item_nr - nr)*fifo->item_size );
	
	fifo->out += item_nr;
	return item_nr;
}

uint16_t fifo_peek( fifo_t *fifo, uint8_t *buffer, uint16_t item_nr )
{
	uint16_t nr;
	uint8_t *fb;
	
	fb = (uint8_t *)(fifo + 1);
	item_nr = min( item_nr , (fifo->in - fifo->out) );	
	nr = min( item_nr , fifo->item_nr - (fifo->in & fifo->item_nr - 1) );
	memcpy( buffer, fb + (fifo->out & fifo->item_nr - 1)*fifo->item_size, nr*fifo->item_size );
	memcpy( buffer + nr*fifo->item_size, fb, (item_nr - nr)*fifo->item_size );
	
	//fifo->out += item_nr;
	return item_nr;
}

uint16_t fifo_length( fifo_t *fifo )
{
	return (fifo->in - fifo->out);
}

int8_t fifo_is_full( fifo_t *fifo )
{
	return ( (fifo->in - fifo->out) ==  fifo->item_nr );
}

int8_t fifo_is_empty( fifo_t *fifo )
{
	return (fifo->in == fifo->out);
}


