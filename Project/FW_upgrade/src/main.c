/**
  ******************************************************************************
  * @file    FW_upgrade/src/main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   IAP thru USB host main file
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "flash_if.h"
#include "parameters.h"

/** @addtogroup STM32F4-Discovery_FW_Upgrade
  * @{
  */

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
USB_OTG_CORE_HANDLE          USB_OTG_Core;
USBH_HOST                    USB_Host;

pFunction Jump_To_Application;
uint32_t JumpAddress;

/* Private function prototypes -----------------------------------------------*/
void initTimer5(void);



/* Private functions ---------------------------------------------------------*/

/**
  * @brief  main
  *         Main routine for IAP application
  * @param  None
  * @retval int
  */
int main(void)
{
  /* STM32 evalboard user initialization */
  BSP_Init();
	
  /* Flash unlock */
  FLASH_If_FlashUnlock();
	
	/* Initialize parameter block */
	v_PARAM_Init();	
	
	STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDOn(LED5);
	
	initTimer5();
	
	if(u32_PARAM_Get_Value(PARAM_ID_FW_UPDATE_FLAG) != 0)
	{
		/*Clear up fwupdate flag */
		v_PARAM_Set_Value(PARAM_ID_FW_UPDATE_FLAG, PARAM_FW_UPDATE_FLAG_NONE); 
		/* Init Host Library */
		USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);
    
		while (1)
		{
    /* Host Task handler */
    USBH_Process(&USB_OTG_Core, &USB_Host);
		}	
	}
	else
	{
    /* Check Vector Table: Test if user code is programmed starting from address 
       "APPLICATION_ADDRESS" */
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    {
			 TIM_DeInit(TIM5);
			//TIM_DeInit(TIM2);
			
      /* Jump to user application */
      JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
      Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
      Jump_To_Application();
    }
		else
		{
			/* Init Host Library */
			USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);
    
			while (1)
			{
				/* Host Task handler */
				USBH_Process(&USB_OTG_Core, &USB_Host);
			}				
		}
  }
}



/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void initTimer5(void)
**
**   Arguments   : n/a
**      
**   Return      : n/a 
** 
**   Description : timer 3 interrupt routine to calculate ADC average values.
**
**   Notes       : restrictions, odd modes
**
**   Author      : Nguyen Trong Tri
**
**   Date        : 2012/02/15 
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/	
void initTimer5(void)
{
   /* System clock */
   unsigned int gTimerClock;

   unsigned long u32_timer_reload;

   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   gTimerClock = SystemCoreClock / 2;
   u32_timer_reload =  gTimerClock;
   /* TIM5 clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

   /* Time base configuration */
   TIM_TimeBaseStructure.TIM_Period = u32_timer_reload / 1000 - 1;
   TIM_TimeBaseStructure.TIM_Prescaler = 1999;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
   TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

   /* Enable the TIM5 global Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   /* Enable Timer 2 interrupt. */
   TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
   TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

   /* Start counting */
   TIM_Cmd(TIM5, ENABLE);
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void TIM5_IRQHandler(void)
**
**   Arguments   : n/a
**      
**   Return      : n/a 
** 
**   Description : timer 3 interrupt routine to calculate ADC average values.
**
**   Notes       : restrictions, odd modes
**
**   Author      : Nguyen Trong Tri
**
**   Date        : 2012/02/15 
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/	
void TIM5_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM5, TIM_IT_Update ) != RESET)
   {
      TIM_ClearITPendingBit(TIM5, TIM_IT_Update );    // Reset Flag
			      /* Toggle Led pin. */
      STM_EVAL_LEDToggle(LED5);
   }
}




#ifdef USE_FULL_ASSERT
/**
  * @brief  assert_failed
  *         Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  File: pointer to the source file name
  * @param  Line: assert_param error line source number
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


/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
