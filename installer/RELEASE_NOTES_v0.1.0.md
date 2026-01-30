# ZephyrSense v0.1.0 - Initial Release

**Release Date:** 2026-01-28
**Installer Size:** 56 MB
**SHA256:** `9831752fac526491883bb25bc7d77d7a367ba57d9c5aa50791116b45e88c2ff9`

## Download

[ZephyrSense-Setup-0.1.0.exe](https://github.com/shyney7/ZephyrSense/releases/download/v0.1.0/ZephyrSense-Setup-0.1.0.exe) (56 MB)

## Installation Instructions

1. **Download** the installer from the link above
2. **Run** `ZephyrSense-Setup-0.1.0.exe`
3. **Windows SmartScreen Warning:** If you see "Windows protected your PC", click **"More info"** → **"Run anyway"**
   - This warning appears because the installer is not code-signed (planned for v0.2)
4. **Accept** the GPL v2 license agreement
5. **Review** the pre-installation information
6. **Choose** installation location (default: `C:\Program Files\ZephyrSense\`)
7. **Optional:** Check "Create a desktop shortcut"
8. **Wait** for installation to complete (~1-2 minutes)
   - The installer will automatically install Visual C++ 2022 Runtime if needed
9. **Launch** ZephyrSense from the Start Menu or Desktop shortcut

## What's Included

### Core Features
- **Serial Port Communication** - Connect to drone-mounted sensors via COM port (115200 baud)
- **Real-Time Map View** - Display sensor readings on interactive map with GPS positioning
- **Dashboard View** - Monitor all sensor values with radial gauge displays
- **Time-Series Graphs** - Visualize historical data with customizable chart series
- **Data Persistence** - SQLite database stores all readings automatically
- **CSV Export** - Export selected data ranges to CSV files
- **Threshold Configuration** - Set warning/critical thresholds for all sensors

### Supported Sensors
- **Particulate Matter:** Partector (number, diameter, mass), Grimm counter
- **Environmental:** Temperature, humidity, pressure, altitude
- **Positioning:** GPS (latitude, longitude)
- **Air Quality:** CO2 concentration

### Four Main Views
1. **Map View** - Geospatial visualization with measurement points
   - Color-coded markers based on thresholds (green/yellow/red)
   - Click markers to view sensor details
   - Time-series graph overlay

2. **Dashboard** - Real-time sensor monitoring
   - RadialBar gauges for all sensors
   - Current/min/max/average values
   - Status indicators

3. **Graphs** - Custom time-series visualization
   - Selectable data series
   - Zoom and pan controls
   - Export to image

4. **Settings** - Configuration and data management
   - Serial port selection and baud rate
   - Threshold configuration
   - CSV export with date range selection
   - Display preferences

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

## Getting Started

### 1. Connect Your Sensor
- Connect your drone sensor unit to a USB serial port
- Note the COM port number (check Device Manager if needed)
- Ensure sensor is transmitting data at 115200 baud

### 2. Configure Serial Connection
1. Launch ZephyrSense
2. Click the hamburger menu (☰) in the top-left corner
3. Navigate to **Settings** → **Connection** tab
4. Select your **COM port** from the dropdown
5. Verify **baud rate** is 115200 (default)
6. Click **"Connect"**
7. Status should change to "Connected"

### 3. View Live Data
- Switch to **Map View** to see sensor readings plotted on map
- Switch to **Dashboard** to monitor all values in real-time
- Switch to **Graphs** to analyze time-series trends

### 4. Export Data
1. Go to **Settings** → **Export** tab
2. Select date/time range
3. Click **"Export CSV"**
4. Choose save location

## Data Storage

ZephyrSense stores its data in your user profile:

- **Settings:** `%APPDATA%\ZephyrSense\ZephyrSense.ini`
- **Database:** `%APPDATA%\ZephyrSense\ZephyrSense.db`

These files are preserved during:
- Application upgrades
- Uninstallation (by default)

To completely remove all data, manually delete `%APPDATA%\ZephyrSense\` after uninstallation.

## Uninstallation

To remove ZephyrSense:

**Option 1: Settings**
1. Open Windows Settings → Apps → Installed apps
2. Search for "ZephyrSense"
3. Click the three dots → "Uninstall"

**Option 2: Start Menu**
1. Open Start Menu
2. Find "ZephyrSense" folder
3. Click "Uninstall ZephyrSense"

**Option 3: Control Panel**
1. Control Panel → Programs → Programs and Features
2. Select "ZephyrSense"
3. Click "Uninstall"

User data in `%APPDATA%\ZephyrSense\` is preserved by default.

## Known Issues

### Windows SmartScreen Warning
- **Issue:** Installer shows "Windows protected your PC" warning
- **Cause:** Installer is not code-signed (certificate cost: $150-500/year)
- **Workaround:** Click "More info" → "Run anyway"
- **Planned Fix:** Code signing in v0.2

### No Auto-Update
- **Issue:** Application does not check for updates automatically
- **Workaround:** Check GitHub releases page manually
- **Planned Fix:** Auto-update checker in v0.2

## Troubleshooting

### "Missing DLL" Error on Startup
- **Solution:** Ensure Visual C++ 2022 Runtime is installed
- The installer should handle this automatically
- If needed, download manually: https://aka.ms/vs/17/release/vc_redist.x64.exe

### Serial Port Not Detected
- **Check:** Device Manager → Ports (COM & LPT)
- **Solution:** Install USB serial driver (CH340, FTDI, etc.)
- **Try:** Unplug and reconnect the USB adapter

### Map Tiles Not Loading
- **Check:** Internet connection is active
- **Reason:** Map tiles require online access (OpenStreetMap)
- **Solution:** Verify firewall is not blocking ZephyrSense

### Application Won't Start
- **Check:** Windows Event Viewer for error details
- **Verify:** System meets minimum requirements
- **Try:** Reinstall the application

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
- **Source Size:** 157 MB (deployed Qt + application)
- **Compressed Size:** 56 MB (64% reduction)
- **Installation Time:** 1-2 minutes

## License

ZephyrSense is free software licensed under the **GNU General Public License v2**.

You are free to:
- Use the software for any purpose
- Study and modify the source code
- Distribute copies
- Distribute modified versions

See the LICENSE file or https://www.gnu.org/licenses/gpl-2.0.html for full terms.

## Source Code

Full source code is available on GitHub:
https://github.com/shyney7/ZephyrSense

## Support

### Bug Reports & Feature Requests
GitHub Issues: https://github.com/shyney7/ZephyrSense/issues

### Documentation
- Installation Guide: See `installer/README.txt` in source
- User Manual: (Coming in v0.2)
- Developer Guide: See `CLAUDE.md` in source

### Contact
- GitHub: [@shyney7](https://github.com/shyney7)
- Email: support@zephyrsense.example.com

## Roadmap

### v0.2 (Planned)
- [ ] Code-signed installer (no SmartScreen warning)
- [ ] Auto-update checker
- [ ] Custom installer icon
- [ ] User manual
- [ ] Video tutorials

### v0.3 (Future)
- [ ] Multi-language support (German, French, Spanish)
- [ ] Custom map tile providers
- [ ] Advanced graph customization
- [ ] Real-time alerts and notifications

## Changelog

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

---

**Thank you for using ZephyrSense!**

For questions, issues, or contributions, please visit our GitHub repository.
