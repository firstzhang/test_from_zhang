#include "stm32f10x.h"
#include "led.h"
#include "usart.h"
#include "bluetooth.h"
#include "DMA.h"
#include "FreeRTOS.h"
#include "task.h"


uint8_t p_feedbackbuff[30];

TaskHandle_t  start_app_handle = NULL;
TaskHandle_t  set_bluetooth_Handle = NULL;
TaskHandle_t  working_Handle = NULL;

void start_app(void);
void set_bluetooth(void);
void working(void);
#if	configGENERATE_RUN_TIME_STATS
TaskHandle_t	prinf_info_Handle = NULL;
#include "base_timer.h"
static void prinf_info()
{
	uint8_t CPU_RunInfo[400];
	while(1)
	{
		memset(CPU_RunInfo,0,400);
		
		vTaskList((char *)CPU_RunInfo);
		printf("name          state priority  stack  task_numb\r\n");
		printf("%s\r\n",CPU_RunInfo);

		memset(CPU_RunInfo,0,400);	
	
		vTaskGetRunTimeStats((char *)CPU_RunInfo);
		printf("name		run_count	use_percent\r\n");
		printf("%s\r\n",CPU_RunInfo);

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
#endif
int main(void)
{
	led_config();
	usart_config();
 	printf("你好！我在这里\n\r");
	buletooth_config();
	#if	configGENERATE_RUN_TIME_STATS
	base_timer_config();
	#endif
	if( pdTRUE == xTaskCreate((TaskFunction_t)start_app,"start_app",512,NULL,1,&start_app_handle))
	{
		vTaskStartScheduler();
	}
	while(1);
}

void start_app()
{
	taskENTER_CRITICAL(); 
	xTaskCreate((TaskFunction_t)set_bluetooth,"set_bluetooth",512,NULL,2,&set_bluetooth_Handle);
	xTaskCreate((TaskFunction_t)working,"working",512,NULL,3,&working_Handle);
	#if	configGENERATE_RUN_TIME_STATS
	xTaskCreate((TaskFunction_t)prinf_info,"prinf_info",512,NULL,4,&prinf_info_Handle);	
	#endif
	vTaskDelete(NULL);
	taskEXIT_CRITICAL(); 
}

void set_bluetooth()
{
	
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	vTaskDelay(1);
	bluetooth_send_CMD("AT\r\n");
	bluetooth_send_CMD("AT+RESET\r\n");   //复位
	bluetooth_send_CMD("AT+ROLE=0\r\n");	//从模式
	bluetooth_send_CMD("AT+INIT\r\n");
	bluetooth_send_CMD("AT+CLASS=0\r\n");	
	bluetooth_send_CMD("AT+INQM=1,1,48\r\n");
	bluetooth_send_CMD("AT+NAME=SMART\r\n"); //命名
	bluetooth_send_CMD("AT+PSWD=8658\r\n");	//密码
//--------------不必要项目----------------
	bluetooth_send_CMD("AT+PSWD?\r\n");			//查询密码
	
	bluetooth_send_CMD("AT+ORGL\r\n");			//恢复默认值
	bluetooth_send_CMD("AT+VERSION?\r\n");
	bluetooth_send_CMD("AT+ADDR?\r\n");
	bluetooth_send_CMD("AT+UART?\r\n");
	bluetooth_send_CMD("AT+CMODE?\r\n");
	bluetooth_send_CMD("AT+STATE?\r\n");
	bluetooth_send_CMD("AT+INQM?\r\n");
	//can't reach here
	GPIO_ResetBits(GPIOB,GPIO_Pin_14); 
	taskENTER_CRITICAL(); 
	vTaskDelete(NULL);				//这两个切换任务的操作要在临界段里面完成
//	xTaskNotifyGive(working_Handle);//因为一旦退出临界段，系统节拍器就会切换任务，当前任务被删除永远也运行不了
														//或者也可以把删除函数放在切换任务的操作前，但是这样不安全切不能删除此任务
	taskEXIT_CRITICAL();
	while(1);
}


void working()
{
	uint32_t iterator=0;
//	prinf_info();
//	if(ulTaskNotifyTake(pdTRUE,portMAX_DELAY) != 1)
//		vTaskDelete(NULL);
	while(iterator != 0xFFFF)
	{
		xTaskNotifyWait(0,0,&iterator,portMAX_DELAY);
	}
		GPIO_ResetBits(GPIOB,GPIO_Pin_14);
		vTaskDelete(set_bluetooth_Handle);
	printf("event = 0x%x\r\n",iterator);
	USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
	DMA_config();
//	prinf_info();

	while(1)	
	{

		if(p_feedbackbuff[0] == '1')
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_1|GPIO_Pin_5);
			GPIO_SetBits(GPIOB,GPIO_Pin_0);
		}
		if(p_feedbackbuff[0]  == '2')
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_5);
			GPIO_SetBits(GPIOB,GPIO_Pin_1);
		}
		if(p_feedbackbuff[0]  == '3')
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_1|GPIO_Pin_0);
			GPIO_SetBits(GPIOB,GPIO_Pin_5);
		}
		if(p_feedbackbuff[0]  == '4')
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_5);
		}
		if(p_feedbackbuff[0] == '5')
		{
		GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_5);
		}
	}
}



