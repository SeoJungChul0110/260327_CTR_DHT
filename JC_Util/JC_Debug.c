#include "./JC_Debug.h"
#include <time.h>  // C

static int st_nLogLevel = DEBUG_MODE;
static char st_cDebugLogFileNameBuf[40];
static char st_cResultLogFileNameBuf[40];

#include <windows.h>
#include <stdio.h>

#define USING_FILE_SHARE_MODE   1

#if USING_FILE_SHARE_MODE
static HANDLE st_hDebugLogFile = NULL;
static HANDLE st_hResultLogFile = NULL;

#else
static FILE* st_LogFileFP = NULL;
#endif 

void SetDebugLogLevel(int _nLevel)
{
    st_nLogLevel = _nLevel;
}

void OpenDebugLogFile(char* _pcLogFile)
{
    memset(st_cDebugLogFileNameBuf, sizeof(st_cDebugLogFileNameBuf), 0);
    sprintf_s(st_cDebugLogFileNameBuf, sizeof(st_cDebugLogFileNameBuf), "%s", _pcLogFile);

    //fprintf(stderr, "\n[%s] %s", __FUNCTION__, cLogFileNameBuf);
    //system("pwd");  // 현재 디렉토리 내용 출력
    //system("dir");  // 현재 디렉토리 내용 출력

    if (strlen(st_cDebugLogFileNameBuf) > 0)
    {
#if USING_FILE_SHARE_MODE
     st_hDebugLogFile = CreateFileA(
        st_cDebugLogFileNameBuf,                   // 로그 파일 이름
        GENERIC_WRITE, //FILE_APPEND_DATA,                   // 쓰기 (파일 끝에 추가)
        FILE_SHARE_READ | FILE_SHARE_WRITE, // 다른 프로세스도 읽기/쓰기 가능
        NULL,                               // 기본 보안
         CREATE_ALWAYS, //OPEN_ALWAYS,                        // 없으면 생성, 있으면 열기
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (st_hDebugLogFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "로그 파일 열기 실패: 오류 코드 %lu\n", GetLastError());
        return;
    }
#else
        errno_t err;
        err = fopen_s(&st_LogFileFP, cLogFileNameBuf, "w");
        if (err != 0 || !st_LogFileFP) {
            perror("로그 파일 초기화 실패");
            return;
        }
        fclose(st_LogFileFP);

        err = fopen_s(&st_LogFileFP, cLogFileNameBuf, "a");
        if (err != 0 || !st_LogFileFP) {
            perror("로그 파일 열기 실패");
            return;
        }
#endif 
    }
}
const char* GetFileName(const char* _pchPath)
{
    const char* p = strrchr(_pchPath, '/');
#ifdef _WIN32
    // Windows의 경우 역슬래시도 고려
    const char* p_win = strrchr(_pchPath, '\\');
    if (p_win && (!p || p_win > p)) p = p_win;
#endif
    return p ? p + 1 : _pchPath;
}

void WriteLog(char* _pcWriteMsg)
{
#if USING_FILE_SHARE_MODE
    if ((strlen(st_cDebugLogFileNameBuf) > 0) && (st_hDebugLogFile))
    {
        DWORD bytesWritten;  
        WriteFile(st_hDebugLogFile, _pcWriteMsg, (DWORD)strlen(_pcWriteMsg), &bytesWritten, NULL);
        //CloseHandle(st_hLogFile);
    }
#else
    if ((strlen(cLogFileNameBuf) > 0) && (st_LogFileFP))
    {
        //        st_LogFileFP =  fopen(cLogFileNameBuf, "a");
        //        if (!st_LogFileFP) {
        //            perror("로그 파일 열기 실패");
        //            return;
        //        }


        fprintf(st_LogFileFP, "%s", _pcWriteMsg);
        //        
        //        // 가변 인자 처리
        //        va_list args;
        //        va_start(args, format);
        //        vfprintf(st_LogFileFP, format, args);
        //        va_end(args);

                //fclose(st_LogFileFP);
    }
#endif 
}
// 로그 파일 닫기
void CloseLogFile() 
{
    if (st_hDebugLogFile) 
    {
        CloseHandle(st_hDebugLogFile);
        st_hDebugLogFile = NULL;
    }
}
void JC_DBGPrint(int _nLevel, const char* fmt, ...)
{
    char chStrBuf[JC_DEBUG_BUF_SIZE];
    memset(chStrBuf, 0, sizeof(chStrBuf));
    va_list args;
    va_start(args, fmt);
    vsprintf_s(chStrBuf, sizeof(chStrBuf), fmt, args);
    va_end(args);


    char chPrintBuf[JC_DEBUG_BUF_SIZE + 100];
    memset(chPrintBuf, 0, sizeof(chPrintBuf));

    if (st_nLogLevel < _nLevel)
        return;
    // 현재 시간 구하기
    time_t now = time(NULL);
    struct tm t;  // 구조체 직접 선언
    errno_t err = localtime_s(&t, &now);  // ✅ 안전하게 사용


    if (_nLevel == ERROR_MODE)
    {
        sprintf_s(chPrintBuf, sizeof(chPrintBuf), "\n%02d%02d_%02d%02d%02d_[E] %s"
            , t.tm_mon + 1, t.tm_mday
            , t.tm_hour, t.tm_min, t.tm_sec
            , chStrBuf);
        fprintf(stderr, "%s", chPrintBuf);
    }
    else if (_nLevel == INFO_MODE)
    {
        sprintf_s(chPrintBuf, sizeof(chPrintBuf), "\n%02d%02d_%02d%02d%02d_[I] %s"
            , t.tm_mon + 1, t.tm_mday
            , t.tm_hour, t.tm_min, t.tm_sec
            , chStrBuf);
        fprintf(stderr, "%s", chPrintBuf);
    }
    else if (_nLevel == WARN_MODE)
    {
        sprintf_s(chPrintBuf, sizeof(chPrintBuf), "\n%02d%02d_%02d%02d%02d_[W] %s"
            , t.tm_mon + 1, t.tm_mday
            , t.tm_hour, t.tm_min, t.tm_sec
            , chStrBuf);
        fprintf(stderr, "%s", chPrintBuf);
    }
    else if (_nLevel == DEBUG_MODE)
    {
#if JC_DEBUG_ENABLE
        sprintf_s(chPrintBuf, sizeof(chPrintBuf), "\n%02d%02d_%02d%02d%02d_[D] %s"
            , t.tm_mon + 1, t.tm_mday
            , t.tm_hour, t.tm_min, t.tm_sec
            , chStrBuf);
#endif
    }
    else //NONE_MODE
    {
        sprintf_s(chPrintBuf, sizeof(chPrintBuf), "\n%s", chStrBuf);
        //            fprintf(stderr, "%s", chStrBuf);
        //WriteLog("%s", chStrBuf);
        fprintf(stderr, "%s", chPrintBuf);
    }


#if USING_FILE_SHARE_MODE
    if (st_hDebugLogFile != NULL)
        WriteLog(chPrintBuf);
#else
    if (st_LogFileFP != NULL)
        WriteLog(chPrintBuf);
#endif 
}


//----------------------

void OpenResultLogFile(char* _pcLogFile)
{
    memset(st_cResultLogFileNameBuf, sizeof(st_cResultLogFileNameBuf), 0);
    sprintf_s(st_cResultLogFileNameBuf, sizeof(st_cResultLogFileNameBuf), "%s", _pcLogFile);

    //fprintf(stderr, "\n[%s] %s", __FUNCTION__, cLogFileNameBuf);
    //system("pwd");  // 현재 디렉토리 내용 출력
    //system("dir");  // 현재 디렉토리 내용 출력

    if (strlen(st_cDebugLogFileNameBuf) > 0)
    {
        st_hResultLogFile = CreateFileA(
            st_cResultLogFileNameBuf,                   // 로그 파일 이름
            GENERIC_WRITE, //FILE_APPEND_DATA,                   // 쓰기 (파일 끝에 추가)
            FILE_SHARE_READ | FILE_SHARE_WRITE, // 다른 프로세스도 읽기/쓰기 가능
            NULL,                               // 기본 보안
            OPEN_ALWAYS,                        // 없으면 생성, 있으면 열기
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (st_hResultLogFile == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "로그 파일 열기 실패: 오류 코드 %lu\n", GetLastError());
            return;
        }
    }
}

void WriteResultLog(const char* fmt, ...)
{
    char chStrBuf[JC_DEBUG_BUF_SIZE];
    memset(chStrBuf, 0, sizeof(chStrBuf));
    va_list args;
    va_start(args, fmt);
    vsprintf_s(chStrBuf, sizeof(chStrBuf), fmt, args);
    va_end(args);


    char chPrintBuf[JC_DEBUG_BUF_SIZE + 100];
    memset(chPrintBuf, 0, sizeof(chPrintBuf));


    sprintf_s(chPrintBuf, sizeof(chPrintBuf), "%s", chStrBuf);
    fprintf(stderr, "[R] %s", chPrintBuf);

    if (st_hResultLogFile != NULL)
        WritenResult(chPrintBuf);
}

// 로그 파일 닫기
void CloseResultLogFile()
{
    if (st_hResultLogFile)
    {
        CloseHandle(st_hResultLogFile);
        st_hResultLogFile = NULL;
    }
}


void WritenResult(char* _pcWriteMsg)
{
    if ((strlen(st_cResultLogFileNameBuf) > 0) && (st_hResultLogFile))
    {
        DWORD bytesWritten;
        WriteFile(st_hResultLogFile, _pcWriteMsg, (DWORD)strlen(_pcWriteMsg), &bytesWritten, NULL);
        //CloseHandle(st_hLogFile);
    }
}


//---------------
void  ShareWriteResultLog(char* _pcLogFile, const char* fmt, ...)
{
#if 0
    char chStrBuf[JC_DEBUG_BUF_SIZE];
    memset(chStrBuf, 0, sizeof(chStrBuf));

    va_list args;
    va_start(args, fmt);
    vsprintf_s(chStrBuf, sizeof(chStrBuf), fmt, args);
    va_end(args);


    char chPrintBuf[JC_DEBUG_BUF_SIZE + 100];
    memset(chPrintBuf, 0, sizeof(chPrintBuf));


    sprintf_s(chPrintBuf, sizeof(chPrintBuf), "%s", chStrBuf);
    //fprintf(stderr, "[S] %s", chPrintBuf);

    if (st_hResultLogFile != NULL)
        ShareWritenResult(_pcLogFile, chPrintBuf);
#else// 1. 버퍼 크기를 넉넉하게 잡습니다. (필요에 따라 조절)
    char chPrintBuf[JC_DEBUG_BUF_SIZE];
    memset(chPrintBuf, 0, sizeof(chPrintBuf));

    va_list args;
    va_start(args, fmt);

    // 2. vsnprintf_s를 사용하여 버퍼 크기를 넘지 않도록 안전하게 기록합니다.
    // _TRUNCATE를 사용하면 버퍼가 모자랄 경우 자르고 종료하여 크래시를 방지합니다.
    vsnprintf_s(chPrintBuf, sizeof(chPrintBuf), _TRUNCATE, fmt, args);

    va_end(args);

    // 3. 로그 파일 핸들 확인 및 기록
    // st_hResultLogFile이 전역 변수라면 그대로 유지, 
    // 아니라면 ShareWritenResult 내부에서 체크하도록 설계하는 것이 좋습니다.
    if (st_hResultLogFile != NULL)
    {
        ShareWritenResult(_pcLogFile, chPrintBuf);
    }
#endif 
}
// 여러 프로세스가 로그 작성

void ShareWritenResult(char* _pcLogFile, char* _pcWriteMsg) {

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD  dwWritten = 0;
    OVERLAPPED ol;
    ZeroMemory(&ol, sizeof(ol));
    
    // 로그 파일 열기 (없으면 생성, 있으면 이어쓰기)
    hFile = CreateFileA(
        _pcLogFile,
        FILE_APPEND_DATA,                  // 필요 시 GENERIC_READ | FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "\n파일 열기 실패: %d", GetLastError());
        return;
    }

    //// 파일 잠금 영역 설정
    //if (!LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &ol)) {
    //    fprintf(stderr, "\n파일 잠금 실패: %d", GetLastError());
    //    CloseHandle(hFile);
    //    return;
    //}

    WriteFile(hFile, _pcWriteMsg, strlen(_pcWriteMsg), &dwWritten, NULL);

    // 잠금 해제
    //UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &ol);

    CloseHandle(hFile);
}
