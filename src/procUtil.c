#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/procUtil.h"


#pragma comment(lib,"ws2_32")
#include <winsock2.h>
nodeType ext_tMyNode = { 0 };               // node information -> global variable
SOCKET ext_tReqSock, ext_tRecvProcSock, ext_flSock, ext_frSock, ext_fsSock, ext_pfSock;
HANDLE ext_hMutex;
nodeInfoType ext_initNode = { 0 };

int ext_nSilentMode = 1; // silent mode


int lookup()
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	int targetKey;
	int i;
	int retVal;
	int Store = 0;
	int addrSize;
	char fileName[DEF_FILE_NAME_MAX_LEN + 1];
	nodeInfoType succNode = { 0 };
	nodeInfoType ownNode = { 0 };
	struct sockaddr_in peerAddr;
	chordHeaderType tempMsg, bufMsg;
	int temp;
	char fileBuf[DEF_FILE_BUF_LEN];
	int totalbytes;
	FILE* fp;
	int numTotal;
	addrSize = sizeof(peerAddr);

	JC_Print(INFO_MODE, "CHORD> Input File name to search and download: ");
	scanf("%s", fileName);
	targetKey = strHash(fileName);
	JC_Print(INFO_MODE, "CHORD> Input File name: %s, Key: %d",  fileName, targetKey);

	// 자신이 파일을 포함하는지 확인
	for (i = 0; i < ext_tMyNode.tFileInfo.nFileNum; i++) 
	{
		if (targetKey == ext_tMyNode.tFileInfo.tFileRef[i].nKey) {
			JC_Print(INFO_MODE, "CHORD> The file %s is at this node itself!",  ext_tMyNode.tFileInfo.tFileRef[i].chNameBuf);//done
			return 0;
		}
	}

	// 남의 파일을 레퍼런스 하는지 확인
	for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++) 
	{
		if (targetKey == ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey) 
		{
			ownNode = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].tOwnerNodeInfo;
			JC_Print(INFO_MODE, "CHORD> File Owner IP Addr: %s, Port No: %d, ID: %d",  
				inet_ntoa(ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].tOwnerNodeInfo.tAddrInfo.sin_addr), 
				ntohs(ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].tOwnerNodeInfo.tAddrInfo.sin_port), 
				ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].tOwnerNodeInfo.unID);
			memset(&bufMsg, 0, sizeof(bufMsg));
			bufMsg.usMsgID = 11;
			bufMsg.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			bufMsg.sMoreInfo = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey;
			bufMsg.unBodySize = 0;

			retVal = sendto(ext_tReqSock, (char*)&bufMsg, sizeof(bufMsg), 0, (struct sockaddr*)&ownNode.tAddrInfo, sizeof(ownNode.tAddrInfo));
			if (retVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "File Reference Info request Sendto error!");
				exit(1);
			}
			retVal = recvfrom(ext_tReqSock, (char*)&tempMsg, sizeof(tempMsg), 0, NULL, NULL);//이 tempMsg에 totalBytes값이 있다.
			if (retVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "File Reference Info request Recvfrom error!");
				exit(1);
			}
			JC_Print(INFO_MODE, "CHORD> own IP Addr: %s, Port No: %d, ID: %d",  inet_ntoa(ownNode.tAddrInfo.sin_addr), ntohs(ownNode.tAddrInfo.sin_port), ownNode.unID);
			retVal = connect(ext_frSock, (struct sockaddr*)&ownNode.tAddrInfo, sizeof(ownNode.tAddrInfo));
			if (retVal == SOCKET_ERROR) err_quit("connect()");


			// 다운로드 파일 확인
			char DownLoadFileName[DEF_FILE_NAME_MAX_LEN];
			memset(DownLoadFileName, 0, sizeof(DownLoadFileName));
			sprintf_s(DownLoadFileName, sizeof(DownLoadFileName), "%s.down", ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].chNameBuf);

			//fp = fopen(ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].chNameBuf, "wb");
			fp = fopen(DownLoadFileName, "wb");
			if (fp == NULL) {
				perror("파일 입출력 오류");
				exit(1);

			}

			totalbytes = tempMsg.unBodySize;
			JC_Print(INFO_MODE, "CHORD> totalbytes: %d",  totalbytes);

			numTotal = 0;
			temp = totalbytes;
			while (1) {
				if (DEF_FILE_BUF_LEN > temp) {
					retVal = recvn(ext_frSock, fileBuf, temp, 0);
				}
				else
					retVal = recvn(ext_frSock, fileBuf, DEF_FILE_BUF_LEN, 0);
				temp -= retVal;
				//JC_Print(INFO_MODE, "retVal : %d",  retVal);
				if (retVal == SOCKET_ERROR) {
					err_display("recv()");
					break;
				}
				else if (retVal == 0)
					break;
				else {
					fwrite(fileBuf, 1, retVal, fp);
					if (ferror(fp)) {
						perror("파일 입출력 오류");
						break;
					}
					numTotal += retVal;
				}
				if (numTotal == totalbytes)
					break;
			}
			fclose(fp);
			// 전송 결과 출력
			if (numTotal == totalbytes)
			{
				JC_Print(INFO_MODE, "-> 파일 전송 완료!");
			}
			else
			{
				JC_Print(INFO_MODE, "-> 파일 전송 실패!");
			}

			JC_Print(INFO_MODE, "CHORD> File %s has been received successfully",  ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].chNameBuf);
			return 0;
		}
	}
	succNode = FindSuccessor(ext_tReqSock, targetKey);
	JC_Print(INFO_MODE, "CHORD> File's Successor IP Addr: %s, Port No: %d, ID: %d",  inet_ntoa(succNode.tAddrInfo.sin_addr), ntohs(succNode.tAddrInfo.sin_port), succNode.unID);
	memset(&bufMsg, 0, sizeof(bufMsg));
	bufMsg.usMsgID = 12;
	bufMsg.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
	bufMsg.sMoreInfo = targetKey;
	bufMsg.unBodySize = 0;

	retVal = sendto(ext_tReqSock, (char*)&bufMsg, sizeof(bufMsg), 0, (struct sockaddr*)&succNode.tAddrInfo, sizeof(succNode.tAddrInfo));
	if (retVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "File Reference Info request Sendto error!");
		exit(1);
	}
	retVal = recvfrom(ext_tReqSock, (char*)&tempMsg, sizeof(tempMsg), 0, NULL, NULL);
	if (retVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "File Reference Info request Recvfrom error!");
		exit(1);
	}
	ownNode = tempMsg.tNodeInfo;
	JC_Print(INFO_MODE, "CHORD> File Owner IP Addr: %s, Port No: %d, ID: %d",  inet_ntoa(ownNode.tAddrInfo.sin_addr), ntohs(ownNode.tAddrInfo.sin_port), ownNode.unID);
	memset(&bufMsg, 0, sizeof(bufMsg));
	bufMsg.usMsgID = 11;
	bufMsg.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
	bufMsg.sMoreInfo = targetKey;
	bufMsg.unBodySize = 0;

	retVal = sendto(ext_tReqSock, (char*)&bufMsg, sizeof(bufMsg), 0, (struct sockaddr*)&ownNode.tAddrInfo, sizeof(ownNode.tAddrInfo));
	if (retVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "File Reference Info request Sendto error!");
		exit(1);
	}
	retVal = recvfrom(ext_tReqSock, (char*)&tempMsg, sizeof(tempMsg), 0, NULL, NULL);//이 tempMsg에 totalBytes값이 있다.
	if (retVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "File Reference Info request Recvfrom error!");
		exit(1);
	}
	retVal = connect(ext_frSock, (struct sockaddr*)&ownNode.tAddrInfo, sizeof(ownNode.tAddrInfo));
	if (retVal == SOCKET_ERROR) err_quit("connect()");

	fp = fopen(fileName, "wb");//여기서 에러구나
	if (fp == NULL) {
		perror("파일 입출력 오류");
		//closesocket(frSock);
		exit(1);

	}

	totalbytes = tempMsg.unBodySize;
	JC_Print(INFO_MODE, "CHORD> totalbytes: %d",  totalbytes);

	numTotal = 0;
	temp = totalbytes;
	while (1) {
		if (DEF_FILE_BUF_LEN > temp) {
			retVal = recvn(ext_frSock, fileBuf, temp, 0);
		}
		else
			retVal = recvn(ext_frSock, fileBuf, DEF_FILE_BUF_LEN, 0);
		temp -= retVal;
		if (retVal == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retVal == 0)
			break;
		else {
			fwrite(fileBuf, 1, retVal, fp);
			if (ferror(fp)) {
				perror("파일 입출력 오류");
				break;
			}
			numTotal += retVal;
		}
		if (numTotal == totalbytes)
			break;
	}
	fclose(fp);
	// 전송 결과 출력
	if (numTotal == totalbytes)
	{
		JC_Print(INFO_MODE, "-> 파일 전송 완료!");
	}
	else
	{
		JC_Print(INFO_MODE, "-> 파일 전송 실패!");
	}
	JC_Print(INFO_MODE, "CHORD> File %s has been received successfully!");


	return 0; /* Success */
}

// 노드(ext_tMyNode)의 finger table을 갱신함
int fix_finger()
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	int i;

	// 1. 현재 노드 초기 상태인지 확인
	if (!memcmp(&ext_tMyNode.tNodeInfo, &ext_initNode, sizeof(ext_initNode))) 
	{
		return -1;
	}

	// 2. finger table 갱신
	for (i = 1; i < DEF_BASE_M; i++) 
	{
#if JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "[%s] # 1. find_successor (MyNode.nodeInfo.unID:%d, twoPow(%d):%d) => cur(%d, %s:%d) "
			, __FUNCTION__
			, ext_tMyNode.tNodeInfo.unID, i, twoPow(i)
			, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
			, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
			, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port));
#endif 
		ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i] = FindSuccessor(ext_tReqSock, modPlus(DEF_ID_SPACE, ext_tMyNode.tNodeInfo.unID, twoPow(i)));
		JC_Print(INFO_MODE, "CHORD> FixFinger: finger[%d] has been updated to %s<ID %d>", i + 1, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr), ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID);

#if JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "[%s] # 2. find_successor (MyNode.nodeInfo.unID:%d, twoPow(%d):%d) => cur(%d, %s:%d) "
			, __FUNCTION__
			, ext_tMyNode.tNodeInfo.unID, i, twoPow(i)
			, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
			, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
			, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port));
#endif 
	}
	JC_Print(INFO_MODE, "CHORD> Node join has been successfully finished.");
	return 0;
}

int move_keys()
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 


	int nRetVal;
	int nFileSize = 0;
	chordHeaderType tRecvMsgHead, tSendMsgHead;
	char* pchBody = NULL;
	int i;
	memset(&tSendMsgHead, 0, sizeof(tSendMsgHead));
	tSendMsgHead.usMsgID = DEF_HEAD_MSG_ID_MOVE_KEY;
	tSendMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
	tSendMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
	tSendMsgHead.sMoreInfo = 0;
	tSendMsgHead.unBodySize = 0;

#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 1-1. ## SendTo(DEF_HEAD_MSG_ID_MOVE_KEY, %s:%d) - nodeInfo[ID : %d] "
		, __FUNCTION__
		, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr), ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
		, tSendMsgHead.tNodeInfo.unID);
#endif 
	nRetVal = sendto(ext_tReqSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "MoveKeys request Sendto error!");
		exit(1);
	}

	//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(bufMsg.nodeInfo.tAddrInfo.sin_addr), ntohs(bufMsg.nodeInfo.tAddrInfo.sin_port));
	//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(myNode.tChordInfo.tFingerInfo.tFingers[0].addrInfo.sin_addr), ntohs(myNode.tChordInfo.tFingerInfo.tFingers[0].addrInfo.sin_port));

	JC_Print(INFO_MODE, "CHORD> MoveKeys request Message has been sent.");

	nRetVal = recvfrom(ext_tReqSock, (char*)&tRecvMsgHead, sizeof(tRecvMsgHead), 0, NULL, NULL);
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "MoveKeys tempMsg request Recvfrom error!");
		exit(1);
	}
#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 1-2. ## Recv(DEF_HEAD_MSG_ID_MOVE_KEY, %s:%d) moreInfo[KeyCnt:%d, bodySize:%d]"
		, __FUNCTION__
		, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
		, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
		, tRecvMsgHead.sMoreInfo, tRecvMsgHead.unBodySize);
#endif 

#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 2-1. ## SendTo(DEF_HEAD_MSG_ID_MOVE_KEY, %s:%d) - nodeInfo[ID:%d] "
		, __FUNCTION__
		, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr), ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
		, tSendMsgHead.tNodeInfo.unID);
#endif 
	nRetVal = sendto(ext_tReqSock, (char*)&tSendMsgHead, sizeof(tSendMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo));
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "MoveKeys body request Sendto error!");
		exit(1);
	}

	//JC_Print(INFO_MODE, "bodysize : %d",  tempMsg.unBodySize);
	pchBody = (char*)malloc(sizeof(char) * tRecvMsgHead.unBodySize);
	nRetVal = recvfrom(ext_tReqSock, (char*)pchBody, tRecvMsgHead.unBodySize, 0, NULL, NULL);
	if (nRetVal == SOCKET_ERROR) {
		JC_Print(INFO_MODE, "MoveKeys body request Recvfrom error!");
		exit(1);
	}

#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 2-2. ## Recv(DEF_HEAD_MSG_ID_MOVE_KEY, %s:%d) #=> moreInfo[fileNum:%d]"
		, __FUNCTION__
		, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
		, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
		, tRecvMsgHead.sMoreInfo);
#endif 

	JC_Print(INFO_MODE, "CHORD> MoveKeys response Message has been received.");
	WaitForSingleObject(ext_hMutex, INFINITE);
	ext_tMyNode.tChordInfo.tFRefInfo.nFileNum = tRecvMsgHead.sMoreInfo;
	ReleaseMutex(ext_hMutex);

#if JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 3. FRefInfo.fileRef 복사 ", __FUNCTION__);
#endif 
	for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++) 
	{
		WaitForSingleObject(ext_hMutex, INFINITE);
		memcpy(&ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i], pchBody, 76);
		ReleaseMutex(ext_hMutex);
		pchBody = pchBody + 76;
	}
	JC_Print(INFO_MODE, "CHORD> You got %d keys from your successor node.",  tRecvMsgHead.sMoreInfo);

	return 0;
}

// 특정 키 _nIDKey의 후속 노드(successor node)를 찾dma
nodeInfoType FindSuccessor(SOCKET _tSocket, int _nIDKey)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	int nRetVal;
	chordHeaderType tTempMsgHead, tMsgHead;
	nodeInfoType tPredNodeInfo;

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "\t\t[%s] 1. IDKey가 (nodeInfo.ID, finger[0]) 구간 안에 있나 #=> %d (%d ~ %d) "
		, __FUNCTION__
		, _nIDKey, ext_tMyNode.tNodeInfo.unID, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID);
#endif 
	// 1. IDKey가 (나, finger[0]) 구간 안에 있나?, 내가 _nIDKey의 바로 전 노드인지 검사
	if ((modIn(DEF_ID_SPACE, _nIDKey, ext_tMyNode.tNodeInfo.unID, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID, 0, 1)))
	{

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 1. 리턴 fingerInfo.finger[0]", __FUNCTION__);
#endif 

		return ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0];
	}


#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "\t\t[%s] 2. 내 ID가 _nIDKey와 같다면 #=>  (IDKey:%d == %d) "
		, __FUNCTION__
		, _nIDKey, ext_tMyNode.tNodeInfo.unID);
#endif 
	// 2. 내 ID가 키와 정확히 같다면
	if (ext_tMyNode.tNodeInfo.unID == _nIDKey) 
	{
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 2. 리턴 My Node", __FUNCTION__);
#endif 
		return ext_tMyNode.tNodeInfo;
	}

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "\t\t[%s] 3. 내 ID가 _nIDKey와 같지 않으면  #=> find_predecessor(IDKey:%d) 를 찾자)"
		, __FUNCTION__, _nIDKey);
#endif 
	if (ext_tMyNode.tNodeInfo.unID != _nIDKey)
	{
		// predecessor를 찾자
		tPredNodeInfo = find_predecessor(_tSocket, _nIDKey);
		//JC_Print(INFO_MODE, "predNode in findsucc: %d",  predNode.ID);

		// 3-1. predecessor가 나 자신이라면
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s]\t 3-1. predecessor가 나 자신이라면 (tPredNodeInfo.unID == Node.nodeInfo.unID)[%d == %d] "
			, __FUNCTION__, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID);
#endif 
		if (tPredNodeInfo.unID == ext_tMyNode.tNodeInfo.unID)
		{
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "\t\t[%s]\t 3-1. 리턴 Finger[0] :%d"
				, __FUNCTION__, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].unID);
#endif 
			return ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0];
		}
		memset(&tTempMsgHead, 0, sizeof(tTempMsgHead));
		memset(&tMsgHead, 0, sizeof(tMsgHead));
		tMsgHead.usMsgID = DEF_HEAD_MSG_ID_FIND_SUCC;// 5;
		tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
		tMsgHead.sMoreInfo = 0;
		tMsgHead.unBodySize = 0;
		//A에서 A노드에 보내주면 recv에서 블락되서 안빠져나온다.
		//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(predNode.addrInfo.sin_addr), ntohs(predNode.addrInfo.sin_port));
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s]\t 3-2. ## SendTo(DEF_HEAD_MSG_ID_FIND_SUCC, %s:%d) - tPredNodeInfo.unID (%d)] "
			, __FUNCTION__
			, inet_ntoa(tPredNodeInfo.tAddrInfo.sin_addr), ntohs(tPredNodeInfo.tAddrInfo.sin_port)
			, tPredNodeInfo.unID);
#endif 
		nRetVal = sendto(_tSocket, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tPredNodeInfo.tAddrInfo, sizeof(tPredNodeInfo.tAddrInfo));
		if (nRetVal == SOCKET_ERROR) {
			JC_Print(ERROR_MODE, "Successor Info request Sendto error!");
			exit(1);
		}
		nRetVal = recvfrom(_tSocket, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
		//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(tempMsg.tNodeInfo.tAddrInfo.sin_addr), ntohs(tempMsg.tNodeInfo.tAddrInfo.sin_port));
		if (nRetVal == SOCKET_ERROR) 
		{
			JC_Print(WARN_MODE, "Successor Info request Recvfrom error!");
			//exit(1);
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "\t\t[%s]\t 3-2-1. 수신에러(무엇을 주는건가?) ", __FUNCTION__);
			printNodeInfoType(&tTempMsgHead.tNodeInfo, "3-2-1. RecvNodeInfo");
#endif 
			return tTempMsgHead.tNodeInfo;
		}
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 3-3. RecvNodeInfo ", __FUNCTION__);
		printNodeInfoType(&tTempMsgHead.tNodeInfo, "3-3. RecvNodeInfo");
#endif 
		return tTempMsgHead.tNodeInfo;
	}

	//retun NULL;
}

/*
1. 시작: tTempNode = 나 자신
2. 만약 tTempNode == successor(tTempNode): 자기 자신이 유일 노드 → 반환

3. while (_nIDKey ∉ (tTempNode.ID, successor(tTempNode).ID)):
	 a. tTempNode = 가장 가까운 predecessor(tTempNode, _nIDKey)
	 b. tTempNode로부터 successor 정보 요청 → 갱신
	 c. 여전히 구간 밖이면 tTempNode에 직접 find_predecessor 요청 → 반환

4. 구간 안이면 현재 tTempNode가 predecessor → 반환
*/
nodeInfoType find_predecessor(SOCKET _tSocket, int _nIDKey)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	int nRetVal;
	chordHeaderType tTempMsgHead, tMsgHead;
	nodeType tTempNode = ext_tMyNode;

	// 1. successor가 자기 자신이면 (즉, 네트워크에 노드가 1개뿐인 경우), 자기 자신이 predecessor이므로 반환.
	if (tTempNode.tNodeInfo.unID == tTempNode.tChordInfo.tFingerInfo.tFingers[0].unID) // special case: the initial node
	{
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 1. successor가 자기 자신 ", __FUNCTION__);
#endif 
		return tTempNode.tNodeInfo;
	}

	//JC_Print(INFO_MODE, "%d    %d    %d",  IDKey, tempNode.nodeInfo.unID, tempNode.tChordInfo.tFingerInfo.tFingers[0].unID);
	// _nIDKey가 (n, successor(n)) 구간에 포함되지 않는 동안
	while (!modIn(DEF_ID_SPACE, _nIDKey, tTempNode.tNodeInfo.unID, tTempNode.tChordInfo.tFingerInfo.tFingers[0].unID, 0, 1))
	{
		WaitForSingleObject(ext_hMutex, INFINITE);

		// 2-1. 가장 가까운 선행 노드 찾기
		// find_closest_predecessor()를 통해 tTempNode의 finger table에서 _nIDKey에 가장 가까운 predecessor 후보를 찾음

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 2-1. 가장 가까운 선행 노드 찾기 ", __FUNCTION__);
#endif 
		tTempNode.tNodeInfo = find_closest_predecessor(tTempNode, _nIDKey);
		ReleaseMutex(ext_hMutex);
		//JC_Print(INFO_MODE, "temp : %d",  tempNode.nodeInfo.unID);//58을 리턴

		// 2-2. 해당 노드에 successor 정보 요청
		// 이 노드가 successor가 누구인지 다시 확인
		// 받은 successor 정보를 finger[0]에 저장
		memset(&tMsgHead, 0, sizeof(tMsgHead));
		tMsgHead.usMsgID = DEF_HEAD_MSG_ID_FIND_SUCC;//  5;
		tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
		tMsgHead.sMoreInfo = 0;
		tMsgHead.unBodySize = 0;

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 2-2. ## SendTo(DEF_HEAD_MSG_ID_FIND_SUCC, %s:%d)  해당 노드에 successor 정보 요청 "
			, __FUNCTION__
			, inet_ntoa(tTempNode.tNodeInfo.tAddrInfo.sin_addr)
			, ntohs(tTempNode.tNodeInfo.tAddrInfo.sin_port));
#endif 
		nRetVal = sendto(_tSocket, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tTempNode.tNodeInfo.tAddrInfo, sizeof(tTempNode.tNodeInfo.tAddrInfo));
		if (nRetVal == SOCKET_ERROR) {
			JC_Print(ERROR_MODE, "Successor Info request Sendto error!");
			exit(1);
		}

		nRetVal = recvfrom(_tSocket, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
		//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(tempMsg.tNodeInfo.tAddrInfo.sin_addr), ntohs(tempMsg.tNodeInfo.tAddrInfo.sin_port));
		if (nRetVal == SOCKET_ERROR) {
			JC_Print(ERROR_MODE, "Successor Info request Recvfrom error!");
			break;
		}
		tTempNode.tChordInfo.tFingerInfo.tFingers[0] = tTempMsgHead.tNodeInfo;

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 2-3. ## Recv(DEF_HEAD_MSG_ID_FIND_SUCC, %s:%d) SuccNode(%d,  %s:%d)"
			, __FUNCTION__
			, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
			, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
			, tTempNode.tNodeInfo.unID
			, inet_ntoa(tTempNode.tNodeInfo.tAddrInfo.sin_addr)
			, ntohs(tTempNode.tNodeInfo.tAddrInfo.sin_port));
#endif 


#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "\t\t[%s] 2-4. _nIDKey가 (n, successor(n)) 구간에 아직도 없음 → 원격 노드에 직접 predecessor 요청 "
			, __FUNCTION__);
#endif 
		// 2-4. _nIDKey가 (n, successor(n)) 구간에 아직도 없음 → 원격 노드에 직접 predecessor 요청
		// _nIDKey의 predecessor를 요청
		// 반환 받은 노드가 최종 결과
		//JC_Print(INFO_MODE, "%d %d",  tempNode.nodeInfo.unID, tempNode.tChordInfo.tFingerInfo.tFingers[0].unID);//58 39 -> 58 2
		if (!modIn(DEF_ID_SPACE, _nIDKey, tTempNode.tNodeInfo.unID, tTempNode.tChordInfo.tFingerInfo.tFingers[0].unID, 0, 1)) 
		{
			memset(&tMsgHead, 0, sizeof(tMsgHead));
			tMsgHead.usMsgID = DEF_HEAD_MSG_ID_FIND_PRED;// 7;
			tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tMsgHead.tNodeInfo.unID = _nIDKey;
			tMsgHead.sMoreInfo = 0;
			tMsgHead.unBodySize = 0;

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "\t\t[%s]\t2-4-1. ## SendTo(DEF_HEAD_MSG_ID_FIND_PRED, %s:%d)  원격 노드에 직접 predecessor 요청 "
				, __FUNCTION__
				, inet_ntoa(tTempNode.tNodeInfo.tAddrInfo.sin_addr)
				, ntohs(tTempNode.tNodeInfo.tAddrInfo.sin_port));
#endif 
			nRetVal = sendto(_tSocket, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tTempNode.tNodeInfo.tAddrInfo, sizeof(tTempNode.tNodeInfo.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(ERROR_MODE, "Find Predecessor request Sendto error!");
				exit(1);
			}

			nRetVal = recvfrom(_tSocket, (char*)&tTempMsgHead, sizeof(tTempMsgHead), 0, NULL, NULL);
			//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(tempMsg.tNodeInfo.tAddrInfo.sin_addr), ntohs(tempMsg.tNodeInfo.tAddrInfo.sin_port));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(ERROR_MODE, "Find Predecessor request Recvfrom error!");
				exit(1);
			}

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "\t\t[%s]\t2-4-2. ## Recv(DEF_HEAD_MSG_ID_FIND_PRED, %s:%d) PredNode(%d,  %s:%d)"
				, __FUNCTION__
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo.sin_port)
				, tTempMsgHead.tNodeInfo.unID
				, inet_ntoa(tTempMsgHead.tNodeInfo.tAddrInfo.sin_addr)
				, ntohs(tTempMsgHead.tNodeInfo.tAddrInfo.sin_port));
#endif 
			//tempNode.nodeInfo = tempMsg.tNodeInfo;
			return tTempMsgHead.tNodeInfo;
		}
	}

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "\t\t[%s]2-5. Return tTempMsgHead(%d,  %s:%d)"
		, __FUNCTION__
		, tTempNode.tNodeInfo.unID
		, inet_ntoa(tTempNode.tNodeInfo.tAddrInfo.sin_addr)
		, ntohs(tTempNode.tNodeInfo.tAddrInfo.sin_port));
#endif 
	return tTempNode.tNodeInfo;
}

// 지정된 키 _nIDKey보다 직전에 위치한 노드(finger table 상 가장 가까운 선행 노드)를 찾기
nodeInfoType find_closest_predecessor(nodeType _tCurNode, int _nIDKey)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	int i;

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "\t\t[%s]1. finger table 상 가장 가까운 선행 노드 찾기(IDKey:%d) ", __FUNCTION__, _nIDKey);
#endif 
	// 높은 finger는 더 큰 범위를 커버하므로 빠르게 가까운 노드를 찾기 위함
	for (i = DEF_BASE_M - 1; i >= 0; i--) 
	{
		// 비어있는 엔트리 확인
		if (!memcmp(&_tCurNode.tChordInfo.tFingerInfo.tFingers[i], &ext_initNode, sizeof(ext_initNode))) 
			continue;//이거해도 i는 +1된다.

		// 해당 finger 노드가 (n, key) 구간에 있는지 확인
		if (modIn(DEF_ID_SPACE, _tCurNode.tChordInfo.tFingerInfo.tFingers[i].unID, _tCurNode.tNodeInfo.unID, _nIDKey, 0, 0))
		{
			// 1.1 finger[i]가 가장 가까운 predecessor이면 반환
#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
			JC_Print(DEBUG_MODE, "\t\t[%s]\t1-1. return _tCurNode.tChordInfo.tFingerInfo.tFingers[%d] (%d, %s:%d)"
				, __FUNCTION__
				, i
				, _tCurNode.tChordInfo.tFingerInfo.tFingers[i].unID
				, inet_ntoa(_tCurNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
				, ntohs(_tCurNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port));
#endif 
			return _tCurNode.tChordInfo.tFingerInfo.tFingers[i];
		}
	}

#if JC_DEBUG_PING_FIX || JC_DEBUG_JOIN
	JC_Print(DEBUG_MODE, "[%s] 1-2. return 못찾음 자기를 리턴 (%d, %s:%d)"
		, __FUNCTION__
		, i
		, _tCurNode.tNodeInfo.unID
		, inet_ntoa(_tCurNode.tNodeInfo.tAddrInfo.sin_addr)
		, ntohs(_tCurNode.tNodeInfo.tAddrInfo.sin_port));
#endif 
	// 1-2 못 찾은 경우, 자기 자신 반환
	return _tCurNode.tNodeInfo;
}

void notify(nodeInfoType _tTargetNodeInfo)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 

	WaitForSingleObject(ext_hMutex, INFINITE);
	// 1. 현재 pred 정보 가져오기 
	nodeInfoType tPreNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tPre;
	ReleaseMutex(ext_hMutex);

	// 2. pred 업데이트 조건 검사
	if (!memcmp(&tPreNodeInfo, &ext_initNode, sizeof(ext_initNode))  // 현재 pred가 비어 있는 경우
		|| !memcmp(&tPreNodeInfo, &ext_tMyNode.tChordInfo.tFingerInfo.tPre, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tPre))  //현재 pred와 자신의 pred가 같은 경우
		|| modIn(DEF_ID_SPACE, _tTargetNodeInfo.unID, tPreNodeInfo.unID, ext_tMyNode.tNodeInfo.unID, 0, 0)) // 새로운 노드 _tTargetNodeInfo가 기존 pred보다 더 적합한 pred일 경우
	{
		WaitForSingleObject(ext_hMutex, INFINITE);
		// predecessor 갱신
#if JC_DEBUG_JOIN
		JC_Print(DEBUG_MODE, "[%s] predecessor 갱신[%d, %s:%d -> %d, %s:%d]"
			, __FUNCTION__
			, ext_tMyNode.tChordInfo.tFingerInfo.tPre.unID
			, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_addr)
			, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_port)
			, _tTargetNodeInfo.unID
			, inet_ntoa(_tTargetNodeInfo.tAddrInfo.sin_addr)
			, ntohs(_tTargetNodeInfo.tAddrInfo.sin_port));
#endif
		ext_tMyNode.tChordInfo.tFingerInfo.tPre = _tTargetNodeInfo;
		ReleaseMutex(ext_hMutex);

	}
}


