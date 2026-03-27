#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/JC_Test.h"
#include "../JC_Util/JC_Config.h"
#include "../include/UtilFun.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <Windows.h>


#pragma comment(lib,"ws2_32")
#include <winsock2.h>


#pragma warning ( disable : 4996 )

#define DIRECTORY_TYPE 1
#define FILE_TYPE 0

struct _finddata_t fd;

Config ext_tConfig = { 0 };

int ext_nLogLevel = INFO_MODE;
char ext_chResultLogFile[100] = { 0 }; // 통계 분석을 위한 로그 파일 
int ext_nUsingLoadSubNodeForTest = 0; // 테스트 모드 사용 여부 
int ext_nUsingPacketCntForTest = 0;
int ext_nUsingTestNodeIdx = -1;

int IsDirectory()
{
    if (fd.attrib & _A_SUBDIR)
        return DIRECTORY_TYPE;
    else
        return FILE_TYPE;

}

void FileSearch(char _chPath[])
{
    intptr_t tHandle;
    int bIsDir = 0;
    //char _chPath[_MAX_PATH];

    //strcpy(_chPath, _chPath);
    strcat(_chPath, "\\");
    strcat(_chPath, "*.*");

    if ((tHandle = _findfirst(_chPath, &fd)) == -1)
    {
        //printf("%s - No such file or directory\n",path);
        return;
    }

    while (_findnext(tHandle, &fd) == 0)
    {
        char chFullPath[_MAX_PATH];
        strcpy(chFullPath, _chPath);
        strcat(chFullPath, fd.name);

        bIsDir = IsDirectory();    // 디렉토리 판별

        if (IsDirectory() && fd.name[0] != '.')  //<.>, <..>이 아닌 디렉토리 
        {
            strcat(chFullPath, "\\");
            FileSearch(chFullPath);    // 하위 디렉토리 검색
        }
        else if (bIsDir == FILE_TYPE && fd.size != 0 && fd.name[0] != '.')
        {
            //#fprintf(stderr,  ">> %s, %d 바이트\n", file_pt, fd.size);
            fprintf(stderr, "%s \n", fd.name);
            //unsigned int strHash
        }
    }
    _findclose(tHandle);
}

// 비교 함수 (오름차순 정렬)
int CompareSubNodeByID(const void* _pvDataA, const void* _pvDataB) 
{
    SubNodeInfo* na = (SubNodeInfo*)_pvDataA;
    SubNodeInfo* nb = (SubNodeInfo*)_pvDataB;
    return na->unID - nb->unID; // 오름차순
   //return nb->unID - na->unID; // 내림차순
}

//-------------------------------------------------------------------------

int LoadConfigSubNode(const char* _phFileName, Config* _ptConfig)
{
#if JC_FUNCTION_CALL
    JC_FUNC(DEBUG_MODE, "");
#endif 

    FILE* pFileFP = NULL;

    int err = fopen_s(&pFileFP, _phFileName, "r");
    if (err != 0 || pFileFP == NULL) {
        fprintf(stderr, "Failed to open config file: %s\n", _phFileName);
        perror("fopen_s");
        return -1;
    }

    char chLineBuf[256];

    while (fgets(chLineBuf, sizeof(chLineBuf), pFileFP))
    {

        // 주석 또는 빈 줄은 건너뜀
        if (chLineBuf[0] == '#' || chLineBuf[0] == '\n') continue;


        char chKeyBuf[64] = { 0 }, chValueBuf[128] = { 0 };


        if (sscanf_s(chLineBuf, "%63[^=]=%127s", chKeyBuf, (unsigned)_countof(chKeyBuf),
            chValueBuf, (unsigned)_countof(chValueBuf)) == 2)
        {

            if (strcmp(chKeyBuf, "NODE_CNT") == 0)
            {

                _ptConfig->nNodeCnt = atoi(chValueBuf);
                if (_ptConfig->nNodeCnt == 0)
                {
                    _ptConfig->nNodeCnt = DEF_BASE_M;
                }
            }
            else if (strncmp(chKeyBuf, "NODE_", 5) == 0)
            {

                int nIdx = atoi(chKeyBuf + 5) - 1;
                if (nIdx >= 0 && nIdx < ext_tConfig.nNodeCnt) //MAX_TEST_SUB_NODE_CNT)
                {
                    char chIPAddr[64] = { 0 };
                    char chPort[6] = { 0 };


                    // 나보다 큰것을 먼저 넣는다. 
                    if (sscanf_s(chValueBuf, "%63[^:]:%s",
                        chIPAddr, (unsigned)_countof(chIPAddr), &chPort) == 2)
                    {
                        //chIPAddr, (unsigned)_countof(chIPAddr), &nPort) == 2)
                        strcpy_s(_ptConfig->tSubNodes[nIdx].chIPAddrBuf,
                            sizeof(_ptConfig->tSubNodes[nIdx].chIPAddrBuf), chIPAddr);
                        _ptConfig->tSubNodes[nIdx].nPort = atoi(chPort);

                        // ID 값 만들기 
                        char* pCmbinedStr = ConcatString(2, _ptConfig->tSubNodes[nIdx].chIPAddrBuf, chPort);

                        _ptConfig->tSubNodes[nIdx].unID = strHash(pCmbinedStr);
                        if (pCmbinedStr) 
                        {
                            free(pCmbinedStr);          // 동적 메모리 해제!
                        }

                        // IP 세팅 
                        _ptConfig->tSubNodes[nIdx].tAddrInfo.sin_family = AF_INET;
                        if ((_ptConfig->tSubNodes[nIdx].tAddrInfo.sin_addr.s_addr = inet_addr(chIPAddr)) == INADDR_NONE) {
                            JC_Print(ERROR_MODE, "\a[ERROR] <IP Addr> is wrong!");
                            exit(1);
                        }
                        _ptConfig->tSubNodes[nIdx].tAddrInfo.sin_port = htons(atoi(chPort));
                    }                   
                }
            }
            else
            {
                char* key = strtok(chLineBuf, "=");
                char* value = strtok(NULL, "\n");

                if (!key || !value) continue;

                // 공백 제거
                while (*key == ' ') key++;
                while (*value == ' ') value++;

                if (strcmp(key, "RESULT_LOG_FILE") == 0)
                {

                    strncpy(ext_chResultLogFile, value, sizeof(ext_chResultLogFile) - 1);
                    ext_chResultLogFile[sizeof(ext_chResultLogFile) - 1] = '\0';  // null 종료 보장
                    fprintf(stderr, "\n 1==>[%s] ", value);
                    fprintf(stderr, "\n 2==>[%s] ", ext_chResultLogFile);
                }
                else if (strcmp(key, "LOG_LEVEL") == 0)
                {
                    ext_nLogLevel = atoi(value);
                }
                else if (strcmp(key, "USING_LOAD_SUB_NODE_FOR_TEST") == 0)
                {
                    ext_nUsingLoadSubNodeForTest = atoi(value);
                }
                else if (strcmp(key, "USING_PACKET_CNT_FOR_TEST") == 0)
                {
                    ext_nUsingPacketCntForTest = atoi(value);
                }
                else if (strcmp(key, "USING_TEST_NODE_IDX") == 0)
                {
                    ext_nUsingTestNodeIdx = atoi(value);

                    fprintf(stderr, "\n# 1.ext_nUsingTestNodeIdx[%s]", value);
                    fprintf(stderr, "\n# 1. ext_nUsingTestNodeIdx[%d]", ext_nUsingTestNodeIdx);
                    fprintf(stderr, "\n# 1. ext_tConfig.nNodeCnt[%d]", ext_tConfig.nNodeCnt);
                    // MAX IDX 설정 확인
                    if (ext_nUsingTestNodeIdx >= DEF_BASE_M)
                    {
                        ext_nUsingTestNodeIdx = DEF_BASE_M - 1;
                    }
                    fprintf(stderr, "\n# 2.ext_nUsingTestNodeIdx[%d]", ext_nUsingTestNodeIdx);
                    
                }
            }
        }
    } //end while 
    fclose(pFileFP);

    fprintf(stderr, "\n# [%s]", (ext_nUsingLoadSubNodeForTest == 1 ? "TEST_MODE" : "EXE_MODE"));
    fprintf(stderr, "\n# RESULT_LOG_FILE [%s]", ext_chResultLogFile);
    fprintf(stderr, "\n# LOG_LEVEL [%s]", (ext_nLogLevel == NONE_MODE ? "NONE"
        : (ext_nLogLevel == NONE_MODE ? "NONE"
            : (ext_nLogLevel == ERROR_MODE ? "ERROR"
                : (ext_nLogLevel == WARN_MODE ? "WARN"
                    : (ext_nLogLevel == INFO_MODE ? "INFO"
                        : (ext_nLogLevel == DEBUG_MODE ? "DEBUG"
                            : "UNKNOWN")))))));
    fprintf(stderr, "\n# USING_LOAD_SUB_NODE [%s]", (ext_nUsingLoadSubNodeForTest == 1 ? "TRUE" : "FALSE"));
    fprintf(stderr, "\n# TEST [USING_PACKET_CNT: %d, TEST_NODE_IDX:%d]"
            , ext_nUsingPacketCntForTest, ext_nUsingTestNodeIdx);
    //printNodeType(&ext_tMyNode);

    return 0;
}


//-----------------------------------------------------------
// successor 찾기 함수
int FindSuccAtSubNode(int _nStartID, SubNodeInfo * _ptSubNode, int _nNodeCn)
{
    // 노드 집합은 정렬되어 있다고 가정
    for (int i = 0; i < _nNodeCn; i++) {
        if (_ptSubNode[i].unID >= _nStartID) {
            //return _ptSubNode[i].unID;
            return i;
        }
    }
    // 못 찾으면 첫 번째 노드 (wrap-around)
    //return _ptSubNode[0].unID;
    return 0;
}



int SetConfigSubNode(Config* _ptConfig)
{
    // 정렬 
    qsort(_ptConfig->tSubNodes, _ptConfig->nNodeCnt, sizeof(SubNodeInfo), CompareSubNodeByID);

    //JC_Print(DEBUG_MODE, "1. NODE_CNT = %d", _ptConfig->nNodeCnt);
    //for (int i = 0; i < _ptConfig->nNodeCnt; i++)
    //{
    //    JC_Print(DEBUG_MODE, "NODE_%d = [%d] %s:%d", i,
    //        _ptConfig->tSubNodes[i].unID,
    //        _ptConfig->tSubNodes[i].chIPAddrBuf, _ptConfig->tSubNodes[i].nPort);
    //}

    // 1. 나와 같은 노드 찾기
    JC_Print(DEBUG_MODE, "SET FINGER TABLE = %d", _ptConfig->nNodeCnt);
    int nMyNodeFingerIdx = -1;
    for (int i = 0; i < _ptConfig->nNodeCnt; i++)
    {
        if (ext_tMyNode.tNodeInfo.unID == _ptConfig->tSubNodes[i].unID)
        {
            nMyNodeFingerIdx = i;
            /*        JC_Print(DEBUG_MODE, "1. 나와 같은 노드 찾기: nMyNodeFingerIdx: %d(Id:%d)"
                        , nMyNodeFingerIdx, ext_tMyNode.tNodeInfo.unID);*/
            break;
        }
    }

    // 1. Start, Success 등록 
    for (int i = 0; i < DEF_BASE_M; i++)
    {
        int nStart = (ext_tMyNode.tNodeInfo.unID + (1 << i)) % DEF_ID_SPACE;
        int nSuccNodeIdx = FindSuccAtSubNode(nStart, _ptConfig->tSubNodes, _ptConfig->nNodeCnt);

        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nStart = nStart;
        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID = _ptConfig->tSubNodes[nSuccNodeIdx].unID;
        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo = _ptConfig->tSubNodes[nSuccNodeIdx].tAddrInfo;
    }
     
    // 2. Success 등록
#if 0 // 그냥 순차적으로등록
    // 2. 나는 FingerTable[0] = IDX
    // 3. 나보다 큰것은 [1~N] = IDX + i
    for (int i = 0, j = nMyNodeFingerIdx; j < _ptConfig->nNodeCnt; i++, j++)
    {
        /*      JC_Print(INFO_MODE, "2, i <= k: %d, %d(ID:%d => %d)"
                  , i, j
                  , ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
                  , _ptConfig->tSubNodes[j].unID);*/
        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID = _ptConfig->tSubNodes[j].unID;
        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo = _ptConfig->tSubNodes[j].tAddrInfo;
    }
    // 4. 나머지는 [N~]
    for (int i = _ptConfig->nNodeCnt - nMyNodeFingerIdx, j = 0; i < _ptConfig->nNodeCnt; i++, j++)
    {
        //JC_Print(INFO_MODE, "4, i <= k: %d, %d(ID:%d => %d)"
        //    , i, j
        //    , ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
        //    , _ptConfig->tSubNodes[j].unID);
        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID = _ptConfig->tSubNodes[j].unID;
        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo = _ptConfig->tSubNodes[j].tAddrInfo;
    }
#else

#endif 

    return 0;
}

/*

#if 1
    char cwd[256];
    _getcwd(cwd, sizeof(cwd));
    fprintf(stderr, "Current dir: %s\n", cwd);
#endif


    //char dir[_MAX_PATH] = "..\\log\\";
    char dir[_MAX_PATH] = "..\\conf\\iapl_chord.conf";

    FileSearch(dir);

    // conf 파일 
char chFileNameBuf[128];
memset(chFileNameBuf, 0, sizeof(chFileNameBuf));
strcpy(chFileNameBuf, "..\\conf\\iapl_chord.conf");
// strcpy(chFileNameBuf, "../conf/iapl_chord.conf");
// strcpy(chFileNameBuf, "..\conf\iapl_chord.conf");


char m_chValueBuf[256];
memset(m_chValueBuf, 0, sizeof(m_chValueBuf));
if (ReadConfig(chFileNameBuf, "TCP_SERVER_IP", m_chValueBuf) == 0)
{
    //strcpy(g_byLocalAddrBuf, m_chValueBuf);

    JC_Print(INFO_MODE, "[ReadIPConfig] m_chValueBuf[%s]", m_chValueBuf);
}
else
{
    JC_Print(ERROR_MODE, "%s__%s(%d):", __FILE__, __FUNCTION__, __LINE__);
    JC_Print(ERROR_MODE, "[ReadIPConfig] LOCAL_IP");
}

*/

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")


int SendDataByUDP(char * _pchServerIP, char* _pchServerPort, const void* _ptMsgHead, int _nDataLen)
{
    WSADATA tWsa;
    SOCKET tSock;
    struct sockaddr_in tServerAddr;
    WSAStartup(MAKEWORD(2, 2), &tWsa);

    tSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (tSock == INVALID_SOCKET) {
        JC_Print(ERROR_MODE, "Socket() error(%s : %s): %d\n"
                , _pchServerIP, _pchServerPort, WSAGetLastError());
        return 1;
    }

    // 전송 대상 주소
    memset(&tServerAddr, 0, sizeof(tServerAddr));
    tServerAddr.sin_family = AF_INET;
    tServerAddr.sin_port = htons(atoi(_pchServerPort)); // 포트는 네트워크 바이트 순서로
    tServerAddr.sin_addr.s_addr = inet_addr(_pchServerIP);
    
    int nRet = sendto(tSock, (const char*)_ptMsgHead, _nDataLen, 0,
        (struct sockaddr*)&tServerAddr, sizeof(tServerAddr));

    if (nRet == SOCKET_ERROR) 
    {
        JC_Print(ERROR_MODE, "Send error(%s : %s): %d"
            , _pchServerIP, _pchServerPort, WSAGetLastError());
        return -1;
    }
    else 
    {
        //JC_DumpMsg((char*)_ptMsgHead, nRet);
        JC_Print(INFO_MODE, "Send %d bytes(%s : %s)"
            , nRet, _pchServerIP, _pchServerPort);
    }
    closesocket(tSock);
    WSACleanup();

    Sleep(10);
    return nRet;
}

//-------------------------------------------------------------
//void FindNode()
//{
//#if JC_FUNCTION_CALL
//	JC_FUNC(DEBUG_MODE, "");
//#endif 
//
//    /*
//    // 찾기위한 서브 정보를 입력
//        자산의 IP를 확인하고, Search Node 호출
//        SeachNodeAtFingleTable(IP, Port)
//    */
//    char chNodeIPAddrBuf[16];
//    char chNodePortBuf[6];
//    JC_Print(INFO_MODE, "CHORD> Input For Find Node IP Port.");
//    JC_Print(INFO_MODE, "CHORD> IP와 Port 입력 (예: 192.168.0.10 8080):");
//    scanf_s("%63s %15s", chNodeIPAddrBuf, (unsigned)_countof(chNodeIPAddrBuf), chNodePortBuf, (unsigned)_countof(chNodePortBuf));
//
//    JC_Print(INFO_MODE, "CHORD> 노드 찾기 (%s:%d) -> (%s:%d)"
//        , inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port)
//        , chNodeIPAddrBuf, atoi(chNodePortBuf));
//
//    JC_RESULT("\n(%s:%d) -> (%s:%d),"        
//        , inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port)
//        , chNodeIPAddrBuf, atoi(chNodePortBuf));
//    // 찾는 정보를 기술
//    SeachNodeAtFingleTable(chNodeIPAddrBuf, chNodePortBuf);
//
//}

// 타겟에 가까운” finger 노드(즉 타겟 이전에 있으면서 타겟에 가장 근접한 노드)
// 찬지 못하면 기본값으로 자신(ext_tMyNode.tNodeInfo) 을 반환
// 1. 키 K는(predecessor, node] 구간의 successor 노드가 담당
// 2. 탐색 시 현재 노드의 Finger Table에서
//     K보다 작거나 같으면서 가장 가까운 노드(closest predecessor)를 선택해 hop
nodeInfoType SearchNodeAtFingleTable(unsigned int _unTargetId)
{
#if JC_FUNCTION_CALL
    JC_FUNC(DEBUG_MODE, "");
#endif 
    // tClosestNodeInfo는 최종 반환할 노드 정보(초기값: 자기 자신).
    nodeInfoType tClosestNodeInfo = ext_tMyNode.tNodeInfo;
    // nClosestDiff는 현재까지 발견한 타겟까지의 최소 거리를 저장. 처음엔 ID 공간 전체로 초기화.
    unsigned int unClosestDiff = DEF_ID_SPACE; //1024; 

    //WaitForSingleObject(ext_hMutex, INFINITE);

#if JC_DEBUG_SEARCH_TEST
        JC_Print(DEBUG_MODE, "#> Check At modin([나(%u) < Search(%u) < Finger[0](%u))"
            , ext_tMyNode.tNodeInfo.unID, _unTargetId, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID);
#endif 

    // 2. 내 ID가 키와 정확히 같다면 완료
    if (ext_tMyNode.tNodeInfo.unID == _unTargetId)
    {
#if JC_DEBUG_SEARCH_TEST
        JC_Print(DEBUG_MODE, "\t\t\t=> Found");
#endif 
        return ext_tMyNode.tNodeInfo;
    }
    // 1. IDKey가 (나, finger[0]) 구간 안에 있나?, 내가 _nIDKey의 바로 전 노드인지 검사
    if ((modIn(DEF_ID_SPACE, _unTargetId, ext_tMyNode.tNodeInfo.unID, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID, 0, 1)))
    {

#if JC_DEBUG_SEARCH_TEST
        JC_Print(DEBUG_MODE, "\t0.=> 리턴 finger[0] ");
#endif 

        return ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0];
    }


#if JC_DEBUG_SEARCH_TEST
    JC_Print(DEBUG_MODE, "#> Check At FingerTable : %u (Search Node ID:%u)", DEF_BASE_M, _unTargetId);
#endif // 높은 finger는 더 큰 범위를 커버하므로 빠르게 가까운 노드를 찾기 위함
    for (int i = DEF_BASE_M - 1; i >= 0; i--)
    {
        nodeInfoType tFingerNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i];

#if JC_DEBUG_SEARCH_TEST
        JC_Print(DEBUG_MODE, "\t%d-1.[tClosestNodeInfo (ID:%u)(%s:%d)] > Fingers[%u] (ID:%u)(%s:%d)"
            , i
            , tClosestNodeInfo.unID
            , inet_ntoa(tClosestNodeInfo.tAddrInfo.sin_addr)
            , ntohs(tClosestNodeInfo.tAddrInfo.sin_port)
            , i
            , tFingerNodeInfo.unID
            , inet_ntoa(tFingerNodeInfo.tAddrInfo.sin_addr)
            , ntohs(tFingerNodeInfo.tAddrInfo.sin_port));
#endif 
        //################################### 
        // 초기값과 동일하면 스킵
        if (memcmp(&tFingerNodeInfo, &ext_initNode, sizeof(ext_initNode)) == 0) 
        {
#if JC_DEBUG_SEARCH_TEST
            JC_Print(DEBUG_MODE, "\t\t\t=> Continue");
#endif 
            continue;
        }

        /*_
        불필요한 네트워크 통신(Hop)을 1회 줄여주는 최적화(Optimization)
        Chord 알고리즘의 SearchNodeAtFingerTable (보통 closest_preceding_node라고 부름) 함수는 원래 "Target 바로 앞의 노드"를 찾아서 그 노드에게 길을 물어보는 역할        
        */
        // 2. 내 ID가 키와 정확히 같다면 완료
        if (tFingerNodeInfo.unID == _unTargetId)
        {
#if JC_DEBUG_SEARCH_TEST
            JC_Print(DEBUG_MODE, "\t\t\t=> Found");
#endif 
            tClosestNodeInfo = tFingerNodeInfo;
            break;
        }

#if JC_DEBUG_SEARCH_TEST
        JC_Print(DEBUG_MODE, "\t%d-2. Check Range(modein)Fingers[%u] (ID: %u< %d <%u)(%s:%d)"
            ,i , i
            , ext_tMyNode.tNodeInfo.unID
            , _unTargetId
            , tFingerNodeInfo.unID
            , inet_ntoa(tFingerNodeInfo.tAddrInfo.sin_addr)
            , ntohs(tFingerNodeInfo.tAddrInfo.sin_port));
#endif 
        // 범위 검사 (modIn 호출)
        // tFingerNodeInfo.unID가 (myID, _nTargetId] (모듈러 링상) 범위에 있는지를 확인
        if (modIn(DEF_ID_SPACE, tFingerNodeInfo.unID, ext_tMyNode.tNodeInfo.unID, _unTargetId, 0, 0)) // 타켓이 (myID, fingerID] 범위에 있는지 확인
        { 
            //int nDiff = (_nTargetId - tFingerNodeInfo.unID + 1024) % 1024;
            // 거리 계산 및 비교
            unsigned int unDiff = (_unTargetId - tFingerNodeInfo.unID + DEF_ID_SPACE) % DEF_ID_SPACE;

#if JC_DEBUG_SEARCH_TEST
            JC_Print(DEBUG_MODE, "\t\t Check Range(modein) nDiff=%u, nDiff=%u", unDiff, unClosestDiff);
#endif 
            if (unDiff < unClosestDiff) 
            {
                tClosestNodeInfo = tFingerNodeInfo;
                unClosestDiff = unDiff;
#if JC_DEBUG_SEARCH_TEST
                JC_Print(DEBUG_MODE, "\t\t\t=> Found Closest");
#endif 
            }

        }
    }
    //ReleaseMutex(ext_hMutex);
    return tClosestNodeInfo;
}

// Packet Cnt 기반으로 다음 노드를 찾는다. 
nodeInfoType CalNextNodeByPacketCnt(nodeInfoType * _ptNextNodeInfo)
{
    nodeInfoType tNextNodeInfoByPacketCnt = { 0 };
    tNextNodeInfoByPacketCnt = *_ptNextNodeInfo;

    nodeInfoType tUniqueNodeInfoList[DEF_BASE_M]; // 중복 제거된 ID를 저장할 배열
    int nUniqueNodeListCnt = 0;

    // 1️. 중복 제거
    for (int i = 0; i < DEF_BASE_M; i++) 
    {
        BOOL bExists = FALSE;
        for (int j = 0; j < nUniqueNodeListCnt; j++) 
        {
            if (ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID == tUniqueNodeInfoList[j].unID)
            {
                bExists = TRUE;
                break;
            }
        }
        if (!bExists) {
            tUniqueNodeInfoList[nUniqueNodeListCnt++] = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i];
        }
    }
    // 중복제거 확인
    for (int j = 0; j < nUniqueNodeListCnt; j++)
    {

        JC_Print(DEBUG_MODE, "# [%2d] (%s : %d) ID: [%4u] - R:%3d, S:%3d"
            , j
            , inet_ntoa(tUniqueNodeInfoList[j].tAddrInfo.sin_addr)
            , ntohs(tUniqueNodeInfoList[j].tAddrInfo.sin_port)
            , tUniqueNodeInfoList[j].unID
            , tUniqueNodeInfoList[j].nRecvPacketCnt
            , tUniqueNodeInfoList[j].nSendPacketCnt);
    }
    
    // 2. 노드 Index 찾기 
    int nNextNodeIdx = 0;
    int nTotalSendPacketCnt = 0;
    for (int i = 0; i < nUniqueNodeListCnt; i++) 
    {
        if (tUniqueNodeInfoList[i].unID == _ptNextNodeInfo->unID) 
        {
            nNextNodeIdx = i;
            //break;
        }
        nTotalSendPacketCnt = nTotalSendPacketCnt + tUniqueNodeInfoList[i].nSendPacketCnt;
    }

    // packet cnt로 계산한 Idx
    //int nNextNodeIdxByPacket = (nNextNodeIdx + tUniqueNodeInfoList[nNextNodeIdx].nSendPacketCnt) % nUniqueNodeListCnt;
    int nNextNodeIdxByPacket = (nNextNodeIdx + nTotalSendPacketCnt) % nUniqueNodeListCnt;
    tNextNodeInfoByPacketCnt = tUniqueNodeInfoList[nNextNodeIdxByPacket];

//################################################################
    JC_Print(DEBUG_MODE, "[#TotalPacketCnt(%d)]  (%s : %d) ID: [%4u] => (%s : %d) ID: [%4u] "
        , nTotalSendPacketCnt
        , inet_ntoa(_ptNextNodeInfo->tAddrInfo.sin_addr)
        , ntohs(_ptNextNodeInfo->tAddrInfo.sin_port)
        , _ptNextNodeInfo->unID
        , inet_ntoa(tNextNodeInfoByPacketCnt.tAddrInfo.sin_addr)
        , ntohs(tNextNodeInfoByPacketCnt.tAddrInfo.sin_port)
        , tNextNodeInfoByPacketCnt.unID);

        return tNextNodeInfoByPacketCnt;
}


void ProcessSearchNode(char * _pchSeachNodeIP, char* _pchSeachNodePort)
{
#if JC_FUNCTION_CALL
    JC_FUNC(DEBUG_MODE, "");
#endif 
    // 자신의 정보를 기술 
    /*  
        1. FingerTable에서 노드 찾기
            ext_tMyNode.tChordInfo.tFingerInfo
        2. 못찾으면, 
            Send Mesg
    */


    // 공유메모리에 기록한다. 
    ext_pSHMBuf->nCnt++;

    //-----------------------------------
    // 검색 노드
    nodeInfoType tSearchNodeInfo = { 0 };
    // ID 값 만들기 
    char* pCmbinedStr = ConcatString(2, _pchSeachNodeIP, _pchSeachNodePort);
    tSearchNodeInfo.unID = strHash(pCmbinedStr);
    if (pCmbinedStr)
    {
        free(pCmbinedStr);          // 동적 메모리 해제!
    }

    // 수신한 검색 노드의 IP와 포트를 출력
    ShareWriteResultLog(ext_chResultLogFile, "%s:%d,"
        , inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr)
        , ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port));

    // 결과 로그 남기기 위한 부분
    //Sleep(DEF_RESULT_LOG_CAP_TIME);

    // IP 세팅 
    tSearchNodeInfo.tAddrInfo.sin_family = AF_INET;
    if ((tSearchNodeInfo.tAddrInfo.sin_addr.s_addr = inet_addr(_pchSeachNodeIP)) == INADDR_NONE) {
        JC_Print(ERROR_MODE, "\a[ERROR] <IP Addr> is wrong!");
        return;
    }
    tSearchNodeInfo.tAddrInfo.sin_port = htons(atoi(_pchSeachNodePort));

    //#############################################
    //-------------------------------------
    /*
    // 1. FingerTable에서 노드 찾기(구현 필요)
        - FingerTable에서 동일한 노드 찾기
        - 없으면 가장 가까운 노드 찾기(나보다는 크면서 가장 작은 노드)
    //ext_tMyNode.tChordInfo.tFingerInfo.tFingers
    */
    int nIsFoundNode = 0;
    // ext_tMyNode.tChordInfo.tFingerInfo
    		
    char chSendNodeIPBuf[16];
    char chSendNodePortBuf[6];

    // 검색된 정보로 전송해야함
    // 5000 > 5001 > 5002 > 5003로 전달하는 테스트 
    // 자신 + 1, 5004면 종료
    // # 로그 확인 
    // SEARCH NODE INFO(127.0.0.1:5001) > 127.0.0.1:5000 > 127.0.0.1:5001 > 127.0.0.1:5002 > 127.0.0.1:5003
    int nSendNodePort = ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port) + 1;

    sprintf(chSendNodeIPBuf, "%s", "127.0.0.1");
    sprintf(chSendNodePortBuf, "%d", nSendNodePort);


    if (tSearchNodeInfo.unID == ext_tMyNode.tNodeInfo.unID) // 도달 여부 확인
    {
        nIsFoundNode = 1;
    }
    //#############################################


    if(nIsFoundNode)
    {


        // 노드를 찾았다면 멈춤
        JC_Print(INFO_MODE, "Complete [SerchNode (%s:%s) !!!", _pchSeachNodeIP, _pchSeachNodePort);
        ShareWriteResultLog(ext_chResultLogFile, " # FOUND ");
        // 공유메모리에 기록한다. 
        ext_pSHMBuf->nCnt = 0;
        memset(ext_pSHMBuf->chMsgBuf, 0, sizeof(ext_pSHMBuf->chMsgBuf));


        return;
    }
    else // 못찾으면..다음 노드로 전송하는 부분 
    {
        if (ext_pSHMBuf->nCnt > DEF_MAX_HOP)
        {
            // 공유메모리에 기록한다. 
            ext_pSHMBuf->nCnt = 0;
            memset(ext_pSHMBuf->chMsgBuf, 0, sizeof(ext_pSHMBuf->chMsgBuf));
            return;
        }
        JC_Print(INFO_MODE, "Search Fingle Table [SerchNode (ID:%d)(%s:%s) !!!"
            , tSearchNodeInfo.unID
            , _pchSeachNodeIP, _pchSeachNodePort);
        // 노드 검색 
        nodeInfoType tNextNode = SearchNodeAtFingleTable(tSearchNodeInfo.unID);

        if (tNextNode.unID == ext_tMyNode.tNodeInfo.unID) {
            JC_Print(INFO_MODE, "No closer node found in Finger Table..(tNextNode.unID == ext_tMyNode.tNodeInfo.unID=> %d, %d"
            , tNextNode.unID, ext_tMyNode.tNodeInfo.unID);
            nIsFoundNode = 1;
        }
        else
        {
            // 패킷 카운트를 사용해서 노드 변경 
            if (ext_nUsingPacketCntForTest > 0)
            {
                tNextNode = CalNextNodeByPacketCnt(&tNextNode);
            }

            sprintf(chSendNodeIPBuf, "%s", inet_ntoa(tNextNode.tAddrInfo.sin_addr));
            sprintf(chSendNodePortBuf, "%d", ntohs(tNextNode.tAddrInfo.sin_port));

            // 검색 메시지 작성
            chordHeaderType tSendMsgHead;
            memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
            tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_FIND_NODE;
            tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_REQ;
            tSendMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
            tSendMsgHead.sMoreInfo = 0;
            tSendMsgHead.unBodySize = sizeof(tSearchNodeInfo);

            //전송할 노드 확인            // 
            char* pCmbinedStr = ConcatString(2, chSendNodeIPBuf, chSendNodePortBuf);
            int SendNodeID = strHash(pCmbinedStr);


            // 메시지 전송
        /*    JC_Print(DEBUG_MODE, "sizeof(chordHeaderType): %d, sizeof(tSendMsgHead): %d",
                sizeof(chordHeaderType), sizeof(tSendMsgHead))*/;
            int nRetVal = SendDataByUDP(chSendNodeIPBuf, chSendNodePortBuf, &tSendMsgHead, sizeof(chordHeaderType));
            //int nRetVal = proSendMsg(_pchIP, _pchPort, &tSendMsgHead, sizeof(tSendMsgHead));
            //JC_Print(INFO_MODE, "Send Head: %d", nRetVal);

            // 바디 데이터 전송 (검색 정보 전송 )
            nRetVal = SendDataByUDP(chSendNodeIPBuf, chSendNodePortBuf, &tSearchNodeInfo, tSendMsgHead.unBodySize);
            //JC_Print(INFO_MODE, "Send B: %d(%d)", nRetVal, sizeof(nodeInfoType));


            // 송신 패킷 수 계산
         /*   char* pCmbinedStr = ConcatString(2, chSendNodeIPBuf, chSendNodePortBuf);
            int SendNodeID = strHash(pCmbinedStr);*/

            if (ext_nUsingPacketCntForTest > 0)
            {
                // FingerTable에서 등록
                for (int i = 0; i < DEF_BASE_M; i++)
                {
                    if (ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID == SendNodeID)
                    {
                        ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nSendPacketCnt++;
                        //JC_Print(DEBUG_MODE, "#=> Finger Cnt(IDX:%d) (ID:%d) : %d"
                        //    , i
                        //    , ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
                        //    , ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nSendPacketCnt);
                    }
                }
                //// LoadConfigFile에서 등록
                //for (int i = 0; i < ext_tConfig.nNodeCnt; i++) //MAX_TEST_SUB_NODE_CNT
                //{
                //    if (ext_tConfig.tSubNodes[i].unID == SendNodeID)
                //    {
                //        ext_tConfig.tSubNodes[i].nSendPacketCnt++;
                //        JC_Print(DEBUG_MODE, "#=> Sub Cnt(IDX: % d) (ID: %d) : %d"
                //            , i
                //            , ext_tConfig.tSubNodes[i].unID
                //            , ext_tConfig.tSubNodes[i].nSendPacketCnt);
                //    }
                //}
            }

        }
        
    }


#if JC_FUNCTION_CALL
    JC_FUNC(DEBUG_MODE, "END");
#endif 
}


int SearchNodeTest()
{
    JC_Print(NONE_MODE, "[# Find Node]");

    /*
     // 찾기위한 서브 정보를 입력
         자산의 IP를 확인하고, Search Node 호출
         SeachNodeAtFingleTable(IP, Port)
     */
    char chNodeIPAddrBuf[16];
    char chNodePortBuf[6];

    int nTestCnt = 1;
    /*
    * BASE_M    2	4	6	8	10	12
4	            16	64	256	1024	4096
    */

    //int nBASE_M_BUF[] = { 2, 4,	6, 8,10, 12 };

    ShareWriteResultLog(ext_chResultLogFile, "\n");
    
    // 결과 로그 남기기 위한 부분
    int nSendTestCnt = 0;
    // 1. 전송 테스트 횟수 설정
    if (ext_nUsingTestNodeIdx > -1 && ext_nUsingPacketCntForTest > 1) // 패킷카운트를 사용할 경우
    {
        nSendTestCnt = ext_nUsingPacketCntForTest;
    }
    else
    {
        nSendTestCnt = DEF_BASE_M;
    }
#if 1
    int i = 0; // 루프 제어 변수 초기화

    if (nSendTestCnt > 0) // nSendTestCnt가 0인 경우를 대비한 안전 장치
    {
        do
        {
            // 처리하는게 없다면 
            if (strlen(ext_pSHMBuf->chMsgBuf) < 1)
            {
                // 2. 검색 IP 입력 
                if (ext_nUsingTestNodeIdx >= 0) // 패킷카운트를 사용할 경우
                {
                    // 2. 검색 IP 입력 
                    sprintf_s(chNodeIPAddrBuf, sizeof(chNodeIPAddrBuf), "%s", ext_tConfig.tSubNodes[ext_nUsingTestNodeIdx].chIPAddrBuf);
                    sprintf_s(chNodePortBuf, sizeof(chNodePortBuf), "%d", ext_tConfig.tSubNodes[ext_nUsingTestNodeIdx].nPort);
                }
                else
                {
                    // 2. 검색 IP 입력 
                    sprintf_s(chNodeIPAddrBuf, sizeof(chNodeIPAddrBuf), "%s", ext_tConfig.tSubNodes[i].chIPAddrBuf);
                    sprintf_s(chNodePortBuf, sizeof(chNodePortBuf), "%d", ext_tConfig.tSubNodes[i].nPort);
                }

                // 공유메모리에 기록한다. 
                ext_pSHMBuf->nCnt = 1;
                sprintf_s(ext_pSHMBuf->chMsgBuf, "%s", chNodePortBuf);
                JC_Print(INFO_MODE, "CHORD> 노드 찾기 %3d_Space(%u, %u), (%s:%d)>"
                    , i, DEF_BASE_M, DEF_ID_SPACE, chNodeIPAddrBuf, atoi(chNodePortBuf));


                // 로그 기록
                ShareWriteResultLog(ext_chResultLogFile, "\nNode(%d),%3d,%s:%d - %s:%d > "
                    , DEF_BASE_M, i
                    , inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port)
                    , chNodeIPAddrBuf, atoi(chNodePortBuf));

                // 1. 노드 검색 처리
                ProcessSearchNode(chNodeIPAddrBuf, chNodePortBuf);

                i++; // 인덱스 증가
            }
            else 
            {
                if (ext_pSHMBuf->nCnt > DEF_MAX_HOP)
                {
                    ShareWriteResultLog(ext_chResultLogFile, " # Fail(%d)", ext_pSHMBuf->nCnt);
                    continue;
                }
                Sleep(DEF_RESULT_LOG_CAP_TIME);
            }

        } while (i < nSendTestCnt); // 조건 검사
    }
#else
    for (int i = 0; i < nSendTestCnt; i++)
    {
        // 2. 검색 IP 입력 
        if (ext_nUsingTestNodeIdx >= 0) // 패킷카운트를 사용할 경우
        {
            sprintf(chNodeIPAddrBuf, "%s", ext_tConfig.tSubNodes[ext_nUsingTestNodeIdx].chIPAddrBuf);
            sprintf(chNodePortBuf, "%d", ext_tConfig.tSubNodes[ext_nUsingTestNodeIdx].nPort);
        }
        else
        {
            sprintf(chNodeIPAddrBuf, "%s", ext_tConfig.tSubNodes[i].chIPAddrBuf);
            sprintf(chNodePortBuf, "%d", ext_tConfig.tSubNodes[i].nPort);
        }
        JC_Print(INFO_MODE, "CHORD> 노드 찾기 %3d_Space(%u, %u), (%s:%d)>"
            , i, DEF_BASE_M, DEF_ID_SPACE, chNodeIPAddrBuf, atoi(chNodePortBuf));
        // 순서, NODE 갯수, Finger 범위, 검색 노드
        ShareWriteResultLog(ext_chResultLogFile, "\nNode(%d),%3d,%s:%d - %s:%d > "
            , DEF_BASE_M, i
            , inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port)
            , chNodeIPAddrBuf, atoi(chNodePortBuf));


        // 1. 5000 ~ 5001을 찾음
        // 검색하기 위한 노드에 대한 처리
        ProcessSearchNode(chNodeIPAddrBuf, chNodePortBuf);
#if JC_DEBUG_SENT_DELAY_KEY
        {
            fprintf(stderr, "\n계속하려면 아무 키나 누르십시오...");
            system("pause");
        }
#else 
        // 10 - 5
        Sleep(DEF_RESULT_LOG_CAP_TIME);
#endif 
#endif 
        return 1;
}
    
int SearchNodeTestByPacket()
{
    return 0;
}