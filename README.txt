copyright (c) Andreas Verhoeven <andreasverhoeven@hotmail.com>, 2007.

LICENSE:
	You are free to use this code in whatever you like, however you like.

	Do not re-release any binary files resulting from these projects under
	your own name.



HOW?
	This apps changes the maximum thumbnail size for the Aero 
	live-window-taskbar-hover previews into a user defined value.
	Currently, this value is not changeble, since it's hardcoded in
	explorer. 
	This app finds out the position in explorer.exe's memory
	of those maximum thumbnail sizes and changes them.
	The correct memory position were found by debugging 
	explorer.exe with windbg and reading the ASSEMBLER instructions.

	Also, the app also loads a hook DLL into explorer which tries
	to detect taskband resets and re-apply the change.

	Next to that,the hook also gives an option to fade the 
	thumbnails in with a nice animation, rather than just appearing.


WHAT?
	+ VistaThumbnailSizer, application for changing thumbnail sizes
		and loading the hook -> creates AveThumbnailSizer.exe

	+ VistaThumbnailSizerHook, the hook DLL that gets loaded into
		explorer -> creates taskbandhook.dll
	

REMARKS:
	Output paths are hardcoded to c:\thumbnail\. Change as you like.	


NEEDED:
	+ Vista PlatformSDK
	+ WTL 7.5+
