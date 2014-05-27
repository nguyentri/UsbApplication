/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : hex_parser.h
**   Project     : USB Boot-loader
**   Author      : Nguyen Trong Tri
**   Version     : 1.0
**   Date        : 2014/03/18
**   Description : Header file of Hex Parser module.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef __HEX_PARSER_H__
#define __HEX_PARSER_H__


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   INCLUDE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Hex processing result. */
#define HEX_RECORD_OK             0
#define HEX_RECORD_BAD            1
#define HEX_RECORD_FINISHED       2

/* Hex record type. */
#define HEX_RECORD_TYPE_DATA                0
#define HEX_RECORD_TYPE_END                 1
#define HEX_RECORD_TYPE_EXTD_SEG_ADD        2
#define HEX_RECORD_TYPE_EXTD_SEG_START      3
#define HEX_RECORD_TYPE_EXTD_LINEAR_ADD     4
#define HEX_RECORD_TYPE_EXTD_LINEAR_START   5

/* Maximum data length for each hex record. */
#define HEX_DATA_LEN_MAX          64

/* Hex record structure. */
typedef struct
{
   uint8_t    u8_type;
   uint16_t   u16_address;
   uint8_t    u8_data_len;
   uint8_t    u8_data[HEX_DATA_LEN_MAX];
} HEX_RECORD_STRUCT;


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Function to process the hex buffer received via Modbus communication. */
uint8_t u8_Hex_Processing (uint8_t *pu8_hex_buf, uint8_t u8_hex_len);


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


#endif

/* 
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
