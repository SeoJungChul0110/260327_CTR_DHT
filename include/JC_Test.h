#pragma once

#include "DefStruct.h"
#include "UtilFun.h"
#include "PrintFunc.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int IsDirectory();
void FileSearch(char _chPath[]);

int CompareSubNodeByID(const void* _pvDataA, const void* _pvDataB);// 비교 함수 (오름차순 정렬)
int SendDataByUDP(char* _pchServerIP, char* _pchServerPort, const void* _ptMsgHead, int _nDataLen);
//-------------------

int LoadConfigSubNode(const char* _phFileName, Config * _ptConfig); // Config의 내용으로 변경한다.

int FindSuccAtSubNode(int _nStartID, SubNodeInfo* _ptSubNode, int _nNodeCn);
int SetConfigSubNode(Config* _ptConfig);


//void FindNode();
nodeInfoType SearchNodeAtFingleTable(unsigned int _unTargetId);
void ProcessSearchNode(char* _pchSeachNodeIP, char* _pchSeachNodePort);

nodeInfoType CalNextNodeByPacketCnt(nodeInfoType * _ptNextNodeInfo);

int SearchNodeTest();
int SearchNodeTestByPacket();
