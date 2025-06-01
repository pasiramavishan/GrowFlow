#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "DHT.h"

#define GLOBAL_NAME "ESP32Pro"
#define MAX_DEVICES 10
#define TIMEOUT 2000   // 10 seconds timeout for inactive devices

#define MOIS_PIN 32   // 34
#define DHTPIN 26 // DHT11 Data Pin // 26
#define DHTTYPE DHT11 // DHT11 sensor type

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT
bool sendOK = false;



#define DHTTYPE DHT11 // DHT11 sensor type

typedef struct message {
  char type[10];
  char device[20];
  char sender[20];
  // char sender[20];
  char isSelected[10];
  float temperature;
  float humidity;
  float moisture;
  // int charge;
} message;

typedef struct messageControl {
  char type[10];      // Add identifier for message type
  char device[20];
  char sender[20];
  char isSelected[10];
  char isManual[10];
  char isRain[10];
} messageControl;

message myData, receivedData;

// Structure to store peer information
typedef struct {
  uint8_t mac[6];
  int channel;
} PeerInfo;

PeerInfo peerMACs[10];  // Store up to 10 peer MAC addresses and channels
int peerCount = 0;      // Number of peers discovered

String isSelected = "off", isManual = "off" , sender;
float temperature = 30; // Example data
float humidity = 80;    // Example data
float moisture = 250;          // Example data
// int charge = 98;      // Example data
String isRain = "no";
bool isAutomatic = false;
unsigned long lastSendTime = 0; // Variable to store the last send time
const unsigned long sendInterval = 3600000; // 10 seconds in milliseconds
unsigned long lastUpdateTime = 0;




// Structure for tracking active devices
struct Device {
    char deviceID[20];  // Use device field as the unique ID
    unsigned long lastActiveTime;
};

Device deviceList[MAX_DEVICES]; // List of devices
int deviceCount = 0;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void getSensorData() {
  
  moisture = analogRead(MOIS_PIN)/26;
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  if (isnan(temperature)) {
    temperature = 0;
  }

  // Check if humidity is NaN and set it to 0 if true
  if (isnan(humidity)) {
    humidity = 0;
  }


  // Output sensor data to Serial Monitor
  Serial.print("Temp    :  ");
  Serial.print(temperature);
  Serial.print("   Humidity:  ");
  Serial.print(humidity);
  Serial.print("   Moisture:  ");
  Serial.println(moisture);
}


// Function to check if a device exists in the list
bool isDevicePresent(const char* deviceID) {
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(deviceList[i].deviceID, deviceID) == 0) {
            return true; // Device is present
        }
    }
    return false; // Device is not in the list
}

// Function to add/update a device in the list
void updateDeviceList(const char* deviceID) {
    if (!isDevicePresent(deviceID)) {
        // Add new device if there is space
        if (deviceCount < MAX_DEVICES) {
            strncpy(deviceList[deviceCount].deviceID, deviceID, sizeof(deviceList[deviceCount].deviceID) - 1);
            deviceList[deviceCount].lastActiveTime = millis();
            deviceCount++;
        }
    } else {
        // Update the last active time
        for (int i = 0; i < deviceCount; i++) {
            if (strcmp(deviceList[i].deviceID, deviceID) == 0) {
                deviceList[i].lastActiveTime = millis();
                break;
            }
        }
    }
}

// Function to remove inactive devices
void removeInactiveDevices() {
    unsigned long currentTime = millis();
    for (int i = 0; i < deviceCount; i++) {
        if (currentTime - deviceList[i].lastActiveTime > TIMEOUT) {
            Serial.print("Removing inactive device: ");
            Serial.println(deviceList[i].deviceID);
            
            // Shift remaining devices to fill the gap
            for (int j = i; j < deviceCount - 1; j++) {
                deviceList[j] = deviceList[j + 1];
            }
            deviceCount--;
            i--; // Adjust index after removal
        }
    }
}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  char messageType[10];
  memcpy(messageType, data, 10);  // Extract the first 10 bytes (message type)
  messageType[9] = '\0'; // Ensure null termination

  if (strcmp(messageType, "DATA") == 0) {
    message receiveData;
    memcpy(&receiveData, data, sizeof(receiveData));

    if (strcmp(receiveData.device, GLOBAL_NAME) != 0) {
      // If the data is not for this device, forward it to all peers
      if (isDevicePresent(receiveData.device)){
        Serial.println("Message ignored (device already registered).");

      }

      else{
        // Serial.println("\nReceived SENSOR DATA for this device:");
        Serial.print("Received SENSOR DATA for the device "); Serial.println(receiveData.device);
        Serial.print("from: "); Serial.println(receiveData.sender);
        Serial.print("is Device "); Serial.println(receiveData.isSelected);
        Serial.print("Temperature: "); Serial.println(receiveData.temperature);
        Serial.print("Humidity: "); Serial.println(receiveData.humidity);
        Serial.print("moisture: "); Serial.println(receiveData.moisture);
        // Serial.print("Charge: "); Serial.println(receiveData.charge);
        Serial.println("Forwarding data to all peers...");
        updateDeviceList(receiveData.device);
        for (int i = 0; i < peerCount; i++) {
          sendOtherData(peerMACs[i].mac, receiveData.device, receiveData.isSelected,receiveData.temperature, receiveData.humidity, receiveData.moisture);
        }
      }

    } 
    // else {
    //   // If the data is for this device, process it

    // }
  } else if (strcmp(messageType, "CTRL") == 0) {
    messageControl receivedControl;
    memcpy(&receivedControl, data, sizeof(receivedControl));
    Serial.println("Control data received for this device:");
    Serial.print("from: "); Serial.println(receivedControl.sender);
    Serial.print("For the device: "); Serial.println(receivedControl.device);
    Serial.print("isSelected: "); Serial.println(receivedControl.isSelected);
    Serial.print("isManual: "); Serial.println(receivedControl.isManual);
    Serial.print("will it rain: "); Serial.println(receivedControl.isRain);

    if (strcmp(receivedControl.device, GLOBAL_NAME) == 0) {
      // If the control message is for this device, process it
      isManual = receivedControl.isManual;
      isSelected = receivedControl.isSelected;
      sender = receivedControl.sender;
      isRain = receivedControl.isRain;
      // Serial.println("Control data received for this device:");
      // Serial.print("from: "); Serial.println(sender);
      // Serial.print("isSelected: "); Serial.println(isSelected);
      // Serial.print("isManual: "); Serial.println(isManual);
      // Serial.print("will it rain: "); Serial.println(isRain);
      sendOK = true;
      
      if (isManual.equals("on"))
      {
        if (isSelected.equals("on")){
          digitalWrite(2, HIGH);
        }
        else{
          digitalWrite(2, LOW);
        }
      }
    }
    else if (strcmp(receivedControl.device, "ALL") == 0) {
      isRain = receivedControl.isRain;
      if (isManual == "off")
      {
        isAutomatic = true;
      }
      if (isDevicePresent(receivedControl.device)){
          Serial.println("Message ignored (device already registered).");

      }
      else{
      Serial.println("Forwarding data to all peers...");
      updateDeviceList(receivedControl.device);      
    
      Serial.println("Forwarding control data to all peers...");

      for (int i = 0; i < peerCount; i++) {
        sendOtherControl(peerMACs[i].mac, receivedControl.device, receivedControl.isManual, receivedControl.isSelected, receivedControl.isRain);
      }

      }
    }
    else {
      // If the control message is not for this device, forward it to all peers
        if (isDevicePresent(receivedControl.device)){
          Serial.println("Message ignored (device already registered).");

        }

        else{
          Serial.println("Forwarding data to all peers...");
          updateDeviceList(receivedControl.device);      
        
          Serial.println("Forwarding control data to all peers...");
          for (int i = 0; i < peerCount; i++) {
            sendOtherControl(peerMACs[i].mac, receivedControl.device, receivedControl.isManual, receivedControl.isSelected, receivedControl.isRain);
          }
        }
    }
  } else {
    Serial.println("\nUnknown message type received!");
  }
}

// Check if a peer is already in the list
bool isPeerInList(uint8_t *peerMAC) {
  for (int i = 0; i < peerCount; i++) {
    if (memcmp(peerMACs[i].mac, peerMAC, 6) == 0) {
      return true; // Peer already exists
    }
  }
  return false; // Peer not found
}

// Add a peer dynamically
void addPeer(uint8_t *peerMAC, int channel) {
  if (peerCount >= 10) {
    Serial.println("Peer list is full!");
    return;
  }

  if (isPeerInList(peerMAC)) {
    Serial.println("Peer already in list!");
    return;
  }

  // Set the Wi-Fi channel to match the peer's channel
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  Serial.print("Set Wi-Fi channel to: ");
  Serial.println(channel);

  // Add the peer to the list
  memcpy(peerMACs[peerCount].mac, peerMAC, 6);
  peerMACs[peerCount].channel = channel;
  peerCount++;

  // Add the peer to ESP-NOW
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = channel;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("Added peer successfully");
  } else {
    Serial.println("Failed to add peer");
  }
}

// Scan for ESP32 devices and add as peers
void scanForPeers() {
  Serial.println("Scanning for ESP32 devices...");
  int numNetworks = WiFi.scanNetworks();
  if (numNetworks == 0) {
    Serial.println("No devices found!");
    return;
  }

  // Clear the peer list before scanning
  peerCount = 0;

  for (int i = 0; i < numNetworks; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.indexOf("ESP") != -1) {
      Serial.print("Found ESP32: ");
      Serial.println(WiFi.BSSIDstr(i));

      uint8_t peerMAC[6];
      WiFi.BSSID(i, peerMAC);
      int peerChannel = WiFi.channel(i); // Get the peer's Wi-Fi channel
      addPeer(peerMAC, peerChannel);
    }
  }

  WiFi.scanDelete();
}

// Send this device's data to all peers
void sendDataToAllPeers() {
  strcpy(myData.type, "DATA");
  strcpy(myData.device, GLOBAL_NAME);
  strcpy(myData.sender, GLOBAL_NAME);
  strcpy(myData.isSelected, isSelected.c_str());
  myData.temperature = temperature;
  myData.humidity = humidity;
  myData.moisture = moisture;
  // myData.charge = charge;

  for (int i = 0; i < peerCount; i++) {
    esp_err_t result = esp_now_send(peerMACs[i].mac, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Data Sent to peer!");
    } else {
      Serial.println("Send Failed to peer");
    }
  }
}

// Send control data to a peer
void sendOtherControl(uint8_t *peerMAC, String device, String isManual, String isSelected, String isRain) {
  messageControl controlMessage;
  strcpy(controlMessage.type, "CTRL");
  strcpy(controlMessage.device, device.c_str());
  strcpy(controlMessage.sender, GLOBAL_NAME);
  strcpy(controlMessage.isManual, isManual.c_str());
  strcpy(controlMessage.isSelected, isSelected.c_str());
  strcpy(controlMessage.isRain, isRain.c_str());

  esp_err_t result = esp_now_send(peerMAC, (uint8_t *)&controlMessage, sizeof(controlMessage));
  if (result == ESP_OK) {
    Serial.println("Control Data Sent!");
  } else {
    Serial.println("Control Data Send Failed");
  }
}

// Send other data to a peer
void sendOtherData(uint8_t *peerMAC, String device, String isSelected ,int temperature, int humidity, int moisture) {
  message otherData;
  strcpy(otherData.type, "DATA");
  strcpy(otherData.device, device.c_str());
  strcpy(otherData.sender, GLOBAL_NAME);
  strcpy(otherData.isSelected, isSelected.c_str());
  otherData.temperature = temperature;
  otherData.humidity = humidity;
  otherData.moisture = moisture;
  // otherData.charge = charge;

  esp_err_t result = esp_now_send(peerMAC, (uint8_t *)&otherData, sizeof(otherData));
  if (result == ESP_OK) {
    Serial.println("Data Sent!");
  } else {
    Serial.println("Send Failed");
  }
}

void checkWater(){
  if (isRain == "no"){
    Serial.println("I am watering");
    digitalWrite(2, HIGH);
    delay(10000);
    digitalWrite(2, LOW);
  }
    if (isRain == "yes"){
    Serial.println("It is raining. I don't water");
  }


}
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);

  delay(100);
  dht.begin();

  delay(100);
  
  getSensorData();

  pinMode(2, OUTPUT);
  Serial.println("WiFi Connected!");
  Serial.print("WiFi Channel: ");
  Serial.println(WiFi.channel());

  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  scanForPeers();
  delay(1000);
  sendDataToAllPeers(); // Send this device's data to all peers
  lastSendTime = millis(); // Update last send time
}

void loop() {
  if (millis() - lastSendTime >= sendInterval || sendOK) {
    getSensorData();
    delay(100);
    scanForPeers();
    delay(1000);
    sendDataToAllPeers(); // Send this device's data to all peers
    lastSendTime = millis(); // Update last send time
    sendOK = false;
  }
  if (millis() - lastUpdateTime >= 1000) {
    removeInactiveDevices();
    lastUpdateTime = millis(); // Update last send time
  }

  if (isAutomatic){
    checkWater();
    isAutomatic = false;
  }



}