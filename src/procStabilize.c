#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/procStabilize.h"


#pragma comment(lib,"ws2_32")
#include <winsock2.h>


void stabilizeLeave(SOCKET _tSock, int _nLeaveID)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	nodeInfoType tPredNodeInfo, tSuccNodeInfo;
	nodeInfoType ppred;
	int nRetVal;
	chordHeaderType tTempMsgHead, tMsgHead;

	// 무한정 기다림
	WaitForSingleObject(ext_hMutex, INFINITE);
	tSuccNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0]; /*successor */
	ReleaseMutex(ext_hMutex);
	//JC_Print(INFO_MODE, "%d, %d",  myNode.nodeInfo.unID, succNode.ID);

	// 후임 노드(successor)에게 선임 노드(predecessor) 정보를 요청
	// 특정 노드(_nLeaveID)는 탈퇴하려 하므로 그것을 피해서 다른 정상 노드를 찾으려는 목적
	while (1)
	{
		memset(&tMsgHead, 0, sizeof(tMsgHead));
		tMsgHead.usMsgID = DEF_HEAD_MSG_ID_GET_PRED; // 3;
		tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
		tMsgHead.sMoreInfo = 0;
		tMsgHead.unBodySize = 0;


		JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_GET_PRED, %s:%d) "
			, __FUNCTION__
			, inet_ntoa(tSuccNodeInfo.tAddrInfo.sin_addr), ntohs(tSuccNodeInfo.tAddrInfo.sin_port));
		//JC_Print(INFO_MODE, "%d",  succNode.ID);
		nRetVal = sendto(_tSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tSuccNodeInfo.tAddrInfo, sizeof(tSuccNodeInfo.tAddrInfo));
		if (nRetVal == SOCKET_ERROR) {
			JC_Print(INFO_MODE, "Predecessor Info request Sendto error!");
			exit(1);
		}

		//JC_Print(INFO_MODE, "CHORD> PredInfo request Message has been sent.");

		nRetVal = recvfrom(_tSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
		if (nRetVal == SOCKET_ERROR) {
			JC_Print(INFO_MODE, "[%s] [DEF_HEAD_MSG_ID_GET_PRED] Predecessor Info request Recvfrom error!", __FUNCTION__);
			exit(1);
		}
		JC_Print(DEBUG_MODE, "[%s] ## Recv(DEF_HEAD_MSG_ID_GET_PRED, %s:%d)_nLeaveID:%d != NodeID:%d "
			, __FUNCTION__
			, inet_ntoa(tSuccNodeInfo.tAddrInfo.sin_addr), ntohs(tSuccNodeInfo.tAddrInfo.sin_port)
			, _nLeaveID, tTempMsgHead.tNodeInfo.unID);
		//JC_Print(INFO_MODE, "CHORD> PredInfo response Message has been received.");

		// 탈퇴할려는 아이디가 아는 선임노드
		if (tTempMsgHead.tNodeInfo.unID != _nLeaveID) {
			tPredNodeInfo = tTempMsgHead.tNodeInfo;
			break;
		}
		else {
			continue;
		}
	} // end while
	//JC_Print(INFO_MODE, "39id : %d",  P.ID);

	JC_Print(INFO_MODE, "You got your successor's predecessor node from your seccessor node.");

	if (memcmp(&tPredNodeInfo, &ext_initNode, sizeof(ext_initNode)))
	{
		//////////////////////////////////////////
		if ((ext_tMyNode.tNodeInfo.unID == tSuccNodeInfo.unID))
		{
			WaitForSingleObject(ext_hMutex, INFINITE);
			ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0] = tPredNodeInfo;
			ReleaseMutex(ext_hMutex);
			tSuccNodeInfo = tPredNodeInfo;
		}
		while (1)
		{
			memset(&tMsgHead, 0, sizeof(tMsgHead));
			tMsgHead.usMsgID = DEF_HEAD_MSG_ID_GET_PRED; // 3;
			tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tMsgHead.sMoreInfo = 0;
			tMsgHead.unBodySize = 0;
			nRetVal = sendto(_tSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tPredNodeInfo.tAddrInfo, sizeof(tPredNodeInfo.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "Predecessor Info request Sendto error!");
				exit(1);
			}
			nRetVal = recvfrom(_tSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "[%s] Predecessor Info request Recvfrom error!", __FUNCTION__);
				//exit(1);
				//break;
				continue;
			}
			if (tTempMsgHead.tNodeInfo.unID != _nLeaveID) {
				ppred = tTempMsgHead.tNodeInfo;
				break;
			}
			else {
				continue;
			}
		}

		while (memcmp(&ppred, &ext_initNode, sizeof(ext_initNode)))
		{
			if (modIn(DEF_ID_SPACE, ppred.unID, ext_tMyNode.tNodeInfo.unID, tSuccNodeInfo.unID, 0, 0))
			{
				tPredNodeInfo = ppred;
				memset(&tMsgHead, 0, sizeof(tMsgHead));
				tMsgHead.usMsgID = DEF_HEAD_MSG_ID_GET_PRED;
				tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
				tMsgHead.sMoreInfo = 0;
				tMsgHead.unBodySize = 0;

				//JC_Print(INFO_MODE, "pp  : %d",  P.ID);
				nRetVal = sendto(_tSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&ppred.tAddrInfo, sizeof(ppred.tAddrInfo));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Predecessor Info request Sendto error!");
					exit(1);
				}
				nRetVal = recvfrom(_tSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "[%s] Predecessor Info request Recvfrom error!", __FUNCTION__);
					//exit(1);
					continue;
				}

				if (tTempMsgHead.tNodeInfo.unID != _nLeaveID) {
					ppred = tTempMsgHead.tNodeInfo;
					continue;
				}
				else {
					continue;
				}
			}
			else {
				break;
			}
		}
		//JC_Print(INFO_MODE, "pre %d",  P.ID);
		////////////////////////////////////////
		if (modIn(DEF_ID_SPACE, tPredNodeInfo.unID, ext_tMyNode.tNodeInfo.unID, tSuccNodeInfo.unID, 0, 0)) {
			WaitForSingleObject(ext_hMutex, INFINITE);
			ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0] = tPredNodeInfo;
			ReleaseMutex(ext_hMutex);
			tSuccNodeInfo = tPredNodeInfo;
		}
		else {  // Actually not necessary
			memset(&tMsgHead, 0, sizeof(tMsgHead));
			tMsgHead.usMsgID = DEF_HEAD_MSG_ID_UPD_SUCC;
			tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
			tMsgHead.sMoreInfo = 0;
			tMsgHead.unBodySize = 0;
			nRetVal = sendto(_tSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tPredNodeInfo.tAddrInfo, sizeof(tPredNodeInfo.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "Successor Update request Sendto error!");
				exit(1);
			}
			JC_Print(INFO_MODE, "CHORD> SuccUpdate request Message has been sent.");

			nRetVal = recvfrom(_tSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
			//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(tempMsg.tNodeInfo.tAddrInfo.sin_addr), ntohs(tempMsg.tNodeInfo.tAddrInfo.sin_port));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "Successor Update request Recvfrom error!");
				exit(1);
			}

			JC_Print(INFO_MODE, "CHORD> SuccUpdate response Message has been received.");
			JC_Print(INFO_MODE, "CHORD> Your predecessor's successor has been updated as your node.");
			notify(tPredNodeInfo);
		}
		JC_Print(INFO_MODE, "CHORD> Your predecessor has been updated.");
	} // end if (memcmp(&tPredNodeInfo, &ext_initNode, sizeof(ext_initNode))) 

	memset(&tMsgHead, 0, sizeof(tMsgHead));
	tMsgHead.usMsgID = DEF_HEAD_MSG_ID_UPD_PRED;
	tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
	tMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
	tMsgHead.sMoreInfo = 0;
	tMsgHead.unBodySize = 0;

	nRetVal = sendto(_tSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "Sendto error!");
		exit(1);
	}
	JC_Print(INFO_MODE, "CHORD> PredUpdate request Message has been sent.");

	nRetVal = recvfrom(_tSock, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
	//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(tempMsg.tNodeInfo.tAddrInfo.sin_addr), ntohs(tempMsg.tNodeInfo.tAddrInfo.sin_port));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "Predecessor Update request Recvfrom error!");
		exit(1);
	}
	JC_Print(INFO_MODE, "CHORD> PredUpdate response Message has been received.");
	JC_Print(INFO_MODE, "CHORD> Your successor's predecessor has been updated as your node.");
}

// 노드가 조인할 때 네트워크를 안정화(stabilization) 하는 데 사용
// 새 노드가 네트워크에 들어온 뒤, 자신의 successor와 predecessor를 갱신
void stabilizeJoin(SOCKET _tSock)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	nodeInfoType tPredNodInfo, tSuccNodeInfo;
	int nRetVal;
	chordHeaderType tRecvMsgHead, tSendMsgHead;

	WaitForSingleObject(ext_hMutex, INFINITE);
	tSuccNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0]; /*successor */
	ReleaseMutex(ext_hMutex);

	// 1. 현재 successor 정보 획득
#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 1. 현재 successor 정보 finger[0] [%d, %s:%d]", __FUNCTION__
		, tSuccNodeInfo.unID
		, inet_ntoa(tSuccNodeInfo.tAddrInfo.sin_addr)
		, ntohs(tSuccNodeInfo.tAddrInfo.sin_port));
#endif 
	memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
	tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_GET_PRED;
	tSendMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
	tSendMsgHead.sMoreInfo = 0;
	tSendMsgHead.unBodySize = 0;

	// 2. succ에게 pred 정보 요청
#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_GET_PRED, %s:%d)] - 2. succ에게 pred 정보 요청"
		, __FUNCTION__
		, inet_ntoa(tSuccNodeInfo.tAddrInfo.sin_addr)
		, ntohs(tSuccNodeInfo.tAddrInfo.sin_port));
#endif 
	nRetVal = sendto(_tSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tSuccNodeInfo.tAddrInfo, sizeof(tSuccNodeInfo.tAddrInfo));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "Predecessor Info request Sendto error!");
		exit(1);
	}

	JC_Print(INFO_MODE, "CHORD> PredInfo request Message has been sent.");

	nRetVal = recvfrom(_tSock, (char*)&tRecvMsgHead, sizeof(tRecvMsgHead), 0, NULL, NULL);
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "[%s] Predecessor Info request Recvfrom error!", __FUNCTION__);
		exit(1);
	}
	JC_Print(INFO_MODE, "CHORD> PredInfo response Message has been received.");
	tPredNodInfo = tRecvMsgHead.tNodeInfo;

	// 3. succ에게 pred 정보 수신
#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] ## Recv(DEF_HEAD_MSG_ID_GET_PRED)] - 3. succ에게 pred 정보 수신 (nodeInfo:%d, %s:%d)"
		, __FUNCTION__
		, tPredNodInfo.unID
		, inet_ntoa(tPredNodInfo.tAddrInfo.sin_addr)
		, ntohs(tPredNodInfo.tAddrInfo.sin_port));
#endif 
	//JC_Print(INFO_MODE, "39id : %d",  P.ID);

	JC_Print(INFO_MODE, "You got your successor's predecessor node from your seccessor node.");

	// 4. 받은 predecessor가 initNode가 아니라면 (즉, 유효한 노드)
#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 4.  받은 predecessor가 initNode가 아니라면 (즉, 유효한 노드) "
		, __FUNCTION__);
	printNodeInfoType(&tPredNodInfo, "PredNodInfo");
	printNodeInfoType(&ext_initNode, "ext_initNode");
#endif 
	if (memcmp(&tPredNodInfo, &ext_initNode, sizeof(ext_initNode)))
	{

		if (modIn(DEF_ID_SPACE, tPredNodInfo.unID, ext_tMyNode.tNodeInfo.unID, tSuccNodeInfo.unID, 0, 0))
		{
#if JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "[%s] 4-1. 원형 공간에서 범위 포함(Pred:%d, MyNode:%d , Succ:%d ) => successor를 해당 predecessor ㅂㄴ경"
				, __FUNCTION__
				, tPredNodInfo.unID, ext_tMyNode.tNodeInfo.unID, tSuccNodeInfo.unID);
#endif 
			WaitForSingleObject(ext_hMutex, INFINITE);
			ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0] = tPredNodInfo;
			ReleaseMutex(ext_hMutex);
			tSuccNodeInfo = tPredNodInfo;
		}
		else
		{
#if JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "[%s] 4-2-1. 원형 공간에서 범위 미포함(Pred:%d, MyNode:%d , Succ:%d ) => successor를 해당 predecessor 변경"
				, __FUNCTION__
				, tPredNodInfo.unID, ext_tMyNode.tNodeInfo.unID, tSuccNodeInfo.unID);
#endif 
			// Actually not necessary
			memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
			tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_UPD_SUCC;
			tSendMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tSendMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
			tSendMsgHead.sMoreInfo = 0;
			tSendMsgHead.unBodySize = 0;

#if JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_UPD_SUCC, %s:%d)] - 4-2-2. pred에게 succ 업데이트 요청(nodeinfo:%d)"
				, __FUNCTION__
				, inet_ntoa(tPredNodInfo.tAddrInfo.sin_addr)
				, ntohs(tPredNodInfo.tAddrInfo.sin_port)
				, tSendMsgHead.tNodeInfo.unID);
#endif 
			nRetVal = sendto(_tSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&tPredNodInfo.tAddrInfo, sizeof(tPredNodInfo.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "Successor Update request Sendto error!");
				exit(1);
			}
			JC_Print(INFO_MODE, "CHORD> SuccUpdate request Message has been sent.");

			nRetVal = recvfrom(_tSock, (char*)&tRecvMsgHead, sizeof(tRecvMsgHead), 0, NULL, NULL);
			//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(tempMsg.tNodeInfo.tAddrInfo.sin_addr), ntohs(tempMsg.tNodeInfo.tAddrInfo.sin_port));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "Successor Update request Recvfrom error!");
				exit(1);
			}

			// 
#if JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "[%s] ## Recv(DEF_HEAD_MSG_ID_UPD_SUCC, %d, %s:%d)] - 4-2-3. pred에게  수신[moreInfo: %d] "
				, __FUNCTION__
				, tPredNodInfo.unID
				, inet_ntoa(tPredNodInfo.tAddrInfo.sin_addr)
				, ntohs(tPredNodInfo.tAddrInfo.sin_port)
				, tRecvMsgHead.sMoreInfo);
#endif 
			JC_Print(INFO_MODE, "CHORD> SuccUpdate response Message has been received.");
			JC_Print(INFO_MODE, "CHORD> Your predecessor's successor has been updated as your node.");
			printNodeInfoType(&ext_tMyNode.tChordInfo.tFingerInfo.tPre, "6-3> ");
			notify(tPredNodInfo);
			printNodeInfoType(&ext_tMyNode.tChordInfo.tFingerInfo.tPre, "6-4> ");
		}
		JC_Print(INFO_MODE, "CHORD> Your predecessor has been updated.");
	}
	memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
	tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_UPD_PRED;
	tSendMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
	tSendMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
	tSendMsgHead.sMoreInfo = 0;
	tSendMsgHead.unBodySize = 0;

#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_UPD_PRED, %s:%d)] - 4-2-4.succ 에게 pred 업데이트 요청[nodeinfo.ID:%d]"
		, __FUNCTION__
		, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
		, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
		, tSendMsgHead.tNodeInfo.unID);
#endif 
	nRetVal = sendto(_tSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "Sendto error!");
		exit(1);
	}
	JC_Print(INFO_MODE, "CHORD> PredUpdate request Message has been sent.");

	nRetVal = recvfrom(_tSock, (char*)&tRecvMsgHead, sizeof(tRecvMsgHead), 0, NULL, NULL);
	//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(tempMsg.tNodeInfo.tAddrInfo.sin_addr), ntohs(tempMsg.tNodeInfo.tAddrInfo.sin_port));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "Predecessor Update request Recvfrom error!");
		exit(1);
	}
#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] ## Recv(DEF_HEAD_MSG_ID_UPD_PRED, %d, %s:%d)] - 4-2-5. pred에게 수신[moreinfo:%d] "
		, __FUNCTION__
		, tPredNodInfo.unID
		, inet_ntoa(tPredNodInfo.tAddrInfo.sin_addr)
		, ntohs(tPredNodInfo.tAddrInfo.sin_port)
		, tRecvMsgHead.sMoreInfo);
#endif 
	printNodeInfoType(&ext_tMyNode.tChordInfo.tFingerInfo.tPre, "8> ");
	JC_Print(INFO_MODE, "CHORD> PredUpdate response Message has been received.");
	JC_Print(INFO_MODE, "CHORD> Your successor's predecessor has been updated as your node.");

#if JC_DEBUG_JOIN
	printNodeType(&ext_tMyNode);
#endif 
}
