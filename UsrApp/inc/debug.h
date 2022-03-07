  #ifndef _DEBUG_H
#define _DEBUG_H

#include "main.h"

/* keil中默认不支持匿名联合体，故需要声明下 */
//#pragma anon_unions

#define RX_BUF_MAX_LEN 1024 //最大接收缓存字节数
#define DEBUG_USART		USART1
enum DebugLevel
{
	Release,
	Monitor
};


#define DEBUG 1
#define RELEASE_VERSION			0		// 置1后将关闭所有打印信息

#if RELEASE_VERSION		
#undef DEBUG
#endif

#ifdef DEBUG
// 打印运行信息，定位标识：I
#define p_info(...)                                                  \
do                                                                   \
{                                                                    \
	if(!dbg_level)                                                   \
		break;                                                       \
	printf("[I: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);\
	printf(__VA_ARGS__);                                             \
	printf("\r\n");                                                  \
}while(0)

// 打印错误信息，定位标识：E
#define p_err(...)                                                   \
do																	 \
{																	 \
	if(!dbg_level)													 \
		break;														 \
	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);\
	printf(__VA_ARGS__);											 \
	printf("\r\n"); 												 \
}while(0)

// 打印调试信息，定位标识：D
#define p_dbg(...)                                                   \
do																	 \
{																	 \
	if(!dbg_level)													 \
		break;														 \
	printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);\
	printf(__VA_ARGS__);											 \
	printf("\r\n"); 												 \
}while(0)

// 打印时间戳
#define ERR_PRINT_TIME	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)
#define DBG_PRINT_TIME	printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)

// 定位具体位置（函数、行、状态）
#define p_dbg_track do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s,%d",  __FUNCTION__, __LINE__); printf("\r\n");}while(0)
#define p_dbg_enter do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("enter %s\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_dbg_exit do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("exit %s\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_dbg_status do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("status %d\n", status); printf("\r\n");}while(0)

// 定位错误位置
#define p_err_miss do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s miss\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_err_mem do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s mem err\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_err_fun do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s err in %d\n", __FUNCTION__, __LINE__); printf("\r\n");}while(0)

#else
#define ERR_PRINT_TIME	
#define DBG_PRINT_TIME	
#define p_info(...) 
#define p_err(...) 
#define p_dbg_track 
#define p_dbg(...) 
#define p_dbg_enter 
#define p_dbg_exit 
#define p_dbg_status 
#define p_err_miss 
#define p_err_mem 
#define p_err_fun

#endif

typedef struct // 串口数据帧的处理结构体
{
	uint8_t *pRxBuffer;
	union 
	{
		__IO uint16_t InfAll;
		struct
		{
			__IO uint16_t FramLength : 15;	// 14:0
			__IO uint16_t FramFinishFlag : 1; // 15
		} InfBit;
	};
} STRUCT_USARTx_Fram;

extern uint8_t dbg_rxdata;
extern STRUCT_USARTx_Fram dbg_Fram_Record;
extern int dbg_level;

uint32_t os_time_get(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart); // 串口接收回调函数

#endif
