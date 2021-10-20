#include <DHT.h>               // include Adafruit DHT library code
#define DHT11_PIN 2
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <Adafruit_GFX.h>       // include Adafruit graphics library
#include <Adafruit_ILI9341.h>   // include Adafruit ILI9341 TFT library
#include "mono.h"
#include "frame.h"

#define TFT_CS    D0     // TFT CS  pin is connected to NodeMCU pin D0
#define TFT_RST   D3     // TFT RST pin is connected to NodeMCU pin D3
#define TFT_DC    D4     // TFT DC  pin is connected to NodeMCU pin D4

#define GLCD_CL_BLACK 0x0000
#define GLCD_CL_WHITE 0xFFFF
#define GLCD_CL_GRAY 0x7BEF
#define GLCD_CL_LIGHT_GRAY 0xC618
#define GLCD_CL_GREEN 0x07E0
#define GLCD_CL_LIME 0x87E0
#define GLCD_CL_BLUE 0x001F
#define GLCD_CL_RED 0xF800
#define GLCD_CL_AQUA 0x5D1C
#define GLCD_CL_YELLOW 0xFFE0
#define GLCD_CL_MAGENTA 0xF81F
#define GLCD_CL_CYAN 0x07FF
#define GLCD_CL_DARK_CYAN 0x03EF
#define GLCD_CL_ORANGE 0xFCA0
#define GLCD_CL_PINK 0xF97F
#define GLCD_CL_BROWN 0x8200
#define GLCD_CL_VIOLET 0x9199
#define GLCD_CL_SILVER 0xA510
#define GLCD_CL_GOLD 0xA508
#define GLCD_CL_NAVY 0x000F
#define GLCD_CL_MAROON 0x7800
#define GLCD_CL_PURPLE 0x780F
#define GLCD_CL_OLIVE 0x7BE0

#define DHTPIN    D8           // DHT11 data pin is connected to NodeMCU pin D1 (GPIO5)
#define DHTTYPE DHT11        // DHT11 sensor is used

DHT dht11(DHTPIN, DHTTYPE);  // configure DHT library
// initialize ILI9341 TFT library with hardware SPI module
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ADS1015 ads; 

const char *ssid =  "SLT-4G_16DB1D";     // replace with your wifi ssid and wpa2 key
const char *pass =  "B552F6C9";

const char* mqttServer = "192.168.1.101";
const int mqttPort = 1883;

const int AirValue = 842;   //you need to replace this value with Value_1
const int WaterValue = 445;  //you need to replace this value with Value_2
int intervals = (AirValue - WaterValue)/3;
int soilMoistureValue = 0;

//WiFiClient client;

int16_t adc0, adc1, adc2, adc3;

void callback(char* topic, byte* payload, unsigned int length) {
   Serial.print("Message arrived in topic: ");
   Serial.println(topic);

   Serial.print("message");
   for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }

  Serial.println();
  Serial.println("--------------");
}

WiFiClient espClient;
PubSubClient client(mqttServer, mqttPort, callback, espClient);


void setup(void) {
  // put your setup code here, to run once:

  Serial.begin(9600);
  tft.begin();
  ads.setGain(GAIN_ONE); 
  ads.begin();
  dht11.begin();

  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.drawFastHLine(0, 222,  tft.width(), ILI9341_BLUE);   // draw horizontal blue line at position (0, 50)
  tft.drawFastHLine(0, 150,  tft.width(), ILI9341_BLUE);  // draw horizontal blue line at position (0, 102)

  int space = 20;
  for(int i = 0; i < 5; i++)
  {
    delay(2);
      int h = 40,w = 40, row, col, buffidx=0;
      for (row=0; row<h; row++) { // For each scanline...
        for (col=0; col<w; col++) { // For each pixel...
          //To read from Flash Memory, pgm_read_XXX is required.
          //Since image is stored as uint16_t, pgm_read_word is used as it uses 16bit address
          tft.drawPixel(col + space, row + 160, pgm_read_word(main_icon[i] + buffidx));
          buffidx++;
        } // end pixel
      }
     space = space + 60;
  }
  
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(50);
      tft.drawBitmap(0,225,wifi_icon , 20, 15, ILI9341_RED);
  }
  Serial.println("");
  Serial.println("Wi-Fi connected"); 
      
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
    while (!client.connected()) {
      Serial.println("connecting to MQTT...");
      tft.drawRect(30,225, 13, 13, ILI9341_RED);
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.setCursor(34, 228);  
      tft.print("M");

      if (client.connect("ESP8266Client")) {
        client.subscribe("LED");
        Serial.println("connected");  
      } else {
        Serial.println("failed with state ");
        Serial.print(client.state());
        delay(2000);  
      }
  }

  tft.drawBitmap(0,225,wifi_icon , 20, 15, ILI9341_WHITE);
  tft.drawRect(30,225, 13, 13, ILI9341_GREEN);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(34, 228);  
  tft.print("M");


//
//  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
//  tft.setCursor(25, 5);  
//  tft.print("MQTT");
//  
//  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);     // set text color to green with black background
//  tft.setCursor(25, 61);              // move cursor to position (25, 61) pixel
//  tft.print("TEMPERATURE =");
//  
//  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);  // set text color to yellow with black background
//  tft.setCursor(25, 113);              // move cursor to position (34, 113) pixel
//  tft.print("HUMIDITY =");
//
//  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);  // set text color to yellow with black background
//  tft.setCursor(25, 165);              // move cursor to position (34, 113) pixel
//  tft.print("UV LEVEL =");
//
//  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);     // set text color to green with black background
//  tft.setCursor(25, 217);              // move cursor to position (25, 61) pixel
//  tft.print("SOIL MOISTURE =");
//
//  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);  // set text color to yellow with black background
//  tft.setCursor(25, 268);              // move cursor to position (34, 113) pixel
//  tft.print("Water Level =");
//  
//  tft.setTextSize(2);                     // text size = 2
//  tft.drawCircle(83, 80, 2, ILI9341_RED);  // print degree symbol ( ° )
//  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);  // set text color to red with black background
//  tft.setCursor(89, 78);
//  tft.print("C");

}

void loop(void) {

  // put your main code here, to run repeatedly:
  
  client.loop();
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);

  // read humidity in rH%
  int Humi = dht11.readHumidity() * 10;
  // read temperature in degree Celsius
  int Temp = dht11.readTemperature() * 10;

  int SoilMoisture = analogRead(A0);
  int percentageSoilMoisture = map(SoilMoisture, WaterValue, AirValue, 100, 0);
  int UvLevel = adc2;
  int WaterLevel = adc0;

  String toSend = String(Temp) + "," + String(Humi);
  client.publish("data",toSend.c_str());
  // print humidity (in %)
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);  // set text color to cyan with black background
  tft.setCursor(22, 210);
  tft.printf("%02u.%1u %%", (Humi/10)%100, Humi % 10);

  
  // print temperature (in °C)
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);  // set text color to red with black background
  tft.setCursor(82, 210);
  if(Temp < 0)    // if temperature < 0
    tft.printf("-%02u.%1u", (abs(Temp)/10)%100, abs(Temp) % 10);
  else            // temperature >= 0                   // text size = 2
    tft.printf(" %02u.%1u", (Temp/10)%100, Temp % 10);
    tft.drawCircle(116, 210, 1, ILI9341_RED);  // print degree symbol ( ° )
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);  // set text color to red with black background
    tft.setCursor(122, 210);
    tft.print("C");


  // print Water level (in %)
  tft.setTextColor(GLCD_CL_AQUA, ILI9341_BLACK);  // set text color to cyan with black background
  tft.setCursor(142, 210);
  tft.printf("%02u.%1u%%", (WaterLevel/10)%100, WaterLevel % 10);

  // print UV (in %)
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);  // set text color to cyan with black background
  tft.setCursor(203, 210);
  tft.printf("%02u.%1u%%", (UvLevel/10)%100, UvLevel % 10);

  // print Moisture (in %)
  tft.setTextColor(GLCD_CL_BROWN, ILI9341_BLACK);  // set text color to cyan with black background
  tft.setCursor(270, 210);
  tft.printf("%d%%", percentageSoilMoisture);

//  if(SoilMoisture > WaterValue && SoilMoisture < (WaterValue + intervals))
//  {
//    //Serial.println("Very Wet");
//  }
//  else if(SoilMoisture > (WaterValue + intervals) && SoilMoisture < (AirValue - intervals))
//  {
//    //Serial.println("Wet");
//  }
//  else if(SoilMoisture < AirValue && SoilMoisture > (AirValue - intervals))
//  {
//    //Serial.println("Dry");
//  }

  if(WaterLevel/10 < 10){
    face(4);  
  }else{
    face(0);  
  }

  delay(1000);

}

void face(int number){
 for(int i = number; i < number + 4; i++)
  {
    delay(20);
    int h = 141,w = 250, row, col, buffidx=0;
      for (row=0; row<h; row++) { // For each scanline...
        for (col=0; col<w; col++) { // For each pixel...
          //To read from Flash Memory, pgm_read_XXX is required.
          //Since image is stored as uint16_t, pgm_read_word is used as it uses 16bit address
          tft.drawPixel(col + 35, row, pgm_read_word(evive_in_hand[i] + buffidx));
          buffidx++;
        } // end pixel
      }
  }
}

//unsigned long testText() {
//
//  tft.fillScreen(ILI9341_BLACK);
//  unsigned long start = micros();
//  tft.setCursor(0, 0);
//  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
//  tft.println("Hello World!");
//  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
//  tft.println(1234.56);
//  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
//  tft.println(0xDEADBEEF, HEX);
//  tft.println();
//  tft.setTextColor(ILI9341_GREEN);
//  tft.setTextSize(5);
//  tft.println("Groop");
//  tft.setTextSize(2);
//  tft.println("I implore thee,");
//  tft.setTextSize(1);
//  tft.println("my foonting turlingdromes.");
//  tft.println("And hooptiously drangle me");
//  tft.println("with crinkly bindlewurdles,");
//  tft.println("Or I will rend thee");
//  tft.println("in the gobberwarts");
//  tft.println("with my blurglecruncheon,");
//  tft.println("see if I don't!");
//  return micros() - start;
//}
