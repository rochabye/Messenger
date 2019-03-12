
#include "stdafx.h"
#include "MessengerClient.h"
#include "MessengerClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMessengerClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CMessengerClientApp::CMessengerClientApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

}


CMessengerClientApp theApp;


BOOL CMessengerClientApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	WSADATA temp;
	WSAStartup(0x0202, &temp);

	CMessengerClientDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	AfxEnableControlContainer();

	CShellManager *pShellManager = new CShellManager;

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));

	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	WSACleanup();
	return FALSE;
}

