#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/procRecvMsg.h"
#include "../include/JC_Test.h"

int ext_bUsePingAndFixFinger = 1;


// thread function for handling receiving messages 
void procRecvMsg(void* _pvArg)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	struct sockaddr_in tPeerSockAddr;
	chordHeaderType tSendMsgHead, tRecvMsgHead;
	nodeInfoType tSuccNodeInfo, tPredNodeInfo;
	nodeType tPreNode = { 0 };

	int nOptVal = 5000;  // 5 seconds
	int nRetVal; // return value
	fileInfoType tFileInfo;
	char chFileBuf[DEF_FILE_BUF_LEN];
	char chFileNameBuf[DEF_FILE_NAME_MAX_LEN + 1];
	FILE* pFileFp = NULL;
	char* pchBody = NULL;
	struct sockaddr_in tServerSockAddr;
	int nKeyCnt;
	int i, j, nTargetKey, nResultCode, nAddrSize, nReadLen, nTotalLen;
	int* pnExitFlag = (int*)_pvArg;
	int nTotalBytes;
	nAddrSize = sizeof(tPeerSockAddr);


	while (!(*pnExitFlag)) 
	{
		//memset(&tempMsg, 0, sizeof(tempMsg));
		memset(&tPeerSockAddr, 0, sizeof(tPeerSockAddr));
		// 1. Non-blocking 수신 시도
		nRetVal = recvfrom(ext_tRecvProcSock, (char*)&tRecvMsgHead, sizeof(tRecvMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, &nAddrSize);

		//fprintf(stderr, "\n ---- 1(nRetVal:%d)", nRetVal);
		if (nRetVal == SOCKET_ERROR) 
		{
			int nErr = WSAGetLastError(); // 또는 errno

			// 데이터가 아직 도착하지 않은 경우 (Non-blocking 핵심 처리)
			if (nErr == WSAEWOULDBLOCK)
			{
				// CPU 점유율 폭주 방지를 위해 아주 짧은 휴식 또는 다른 작업 수행
				Sleep(10);
				continue;
			}

			// 실제 통신 에러 발생 시
			continue;
		}


		// 2. 메시지 타입 체크 (기존과 동일)
		// 요청메시지가 아니면 스킵
		if (tRecvMsgHead.usMsgType  != DEF_HEAD_MSG_TYPE_REQ) 
		{
			JC_Print(ERROR_MODE, "\a[ERROR] Unexpected Response Message Received. Therefore Message Ignored!");
			JC_DumpMsg((char*)&tRecvMsgHead, nRetVal);
			continue;
		}

		//JC_Print(INFO_MODE, "\n msgID :%d(DataLen:%d)", tRecvMsgHead.usMsgID, nRetVal);
		// 3. 메시지 처리 (Switch 문)
		switch (tRecvMsgHead.usMsgID) 
		{
			JC_Print(INFO_MODE, "\n Switch >>> ");
			case DEF_HEAD_MSG_ID_PING: // 0
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_PING (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if 0 //JC_DEBUG_RECV && (JC_DEBUG_PING_FIX)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_PING"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_PING;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR)
				{
					JC_Print(WARN_MODE, "Pingpong response Sendto error(%s:%d)!"
						, inet_ntoa(tPeerSockAddr.sin_addr)
						, ntohs(tPeerSockAddr.sin_port));
					exit(1);
				}
				break;
			}
			case DEF_HEAD_MSG_ID_JOIN: //1:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_JOIN (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_JOIN"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;
				tSuccNodeInfo = FindSuccessor(ext_tReqSock, tRecvMsgHead.tNodeInfo.unID);
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_JOIN;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.tNodeInfo = tSuccNodeInfo;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;

#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_JOIN, %s:%d) - tSuccNodeInfo(%d, %s:%d)] "
					, __FUNCTION__
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port)
					, tSuccNodeInfo.unID, inet_ntoa(tSuccNodeInfo.tAddrInfo.sin_addr), ntohs(tSuccNodeInfo.tAddrInfo.sin_port));
#endif 
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Join Info response Sendto error!");
					exit(1);
				}

				//JC_Print(INFO_MODE, "Succ : %s",  inet_ntoa(succNode.tAddrInfo.sin_addr));
#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				printNodeType(&ext_tMyNode);
#endif 
				break;

			}
			case DEF_HEAD_MSG_ID_MOVE_KEY: //2:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_MOVE_KEY (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_MOVE_KEY - Recv(NodeID:%d)"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port), tRecvMsgHead.tNodeInfo.unID);
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));

#endif 
				nKeyCnt = 0;
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));

				for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++)
				{
					if (modIn(DEF_ID_SPACE, tRecvMsgHead.tNodeInfo.unID, ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey, ext_tMyNode.tNodeInfo.unID, 1, 0))
					{
						tFileInfo.tFileRef[nKeyCnt] = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i];
						tSendMsgHead.unBodySize += sizeof(tFileInfo.tFileRef[nKeyCnt]);
						nKeyCnt++;

						for (j = i; j < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum - 1; j++)
						{
							WaitForSingleObject(ext_hMutex, INFINITE);
							ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[j] = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[j + 1];
							ReleaseMutex(ext_hMutex);
						}
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tChordInfo.tFRefInfo.nFileNum--;
						ReleaseMutex(ext_hMutex);
						i--;// 배열 땡겼으니까
					}
				}
				pchBody = &tFileInfo.tFileRef[0];
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_MOVE_KEY;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nKeyCnt;

#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				//:76
				JC_Print(DEBUG_MODE, "fileRefType Size :%d, tFileInfo.tFileRef[nKeyCnt]:%d", sizeof(fileRefType), sizeof(tFileInfo.tFileRef[0]));
				JC_Print(DEBUG_MODE, "[%s] 1-1. ## SendTo(DEF_HEAD_MSG_ID_MOVE_KEY, %s:%d) - moreInfo[KeyCnt : %d, bodySize:%d] "
					, __FUNCTION__
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port), tSendMsgHead.sMoreInfo, tSendMsgHead.unBodySize);
#endif 
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Movekeys tempMsg response Sendto error!");
					exit(1);
				}


				nRetVal = recvfrom(ext_tRecvProcSock, (char*)&tRecvMsgHead, sizeof(tRecvMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, &nAddrSize);
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Movekeys tempMsg response Recvfrom error!");
					exit(1);
				}
#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "[%s] 1-2. ## Recv(DEF_HEAD_MSG_ID_MOVE_KEY, %s:%d) moreInfo[ID:%d]"
					, __FUNCTION__
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port)
					, tRecvMsgHead.tNodeInfo.unID);
#endif 

#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "[%s] 2. ## SendTo(DEF_HEAD_MSG_ID_MOVE_KEY, %s:%d) - BodySize : %d] "
					, __FUNCTION__
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port), tSendMsgHead.unBodySize);
#endif 

				nRetVal = sendto(ext_tRecvProcSock, (char*)pchBody, tSendMsgHead.unBodySize, 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Movekeys body response Sendto error!");
					exit(1);
				}
				break;

			}
			case DEF_HEAD_MSG_ID_GET_PRED: //3:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_GET_PRED (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_GET_PRED"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				//WaitForSingleObject(hMutex, INFINITE);
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_GET_PRED;//받는쪽도 msgID 4인걸 확인하고 받도록 해준다.
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.tNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tPre;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;


#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "[%s] SendTo(DEF_HEAD_MSG_ID_GET_PRED, %s:%d) [nodeInfo.ID:%d, moreInfo:%d] "
					, __FUNCTION__
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port)
					, tSendMsgHead.tNodeInfo.unID, tSendMsgHead.sMoreInfo);
#endif 
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Predecessor Info response Sendto error!");
					exit(1);
				}
				//ReleaseMutex(hMutex);
				break;

			}
			case DEF_HEAD_MSG_ID_UPD_PRED:// 4:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_UPD_PRED (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_UPD_PRED [nodeinfo.ID:%d]"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port)
					, tRecvMsgHead.sMoreInfo);
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				//WaitForSingleObject(hMutex, INFINITE);
				notify(tRecvMsgHead.tNodeInfo);
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_UPD_PRED;//받는쪽도 msgID 4인걸 확인하고 받도록 해준다.
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;

#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "[%s] SendTo(DEF_HEAD_MSG_ID_UPD_PRED, %s:%d) [moreInfo:%d] "
					, __FUNCTION__
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port)
					, tSendMsgHead.sMoreInfo);
#endif 
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Predecessor Update response Sendto error!");
					exit(1);
				}
				//ReleaseMutex(hMutex);
				break;

			}
			case DEF_HEAD_MSG_ID_FIND_SUCC: //5:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_FIND_SUCC (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_FIND_SUCC"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_FIND_SUCC;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.tNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0];
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;

				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Successor Info response Sendto error!");
					exit(1);
				}
				break;

			}
			case DEF_HEAD_MSG_ID_UPD_SUCC: //6:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_UPD_SUCC (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_UPD_SUCC"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				WaitForSingleObject(ext_hMutex, INFINITE);
				ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0] = tRecvMsgHead.tNodeInfo;
				ReleaseMutex(ext_hMutex);
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_UPD_SUCC;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;

#if JC_DEBUG_RECV && (JC_DEBUG_JOIN)
				JC_Print(DEBUG_MODE, "[%s] SendTo(DEF_HEAD_MSG_ID_UPD_SUCC, %s:%d) [moreInfo:%d] "
					, __FUNCTION__
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port)
					, tSendMsgHead.sMoreInfo);
#endif 
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Successor Update response Sendto error!");
					exit(1);
				}
				break;
			}
			case DEF_HEAD_MSG_ID_FIND_PRED: //7:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_FIND_PRED (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_FIND_PRED"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				tPredNodeInfo = find_predecessor(ext_tReqSock, tRecvMsgHead.tNodeInfo.unID);
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_FIND_PRED;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.tNodeInfo = tPredNodeInfo;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;

				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Find Predecessor response Sendto error!");
					exit(1);
				}
				break;
			}
			case DEF_HEAD_MSG_ID_LEAVE_KEY: //8:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_LEAVE_KEY (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_LEAVE_KEY"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				/*for (i = 0; i < myNode.fileInfo.fileNum; i++)
				{
					JC_Print(INFO_MODE, "CHORD> %dth own file name: %s, key: %d",  i + 1, myNode.fileInfo.fileRef[i].Name, myNode.fileInfo.fileRef[i].Key);
				}*/
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_LEAVE_KEY;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;

				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "LeaveKeys response Sendto error!");
					exit(1);
				}
				JC_Print(INFO_MODE, "bufsize : %d", tRecvMsgHead.unBodySize);
				pchBody = (char*)malloc(sizeof(char) * tRecvMsgHead.unBodySize);//이 메모리 영역을 할당해주어야 한다 그렇지 않으면 바로 밑에서 body가 값을 받아올때 뒤에 어떤 배열이 있을 수 있는데 그 배열에 덮어쓰게 되어버린다. 그래서 받아오는 곳의 공간을 확보해주어야 한다!!!
				nRetVal = recvfrom(ext_tRecvProcSock, (char*)pchBody, tRecvMsgHead.unBodySize, 0, (struct sockaddr*)&tPeerSockAddr, &nAddrSize);
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "LeaveKeys response body Recvfrom error!");
					exit(1);
				}

				JC_Print(INFO_MODE, "CHORD> keyCount: %d, bodySize: %d", tRecvMsgHead.sMoreInfo, tRecvMsgHead.unBodySize);
				JC_Print(INFO_MODE, "CHORD> RefFileNum: %d", ext_tMyNode.tChordInfo.tFRefInfo.nFileNum);

				/*for (i = 0; i < myNode.fileInfo.fileNum; i++)
				{
					JC_Print(INFO_MODE, "CHORD> %dth own file name: %s, key: %d",  i + 1, myNode.fileInfo.fileRef[i].Name, myNode.fileInfo.fileRef[i].Key);
				}*/
				for (i = 0; i < tRecvMsgHead.sMoreInfo; i++) {
					WaitForSingleObject(ext_hMutex, INFINITE);
					//memcpy(&ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[ext_tMyNode.tChordInfo.tFRefInfo.nFileNum], pchBody, 76);
					memcpy(&ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[ext_tMyNode.tChordInfo.tFRefInfo.nFileNum], pchBody, sizeof(fileRefType));
					JC_Print(INFO_MODE, "CHORD> Name: %s, Key: %d", ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[ext_tMyNode.tChordInfo.tFRefInfo.nFileNum].chNameBuf, ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[ext_tMyNode.tChordInfo.tFRefInfo.nFileNum].nKey);
					ext_tMyNode.tChordInfo.tFRefInfo.nFileNum++;
					ReleaseMutex(ext_hMutex);
					pchBody = pchBody + sizeof(fileRefType);// 76;
				}

				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "LeaveKeys response body Sendto error!");
					exit(1);
				}
				break;
			}
			case DEF_HEAD_MSG_ID_ADD_FILE_REF: //9:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_ADD_FILE_REF (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_ADD_FILE_REF"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;
				if (ext_tMyNode.tChordInfo.tFRefInfo.nFileNum == DEX_FILE_MAX_CNT) { // file ref number is full 
					JC_Print(INFO_MODE, "\a[ERROR] My Node Cannot Add more file ref info. File Ref Space is Full!");
					nResultCode = -1;
				}
				WaitForSingleObject(ext_hMutex, INFINITE);
				// file ref info update
				ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[ext_tMyNode.tChordInfo.tFRefInfo.nFileNum] = tRecvMsgHead.tFileIRef;
				ext_tMyNode.tChordInfo.tFRefInfo.nFileNum++;
				ReleaseMutex(ext_hMutex);

				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_ADD_FILE_REF;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "File Reference Add tempMsg response Sendto error!");
					exit(1);
				}
				break;

			}
			case DEF_HEAD_MSG_ID_DEL_FILE_REF:// 10:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_DEL_FILE_REF (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_DEL_FILE_REF"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				nTargetKey = tRecvMsgHead.sMoreInfo;

				for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++) {
					if (ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey == nTargetKey) {
						for (j = i; j < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum - 1; j++) {
							WaitForSingleObject(ext_hMutex, INFINITE);
							ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[j] = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[j + 1];
							ReleaseMutex(ext_hMutex);
						}
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tChordInfo.tFRefInfo.nFileNum--;
						ReleaseMutex(ext_hMutex);
						i--;// 배열 땡겼으니까
					}
				}
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_DEL_FILE_REF;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;
				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "File Reference Delete tempMsg response Sendto error!");
					exit(1);
				}

				break;
			}
			case DEF_HEAD_MSG_ID_DOWNLOAD_FILE: //11:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_DOWNLOAD_FILE (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_DOWNLOAD_FILE"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				nTargetKey = tRecvMsgHead.sMoreInfo;

				for (i = 0; i < ext_tMyNode.tFileInfo.nFileNum; i++)
				{
					if (nTargetKey == ext_tMyNode.tFileInfo.tFileRef[i].nKey) {
						ZeroMemory(chFileNameBuf, DEF_FILE_NAME_MAX_LEN);
						WaitForSingleObject(ext_hMutex, INFINITE);
						strcpy(chFileNameBuf, ext_tMyNode.tFileInfo.tFileRef[i].chNameBuf);
						ReleaseMutex(ext_hMutex);
						pFileFp = fopen(chFileNameBuf, "rb");
						if (pFileFp == NULL) {
							perror("파일 입출력 오류");
							return;
						}
						break;
					}
				}

				fseek(pFileFp, 0, SEEK_END);
				nTotalBytes = ftell(pFileFp);

				JC_Print(INFO_MODE, "totalbytes: %d", nTotalBytes);

				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_DOWNLOAD_FILE;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = nTotalBytes;

				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "File  Down response Sendto error!");
					exit(1);
				}

				nAddrSize = sizeof(tServerSockAddr);
				ext_fsSock = accept(ext_flSock, (struct sockaddr*)&tServerSockAddr, &nAddrSize);//지금 여기서 블락킹 되있는것 같은데
				if (ext_fsSock == INVALID_SOCKET) {
					err_display("accept()");
					exit(1);
				}

				nTotalLen = 0;

				rewind(pFileFp);
				while (1) {
					nReadLen = (int)fread(chFileBuf, 1, DEF_FILE_BUF_LEN, pFileFp);
					//JC_Print(INFO_MODE, "numRead : %d",  numRead);
					if (nReadLen > 0) {
						nRetVal = send(ext_fsSock, chFileBuf, nReadLen, 0);
						if (nRetVal == SOCKET_ERROR) {
							err_display("send()");
							break;
						}
						nTotalLen += nReadLen;
					}
					else if (nReadLen == 0 && nTotalLen == nTotalBytes) {
						JC_Print(INFO_MODE, "파일 전송 완료!: %d 바이트", nTotalLen);
						break;
					}
					else {
						perror("파일 입출력 오류");
						break;
					}
				}
				break;
			}
			case DEF_HEAD_MSG_ID_GET_FILE_REF: //12:
			{
				// fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_GET_FILE_REF (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_GET_FILE_REF"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				// // JC_DumpMsg((char*)&tMsgHead, sizeof(tMsgHead));
#endif 
				nResultCode = 0;

				nTargetKey = tRecvMsgHead.sMoreInfo;
				for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++) {
					if (nTargetKey == ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey) {
						WaitForSingleObject(ext_hMutex, INFINITE);
						tSendMsgHead.tNodeInfo = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].tOwnerNodeInfo;
						ReleaseMutex(ext_hMutex);
						break;
					}
				}
				memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
				tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_GET_FILE_REF;
				tSendMsgHead.usMsgType = DEF_HEAD_MSG_TYPE_RSP;
				tSendMsgHead.sMoreInfo = nResultCode;
				tSendMsgHead.unBodySize = 0;

				nRetVal = sendto(ext_tRecvProcSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPeerSockAddr, sizeof(tPeerSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "File Reference Info response Sendto error!");
					exit(1);
				}
				break;

			}
			case DEF_HEAD_MSG_ID_FIND_NODE: //13:
			{
				fprintf(stderr, "\n Switch >>> >>> DEF_HEAD_MSG_ID_FIND_NODE (nRetVal:%d )", tRecvMsgHead.usMsgID);
#if JC_DEBUG_RECV && (1)
				JC_Print(DEBUG_MODE, "RecvFrom(%s:%d) DEF_HEAD_MSG_ID_FIND_NODE"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				JC_DumpMsg((char*)&tRecvMsgHead, sizeof(tRecvMsgHead));
#endif 


				//# 검색하는 노드 정보 확인
				char chSendNodeIPAddrBuf[16];
				char chSendNodePortBuf[6];
				sprintf(chSendNodeIPAddrBuf, "%s", inet_ntoa(tRecvMsgHead.tNodeInfo.tAddrInfo.sin_addr));
				sprintf(chSendNodePortBuf, "%d", ntohs(tRecvMsgHead.tNodeInfo.tAddrInfo.sin_port));

				JC_Print(INFO_MODE, "Recv Head %d bytes([SENDER NODE] (ID:%d) (%s:%s)" 
					, tRecvMsgHead.unBodySize
					, tRecvMsgHead.tNodeInfo.unID
					, chSendNodeIPAddrBuf, chSendNodePortBuf);
				
				// 수신 패킷 수 계산
				if (ext_nUsingPacketCntForTest > 0)
				{
					for (i = 0; i < DEF_BASE_M; i++)
					{
						if (ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID == tRecvMsgHead.tNodeInfo.unID)
						{
							ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nRecvPacketCnt++;
						}

					}

					//// LoadConfigFile에서 등록
					//for (int i = 0; i < ext_tConfig.nNodeCnt; i++)//MAX_TEST_SUB_NODE_CNT
					//	if (ext_tConfig.tSubNodes[i].unID == tRecvMsgHead.tNodeInfo.unID)
					//	{
					//		ext_tConfig.tSubNodes[i].nRecvPacketCnt++;
					//	}
				}

				if (tRecvMsgHead.unBodySize > 0)
				{
					nodeInfoType tSearchNodeInfo = { 0 };
					memset(&tPeerSockAddr, 0, sizeof(tPeerSockAddr));
					nRetVal = recvfrom(ext_tRecvProcSock, (char*)&tSearchNodeInfo, tRecvMsgHead.unBodySize, 0, (struct sockaddr*)&tPeerSockAddr, &nAddrSize);


					char chSearchNodeIPAddrBuf[16];
					char chSearchNodePortBuf[6];
					sprintf(chSearchNodeIPAddrBuf, "%s", inet_ntoa(tSearchNodeInfo.tAddrInfo.sin_addr));
					sprintf(chSearchNodePortBuf, "%d", ntohs(tSearchNodeInfo.tAddrInfo.sin_port));


					JC_Print(INFO_MODE, "Recv Body %d bytes [SEARCH NODE] (ID:%d)(%s:%s)"
						, nRetVal, tSearchNodeInfo.unID
						, chSearchNodeIPAddrBuf
						, chSearchNodePortBuf);
					//JC_DumpMsg((char*)&tNodeInfo, nRetVal);
					// 현재의 FingerTable에서 재검색 
					// 검색하기 위한 노드에 대한 처리
					ProcessSearchNode(chSearchNodeIPAddrBuf, chSearchNodePortBuf);

					JC_Print(INFO_MODE, "End ProcessSearchNode");
				}
				
			}
			break;
			default:
			{
				// fprintf(stderr, "\n Switch >>> >>> default (nRetVal:%d )", tRecvMsgHead.usMsgID);
				JC_Print(WARN_MODE, "RecvFrom(%s:%d) NONEMSG"
					, inet_ntoa(tPeerSockAddr.sin_addr)
					, ntohs(tPeerSockAddr.sin_port));
				JC_DumpMsg((char*)&tRecvMsgHead, sizeof(tRecvMsgHead));
			
			}
			break;
		} // end case

		//fprintf(stderr, "\n Switch end");
		
		if (ext_nSilentMode == 0) 
		{
			JC_Print(INFO_MODE, "CHORD> ");
			JC_Print(INFO_MODE, "CHORD> ");
		}
		memset(&tRecvMsgHead, 0, sizeof(tRecvMsgHead));
	} //end while 
}



int proSendMsg(char* _pchServerIP, char* _pchServerPort, chordHeaderType* _ptMsgHead, int _nDataLen)
{

	int nSock;
	struct sockaddr_in tServerAddr;
	int nSendLen = -1;

	// --- WinSock 초기화 ---
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup() failed: %d\n", WSAGetLastError());
		return -1;
	}

	JC_Print(INFO_MODE, "IP(%s)(%s)", _pchServerPort, _pchServerIP);
	// 소켓 생성
	nSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (nSock < 0) {
		perror("socket() failed");
		return 1;
	}
	// 목적지 주소 설정
	memset(&tServerAddr, 0, sizeof(tServerAddr));
	tServerAddr.sin_family = AF_INET;
	tServerAddr.sin_addr.s_addr = inet_addr(_pchServerIP);
	tServerAddr.sin_port = htons(atoi(_pchServerPort));

	//printf("메시지 입력(q 입력 시 종료): ");
	//fgets(buffer, sizeof(buffer), stdin);
	JC_DumpMsg((char*)_ptMsgHead, _nDataLen);
	// 메시지 전송
	int nServerAddrLen = sizeof(tServerAddr);
	JC_Print(WARN_MODE, "sendto(%s:%d)!"
		, inet_ntoa(tServerAddr.sin_addr)
		, ntohs(tServerAddr.sin_port));
	nSendLen = sendto(nSock, (const char*)_ptMsgHead, _nDataLen, 0, (struct sockaddr*)&tServerAddr, nServerAddrLen);
	if (nSendLen < 0) {
		perror("sendto() failed");
	}
	// --- 소켓 종료 ---
	closesocket(nSock);
	WSACleanup();

	JC_Print(INFO_MODE, "전송 완료: %d\n", nSendLen);
	//Sleep(DEF_TIME_PING);

	return nSendLen;
}
