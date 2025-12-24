# GrowFlow

## 🌱 GrowNet ESP32 Firmware

### 📝 Overview
GrowNet is a multi-node ESP32 network designed for **real-time sensor monitoring** and **device control**.  
The system consists of a **main node** (bridge) and multiple **device nodes**, enabling communication via **ESP-NOW** while optionally connecting the main node to a Wi-Fi network (router or mobile hotspot).

---

### 🛠️ Firmware Architecture

#### 🌐 Main Node (Bridge Node)
The main node acts as the central hub with these responsibilities:
- 🔗 **Network Management:** Connects to a Wi-Fi network chosen by the user.
- 📡 **ESP-NOW Coordination:** Maintains communication with all ESP32 device nodes.
- 🔄 **Data Forwarding:** Relays sensor and control messages between device nodes and the router/hotspot.
- 🎚️ **Channel Management:** Dynamically sets the Wi-Fi channel to match connected peers.
- ⏱️ **Scheduling:** Periodically sends its own sensor data and relays data from other nodes.

#### 🤖 Device Nodes
Each device node functions as a sensor and actuator endpoint:
- 🌡️ **Sensor Monitoring:** Reads local sensor data (e.g., temperature, humidity, pH, battery/charge status).
- 🎛️ **Control Handling:** Receives control commands from the main node or peers for automatic or manual operation.
- 📡 **ESP-NOW Communication:** Sends sensor data to the main node and receives forwarded messages.
- 🔍 **Peer Discovery:** Scans and adds nearby ESP32 devices as peers automatically.
- 🔄 **Message Forwarding:** Forwards messages not intended for itself to other peers to maintain network-wide communication.

---

### ✨ Features
- 📡 **Multi-device Communication:** Direct ESP-NOW communication between nodes without Wi-Fi dependency.
- ⚙️ **Automatic/Manual Modes:** Users can control devices manually or allow automatic operation.
- 🤝 **Dynamic Peer Management:** Nodes automatically detect and manage peers.
- 🌐 **Bridge Functionality:** Main node connects to Wi-Fi and relays data for remote monitoring.
- 🔔 **Notifications:** Receive updates when device states (e.g., watering system) are toggled.

---

https://github.com/user-attachments/assets/2996a79c-5b50-44c8-99ba-38fa615fbfe3




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


<table>
  <tr>
    <td>
      <img src="https://github.com/user-attachments/assets/0bee2dfb-4968-4957-859a-258ad16bc611" width="400"/>
    </td>
    <td>
      <img src="https://github.com/user-attachments/assets/fbdcee73-0e72-474a-a1af-bcebed13cb8b" width="400"/>
    </td>
  </tr>
</table>


 

