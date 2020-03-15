#ifndef _SHELL_H
#define _SHELL_H

#include "ertos.h"

#ifdef _cplusplus
extern "C" {
#endif 

#define CMD_HISTROY_LINE		5							/**< 命令行数 */
#define CMD_HISTROY_SIZE		80							/**< 命令行数size */


#define SHELL_PROMPT			"sh"						/**< 提示符 */
#define SHELL_PROMPT_CHAR		" >"						/**< 提示符标识*/


/**
 *	shell对象类型
 */
typedef struct _shell_t
{
	device_t *device;										/**< 设备指针 */
	sem_t sem;												/**< 信号量 */
	uint16_t cur_history;									/**< 当前历史命令 */
	uint16_t histoty_cnt;									/**< 历史命令数量*/
	char cmd_history[CMD_HISTROY_LINE][CMD_HISTROY_SIZE];	/**< 历史命令记录*/
	uint8_t echo_mode : 1;   								/**< 1:命令回显, 0:不回显 */
	uint8_t prompt_mode : 1;								/**< 1:开启提示符,0 :反之 */

	char line[CMD_HISTROY_SIZE];							/**< 当前行命令 */
	uint8_t line_position;									/**< 命令行位置 */
	uint8_t line_curpos;									/**< 命令行当前位置 */
	
}shell_t;



#ifdef _cplusplus
}
#endif 

#endif /*_SHELL_H*/
