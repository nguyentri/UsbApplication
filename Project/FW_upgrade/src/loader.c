/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : loader.c
**   Project     : USB Boot-loader
**   Author      : Nguyen Trong Tri
**   Version     : 1.0
**   Date        : 2014/03/18
**   Description : This is a hex loader module.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"
#include "flash_if.h"
#include "hex_parser.h"
#include "loader.h"

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#define LOADER_HEX_FILE_NAME_LEN_MAX                       20

#define LOADER_HEX_LINE_LEN_MAX                            64

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   TYPEDEF SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLES SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

//static FIL file_hex;

/* Store firmware hex file name. */
//static char stri_hex_filename[LOADER_HEX_FILE_NAME_LEN_MAX];

static char stri_hex_line[LOADER_HEX_LINE_LEN_MAX];

static uint8_t u8_hex_line_bin[LOADER_HEX_LINE_LEN_MAX];

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : int32_t s32_LOADER_Process (FIL* file_hex)
**   Arguments   : 
**     (in)- file_hex : main application hex file 
**   Return      : n/a
**   Description : Load the hex file.
**   Notes       : 
**   Author      : Nguyen trong Tri.
**   Date        : 2014/03/12.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t s32_LOADER_Process (FIL* file_hex)
{
 //  FRESULT res;
   uint32_t u32_hex_len;
   uint8_t u8_idx;
   char stri_temp[3];
	
	 FIL* fil_tem = file_hex;

   memset(stri_temp, 0, sizeof(stri_temp));

  /* Read each line of the hex file. */
  while (f_gets(stri_hex_line, sizeof(stri_hex_line), fil_tem) != NULL)
  {
	 /* Clear buffer. */
	 memset(u8_hex_line_bin, 0, sizeof(u8_hex_line_bin));

	 /* Do not count linefeed (\n) in the hex line. */
	 u32_hex_len = strlen(stri_hex_line) - 1;
	 
	 /* Convert to binary data. */
	 u8_hex_line_bin[0] = stri_hex_line[0];
	 for (u8_idx = 1; u8_idx <= ((u32_hex_len - 1) / 2); u8_idx++)
	 {
		stri_temp[0] = stri_hex_line[u8_idx * 2 - 1];
		stri_temp[1] = stri_hex_line[u8_idx * 2];
		u8_hex_line_bin[u8_idx] = strtoul((const char *)stri_temp, 0, 16);
	 }

	 /* Process hex record. */
	 u8_Hex_Processing(u8_hex_line_bin, u8_idx);         
  }

  /* Return OK. */
  return (LOADER_NO_ERR);
}



