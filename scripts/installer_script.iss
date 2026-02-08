; 安装程序脚本，用于创建应用程序的安装程序

#define MyAppName "YUView"
#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif
#define MyAppPublisher "YUView Development Team"
#define MyAppURL "https://gitee.com/vance_wu/YUView"

[Setup]
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=..\release
OutputBaseFilename={#MyAppName}_Setup_v{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
LicenseFile=..\LICENSE.GPL3
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppName}.exe
CreateUninstallRegKey=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimp"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\release\v{#MyAppVersion}\{#MyAppName}.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\release\v{#MyAppVersion}\LICENSE.GPL3"; DestDir: "{app}"; Flags: ignoreversion

[Dirs]
Name: "{app}\logs"; Permissions: everyone-full

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppName}.exe"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppName}.exe"; Tasks: desktopicon
Name: "{userstartmenu}\{#MyAppName}"; Filename: "{app}\{#MyAppName}.exe"

[Registry]
Root: HKCU; Subkey: "Software\{#MyAppName}"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"; Flags: uninsdeletevalue

[Run]
Filename: "{app}\{#MyAppName}.exe"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\logs"