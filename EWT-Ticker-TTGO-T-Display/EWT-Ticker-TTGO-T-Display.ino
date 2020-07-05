/*
 * Energy Web Token (EWT) Price Ticker Display for TTGO T-Display
 * This library does not require an API key due to hacing an embedded 
 * CoinMarketCap root certificate and consuming secured HTTPS RESTful APIs.
 * 
*/

// libraries
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
 
// images
#include "alert.h"
#include "info.h"
#include "ew_logo.h"
#include "ewt_symbol.h"

// network config
#define wifi_ssid "YOUR-WIFI-SSID"
#define wifi_password "YOUR-WIFI-PASSWORD"



WiFiClientSecure client;
WiFiClient espClient2;

unsigned long api_mtbs = 60000;
unsigned long api_due_time = 0;

// CMC Root CA_Cert (Expires: Tuesday, 13. May 2025)
const char* cmc_root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
"-----END CERTIFICATE-----\n";


// SPI TFT settings edit UserSetup !!!
#include <TFT_eSPI.h>
#include <SPI.h>

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI        19
#define TFT_SCLK        18
#define TFT_CS          5
#define TFT_DC          16
#define TFT_RST         23

#define TFT_BL          4  // Display backlight control pin
#define ADC_EN          14
#define ADC_PIN         34
#define BUTTON_1        35
#define BUTTON_2        0

TFT_eSPI tft = TFT_eSPI(135, 240);

// supportet colors
#define ST7735_BLACK   0x0000
#define ST7735_GRAY    0x8410
#define ST7735_WHITE   0xFFFF
#define ST7735_RED     0xF800
#define ST7735_ORANGE  0xEC80
#define ST7735_YELLOW  0xFFE0
#define ST7735_LIME    0x07FF
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_AQUA    0x04FF
#define ST7735_BLUE    0x001F
#define ST7735_MAGENTA 0xF81F
#define ST7735_PINK    0xF8FF
#define ST7735_CHYN    0x2D05


// begin setup
void setup() {
  Serial.begin(115200);
  Serial.println(F("Boot Ticker"));


  uint16_t time = millis();
  tft.setRotation(1);  //rotate 90 degree
  tft.fillScreen(ST7735_BLACK);
  time = millis() - time;

  if (TFT_BL > 0) {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.fillScreen(ST7735_BLACK);
    tft.setSwapBytes(true);
    tft.pushImage(0, 0, ew_logo_Width, ew_logo_Height, ew_logo);
    delay(3000);
  }


  // Starup

  tft.fillRect(0, 0, 240, 135, ST7735_BLACK);
  Serial.print("Connecting to ");
  tft.drawString("Connecting to ", 15, 10, 2);;
  Serial.println(wifi_ssid);
  tft.drawString(wifi_ssid, 15, 25, 2);
  tft.pushImage(200, 2, infoWidth, infoHeight, info);
  delay(1000);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  tft.setTextColor(ST7735_GREEN);
  tft.drawString("WiFi connected", 15, 40, 2);
  tft.setTextColor(ST7735_WHITE);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
  tft.fillRect(0, 0, 240, 135, ST7735_BLACK);

  tft.pushImage(0, 0, ewt_symbol_Width, ewt_symbol_Height, ewt_symbol);
}


void printTickerData() {
  Serial.println("----------------------------");
  Serial.println("Getting ticker data for EWT");
  
  HTTPClient http;
 
    http.begin("https://web-api.coinmarketcap.com/v1/cryptocurrency/quotes/latest?id=5268", cmc_root_ca);
    int httpCode = http.GET();
 
    if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();        
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        JsonObject responseObject = doc.as<JsonObject>();
        
        String cmc_rank = responseObject["data"]["5268"]["cmc_rank"];
        double price_usd = responseObject["data"]["5268"]["quote"]["USD"]["price"];
        int volume_24h = responseObject["data"]["5268"]["quote"]["USD"]["volume_24h"];
        double percent_change_1h = responseObject["data"]["5268"]["quote"]["USD"]["percent_change_1h"];
        double percent_change_24h = responseObject["data"]["5268"]["quote"]["USD"]["percent_change_24h"];
        double percent_change_7d = responseObject["data"]["5268"]["quote"]["USD"]["percent_change_7d"];
        String last_updated = responseObject["data"]["5268"]["quote"]["USD"]["last_updated"];
        Serial.println(cmc_rank);
        Serial.println(price_usd);               
        Serial.println(volume_24h);         
        Serial.println(percent_change_1h);            
        Serial.println(percent_change_24h);            
        Serial.println(percent_change_7d);            
        Serial.println(last_updated);              
        
        
        tft.setTextColor(ST7735_GRAY);
        tft.drawString("USD Price", 110, 11, 2);
        
        tft.setTextColor(ST7735_WHITE);
        
        tft.fillRect(117, 37, 123, 38, ST7735_BLACK); //price
        tft.fillRect(185, 80, 55, 20, ST7735_BLACK); //rank       
        
        
        tft.setTextColor(ST7735_YELLOW);
        
        if(percent_change_1h < 0){
        tft.setTextColor(ST7735_RED);
        }
        if(percent_change_1h > 0){
        tft.setTextColor(ST7735_GREEN);
        }
        
        
        tft.drawString(String(price_usd), 105, 33, 6);
        
        tft.setTextColor(ST7735_AQUA);
        tft.drawString("Volume: ", 105, 80, 2);        
        tft.drawString(String(volume_24h).c_str(), 160, 80, 2);
        
        tft.drawLine(11, 103, 229, 103, ST7735_GRAY);        
                
        // hours change
        tft.fillRect(100, 110, 140, 25, ST7735_BLACK);    
        tft.setTextColor(ST7735_YELLOW);
        
        if(percent_change_1h < 0){
        tft.setTextColor(ST7735_RED);
        }
        if(percent_change_1h > 0){
        tft.setTextColor(ST7735_GREEN);
        }
        tft.drawString("% Price 1h: ", 11, 110, 4);
        tft.drawString(String(percent_change_1h), 156, 110, 4);
        delay(20000);
        
        
        // 24 hours change
        tft.fillRect(100, 110, 140, 25, ST7735_BLACK);    
        tft.setTextColor(ST7735_YELLOW);
        
        if(percent_change_24h < 0){
        tft.setTextColor(ST7735_RED);
        }
        if(percent_change_24h > 0){
        tft.setTextColor(ST7735_GREEN);
        }
        tft.drawString("% Price 24h: ", 11, 110, 4);
        tft.drawString(String(percent_change_24h), 156, 110, 4);
        delay(20000);
        
        
        // 7d hours change
        tft.fillRect(100, 110, 140, 25, ST7735_BLACK);    
        tft.setTextColor(ST7735_YELLOW);
        
        if(percent_change_7d < 0){
          tft.setTextColor(ST7735_RED);
        }
        if(percent_change_7d > 0){
          tft.setTextColor(ST7735_GREEN);
        }
        tft.drawString("% Price 7d:", 11, 110, 4);
        tft.drawString(String(percent_change_7d), 156, 110, 4);
        delay(20000);
      }
 
    else {
      Serial.print("Error getting data: ");      
      tft.fillRect(200, 2, 40, 32, ST7735_BLACK);
      tft.pushImage(203, 2, alertWidth, alertHeight, alert);
    }
    
    Serial.println("----------------------------");
    http.end();
  
}


float RSSI = 0.0;

void loop() {
  unsigned long timeNow = millis();
  if ((timeNow > api_due_time))  {
    // int signal bars
    Serial.print("WiFi Signal strength: ");
    Serial.print(WiFi.RSSI());
    tft.fillRect(200, 2, 40, 32, ST7735_BLACK);


    int bars;
    RSSI = WiFi.RSSI();

    if (RSSI >= -55) {
      bars = 5;
      Serial.println(" 5 bars");
    } else if (RSSI < -55 & RSSI >= -65) {
      bars = 4;
      Serial.println(" 4 bars");
    } else if (RSSI < -65 & RSSI >= -70) {
      bars = 3;
      Serial.println(" 3 bars");
    } else if (RSSI < -70 & RSSI >= -78) {
      bars = 2;
      Serial.println(" 2 bars");
    } else if (RSSI < -78 & RSSI >= -82) {
      bars = 1;
      Serial.println(" 1 bars");
    } else {
      bars = 0;
      Serial.println(" 0 bars");
    }

    // signal bars
    for (int b = 0; b <= bars; b++) {
      tft.fillRect(202 + (b * 6), 23 - (b * 4), 5, b * 4, ST7735_GRAY);
    }
    
    printTickerData();

  }

}