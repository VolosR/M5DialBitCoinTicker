
#include "M5Dial.h"
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets
#include <ArduinoJson.h>  //https://github.com/arduino-libraries/Arduino_JSON
#include "midleFont.h"
#include "tinyFont.h"
#include "font18.h"
#include "bigFont.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

#define small font18
#define big bigFont
#define midle midleFont

String ssid = "xxxxxxxx";
String password = "xxxxxxxxxxx";

const char* websocket_server = "iot.kwind.app";
const uint16_t websocket_port = 443;
const char* websocket_path = "/";
WebSocketsClient webSocket;

//......................................colors
#define backColor TFT_BLACK
#define gaugeColor 0x055D
#define dataColor 0x0311
#define purple 0xEA16
#define needleColor TFT_MAGENTA

//............................dont edit this
int cx=120;
int cy=120;
int r=120;
int ir=116;
int n=0;
int angle=0;
int bri=0;
int brightness[5]={20,40,80,160,250};


float x[360]; //outer points of Speed gaouges
float y[360];
float px[360]; //ineer point of Speed gaouges
float py[360];
float lx[360]; //text of Speed gaouges
float ly[360];
float nx[360]; //needle low of Speed gaouges
float ny[360];

float x2[360]; //outer points of Speed gaouges
float y2[360];
float px2[360]; //ineer point of Speed gaouges
float py2[360];


double rad=0.01745;
unsigned short color1;
unsigned short color2;
float sA;
float rA;
int blinkPeriod=500;
unsigned long currentTimeL=0;
unsigned long currentTimeR=0;


int deb1=0;
int deb2=0;
int debB=0;



//........................................................colors

unsigned short grays[13];
long oldPosition = -999;
// .........important variables

double bitCoinValue=0; //...speed variable 0-240
double alarmValue=100000;
float change=0;
String halvingDate="Mon, 17 Apr 2028";
int secondsPassed=0;
long timePassed=0;


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("Disconnected from WebSocket server");
            break;
        case WStype_CONNECTED:
            Serial.println("Connected to WebSocket server");
            delay(40);
            break;
        case WStype_TEXT: {
            Serial.printf("Message from server: %s\n", payload);
              
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

             bitCoinValue = doc["price"];
             change = doc["percent_change_24h"];
             secondsPassed=0;
             draw();
            break;
        }
        case WStype_BIN:
            Serial.println("Binary message received");
            break;
        case WStype_PING:
            Serial.println("Ping received");
            break;
        case WStype_PONG:
            Serial.println("Pong received");
            break;
        case WStype_ERROR:
            Serial.println("WebSocket Error");
            break;
    }
}


void setup() {

  Serial.begin(115200);
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, true);
  M5Dial.Display.setBrightness(brightness[bri]);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Can't connect to wifi!");
    }

    Serial.println("connected");

    webSocket.beginSSL(websocket_server, websocket_port, websocket_path);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000); // 5 seconds
 
    sprite.createSprite(240,240);
    sprite.setSwapBytes(true);
    sprite.setTextDatum(4);
    sprite.setTextColor(TFT_WHITE,backColor);
    sprite.setTextDatum(4);


  int a=120;
  for(int i=0;i<360;i++)
    {
       x[i]=((r-10)*cos(rad*a))+cx;
       y[i]=((r-10)*sin(rad*a))+cy;
       px[i]=((r-14)*cos(rad*a))+cx;
       py[i]=((r-14)*sin(rad*a))+cy;
       lx[i]=((r-28)*cos(rad*a))+cx;
       ly[i]=((r-28)*sin(rad*a))+cy;
       nx[i]=((r-36)*cos(rad*a))+cx;
       ny[i]=((r-36)*sin(rad*a))+cy;

       x2[i]=((r-50)*cos(rad*a))+cx;
       y2[i]=((r-50)*sin(rad*a))+cy;
       px2[i]=((r-62)*cos(rad*a))+cx;
       py2[i]=((r-62)*sin(rad*a))+cy;
    

       a++;
       if(a==360)
       a=0;
    }

      // generate 13 levels of gray
  int co = 210;
  for (int i = 0; i < 13; i++) {
    grays[i] = tft.color565(co, co, co);
    co = co - 20;
  }
     
}

int x1=0;

void draw()
{
  sprite.fillSprite(backColor);
  sprite.fillSmoothCircle(cx, cy, r+2, backColor);
  sprite.loadFont(small);
   
  sprite.drawSmoothArc(cx, cy, r, 117, 30, 30+secondsPassed, grays[6], backColor);
  sprite.drawSmoothArc(cx, cy, r-5, r-6, 30, 330, grays[4], backColor);
  sprite.drawSmoothArc(cx, cy, r-9, r-8, 270, 330, gaugeColor, backColor);
  sprite.drawSmoothArc(cx, cy, r-26, r-32, 30, 330, grays[9], backColor);
  sprite.drawSmoothArc(cx, cy, r-46, r-52, 30, 330, grays[9], backColor);
  

  for(int i=0;i<5;i++)
  if(i<=bri)
  sprite.drawSmoothArc(cx, cy, r, 114, (i*5), (i*5)+3, TFT_ORANGE, backColor);
  else
  sprite.drawSmoothArc(cx, cy, r, 114, (i*5), (i*5)+3, 0x5260, backColor);

  
  //.....................................................draw GAUGES
     for(int i=0;i<26;i++){
     if(i<20) {color1=grays[4]; color2=grays[4];} else {color1=gaugeColor; color2=gaugeColor;}

    

  if(i%2==0) {
  sprite.drawWedgeLine(x[i*12],y[i*12],px[i*12],py[i*12],3,1,color1);
  
  sprite.setTextColor(color2,backColor);
  sprite.drawString(String(i*5),lx[i*12],ly[i*12]);
  }else
  {sprite.drawWedgeLine(x[i*12],y[i*12],px[i*12],py[i*12],1,1,grays[4]);
  
  }
  }

  // ........................................needles draw
   sA=(bitCoinValue/1000)*2.4;
   rA=(alarmValue/1000)*2.4;
   sprite.drawWedgeLine(px[(int)sA],py[(int)sA],nx[(int)sA],ny[(int)sA],3,3,needleColor);
   sprite.drawWedgeLine(px[(int)rA],py[(int)rA],nx[(int)rA],ny[(int)rA],2,2,TFT_RED);


  //.....................................drawing  TEXT
  sprite.unloadFont();
  sprite.loadFont(midle);
  sprite.setTextColor(grays[5],backColor);
  sprite.drawString("USD",cx+30,cy+50);
  sprite.unloadFont();
  sprite.loadFont(big);
  sprite.setTextColor(grays[0],backColor);
  sprite.drawString(String((int)bitCoinValue),cx,cy+10);
  sprite.unloadFont();
  sprite.setTextColor(TFT_RED,backColor);
  sprite.drawString("CHANGE 24h",120,198);

  sprite.fillSmoothRoundRect(118,54,48,19,3,TFT_ORANGE,TFT_BLACK);
  
  sprite.loadFont(small);
  sprite.setTextColor(backColor,TFT_ORANGE);
  sprite.drawString("BitCoin",142,66,2);
   sprite.setTextColor(grays[5],backColor);
  sprite.drawString("$",174,166,2);
   sprite.setTextColor(grays[3],backColor);
  sprite.drawString(String(change)+" %",120,216,2);
  sprite.unloadFont();
  
  sprite.setTextColor(grays[3],backColor);
  sprite.setTextColor(gaugeColor,backColor);
  sprite.drawString(halvingDate,120,86,2);
  sprite.setTextColor(grays[5],backColor);
  sprite.drawString("HALVING:",88,69);
  sprite.setTextColor(grays[6],backColor);
  sprite.drawString("ALARM:",86,160);
  sprite.drawString(String((int)alarmValue),86,169);
  sprite.setTextColor(grays[8],backColor);
  sprite.drawString("VOLOS",144,230);

 
  
   //..............................................push Sprite to screen  
    M5Dial.Display.pushImage(0, 0, 240, 240, (uint16_t*)sprite.getPointer()); 
  
}


void loop() {

    webSocket.loop();
    M5Dial.update();

    if (M5Dial.BtnA.wasPressed())
    {
      bri++;
      if(bri==5)
      bri=0;
      M5Dial.Display.setBrightness(brightness[bri]);
    }

    if(millis()>timePassed+500)
    {
      timePassed=millis();
      if(secondsPassed<300)
      secondsPassed++;
      if(alarmValue<=bitCoinValue)
      M5Dial.Speaker.tone(4000, 200);
      draw();
    }

    
    long newPosition = M5Dial.Encoder.read();
    if (newPosition > oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        if(alarmValue<125000) alarmValue=alarmValue+1000;
        oldPosition = newPosition;
          draw();
    }
    if (newPosition < oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        if(alarmValue>0) alarmValue=alarmValue-1000;
        oldPosition = newPosition;
          draw();
    }

}

