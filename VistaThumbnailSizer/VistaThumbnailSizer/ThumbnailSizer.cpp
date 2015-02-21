#include "StdAfx.h"
#include "ThumbnailSizer.h"



// gets the handle to the taskbar's taskband
HWND CThumbnailSizer::getTaskBandHandle()
{
	const WCHAR* grandParent   = L"Shell_TrayWnd";
	const WCHAR* parent		   = L"ReBarWindow32";
	const WCHAR* className	   = L"MSTaskSwWClass";

	HWND hwnd = FindWindowW(grandParent, NULL);
	if(NULL == hwnd)
		return NULL;

	hwnd = FindWindowExW(hwnd, NULL, parent, NULL);
	if(NULL == hwnd)
		return NULL;

	hwnd = FindWindowExW(hwnd, NULL, className, NULL);

	return hwnd;
}


#ifdef _M_X64
	#define THUMBNAIL_MAX_CX_OFFSET	0x44
	#define THUMBNAIL_MAX_CY_OFFSET	0x48
#else
	// byte offsets to the maximum thumbnail sizes
	#define THUMBNAIL_MAX_CX_OFFSET 0x24
	#define THUMBNAIL_MAX_CY_OFFSET	0x28
#endif


// method to get the maximum thumbnail sizes
BOOL CThumbnailSizer::getThumbnailSizes(DWORD* maxCX, DWORD* maxCY)
{
	HWND hwnd = getTaskBandHandle();
	if(NULL == hwnd)
		return FALSE;

	// Explorer uses thunks to associate a window
	// with an object instance: the thunk stores the pointer to the
	// window in the userdata bytes of the window which we can access
	// with the GetWindowLong() call.
	BYTE* data = (BYTE*)GetWindowLong(hwnd, 0);
	if(NULL == data)
		return FALSE;

	// since this pointer lives in explorer's memoryspace and this 
	// app is executed in its own memoryspace, the pointer we have
	// is of no use: thus we need to actually read beyond process
	// boundaries using ReadProcess memory; for that to work,
	// we need to open a handle to the process with suffient rights.
	DWORD dwPid = -1;
	GetWindowThreadProcessId(hwnd, &dwPid);
	if(dwPid <= 0)
		return FALSE;

	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ, NULL, dwPid);
	if(NULL == hProc)
		return FALSE;

	if(maxCX != NULL)
	{
		SIZE_T numRead = 0;
		ReadProcessMemory(hProc, data + THUMBNAIL_MAX_CX_OFFSET, maxCX, sizeof(DWORD), &numRead); 
	}

	if(maxCY != NULL)
	{
		SIZE_T numRead = 0;
		ReadProcessMemory(hProc, data + THUMBNAIL_MAX_CY_OFFSET, maxCY, sizeof(DWORD), &numRead); 
	}

	CloseHandle(hProc);

	return TRUE;
}

// sets the maximum thumbnail size.
BOOL CThumbnailSizer::putThumbnailSizes(DWORD maxCX, DWORD maxCY)
{
	// idem as getThumbnailSizes(), but for writing the size instead
	// of reading.

	HWND hwnd = getTaskBandHandle();
	if(NULL == hwnd)
		return FALSE;

	BYTE* data = (BYTE*)GetWindowLong(hwnd, 0);
	if(NULL == data)
		return FALSE;

	DWORD dwPid = -1;
	GetWindowThreadProcessId(hwnd, &dwPid);
	if(dwPid <= 0)
		return FALSE;

	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, NULL, dwPid);
	if(NULL == hProc)
		return FALSE;

	SIZE_T numWritten=0;
	WriteProcessMemory(hProc, data + THUMBNAIL_MAX_CX_OFFSET, &maxCX, sizeof(DWORD), &numWritten);
	WriteProcessMemory(hProc, data + THUMBNAIL_MAX_CY_OFFSET, &maxCY, sizeof(DWORD), &numWritten);

	CloseHandle(hProc);

	return TRUE;
}