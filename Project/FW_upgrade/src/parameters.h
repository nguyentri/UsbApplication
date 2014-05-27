/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : parameters.h
**   Project     : IMT - CANOpen Voltage monitor - Main App.
**   Author      : Nguyen Anh Huy
**   Version     : 1.0.1
**   Date        : 2012/8/20.
**   Description : Header file of Parameters module.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__


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

/*
** Total Number of parameters. Each parameter is 4 bytes.
** Caution: This value must be identical on the bootloader and application.
** Different values shall lead to data corruption.
*/

/* The number of pramaters.*/
#define PARAM_ITEM_NUMBER                      NB_OF_VAR

/*
** parameter address. 
*/
#define PARAM_ID_CHECKSUM                      0
#define PARAM_ID_BOOT_LOADER_VERSION           1
#define PARAM_ID_MAIN_APP_VERSION              2
/* Parameter firmware update flag address. */
#define PARAM_ID_FW_UPDATE_FLAG                3

/* Set to 1 to force parameters update. */
#define PARAM_FW_UPDATE_FLAG_NONE					 0

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Initialize the Parameter module. */
void v_PARAM_Init (void);

/* Get parameter value. */
uint32_t u32_PARAM_Get_Value (uint32_t u32_param_id);

/* Set parameter value. */
void v_PARAM_Set_Value (uint32_t u32_param_id, uint32_t u32_param_value);


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
