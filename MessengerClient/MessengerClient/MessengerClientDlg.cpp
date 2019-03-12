

#include "stdafx.h"
#include "MessengerClient.h"
#include "MessengerClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


CMessengerClientDlg::CMessengerClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MESSENGERCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMessengerClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAT_LIST, m_chat_list);
}

BEGIN_MESSAGE_MAP(CMessengerClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SEND_BTN, &CMessengerClientDlg::OnBnClickedSendBtn)
	ON_MESSAGE(25001, &CMessengerClientDlg::On25001)
	ON_MESSAGE(25002, &CMessengerClientDlg::On25002)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CMessengerClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);	
	SetIcon(m_hIcon, FALSE);

	mh_socket = socket(AF_INET, SOCK_STREAM, 0); // create socket 
	struct sockaddr_in srv_addr;
	memset(&srv_addr, 0, sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	srv_addr.sin_port = htons(18000);
	WSAAsyncSelect(mh_socket, m_hWnd, 25001, FD_CONNECT);
	m_connect_flag = SOCKET_CONNECTING;

	AddEventString("서버에 접속을 시도합니다");
	connect(mh_socket, (LPSOCKADDR)&srv_addr, sizeof(srv_addr) );

	return TRUE; 
}

void CMessengerClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CMessengerClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CMessengerClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMessengerClientDlg::OnBnClickedSendBtn()
{
	CString str;
	GetDlgItemText(IDC_CHAT_EDIT, str);
	if (m_connect_flag == SOCKET_CONNECTED) {
		SendFrameData(mh_socket, 1, str.GetLength() + 1, (char*)(const char*)str);
		SetDlgItemText(IDC_CHAT_LIST, str);
		GotoDlgCtrl(GetDlgItem(IDC_CHAT_EDIT));

	}
}
void CMessengerClientDlg::SendFrameData(SOCKET ah_socket, char a_message_id,
	unsigned short int a_body_size, char* ap_send_data)
{
	char *p_send_data = new char[4 + a_body_size];
	*p_send_data = MSG_KEY;
	*(p_send_data + 1) = a_message_id;
	*(unsigned short *)(p_send_data + 2) = a_body_size;

	memcpy(p_send_data + 4, ap_send_data, a_body_size);
	send(ah_socket, p_send_data, a_body_size + 4, 0);
	delete[] p_send_data;
}


afx_msg LRESULT CMessengerClientDlg::On25001(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam)) {
		m_connect_flag = SOCKET_DISCONNECT;
		closesocket(mh_socket);
		mh_socket = INVALID_SOCKET;
		AddEventString("서버에 접속을 실패했습니다");
	}
	else {
		m_connect_flag = SOCKET_CONNECTED;
		WSAAsyncSelect(mh_socket, m_hWnd, 25002, FD_READ | FD_CLOSE);
		AddEventString("서버에 접속했습니다");

	}
	return 0;
}


afx_msg LRESULT CMessengerClientDlg::On25002(WPARAM wParam, LPARAM lParam)
{
	CString str;
	if (WSAGETSELECTEVENT(lParam) == FD_READ) {
		WSAAsyncSelect(wParam, m_hWnd, 2002, FD_CLOSE);
		char key;
		recv(wParam, &key, 1, 0);
		if (key == MSG_KEY) {
			char message_id;
			recv(wParam, &message_id, 1, 0);
			unsigned short body_size;
			recv(wParam, (char*)&body_size, 2, 0);
			char* p_body_data = NULL;

			if (body_size > 0) {
				p_body_data = new char[body_size];
				int total = 0, x, retry = 0;
				while (total < body_size ){
					x = recv(wParam, p_body_data + total, body_size - total, 0);
					if (x == SOCKET_ERROR) break;
					total = total + x;
					if (total < body_size) {
						Sleep(50);
						retry++;
						if (retry > 5) break;
					}
				}
			}
			if (message_id == 1) {
				AddEventString(p_body_data);
			}
			if (p_body_data != NULL) delete[] p_body_data;

			WSAAsyncSelect(wParam, m_hWnd, 25002, FD_READ | FD_CLOSE);
		}
	}
	else {
		closesocket(mh_socket);
		mh_socket = INVALID_SOCKET;
		m_connect_flag = SOCKET_DISCONNECT;
		AddEventString("서버가 연결을 해제 했습니다.");
	}
	return 0;
}

void CMessengerClientDlg::AddEventString(const char *ap_string) {
	while (m_chat_list.GetCount() > 500) {
		m_chat_list.DeleteString(0);
	}
	int index = m_chat_list.InsertString(-1, (LPCTSTR)ap_string);
	m_chat_list.SetCurSel(index);
}

void CMessengerClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	if (mh_socket != INVALID_SOCKET) {
		closesocket(mh_socket);
		mh_socket = INVALID_SOCKET;
	}
}
