/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : main.c
**   Project     : DQA
**   Author      : Nguyen Trong Tri
**   Version     : 1.0
**   Date        : 2013/03/31
**   Description : This is a main module.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Application includes */
//#include "main.h"
#include "data_types.h"
#include "rtc.h"
#include "adc_dma.h"
#include "cli.h"
#include "parameters.h"
#include "fwupgrade.h"
#include "sub_dev_rx.h"

/* USB Lib. */
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "usb_bsp.h"


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Led signal. */
#define LED_CODE_GPIO_PORT                        GPIOC
#define LED_CODE_GPIO_CLK                         RCC_AHB1Periph_GPIOC
#define LED_CODE_GPIO_PIN                         GPIO_Pin_4

#define LED_SCALE_GPIO_PORT                       GPIOB
#define LED_SCALE_GPIO_CLK                        RCC_AHB1Periph_GPIOB
#define LED_SCALE_GPIO_PIN                        GPIO_Pin_0

#define BLINK_LED_TASK_DELAY                      (portTickType)(1000 / portTICK_RATE_MS)
#define BLINK_LED_TASK_STACK_SIZE                 (configMINIMAL_STACK_SIZE)
#define BLINK_LED_TASK_PRIORITY                   (tskIDLE_PRIORITY + 1)


/* USB Host process task delay (10mS)*/
#define USBH_PROCESS_TASK_DELAY                   (portTickType)(10 / portTICK_RATE_MS)

/* USB Host process task stack size. */
#define USBH_PROCESS_TASK_STACK_SIZE              (configMINIMAL_STACK_SIZE*8)

/* USB Host process task priority. */
#define USBH_PROCESS_TASK_PRIORITY                (tskIDLE_PRIORITY + 2)


/* Init Function task delay (1mS)*/
#define INIT_FUNC_TASK_DELAY                      (portTickType)(1 / portTICK_RATE_MS)

/* USB Host process task stack size. */
#define INIT_FUNC_TASK_STACK_SIZE                 (configMINIMAL_STACK_SIZE)

/* USB Host process task priority. */
#define INIT_FUNC_TASK_PRIORITY                   (tskIDLE_PRIORITY + 2)

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void TIM_Configuration_Delay (void);

/* De-Initialize all Interrupt. */
void v_DeInit_NVIC(void);

/* USB Host Process Init. */
static void v_USBH_Process_Init (void);

/* USB Host Process Task. */
static void v_USBH_Process_Task (void *pvParameters);

/* Init Led. */
static void v_Led_Init (void);

/* Blink Led Task. */
static void v_Blink_Led_Code_Task  (void *pvParameters);

/* Create a task to initialize all modules. */
static void v_Initial_Func_Init (void);
static void v_Initial_Func_Task (void *pvParameters);

extern void USB_OTG_BSP_DeInit(void);

void vApplicationStackOverflowHook (xTaskHandle *pxTask, signed portCHAR *pcTaskName);

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*Init Task handle */
xTaskHandle xInit_Func_Task_Handle;


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int main (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : This is a main function of the application.
**                 It initializes the hardware, drivers, application modules, RTOS...
**   Notes       : This function must call a vTaskStartScheduler() function at
**                 the end to transfer the management to FreeRTOS.
**                 Should never return.
**   Author      : Nguyen Anh Huy
**   Date        : 2009/09/01
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int main (void)
{
  /*!< At this stage the microcontroller clock setting is already configured to 
       120 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f2xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f2xx.c file
     */  

#ifdef SERIAL_DEBUG
  DebugComPort_Init();
#endif

   /* Initialize a init func task. */
   v_Initial_Func_Init();

   /* Start scheduler */
   vTaskStartScheduler();
   
   /* We should never get here as control is now taken by the scheduler */
   for( ;; );
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_DeInit_NVIC (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : 
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/10/10
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_DeInit_NVIC (void)
{
   USB_OTG_BSP_DeInit();
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static void v_Initial_Func_Init (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Initialize a initial task.
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/12/12
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_Initial_Func_Init (void)
{
   xTaskCreate(v_Initial_Func_Task, (signed portCHAR *)"Init Task", INIT_FUNC_TASK_STACK_SIZE,
               NULL,INIT_FUNC_TASK_PRIORITY, &xInit_Func_Task_Handle);
}
/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static void v_Initial_Func_Task (void *pvParameters);
**   Arguments   : n/a
**   Return      : n/a
**   Description : 
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/12/12
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_Initial_Func_Task (void *pvParameters)
{
   for (;;)
   {
      NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
   
      /* Init TIMER to use Delay ms. */
      TIM_Configuration_Delay();
   
      /* Init Led. */
      v_Led_Init();
		 
		  /* Init RTC. */
		  rtc();
		 
			/* Init ADC. */
		  v_ADC3_DMA2_Config();
		 
		  v_CLI_Init();
		 
      /* Init Parameter module. */
      v_PARAM_Init();
		 
		  v_FW_Main_CPU_Update_Init();
		
		  v_SubDeviceUARTInit(115200);
		 
      /* Init Display module. */
			// v_Display_Init();
   
      /* Init Log module. */
			// v_LOG_Init();
   
      /* De-Init Host Library */
      USBH_DeInit(&USB_OTG_Core, &USB_Host);
		 
      /* Init Host Library */
      USBH_Init(&USB_OTG_Core, 
#ifdef USE_USB_OTG_FS  
               USB_OTG_FS_CORE_ID,
#else 
               USB_OTG_HS_CORE_ID,
#endif 
               &USB_Host,
               &USBH_MSC_cb, 
               &USR_Callbacks);
        
       /* Init USBH Process Task. */
      v_USBH_Process_Init ();

      vTaskDelay(INIT_FUNC_TASK_DELAY);
      vTaskDelete (xInit_Func_Task_Handle);
   }
}
/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static void v_USBH_Process_Init (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : 
**   Notes       : 
**   Author      : Nguyen Trong Tri
**   Date        : 2012/07/30
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

static void v_USBH_Process_Init (void)
{
   x_USB_USR_Queue = xQueueCreate(USH_USR_QUEUE_SIZE, sizeof(USH_USR_MSG));
   /* Create USB Host process task. */
   xTaskCreate(v_USBH_Process_Task, (signed portCHAR *)"USBH_Process", USBH_PROCESS_TASK_STACK_SIZE,
               NULL, USBH_PROCESS_TASK_PRIORITY, NULL);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static void v_USBH_Process_Task (void *pvParameters)
**   Arguments   : n/a
**   Return      : n/a
**   Description : 
**   Notes       : 
**   Author      : Nguyen Anh Huy
**   Date        : 2012/07/30
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_USBH_Process_Task (void *pvParameters)
{
   for (;;)
   {
      /* Delay USB Host Process task for 10ms. */
      vTaskDelay(USBH_PROCESS_TASK_DELAY);
      USBH_Process(&USB_OTG_Core, &USB_Host);
   }
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static void v_Led_Init (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Init Led Signal to know code is alive and valid packet from scale.
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/11/12
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_Led_Init (void)
{
	 STM_EVAL_LEDInit(LED3);

   xTaskCreate(v_Blink_Led_Code_Task, (signed portCHAR *)"Blink Led", BLINK_LED_TASK_STACK_SIZE,
               NULL, BLINK_LED_TASK_PRIORITY, NULL);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_Blink_Led_Valid_Packet_Scale (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Blink Led when got a valid packet from scale.
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/11/12
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Blink_Led_Valid_Packet_Scale (void)
{

   /* Toggle Led pin. */
   GPIO_ToggleBits(LED_SCALE_GPIO_PORT, LED_SCALE_GPIO_PIN);
   vTaskDelay(200);
   /* Toggle Led pin. */
   GPIO_ToggleBits(LED_SCALE_GPIO_PORT, LED_SCALE_GPIO_PIN);
}
/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static void v_Blink_Led_Task (void *pvParameters)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Blink Led Task.
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/11/12
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_Blink_Led_Code_Task  (void *pvParameters)
{
   for (;;)
	 {
      /*Delay 1s. */
      vTaskDelay(BLINK_LED_TASK_DELAY);
      /* Toggle Led pin. */
      STM_EVAL_LEDToggle(LED3);
		  /*Upgrade firmware poll */
		  v_FW_Main_CPU_Update_Poll(); 
   }
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void TIM_Configuration_Delay(void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Init timer 3
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/11/10
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

void TIM_Configuration_Delay (void)
{
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
   /* Reset TIM3 IP */
   RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, ENABLE);
   /* Release reset signal of TIM3 IP */
   RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, DISABLE);

}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_Delay_ms (unsigned int u32_delay_period_ms)
**   Arguments   : unsigned int u32_delay_period_ms: number of miliseconds
**   Return      : n/a
**   Description : Delay n miliseconds
**   Notes       : 
**   Author      : Pham Van Han
**   Date        : 2012/11/12
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Delay_ms (unsigned int u32_delay_period_ms)
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		U32 TIMCounter;
   TIMCounter = u32_delay_period_ms;

   TIM_TimeBaseStructure.TIM_Period = 999;
   TIM_TimeBaseStructure.TIM_Prescaler = 60;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;   
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down; 
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

   TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
   TIM_ARRPreloadConfig(TIM3, ENABLE);

	TIM_Cmd(TIM3, ENABLE);
	TIM_SetCounter(TIM3, 999);
	while (TIMCounter)
	{
		if (TIM_GetCounter(TIM3) == 0)
      {
         TIMCounter--;
      	TIM_SetCounter(TIM3, 999);
      }
	}
	TIM_Cmd(TIM3, DISABLE);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void vApplicationStackOverflowHook (xTaskHandle *pxTask,
**                                                     signed portCHAR *pcTaskName)
**   Arguments   :
**      (in) *pxTask      : handle of the offending task creates stack overflow.
**      (in) *pcTaskName  : name of the offending task creates stack overflow.
**   Return      : n/a
**   Description : Stack overflow hook function.
**   Notes       : Only need when configCHECK_FOR_STACK_OVERFLOW is not set to 0.
**                 Stack overflow checking introduces a context switch overhead
**                 so its use is only recommended during the development or
**                 testing phases. 
**   Author      : Nguyen Anh Huy.
**   Date        : 2011/12/26.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void vApplicationStackOverflowHook (xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
   (void)pxTask;
   (void)pcTaskName;

   for(;;);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
