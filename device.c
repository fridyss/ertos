#include "device.h"
#include "errno.h"
#include "mem_manager.h"
#include "string.h"
#include "service.h"

/* 设备注册 */
err_t device_register( device_t *dev, const char *name, uint16_t flag )
{
	if( dev == NULL || name == NULL )
		return -EERR;

	if( device_find(name) != NULL )
		return -EERR;

	object_init( &(dev->parent), Object_Class_Device, name );
	dev->flag = flag;
	dev->ref_count = 0;
	dev->open_flag = 0;

	return EOK;
}

/* 设备反注册 */
err_t device_unregister( device_t *dev )
{
	
	if( dev == NULL )
		return -EERR;
	if( object_get_type( (object_t *)dev ) != Object_Class_Device || \
			object_is_static_object( (object_t *)dev) )
	{
		return -EERR;
	}

	object_detach( (object_t *)dev );
	return EOK;
}

/* 设备查找 */
device_t *device_find( const char *name )
{
	device_t *dev;

	if( name == NULL )
		return NULL;
	dev = (device_t *)object_find( name, Object_Class_Device );
	return (device_t *)dev;
}

device_t *device_create( uint8_t type, size_t attch_size )
{
	device_t *dev = NULL;
	size_t size = sizeof(device_t);
	
	MEM_ALIGN( size, MEM_ALIGN_BYTE );
	MEM_ALIGN( attch_size, MEM_ALIGN_BYTE);

	size += attch_size;
	dev = (device_t *)mem_alloc(size);
	if(dev)
	{
		memset( dev, 0, sizeof(device_t) );
		dev->type = (device_class_type_t)type;
	}
	
	return dev;
}

void device_destroy( device_t *dev )
{
	if(device_unregister(dev) == EOK)
	{
		mem_free( (uint8_t*)dev );
	}
}

err_t device_init( device_t *dev )
{
	err_t ret = -ERROR;
	
	if(dev == NULL)
		return -EERR;
	if( dev->init != NULL )
	{
		if( !(dev->flag & DEVICE_FLAG_ACTIVATED) )
		{/*未被初始化过*/
			ret = dev->init( dev ) ;
			if( ret == EOK )
			{
				dev->flag |= DEVICE_FLAG_ACTIVATED; 	
			}
			else
			{
				kprintf(" device init failed : %s, err code :%d \n ",  \
					    dev->parent.name, ret);
			}
		}
	}

	return ret;
	
}

err_t device_open( device_t *dev, uint16_t open_flag )
{
	err_t ret = -EERR;

	if( dev == NULL || object_get_type( (object_t *) dev) != Object_Class_Device)
		return -EERR;
	
	if( !(dev->flag & DEVICE_FLAG_ACTIVATED) )
	{	/*如果设备没有初始化, 则需要进行初始化*/
		if( dev->init != NULL )	
		{
			ret = dev->init( dev );
			if(ret != EOK)
				return ret;
			dev->flag |= DEVICE_FLAG_ACTIVATED; 
		}
	}

	if( (dev->flag & DEVICE_FLAG_STANDALONE) && \
		(dev->open_flag & DEVICE_OFLAG_OPEN))
	{
		/*如果设备是独立设备而且被打开了，反而设备忙*/
		return -EBUSY;
	}

	if( dev->open != NULL )
	{
		ret = dev->open( dev , open_flag);
	}
	else
	{
		dev->open_flag = (open_flag & DEVICE_OFLAG_MASK); 
	}

    if (ret == EOK || ret == -ENOSYS)
    {
        dev->open_flag |= DEVICE_OFLAG_OPEN;
        dev->ref_count++;
    }
	return ret;
}

err_t device_close( device_t *dev )
{
	err_t ret = -EERR;
	
	if( dev == NULL || object_get_type( (object_t *) dev) != Object_Class_Device )
		return -EERR;

	/*设备关联个数，为0,返回错误*/
	if( dev->ref_count == 0 )
		return -EERR;

	dev->ref_count--;

	if( dev->close != NULL )
	{
		ret = dev->close( dev );	
	}
	
	if( ret == EOK || ret == -ENOSYS )
        dev->open_flag = DEVICE_OFLAG_CLOSE;

	return ret;
}

size_t device_read( device_t *dev, offset_t pos, void *buffer, size_t size )
{
	if( dev == NULL || object_get_type( (object_t *) dev) != Object_Class_Device )
		return 0;

	if( dev->ref_count == 0 )
	{
		/*没有相关关联设备, 返回读取数0*/
		return 0;
	}

	if( dev->read != NULL )
		return dev->read( dev, pos, buffer, size );

	return 0;
}

size_t device_write( device_t *dev, offset_t pos, const void *buffer, size_t size )
{
	if( dev == NULL || object_get_type( (object_t *) dev) != Object_Class_Device )
		return 0;

	if( dev->ref_count == 0 )
	{
		/*没有相关关联设备, 返回写入数0*/
		return 0;
	}

	if( dev->write != NULL )
		return dev->write( dev, pos, buffer, size );
	
	return 0;
}

err_t device_control( device_t *dev, int cmd, void *arg )
{
	
	if( dev == NULL || object_get_type( (object_t *) dev) != Object_Class_Device )
		return -EERR;

	if( dev->control != NULL )
		return dev->control( dev, cmd, arg );

	return -EERR;
	
}

err_t device_set_rx_callback_fun( device_t *dev, rx_callback_t rx_cb )
{
	if( dev == NULL || object_get_type( (object_t *) dev) != Object_Class_Device )
		return -EERR;

	dev->rx_callback = rx_cb;
	return EOK;	
}

err_t device_set_tx_callback_fun( device_t *dev, tx_callback_t tx_cb )
{
	if( dev == NULL || object_get_type( (object_t *) dev) != Object_Class_Device )
		return -EERR;
	dev->tx_callback = tx_cb;
    return EOK;
}

