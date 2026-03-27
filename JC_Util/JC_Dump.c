#include "./JC_Dump.h"

#include <ctype.h>

#if 1
void printchar(unsigned char c)
{
	if(isprint(c))
		fprintf(stderr,  "%c", c);
	else
		fprintf(stderr,  ".");
}


void JC_DumpMsgLog(char * _pchTitleBuf, unsigned char *_pbyBuff, int _nLen)
{
    int i;
    fprintf(stderr, "\n----------- %s[%d Bytes] -----------\n", _pchTitleBuf, _nLen);
    if (_nLen > 2048)    _nLen = 2048;
    for (i = 0; i < _nLen; i++)
    {
        if (i % 16 == 0)
            fprintf(stderr, "0x%08x  ", (unsigned int)&_pbyBuff[i]);
        fprintf(stderr, "%02x ", _pbyBuff[i]);
        if (i % 16 - 15 == 0)
        {
            int j;
            fprintf(stderr, "  ");
            for (j = i - 15; j <= i; j++)
                printchar(_pbyBuff[j]);
            fprintf(stderr, "\n");
        }
    }

    if (i % 16 != 0)
    {
        int j;
        int spaces = (_nLen - i + 16 - i % 16) * 3 + 2;
        for (j = 0; j < spaces; j++)
            fprintf(stderr, " ");
        for (j = i - i % 16; j < _nLen; j++)
            printchar(_pbyBuff[j]);
    }
    fprintf(stderr, "\n");
}

void JC_DumpMsg(unsigned char *_pbyBuff, int _nLen)
{
    int i;
    fprintf(stderr, "\n[%d Bytes]\n", _nLen);
    for (i = 0; i < _nLen; i++)
    {
        if (i % 16 == 0)
            fprintf(stderr, "0x%08x  ", (unsigned int)&_pbyBuff[i]);
        fprintf(stderr, "%02x ", _pbyBuff[i]);
        if (i % 16 - 15 == 0)
        {
            int j;
            fprintf(stderr, "  ");
            for (j = i - 15; j <= i; j++)
                printchar(_pbyBuff[j]);
            fprintf(stderr, "\n");
        }
    }

    if (i % 16 != 0)
    {
        int j;
        int spaces = (_nLen - i + 16 - i % 16) * 3 + 2;
        for (j = 0; j < spaces; j++)
            fprintf(stderr, " ");
        for (j = i - i % 16; j < _nLen; j++)
            printchar(_pbyBuff[j]);
    }
    fprintf(stderr, "\n");
}

void JC_PrintBuf(unsigned char *_pbyBuff, int _nLen)
{
    int i;
    fprintf(stderr,  "\nDATA(%d bytes )]\n ", _nLen);
    for(i=0;i<_nLen;i++)
    {
        fprintf(stderr,  "0x%x, ",_pbyBuff[i]);
        if(i%32-31==0)
        {
//                int j;
//                fprintf(stderr,  "  ");
//                for(j=i-15;j<=i;j++)
//                        printchar(_pbyBuff[j]);
                fprintf(stderr,  "\n");
        }
    }    
    fprintf(stderr,  "\n");
}
void JC_PrintBufLog(char * _pchTitleBuf, unsigned char *_pbyBuff, int _nLen)
{
    int i;
    fprintf(stderr,  "\n%s(%d bytes )]\n", _pchTitleBuf, _nLen);
    for(i=0;i<_nLen;i++)
    {
        fprintf(stderr,  "%02x ",_pbyBuff[i]);
        if(i%32-31==0)
        {
//                int j;
//                fprintf(stderr,  "  ");
//                for(j=i-15;j<=i;j++)
//                        printchar(_pbyBuff[j]);
                //fprintf(stderr,  "\n");
        }
    }    
                fprintf(stderr,  "\n");
}
void JC_PrintChar(unsigned char c)
{
	if(isprint(c))
		fprintf(stderr, "%c", c);
	else
		fprintf(stderr, ".");
}
#endif 
