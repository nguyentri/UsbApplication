/*
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**© Copyright Camlin Technologies Limited, 2012. All rights reserved.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**
**   File Name   : header_template.h
**   Project     : IMT
**   Author      : Nguyen Anh Huy
**   Revision    :
**   Date        :
**   Description : This is a template for header file.
**
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef __ASSEM_TEST_INFO_H__
#define __ASSEM_TEST_INFO_H__


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
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
**   PROTOTYPE SECTION
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

extern int32_t s32_ATI_Flash_Type_Set (FILE *pf_cli, char *pstri_flash_type);

extern int32_t s32_ATI_Flash_Type_Get (FILE *pf_cli);

extern int32_t s32_ATI_Xtal_Pulling_Set (FILE *pf_cli, char *pstri_xtal_pulling);

extern int32_t s32_ATI_Xtal_Pulling_Get (FILE *pf_cli);

extern int32_t s32_ATI_Assem_Company_Set (FILE *pf_cli, char *pstri_assem_company);

extern int32_t s32_ATI_Assem_Company_Get (FILE *pf_cli);

extern int32_t s32_ATI_Assem_Tester_Set (FILE *pf_cli, char *pstri_assem_tester);

extern int32_t s32_ATI_Assem_Tester_Get (FILE *pf_cli);

extern int32_t s32_ATI_Assem_Test_Time_Set (FILE *pf_cli, char *pstri_assem_test_time);

extern int32_t s32_ATI_Assem_Test_Time_Get (FILE *pf_cli);

extern int32_t s32_ATI_Final_Tester_Set (FILE *pf_cli, char *pstri_final_tester);

extern int32_t s32_ATI_Final_Tester_Get (FILE *pf_cli);

extern int32_t s32_ATI_Final_Test_Time_Set (FILE *pf_cli, char *pstri_final_test_time);

extern int32_t s32_ATI_Final_Test_Time_Get (FILE *pf_cli);

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
