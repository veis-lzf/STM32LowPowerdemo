#include "PowerManagement.h"
#include "debug.h"
#include "usart.h"
#include "gpio.h"

static void CAN2_RX_CfgExtiMode(void);
static void ExitLowPowerMode(void);
static void LowPowerPreProc(void);
static void EnterSleepMode(PowerMode mode);

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
	HAL_Delay(1000); // 等待1s进入低功耗
	
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


static void CAN2_RX_CfgExtiMode(void)
{
	HAL_GPIO_DeInit(CAN2_RX_GPIO_Port, CAN2_RX_Pin);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = CAN2_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(CAN2_RX_GPIO_Port, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

// 低功耗前预处理
static void LowPowerPreProc(void)
{
	p_dbg_enter;
	// 初始化CAN2_RX引脚作为外部中断
	CAN2_RX_CfgExtiMode();
		
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
	case CAN2_WAKE:
		p_dbg("CAN2 WAKE!");
		break;

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


