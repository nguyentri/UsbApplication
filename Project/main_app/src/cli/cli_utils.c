/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : cli_utils.c
**   Project     : IMT - Main CPU - Main Application.
**   Author      : Nguyen Anh Huy
**   Revision    : 1.0.0.1
**   Date        : 2013/05/20.
**   Description : Command Line Utils module.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Standard includes. */
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* Application includes. */
#include "stm32f4xx.h"
#include "retarget.h"
#include "cli_utils.h"


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

/* Command line buffer. */
extern char stri_cmd_buf[64];


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_CLI_Remove_Terminal_Chars (char *pstri_buf)
**   Arguments   :
**      (in-out) pstri_buf - pointer to string buffer.
**   Return      : n/a
**   Description : Function to remove terminal characters.
**   Notes       : 
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_CLI_Remove_Terminal_Chars (char *pstri_buf)
{
   uint32_t u32_idx;
   
   for (u32_idx = 0; u32_idx < strlen(pstri_buf); u32_idx++)
   {
      if ((pstri_buf[u32_idx] == '\r') || (pstri_buf[u32_idx] == '\n'))
      {
         pstri_buf[u32_idx] = 0;
      }
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_CLI_Check_Quit_Cmd (FILE *pf_handle, const char *pstri_qcmd)
**   Arguments   :
**      (in) pf_handle - pointer to file handle.
**      (in) pstri_qcmd - pointer to quit command string.
**   Return      :
**       0 - Is quit commmand.
**       1 - Is not quit command.
**   Description : Function to check quit commmand.
**   Notes       : 
**   Author      : Nguyen Anh Huy.
**   Date        : 2013/05/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_CLI_Check_Quit_Cmd (FILE *pf_handle, const char *pstri_qcmd)
{
   int32_t s32_result;
   
   s32_result = 0;
   
   while (fpeek(pf_handle, '\n') != -1)
   {
      /* Get the command. */
      fgets(stri_cmd_buf, sizeof(stri_cmd_buf), pf_handle);
      /* remove terminated chars. */
      v_CLI_Remove_Terminal_Chars(stri_cmd_buf);
      
      /* Compare to quit command. */
      if (strcmp(stri_cmd_buf, pstri_qcmd) == 0)
      {
         s32_result = 1;
         break;
      }
   }
   
   return (s32_result);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
