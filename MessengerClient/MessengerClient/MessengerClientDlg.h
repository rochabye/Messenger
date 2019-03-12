
#pragma once

#define MSG_KEY 27

class CMessengerClientDlg : public CDialogEx
{
public:
	CMessengerClientDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MESSENGERCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	void AddEventString(const char *ap_string);
	void SendFrameData(SOCKET ah_socket, char a_message_id, unsigned short int a_body_size, char* ap_send_data);
protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListBox m_chat_list;
	afx_msg void OnBnClickedSendBtn();

private:
	enum {
		SOCKET_DISCONNECT = 0,
		SOCKET_CONNECTING,
		SOCKET_CONNECTED,
	};
	SOCKET mh_socket = INVALID_SOCKET;
	char m_connect_flag = 0; // 0: disconnect 1:connecting 2:connected
protected:
	afx_msg LRESULT On25001(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On25002(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnDestroy();
};
