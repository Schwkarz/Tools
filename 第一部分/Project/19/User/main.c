#include "stm32f4xx.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"
#include "led.h"
#include "uart1.h"
#include "stdio.h"
#include "timers.h"

#define START_TASK_PRIO 1
#define START_STAKE_SIZE 128
static TaskHandle_t StartTask_Handler;

TimerHandle_t Timer1_Handler = NULL;
static void Timer1_Callback(void *parameter)
{
	static uint16_t timer_count = 0;
	printf("这是第%d次进入回调函数! \r\n",timer_count);
}

void StartTask(void *parameter)
{
	//进入临界段
	taskENTER_CRITICAL();
	
	Timer1_Handler = xTimerCreate((const char *)"Timer1",
								  (const TickType_t)1000,
								  (BaseType_t)pdTRUE,
								  (void * const)1,
								  (TimerCallbackFunction_t)Timer1_Callback);
	xTimerStart(Timer1_Handler,portMAX_DELAY);
	//删除开始任务
	vTaskDelete(StartTask_Handler);
	//退出临界段
	taskEXIT_CRITICAL();

}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	led_Init();
	delay_init((uint8_t)180);
	UART1_Init();
	
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

