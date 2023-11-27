#include "HX711.h"
#include <M5StickCPlus.h>
#include <vector>
#include <numeric>
#include <WiFi.h>
#include <HTTPClient.h>

using namespace std;

HX711 scale;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 33;
const int LOADCELL_SCK_PIN = 32;

const long current_offset = 4294704982;
const float current_scale = 23.3;

vector<double> weights;

const char* ssid = "TheNine_Resident_WiFi";
const char* password = "lion404furry";
//const char* ssid = "utexas-iot";
//const char* password = "10908760761514842181";
const char* url = "https://scalekit-backend.onrender.com/api/send-weight";

void setup() {
  M5.begin();
  Serial.begin(115200);

  setupScale();
}

void loop() {
  M5.update();

  // Check if scale is ready to read
  if (scale.is_ready()) {

    captureWeights();
  
    if (M5.BtnA.wasPressed()) {
      double weight = getFinalWeight();
      weights.clear();
      sendDataToServer(weight);
    }

    if (M5.BtnB.wasPressed()) {
      // Action to be performed when button B is pressed
      // Replace this with your desired action
      M5.Lcd.println("Button B Pressed!");
    }
  } else {
    Serial.println("HX711 not found.");
  }
  
  delay(500); // Wait for half a second
}


void setupScale() {
  // Initialize the HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Set the calibration values
  scale.set_offset(current_offset);
  scale.set_scale(current_scale);
  
  M5.Lcd.println("Scale Setup Complete!");
  M5.Lcd.println();
  M5.Lcd.println("Scale is ready for use. Place your weight!");
  M5.Lcd.println();
}

void captureWeights() {
  long reading = scale.read(); // Get the raw data from the HX711
  float weight = scale.get_units(10); // Convert the data to weight
  
  Serial.print("Reading: ");
  Serial.print(reading);
  Serial.print(" | Weight: ");
  Serial.print(weight); // Print weight with 5 decimal places
  Serial.println(" grams");

  weights.push_back(weight);
  
}

int getFinalWeight() {
  
  double sum = accumulate(weights.begin(), weights.end(), 0);

  return static_cast<double>(sum) / weights.size();
}

void setupWiFi() {
  // connect to Wi-Fi
  WiFi.begin(ssid, password);

  // adding delay for wifi to connect
  delay(2000);

  M5.Lcd.println("Connected to WiFi");
  M5.Lcd.println();
}

void sendDataToServer(double weight) {
  
  // Prepare JSON data for the POST request
  String jsonPayload = "{\"weight\":" + String(weight, 2) + "}";

  M5.Lcd.print("Sending weight to server!! -> ");
  M5.Lcd.println(weight, 2);
  M5.Lcd.println();

  setupWiFi();

  // HTTP POST request
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Send the POST request with the JSON payload
  int httpCode = http.POST(jsonPayload);

  M5.Lcd.println(httpCode);

  if (httpCode == 201) {
    String payload = http.getString();
    M5.Lcd.println("HTTP Response:");
    M5.Lcd.println(payload);
    M5.Lcd.println();
  } else {
    M5.Lcd.println("HTTP request failed");
    M5.Lcd.println();
  }

  http.end();
}
