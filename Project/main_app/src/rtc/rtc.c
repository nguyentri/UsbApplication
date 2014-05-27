/**
  ******************************************************************************
  * @file    RTC/RTC_Calendar/main.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    18-January-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"


/* RTC task delay (1S)*/
#define RTC_TASK_DELAY                      (portTickType)(1000 / portTICK_RATE_MS)

/* RTC task stack size. */
#define RTC_TASK_STACK_SIZE                 (configMINIMAL_STACK_SIZE*2)

/* RTC task priority. */
#define RTC_TASK_PRIORITY                   (tskIDLE_PRIORITY + 1)



/** @addtogroup RTC_Calendar
  * @{
  */ 

/* Private TpDef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Uncomment the corresponding line to select the RTC Clock source */
#define RTC_CLOCK_SOURCE_LSE         /* LSE used as RTC source clock */
/*#define RTC_CLOCK_SOURCE_LSI */   /* LSI used as RTC source clock. The RTC Clock
                                         may varies due to LSI frequency dispersion. */ 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
xSemaphoreHandle g_rtc_mutex;

RTC_TimeTypeDef  RTC_TimeStructure;
RTC_DateTypeDef	 RTC_DateStructure;
RTC_InitTypeDef  RTC_InitStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;

__IO uint32_t   uwLsiFreq = 0;
__IO uint32_t   uwCaptureNumber = 0; 
__IO uint32_t   uwPeriodValue = 0;
__IO uint32_t uwAsynchPrediv = 0;
__IO uint32_t uwSynchPrediv = 0;


/* Private function prototypes -----------------------------------------------*/
static void RTC_Config(void);
static void v_RTC_Task (void *pvParameters);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void rtc(void)
{
//   NVIC_InitTypeDef  NVIC_InitStructure;
//   EXTI_InitTypeDef  EXTI_InitStructure;
  
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files (startup_stm32f40xx.s/startup_stm32f427x.s) before to branch to 
       application main. 
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */ 
  RTC_Config() ; 
	
  /* Get the LSI frequency:  TIM5 is used to measure the LSI frequency */
  uwLsiFreq = 31448;

  /* Create the global mutex for sharing rtc structure. */
   g_rtc_mutex = xSemaphoreCreateMutex();	
	
  /* Turn on LED2 */
  //STM_EVAL_LEDOn(LED2);

  /* Calendar Configuration */
  /* ck_spre(1Hz) = RTCCLK(LSI) /(AsynchPrediv + 1)*(SynchPrediv + 1)*/
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv	= (uwLsiFreq/128) - 1;
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);	
	
  /* Set the date: Friday January 7th 2014 */
  RTC_DateStructure.RTC_Year = 0x0E;
  RTC_DateStructure.RTC_Month = RTC_Month_June;
  RTC_DateStructure.RTC_Date = 0x06;
  RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Friday;
  RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
	
	/* Set time */
  RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
  RTC_TimeStructure.RTC_Hours   = 0x09;
  RTC_TimeStructure.RTC_Minutes = 0x00;
  RTC_TimeStructure.RTC_Seconds = 0x00;  
  RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
 
 
	/*Create RTC task. */
  xTaskCreate(v_RTC_Task, (signed portCHAR *)"RTC Task", RTC_TASK_STACK_SIZE,
               NULL,RTC_TASK_PRIORITY, NULL);
}


/**
  * @brief  RTC task.
  * @param  None
  * @retval None
  */
static void v_RTC_Task (void *pvParameters)
{
	for(;;)
	{
			RTC_TimeShow();
//			RTC_AlarmShow();
			vTaskDelay(RTC_TASK_DELAY);
	}
}


/**
  * @brief  Configure the RTC peripheral by selecting the clock source.
  * @param  None
  * @retval None
  */
static void RTC_Config(void)
{
 // NVIC_InitTypeDef NVIC_InitStructure; 
 // EXTI_InitTypeDef EXTI_InitStructure;

  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);

  /* LSI used as RTC source clock */
  /* The RTC Clock may varies due to LSI frequency dispersion. */   
  /* Enable the LSI OSC */ 
  RCC_LSICmd(ENABLE);

  /* Wait till LSI is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
   
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Calendar Configuration with LSI supposed at 32KHz */
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv  = 0xFF; /* (32KHz / 128) - 1 = 0xFF*/
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);  
	
  /* Enable the RTC Wakeup Interrupt */
//   NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
//   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//   NVIC_Init(&NVIC_InitStructure);  

  /* Configure the RTC WakeUp Clock source: CK_SPRE (1Hz) */
  RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
  RTC_SetWakeUpCounter(0x0);

  /* Enable the RTC Wakeup Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
  
  /* Enable Wakeup Counter */
  RTC_WakeUpCmd(ENABLE);	
}


/**
  * @brief  Display the current time.
  * @param  None
  * @retval None
  */
void RTC_TimeShow(void)
{
 // uint8_t showtime[50] = {0};
  /* Get the current Time */
  RTC_GetTime(RTC_Format_BCD, &RTC_TimeStructure);
  RTC_GetDate(RTC_Format_BCD, &RTC_DateStructure);
  /* Display time Format : hh:mm:ss */
 // sprintf((char*)showtime,"%0.2d:%0.2d:%0.2d",RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
}

/**
  * @brief  Display the current Alarm.
  * @param  None
  * @retval None
  */
void RTC_AlarmShow(void)
{
  uint8_t showalarm[50] = {0};
  /* Get the current Alarm */
  RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
  sprintf((char*)showalarm,"%0.2d:%0.2d:%0.2d", RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours, RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes, RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
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
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


