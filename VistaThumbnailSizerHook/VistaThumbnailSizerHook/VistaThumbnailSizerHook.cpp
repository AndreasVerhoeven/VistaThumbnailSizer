// VistaThumbnailSizerHook.cpp : Defines the entry point for the DLL application.
//

/// This DLL gets loaded into explorer.exe to catch
/// taskbar recreation (not notified by TaskbarCreated), for instance,
/// when changing the position of the taskbar.
/// Also, the DLL adds an option for making thumbnails fade-in.

#include "stdafx.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif


// data shared between all instances of this DLL
#pragma data_seg(".AVETASKBANDHOOK")
HHOOK	hook		= NULL;
HWND	owner		= NULL;
HWND	taskband	= NULL;
UINT	notifyMsg	= 0;
BOOL	doFadeAnim	= TRUE;
DWORD	fadeDuration= 350;
#pragma data_seg()
#pragma comment(linker, "/section:.AVETASKBANDHOOK,rws")

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

// sets the alpha level of a window
void SetWinAlpha(HWND hwnd, BYTE alpha)
{
	if(!::IsWindow(hwnd))
		return;

	LONG l = GetWindowLong(hwnd, GWL_EXSTYLE);
	if(!(l & WS_EX_LAYERED))
		SetWindowLong(hwnd, GWL_EXSTYLE, l | WS_EX_LAYERED);
	
	 SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

// gets the taskbar handle
HWND getTaskBandHandle()
{
	const WCHAR* topParent   = L"Shell_TrayWnd";
	const WCHAR* grandParent = L"ReBarWindow32";
	const WCHAR* parent		 = L"MSTaskSwWClass";

	HWND hwnd = FindWindowW(topParent, NULL);
	if(NULL == hwnd)
		return NULL;

	hwnd = FindWindowExW(hwnd, NULL, grandParent, NULL);
	if(NULL == hwnd)
		return NULL;

	hwnd = FindWindowExW(hwnd, NULL, parent, NULL);

	return hwnd;
}


// timer procedure for fade anmation
VOID CALLBACK FadeTimerProc(HWND hwnd,   UINT uMsg,   UINT_PTR idEvent,   DWORD dwTime)
{
	// our fade animation is frame-dropping based: it's always fadeDuration milliseconds
	// in length. Based on the progress thru time, we advance in the animation.
	BOOL fadingIn = (BOOL)GetProp(hwnd, L"avefadingin");
	int progress = (int)GetProp(hwnd, L"aveprogress");
	DWORD prevTick = (DWORD)GetProp(hwnd, L"avetick");

	// calculate the new progress in time
	DWORD curTick = GetTickCount();
	DWORD prog = min(curTick - prevTick,fadeDuration);

	if(fadingIn)
	{
		// use a nice non-linear - in this case sin based - animation
		if(prog >= fadeDuration || progress==255)
		{
			progress = 255;
			KillTimer(hwnd, idEvent);
		}
		else
		{
			progress = int(sin( 3.1492f / 2.0f * float(prog) / float(fadeDuration)) * 255.0f);
		}

		SetWinAlpha(hwnd, progress);
	}
	else
	{
		// if fading out, simply use a linear animation to reset the progress -> nicer effect
		// when hovering over other taskbar buttons quickly.
		progress = max(progress - 10, 0);
		if(0 == progress)
			KillTimer(hwnd, idEvent);
	}

	SetProp(hwnd, L"aveprogress", (HANDLE)progress);

	
}

// hook callback function
LRESULT CALLBACK CallWndProc(int code, WPARAM wParam,  LPARAM lParam)
{
	CWPSTRUCT* cpw = reinterpret_cast<CWPSTRUCT*>(lParam);

	// first, catch thumbnail windows if we want to do a fade animation.
	if(cpw != NULL)
	{
		if(doFadeAnim)
		{
			if(cpw->message == WM_WINDOWPOSCHANGING)
			{
				WCHAR clsName[100] = {0};
				GetClassName(cpw->hwnd, clsName, 100);
				if(wcscmp(clsName, L"ThumbnailClass") == 0)
				{
					WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(cpw->lParam);
					if(wp!= NULL)
					{
						if(wp->flags & SWP_SHOWWINDOW)
						{
							SetWinAlpha(cpw->hwnd, 0);
							SetProp(cpw->hwnd, L"avefadingin", (HANDLE)1);
							SetProp(cpw->hwnd, L"avetick", (HANDLE)GetTickCount());
							SetTimer(cpw->hwnd, 1, 10, FadeTimerProc);
						}
						else if(wp->flags & SWP_HIDEWINDOW)
						{
							SetProp(cpw->hwnd, L"avefadingin", (HANDLE)0);
							SetTimer(cpw->hwnd, 1, 10, FadeTimerProc);
						}
					}
				}
			}
		}

		// if the window is the taskband and it's different from the one we think is the taskbar,
		// reset.
		if(!IsWindow(taskband))
		{
			HWND tb = getTaskBandHandle();
			if(cpw->hwnd == tb)
			{
				taskband = tb;
				PostMessage(owner, notifyMsg, 0L, 0L);
			}
		}
		else if(cpw->hwnd == taskband && (cpw->message == WM_WINDOWPOSCHANGING || cpw->message == WM_WININICHANGE))
		{
			taskband = NULL;
		}
	}

	return CallNextHookEx(hook, code, wParam, lParam);
}

// method to start the hook
BOOL CALLBACK StartHook(HMODULE hMod, HWND hwnd)
{
	if(hook != NULL)
		return FALSE;

	notifyMsg = ::RegisterWindowMessageW(L"AveThumbnailSizerTaskbandRecreated");

	owner = hwnd;

	taskband = getTaskBandHandle();

	DWORD threadid = GetWindowThreadProcessId(taskband, 0);
	hook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hMod, threadid);
	if(NULL == hook)
		return FALSE;

	return TRUE;
}

// method to stop the hook
BOOL CALLBACK StopHook()
{
	if(NULL == hook)
		return FALSE;

	BOOL res = UnhookWindowsHookEx(hook);
	if(res || !IsWindow(taskband))
	{
		hook = NULL;
	}

	return res;
}

// returns TRUE iff the hook is running
BOOL CALLBACK IsHookRunning()
{
	return hook != NULL;
}

void CALLBACK SetFadeAnim(BOOL enabled, DWORD duration)
{
	doFadeAnim = enabled;
	fadeDuration = duration;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

