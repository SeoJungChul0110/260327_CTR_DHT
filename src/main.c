#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/JC_Test.h"
#include "../include/procUtil.h"
#include "../include/UserCommand.h"


SharedData* ext_pSHMBuf;

int main(int _nArgc, char *_ppArgv[])
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	JC_Print(INFO_MODE, "\n\n[## START] Main");

	WSADATA tWsaData;
	HANDLE hThread[2];
	int nExitFlag = 0; // indicates termination condition
	char chCmdChar = '\0';
	int nJoinFlag = 0; // indicates the join/create status
	char chSockAddrBuf[21];
	int nOptVal = 5000;  // 5 seconds
	int nRetVal; // return value
	nodeInfoType tSuccNodeInfo = { 0 }, tPredNodeInfo = { 0 }, tTargetNodeInfo = { 0 };
	FILE *pFileFp = NULL;
	char* pchBody = NULL;

	/* step 0: Program Initialization  */
	/* step 1: Commnad line argument handling  */
	/* step 2: Winsock handling */
	/* step 3: Prompt handling (loop) */
	/* step 4: User input processing (switch) */
	/* step 5: Program termination */

	/* step 1: Commnad line argument handling  */
	char * pchIPAddr = "127.0.0.1";
	char * pchPort = NULL;

	// 인자값 확인 
	for (int i = 0; i < _nArgc; i++)
		fprintf(stderr, "\n%2d > (%s)",i,  _ppArgv[i]);

	if ((strcmp(_ppArgv[1], "c") == 0 || strcmp(_ppArgv[1], "C") == 0)
		|| (strcmp(_ppArgv[1], "j") == 0 || strcmp(_ppArgv[1], "J") == 0))
	{
		if (_nArgc == 4) // iapl_chord.exe c 127.0.0.1 5000
		{
			pchIPAddr = _ppArgv[2];
			pchPort = _ppArgv[3];
		}
		else if (_nArgc == 3) // iapl_chord.exe c 5000
		{
			pchPort = _ppArgv[2];
		}
		else
		{
			JC_Print(ERROR_MODE, "[ERROR] Usage : (c | j) <IP Addr> <Port No>!");
			exit(1);
		}
	}
	else
	{
		if (_nArgc == 3) // iapl_chord.exe 127.0.0.1 5000
		{
			pchIPAddr = _ppArgv[1];
			pchPort = _ppArgv[2];
		}
		else if (_nArgc == 2) // iapl_chord.exe 5000
		{
			pchPort = _ppArgv[1];
		}
		else
		{
			JC_Print(ERROR_MODE, "[ERROR] Usage : <IP Addr> <Port No>!");
			exit(1);
		}
	}


	//--------------
	char chLogFileName[1024];
	//memset(chLogFileName, 0, sizeof(chLogFileName));
	//sprintf_s(chLogFileName, sizeof(chLogFileName), (const char *)"dir ..\\log\\"); // /* */
	//system(chLogFileName);

	memset(chLogFileName, 0, sizeof(chLogFileName));
	sprintf_s(chLogFileName, sizeof(chLogFileName), DEF_DEBUG_LOG_FILE_NAME, pchIPAddr, pchPort);
	OpenDebugLogFile(chLogFileName);
	// 설정 정보 읽어 오기 (sub node, log_level, result log
	if (LoadConfigSubNode(DEF_SUB_NODE_CONFIG_FILE, &ext_tConfig) == 0)
	{
		JC_Print(DEBUG_MODE, "Config] Sub NODE_CNT = %d", ext_tConfig.nNodeCnt);
		for (int i = 0; i < ext_tConfig.nNodeCnt; i++)
		{
			JC_Print(DEBUG_MODE, "\tNODE_%d = [%d] %s:%d", i,
				ext_tConfig.tSubNodes[i].unID,
				ext_tConfig.tSubNodes[i].chIPAddrBuf, ext_tConfig.tSubNodes[i].nPort);
		}
	}
	else
	{
		JC_Print(ERROR_MODE, "Failed to load config\n");
	}

	// 로그레벨 설정 
	SetDebugLogLevel(ext_nLogLevel);

	// 결과 로그 설정 ..
	OpenResultLogFile(ext_chResultLogFile);

	//JC_RESULT("TEST] %s:%d > ", pchIPAddr, atoi(pchPort));
	//ShareWriteResultLog(ext_chResultLogFile, "\nSTART] %s:%d > ", pchIPAddr, atoi(pchPort));

	// 수신한 검색 노드의 IP와 포트를 출력
	//AddResultLog(ext_chResultLogFile, "TEST] % s:% d > ", pchIPAddr, atoi(pchPort));
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	HANDLE hMapFile;
	// 1. 공유 메모리 객체 생성
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // 물리적 파일이 아닌 페이징 파일 사용
		NULL,                    // 기본 보안 속성
		PAGE_READWRITE,          // 읽기/쓰기 권한
		0,                       // 최대 크기(상위 32비트)
		SHM_SIZE,                // 최대 크기(하위 32비트)
		SHM_NAME                 // 공유 메모리 이름
	);

	if (hMapFile == NULL) {
		printf("공유 메모리 생성 실패: %d\n", GetLastError());
		return 1;
	}
	// 2. 메모리를 현재 프로세스 주소 공간에 매핑
	ext_pSHMBuf = (char*)MapViewOfFile(
		hMapFile,               // 공유 메모리 핸들
		FILE_MAP_ALL_ACCESS,    // 전체 접근 권한
		0, 0,                   // 오프셋
		SHM_SIZE                // 매핑할 크기
	);

	if (ext_pSHMBuf == NULL) {
		printf("메모리 매핑 실패: %d\n", GetLastError());
		CloseHandle(hMapFile);
		return 1;
	}

	ext_tMyNode.tNodeInfo.tAddrInfo.sin_family = AF_INET;
	//--------------
	if ((ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr.s_addr = inet_addr(pchIPAddr)) == INADDR_NONE) {
		JC_Print(ERROR_MODE, "\a[ERROR] <IP Addr> is wrong!");
		exit(1);
	}
	ext_tMyNode.tNodeInfo.tAddrInfo.sin_port = htons(atoi(pchPort));

	strcpy(chSockAddrBuf, pchIPAddr);
	strcat(chSockAddrBuf, pchPort);
	JC_Print(INFO_MODE, "strSoclAddr: %s",  chSockAddrBuf);
	ext_tMyNode.tNodeInfo.unID = strHash(chSockAddrBuf);


	JC_Print(INFO_MODE, ">>> Welcome to ChordNode Program! ");
	JC_Print(INFO_MODE, ">>> Your IP address: %s, Port No: %d, ID: %d", pchIPAddr, atoi(pchPort), ext_tMyNode.tNodeInfo.unID);
	JC_Print(INFO_MODE, ">>> Silent Mode is ON!");

	/* step 2: Winsock handling */
	if (WSAStartup(MAKEWORD(2, 2), &tWsaData) != 0) { /* Load Winsock 2.2 DLL */
		JC_Print(ERROR_MODE, "\a[ERROR] WSAStartup() error!");
		exit(1);
	}
	ext_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (ext_hMutex == NULL) {
		JC_Print(ERROR_MODE, "\a[ERROR] CreateMutex() error!");
		exit(1);
	}

	ext_tRecvProcSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (ext_tRecvProcSock == INVALID_SOCKET) err_quit("rp socket()");
	//{
	//	// 소켓 Non-blocking 설정
	//	u_long mode = 1;
	//	ioctlsocket(ext_tRecvProcSock, FIONBIO, &mode);
	//}

	ext_flSock = socket(PF_INET, SOCK_STREAM, 0);
	if (ext_flSock == INVALID_SOCKET) err_quit("fl socket()");

	ext_tReqSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (ext_tReqSock == INVALID_SOCKET) err_quit("rq socket()");

	ext_frSock = socket(PF_INET, SOCK_STREAM, 0);
	if (ext_frSock == INVALID_SOCKET) err_quit("fr socket()");

	nRetVal = setsockopt(ext_tReqSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nOptVal, sizeof(nOptVal));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(ERROR_MODE, "\a[ERROR] setsockopt() Error!");
		exit(1);
	}

	// Node Info 설정 
	ZeroMemory(&ext_tMyNode.tNodeInfo.tAddrInfo, sizeof(ext_tMyNode.tNodeInfo.tAddrInfo));
	ext_tMyNode.tNodeInfo.tAddrInfo.sin_family = AF_INET;
	ext_tMyNode.tNodeInfo.tAddrInfo.sin_port = htons(atoi(pchPort));
	ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr.s_addr = inet_addr(pchIPAddr);//htonl(INADDR_ANY);
	//ext_tMyNode.tNodeInfo.unID = -1;
	JC_Print(INFO_MODE, "%s, %d", inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port));
	nRetVal = bind(ext_tRecvProcSock, (SOCKADDR *)&ext_tMyNode.tNodeInfo.tAddrInfo,
		sizeof(ext_tMyNode.tNodeInfo.tAddrInfo));
	if (nRetVal == SOCKET_ERROR) err_quit("rpSock bind()");

	nRetVal = bind(ext_flSock, (SOCKADDR *)&ext_tMyNode.tNodeInfo.tAddrInfo,
		sizeof(ext_tMyNode.tNodeInfo.tAddrInfo));
	if (nRetVal == SOCKET_ERROR) err_quit("flSock bind()");

	nRetVal = listen(ext_flSock, SOMAXCONN);
	if (nRetVal == SOCKET_ERROR)
		ErrorHandling("listen() error");

#if JC_DEBUG_CREAT
	printNodeType(&ext_tMyNode);
#endif 
#if JC_USING_LOAD_SUB_NODE_FOR_TEST
	// 자동 실행
	if ((strcmp(_ppArgv[1], "c") == 0 || strcmp(_ppArgv[1], "C") == 0))
	{
		// Create
		chCmdChar = 'c';
		procUserCommand(chCmdChar);

		if (ext_nUsingLoadSubNodeForTest == 1) // 테스트 모드를 사용하면 
		{
			chCmdChar = 'n';
			procUserCommand(chCmdChar);
		}
	}
	else if ((strcmp(_ppArgv[1], "j") == 0 || strcmp(_ppArgv[1], "J") == 0))
	{
		// Join
		if (ext_nUsingLoadSubNodeForTest == 1) // 테스트 모드를 사용하면 
		{
			chCmdChar = 'n';
			procUserCommand(chCmdChar);
		}


		// Join
		chCmdChar = 'j';
		procUserCommand(chCmdChar);


#if 0
		// Add File
		chCmdChar = 'a';
		procUserCommand(chCmdChar);
#endif 
	}
#endif 
	showCommand();

	//3번전에 스레드를 이용해서 소켓통신 데이터를 받을 수 있도록 해야한다.
	/* step 3: Prompt handling (loop) */
	//세번째 인자인 함수 이름 앞에 (void *)를 넣으세요.
	//네번째 인자는 메인 쓰레드가 종료될 때(Q입력시) Flag를 1로 만들어
	//다른 쓰레드도 같이 종료되도록 만드는데 사용합니다.
	do {
		/* step 4: User input processing (switch) */
		chCmdChar = getUserCommand();

		// 입력에 따른 프로세스 처리 
		procUserCommand(chCmdChar);

	} while (chCmdChar != 'q');

	/* step 5: Program termination */

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	UnmapViewOfFile(ext_pSHMBuf);
	CloseHandle(hMapFile);

	CloseLogFile();
	CloseResultLogFile();

	closesocket(ext_tReqSock);
	closesocket(ext_tRecvProcSock);
	closesocket(ext_frSock);
	closesocket(ext_flSock);
	closesocket(ext_fsSock);
	closesocket(ext_pfSock);
	CloseHandle(ext_hMutex);

	WSACleanup();

	JC_Print(INFO_MODE, "*************************  B  Y  E  *****************************");

	JC_Print(INFO_MODE, "[## END] Main\n\n");
	return 0;
}
