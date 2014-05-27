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
**   Description : Functions to help with processing command lines.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <string.h>
#include <stdint.h>
#include "cmdline.h"

//*****************************************************************************
//
// Defines the maximum number of arguments that can be parsed.
//
//*****************************************************************************
#define MAX_ARGS                8

//*****************************************************************************
//
//! Process a command line string into arguments and execute the command.
//!
//! \param pcCmdLine points to a string that contains a command line that was
//! obtained by an application by some means.
//!
//! This function will take the supplied command line string and break it up
//! into individual arguments.  The first argument is treated as a command and
//! is searched for in the command table.  If the command is found, then the
//! command function is called and all of the command line arguments are passed
//! in the normal argc, argv form.
//!
//! The command table is contained in an array named <tt>g_sCmdTable</tt> which
//! must be provided by the application.
//!
//! \return Returns \b CMDLINE_BAD_CMD if the command is not found,
//! \b CMDLINE_TOO_MANY_ARGS if there are more arguments than can be parsed.
//! Otherwise it returns the code that was returned by the command function.
//
//*****************************************************************************
int32_t s32_Cmd_Line_Process (char *pstri_cmd_line)
{
    static char *pstri_argv[10];
    char *pc_char;
    int32_t s32_argc;
    int32_t s32_find_arg = 1;
    CMD_LINE_ENTRY_T *pCmdEntry;

    //
    // Initialize the argument counter, and point to the beginning of the
    // command line string.
    //
    s32_argc = 0;
    pc_char = pstri_cmd_line;

    //
    // Advance through the command line until a zero character is found.
    //
    while(*pc_char)
    {
        //
        // If there is a space, then replace it with a zero, and set the flag
        // to search for the next argument.
        //
        if(*pc_char == ' ')
        {
            *pc_char = 0;
            s32_find_arg = 1;
        }

        //
        // Otherwise it is not a space, so it must be a character that is part
        // of an argument.
        //
        else
        {
            //
            // If s32_find_arg is set, then that means we are looking for the start
            // of the next argument.
            //
            if(s32_find_arg)
            {
                //
                // As long as the maximum number of arguments has not been
                // reached, then save the pointer to the start of this new arg
                // in the pstri_argv array, and increment the count of args, s32_argc.
                //
                if(s32_argc < MAX_ARGS)
                {
                    pstri_argv[s32_argc] = pc_char;
                    s32_argc++;
                    s32_find_arg = 0;
                }

                //
                // The maximum number of arguments has been reached so return
                // the error.
                //
                else
                {
                    return(CMDLINE_TOO_MANY_ARGS);
                }
            }
        }

        //
        // Advance to the next character in the command line.
        //
        pc_char++;
    }

    //
    // If one or more arguments was found, then process the command.
    //
    if(s32_argc)
    {
        //
        // Start at the beginning of the command table, to look for a matching
        // command.
        //
        pCmdEntry = &astru_cmd_table[0];

        //
        // Search through the command table until a null command string is
        // found, which marks the end of the table.
        //
        while(pCmdEntry->pstri_cmd)
        {
            //
            // If this command entry command string matches pstri_argv[0], then call
            // the function for this command, passing the command line
            // arguments.
            //
            if(!strcmp(pstri_argv[0], pCmdEntry->pstri_cmd))
            {
                return(pCmdEntry->pfn_cmd(s32_argc, pstri_argv));
            }

            //
            // Not found, so advance to the next entry.
            //
            pCmdEntry++;
        }
    }

    //
    // Fall through to here means that no matching command was found, so return
    // an error.
    //
    return(CMDLINE_BAD_CMD);
}

