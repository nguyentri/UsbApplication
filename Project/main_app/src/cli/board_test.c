/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : board_test.c
**   Project     : IMT - board test
**   Author      : Nguyen Anh Huy
**   Revision    :
**   Date        : 2012/05/05
**   Description : This module contains functions to test product.
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
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Application includes. */
#include "stm32f4xx.h"
#include "data_types.h"
#include "version.h"
#include "measurement.h"
#include "retarget.h"
#include "ff.h"
#include "spi_flash.h"
#include "gps.h"
#include "ds28cm00.h"
#include "cli_utils.h"
#include "iwdg.h"
#include "fsmc.h"
#include "fpga_data.h"
#include "parm.h"
#include "fm24v10.h"
#include "canfestival_task.h"
#include "board_test.h"

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Macro to activate programming B pin. */
#define Prog_B_Pin_Activate()      GPIO_ResetBits(PROG_B_GPIO_PORT, PROG_B_PIN)

/* Macro to deactivate programming B pin. */
#define Prog_B_Pin_Deactivate()    GPIO_SetBits(PROG_B_GPIO_PORT, PROG_B_PIN)

/* Measurement Output Task delay. */
#define MEASOUT_TASK_DELAY             (portTickType)(1000 / portTICK_RATE_MS)

/* FPGA Validation Pin define */
#define FPGA_VALID_GPIO_PERIPH     RCC_AHB1Periph_GPIOD
#define FPGA_VALID_PORT            GPIOD
#define FPGA_VALID_PIN             GPIO_Pin_6

/* Define program B pin.  */
#define PROG_B_PIN                        GPIO_Pin_1
#define PROG_B_GPIO_PORT                  GPIOA
#define PROG_B_GPIO_CLK                   RCC_AHB1Periph_GPIOA


#define BOARD_TEST_QUIT_CMD_CHECK()                        \
{                                                          \
   if (s32_CLI_Check_Quit_Cmd(pf_measout, "qtest") != 0)   \
   {                                                       \
      /* exit raw measurement output loop. */              \
      break;                                               \
   }                                                       \
}

 

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


static void v_Test_Connection (FILE *pf_measout);

static int32_t s32_Test_Pair_Pins (GPIO_TypeDef* Port_1, uint16_t Pin_1, GPIO_TypeDef* Port_2, uint16_t Pin_2);

static int32_t s32_Test_SD (FILE *pf_measout);

//static void v_Test_FPGA_ProdTest_File (void);

static void v_Test_CANOpen (FILE *pf_measout);

static void v_Test_GPS (FILE *pf_measout);

static void v_Test_FRAM (FILE *pf_measout);

static void v_Test_FPGA_Flash (FILE *pf_measout);

/* Function inits Programming B pin. */
static void v_Prog_B_Pin_Init(void);

static void v_Test_PWM (FILE *pf_measout);


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* File to  test sdcard. */
static FIL file_test;
/* String read from sd card. */
//static char str_test_read[256];
/* SDcard test file name. */
static const char stri_test[] = "Test SDCard.";

/* File system object. */
extern FATFS stru_fatfs; 


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Board_Test (FILE *pf_measout)
**
**   Arguments      :
**      pf_measout - pointer to UART redirect port handler to print test result.
**
**   Return         : n/a 
**
**   Description    : Perform board test process and output test data.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Board_Test (FILE *pf_measout)
{
   uint32_t u32_idx;

   GPIO_InitTypeDef GPIO_InitStructure;
   
   /* Set invalid fpga firmware. */
   v_Set_FPGA_Fw_Flag(FPGA_FIRMWARE_INVALID);

   v_FSMC_DeInit();
   v_FPGA_EXTI_Disable(EXTI_Line1, EXTI1_IRQn);


   /* Enable Control Pin: GPS-PPS PI6. */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

   /* Configure pin as output. */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOI, &GPIO_InitStructure);

   /* Enable FSMC pins. */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOE |
                          RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOA,
                          ENABLE);

   /* Temporary disable watchdog to avoid MCU reset if there is any board issue. */
   v_IWDG_Soft_Disable();

   /* Do the test once. */
   for (u32_idx = 0; u32_idx < 1; u32_idx++)
   {
      fprintf(pf_measout, "\n\rIMT - BOARD TEST\n\r");
   
      /* Print Board Test Firmware Version. */
      fprintf(pf_measout, "\n\rBoard Test Firmware Version: %d.%d.%d.%d\n\r",
              MAINCPU_MAINAPP_MAJOR_VERSION, MAINCPU_MAINAPP_MINOR_VERSION,
              MAINCPU_MAINAPP_BUILD_VERSION, MAINCPU_MAINAPP_SVN_REVISION);
   
       /* Print Serial number provider. */
      fprintf(pf_measout, "\n\rSERIAL NUMBER PROVIDER\n\r");
      fprintf(pf_measout, "Family code: 0x%x\n\r",
              u8_DS28CM00_Read(DS28CM00_ADDR_FAMILY_CODE));
      fprintf(pf_measout, "Serial number: 0x%02x%02x%02x%02x%02x%02x\n\r",
              u8_DS28CM00_Read(DS28CM00_ADDR_SERIAL_NUMBER_5), u8_DS28CM00_Read(DS28CM00_ADDR_SERIAL_NUMBER_4),u8_DS28CM00_Read(DS28CM00_ADDR_SERIAL_NUMBER_3),
              u8_DS28CM00_Read(DS28CM00_ADDR_SERIAL_NUMBER_2), u8_DS28CM00_Read(DS28CM00_ADDR_SERIAL_NUMBER_1),u8_DS28CM00_Read(DS28CM00_ADDR_SERIAL_NUMBER_0));
      fprintf(pf_measout, "CRC of Family Code and 48-bit Serial Number: 0x%x\n\r",
              u8_DS28CM00_Read(DS28CM00_ADDR_CRC));
   
      fprintf(pf_measout, "\n\rStarting Tests...\n\r");
   
      /* FPGA Flash Test. */
      v_Test_FPGA_Flash(pf_measout);
      /* Wait for FPGA firmware activate*/
      for (u32_idx = 0; u32_idx < 10; u32_idx++)
      {
         /* Delay task. */
         vTaskDelay(MEASOUT_TASK_DELAY);
      }
      
      BOARD_TEST_QUIT_CMD_CHECK();
      
      /* Test SDCARD and load FPGA progam. */
      if (s32_Test_SD(pf_measout) !=  (-1))
      {
//      /* Initialize the SPI FLASH driver. */
//      sFLASH_Init();
//
//      /* Activate program_B pin. */
//      Prog_B_Pin_Activate();
//       /* Load FPGA Production code to flash. */
//      fprintf(pf_measout, "Programming FPGA Production code... ");
//
//      if (s32_LOADER_Process() == LOADER_NO_ERR)
//      {
//         sFLASH_DeInit();
//
//         fprintf(pf_measout, "OK\n\r");
//      }
//      else
//      {
//         fprintf(pf_measout, "Failed\n\r");
//      }
//
//      /* Deactivate program_B pin. */
//      Prog_B_Pin_Deactivate();
//
//      fprintf(pf_measout, "Waiting for FPGA to bootup...\n\r");
//      /* Delay STM32 for 15s so that FPGA has time to load its app from flash. */
//      vTaskDelay(15 * MEASOUT_TASK_DELAY);
      }
//       vTaskDelay(MEASOUT_TASK_DELAY);
      /* Mount the disk. */
      f_mount(0, &stru_fatfs);

      BOARD_TEST_QUIT_CMD_CHECK();
      
      /* Test STM32-FPGA Connection. */
      v_Test_Connection(pf_measout);
      BOARD_TEST_QUIT_CMD_CHECK();

      /* Test FRAM. */
      v_Test_FRAM(pf_measout);
      BOARD_TEST_QUIT_CMD_CHECK();

      /* Test CAN. */
      v_Test_CANOpen(pf_measout);
      BOARD_TEST_QUIT_CMD_CHECK();

      /* Test GPS.*/
      v_Test_GPS(pf_measout);
      
      /* Test PWM. */
      v_Test_PWM(pf_measout);
   }

   /* Finish board test, re-initialize FSMC and interrupt from FPGA. */
   /* Intialize FSMC. */
   v_FSMC_Init();

   /* Initialize FPGA external interrupt pin. */
   v_FPGA_EXTI_Pin_Init();

   /* Enable FPGA external interrupt. */
   v_FPGA_EXTI_Enable(EXTI_Line1, EXTI1_IRQn, EXTI_Trigger_Rising);
   
   /* Re-enable watchdog. */
   v_IWDG_Soft_Enable();
 }

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Test_Connection (void)
**
**   Arguments      : n/a
**
**   Return         : n/a 
**
**   Description    : This function tests connection between STM32 and FPGA.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Test_Connection (FILE *pf_measout)
{
   int32_t s32_result;
   uint8_t u8_fpga_valid;
   GPIO_InitTypeDef GPIO_InitStructure;

   /* STM32 - FPGA Connection Test. */
   fprintf(pf_measout, "\n\rSTM32-FPGA CONNECTION TEST\n\r");

   GPIO_DeInit(FPGA_VALID_PORT);
   
   /* Validation for application compliance  */           
   GPIO_InitStructure.GPIO_Pin = FPGA_VALID_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(FPGA_VALID_PORT, &GPIO_InitStructure);

   vTaskDelay(MEASOUT_TASK_DELAY);

   u8_fpga_valid = GPIO_ReadInputDataBit(FPGA_VALID_PORT, FPGA_VALID_PIN);
   if (u8_fpga_valid == 1)
   {
      fprintf(pf_measout, "FPGA Firmware is not valid (check pin is 1). Toggling Test is bypassed.\n\r");
      return;
   }

//    /* Test SPI2_CS - SPI2_MISO. */
//    fprintf(pf_measout, "Testing SPI2_CS - MISO: ");
//    s32_result = s32_Test_Pair_Pins(GPIOI, GPIO_Pin_0, GPIOB, GPIO_Pin_14);
//    if (s32_result == 0)
//    {
//       fprintf(pf_measout, "Pass.\n\r");
//    }
//    else
//    {
//       fprintf(pf_measout, "Fail.\n\r");
//    }

//    /* Test SPI2_MOSI - SPI2_CLK. */
//    fprintf(pf_measout, "Testing SPI2_MOSI - CLK: ");
//    s32_result = s32_Test_Pair_Pins(GPIOB, GPIO_Pin_15, GPIOI, GPIO_Pin_1);
//    if (s32_result == 0)
//    {
//       fprintf(pf_measout, "Pass.\n\r");
//    }
//    else
//    {
//       fprintf(pf_measout, "Fail.\n\r");
//    }

   /* Test D0 - D1. */
   fprintf(pf_measout, "Testing D0 - D1: ");
   s32_result = s32_Test_Pair_Pins(GPIOD, GPIO_Pin_14, GPIOD, GPIO_Pin_15);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test D2 - D3. */
   fprintf(pf_measout, "Testing D2 - D3: ");
   s32_result = s32_Test_Pair_Pins(GPIOD, GPIO_Pin_0, GPIOD, GPIO_Pin_1);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test D4 - D5. */
   fprintf(pf_measout, "Testing D4 - D5: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_7, GPIOE, GPIO_Pin_8);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test D6 - D7. */
   fprintf(pf_measout, "Testing D6 - D7: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_9, GPIOE, GPIO_Pin_10);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test D8 - D9. */
   fprintf(pf_measout, "Testing D8 - D9: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_11, GPIOE, GPIO_Pin_12);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test D10 - D11. */
   fprintf(pf_measout, "Testing D10 - D11: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_13, GPIOE, GPIO_Pin_14);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test D12 - D13. */
   fprintf(pf_measout, "Testing D12 - D13: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_15, GPIOD, GPIO_Pin_8);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test D14 - D15. */
   fprintf(pf_measout, "Testing D14 - D15: ");
   s32_result = s32_Test_Pair_Pins(GPIOD, GPIO_Pin_9, GPIOD, GPIO_Pin_10);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A1 - A2. */
   fprintf(pf_measout, "Testing A1 - A2: ");
   s32_result = s32_Test_Pair_Pins(GPIOF, GPIO_Pin_1, GPIOF, GPIO_Pin_2);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A3 - A4. */
   fprintf(pf_measout, "Testing A3 - A4: ");
   s32_result = s32_Test_Pair_Pins(GPIOF, GPIO_Pin_3, GPIOF, GPIO_Pin_4);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A5 - A6. */
   fprintf(pf_measout, "Testing A5 - A6: ");
   s32_result = s32_Test_Pair_Pins(GPIOF, GPIO_Pin_5, GPIOF, GPIO_Pin_12);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A7 - A8. */
   fprintf(pf_measout, "Testing A7 - A8: ");
   s32_result = s32_Test_Pair_Pins(GPIOF, GPIO_Pin_13, GPIOF, GPIO_Pin_14);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A9 - A10. */
   fprintf(pf_measout, "Testing A9 - A10: ");
   s32_result = s32_Test_Pair_Pins(GPIOF, GPIO_Pin_15, GPIOG, GPIO_Pin_0);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A11 - A12. */
   fprintf(pf_measout, "Testing A11 - A12: ");
   s32_result = s32_Test_Pair_Pins(GPIOG, GPIO_Pin_1, GPIOG, GPIO_Pin_2);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A13 - A14. */
   fprintf(pf_measout, "Testing A13 - A14: ");
   s32_result = s32_Test_Pair_Pins(GPIOG, GPIO_Pin_3, GPIOG, GPIO_Pin_4);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A15 - A16. */
   fprintf(pf_measout, "Testing A15 - A16: ");
   s32_result = s32_Test_Pair_Pins(GPIOG, GPIO_Pin_5, GPIOD, GPIO_Pin_11);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A17 - A18. */
   fprintf(pf_measout, "Testing A17 - A18: ");
   s32_result = s32_Test_Pair_Pins(GPIOD, GPIO_Pin_12, GPIOD, GPIO_Pin_13);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A19 - A20. */
   fprintf(pf_measout, "Testing A19 - A20: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_3, GPIOE, GPIO_Pin_4);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A21 - A22. */
   fprintf(pf_measout, "Testing A21 - A22: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_5, GPIOE, GPIO_Pin_6);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A23 - A24. */
   fprintf(pf_measout, "Testing A23 - A24: ");
   s32_result = s32_Test_Pair_Pins(GPIOE, GPIO_Pin_2, GPIOG, GPIO_Pin_13);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test A25 - CLK. */
   fprintf(pf_measout, "Testing A25 - CLK: ");
   s32_result = s32_Test_Pair_Pins(GPIOG, GPIO_Pin_14, GPIOD, GPIO_Pin_3);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test NE - NL. */
   fprintf(pf_measout, "Testing NE - NL: ");
   s32_result = s32_Test_Pair_Pins(GPIOG, GPIO_Pin_12, GPIOB, GPIO_Pin_7);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test OE - WE. */
   fprintf(pf_measout, "Testing OE - WE: ");
   s32_result = s32_Test_Pair_Pins(GPIOD, GPIO_Pin_4, GPIOD, GPIO_Pin_5);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* Test OE - WAIT. */
   fprintf(pf_measout, "Testing OE - WAIT: ");
   s32_result = s32_Test_Pair_Pins(GPIOD, GPIO_Pin_4, GPIOD, GPIO_Pin_6);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

//    /* Test GPS_RX - TX. */
//    fprintf(pf_measout, "Testing GPS_RX - TX: ");
//    s32_result = s32_Test_Pair_Pins(GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3);
//    if (s32_result == 0)
//    {
//       fprintf(pf_measout, "Pass.\n\r");
//    }
//    else
//    {
//       fprintf(pf_measout, "Fail.\n\r");
//    }

   /* Test Spare 1 - Spare 2. */
   fprintf(pf_measout, "Testing Spare 1 - 2: ");
   s32_result = s32_Test_Pair_Pins(GPIOB, GPIO_Pin_1, GPIOB, GPIO_Pin_0);
   if (s32_result == 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int32_t s32_Test_Pair_Pins (GPIO_TypeDef* Port_1, uint16_t Pin_1, GPIO_TypeDef* Port_2, uint16_t Pin_2)
**
**   Arguments      : 
**     (in) Port_1 - port test 1
**     (in) Pin_1  - pin test 1
**     (in) Port_2 - port test 2
**     (in) Pin_2  - pin test 2
**
**   Return         : 
**       (0) - test pair pins is pass.
**      (-1) - test pair pins is failure.
**   Description    : This function tests connection between pair pins.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_Test_Pair_Pins (GPIO_TypeDef* Port_1, uint16_t Pin_1, GPIO_TypeDef* Port_2, uint16_t Pin_2)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   uint8_t u8_state;
   uint32_t u32_idx;

   /* Configure pin 1 as output. */
   GPIO_InitStructure.GPIO_Pin = Pin_1;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(Port_1, &GPIO_InitStructure);
   /* Configure pin 2 as input. */
   GPIO_InitStructure.GPIO_Pin = Pin_2;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(Port_2, &GPIO_InitStructure);

   /* Clear control bit. */
   GPIO_ResetBits(GPIOI, GPIO_Pin_6);

   /* Set pin 1 to 1. */
   GPIO_SetBits(Port_1, Pin_1);
   for (u32_idx = 0; u32_idx < 10000; u32_idx++)
   {
   }
   /* Read pin 2. */
   u8_state = GPIO_ReadInputDataBit(Port_2, Pin_2);
   if (u8_state != 1)
   {
       return (-1);
   }

   /* Set pin 1 to 0. */
   GPIO_ResetBits(Port_1, Pin_1);
   for (u32_idx = 0; u32_idx < 10000; u32_idx++)
   {
   }
   /* Read pin 2. */
   u8_state = GPIO_ReadInputDataBit(Port_2, Pin_2);
   if (u8_state != 0)
   {
       return (-1);
   }

   /* Configure pin 2 as output. */
   GPIO_InitStructure.GPIO_Pin = Pin_2;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(Port_2, &GPIO_InitStructure);
   /* Configure pin 1 as input. */
   GPIO_InitStructure.GPIO_Pin = Pin_1;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(Port_1, &GPIO_InitStructure);

   /* Set control bit. */
   GPIO_SetBits(GPIOI, GPIO_Pin_6);

   /* Set pin 2 to 1. */
   GPIO_SetBits(Port_2, Pin_2);
   for (u32_idx = 0; u32_idx < 10000; u32_idx++)
   {
   }
   /* Read pin 1. */
   u8_state = GPIO_ReadInputDataBit(Port_1, Pin_1);
   if (u8_state != 1)
   {
       return (-1);
   }

   /* Set pin 2 to 0. */
   GPIO_ResetBits(Port_2, Pin_2);
   for (u32_idx = 0; u32_idx < 10000; u32_idx++)
   {
   }
   /* Read pin 1. */
   u8_state = GPIO_ReadInputDataBit(Port_1, Pin_1);
   if (u8_state != 0)
   {
       return (-1);
   }

   return (0);
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int32_t s32_Test_SD (void)
**
**   Arguments      : n/a
**
**   Return         : 
**     (0)  - test SDcard is pass.
**     (-1) - test SDcard is failure.
**
**   Description    : This function tests SDcard connection.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_Test_SD (FILE *pf_measout)
{
   FRESULT res;
   uint32_t u32_byte_written;

   /* SDCard Test. */
   fprintf(pf_measout, "\n\rSDCARD TEST\n\r");

   /* Try to open a file. */
   fprintf(pf_measout, "Open/Create file on SDCard: ");

   res = f_open(&file_test, "0:stm32sd.tst", FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
   if (res == FR_OK)
   {
      fprintf(pf_measout, "Pass.\n\r");
      fprintf(pf_measout, "Write data to file: ");
      /* Try to write some data to it. */
      res = f_write(&file_test, stri_test, sizeof(stri_test), &u32_byte_written);
      if (res == FR_OK)
      {
         fprintf(pf_measout, "Pass.\n\r");
//         v_Test_FPGA_ProdTest_File ();
         f_close(&file_test);
         return (0);
      }
      else
      {
         fprintf(pf_measout, "Fail.\n\r");
         f_close(&file_test);
         return (-1);
      }  
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
      return (-1);
   }
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Test_FPGA_ProdTest_File (void)
**
**   Arguments      : n/a
**
**   Return         : n/a
**
**   Description    : This function tests FPGA's ProdTest file on SDCard
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
//void v_Test_FPGA_ProdTest_File (void)
//{
//   FRESULT res;
//   uint32_t u32_byte_read;
//   uint16_t u16_idx;
//
////   /* SDCard Test. */                                                     
////   fprintf(pf_measout, "\n\rSDCARD TEST\n\r");
//
//   /* Try to open a file. */
//   fprintf(pf_measout, "Open FPGA's ProdTest file on SDCard: ");   
//   res = f_open(&file_test, "0:fpga_prd.mcs", FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
//   if (res == FR_OK)
//   {
//      fprintf(pf_measout, "Pass.\n\r");
//      fprintf(pf_measout, "Trying to read some data from file: ");
//      /* Try to read some data to it. */
//      res = f_read(&file_test, str_test_read, sizeof(str_test_read), &u32_byte_read);
//      if (res == FR_OK)
//      {
//         for (u16_idx = 0; u16_idx < 256; u16_idx++)
//         {
//            if (str_test_read[u16_idx] == NULL)
//            {
//               fprintf(pf_measout, "Fail.\n\r");
//               break;
//            }
//         }
//         fprintf(pf_measout, "Pass.\n\r");
//      }
//      else
//      {
//         fprintf(pf_measout, "Fail.\n\r");
//      }
//      f_close(&file_test);
//   }
//   else
//   {
//      fprintf(pf_measout, "Fail.\n\r");
//   }
//
//}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Test_CANOpen (void)
**
**   Arguments      : n/a
**
**   Return         : n/a
**
**   Description    : This function tests CAN1, CAN2 connection.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Test_CANOpen (FILE *pf_measout)
{
   uint32_t u32_port1_tx_port2_rx;
   uint32_t u32_port1_rx_port2_tx;
   uint32_t u32_idx;
   
   /* CANOpen Test. */
   fprintf(pf_measout, "\n\rCAN PORTS TEST\n\r");

   /* Clear can port statuses so that the test will receive the most updated statuses. */
   v_CANFES_MainCPU_HBeat_Status_Clear();

   /* If we get error status, we continuously check for 5s. */
   for (u32_idx = 0; u32_idx < 5; u32_idx++)
   {
      /* Get port statuses. */
      v_CANFES_MainCPU_HBeat_Status_Get(&u32_port1_tx_port2_rx,
                                        &u32_port1_rx_port2_tx);
      if ((u32_port1_tx_port2_rx == 0) || (u32_port1_rx_port2_tx == 0))
      {
         vTaskDelay(MEASOUT_TASK_DELAY);
      }
      else
      {
         break;
      }
   }

   /* CAN1 send data. */
   fprintf(pf_measout, "CAN1 Sending, CAN2 Receiving: ");
   if (u32_port1_tx_port2_rx != 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }

   /* CAN2 send data. */
   fprintf(pf_measout, "CAN2 Sending, CAN1 Receiving: ");
   if (u32_port1_rx_port2_tx != 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Test_FRAM (void)
**
**   Arguments      : n/a
**
**   Return         : n/a
**
**   Description    : This function tests FRAM.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Test_FRAM (FILE *pf_measout)
{
   uint8_t u8_cur_data = 0;
   uint8_t u8_test_data = 0;
   uint32_t u32_test_pass = 0;
   
#define FRAM_TEST_ADDR       0x3FFF
#define FRAM_TEST_VALUE      0xA5
   
   /* FRAM Test. */
   fprintf(pf_measout, "\n\rFRAM TEST\n\r");
   fprintf(pf_measout, "Write and Read single byte: ");
   
   /* Read current value at TEST ADDRESS. */
   s32_FM24V10_Read_Byte(FRAM_TEST_ADDR, &u8_cur_data);
   
   /* Write TEST VALUE to TEST ADDRESS of FRAM. */
   if (s32_FM24V10_Write_Byte(FRAM_TEST_ADDR, FRAM_TEST_VALUE) == 0)
   {
      /* Read back value at TEST ADDRESS and check if it is correct. */
      if (s32_FM24V10_Read_Byte(FRAM_TEST_ADDR, &u8_test_data) == 0)
      {
         /* Read data from FRAM successfully here.
         ** Compare the read back value and TEST VALUE.
         */
         if (u8_test_data == FRAM_TEST_VALUE)
         {
            /* Read back value is correct. Test passes. */
            u32_test_pass = 1;
         }
         else
         {
            /* Read back value is not correct. Test fails. */
            u32_test_pass = 0;
         }
      }
      else
      {
         /* Can't read data from FRAM. Test fails. */
         u32_test_pass = 0;
      }
   }
   else
   {
      /* Error in write data to FRAM. Test fails. */
      u32_test_pass = 0;
   }

   if (u32_test_pass == 1)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Test_GPS (void)
**
**   Arguments      : n/a
**
**   Return         : n/a 
**
**   Description    : Test GPS configuration and GPS message.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/10/29.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Test_GPS (FILE *pf_measout)
{
   U32 u32_idx;

   U8 u8_gps_tx_test_flag = 0;

   U32 u32_gps_configuration;

   fprintf(pf_measout, "\n\rGPS TEST\n\r");
   
   v_GPS_Clear_Test_Status();

   fprintf(pf_measout, "Test GPS Configuration: ");

   u32_gps_configuration = (U32)s32_PARM_Read(PARM_ID_GPS_SETUP_OUTPUT_MSG);

   /* Query GPS to ensure it is working in the correct configuration. */
   v_GPS_Setup_Command_Output_Msg(u32_gps_configuration, GPS_CMD_BOARD_TEST_QUERY);

   for (u32_idx = 0; u32_idx <= 5; u32_idx++)
   {
      /* Get configuration. */
      if (u32_gps_configuration == u32_Get_GPS_Setup_Field_Configuration(GPS_OUTPUT_MSG_ID))
      {  
         u8_gps_tx_test_flag = 1;
         break;   
      }
      else
      {
         vTaskDelay(MEASOUT_TASK_DELAY);
      }
   }

   if (u8_gps_tx_test_flag != 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");      
   }

   fprintf(pf_measout, "Test GPS Messages: ");

   for (u32_idx = 0; u32_idx <= 5; u32_idx++)
   {
      if (u32_GPS_Get_Test_Rx_Status() != 0)
      {
         break;   
      }
      else
      {
         vTaskDelay(MEASOUT_TASK_DELAY);
      }
   }

   if (u32_GPS_Get_Test_Rx_Status() != 0)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");      
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Test_FPGA_Flash (void)
**
**   Arguments      : n/a
**
**   Return         : n/a 
**
**   Description    : Test FPAG flash. Write a known value to FPGA flash address 0xFFFFFF 
**                    and then read back to verify if the data is written correctly.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Trong Tri
**
**   Date           : 2013/02/18.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Test_FPGA_Flash (FILE *pf_measout)
{
   uint8_t u8_data = 0x12;
   uint8_t u8_data_read = 0;

   fprintf(pf_measout, "\n\rFPGA FLASH TEST\n\r");

   /* Initialize the SPI FLASH driver. */
   sFLASH_Init();

   /* Initialize program_B pin. */
   v_Prog_B_Pin_Init();

   /* Activate program_B pin. */
   Prog_B_Pin_Activate();

   /* Trying to write single byte at FPGA flash address 0xFFFFFF. */
   fprintf(pf_measout, "Write and Read single byte at address 0xFFFFFF: ");
   sFLASH_WriteBuffer(&u8_data, (uint32_t)0xFFFFFF, (uint16_t)0x1);
   sFLASH_ReadBuffer(&u8_data_read, (uint32_t)0xFFFFFF, (uint16_t)0x1);
   /* Test result read. */
   if (u8_data_read == 0x12)
   {
      fprintf(pf_measout, "Pass.\n\r");
   }
   else
   {
      fprintf(pf_measout, "Fail.\n\r");
   }
   
   /* Deactivate program_B pin. */
   Prog_B_Pin_Deactivate();

   sFLASH_DeInit();
}



/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Program_B_Pin_Init(void)
**
**   Arguments      : n/a.
** 
**   Return         : n/a.
**
**   Description    : This function to initialize program_B pin.
**
**   Notes          : 
**
**   Author         : Nguyen Trong Tri.
**
**   Date           : 2012/07/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

void v_Prog_B_Pin_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the PROG_B Clock */
  RCC_AHB1PeriphClockCmd(PROG_B_GPIO_CLK, ENABLE);

  /* Configure the PROG_B pin */
  GPIO_InitStructure.GPIO_Pin = PROG_B_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(PROG_B_GPIO_PORT, &GPIO_InitStructure);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void v_Test_PWM (void)
**
**   Arguments      : n/a
**
**   Return         : n/a
**
**   Description    : This function tests PWM.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/05/05.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_Test_PWM (FILE *pf_measout)
{
   /* PWM Test. */
   fprintf(pf_measout, "\n\rPWM TEST\n\r");
   fprintf(pf_measout, "PWM 10KHz for 10s:...\n\r");
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
