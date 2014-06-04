/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : adc_dma.c
**   Project     : IMT - MainCPU - Main App.
**   Author      : Nguyen Trong Tri
**   Version     : 1.0.1
**   Date        : 2013/03/14.
**   Description : This module to read PCB temperature.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include "stm32f4xx.h"
#include "adc_dma.h"

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* ADC3 data register address. */
#define ADC3_DR_ADDRESS              ((uint32_t)0x4001224C)// ADC3->DR

#define ADC1_DR_ADDRESS    				((uint32_t)0x4001204C)

/* Calculation average period. */
#define  CALC_AVER_PERIOD             1000

/* Get adc buffer length. */
#define  AVER_ADC_BUFFER_SIZE         CALC_AVER_PERIOD / INT_PERIOD_MS

/* maximum adc buffer size. */
#define  ADC_BUFFER_SIZE_MAX          100

/* interrupt period ms */
#define  INT_PERIOD_MS                50

/* 1s */                                   
#define  ONESEC                       1000

/* voltage reference. */
#define  Vref                         (float)3.3

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/ 

/* Buffer holding PCB tempratures. */ 
static volatile float  flt_temp_adc_buffer [MEAS_ADC_NUM][AVER_ADC_BUFFER_SIZE];

/* Buffer holding adc value read. */
static uint16_t u16_ADC3ConvertedValue[MEAS_ADC_NUM] = {0};

/* Temperature value. */
//static float flt_temp[MEAS_ADC_NUM] = {0};

__IO uint32_t ValADC = 0;

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Function to calculate average of buffer.*/
static float  flt_Cal_Aver_Buffer (volatile float *pflt_buffer, uint16_t u16_buf_len);

/* Function to initialize timer 3. */
static void initTimer3(void);

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_ADC3_DMA2_Config(void)
**
**   Arguments   : n/a
**
**   Return      : n/a
**
**   Description :  
**      - Enable peripheral clocks.                                              
**      - DMA2_Stream0 channel2 configuration.                                  
**      - Configure ADC Channel6, 7, 8 pin as analog input.                           
**      - Configure ADC3 Channel4, 5, 6.  
**
**   Notes       : restrictions, odd modes
**
**   Author      : Nguyen Trong Tri
**
**   Date        : 2013/03/14.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_ADC3_DMA2_Config(void)
{
   ADC_InitTypeDef       ADC_InitStructure;
   ADC_CommonInitTypeDef ADC_CommonInitStructure;
   DMA_InitTypeDef       DMA_InitStructure;
   GPIO_InitTypeDef      GPIO_InitStructure;

   /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

   /* DMA2 Stream0 channel2 configuration **************************************/
   DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
   DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&u16_ADC3ConvertedValue;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   DMA_InitStructure.DMA_BufferSize = MEAS_ADC_NUM;
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
   DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
   DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
   DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA2_Stream0, &DMA_InitStructure);
   DMA_Cmd(DMA2_Stream0, ENABLE);

   /* Configure ADC3 Channel 0,1 pin as analog input ******************************/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
   GPIO_Init(GPIOC, &GPIO_InitStructure);

   /* ADC Common Init **********************************************************/
   ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
   ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
   ADC_CommonInit(&ADC_CommonInitStructure);

   /* ADC3 Init ****************************************************************/
   ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
   ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfConversion = MEAS_ADC_NUM;
   ADC_Init(ADC1, &ADC_InitStructure);

   /* ADC3 regular channel10, 11 configuration *************************************/
   ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_112Cycles);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_112Cycles);
	  ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 3, ADC_SampleTime_112Cycles);	 


  /* Enable VBAT channel */
//  ADC_VBATCmd(ENABLE);
 
  /* Enable Temp channel */
  ADC_TempSensorVrefintCmd(ENABLE);

   /* Enable DMA request after last transfer (Single-ADC mode) */
   ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

   /* Enable ADC3 DMA */
   ADC_DMACmd(ADC1, ENABLE);

   /* Enable ADC3 */
   ADC_Cmd(ADC1, ENABLE);

   /* Start ADC3 Software Conversion */ 
   ADC_SoftwareStartConv(ADC1);

   /* Initialize timer3. */
   initTimer3();
}



/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void initTimer3(void)
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
void initTimer3(void)
{
   /* System clock */
   unsigned int gTimerClock;

   unsigned long u32_timer_reload;

   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   gTimerClock = SystemCoreClock / 2;
   u32_timer_reload =  (gTimerClock * INT_PERIOD_MS) / ONESEC;
   /* TIM3 clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

   /* Time base configuration */
   TIM_TimeBaseStructure.TIM_Period = u32_timer_reload / 1000 - 1;
   TIM_TimeBaseStructure.TIM_Prescaler = 1999;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

   /* Enable the TIM3 global Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   /* Enable Timer 2 interrupt. */
   TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
   TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

   /* Start counting */
   TIM_Cmd(TIM3, ENABLE);
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void TIM3_IRQHandler(void)
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
void TIM3_IRQHandler(void)
{
   /* Sensor index. */
   uint8_t u8_sensor_id;

   /* Average adc buffer index.  */
   static uint16_t  u16_aver_adc_buf_idx;

   if (TIM_GetITStatus(TIM3, TIM_IT_Update ) != RESET)
   {
      TIM_ClearITPendingBit(TIM3, TIM_IT_Update );    // Reset Flag

      if (u16_aver_adc_buf_idx < AVER_ADC_BUFFER_SIZE )
      {  
         for (u8_sensor_id = 0; u8_sensor_id < MEAS_ADC_NUM; u8_sensor_id++)
         {
            /* Get temperature adc value. */
      		flt_temp_adc_buffer[u8_sensor_id][u16_aver_adc_buf_idx] = (float)u16_ADC3ConvertedValue[u8_sensor_id];
         }
         /* Increase average adc buffer index. */
         u16_aver_adc_buf_idx ++;
      }
      else 
      {
         /* Reset aver adc buffer index. */
         u16_aver_adc_buf_idx = 0;
      }  		
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : float  flt_Cal_Aver_Buffer (float *pflt_buffer, uint16_t u16_buf_len)
**
**   Arguments   : 
**      pflt_buffer - pointer to buffer.
**      u16_buf_len -  length of buffer.
**
**   Return      :
**      Return average value of buffer. 
** 
**   Description : This function to calculate  the average of a buffer.
**
**   Notes       : restrictions, odd modes
**
**   Author      : Nguyen Trong Tri
**
**   Date        : 2012/02/15 
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/							 
static float  flt_Cal_Aver_Buffer (volatile float *pflt_buffer, uint16_t u16_buf_len)
{ 
   float flt_sum = 0;
   uint16_t  j;
    
   /*Calculate average ADC */
   for (j = 0; j < u16_buf_len; j++)
   {
      flt_sum  = flt_sum  +  *(pflt_buffer + j);
   } 
   return (float)(flt_sum / u16_buf_len) ;
}
 

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : float flt_ADC_Get_Temp (uint8_t u8_sensor_id)
**
**   Arguments   : u8_sensor_id - temperature id (Supply, ADC, FPGA).
**      
**   Return      :
**      Return temperature value.
** 
**   Description : This function to get temperature value.
**
**   Notes       : restrictions, odd modes
**
**   Author      : Nguyen Trong Tri
**
**   Date        : 2013/03/19 
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/	
float flt_ADC_Get_Temp (void)
{
	 float t_adcAver_f = 0;
	 float Voltage = 0;
	 float temp = 0;
	
	 t_adcAver_f = flt_Cal_Aver_Buffer (flt_temp_adc_buffer[MEAS_INT_TEMPSS_ID], AVER_ADC_BUFFER_SIZE);	
	 /* Convert ADC value to temperture. */  
	 Voltage = (t_adcAver_f *3300)/4095;
	 temp = ((Voltage - (float)760)/(float)2.5) - 15;
	 return temp;		   
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : float flt_Get_TempIntSS(void)
**
**   Arguments   : n/a
**      
**   Return      :
**      Return internal temperature sensor value.
** 
**   Description : This function to get maximum temperature value.
**
**   Notes       : restrictions, odd modes
**
**   Author      : Nguyen Trong Tri
**
**   Date        : 2013/04/12 
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/	

float flt_Get_TempIntSS(void)
{
		double Voltage = 0;

		double temp = 0;
 
    Voltage = (ValADC *3300)/4095;
    temp = ((Voltage - 760)/2.5)+25;
	
   return (float)temp;
}


float Get_Meas_Data_f(uint8_t u8_id)
{
	 float Voltage = 0;
   float	 t_adcAver_f = flt_Cal_Aver_Buffer (flt_temp_adc_buffer[u8_id], AVER_ADC_BUFFER_SIZE);	
	  /* Convert ADC value to voltage. */  
	 Voltage = (t_adcAver_f *3300)/4095;
	 return Voltage;
}	

/* 
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

