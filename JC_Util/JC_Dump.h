/**************************************************
File Name	: 
Description	: 
Histroy		:

Copyright (c) 2021
Create by jungchul seo 
***************************************************/

#ifndef __JC_DUMP_H__
#define __JC_DUMP_H__

#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
//#include <unistd.h>
//#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif 

void JC_DumpMsg(unsigned char * _pbyBuff, int _nLen);
void JC_DumpMsgLog(char * _pchTitleBuf, unsigned char *_pbyBuff, int _nLen);
void JC_PrintBuf(unsigned char *_pbyBuff, int _nLen);
void JC_PrintBufLog(char * _pchTitleBuf, unsigned char *_pbyBuff, int _nLen);
void JC_PrintChar(unsigned char c);

#ifdef __cplusplus
}
#endif


#endif // __JC_DUMP_H__
