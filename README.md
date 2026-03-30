"# 260327\_CTR\_DHT"

#  create a new repository on the command line

echo "# Test" >> README.md
git init
git add README.md
git commit -m "first commit"
git branch -M main
git remote add origin https://github.com/SeoJungChul0110/260327\_CTR\_DHT.git
git push -u origin main


# push an existing repository from the command line
git remote add origin https://github.com/SeoJungChul0110/260327\_CTR\_DHT.git
git branch -M main
git push -u origin main

# 1. 준비 사항
	1. 메인 Node 실행 > 1_MainNode.bat
		iapl_chord.exe c 6000
		- 메인 Node 생성 :  (c)reate
			
	2-1. 테스트용 Node 실행 
		> 2_SubNode.bat
			Input number of instacne: 9

		iapl_chord.exe j 6001~6009
		- 서브 Node 생성 : (c)reate,
		- 메인 Node에 조인 : (j)oin
		- 파일 등록 :	(a)dd
	
	2-2. 테스트용 메시지 보내기 (Main Node에서) 
		- SearchNodeTest()
		- ShareWriteResultLog
		[동작]

	3. 테스트용 Node 종료 > 3_StopNode.bat
		
		
# 2. 테스트 내용(기본 DHT)
	0. 설정 변경 
		# 검색할때 패킷 카운트를 사용한다, USING_TEST_NODE_IDX가 0이상이면 전송 횟수값
		USING_PACKET_CNT_FOR_TEST=0
		# 테스트할 노드 인덱스 -1이 아니면, 선택
		USING_TEST_NODE_IDX=-1
	
		# define SHA_DIGEST_LENGTH 20

		#define DEF_BASE_M					
			4	6	8	10	16	32	64	128	512	1024

	1. 메인 Node 실행 > 1_MainNode.bat
		iapl_chord.exe c 6000
		- 메인 Node 생성 :  (c)reate
			
	2-1. 테스트용 Node 실행 
		> 2_SubNode.bat
			Input number of instacne: 9

		iapl_chord.exe j 6001~6009
		- 서브 Node 생성 : (c)reate,
		- 메인 Node에 조인 : (j)oin
		- 파일 등록 :	(a)dd
	
	2-2. 테스트용 메시지 보내기 (Main Node에서) 
		- SearchNodeTest()
		- ShareWriteResultLog
		[동작]

	3. 테스트용 Node 종료 > 3_StopNode.bat

# 3. 테스트 내용(패킷 카운)
	0. 설정 변경 
		# 테스트할 노드 인덱스 -1이 아니면, 선택
		USING_TEST_NODE_IDX=1
		# 검색할때 패킷 카운트를 사용한다, USING_TEST_NODE_IDX가 0이상이면 전송 횟수값
		USING_PACKET_CNT_FOR_TEST=1

		int ext_nUsingPacketCntForTest
		int ext_nUsingTestNodeIdx
		typedef struct
		{
			int nID;
			struct sockaddr_in tAddrInfo;
			char chIPAddrBuf[64];
			int nPort;
			int nRecvPacketCnt; // 수신된 패킷 수
			int nSendPacketCnt; // 송신 패킷 수
		} SubNodeInfo;
		typedef struct {             /* Finger Table Structure */
			nodeInfoType tPre;          /* Predecessor pointer */
			nodeInfoType tFingers[DEF_BASE_M];   /* Fingers (array of pointers) */
		} fingerInfoType;
		

		# 수신 처리 
		case DEF_HEAD_MSG_ID_FIND_NODE: //13:
			ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nRecvPacketCnt++;
		# 송신 처리 
			ext_tMyNode.tChordInfo.tFingerInfo.tFingers[i].nSendPacketCnt++;

		# 패킷 카운트를 사용해서 노드 변경
		CalNextNodeByPacketCnt
			// 1️. 중복 제거
			// 2. 노드 Index 찾기 
		------------------------


		# define SHA_DIGEST_LENGTH 20

		#define DEF_BASE_M					
			4	6	8	10	16	32	64	128	512	1024

# 4. 제안 모델 실행 흐름도
![제안 모델 실행 흐름도](https://github.com/SeoJungChul0110/260327_CTR_DHT/blob/db57ba69e7dc7a763aaaa5271d9d48deacfd470a/ReadMe/1.%20%EC%A0%9C%EC%95%88%20%EB%AA%A8%EB%8D%B8%20%20%EC%8B%A4%ED%96%89%20%ED%9D%90%EB%A6%84%EB%8F%84.png)

