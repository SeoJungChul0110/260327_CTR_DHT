#pragma once


#pragma comment(lib,"ws2_32")


#include "./JC_Define.h"

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <process.h> 
#include <math.h>


//#pragma pack(push, 1) // 1바이트 단위로 정렬

typedef struct {                /* Node Info Type Structure */
	unsigned int unID;                     /* ID */
	int nStart;					// finger[i].start=(n+2i−1)mod2m(1≤i≤m)
	struct sockaddr_in tAddrInfo;/* Socket address */
	int nRecvPacketCnt; // 수신된 패킷 수
	int nSendPacketCnt; // 송신 패킷 수
} nodeInfoType;

typedef struct {            /* File Type Structure */
	char chNameBuf[DEF_FILE_NAME_MAX_LEN];       /* File Name */
	int  nKey;               /* File Key */
	nodeInfoType tOwnerNodeInfo;         /* Owner's Node */
	nodeInfoType tRefOwnerNodeInfo;      /* Ref Owner's Node */
} fileRefType;

typedef struct {               /* Global Information of Current Files */
	unsigned int nFileNum;         /* Number of files */
	fileRefType  tFileRef[DEX_FILE_MAX_CNT];   /* The Number of Current Files */
} fileInfoType;

typedef struct {             /* Finger Table Structure */
	nodeInfoType tPre;          /* Predecessor pointer */
	nodeInfoType tFingers[DEF_BASE_M];   /* Fingers (array of pointers) */
} fingerInfoType;

typedef struct {                /* Chord Information Structure */
	fileInfoType   tFRefInfo;   /* File Ref Own Information */
	fingerInfoType tFingerInfo;   /* Finger Table Information */
} chordInfoType;

typedef struct {            /* Node Structure */
	nodeInfoType  tNodeInfo;     /* Node's IPv4 Address */
	fileInfoType  tFileInfo;     /* File Own Information */
	chordInfoType tChordInfo;    /* Chord Data Information */
} nodeType;

#define DEF_HEAD_MSG_ID_PING			0
#define DEF_HEAD_MSG_ID_JOIN			1
#define DEF_HEAD_MSG_ID_MOVE_KEY		2
#define DEF_HEAD_MSG_ID_GET_PRED		3
#define DEF_HEAD_MSG_ID_UPD_PRED		4
#define DEF_HEAD_MSG_ID_FIND_SUCC		5
#define DEF_HEAD_MSG_ID_UPD_SUCC		6
#define DEF_HEAD_MSG_ID_FIND_PRED		7
#define DEF_HEAD_MSG_ID_LEAVE_KEY		8
#define DEF_HEAD_MSG_ID_ADD_FILE_REF	9
#define DEF_HEAD_MSG_ID_DEL_FILE_REF	10
#define DEF_HEAD_MSG_ID_DOWNLOAD_FILE	11
#define DEF_HEAD_MSG_ID_GET_FILE_REF	12


// 테스트용 프로토콜 정의 
#define DEF_HEAD_MSG_ID_FIND_NODE		13



#define DEF_HEAD_MSG_TYPE_REQ		0
#define DEF_HEAD_MSG_TYPE_RSP		1

typedef struct {
	unsigned short usMsgID;      // message ID
	unsigned short usMsgType;    // message type (0: request, 1: response)
	nodeInfoType   tNodeInfo;   // node address info 
	short          sMoreInfo;   // more info 
	fileRefType    tFileIRef;   // file (reference) info
	unsigned int   unBodySize;   // body size in Bytes
} chordHeaderType;             // CHORD message header type


extern nodeType ext_tMyNode ;               // node information -> global variable
extern SOCKET ext_tRecvProcSock;
extern SOCKET ext_tReqSock, ext_flSock, ext_frSock, ext_fsSock, ext_pfSock;
extern HANDLE ext_hMutex;
extern nodeInfoType ext_initNode;
extern int ext_nSilentMode; // silent mode


#define MAX_TEST_SUB_NODE_CNT   DEF_BASE_M //500

typedef struct
{
	int unID;
	struct sockaddr_in tAddrInfo;
	char chIPAddrBuf[64];
	int nPort;
	//int nRecvPacketCnt; // 수신된 패킷 수
	//int nSendPacketCnt; // 송신 패킷 수
} SubNodeInfo;

typedef struct
{
	int nNodeCnt;
	SubNodeInfo tSubNodes[MAX_TEST_SUB_NODE_CNT];
} Config;
//#pragma pack(pop)

extern 	Config ext_tConfig;
extern int ext_bUsePingAndFixFinger; // procPingAndFixFinger 사용 여부,

extern int ext_nLogLevel;
extern char ext_chResultLogFile[100]; // 통계 분석을 위한 로그 파일 
extern int ext_nUsingLoadSubNodeForTest; // 테스트 모드 사용 여부 

//테스트할 노드 인덱스 -1이 아니면, 선택
extern int ext_nUsingTestNodeIdx;
// 검색할때 패킷 카운트를 사용한다, USING_TEST_NODE_IDX가 0이상이면 전송 횟수값
extern int ext_nUsingPacketCntForTest; 

// 공유할 데이터 구조체 정의
typedef struct {
	int nCnt;      // 현재 접속 중인 프로세스 수
	char chMsgBuf[256];     // 전달할 메시지
} SharedData ;
// 공유 메모리 
extern SharedData * ext_pSHMBuf;