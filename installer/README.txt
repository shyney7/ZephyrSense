========================================
ZephyrSense v0.1.0 - Installation Guide
========================================

Thank you for installing ZephyrSense!

ABOUT
-----
ZephyrSense is a geospatial environmental sensor data visualization application
designed for drone-mounted measurement instruments. It receives real-time data
via serial port and displays sensor readings on an interactive map.

SYSTEM REQUIREMENTS
-------------------
- Operating System: Windows 11 or Windows 10 (64-bit)
- Processor: Intel Core i3 or equivalent (dual-core minimum)
- RAM: 4 GB minimum, 8 GB recommended
- Disk Space: 250 MB for application and data storage
- Display: 1280x720 minimum resolution
- Serial Port: USB serial adapter or built-in COM port for sensor connection

INSTALLATION
------------
The installer will:
1. Install ZephyrSense application files to Program Files
2. Install Visual C++ 2022 Runtime (if not already present)
3. Create Start Menu shortcuts
4. Optionally create a Desktop shortcut

The installation requires administrator privileges.

GETTING STARTED
---------------
After installation:

1. Launch ZephyrSense from the Start Menu or Desktop shortcut

2. Configure Serial Connection:
   - Click the hamburger menu (top-left)
   - Navigate to Settings > Connection tab
   - Select your COM port from the dropdown
   - Set baud rate to 115200 (default for drone sensors)
   - Click "Connect"

3. View Live Data:
   - Map View: See sensor readings plotted on an interactive map
   - Dashboard: Monitor all sensor values in real-time with radial gauges
   - Graphs: Visualize time-series data and trends
   - Settings: Configure thresholds, export data to CSV

SERIAL PORT SETUP
-----------------
ZephyrSense expects data via serial port at 115200 baud, 8N1 format.

Supported data format:
- Binary data frames delimited by '<' and '>' characters
- Includes: GPS coordinates, temperature, humidity, pressure, altitude,
  particulate matter (Partector, Grimm), CO2 concentration

For detailed data structure, refer to the project documentation.

DATA STORAGE
------------
ZephyrSense stores its data in your user profile:
- Settings: %APPDATA%\ZephyrSense\ZephyrSense.ini
- Database: %APPDATA%\ZephyrSense\ZephyrSense.db

These files are preserved during application upgrades and are not removed
during uninstallation.

TROUBLESHOOTING
---------------
Problem: "Missing DLL" errors on startup
Solution: Ensure Visual C++ 2022 Runtime is installed (installer handles this)

Problem: Serial port not detected
Solution:
  - Check device manager for COM port availability
  - Install USB serial driver (CH340, FTDI, etc.) if needed
  - Try unplugging and reconnecting the USB adapter

Problem: Map tiles not loading
Solution: Ensure internet connection is active (map tiles require online access)

Problem: Application won't start
Solution:
  - Check Windows Event Viewer for error details
  - Verify system meets minimum requirements
  - Try reinstalling the application

UNINSTALLATION
--------------
To remove ZephyrSense:
1. Open Windows Settings > Apps > Installed apps
2. Search for "ZephyrSense"
3. Click the three dots and select "Uninstall"
   OR
   Run the uninstaller from Start Menu > ZephyrSense > Uninstall ZephyrSense

Note: User settings and database files in %APPDATA%\ZephyrSense\ are preserved.
To completely remove all data, manually delete this folder after uninstallation.

LICENSE
-------
ZephyrSense is free software licensed under the GNU General Public License v2.
See the license agreement for full details.

SUPPORT
-------
For bug reports, feature requests, or questions:
- GitHub: https://github.com/yourorg/zephyrsense
- Email: support@zephyrsense.example.com

VERSION HISTORY
---------------
v0.1.0 (2026-01-28)
- Initial release
- Serial port data acquisition
- Real-time map visualization
- Radial gauge dashboard
- Time-series graphs
- SQLite data storage
- CSV export functionality
- Configurable sensor thresholds

========================================
