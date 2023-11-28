#include "stm32f4xx.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"
#include "led.h"
#include "uart1.h"
#include "stdio.h"
#include "key.h"

#define START_TASK_PRIO 1
#define START_STAKE_SIZE 128
static TaskHandle_t StartTask_Handler;

#define KEY_TASK_PRIO 5
#define KEY_STAKE_SIZE 128
static TaskHandle_t KeyTask_Handler;

#define LED_TASK_PRIO 1
#define LED_STAKE_SIZE 128
static TaskHandle_t LedTask_Handler;
bool_t led_flag = 0;

void KeyTask(void *parameter);
void LedTask(void *parameter);

void StartTask(void *parameter)
{
	//进入临界段
	taskENTER_CRITICAL();

	xTaskCreate((TaskFunction_t)KeyTask,
				(const char *)"KeyTask",
				(const uint16_t)KEY_STAKE_SIZE,
				(void *)NULL,
				(UBaseType_t)KEY_TASK_PRIO,
				(TaskHandle_t*)&KeyTask_Handler);

	xTaskCreate((TaskFunction_t)LedTask,
				(const char *)"LedTask",
				(const uint16_t)LED_STAKE_SIZE,
				(void *)NULL,
				(UBaseType_t)LED_TASK_PRIO,
				(TaskHandle_t*)&LedTask_Handler);
	
	vTaskSuspend(NULL);
	//删除开始任务
	vTaskDelete(StartTask_Handler);
	//退出临界段
	taskEXIT_CRITICAL();

}
uint8_t KeyScan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0)
	{
		delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0);
		return 1;
	}
	else
		return 0;
}
void KeyTask(void *parameter)	
{
	while(1)
	{
		if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0) && (led_flag == 0))
		{
			vTaskDelay(10);
			while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0);
			printf("挂起LED任务！ \r\n");
			vTaskSuspend(LedTask_Handler);
			led_flag = 1;
			led_red_toggle();
		}
		else if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0) && (led_flag == 1))
		{
			vTaskDelay(10);
			while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0);
			printf("恢复LED任务！ \r\n");
			vTaskResume(LedTask_Handler);
			led_flag = 0;
			led_red_toggle();
		}
		vTaskDelay(20);
	}
}

void LedTask(void *parameter)
{
	while(1)
	{
		led_blue_on();
		printf("LedTask Running,LedBlue ON\r\n");
		vTaskDelay(500);

		led_blue_off();
		printf("LedTask Running,LedBlue OFF\r\n");
		vTaskDelay(500);
	}
}


int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	led_Init();
	delay_init((uint8_t)180);
	UART1_Init();
	key_Init();
	
	xTaskCreate((TaskFunction_t)StartTask,
				(const char*)"start_task",
				(uint16_t)START_STAKE_SIZE,
				(void *)NULL,
				(UBaseType_t)START_TASK_PRIO,
				(TaskHandle_t*)&StartTask_Handler);

	vTaskStartScheduler(); 

	while(1)
	{
		//正常启动调度器后，不会运行到此处
	}
}

