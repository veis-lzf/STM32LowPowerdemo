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

// ½øÈëµÍ¹¦ºÄ
void SystemEnterLowerPower(PowerMode mode)
{
	p_info("enter low power mode!\r\n");
	LowPowerPreProc();
	HAL_Delay(1000); // µÈ´ý1s½øÈëµÍ¹¦ºÄ
	
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
³ÌÐòÒÔWFIÖ¸Áî½øÈëË¯ÃßÄ£Ê½£¬ËùÒÔÖ»Òª²úÉúÈÎÒâÖÐ¶Ï¶¼»áÍË³öË¯ÃßÄ£Ê½¡£ËùÒÔ½øÈëË¯ÃßÄ£Ê½Ç°ÏÈµ÷Ó
ÃHAL_SuspendTick()º¯Êý¹ÒÆðÏµÍ³µÎ´ð¶¨Ê±Æ÷£¬·ñÔò½«»á±»ÏµÍ³µÎ´ð¶¨Ê±Æ÷£¨SysTick
£©ÖÐ¶ÏÔÚ1msÄÚ»½ÐÑ¡£³ÌÐòÔËÐÐµ½HAL_PWR_EnterSLEEPMode
£¨£©º¯ÊýÊ±£¬ÏµÍ³½øÈëË¯ÃßÄ£Ê½£¬³ÌÐòÍ£Ö¹ÔËÐÐ¡£µ±°´ÏÂWAKEUP°´¼üÊ±£¬´¥·¢Íâ²¿ÖÐ¶Ï0
£¬´ËÊ±ÏµÍ³±»»½ÐÑ¡£¼ÌÐøÖ´ÐÐHAL_ResumeTick()Óï¾ä»Ø¸´ÏµÍ³µÎ´ð¶¨Ê±Æ÷¡£
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

// µÍ¹¦ºÄÇ°Ô¤´¦Àí
static void LowPowerPreProc(void)
{
	p_dbg_enter;
	// ³õÊ¼»¯CAN2_RXÒý½Å×÷ÎªÍâ²¿ÖÐ¶Ï
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

	p_dbg_exit; // ´Ë´¦ÔÙ¹Ø±Õ´®¿ÚÇ°´òÓ¡ÍË³öÈÕÖ¾
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
	HAL_UART_DeInit(&huart1);
}

// ÍË³öµÍ¹¦ºÄ
static void ExitLowPowerMode(void)
{
	SystemClock_Config();  // ÖØÐÂ³õÊ¼»¯ÎªÍâ²¿¾§Õñ
	
	MX_USART1_UART_Init(); // ³õÊ¼»¯TRACE´®¿Ú
	p_dbg_enter; // ´Ë´¦ÔÚ´ò¿ª´®¿Úºó´òÓ¡¸ú×ÙÈÕÖ¾
	
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
	wakeFlag.isProcessed = Processed; // ÍË³öÇ°±ê¼ÇÒÑ¾­ÖØÐÂ³õÊ¼»¯Ïà¹ØÍâÉè
	p_dbg_exit;
}


