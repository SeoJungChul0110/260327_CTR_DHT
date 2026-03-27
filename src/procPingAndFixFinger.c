#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/procPingAndFixFinger.h"




void procPingAndFixFinger(void* _pvArg)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 

	int* pnExitFlag = (int*)_pvArg;
	unsigned int unDelayTime, unVarTime;
	int nRetVal, nOptVal = 5000;
	int i;
	int nIsPred = 0;
	int nIsUnPongPre = 0;
	int nIsUnPongSucc = 0;
	int nPreLeaveID = -1;
	int nSuccLeaveID = -1;
	int nIsEmpty = 0;
	chordHeaderType tTempMsgHead, tMsgHead;

	// 메시지 만들기 
	tMsgHead.usMsgID = DEF_HEAD_MSG_ID_PING; // 0;
	tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
	srand(time(NULL));

	// timeput 설정
	struct timeval tTimeVal;
	tTimeVal.tv_sec = 1;         // 초
	tTimeVal.tv_usec = 0;        // 마이크로초 (Windows는 무시됨)

#if DEF_USING_RECV_WAIT_TIMEVAL 
	int nResult = setsockopt(ext_pfSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeVal, sizeof(tTimeVal));
	if (nResult == SOCKET_ERROR) {
		JC_Print(ERROR_MODE, "setsockopt(SO_RCVTIMEO) failed: %d\n", WSAGetLastError());
	}

	ext_pfSock = socket(AF_INET, SOCK_DGRAM, 0); // for ping-pong and fix-finger
	nRetVal = setsockopt(ext_pfSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nOptVal, sizeof(nOptVal));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(ERROR_MODE, "\a[ERROR] setsockopt() Error!");
		exit(1);
	}
	int nRetVal = recvfrom(ext_pfSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
	if (nRetVal == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAETIMEDOUT) {
			JC_Print(INFO_MODE, "recvfrom timed out");
		}
		else {
			JC_Print(ERROR_MODE, "recvfrom error: %d", err);
		}
		// 회복 로직 추가
	}
#else
	ext_pfSock = socket(AF_INET, SOCK_DGRAM, 0); // for ping-pong and fix-finger
	nRetVal = setsockopt(ext_pfSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nOptVal, sizeof(nOptVal));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(ERROR_MODE, "\a[ERROR] setsockopt() Error!");
		exit(1);
	}
#endif 

	int nTestCnt = 0;
	// 테스트를 위해서 subNode 값을 임의로 등록했을대는 사용을 안한다.  ()st_bUsePingAndFixFinger
	while (!(*pnExitFlag) || !(ext_bUsePingAndFixFinger == 1))
	{

		//JC_RESULT("%d:%d > ", ext_tMyNode.tNodeInfo.unID, nTestCnt++);
		//JC_Print(INFO_MODE, "=> procPingAndFixFinger() = %d", ext_bUsePingAndFixFinger);
		// 테스트를 위해서 subNode 값을 임의로 등록했을대는 사용을 안한다. 
		if (ext_bUsePingAndFixFinger == 0)
		{
			Sleep(DEF_TIME_PING);
			continue;
		}
//#if JC_DEBUG_PING_FIX 
//		JC_Print(NONE_MODE, "\n\n1");
//#endif 
		nIsUnPongPre = 0;
		nIsUnPongSucc = 0;
		nPreLeaveID = -1;

		// 1. fingerInfo.Pre 확인
#if JC_DEBUG_PING_FIX 
		JC_Print(DEBUG_MODE, "[%s] 1. nodeInfo.ID != fingerInfo.Pre.ID 확인(%d != %d) "
			, __FUNCTION__
			, ext_tMyNode.tNodeInfo.unID, ext_tMyNode.tChordInfo.tFingerInfo.tPre.unID)
#endif 
		if (ext_tMyNode.tNodeInfo.unID != ext_tMyNode.tChordInfo.tFingerInfo.tPre.unID ) 
		{
#if JC_DEBUG_PING_FIX 
			JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_PING, %s:%d) - fingerInfo.Pre.ID (%d)] "
				, __FUNCTION__
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_port)
				, ext_tMyNode.tChordInfo.tFingerInfo.tPre.unID);
#endif 
			//JC_Print(INFO_MODE, "addr : %s, %d",  inet_ntoa(myNode.chordInfo.fingerInfo.Pre.addrInfo.sin_addr), ntohs(myNode.chordInfo.fingerInfo.Pre.addrInfo.sin_port));
			nRetVal = sendto(ext_pfSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(ERROR_MODE, "Pingpong Pre request Sendto error(%s:%d)!"
					, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_addr)
					, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_port));
				exit(1);
			}
			nRetVal = recvfrom(ext_pfSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) 
			{
				JC_Print(WARN_MODE, "PingPong Pre request Recvfrom error(%s:%d)!"
					, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_addr)
					, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_port));
				//응답이 제시간에 안오면 여길 들어오니까 여기서 해결하면 된다.
				nPreLeaveID = ext_tMyNode.tChordInfo.tFingerInfo.tPre.unID;
				memset(&ext_tMyNode.tChordInfo.tFingerInfo.tPre, 0, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tPre));
				nIsUnPongPre = 1;
			}

#if JC_DEBUG_PING_FIX
			JC_Print(DEBUG_MODE, "[%s] ## Recv(DEF_HEAD_MSG_ID_PING, %s:%d) #=> PreLeaveID:%d, IsUnPongPre:%d, IsUnPongSucc:%d"
				, __FUNCTION__
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_port)
				, nPreLeaveID, nIsUnPongPre, nIsUnPongSucc);
#endif 
		}

		// 2. fingerInfo.finger[0] 확인
#if JC_DEBUG_PING_FIX 
		JC_Print(DEBUG_MODE, "[%s] 2. nodeInfo.ID != fingerInfo.finger[0].ID 확인( %d != %d) "
			, __FUNCTION__
			, ext_tMyNode.tNodeInfo.unID, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID)
#endif 
		if (ext_tMyNode.tNodeInfo.unID != ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID) 
		{
#if JC_DEBUG_PING_FIX
			JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_PING, %s:%d) - fingerInfo.finger[0].ID (%d)] "
				, __FUNCTION__
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
				, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID);
#endif 
			nRetVal = sendto(ext_pfSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(ERROR_MODE, "Pingpong Finger[0] request Sendto error(%s:%d)!"
					, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
					, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port));
				exit(1);
			}

			nRetVal = recvfrom(ext_pfSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) 
			{
				JC_Print(WARN_MODE, "Pingpong Finger[0] request Recvfrom error(%s:%d)!"
					, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
					, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port));
				nSuccLeaveID = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID;
				nIsUnPongSucc = 1;

				// 이웃 노드(predecessor, successor)가 동시에 응답하지 않을 때
				if ((nIsUnPongPre == 1) && (nIsUnPongSucc == 1)) 
				{
					// 5,4, 3, 2, 1
					for (i = 5; i > 0; i--) 
					{
						nIsEmpty = 1;
						if ((ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID != nPreLeaveID) 
							&& (ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID != nSuccLeaveID))
						{
							ext_tMyNode.tChordInfo.tFingerInfo.tPre = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i];
							ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0] = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i];
							stabilizeLeave(ext_pfSock, nSuccLeaveID);
							nIsEmpty = 0;
							break;
						}
					}

					//유효한 노드가 없다면
					if (nIsEmpty == 1) 
					{
						ext_tMyNode.tChordInfo.tFingerInfo.tPre = ext_tMyNode.tNodeInfo;
						// 0, 1, 2, 3,4, 5, 6, 7, 8, 9
						for (i = 0; i < DEF_BASE_M; i++) 
						{
							ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i] = ext_tMyNode.tNodeInfo;
						}
					}
				}
				else if (nIsUnPongSucc == 1) 
				{
					ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0] = ext_tMyNode.tChordInfo.tFingerInfo.tPre;
					stabilizeLeave(ext_pfSock, nSuccLeaveID);
					nSuccLeaveID = -1;
				}
			}
#if JC_DEBUG_PING_FIX
			JC_Print(DEBUG_MODE, "[%s] ## Recv(DEF_HEAD_MSG_ID_PING, %s:%d) #=> PreLeaveID:%d, IsUnPongPre:%d, IsUnPongSucc:%d"
				, __FUNCTION__
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
				, nPreLeaveID, nIsUnPongPre, nIsUnPongSucc);
			//printNodeType(&ext_tMyNode);
#endif 

		}

		// 3. fingerInfo.finger[N~1] 확인
		for (i = DEF_BASE_M - 1; i > 0; i--) 
		{
#if JC_DEBUG_PING_FIX 
			JC_Print(DEBUG_MODE, "[%s] 3. nodeInfo.ID != fingerInfo.finger[%d] 확인 (%d != %d) "
				, __FUNCTION__
				, i
				, ext_tMyNode.tNodeInfo.unID, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID)
#endif 
			if ((ext_tMyNode.tNodeInfo.unID != ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID)
				&& 0 != ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID)
			{
#if JC_DEBUG_PING_FIX
				JC_Print(DEBUG_MODE, "[%s] 3-1. ## SendTo(DEF_HEAD_MSG_ID_PING, %s:%d) - fingerInfo.finger[%d].ID (%d)] "
					, __FUNCTION__
					, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
					, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port)
					, i, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID);
#endif 
				nRetVal = sendto(ext_pfSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, 
(struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(ERROR_MODE, "Pingpong Finger[%d] request Sendto error(%s:%d)!", i
						, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
						, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port));
					//exit(1);
					continue;
				}

				nRetVal = recvfrom(ext_pfSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
				if (nRetVal == SOCKET_ERROR) 
				{
					JC_Print(WARN_MODE, "Pingpong Finger[%d] request Recvfrom error(%s:%d)!"
						, i
						, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
						, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)); 
					
					if (i == DEF_BASE_M - 1)
					{
						// 가장 끝 노드면 → Predecessor로 대체
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i] = ext_tMyNode.tChordInfo.tFingerInfo.tPre;
						ReleaseMutex(ext_hMutex);
					}
					else 
					{
						// 그 외 → 바로 다음 finger[i+1]로 대체 (더 가까운 노드)
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i] = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i + 1];
						ReleaseMutex(ext_hMutex);
					}

					/*if (i == 0){
						isPred = 1;
					}*/

				}

#if JC_DEBUG_PING_FIX
				JC_Print(DEBUG_MODE, "[%s] 3-2. ## Recv(DEF_HEAD_MSG_ID_PING, %s:%d) - chordInfo.fingerInfo.finger[%d].ID (%d)] "
					, __FUNCTION__
					, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
					, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port)
					, i, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID);
#endif 
			}
		}

		// 4. finger entry (finger[1] ~ finger[DEF_BASE_M-1])를 주기적으로 갱신하여 라우팅 정확성과 빠른 검색을 유지
#if JC_DEBUG_PING_FIX
		JC_Print(DEBUG_MODE, "[%s] 4. finger entry udpate find_successor => NodeID:%d , finger[1~%d]"
			, __FUNCTION__
			, ext_tMyNode.tNodeInfo.unID, DEF_BASE_M);
		//= > modPlus:twoPow(% d) : % d, % d ) i, twoPow(i)
		//	, modPlus(DEF_RINF_MAX_CNT, ext_tMyNode.tNodeInfo.unID, twoPow(i)));
#endif 
		for (i = 1; i < DEF_BASE_M; i++) 
		{
			// modPlus(32, 10, 8) → (10 + 8) % 32 = 18
			// find_successor()의 두 번째 인자로 18을 넘겨서 finger[3]에는 노드 18을 관리하는 successor를 저장하게 됩니다.

#if JC_DEBUG_PING_FIX
			JC_Print(DEBUG_MODE, "[%s] \t# 4-1. Before (fingerInfo.finger[%d].ID = %d, %s:%d) modPlus(%d, %d) = %d"
				, __FUNCTION__, i, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port)
				, ext_tMyNode.tNodeInfo.unID, twoPow(i)
				, modPlus(DEF_ID_SPACE, ext_tMyNode.tNodeInfo.unID, twoPow(i)));
#endif 
			ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i] = FindSuccessor(ext_pfSock, modPlus(DEF_ID_SPACE, ext_tMyNode.tNodeInfo.unID, twoPow(i)));

#if JC_DEBUG_PING_FIX
			JC_Print(DEBUG_MODE, "[%s] \t4-2. After (fingerInfo.finger[%d].ID = %d, %s:%d)"
				, __FUNCTION__, i, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port));
#endif 
			if (ext_nSilentMode == 0) 
			{
				JC_Print(INFO_MODE, "CHORD> Periodic FixFinger: finger[%d] has been updated to %s<ID %d>",  i + 1, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr), ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID);
			}
		}
		//ReleaseMutex(hMutex);
		//WaitForSingleObject(hMutex, INFINITE);
		unVarTime = rand() % DEF_TIME_PING;
		unDelayTime = DEF_TIME_PING + unVarTime;  // delay: 8~10 sec
#if JC_DEBUG_PING_FIX
		JC_Print(DEBUG_MODE, "[%s] Sleep(%d)", __FUNCTION__, unDelayTime);
		//printNodeType(&ext_tMyNode);
#endif 
		Sleep(unDelayTime);
	}
}