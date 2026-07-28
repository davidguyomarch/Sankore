; Open-Sankoré Inno Setup Script (Qt6)
; Builds the Windows installer from the windeployqt output

#define MyAppName "Open-Sankoré"
#define MyAppVersion "4.0.0"
#define MyAppPublisher "Open-Sankoré Project"
#define MyAppURL "https://github.com/davidguyomarch/Sankore"
#define MyAppExeName "Open-Sankore.exe"

[Setup]
AppId={{E63D17F8-D9DA-479D-B9B5-0D101A03703B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={autopf}\Open-Sankore
DefaultGroupName={#MyAppName}
OutputDir=..\build\installer
OutputBaseFilename=Open-Sankore-{#MyAppVersion}-setup
SetupIconFile=..\resources\win\uniboard.ico
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "fileassoc"; Description: "Associate .ubz files with Open-Sankoré"; GroupDescription: "File associations:"

[Files]
; All files from the windeployqt output directory
Source: "..\build\win32\release\product\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; .ubz file association
Root: HKCR; Subkey: ".ubz"; ValueType: string; ValueName: ""; ValueData: "SankoreFile"; Flags: uninsdeletevalue; Tasks: fileassoc
Root: HKCR; Subkey: "SankoreFile"; ValueType: string; ValueName: ""; ValueData: "Open-Sankoré document"; Flags: uninsdeletekey; Tasks: fileassoc
Root: HKCR; Subkey: "SankoreFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},1"; Tasks: fileassoc
Root: HKCR; Subkey: "SankoreFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: fileassoc

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Name: {app}; Type: filesandordirs
