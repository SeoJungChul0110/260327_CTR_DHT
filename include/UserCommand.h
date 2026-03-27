#pragma once

#include "DefStruct.h"
#include "UtilFun.h"
#include "procUtil.h"
#include "procPingAndFixFinger.h"
#include "procRecvMsg.h"

// For showing commands
void showCommand(void);

// 사용자 입력 명령어 확인
char getUserCommand(void);

//  User input processing (switch) 
void procUserCommand(char _chCmdChar);

