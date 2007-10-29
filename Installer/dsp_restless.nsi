; Copyright © 2007  Sebastian Pipping <webmaster@hartwork.org>
;
; This script was derived from the 'Winamp Plugin Installer' example:
; http://nsis.sourceforge.net/Winamp_Plugin_Installer_Scripts

!define VERSION "1.2.0"
!define VER_ALT "1_2_0"
!define PLUG "Restless Winamp Plugin"
!define PLUG_ALT "dsp_restless"
!define PLUG_FILE "dsp_restless"



SetCompressor lzma
Name "${PLUG} ${VERSION}"
OutFile "..\${PLUG_ALT}_${VER_ALT}_setup.exe"
InstallDir $PROGRAMFILES\Winamp
InstProgressFlags smooth
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" "UninstallString"
DirText "Please select your Winamp path below (you will be able to proceed when Winamp is detected):"
AutoCloseWindow true
XPStyle on
ShowInstDetails show



PageEx directory
  Caption " "
PageExEnd

Page instfiles



Function CloseWinamp
  Push $5
  loop:
    FindWindow $5 "Winamp v1.x"
    IntCmp $5 0 done
    SendMessage $5 16 0 0
    Sleep 100
    Goto loop
  done:
  Pop $5
  Sleep 100
FunctionEnd



Section ""
  Call CloseWinamp

  SetOverwrite on
  SetOutPath "$INSTDIR\Plugins"
  File "..\Binary\${PLUG_FILE}.dll"
  SetOverwrite off
SectionEnd



Function .onInstSuccess
  MessageBox MB_YESNO "${PLUG} was installed. Do you want to run Winamp now?" IDNO end
    ExecShell open "$INSTDIR\Winamp.exe"
  end:
FunctionEnd

Function .onVerifyInstDir
  IfFileExists $INSTDIR\Winamp.exe good
    Abort
  good:
FunctionEnd
