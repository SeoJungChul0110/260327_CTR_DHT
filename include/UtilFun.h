#pragma once

#include "DefStruct.h"

#include <openssl/sha.h>
#include <stdio.h>

// A Simple Hash Function from a string to the ID/key space
uint32_t strToHash(const char*);
unsigned strHash(const char*);


// For receiving a file
int recvn(SOCKET _tSock, char* _pchBuf, int _nLen, int _nFlags);


// For getting a power of 2 
unsigned int twoPow(int _nPower);

// For modN modular operation of "minend - subtrand"
unsigned int modMinus(unsigned int _nModN, unsigned int _nMinuEnd, unsigned int _nSubTrand);

// For modN modular operation of "addend1 + addend2"
unsigned int modPlus(unsigned int _nModN, unsigned int _nAddEnd_1, unsigned int _nAddEnd_2);

// For checking if _nTargetNum is "in" the range using left and right modes 
// under modN modular environment 
unsigned int modIn(unsigned int _nModN, unsigned int _nTargetNum, unsigned int _nRange1, unsigned int _nRange2, unsigned int _nLeftMode, unsigned int _nRightMode);

// For handling fgets function
char* fgetsCleanup(char*);

// For flushing stdin
void flushStdin(void);


void ErrorHandling(char* _pchMsg);
void err_quit(char* _pchMsg);
void err_display(char* _pchMsg);

