
// MessengerServerDlg.h: 헤더 파일
//

#pragma once
struct UserData {
	SOCKET h_socket; 
	char ip_address[16]; // 255.255.255.255 + /0 
};

#define MAX_USER_COUNT 100

class CMessengerServerDlg : public CDialogEx
{
public:
	CMessengerServerDlg(CWnd* pParent = nullptr);
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MESSENGERSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	void AddEventString(const char *ap_string);

	void SendFrameData(SOCKET ah_socket, char a_message_id,
		unsigned short int a_body_size, char* ap_send_data);
protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CListBox m_event_list;
	SOCKET mh_listen_socket;
	UserData m_user_list[MAX_USER_COUNT];
protected:
	afx_msg LRESULT On25001(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On25002(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnDestroy();
};
