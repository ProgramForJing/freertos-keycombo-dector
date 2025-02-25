#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Semphr.h"
#include "Assist.h"
#include "Key.h"
#include <stdlib.h>

TaskHandle_t xTaskHandle_BuzzerFlash = NULL;

void Key_vKeyTask(void * pvParameters);
void Key_Func_key1(Key_Structure * keyStructure);
void Key_Func_key2(Key_Structure * keyStructure);
void vBuzzerFlashFunc(void * pvParameters);

Key_Structure key1;
Key_Structure key2;

TaskHandle_t keyTaskHandle;
/*!
 *  命令输入模式下的命令回调函数，在这些回调函数中设置对应命令想要执行的功能
 *	L表示长按，S表示短按
 *	LL表示连续两次长按， LS表示一次长按后再一次短按
 *  依次类推
 */
void Key_Command_Key1_L(void);
void Key_Command_Key1_S(void);
void Key_Command_Key1_LL(void);
void Key_Command_Key1_LS(void);
void Key_Command_Key1_SL(void);
void Key_Command_Key1_SS(void);

/*!
 *  使用一个蜂鸣器和LED灯来验证按键检测模块
 */

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	Key_StructInit(&key1);
	key1.inputMode = INPUT_MODE_COMMAND;
	key1.commandFunc[0] = Key_Command_Key1_L;
	key1.commandFunc[1] = Key_Command_Key1_S;
	key1.commandFunc[2] = Key_Command_Key1_LL;
	key1.commandFunc[3] = Key_Command_Key1_LS;
	key1.commandFunc[4] = Key_Command_Key1_SL;
	key1.commandFunc[5] = Key_Command_Key1_SS;
	Key_Init(&key1, GPIOB, GPIO_Pin_1, "key1");
	
	Key_StructInit(&key2);
	Key_Init(&key2, GPIOB, GPIO_Pin_0, "key2");

	xTaskCreate(Key_vKeyTask, "GetKeyStatusTask", 1024, NULL, 4, &keyTaskHandle);
	
	LED_Init();
	Buzzer_Init();
	Buzzer_Ring(DISABLE);

	vTaskStartScheduler();
	
	while(1);
}

/**< 按键任务函数 */
void Key_vKeyTask(void * pvParameters)
{
	for(;;)
	{
		Key_Func_key1(&key1);
		Key_Func_key2(&key2);
		vTaskDelay(20);
	}
}

 /*******************************************************************************
  * @brief    设置按键按下、长按、短按、松开时想要执行的功能
  *
  * @param    [in]keyStructure  按键结构体指针，包含按键一系列信息
  *
  * @retval   void
  ******************************************************************************/
void Key_Func_key1(Key_Structure * keyStructure)
{
	KEY_STATE ks = Key_GetStatus(keyStructure);
	if(ks == KEY_STATE_DISABLE)
	{
		//按键失能
		//执行相应操作
		//...
	}
	else if(ks == KEY_STATE_INVALID)
	{
		//输入无效
		//执行相应操作
		//...
	}
	if(keyStructure->inputMode == INPUT_MODE_NORMAL)
	{
		if(ks == KEY_STATE_DOWN)
		{
			//这里执行按键按下时应该执行的动作
			//...
			
			TickType_t currentTime = xTaskGetTickCount();
			if(currentTime - keyStructure->pressEntryPoint > keyStructure->shortPressThreshold &&
				currentTime - keyStructure->pressEntryPoint < keyStructure->longPressErrorThreshold)
			{
				//执行长按时应该执行的动作
				//...
			}
			else if(currentTime - keyStructure->pressEntryPoint >= keyStructure->longPressErrorThreshold)
			{
				//长按时间过长时要执行的动作
				//...
			}
			
		}
		else if(ks == KEY_STATE_UP)
		{
			if(keyStructure->prevprevState == KEY_STATE_DOWN)
			{
				//这里执行按键按下一次后应该执行的动作，不论长按还是短按
				//...
			}
		}
	}
	else
	{
		//命令输入模式下会通过软件定时器一段时间后自动执行命令
	}
}

void Key_Func_key2(Key_Structure * keyStructure)
{
	KEY_STATE ks = Key_GetStatus(keyStructure);
	if(ks == KEY_STATE_DISABLE)
	{
		//按键失能
		//执行相应操作
		//...
	}
	else if(ks == KEY_STATE_INVALID)
	{
		//输入无效
		//执行相应操作
		//...
	}
	if(keyStructure->inputMode == INPUT_MODE_NORMAL)
	{
		if(ks == KEY_STATE_DOWN)
		{
			//这里执行按键按下时应该执行的动作
			//...
			if(xTaskHandle_BuzzerFlash == NULL)
			{
				Buzzer_Ring(ENABLE);
			}
			
			TickType_t currentTime = xTaskGetTickCount();
			if(currentTime - keyStructure->pressEntryPoint > keyStructure->shortPressThreshold &&
				currentTime - keyStructure->pressEntryPoint < keyStructure->longPressErrorThreshold)
			{
				//执行长按时应该执行的动作
				//...
				if(xTaskHandle_BuzzerFlash == NULL)
				{
					xTaskCreate(vBuzzerFlashFunc, "BuzzerFlash", 512, NULL, 3, &xTaskHandle_BuzzerFlash);
				}
			}
			else if(currentTime - keyStructure->pressEntryPoint >= keyStructure->longPressErrorThreshold)
			{
				//长按时间过长时要执行的动作
				//...
				if(xTaskHandle_BuzzerFlash != NULL)
				{
					vTaskDelete(xTaskHandle_BuzzerFlash);
					xTaskHandle_BuzzerFlash = NULL;
				}
				Buzzer_Ring(DISABLE);
			}
			
		}
		else if(ks == KEY_STATE_UP)
		{
			if(keyStructure->prevprevState == KEY_STATE_DOWN)
			{
				//这里执行按键按下一次后应该执行的动作，不论长按还是短按
				//...
				Buzzer_Ring(DISABLE);
			}
		}
	}
	else
	{
		//命令输入模式下会通过软件定时器一段时间后自动执行命令
	}
}

void vBuzzerFlashFunc(void * pvParameters)
{
	for(;;)
	{
		Buzzer_Ring(DISABLE);
		vTaskDelay(100);
		Buzzer_Ring(ENABLE);
		vTaskDelay(100);
		Buzzer_Ring(DISABLE);
		vTaskDelay(100);
		Buzzer_Ring(ENABLE);
		vTaskDelay(100);
	}
}

void Key_Command_Key1_SS(void)
{
	LED1_Light(ENABLE);
	LED2_Light(ENABLE);
}

void Key_Command_Key1_S(void)
{
	LED1_Light(ENABLE);
}

void Key_Command_Key1_L(void)
{
	LED1_Light(DISABLE);
}

void Key_Command_Key1_SL(void)
{
	LED1_Light(ENABLE);
	LED2_Light(DISABLE);
}

void Key_Command_Key1_LS(void)
{
	LED1_Light(DISABLE);
	LED2_Light(ENABLE);
}

void Key_Command_Key1_LL(void)
{
	LED2_Light(DISABLE);
	LED1_Light(DISABLE);
}
