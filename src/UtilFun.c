#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../include/UtilFun.h"

static const unsigned char sTable[256] =
{
	0xa3, 0xd7, 0x09, 0x83, 0xf8, 0x48, 0xf6, 0xf4, 0xb3, 0x21, 0x15, 0x78, 0x99, 0xb1, 0xaf, 0xf9,
	0xe7, 0x2d, 0x4d, 0x8a, 0xce, 0x4c, 0xca, 0x2e, 0x52, 0x95, 0xd9, 0x1e, 0x4e, 0x38, 0x44, 0x28,
	0x0a, 0xdf, 0x02, 0xa0, 0x17, 0xf1, 0x60, 0x68, 0x12, 0xb7, 0x7a, 0xc3, 0xe9, 0xfa, 0x3d, 0x53,
	0x96, 0x84, 0x6b, 0xba, 0xf2, 0x63, 0x9a, 0x19, 0x7c, 0xae, 0xe5, 0xf5, 0xf7, 0x16, 0x6a, 0xa2,
	0x39, 0xb6, 0x7b, 0x0f, 0xc1, 0x93, 0x81, 0x1b, 0xee, 0xb4, 0x1a, 0xea, 0xd0, 0x91, 0x2f, 0xb8,
	0x55, 0xb9, 0xda, 0x85, 0x3f, 0x41, 0xbf, 0xe0, 0x5a, 0x58, 0x80, 0x5f, 0x66, 0x0b, 0xd8, 0x90,
	0x35, 0xd5, 0xc0, 0xa7, 0x33, 0x06, 0x65, 0x69, 0x45, 0x00, 0x94, 0x56, 0x6d, 0x98, 0x9b, 0x76,
	0x97, 0xfc, 0xb2, 0xc2, 0xb0, 0xfe, 0xdb, 0x20, 0xe1, 0xeb, 0xd6, 0xe4, 0xdd, 0x47, 0x4a, 0x1d,
	0x42, 0xed, 0x9e, 0x6e, 0x49, 0x3c, 0xcd, 0x43, 0x27, 0xd2, 0x07, 0xd4, 0xde, 0xc7, 0x67, 0x18,
	0x89, 0xcb, 0x30, 0x1f, 0x8d, 0xc6, 0x8f, 0xaa, 0xc8, 0x74, 0xdc, 0xc9, 0x5d, 0x5c, 0x31, 0xa4,
	0x70, 0x88, 0x61, 0x2c, 0x9f, 0x0d, 0x2b, 0x87, 0x50, 0x82, 0x54, 0x64, 0x26, 0x7d, 0x03, 0x40,
	0x34, 0x4b, 0x1c, 0x73, 0xd1, 0xc4, 0xfd, 0x3b, 0xcc, 0xfb, 0x7f, 0xab, 0xe6, 0x3e, 0x5b, 0xa5,
	0xad, 0x04, 0x23, 0x9c, 0x14, 0x51, 0x22, 0xf0, 0x29, 0x79, 0x71, 0x7e, 0xff, 0x8c, 0x0e, 0xe2,
	0x0c, 0xef, 0xbc, 0x72, 0x75, 0x6f, 0x37, 0xa1, 0xec, 0xd3, 0x8e, 0x62, 0x8b, 0x86, 0x10, 0xe8,
	0x08, 0x77, 0x11, 0xbe, 0x92, 0x4f, 0x24, 0xc5, 0x32, 0x36, 0x9d, 0xcf, 0xf3, 0xa6, 0xbb, 0xac,
	0x5e, 0x6c, 0xa9, 0x13, 0x57, 0x25, 0xb5, 0xe3, 0xbd, 0xa8, 0x3a, 0x01, 0x05, 0x59, 0x2a, 0x46
};

#define PRIME_MULT 1717
#include <string.h>
uint32_t strToHash(const char* _pchStr) 
{
	unsigned char byHashBuf[SHA_DIGEST_LENGTH];
	uint32_t nKey;

	SHA1(_pchStr, strlen(_pchStr), byHashBuf);
	memcpy(&nKey, byHashBuf + 16, sizeof(nKey));
	return nKey;
}

unsigned int strHash(const char* _pchStr) /* Hash: String to Key */
{
	// [안전장치] NULL 포인터 체크
	if (_pchStr == NULL) return 0;

	unsigned int unLen = (unsigned int)strlen(_pchStr);
	unsigned int unHash = 0;
	unsigned int unHashRet;

	// 디버그 로그 (필요 없으면 주석 처리)
	// fprintf(stderr, "\n-------------------------- # - 1-1  ");

	// =========================================================
	// [수정] SHA-256 사용하도록 코드 통합 (#if 1 제거)
	// =========================================================
	unsigned char byHashBuf[SHA256_DIGEST_LENGTH]; // 32 bytes

	// SHA256 함수 호출
	// _pchStr을 (unsigned char*)로 캐스팅하여 경고 방지
	SHA256((const unsigned char*)_pchStr, unLen, byHashBuf);

	// [해시값 추출]
	// 32바이트 중 마지막 4바이트를 정수형(int)으로 복사
	// (sizeof(unHash)는 보통 4입니다)
	memcpy(&unHash, byHashBuf + (SHA256_DIGEST_LENGTH - sizeof(unHash)), sizeof(unHash));


	// ID Space 나머지 연산
	unHashRet = (unsigned int)(unHash % DEF_ID_SPACE);

	JC_Print(INFO_MODE, "[strHash] [%s](%d) 2. hash(%u) %% ringSize(%u) => unHash(%u)", _pchStr, unLen, unHash, DEF_ID_SPACE, unHashRet);

	return unHashRet;
}

//unsigned int strHash(const char* _pchStr)  /* Hash: String to Key */
//{
//	// [안전장치] NULL 포인터 체크
//	if (_pchStr == NULL) return 0;
//
//	unsigned int unLen = (unsigned int) strlen(_pchStr); // (수정된 코드)
//	unsigned int unHash = unLen;
//	unsigned int unHashRet;
//
//	fprintf(stderr, "\n-------------------------- # - 1-1  ");
//
//	// [변경 1] 버퍼 크기를 SHA-256 상수로 변경 (20 -> 32 bytes)
//#if 1
//	unsigned char byHashBuf[SHA_DIGEST_LENGTH];
//	//uint32_t nKey;
//	
//	fprintf(stderr, "\n-------------------------- # - 1-1  ");
//	SHA1(_pchStr, unLen, byHashBuf);
//	memcpy(&unHash, byHashBuf + 16, sizeof(unHash));
//#else 
//	unsigned char byHashBuf[SHA256_DIGEST_LENGTH]; 
//
//	// [변경 2] SHA1 함수 대신 SHA256 함수 사용
//	SHA256((unsigned char*)_pchStr, unLen, byHashBuf);
//
//	// [변경 3] 해시값 추출 (데이터가 늘어났으니 offset 조절 가능)
//	// 기존에는 끝부분(16)을 썼는데, SHA-256은 32바이트이므로
//	// 뒤쪽을 쓰고 싶다면 offset을 28로 하거나, 그냥 0부터 써도 무방합니다.
//	// 여기서는 기존 로직처럼 '뒤쪽 데이터'를 쓴다고 가정하고 32byte - 4byte = 28 위치를 사용해보겠습니다.
//	// 혹은 단순히 앞부분을 써도 됩니다: memcpy(&unHash, byHashBuf, sizeof(unHash));
//
//	memcpy(&unHash, byHashBuf + (SHA256_DIGEST_LENGTH - sizeof(unHash)), sizeof(unHash));
//#endif 
//
//
//	unHashRet = (unsigned int)(unHash % DEF_ID_SPACE);
//	JC_Print(INFO_MODE, "[strHash] [%s](%d) 2. hash(%u) % ringSize(%u) => unHash(%u)"
//			, _pchStr, strlen(_pchStr), unHash, DEF_ID_SPACE, unHashRet);
//
//
//	return unHashRet;
//}


int recvn(SOCKET _tSock, char* _pchBuf, int _nLen, int _nFlags)
{
	int nReceived;
	char* pchPtr = _pchBuf;
	int nLeft = _nLen; // 수신할 데이터 

	while (nLeft > 0)
	{
		//수신한 데이터
		nReceived = recv(_tSock, pchPtr, nLeft, _nFlags);
		if (nReceived == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		else if (nReceived == 0) {
			break;
		}
		nLeft -= nReceived;
		pchPtr += nReceived;
	}
	return (_nLen - nLeft);
}

unsigned int twoPow(int _nPower)
{
	int i;
	unsigned int unResult = 1;

	if (_nPower >= 0)
		for (i = 0; i < _nPower; i++)
			unResult *= 2;
	else
		unResult = -1;

	return unResult;
}

/* 원형 구조에서 뺄셈 결과 => (_nMinuEnd − _nSubTrand) mod _nModN
* _nModN	모듈로 기준값(보통 2 ^ m 또는 ID 공간 크기)
* _nMinuEnd	감수 : 결과에서 시작되는 값(b)
* _nSubTrand	피감수 : 빼려는 값(a)
*/ 
unsigned int modMinus(unsigned int _nModN, unsigned int _nMinuEnd, unsigned int _nSubTrand)
{
	if (_nMinuEnd - _nSubTrand >= 0)
		return _nMinuEnd - _nSubTrand;
	else
		return (_nModN - _nSubTrand) + _nMinuEnd;

	//return (_nMinuEnd - _nSubTrand + _nModN) % _nModN;
}

/* 원형 구조에서 덧셈 결과 => (_nMinuEnd − _nSubTrand) mod _nModN
* _nModN	모듈로 기준값 (예: 2^m)
* _nAddEnd_1	첫 번째 피연산자 
* _nAddEnd_2	두 번째 피연산자 
*/
unsigned int modPlus(unsigned int _nModN, unsigned int _nAddEnd_1, unsigned int _nAddEnd_2)
{
	if (_nAddEnd_1 + _nAddEnd_2 < _nModN)
		return _nAddEnd_1 + _nAddEnd_2;
	else
		return (_nAddEnd_1 + _nAddEnd_2) - _nModN;

	// return (_nAddEnd_1 + _nAddEnd_2) % _nModN;
}


/* 원형 ID 공간에서 범위 포함 여부 판단
	int _nModN,        // 모듈 기준값 (예: 2^m)
	int _nTargetNum,   // 검사 대상 값
	int _nRange1,      // 구간 시작
	int _nRange2,      // 구간 끝
	int _nLeftMode,    // 왼쪽 포함 여부 (1: 포함, 0: 제외)
	int _nRightMode    // 오른쪽 포함 여부 (1: 포함, 0: 제외)
*/
unsigned int modIn(unsigned int _nModN, unsigned int _nTargetNum, unsigned int _nRange1, unsigned int _nRange2, unsigned int _nLeftMode, unsigned int _nRightMode)
{
	int nResult = 0;

	if (_nRange1 == _nRange2) {
		if ((_nLeftMode == 0) || (_nRightMode == 0))
			return 0;
	}

	if (modPlus(DEF_ID_SPACE, _nRange1, 1) == _nRange2) {
		if ((_nLeftMode == 0) && (_nRightMode == 0))
			return 0;
	}

	if (_nLeftMode == 0)
		_nRange1 = modPlus(DEF_ID_SPACE, _nRange1, 1);
	if (_nRightMode == 0)
		_nRange2 = modMinus(DEF_ID_SPACE, _nRange2, 1);

	if (_nRange1 < _nRange2) {
		if ((_nTargetNum >= _nRange1) && (_nTargetNum <= _nRange2))
			nResult = 1;
	}
	else if (_nRange1 > _nRange2) {
		if (((_nTargetNum >= _nRange1) && (_nTargetNum < _nModN))
			|| ((_nTargetNum >= 0) && (_nTargetNum <= _nRange2)))
			nResult = 1;
	}
	else if ((_nTargetNum == _nRange1) && (_nTargetNum == _nRange2))
		nResult = 1;

	return nResult;
}


char* fgetsCleanup(char* _pchStr)
{
	if (_pchStr[strlen(_pchStr) - 1] == '\n')
		_pchStr[strlen(_pchStr) - 1] = '\0';
	else
		flushStdin();

	return _pchStr;
}

void flushStdin(void)
{
	int _nchChar;

	fseek(stdin, 0, SEEK_END);
	if (ftell(stdin) > 0)
		do
			_nchChar = getchar();
	while (_nchChar != EOF && _nchChar != '\n');
}


void ErrorHandling(char* _pchMsg)
{
	fputs(_pchMsg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void err_quit(char* _pchMsg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)_pchMsg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

void err_display(char* _pchMsg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	JC_Print(INFO_MODE, "[%s] %s", _pchMsg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
