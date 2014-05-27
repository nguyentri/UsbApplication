/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : cli.c
**   Project     : IMT - Main CPU - Main Application.
**   Author      : Nguyen Anh Huy
**   Revision    : 1.0.0.1
**   Date        : 2013/05/20.
**   Description : Command Line Interface module.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Application includes. */
#include "stm32f4xx.h"
//#include "iwdg.h"
#include "retarget.h"
#include "cmdline.h"
#include "cli_utils.h"
#include "cli.h"


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* CLI UART port */
#define CLI_UART_PORT             RETARGET_UART_PORT_2

/* CLI task stack size. */
#define CLI_TASK_STACK_SIZE       (configMINIMAL_STACK_SIZE + 512)

/* CLI task priority (lowest priority except Idle Task). */
#define CLI_TASK_PRIORITY         (tskIDLE_PRIORITY + 1)

/*
** CLI task processing rate (delay).
** The task shall be activated every 100ms.
*/
#define CLI_TASK_DELAY            (portTickType)(100 / portTICK_RATE_MS)


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* CLI Task. */
static void v_CLI_Task (void *pvParameters);

/* Function to handle "help" command. */
static int32_t s32_CLI_Cmd_Help (int32_t argc, char *argv[]);
/* Function to handle "help" command. */
static int32_t s32_CLI_Cmd_Test (int32_t argc, char *argv[]);
/* Function to handle "help" command. */
static int32_t s32_CLI_Cmd_Raw (int32_t argc, char *argv[]);
/* Function to handle "help" command. */
static int32_t s32_CLI_Cmd_Debug (int32_t argc, char *argv[]);
/* Function to handle "set" command. */
static int32_t s32_CLI_Cmd_Set (int32_t argc, char *argv[]);
/* Function to handle "get" command. */
static int32_t s32_CLI_Cmd_Get (int32_t argc, char *argv[]);

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* File pointer. */
FILE *pf_cli;

/* CLI task name. */
static const char stri_cli_task_name[] = "CLI";

/* CLI command buffer. */
char stri_cmd_buf[64];

/* The table of commands supported by this application. */
CMD_LINE_ENTRY_T astru_cmd_table[] =
{
   {"help",   s32_CLI_Cmd_Help,   "Display help menu."},
   {"test",   s32_CLI_Cmd_Test,   "Test board."},
   {"raw",    s32_CLI_Cmd_Raw,    "Output raw measurement data."},
   {"debug",  s32_CLI_Cmd_Debug,  "Output debug information."},
   {"set",    s32_CLI_Cmd_Set,    "Set assembly information."},
   {"get",    s32_CLI_Cmd_Get,    "Get assembly information."},
   {NULL,     NULL,               NULL}   
};

/* Clear screen string. */
const char stri_clear_screen[] = {0x1B, '[', '2', 'J', 0};

/* Help menu table 1. */
const char stri_help_menu[] =
   "\n\r"
   "+ Command -+ Syntax -----+ Description-----------------------------------|\n\r"
   "|          |             |                                               |\n\r"
   "| help     | help        | Display help menu                             |\n\r"
   "|------------------------------------------------------------------------|\n\r"
   "| test     | test        | Perform board test process                    |\n\r"
   "|------------------------------------------------------------------------|\n\r"
   "| raw      | raw         | Output raw measurement data                   |\n\r"
   "|------------------------------------------------------------------------|\n\r"
   "| debug    | debug       | Output debug information                      |\n\r"
   "|------------------------------------------------------------------------|\n\r"
   "| fsmc | <fsmc> <value> <number of cycles to write and read> | Test fsmc |\n\r"
   "|------------------------------------------------------------------------|\n\r";/* Command prompt. */

const char stri_cmd_prompt[] = "CLI>";

/* Bad command error. */
const char stri_bad_cmd[] = "Bad command.\n\r";

/* Too many arguments error. */
const char stri_too_many_agr[] = "Too many arguments.\n\r";

/* Invalid arguments error. */
const char stri_invalid_agr[] = "Invalid arguments.\n\r";


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_CLI_Init (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Initialize Command Line module.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_CLI_Init (void)
{
   USART_InitTypeDef USART_InitStructure;

	/* USARTx configured as follow:
	        - BaudRate = 115200 baud  
	        - Word Length = 8 Bits
	        - One Stop Bit
	        - No parity
	        - Hardware flow control disabled (RTS and CTS signals)
	        - Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   
   pf_cli = RETARGET_Init(CLI_UART_PORT, &USART_InitStructure, 0);

   /* Create a task to output the measurement every 1 second. */
   xTaskCreate(v_CLI_Task,
               (signed portCHAR *)stri_cli_task_name,
               CLI_TASK_STACK_SIZE,
               NULL,
               CLI_TASK_PRIORITY,
               NULL);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_CLI_Task (void *pvParameters)
**   Arguments   :
**      pvParameters - Pointer to Commmand Line Task Parameters.
**   Return      : n/a
**   Description : Commmand Line Task.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_CLI_Task (void *pvParameters)
{
   while (1)
   {
      /* Delay CLI task for 100ms. */
      vTaskDelay(CLI_TASK_DELAY);

      /*
      ** User has pressed 'Enter', Carriage Return character is
      ** available in the command string.
      ** Process the command.
      */
      if (fpeek(pf_cli, '\n') != -1)
      {
         /* Get the command. */
         fgets(stri_cmd_buf, sizeof(stri_cmd_buf), pf_cli);
         v_CLI_Remove_Terminal_Chars(stri_cmd_buf);

         /* Process the command. */
         switch (s32_Cmd_Line_Process(stri_cmd_buf))
         {
            /* Bad command. */
            case CMDLINE_BAD_CMD:
               /* Print Bad Command. */
               fprintf(pf_cli, "%s", stri_bad_cmd);
            break;

            /* Too many arguments. */
            case CMDLINE_TOO_MANY_ARGS:
               /* Print Too Many Arguments. */
               fprintf(pf_cli, "%s", stri_too_many_agr);
            break;

            case CMDLINE_INVALID_ARGS:
               /* Print Invalid Arguments. */
               fprintf(pf_cli, "%s", stri_invalid_agr);
            break;

            /* Process command sucessfully. */
            default:
               /* Do nothing here. */
            break;
         }
         
         /* Print Command Prompt. */
         fprintf(pf_cli, "%s", stri_cmd_prompt);
      }
      
      /* Set task running flag is true. */
//      v_Set_Task_Running_Flag(USART_MEASOUT_TASK_IDX, __TRUE);
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_CLI_Cmd_Help (int32_t argc, char *argv[])
**   Arguments   :
**     (in) argc - argument count.
**     (in) argv - argument vector.
**   Return      : n/a        
**   Description : Print Help Menu.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static int32_t s32_CLI_Cmd_Help (int32_t argc, char *argv[])
{
   /* Print Help Menu. */
   fprintf(pf_cli, "%s", stri_help_menu);

   return (0);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_CLI_Cmd_Test (int32_t argc, char *argv[])
**   Arguments   :
**     (in) argc - argument count.
**     (in) argv - argument vector.
**   Return      : n/a
**   Description : Run board test process.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static int32_t s32_CLI_Cmd_Test (int32_t argc, char *argv[])
{
   /* Run board test process. */
 //  v_Board_Test(pf_cli);

   return (0);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_CLI_Cmd_Raw (int32_t argc, char *argv[])
**   Arguments   :
**     (in) argc - argument count.
**     (in) argv - argument vector.
**   Return      : n/a
**   Description : Output raw measurement data.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static int32_t s32_CLI_Cmd_Raw (int32_t argc, char *argv[])
{
   /* Output raw measurement data. */
//   v_Raw_Meas_Output(pf_cli);

   return (0);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_CLI_Cmd_Debug (int32_t argc, char *argv[])
**   Arguments   :
**     (in) argc - argument count.
**     (in) argv - argument vector.
**   Return      : n/a
**   Description : Print Help Menu.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static int32_t s32_CLI_Cmd_Debug (int32_t argc, char *argv[])
{
   /* Print Help Menu. */
   fprintf(pf_cli, "%s", stri_help_menu);

   return (0);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_CLI_Cmd_Set (int32_t argc, char *argv[])
**   Arguments   :
**     (in) argc - argument count.
**     (in) argv - argument vector.
**   Return      : 
**      CMDLINE_BAD_CMD - Bad command line.
**      CMDLINE_INVALID_ARGS - Invalid argument.
**      CMDLINE_OK - Command line is OK.
**   Description : Save command line to non-volatile memory.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static int32_t s32_CLI_Cmd_Set (int32_t argc, char *argv[])
{
   int32_t s32_result = 0;
   
   if (argc != 3)
   {
      return (CMDLINE_BAD_CMD);
   }
//    
//    if (strcmp(argv[1], "flash") == 0)
//    {
//       s32_result = s32_ATI_Flash_Type_Set(pf_cli, argv[2]);
//    }
//    else if (strcmp(argv[1], "xtal") == 0)
//    {
//       s32_result = s32_ATI_Xtal_Pulling_Set(pf_cli, argv[2]);
//    }
//    else if (strcmp(argv[1], "acompany") == 0)
//    {
//       s32_result = s32_ATI_Assem_Company_Set(pf_cli, argv[2]);
//    }
//    else if (strcmp(argv[1], "atester") == 0)
//    {
//       s32_result = s32_ATI_Assem_Tester_Set(pf_cli, argv[2]);
//    }
//    else if (strcmp(argv[1], "atime") == 0)
//    {
//       s32_result = s32_ATI_Assem_Test_Time_Set(pf_cli, argv[2]);
//    }
//    else if (strcmp(argv[1], "ftester") == 0)
//    {
//       s32_result = s32_ATI_Final_Tester_Set(pf_cli, argv[2]);
//    }
//    else if (strcmp(argv[1], "ftime") == 0)
//    {
//       s32_result = s32_ATI_Final_Test_Time_Set(pf_cli, argv[2]);
//    }
//    else
//    {
//       s32_result = CMDLINE_INVALID_ARGS;
//    }
   
   return (s32_result);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_CLI_Cmd_Get (int32_t argc, char *argv[])
**   Arguments   :
**     (in) argc - argument count.
**     (in) argv - argument vector.
**   Return      : 
**      CMDLINE_BAD_CMD - Bad command line.
**      CMDLINE_INVALID_ARGS - Invalid argument.
**      CMDLINE_OK - Command line is OK.
**   Description : Get command line from non-volatile memory.
**   Notes       : restrictions, odd modes
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static int32_t s32_CLI_Cmd_Get (int32_t argc, char *argv[])
{
   int32_t s32_result = 0;
   
   if (argc != 2)
   {
      return (CMDLINE_BAD_CMD);
   }
   
//    if (strcmp(argv[1], "flash") == 0)
//    {
//       s32_result = s32_ATI_Flash_Type_Get(pf_cli);
//    }
//    else if (strcmp(argv[1], "xtal") == 0)
//    {
//       s32_result = s32_ATI_Xtal_Pulling_Get(pf_cli);
//    }
//    else if (strcmp(argv[1], "acompany") == 0)
//    {
//       s32_result = s32_ATI_Assem_Company_Get(pf_cli);
//    }
//    else if (strcmp(argv[1], "atester") == 0)
//    {
//       s32_result = s32_ATI_Assem_Tester_Get(pf_cli);
//    }
//    else if (strcmp(argv[1], "atime") == 0)
//    {
//       s32_result = s32_ATI_Assem_Test_Time_Get(pf_cli);
//    }
//    else if (strcmp(argv[1], "ftester") == 0)
//    {
//       s32_result = s32_ATI_Final_Tester_Get(pf_cli);
//    }
//    else if (strcmp(argv[1], "ftime") == 0)
//    {
//       s32_result = s32_ATI_Final_Test_Time_Get(pf_cli);
//    }
//    else
//    {
//       s32_result = CMDLINE_INVALID_ARGS;
//    }
   
   return (s32_result);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
