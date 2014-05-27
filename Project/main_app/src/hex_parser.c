/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : hex_parser.c
**   Project     : USB Boot-loader
**   Author      : Nguyen Trong Tri
**   Version     : 1.0
**   Date        : 2014/03/14
**   Description : Hex Parser - this module contains functions to parse the
**                 Intel/Keil hex data and write to flash memory.
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

/* Application includes. */
#include "flash_if.h"
#include "hex_parser.h"


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Elements offset in a Hex record. */
#define HEX_DATA_LEN_OFFSET       1
#define HEX_HIGH_ADDRESS_OFFSET   2
#define HEX_LOW_ADDRESS_OFFSET    3
#define HEX_RECORD_TYPE_OFFSET    4
#define HEX_DATA_OFFSET           5

/* Total bytes that is not data in a Hex record. */
#define HEX_RECORD_OVERHEAD       6


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Local function to analyze a Hex record. */
static uint8_t u8_Hex_Analyze (uint8_t *pu8_hex_buf, uint8_t u8_hex_len,
                          HEX_RECORD_STRUCT *pstru_hex_record);


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Processing Hex record structure. */
static HEX_RECORD_STRUCT stru_hex_record;

/* Current flash address to write firmware data to. */
static uint32_t u32_fw_full_addr = 0;

/*
** Current extended flash address.
** This address must be combined with address offset
** in the hex record to form the full address.
*/
static uint16_t u16_fw_extd_addr = 0;

/* First hex record data. */
static uint8_t au8_first_data[HEX_DATA_LEN_MAX];

/* First hex record data length. */
static uint8_t u8_first_data_len;


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : uint8_t u8_Hex_Processing (uint8_t *pu8_hex_buf, uint8_t u8_hex_len)
**   Arguments   :
**      (in) pu8_hex_buf   -   Hex buffer to process.
**      (in) u8_hex_len    -   Hex buffer length.
**   Return      :
**      HEX_RECORD_OK      -   Process the hex buffer sucessully.
**      HEX_RECORD_BAD     -   Hex buffer is corrupted or not supported.
**      HEX_RECORD_FINISH  -   A end of file hex record has been received.
**   Description : This function called by the Modbus call-back function to
**                 process a received hex buffer.
**                 A Hex buffer can be constructed by one or more hex records.
**                 Base on the record type, this function can write the data
**                 to flash memory, set the address, or return error if hex
**                 record is corrupted or not supported.
**   Notes       : Only hex record type 0, 1, 4, 5 are supported now.
**   Author      : Nguyen Trong Tri.
**   Date        : 2014/09/25.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
uint8_t u8_Hex_Processing (uint8_t *pu8_hex_buf, uint8_t u8_hex_len)
{
   /* Pointer to the processing hex record in buffer. */
   uint8_t *pu8_hex;
   /* Hex buffer length has not been processed. */
   uint8_t u8_len;
   /* Process result. */
   uint8_t u8_result;

   /* Maximum application flash size. */
   uint32_t u32_app_flash_size;

   /*
   ** Save pointer to hex buffer and hex length to local
   ** variables for internal processing.
   */
   pu8_hex = pu8_hex_buf;
   u8_len = u8_hex_len;
   u8_result = HEX_RECORD_OK;

   u32_app_flash_size = 0x8100000;

   /* Process each record in the hex buffer. */
   for ( ;u8_len != 0; )
   {
      if (u8_Hex_Analyze(pu8_hex, u8_len, &stru_hex_record) == HEX_RECORD_BAD)
      {
         /*
         ** If bad record, stop processing and
         ** return Bad Hex Record notification.
         */
         u8_result = HEX_RECORD_BAD;
         break;
      }

      /* Hex record processing state machine. */
      switch (stru_hex_record.u8_type)
      {
         /*
         ** Record type = 0 - Data.
         ** Save data to the flash page.
         */
         case HEX_RECORD_TYPE_DATA:
            /* Get full address. */
            u32_fw_full_addr = ((uint32_t)u16_fw_extd_addr << 16) + (uint32_t)(stru_hex_record.u16_address);
            /*
            ** If the address exceed maximum flash size
            ** for application, return error.
            */
            if (u32_fw_full_addr >= u32_app_flash_size)
            {
               u8_result = HEX_RECORD_BAD;
            }
            else
            {  
//               /*
//               ** If the address is located on new flash block,
//               ** erase this flash block.
//               */
//               if ((u32_fw_full_addr % FLASH_PAGE_SIZE) == 0)
//               {
//                  /* Erase this block. */
//                  FlashErase(u32_fw_full_addr);
//               }

               /*
               ** If this is the first record data, the address
               ** is equal to the application start address.
               */
               if (u32_fw_full_addr == APPLICATION_ADDRESS)
               {
                  /*
                  ** Do not write this record data to flash now.
                  ** Save first hex record data and length.
                  ** They shall be writen to flash at the end to
                  ** ensure that the whole application is transfered.
                  ** If the transmission is conrrupted in the middle by any reason,
                  ** the first hex record data shall not be writen to flash and
                  ** the boot-loader shall recognize this and doesn't call the application.
                  */
                  u8_first_data_len = stru_hex_record.u8_data_len;
                  memcpy(au8_first_data, stru_hex_record.u8_data, u8_first_data_len);
               }
               else
               {
                  /* Write firmware data to flash. */
                  FLASH_If_Write((volatile uint32_t *)&u32_fw_full_addr, (uint32_t *)(stru_hex_record.u8_data), (uint32_t)(stru_hex_record.u8_data_len / 4));
               }
            }
         break;

         /*
         ** Record type = 1 - End of file.
         ** Set the flag to finish firmware update.
         */
         case HEX_RECORD_TYPE_END:
            /*
            ** The whole application has been transfered.
            ** Write the first hex record data to flash.
            */
            u32_fw_full_addr = APPLICATION_ADDRESS;
            FLASH_If_Write((volatile uint32_t *)&u32_fw_full_addr, (uint32_t *)au8_first_data, (uint32_t)u8_first_data_len / 4);
            /* Return hex transfer finished. */
            u8_result = HEX_RECORD_FINISHED;
         break;

         /*
         ** Record type = 2 - Extended segment address.
         ** Not supported now, return hex record error.
         */
         case HEX_RECORD_TYPE_EXTD_SEG_ADD:
            u8_result = HEX_RECORD_BAD;
         break;

         /*
         ** Record type = 3 - Start segment address.
         ** Not supported now, return hex record error.
         */
         case HEX_RECORD_TYPE_EXTD_SEG_START:
            u8_result = HEX_RECORD_BAD;
         break;

         /*
         ** Record type = 4 - Extended linear address.
         ** Set the upper linear address to new value.
         */
         case HEX_RECORD_TYPE_EXTD_LINEAR_ADD:
            u16_fw_extd_addr = ((uint16_t)(stru_hex_record.u8_data[0]) << 8) +
                               (uint16_t)(stru_hex_record.u8_data[1]);
         break;

         /*
         ** Record type = 5 - Start linear address.
         ** Check if start address == application start address, or
         ** boot-loader start address (0x0).
         ** If not, return error.
         */
         case HEX_RECORD_TYPE_EXTD_LINEAR_START:
            u32_fw_full_addr = ((uint32_t)(stru_hex_record.u8_data[0]) << 24) +
                               ((uint32_t)(stru_hex_record.u8_data[1]) << 16) +
                               ((uint32_t)(stru_hex_record.u8_data[2]) << 8) +
                               (uint32_t)(stru_hex_record.u8_data[3]);
//            if ((u32_fw_full_addr != APPLICATION_ADDRESS) &&
//                (u32_fw_full_addr != 0) &&
//                ((u32_fw_full_addr%0x10000) != 0))
//            {
//               u8_result = HEX_RECORD_BAD;
//            }
         break;

         default:
            u8_result = HEX_RECORD_BAD;
         break;
      }

      /*
      ** Stop processing hex buffer if the result is bad
      ** or received end-of-file record.
      */
      if (u8_result != HEX_RECORD_OK)
      {
         break;
      }

      /* Processing result is good, move to the next record. */
      u8_len = u8_len - (stru_hex_record.u8_data_len + HEX_RECORD_OVERHEAD);
      pu8_hex = pu8_hex + (stru_hex_record.u8_data_len + HEX_RECORD_OVERHEAD);
   }

   /* Return the result. */
   return (u8_result);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static uint8_t u8_Hex_Analyze (uint8_t *pu8_hex_buf, uint8_t u8_hex_len,
**                                           HEX_RECORD_STRUCT *pstru_hex_record)
**   Arguments   :
**      (in) pu8_hex_buf        -   Pointer to the begining of hex buffer
**                                  to analyze.
**      (in) u8_hex_len         -   Hex buffer length.
**      (in) pstru_hex_record   -   Pointer to hex record structure to store
**                                  the analyzed result data.
**   Return      :
**      HEX_RECORD_OK    -   The hex record is processed succesfully.
**      HEX_RECORD_BAD   -   The hex record is corrupted.
**   Description : This function analyze a hex record to get type, address,
**                 data...
**   Notes       : This local function analyzes the first hex record in the
**                 buffer only. The caller needs to call this function
**                 repeatedly to process all records available in the buffer.
**   Author      : Nguyen Trong Tri.
**   Date        : 2014/09/25.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static uint8_t u8_Hex_Analyze (uint8_t *pu8_hex_buf, uint8_t u8_hex_len,
                          HEX_RECORD_STRUCT *pstru_hex_record)
{
   uint8_t u8_data_len;
   uint8_t u8_calc_checksum;
   uint8_t u8_idx;

   /* Check if we have a valid hex record. */
   if ((*pu8_hex_buf) != ':')
   {
      return (HEX_RECORD_BAD);
   }

   /* Validate hex buffer length. */
   u8_data_len = *(pu8_hex_buf + HEX_DATA_LEN_OFFSET);
   if (u8_hex_len < (HEX_RECORD_OVERHEAD + u8_data_len))
   {
      return (HEX_RECORD_BAD);
   }

   /* Validate checksum. */
   u8_calc_checksum = 0;
   for (u8_idx = 0; u8_idx < u8_data_len + 4; u8_idx++)
   {
      u8_calc_checksum += pu8_hex_buf[u8_idx + 1];
   }
   u8_calc_checksum ^= 0xFF;
   u8_calc_checksum += 0x01;
   if (u8_calc_checksum != pu8_hex_buf[u8_data_len + HEX_DATA_OFFSET])
   {
      return (HEX_RECORD_BAD);
   }

   /* Get data length. */
   pstru_hex_record->u8_data_len = u8_data_len;

   /* Get address. */
   pstru_hex_record->u16_address = ((uint16_t)(*(pu8_hex_buf + HEX_HIGH_ADDRESS_OFFSET)) << 8) +
                                   (*(pu8_hex_buf + HEX_LOW_ADDRESS_OFFSET));

   /* Get record type. */
   pstru_hex_record->u8_type = *(pu8_hex_buf + HEX_RECORD_TYPE_OFFSET);

   /* Get data. */
   memcpy(pstru_hex_record->u8_data, pu8_hex_buf + HEX_DATA_OFFSET,
          pstru_hex_record->u8_data_len);

   return (HEX_RECORD_OK);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
