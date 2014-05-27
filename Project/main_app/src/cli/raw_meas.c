/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : raw_meas.c
**   Project     : IMT - Main CPU - Main Application.
**   Author      : Nguyen Anh Huy
**   Revision    : 1.0.0.1
**   Date        : 2012/03/08.
**   Description : This module to output the raw measurement to UART port.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Standard includes. */
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Application includes. */
#include "stm32f4xx.h"
#include "data_types.h"
#include "canfestival_task.h"
#include "raw_meas.h"
#include "time.h"
#include "gps.h"
#include "fpga_data.h"
#include "retarget.h"
#include "cli_utils.h"
#include "iwdg.h"


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#define MEASOUT_TASK_DELAY        (portTickType)(1000 / portTICK_RATE_MS)

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/* Output debug header. */
const char str_measout_header[] = "Second, Minute, Hour, Day, Month, Year, Timezone, GPS Lock, GPS Number of Satellite, PD Noise Level A,B,C,N, Sampling Frequency, AC Period, AC Frequency, Top Oil Temperature, Bottom Oil Temperature, Ambient Temperature, Ambient Humidty, Estimated Bushing Temperature, Power System Voltage A,B,C, DFT Real Component PriA,B,C,SecA,B,C, DFT Image Component PriA,B,C,SecA,B,C, Angle PriA_B,A_C,SecA_B,A_C, DFT Frequency, Number of DFT Power Cycle, Amplitude PriA,B,C,SecA,B,C, Noise PriA,B,C,SecA,B,C, PD Number A,B,C,N, PD Highest Level A,B,C,N, PD Lowest Level A,B,C,N, PD Average Level A,B,C,N, PD Position A,B,C,N, PD Event, PD Position A,B,C,N, UHF Amplitude 1,2,3,4,5,6, PD Noise Sigma A,B,C,N, UHF Position 1,2,3,4,5,6, VHF Position,";

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : static void v_MEASOUT_Task (void *pvParameters)
**
**   Arguments      :
**      void *pvParameters -- not used     
**
**   Return         : n/a
**
**   Description    : output the measurement to UART port every 1 second.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/03/08.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Raw_Meas_Output (FILE *pf_measout)
{   
   /*
   ** Infinite loop, activated every 1 second
   ** to output the measurement to UART port.
   */

   /* GPS time struct. */
   GPS_TIME_T stru_gps_time;

   /* Print the header. */
   fprintf(pf_measout, str_measout_header);

   for(;;)
   {
       /* Time: second, minute, hour, day, month, year, timezone. */
       stru_gps_time = stru_GPS_Get_DateTime();
       fprintf(pf_measout,
               "%u,%u,%u,%u,%u,%u,%d,",
//               30, 40, 8, 23, 6, 2012, TIMEZONE);
               stru_gps_time.u8_sec, stru_gps_time.u8_min, stru_gps_time.u8_hour,
               stru_gps_time.u8_day, stru_gps_time.u8_month, stru_gps_time.u16_year, GPS_Time_Zone);
       
       /* GPS status: GPS lock, GPS number of satellite in view. */
       fprintf(pf_measout,
               "%u,%u,",
//               1, 2);
               u8_GPS_Get_Status(), u8_GPS_Get_Num_Sat_In_Use());
       
                /* PD Level noise. */
//       fprintf(pf_measout,
//               "%u,%u,%u,%u,",
//               3, 4, 5, 6);
       fprintf(pf_measout,
               "%u,%u,%u,%u,",
               (u32_FPGA_Get_Configuration_Data(PD_CONF_ITEM_NOISE_LV_CHA)),
               (u32_FPGA_Get_Configuration_Data(PD_CONF_ITEM_NOISE_LV_CHB)),
               (u32_FPGA_Get_Configuration_Data(PD_CONF_ITEM_NOISE_LV_CHC)),
               (u32_FPGA_Get_Configuration_Data(PD_CONF_ITEM_NOISE_LV_CHN)));

       /* Sampling Frequency. */
       fprintf(pf_measout,
               "%u,",
               u32_FPGA_Get_Configuration_Data(GNR_CONF_ITEM_SAMPLING_FREQ));

       /* AC period. */
       fprintf(pf_measout,
               "%u,",
//               0x80000000);
               s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_AC_PERIOD_ITEM));

       /* AC Frequency. */
//       fprintf(pf_measout,
//               "%0.2f,",
//               7.24f);
       fprintf(pf_measout,
               "%0.2f,",
               (flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_FREQ_ITEM)));

       /* Top Oil Temperature, Bottom Oil Temperature,
       ** Ambient Temperature and Humidty, Estimated Bushing Temperature. */
//       fprintf(pf_measout,
//               "%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,",
//               8.34, 9.45, 10.56, 11.67, 12.78, 13.89, 14.91, 15.12);
       fprintf(pf_measout,
               "%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,",
               flt_Get_Sensors_Data(SS_ITEM_TOP_OIL_TEMP),
               flt_Get_Sensors_Data(SS_ITEM_BOTTOM_OIL_TEMP),
               flt_Get_Sensors_Data(SS_ITEM_AMBIENT_TEMP),
               flt_Get_Sensors_Data(SS_ITEM_AMBIENT_HUMID),
               0.0);
       /* Power System Voltage A-B-C. */
       fprintf(pf_measout,
               "%0.2f,%0.2f,%0.2f,",
               flt_Get_Sensors_Data (SS_ITEM_VOLT_A),
               flt_Get_Sensors_Data (SS_ITEM_VOLT_B),
               flt_Get_Sensors_Data (SS_ITEM_VOLT_C));

       /* DFT real components. */  
       fprintf(pf_measout,
               "%d,%d,%d,%d,%d,%d,",
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_REAL_DFT_PRI_A)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_REAL_DFT_PRI_B)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_REAL_DFT_PRI_C)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_REAL_DFT_SEC_A)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_REAL_DFT_SEC_B)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_REAL_DFT_SEC_C)));

       /* DFT image components. */  
       fprintf(pf_measout,
               "%d,%d,%d,%d,%d,%d,",
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_IMAG_DFT_PRI_A)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_IMAG_DFT_PRI_B)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_IMAG_DFT_PRI_C)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_IMAG_DFT_SEC_A)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_IMAG_DFT_SEC_B)),
               (s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_VOLT_IMAG_DFT_SEC_C)));


       /* Primary A-B, A-C Angle. */
//       fprintf(pf_measout,
//               "%0.3f,%0.3f,",
//               160.123, 170.456);
       fprintf(pf_measout,
               "%0.3f,%0.3f,",
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_ANGLE_PRIM_A_B_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_ANGLE_PRIM_A_C_ITEM));

       /* Secondary A-B, A-C Angle. */
//       fprintf(pf_measout,
//               "%0.3f,%0.3f,",
//               -160.123, -170.456);
       fprintf(pf_measout,
               "%0.3f,%0.3f,",
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_ANGLE_SEC_A_B_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_ANGLE_SEC_A_C_ITEM));

       /* DFT Frequency. */
       fprintf(pf_measout,
               "%d,",
               u32_FPGA_Get_Configuration_Data(GNR_CONF_ITEM_DFT_FREQ));

       /* Number of DFT Power Cycle. */
       fprintf(pf_measout,
               "%d,",
               s32_FPGA_Get_Phase_Data_Raw_Data(MEAS_NB_DFT_ITEM));

       /* Primary A, B, C Amplitude */
//       fprintf(pf_measout,
//               "%0.2f,%0.2f,%0.2f,",
//               18.43, 19.54, 20.65);
       fprintf(pf_measout,
               "%0.2f,%0.2f,%0.2f,",
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_VOLT_PRIM_A_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_VOLT_PRIM_B_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_VOLT_PRIM_C_ITEM));

       /* Secondary A, B, C Amplitude */
//       fprintf(pf_measout,
//               "%0.2f,%0.2f,%0.2f,",
//               21.12, 22.23, 23.34);
       fprintf(pf_measout,
               "%0.2f,%0.2f,%0.2f,",
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_VOLT_SEC_A_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_VOLT_SEC_B_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_VOLT_SEC_C_ITEM));

       /* Primary A, B, C Noise. */
//       fprintf(pf_measout,
//               "%0.2f,%0.2f,%0.2f,",
//               24.12, 25.23, 26.34);
       fprintf(pf_measout,
               "%0.2f,%0.2f,%0.2f,",
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_NOISE_PRIM_A_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_NOISE_PRIM_B_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_NOISE_PRIM_C_ITEM));

       /* Secondary A, B, C Noise. */
//       fprintf(pf_measout,
//               "%0.2f,%0.2f,%0.2f,",
//               27.12, 28.23, 29.34);
       fprintf(pf_measout,
               "%0.2f,%0.2f,%0.2f,",
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_NOISE_SEC_A_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_NOISE_SEC_B_ITEM),
               flt_FPGA_Get_Phase_Data_Calc_Data(MEAS_NOISE_SEC_C_ITEM));

       /* PD Number A, B, C, N. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,",
               0, 0, 0, 0);

       /* PD highest level A, B, C, N. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,",
               0, 0, 0, 0);
       
       /* PD lowest level A, B, C, N. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,",
               0, 0, 0, 0);
       
       /* PD average A, B, C, N. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,",
               0, 0, 0, 0);
      
       /* PD Position A, B, C, N. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,",
               0, 0, 0, 0);
       
       /* PD Event. */
       fprintf(pf_measout,
               "%d,",
               0);
      
       /* PD Noise Sigma A, B, C, N. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,",
               0, 0, 0, 0);
       
       /* UHF Amplitude 1, 2, 3, 4, 5, 6. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,%d,%d,",
               0, 0, 0, 0, 0, 0);
       
       /* UHF Position 1, 2, 3, 4, 5, 6. */
       fprintf(pf_measout,
               "%d,%d,%d,%d,%d,%d,",
               0, 0, 0, 0, 0, 0);
       
       /* VHF Position. */
       fprintf(pf_measout,
               "%d,\r\n",
               0);

      /* Set task running flag is true. */
      v_Set_Task_Running_Flag(USART_MEASOUT_TASK_IDX, __TRUE);
      
      /* */
      if (s32_CLI_Check_Quit_Cmd(pf_measout, "qraw") != 0)
      {
         /* exit raw measurement output loop. */
         break;
      }

      vTaskDelay(MEASOUT_TASK_DELAY);
   }
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
