/*
  Project: Toggle Button Digital Weather Station (DHT11 & I2C LCD)
  Author: Wired Wanderer (YouTube Channel)
  Date: July 25, 2025
  Description: Reads temperature and humidity from a DHT11 sensor and displays it on a 16x2 I2C LCD.
               The display is toggled ON/OFF with a single push button press.
               Each time it turns ON, it shows a "Welcome to Wired Wanderer" message first.
  This code is part of the "Simple Arduino Projects" playlist on the Wired Wanderer YouTube channel.
  Find the video tutorial here: [Link to your YouTube video once uploaded]

  Libraries Required:
  1. DHT sensor library by Adafruit (Install via Arduino IDE Library Manager)
  2. LiquidCrystal I2C library (e.g., by Frank de Brabander or Marco Schwartz - Install via Arduino IDE Library Manager)
*/

// Include necessary libraries
#include <Wire.h> // Required for I2C communication
#include <LiquidCrystal_I2C.h> // Library for I2C LCD
#include <DHT.h> // Library for DHT sensor

// --- Define DHT Sensor Connections ---
#define DHTPIN 2     // Digital pin where the DHT11 sensor's data pin is connected
#define DHTTYPE DHT11 // Type of DHT sensor used (DHT11 or DHT22)

// --- Define Button Connection and Debounce ---
#define BUTTON_PIN 7 // Digital pin where the push button is connected
const long debounceDelay = 50; // The debounce time; increase if button is "bouncy"
int buttonState;               // Current reading from the input pin
int lastButtonState = HIGH;    // Previous reading from the input pin (HIGH because of INPUT_PULLUP)
unsigned long lastDebounceTime = 0; // The last time the output pin was toggled

// --- Initialize DHT sensor ---
DHT dht(DHTPIN, DHTTYPE);

// --- Initialize the LCD ---
// Set the LCD I2C address, columns, and rows
// Common I2C addresses are 0x27 or 0x3F. Check your specific LCD module.
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16x2 LCD

// --- Project State Variables ---
boolean displayOn = false; // Tracks whether the display is currently ON or OFF

void setup() {
  Serial.begin(9600); // Start serial communication for debugging (optional)
  Serial.println("Starting Wired Wanderer Weather Station (Toggle Button)...");

  // Set up the button pin as an input with internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button connected to GND, so it reads LOW when pressed

  // Initialize LCD but keep it off initially
  lcd.init();
  lcd.noBacklight(); // Start with backlight off
  lcd.noDisplay();   // Start with display content off

  dht.begin(); // Start the DHT sensor (it runs in the background)
}

void loop() {
  // Read the state of the button
  int reading = digitalRead(BUTTON_PIN);

  // If the button state has changed (from last reading)
  if (reading != lastButtonState) {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }

  // If the debounce time has passed and the button state is stable
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state is stable AND it's a new press (i.e., went from HIGH to LOW)
    if (reading != buttonState) {
      buttonState = reading; // Update the current button state

      // If the button is now pressed (LOW, because of INPUT_PULLUP)
      if (buttonState == LOW) {
        displayOn = !displayOn; // Toggle the display state (true to false, false to true)

        if (displayOn) {
          // If turning ON, show welcome message first
          showWelcomeMessage();
        } else {
          // If turning OFF, clear and turn off display
          lcd.clear();
          lcd.noBacklight();
          lcd.noDisplay();
        }
      }
    }
  }
  // Save the current reading for the next loop iteration
  lastButtonState = reading;


  // --- Logic for when display is ON ---
  if (displayOn) {
    // Ensure display is on (in case it was turned off by error or first loop iteration)
    lcd.backlight();
    lcd.display();

    // Read and display sensor data
    readAndDisplayDHT();
  }

  // Small delay for the loop to run smoothly
  delay(10);
}

// --- Function to display welcome message ---
void showWelcomeMessage() {
  lcd.clear();
  lcd.backlight(); // Ensure backlight is on
  lcd.display();   // Ensure display is on

  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("Wired Wanderer");
  delay(4000); // Display welcome for 4 seconds
  lcd.clear(); // Clear after welcome
}

// --- Function to read DHT and display on LCD ---
void readAndDisplayDHT() {
  // Wait a bit between measurements if continuously ON
  // No need for a large delay here, as loop() handles general timing
  // or we read fast while button is ON
  // DHT11 sensors need at least 1 second between readings.
  static unsigned long lastDHTReadTime = 0;
  const long DHT_READ_INTERVAL = 3000; // Read every 3 seconds

  if (millis() - lastDHTReadTime >= DHT_READ_INTERVAL) {
    lastDHTReadTime = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      lcd.clear();
      lcd.print("DHT Error!");
      lcd.setCursor(0, 1);
      lcd.print("Check wiring");
    } else {
      // --- Display on Serial Monitor ---
      Serial.print("Humidity: "); Serial.print(h); Serial.print(" %\t");
      Serial.print("Temperature: "); Serial.print(t); Serial.println(" *C");

      // --- Display on LCD ---
      lcd.clear(); // Clear to prevent ghosting or partial updates

      // Display Temperature
      lcd.setCursor(0, 0); // First line, first column
      lcd.print("Temp: ");
      lcd.print(t);
      lcd.print((char)223); // Degree symbol
      lcd.print("C");

      // Display Humidity
      lcd.setCursor(0, 1); // Second line, first column
      lcd.print("Humi: ");
      lcd.print(h);
      lcd.print(" %");
    }
  }
}