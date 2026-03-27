
#########################################################################
# ChatGPT

#########################################################################
# 260114 준비 사항
	1. Xshell7 실행
	2. 실행 위치 이동 [D:\Hongik\Source\DHT_250630\8_ChordProtocol-master\bin]

	3-2. 소스 확인 및 컴파일
		> 노드 갯수		JC_Define.h > DEF_BASE_M
		> 로그 딜레이	JC_Define.h > DEF_RESULT_LOG_CAP_TIME
		
	3-1. 설정 파일 확인
		conf/iapl_chord.conf

	4. 기본 테스트
		> 메인 Node 생성		iapl_chord.exe c 6000
		> Finger Table 확인		f 
			# MyNode		 (127.0.0.1 : 5000) ID:   46
			# Predecessor	 (127.0.0.1 : 5000) ID:   46
			# FingerTable(Size : 6)(Range ~64)
			 [ 0](127.0.0.1:6004)[ID:  56] R:  0, S:  0
			 [ 1](127.0.0.1:6004)[ID:  56] R:  0, S:  0
			 [ 2](127.0.0.1:6004)[ID:  56] R:  0, S:  0
			 [ 3](127.0.0.1:6004)[ID:  56] R:  0, S:  0
			 [ 4](127.0.0.1:6003)[ID:   0] R:  0, S:  0
			 [ 5](127.0.0.1:6000)[ID:  15] R:  0, S:  0
			# ConfigSubNode(Size : 6)
				[ 0] (127.0.0.1 : 6003) ID: [   0]
				[ 1] (127.0.0.1 : 6001) ID: [   9]
				[ 2] (127.0.0.1 : 6000) ID: [  15]
				[ 3] (127.0.0.1 : 6002) ID: [  40]
				[ 4] (127.0.0.1 : 6005) ID: [  41]
				[ 5] (127.0.0.1 : 6004) ID: [  56]
		
		> 서브 Node 생성	iapl_chord.exe j 6001 ~
		~
# 260114 테스트 내용(기본 DHT)
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

# 260210 테스트 내용(패킷 카운드 )
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

# 260114 테스트 결과 분석
	1. 결과파일 읽기
		쉽표, 공백 
	2. 32개 부터 못찾음
		
		0121_175119_[I] [strHash] [127.0.0.16030](13) 2. hash(3831993107)- ringSize(2147483648) => unHash(1684509459)
0121_175119_[I] Search Fingle Table [SerchNode (ID:1684509459)(127.0.0.1:6030) !!!
0121_175119_[I] No closer node found in Finger Table..(tNextNode.unID == ext_tMyNode.tNodeInfo.unID=> 719389391, 719389391
0121_175119_[I] CHORD> 노드 찾기  27_(32, 2147483648d), (127.0.0.1:6029)>
0121_175120_[I] [strHash] [127.0.0.16029](13) 2. hash(3983935179)- ringSize(2147483648) => unHash(1836451531)
0121_175120_[I] Search Fingle Table [SerchNode (ID:1836451531)(127.0.0.1:6029) !!!
0121_175120_[I] No closer node found in Finger Table..(tNextNode.unID == ext_tMyNode.tNodeInfo.unID=> 719389391, 719389391
0121_175120_[I] CHORD> 노드 찾기  28_(32, 2147483648d), (127.0.0.1:6001)>
0121_175121_[I] [strHash] [127.0.0.16001](13) 2. hash(1923583881)- ringSize(2147483648) => unHash(1923583881)
0121_175121_[I] Search Fingle Table [SerchNode (ID:1923583881)(127.0.0.1:6001) !!!
0121_175121_[I] No closer node found in Finger Table..(tNextNode.unID == ext_tMyNode.tNodeInfo.unID=> 719389391, 719389391
0121_175121_[I] CHORD> 노드 찾기  29_(32, 2147483648d), (127.0.0.1:6010)>
0121_175122_[I] [strHash] [127.0.0.16010](13) 2. hash(1942336390)- ringSize(2147483648) => unHash(1942336390)
0121_175122_[I] Search Fingle Table [SerchNode (ID:1942336390)(127.0.0.1:6010) !!!
0121_175122_[I] No closer node found in Finger Table..(tNextNode.unID == ext_tMyNode.tNodeInfo.unID=> 719389391, 719389391
0121_175122_[I] CHORD> 노드 찾기  30_(32, 2147483648d), (127.0.0.1:6007)>
0121_175123_[I] [strHash] [127.0.0.16007](13) 2. hash(4107969009)- ringSize(2147483648) => unHash(1960485361)
0121_175123_[I] Search Fingle Table [SerchNode (ID:1960485361)(127.0.0.1:6007) !!!
0121_175123_[I] No closer node found in Finger Table..(tNextNode.unID == ext_tMyNode.tNodeInfo.unID=> 719389391, 719389391
0121_175123_[I] CHORD> 노드 찾기  31_(32, 2147483648d), (127.0.0.1:6004)>
0121_175124_[I] [strHash] [127.0.0.16004](13) 2. hash(4150551800)- ringSize(2147483648) => unHash(2003068152)
0121_175124_[I] Search Fingle Table [SerchNode (ID:2003068152)(127.0.0.1:6004) !!!
0121_175124_[I] No closer node found in Finger Table..(t


