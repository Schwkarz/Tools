#include "stm32f4xx.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"
#include "led.h"
#include "uart1.h"
#include "stdio.h"
#include "queue.h"


QueueHandle_t Test_Queue = NULL;
#define QUEUE_LENGTH 4
#define QUEUE_SIZE 4



#define START_TASK_PRIO 1
#define START_STAKE_SIZE 128
static TaskHandle_t StartTask_Handler;

#define QUEUERECEIVE_TASK_PRIO 3
#define QUEUERECEIVE_STAKE_SIZE 128
static TaskHandle_t QueueReceiveTask_Handler;

#define QUEUESEND_TASK_PRIO 2
#define QUEUESEND_STAKE_SIZE 128
static TaskHandle_t QueueSendTask_Handler;

void QueueReceiveTask(void *parameter)
{
	BaseType_t xReturn = pdFALSE;
	uint32_t r_queue;
	while(1)
	{
		xReturn = xQueueReceive(Test_Queue,
								&r_queue,
								portMAX_DELAY);
		if(xReturn == pdFALSE)
		{
			printf("接收出错！错误代码：0x%lx\r\n",xReturn);
		}
		else
		{
			taskENTER_CRITICAL();
			printf("The received data is : %d \r\n",r_queue);\
			taskEXIT_CRITICAL();
		}
	}
}

void QueueSendTask(void *parameter)
{
	BaseType_t xReturn = pdFALSE;
	uint32_t queue_send[5];
	for(uint8_t i = 0;i < 5;i++)
	{
		queue_send[i] = i*i*i;
	}
	while(1)
	{
		led_red_toggle();
		static uint8_t n = 0;
		xReturn = xQueueSend(Test_Queue,
				   &queue_send[n++],
				   portMAX_DELAY);
		if(xReturn == pdTRUE)
		{
			printf("第%d次发送成功 \r\n",n);
		}
		else
		{
			printf("第%d次发送失败 \r\n",n);
		}
		if(n == 5)
		{
			n=0;
		}
		vTaskDelay(1000);
	}
}

void StartTask(void *parameter)
{
	//进入临界段
	taskENTER_CRITICAL();

	Test_Queue = xQueueCreate((UBaseType_t) QUEUE_LENGTH,
							  (UBaseType_t) QUEUE_SIZE);
	if(Test_Queue!=NULL)
	{
		printf("创建Test_Queue消息队列成功！ \r\n");
	}

	xTaskCreate((TaskFunction_t)QueueReceiveTask,
				(const char*)"QueueReceiveTask",
				(uint16_t)QUEUERECEIVE_STAKE_SIZE,
				(void *)NULL,
				(UBaseType_t)QUEUERECEIVE_TASK_PRIO,
				(TaskHandle_t*)&QueueReceiveTask_Handler);

	xTaskCreate((TaskFunction_t)QueueSendTask,
				(const char*)"QueueSendTask",
				(uint16_t)QUEUESEND_STAKE_SIZE,
				(void *)NULL,
				(UBaseType_t)QUEUESEND_TASK_PRIO,
				(TaskHandle_t*)&QueueSendTask_Handler);
	printf("创建任务成功\r\n");
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

