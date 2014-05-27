/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   File Name   : data_types.h
**   Project     : IMT - Main CPU - Main application.
**   Author      : Nguyen Anh Huy.
**   Version     : 1.0.0.1
**   Date        : 2012/01/18.
**   Description : Header file content all data types used in the project.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__


typedef signed char           S8;
typedef unsigned char         U8;
typedef short                 S16;
typedef unsigned short        U16;
typedef long                  S32;
typedef unsigned long         U32;
typedef long long             S64;
typedef unsigned long long    U64;
//typedef unsigned int          BOOL;

#ifndef __TRUE
   #define __TRUE             1
#endif

#ifndef __FALSE
   #define __FALSE            0
#endif

#ifndef NULL
   #ifdef __cplusplus // EC++
      #define NULL            0
   #else
      #define NULL            ((void *)0)
   #endif
#endif


#if defined(__BIG_ENDIAN) && !defined(__LITTLE_ENDIAN)

#define htons(A) (A)
#define htonl(A) (A)
#define ntohs(A) (A)
#define ntohl(A) (A)

#elif defined(__LITTLE_ENDIAN) && !defined(__BIG_ENDIAN)

#define htons(A) ((((U16)(A) & 0xff00) >> 8) | \
                  (((U16)(A) & 0x00ff) << 8))

#define htonl(A) ((((U32)(A) & 0xff000000) >> 24) | \
                  (((U32)(A) & 0x00ff0000) >> 8) | \
                  (((U32)(A) & 0x0000ff00) << 8) | \
                  (((U32)(A) & 0x000000ff) << 24))

#define ntohs htons

#define ntohl htonl

#else

//#error "Either __BIG_ENDIAN or __LITTLE_ENDIAN must be #defined, but not both."

#endif



#endif

/* 
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   END
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
