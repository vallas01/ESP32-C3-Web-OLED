#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <U8g2lib.h>

// --------------------------------------------------
// Wi-Fi
// --------------------------------------------------

const char* ssid = "1127";
const char* password = "SaltyTao1!";

// --------------------------------------------------
// LED
// --------------------------------------------------

const int LED_PIN = 8;

// LED is active LOW:
// LOW  = ON
// HIGH = OFF

// --------------------------------------------------
// OLED
// SDA = GPIO 5
// SCL = GPIO 6
// --------------------------------------------------

U8G2_SSD1306_72X40_ER_F_HW_I2C display(
  U8G2_R0,
  U8X8_PIN_NONE,
  6,  // SCL
  5   // SDA
);

// --------------------------------------------------
// Web server
// --------------------------------------------------

WebServer server(80);

// --------------------------------------------------
// OLED animation
// --------------------------------------------------

unsigned long lastDisplayChange = 0;

const unsigned long displayInterval = 1500;

int displayScreen = 0;

// 0 = HELLO Ricky
// 1 = Emoticon

// --------------------------------------------------
// Display HELLO / Ricky
// --------------------------------------------------

void showHello() {

  display.clearBuffer();

  display.drawFrame(2, 1, 68, 38);

  // HELLO
  display.setFont(u8g2_font_t0_22_tf);

  const char *hello = "HELLO";

  int helloWidth = display.getStrWidth(hello);
  int helloX = (72 - helloWidth) / 2;

  display.drawStr(helloX, 17, hello);

  // Ricky
  const char *name = "Ricky";

  int nameWidth = display.getStrWidth(name);
  int nameX = (72 - nameWidth) / 2;

  display.drawStr(nameX, 36, name);

  display.sendBuffer();
}

// --------------------------------------------------
// Display emoticon
// --------------------------------------------------

void showEmoticon() {

  display.clearBuffer();

  display.setFont(u8g2_font_emoticons21_tr);

  display.drawGlyph(27, 28, 0x21);

  display.sendBuffer();
}

// --------------------------------------------------
// Blank OLED
// --------------------------------------------------

void blankDisplay() {

  display.clearBuffer();
  display.sendBuffer();
}

// --------------------------------------------------
// Main web page
// --------------------------------------------------

void handleRoot() {

  bool ledOn = (digitalRead(LED_PIN) == LOW);

  String html = "";

  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";

  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";

  html += "<title>ESP32-C3 LED</title>";

  html += "<style>";

  html += "body {";
  html += "font-family: Arial;";
  html += "text-align: center;";
  html += "margin-top: 60px;";
  html += "}";

  html += ".switch {";
  html += "position: relative;";
  html += "display: inline-block;";
  html += "width: 100px;";
  html += "height: 50px;";
  html += "}";

  html += ".switch input {";
  html += "display: none;";
  html += "}";

  html += ".slider {";
  html += "position: absolute;";
  html += "cursor: pointer;";
  html += "top: 0;";
  html += "left: 0;";
  html += "right: 0;";
  html += "bottom: 0;";
  html += "background-color: #777;";
  html += "border-radius: 50px;";
  html += "transition: .3s;";
  html += "}";

  html += ".slider:before {";
  html += "position: absolute;";
  html += "content: '';";
  html += "height: 42px;";
  html += "width: 42px;";
  html += "left: 4px;";
  html += "bottom: 4px;";
  html += "background-color: white;";
  html += "border-radius: 50%;";
  html += "transition: .3s;";
  html += "}";

  html += "input:checked + .slider {";
  html += "background-color: #2196F3;";
  html += "}";

  html += "input:checked + .slider:before {";
  html += "transform: translateX(50px);";
  html += "}";

  html += "</style>";

  html += "</head>";

  html += "<body>";

  html += "<h1>ESP32-C3 LED</h1>";

  if (ledOn) {
    html += "<p>LED is ON</p>";
  } else {
    html += "<p>LED is OFF</p>";
  }

  html += "<label class='switch'>";

  html += "<input type='checkbox' ";

  if (ledOn) {
    html += "checked ";
  }

  html += "onchange=\"window.location='/led/toggle'\">";

  html += "<span class='slider'></span>";

  html += "</label>";

  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

// --------------------------------------------------
// Toggle LED and OLED
// --------------------------------------------------

void handleLedToggle() {

  // LED is currently ON
  if (digitalRead(LED_PIN) == LOW) {

    // Turn LED OFF
    digitalWrite(LED_PIN, HIGH);

    // Blank OLED
    blankDisplay();

  } else {

    // Turn LED ON
    digitalWrite(LED_PIN, LOW);

    // Start OLED animation with HELLO
    displayScreen = 0;

    showHello();

    lastDisplayChange = millis();
  }

  // Return to main page
  server.sendHeader("Location", "/");
  server.send(303);
}

// --------------------------------------------------
// Update OLED animation
// --------------------------------------------------

void updateDisplay() {

  // Nothing to do if LED is OFF
  if (digitalRead(LED_PIN) == HIGH) {
    return;
  }

  unsigned long currentTime = millis();

  if (currentTime - lastDisplayChange >= displayInterval) {

    lastDisplayChange = currentTime;

    if (displayScreen == 0) {

      // HELLO → Emoticon
      displayScreen = 1;

      showEmoticon();

    } else {

      // Emoticon → HELLO
      displayScreen = 0;

      showHello();
    }
  }
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup() {

  Serial.begin(115200);

  delay(2000);

  // LED
  pinMode(LED_PIN, OUTPUT);

  // Start with LED OFF
  digitalWrite(LED_PIN, HIGH);

  // OLED
  display.begin();

  // Start with OLED blank
  blankDisplay();

  // Wi-Fi
  Serial.println();
  Serial.println("ESP32-C3 Web Server");
  Serial.println("--------------------");

  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Web server
  server.on("/", handleRoot);

  server.on("/led/toggle", handleLedToggle);

  server.begin();

  Serial.println("Web server started!");
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------

void loop() {

  // Check for web requests continuously
  server.handleClient();

  // Update OLED animation without blocking
  updateDisplay();
}