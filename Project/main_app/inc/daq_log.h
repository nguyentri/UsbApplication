/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : daq_log.h
**   Project     : USB DAQ 
**   Author      : Nguyen Trong Tri
**   Version     : 1.0
**   Date        : 2014/03/14
**   Description : Header file of daq_log.c
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

void v_LOG_Task (void);
int32_t s32_Put_Meas_To_Log_File(void);
int32_t s32_Delete_Log_File(char* psDelLogFname);
int32_t s32_Create_New_File(void);
