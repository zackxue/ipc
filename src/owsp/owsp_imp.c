#include "owsp_imp.h"
#include "owsp_debug.h"

#include "socket_rw.h"

#define HTTP_WRITE_TIMEOUT 5
#define MAX_CAM_CH 16
#define DEVICE_MODEL "DVRXXX"
#define OWSP_RESPONSE_LEN (sizeof(OwspPacketHeader)+sizeof(TLV_HEADER)+512)

//yqw test
#define     MAX_MOB_BITRATE 128
#define     USC_MOB_WIDTH   352
#define     USC_MOB_HEIGHT  288
#define     USC_MOB_BITRATE 96
#define     MIN_MOB_BITRATE 48
#define     USC_MOB_FRMRATE 8
#define     USC_MOB_IKEYGOP (2*USC_MOB_FRMRATE)

#define SHM_BUF_SIZE (1*1024*1024) //缓冲区大小
#define MAX_PKG_SIZE (SHM_BUF_SIZE/10) //最大数据包大小


extern int  SendSock(int Sock, char SndBuf[], unsigned int SndSize);

int OWSP_SendStreamDataFormat(OWSP_CONX _owsp, TLV_V_StreamDataFormat _StrmInfo)
{
	TLV_V_StreamDataFormat StrmInfo;
    memset(&StrmInfo, 0, sizeof(StrmInfo));

    memcpy(&StrmInfo.videoFormat.codecId, "H264", 4);

    StrmInfo.videoChannel = _owsp.Chnl;
    StrmInfo.audioChannel = 0;
    StrmInfo.dataType = OWSP_SDT_VIDEO_ONLY;
    StrmInfo.videoFormat.bitrate = USC_MOB_BITRATE*1000;
    StrmInfo.videoFormat.width   = 640;
    StrmInfo.videoFormat.height  = 360;
    StrmInfo.videoFormat.framerate  = USC_MOB_FRMRATE;
    StrmInfo.videoFormat.colorDepth = 24;

	char BuffRet[OWSP_RESPONSE_LEN];

    if(OWSP_SendPacket(&_owsp, TLV_T_STREAM_FORMAT_INFO,
        BuffRet, (char *)&StrmInfo, sizeof(StrmInfo)) <= 0) {
        return -1;
    }

	printf("TLV_V_StreamDataFormat sent.\n");
	return 0;

}


int OWSP_SendFrame(OWSP_CONX _owsp, char *buf, int size, int bIFrame)
{
#if 0//so inconceivable
	if (new_connect) {
		OWSP_TRACE("!!!!!!!!!!!TEST!!!!!!!!!!");
		TLV_V_StreamDataFormat StrmInfo;
	    memset(&StrmInfo, 0, sizeof(StrmInfo));

	    memcpy(&StrmInfo.videoFormat.codecId, "H264", 4);

	    StrmInfo.videoChannel = _owsp.Chnl;
	    StrmInfo.audioChannel = 0;
	    StrmInfo.dataType = OWSP_SDT_VIDEO_ONLY;
	    StrmInfo.videoFormat.bitrate = USC_MOB_BITRATE*1000;
	    StrmInfo.videoFormat.width   = 640;
	    StrmInfo.videoFormat.height  = 360;
	    StrmInfo.videoFormat.framerate  = USC_MOB_FRMRATE;
	    StrmInfo.videoFormat.colorDepth = 24;

		char BuffRet[OWSP_RESPONSE_LEN];

	    if(OWSP_SendPacket(&_owsp, TLV_T_STREAM_FORMAT_INFO,
	        BuffRet, (char *)&StrmInfo, sizeof(StrmInfo)) <= 0) {
	        return -1;
	    }

//		printf("TLV_V_StreamDataFormat sent.\n");
	}
#endif
	TLV_V_StreamDataFormat _StrmInfo;//so inconceivable

	OWSP_FRMI FRMI;
	struct timeval tmpVal;
    gettimeofday(&tmpVal, NULL);

    FRMI.iChnl = 0;
    FRMI.iFrmCnt ++;
    FRMI.tStmp = (tmpVal.tv_sec % (24*3600))*1000;
    FRMI.iType = bIFrame;

    char tmpSndBuf[MAX_PKG_SIZE+MAX_PKG_SIZE/2];
    int nRet = OWSP_VFrm_Create(&FRMI, _owsp.PackCnt ++, tmpSndBuf, buf, size);

    /*if(SendSock(_owsp.Sock, tmpSndBuf, nRet) <= 0) {
        return -1;
    }*/

	SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, _owsp.Sock, (void *)tmpSndBuf, nRet, HTTP_WRITE_TIMEOUT);
	SOCKETRW_writen(&rw_ctx);
	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
	{
		OWSP_TRACE("errno :%d\r\n", errno);
		return -1;
	}
	return rw_ctx.actual_len;
}

int  OWSP_Sock_InitParam(int Socket)
{
    int nRet;

    struct linger  LINGER;
    LINGER.l_onoff = 1;
    LINGER.l_linger = 1;

    nRet = setsockopt( Socket,
        SOL_SOCKET, SO_LINGER,
        (char *)&LINGER, sizeof(LINGER));
    if( nRet != 0 ) {
        return nRet;
    }

    struct timeval RCV_TO;
    RCV_TO.tv_sec = 5;
    RCV_TO.tv_usec = 0;
    nRet = setsockopt( Socket,
        SOL_SOCKET, SO_RCVTIMEO,
        (char *)&RCV_TO, sizeof(RCV_TO));
    if( nRet != 0 ) {
        return nRet;
    }

    struct timeval SND_TO;
    SND_TO.tv_sec = 5;
    SND_TO.tv_usec = 0;
    nRet = setsockopt( Socket,
        SOL_SOCKET, SO_SNDTIMEO,
        (char *)&SND_TO, sizeof(SND_TO));
    if( nRet != 0 ) {
        return nRet;
    }

    fcntl( Socket, F_SETFL, (fcntl(Socket, F_GETFL, 0) & O_NONBLOCK));
    //fcntl(Socket, F_SETFL, flags &(~O_NONBLOCK));

    return 0;
}



int  OWSP_SendPacket(OWSP_CONX *pConX,
    int nType, char Buff[], char pPack[], int nSize)
{
    int  nRet = OWSP_Pack_Create(nType,
            pConX->PackCnt ++,  //Here to Auto Increase
            Buff, pPack, nSize);
    /*if(SendSock(pConX->Sock, Buff, nRet) <= 0) {
        return -1;
    }//*/

	SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, pConX->Sock, (void *)Buff, nRet, HTTP_WRITE_TIMEOUT);
	SOCKETRW_writen(&rw_ctx);

	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
	{
		return -1;
	}
	else
	{
		return rw_ctx.actual_len;
	}

    return !0;
}

int  OWSP_Pack_Create(int nType, int iPackNum, char Buff[], char pPack[], int nSize)
{
	OwspPacketHeader PckHdr;
	TLV_HEADER       TlvHdr;

    int  tmpPckLen = 0;
    int  RetPckLen = 0;

	memset(&PckHdr, 0, sizeof(PckHdr));
	memset(&TlvHdr, 0, sizeof(TlvHdr));

    TlvHdr.tlv_type = nType;
    TlvHdr.tlv_len  = nSize;

	tmpPckLen += sizeof(TlvHdr);
	tmpPckLen += nSize;

	PckHdr.packet_length = htonl(tmpPckLen + 4);
	PckHdr.packet_seq    = iPackNum;

	memcpy(Buff + RetPckLen, &PckHdr, sizeof(PckHdr));
	RetPckLen += sizeof(PckHdr);

	memcpy(Buff + RetPckLen, &TlvHdr, sizeof(TlvHdr));
	RetPckLen += sizeof(TlvHdr);

	memcpy(Buff + RetPckLen, pPack, nSize);
	RetPckLen += nSize;

    return RetPckLen;
}

int  OWSP_TlvHdr_Write(char Buff[], int nType, int nSize)
{
    TLV_HEADER TlvHdr;
    TlvHdr.tlv_type  = nType;
    TlvHdr.tlv_len   = nSize;

    memcpy(Buff, &TlvHdr, sizeof(TlvHdr));

    return sizeof(TlvHdr);
}

int  OWSP_VFrm_Create(OWSP_FRMI * pFrmI, int iPackNum, char Buff[], char pPack[], int nSize)
{
	OwspPacketHeader			PckHdr;
	TLV_V_VideoFrameInfoEx	    TlvVFrmEx;

    memset( &TlvVFrmEx, 0, sizeof(TlvVFrmEx));

    TlvVFrmEx.channelId  = pFrmI->iChnl;
    TlvVFrmEx.frameIndex = pFrmI->iFrmCnt;
    TlvVFrmEx.time       = pFrmI->tStmp;
    TlvVFrmEx.dataSize   = nSize;

    char *pBuff = Buff + sizeof(PckHdr);  //Skip PacketHdr Write

    if(nSize >= 0xFFFF) {
        OWSP_TlvHdr_Write(pBuff,
            TLV_T_VIDEO_FRAME_INFO_EX, sizeof(TlvVFrmEx));
        pBuff += sizeof(TLV_HEADER);
        memcpy(pBuff, &TlvVFrmEx, sizeof(TlvVFrmEx));
        pBuff += sizeof(TlvVFrmEx);
    }
    else {  //Forece to Write Bytes of sizeof(TLV_V_VideoFrameInfo)
        int tmpFrmSize = sizeof(TlvVFrmEx)-sizeof(TlvVFrmEx.dataSize);
        OWSP_TlvHdr_Write(pBuff,
            TLV_T_VIDEO_FRAME_INFO, tmpFrmSize);
        pBuff += sizeof(TLV_HEADER);
        memcpy(pBuff, &TlvVFrmEx, tmpFrmSize);
        pBuff += tmpFrmSize;
    }

    //Now To Write Data Frame
    int   nPckLen = nSize;
    char *pPckBuf = pPack;
    int   tmpSize = 0;
    while(nPckLen > 0 ) {
        tmpSize = (nPckLen%0xFFFF);

        OWSP_TlvHdr_Write(pBuff,
            pFrmI->iType ? TLV_T_VIDEO_IFRAME_DATA
                        : TLV_T_VIDEO_PFRAME_DATA
            , tmpSize);
        pBuff   += sizeof(TLV_HEADER);

        memcpy(pBuff, pPckBuf, tmpSize);
        pBuff   += tmpSize;
        pPckBuf += tmpSize;

        nPckLen -= tmpSize;
    }

    //Now To Count & Write PacketHdr
    PckHdr.packet_length = htonl((pBuff - Buff) - 4);
    PckHdr.packet_seq    = iPackNum;

    memcpy(Buff, &PckHdr, sizeof(PckHdr));

    return (pBuff-Buff);
}

int  OWSP_Mob_Info_ReqProc(OWSP_CONX *pConX, char Buff[], int Size)
{
    if(!Buff || Size != sizeof(TLV_V_PhoneInfoRequest)) { return 0; }
	OWSP_DBG("OWSP TLV_V_PhoneInfoRequest Run\n");

	TLV_V_PhoneInfoRequest MobInfo;

    memcpy(&MobInfo, Buff, sizeof(MobInfo));

    //FIXME: Need Check Mobile Information Here.

    char BuffRet[OWSP_RESPONSE_LEN];

    TLV_V_PhoneInfoResponse MobRet;
	MobRet.result = _RESPONSECODE_SUCC;
	if(OWSP_SendPacket(pConX, TLV_T_PHONE_INFO_ANSWER,
        BuffRet, (char *)&MobRet, sizeof(MobRet)) <= 0) {
        return -1;
    }

	return	!0;
}

int  OWSP_Dvs_Info_ReqProc(OWSP_CONX *pConX, char Buff[], int Size)
{
    if(!Buff || Size != sizeof(TLV_V_DVSInfoRequest)) { return 0; }
	OWSP_DBG("OWSP TLV_V_DVSInfoRequest Run\n");

    TLV_V_DVSInfoRequest DvsRet;

    char BuffRet[OWSP_RESPONSE_LEN];

    //DvsRet. //FIXME:
	//Dev_Interf->GetOwspDVSInfo(&tlv_dvsReq);
	if(OWSP_SendPacket(pConX, TLV_T_DVS_INFO_REQUEST,
        BuffRet, (char *)&DvsRet, sizeof(DvsRet)) <= 0) {
        return -1;
    }

	OWSP_DBG("owsp send dvs request packet successfule\n");

    return !0;
}

int  OWSP_Ver_Info_ReqProc(OWSP_CONX *pConX, char Buff[], int Size)
{
    if(!Buff || Size != sizeof(TLV_V_VersionInfoRequest)) { return 0; }
	OWSP_DBG("OWSP TLV_V_VersionInfoRequest Run\n");

	TLV_V_VersionInfoRequest VerReq;

    memcpy(&VerReq, Buff, sizeof(VerReq));

	OWSP_DBG("VerMajor %d, VerMinor %d\n", VerReq.versionMajor, VerReq.versionMinor);

    char BuffRet[OWSP_RESPONSE_LEN];

    TLV_V_VersionInfoResponse ReqRet;
#if 0
	if(0) { //Maybe Need More Check
        OWSP_DBG("StdVerMaj %d, StdVerMin %d",
            VERSION_MAJOR, VERSION_MINOR);

        ReqRet.result = _RESPONSECODE_PDA_VERSION_ERROR;

        OWSP_SendPacket(pConX, TLV_T_VERSION_INFO_ANSWER,
            BuffRet, (char *)&ReqRet, sizeof(ReqRet));

		return 0;
	}
#endif
	ReqRet.result = _RESPONSECODE_SUCC;

	if(OWSP_SendPacket(pConX, TLV_T_VERSION_INFO_ANSWER,
        BuffRet, (char *)&ReqRet, sizeof(ReqRet)) <= 0) {
        return -1;
    }

	return	!0;
}

//yqw+
//#include "user.h"

int  OWSP_Auth_Verify(char Usr[], char Pwd[])
{
	return 1;
	/*
    int nRet = -1;
    User * pUsr = PUserStruct(NULL, NULL);
    if(!pUsr) {
        return 0;
    }

    nRet = pUsr->Load(pUsr);
    if(0 != nRet) {
        PUserDestruct(pUsr, NULL);
        return 0;
    }

    nRet = pUsr->Verify(pUsr, Usr, Pwd);

    PUserDestruct(pUsr, NULL);

    return nRet;//*/
}

int  OWSP_Usr_Login_ReqProc(OWSP_CONX *pConX, char Buff[], int Size)
{
    if(!Buff || Size != sizeof(TLV_V_LoginRequest)) { return -1; }
	OWSP_DBG("OWSP TLV_V_LoginRequest Run\n");

    TLV_V_LoginRequest       LoginReq;
    TLV_V_LoginResponse      LoginRet;

    char BuffRet[OWSP_RESPONSE_LEN];

    memcpy(&LoginReq, Buff, sizeof(LoginReq));

	OWSP_DBG("User_Login_Request deviceid %u, channel %d, flag %d\n",
        (unsigned int)LoginReq.deviceId, LoginReq.channel, LoginReq.flag);
	OWSP_DBG("User_Login_Request username:%s, pwd:%s\n",
        LoginReq.userName, LoginReq.password);

    //Check UserName & Password
    if(!OWSP_Auth_Verify(LoginReq.userName, LoginReq.password)) {
        LoginRet.result = _RESPONSECODE_USER_PWD_ERROR;
        goto LOGIN_FAILED;
    }

    //Check Available Connection
   if(!(pConX->pNum) || *(pConX->pNum) >= MAX_CAM_CH) {
        LoginRet.result = _RESPONSECODE_MAX_USER_ERROR;
        goto LOGIN_FAILED;
    }//*/

	if(  LoginReq.channel >= MAX_CAM_CH ) { //FIME: Maybe Need More Check
        TLV_V_ChannelResponse    ChlRet;
        ChlRet.result = _RESPONSECODE_INVALID_CHANNLE;
        ChlRet.currentChannel = 99; //
//        ChlRet.currentChannel = 0;  //

        OWSP_SendPacket(pConX, TLV_T_LOGIN_ANSWER,
            BuffRet, (char *)&ChlRet, sizeof(ChlRet));
        return -1;
    }

    if(0) {
//        LoginRet.result = _RESPONSECODE_USER_PWD_ERROR;
//        LoginRet.result = _RESPONSECODE_MAX_USER_ERROR;
//        LoginRet.result = _RESPONSECODE_NOT_START_ENCODE;
//        LoginRet.result = _RESPONSECODE_TASK_DISPOSE_ERROR;
//        LoginRet.result = _RESPONSECODE_PROTOCOL_ERROR;
    }
    else {
        LoginRet.result = _RESPONSECODE_SUCC;
    }

#if 1
    TLV_V_VersionInfoRequest VerReq;
    VerReq.versionMajor = VERSION_MAJOR;
    VerReq.versionMinor = VERSION_MINOR;
    OWSP_SendPacket(pConX, TLV_T_VERSION_INFO_REQUEST,
        BuffRet, (char *)&VerReq, sizeof(VerReq));
#endif

#if 1  //Send DVR Detail Information To Mobile Client
    TLV_V_DVSInfoRequest DvsReq;
    memset(&DvsReq, 0, sizeof(DvsReq));

    strncpy(DvsReq.equipmentName,
        DEVICE_MODEL,
        sizeof(DvsReq.equipmentName)-1);

    DvsReq.channleNumber = MAX_CAM_CH;

    memcpy(DvsReq.companyIdentity,
        OWSP_DEFAULT_IDENTITY,
        sizeof(DvsReq.companyIdentity));

    OWSP_SendPacket(pConX, TLV_T_DVS_INFO_REQUEST,
        BuffRet, (char *)&DvsReq, sizeof(DvsReq));
#endif

	if(OWSP_SendPacket(pConX, TLV_T_LOGIN_ANSWER,
        BuffRet, (char *)&LoginRet, sizeof(LoginRet)) <= 0) {
        return -1;
    }

    pConX->Chnl = LoginReq.channel;
    //pConX->Stat = 1;

    return !0;

LOGIN_FAILED:
	if(OWSP_SendPacket(pConX, TLV_T_LOGIN_ANSWER,
        BuffRet, (char *)&LoginRet, sizeof(LoginRet)) <= 0) {
        return -1;
    }
    return -1;
}

int  OWSP_Chn_Switch_ReqProc(OWSP_CONX *pConX, char Buff[], int Size)
{
    if(!Buff || Size != sizeof(TLV_V_ChannelRequest)) { return 0; }
	OWSP_DBG("OWSP TLV_V_ChannelRequest Run\n");

    TLV_V_ChannelRequest   ChnReq;
    TLV_V_ChannelResponse  ChnRet;

    char BuffRet[OWSP_RESPONSE_LEN];

    memcpy(&ChnReq, Buff, sizeof(ChnReq));

    if(ChnReq.sourceChannel == ChnReq.destChannel) {
        return 0;
    }

    if(ChnReq.destChannel >= MAX_CAM_CH) {
        ChnRet.result = _RESPONSECODE_INVALID_CHANNLE;
        goto SWTCH_FAILED;
    }

//    ChnRet.result = _RESPONSECODE_INVALID_CHANNLE;
//    ChnRet.result = _RESPONSECODE_PROTOCOL_ERROR;
//    ChnRet.result = _RESPONSECODE_MAX_USER_ERROR;
    ChnRet.result = _RESPONSECODE_SUCC;
    ChnRet.currentChannel = ChnReq.destChannel;

	if(OWSP_SendPacket(pConX, TLV_T_CHANNLE_ANSWER,
        BuffRet, (char *)&ChnReq, sizeof(ChnReq)) <= 0) {
        return -1;
    }

    pConX->Chnl = ChnReq.destChannel;
    //pConX->Stat = 1;

    return !0;

SWTCH_FAILED:
    if(OWSP_SendPacket(pConX, TLV_T_CHANNLE_ANSWER,
        BuffRet, (char *)&ChnReq, sizeof(ChnReq)) <= 0) {
        return -1;
    }

    return -1;
}

typedef enum
{
	PTZ_CMD_NULL = -1,
	PTZ_CMD_UP,
	PTZ_CMD_DOWN,
	PTZ_CMD_LEFT,
	PTZ_CMD_RIGHT,
	PTZ_CMD_LEFT_UP,
	PTZ_CMD_RIGHT_UP,
	PTZ_CMD_LEFT_DOWN,
	PTZ_CMD_RIGHT_DOWN,
	PTZ_CMD_AUTOPAN,
	PTZ_CMD_IRIS_OPEN,
	PTZ_CMD_IRIS_CLOSE,
	PTZ_CMD_ZOOM_IN,
	PTZ_CMD_ZOOM_OUT,
	PTZ_CMD_FOCUS_FAR,
	PTZ_CMD_FOCUS_NEAR,
	PTZ_CMD_STOP,
	PTZ_CMD_WIPPER_ON,
	PTZ_CMD_WIPPER_OFF,
	PTZ_CMD_LIGHT_ON,
	PTZ_CMD_LIGHT_OFF,
	PTZ_CMD_POWER_ON,
	PTZ_CMD_POWER_OFF,
	PTZ_CMD_GOTO_PRESET,
	PTZ_CMD_SET_PRESET,
	PTZ_CMD_CLEAR_PRESET,
	PTZ_CMD_TOUR,
	PTZ_COMMAND_CNT,
}PTZ_COMMAND;


//yqw+:get trouble with ReqPtzCtl()
/*void ReqPtzCtl(int nChn, int nCmd, int nArg)
{
	//if (bUdpInited)
	{
		printf("Send Ptz Control Cmd\n");
		int nError = 0;
        char SendPack[sizeof(A2NCmd)+sizeof(Ptz_Ctl_t)];
        A2NCmd * tmpCmd        = (A2NCmd*)SendPack;
        Ptz_Ctl_t * tmpData = (Ptz_Ctl_t *)(SendPack+sizeof(*tmpCmd));

		tmpCmd->nHead = -1;
		tmpCmd->nCmd = CMD_PTZ_CTL;
        tmpCmd->nLen = sizeof(*tmpData);

        tmpData->nChn = nChn;
        tmpData->nCmd = nCmd;
        tmpData->nArg = nArg;

#ifdef _USE_UNIX_DOMAIN_SOCKET_
		struct sockaddr_un	ClientAddr;
		memset(&ClientAddr,0,sizeof(struct sockaddr_un));
		ClientAddr.sun_family = AF_UNIX;
		sprintf(ClientAddr.sun_path,"/tmp/streamio.socket");

		int nClientAddrSize;
		nClientAddrSize = offsetof(struct sockaddr_un,sun_path) + strlen(ClientAddr.sun_path);

#else
		struct sockaddr_in	ClientAddr;
		bzero(&ClientAddr,sizeof(struct sockaddr_in));
		ClientAddr.sin_family = AF_INET;
		ClientAddr.sin_port = htons(2012);
		ClientAddr.sin_addr.s_addr = inet_addr(LOCAL_ADDR);
#endif

		do
		{
#ifdef _USE_UNIX_DOMAIN_SOCKET_
			int nRet = sendto(sUdpData,&SendPack,sizeof(SendPack),0,(struct sockaddr *)&ClientAddr,nClientAddrSize);
#else
			int nRet = sendto(sUdpData,&SendPack,sizeof(SendPack),0,(struct sockaddr *)&ClientAddr,sizeof(struct sockaddr_in));
#endif
			if (-1 == nRet)
			{
				nError = errno;
			}
		} while (EAGAIN == nError || EWOULDBLOCK == nError);
	}
}//*/

int  OWSP_Ptz_Ctrl_ReqProc(OWSP_CONX *pConX, char Buff[], int Size)
{
    if(!Buff
    || Size != sizeof(TLV_V_ControlRequest)
//    || Size != (sizeof(TLV_V_ControlRequest) + sizeof(ControlArgData))
    ) {
        return 0;
    }
	OWSP_DBG("OWSP TLV_V_ControlRequest Run\n");

    TLV_V_ControlRequest   PtzReq;
    TLV_V_ControlResponse  PtzRet;

    char BuffRet[OWSP_RESPONSE_LEN];

    memcpy(&PtzReq, Buff, sizeof(PtzReq));

	printf("ptzCMD:%d\n", PtzReq.cmdCode);
    printf("ptzSIZ:%d\n", PtzReq.size);
    PtzRet.result = _RESPONSECODE_INVALID_CHANNLE;

#if 1
    unsigned int PtzChn = 0;
    unsigned int PtzCmd = 0;
    int ii;

    const unsigned int PTZ_CONV_TBL[][2] = {
        {OWSP_ACTION_MD_UP,       PTZ_CMD_UP},
        {OWSP_ACTION_MD_DOWN,     PTZ_CMD_DOWN},
        {OWSP_ACTION_MD_LEFT,     PTZ_CMD_LEFT},
        {OWSP_ACTION_MD_RIGHT,    PTZ_CMD_RIGHT},
        {OWSP_ACTION_Circle_Add,  PTZ_CMD_IRIS_OPEN},
        {OWSP_ACTION_Circle_Reduce, PTZ_CMD_IRIS_CLOSE},
        {OWSP_ACTION_ZOOMADD,     PTZ_CMD_ZOOM_IN},
        {OWSP_ACTION_ZOOMReduce,  PTZ_CMD_ZOOM_OUT},
        {OWSP_ACTION_FOCUSReduce, PTZ_CMD_FOCUS_NEAR},
        {OWSP_ACTION_FOCUSADD,    PTZ_CMD_FOCUS_FAR},
        {OWSP_ACTION_MD_STOP,     PTZ_CMD_STOP},
        {OWSP_ACTION_AUTO_CRUISE, PTZ_CMD_AUTOPAN},
    };
    for(ii = 0; (PtzReq.channel < MAX_CAM_CH) && (ii < sizeof(PTZ_CONV_TBL)/sizeof(PTZ_CONV_TBL[0])) ; ii ++) {
        if(PtzReq.cmdCode == PTZ_CONV_TBL[ii][0]) {
            //extern void ReqPtzCtl(int nChn, int nCmd, int nArg);
            //ReqPtzCtl(PtzReq.channel, PTZ_CONV_TBL[ii][1], 0);
            PtzRet.result = _RESPONSECODE_SUCC;
            break;
        }
    }
#endif

    if(OWSP_SendPacket(pConX, TLV_T_CONTROL_ANSWER,
        BuffRet, (char *)&PtzRet, sizeof(PtzRet)) <= 0) {
        return -1;
    }

    return !0;
}

int  OWSP_Chn_Suspend_ReqProc(OWSP_CONX *pConX, char Buff[], int Size)
{
    if(!Buff || Size != sizeof(TLV_V_SuspendSendDataRequest)) { return 0; }
	OWSP_DBG("OWSP TLV_V_SuspendSendDataRequest Run\n");

    return !0;
}
