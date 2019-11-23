!define APPNAME "Audacity Packer"
Name "Audacity Packer"
Outfile "audacity-packer-installer.exe"
RequestExecutionLevel admin
InstallDir "$ProgramFiles\${APPNAME}"

!include LogicLib.nsh
!include MUI.nsh
!include x64.nsh

Function .onInit
SetShellVarContext all
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
    MessageBox mb_iconstop "Administrator rights required!"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
${EndIf}
${If} ${RunningX64}
  SetRegView 64
  StrCpy $INSTDIR "$PROGRAMFILES64\${APPNAME}"
${EndIf}
FunctionEnd

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section
SetOutPath "$INSTDIR"
WriteUninstaller "$INSTDIR\Uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Audacity Packer"   "DisplayName" "${APPNAME}"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Audacity Packer"   "UninstallString" "$INSTDIR\Uninstall.exe"
${If} ${RunningX64}
File "/oname=audacity-packer.exe" "audacity-packer-64.exe"
${Else}
File "/oname=audacity-packer.exe" "audacity-packer-32.exe"
${EndIf}
CreateShortCut "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\audacity-packer.exe"
SectionEnd


Section "Uninstall"
Delete "$INSTDIR\audacity-packer.exe"
RMDir  "$INSTDIR"
Delete "$SMPROGRAMS\${APPNAME}.lnk"
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Audacity Packer"
Delete "$INSTDIR\Uninstall.exe"
RMDir "$INSTDIR"
SectionEnd


