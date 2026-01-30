; ZephyrSense v0.2.0 Inno Setup Installer Script
; Generated: 2026-01-30

[Setup]
; Application Identity
AppId={{B8E5F7C2-9A4D-4E1B-8F3C-2D6A5E7B9C1D}}
AppName=ZephyrSense
AppVersion=0.2.0
AppPublisher=Marcel Oliveira Brito
AppPublisherURL=https://github.com/shyney7/ZephyrSense
AppSupportURL=https://github.com/shyney7/ZephyrSense/issues
AppUpdatesURL=https://github.com/shyney7/ZephyrSense/releases
DefaultDirName={autopf}\ZephyrSense
DefaultGroupName=ZephyrSense
AllowNoIcons=yes

; License and Documentation
LicenseFile=license.txt
InfoBeforeFile=README.txt

; Output Configuration
OutputDir=Output
OutputBaseFilename=ZephyrSense-Setup-0.2.0
Compression=lzma2/max
SolidCompression=yes

; Installer UI
WizardStyle=modern
; SetupIconFile requires a .ico file - commented out for now
SetupIconFile=appico.ico

; Platform Requirements
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
MinVersion=10.0.10240

; Uninstaller
UninstallDisplayName=ZephyrSense
UninstallDisplayIcon={app}\appZephyrSense.exe
DisableProgramGroupPage=yes

; Version Information
VersionInfoVersion=0.2.0.0
VersionInfoCompany=ZephyrSense
VersionInfoDescription=ZephyrSense Installer
VersionInfoCopyright=Copyright (C) 2026 ZephyrSense
VersionInfoProductName=ZephyrSense
VersionInfoProductVersion=0.2.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main executable
Source: "..\build\Release\appZephyrSense.exe"; DestDir: "{app}"; Flags: ignoreversion

; All DLLs in root directory
Source: "..\build\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; QML modules and plugins (recursive subdirectories)
Source: "..\build\Release\qml\*"; DestDir: "{app}\qml"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\geoservices\*"; DestDir: "{app}\geoservices"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\position\*"; DestDir: "{app}\position"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\qmltooling\*"; DestDir: "{app}\qmltooling"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\Release\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs

; VC++ Runtime (temporary location for installation)
Source: "..\build\Release\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

; Documentation
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "license.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\ZephyrSense"; Filename: "{app}\appZephyrSense.exe"; Comment: "Geospatial environmental sensor data visualization"
Name: "{group}\Uninstall ZephyrSense"; Filename: "{uninstallexe}"
Name: "{autodesktop}\ZephyrSense"; Filename: "{app}\appZephyrSense.exe"; Tasks: desktopicon; Comment: "Geospatial environmental sensor data visualization"

[Run]
; Install VC++ Runtime silently if needed
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ 2022 Runtime..."; Flags: waituntilterminated

; Optional: Launch application after installation
Filename: "{app}\appZephyrSense.exe"; Description: "{cm:LaunchProgram,ZephyrSense}"; Flags: nowait postinstall skipifsilent

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  // Check if Visual Studio 2022 x64 runtime is already installed
  // This is optional - the [Run] section will install it if needed
  // Note: This check can be enhanced to detect existing installations
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Application data directory will be created automatically by the app
    // in %APPDATA%\ZephyrSense\ on first run
  end;
end;

function InitializeUninstall(): Boolean;
begin
  Result := True;
  // Ask user if they want to preserve application data
  if MsgBox('Do you want to keep your ZephyrSense settings and database?' + #13#10 +
            'These files are stored in %APPDATA%\ZephyrSense\' + #13#10#13#10 +
            'Choose "Yes" to keep your data (recommended for upgrades)' + #13#10 +
            'Choose "No" to completely remove all data',
            mbConfirmation, MB_YESNO) = IDNO then
  begin
    // User chose to delete app data
    MsgBox('Application data will remain in %APPDATA%\ZephyrSense\' + #13#10 +
           'You can manually delete this folder if desired.',
           mbInformation, MB_OK);
  end;
end;
