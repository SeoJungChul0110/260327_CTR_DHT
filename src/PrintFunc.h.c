#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/PrintFunc.h"


/*
int ID;                  
struct sockaddr_in addrInfo;
*/
void printNodeInfoType(nodeInfoType* _ptNodeInfo, char* _chPreStr)
{
	// IP 출력 (inet_ntoa는 IPv4 전용)
	//const char* pchIPAddr = inet_ntoa(_pNodeInfo->addrInfo.sin_addr);
	//int nPort = ntohs(_pNodeInfo->addrInfo.sin_port); // 네트워크 바이트 순서 → 호스트 바이트 순서

	JC_Print(NONE_MODE, "\t%s[ID : %4d][%s : %d] ", _chPreStr, _ptNodeInfo->unID , inet_ntoa(_ptNodeInfo->tAddrInfo.sin_addr), ntohs(_ptNodeInfo->tAddrInfo.sin_port));
}

/*
typedef struct {              
unsigned int fileNum;        
fileRefType  fileRef[FileMax];  
} fileInfoType;
*/
void printFileInfoType(fileInfoType* _ptFileInfo, char* _chPreStr)
{
	JC_Print(NONE_MODE, "%s- FileNum:%4d", _chPreStr, _ptFileInfo->nFileNum);
	JC_Print(NONE_MODE, "%s- FILE_REF MAX[%d]", _chPreStr, DEX_FILE_MAX_CNT);

	for (int i = 0; i < DEX_FILE_MAX_CNT; i++)
	{
		printFileRefType(i, &_ptFileInfo->tFileRef[i], _chPreStr);
	}
}


/*
typedef struct {          
char Name[FNameMax];    
int  Key;              
nodeInfoType owner;       
nodeInfoType refOwner;     
} fileRefType;
*/

void printFileRefType(int _nIdx, fileRefType * _ptFileRef, char* _chPreStr)
{
	char chPreStrBuf[20];
	//JC_Print(NONE_MODE, "\n%s[FILE_REF]", _chPreStr);
	memset(chPreStrBuf, 0, sizeof(chPreStrBuf));
	if (strlen(_ptFileRef->chNameBuf) > 0)
	{
		JC_Print(NONE_MODE, "%s %5d] [FileName (%s)] [Key:%4d]", _chPreStr, _nIdx, _ptFileRef->chNameBuf, _ptFileRef->nKey);
		sprintf_s(chPreStrBuf, sizeof(chPreStrBuf), "%s[OWNER%4s]", _chPreStr, " ");
		printNodeInfoType(&_ptFileRef->tOwnerNodeInfo, chPreStrBuf);
		sprintf_s(chPreStrBuf, sizeof(chPreStrBuf), "%s[REF_OWNER]", _chPreStr);
		printNodeInfoType(&_ptFileRef->tRefOwnerNodeInfo, chPreStrBuf);
	}
	//else
	//{
	//	JC_Print(NONE_MODE, "%s(%2d) NULL", _chPreStr, _nIdx);
	//}

}

void printChordInfoType(chordInfoType* _ptChordInfo, char* _chPreStr)
{
	char chPreStrBuf[20];
	JC_Print(NONE_MODE, "%s[File Ref]", _chPreStr);
	memset(chPreStrBuf, 0, sizeof(chPreStrBuf));
	sprintf_s(chPreStrBuf, sizeof(chPreStrBuf), "%s\t", _chPreStr);
	printFileInfoType(&_ptChordInfo->tFRefInfo, chPreStrBuf);

	JC_Print(NONE_MODE, "%s[Finger Info]", _chPreStr);
	JC_Print(NONE_MODE, "%s[PRED]", chPreStrBuf);
	printNodeInfoType(&_ptChordInfo->tFingerInfo.tPre, chPreStrBuf);

	JC_Print(NONE_MODE, "%s[FINGER]", chPreStrBuf);
	for (int i = 0; i < DEF_BASE_M; i++)
	{
		sprintf_s(chPreStrBuf, sizeof(chPreStrBuf), "%s%5d] ", _chPreStr,i);
		printNodeInfoType(&_ptChordInfo->tFingerInfo.tFingers[i], chPreStrBuf);
	}
}

void printNodeType(nodeType* _ptNode)
{
	JC_Print(NONE_MODE, "\n[# Node]");
	printNodeInfoType(&_ptNode->tNodeInfo, "");

	JC_Print(NONE_MODE, "[# File Info]");
	printFileInfoType(&_ptNode->tFileInfo, "\t");

	JC_Print(NONE_MODE, "[# Chord Info]");
	printChordInfoType(&_ptNode->tChordInfo, "\t");
}