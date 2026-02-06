# ZephyrSense v0.3.0 - Dockable Graphs & Non-Blocking I/O

**Release Date:** 2026-02-06
**Installer Size:** ~XX MB
**SHA256:** `<to be filled after build>`

## Download

[ZephyrSense-Setup-0.3.0.exe](https://github.com/shyney7/ZephyrSense/releases/download/v0.3.0/ZephyrSense-Setup-0.3.0.exe) (~XX MB)

## What's New in v0.3.0

This release introduces dockable per-sensor graphs powered by KDDockWidgets, a dedicated I/O thread for non-blocking database and CSV writes, and improved serial frame parsing.

### Dockable Sensor Graphs

- **KDDockWidgets Integration:** Replaced the single chart + legend with 9 individually dockable sensor graphs using [KDDockWidgets 2.4.0](https://github.com/KDAB/KDDockWidgets) (QtQuick frontend)
- **Per-Sensor Charts:** Each sensor (PNC UFP, Diameter UFP, Mass UFP, PM0.3, PNC PM, Temperature, Humidity, Pressure, CO2) gets its own chart with independent Y-axis scaling via `getYBoundsForColumn()`
- **Flexible Layout:** Charts can be floated, tabbed, and arranged side-by-side within the Graphs view
- **New Component:** `SensorDockChart.qml` - self-contained chart component with required `chartModel` and `sensorColumn` properties

### Non-Blocking I/O Thread

- **Dedicated I/O Thread:** New `IOWorker`/`IOThread` classes handle database and CSV writes on a separate thread, preventing UI blocking during I/O operations
- **Qt::QueuedConnection:** Signal-slot connections across thread boundaries ensure thread-safe data transfer
- **Persistent CSV File:** CSV file stays open while export is enabled, eliminating per-reading open/close overhead
- **SQLite WAL Mode:** Enabled Write-Ahead Logging for better concurrent read/write performance

### Serial Frame Parser Fix

- **Binary Data Handling:** Fixed parser to correctly handle binary sensor data that contains delimiter bytes (`<` or `>`)
- **Position-Based Validation:** End delimiter is now checked at the expected position (start + 42-byte struct + 1) instead of searching the entire buffer

### UI Improvements

- **Sensor Renaming:** Clearer sensor names across all views (PNC UFP, Diameter UFP, PM0.3, PNC PM, CO2)
- **Unit Format:** Changed from `(unit)` to `[unit]` with proper Unicode symbols
- **Typography:** Bigger and bolder graph titles, axis labels, and dashboard gauge names
- **Editable Thresholds:** All threshold SpinBoxes now accept custom value entry
- **Reordered Thresholds:** Core sensors reordered in ThresholdsTab for consistency
- **StackLayout Navigation:** Main.qml switched from StackView to StackLayout to preserve dock state across view switches

### Files Changed

| Component | Changes |
|-----------|---------|
| CMakeLists.txt | KDDockWidgets integration, IOThread sources |
| main.cpp | KDDW init, IOThread setup, queued connections |
| Main.qml | StackView to StackLayout |
| GraphsView.qml | DockingArea with 9 DockWidget tabs |
| SensorDockChart.qml | New per-sensor chart component |
| RadialBarGauge.qml | Fix StackLayout resize (width:height to gaugeSize) |
| DashboardView.qml | Sensor label renaming |
| ThresholdsTab.qml | Reorder, editable SpinBoxes |
| SensorReadingModel | Sensor label renaming |
| TimeSeriesChartModel | New `getYBoundsForColumn()` method |
| IOThread / IOWorker | New I/O thread classes |
| SerialHandler | Binary frame parser fix |
| DatabaseManager | WAL mode, simplified (I/O moved to worker) |
| CsvExporter | Simplified (I/O moved to worker) |

## Upgrade Instructions

### From v0.2.0 or v0.1.0

1. **Download** the v0.3.0 installer
2. **Run** the installer - it will upgrade the existing installation
3. **Your settings and database are preserved** in `%APPDATA%\ZephyrSense\`

### Fresh Install

1. **Download** the installer from the link above
2. **Run** `ZephyrSense-Setup-0.3.0.exe`
3. **Windows SmartScreen Warning:** If you see "Windows protected your PC", click **"More info"** then **"Run anyway"**
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
