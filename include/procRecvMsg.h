#pragma once

#include "procUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>

// thread function for handling receiving messages 
void procRecvMsg(void*);

int proSendMsg(char* _pchServerIP, char* _pchServerPort, chordHeaderType* _ptMsgHead, int _nDataLen);

