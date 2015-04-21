

bool LoadDevCfg(TDevCfg* DevCfg) 
{
int i,m,n; 

{
CreateNewFile:

/***********************************************/
memset(DevCfg, 0 , sizeof(TDevCfg)); 

DevCfg->DevInfoPkt.DevType = __DEVICETYPE;
sprintf(DevCfg->DevInfoPkt.SoftVersion, "");
sprintf(DevCfg->DevInfoPkt.FileVersion, "");
sprintf(DevCfg->DevInfoPkt.DevName, "Device Name");
sprintf(DevCfg->DevInfoPkt.DevDesc, "Device Description");
DevCfg->DevInfoPkt.VideoChlCount = VIDEOCHLCOUNT;
DevCfg->DevInfoPkt.AudioChlCount = AUDIOCHLCOUNT;
DevCfg->DevInfoPkt.DiChlCount = DICHLCOUNT;
DevCfg->DevInfoPkt.DoChlCount = DOCHLCOUNT;
DevCfg->DevInfoPkt.RS485DevCount =RS485DEVCOUNT;
DevCfg->DevInfoPkt.Language = cn;
DevCfg->DevInfoPkt.OEMType = 1;//tony 20120505 modified //0
DevCfg->DevInfoPkt.RebootHM.w = w_close;
DevCfg->DevInfoPkt.RebootHM.start_h = 0;
DevCfg->DevInfoPkt.RebootHM.start_m = 0;
DevCfg->DevInfoPkt.Info.NotExistAudio = false;
DevCfg->DevInfoPkt.Info.NotExistRS485 = false;
DevCfg->DevInfoPkt.Info.NotExistIO = false;
//NetCfg
DevCfg->NetCfgPkt.CmdPort = Port_Ax_CmdData;
DevCfg->NetCfgPkt.rtspPort = Port_Ax_RTSP;
DevCfg->NetCfgPkt.HttpPort = Port_Ax_http;
WriteHttpPort(DevCfg->NetCfgPkt.HttpPort);
DevCfg->NetCfgPkt.Lan.IPType = 0;//静态IP
sprintf(DevCfg->NetCfgPkt.Lan.DevIP, "192.168.100.168");
sprintf(DevCfg->NetCfgPkt.Lan.DevMAC, "00:00:00:00:00:00");
sprintf(DevCfg->NetCfgPkt.Lan.SubMask, "255.255.0.0");
sprintf(DevCfg->NetCfgPkt.Lan.Gateway, "192.168.1.1");
sprintf(DevCfg->NetCfgPkt.Lan.DNS1, "192.168.1.1");
//sprintf(DevCfg->NetCfgPkt.Lan.DNS2, "192.168.1.1");
DevCfg->NetCfgPkt.DDNS.Active =false;
DevCfg->NetCfgPkt.DDNS.DDNSType = 0;
sprintf(DevCfg->NetCfgPkt.DDNS.DDNSDomain, "Domain");
sprintf(DevCfg->NetCfgPkt.DDNS.HostAccount, "Account");
sprintf(DevCfg->NetCfgPkt.DDNS.HostPassword, "Password");
DevCfg->NetCfgPkt.PPPOE.AutoStart =  false;
sprintf(DevCfg->NetCfgPkt.PPPOE.Account, "Account");
sprintf(DevCfg->NetCfgPkt.PPPOE.Password, "Password");
DevCfg->NetCfgPkt.uPnP.Active = false;
//UserCfg
memset(&DevCfg->UserCfgPkt, 0, sizeof(DevCfg->UserCfgPkt));
DevCfg->UserCfgPkt.Count = 1;
DevCfg->UserCfgPkt.Lst[0].UserGroup = GROUP_ADMIN;
DevCfg->UserCfgPkt.Lst[0].Authority = USER_ADMIN;
strcpy(DevCfg->UserCfgPkt.Lst[0].UserName, "admin");
strcpy(DevCfg->UserCfgPkt.Lst[0].Password, "admin");
#ifdef A4
DevCfg->WiFiCfgPkt.Active = false;
sprintf(DevCfg->WiFiCfgPkt.DevIP, "192.168.2.168");
sprintf(DevCfg->WiFiCfgPkt.SubMask, "255.255.0.0");
sprintf(DevCfg->WiFiCfgPkt.Gateway, "192.168.2.1");
//PPkt->SSID;
DevCfg->WiFiCfgPkt.Channel = 1;//频道1..14 default 1=Auto
DevCfg->WiFiCfgPkt.EncryptType = 0;//(Encrypt_None,Encrypt_WEP,Encrypt_WPA);
DevCfg->WiFiCfgPkt.WEPKeyBit = 0;//(kBit64,kBit128);
DevCfg->WiFiCfgPkt.WEPIndex = 0;//0..3;//=0
#endif
CreateNewFile2:
//VideoCfg
for (i=0;i<VIDEOCHLCOUNT;i++)
{
	DevCfg->VideoCfgPkt[i].Chl = i;
	DevCfg->VideoCfgPkt[i].Active =  true;
	TVideoFormat* fmt = &DevCfg->VideoCfgPkt[i].VideoFormat;
	sprintf(fmt->Title, "Channel %d", i+1);//从 1 开始 1 2 3 4
	      
	fmt->IsShowTime = true;
	fmt->TimeFontSize = 20;
	fmt->TimeColor = cl_White;
	fmt->TimeX = 16;//(i % 2)*fmt->Width  + 16;//40
	fmt->TimeY = 16;//(i / 2)*fmt->Height + 16;//40
	      
	fmt->IsShowTitle = true;
	fmt->TitleFontSize = 20;
	fmt->TitleColor = cl_White;
	fmt->TitleX = 16;//(i % 2)*fmt->Width  + 16;//40
	fmt->TitleY = 32;//(i / 2)*fmt->Height + 48;//64
	      
	fmt->IsShowFrameRate = false;
	fmt->FrameRateFontSize = 20;
	fmt->FrameRateColor = cl_White;
	fmt->FrameRateX = 16;//(i % 2)*fmt->Width  + 16;//40
	fmt->FrameRateY = 48;//(i / 2)*fmt->Height + 80;//88
	/*
	      fmt->IsShowWaterMark = false;
	      fmt->WaterMarkFontSize = 20;
	      fmt->WaterMarkColor = cl_Black;
	      fmt->WaterMarkX = 16;//(i % 2)*fmt->Width  + 16;
	      fmt->WaterMarkY = 144;//(i / 2)*fmt->Height + fmt->Height - 16;
	*/
	fmt->DeInterlaceType = 1;
	fmt->IsDeInterlace = false;
	fmt->FlipHorizontal = true;//false
	fmt->FlipVertical = true;//false
}

//AudioCfg  缺省值要修改
for (i=0;i<AUDIOCHLCOUNT;i++)
{
#if defined(A1) || defined(A4)
	DevCfg->AudioCfgPkt[i].Chl = i;
	DevCfg->AudioCfgPkt[i].Active = false;
	DevCfg->AudioCfgPkt[i].InputType = LINE_IN;//
	TAudioFormat* fmt = &DevCfg->AudioCfgPkt[i].AudioFormat;
	fmt->wFormatTag = PCM;
	fmt->nChannels = 1;
	fmt->nSamplesPerSec = 8000;//tony 20120614 audio_test modified pre is 16000
	fmt->wBitsPerSample = 16;//16
	fmt->cbSize = sizeof(TAudioFormat);
	fmt->nBlockAlign = (fmt->wBitsPerSample / 8)*fmt->nChannels;
	fmt->nAvgbytesPerSec = fmt->wBitsPerSample*fmt->nChannels*fmt->nBlockAlign;
	fmt->cbSize = sizeof(TAudioFormat);
#endif
}
//AlmCfg
DevCfg->AlmCfgPkt.AlmOutTimeLen = 10; //报警输出时长(秒)
DevCfg->AlmCfgPkt.AutoClearAlarm = true;
for (i=0; i<ALMCFGLST; i++)
{
DevCfg->AlmCfgPkt.Lst[i].AlmType = Alm_None;
DevCfg->AlmCfgPkt.Lst[i].Channel = 0;
DevCfg->AlmCfgPkt.Lst[i].NetSend = true;
DevCfg->AlmCfgPkt.Lst[i].ActiveDO = true;//DI关联DO通道 0 close
DevCfg->AlmCfgPkt.Lst[i].DOChannel = 0;// 0-255 do channel
DevCfg->AlmCfgPkt.Lst[i].GotoPTZPoint = 0;
DevCfg->AlmCfgPkt.Lst[i].IsAlmRec = true;
DevCfg->AlmCfgPkt.hm[i].w = w_1_7;
DevCfg->AlmCfgPkt.hm[i].start_h = 0;
DevCfg->AlmCfgPkt.hm[i].start_m = 0;
DevCfg->AlmCfgPkt.hm[i].stop_h = 23;
DevCfg->AlmCfgPkt.hm[i].stop_m = 59;
}
#if defined(A4)
DevCfg->AlmCfgPkt.Lst[0].AlmType = Alm_MotionDetection;
DevCfg->AlmCfgPkt.Lst[0].Channel   = 0;
DevCfg->AlmCfgPkt.Lst[0].DOChannel = 0;
DevCfg->AlmCfgPkt.Lst[0].ActiveDO = true;
DevCfg->AlmCfgPkt.Lst[0].IsAlmRec = false;
DevCfg->AlmCfgPkt.Lst[1].AlmType = Alm_DigitalInput;
DevCfg->AlmCfgPkt.Lst[1].Channel   = 0;
DevCfg->AlmCfgPkt.Lst[1].DOChannel = 0;
DevCfg->AlmCfgPkt.Lst[1].ActiveDO = true;
DevCfg->AlmCfgPkt.Lst[1].IsAlmRec = false;
    /*DevCfg->AlmCfgPkt.Lst[2].AlmType = Alm_VideoLost;
    DevCfg->AlmCfgPkt.Lst[2].Channel   = 0;
    DevCfg->AlmCfgPkt.Lst[2].DOChannel = 0;
    DevCfg->AlmCfgPkt.Lst[2].ActiveDO = true;
    DevCfg->AlmCfgPkt.Lst[2].IsAlmRec = false;*/
#endif
//DiDoCfg
DevCfg->DiDoCfgPkt.DiCfgPkt.UsedCount = DICHLCOUNT;
for (i=0; i<DICHLCOUNT; i++)
{      
DevCfg->DiDoCfgPkt.DiCfgPkt.Lst[i].Active = true; 
DevCfg->DiDoCfgPkt.DiCfgPkt.Lst[i].InputType = 0;// 0 低电平　1 高电平
}
DevCfg->DiDoCfgPkt.DoCfgPkt.UsedCount = DOCHLCOUNT;
for (i=0; i<DOCHLCOUNT; i++)
{
DevCfg->DiDoCfgPkt.DoCfgPkt.Lst[i].Active = true;
DevCfg->DiDoCfgPkt.DoCfgPkt.Lst[i].OutType = 0;// 0 低电平　1 高电平
}
//MDCfg
for (i=0;i<VIDEOCHLCOUNT;i++)
{
TMDCfgPkt* PPkt = &DevCfg->MDCfgPkt[i];
PPkt->Chl = i;
for (m=0; m<MDLSTCOUNT; m++)
{        
PPkt->Lst[m].Active = false;
PPkt->Lst[m].Reserved = 0;
PPkt->Lst[m].Sensitive = 30;//155
PPkt->Lst[i].Tag = 0;
PPkt->Lst[m].Rect1.left = 15+ m*120;
PPkt->Lst[m].Rect1.top =  15;
PPkt->Lst[m].Rect1.right = 260 + m*120;
PPkt->Lst[m].Rect1.bottom = 260;
PPkt->hm[m].w = w_1_7;
PPkt->hm[m].start_h = 0;
PPkt->hm[m].start_m = 0;
PPkt->hm[m].stop_h = 23;
PPkt->hm[m].stop_m = 59;
}
}
//HideAreaCfg
for (i=0;i<VIDEOCHLCOUNT;i++)
{
THideAreaCfgPkt* PPkt = &DevCfg->HideAreaCfgPkt[i];
PPkt->Chl = i;
for (m=0; m<HIDEAREALSTCOUNT; m++)
{
PPkt->Active = false;
PPkt->Lst[m].Active = false;        
PPkt->Lst[m].Rect1.left= 0;
PPkt->Lst[m].Rect1.top= 0;
PPkt->Lst[m].Rect1.right= 0;
PPkt->Lst[m].Rect1.bottom= 0;
}
}
//RS485CfgPkt
DevCfg->RS485CfgPkt.BPS = BaudRate_2400;
DevCfg->RS485CfgPkt.DataBit = DataBit_8;
DevCfg->RS485CfgPkt.ParityCheck = ParityCheck_None;
DevCfg->RS485CfgPkt.StopBit = StopBit_1;
for (i=0; i<VIDEOCHLCOUNT; i++)
{ 
DevCfg->RS485CfgPkt.Lst[i].PTZProtocol = Pelco_D;//如果是云台
DevCfg->RS485CfgPkt.Lst[i].PTZSpeed = 0x32;
DevCfg->RS485CfgPkt.Lst[i].Address = i + 1;
DevCfg->RS485CfgPkt.Lst[i].Reserved = 0;
}
#ifdef A4
//RecCfg
for (i=0; i<VIDEOCHLCOUNT; i++)
{
DevCfg->RecCfgPkt[i].RecStreamType = 1;
DevCfg->RecCfgPkt[i].Rec_AlmPrevTimeLen = 0;
DevCfg->RecCfgPkt[i].Rec_AlmTimeLen = 10;
DevCfg->RecCfgPkt[i].Rec_NmlTimeLen = 600;
/*********tony 20120607 sd begin*********/ 
for(m = 0;m < 7 ; m++)
{
for(n = 0;n < RECPLANLST;n++)
{
DevCfg->RecCfgPkt[i].Plan.Week[m][n].stop_h = 23;
DevCfg->RecCfgPkt[i].Plan.Week[m][n].stop_m = 59;
}
}
/*********tony 20120607 sd end*********/ 
}
//DiskCfg
DevCfg->DiskCfgPkt.IsFillOverlay = true;
#endif

}


if (DevCfg->DevInfoPkt.MaxUserConn == 0)
{
DevCfg->DevInfoPkt.MaxUserConn = 10;//最大用户连接数 default 10
}
if (DevCfg->NetCfgPkt.rtspPort == DevCfg->NetCfgPkt.CmdPort)
{
DevCfg->NetCfgPkt.rtspPort = Port_Ax_RTSP;
}
UpdatePTZNameLst();
return true;
}


