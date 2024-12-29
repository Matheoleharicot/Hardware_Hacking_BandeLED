#include <ESP8266WiFi.h>

// WiFi configuration
const char* ssid = "ssid";          
const char* password = "password"; 

WiFiServer server(80); // Create a server on port 80 (HTTP)

// Define pins for the MOSFETs
const int pinRed = D0;   // Red
const int pinBlue = D1;  // Blue
const int pinGreen = D2; // Green

bool autoMode = false;  // Automatic mode (cycling)
int delayTime = 500;    // Delay between colors (in ms)

// Function to set a color
void setColor(bool red, bool green, bool blue) {
  digitalWrite(pinRed, red ? LOW : HIGH);    
  digitalWrite(pinGreen, green ? LOW : HIGH); 
  digitalWrite(pinBlue, blue ? LOW : HIGH);  
}

// Function to cycle through colors
void cycleColors() {
  setColor(false, true, true); // Red
  delay(delayTime);
  setColor(true, false, true); // Green
  delay(delayTime);
  setColor(true, true, false); // Blue
  delay(delayTime);
  setColor(false, false, true); // Yellow
  delay(delayTime);
  setColor(false, true, false); // Magenta
  delay(delayTime);
  setColor(true, false, false); // Cyan
  delay(delayTime);
}

void setup() {
  pinMode(pinRed, OUTPUT);
  pinMode(pinBlue, OUTPUT);
  pinMode(pinGreen, OUTPUT);
  setColor(true, true, true); // Turn off all LEDs (HIGH = OFF)

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println("IP address: " + WiFi.localIP().toString());
  server.begin();
}

void loop() {
  if (autoMode) {
    cycleColors();
  }

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/color") != -1) {
    autoMode = false; // Disable automatic mode
    if (request.indexOf("red=1&green=0&blue=1") != -1) setColor(false, true, true); // Red
    if (request.indexOf("red=1&green=1&blue=0") != -1) setColor(true, false, true); // Green
    if (request.indexOf("red=0&green=1&blue=1") != -1) setColor(true, true, false); // Blue
    if (request.indexOf("red=1&green=0&blue=0") != -1) setColor(false, false, true); // Yellow
    if (request.indexOf("red=0&green=1&blue=0") != -1) setColor(false, true, false); // Magenta
    if (request.indexOf("red=0&green=0&blue=1") != -1) setColor(true, false, false); // Cyan
  }

  if (request.indexOf("/auto") != -1) {
    autoMode = true; 
  }

  if (request.indexOf("/speed?value=") != -1) {
    int idx = request.indexOf("value=") + 6;
    delayTime = request.substring(idx, request.indexOf(' ', idx)).toInt();
    delayTime = constrain(delayTime, 100, 2000); 
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Control of LEDs</title>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }");
  client.println(".color-circle { display: inline-block; width: 50px; height: 50px; border-radius: 50%; margin: 10px; cursor: pointer; }");
  client.println(".button { display: inline-block; padding: 10px 20px; margin: 20px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }");
  client.println(".button:hover { background-color: #45a049; }");
  client.println(".slider { width: 80%; margin: 20px auto; }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>Control of LEDs</h1>");
  client.println("<p>Click on a color to display it:</p>");

  client.println("<div class=\"color-circle\" style=\"background-color:red;\" onclick=\"location.href='/color?red=1&green=0&blue=1'\"></div>");
  client.println("<div class=\"color-circle\" style=\"background-color:green;\" onclick=\"location.href='/color?red=1&green=1&blue=0'\"></div>");
  client.println("<div class=\"color-circle\" style=\"background-color:blue;\" onclick=\"location.href='/color?red=0&green=1&blue=1'\"></div>");
  client.println("<div class=\"color-circle\" style=\"background-color:yellow;\" onclick=\"location.href='/color?red=1&green=0&blue=0'\"></div>");
  client.println("<div class=\"color-circle\" style=\"background-color:magenta;\" onclick=\"location.href='/color?red=0&green=1&blue=0'\"></div>");
  client.println("<div class=\"color-circle\" style=\"background-color:cyan;\" onclick=\"location.href='/color?red=0&green=0&blue=1'\"></div>");

  client.println("<button class=\"button\" onclick=\"location.href='/auto'\">Cycle Colors</button>");

  client.println("<p>Adjust the speed of the cycle:</p>");
  client.println("<input class=\"slider\" type=\"range\" min=\"100\" max=\"2000\" value=\"" + String(delayTime) + "\" onchange=\"location.href='/speed?value=' + this.value\">");

  client.println("</body>");
  client.println("</html>");
}
