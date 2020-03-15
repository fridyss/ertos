#include "object.h"
#include "device.h"
#include "string.h"
#include "mem_manager.h"

#define OBJ_CONTAINER_LIST_INIT(c) \
		{ &(object_container[c].list), &(object_container[c].list) }

static object_info_t object_container[Object_Class_Unknown] =
{
	{ OBJ_CONTAINER_LIST_INIT(Object_Class_Device), Object_Class_Device, sizeof(device_t) },
};

/* 获取对象信息        */
object_info_t *object_get_info( object_class_type_t type )
{
	return &object_container[type];
}

/* 静态对象初始化 */
void object_init( object_t *object, object_class_type_t type, const char *name )
{
	object_info_t *obj_info = NULL;
	obj_info = object_get_info( type );
	if( obj_info == NULL )
		return;
	object->type = type | Object_Class_Static;
	strncpy( object->name, name, OBJECT_NAME_MAX );
	OBJECT_ENTER_CRITICAL();
	list_add_tail( &(object->list) , &(obj_info->list) );
	OBJECT_EXIT_CRITICAL();
}

/* 静态对象分离*/
void object_detach( object_t *object )
{
	if( object == NULL )
		return ;

	object->type = 0;
	
	OBJECT_ENTER_CRITICAL();
	list_node_del( &(object->list) );
	OBJECT_EXIT_CRITICAL();
	
}

/* 动态对象分配 */
object_t * object_alloc( object_class_type_t type, char *name )
{
	object_t * obj = NULL;
	object_info_t * obj_info = NULL;

	obj_info = object_get_info( type );
	obj = (object_t *)mem_alloc( obj_info->size );
	memset( obj, 0, sizeof(object_t) );
	obj->flag = 0;
	obj->type = type;
	memcpy( obj->name, name,  OBJECT_NAME_MAX);
	
	OBJECT_ENTER_CRITICAL();
	list_add_tail( &(obj->list) , &(obj_info->list) );
	OBJECT_EXIT_CRITICAL();
	
	return obj;
}

/* 动态对象删除 */
void object_delete( object_t *obj )
{
	if( obj == NULL || (obj->type | Object_Class_Static) )
		return ;
	obj->type = 0;
	
	OBJECT_ENTER_CRITICAL();
	list_node_del( &(obj->list) );
	OBJECT_EXIT_CRITICAL();

	mem_free( (uint8 *)obj );
	
}

/* 对象是否属于静态内存*/
bool_t object_is_static_object( object_t *obj )
{
	if( obj == NULL )
		return FALSE;
	if( obj->type & Object_Class_Static )
		return TRUE;
	return FALSE;
}

/* 对象类型 */
uint8_t object_get_type( object_t *obj )
{
	if(obj == NULL)
		return 0;
	return obj->type & (~Object_Class_Static);
}


/* 对象查找 */
object_t * object_find( const char *name, object_class_type_t type )
{
	object_t *obj = NULL; 
	object_info_t *obj_info = NULL;
	
	obj_info = object_get_info( type );

	if( name == NULL || type >= Object_Class_Unknown \
					 || obj_info == NULL )
		return NULL;	
	OBJECT_ENTER_CRITICAL();
	list_head_t *pos1,*pos2;
	list_for_each_safe( pos1, pos2, &(obj_info->list) )
	{
		obj = list_entry( pos1, object_t, list );
		if( memcmp( obj->name, name, OBJECT_NAME_MAX ) == 0 )
		{
			OBJECT_EXIT_CRITICAL();
			return obj;
		}
	}

	OBJECT_EXIT_CRITICAL();
	return obj;
}


