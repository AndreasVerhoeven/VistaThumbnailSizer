// VistaThumbnailSizer.cpp : main source file for VistaThumbnailSizer.exe
//

#include "stdafx.h"

#include "resource.h"

#include "MainDlg.h"

CAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	const WCHAR* cmdLine = ::GetCommandLineW();

	// register window messages. Messages starting with Ave are used internally,
	// TaskbarCreated is broadcasted by explorer when the taskbar is created.
	CMainDlg::WM_AVE_ACTION = ::RegisterWindowMessage(_T("AvePleaseDoThisForMeOkayThumbnail"));
	CMainDlg::taskbandRecreatedMsg = ::RegisterWindowMessageW(L"AveThumbnailSizerTaskbandRecreated");
	CMainDlg::taskbarRecreatedMsg = ::RegisterWindowMessage(L"TaskbarCreated");

	// We only want one instance of this app to run, thus we create a mutex - if the mutex
	// already exists, another instance of this app owns it, thus we need to stop executing
	// this instance and show the dialog of the other instance by broadcasting a private, registered
	// message.
	HANDLE mutex = CreateMutex(NULL, TRUE, L"AveThumbnailMutexForRunningOnlyOneApp");
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		SendMessage(HWND_BROADCAST, CMainDlg::WM_AVE_ACTION, 3, 0);
		CloseHandle(mutex);
		return 0;
	}

	// if the commandline to this app includes -kill, we kill all running instances
	// of this app by broadcasting an Ave message.
	if(wcsstr(cmdLine, L"-kill") != 0)
	{
		SendMessage(HWND_BROADCAST, CMainDlg::WM_AVE_ACTION, 2, 0);
		return 0;
	}

	// if the commandline contains -show, show the dialog for this app.
	if(wcsstr(cmdLine, L"-show") != 0)
	{
		SendMessage(HWND_BROADCAST, CMainDlg::WM_AVE_ACTION, 3, 0);
		return 0;
	}


	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	int nRet = 0;
	CMainDlg dlgMain;
	dlgMain.Create(NULL);

	// if the commandline does not contain the -hide switch, only then show the dialog.
	if(wcsstr(cmdLine, L"-hide") == NULL)
		dlgMain.ShowWindow(SW_SHOWNORMAL);

	// if the commandline does not contain the -nohook switch, only then we load the hook.
	if(wcsstr(cmdLine, L"-nohook") == NULL)
	{
		if(dlgMain.IsHookRunning && !dlgMain.IsHookRunning())
		{
			dlgMain.StartHook(dlgMain.hMod, dlgMain.m_hWnd);
		}
	}

	nRet = theLoop.Run();
	_Module.RemoveMessageLoop();

	// if the main message loop ended, make sure to completely
	// end the app.
	if(::IsWindow(dlgMain.m_hWnd))
		dlgMain.DestroyWindow();

	// close the mutex we use for only having one instance of this app
	CloseHandle(mutex);

	// and finally, stop the hook and unload the DLL.
	if(dlgMain.hMod && dlgMain.StopHook)
	{
		dlgMain.StopHook();
		FreeLibrary(dlgMain.hMod);
	}

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
