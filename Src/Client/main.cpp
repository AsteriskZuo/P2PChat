
#include "stdafx.h"
#include "conductor.h"
#include "MainFrame.h"
#include "peer_connection_client.h"
#include "rtc_base/checks.h"
#include "rtc_base/ssladapter.h"
#include "rtc_base/win32socketinit.h"
#include "rtc_base/win32socketserver.h"
#include "setdebugnew.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	::_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	::_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#endif

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

 	rtc::EnsureWinsockInit();
	rtc::Win32SocketServer w32_ss;
 	rtc::Win32Thread w32_thread(&w32_ss);
 	rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

	MainFrame* pFrame = new MainFrame();
	if (pFrame == NULL) return 0;
#if defined(WIN32) && !defined(UNDER_CE)
	pFrame->Create(NULL, _T("p2pChat"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
#else
	pFrame->Create(NULL, _T("p2pChat"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#endif
	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);

	rtc::InitializeSSL();

	PeerConnectionClient client;
	rtc::scoped_refptr<Conductor> conductor(
		new rtc::RefCountedObject<Conductor>(&client, pFrame));

	CPaintManagerUI::MessageLoop();

	rtc::CleanupSSL();

	return 0;
}


