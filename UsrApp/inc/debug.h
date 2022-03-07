  #ifndef _DEBUG_H
#define _DEBUG_H

#include "main.h"

/* keil��Ĭ�ϲ�֧�����������壬����Ҫ������ */
//#pragma anon_unions

#define RX_BUF_MAX_LEN 1024 //�����ջ����ֽ���
#define DEBUG_USART		USART1
enum DebugLevel
{
	Release,
	Monitor
};


#define DEBUG 1
#define RELEASE_VERSION			0		// ��1�󽫹ر����д�ӡ��Ϣ

#if RELEASE_VERSION		
#undef DEBUG
#endif

#ifdef DEBUG
// ��ӡ������Ϣ����λ��ʶ��I
#define p_info(...)                                                  \
do                                                                   \
{                                                                    \
	if(!dbg_level)                                                   \
		break;                                                       \
	printf("[I: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);\
	printf(__VA_ARGS__);                                             \
	printf("\r\n");                                                  \
}while(0)

// ��ӡ������Ϣ����λ��ʶ��E
#define p_err(...)                                                   \
do																	 \
{																	 \
	if(!dbg_level)													 \
		break;														 \
	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);\
	printf(__VA_ARGS__);											 \
	printf("\r\n"); 												 \
}while(0)

// ��ӡ������Ϣ����λ��ʶ��D
#define p_dbg(...)                                                   \
do																	 \
{																	 \
	if(!dbg_level)													 \
		break;														 \
	printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);\
	printf(__VA_ARGS__);											 \
	printf("\r\n"); 												 \
}while(0)

// ��ӡʱ���
#define ERR_PRINT_TIME	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)
#define DBG_PRINT_TIME	printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)

// ��λ����λ�ã��������С�״̬��
#define p_dbg_track do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s,%d",  __FUNCTION__, __LINE__); printf("\r\n");}while(0)
#define p_dbg_enter do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("enter %s\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_dbg_exit do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("exit %s\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_dbg_status do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("status %d\n", status); printf("\r\n");}while(0)

// ��λ����λ��
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

typedef struct // ��������֡�Ĵ���ṹ��
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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart); // ���ڽ��ջص�����

#endif
