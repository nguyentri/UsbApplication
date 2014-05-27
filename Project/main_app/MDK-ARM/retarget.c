/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : retarget.c
**   Project     : IMT - MainCPU - Main Application
**   Author      : Nguyen Anh Huy
**   Revision    : 1.0.0.1
**   Date        : 2012/02/24.
**   Description : Retarget file I/O to UART port.
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

/* Application includes. */
#include "stm32f4xx.h"
#include "data_types.h"
#include "retarget.h"

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   DEFINE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** Definition for Retarget COM2, connected to USART2.
*/ 
#define RETARGET_COM2                            USART2
#define RETARGET_COM2_CLK                        RCC_APB1Periph_USART2
#define RETARGET_COM2_TX_PIN                     GPIO_Pin_2
#define RETARGET_COM2_TX_GPIO_PORT               GPIOA
#define RETARGET_COM2_TX_GPIO_CLK                RCC_AHB1Periph_GPIOA
#define RETARGET_COM2_TX_SOURCE                  GPIO_PinSource2
#define RETARGET_COM2_TX_AF                      GPIO_AF_USART2
#define RETARGET_COM2_RX_PIN                     GPIO_Pin_3
#define RETARGET_COM2_RX_GPIO_PORT               GPIOA
#define RETARGET_COM2_RX_GPIO_CLK                RCC_AHB1Periph_GPIOA
#define RETARGET_COM2_RX_SOURCE                  GPIO_PinSource3
#define RETARGET_COM2_RX_AF                      GPIO_AF_USART2
#define RETARGET_COM2_IRQn                       USART2_IRQn

/*
** Definition for Retarget COM6, connected to USART6.
*/ 
#define RETARGET_COM6                            USART6
#define RETARGET_COM6_CLK                        RCC_APB2Periph_USART6
#define RETARGET_COM6_TX_PIN                     GPIO_Pin_6
#define RETARGET_COM6_TX_GPIO_PORT               GPIOC
#define RETARGET_COM6_TX_GPIO_CLK                RCC_AHB1Periph_GPIOC
#define RETARGET_COM6_TX_SOURCE                  GPIO_PinSource6
#define RETARGET_COM6_TX_AF                      GPIO_AF_USART6
#define RETARGET_COM6_RX_PIN                     GPIO_Pin_7
#define RETARGET_COM6_RX_GPIO_PORT               GPIOC
#define RETARGET_COM6_RX_GPIO_CLK                RCC_AHB1Periph_GPIOC
#define RETARGET_COM6_RX_SOURCE                  GPIO_PinSource7
#define RETARGET_COM6_RX_AF                      GPIO_AF_USART6
#define RETARGET_COM6_IRQn                       USART6_IRQn

/*
** Definition structure for Retarget buffer.
*/
typedef struct
{
   /* Data buffer. */
   uint8_t volatile   u8_data[RETARGET_BUFFER_LENGTH];
   /* Index to put data in the buffer. */
   uint32_t volatile  u32_in_idx;
   /* Index to get data out the buffer. */
   uint32_t volatile  u32_out_idx;
} RETARGET_BUFFER;

/*
** Definition structure for FILE (stdio)
*/
struct __FILE
{
   /* Handle of file. */
   int handle;

   /* UART Port. */
   ENM_RETARGET_UART_PORT enm_port;

   /* Pointer to RX Buffer. */
   RETARGET_BUFFER volatile *pstruc_rx_buf;

   /* Pointer to TX buffer. */
   RETARGET_BUFFER volatile *pstruc_tx_buf;
};

/*
** Macros to check if data buffer is empty or full.
*/
#define IS_RX_BUFFER_FULL(f)                 (u32_RETARGET_Is_Buffer_Full((f->pstruc_rx_buf)->u32_in_idx, (f->pstruc_rx_buf)->u32_out_idx, RETARGET_BUFFER_LENGTH))

#define IS_RX_BUFFER_EMPTY(f)                (u32_RETARGET_Is_Buffer_Empty((f->pstruc_rx_buf)->u32_in_idx, (f->pstruc_rx_buf)->u32_out_idx))

#define IS_TX_BUFFER_FULL(f)                 (u32_RETARGET_Is_Buffer_Full((f->pstruc_tx_buf)->u32_in_idx, (f->pstruc_tx_buf)->u32_out_idx, RETARGET_BUFFER_LENGTH))

#define IS_TX_BUFFER_EMPTY(f)                (u32_RETARGET_Is_Buffer_Empty((f->pstruc_tx_buf)->u32_in_idx, (f->pstruc_tx_buf)->u32_out_idx))


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/* Function to check if data buffer is full. */
static uint32_t u32_RETARGET_Is_Buffer_Full (volatile uint32_t u32_in_idx,
                                             volatile uint32_t u32_out_idx,
                                             uint32_t u32_size);

/* Function to check if data buffer is empty. */
static uint32_t u32_RETARGET_Is_Buffer_Empty (volatile uint32_t u32_in_idx,
                                              volatile uint32_t u32_out_idx);


static FILE retarget_uart_port[RETARGET_UART_PORT_NUM];
static RETARGET_BUFFER retarget_buf[RETARGET_UART_PORT_NUM * 2];

/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   VARIABLE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

/*
** Variables to configure UART port hardware.
*/
USART_TypeDef* RETARGET_COM_USART[RETARGET_UART_PORT_NUM] = {RETARGET_COM2, RETARGET_COM6}; 

GPIO_TypeDef* RETARGET_COM_TX_PORT[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_TX_GPIO_PORT, RETARGET_COM6_TX_GPIO_PORT};
 
GPIO_TypeDef* RETARGET_COM_RX_PORT[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_RX_GPIO_PORT, RETARGET_COM6_RX_GPIO_PORT};

const uint32_t RETARGET_COM_USART_CLK[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_CLK, RETARGET_COM6_CLK};

const uint32_t RETARGET_COM_TX_PORT_CLK[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_TX_GPIO_CLK, RETARGET_COM6_TX_GPIO_CLK};
 
const uint32_t RETARGET_COM_RX_PORT_CLK[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_RX_GPIO_CLK, RETARGET_COM6_RX_GPIO_CLK};

const uint16_t RETARGET_COM_TX_PIN[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_TX_PIN, RETARGET_COM6_TX_PIN};

const uint16_t RETARGET_COM_RX_PIN[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_RX_PIN, RETARGET_COM6_RX_PIN};
 
const uint8_t RETARGET_COM_TX_PIN_SOURCE[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_TX_SOURCE, RETARGET_COM6_TX_SOURCE};

const uint8_t RETARGET_COM_RX_PIN_SOURCE[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_RX_SOURCE, RETARGET_COM6_RX_SOURCE};

const uint8_t RETARGET_COM_TX_AF[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_TX_AF, RETARGET_COM6_TX_AF};
 
const uint8_t RETARGET_COM_RX_AF[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_RX_AF, RETARGET_COM6_RX_AF};

const uint8_t RETARGET_COM_IRQ[RETARGET_UART_PORT_NUM] = {RETARGET_COM2_IRQn, RETARGET_COM6_IRQn};

/*
** Modified stdout, stdin, stderror of stdio.h.
*/
FILE __stdout, __stdin, __stderror;


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   FUNCTION SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : FILE *RETARGET_Init (ENM_RETARGET_UART_PORT enm_port,
**                                         USART_InitTypeDef* USART_InitStruct,
**                                         uint32_t u32_retarget_stdio)
**
**   Arguments      :
**      enm_port             : UART port to output data.
**      USART_InitStruct     : Pointer to UART configuration structure.
**      u32_retarget_stdio   : 0 - Do not retarget stdout/stdin/stderror
**                             1 - Retarget stdout/stdin/stderror to this UART port
**
**   Return         : Pointer to FILE structure that has been initialized.
**
**   Description    : This function shall initialize a file structure to use UART port
**                    as data input/output. The UART port is also configured accordingly
**                    to the input settings (USART_InitStruct).
**                    Allow to retarget stdout/stdin/stderror.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy
**
**   Date           : 2012/02/24.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
FILE *RETARGET_Init (ENM_RETARGET_UART_PORT enm_port, USART_InitTypeDef* USART_InitStruct, uint32_t u32_retarget_stdio)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   /* Reset file structure related to this UART port. */
   memset((void *)&retarget_uart_port[enm_port], 0, sizeof(FILE));

   /* Initialize the retarget structure. */
   retarget_uart_port[enm_port].handle = 0;
   retarget_uart_port[enm_port].enm_port = enm_port;
   retarget_uart_port[enm_port].pstruc_rx_buf = &retarget_buf[2 * enm_port];
   retarget_uart_port[enm_port].pstruc_tx_buf = &retarget_buf[(2 * enm_port) + 1];

   /* Initialize stdout/stdin if requested. */
   if (u32_retarget_stdio != 0)
   {
      __stdin.handle = 0;
      __stdin.enm_port = enm_port;
      __stdin.pstruc_rx_buf = &retarget_buf[2 * enm_port];
      __stdin.pstruc_tx_buf = &retarget_buf[(2 * enm_port) + 1];
      __stdout.handle = 0;
      __stdout.enm_port = enm_port;
      __stdout.pstruc_rx_buf = &retarget_buf[2 * enm_port];
      __stdout.pstruc_tx_buf = &retarget_buf[(2 * enm_port) + 1];
   }

   /* Enable UART clock */
   if (enm_port == RETARGET_UART_PORT_2)
   {
      RCC_APB1PeriphClockCmd(RETARGET_COM_USART_CLK[enm_port], ENABLE);
   }
   else if (enm_port == RETARGET_UART_PORT_6)
   {
      RCC_APB2PeriphClockCmd(RETARGET_COM_USART_CLK[enm_port], ENABLE);
   }

   /* Enable GPIO clock */
   RCC_AHB1PeriphClockCmd(RETARGET_COM_TX_PORT_CLK[enm_port] | RETARGET_COM_RX_PORT_CLK[enm_port], ENABLE);

   /* Connect PXx to USARTx_Tx*/
   GPIO_PinAFConfig(RETARGET_COM_TX_PORT[enm_port], RETARGET_COM_TX_PIN_SOURCE[enm_port], RETARGET_COM_TX_AF[enm_port]);

   /* Connect PXx to USARTx_Rx*/
   GPIO_PinAFConfig(RETARGET_COM_RX_PORT[enm_port], RETARGET_COM_RX_PIN_SOURCE[enm_port], RETARGET_COM_RX_AF[enm_port]);

   /* Configure USART Tx as alternate function  */
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

   GPIO_InitStructure.GPIO_Pin = RETARGET_COM_TX_PIN[enm_port];
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(RETARGET_COM_TX_PORT[enm_port], &GPIO_InitStructure);

   /* Configure USART Rx as alternate function  */
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Pin = RETARGET_COM_RX_PIN[enm_port];
   GPIO_Init(RETARGET_COM_RX_PORT[enm_port], &GPIO_InitStructure);

   /* USART configuration */
   USART_Init(RETARGET_COM_USART[enm_port], USART_InitStruct);

   /* Enable the USARTx Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = RETARGET_COM_IRQ[enm_port];
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
 
   /* Clear TX and RX Interrupt. */
   USART_ClearITPendingBit(RETARGET_COM_USART[enm_port], USART_IT_TXE| USART_IT_RXNE);
   /* Enable RX interrupt to begin data receiving. */
   USART_ITConfig(RETARGET_COM_USART[enm_port], USART_IT_RXNE, ENABLE);
      
   /* Enable USART */
   USART_Cmd(RETARGET_COM_USART[enm_port], ENABLE);

   /* Return pointer to the retarget structure. */
   return (&retarget_uart_port[enm_port]);
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int fputc (int ch, FILE *f)
**
**   Arguments      :
**      ch          : character to output.
**      f           : pointer to retarget file structure.
**
**   Return         : Sent character if successful or EOF if error occurs.
**
**   Description    : This function put the output character to TX buffer of the
**                    FILE structure.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy.
**
**   Date           : 2012/02/24.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int fputc (int ch, FILE *f)
{
   USART_TypeDef *pstru_uart_port;

   switch (f->enm_port)
   {
      case RETARGET_UART_PORT_2:
         pstru_uart_port = USART2;
      break;

      case RETARGET_UART_PORT_6:
         pstru_uart_port = USART6;
      break;

      default:
      return (EOF);
   }

   /* If TX Buffer is full. */
   if (IS_TX_BUFFER_FULL(f))
   {
      return (EOF);
   }

   /* Put character to TX data buffer. */
   (f->pstruc_tx_buf)->u8_data[(f->pstruc_tx_buf)->u32_in_idx] = (uint8_t)ch;
   (f->pstruc_tx_buf)->u32_in_idx++;
   if ((f->pstruc_tx_buf)->u32_in_idx >= RETARGET_BUFFER_LENGTH)
   {
      (f->pstruc_tx_buf)->u32_in_idx = 0;
   }

   /* Enable USART TX Interrupt. */
   USART_ITConfig(pstru_uart_port, USART_IT_TXE, ENABLE);

   return ch;
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int fgetc (FILE *f)
**
**   Arguments      :
**      f           : Pointer to retarget file structure to read data.
**
**   Return         : character read from the retarget file structure (USART) if
**                    successful or EOF if error occurs.
**
**   Description    : This function read one character from the retarget USART port.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy.
**
**   Date           : 2012/02/24.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int fgetc (FILE *f)
{
   USART_TypeDef *pstru_uart_port;
   uint8_t u8_char;

   switch (f->enm_port)
   {
      case RETARGET_UART_PORT_2:
         pstru_uart_port = USART2;
      break;

      case RETARGET_UART_PORT_6:
         pstru_uart_port = USART6;
      break;

      default:
      return (EOF);
   }

   /* If RX buffer is empty. */
   if (IS_RX_BUFFER_EMPTY(f))
   {
      return (EOF);
   }

   /* Read character from RX buffer. */
   u8_char = (f->pstruc_rx_buf)->u8_data[(f->pstruc_rx_buf)->u32_out_idx];
   (f->pstruc_rx_buf)->u32_out_idx++;
   if ((f->pstruc_rx_buf)->u32_out_idx >= RETARGET_BUFFER_LENGTH)
   {
      (f->pstruc_rx_buf)->u32_out_idx = 0;
   }

   /* Enable RX Interrupt. */
   USART_ITConfig(pstru_uart_port, USART_IT_RXNE, ENABLE);

   return (u8_char);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : int32_t fpeek (FILE *f, char ch)
**
**   Arguments      :
**      f    -   Pointer to retarget file structure to read data.
**      ch   -   Character to detect.
**
**   Return         :
**      -1   -   Can't find character in buffer.
**      >=0  -   Position of character found in buffer.
**
**   Description    : This function find a character in UART buffer
**                    and return its position.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy.
**
**   Date           : 2012/02/24.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int32_t fpeek (FILE *f, char ch)
{
   uint32_t u32_index;

   u32_index = (f->pstruc_rx_buf)->u32_out_idx;
   while (u32_index != (f->pstruc_rx_buf)->u32_in_idx)
   {
      if ((f->pstruc_rx_buf)->u8_data[u32_index] == ch)
      {
         return (u32_index);
      }
      u32_index++;
      if (u32_index >= RETARGET_BUFFER_LENGTH)
      {
         u32_index = 0;
      }
   }

   /* Can't find character in buffer. */
   return (-1);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : void RETARGET_IRQHANDLER (ENM_RETARGET_UART_PORT enm_port)
**
**   Arguments      :
**      enm_port    : USART port to handle interrupt.
**
**   Return         : n/a
**
**   Description    : Function to handle interrupt service of USART port.
**
**   Notes          : called from IRQ of USART.
**
**   Author         : Nguyen Anh Huy.
**
**   Date           : 2012/02/24.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void RETARGET_IRQHANDLER (ENM_RETARGET_UART_PORT enm_port)
{
   USART_TypeDef *pstru_uart_port;
   FILE *f;

   switch (enm_port)
   {
      case RETARGET_UART_PORT_2:
         pstru_uart_port = USART2;
         f = &retarget_uart_port[enm_port];
      break;

      case RETARGET_UART_PORT_6:
         pstru_uart_port = USART6;
         f = &retarget_uart_port[enm_port];
      break;
   }   

   /* RX Interrupt. */
  if(USART_GetITStatus(pstru_uart_port, USART_IT_RXNE) != RESET)
  {
      /* Clear RX Interrupt. */
      USART_ClearITPendingBit(pstru_uart_port, USART_IT_RXNE);

      /* Read one byte from the receive data register */
      (f->pstruc_rx_buf)->u8_data[(f->pstruc_rx_buf)->u32_in_idx] = USART_ReceiveData(pstru_uart_port);

      (f->pstruc_rx_buf)->u32_in_idx++;
      /* If we reach the end of RX buffer, return to the first. */
      if ((f->pstruc_rx_buf)->u32_in_idx >= RETARGET_BUFFER_LENGTH)
      {
         (f->pstruc_rx_buf)->u32_in_idx = 0;
      }

      /* RX Buffer is full. */
      if (IS_RX_BUFFER_FULL(f))
      {
         /* Disable RX interrupt. */
         USART_ITConfig(pstru_uart_port, USART_IT_RXNE, DISABLE);
      }
   }

   /* TX Interrupt. */
   if(USART_GetITStatus(pstru_uart_port, USART_IT_TXE) != RESET)
   {
      /* Clear TX Interrupt. */
      USART_ClearITPendingBit(pstru_uart_port, USART_IT_TXE);

      /* Write one byte to the transmit data register */
      USART_SendData(pstru_uart_port, (f->pstruc_tx_buf)->u8_data[(f->pstruc_tx_buf)->u32_out_idx]);

      /* Increase TX buffer out index to the next byte to send. */
      (f->pstruc_tx_buf)->u32_out_idx++;
      /* If we reach the end of TX buffer, return to the first. */
      if ((f->pstruc_tx_buf)->u32_out_idx >= RETARGET_BUFFER_LENGTH)
      {
         (f->pstruc_tx_buf)->u32_out_idx = 0;
      }

      /* We have transfer all data in buffer. */
      if (IS_TX_BUFFER_EMPTY(f))
      {
         /* Disable TX interrupt. */
         USART_ITConfig(pstru_uart_port, USART_IT_TXE, DISABLE);
      }
   }
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : static uint32_t u32_RETARGET_Is_Buffer_Full (volatile uint32_t u32_in_idx,
**                                                                 volatile uint32_t u32_out_idx,
**                                                                 uint32_t u32_size)
**
**   Arguments      :
**      u32_in_idx  : Index to put data in the buffer.
**      u32_out_idx : Index to get data out the buffer.
**      u32_size    : Buffer size.
**
**   Return         : 1 - if buffer is full.
**                    0 - if buffer is not full.
**
**   Description    : Determine whether the buffer is full or not.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy.
**
**   Date           : 2012/02/24.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static uint32_t u32_RETARGET_Is_Buffer_Full (volatile uint32_t u32_in_idx,
                                             volatile uint32_t u32_out_idx,
                                             uint32_t u32_size)
{
    return((((u32_in_idx + 1) % u32_size) == u32_out_idx) ? 1 : 0);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   Function       : static uint32_t u32_RETARGET_Is_Buffer_Empty (volatile uint32_t u32_in_idx,
**                                                                  volatile uint32_t u32_out_idx,
**                                                                  uint32_t u32_size)
**
**   Arguments      :
**      u32_in_idx  : Index to put data in the buffer.
**      u32_out_idx : Index to get data out the buffer.
**      u32_size    : Buffer size.
**
**   Return         : 1 - if buffer is full.
**                    0 - if buffer is not full.
**
**   Description    : Determine whether the buffer is empty or not.
**
**   Notes          : restrictions, odd modes
**
**   Author         : Nguyen Anh Huy.
**
**   Date           : 2012/02/24.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
static uint32_t u32_RETARGET_Is_Buffer_Empty (volatile uint32_t u32_in_idx,
                                              volatile uint32_t u32_out_idx)
{
    return((u32_in_idx  == u32_out_idx) ? 1 : 0);
}


/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
