#include "mem_manager.h"
#include "string.h"
#include "list.h"



#define OPEN_MEM_MANAGE_DEBUGx

#ifdef OPEN_MEM_MANAGE_DEBUG
#define MEM_MANAGE_DEBUG        do{printf("%s, %s, %d \r\n", __FILE__, __func__, __LINE__);}while(0);
#else
#define MEM_MANAGE_DEBUG
#endif

static uint8 g_mem_pool[MEM_POOL_SIZE];
static mem_node_t *g_mem_node_head = NULL;


static int8 mem_fragment_combine(mem_node_t *node)
{

	if(node == NULL)
	{
		MEM_MANAGE_DEBUG
		return -1;
	}
	
	mem_node_t	*prev_node, *next_node;
	prev_node = list_entry(node->list.prev, mem_node_t, list);
	next_node = list_entry(node->list.next, mem_node_t, list);

	if(prev_node == NULL || next_node == NULL)
	{
		MEM_MANAGE_DEBUG
		return -1;
	}
	
	if(next_node->ifused == UNUSED)
	{
		node->mem_size += (next_node->mem_size + sizeof(mem_node_t));
		list_node_del(&(next_node->list));
	}

	if(prev_node->ifused == UNUSED)
	{
		prev_node->mem_size += (node->mem_size + sizeof(mem_node_t));
		list_node_del(&(node->list));
	}
	
	return 0;
}

#ifdef OPEN_MEM_MANAGE_CALL_INFO
void *mem_alloc_debug(uint16 size, char *fun, int line)
#else
void *mem_alloc(uint16 size)
#endif
{
	void *ret = NULL;
	list_head_t *pos1, *pos2;
#ifdef OPEN_MEM_MANAGE
	if(fun == NULL || g_mem_node_head == NULL || size <= 0)
#else
	if(g_mem_node_head == NULL || size <= 0)
#endif
	{
		MEM_MANAGE_DEBUG
		return NULL;
	}

	size = (size%MEM_ALIGN_BYTE)?\
			   (size - size%MEM_ALIGN_BYTE + MEM_ALIGN_BYTE):\
			   (size); 

	list_for_each_safe(pos1, pos2, &(g_mem_node_head->list))
	{
		mem_node_t *node = list_entry(pos1, mem_node_t, list);

		if(node->ifused == UNUSED)
		{
			if(node->mem_size - sizeof(mem_node_t) > size)
			{
				mem_node_t *new_node = (mem_node_t *)((uint8*)(node + 1) + size);
				new_node->mem_size = node->mem_size - sizeof(mem_node_t) - size; 
				new_node->ifused = UNUSED;
				
				#ifdef OPEN_MEM_MANAGE_CALL_INFO	
					memset(node->fun, 0, sizeof(node->fun));
					node->line = 0;
				#endif

				node->mem_size = size;
				node->ifused = USED;
				#ifdef OPEN_MEM_MANAGE_CALL_INFO	
					memset(node->fun, 0, sizeof(node->fun));

					uint8 fun_name_len = (strlen(fun) < sizeof(node->fun))?\
						 				 (strlen(fun)):\
						 				 (sizeof(node->fun) - 1);
					
					memcpy(node->fun, fun, fun_name_len);
					node->line = line;
					
				#endif
				
				if(list_add_tail( &(new_node->list), &(node->list) ) != 0)
				{
					MEM_MANAGE_DEBUG
					return NULL;
				}
				ret = (void *)(node + 1);

			}
			else if(node->mem_size == size || \
						(node->mem_size > size && node->mem_size - size <= sizeof(mem_node_t)) )
			{
				node->mem_size = size + (node->mem_size - size);
				node->ifused = USED;
				
				#ifdef OPEN_MEM_MANAGE_CALL_INFO	
				memset(node->fun, 0, sizeof(node->fun));
				
				uint8 fun_name_len = (strlen(fun) < sizeof(node->fun))? \
									 (strlen(fun)): \
									 (sizeof(node->fun) - 1);
				
				memcpy(node->fun, fun, fun_name_len);
				node->line = line;

				#endif
				ret = (void *)(node + 1);
			}
			/**/
		}
	}

#ifdef OPEN_MEM_MANAGE_DEBUG	
	printf("*********************************%s********************************************************\r\n",__func__);
#endif	
	uint16 idx = 0;
	list_for_each_safe(pos1, pos2, &(g_mem_node_head->list))
	{
		mem_node_t *node = list_entry(pos1, mem_node_t, list);

#ifdef OPEN_MEM_MANAGE_DEBUG	
			/*如果开启内存调试*/
			printf("mem pool block[%d], addr :%p, size :%d, ifused :%d (0 USED/1 UNUSED)\r\n", idx++, node, (node->mem_size), node->ifused);
		#ifdef OPEN_MEM_MANAGE_CALL_INFO
				//if(node->ifused == USED)
					//printf("fun :%s, line :%d \r\n", node->fun, node->line);
		#endif		
#endif
	}
	if(ret == NULL)
	{
		MEM_MANAGE_DEBUG
	}
	return ret;
}

int8 mem_free(uint8* ptr)
{
	if((ptr == NULL) || (ptr < g_mem_pool) || (ptr > g_mem_pool + MEM_POOL_SIZE))
	{
		MEM_MANAGE_DEBUG
		return -1;
	}
	mem_node_t *node = (mem_node_t*)ptr - 1;
	node->ifused = UNUSED;
	
	#ifdef OPEN_MEM_MANAGE_CALL_INFO
		memset(node->fun, 0, sizeof(node->fun));
		node->line = 0;
	#endif
	if(mem_fragment_combine(node) != 0)
	{
		MEM_MANAGE_DEBUG
		return -1;
	}
	
#ifdef OPEN_MEM_MANAGE_DEBUG	
	printf("*********************************%s********************************************************\r\n",__func__);
#endif	
	uint16 idx = 0;
	list_head_t *pos1, *pos2;
	list_for_each_safe(pos1, pos2, &(g_mem_node_head->list))
	{
		mem_node_t *node = list_entry(pos1, mem_node_t, list);

#ifdef OPEN_MEM_MANAGE_DEBUG	
			/*如果开启内存调试*/
			printf("mem pool block[%d], addr :%p, size :%d, ifused :%d (0 USED/1 UNUSED)\r\n", idx++, node, (node->mem_size), node->ifused);
		#ifdef OPEN_MEM_MANAGE_CALL_INFO
				//if(node->ifused == USED)
					//printf("fun :%s, line :%d \r\n", node->fun, node->line);
		#endif		
#endif
	}	
	
	return -1;
}

int8 mem_init()
{
	mem_node_t  *mem_node1;
	//printf("g_mem_pool addr :%p, mem_node_t size :%d \r\n", \
	//						   g_mem_pool, sizeof(mem_node_t));
	g_mem_node_head = (mem_node_t*)g_mem_pool;
	list_init(&(g_mem_node_head->list));
	g_mem_node_head->mem_size = 0;
	g_mem_node_head->ifused = USED;

	mem_node1 = g_mem_node_head + 1;
	mem_node1->mem_size = sizeof(g_mem_pool) - sizeof(mem_node_t)*2;
	mem_node1->ifused = UNUSED;
#ifdef OPEN_MEM_MANAGE_CALL_INFO
	memset(mem_node1->fun, 0, sizeof(mem_node1->fun));
	mem_node1->line = 0;
#endif
	
	if(list_add_tail( &(mem_node1->list), &(g_mem_node_head->list) ) != 0)
	{
		MEM_MANAGE_DEBUG
		return -1;
	}
	
	return 0;
}




