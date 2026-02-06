# ZephyrSense v0.3.1 - Timer Optimization for Dockable Graphs

**Release Date:** 2026-02-06
**Installer Size:** ~XX MB
**SHA256:** `<to be filled after build>`

## Download

[ZephyrSense-Setup-0.3.1.exe](https://github.com/shyney7/ZephyrSense/releases/download/v0.3.1/ZephyrSense-Setup-0.3.1.exe) (~XX MB)

## What's New in v0.3.1

This patch release optimizes resource usage in the StackLayout-based navigation introduced in v0.3.0. View timers now pause when their view is hidden, while detached (floating) sensor graphs continue receiving live updates across view switches.

### Timer Visibility Guards

The switch from `StackView` to `StackLayout` in v0.3.0 kept all four views instantiated simultaneously. This meant live-update timers in MapView, DashboardView, and GraphsView continued running even when the user navigated to a different view, causing unnecessary database queries in the background.

**What changed:**

| View | Timer | Behavior |
|------|-------|----------|
| **MapView** | `liveUpdateTimer` (prune old readings) | Pauses when view is hidden |
| **DashboardView** | `updateTimer` (fetch latest reading) | Pauses when view is hidden |
| **GraphsView** | `liveUpdateTimer` (load live data) | Pauses when hidden **unless** any dock widget is floating |

### Smart Floating Dock Detection (GraphsView)

GraphsView uses a smarter condition than the other views because of its dockable architecture:

- New `hasFloatingDocks` property reactively tracks the `isFloating` state of all 9 dock widgets via KDDockWidgets signals
- When the user detaches a sensor chart to a floating window (e.g., to monitor CO2 while working on the map), the live update timer keeps running
- When all docks are re-docked and the user switches away, the timer pauses automatically
- No manual state management required -- fully declarative QML bindings

### Files Changed

| File | Change |
|------|--------|
| `qml/views/GraphsView.qml` | Added `hasFloatingDocks` property, timer visibility guard with floating dock override |
| `qml/views/MapView.qml` | Added `&& visible` to timer running condition |
| `qml/views/DashboardView.qml` | Added `&& visible` to timer running condition |

## Upgrade Instructions

### From v0.3.0

1. **Download** the v0.3.1 installer
2. **Run** the installer -- it will upgrade the existing installation
3. **Your settings, database, and dock layouts are preserved**

### From v0.2.0 or v0.1.0

1. **Download** the v0.3.1 installer
2. **Run** the installer -- it will upgrade the existing installation
3. **Your settings and database are preserved** in `%APPDATA%\ZephyrSense\`

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
- **Workaround:** Click "More info" then "Run anyway"

### No Auto-Update
- **Issue:** Application does not check for updates automatically
- **Workaround:** Check GitHub releases page manually

## Technical Details

### Built With
- **Qt Framework:** 6.10.1 (MSVC2022 x64)
- **Build System:** CMake 3.16+ with Ninja
- **Database:** SQLite (via Qt SQL) with WAL mode
- **Maps:** Qt Location with OpenStreetMap tiles
- **Docking:** KDDockWidgets 2.4.0 (QtQuick frontend)
- **Language:** C++17 with QML UI

### Installer Details
- **Tool:** Inno Setup 6.7.0
- **Compression:** LZMA2/max with solid compression

## Changelog

### v0.3.1 (2026-02-06) - Timer Optimization for Dockable Graphs
- Pause MapView and DashboardView timers when hidden (StackLayout optimization)
- GraphsView timer stays alive when any dock widget is floating
- Added reactive `hasFloatingDocks` property using KDDockWidgets `isFloating` signals
- Detached sensor charts keep updating while user works in another view

### v0.3.0 (2026-02-06) - Dockable Graphs & Non-Blocking I/O
- Dockable per-sensor graphs with KDDockWidgets 2.4.0
- 9 individual charts with independent Y-axis scaling
- Non-blocking I/O thread for database and CSV writes
- Fixed serial frame parser for binary data with delimiter bytes
- SQLite WAL mode for concurrent read/write performance
- Renamed sensors (PNC UFP, Diameter UFP, PM0.3, PNC PM, CO2)
- Unit format changed from (unit) to [unit] with Unicode symbols
- Bigger/bolder graph titles, axis labels, and gauge names
- Editable threshold SpinBoxes for custom value entry
- StackLayout navigation to preserve dock state

### v0.2.0 (2026-01-30) - Performance & QML Improvements
- Fixed Settings view freeze with lazy tab loading
- Moved gauge color logic to C++ for better performance
- Added strong typing to QML properties
- Added function type annotations throughout
- Implemented ComponentBehavior: Bound for delegates
- Removed ~85 lines of JavaScript from RadialBarGauge
- Cleaned up unnecessary clip properties

### v0.1.0 (2026-01-28) - Initial Release
- Serial port data acquisition (115200 baud)
- Real-time map visualization with GPS positioning
- Radial gauge dashboard for all sensors
- Time-series graph view with customizable series
- SQLite database for historical data storage
- CSV export with date range selection
- Configurable sensor thresholds (warning/critical)
- Four main views: Map, Dashboard, Graphs, Settings
- Professional Windows installer with VC++ Runtime
- Theme support (light/dark modes)

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
