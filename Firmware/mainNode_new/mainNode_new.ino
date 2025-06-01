#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
// #include <HTTPClient.h>
#include <time.h> 
#include <esp_now.h>
#include <esp_wifi.h>


WebServer server(80);

// Variables to hold configuration data
String ssid = "";
String password = "";
String firebaseUser = "";
String firebasePass = "";
String city = "colombo"; // Replace with your city

// Firebase and MQTT configuration
#define FIREBASE_PROJECT_ID "growflow-bc478"
#define FIREBASE_API_KEY "AIzaSyAqbuNguEpLLXNJDk6c99jUUrdhzsfLxQU"
#define FIREBASE_HOST "growflow-bc478.firebaseio.com"

#define MAX_DEVICES 10
#define TIMEOUT 1000  // 10 seconds timeout for inactive devices

const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttClientID = "esp32_client_grow_flow";

// Timer set to 10 seconds (10000)
// unsigned long timerDelay = 10000;
// unsigned long lastTime = 0;

String jsonBuffer;


// ESP-NOW and other variables
uint8_t peerMAC[6];
bool peerAdded = false;
bool sendTrue = false;
String isSelected = "no";
String isManual;
// bool runAutomatic = false;

String openWeatherMapApiKey = "958edb48e400cc706dc1602c11824bfe";

// unsigned long lasreceiveDatatFirebaseSendTime = 0;
// const unsigned long sendInterval = 10000;  // 10 seconds
unsigned long lastUpdateTime,  lastUpdateTime2= 0;

String countryCode = "LK";
String timeOfDay = "";
String isRain;
// bool isConnected = false;
bool serverStopped = false;




typedef struct message {
  char type[10];
  char device[20];
  char sender[20];
  char isSelected[10];
  float temperature;
  float humidity;
  float moisture;
  // int charge;
} message;

typedef struct messageSend {
  char type[10];
  char device[20];
  char sender[20];
  char isSelected[10];
  char manual[10];
  char isRain[10];
} messageSend;

// Structure for tracking active devices
struct Device {
    char deviceID[20];  // Use device field as the unique ID
    unsigned long lastActiveTime;
};

Device deviceList[MAX_DEVICES]; // List of devices
int deviceCount = 0;

message receivedData;
messageSend myData;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Function prototypes
void startAPMode();
void connectToWiFi();
void handleRoot();
void handleConfig();
void sendDataToFirestore(const char* deviceName, int humidity, int temperature, int moisture, const char* isSelected);
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void addPeer(uint8_t *peerMAC);
void scanForPeers();
void reconnectWiFi();
void reconnectToMQTT();
void sendDataToPeer(uint8_t *peerMAC, String label, String isSelected, String isManual);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
// void willRainInNext12Hours();
// String removeChunkedEncoding(String response);

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
            Serial.print(F("Removing inactive device: "));
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

void checkIfRainingNext24Hours(String city, String countryCode) {
  // Construct the API path for the 5-day forecast
  String serverPath = "/data/2.5/forecast?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

  // Use WiFiClient to send the HTTP GET request
  jsonBuffer = httpGETRequest(serverPath);
  // Serial.println("Raw response:");
  // Serial.println(jsonBuffer);

  // Check if the response contains an error
  if (jsonBuffer.indexOf("404 Not Found") >= 0) {
    Serial.println("Error: City not found. Check the city name and country code.");
    isRain = "no"; // Assume no rain if city is not found
  } else if (jsonBuffer.indexOf("401 Unauthorized") >= 0) {
    Serial.println("Error: Invalid API key. Check your OpenWeatherMap API key.");
    isRain = "no"; // Assume no rain if API key is invalid
  } else {
    // Extract the JSON payload from the response
    int jsonStart = jsonBuffer.indexOf("{");
    int jsonEnd = jsonBuffer.lastIndexOf("}");
    if (jsonStart >= 0 && jsonEnd >= 0) {
      String jsonPayload = jsonBuffer.substring(jsonStart, jsonEnd + 1);
      Serial.println("JSON payload:");
      Serial.println(jsonPayload);

      // Parse the JSON response
      JSONVar myObject = JSON.parse(jsonPayload);

      // Check if parsing was successful
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        isRain = "no"; // Assume no rain if parsing fails
      } else {
        // Get the current time (Unix timestamp)
        unsigned long currentTime = (unsigned long)myObject["list"][0]["dt"];

        // Get sunrise and sunset times
        unsigned long sunrise = (unsigned long)myObject["city"]["sunrise"];
        unsigned long sunset = (unsigned long)myObject["city"]["sunset"];

        // Determine the time of day
        timeOfDay = getTimeOfDay(currentTime, sunrise, sunset);

        // Get the list of forecast entries
        JSONVar forecastList = myObject["list"];

        // Loop through the forecast entries for the next 24 hours
        for (int i = 0; i < forecastList.length(); i++) {
          unsigned long forecastTime = (unsigned long)forecastList[i]["dt"];

          // Check if the forecast time is within the next 24 hours
          if (forecastTime <= currentTime + 86400) { // 86400 seconds = 24 hours
            // Check if rain is predicted
            if (forecastList[i].hasOwnProperty("rain")) {
              JSONVar rainData = forecastList[i]["rain"];
              if ((double)rainData["3h"] > 0) { // Check if rain volume is greater than 0
                isRain = "yes";
                break; // Exit the loop if rain is found
              }
            }
          }
        }

        // If no rain is found in the next 24 hours
        if (isRain != "yes") {
          isRain = "no";
        }
      }
    } else {
      Serial.println("Error: No JSON payload found in the response.");
      isRain = "no"; // Assume no rain if no JSON payload is found
    }
  }
}

String httpGETRequest(String path) {
  WiFiClient client;
  String payload = "";

  // Connect to the server
  if (client.connect("api.openweathermap.org", 80)) {
    Serial.println("Connected to server");

    // Send HTTP GET request
    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: api.openweathermap.org\r\n" +
                 "Connection: close\r\n\r\n");

    // Wait for the server to respond
    unsigned long startTime = millis();
    while (client.connected() && millis() - startTime < 5000) { // 5-second timeout
      while (client.available()) {
        String line = client.readStringUntil('\n');
        payload += line + "\n"; // Append the line to the payload
      }
    }

    client.stop(); // Close the connection
    Serial.println("Connection closed");
  } else {
    Serial.println("Failed to connect to server");
  }

  return payload;
}

String getTimeOfDay(unsigned long currentTime, unsigned long sunrise, unsigned long sunset) {
  if (currentTime < sunrise) {
    return "Night";
  } else if (currentTime >= sunrise && currentTime < sunrise + 14400) { // 4 hours after sunrise
    return "Morning";
  } else if (currentTime >= sunrise + 14400 && currentTime < sunset - 14400) { // 4 hours before sunset
    return "Afternoon";
  } else if (currentTime >= sunset - 14400 && currentTime < sunset) { // Sunset - 4 hours to sunset
    return "Evening";
  } else {
    return "Night";
  }
}



void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  delay(100);
  Serial.println("LED ON");
  digitalWrite(2, HIGH);
  startAPMode();
  


  while (!serverStopped){
    server.handleClient();
  }

  reconnectWiFi();  // Connect to WiFi
  delay(1000);

  Serial.println(F("WiFi Connected!"));


  if (esp_now_init() != ESP_OK) {
    Serial.println(F("ESP-NOW initialization failed!"));
    return;  // Return early if initialization fails
  }

  delay(1000);





  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  reconnectToMQTT();  // Connect to MQTT broker

  auth.user.email = firebaseUser;
  auth.user.password = firebasePass;
  config.api_key = FIREBASE_API_KEY;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  scanForPeers();


  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(onDataRecv);
  
  delay(1000);
  
  // Set Wi-Fi channel to 11
  if (esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE) == ESP_OK) {
    Serial.println(F("Wi-Fi channel set to 11."));
  } else {
    Serial.println(F("Failed to set Wi-Fi channel."));
  }

  delay(1000);
  checkIfRainingNext24Hours(city, "LK");

  // Print the result
  Serial.print("Will it rain in Wattala in the next 24 hours? ");
  Serial.println(isRain);

  // Print the time of day
  Serial.print("Current time of day: ");
  Serial.println(timeOfDay);


  Serial.print(F("WiFi Channel: "));
  Serial.println(WiFi.channel());

  Serial.print(F("MAC Adress is: "));
  Serial.println(WiFi.macAddress());

  digitalWrite(2, LOW);
  Serial.println("LED OFF");

}

void loop() {
  reconnectWiFi();    // Ensure WiFi is connected
  if (!client.connected()) {
    reconnectToMQTT(); // Ensure MQTT is connected
  }

  client.loop();  // Keep MQTT communication active

  if (sendTrue)
  {
    sendDataToFirestore(receivedData.device, receivedData.humidity, receivedData.temperature, receivedData.moisture, receivedData.isSelected);
    sendTrue = false; 
  }

  if (millis() - lastUpdateTime >= 3600000) {
    // removeInactiveDevices();
    // Check if it is raining in the next 24 hours for a specific city
    checkIfRainingNext24Hours(city, "LK");

    // Print the result
    Serial.print("Will it rain in Wattala in the next 24 hours? ");
    Serial.println(isRain);

    // Print the time of day
    Serial.print("Current time of day: ");
    Serial.println(timeOfDay);
    lastUpdateTime = millis(); // Update last send time

    if (timeOfDay == "Night")
    {
      scanForPeers();
      delay(1000);
      sendDataToAll(peerMAC, "ALL", isSelected, isManual);


      
    }
  }

  // if (millis() - lastUpdateTime2 >= 2000) {
  //   removeInactiveDevices();
  //   // willRainInNext12Hours();
  //   lastUpdateTime2 = millis(); // Update last send time
  // }
}

// Web server and configuration functions
void startAPMode() {
  // Start in AP+STA mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32_Config", "");
  Serial.println(F("Access Point Started"));
  Serial.println(WiFi.softAPIP());



  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.begin();
  Serial.println(F("Web server started."));
}

void connectToWiFi() {

  // Stop the web server if it was running

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("\nWiFi Connected!"));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());



}



void handleRoot() {
  String html = "<form action='/config' method='POST'>";
  html += "SSID: <input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='password'><br>";
  html += "Firebase User: <input type='text' name='firebaseUser'><br>";
  html += "Firebase Pass: <input type='password' name='firebasePass'><br>";
  html += "City: <input type='text' name='city'><br>";
  html += "<input type='submit' value='Submit'>";
  html += "</form>";
  server.send(200, "text/html", html);
}



void handleConfig() {
  // Get form data
  // Get form data
  ssid = server.arg("ssid");
  password = server.arg("password");
  firebaseUser = server.arg("firebaseUser");
  firebasePass = server.arg("firebasePass");
  city = server.arg("city");


  // Send a response to the browser
  server.send(200, "text/plain", "Credentials Received. Connecting to WiFi...");
  // Serial.println(F("Credentials received. Connecting to WiFi..."));

  // Print the values
  // Serial.println("SSID: " + ssid);
  // Serial.println("Password: " + password);
  // Serial.println("Firebase User: " + firebaseUser);
  // Serial.println("Firebase Password: " + firebasePass);
  // Serial.println("Current city: " + city);

  server.stop();
  serverStopped = true;
  // Serial.println("Web server stopped.");


  // isConnect = true;
  delay(1000);
  connectToWiFi();
}



void sendDataToFirestore(const char* deviceName, int humidity, int temperature, int moisture, const char* isSelected) {
  if (Firebase.ready()) {
    String documentPath = firebaseUser + "/" + String(deviceName);
    FirebaseJson content;

    // content.set("fields/Moisture/doubleValue", Moisture);
    content.set("fields/humidity/doubleValue", humidity);
    content.set("fields/temperature/doubleValue", temperature);
    // content.set("fields/charge/doubleValue", charge);
    content.set("fields/isSelected/stringValue", isSelected);
    content.set("fields/moisture/doubleValue", moisture);

    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "humidity,temperature,isSelected,moisture")) {
      Serial.println("Data sent successfully!");
      delay(3000);
      removeInactiveDevices();
    } else {
      // Serial.println("Error sending data: " + fbdo.errorReason());
    }
  }
}




// Callback when data is received
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  char messageType[10];
  memcpy(messageType, data, 10);  // Extract the first 10 bytes (message type)
  messageType[9] = '\0'; // Ensure null termination
  
  
  if (strcmp(messageType, "DATA") == 0) {
    // message receiveData;
    memcpy(&receivedData, data, sizeof(receivedData));

      // If the data is not for this device, forward it to all peers
      if (isDevicePresent(receivedData.device)){
        // Serial.println("Message ignored (device already registered).");

      }

      else{
        // delay(1000);
        Serial.println("Forwarding data to all peers...");
        updateDeviceList(receivedData.device);
          sendTrue = true;
      }
    Serial.println("\nReceived Message:");
    Serial.print("Device: "); Serial.println(receivedData.device);
    Serial.print("sender: "); Serial.println(receivedData.sender);
    Serial.print("Temperature: "); Serial.println(receivedData.temperature);
    Serial.print("Humidity: "); Serial.println(receivedData.humidity);
    Serial.print("moisture: "); Serial.println(receivedData.moisture);
    Serial.print("isSelected: "); Serial.println(receivedData.isSelected);
    // Serial.print("Charge: "); Serial.println(receivedData.charge);
  }
  

}

// Add a peer dynamically
void addPeer(uint8_t *peerMAC) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    // Serial.println("Added peer successfully");
    peerAdded = true;
  } else {
    // Serial.println("Failed to add peer");
  }
}



// Scan for ESP32 devices and add as peer
void scanForPeers() {
  Serial.println("Scanning for ESP32 devices...");
  int numNetworks = WiFi.scanNetworks();
  if (numNetworks == 0) {
    // Serial.println("No devices found!");
    return;
  }

  for (int i = 0; i < numNetworks; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.indexOf("ESP") != -1) {
      // Serial.print("Found ESP32: ");
      // Serial.println(WiFi.BSSIDstr(i));

      WiFi.BSSID(i, peerMAC);
      addPeer(peerMAC);
      break;
    }
  }
}



void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    // Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {  // Try for 10 seconds
      delay(500);
      Serial.print(".");
      attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      // Serial.println("\nWiFi Connected!");
      // Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      // Serial.println("Failed to set Wi-Fi channel");
      // }
    } else {
      
      ESP.restart();
    }
  }
}

void reconnectToMQTT() {
  while (!client.connected()) {
    if (client.connect(mqttClientID)) {
      client.subscribe("home/relay/outgoing");
      // Serial.println("Connected to MQTT Broker.");
    } else {
      // Serial.println("MQTT Connection failed, retrying...");
      delay(1000);
    }
  }
}

void sendDataToAll(uint8_t *peerMAC, String label, String isSelected, String isManual) {
  if (peerAdded) {
    // Copy the received values to the myData structure
    strcpy(myData.type, "CTRL");
    strncpy(myData.device, label.c_str(), sizeof(myData.device) - 1);
    strncpy(myData.isSelected, isSelected.c_str(), sizeof(myData.isSelected) - 1);
    strncpy(myData.manual, isManual.c_str(), sizeof(myData.manual) - 1);
    strncpy(myData.isRain, isRain.c_str(), sizeof(myData.isRain) - 1);

    // Ensure the strings are null-terminated
    myData.device[sizeof(myData.device) - 1] = '\0';
    myData.isSelected[sizeof(myData.isSelected) - 1] = '\0';
    myData.manual[sizeof(myData.manual) - 1] = '\0';
    myData.isRain[sizeof(myData.isRain) - 1] = '\0';

    // Send the struct data over ESP-NOW
    esp_err_t result = esp_now_send(peerMAC, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Data Sent!");
    } else {
      Serial.println("Send Failed");
    }
  }
}

void sendDataToPeer(uint8_t *peerMAC, String label, String isSelected, String isManual) {
  if (peerAdded) {
    // Copy the received values to the myData structure
    strcpy(myData.type, "CTRL");
    strncpy(myData.device, label.c_str(), sizeof(myData.device) - 1);
    strncpy(myData.isSelected, isSelected.c_str(), sizeof(myData.isSelected) - 1);
    strncpy(myData.manual, isManual.c_str(), sizeof(myData.manual) - 1);
    strncpy(myData.isRain, isRain.c_str(), sizeof(myData.isRain) - 1);

    // Ensure the strings are null-terminated
    myData.device[sizeof(myData.device) - 1] = '\0';
    myData.isSelected[sizeof(myData.isSelected) - 1] = '\0';
    myData.manual[sizeof(myData.manual) - 1] = '\0';
    myData.isRain[sizeof(myData.isRain) - 1] = '\0';

    // Send the struct data over ESP-NOW
    esp_err_t result = esp_now_send(peerMAC, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Data Sent!");
    } else {
      Serial.println("Send Failed");
    }
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, message);

  if (error) return;

  String label = doc["label"];
  isSelected = doc["isSelected"].as<String>();
  isManual = doc["isManual"].as<String>();
  Serial.print("device: ");Serial.println(label);
  Serial.print("device: ");Serial.println(isSelected);
  Serial.print("Manual mode: ");Serial.println(isManual);
  
  scanForPeers();
  delay(1000);
  sendDataToPeer(peerMAC, label, isSelected, isManual);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



// Helper function to remove chunked encoding
String removeChunkedEncoding(String response) {
  String cleanJson = "";
  int index = 0;

  while (index < response.length()) {
    int chunkSizeEnd = response.indexOf("\r\n", index);
    if (chunkSizeEnd == -1) break; 

    String chunkSizeStr = response.substring(index, chunkSizeEnd);
    int chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16); 

    if (chunkSize == 0) break; 

    index = chunkSizeEnd + 2;
    cleanJson += response.substring(index, index + chunkSize);
    index += chunkSize + 2; 
  }

  return cleanJson;
}