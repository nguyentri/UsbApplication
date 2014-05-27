/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : assem_test_info.c
**   Project     : IMT - Main CPU - Main Application.
**   Author      : Nguyen Anh Huy
**   Revision    : 1.0.0.1
**   Date        : 2013/05/20.
**   Description : This module to set or get test information from parameter module (non-volatine memory).
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
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Application includes. */
#include "stm32f4xx.h"
#include "data_types.h"
#include "parm.h"
#include "retarget.h"
#include "cmdline.h"
#include "cli_utils.h"
#include "assem_test_info.h"


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


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



/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Flash_Type_Set (FILE *pf_cli, char *pstri_flash_type)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**     (in) pstri_flash_type - pointer to string flash type.
**   Return      :
**      CMDLINE_INVALID_ARGS - Invalid argument.
**      CMDLINE_OK - Command line is OK.
**   Description : Set ATI information flash type to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Flash_Type_Set (FILE *pf_cli, char *pstri_flash_type)
{
   int32_t s32_result = CMDLINE_OK;
   int32_t s32_write_result;
   
   if (strcmp(pstri_flash_type, "0") == 0)
   {
      s32_write_result = s32_PARM_Write(PARM_ID_ASSEM_INFO_FPGA_FLASH, PARM_ASSEM_INFO_FLASH_M25P128);
   }
   else if (strcmp(pstri_flash_type, "1") == 0)
   {
      s32_write_result = s32_PARM_Write(PARM_ID_ASSEM_INFO_FPGA_FLASH, PARM_ASSEM_INFO_FLASH_W25Q128);
   }
   else if (strcmp(pstri_flash_type, "2") == 0)
   {
      s32_write_result = s32_PARM_Write(PARM_ID_ASSEM_INFO_FPGA_FLASH, PARM_ASSEM_INFO_FLASH_S25FL128);
   }
   else
   {
      s32_result = CMDLINE_INVALID_ARGS;
   }
   
   if (s32_result == CMDLINE_OK)
   {
      if (s32_write_result == 0)
      {
         fprintf(pf_cli, "OK\r\n");
      }
      else
      {
         fprintf(pf_cli, "Write error\r\n");
      }
   }
   
   return (s32_result);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Flash_Type_Get (FILE *pf_cli)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**   Return      : (0)
**   Description : Get ATI information flash type from parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Flash_Type_Get (FILE *pf_cli)
{   
   switch (s32_PARM_Read(PARM_ID_ASSEM_INFO_FPGA_FLASH))
   {
      case 0:
         fprintf(pf_cli, "M25P128\r\n");
      break;
      
      case 1:
         fprintf(pf_cli, "W25Q128\r\n");
      break;

      case 2:
         fprintf(pf_cli, "S25FL128\r\n");
      break;

      default:
         fprintf(pf_cli, "Invalid flash type\r\n");
      break;
   }
   return (0);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Xtal_Pulling_Set (FILE *pf_cli, char *pstri_xtal_pulling)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**     (in) pstri_xtal_pulling - pointer to string flash type.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Set ATI xtal pulling to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Xtal_Pulling_Set (FILE *pf_cli, char *pstri_xtal_pulling)
{
   int32_t s32_write_result;
   
   s32_write_result = s32_PARM_Write(PARM_ID_ASSEM_INFO_FPGA_XTAL_PULLING, atoi(pstri_xtal_pulling));
   
   if (s32_write_result == 0)
   {
      fprintf(pf_cli, "OK\r\n");
   }
   else
   {
      fprintf(pf_cli, "Write error\r\n");
   }
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Xtal_Pulling_Get (FILE *pf_cli)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Get ATI xtal pulling from parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Xtal_Pulling_Get (FILE *pf_cli)
{
   char stri_xtal[11];

   /* Convert Xtal value to string. */
   sprintf(stri_xtal, "%d", s32_PARM_Read(PARM_ID_ASSEM_INFO_FPGA_XTAL_PULLING));

   /* Print value with linefeed and carriage return to UART. */
   fprintf(pf_cli, stri_xtal);
   fprintf(pf_cli, "\r\n");
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Assem_Company_Set (FILE *pf_cli, char *pstri_assem_company)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**     (in) pstri_assem_company - pointer to assemble company string.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : ATI assemble company set to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Assem_Company_Set (FILE *pf_cli, char *pstri_assem_company)
{
   uint32_t u32_max_len;
   uint8_t au8_value[20];
   int32_t *ps32_value;
   uint32_t u32_idx;
   int32_t s32_write_result;
   
   memset(au8_value, 0, sizeof(au8_value));
   
   if (strlen(pstri_assem_company) >= 20)
   {
      /* Max length is only 19 characters since we need one character for null terminated. */
      u32_max_len = 19;
   }
   else
   {
      u32_max_len = strlen(pstri_assem_company);
   }
   
   memcpy(au8_value, pstri_assem_company, u32_max_len);
   ps32_value = (int32_t *)au8_value;
   for (u32_idx = 0; u32_idx < 5; u32_idx++)
   {
      s32_write_result = s32_PARM_Write((ENUM_PARM_ID)(PARM_ID_ASSEM_TEST_COMPANY_0 + u32_idx), *ps32_value);
      ps32_value += 1;
      if (s32_write_result != 0)
      {
         break;
      }
   }
   
   if (s32_write_result == 0)
   {
      fprintf(pf_cli, "OK\r\n");
   }
   else
   {
      fprintf(pf_cli, "Write error\r\n");
   }
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Assem_Company_Get (FILE *pf_cli)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Get ATI assemble company string from parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Assem_Company_Get (FILE *pf_cli)
{
   uint8_t au8_value[20];
   int32_t *ps32_value;
   uint32_t u32_idx;
   
   ps32_value = (int32_t *)au8_value;
   for (u32_idx = 0; u32_idx < 5; u32_idx++)
   {
      *ps32_value = s32_PARM_Read((ENUM_PARM_ID)(PARM_ID_ASSEM_TEST_COMPANY_0 + u32_idx));
      ps32_value += 1;
   }
   
   fprintf(pf_cli, (const char *)au8_value);
   fprintf(pf_cli, "\r\n");
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Assem_Tester_Set (FILE *pf_cli, char *pstri_assem_tester)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**     (in)  pstri_assem_tester - pointer to assemble tester string
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Set ATI assemble tester string to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Assem_Tester_Set (FILE *pf_cli, char *pstri_assem_tester)
{
   uint32_t u32_max_len;
   uint8_t au8_value[20];
   int32_t *ps32_value;
   uint32_t u32_idx;
   int32_t s32_write_result;
   
   memset(au8_value, 0, sizeof(au8_value));
   
   if (strlen(pstri_assem_tester) >= 20)
   {
      /* Max length is only 19 characters since we need one character for null terminated. */
      u32_max_len = 19;
   }
   else
   {
      u32_max_len = strlen(pstri_assem_tester);
   }
   
   memcpy(au8_value, pstri_assem_tester, u32_max_len);
   ps32_value = (int32_t *)au8_value;
   for (u32_idx = 0; u32_idx < 5; u32_idx++)
   {
      s32_write_result = s32_PARM_Write((ENUM_PARM_ID)(PARM_ID_ASSEM_TEST_OPERATOR_0 + u32_idx), *ps32_value);
      ps32_value += 1;
      if (s32_write_result != 0)
      {
         break;
      }
   }
   
   if (s32_write_result == 0)
   {
      fprintf(pf_cli, "OK\r\n");
   }
   else
   {
      fprintf(pf_cli, "Write error\r\n");
   }
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Assem_Tester_Get (FILE *pf_cli)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Get ATI assemble tester string to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Assem_Tester_Get (FILE *pf_cli)
{
   uint8_t au8_value[20];
   int32_t *ps32_value;
   uint32_t u32_idx;
   
   ps32_value = (int32_t *)au8_value;
   for (u32_idx = 0; u32_idx < 5; u32_idx++)
   {
      *ps32_value = s32_PARM_Read((ENUM_PARM_ID)(PARM_ID_ASSEM_TEST_OPERATOR_0 + u32_idx));
      ps32_value += 1;
   }
   
   fprintf(pf_cli, (const char *)au8_value);
   fprintf(pf_cli, "\r\n");
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Assem_Test_Time_Set (FILE *pf_cli, char *pstri_assem_test_time)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**     (in)  pstri_assem_test_time - pointer to assemble test time string.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Set ATI assemble test time to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Assem_Test_Time_Set (FILE *pf_cli, char *pstri_assem_test_time)
{
   int32_t s32_write_result;
   
   s32_write_result = s32_PARM_Write(PARM_ID_ASSEM_TEST_UNIX_TIME, atoi(pstri_assem_test_time));
   
   if (s32_write_result == 0)
   {
      fprintf(pf_cli, "OK\r\n");
   }
   else
   {
      fprintf(pf_cli, "Write error\r\n");
   }
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Assem_Test_Time_Get (FILE *pf_cli)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Get ATI assemble test time from parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Assem_Test_Time_Get (FILE *pf_cli)
{
   char stri_assem_test_time[11];

   /* Convert Xtal value to string. */
   sprintf(stri_assem_test_time, "%d", s32_PARM_Read(PARM_ID_ASSEM_TEST_UNIX_TIME));

   /* Print value with linefeed and carriage return to UART. */
   fprintf(pf_cli, stri_assem_test_time);
   fprintf(pf_cli, "\r\n");
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Final_Tester_Set (FILE *pf_cli, char *pstri_final_tester)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**     (in)  pstri_final_tester - pointer to final tester string.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Set ATI final tester string to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Final_Tester_Set (FILE *pf_cli, char *pstri_final_tester)
{
   uint32_t u32_max_len;
   uint8_t au8_value[20];
   int32_t *ps32_value;
   uint32_t u32_idx;
   int32_t s32_write_result;
   
   memset(au8_value, 0, sizeof(au8_value));
   
   if (strlen(pstri_final_tester) >= 20)
   {
      /* Max length is only 19 characters since we need one character for null terminated. */
      u32_max_len = 19;
   }
   else
   {
      u32_max_len = strlen(pstri_final_tester);
   }
   
   memcpy(au8_value, pstri_final_tester, u32_max_len);
   ps32_value = (int32_t *)au8_value;
   for (u32_idx = 0; u32_idx < 5; u32_idx++)
   {
      s32_write_result = s32_PARM_Write((ENUM_PARM_ID)(PARM_ID_FINAL_TEST_OPERATOR_0 + u32_idx), *ps32_value);
      ps32_value += 1;
      if (s32_write_result != 0)
      {
         break;
      }
   }
   
   if (s32_write_result == 0)
   {
      fprintf(pf_cli, "OK\r\n");
   }
   else
   {
      fprintf(pf_cli, "Write error\r\n");
   }
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Final_Tester_Get (FILE *pf_cli)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Get ATI final tester string from parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Final_Tester_Get (FILE *pf_cli)
{
   uint8_t au8_value[20];
   int32_t *ps32_value;
   uint32_t u32_idx;
   
   ps32_value = (int32_t *)au8_value;
   for (u32_idx = 0; u32_idx < 5; u32_idx++)
   {
      *ps32_value = s32_PARM_Read((ENUM_PARM_ID)(PARM_ID_FINAL_TEST_OPERATOR_0 + u32_idx));
      ps32_value += 1;
   }
   
   fprintf(pf_cli, (const char *)au8_value);
   fprintf(pf_cli, "\r\n");
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Final_Test_Time_Set (FILE *pf_cli, char *pstri_final_test_time)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**     (in)  pstri_final_test_time - pointer to final test time string.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Set ATI final test time to parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Final_Test_Time_Set (FILE *pf_cli, char *pstri_final_test_time)
{
   int32_t s32_write_result;
   
   s32_write_result = s32_PARM_Write(PARM_ID_FINAL_TEST_UNIX_TIME, atoi(pstri_final_test_time));
   
   if (s32_write_result == 0)
   {
      fprintf(pf_cli, "OK\r\n");
   }
   else
   {
      fprintf(pf_cli, "Write error\r\n");
   }
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_ATI_Final_Test_Time_Get (FILE *pf_cli)
**   Arguments   :
**     (out) pf_cli - pointer to command line file.
**   Return      :
**      CMDLINE_OK - Command line is OK.
**   Description : Get ATI final test time from parameter module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_ATI_Final_Test_Time_Get (FILE *pf_cli)
{
   char stri_final_test_time[11];

   /* Convert Xtal value to string. */
   sprintf(stri_final_test_time, "%d", s32_PARM_Read(PARM_ID_FINAL_TEST_UNIX_TIME));

   /* Print value with linefeed and carriage return to UART. */
   fprintf(pf_cli, stri_final_test_time);
   fprintf(pf_cli, "\r\n");
   
   return (CMDLINE_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
