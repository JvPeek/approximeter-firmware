
#include "main.h"

#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
// #include <Arduino.h>
// #include <ArduinoOTA.h>
// #include <ESPmDNS.h>
#include <SPI.h>
// #include <WebServer.h>
//  #include <WiFi.h>
//  #include <WiFiClient.h>
//  #include <WiFiUdp.h>
#include <Wire.h>
#include <esp_bt.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>

#include "credentials.h"

TaskHandle_t DataTask;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int i;
int sign;
long value;
unsigned long tempmicros;
unsigned long ipmillis;
bool wlan_on = true;
#define ledPin 47
#define clockpin 2
#define datapin 4
#define VCC_DISPLAY 17
#define WLAN true
#define Button 0

void decode() {
    sign = 1;
    value = 0;

    for (i = 0; i < 23; i++) {
        while (digitalRead(clockpin) == HIGH) {
        }  // wait until clock returns to HIGH- the first bit is not needed

        while (digitalRead(clockpin) == LOW) {
        }  // wait until clock returns to LOW
        if (digitalRead(datapin) == LOW) {
            if (i < 20) {
                value |= 1 << i;
            }
            if (i == 20) {
                sign = -1;
            }
        }
    }
    result = (value * sign) / 100.00;
}

void readData(void* parameter) {
    SupplyCalip = analogRead(10) / 4096.0 * 3.3;
    VoltageBattery = analogRead(9) * 2.0 / 4096.0 * 3.3;

    int not_sampled = 1;
    while (not_sampled) {
        tempmicros = micros();
        while ((digitalRead(clockpin) == HIGH)) {
        }
        // if clock is LOW wait until it turns to HIGH
        tempmicros = micros();
        while ((digitalRead(clockpin) == LOW)) {
        }  // wait for the end of the HIGH pulse
        if ((micros() - tempmicros) >
            500) {     // if the HIGH pulse was longer than 500 micros we are
                       // at the start of a new bit sequence
            decode();  // decode the bit sequence
            not_sampled = 0;
        }
    }
}
void iconBar(Adafruit_SSD1306 display) {}
void mainDisplay(void* param) {
    TwoWire I2C_1 = TwoWire(0);
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_1, OLED_RESET);
    I2C_1.begin(11, 12);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;  // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.drawBitmap(0, 0, bootlogo, 128, 32, 1);
    display.display();

    while (TRVE) {
        // bool wifiConnected = WiFi.status() == WL_CONNECTED;

        display.clearDisplay();
        display.setTextSize(1);  // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.print("CLP:");
        display.print(SupplyCalip);
        display.print(" BAT:");
        display.println(VoltageBattery);
        display.setTextSize(3);  // Draw 3X-scale text
        display.print(result);
        display.setTextSize(1);  // Draw 3X-scale text
        display.println("mm");
        pixels.setPixelColor(0, pixels.Color(255, 0, 255));
        // pixels.setPixelColor(
        //     0, wifiConnected ? pixels.Color(255, 255, 255) : pixels.Color(0,
        //     0, 0));
        if (result > 0) {
            display.fillRect(0, 30, map(result, 0, 157, 0, 127), 31, WHITE);
        }

        display.display();
    }
}
void servicesWorker(void * params) {
    while (TRVE) {
    }
}
void setup() {
    delay(500);
    Serial.begin(115200);
    Serial.println("boot");

    pinMode(clockpin, INPUT_PULLUP);
    pinMode(datapin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    pinMode(VCC_DISPLAY, OUTPUT);
    digitalWrite(VCC_DISPLAY, HIGH);
    digitalWrite(ledPin, LOW);
    pinMode(Button, INPUT_PULLUP);

    float Voltage = 1.5;
    analogWrite(
        21,
        (int)(Voltage * 255 /
              3.24));  // DO NOT CHANGE. EVER. BREAKS HARDWARE AND HEARTS!
                       // And also kills a random kitten somewhere!
    delay(500);

    Serial.println("boot done");
    if (!digitalRead(Button)) {
        while (TRVE) {
        }
    }
    delay(500);

    xTaskCreatePinnedToCore(mainDisplay, /* Function to implement the task */
                            "Display",   /* Name of the task */
                            40000,       /* Stack size in words */
                            NULL,        /* Task input parameter */
                            1,           /* Priority of the task */
                            &DataTask, 0);

    xTaskCreate(servicesWorker, /* Function to implement the task */
                "Services",     /* Name of the task */
                10000,          /* Stack size in words */
                NULL,           /* Task input parameter */
                0,              /* Priority of the task */
                NULL);
}

void loop() { readData(NULL); }