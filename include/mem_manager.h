#ifndef __MEM_MENAGER_H
#define __MEM_MENAGER_H

#include "type.h"
#include "stdio.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MEM_POOL_SIZE    (46*1024)

typedef enum {USED, UNUSED}mem_flag_t;

typedef struct _mem_node
{
	list_head_t list;
	mem_flag_t ifused;
	uint16 mem_size;
#ifdef  OPEN_MEM_MANAGE_CALL_INFO
	char fun[32];
	int line;
#endif

}mem_node_t;

#ifdef OPEN_MEM_MANAGE_CALL_INFO
extern  void *mem_alloc_debug(uint16 size, char *fun, int line);
#define mem_alloc(size)		mem_alloc_debug((size), __FUNCTION__, __LINE__)
#else
extern void *mem_alloc(uint16 size); 
#endif

extern int8 mem_free(uint8* ptr);
extern int8 mem_init(void);


#ifdef __cplusplus
}
#endif


#endif /*__MEM_MENAGER_H*/
