/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : fwugrade.c
**   Project     : IMT - main CPU - main application.
**   Author      : Nguyen Trong Tri
**   Revision    : 1.0.0.1
**   Date        : 2012/07/18.
**   Description : This module contains functions needed to trigger the  firmware
**                 update and call loader processing.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Stm32 includes. */
#include "stm32f4xx.h"

/* Application includes. */
#include "parameters.h"
#include "data_types.h"


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#define FPGA_FW_DETECT_DELAY_MS           20000


#define PARM_FW_UPDATE_FLAG_ACTIVE			  1


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void EXTILine0_Config(void);

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/* Firmware update active Flag */
static uint8_t u8_fw_maincpu_update_activate = 0;


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_FW_Main_CPU_Update_Activate (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Activate the firmware update mechanism.
**   Notes       : The application calls this function to activate the firmware
**                 update mechanism. 
**   Author      : Nguyen Anh Huy.
**   Date        : 2012/03/28.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_FW_Main_CPU_Update_Activate (void)
{
   u8_fw_maincpu_update_activate = __TRUE;
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_FW_Main_CPU_Update_Poll (uint16_t u16_time_passed_ms)
**   Arguments   :
**      (in) u16_time_passed_ms   -  Number of milisecond from the last call.
**   Return      : n/a
**   Description : The application need to call this function periodically to
**                 process delay timer for firmware update. If the timer reached 
**                 a pre-defined interval value, STM32 reset and jump to bootloader.
**   Notes       : 
**   Author      : Nguyen Anh Huy.
**   Date        : 2012/03/28.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_FW_Main_CPU_Update_Poll (void)
{
   /* Firmware programming finalization result. */
   /* If the firmware update mechanism is activated. */
   if (u8_fw_maincpu_update_activate == __TRUE)
   {	
	//	v_TaskDelay(10);
		/*Set par */
 		v_PARAM_Set_Value(PARAM_ID_FW_UPDATE_FLAG, PARM_FW_UPDATE_FLAG_ACTIVE);  
	//	v_TaskDelay(10);
       /*
        ** reset system.
        */
        NVIC_SystemReset();
   }
}


void v_FW_Main_CPU_Update_Init(void)
{
	EXTILine0_Config();
}

/**
  * @brief  Configures EXTI Line0 (connected to PA0 pin) in interrupt mode
  * @param  None
  * @retval None
  */
void EXTILine0_Config(void)
{
  
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef 	 EXTI_InitStructure;
  /* Enable GPIOA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Configure PA0 pin as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Connect EXTI Line0 to PA0 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

  /* Configure EXTI Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
