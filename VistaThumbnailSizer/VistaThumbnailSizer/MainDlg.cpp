// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"

#include "ThumbnailSizer.h"

UINT CMainDlg::taskbandRecreatedMsg = 0;
UINT CMainDlg::WM_AVE_ACTION = 0;
UINT CMainDlg::taskbarRecreatedMsg = 0;

//gets the settingspath. path must be non-NULL and must be able to hold at least MAX_PATH
// characters.
void GetSettingsPath(WCHAR* path)
{
	WCHAR appDataPath[MAX_PATH] = {0};
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath);
	PathAppend(appDataPath, L"avethumbnailapp.ini");

	wcscpy_s(path, MAX_PATH, appDataPath);
}

// gets the settings for this app.
void GetSettings(DWORD* maxCx, DWORD* maxCy, BOOL* doFadeAnim, DWORD* fadeDuration)
{
	WCHAR path[MAX_PATH] = {0};
	GetSettingsPath(path);


	if(maxCx != NULL)
	{
		UINT u = (UINT)*maxCx;
		u = GetPrivateProfileIntW(L"thumbnail", L"maxcx", u, path);
		*maxCx = (DWORD)u;
	}

	if(maxCy != NULL)
	{
		UINT u = (UINT)*maxCy;
		u = GetPrivateProfileIntW(L"thumbnail", L"maxcy", u, path);
		*maxCy = (DWORD)u;
	}

	if(doFadeAnim != NULL)
	{
		UINT u = (UINT)*doFadeAnim;
		u = GetPrivateProfileIntW(L"thumbnail", L"doFadeAnim", u, path);
		*doFadeAnim = (BOOL)u;
	}

	if(fadeDuration != NULL)
	{
		UINT u = (UINT)*fadeDuration;
		u = GetPrivateProfileIntW(L"thumbnail", L"fadeDuration", u, path);
		*fadeDuration = (DWORD)u;
	}
}

// writes an int to an INI file (cf. WritePrivateProfileStringW and GetPrivateProfileIntW).
BOOL WritePrivateProfileIntW(const WCHAR* app, const WCHAR* key, DWORD val, const WCHAR* path)
{
	WCHAR str[100] = {0};
	_itow_s(val, str, 100, 10);
	return WritePrivateProfileStringW(app, key, str, path);
}

// writes settings for this app.
void WriteSettings(DWORD maxCx, DWORD maxCy, BOOL doFadeAnim, DWORD fadeDuration)
{
	WCHAR path[MAX_PATH] = {0};
	GetSettingsPath(path);

	WritePrivateProfileIntW(L"thumbnail", L"maxcx", maxCx, path);
	WritePrivateProfileIntW(L"thumbnail", L"maxcy", maxCy, path);
	WritePrivateProfileIntW(L"thumbnail", L"doFadeAnim", doFadeAnim, path);
	WritePrivateProfileIntW(L"thumbnail", L"fadeDuration", fadeDuration, path);
}

// gets a full filepath for this app by name.
void GetFilePath(WCHAR* buf, const WCHAR* name)
{
	GetModuleFileName(NULL, buf, MAX_PATH);
	PathRemoveFileSpec(buf);
	PathAppendW(buf, name);
}



LRESULT CMainDlg::OnTaskbandRecreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// if the taskband is recreated (taskbar moved for example),
	// rewrite the maximum thumbnail sizes.
	CThumbnailSizer::putThumbnailSizes(cx, cy);
	return 0;
}

LRESULT CMainDlg::OnAveAction(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(2 == wParam)
	{
		// -kill
		PostQuitMessage(0);
	}
	else if(3 == wParam)
	{
		// - show
		ShowWindow(SW_SHOWNORMAL);
	}

	return 0;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	WriteSettings(cx, cy, doFadeAnim, fadeDuration);
	PostQuitMessage(0);
	return 0;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// we actually don't close the window, we hide it.
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	isInit = FALSE;

	// load the hook DLL and bind the function pointers from the hook DLL.
	WCHAR hookPath[MAX_PATH] = {0};
	GetFilePath(hookPath, L"taskbandhook.dll");
	hMod = LoadLibrary(hookPath);
	if(hMod != NULL)
	{
		StartHook     = (PStartHook)GetProcAddress(hMod, "StartHook");
		StopHook      = (PStopHook)GetProcAddress(hMod, "StopHook");
		IsHookRunning = (PIsHookRunning)GetProcAddress(hMod, "IsHookRunning");
		SetFadeAnim	  = (PSetFadeAnim) GetProcAddress(hMod, "SetFadeAnim");
	}
	else
	{
		StartHook = NULL;
		StopHook = NULL;
		IsHookRunning = NULL;
		SetFadeAnim = NULL;
	}

	// default values
	doFadeAnim = TRUE;
	fadeDuration = 350;

	// bind sliders
	sliderCx = GetDlgItem(IDC_SLIDER_CX);
	sliderCy = GetDlgItem(IDC_SLIDER_CY);
	sliderFadeDuration = GetDlgItem(IDC_FADEDURATION);

	// set slider ranges to some values that make sense
	sliderCx.SetRange(0, 1024);
	sliderCy.SetRange(0, 1024);
	sliderFadeDuration.SetRange(10, 1000);

	cx = 0;
	cy = 0;	

	// set the sliders to the current thumbnail sizes.
	if(CThumbnailSizer::getThumbnailSizes(&cx, &cy))
	{
		sliderCx.SetPos(cx);
		sliderCy.SetPos(cy);
	}

	// load settings and update the controls to indicate the settings
	GetSettings(&cx, &cy, &doFadeAnim, &fadeDuration);
	CThumbnailSizer::putThumbnailSizes(cx, cy);
	
	sliderFadeDuration.SetPos(fadeDuration);
	CheckDlgButton(IDC_DOFADE, doFadeAnim);
	BOOL bDummy=FALSE;
	OnBnClickedDofade(0,0,0, bDummy);

	isInit = TRUE;
	return TRUE;
}

LRESULT CMainDlg::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if(isInit)
	{
		// change the fade anim on slider change
		if((HWND)lParam == sliderFadeDuration.m_hWnd)
		{
			fadeDuration = sliderFadeDuration.GetPos();
			if(SetFadeAnim != NULL)
				SetFadeAnim(doFadeAnim, fadeDuration);
			return 0;
		}

		// if shift is hold, we sync the two thumbnail sizes sliders.
		if(GetKeyState(VK_SHIFT) < 0)
		{
			isInit = FALSE;
			if((HWND)lParam == sliderCx.m_hWnd)
				sliderCy.SetPos(sliderCx.GetPos());
			else if((HWND)lParam == sliderCy.m_hWnd)
				sliderCx.SetPos(sliderCy.GetPos());
			isInit = TRUE;
		}

		cx = sliderCx.GetPos();
		cy = sliderCy.GetPos();

		CThumbnailSizer::putThumbnailSizes(cx, cy);
	}

	return 0;
}

LRESULT CMainDlg::OnQuitClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostQuitMessage(0);
	return 0;
}
LRESULT CMainDlg::OnBnClickedDofade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// if the fade checkbox is changed, update the fade anim
	doFadeAnim = IsDlgButtonChecked(IDC_DOFADE);
	
	::EnableWindow(GetDlgItem(IDC_FADESPEED), doFadeAnim);
	::EnableWindow(GetDlgItem(IDC_FADE_TXT1), doFadeAnim);
	::EnableWindow(GetDlgItem(IDC_FADE_TXT2), doFadeAnim);
	sliderFadeDuration.EnableWindow(doFadeAnim);

	if(SetFadeAnim != NULL)
		SetFadeAnim(doFadeAnim, fadeDuration);

	return 0;
}
