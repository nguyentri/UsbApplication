/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : retarget.h
**   Project     : IMT - MainCPU - Main Application
**   Author      : Nguyen Anh Huy
**   Revision    : 1.0.0.1
**   Date        : 2012/02/24.
**   Description : Header file for retarget module.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef __RETARGET_H__
#define __RETARGET_H__


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

/* UART port to perform function. */
typedef enum
{
   RETARGET_UART_PORT_2 = 0,
   RETARGET_UART_PORT_6 = 1,
   RETARGET_UART_PORT_NUM
} ENM_RETARGET_UART_PORT;

/* Buffer max length of retarget. */
#define RETARGET_BUFFER_LENGTH                   2048


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Function to initialize the retarget USART port. */
extern FILE *RETARGET_Init (ENM_RETARGET_UART_PORT enm_port, USART_InitTypeDef* USART_InitStruct, uint32_t u32_retarget_stdio);

/* Interrupt service function for USART port. */
extern void RETARGET_IRQHANDLER (ENM_RETARGET_UART_PORT enm_port);

extern int32_t fpeek (FILE *f, char ch);


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
