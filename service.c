#include "service.h"
#include "device.h"
#include "stdarg.h"
#include "errno.h"



/*控制台*/
static device_t *console = NULL;


/* 内核打印函数 */
void kprintf( const char *fmt,... )
{
	if(console == NULL)
		return ;
	va_list args;
	static char log_buffer[ERTOS_CONSOLE_BUFFER_SIZE];
	va_start(args, fmt);
	uint16 length = vsnprintf(log_buffer, ERTOS_CONSOLE_BUFFER_SIZE - 1, fmt, args);
	if( length > ERTOS_CONSOLE_BUFFER_SIZE - 1 )
	{
		length = ERTOS_CONSOLE_BUFFER_SIZE - 1;
	}
	if( console != NULL )
	{
		uint16 old_flag = console->open_flag;
		console->open_flag |= DEVICE_FLAG_STREAM; 
		device_write(console, 0, log_buffer, length);
		console->open_flag |= old_flag;
	}
	va_end(args);
}


/* 为控制台关联设备 */
err_t  console_set_device( const char *name )
{
	device_t *dev = NULL;

	if( name == NULL )
		return -EERR;

	dev = device_find(name);
	if(dev)
	{
		if(console)
		{/*如果控制台不为空，则需先关闭*/
			device_close(console);
		}
		device_open(dev, DEVICE_OFLAG_RDWR | DEVICE_FLAG_STREAM);
		console = dev;
	}
	return EOK;
}
