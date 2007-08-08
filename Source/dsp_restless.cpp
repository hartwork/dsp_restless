////////////////////////////////////////////////////////////////////////////////
/// Restless Winamp Plugin
///
/// Copyright © 2007  Sebastian Pipping <webmaster@hartwork.org>
///
/// -->  http://www.hartwork.org/
///
/// This source code is released as Public Domain.
////////////////////////////////////////////////////////////////////////////////


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Dsp.h"
#include "wa_ipc.h"



#ifdef __cplusplus
# define DLLEXPORT extern "C" __declspec(dllexport)
#else
# define DLLEXPORT __declspec(dllexport)
#endif



static const char * const INI_SECTION = "Restless Winamp Plugin";
static const char * const INI_KEY = "Hit_Start_After_Millis";

DWORD lastPlayingTimestamp = 0;
UINT_PTR hTimer = 0;
unsigned int hitStartAfterMillis = 5000;
const char * fullWinampIniPath = NULL;



void config_restless(struct winampDSPModule * this_mod);
int init_restless(struct winampDSPModule * this_mod);
int apply_restless(struct winampDSPModule * this_mod, short int * samples, int numsamples, int bps, int nch, int srate);
void quit_restless(struct winampDSPModule * this_mod);

winampDSPModule * getModule(int which);

BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int iValue, LPCTSTR lpFileName);



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
winampDSPHeader header = {
	DSP_HDRVER,
	"Restless Winamp Plugin // 2007-08-08",
	getModule
};



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
winampDSPModule mod_restless = {
	"Restless Winamp Plugin",
	NULL,             // hwndParent
	NULL,			  // hDllInstance
	config_restless,
	init_restless,
	apply_restless,
	quit_restless,
	0                 // userData
};



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
void config_restless(struct winampDSPModule * this_mod) {
	MessageBox(
		this_mod->hwndParent,
		"Restless Winamp Plugin\n"
		"\n"
		"Copyright © 2007 Sebastian Pipping   \n"
		"<webmaster@hartwork.org>\n"
		"\n"
		"-->  http://www.hartwork.org",
		"About",
		MB_ICONINFORMATION
	);
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
VOID CALLBACK TimerProc(HWND /*hwnd*/, UINT /*uMsg*/,
		UINT_PTR /*idEvent*/, DWORD /*dwTime*/) {
	const DWORD now = GetTickCount();
	if (now - lastPlayingTimestamp > hitStartAfterMillis)
	{
		lastPlayingTimestamp = now;
		SendMessage(mod_restless.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
	}
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
int init_restless(struct winampDSPModule * /*this_mod*/) {
	// Get Winamp.ini path
	fullWinampIniPath = (char *)SendMessage(mod_restless.hwndParent,
			WM_WA_IPC, 0, IPC_GETINIFILE);

	// Read config
	if (fullWinampIniPath != NULL) {
		const int candidate = GetPrivateProfileInt(INI_SECTION,
				INI_KEY, -1, fullWinampIniPath);
		if ((candidate >= 1000) && (candidate <= 60000)) {
			hitStartAfterMillis = candidate;
		}
	}

	// Setup timer, half the time to get proper delay
	lastPlayingTimestamp = GetTickCount();
	hTimer = SetTimer(NULL, 0, hitStartAfterMillis / 2, TimerProc);

	return 0;	// Fine
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
int apply_restless(struct winampDSPModule * /*this_mod*/,
		short int * /*samples*/, int numsamples, int /*bps*/,
		int /*nch*/, int /*srate*/) {
	lastPlayingTimestamp = GetTickCount();
	return numsamples;
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
void quit_restless(struct winampDSPModule * /*this_mod*/) {
	KillTimer(NULL, hTimer);

	// Write config
	if (fullWinampIniPath != NULL) {
		WritePrivateProfileInt(INI_SECTION, INI_KEY,
				hitStartAfterMillis, fullWinampIniPath);
	}
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
winampDSPModule * getModule(int which) {
	return (which ? NULL : &mod_restless);
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT winampDSPHeader * winampDSPGetHeader2() {
	return &header;
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName,
		int iValue, LPCTSTR lpFileName) {
	TCHAR szBuffer[ 16 ];
	wsprintf(szBuffer, TEXT("%i"), iValue);
    return WritePrivateProfileString(lpAppName, lpKeyName, szBuffer, lpFileName);
}
