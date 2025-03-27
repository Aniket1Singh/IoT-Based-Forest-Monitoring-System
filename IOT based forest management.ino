#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

// WiFi credentials
const char *ssid = "My-Network";
const char *password = "aniket123";

// DHT11 Sensor
#define DHTPIN 21
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// PIR Motion Sensor
#define PIR_PIN 25

// Flame Sensor
#define FLAME_PIN 2

// MQ-135 Gas Sensor
#define MQ135_A0 34  // Analog Pin
#define MQ135_D0 33  // Digital Pin

// GPS Module
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600
HardwareSerial gpsSerial(2);  // Using Serial2 for GPS
TinyGPSPlus gps;  // TinyGPSPlus object

WebServer server(80);

void readSensors(float &temperature, float &humidity, String &motionStatus, String &flameStatus, String &gasStatus, String &gpsData) {
    // Read DHT11 data
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Read PIR Motion Sensor
    int pirState = digitalRead(PIR_PIN);
    motionStatus = (pirState == HIGH) ? "Motion Detected" : "No Motion";

    // Read Flame Sensor
    int flameState = digitalRead(FLAME_PIN);
    flameStatus = (flameState == HIGH) ? "Fire Detected!" : "No Fire";

    // Read MQ-135 Gas Sensor
    int mqAnalog = analogRead(MQ135_A0);
    int mqDigital = digitalRead(MQ135_D0);
    gasStatus = (mqAnalog > 2300 || mqDigital == LOW) ? "Smoke Detected!" : "No Smoke";

    // Read GPS Data using TinyGPSPlus
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
    
    if (gps.location.isValid()) {
        gpsData = "Lat: " + String(gps.location.lat(), 6) + ", Lng: " + String(gps.location.lng(), 6);
    } else {
        gpsData = "No GPS Signal";
    }
}

void handleRoot() {
    float temperature, humidity;
    String motionStatus, flameStatus, gasStatus, gpsData;
    readSensors(temperature, humidity, motionStatus, flameStatus, gasStatus, gpsData);

    char msg[4000];
    snprintf(msg, 4000,
             "<html>\
             <head>\
               <meta http-equiv='refresh' content='0.5'/>\
               <meta name='viewport' content='width=device-width, initial-scale=1'>\
               <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css'>\
               <title>IoT Forest Management System</title>\
               <style>\
               body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; background: linear-gradient(to right, #3e8e41, #a8e063); color: white; }\
               h2 { font-size: 3.0rem; margin-top: 20px; }\
               .container { width: 90%%; margin: auto; padding: 20px; }\
               .card { background-color: rgba(255, 255, 255, 0.2); padding: 20px; border-radius: 10px; box-shadow: 2px 2px 12px 1px rgba(0, 0, 0, 0.5); margin: 10px; }\
               p { font-size: 1.8rem; }\
               .sensor-labels { font-size: 1.5rem; padding-bottom: 10px; display: block; }\
               </style>\
             </head>\
             <body>\
                 <h2>IoT-Based Forest Management System</h2>\
                 <p>Developed by Aniket Singh & Sohom Dasgupta</p>\
                 <p>This system monitors environmental parameters in forests to detect potential hazards like fire, gas leaks, and intrusions, ensuring better safety and conservation efforts.</p>\
                 <div class='container'>\
                     <div class='card'>\
                         <p><i class='fas fa-thermometer-half' style='color:#ff5733;'></i> <span class='sensor-labels'>Temperature: %.2f°C</span></p>\
                         <p><i class='fas fa-tint' style='color:#00add6;'></i> <span class='sensor-labels'>Humidity: %.2f%%</span></p>\
                         <p><i class='fas fa-eye' style='color:#3498db;'></i> <span class='sensor-labels'>Motion: %s</span></p>\
                         <p><i class='fas fa-fire' style='color:#e74c3c;'></i> <span class='sensor-labels'>Flame Sensor: %s</span></p>\
                         <p><i class='fas fa-smog' style='color:#9b59b6;'></i> <span class='sensor-labels'>Gas Sensor: %s</span></p>\
                         <p><i class='fas fa-map-marker-alt' style='color:#2ecc71;'></i> <span class='sensor-labels'>GPS Data: %s</span></p>\
                     </div>\
                 </div>\
             </body>\
             </html>",
             temperature, humidity, motionStatus.c_str(), flameStatus.c_str(), gasStatus.c_str(), gpsData.c_str()
            );
    server.send(200, "text/html", msg);
}

void setup(void) {
    Serial.begin(115200);
    dht.begin();

    pinMode(PIR_PIN, INPUT);
    pinMode(FLAME_PIN, INPUT);
    pinMode(MQ135_A0, INPUT);
    pinMode(MQ135_D0, INPUT);

    // Initialize GPS
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("WiFi connected, IP: " + WiFi.localIP().toString());

    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}

void loop(void) {
    server.handleClient();
}
