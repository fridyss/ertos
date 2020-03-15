#ifndef __SERIAL_H
#define __SERIAL_H

#include "type.h"
#include "stdio.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

/*波特率*/
#define BAUD_RATE_2400                  2400
#define BAUD_RATE_4800                  4800
#define BAUD_RATE_9600                  9600
#define BAUD_RATE_19200                 19200
#define BAUD_RATE_38400                 38400
#define BAUD_RATE_57600                 57600
#define BAUD_RATE_115200                115200
#define BAUD_RATE_230400                230400
#define BAUD_RATE_460800                460800
#define BAUD_RATE_921600                921600
#define BAUD_RATE_2000000               2000000
#define BAUD_RATE_3000000               3000000

/*数据位*/
#define DATA_BITS_5                     5
#define DATA_BITS_6                     6
#define DATA_BITS_7                     7
#define DATA_BITS_8                     8
#define DATA_BITS_9                     9

/*停止位*/
#define STOP_BITS_1                     0
#define STOP_BITS_2                     1
#define STOP_BITS_3                     2
#define STOP_BITS_4                     3

/* 奇偶位 */
#define PARITY_NONE                     0
#define PARITY_ODD                      1
#define PARITY_EVEN                     2

/* 字节流 */
#define BIT_ORDER_LSB                   0
#define BIT_ORDER_MSB                   1

/* 不归零模式 */
#define NRZ_NORMAL                      0       /* Non Return to Zero : normal mode */
#define NRZ_INVERTED                    1       /* Non Return to Zero : inverted mode */

#ifndef SERIAL_RB_BUFSIZE
#define SERIAL_RB_BUFSIZE              64
#endif

#define SERIAL_EVENT_RX_IND            0x01    /* Rx indication */
#define SERIAL_EVENT_TX_DONE           0x02    /* Tx complete   */
#define SERIAL_EVENT_RX_DMADONE        0x03    /* Rx DMA transfer done */
#define SERIAL_EVENT_TX_DMADONE        0x04    /* Tx DMA transfer done */
#define SERIAL_EVENT_RX_TIMEOUT        0x05    /* Rx timeout    */

#define SERIAL_DMA_RX                  0x01
#define SERIAL_DMA_TX                  0x02

#define SERIAL_RX_INT                  0x01
#define SERIAL_TX_INT                  0x02

#define SERIAL_ERR_OVERRUN             0x01
#define SERIAL_ERR_FRAMING             0x02
#define SERIAL_ERR_PARITY              0x03

#define SERIAL_TX_DATAQUEUE_SIZE     2048
#define SERIAL_TX_DATAQUEUE_LWM      30


#define SERIAL_CONFING_DEFAULT 		       \
{										   \
	BAUD_RATE_115200, /* 115200 bits/s */  \
	DATA_BITS_8,	  /* 8 databits */	   \
	STOP_BITS_1,	  /* 1 stopbit */	   \
	PARITY_NONE,	  /* No parity	*/	   \
	BIT_ORDER_LSB,	  /* LSB first sent */ \
	NRZ_NORMAL, 	  /* Normal mode */    \
	SERIAL_RB_BUFSIZE, /* Buffer size */  \
	0									   \
}									       

typedef struct _serial_configure
{
    uint32_t baud_rate;				       /* 波特率 bits/s */  
    uint32_t data_bits               :4;   /* 数据位 */				
    uint32_t stop_bits               :2;   /* 停止位 */	   
    uint32_t parity                  :2;   /* 奇偶位	*/	   
    uint32_t bit_order               :1;   /* 字节传输顺序 */ 
    uint32_t invert                  :1;   /* 模式 */    
    uint32_t bufsz                   :16;  /* 缓冲区size */  
    uint32_t reserved                :6;   /* 保留 */	
}serial_configure_t;


typedef struct _serial_device serial_device_t;

typedef err_t (*usart_confingure_t)( serial_device_t *serial, serial_configure_t *cfg );
typedef err_t (*usart_control_t)( serial_device_t *serial, int cmd, void *arg );
typedef int (*usart_putc_t)( serial_device_t *serial, char c );
typedef int (*usart_getc_t)( serial_device_t *serial );


/* 通用异步串口操作接口类型 */
typedef struct _uasrt_ops
{
	usart_confingure_t configure;
	usart_control_t control;
	usart_putc_t putc;
	usart_getc_t getc;
	
}uasrt_ops_t;


struct _serial_device
{
	device_t 			parent;		/* 继承设备类 */
	const uasrt_ops_t 	*ops;       /* 串口相关操作接口  */
	serial_configure_t  config;		/* 串口配置信息 */

	void *serial_rx;				/* 接收数据指针 */
	void *serial_tx;				/* 发送数据指针 */
};

/* 串口中断处理函数 */
err_t serial_isr( serial_device_t *serial, int event );

/* 串口设备注册函数 */
err_t serial_register( serial_device_t *serial,
							  const char *name,
							  uint32_t flag,
							  void *data );

#ifdef __cplusplus
}
#endif

#endif /*__SERIAL_H*/

