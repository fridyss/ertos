/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   e_list.h
 * Author: fridy
 *
 * Created on 2019年9月2日, 下午5:09
 */

#ifndef _LIST_H
#define _LIST_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct list_head
{
    struct list_head *prev,*next;
}list_head_t;

/*链表头初始化*/
#define LIST_INIT(name)  {&(name), &(name)}

/*获取结构体type,成员变量number的偏移量*/
#define offset_of(type, number)	                ((uint32) &((type *)0)->number)

/*根据"结构体(type)变量"中的"域成员变量(member)的指针(ptr)"来获取指向整个结构体变量的指针*/
#define container_of(ptr, type, number)			(type*)((char*)(ptr) - offset_of(type, number))

/*遍历双向链表*/
#define list_for_each(pos_ptr, head_ptr)  \
                for (pos_ptr = (head_ptr) ->next; pos_ptr != (head_ptr); pos_ptr = pos_ptr->next)

/*遍历双向链表安全地,避免在遍历链表时，要对链表进行删除操作*/
#define list_for_each_safe(pos_ptr, n, head_ptr)  \
				for((pos_ptr) = (head_ptr)->next, (n) = (pos_ptr)->next; \
							(pos_ptr) != (head_ptr);\
							(pos_ptr) = (n), (n) = (n)->next)

//通过成员变量地址获取list起始地址。
#define list_entry(ptr, type, member) \
        	container_of(ptr, type, member)

extern int8 list_init(list_head_t *head);
extern int8 list_node_add_tail(list_head_t *new, list_head_t *node);
extern int8 list_node_add(list_head_t *new, list_head_t *node);
extern int8 list_add_tail(list_head_t *new, list_head_t *head);
extern int8 list_node_del(list_head_t *node);
extern int8 list_node_replace(list_head_t *new, list_head_t *old);
extern int8 list_node_move(list_head_t *node1, list_head_t *node2);
extern int8 list_del_tail(list_head_t *head);
extern int8 list_is_last(list_head_t *node, list_head_t *head);
extern int8 list_is_empty(list_head_t *head);
extern int8 list_is_empty_save(list_head_t *head);
extern int8 list_singular(list_head_t* head);
extern uint16_t list_length(list_head_t *head);

#ifdef __cplusplus
}
#endif

#endif /* _LIST_H */

