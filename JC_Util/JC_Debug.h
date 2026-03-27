/**************************************************
File Name	: 
Description	: 
Histroy		:

Copyright (c) 2021
Create by jungchul seo 
***************************************************/
#ifndef __JC_DEBUG_H__
#define __JC_DEBUG_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>

#define JC_DEBUG_BUF_SIZE	1024
#define JC_DEBUG_ENABLE 1

#define OUTER_DELIMETER "##################################" // ############################
#define INNER_DELIMETER "----------------------------------" // ----------------------------
#define INNER_SEPARATOR "==================================" //============================

typedef enum {
	NONE_MODE = 0,
	ERROR_MODE = 1,
	WARN_MODE = 3,
	INFO_MODE = 4,
	DEBUG_MODE = 5,
} PRINT_TYPE;

const char* GetFileName(const char* path);
//src/main.cpp : main at 51
#define JC_FUNC(loglevel, s) 							\
        if(s =="" || strlen(s) == 0) {                                           \
			JC_Print(DEBUG_MODE, "[# FUNC] %s > %s at %d ",__FUNCTION__,GetFileName(__FILE__),__LINE__); \
        } else { \
            JC_Print(DEBUG_MODE, "[# FUNC] %s > %s at %d [%s]",__FUNCTION__,GetFileName(__FILE__),__LINE__, s); \
        }    


#ifdef __cplusplus
extern "C" {
#endif 
    

void SetDebugLogLevel(int _nLevel);
void OpenDebugLogFile(char* _pcLogFile);
static void WriteLog(char* _pcWriteMsg);
void CloseLogFile();

#define JC_LOGO(loglevel, s) 							\
	 JC_Print(loglevel, "%s",INNER_SEPARATOR); 		\
	 JC_Print(loglevel, "----   %s   ----\n",s); \
	 JC_Print(loglevel, INNER_DELIMETER);

#define	JC_Print(level, ...)	JC_DBGPrint(level, __VA_ARGS__);
void JC_DBGPrint(int level, const char *fmt, ...);


void OpenResultLogFile(char* _pcLogFile);
void WriteResultLog(const char* fmt, ...);
void CloseResultLogFile();
static void WritenResult(char* _pcWriteMsg);
#define	JC_RESULT(...)	// WriteResultLog( __VA_ARGS__);


void  ShareWriteResultLog(char* _pcLogFile, const char* fmt, ...);
void ShareWritenResult(char* _pcLogFile, char* _pcWriteMsg);


#ifdef __cplusplus
}
#endif

#endif


