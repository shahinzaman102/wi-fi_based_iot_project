#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT_Client.h>
#include <FirebaseESP8266.h>

#define username "internet_of_things"
#define password "YOUR_PASSWORD_HERE"

#define network "YOUR_NETWORK_NAME_HERE"
#define wifiPassword "YOUR_PASSWORD_HERE"

#define adafruitServer "io.adafruit.com"
#define adafruitPort 1883
#define adafruitUsername "YOUR_USERNAME_HERE"
#define adafruitKey "YOUR_KEY_HERE"

#define firebaseProject "REALTIME_DATABASE_URL_HERE"
#define firebaseSecret "FIREBASE_SECRET_HERE"

ESP8266WebServer server;

String ledwebpage = "<html><title>My First Webpage</title><body style=\"background-color: palegreen;\"><center><h1>IoT</h1></center><form action=\"/led\" method=\"get\"><button style=\"font-size:60\" type=\"submit\" value=\"0\" name=\"state\"> LED ON</button><button style=\"font-size:60\" type=\"submit\" value=\"1\" name=\"state\"> LED OFF</button></form></body></html>";

WiFiClient esp;
Adafruit_MQTT_Client mqtt(&esp, adafruitServer, adafruitPort, adafruitUsername, adafruitKey);
Adafruit_MQTT_Publish feed(&mqtt, adafruitUsername "/feeds/ldr");

FirebaseData ldrData;
FirebaseData ledData;

#define ledPin 16

void setup() {
  Serial.begin(115200);
  pinMode(16, OUTPUT);
  WiFi.softAP(username, password);
  Serial.println(WiFi.softAPIP());
  server.begin();
  server.on("/led", Led);

  server.on("/", HTTP_GET, handleRoot); // Serve the HTML page
  server.on("/mqtt", HTTP_GET, handleMqtt); // Route for MQTT/Firebase
  server.on("/ldr", HTTP_GET, handleLDR); // Route for LDR data

  server.begin(); // Start the server

  Serial.println("Setup completed.");

  WiFi.begin(network, wifiPassword);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Web server started");  // Debug print

  Serial.println("Connecting to Adafruit MQTT");
  while (!mqtt.connected()) {
    Serial.print(".");
    delay(1000);
    
    if (mqtt.connect()) {
      Serial.println("Connected to Adafruit MQTT");
    } else {
      Serial.println("Failed to connect to Adafruit MQTT");
      Serial.println("Connecting again...");
    }
  }

  Firebase.begin(firebaseProject, firebaseSecret);
  Serial.println("Setup completed.");
}

void loop() {
  // ... (other code)
  Serial.println("Looping...");
  if (mqtt.connected()) {
    int data = analogRead(A0);  // Read the sensor value
    Serial.println("Sensor Value: " + String(data));  // Debug print

    if (feed.publish(data)) {
      Serial.println("Data published to Adafruit");
    } else {
      Serial.println("Failed to publish data to Adafruit");
    }

    // Firebase code
    Serial.println("Writing data to Firebase");
    Firebase.setInt(ldrData, "/LDR", data);
  }

  server.handleClient();

  delay(2500);
  Serial.println("Loop iteration completed.");
}

void handleRoot() {
  server.send(200, "text/html", ledwebpage);
}

void handleMqtt() {
  // Handle MQTT/Firebase related tasks here
  // You can add your MQTT and Firebase code here
  server.send(200, "text/plain", "MQTT/Firebase request handled");
}

void handleLDR() {
  int ldrValue = analogRead(A0);
  Serial.println("LDR Value: " + String(ldrValue));
  Firebase.setInt(ldrData, "/LDR", ldrValue);

  server.send(200, "text/plain", "LDR data sent to Firebase: " + String(ldrValue));
}

void Led() {
  server.send(200, "text/html", ledwebpage);

  if((server.arg("state")=="0")) {
    digitalWrite(16, LOW);
  }
  else {
    digitalWrite(16, HIGH);
  }
}
