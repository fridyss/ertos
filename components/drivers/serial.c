#include "serial.h"
#include "errno.h"
#include "fifo.h"

/*
 * Serial poll routines
 */
int _serial_poll_rx( serial_device_t *serial, uint8_t *data, int length )
{
    int ch;
    int size;

    if(serial != NULL)
		return -EERR;
    size = length;

    while (length)
    {
        ch = serial->ops->getc(serial);
        if (ch == -1) 
			break;
        *data++ = ch;
		length --;

        if (ch == '\n') 
			break;
    }

    return size - length;
}

int _serial_poll_tx( serial_device_t *serial, const uint8_t *data, int length )
{
    int size;
	
    if(serial == NULL)
		return -EERR;

    size = length;
    while (length)
    {
        if (*data == '\n' && (serial->parent.open_flag & DEVICE_FLAG_STREAM))
        {
            serial->ops->putc(serial, '\r');
        }

        serial->ops->putc(serial, *data);
      
        ++ data;
        -- length;
    }

    return size - length;
}



/* 串口初始化 */
static err_t serial_init( device_t *dev )
{
	err_t ret = -EERR;
	serial_device_t *serial = NULL;
	if( dev == NULL || dev->type != Device_Class_Char )
		return -EERR;
	serial = (serial_device_t *) dev;

	serial->serial_rx = NULL;
	serial->serial_tx = NULL;
	
	if( serial->ops->configure )
		ret = serial->ops->configure( serial, &serial->config );
	return ret;
}



/* 串口打开 */
static err_t serial_open(device_t *dev, uint16_t open_flag)
{
	uint16_t stream_flag = 0;
	if( dev == NULL )
		return -EERR;
	serial_device_t *serial = (serial_device_t *)dev;
	
	/*如果打开标志记, 在设备标志位中没有标志位，则返回io错误*/
	if( (open_flag & DEVICE_FLAG_INT_RX) && !(dev->flag & DEVICE_FLAG_INT_RX) )
		return -EIO;
	if( (open_flag & DEVICE_FLAG_INT_TX) && !(dev->flag & DEVICE_FLAG_INT_TX) )
		return -EIO;

	if( (open_flag & DEVICE_FLAG_STREAM) || (dev->open_flag & DEVICE_FLAG_STREAM) )
		stream_flag = DEVICE_FLAG_STREAM; /*记录设备流是否被打开的标志位*/

	/*记录设备打开标志位*/
	dev->open_flag = open_flag & 0xff; 
	if( serial->serial_rx == NULL )
	{
		/*接收数据指针为空*/
		if( open_flag & DEVICE_FLAG_INT_RX )
		{
			uint16_t size = serial->config.bufsz;
			serial->serial_rx =  (fifo_t *)fifo_alloc( size, 1 ); /*分配*/
			dev->open_flag |= DEVICE_FLAG_INT_RX;
			serial->ops->control(serial, DEVICE_CTRL_SET_INT, (void *)DEVICE_FLAG_INT_RX);
		}
		else
		{
			serial->serial_rx = NULL;
		}
	}
	else
	{
		dev->open_flag |= DEVICE_FLAG_INT_RX;
	}

	if( serial->serial_tx == NULL )
	{
		if( open_flag & DEVICE_FLAG_INT_TX )
		{
			uint16_t size = serial->config.bufsz;
			//serial->serial_tx =  (fifo_t *)fifo_alloc( size, 1 ); /*分配*/
			//serial->ops->control(serial, DEVICE_CTRL_SET_INT, (void *)DEVICE_FLAG_INT_TX);
		}
		 else
        {
            serial->serial_tx = NULL;
        }
	}
	else
	{
		dev->open_flag |= DEVICE_FLAG_INT_TX;

	}
	
	/* set stream flag */
    dev->open_flag |= stream_flag;
	return EOK;
}

/* 串口关闭 */
static err_t serial_close(device_t *dev)
{
	serial_device_t *serial = (serial_device_t *)dev;
	if( dev == NULL)
		return -EERR;
	if( dev->ref_count > 1 )
		return EOK;

	if( dev->open_flag & DEVICE_FLAG_INT_RX )
	{
		fifo_t *fifo = (fifo_t *)serial->serial_rx;
		fifo_free(fifo);
		serial->serial_rx = NULL;
		dev->flag &= ~DEVICE_FLAG_INT_RX; 
		serial->ops->control(serial, DEVICE_CTRL_CLR_INT, (void*)DEVICE_FLAG_INT_RX);
	}

	if( dev->open_flag & DEVICE_FLAG_INT_TX )
	{
	    serial->serial_tx = NULL;
        dev->open_flag &= ~DEVICE_FLAG_INT_TX;
		serial->ops->control(serial, DEVICE_CTRL_CLR_INT, (void*)DEVICE_FLAG_INT_TX);
	}
	return EOK;
}

/* 串口读操作 */
static int16_t serial_read( device_t *dev, offset_t pos, void *buffer, size_t size )
{
	serial_device_t *serial;
	size_t max = size;
    char *ptr = (char *)(buffer);
	if( dev == NULL || buffer == NULL )
		return -EERR;
	serial = (serial_device_t *)dev;
	
	if( dev->open_flag & DEVICE_FLAG_INT_RX )
	{/*打开接收标志*/
		fifo_t *fifo = (fifo_t *) serial->serial_rx;
		while(size)
		{
			char ch;
			mcu_enter_critical();
			
			if( fifo_is_empty(fifo) )
			{/*接收fifo为空,则*/
				mcu_exit_critical();
				break;
			}
	
			fifo_get(fifo, (uint8* )&ch, 1);
			mcu_exit_critical();
			*ptr++ = ch;
			size--;
		}
		return max - size; /*返回读取个数*/
	}	

	/*如果没有打开接收标志位*/
	return  _serial_poll_rx(serial, buffer, size);		
}




/* 串口写操作 */
static int16_t serial_write(device_t *dev, offset_t pos, const void *buffer, size_t size)
{
	serial_device_t *serial = (serial_device_t *)dev;
	size_t wsize = 0;
    uint8 *ptr_buffer = (uint8 *)buffer;
	if( dev == NULL || serial == NULL)
		return -EERR;

	if( dev->open_flag & DEVICE_FLAG_INT_TX )
	{/*打开中断发送标志*/
		while( size )
		{
			if(serial->ops->putc(serial, *ptr_buffer) == -1)
			{
				continue;
			}
			ptr_buffer++;
			wsize++;
			size--;
		}
		return wsize;
	}
	/*如果没有打开接收标志位*/
	return  _serial_poll_tx(serial, buffer, size);	

}

/* 串口控制函数 */
static err_t serial_control(device_t *dev, int cmd, void *args)
{
	serial_device_t *serial = (serial_device_t *)dev;
	err_t ret = EOK;
	
	if( dev == NULL || serial == NULL)
		return -EERR;
	switch( cmd )
	{
		case DEVICE_CTRL_SUSPEND:
			dev->flag |= DEVICE_FLAG_SUSPENDED;
			break;
		case DEVICE_CTRL_RESUME:
			dev->flag &= ~DEVICE_FLAG_SUSPENDED;
			break;
		case DEVICE_CTRL_CONFIG:
		{
			if(args)
			{
				serial_configure_t *cfg = (serial_configure_t *)args;
				if(cfg->bufsz != serial->config.bufsz && dev->ref_count)
				{
					return EBUSY;
				}
				serial->config = *cfg;
				if(dev->ref_count)
				{
					serial->ops->configure( serial, cfg );
				}
			}

		}
			break;
        default :
            ret = serial->ops->control(serial, cmd, args);
            break;

	}
	return ret;
}

/* 串口中断处理函数 */
err_t serial_isr( serial_device_t *serial, int event )
{
	switch (event & 0xff)
   {
	  case SERIAL_EVENT_RX_IND:
	  {
		  int ch = -1;
		  uint32 level;
		  fifo_t* rx_fifo;
	
		  rx_fifo = (fifo_t*)serial->serial_rx;
		  if(rx_fifo == NULL)
			return -EERR;
		  while (1)
		  {
			  ch = serial->ops->getc(serial);
			  if (ch == -1) break;


			  /* disable interrupt */
			 level = MCU_SET_INTERRUPT_MASK_FROM_ISR();
			 {
				fifo_put( rx_fifo, (uint8 *)&ch, 1);
			 }
			 MCU_CLEAR_INTERRUPT_MASK_FROM_ISR(level);

		  }

		  /* invoke callback */
		  if (serial->parent.rx_callback != NULL)
		  {
			  level = MCU_SET_INTERRUPT_MASK_FROM_ISR();
			  {
					uint16 length = fifo_length(rx_fifo);
					serial->parent.rx_callback(&serial->parent, length);
			  }
			  MCU_CLEAR_INTERRUPT_MASK_FROM_ISR(level);

		  }
		  break;
	  }
	  
	  case SERIAL_EVENT_TX_DONE:
	  {
		  fifo_t* tx_fifo;
		 // tx_fifo = (struct rt_serial_tx_fifo*)serial->serial_tx;
		  //rt_completion_done(&(tx_fifo->completion));
		  break;
	  }

	  default :
	  	   break;
	  
  }
   return EOK;

}

/* 串口设备注册函数 */
err_t serial_register( serial_device_t *serial,
							  const char *name,
							  uint32_t flag, 
							  void *data )
{

	device_t *dev = (device_t *)serial;

	if( serial == NULL || name == NULL )
		return -EERR;

	dev->rx_callback = NULL;
	dev->tx_callback = NULL;
	
	dev->type = Device_Class_Char; /*字符设备*/
	dev->init = serial_init;
	dev->open = serial_open;
	dev->close = serial_close;
	dev->read = serial_read;
	dev->write = serial_write;
	dev->control = serial_control;
	dev->user_data = data;
	return device_register( dev, name, flag );
}

