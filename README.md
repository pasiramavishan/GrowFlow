# GrowFlow

## GrowFlow Mobile App 🌿

GrowFlow is a mobile application designed to help users monitor and control their IoT-enabled plant devices. The app supports both real-time monitoring and device control, making plant care smarter and easier.

### 🌟 Key Features

- **Real-time device control:** Connects to multiple IoT devices via MQTT, allowing users to control and monitor all devices from the app.  
- **Automatic and manual modes:** Users can switch between automatic watering (based on sensor data) and manual control to decide when to water plants.  
- **Live data updates:** Displays real-time readings such as temperature, humidity, and soil moisture from connected devices.  
- **Visual monitoring:** Sensor data is shown as live graphs for easy understanding of plant and environment conditions.  
- **Multi-device management:** Supports adding, viewing, and removing multiple devices with individual control panels.  
- **Clean, responsive UI:** Built with Flutter for smooth performance and mobile-friendly interface.  

### 📱 How It Works

- Devices send sensor data (temperature, humidity, soil moisture) via MQTT.  
- The app listens to the MQTT topics and updates the UI in real-time.  
- Users can toggle devices on/off manually or put them in automatic mode.  
- Each device has a dedicated control panel showing live data and visual graphs.  
