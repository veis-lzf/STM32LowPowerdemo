 #ifndef __POWERMANAGEMENT_H__
#define __POWERMANAGEMENT_H__

#include "main.h"

typedef enum
{
	LP_SLEEP = (uint8_t)0x01,
	LP_DEEP_SLEEP,              
	LP_STOP0, 					/*!< Stop 0: stop mode with main regulator */
	LP_STOP1,					/*!< Stop 1: stop mode with low power regulator */
	LP_STANDBY, 				/*!< Standby mode */
	LP_SHUTDOWN, 				/*!< Shutdown mode */
} PowerMode;

// 唤醒源定义
typedef enum 
{
	NONE_WAKE = (uint32_t)0x00000000,
	AIN1_WAKE = (uint32_t)0x00000001,
	AIN2_WAKE = (uint32_t)0x00000002,
	CAN1_WAKE = (uint32_t)0x00000004,
	CAN2_WAKE = (uint32_t)0x00000008,
	
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

