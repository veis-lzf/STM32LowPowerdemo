#include "debug.h"
#include "usart.h"
#include <string.h>
#include "PowerManagement.h"

#define LOWPWR_CMD	101
#define ENTER_LWP	0x01

uint8_t DataRxBuffer[RX_BUF_MAX_LEN] = {0};
uint8_t dbg_rxdata = 0;
static uint32_t count = 0;

// debug���ڽ��ռ�¼��
STRUCT_USARTx_Fram dbg_Fram_Record = 
{
	DataRxBuffer,
	0
};

// ���Եȼ�
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

// ��ʽ��AT+cmdCfg=vl_CmdId,vl_Type,vl_P1,vl_P2,vl_P3
// ���ö�ʱ�����666
// ���ùرպͿ�ʼʱ�����ͣ�1
// ����ʱ��͹ر�ʱ�䣺vl_P1��vl_P2
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
 * @brief      ��ȡϵͳʱ���׼
 *
 * @return     ����ϵͳCPU����ʱ��
 */
uint32_t os_time_get(void)
{
	return HAL_GetTick();
}

/**
 * @brief      ���ڽ��ջص�����
 *
 * @param      ����ʵ��
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
			AT_DeviceHandle(DataRxBuffer); // ���ý����ӿں���
		}
//		HAL_UART_Transmit(&huart1, &dbg_rxdata, 1, 0);
	}
	HAL_UART_Receive_IT(huart, &dbg_rxdata, 1);
}

/**
 * @brief      ��дfputc
 *
 * @param[in]  ch    �����Ͳ���
 * @param      f     �豸�ļ�
 *
 * @return     ���ط��͵��ַ�
 */
int fputc(int ch, FILE *f)
{
	/* �ض���fputc����������1 */
	HAL_UART_Transmit(&huart1, (unsigned char *)&ch, 1, 100);

	return (ch);
}
