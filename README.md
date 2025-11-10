# ESP32 GPS Tracker

## Getting Started

This firmware uses `ESP-IDF Version - v5.5`.  
You can use Docker if you don't want to install the framework natively.

- First, install Docker and run:

```bash
docker run --rm -it -v ${PWD}:/project --privileged -w /project espressif/idf:release-v5.5 idf.py clean build flash monitor -p /dev/<port>
```

- Run `idf.py menuconfig`. Make sure to update the following configs for your environment:
  - GPS_TRACKER_SNTP_TIME_SERVER
  - GPS_TRACKER_SNTP_TIME_ZONE
  - GPS_TRACKER_WIFI_SSID
  - GPS_TRACKER_WIFI_PASSWORD
  - GPS_TRACKER_MQTT_BROKER_URL
  - GPS_TRACKER_PAYLOAD_GEN_INTERVAL_MS

**The fastest and easiest way to test this firmware is to create a WiFi AP (Hotspot) with the following credentials:**

- SSID: "gps-tracker"
- PASS: "abcdefgh"

## Running the Cloud Platform

There is no actual "cloud" platform in this project.  
Instead, I wrote a Python program that you can run locally on your PC to view messages sent from the ESP32 in the console.  
The program also provides a dashboard based on "Python Dash".

Navigate to the `mqtt_tester` directory to run it. Make sure your system has the Python package manager `uv`.  
If not, install it with:

```bash
pip install uv
```

Then, inside the directory, run the local "cloud" platform with:

```bash
uv python3 main.py
```

If the ESP32 is running, you should see logs similar to the following:

```bash
Uninstalled 1 package in 11ms
Installed 33 packages in 68ms
/home/rick/Data/personal/repos/gps-tracker/mqtt_tester/main.py:63: DeprecationWarning: Callback API version 1 is deprecated, update to latest version
  client = mqtt.Client()
Dash is running on http://0.0.0.0:8050/

 * Serving Flask app 'main'
 * Debug mode: on
/home/rick/Data/personal/repos/gps-tracker/mqtt_tester/main.py:63: DeprecationWarning: Callback API version 1 is deprecated, update to latest version
  client = mqtt.Client()
Connected with result code 0
Connected with result code 0
{'id': 'ESP32_001', 'lat': '-41.986', 'lng': '69.926', 'bat': '6.667', 'date': '2025-11-10', 'time': '21:46:55'}
{'id': 'ESP32_001', 'lat': '-41.986', 'lng': '69.926', 'bat': '6.667', 'date': '2025-11-10', 'time': '21:46:55'}
{'id': 'ESP32_001', 'lat': '48.224', 'lng': '165.421', 'bat': '70.196', 'date': '2025-11-10', 'time': '21:46:56'}
{'id': 'ESP32_001', 'lat': '48.224', 'lng': '165.421', 'bat': '70.196', 'date': '2025-11-10', 'time': '21:46:56'}
{'id': 'ESP32_001', 'lat': '-47.103', 'lng': '76.809', 'bat': '20.392', 'date': '2025-11-10', 'time': '21:46:57'}
{'id': 'ESP32_001', 'lat': '-47.103', 'lng': '76.809', 'bat': '20.392', 'date': '2025-11-10', 'time': '21:46:57'}
^]{'id': 'ESP32_001', 'lat': '-35.043', 'lng': '-97.31', 'bat': '76.471', 'date': '2025-11-10', 'time': '21:46:58'}
```

You can access the dashboard at `http://0.0.0.0:8050`.
