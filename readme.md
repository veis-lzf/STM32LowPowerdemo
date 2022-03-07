# STM32G0低功耗介绍

## 一、低功耗模式介绍

### 1、STM32G0按照分类可以分为4种模式

（1）sleep（sleep和low-power sleep）模式：功耗高，支持任意中断/事件唤醒

（2）stop（stop0和stop1）模式：功耗较低，支持任意外部中断和RTC闹钟唤醒

（3）standby模式：功耗更低，只支持RTC闹钟唤醒、WKUP唤醒、NRST引脚复位和IWDG复位（打开了LSI和LSE）

（4）shutdown模式：功耗最低，只支持RTC闹钟唤醒、WKUP唤醒、NRST引脚复位（只打开了LSE）

### 2、官方参考手册介绍如下图

![](https://img2022.cnblogs.com/blog/1176509/202203/1176509-20220306211633396-22551909.png)

![](https://img2022.cnblogs.com/blog/1176509/202203/1176509-20220306211651642-1966041238.png)

![](https://img2022.cnblogs.com/blog/1176509/202203/1176509-20220306211711237-385088834.png)

### 3、工作模式转换图

![](https://img2022.cnblogs.com/blog/1176509/202203/1176509-20220306211727717-638225558.png)

### 4、扫盲知识

（1）STOP模式下，只要有外部中断进来就可以唤醒，无需用户自己配置具体代码去实现唤醒操作

（2）STOP模式下被唤醒之后，单片机先执行外部中断回调函数，然后再接着刚刚进入STOP模式下的语句继续执行

（3）待机模式下被唤醒之后，单片机是类似于REST，从头开始执行的

（4）RTC闹钟唤醒实质也就是外部中断唤醒，是由片内自己解决了

（5）外部中断唤醒之后，在重新初始化一些引脚配置

（6）对于串口唤醒这些特殊唤醒方式，其实使用的还是外部中断，进入低功耗之前需要将串口引脚重置然后配置成外部中断输入引脚，外部中断触发唤醒之后，再重新将引脚配置为串口即可

（7）对于一些输入脚进入低功耗之前可以全部配置为浮空输入，或者Anglog模式，是最省电的

（8）低功耗唤醒之后，默认时钟用的是HSI 8M，用户需要自己重新配置时钟，否则时钟不准确

（9）对于ADC脚想要外部中断唤醒，进入低功耗之前重新配置的之前需要使用HAL_ADC_DeInit(&hadc1);，否则可能不成功

> 唤醒调用流程：中断产生->中断服务函数->中断回调->低功耗函数->上下文继续执行



## 二、示例代码

> PowerManagement.h，移植时候只需要增加用户需要的唤醒源到eAwakeupSrc

```c
#ifndef __POWERMANAGEMENT_H__
#define __POWERMANAGEMENT_H__

#include "main.h"

typedef enum
{
	LP_SLEEP = (uint8_t)0x01,
	LP_DEEP_SLEEP,              
	LP_STOP0, 				/*!< Stop 0: stop mode with main regulator */
	LP_STOP1,				/*!< Stop 1: stop mode with low power regulator */
	LP_STANDBY, 				/*!< Standby mode */
	LP_SHUTDOWN, 				/*!< Shutdown mode */
} PowerMode;

// 唤醒源定义
typedef enum 
{
	NONE_WAKE = (uint32_t)0x00000000,
	AIN1_WAKE = (uint32_t)0x00000001,
	AIN2_WAKE = (uint32_t)0x00000002,
} eAwakeupSrc;

// 唤醒中断处理状态
typedef enum 
{
	no_Processed = (uint8_t)0,
	Processed,
} ProcStatus;

#define WAKE_MASK	(uint32_t)0xffffffff

typedef struct
{
	__IO uint32_t flag; 	// 中断唤醒标志位，支持32个中断标志
	__IO uint32_t mask; 	// 掩码
	ProcStatus isProcessed; // 是否已处理

} sAwakeupFlag;
extern sAwakeupFlag wakeFlag;

// 进入低功耗模式
void SystemEnterLowerPower(PowerMode mode);

#endif /* __POWERMANAGEMENT_H__ */
```



> PowerManagement.c，移植时候根据实际应用场景编写LowPowerPreProc函数

```c
#include "PowerManagement.h"
#include "debug.h"
#include "usart.h"
#include "gpio.h"

static void ExitLowPowerMode(void);
static void LowPowerPreProc(void);
static void EnterSleepMode(PowerMode mode);

// 定义唤醒标志变量
sAwakeupFlag wakeFlag = 
{
	NONE_WAKE,
	WAKE_MASK,
	Processed,
};

// 进入低功耗
void SystemEnterLowerPower(PowerMode mode)
{
	p_info("enter low power mode!\r\n");
	LowPowerPreProc();
	HAL_Delay(1000); // 等待1s进入低功耗，便于打印跟踪
	
	__HAL_RCC_PWR_CLK_ENABLE();

	switch(mode)
	{
	case LP_SLEEP:
		p_info("enter LP_SLEEP mode!\r\n");
		EnterSleepMode(LP_SLEEP);
		break;

	case LP_DEEP_SLEEP:
		p_info("enter LP_DEEP_SLEEP mode!\r\n");
		EnterSleepMode(LP_DEEP_SLEEP);
		break;
		
	case LP_STOP0:
		p_info("enter LP_STOP0 mode!\r\n");
		HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE);
		break;
		
	case LP_STOP1:
		p_info("enter LP_STOP1 mode!\r\n");
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
		break;
		
	case LP_STANDBY:
		p_info("enter LP_STANDBY mode!\r\n");
		HAL_PWR_EnterSTANDBYMode();
		break;

	case LP_SHUTDOWN:
		p_info("enter LP_SHUTDOWN mode!\r\n");
		HAL_PWREx_EnterSHUTDOWNMode();
		break;

	}

	ExitLowPowerMode();
}

/*
程序以WFI指令进入睡眠模式，所以只要产生任意中断都会退出睡眠模式。所以进入睡眠模式前先调?
肏AL_SuspendTick()函数挂起系统滴答定时器，否则将会被系统滴答定时器（SysTick
）中断在1ms内唤醒。程序运行到HAL_PWR_EnterSLEEPMode
（）函数时，系统进入睡眠模式，程序停止运行。当按下WAKEUP按键时，触发外部中断0
，此时系统被唤醒。继续执行HAL_ResumeTick()语句回复系统滴答定时器。
*/
static void EnterSleepMode(PowerMode mode)
{
  /* Suspend Tick increment to prevent wakeup by Systick interrupt.
     Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base) */
  HAL_SuspendTick();

  /* Request to enter SLEEP mode */
  if(mode == LP_SLEEP)
  	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  else if(mode == LP_DEEP_SLEEP)
  	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);

  /* Resume Tick interrupt if disabled prior to sleep mode entry */
  HAL_ResumeTick();
  HAL_PWREx_DisableLowPowerRunMode();
}

// 低功耗前预处理:把不需要维持的端口反初始化为模拟输入模式，降低功耗
static void LowPowerPreProc(void)
{
	p_dbg_enter;
    
 	HAL_GPIO_WritePin(GSM_POW_GPIO_Port, GSM_POW_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(WIFI_LED_SW_GPIO_Port, WIFI_LED_SW_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPRS_LED_SW_GPIO_Port, GPRS_LED_SW_Pin, GPIO_PIN_RESET);
 	HAL_GPIO_WritePin(CAN_LED_SW_GPIO_Port, CAN_LED_SW_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPS_LED_SW_GPIO_Port, GPS_LED_SW_Pin, GPIO_PIN_RESET);

	HAL_GPIO_DeInit(GSM_POW_GPIO_Port, GSM_POW_Pin);
	HAL_GPIO_DeInit(WIFI_LED_SW_GPIO_Port, WIFI_LED_SW_Pin);
	HAL_GPIO_DeInit(GPRS_LED_SW_GPIO_Port, GPRS_LED_SW_Pin);
	HAL_GPIO_DeInit(CAN_LED_SW_GPIO_Port, CAN_LED_SW_Pin);
	HAL_GPIO_DeInit(GPS_LED_SW_GPIO_Port, GPS_LED_SW_Pin);

	p_dbg_exit; // 此处再关闭串口前打印退出日志
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
	HAL_UART_DeInit(&huart1);
}

// 退出低功耗
static void ExitLowPowerMode(void)
{
	SystemClock_Config();  // 重新初始化为外部晶振
	
	MX_USART1_UART_Init(); // 初始化TRACE串口
	p_dbg_enter; // 此处在打开串口后打印跟踪日志
	
	MX_GPIO_Init();
	HAL_GPIO_WritePin(GSM_POW_GPIO_Port, GSM_POW_Pin, GPIO_PIN_SET);

	switch(wakeFlag.flag)
	{
	case AIN1_WAKE:
		p_dbg("AIN1 WAKE!");
		break;

	case AIN2_WAKE:
		p_dbg("AIN2 WAKE!");
		break;
	}
	wakeFlag.flag = (uint32_t)(wakeFlag.mask & NONE_WAKE);
	wakeFlag.isProcessed = Processed; // 退出前标记已经重新初始化相关外设
	p_dbg_exit;
}
```



> debug.c 调试接口文件，可以使用AT+cmdCfg进行调试，例如进入STOP1模式发送：AT+cmdCfg=101,1,4，序号见PowerMode
>
> 调试接口文件之前有分享过，详情可以见：https://www.cnblogs.com/veis/p/15086204.html

```c
#include "debug.h"
#include "usart.h"
#include <string.h>
#include "PowerManagement.h"

#define LOWPWR_CMD	101
#define ENTER_LWP	0x01

uint8_t DataRxBuffer[RX_BUF_MAX_LEN] = {0};
uint8_t dbg_rxdata = 0;
static uint32_t count = 0;

// debug串口接收记录集
STRUCT_USARTx_Fram dbg_Fram_Record = 
{
	DataRxBuffer,
	0
};

// 调试等级
int dbg_level = Monitor;

static int OnCfgDebug(uint32_t vp_Type, uint32_t vp_P1, uint32_t vp_P2, uint32_t vp_P3)
{
	p_info("info:OnCfgDebug:Type=%d,P1=%d,P2=%d,P3=%d.", vp_Type, vp_P1, vp_P2, vp_P3);
	
	switch(vp_Type)
	{
	case ENTER_LWP:
	{
		p_dbg("OK");
		SystemEnterLowerPower(vp_P1);
		break;
	}
	default:
		p_info("warn:PARAM INVALID!");
		break;
	}
	memset(DataRxBuffer, 0, RX_BUF_MAX_LEN);
	return 0;
}

// 格式：AT+cmdCfg=vl_CmdId,vl_Type,vl_P1,vl_P2,vl_P3
// 设置定时器命令：666
// 设置关闭和开始时间类型：1
// 开启时间和关闭时间：vl_P1，vl_P2
static int AT_DeviceHandle(const unsigned char *data_buf)
{
	count = 0;
	
	uint32_t i, vl_CmdId, vl_Type, vl_P1, vl_P2, vl_P3;
	uint32_t nlen = strlen((const char *)data_buf);
	char vl_FormateStr[64];
	
	vl_CmdId = 0;
	vl_Type = 0;
	vl_P1 = 0;
	vl_P2 = 0;
	vl_P3 = 0;
	
//	p_dbg("data_buf=%s", data_buf);
	if(!strstr((const char *)data_buf, "="))
		goto RETURN;

	memset(vl_FormateStr, 0, sizeof(vl_FormateStr)/sizeof(vl_FormateStr[0]));
	memcpy(vl_FormateStr, "AT+cmdCfg=%d", strlen("AT+cmdCfg=%d"));
	
//	p_dbg("nlen=%d", nlen);
	for (i = 0; i < nlen; i++)
	{
		if ((',' == data_buf[i]) && (i < nlen - 1))
			memcpy(vl_FormateStr + strlen(vl_FormateStr), ",%d", strlen(",%d"));
	}
//	p_dbg("vl_FormateStr=%s", vl_FormateStr);
	sscanf((const char *)data_buf, vl_FormateStr, &vl_CmdId, 
		&vl_Type, &vl_P1, &vl_P2, vl_P3);
	
	memset((char *)data_buf, 0, nlen);
	
	p_dbg("vl_CmdId=%d, vl_Type=%d, vl_P1=%d, vl_P2=%d, vl_P3=%d", vl_CmdId, vl_Type, vl_P1, vl_P2, vl_P3);
	
	if (LOWPWR_CMD == vl_CmdId)
		return OnCfgDebug(vl_Type, vl_P1, vl_P2, vl_P3);

RETURN:
	return -1;
}


/**
 * @brief      获取系统时间基准
 *
 * @return     返回系统CPU运行时间
 */
uint32_t os_time_get(void)
{
	return HAL_GetTick();
}

/**
 * @brief      串口接收回调函数
 *
 * @param      串口实例
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == DEBUG_USART)
	{
		if(dbg_rxdata != 0x0d && dbg_rxdata != 0x0a)
		{
			DataRxBuffer[count++] = dbg_rxdata;
		}
		else if(dbg_rxdata != 0x0a)
		{
			DataRxBuffer[count] = '\0';
			AT_DeviceHandle(DataRxBuffer); // 调用解析接口函数
		}
//		HAL_UART_Transmit(&huart1, &dbg_rxdata, 1, 0);
	}
	HAL_UART_Receive_IT(huart, &dbg_rxdata, 1);
}

/**
 * @brief      重写fputc
 *
 * @param[in]  ch    待发送参数
 * @param      f     设备文件
 *
 * @return     返回发送的字符
 */
int fputc(int ch, FILE *f)
{
	/* 重定向fputc函数到串口1 */
	HAL_UART_Transmit(&huart1, (unsigned char *)&ch, 1, 100);

	return (ch);
}
```

```c
#ifndef _DEBUG_H
#define _DEBUG_H

#include "main.h"

/* keil V5工具链中默认不支持匿名联合体，故需要声明下 */
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
```



> 主函数：main.c，主函数只是打印1Hz的Trace日志，便于观测MCU是否唤醒

```c
#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "PowerManagement.h"
#include "debug.h"

void SystemClock_Config(void);

int main(void)
{
  uint32_t nCount = 0;

  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  // 使能串口接收
  HAL_UART_Receive_IT(&huart1, &dbg_rxdata, 1);

    while (1)
    {
        nCount++;
        if(nCount != 0 && nCount % 10 == 0)
        {
            p_info("DEBUG_1HZ_TRACE:%d\r\n", HAL_GetTick());
            nCount = 0;
        }
        HAL_Delay(100);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV6;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief      下降沿中断回调函数
 *
 * @param[in]  GPIO_Pin		gpio引脚序号
 *
 * @return     空
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)
    {
	case AIN1_Pin:
                p_info("AIN1=1");
                wakeFlag.flag = (uint32_t)(wakeFlag.mask & AIN1_WAKE);
                wakeFlag.isProcessed = no_Processed;
                break;
	case AIN2_Pin:
		p_info("AIN2=1");
                wakeFlag.flag = (uint32_t)(wakeFlag.mask & AIN2_WAKE);
		wakeFlag.isProcessed = no_Processed;
		break;
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return
    state */
    __disable_irq();
    while (1)
    {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
    number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line
    ) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
```



## 三、测试log

> 进入stop1模式，通过PD11（AIN1）下降沿唤醒

![](https://img2022.cnblogs.com/blog/1176509/202203/1176509-20220306212008212-1172975721.png)

> 进入stop1模式，通过PD11（AIN2）下降沿唤醒

![](https://img2022.cnblogs.com/blog/1176509/202203/1176509-20220306212027585-1460348806.png)