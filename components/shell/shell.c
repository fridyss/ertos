#include "shell.h"
#include "mem_manager.h"
#include "string.h"
#include "init.h"
#include "device.h"
#include "sched.h"
#include "mcu.h"

shell_t *shell = NULL;

/* 提示符 */
static char *shell_prompt = NULL;

task_handle_t shell_handle;

/**
 *	设置提示符
 */
err_t shell_set_prompt( const char *prompt )
{
	size_t prompt_size = 0;
	if( prompt == NULL )
		return -EERR;
	
	prompt_size = strlen(prompt) + strlen(SHELL_PROMPT_CHAR);	/*加上提示尾*/
	if( shell_prompt )
	{
		/*如果不为空, 则释放现有内存*/
		mem_free( (uint8 *)shell_prompt );
		shell_prompt = NULL;
	}
	
	shell_prompt = mem_alloc( prompt_size + 1 );  /*预留结束符*/
	if( shell_prompt )
	{
		memset( shell_prompt, 0, prompt_size + 1);
		memcpy( shell_prompt, prompt, prompt_size );
	}

	strcat(shell_prompt, SHELL_PROMPT_CHAR);
	
	return EOK;
}


/**
 *	获取提示符
 */
const char * shell_get_prompt()
{
	static char shell_prompt[CMD_HISTROY_SIZE + 1] = {0};

	/*没开启, 则直接返回*/
	if( !shell && !shell->prompt_mode )
	{
		return NULL;
	}

	if( shell_prompt == NULL )
	{
		shell_set_prompt(SHELL_PROMPT);
	}
	
	return shell_prompt;
}

/**
 *	获取提示模式
 */
int8_t shell_get_prompt_mode( void )
{
	if( shell == NULL )
		return -EERR;
	return shell->prompt_mode;	
}

/**
 *	设置提示模式
 */
err_t shell_set_prompt_mode( uint8 mode )
{
	if( shell == NULL )
		return -EERR;

	shell->prompt_mode = mode;
	return EOK;
}

/**
 *	设备回显模式
 */
void shell_set_echo_mode( uint8_t mode )
{
	if( shell == NULL )
		return NULL;
	shell->echo_mode = (mode)?(1):(0);
}

/**
 *	获取回显模式
 */
int8_t shell_get_echo_mode( void )
{
	if( shell == NULL )
		return -EERR;
	return shell->echo_mode;
}

/**
 *	获取字符
 */
static char shell_get_char( void )
{
	char ch;
	if( shell == NULL )
		return NULL;
	while( device_read( shell->device, -1, &ch, 1) != 1)
		sem_pend( &(shell->sem), PORT_MAX_DELAY );
	return ch;
}

/**
 *	接收回调
 */
static err_t shell_rx_callback( device_t dev, size_t size )
{
	if( shell == NULL )
		return -EERR;
	sem_post( &(shell->sem) );
	return EOK;
}

/**
 *	为shell设置设备
 */
void shell_set_device( const char *name )
{
	device_t *dev = NULL;
	
	if( shell == NULL || name == NULL )
		return ;

	dev = device_find( name );
	
	if( dev == NULL )
		return ;
	
	if( dev == shell->device )
		return;
	if( device_open( dev, DEVICE_OFLAG_RDWR | DEVICE_FLAG_INT_RX | \ 
							DEVICE_FLAG_STREAM ) == EOK )
	{
		/*成功打开*/
		if( shell->device != NULL )
		{
			device_close( shell->device );
			device_set_rx_callback_fun( shell->device, NULL );
		}
		memset( shell->line, 0, sizeof(shell->line) );
		shell->line_curpos = 0;
		shell->line_position = 0;
		shell->device = dev;
		device_set_rx_callback_fun( shell->device, shell_rx_callback);
	}
	
}

/**
 *	获取shell设备名
 */
const char *shell_get_device( void )
{
	if( shell == NULL )
		return NULL;
	return shell->device->parent.name;
}

/**
 *	自动补全命令
 */
static void shell_auto_complete( char *prefix )
{

}


static bool_t shell_handle_history( shell_t *shell )
{

}

static void shell_push_history( shell_t *shell )
{

}

/**
 *	shell 任务函数
 */
void task_shell_entry( void *params )
{

}

/**
 *	shell 初始化
 */
err_t shell_init( void )
{
	err_t ret = EOK;

	shell = mem_alloc( sizeof( shell_t ) );
	if(shell == NULL)
		return -ENOMEM;
 
	sem_metux_create( &shell->sem );
    shell_set_prompt_mode( 1 );

	MCU_ENTER_CRITICAL();	
	get_scheduler()->task_create( 
	                        		&shell_handle, 
	                        		task_shell_entry, 
	                        		"shell",
	                   		 		4096, NULL, 1
	               				);
	MCU_EXIT_CRITICAL(); 
	return ret;
	
}
INIT_APP_EXPORT(shell_init);
