#pragma once

#include "DefStruct.h"
#include "UtilFun.h"
#include "PrintFunc.h"
#pragma comment(lib,"ws2_32")

void stabilizeLeave(SOCKET socket, int leaveID);
void stabilizeJoin(SOCKET socket);