// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ThumbnailSizer.h"

typedef BOOL (CALLBACK* PStartHook)(HMODULE, HWND);
typedef BOOL (CALLBACK* PStopHook)();
typedef BOOL (CALLBACK* PIsHookRunning)();
typedef void (CALLBACK* PSetFadeAnim)(BOOL,DWORD);

class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	CTrackBarCtrl sliderCx;
	CTrackBarCtrl sliderCy;
	CTrackBarCtrl sliderFadeDuration;

	BOOL isInit;

	static UINT taskbandRecreatedMsg;
	static UINT WM_AVE_ACTION;
	static UINT taskbarRecreatedMsg;

	PStartHook     StartHook;
	PStopHook      StopHook;
	PIsHookRunning IsHookRunning;
	PSetFadeAnim   SetFadeAnim;
	HMODULE hMod;

	DWORD cx;
	DWORD cy;

	BOOL doFadeAnim;
	DWORD fadeDuration;

public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		if(uMsg == WM_AVE_ACTION)
		{
			OnAveAction(uMsg, wParam, lParam, bHandled);
		}
		else if(uMsg == taskbandRecreatedMsg)
		{
			OnTaskbandRecreated(uMsg, wParam, lParam, bHandled);
		}
		else if(uMsg == taskbarRecreatedMsg)
		{
			if(StopHook != NULL && StartHook != NULL)
			{
				StopHook();
				StartHook(hMod, m_hWnd);

				CThumbnailSizer::putThumbnailSizes(cx, cy);
				BOOL bDummy=FALSE;
				OnBnClickedDofade(0,0,0, bDummy);
			}
		}

		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_QUIT, BN_CLICKED, OnQuitClicked)

		COMMAND_HANDLER(IDC_DOFADE, BN_CLICKED, OnBnClickedDofade)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAveAction(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTaskbandRecreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnQuitClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDofade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
