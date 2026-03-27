#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "./JC_Config.h"

int FileCheck(char *_pchFileName, char *_pchSectorName, char *_pchValue)
{
    FILE	*pFileFp = NULL;
    char	chLineBuf[MAX_CFG_LINE];
    int	i, copied = 0;
    char	*pchVp;
    char	*pchPp;
    errno_t tErr;

    //if((fp = fopen(_pchFileName, "r")) == NULL)
    tErr = fopen_s(&pFileFp, _pchFileName, "r");
    if (pFileFp != 0 || pFileFp == NULL)
    {
            return -1;
    }

    while(fgets(chLineBuf, MAX_CFG_LINE, pFileFp) != NULL) {
        i = strlen(chLineBuf) - 1;
        if(chLineBuf[i] == '\n') {
                chLineBuf[i] = '\0';
        }
        if((pchPp = SkipWhite(chLineBuf)) == NULL) {
                continue;
        }
        pchVp = strchr(pchPp, '=');

        if(pchVp == (char *) 0) {
                continue;
        }
        *pchVp++ = '\0';
        if((pchVp = SkipWhite(pchVp)) == NULL) {
                continue;
        }
        if(strcmp(_pchSectorName, pchPp) == 0) {
            copied = 1;

            //strcpy(_pchValue, vp);
            strcpy_s(_pchValue, strlen(pchVp), pchVp);
            break;
        }
    }

    fclose(pFileFp);

    if(copied) {
            return 0;
    }

    return (-1);
}
int ReadConfig(char *_pchFileName, char *_pchSectorName, char *_pchValue)
{
    FILE* pFileFP = NULL;
    char	chLineBuf[MAX_CFG_LINE];
    int	i, nCopied = 0;
    char	*pchVP;
    char	*pchPP;    errno_t tErr;

    //if((fp = fopen(_pchFileName, "r")) == NULL)
    tErr = fopen_s(&pFileFP, _pchFileName, "r");
    if (tErr != 0 || pFileFP == NULL)
    {
            fprintf(stderr, "\n## JC_UTIL]  ReadConfig error: %s", _pchFileName);
            perror("fopen_s");
            return -1;
    }
    else
    {
#if JC_CONFIG_DEBUG
            fprintf(stderr, "\n## JC_UTIL] File Name:%s, pname:%s", _pchFileName, _pchSectorName);
#endif
    }

#if 1
    while (fgets(chLineBuf, sizeof(chLineBuf), pFileFP))
    {
        char fileKey[128], fileValue[128];

        // 키=값 형식 파싱
        if (sscanf_s(chLineBuf, "%127[^=]=%127s", fileKey, (unsigned)_countof(fileKey),
            fileValue, (unsigned)_countof(fileValue)) == 2)
        {
            if (strcmp(fileKey, _pchSectorName) == 0)
            {
                strcpy_s(_pchValue, sizeof(fileValue), fileValue);
                fclose(pFileFP);
                return 0; // 성공
            }
        }
    }
#else
    while(fgets(chLineBuf, MAX_CFG_LINE, pFileFP) != NULL) 
    {
        i = strlen(chLineBuf) - 1;
        if(chLineBuf[i] == '\n') {
                chLineBuf[i] = '\0';
        }
        if((pchPP = SkipWhite(chLineBuf)) == NULL) {
                continue;
        }
        pchVP = strchr(pchPP, '=');

        if(pchVP == (char *) 0) {
                continue;
        }
        *pchVP++ = '\0';
        if((pchVP = SkipWhite(pchVP)) == NULL) {
                continue;
        }
        if(strcmp(_pchSectorName, pchPP) == 0) {
                nCopied = 1;

               // strcpy(_pchValue, vp);
                strcpy_s(_pchValue, strlen(pchVP), pchVP);
                break;
        }
    }
#endif 

#if JC_CONFIG_DEBUG
    fprintf(stderr, "\n## JC_UTIL] _pchValue\n[%s]", _pchValue);
#endif
    fclose(pFileFP);

    if(nCopied) {
            return 0;
    }

    return (-1);
}

char* SkipWhite(char *ptr)
{
    if (ptr == NULL) {
            return (NULL);
    }
    while (*ptr != 0 && isspace(*ptr)) {
            ptr++;
            if (*ptr == 0 || *ptr == '#') {
                    return (NULL);
            }
    }
    return (ptr);
}

char* SkipNotWhite(char *ptr)
{
    if (ptr == NULL) {
            return (NULL);
    }
    while (*ptr != 0 && !isspace(*ptr)) {
            ptr++;
            if (*ptr == 0 || *ptr == '#') {
                    return (NULL);
            }
    }
    return (ptr);
}

// 라인단위로 읽음
int ReadDataByLine(char *_pchInData, int _nInDataLen, char *_pchOutValue, char * _pSearchStr)
{
    //char * pLineStr = "\n\r";
    char * pPtr = _pchInData;
    int nValueLen =0;
    char* pEndStringPtr = strstr(pPtr, _pSearchStr);
    
    //fprintf(stderr, "#=>[ReadDataByLine] (%d)[%s]\n", _nInDataLen, pPtr);
    if(_nInDataLen > 0)
    {        
        // 개행 문자를 찾으면, 문자열을 복사해서 전달
        if(pEndStringPtr != NULL)
        {
            nValueLen = pEndStringPtr - pPtr;
            memcpy(_pchOutValue, pPtr, nValueLen);
            //_pchOutValue[nValueLen] = '\0';
            
            //fprintf(stderr, "#=> return(%d)\n", nValueLen + strlen(_pSearchStr));
            return nValueLen + strlen(_pSearchStr);
        }
        else
        {
            if(_nInDataLen > 0)
            {
                nValueLen = _nInDataLen;
                memcpy(_pchOutValue, pPtr, nValueLen);
                //_pchOutValue[nValueLen] = '\0';
                
                //fprintf(stderr, "#=>[_nInDataLen > 0)] return(%d)\n", nValueLen);
                return nValueLen;
            }
            else
            {
                //fprintf(stderr, "#=>[_nInDataLen =< 0)] return(0)\n");
                return 0;
            }
        }
    }
    else
    {
        //fprintf(stderr, "#=> return(-1)\n");
        return -1;
    }
    
}



// count: 연결할 문자열 개수
char* ConcatString(int _nStrCnt, ...)
{
    if (_nStrCnt <= 0) return NULL;

    va_list tArgsList;
    va_start(tArgsList, _nStrCnt);

    // 총 길이 계산
    size_t nTotalLen = 0;
    for (int i = 0; i < _nStrCnt; i++) {
        const char* str = va_arg(tArgsList, const char*);
        if (str) nTotalLen += strlen(str);
    }
    va_end(tArgsList);

    // 메모리 할당
    char* result = (char*)malloc(nTotalLen + 1); // +1 for null terminator
    if (!result) return NULL;

    result[0] = '\0';  // 빈 문자열로 초기화

    // 다시 문자열들을 붙임
    va_start(tArgsList, _nStrCnt);
    for (int i = 0; i < _nStrCnt; i++) {
        const char* str = va_arg(tArgsList, const char*);
        if (str) strcat(result, str);
    }
    va_end(tArgsList);

    return result;

    /*

    int main()
{
    char* combined = concat_n(4, "Node_", "1", ":", "5000");
    if (combined) {
        printf("%s\n", combined); // 출력: Node_1:5000
        free(combined);          // 동적 메모리 해제!
    }

    return 0;
}
    */
}
