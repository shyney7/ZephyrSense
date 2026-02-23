# ZephyrSense Installer Guide

## Overview

This directory contains the Inno Setup installer configuration for creating a professional Windows offline installer for ZephyrSense v0.1.

## Directory Structure

```
installer/
├── installer.iss          # Main Inno Setup script
├── license.txt            # GPL v2 license (displayed during installation)
├── README.txt             # Installation guide (displayed before installation)
├── Output/                # Compiled installers are placed here
│   └── ZephyrSense-Setup-0.1.0.exe
└── INSTALLER_GUIDE.md     # This file
```

## Prerequisites

### 1. Inno Setup 6
Download and install from: https://jrsoftware.org/isdl.php

**Installation path:** `C:\Program Files (x86)\Inno Setup 6\`

### 2. Release Build
Build the Release version of ZephyrSense:

```bash
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

**Output:** `build/Release/appZephyrSense.exe` (617 KB)

### 3. Qt Deployment
Deploy Qt dependencies using windeployqt:

```bash
C:\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe --release --qmldir qml\ build\Release\appZephyrSense.exe
```

**Result:** ~122 Qt DLLs and plugin directories in `build/Release/`

### 4. VC++ Runtime (Optional but Recommended)
Download vc_redist.x64.exe from:
https://aka.ms/vs/17/release/vc_redist.x64.exe

**Place in:** `build/Release/vc_redist.x64.exe`

If not present, the installer will still build, but users will need to install VC++ Runtime manually.

## Building the Installer

### Automated Build (Recommended)

Run the batch script from the project root:

```batch
build_installer.bat
```

This script will:
1. Check for Release build (`appZephyrSense.exe`)
2. Verify Qt deployment (`Qt6Core.dll` and others)
3. Check for VC++ Runtime (`vc_redist.x64.exe`)
4. Verify Inno Setup installation
5. Compile the installer using Inno Setup

**Output:** `installer/Output/ZephyrSense-Setup-0.1.0.exe` (~100-120 MB compressed)

### Manual Build

If you prefer to build manually:

```batch
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\installer.iss
```

Or open `installer.iss` in Inno Setup IDE and press F9 to compile.

## Installer Features

### Included Components
- **Application executable:** `appZephyrSense.exe` (617 KB)
- **Qt 6.10.1 Runtime:** ~122 DLLs (Qt6Core, Qt6Gui, Qt6Qml, Qt6Quick, etc.)
- **Qt Plugins:**
  - `platforms/` - Windows platform integration
  - `sqldrivers/` - SQLite database driver
  - `imageformats/` - Image loading (PNG, JPG, etc.)
  - `iconengines/` - Icon rendering
  - `geoservices/` - Map tile providers
  - `position/` - GPS/positioning
  - `networkinformation/` - Network status
  - `tls/` - SSL/TLS support
  - `styles/` - UI styles
  - `qmltooling/` - QML debugging tools
- **QML Modules:**
  - `qml/QtQuick/`
  - `qml/QtQuick/Controls/`
  - `qml/QtGraphs/`
  - `qml/QtLocation/`
  - And more...
- **DirectX Shader Compiler:** `dxcompiler.dll`, `dxil.dll`, `d3dcompiler_47.dll`
- **OpenGL Software Renderer:** `opengl32sw.dll` (20 MB, fallback renderer)
- **VC++ 2022 Runtime:** `vc_redist.x64.exe` (installed automatically)
- **Documentation:** `README.txt`, `license.txt`

### Installation Process
1. **Welcome screen** with application info
2. **License agreement** (GPL v2) - must be accepted
3. **Pre-installation information** (README.txt with system requirements)
4. **Destination folder** selection (default: `C:\Program Files\ZephyrSense\`)
5. **Start Menu folder** selection (default: `ZephyrSense`)
6. **Desktop shortcut** option (unchecked by default)
7. **Installation progress** with VC++ Runtime installation
8. **Completion** with optional "Launch ZephyrSense" checkbox

### Created Shortcuts
- **Start Menu:** `ZephyrSense` → `C:\Program Files\ZephyrSense\appZephyrSense.exe`
- **Start Menu:** `Uninstall ZephyrSense` → Uninstaller
- **Desktop** (optional): `ZephyrSense` → `C:\Program Files\ZephyrSense\appZephyrSense.exe`

### Uninstaller
- Removes all files from `C:\Program Files\ZephyrSense\`
- Removes Start Menu shortcuts
- Removes Desktop shortcut (if created)
- **Preserves user data** in `%APPDATA%\ZephyrSense\`:
  - `ZephyrSense.ini` (settings)
  - `ZephyrSense.db` (SQLite database)

## Testing Checklist

### Pre-Installation Tests
- [ ] Verify Release build exists: `build\Release\appZephyrSense.exe`
- [ ] Verify Qt DLLs deployed: `build\Release\Qt6Core.dll`, `Qt6Gui.dll`, etc.
- [ ] Verify plugins deployed: `build\Release\platforms\`, `sqldrivers\`, etc.
- [ ] Verify QML modules: `build\Release\qml\QtQuick\`, `QtGraphs\`, etc.
- [ ] Verify VC++ Runtime: `build\Release\vc_redist.x64.exe`
- [ ] Run `build_installer.bat` successfully
- [ ] Verify output file created: `installer\Output\ZephyrSense-Setup-0.1.0.exe`
- [ ] Check installer size: 100-120 MB compressed

### Installation Tests (Clean Windows 11 VM or fresh user account)
- [ ] Run installer as administrator
- [ ] Accept GPL v2 license
- [ ] Review README information
- [ ] Install to default location
- [ ] Choose to create desktop shortcut
- [ ] Wait for installation to complete (~1-2 minutes)
- [ ] Verify VC++ Runtime installed silently (no user prompt)
- [ ] Verify Start Menu shortcuts created
- [ ] Verify Desktop shortcut created (if selected)
- [ ] Launch application from Start Menu
- [ ] Verify no "missing DLL" errors
- [ ] Verify application window appears

### Application Tests
- [ ] Serial port enumeration works (Settings > Connection)
- [ ] Map view displays correctly
- [ ] Database initializes (check `%APPDATA%\ZephyrSense\ZephyrSense.db`)
- [ ] All four views accessible: Map, Dashboard, Graphs, Settings
- [ ] Settings persist after closing/reopening (check `%APPDATA%\ZephyrSense\ZephyrSense.ini`)

### Uninstallation Tests
- [ ] Run uninstaller from Start Menu or Control Panel
- [ ] Confirm uninstallation
- [ ] Verify all files removed from `C:\Program Files\ZephyrSense\`
- [ ] Verify Start Menu shortcuts removed
- [ ] Verify Desktop shortcut removed
- [ ] Verify user data preserved in `%APPDATA%\ZephyrSense\`
  - [ ] `ZephyrSense.ini` still exists
  - [ ] `ZephyrSense.db` still exists

### Upgrade Tests (Future Versions)
- [ ] Install v0.1.0
- [ ] Create some test data (database entries, settings changes)
- [ ] Install v0.2.0 over existing installation
- [ ] Verify upgrade successful
- [ ] Verify settings preserved
- [ ] Verify database preserved

## Troubleshooting

### Problem: "ISCC.exe not found"
**Solution:** Install Inno Setup 6 from https://jrsoftware.org/isdl.php
Ensure it's installed to `C:\Program Files (x86)\Inno Setup 6\`

### Problem: "Source file not found" errors during compilation
**Cause:** Missing Qt deployment or incorrect paths in `installer.iss`
**Solution:**
1. Verify `build/Release/` contains all DLLs and folders
2. Run windeployqt again: `C:\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe --release --qmldir qml\ build\Release\appZephyrSense.exe`
3. Check that `installer.iss` paths use relative paths: `..\build\Release\...`

### Problem: Installer compiles but application won't start on test system
**Cause:** Missing DLLs not included in installer
**Solution:**
1. Run the application on the test system
2. Note which DLL is missing from the error message
3. Add the missing DLL path to `[Files]` section in `installer.iss`
4. Recompile installer

### Problem: VC++ Runtime not installing
**Cause:** `vc_redist.x64.exe` missing from `build/Release/`
**Solution:** Download from https://aka.ms/vs/17/release/vc_redist.x64.exe
Place in `build/Release/` and rebuild installer

### Problem: Installer size too large (>150 MB)
**Cause:** LZMA2 compression not working or debug symbols included
**Solution:**
1. Verify `Compression=lzma2/max` in `[Setup]` section
2. Verify `SolidCompression=yes` in `[Setup]` section
3. Ensure Release build (not Debug) is being packaged
4. Check for `.pdb` debug symbol files and exclude them

## Customization

### Changing Version Number
Edit three locations:
1. **CMakeLists.txt** line 3: `project(ZephyrSense VERSION 0.1 ...)`
2. **installer.iss** line 8: `AppVersion=0.1.0`
3. **installer.iss** line 20: `OutputBaseFilename=ZephyrSense-Setup-0.1.0`

### Adding New Files
Add `Source:` entries to `[Files]` section in `installer.iss`:

```ini
Source: "..\path\to\newfile.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\path\to\newfolder\*"; DestDir: "{app}\newfolder"; Flags: ignoreversion recursesubdirs
```

### Changing Publisher/URLs
Edit `[Setup]` section in `installer.iss`:
- `AppPublisher=` - Company/organization name
- `AppPublisherURL=` - Website URL
- `AppSupportURL=` - Support/contact URL
- `AppUpdatesURL=` - Download page for updates

### Adding Custom Installer Icon
1. Create a 256x256 ICO file: `installer/icon.ico`
2. Edit `installer.iss` line 26: `SetupIconFile=icon.ico`
3. Rebuild installer

### Multi-Language Support
Add more `[Languages]` entries in `installer.iss`:

```ini
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
```

## Distribution

### Recommended Distribution Methods
1. **GitHub Releases:** Upload to https://github.com/yourorg/zephyrsense/releases
2. **Direct Download:** Host on website with HTTPS
3. **File Sharing:** Google Drive, Dropbox, OneDrive (for testing)

### Security Warning
Without code signing, users will see:
- **Windows SmartScreen:** "Unknown publisher" warning
- **Antivirus:** Some may flag unsigned executables

**To bypass SmartScreen:**
1. User clicks "More info" link
2. User clicks "Run anyway" button

**For production distribution:** Consider purchasing a code signing certificate ($150-500/year)
- Authenticode signing removes SmartScreen warnings
- Increases user trust
- Required for Windows Enterprise environments

## Future Enhancements

### v0.2+ Roadmap
- [ ] Code signing with Authenticode certificate
- [ ] Auto-update mechanism (check for updates at startup)
- [ ] Silent installation mode for enterprise deployment
- [ ] Multi-language installers (German, French, Spanish)
- [ ] Custom installer theme/branding
- [ ] CMake target: `ninja installer` to build installer automatically
- [ ] Installer bundled with MSVC redistributables for offline use
- [ ] Delta updates (only download changed files)

## Version History

### v0.1.0 (2026-01-28)
- Initial installer implementation
- Inno Setup 6 configuration
- GPL v2 license screen
- README pre-installation info
- VC++ Runtime auto-installation
- Start Menu + optional Desktop shortcut
- Complete Qt 6.10.1 deployment
- Compressed installer: ~100-120 MB
- Automated build script: `build_installer.bat`

## License

The installer script (`installer.iss`) is part of ZephyrSense and licensed under GNU GPL v2.

Inno Setup is a separate tool by Jordan Russell, licensed under its own terms.
See: https://jrsoftware.org/files/is/license.txt

## Support

For installer-related issues:
- Check this guide's Troubleshooting section
- Review Inno Setup documentation: https://jrsoftware.org/ishelp/
- Ask on Inno Setup forums: https://groups.google.com/g/innosetup

For ZephyrSense application issues:
- GitHub Issues: https://github.com/yourorg/zephyrsense/issues
- Email: support@zephyrsense.example.com
