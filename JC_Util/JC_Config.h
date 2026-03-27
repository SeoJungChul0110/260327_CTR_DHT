#pragma once

#define JC_CONFIG_DEBUG 1

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#ifndef WIN32
#include		<ctype.h>
#endif
#define		MAX_CFG_LINE	256
#define		EXPONENT		16


int	ReadConfig(char * _pchFileName, char * _pchSectorName, char * _pchValue);
char*   SkipWhite(char * ptr);
char*	SkipNotWhite(char *ptr);
int	FileCheck(char *_pchFileName, char *_pchSectorName, char *_pchValue);

int	ReadDataByLine(char *_pchInData, int _nInDataLen, char *_pchOutValue, char * _pSearchStr);



char* ConcatString(int _nStrCnt, ...); // count: 연결할 문자열 개수