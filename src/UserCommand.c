#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/UserCommand.h"
#include "../include/JC_Test.h"


//  User input processing (switch) 
void showCommand(void)
{
	fprintf(stderr, "\n\n%s", OUTER_DELIMETER);
	fprintf(stderr, "\n\t(c)reate: Create the chord network");
	fprintf(stderr, "\n\t(j)oin  : Join the chord network");
	fprintf(stderr, "\n\t(l)eave : Leave the chord network");
	fprintf(stderr, "\n\t%s", INNER_DELIMETER);
	fprintf(stderr, "\n\t(a)dd   : Add a file to the network");
	fprintf(stderr, "\n\t(d)elete: Delete a file to the network");
	fprintf(stderr, "\n\t(s)earch: File search and download");
	fprintf(stderr, "\n\t%s", INNER_DELIMETER);
	fprintf(stderr, "\n\t(f)inger: Show the finger table");
	fprintf(stderr, "\n\t(i)nfo  : Show the node information");
	fprintf(stderr, "\n\t(p)print  : Show Node");
	fprintf(stderr, "\n\t%s", INNER_DELIMETER);
	fprintf(stderr, "\n\t(n)oad  : Load SubNode(Config");
	fprintf(stderr, "\n\t(t)Test  : Test");
	fprintf(stderr, "\n\t%s", INNER_DELIMETER);
	fprintf(stderr, "\n\t(m)ute  : Toggle the silent mode");
	fprintf(stderr, "\n\t(h)elp  : Show the help message");
	fprintf(stderr, "\n\t%s", INNER_DELIMETER);
	fprintf(stderr, "\n\t(q)uit  : Quit the program");
}


char getUserCommand(void)
{
	char command[7];
	char cmdChar = '\0';
	while (1)
	{
		fprintf(stderr, "\nCHORD> Enter command ('help' for help message).");
		fprintf(stderr, "\nCHORD> ");
		fgets(command, sizeof(command), stdin);
		fgetsCleanup(command);
		if (!strcmp(command, "c") || !strcmp(command, "create"))
			cmdChar = 'c';
		else if (!strcmp(command, "j") || !strcmp(command, "join"))
			cmdChar = 'j';
		else if (!strcmp(command, "l") || !strcmp(command, "leave"))
			cmdChar = 'l';
		else if (!strcmp(command, "a") || !strcmp(command, "add"))
			cmdChar = 'a';
		else if (!strcmp(command, "d") || !strcmp(command, "delete"))
			cmdChar = 'd';
		else if (!strcmp(command, "s") || !strcmp(command, "search"))
			cmdChar = 's';
		else if (!strcmp(command, "f") || !strcmp(command, "finger"))
			cmdChar = 'f';
		else if (!strcmp(command, "i") || !strcmp(command, "info"))
			cmdChar = 'i';
		else if (!strcmp(command, "h") || !strcmp(command, "help"))
			cmdChar = 'h';
		else if (!strcmp(command, "m") || !strcmp(command, "mute"))
			cmdChar = 'm';
		else if (!strcmp(command, "q") || !strcmp(command, "quit"))
			cmdChar = 'q';
		else if (!strcmp(command, "p") || !strcmp(command, "print"))
			cmdChar = 'p';
		else if (!strcmp(command, "t") || !strcmp(command, "test"))
			cmdChar = 't';
		else if (!strcmp(command, "n") || !strcmp(command, "node"))
			cmdChar = 'n';
		else if (!strlen(command))
			continue;
		else {
			fprintf(stderr, "\n[ERROR] Wrong command! Input a correct command.\n");
			continue;
		}
		break;
	}
	return cmdChar;
}
// For showing commands

void procUserCommand(char _chCmdChar)
{
#if JC_FUNCTION_CALL
	JC_FUNC(DEBUG_MODE, "");
#endif 
	HANDLE hThread[2];
	int nExitFlag = 0; // indicates termination condition
	char chCmdChar = '\0';
	int nJoinFlag = 0; // indicates the join/create status
	char chFileNameBuf[DEF_FILE_NAME_MAX_LEN + 1];
	int nKeyCnt;
	struct sockaddr_in tTargetSockAddr;
	chordHeaderType tTmpMsgHead, tMsgHead;
	int nOptVal = 5000;  // 5 seconds
	int nRetVal; // return value
	nodeInfoType tSuccNodeInfo = { 0 }, tPredNodeInfo = { 0 }, tTargetNodeInfo = { 0 };
	fileInfoType tFileInfo;
	fileRefType tFileRefInfo;
	FILE* pFileFp = NULL;
	char* pchBody = NULL;
	int i, j, k, nTargetKey, nResultFlag;
	memset(chFileNameBuf, 0, sizeof(chFileNameBuf));

	switch (_chCmdChar)
	{
		case 'l':
		{
			memset(&tMsgHead, 0, sizeof(tMsgHead));
			//WaitForSingleObject(hMutex, INFINITE);
			//자기가 소유한 파일의 레퍼런스 값들도 다른노드에서 지워주고 가야한다.
			JC_Print(INFO_MODE, "CHORD> Leave start");
			tSuccNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0];
			memset(&tFileInfo, 0, sizeof(tFileInfo));
			nKeyCnt = 0;
			tMsgHead.unBodySize = 0;
			for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++) {
				for (k = 0; k < ext_tMyNode.tFileInfo.nFileNum; k++) {
					if (ext_tMyNode.tFileInfo.tFileRef[k].nKey == ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey)
					{
						for (j = i; j < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum - 1; j++) {
							WaitForSingleObject(ext_hMutex, INFINITE);
							ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[j] = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[j + 1];
							ReleaseMutex(ext_hMutex);
						}
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tChordInfo.tFRefInfo.nFileNum--;
						ReleaseMutex(ext_hMutex);
						i--;// 배열 땡겼으니까

						for (j = k; j < ext_tMyNode.tFileInfo.nFileNum - 1; k++) {
							WaitForSingleObject(ext_hMutex, INFINITE);
							ext_tMyNode.tFileInfo.tFileRef[j] = ext_tMyNode.tFileInfo.tFileRef[j + 1];
							ReleaseMutex(ext_hMutex);
						}
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tFileInfo.nFileNum--;
						ReleaseMutex(ext_hMutex);
						k--;
					}
				}
			}

			//자기가 자기의 소유 파일 레퍼런스 소유한 값을 지워준다.

			for (i = 0; i < ext_tMyNode.tFileInfo.nFileNum; i++) {
				tMsgHead.usMsgID = 10;
				tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
				tMsgHead.sMoreInfo = ext_tMyNode.tFileInfo.tFileRef[i].nKey;
				tMsgHead.unBodySize = 0;
				nRetVal = sendto(ext_tReqSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tFileInfo.tFileRef[i].tRefOwnerNodeInfo.tAddrInfo, sizeof(ext_tMyNode.tFileInfo.tFileRef[i].tRefOwnerNodeInfo.tAddrInfo));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "File Reference Delete msg request for LeaveSendto error!");
					exit(1);
				}
				nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "CHORD> File Reference Delete msg request for Leave Recvfrom error!");
					exit(1);
				}
				//배열 당겨주면 좋지만 별 상관없다.
			}

			for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++) {
				tFileInfo.tFileRef[nKeyCnt] = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i];
				tMsgHead.unBodySize += sizeof(tFileInfo.tFileRef[nKeyCnt]);
				//JC_Print(INFO_MODE, "uuuuu : %s, %d",  keysInfo.fileRef[keyCount].Name, keysInfo.fileRef[keyCount].Key);
				nKeyCnt++;

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

			JC_Print(INFO_MODE, "keyCount: %d, bodySize: %d", nKeyCnt, tMsgHead.unBodySize);

			pchBody = &tFileInfo.tFileRef[0];
			tMsgHead.usMsgID = 8;
			tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tMsgHead.sMoreInfo = nKeyCnt;
			nRetVal = sendto(ext_tReqSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tSuccNodeInfo.tAddrInfo, sizeof(tSuccNodeInfo.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "LeaveKeys request Sendto error!");
				exit(1);
			}
			nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "LeaveKeys request Recvfrom error!");
				exit(1);
			}
			JC_Print(INFO_MODE, "kkkkeyCount: %d, bodySize: %d", nKeyCnt, tMsgHead.unBodySize);
			nRetVal = sendto(ext_tReqSock, (char*)pchBody, tMsgHead.unBodySize, 0, (struct sockaddr*)&tSuccNodeInfo.tAddrInfo, sizeof(tSuccNodeInfo.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "LeaveKeys request body Sendto error!");
				exit(1);
			}

			nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "LeaveKeys request body Recvfrom error!");
				exit(1);
			}

			tMsgHead.usMsgID = 4;
			tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tMsgHead.tNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tPre;
			tMsgHead.sMoreInfo = 0;

			nRetVal = sendto(ext_tReqSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0].tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "predecessor Update request Sendto error!");
				exit(1);
			}
			nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "predecessor Update request Recvfrom error!");
				exit(1);
			}

			tMsgHead.usMsgID = 6;
			tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tMsgHead.tNodeInfo = ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0];
			tMsgHead.sMoreInfo = 0;

			nRetVal = sendto(ext_tReqSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo, sizeof(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "predecessor Update request Sendto error!");
				exit(1);
			}
			nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "predecessor Update request Recvfrom error!");
				exit(1);
			}
			//ReleaseMutex(hMutex);
			exit(1);

			break;
		}// end case
		case 'i':
		{
			JC_Print(NONE_MODE, "[# My Node Info]");
			JC_Print(NONE_MODE, "\tMyNode\t\t (%s : %d) ID: %4d"
				, inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port), ext_tMyNode.tNodeInfo.unID);

			JC_Print(NONE_MODE, "\tFile Info");
			for (i = 0; i < ext_tMyNode.tFileInfo.nFileNum; i++)
			{
				JC_Print(NONE_MODE, "\t\t%2d th own file name: %s, key: %d", i + 1, ext_tMyNode.tFileInfo.tFileRef[i].chNameBuf, ext_tMyNode.tFileInfo.tFileRef[i].nKey);
			}
			JC_Print(NONE_MODE, "\tFile Ref Info");
			for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++)
			{
				JC_Print(NONE_MODE, "\t\t%2dth file ref name: %s, key: %d, owner ID: %d", i + 1, ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].chNameBuf, ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey, ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].tOwnerNodeInfo.unID);
			}
			break;
		}// end case
		case 'f':
		{
			JC_Print(NONE_MODE, "# MyNode\t\t (%s : %d) ID: %4d"
				, inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port), ext_tMyNode.tNodeInfo.unID);
			JC_Print(NONE_MODE, "# Predecessor\t (%s : %d) ID: %4d"
				, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tPre.tAddrInfo.sin_port), ext_tMyNode.tChordInfo.tFingerInfo.tPre.unID);
			JC_Print(NONE_MODE, "# FingerTable(Size : %d)(Range ~%d)", DEF_BASE_M, DEF_ID_SPACE);
			for (int i = 0; i < DEF_BASE_M; i++) 
			{
				//JC_Print(NONE_MODE, " [%2d](%s:%d)ID: [%4d, START:%04d] - R:%3d, S:%3d", i
				JC_Print(NONE_MODE, " [%2d](%s:%d)[ID:%4d] R:%3d, S:%3d", i
					, inet_ntoa(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_addr)
					, ntohs(ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].tAddrInfo.sin_port)
					, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].unID
					//, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nStart
					, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nRecvPacketCnt
					, ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nSendPacketCnt);
			}
			JC_Print(NONE_MODE, "# ConfigSubNode(Size : %d)", ext_tConfig.nNodeCnt);
			for (i = 0; i < ext_tConfig.nNodeCnt; i++)
			{
				JC_Print(NONE_MODE, "\t[%2d] (%s : %d) ID: [%4d]"
					, i
					, inet_ntoa(ext_tConfig.tSubNodes[i].tAddrInfo.sin_addr), ntohs(ext_tConfig.tSubNodes[i].tAddrInfo.sin_port)
					, ext_tConfig.tSubNodes[i].unID);
			}


			/*JC_Print(INFO_MODE, "[READ] 1: [%s]\n", ext_pSHMBuf);
			sprintf_s(ext_pSHMBuf, SHM_SIZE, "Hello, Shared Memory! (Time: %d)", GetTickCount());
			JC_Print(INFO_MODE, "[READ] 2: [%s]\n", ext_pSHMBuf);*/

			break;
		}// end case
		case 'd':
		{
			JC_Print(INFO_MODE, "CHORD> Enter the file name to delete: ");
			scanf("%s", chFileNameBuf);
			nTargetKey = strHash(chFileNameBuf);
			tSuccNodeInfo = FindSuccessor(ext_tReqSock, nTargetKey);
			if (ext_tMyNode.tNodeInfo.unID == tSuccNodeInfo.unID)
			{
				for (i = 0; i < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum; i++) {
					if (ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i].nKey == nTargetKey) {
						for (j = i; j < ext_tMyNode.tChordInfo.tFRefInfo.nFileNum - 1; j++) {
							WaitForSingleObject(ext_hMutex, INFINITE);
							ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i] = ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[i + 1];
							ReleaseMutex(ext_hMutex);
						}
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tChordInfo.tFRefInfo.nFileNum--;
						ReleaseMutex(ext_hMutex);
						i--;// 배열 땡겼으니까
					}
				}
			}
			else {
				memset(&tMsgHead, 0, sizeof(tMsgHead));
				tMsgHead.usMsgID = 10;
				tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
				tMsgHead.sMoreInfo = nTargetKey;
				tMsgHead.unBodySize = 0;

				nRetVal = sendto(ext_tReqSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tSuccNodeInfo.tAddrInfo, sizeof(tSuccNodeInfo.tAddrInfo));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(ERROR_MODE, "File Reference Delete msg request Sendto error!");
					exit(1);
				}
				nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
				if (nRetVal == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						JC_Print(ERROR_MODE, "\a[ERROR] File Reference Delete msg timed out. File Add Failed!");
						return;
					}
					JC_Print(ERROR_MODE, "CHORD> File Reference Delete msg request Recvfrom error!");
					return;
				}

				if ((tTmpMsgHead.usMsgID != 10) || (tTmpMsgHead.usMsgType  != 1)) { // wrong msg
					JC_Print(ERROR_MODE, "\a[ERROR] Wrong Message (not FileRefAdd) Received. File Add Failed!");
					return;
				}

				if (tTmpMsgHead.sMoreInfo == -1) { // failure
					JC_Print(ERROR_MODE, "\a[ERROR] FileRefAdd Request Failed. File Add Failed!");
					return;
				}
			}

			for (i = 0; i < ext_tMyNode.tFileInfo.nFileNum; i++) 
			{
				if (ext_tMyNode.tFileInfo.tFileRef[i].nKey == nTargetKey) 
				{
					for (j = i; j < ext_tMyNode.tFileInfo.nFileNum - 1; j++) {
						WaitForSingleObject(ext_hMutex, INFINITE);
						ext_tMyNode.tFileInfo.tFileRef[i] = ext_tMyNode.tFileInfo.tFileRef[i + 1];
						ReleaseMutex(ext_hMutex);
					}
					WaitForSingleObject(ext_hMutex, INFINITE);
					ext_tMyNode.tFileInfo.nFileNum--;
					ReleaseMutex(ext_hMutex);
					i--;// 배열 땡겼으니까
				}
			}
			JC_Print(INFO_MODE, "CHORD> The file has been successfully deleted ");
			break;
		}// end case
		case 'm':
		{
			if (ext_nSilentMode == 1)
				ext_nSilentMode = 0;
			else
				ext_nSilentMode = 1;
			break;
		}// end case
		case 's':
		{
			lookup();
			break;
		}// end case
		case 'a':
		{
			nResultFlag = 0;
			WaitForSingleObject(ext_hMutex, INFINITE);
			if (ext_tMyNode.tFileInfo.nFileNum == DEX_FILE_MAX_CNT) { // file number is full 
				JC_Print(INFO_MODE, "\a[ERROR] Your Cannot Add more file. File Space is Full!\n");
				return;
			}
			ReleaseMutex(ext_hMutex);
			JC_Print(INFO_MODE, "CHORD> Files to be added must be in the same folder where this program is located.");
			JC_Print(INFO_MODE, "CHORD> Note that the maximum file name size is 32.");
#if JC_USING_LOAD_SUB_NODE_FOR_TEST
			memset(chFileNameBuf, 0, sizeof(chFileNameBuf));
			sprintf(chFileNameBuf, "./file/%s_%d", inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr), ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port));
#else
			JC_Print(INFO_MODE, "CHORD> Enter the file name to add: ");
			//scanf("%s", fileName);
			fgets(chFileNameBuf, sizeof(chFileNameBuf), stdin);
			fgetsCleanup(chFileNameBuf);
#endif 
			// check if the file exits
			if ((pFileFp = fopen(chFileNameBuf, "rb")) == NULL) {
				JC_Print(INFO_MODE, "\a[ERROR] The file '%s' is not in the same folder where this program is!\n", chFileNameBuf);
				return;
			}
			fclose(pFileFp);

			WaitForSingleObject(ext_hMutex, INFINITE);
			strcpy(ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].chNameBuf, chFileNameBuf);
			ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].nKey = strHash(chFileNameBuf);
			ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].tOwnerNodeInfo = ext_tMyNode.tNodeInfo;
			JC_Print(INFO_MODE, "CHORD> Input File Name: %s, Key: %d", chFileNameBuf, ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].nKey);
			ReleaseMutex(ext_hMutex);
			tSuccNodeInfo = FindSuccessor(ext_tReqSock, ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].nKey);

			WaitForSingleObject(ext_hMutex, INFINITE);
			ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].tRefOwnerNodeInfo = tSuccNodeInfo;
			ReleaseMutex(ext_hMutex);
			JC_Print(INFO_MODE, "CHORD> File Successor IP Addr: %s, Port No: %d, ID: %d", 
				inet_ntoa(ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].tRefOwnerNodeInfo.tAddrInfo.sin_addr), 
				ntohs(ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].tRefOwnerNodeInfo.tAddrInfo.sin_port), 
				ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].tRefOwnerNodeInfo.unID);
			WaitForSingleObject(ext_hMutex, INFINITE);
			if (ext_tMyNode.tNodeInfo.unID == ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum].tRefOwnerNodeInfo.unID)
			{
				ext_tMyNode.tChordInfo.tFRefInfo.tFileRef[ext_tMyNode.tChordInfo.tFRefInfo.nFileNum] = ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum];
				JC_Print(INFO_MODE, "CHORD> File Ref Info has been sent successfully to the Successor.");
				JC_Print(INFO_MODE, "CHORD> File Add has been successfully finished.");
				ext_tMyNode.tFileInfo.nFileNum++;
				ext_tMyNode.tChordInfo.tFRefInfo.nFileNum++;
				break;
			}
			tFileRefInfo = ext_tMyNode.tFileInfo.tFileRef[ext_tMyNode.tFileInfo.nFileNum];
			ReleaseMutex(ext_hMutex);
			//소켓통신은 자기가 자기에게 보내면 에러가 나므로 주의하자!

			memset(&tMsgHead, 0, sizeof(tMsgHead));
			tMsgHead.usMsgID = 9;
			tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
			tMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
			tMsgHead.sMoreInfo = 1;
			tMsgHead.tFileIRef = tFileRefInfo;
			tMsgHead.unBodySize = 0;
			nRetVal = sendto(ext_tReqSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tSuccNodeInfo.tAddrInfo, sizeof(tSuccNodeInfo.tAddrInfo));
			if (nRetVal == SOCKET_ERROR) {
				JC_Print(INFO_MODE, "File Reference Add msg request Sendto error!");
				exit(1);
			}
			nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
			if (nRetVal == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					JC_Print(INFO_MODE, "\a[ERROR] FileRefAdd Request timed out. File Add Failed!");
					return;
				}
				JC_Print(INFO_MODE, "CHORD> File Reference Add msg request Recvfrom error!");
				return;
			}

			if ((tTmpMsgHead.usMsgID != 9) || (tTmpMsgHead.usMsgType  != 1)) { // wrong msg
				JC_Print(INFO_MODE, "\a[ERROR] Wrong Message (not FileRefAdd) Received. File Add Failed!");
				return;
			}

			if (tTmpMsgHead.sMoreInfo == -1) { // failure
				JC_Print(INFO_MODE, "\a[ERROR] FileRefAdd Request Failed. File Add Failed!");
				return;
			}

			WaitForSingleObject(ext_hMutex, INFINITE);
			ext_tMyNode.tFileInfo.nFileNum++;
			ReleaseMutex(ext_hMutex);
			JC_Print(INFO_MODE, "CHORD> File Ref Info has been sent successfully to the Successor.");
			JC_Print(INFO_MODE, "CHORD> File Add has been successfully finished.");

			break;
		}// end case
		case 'c':
		{
			// 프로세스의 동작 상태 확인
			if (nJoinFlag)
			{
				JC_Print(WARN_MODE, "\a[ERROR] You are currently in the network; You cannot create the network!\n");
				return;
			}

#if JC_DEBUG_CREAT
			JC_Print(NONE_MODE, "\n\n");
			JC_Print(DEBUG_MODE, "## Create (%s:%d)"
				, inet_ntoa(ext_tMyNode.tNodeInfo.tAddrInfo.sin_addr)
				, ntohs(ext_tMyNode.tNodeInfo.tAddrInfo.sin_port));
#endif 
			// 1. Pre에 현재 노드정보를 등록
			// 2. FingerTable에 현재 노드정보를 등록
			nJoinFlag = 1;
			ext_tMyNode.tChordInfo.tFingerInfo.tPre = ext_tMyNode.tNodeInfo;
			for(i = 0; i < DEF_BASE_M; i++)
			{
				ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i] = ext_tMyNode.tNodeInfo;
			}
			JC_Print(INFO_MODE, "CHORD> You have created a chord network!");
			JC_Print(INFO_MODE, "CHORD> Your finger table has been updated!");

			hThread[0] = (HANDLE)_beginthreadex(NULL, 0, (void*)procRecvMsg, (void*)&nExitFlag, 0, NULL);//뒤에서 세번째가 인자주는 넘이다.
			hThread[1] = (HANDLE)_beginthreadex(NULL, 0, (void*)procPingAndFixFinger, (void*)&nExitFlag, 0, NULL);

#if JC_DEBUG_CREAT
			//printNodeType(&ext_tMyNode);
#endif 
			break;
		}// end case
		case 'j':
		{
			JC_Print(INFO_MODE, "CHORD> You need a helper node to join the existing network.");
			char chJoinNodeIPAddrBuf[16];
			char chJoinNodePortBuf[6];
			memset(chJoinNodeIPAddrBuf, 0, sizeof(chJoinNodeIPAddrBuf));
			memset(chJoinNodePortBuf, 0, sizeof(chJoinNodePortBuf));
#if JC_USING_LOAD_SUB_NODE_FOR_TEST
			sprintf_s(chJoinNodeIPAddrBuf, sizeof(chJoinNodeIPAddrBuf), "%s", "127.0.0.1");
			sprintf_s(chJoinNodePortBuf, sizeof(chJoinNodePortBuf), "%s", "5000");
#else
			JC_Print(INFO_MODE, "CHORD> If you want to create a network, the helper node is yourself.");
			JC_Print(INFO_MODE, "CHORD> Enter IP address of the helper node: ");
			scanf("%s", chTempIPAddrBuf);
			JC_Print(INFO_MODE, "CHORD> Enter port number of the helper node: ");
			scanf("%s", chTempPortBuf);
#endif 

#if JC_DEBUG_JOIN
			JC_Print(NONE_MODE, "\n\n");
			JC_Print(DEBUG_MODE, "## JOIN To (%s:%s)", chJoinNodeIPAddrBuf, chJoinNodePortBuf);
#endif 
			// subNode 값을 임의로 등록했을대는 사용을 안한다.
			if (ext_bUsePingAndFixFinger == 1)
			{
				memset(&tMsgHead, 0, sizeof(tMsgHead));
				tMsgHead.usMsgID = DEF_HEAD_MSG_ID_JOIN;
				tMsgHead.usMsgType  = DEF_HEAD_MSG_TYPE_REQ;
				tMsgHead.tNodeInfo = ext_tMyNode.tNodeInfo;
				tMsgHead.sMoreInfo = 0;
				tMsgHead.unBodySize = 0;

				memset(&tTargetSockAddr, 0, sizeof(tTargetSockAddr));
				tTargetSockAddr.sin_family = AF_INET;
				tTargetSockAddr.sin_addr.s_addr = inet_addr(chJoinNodeIPAddrBuf);
				tTargetSockAddr.sin_port = htons(atoi(chJoinNodePortBuf));
	#if JC_DEBUG_JOIN
				JC_Print(DEBUG_MODE, "[%s] ## SendTo(DEF_HEAD_MSG_ID_JOIN, %s:%d) - MyNode.nodeInfo(%d, %s:%d)] "
					, __FUNCTION__
					, inet_ntoa(tTargetSockAddr.sin_addr)
					, ntohs(tTargetSockAddr.sin_port)
					, tMsgHead.tNodeInfo.unID
					, inet_ntoa(tMsgHead.tNodeInfo.tAddrInfo.sin_addr)
					, ntohs(tMsgHead.tNodeInfo.tAddrInfo.sin_port));
	#endif 
				//JC_Print(INFO_MODE, "%s, %d",  inet_ntoa(targetAddr.sin_addr), ntohs(targetAddr.sin_port));
				nRetVal = sendto(ext_tReqSock, (char*)&tMsgHead, sizeof(tMsgHead), 0, (struct sockaddr*)&tTargetSockAddr, sizeof(tTargetSockAddr));
				if (nRetVal == SOCKET_ERROR) {
					JC_Print(INFO_MODE, "Join Info request Sendto error(%s:%d)!"
						, inet_ntoa(tMsgHead.tNodeInfo.tAddrInfo.sin_addr)
						, ntohs(tMsgHead.tNodeInfo.tAddrInfo.sin_port));
					exit(1);
				}
				JC_Print(INFO_MODE, "CHORD> JoinInfo request Message has been sent.");

				nRetVal = recvfrom(ext_tReqSock, (char*)&tTmpMsgHead, sizeof(tTmpMsgHead), 0, NULL, NULL);
				if (nRetVal == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						JC_Print(INFO_MODE, "\a[ERROR] Join Info Request timed out. File Add Failed!");
						return;
					}
					JC_Print(INFO_MODE, "Join Info request Recvfrom error!");
					return;
				}

				// 응답 메시지 수신 
				if ((tTmpMsgHead.usMsgID != DEF_HEAD_MSG_ID_JOIN) || (tTmpMsgHead.usMsgType  != DEF_HEAD_MSG_TYPE_RSP))
				{ // wrong msg
					JC_Print(WARN_MODE, "\a[ERROR] Wrong Message (not DEF_HEAD_MSG_ID_JOIN) Received. DEF_HEAD_MSG_ID_JOIN Failed!");
					return;
				}

				if (tTmpMsgHead.sMoreInfo == -1) { // failure
					JC_Print(WARN_MODE, "\a[ERROR] Wrong Message (not DEF_HEAD_MSG_ID_JOIN) Received. DEF_HEAD_MSG_ID_JOIN Failed!");
					return;
				}

	#if JC_DEBUG_JOIN
				JC_Print(DEBUG_MODE, "[%s] ## Recv(DEF_HEAD_MSG_ID_JOIN, %s:%d) - Msg(%s:%d) "
					, __FUNCTION__
					, inet_ntoa(tTargetSockAddr.sin_addr)
					, ntohs(tTargetSockAddr.sin_port)
					, inet_ntoa(tTmpMsgHead.tNodeInfo.tAddrInfo.sin_addr)
					, ntohs(tTmpMsgHead.tNodeInfo.tAddrInfo.sin_port));
	#endif 
				JC_Print(INFO_MODE, "CHORD> JoinInfo response Message has been received(%s:%d)."
					, inet_ntoa(tTmpMsgHead.tNodeInfo.tAddrInfo.sin_addr), ntohs(tTmpMsgHead.tNodeInfo.tAddrInfo.sin_port));
				WaitForSingleObject(ext_hMutex, INFINITE);
			
				ext_tMyNode.tChordInfo.tFingerInfo.tFingers[0] = tTmpMsgHead.tNodeInfo;
				ReleaseMutex(ext_hMutex);
				JC_Print(INFO_MODE, "CHORD> You got your successor node from the helper node.");
				JC_Print(INFO_MODE, "CHORD> Successor IP Addr: %s, Port No: %d, ID: %d", inet_ntoa(tTmpMsgHead.tNodeInfo.tAddrInfo.sin_addr), ntohs(tTmpMsgHead.tNodeInfo.tAddrInfo.sin_port), tTmpMsgHead.tNodeInfo.unID);

				move_keys();
	#if JC_DEBUG_JOIN
				//printNodeType(&ext_tMyNode);
	#endif 

				stabilizeJoin(ext_tReqSock);
				fix_finger();

			}
#if JC_DEBUG_JOIN
			//printNodeType(&ext_tMyNode);
#endif 
			hThread[0] = (HANDLE)_beginthreadex(NULL, 0, (void*)procRecvMsg, (void*)&nExitFlag, 0, NULL);//뒤에서 세번째가 인자주는 넘이다.
			hThread[1] = (HANDLE)_beginthreadex(NULL, 0, (void*)procPingAndFixFinger, (void*)&nExitFlag, 0, NULL);
			break;
		}// end case
		case 'h':
		{
			showCommand();

			JC_Print(INFO_MODE, "[SHM_READ] (%d) [%s]\n", ext_pSHMBuf->nCnt, ext_pSHMBuf->chMsgBuf);
			break;
		}// end case
		case 'p':
		{
			printNodeType(&ext_tMyNode);
			break;
		}// end case
		case 't':
		{
			// 노드 찾기 테스트 
			SearchNodeTest();

			break;
		}// end case
		case 'n':
		{
			JC_Print(INFO_MODE, "=> ext_bUsePingAndFixFinger = %d", ext_bUsePingAndFixFinger);
			if (ext_bUsePingAndFixFinger == 1) // procPingAndFixFinger함수를 사용중이면, 정리 
			{
				ext_bUsePingAndFixFinger = 0;
				JC_Print(INFO_MODE, "=> 1 ext_bUsePingAndFixFinger = %d", ext_bUsePingAndFixFinger);
				SetConfigSubNode(&ext_tConfig);
			}
			else
			{
				ext_bUsePingAndFixFinger = 1;
				JC_Print(INFO_MODE, "=> 2 ext_bUsePingAndFixFinger = %d", ext_bUsePingAndFixFinger);
				return;
			}
			// FingerTalle 설정
			//printNodeType(&ext_tMyNode);
			break;
		}// end case
		default:
		{
			JC_Print(ERROR_MODE, "[%s] Not Found Command Cahr(%c)", __FUNCTION__, _chCmdChar);
		}

	}
}