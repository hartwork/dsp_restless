////////////////////////////////////////////////////////////////////////////////
/// Restless Winamp Plugin
///
/// Copyright � 2005  Sebastian Pipping <webmaster@hartwork.org>
/// Copyright � 2017  M. Wysocki
///
/// -->  https://github.com/hartwork/dsp_restless
///
/// This source code is released as Public Domain.
////////////////////////////////////////////////////////////////////////////////


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>
#include <stdio.h>

#include "Dsp.h"
#include "wa_ipc.h"



#ifdef __cplusplus
# define DLLEXPORT extern "C" __declspec(dllexport)
#else
# define DLLEXPORT __declspec(dllexport)
#endif



static const char * const INI_SECTION = "Restless Winamp Plugin";
static const char * const INI_KEY_HIT_START_AFTER_MILLIS = "Hit_Start_After_Millis";
static const char * const INI_KEY_HIT_START_AFTER_MINUTES_SECONDS = "Hit_Start_After_Minutes_Seconds";
static const char * const INI_KEY_ONLY_WHEN_REPEAT = "Hit_Start_Only_When_Repeat_Enabled";
static const char * const INI_KEY_PAUSE_INTERVAL_OPEN = "Pause_Interval_Open";
static const char * const INI_KEY_PAUSE_INTERVAL_CLOSE = "Pause_Interval_Close";
static const char * const INI_KEY_PAUSE_ENABLED = "Pause_Enabled";

DWORD lastPlayingTimestamp = 0;
UINT_PTR hTimer = 0;
unsigned int hitStartAfterMillis = 5000;
const char * fullWinampIniPath = NULL;
unsigned int pauseLeftMinutes = 24 * 60;
unsigned int pauseRightMinutes = 24 * 60;
bool pauseEnabled = false;
bool checkRepeat = false;



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
	"Restless Winamp Plugin // 2017-07-20",
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
		"Copyright � 2005 Sebastian Pipping   \n"
		"<webmaster@hartwork.org>\n"
		"Copyright � 2017 M. Wysocki   \n"
		"\n"
		"-->  https://github.com/hartwork/dsp_restless",
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

		const time_t nowStamp = time(NULL);
		struct tm * nowDetails = localtime(&nowStamp);
		const unsigned int nowHourMinutes = nowDetails->tm_hour * 60
				+ nowDetails->tm_min;

		// Within pause interval?
		if (!(pauseEnabled
				&& (nowHourMinutes >= pauseLeftMinutes)
				&& (nowHourMinutes < pauseRightMinutes))) {
			unsigned int repeatEnabled;
			repeatEnabled = SendMessage(mod_restless.hwndParent,
							WM_WA_IPC, 0, IPC_GET_REPEAT);
			if (((checkRepeat) && (repeatEnabled == 1)) || (!(checkRepeat))) {
				SendMessage(mod_restless.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
			}
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
void extractAroundColon(const char * text, unsigned int & output,
		unsigned int def) {
	unsigned int first;
	unsigned int second;
	if (sscanf(text, "%u:%2u", &first, &second) == 2) {
		if (second <= 59) {
			def = first * 60 + second;
		}
	}
	output = def;
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
				INI_KEY_HIT_START_AFTER_MILLIS, -1, fullWinampIniPath);
		if ((candidate >= 1000) && (candidate <= (1 << 30))) {
			hitStartAfterMillis = candidate;
		}

		char oneColonTwo[10];
		int len = GetPrivateProfileString(INI_SECTION,
				INI_KEY_HIT_START_AFTER_MINUTES_SECONDS, "", oneColonTwo,
				10, fullWinampIniPath);
		if (len >= 4) {
			unsigned int candidate;
			extractAroundColon(oneColonTwo, candidate, 0);
			if ((candidate >= 1) && (candidate <= ((1 << 30) / 1000))) {
				hitStartAfterMillis = candidate * 1000;
			}
		}

		len = GetPrivateProfileString(INI_SECTION,
				INI_KEY_PAUSE_INTERVAL_OPEN, "", oneColonTwo, 10, fullWinampIniPath);
		if (len >= 4) {
			extractAroundColon(oneColonTwo, pauseLeftMinutes, 24 * 60);
		}

		len = GetPrivateProfileString(INI_SECTION,
				INI_KEY_PAUSE_INTERVAL_CLOSE, "", oneColonTwo, 10, fullWinampIniPath);
		if (len >= 4) {
			extractAroundColon(oneColonTwo, pauseRightMinutes, 24 * 60);
		}

		char zeroOrNot[10];
		len = GetPrivateProfileString(INI_SECTION,
				INI_KEY_PAUSE_ENABLED, "0", zeroOrNot, 10, fullWinampIniPath);
		if (len >= 1) {
			if (strcmp(zeroOrNot, "0")) {
				pauseEnabled = true;
			}
		}

		len = GetPrivateProfileString(INI_SECTION,
				INI_KEY_ONLY_WHEN_REPEAT, "0", zeroOrNot, 10, fullWinampIniPath);
		if (len >= 1) {
			if (strcmp(zeroOrNot, "1") == 0) {
				checkRepeat = true;
			}
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
		char oneColonTwo[20];
		WritePrivateProfileInt(INI_SECTION, INI_KEY_HIT_START_AFTER_MILLIS,
				hitStartAfterMillis, fullWinampIniPath);

		if ((hitStartAfterMillis % 1000) == 0) {
			unsigned int seconds = hitStartAfterMillis / 1000;
			sprintf(oneColonTwo, "%02i:%02i", seconds / 60,
					seconds % 60);
			WritePrivateProfileString(INI_SECTION,
					INI_KEY_HIT_START_AFTER_MINUTES_SECONDS,
					oneColonTwo, fullWinampIniPath);
		}

		WritePrivateProfileString(INI_SECTION, INI_KEY_ONLY_WHEN_REPEAT,
				checkRepeat ? "1" : "0", fullWinampIniPath);

		sprintf(oneColonTwo, "%02i:%02i", pauseLeftMinutes / 60,
				pauseLeftMinutes % 60);
		WritePrivateProfileString(INI_SECTION, INI_KEY_PAUSE_INTERVAL_OPEN,
				oneColonTwo, fullWinampIniPath);

		sprintf(oneColonTwo, "%02i:%02i", pauseRightMinutes / 60,
				pauseRightMinutes % 60);
		WritePrivateProfileString(INI_SECTION, INI_KEY_PAUSE_INTERVAL_CLOSE,
				oneColonTwo, fullWinampIniPath);

		WritePrivateProfileString(INI_SECTION, INI_KEY_PAUSE_ENABLED,
				pauseEnabled ? "1" : "0", fullWinampIniPath);
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
