#pragma once

class CThumbnailSizer
{

private:
	// do now allow instantiations of this class - it's a helper class
	// with static methods only.
	CThumbnailSizer(void) = 0;

	static HWND getTaskBandHandle();
public:
	static BOOL getThumbnailSizes(DWORD* maxCX, DWORD* maxCY);
	static BOOL putThumbnailSizes(DWORD maxCX, DWORD maxCY);
};
