# ZephyrSense v0.4.0 - Qt Graphs Migration & Reliability Improvements

**Release Date:** 2026-02-23
**Installer Size:** ~XX MB
**SHA256:** `<to be filled after build>`

## Download

[ZephyrSense-Setup-0.4.0.exe](https://github.com/shyney7/ZephyrSense/releases/download/v0.4.0/ZephyrSense-Setup-0.4.0.exe) (~XX MB)

## What's New in v0.4.0

This release migrates the chart rendering engine from the deprecated Qt Charts module to Qt Graphs, fixes a reliability bug in floating dock detection, and eliminates debug logging overhead in Release builds.

### Qt Charts → Qt Graphs Migration

The `QtCharts` module has been replaced by `QtGraphs`, Qt's modern hardware-accelerated 2D graph engine built on Qt Quick Shapes.

**What changed:**

| Component | Old (Qt Charts) | New (Qt Graphs) |
|-----------|----------------|-----------------|
| Container | `ChartView` | `Item + GraphsView` |
| Data mapper | `VXYModelMapper` (inside series) | `XYModelMapper` (outside `GraphsView`) |
| Time axis format | `DateTimeAxis.format` | `DateTimeAxis.labelFormat` |
| Time axis ticks | `tickCount` | `tickInterval` |
| Value axis format | `ValueAxis.labelFormat` | `ValueAxis.labelDecimals` |
| Axis label fonts | Direct font properties | `GraphsTheme` properties |

No C++ changes were required — `TimeSeriesChartModel` is a pure `QAbstractTableModel` and remains unchanged.

### Fixed: Floating Dock Detection

**Problem (v0.3.x):** The `hasFloatingDocks` QML property used KDDockWidgets' `isFloating` boolean. When two or more sensor charts were detached and then grouped together into a single floating window, `isFloating` returned `false` for all of them (since they were no longer individually floating — they were tabbed inside a floating container). This caused the live update timer to incorrectly stop when the user switched to another view.

**Fix:** Replaced the QML-only approach with a new `DockStateTracker` C++ singleton that uses the reliable `Core::DockWidget::isInMainWindow()` API. The tracker:

- Monitors all 9 dock widgets via signal connections (`isFloatingChanged`, `isOpenChanged`, `windowChanged`)
- Uses a deferred timer to batch rapid state changes into a single evaluation
- Exposes a reactive `hasDocksOutsideMainWindow` bool property to QML
- `GraphsView.qml` now uses `DockStateTracker.hasDocksOutsideMainWindow` in the timer `running` condition

### Release Build Debug Suppression

Debug logging is now fully eliminated in Release builds:

- **C++ side:** `QT_NO_DEBUG_OUTPUT` compile definition removes all `qDebug()` calls at compile time (zero runtime overhead)
- **QML side:** `QLoggingCategory` filter rules suppress `console.log()` and `console.debug()` via the `qml.debug=false` and `js.debug=false` rules
- High-frequency per-reading `console.log` removed from `Main.qml`

### Files Changed

| File | Change |
|------|--------|
| `qml/components/SensorDockChart.qml` | Migrated from `ChartView` + `VXYModelMapper` to `Item` + `GraphsView` + `XYModelMapper` |
| `qml/components/TimeSeriesChart.qml` | Same migration as SensorDockChart |
| `qml/views/GraphsView.qml` | Replaced `hasFloatingDocks` QML property with `DockStateTracker.hasDocksOutsideMainWindow` |
| `src/core/dockstatetracker.h` | New `DockStateTracker` QML singleton (new file) |
| `src/core/dockstatetracker.cpp` | New `DockStateTracker` implementation (new file) |
| `main.cpp` | Added `QLoggingCategory` filter for Release builds |
| `CMakeLists.txt` | `Qt6::Charts` → `Qt6::Graphs`, added `QT_NO_DEBUG_OUTPUT` for Release |
| `Main.qml` | Removed per-reading `console.log` debug output |

## Upgrade Instructions

### From v0.3.x

1. **Download** the v0.4.0 installer
2. **Run** the installer -- it will upgrade the existing installation
3. **Your settings, database, and dock layouts are preserved**

### From v0.2.0 or v0.1.0

1. **Download** the v0.4.0 installer
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
- **Qt Framework:** 6.10.2 (MSVC2022 x64)
- **Build System:** CMake 3.16+ with Ninja
- **Database:** SQLite (via Qt SQL) with WAL mode
- **Maps:** Qt Location with OpenStreetMap tiles
- **Charts:** Qt Graphs 6.10.2 (hardware-accelerated 2D)
- **Docking:** KDDockWidgets 2.4.0 (QtQuick frontend)
- **Language:** C++17 with QML UI

### Installer Details
- **Tool:** Inno Setup 6.7.0
- **Compression:** LZMA2/max with solid compression

## Changelog

### v0.4.0 (2026-02-23) - Qt Graphs Migration & Reliability Improvements
- Migrated chart rendering from deprecated Qt Charts to Qt Graphs (hardware-accelerated)
- Fixed floating dock detection: DockStateTracker uses `isInMainWindow()` instead of `isFloating`
- Fixed: multiple docks grouped in a shared floating window now correctly keep live updates running
- Release builds suppress all `qDebug()` at compile time via `QT_NO_DEBUG_OUTPUT`
- Release builds suppress QML `console.log()` via `QLoggingCategory` filter rules
- Removed high-frequency per-reading debug output from Main.qml
- Updated Qt Framework from 6.10.1 to 6.10.2

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
