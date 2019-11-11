#include "list.h"

/*
 *     对链表头进行初始化
 */
int8 list_init(list_head_t *head)
{
	if(head == NULL)
		return -1;
	head->prev = head;
	head->next = head;
	return 0;
}

/*
 *     对链表节点后面进行添加一个节点.
 */
int8 list_node_add_tail(list_head_t *new, list_head_t *node)
{
	if(new == NULL|| node == NULL)
		return -1;
	new->next = node->next;
	new->prev = node;
	
	node->next->prev = new;
	node->next = new;
	
	return 0;
}


/*
 *     对链表节点前面插入一个节点
 */
int8 list_node_add(list_head_t *new, list_head_t *node)
{
	if(new == NULL || node == NULL)
		return -1;

	new->next = node;
	new->prev = node->prev;
	
	node->prev->next = new;
	node->prev = new;
	return 0;
}



/*
 *     对链表尾巴增加，就是从链表头插入一个节点
 */
int8 list_add_tail(list_head_t *new, list_head_t *head)
{
	if(new == NULL || head == NULL)
		return -1;

	list_node_add(new, head);
	return 0;
}


/*
 *     删除任意节点，但不释放内存, 
 *     测试，如果删除最后节点，则list_is_last(),有两个尾节点。
 *     node, 指向没有释放。
 */
int8 list_node_del(list_head_t *node)
{
	if(node == NULL)
		return -1;

	node->prev->next = node->next;
	node->next->prev = node->prev;
	
	return 0;
}


/*
 *    节点替换
 */
int8 list_node_replace(list_head_t *new, list_head_t *old)
{
	if(new == NULL || old == NULL)
		return -1;
	new->next = old->next;
	new->prev = old->prev;

	old->prev->next = new;
	old->next->prev = new;

	return 0;
}


/*
 *     节点移动,node1 移动到node2后面。
 */
int8 list_node_move(list_head_t *node1, list_head_t *node2)
{
	if(list_node_del(node1) == -1)
		return -1;
	list_node_add_tail(node1, node2);
	return 0;
}


/*
 *     删除链表尾巴的节点，也是删除链表头的前面节点
 */
int8 list_del_tail(list_head_t *head)
{
	if(head == NULL)
		return -1;
	list_node_del(head ->prev);
	return 0;
}

/*
 *     是否是最后一个结点 ，是返回1，反之 0.
 */
int8 list_is_last(list_head_t *node, list_head_t *head)
{
	if(node == NULL|| head == NULL)
		return -1;
	return (node->next == head);
}

/*
 *     判断链表是否为空
 */
int8 list_is_empty(list_head_t *head)
{
	if(head == NULL)
		return -1;
	return (head->next == head);
}


/*
 *     判断链表是否为空，并测试当前链表没有并其他cpu(多核多道程序)更改。 
 */

int8 list_is_empty_save(list_head_t *head)
{
	if(head == NULL)
			return -1;
	list_head_t *next = head->next;

	return (next == head) && (next == head->prev);
}

/*
 *     判断链表是否只有一个节点，是返回1，反之0
 */
int8 list_singular(list_head_t* head)
{
	if(head == NULL ) return -1;
	return !list_is_empty(head) && (head->next == head->prev);  
}

uint16_t list_length(list_head_t *head)
{
    uint16_t nr;
    
    list_head_t *list = head->next;
    while(list != head)
    {   
        nr++;
        list = list->next; 
    }
    return nr;
}
