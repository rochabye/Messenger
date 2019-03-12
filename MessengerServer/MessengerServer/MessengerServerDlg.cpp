
// MessengerServerDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "MessengerServer.h"
#include "MessengerServerDlg.h"
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



CMessengerServerDlg::CMessengerServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MESSENGERSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMessengerServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EVENT_LIST, m_event_list);
}

BEGIN_MESSAGE_MAP(CMessengerServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(25001, &CMessengerServerDlg::On25001)
	ON_MESSAGE(25002, &CMessengerServerDlg::On25002)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


BOOL CMessengerServerDlg::OnInitDialog()
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

	mh_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	// AF_INET : internet basic TCP address. 

	for (int i = 0; i < MAX_USER_COUNT; ++i) {
		m_user_list[i].h_socket = INVALID_SOCKET;
	}

	sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);// inet_addr("10.103.196.181");
	srv_addr.sin_port = htons(18000);

	bind(mh_listen_socket, (LPSOCKADDR)&srv_addr, sizeof(srv_addr));
	// socket connect network card
	AddEventString("service start.");
	listen(mh_listen_socket, 1);

	WSAAsyncSelect(mh_listen_socket, m_hWnd, 25001, FD_ACCEPT);
	return TRUE; 
}

void CMessengerServerDlg::AddEventString(const char *ap_string)
{
	while (m_event_list.GetCount() > 3000) {
		m_event_list.DeleteString(0);
	}
	int index = m_event_list.InsertString(-1, ap_string);
	m_event_list.SetCurSel(index);
}
void CMessengerServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMessengerServerDlg::OnPaint()
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

HCURSOR CMessengerServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



afx_msg LRESULT CMessengerServerDlg::On25001(WPARAM wParam, LPARAM lParam)
{ // wparam -> socket handle, plaram -> socket message 

	sockaddr_in client_addr;
	int sockaddr_in_size = sizeof(sockaddr_in);
	SOCKET h_socket = accept(mh_listen_socket, (LPSOCKADDR)&client_addr, &sockaddr_in_size);
	// mh_listen_socket = wparam
	// client_addr = client ip
	// h_socket = clone socket. listen socket copy
	CString log;
	
	int index;
	for (index = 0; index < MAX_USER_COUNT; ++index) { // search empty 
		if (m_user_list[index].h_socket == INVALID_SOCKET) break;
	}
	log.Format("index : %d", index);
	AddEventString(log);

	if (index < MAX_USER_COUNT) {
		m_user_list[index].h_socket = h_socket;
		strcpy_s(m_user_list[index].ip_address, inet_ntoa(client_addr.sin_addr));
		// client address copy 
		WSAAsyncSelect(m_user_list[index].h_socket, m_hWnd, 25002, FD_READ | FD_CLOSE );
		CString str;
		str.Format("%s에서 접속했습니다.", m_user_list[index].ip_address);
	}
	else { // full
		AddEventString("관리 최대 인원 초과 \n");
		closesocket(h_socket);
	}
	return 0;
}

// message struct : 
// | key (1byte) | message_id (1byte) | body_size (2byte) | .... data .... |

afx_msg LRESULT CMessengerServerDlg::On25002(WPARAM wParam, LPARAM lParam)
{
	// lparam : FD_READ or FD_CLOSE 
	
	CString str;
	if (WSAGETSELECTEVENT(lParam) == FD_READ) {
		WSAAsyncSelect(wParam, m_hWnd, 25002, FD_CLOSE);
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
				while (total < body_size) {
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
				int i;
				for (i = 0; i < MAX_USER_COUNT; ++i) {
					if (m_user_list[i].h_socket == wParam) break;
				}

				CString str2;
				str2.Format("%s: %s", m_user_list[i].ip_address, p_body_data);
				AddEventString(str2);

				for (i = 0; i < MAX_USER_COUNT; ++i) {
					if (m_user_list[i].h_socket != INVALID_SOCKET) {
						SendFrameData(m_user_list[i].h_socket, 1, str2.GetLength() + 1, (char*)(const char*) str2);
					}
				}
			}

		
			if (p_body_data != NULL) delete[] p_body_data;
			WSAAsyncSelect(wParam, m_hWnd, 25002, FD_READ | FD_CLOSE );
		}
		else { // fd_close 
			closesocket(wParam);
			for (int i = 0; i < MAX_USER_COUNT; ++i) {
				if (m_user_list[i].h_socket == wParam) {
					m_user_list[i].h_socket = INVALID_SOCKET;
					str.Format("exit.. : %s ", m_user_list[i].ip_address);
					AddEventString(str);
					break;
				}
			}
		}
	}
	else {// FD_CLOSE

}
	return 0;
}

void CMessengerServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	for (int i = 0; i < MAX_USER_COUNT; ++i) {
		if (m_user_list[i].h_socket != INVALID_SOCKET) {
			closesocket(m_user_list[i].h_socket);
		}
	}
}
void CMessengerServerDlg::SendFrameData(SOCKET ah_socket, char a_message_id,
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
