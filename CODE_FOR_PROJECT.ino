#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define soundSensorPin A0    // Connect the analog output of the sound sensor to A0
#define pulseSensorPin A1    // Connect the pulse sensor to A1
#define buzzerPin 8          // Buzzer connected to pin 8

SoftwareSerial sim800(10, 11); // RX, TX for SIM800A GSM module
SoftwareSerial gpsSerial(3, 4); // RX, TX for GPS module
TinyGPSPlus gps;

#define SOUND_THRESHOLD 45    // Set the threshold value to 45 for sound detection
#define HEART_RATE_THRESHOLD 100  // Set heart rate threshold
const char phoneNumber[] = "+919514370059"; // Replace with the phone number to send SMS

void setup() {
  Serial.begin(9600);          // Initialize Serial Monitor
  sim800.begin(9600);          // Initialize SIM800A communication
  gpsSerial.begin(9600);       // Initialize GPS module communication

  pinMode(soundSensorPin, INPUT);  // Set the sound sensor pin as input
  pinMode(pulseSensorPin, INPUT);  // Set the pulse sensor pin as input
  pinMode(buzzerPin, OUTPUT);      // Set the buzzer pin as output

  delay(1000);
  Serial.println("System initialized. Monitoring for abnormal conditions...");
}

void loop() {
  int soundLevel = analogRead(soundSensorPin);  // Read sound level from sensor
  int heartRate = analogRead(pulseSensorPin);   // Read heart rate from pulse sensor

  Serial.print("Sound Level: ");
  Serial.println(soundLevel);
  Serial.print("Heart Rate: ");
  Serial.println(heartRate);

  // If sound level exceeds threshold or heart rate exceeds threshold, trigger alert
  if (soundLevel > SOUND_THRESHOLD || heartRate > HEART_RATE_THRESHOLD) {
    Serial.println("Abnormal condition detected!");
    digitalWrite(buzzerPin, HIGH); // Activate buzzer
    delay(1000);
    digitalWrite(buzzerPin, LOW);  // Deactivate buzzer

    String gpsLocation = getGPSLocation();  // Get GPS location
    if (gpsLocation.length() > 0) {         // Check if the GPS location is valid
      sendSMS(phoneNumber, "SOS\nGPS Location: " + gpsLocation);
    } else {
      sendSMS(phoneNumber, "SOS\nGPS Location: Not available.");
    }
    delay(10000); // Wait before rechecking
  }

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());  // Process GPS data
  }

  delay(500);  // Small delay to prevent overloading the loop
}

// Function to get GPS location in Google Maps link format
String getGPSLocation() {
  String gpsLink = "";
  unsigned long timeout = millis() + 30000; // 30-second timeout to get GPS fix

  while (millis() < timeout) {
    if (gps.location.isValid()) {
      double latitude = gps.location.lat();
      double longitude = gps.location.lng();
      gpsLink = "https://www.google.com/maps?q=" + String(latitude, 6) + "," + String(longitude, 6);
      Serial.println("GPS Location acquired: " + gpsLink);
      break;
    }

    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
  }

  if (gpsLink.length() == 0) {
    Serial.println("Failed to acquire GPS location within timeout.");
  }

  return gpsLink;
}

// Function to send SMS with the provided message to the specified phone number
void sendSMS(const char *phoneNumber, String message) {
  Serial.println("Sending SMS...");
  sim800.println("AT");              // Test communication with SIM800A
  delay(1000);
  sim800.println("AT+CMGF=1");       // Set SMS mode to text
  delay(1000);
  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);         // Provide the recipient's phone number
  sim800.println("\"");
  delay(1000);
  sim800.print(message);             // Send the message
  sim800.write(26);                  // Ctrl+Z to send the message
  delay(5000);                       // Wait for the message to be sent
  Serial.println("SMS sent: " + message);
}
