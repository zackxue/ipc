// JaAssistDlg.cpp : implementation file
//

#include "stdafx.h"
#include "JaAssist.h"
#include "JaAssistDlg.h"
#include <time.h>
#include <sys/stat.h>

#include "../jastdef.h"
#include "../jastlib.h"
#include "../vlog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct _thread_args
{
	void *data;
	void *LParam;
	int RParam;
}ThreadArgs_t;


#define JAST_FILE_FORMAT	"log\\%d_%d_%d_%d__%04d%02d%02d_%02d%02d%02d.log"//ip-YYYYMMDD-hhmmss
#define JAST_FILE_SIZE		(1024*1024*10)

extern "C" int SOCK_udp_init(int bind_port,int rwtimeout);
extern "C" int SOCK_set_broadcast(int sock);
extern "C" int HTTP_get_content(char *src,int in_size,char **out,int *out_size);
extern "C" JastSession_t *g_SessionTable;
static unsigned short g_JastClientPort = JAST_CPORT_START;

//threads proc
UINT JastDiscoveryProc(LPVOID pParam);
void* JASTC_proc(void *param);

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJaAssistDlg dialog

CJaAssistDlg::CJaAssistDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJaAssistDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJaAssistDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJaAssistDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJaAssistDlg)
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_editoutput);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_devicesList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CJaAssistDlg, CDialog)
	//{{AFX_MSG_MAP(CJaAssistDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, OnButtonDisconnect)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DEVICE, OnDblclkListDevice)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJaAssistDlg message handlers

BOOL CJaAssistDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_devicesList.InsertColumn(0, "ip", LVCFMT_LEFT, 110, 0);
    m_devicesList.InsertColumn(1, "port", LVCFMT_LEFT, 50, 1);
	
	// init session
	JAST_session_init();
	// create a directory to strore log files
	DWORD dwAttr=GetFileAttributes("log");
	if(dwAttr==0xFFFFFFFF){// not such directory
		CreateDirectory("log",NULL);
	}else if(dwAttr & FILE_ATTRIBUTE_DIRECTORY){
		printf("<log> directory is exist now!\n");
	}
//
	m_seldev = 0;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CJaAssistDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CJaAssistDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CJaAssistDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CJaAssistDlg::OnButtonConnect() 
{
	// TODO: Add your control notification handler code here
	
}

void CJaAssistDlg::OnButtonDisconnect() 
{
	// TODO: Add your control notification handler code here

	int i;
	JastSession_t *s = NULL;
	Jast_t *j=NULL;
	
	m_devicesList.DeleteAllItems();
	for(i=1;i<=JAST_session_entries();i++){
		if((s=JAST_session_get(i)) == NULL) return;
		j = (Jast_t *)s->data.context;
		JAST_request_bye(j);
		//JAST_session_del(j->ip_dst);
		//JAST_destroy(j);
		j->trigger = false;
	}	
	_CrtDumpMemoryLeaks();
	
}


FILE* CJaAssistDlg::file_new(const char *name)
{
	FILE *f=fopen(name,"a+");
	if(f==NULL){
		printf("open file failed\n");
		return NULL;
	}
	VLOG(VLOG_CRIT,"create file:%s success",name);
	return f;
}

int CJaAssistDlg::file_write(FILE *f,char *buf,int size)
{
	if(fwrite(buf,size,1,f)!=1){
		printf("write file failed\n");
		return -1;
	}
	return 0;
}

unsigned long CJaAssistDlg::file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}

int CJaAssistDlg::JASTC_start(char *ip,int port)
{
	Jast_t *jast=NULL;
	JastSession_t *s=NULL;
	ThreadId_t tid;
	ThreadArgs_t *args = (ThreadArgs_t *)malloc(sizeof(ThreadArgs_t));
	if(args == NULL){
		printf("ERR: malloc for args failed!\n");
		return JAST_RET_FAIL;
	}
	// init jast session if necessary
	//if(g_SessionTable == NULL){
	//	JAST_session_init();
	//}
	// init jast
	jast = JAST_client_init(ip,port,g_JastClientPort++);
	if(jast == NULL){
		return JAST_RET_FAIL;
	}
	//add a session
	s=JAST_session_add(ip,jast->sock,(void *)jast);
	if(s == NULL)
		return JAST_RET_FAIL;
	// create thread
	args->data = s;
	args->LParam = this;
	args->RParam = 0;	

	THREAD_create(tid,(void (*)(void *))JASTC_proc,(void *)args);

	return JAST_RET_OK;
}

int CJaAssistDlg::JASTC_stop(JastSession_t *s)
{
	Jast_t *jast=NULL;
	if(s != NULL){
		jast = (Jast_t *)s->data.context;
		jast->trigger = false;
	}
	return JAST_RET_OK;
}

int CJaAssistDlg::JASTC_init()
{
	int sock;
	sock=SOCK_udp_init(JAST_DISCOVERY_CPORT,JAST_SOCK_TIMEOUT);
	if(sock == JAST_RET_FAIL)
		return JAST_RET_FAIL;
#ifdef JAST_USE_BROADCAST
	SOCK_set_broadcast(sock);
#endif

	return sock;
}

int CJaAssistDlg::DeviceList_update()
{
	int i;
	JastSession_t *s = NULL;
	Jast_t *j=NULL;
	char tmp[512];
	
	m_devicesList.DeleteAllItems();
	for(i=1;i<=JAST_session_entries();i++){
		if((s=JAST_session_get(i)) == NULL) return -1;
		j = (Jast_t *)s->data.context;
		sprintf(tmp,"item%d",i);
		m_devicesList.InsertItem(i-1,tmp);
		sprintf(tmp,"%d",j->port_dst);
		m_devicesList.SetItemText(i-1,0,j->ip_dst);
		m_devicesList.SetItemText(i-1,1,tmp);
	}
	return 0;
}


void CJaAssistDlg::OnDblclkListDevice(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DWORD dwPos = GetMessagePos();
	CPoint point( LOWORD(dwPos), HIWORD(dwPos) );
	char devip[20];
	char tmp[128];

	m_devicesList.ScreenToClient(&point); 

	LVHITTESTINFO lvinfo;
	lvinfo.pt = point;
	lvinfo.flags = LVHT_ABOVE;
    
	int nItem = m_devicesList.SubItemHitTest(&lvinfo);
	
	printf("select change: %d -> %d !!!!!!!!!!!\n",m_seldev,nItem);

	if((nItem != -1) && (nItem < JAST_session_entries())){
		if(m_seldev != nItem){
			//
			m_devicesList.GetItemText(nItem,0,devip,sizeof(devip));
			sprintf(tmp,"STDOUT FROM DEVICE: %s",devip);
			GetDlgItem(IDC_STATIC_OUTPUT)->SetWindowText(tmp);
			m_editoutput.SetWindowText("");
			//
			m_seldev = nItem;
		}
	}

	*pResult = 0;
}

void CJaAssistDlg::OnButtonRefresh() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(JastDiscoveryProc,this);
	m_editoutput.SetWindowText("");
	GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(FALSE);
}

BOOL CJaAssistDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	OnButtonDisconnect();
	JAST_session_destroy();
	_CrtDumpMemoryLeaks();
	return CDialog::DestroyWindow();
}


// thread processs
void* JASTC_proc(void *param)
{
	int ret;
	int i=0;
	char line[1024];
	char *pos=NULL;
	char logfile[128];
	ThreadArgs_t *args=(ThreadArgs_t *)param;
	JastSession_t *s=NULL;
	Jast_t *jast=NULL;
	fd_set read_set;
	char ip[20],devip[20];
	int port;
	struct timeval timeout;
	int code;
	time_t t;
	struct tm *ptm;	
	FILE *file;
	char *content;
	int content_size;
	unsigned char arr_ip[4];
	CJaAssistDlg *dlg=NULL;
	MillisecondTimer_t t_tmp;
	MilliSecond_t t_lasttime;

	s = (JastSession_t *)args->data;
	jast = (Jast_t *)s->data.context;
	dlg = (CJaAssistDlg *)args->LParam;

	time(&t);

	//MSLEEP(1000);
	if(JAST_request_handshake(jast)==JAST_RET_OK){
		//
		if(sscanf(jast->ip_dst,"%d.%d.%d.%d",&arr_ip[0],&arr_ip[1],&arr_ip[2],&arr_ip[3])!=4)
			goto PROC_EXIT;
		
		time(&t);
		ptm = localtime(&t);
		sprintf(logfile,JAST_FILE_FORMAT,arr_ip[0],arr_ip[1],arr_ip[2],arr_ip[3],
			ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
		if((file=dlg->file_new(logfile)) == NULL)
			goto PROC_EXIT;
		while(jast->trigger){
			timeout.tv_sec = 0;
			timeout.tv_usec = 5*1000;
			FD_ZERO(&read_set);
			FD_SET(jast->sock,&read_set);
			ret = select(jast->sock+1,&read_set,NULL,NULL,&timeout);
			if(ret < 0){
				printf("ERR: JASTC_proc select failed!\n");
			}else if(ret == 0){
				//timeout
			}else{
				if(FD_ISSET(jast->sock,&read_set)){
					if(JAST_read_message(jast,ip,&port) == JAST_RET_FAIL)
						break;
					if(strstr(jast->payload,"HEARTBREAK") != NULL){
						printf("getheartbreak @ %u\n%s\n",time(NULL),jast->payload);
						MilliTimerStart(jast->hb_timer);
					}
					else if(strncmp(jast->payload,"STDOUT",strlen("STDOUT")) == 0){
						if(HTTP_get_content(jast->payload,jast->payload_size,&content,&content_size)==JAST_RET_FAIL)
							break;
						if(dlg->file_write(file,content,content_size)==JAST_RET_FAIL)
							break;
						// write stdout to listbox
						dlg->m_devicesList.GetItemText(dlg->m_seldev,0,devip,sizeof(devip));
						if(strcmp(devip,ip)==0){
							//
							int nLength=dlg->m_editoutput.SendMessage(WM_GETTEXTLENGTH);  
							dlg->m_editoutput.SetSel(nLength,  nLength);  
							dlg->m_editoutput.ReplaceSel(content);
						}
						// check if file size is too big to use new file or not
						if(dlg->file_size(logfile) >= JAST_FILE_SIZE){
							time(&t);
							ptm = localtime(&t);
							sprintf(logfile,JAST_FILE_FORMAT,arr_ip[0],arr_ip[1],arr_ip[2],arr_ip[3],
								ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
							if((file=dlg->file_new(logfile)) == NULL)
								break;
						}
					}
					else{
						if(JAST_parse_response(jast,&code)==JAST_RET_FAIL)
							break;
					}
					
				}
			}
			if(jast->bLogin == true){
				MilliTimerStop(jast->hb_timer,t_tmp,t_lasttime);
				if(t_lasttime >= (JAST_HEARTBREAK_TIME+5000)){
					printf("ERR: it's long time no heartbreak!\n");
					break;
				}
			}
		}
	}

PROC_EXIT:
	printf("INFO: JASTC_proc exit!\n");
	jast->trigger = false;
	JAST_session_del(jast->ip_dst);
	JAST_request_bye(jast);
	JAST_destroy(jast);
	dlg->DeviceList_update();
	free(param);

	return NULL;
}

UINT JastDiscoveryProc(LPVOID pParam)
{
	JastDevice_t devices[256];
	int devnum;
	int i;
	char tmp[128];
	CJaAssistDlg *dlg=(CJaAssistDlg *)pParam;	
	int sock=dlg->JASTC_init();

	if(sock != JAST_RET_FAIL){
		if((devnum=JAST_request_discovery(sock,devices))>0){
			for(i=0;i<devnum;i++){
				sprintf(tmp,"item%d",i+1);
				dlg->m_devicesList.InsertItem(i,tmp);
				sprintf(tmp,"%d",devices[i].port);
				dlg->m_devicesList.SetItemText(i,0,devices[i].ip);
				dlg->m_devicesList.SetItemText(i,1,tmp);
				//
				if(dlg->JASTC_start(devices[i].ip,devices[i].port)==JAST_RET_FAIL)
					break;
			}
		}
	}
	//
	dlg->GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(TRUE);
	dlg->GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(TRUE);
	sprintf(tmp,"STDOUT FROM DEVICE: %s",devices[0].ip);
	dlg->GetDlgItem(IDC_STATIC_OUTPUT)->SetWindowText(tmp);
	//
	closesocket(sock);
	WSACleanup();
	
	return 0;
}

