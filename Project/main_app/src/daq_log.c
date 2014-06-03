/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : daq_log.c
**   Project     : USB DAQ 
**   Author      : Nguyen Trong Tri
**   Version     : 1.0
**   Date        : 2014/03/14
**   Description : This module to record all information of devices to log file.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* standard include */
#include <stdio.h>
#include <string.h>

/* Stm32f4 includes */
#include "stm32f4xx.h"

/* application include */
#include "ff.h"
#include "daq_log.h"
#include "adc_dma.h"
#include "sub_dev_rx.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/*
* Local defination
*/
#define LOG_TASK_DELAY	1000


/*  
* Local variable
*/
static const char l_logHeader_s[] = "Date; Time; Temperature Main CPU; Temperature  C Sub Device; Temperature F Sub Device ; Votage A; Voltage B; Voltage C\r\n";

/* File name of data aquition log file. */
static const char l_logFname_s[] ="mlog.csv";

/* File structure of log file. */
static FIL l_log_fil;

bool b_log_new_line_flag = TRUE;

extern xSemaphoreHandle g_rtc_mutex;
extern RTC_TimeTypeDef  RTC_TimeStructure;
extern RTC_DateTypeDef	RTC_DateStructure;
extern RTC_InitTypeDef  RTC_InitStructure;
extern RTC_AlarmTypeDef RTC_AlarmStructure;

/*
* Function property
*/
/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       :  void v_LOG_Task (void *pvParameters)
**   Arguments      : N/a
**   Return         : 
**       pvParameters -- not used.
**   Description    : log file task handles to put measurements to file.
**   Notes          : 
**   Author         : Nguyen Trong Tri
**   Date           : 2012/10/08.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_LOG_Task (void)
{   
	FRESULT res;
	/* Open/Create csv log file. */
	res = f_open(&l_log_fil, (const char *)l_logFname_s, FA_READ | FA_WRITE| FA_OPEN_ALWAYS);
	if (res == FR_OK)
	{
		/* Open file successfully. */
		if(f_size(&l_log_fil) == 0) /* new file.*/
		{
			/* Write header. */
			f_puts((const char *)l_logHeader_s, &l_log_fil);
		}
		/* Close csv log file. */
		res = f_close(&l_log_fil);
		if (res == FR_OK )
		{                
			/* Infinite loop to write measurements to log file. */
			for(;;)
			{ 
				 /* Wait for new line flag is set. */
				 if ( b_log_new_line_flag == TRUE)
				 {
					/* Clear  new line flag. */
					//b_log_new_line_flag = FALSE;
  
					/* Set initial measurement is true. */
					/* Put measurements to log file. */
					if (s32_Put_Meas_To_Log_File() != 0)
					{
					   /* Set error code is FATTFS or SD card error. */
					   //s32_Set_Error(FATFS_INTERNAL_FUNCTION_ERROR, 0x00);
					}   
				 }    
				  /* Check log limit time. */
				 if (f_size(&l_log_fil) >= 5120UL)
				 {
					 s32_Create_New_File();
				 }
					/* Set task running flag is true. */
						//v_Set_Task_Running_Flag(LOG_TASK_IDX, __TRUE);
					/* Task delay 1s.*/
					vTaskDelay(LOG_TASK_DELAY*9);				 
		   }
		}
	}
	else
	{
		/* Set error code is FATTFS or SD card error. */
		//s32_Set_Error(FATFS_INTERNAL_FUNCTION_ERROR, 0x00);
	}  
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int32_t s32_Put_Meas_To_Log_File(void)
**   Arguments      : N/a
**   Return         : 
**      0   -   put measurements to log file successfully.
**     -1   -   can not put measurements to file.
**   Description    : This function called in log task handle to write  measurements to file.
**   Notes          : 
**   Author         : Nguyen Trong Tri
**   Date           : 2012/10/08.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_Put_Meas_To_Log_File(void)
{
   FRESULT res;

	uint32_t u32_file_size;
	uint32_t u32_cur_pointer;                                 
	/* Put GPS time year to file. */
	//GPS_TIME_T t_rtc_time_st;
	char str_temp[30]="0";

	/* Open/Create log file. */
	res = f_open(&l_log_fil, (const char *)l_logFname_s, FA_READ | FA_WRITE| FA_OPEN_ALWAYS);
	/* Open file successfully. */
	if(f_size(&l_log_fil) == 0) /* new file.*/
	{
		/* Write header. */
		f_puts((const char *)l_logFname_s, &l_log_fil);
	}
	if (res == FR_OK)
	{
		u32_file_size = f_size(&l_log_fil);
		/* Expand file size (cluster pre-allocation) */
		f_lseek(&l_log_fil, u32_file_size);       
		u32_cur_pointer = f_tell(&l_log_fil);
		/* Check if the file has been expanded */
		if (u32_cur_pointer == u32_file_size)  
		{   
					/* Time: second, minute, hour, day, month, year, timezone. */
				// See if we can obtain the semaphore.  If the semaphore is not available
				// wait 1000 ticks to see if it becomes free.
				if( xSemaphoreTake(g_rtc_mutex, ( 1000 )) == pdTRUE )
				{
						// We were able to obtain the semaphore and can now access the
						// shared resource.			
						f_printf(&l_log_fil,
                  "%d/%d/%d;",
                RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date,   RTC_DateStructure.RTC_Year);
						f_printf(&l_log_fil,
                  "%d:%d:%d;",
						RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
					// We have finished accessing the shared resource.  Release the 
					// semaphore.
					xSemaphoreGive( g_rtc_mutex );
				}
				
			v_GetData();	

			/* Top Oil Temperature, Bottom Oil Temperature, Ambient Temperature and Humidty. */
			sprintf(str_temp,"%.3f;%.3f;%.3f;",
						flt_ADC_Get_Temp(), f_Get_TempIntSS(0), f_Get_TempIntSS(1));
			f_puts((const char *)str_temp, &l_log_fil);
				
				/* Voltage  */
			sprintf(str_temp,"%.3f;%.3f;%.3f\r\n",
                f_GetMeas(0), f_GetMeas(1), f_GetMeas(2));
			f_puts((const char *)str_temp, &l_log_fil);
			memset(str_temp, 0, 30);
							
				
			memset(str_temp, 0, 30);
			
			/* Closed csv log file. */
			res = f_close(&l_log_fil);
			if (res == FR_OK)
			{
				return (0);
			} 
			else     
			{
				/* Set error code is fatfs close file error. */
				return (-1);
			}
		}
		else
		{
			/* Set error code is fatfs move pointer file error. */
			return (-1);
		}
	}
	else
	{
		/* Set error code is open file error. */
		return (-1);
	}
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int32_t s32_Delete_Log_File(void)
**   Arguments      : N/a
**   Return         : 
**      0   -   delete log file successfully.
**     -1   -   can not delete this file.
**   Description    : Delete log file.
**   Notes          : 
**   Author         : Nguyen Trong Tri
**   Date           : 2012/10/08.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_Delete_Log_File(char* psDelLogFname)
{
   FRESULT res;
   res = f_unlink(psDelLogFname); 
   if (res == FR_OK)
   {
      return (0);
   }
   else 
   {
      /* Set error code is fatfs remove file error. */
      return (-1);  
   }
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int32_t s32_Create_New_File(void)
**   Arguments      : N/a
**   Return         : 
**      0   -   create new log file successfully.
**     -1   -   can not create log file.
**   Description    : Rename main log file name to log file name format and create new main log file name.
**   Notes          : 
**   Author         : Nguyen Trong Tri
**   Date           : 2012/10/08.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_Create_New_File(void)
{
   FRESULT res;
   char str_temp[3];

   /* Temporary file name. */
   char str_log_newly_filename[] ="logxxxx.csv";
 
	 static uint16_t u16_log_file_idx = 0;
   /* Get file name. */
   sprintf(str_temp,"%04d", u16_log_file_idx);
   strncpy(str_log_newly_filename + 4, str_temp, 4);
 
   /* Open/Create csv log file. */
   res = f_unlink((const XCHAR*)str_log_newly_filename);

   if ((res == FR_OK) || (res == FR_NO_FILE))
   {
      res = f_rename(l_logFname_s, str_log_newly_filename);
      if ((res == FR_OK) || (res == FR_NO_FILE))
      {
         /* main log file exists.*/
         if (res == FR_OK)
         {
            u16_log_file_idx++;
            /* Save log file index to FRAM. */
//            s32_PARM_Write(PARM_ID_LOG_FILE_IDX, (int32_t)u16_log_file_idx);
         }
   
         if (u16_log_file_idx == 9999)
         {
            u16_log_file_idx = 0;
         }

         /* Create new main csv log file. */
         res = f_open(&l_log_fil, (const char *)l_logFname_s, FA_READ | FA_WRITE| FA_CREATE_ALWAYS);
         if (res == FR_OK)
         {
            /* Write header to csv new file. */
            if (f_puts((const char *)l_logFname_s, &l_log_fil) != -1)
            {
               res = f_close(&l_log_fil);
               if (res == FR_OK)
               {
                  return (0);
               }
               else 
               {
                  /* Set error code is fatfs close file error. */
                  return (-1);
               }
            }
            else
            {
               /* Set error code is write file error. */
               return (-1);
            }
         }
         else
         { 
            /* Set error code is  fatfs open file error. */
            return (-1);
         }
   
      }
      else
      {
         /* Set error code is fatfs rename file error. */
         return (-1);
      }
   }
   else
   {
      /* Set error code is fatfs remove file error. */
      return (-1);
   }
}
