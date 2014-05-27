/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : parameters.c
**   Project     : IMT - CANOpen Voltage monitor - Main App.
**   Author      : Nguyen Anh Huy
**   Version     : 1.0.1
**   Date        : 2012/8/20.
**   Description : Parameters module to keep all device's settings.
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
#include "eeprom.h"
//#include "version.h"
#include "data_types.h"
#include "parameters.h"


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** Flash size used for Parameters Block.
*/
#define PARAM_FLASH_SIZE                   0x8000

/* Default device Main App version. */
#define PARAM_MAIN_APP_VERSION_DEFAULT     (((((uint32_t)10000) + \
                                               ((uint32_t)1 * 100) + \
                                               ((uint32_t)1)) << 16) + \
                                             (uint16_t)(1))


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Function to reset all parameters to default values. */
static void v_PARAM_Reset (void);

/* Get parameters block. */
static uint32_t u32_Get_PARAM (uint32_t u32_PARAM_item_num, uint32_t *pu32_buff);

/* Save parameters block. */
static uint32_t u32_Save_PARAM (uint32_t u32_PARAM_item_num, uint32_t *pu32_buff);

/* Initialize virtural address. */
static void v_Init_VirtAdd (void);

/* Local function to force parameter update.
** Useful for development if need to change some parameters in FRAM. */
#if (PARAM_FORCE_CHANGE == 1)
static void v_PARAM_Force_Update (void);
#endif


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Array of parameters. */
static uint32_t au32_PARAM[PARAM_ITEM_NUMBER];

/* Virtual address of parameters. */
uint32_t  VirtAddVarTab[PARAM_ITEM_NUMBER];

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_Param_Init (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Initialize the Parameters module.
**   Notes       : Must be called prior to other Parameters module functions.
**   Author      : Nguyen Anh Huy.
**   Date        : 2012/8/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_PARAM_Init (void)
{
   /* Read emulate eprom status. */
   uint32_t ReadStatus = 1;

   /* Unlock the Flash Program Erase controller. */
   FLASH_Unlock();

   /* EEPROM Init. */
   EE_Init();

   /* Initilize virtual address. */
   v_Init_VirtAdd();

    /*
   ** Find the last valid parameters block.
   */
   ReadStatus = u32_Get_PARAM(PARAM_ITEM_NUMBER, au32_PARAM);

   if (au32_PARAM[PARAM_ID_MAIN_APP_VERSION] != PARAM_MAIN_APP_VERSION_DEFAULT)
   {
      /* Update Main Application version. */
      au32_PARAM[PARAM_ID_MAIN_APP_VERSION] = PARAM_MAIN_APP_VERSION_DEFAULT;
      /* Save to virtual eeprom. */
      EE_WriteVariable(PARAM_ID_MAIN_APP_VERSION, PARAM_MAIN_APP_VERSION_DEFAULT);
   }

   /* Check all parameters exit. */
   if(ReadStatus == 0)
   {
      /*
      ** Update main application based on the version number (high short).
      ** We don't care about revision number in low short.
      */
      switch (au32_PARAM[PARAM_ID_MAIN_APP_VERSION] >> 16)
      {
         /*
         ** In some cases (such as FRAM), erase the storage shall set all data to 0.
         ** This is considered as invalid version value.
         ** We shall need to reset all parameters.
         */
         case 0:
            /* Reset all parameters. */
            v_PARAM_Reset();
         break;

         /* We are at the current version. */
         case (PARAM_MAIN_APP_VERSION_DEFAULT >> 16):
            /* Enable this function to force the Parameters to be updated. */
         break;

         /* Unrecognized version value. */
         default:
            /* Set version to the current version. */
            v_PARAM_Set_Value(PARAM_ID_MAIN_APP_VERSION, PARAM_MAIN_APP_VERSION_DEFAULT);
         break;
      }
   }
   else
   {
      /*
      ** Can't find a valid parameters block.
      ** Reset all values to default.
      */
      v_PARAM_Reset();
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : uint32_t u32_Param_Get_Value (uint32_t u32_param_id)
**   Arguments   :
**      (in) u8_param_id   -   Parameter ID to read value.
**   Return      : parameter value.
**   Description : Read parameter value based on the ID.
**   Notes       :
**   Author      : Nguyen Anh Huy.
**   Date        : 2012/8/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
uint32_t u32_PARAM_Get_Value (uint32_t u32_param_id)
{
   return (au32_PARAM[u32_param_id]);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_Param_Set_Value (uint32_t u32_param_id, uint32_t u32_param_value)
**   Arguments   :
**      (in) u8_param_id       -   Parameter ID to read value.
**      (in) u32_param_value   -   New parameter value.
**   Return      : n/a
**   Description : Set parameter value based on the ID.
**   Notes       :
**   Author      : Nguyen Anh Huy.
**   Date        : 2012/8/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_PARAM_Set_Value (uint32_t u32_param_id, uint32_t u32_param_value)
{
   /* Set parameter value. */
   au32_PARAM[u32_param_id] = u32_param_value;
   /* Save Parameter to eeprom. */
   EE_WriteVariable(u32_param_id, u32_param_value);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : static void v_Param_Reset (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Reset the parameters to default values.
**   Notes       :
**   Author      : Nguyen Anh Huy.
**   Date        : 2012/8/20.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static void v_PARAM_Reset (void)
{
   /* Set parameters to canopen default values. */
   au32_PARAM[PARAM_ID_MAIN_APP_VERSION] = PARAM_MAIN_APP_VERSION_DEFAULT;
   
   /* Set defsault value for force fw update. */
   au32_PARAM[PARAM_ID_FW_UPDATE_FLAG] = PARAM_FW_UPDATE_FLAG_NONE;

   /* Save to Parameters Block. */
   u32_Save_PARAM(PARAM_ITEM_NUMBER, au32_PARAM);
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : uint32_t u32_Get_PARAM(uint32_t u32_PARAM_item_num, uint32_t *pu32_buff)
**   Arguments   : 
**       u32_PARAM_item_num - number of parameter item.
**       pu32_buff -  pointer to parameter buffer.
**   Return      : 
**       0 - All prarameters exist.
**       1 - There are some paramater not exist.
**       2 - no valid eeprom page.
**   Description : Get all parameters to parameter buffer.
**   Notes       :
**   Author      : Nguyen Trong Tri.
**   Date        : 2012/11/14.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
uint32_t u32_Get_PARAM(uint32_t u32_PARAM_item_num, uint32_t *pu32_buff)
{
   uint32_t ReadStatus = 1;
   uint32_t u32_var_id;

   for (u32_var_id = 0; u32_var_id < u32_PARAM_item_num; u32_var_id++)
   {
      ReadStatus = EE_ReadVariable(u32_var_id, (uint32_t*) pu32_buff);

      /* no valid page */
      if (ReadStatus == NO_VALID_PAGE)
      {
         return (2);
      }
      /* variable doesn't exist */
      else if (ReadStatus == 1)
      {
         return (1);
      }
      /* variable exist */
      else
      {
         pu32_buff++;
      }
   }
   return (0);
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : uint32_t u32_Save_PARAM (uint32_t *pu32_buff)
**   Arguments   : 
**       u32_PARAM_item_num - number of parameter item.
**       pu32_buff -  pointer to parameter buffer.
**   Return      : n/a
**   Description : Save all parameters to virtual eeprom.
**   Notes       :
**   Author      : Nguyen Trong Tri.
**   Date        : 2012/11/14.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
uint32_t u32_Save_PARAM (uint32_t u32_PARAM_item_num, uint32_t *pu32_buff)
{
   uint32_t WriteStatus = 1;
   uint32_t u32_var_id;

   /* Write all parameter to eeprom. */
   for (u32_var_id = 0; u32_var_id < u32_PARAM_item_num; u32_var_id++)
   {
      WriteStatus = EE_WriteVariable(u32_var_id, *pu32_buff);
      pu32_buff++;
   }
   return WriteStatus;
}

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function    : void v_Init_VirtAdd (void)
**   Arguments   : n/a
**   Return      : n/a
**   Description : Initialize virtual address of parameters.
**   Notes       :
**   Author      : Nguyen Trong Tri.
**   Date        : 2012/11/14.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void v_Init_VirtAdd (void)
{
   uint32_t u32_var_id;

   /* Address from 0 to PARAM_ITEM_NUMBER.  */
   for (u32_var_id = 0; u32_var_id < PARAM_ITEM_NUMBER; u32_var_id++)
   {
      VirtAddVarTab[u32_var_id] = u32_var_id;
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
