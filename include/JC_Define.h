#pragma once

#ifndef __JC_DEFINE_H__
#define	__JC_DEFINE_H__

#include "../JC_Util/JC_Debug.h"
#include "../JC_Util/JC_Dump.h"


//-------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//-------------------------------------------


///**************************************************************************
//** 운영
//**************************************************************************/
#define DAEMON_NAME		            "iapl_chord"
#define DEF_USING_RECV_WAIT_TIMEVAL	0 // 수신시 Timeout을 사용 
#define DEF_SUB_NODE_CONFIG_FILE	        "../conf/iapl_chord.conf"
#define DEF_DEBUG_LOG_FILE_NAME		"..\\log\\%s_%s.log"

#define SHM_SIZE 1024
#define SHM_NAME "Local\\MySharedMemory" // 공유 메모리 고유 이름

//-------------------------------------------------------------------------------
#define DEF_FILE_NAME_MAX_LEN		80             /* Max length of File Name */
#define DEX_FILE_MAX_CNT			128            /* Max number of Files */
#define DEF_FILE_BUF_LEN			2048         /* file buffer size */
//    * BASE_M     ==>	4 6	8 10 16 20 32 40 50 64 80 100 128

#define JC_DEBUG_SENT_DELAY_KEY		0	// 데이터 전송할때 키입력을 받는다. 
#define DEF_BASE_M					5 // 4 ~ 128           /* base number,   */
#define DEF_ID_SPACE				(int)pow(2, DEF_BASE_M) //1024 //64            /* ringSize = 2^baseM */ID_SPACE

//-------------------------------------------------------------------------------
#define DEF_TIME_PING					2000 // alive check time
#define DEF_RESULT_LOG_CAP_TIME			200	// 로그를 남기기위한 딜레이
#define DEF_MAX_HOP						1000

#define JC_USING_LOAD_SUB_NODE_FOR_TEST		1	// join 및 upload를 자동화 

#define JC_DEBUG_SEARCH_TEST				0	// Search Test 디버깅 모드
///**************************************************************************
//** 디버깅
//**************************************************************************/

#define JC_FUNCTION_CALL                0   // 함수 이름 출력    
#define JC_UTIL_CALL                    0   // Util 함수 출력

#define JC_DEBUG_RECV					1	// 수신 데이터 확인
#define JC_DEBUG_CREAT                  0   // create 디버깅 모드    
#define JC_DEBUG_JOIN					0   // join 디버깅 모드  

	
//---------------------------------------------------------------
// JC_FUNCTION_CALL || 
#define JC_DEBUG_PING_FIX				0	// procPingAndFixFinger 함수 디버깅

//---------------------------------------------------------------

//------------------------------------------------------------




 // 변수 정의 

//-------------------------------------------
#ifdef __cplusplus
}
#endif
//-------------------------------------------

#endif	/* __DEFINE_H__ */

