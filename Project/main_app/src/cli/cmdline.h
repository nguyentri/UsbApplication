/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2013. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : cmdline.c
**   Project     : IMT - Main CPU - Main Application.
**   Author      : Nguyen Anh Huy
**   Revision    : 1.0.0.1
**   Date        : 2013/05/20.
**   Description : Prototypes for command line processing functions.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef __CMDLINE_H__
#define __CMDLINE_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup cmdline_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Defines the value that is returned if the command is not found.
//
//*****************************************************************************
#define CMDLINE_OK              (0)

//*****************************************************************************
//
//! Defines the value that is returned if the command is not found.
//
//*****************************************************************************
#define CMDLINE_BAD_CMD         (-1)

//*****************************************************************************
//
//! Defines the value that is returned if there are too many arguments.
//
//*****************************************************************************
#define CMDLINE_TOO_MANY_ARGS   (-2)

//*****************************************************************************
//
//! Defines the value that is returned if the argument is invalid.
//
//*****************************************************************************
#define CMDLINE_INVALID_ARGS    (-3)


//*****************************************************************************
//
// Command line function callback type.
//
//*****************************************************************************
typedef int (*pfn_cmd_line)(int argc, char *argv[]);

//*****************************************************************************
//
//! Structure for an entry in the command list table.
//
//*****************************************************************************
typedef struct
{
    //
    //! A pointer to a string containing the name of the command.
    //
    const char *pstri_cmd;

    //
    //! A function pointer to the implementation of the command.
    //
    pfn_cmd_line pfn_cmd;

    //
    //! A pointer to a string of brief help text for the command.
    //
    const char *pstri_help;
}
CMD_LINE_ENTRY_T;

//*****************************************************************************
//
//! This is the command table that must be provided by the application.
//
//*****************************************************************************
extern CMD_LINE_ENTRY_T astru_cmd_table[];

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern int32_t s32_Cmd_Line_Process (char *pstri_cmd_line);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __CMDLINE_H__
