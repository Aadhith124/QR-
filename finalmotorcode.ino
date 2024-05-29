#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32Servo.h>
#include <Stepper.h>

// Replace with your network credentials
const char* ssid = "Ga";
const char* password = "12345678";

// Create servo objects
Servo servo1;
Servo servo2;

// Define servo pins
int servo1Pin = 13;
int servo2Pin = 14;

// Initial positions for the servos
int servo1Pos = 90;
int servo2Pos = 0;
int servo2OriginalPos = 0; // Store original position of servo2

// Stepper motor parameters
const int stepsPerRevolution = 204; // Change this according to your stepper motor
Stepper myStepper(stepsPerRevolution, 26, 27, 25, 33); // Pins: IN1, IN3, IN2, IN4

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    delay(100);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    // Attach the servos to the servo pins
    servo1.attach(servo1Pin);
    servo2.attach(servo2Pin);

    // Initialize servos to their initial positions
    servo1.write(servo1Pos);
    servo2.write(servo2Pos);

    // Initialize stepper motor
    myStepper.setSpeed(100); // Set stepper motor speed in RPM
    Serial.println("Stepper motor initialized");

    server.begin();
}

void loop() {
    // Continuous stepper motor control
    myStepper.step(1); // Step the stepper motor continuously

    // Handle HTTP requests from clients
    WiFiClient client = server.available();
    if (client) {
        Serial.println("New Client.");
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        // Send HTML page to client
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.println("<html><body><h1>ESP32 Servo and Stepper Control</h1>");
                        client.println("<form method='get' action='/setServos'>");
                        client.println("Input Value (1, 2, 3, or 4): <input type='number' name='inputValue' min='1' max='4'><br>");
                        client.println("<input type='submit' value='Submit'>");
                        client.println("</form></body></html>");
                        client.println();
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }

                // Check for input from the website
                if (currentLine.startsWith("GET /setServos?inputValue=")) {
                    int index = currentLine.indexOf('=') + 1;
                    int endIndex = currentLine.indexOf(' ', index);
                    String inputValue = currentLine.substring(index, endIndex);
                    int input = inputValue.toInt();
                    Serial.print("Input Value: ");
                    Serial.println(input);

                    // Move servos based on input value
                    if (input == 1) {
                        
                        servo1.write(0);
                        delay(100); // Wait for 0.1 seconds
                        servo1.write(90);
                        Serial.println("Moving Servo 1 to 45 degrees");
                    } else if (input == 2) {
                      
                        servo1.write(180); 
                        delay(100); // Wait for 0.1 seconds
                        servo1.write(90); // -45 degrees is represented as 135 degrees on the servo
                        Serial.println("Moving Servo 1 to -45 degrees");
                    } else if (input == 3) {
                        // Move servo2 to 130 degrees, wait 0.1 seconds, then move back
                        servo2OriginalPos = servo2Pos;
                        servo2.write(130);
                        Serial.println("Moving Servo 2 to 130 degrees");
                        delay(100); // Wait for 0.1 seconds
                        servo2.write(-130);
                        Serial.println("Moving Servo 2 back to original position");
                    } else if (input == 4) {
                        // Do nothing, stepper motor is controlled continuously
                    }
                }
            }
        }
        // Clear the header variable
        currentLine = "";
        client.stop();
        Serial.println("Client Disconnected.");
    }
}
