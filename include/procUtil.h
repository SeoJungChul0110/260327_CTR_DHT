#pragma once

#include "DefStruct.h"
#include "UtilFun.h"
#include "PrintFunc.h"
#pragma comment(lib,"ws2_32")
nodeType ext_tMyNode;               // node information -> global variable

SOCKET ext_tRecvProcSock;
SOCKET ext_tReqSock, ext_flSock, ext_frSock, ext_fsSock, ext_pfSock;
HANDLE ext_hMutex;
nodeInfoType ext_initNode;

int ext_nSilentMode; // silent mode


int move_keys();
int fix_finger();

void notify(nodeInfoType targetNode);


nodeInfoType FindSuccessor(SOCKET _tSocket, int IDKey);
// For finding successor of IDKey for a node curNode

nodeInfoType find_predecessor(SOCKET _tSocket, int _nIDKey);
// For finding predecessor of IDKey for a node curNode

nodeInfoType find_closest_predecessor(nodeType _tCurNode, int _nIDKey);
// For finding closest predecessor of IDKey for a node curNode


int lookup();