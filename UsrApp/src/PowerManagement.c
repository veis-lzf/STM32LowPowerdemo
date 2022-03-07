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

// ����͹���
void SystemEnterLowerPower(PowerMode mode)
{
	p_info("enter low power mode!\r\n");
	LowPowerPreProc();
	HAL_Delay(1000); // �ȴ�1s����͹���
	
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
������WFIָ�����˯��ģʽ������ֻҪ���������ж϶����˳�˯��ģʽ�����Խ���˯��ģʽǰ�ȵ��
�HAL_SuspendTick()��������ϵͳ�δ�ʱ�������򽫻ᱻϵͳ�δ�ʱ����SysTick
���ж���1ms�ڻ��ѡ��������е�HAL_PWR_EnterSLEEPMode
��������ʱ��ϵͳ����˯��ģʽ������ֹͣ���С�������WAKEUP����ʱ�������ⲿ�ж�0
����ʱϵͳ�����ѡ�����ִ��HAL_ResumeTick()���ظ�ϵͳ�δ�ʱ����
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

// �͹���ǰԤ����
static void LowPowerPreProc(void)
{
	p_dbg_enter;
	// ��ʼ��CAN2_RX������Ϊ�ⲿ�ж�
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

	p_dbg_exit; // �˴��ٹرմ���ǰ��ӡ�˳���־
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
	HAL_UART_DeInit(&huart1);
}

// �˳��͹���
static void ExitLowPowerMode(void)
{
	SystemClock_Config();  // ���³�ʼ��Ϊ�ⲿ����
	
	MX_USART1_UART_Init(); // ��ʼ��TRACE����
	p_dbg_enter; // �˴��ڴ򿪴��ں��ӡ������־
	
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
	wakeFlag.isProcessed = Processed; // �˳�ǰ����Ѿ����³�ʼ���������
	p_dbg_exit;
}


