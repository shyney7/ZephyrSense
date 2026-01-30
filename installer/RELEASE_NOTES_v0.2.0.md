# ZephyrSense v0.2.0 - Performance & QML Improvements

**Release Date:** 2026-01-30
**Installer Size:** ~56 MB
**SHA256:** `26de4ef917d6fd7cf6215d403e4ad4f7c0b4ab57a10478959b73790d5b10998c`

## Download

[ZephyrSense-Setup-0.2.0.exe](https://github.com/shyney7/ZephyrSense/releases/download/v0.2.0/ZephyrSense-Setup-0.2.0.exe) (~56 MB)

## What's New in v0.2.0

This release focuses on performance improvements, stability fixes, and Qt 6 QML best practices compliance.

### 🚀 Performance Improvements

- **Settings View Freeze Fix:** Resolved UI freeze when switching tabs in Settings view by implementing lazy tab instantiation using Loader components
- **Gauge Color Logic Moved to C++:** Migrated color calculation logic from QML/JavaScript to C++ (`ThresholdManager`) for significantly improved performance
  - New `getColorForSensor()` method for efficient color lookups
  - New `isSensorEnabledForKey()` method for optimized sensor state checks
- **RadialBarGauge Optimization:** Removed ~85 lines of JavaScript functions, now handled by C++ backend

### 🛠️ Code Quality Improvements

- **Strong Typing:** Replaced `property var` declarations with properly typed properties:
  - `date` type for date properties
  - `list<string>` for string lists
  - `TimeSeriesChartModel` for chart model references
- **Function Type Annotations:** Added explicit parameter and return type annotations to all QML functions
- **Component Behavior:** Added `pragma ComponentBehavior: Bound` and `required` properties to delegates in:
  - NavigationDrawer
  - SensorLegend
- **Cleanup:** Removed unnecessary `clip: true` properties from NavigationDrawer and DisplayTab

### 📁 Files Changed

| Component | Changes |
|-----------|---------|
| Main.qml | Minor updates |
| DateTimePicker.qml | Type annotations |
| DisplayTab.qml | Removed unnecessary clip |
| NavigationDrawer.qml | ComponentBehavior + required properties |
| RadialBarGauge.qml | Major refactor - JS logic removed |
| SensorLegend.qml | ComponentBehavior + required properties |
| TimeSeriesChart.qml | Type improvements |
| DashboardView.qml | Strong typing |
| GraphsView.qml | Strong typing |
| MapView.qml | Strong typing |
| SettingsView.qml | Lazy tab loading via Loader |
| thresholdmanager.cpp | New color/sensor methods |
| thresholdmanager.h | New method declarations |

## Upgrade Instructions

### From v0.1.0

1. **Download** the v0.2.0 installer
2. **Run** the installer - it will upgrade the existing installation
3. **Your settings and database are preserved** in `%APPDATA%\ZephyrSense\`

### Fresh Install

1. **Download** the installer from the link above
2. **Run** `ZephyrSense-Setup-0.2.0.exe`
3. **Windows SmartScreen Warning:** If you see "Windows protected your PC", click **"More info"** → **"Run anyway"**
   - This warning appears because the installer is not code-signed
4. **Accept** the GPL v2 license agreement
5. **Choose** installation location (default: `C:\Program Files\ZephyrSense\`)
6. **Wait** for installation to complete (~1-2 minutes)
7. **Launch** ZephyrSense from the Start Menu or Desktop shortcut

## System Requirements

### Minimum Requirements
- **Operating System:** Windows 10 (64-bit) or Windows 11
- **Processor:** Intel Core i3 or equivalent (dual-core)
- **RAM:** 4 GB
- **Disk Space:** 250 MB
- **Display:** 1280x720 resolution
- **Serial Port:** USB serial adapter or built-in COM port

### Recommended Requirements
- **Operating System:** Windows 11 (64-bit)
- **Processor:** Intel Core i5 or equivalent (quad-core)
- **RAM:** 8 GB
- **Disk Space:** 500 MB
- **Display:** 1920x1080 resolution
- **Internet:** For map tile downloads

## Known Issues

### Windows SmartScreen Warning
- **Issue:** Installer shows "Windows protected your PC" warning
- **Cause:** Installer is not code-signed
- **Workaround:** Click "More info" → "Run anyway"

### No Auto-Update
- **Issue:** Application does not check for updates automatically
- **Workaround:** Check GitHub releases page manually

## Technical Details

### Built With
- **Qt Framework:** 6.10.1 (MSVC2022 x64)
- **Build System:** CMake 3.16+ with Ninja
- **Database:** SQLite (via Qt SQL)
- **Maps:** Qt Location with OpenStreetMap tiles
- **Language:** C++17 with QML UI

### Installer Details
- **Tool:** Inno Setup 6.7.0
- **Compression:** LZMA2/max with solid compression

## Changelog

### v0.2.0 (2026-01-30) - Performance & QML Improvements
- 🚀 Fixed Settings view freeze with lazy tab loading
- 🚀 Moved gauge color logic to C++ for better performance
- 🛠️ Added strong typing to QML properties
- 🛠️ Added function type annotations throughout
- 🛠️ Implemented ComponentBehavior: Bound for delegates
- 🛠️ Removed ~85 lines of JavaScript from RadialBarGauge
- 🧹 Cleaned up unnecessary clip properties

### v0.1.0 (2026-01-28) - Initial Release
- ✨ Serial port data acquisition (115200 baud)
- ✨ Real-time map visualization with GPS positioning
- ✨ Radial gauge dashboard for all sensors
- ✨ Time-series graph view with customizable series
- ✨ SQLite database for historical data storage
- ✨ CSV export with date range selection
- ✨ Configurable sensor thresholds (warning/critical)
- ✨ Four main views: Map, Dashboard, Graphs, Settings
- ✨ Professional Windows installer with VC++ Runtime
- ✨ Theme support (light/dark modes)

## License

ZephyrSense is free software licensed under the **GNU General Public License v2**.

See the LICENSE file or https://www.gnu.org/licenses/gpl-2.0.html for full terms.

## Source Code

Full source code is available on GitHub:
https://github.com/shyney7/ZephyrSense

## Support

### Bug Reports & Feature Requests
GitHub Issues: https://github.com/shyney7/ZephyrSense/issues

### Contact
- GitHub: [@shyney7](https://github.com/shyney7)

---

**Thank you for using ZephyrSense!**

For questions, issues, or contributions, please visit our GitHub repository.
