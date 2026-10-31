#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

// ==================== PINS ====================
#define TFT_CS    15
#define TFT_RST    4
#define TFT_DC     2
#define BUZZER_PIN 14
#define RGB_R      32
#define RGB_G      33
#define RGB_B      25

// ==================== WIFI ====================
const char* AP_SSID = "RobotFace";
const char* AP_PASS = "robot1234";
WebServer server(80);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ==================== EYE CONFIG ====================
int eyeW = 30;
int eyeH = 30;
int eyeR = 4;
int eyeG = 8;

// ==================== STATE ====================
enum Mode { MODE_IDLE, MODE_RANDOM, MODE_PARTY, MODE_HYPNO,
            MODE_HEARTBEAT, MODE_DISCO, MODE_ALERT, MODE_RAINBOW, MODE_TALK };

Mode  currentMode   = MODE_IDLE;
int   currentExpr   = 0;
int   rgbR=0, rgbG=180, rgbB=180;
int   rgbBrightness = 100;
bool  buzzerEnabled = true;
unsigned long lastAuto  = 0;
unsigned long bootTime  = 0;
unsigned long exprCount = 0;
unsigned long msgCount  = 0;
int   connectedClients  = 0;

uint16_t eyeColor = 0x07FF; // cyan

char robotName[24] = "UNIT-01";

String  bubbleText   = "";
bool    bubbleActive = false;
unsigned long bubbleUntil = 0;

// ==================== RGB ====================
void setRGB(int r, int g, int b) {
  float s=rgbBrightness/100.0;
  analogWrite(RGB_R,(int)(r*s)); analogWrite(RGB_G,(int)(g*s)); analogWrite(RGB_B,(int)(b*s));
}
void setRGBRaw(int r, int g, int b) {
  analogWrite(RGB_R,r); analogWrite(RGB_G,g); analogWrite(RGB_B,b);
}

// ==================== BUZZER ====================
void beep(int freq, int dur) { if(buzzerEnabled) tone(BUZZER_PIN,freq,dur); }

void playMelody(int id) {
  if(!buzzerEnabled) return;
  switch(id){
    case 0:{int n[]={2637,3136,3520,3951,4186};for(int i=0;i<5;i++){tone(BUZZER_PIN,n[i],100);delay(130);}break;}
    case 1:{int n[]={3136,3136,0,3136,0,2637,3136,0,4186,0,0,0,2093};
            int d[]={150,150,150,150,150,150,150,150,300,150,150,150,300};
            for(int i=0;i<13;i++){if(n[i])tone(BUZZER_PIN,n[i],d[i]);else noTone(BUZZER_PIN);delay(d[i]+20);}break;}
    case 2:{int n[]={2093,1976,1760,1568};for(int i=0;i<4;i++){tone(BUZZER_PIN,n[i],300);delay(350);}break;}
    case 3:{for(int i=0;i<4;i++){tone(BUZZER_PIN,4500,80);delay(120);tone(BUZZER_PIN,3000,80);delay(120);}break;}
    case 4:{int n[]={3500,4000,4500,4000,4500};for(int i=0;i<5;i++){tone(BUZZER_PIN,n[i],60);delay(80);}break;}
    case 5:{for(int i=0;i<8;i++){tone(BUZZER_PIN,2000+random(2500),40+random(80));delay(60+random(60));}break;}
  }
}

// ==================== SPEECH BUBBLE ====================
void drawEyesBase(int shiftX, int shiftY, int hLeft, int hRight, int eyebrowAngle, uint16_t color);

void showBubble(String text, int durationMs) {
  bubbleText=text; bubbleActive=true; bubbleUntil=millis()+durationMs; msgCount++;
  int bx=4,by=2,bw=120,bh=22;
  tft.fillRoundRect(bx,by,bw,bh,4,ST7735_WHITE);
  tft.drawRoundRect(bx,by,bw,bh,4,0x4208);
  tft.fillTriangle(14,by+bh,20,by+bh,14,by+bh+6,ST7735_WHITE);
  tft.drawLine(14,by+bh+6,14,by+bh,0x4208);
  tft.drawLine(14,by+bh+6,20,by+bh,0x4208);
  tft.setTextColor(ST7735_BLACK); tft.setTextSize(1);
  String display=text.substring(0,min((int)text.length(),18));
  for(int i=1;i<=(int)display.length();i++){
    tft.fillRect(bx+3,by+7,bw-6,10,ST7735_WHITE);
    tft.setCursor(bx+4,by+8); tft.print(display.substring(0,i)); delay(45);
  }
}

void clearBubble() { bubbleActive=false; drawEyesBase(0,0,eyeH,eyeH,0,eyeColor); }

// ==================== CORE EYE DRAW ====================
void drawEyesBase(int shiftX, int shiftY, int hLeft, int hRight, int eyebrowAngle, uint16_t color) {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1); tft.setTextColor(tft.color565(60,60,60));
  int nx=64-(strlen(robotName)*3); tft.setCursor(nx,2); tft.print(robotName);

  int cx=(tft.width()/2)+shiftX, cy=(tft.height()/2)+shiftY;
  int leftX=cx-eyeW-(eyeG/2), leftY=cy-(hLeft/2);
  int rightX=cx+(eyeG/2),     rightY=cy-(hRight/2);

  if(hLeft>2)  tft.fillRoundRect(leftX,leftY,eyeW,hLeft,eyeR,color);
  else         tft.drawFastHLine(leftX,cy,eyeW,color);
  if(hRight>2) tft.fillRoundRect(rightX,rightY,eyeW,hRight,eyeR,color);
  else         tft.drawFastHLine(rightX,cy,eyeW,color);

  if(eyebrowAngle>0){
    tft.fillTriangle(leftX-2,leftY-2,leftX+eyeW+2,leftY-2,leftX+eyeW+2,leftY+eyebrowAngle,ST7735_BLACK);
    tft.fillTriangle(rightX-2,rightY-2,rightX+eyeW+2,rightY-2,rightX-2,rightY+eyebrowAngle,ST7735_BLACK);
  } else if(eyebrowAngle<0){
    int d=-eyebrowAngle;
    tft.fillTriangle(leftX-2,leftY-2,leftX+eyeW+2,leftY-2,leftX-2,leftY+d,ST7735_BLACK);
    tft.fillTriangle(rightX-2,rightY-2,rightX+eyeW+2,rightY-2,rightX+eyeW+2,rightY+d,ST7735_BLACK);
  }
}

void drawEyes(int shiftX, int shiftY, int hLeft, int hRight, int eyebrowAngle, uint16_t color) {
  drawEyesBase(shiftX,shiftY,hLeft,hRight,eyebrowAngle,color);
}

void blink(uint16_t color) {
  for(int h=eyeH;h>=0;h-=6){drawEyes(0,0,h,h,0,color);delay(10);}
  for(int h=0;h<=eyeH;h+=6){drawEyes(0,0,h,h,0,color);delay(10);}
}

void bootScanline() {
  for(int y=0;y<tft.height();y+=3){tft.drawFastHLine(0,y,tft.width(),0x07FF);delay(10);}
  delay(80); tft.fillScreen(ST7735_BLACK);
}

// ==================== EXPRESSIONS ====================
void expressIdle()    { setRGB(rgbR,rgbG,rgbB); drawEyes(0,0,eyeH,eyeH,0,eyeColor); }

void expressOnline() {
  exprCount++; setRGB(0,180,180); eyeColor=0x07FF; beep(3000,80);
  drawEyes(0,0,eyeH,eyeH,0,0x07FF);delay(600);blink(0x07FF);
  drawEyes(10,0,eyeH,eyeH,0,0x07FF);delay(450);
  drawEyes(-10,0,eyeH,eyeH,0,0x07FF);delay(450);
  drawEyes(0,0,eyeH,eyeH,0,0x07FF);
}
void expressAngry() {
  exprCount++; setRGB(255,0,0); eyeColor=ST7735_RED; beep(2400,300);
  drawEyes(0,5,eyeH,eyeH,16,ST7735_RED);delay(200);
  for(int i=0;i<4;i++){drawEyes(-5,5,eyeH,eyeH,16,ST7735_RED);delay(90);drawEyes(5,5,eyeH,eyeH,16,ST7735_RED);delay(90);}
  drawEyes(0,5,eyeH,eyeH,16,ST7735_RED);
}
void expressSad() {
  exprCount++; setRGB(0,0,200); eyeColor=ST7735_BLUE; beep(2000,500);
  drawEyes(0,0,eyeH,eyeH,-12,ST7735_BLUE);delay(300);
  drawEyes(0,8,eyeH,eyeH,-12,ST7735_BLUE);delay(800);
  drawEyes(0,0,eyeH,eyeH,-12,ST7735_BLUE);
}
void expressSurprised() {
  exprCount++; setRGB(220,220,0); eyeColor=ST7735_YELLOW;
  beep(4000,60);delay(100);beep(4500,80);
  drawEyes(0,0,eyeH+12,eyeH+12,0,ST7735_YELLOW);delay(800);
  blink(ST7735_YELLOW); drawEyes(0,0,eyeH+12,eyeH+12,0,ST7735_YELLOW);
}
void expressCurious() {
  exprCount++; setRGB(200,80,0); eyeColor=0x07FF; beep(3200,100);
  drawEyes(0,0,eyeH+8,eyeH-14,-8,0x07FF);delay(300);
  drawEyes(8,0,eyeH+8,eyeH-14,-8,0x07FF);delay(400);
  drawEyes(0,0,eyeH+8,eyeH-14,-8,0x07FF);
}
void expressWink() {
  exprCount++; setRGB(0,200,100); eyeColor=0x07FF; beep(3500,80);
  drawEyes(0,0,eyeH,0,0,0x07FF);delay(300);
  drawEyes(0,0,eyeH,eyeH,0,0x07FF);
}
void expressSleepy() {
  exprCount++; setRGB(0,0,150); eyeColor=0x07FF; beep(1800,400);
  drawEyes(0,0,eyeH/2,eyeH/2,0,0x07FF);delay(500);
  for(int h=eyeH/2;h>=0;h-=4){drawEyes(0,0,h,h,0,0x07FF);delay(55);}
  delay(700);
  for(int h=0;h<=eyeH;h+=4){drawEyes(0,0,h,h,0,0x07FF);delay(55);}
}
void expressConfused() {
  exprCount++; setRGB(180,100,0); eyeColor=ST7735_YELLOW; beep(2600,200);
  drawEyes(0,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);delay(400);
  drawEyes(-4,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);delay(300);
  drawEyes(4,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);delay(300);
  drawEyes(0,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);
}

void runExpression(int e) {
  switch(e){
    case 1: expressOnline();   break;
    case 2: expressAngry();    break;
    case 3: expressSad();      break;
    case 4: expressSurprised();break;
    case 5: expressCurious();  break;
    case 6: expressWink();     break;
    case 7: expressSleepy();   break;
    case 8: expressConfused(); break;
    default: expressIdle();    break;
  }
}

// ==================== TALK ====================
const char* phrases[] = {
  "ONLINE","SCANNING...","BEEP BOOP","HELLO HUMAN",
  "PROCESSING","LOW BATTERY","REBOOTING","ALERT!",
  "ALL CLEAR","UNIT-01 OK","CHARGING","STANDBY"
};
const int exprForPhrase[]={1,5,0,1,5,3,7,2,1,1,7,0};
const int PHRASE_COUNT=12;
int talkIdx=0;

void talkTick() {
  static unsigned long lastTalk=0;
  if(millis()-lastTalk>4000){
    runExpression(exprForPhrase[talkIdx]);
    showBubble(String(phrases[talkIdx]),3000);
    talkIdx=(talkIdx+1)%PHRASE_COUNT;
    lastTalk=millis();
  }
}

// ==================== SPECIAL MODES ====================
void partyTick() {
  static int hue=0; hue=(hue+15)%360;
  float h=hue/60.0;int i=(int)h;float f=h-i;
  int q=(int)(255*(1-f)),t=(int)(255*f),r=0,g=0,b=0;
  switch(i%6){case 0:r=255;g=t;break;case 1:r=q;g=255;break;case 2:g=255;b=t;break;
              case 3:g=q;b=255;break;case 4:r=t;b=255;break;case 5:r=255;b=q;break;}
  float sc=rgbBrightness/100.0;
  setRGBRaw((int)(r*sc),(int)(g*sc),(int)(b*sc));
  drawEyes(random(-8,8),random(-4,4),eyeH+random(-6,6),eyeH+random(-6,6),0,tft.color565(r,g,b));
  beep(2000+random(2000),50); delay(280);
}
void hypnoTick() {
  static int phase=0,dir=1; phase+=dir*4;
  if(phase>=16||phase<=-16)dir=-dir;
  drawEyes(0,0,eyeH+phase,eyeH+phase,0,tft.color565(0,100+phase*8,200-phase*6));
  setRGB(0,100+phase*4,180-phase*4); delay(40);
}
void heartbeatTick() {
  static unsigned long last=0;static int step=0;
  int pat[]={0,180,255,180,0,0,180,255,180,0,0,0,0,0,0,0};
  if(millis()-last>60){
    int v=(int)(pat[step%16]*(rgbBrightness/100.0));
    setRGBRaw(v,0,0);
    if(pat[step%16]==255)beep(3800,30);
    step++;last=millis();
  }
  drawEyes(0,0,eyeH,eyeH,0,ST7735_RED);
}
void discoTick() {
  uint16_t cols[]={ST7735_RED,ST7735_GREEN,ST7735_BLUE,ST7735_YELLOW,ST7735_CYAN,ST7735_MAGENTA,ST7735_WHITE};
  drawEyes(random(-6,6),0,eyeH,eyeH,0,cols[random(0,7)]);
  float sc=rgbBrightness/100.0;
  setRGBRaw((int)(random(256)*sc),(int)(random(256)*sc),(int)(random(256)*sc));
  beep(1500+random(3000),40); delay(140);
}
void alertTick() {
  static bool on=false; on=!on;
  setRGBRaw(on?255:0,0,0);
  drawEyes(0,0,eyeH,eyeH,16,on?ST7735_RED:tft.color565(60,0,0));
  if(on)beep(4500,60); delay(200);
}
void rainbowEyesTick() {
  static int hue=0; hue=(hue+8)%360;
  float h=hue/60.0;int i=(int)h;float f=h-i;
  int q=(int)(255*(1-f)),t=(int)(255*f),r=0,g=0,b=0;
  switch(i%6){case 0:r=255;g=t;break;case 1:r=q;g=255;break;case 2:g=255;b=t;break;
              case 3:g=q;b=255;break;case 4:r=t;b=255;break;case 5:r=255;b=q;break;}
  eyeColor=tft.color565(r,g,b);
  drawEyes(0,0,eyeH,eyeH,0,eyeColor);
  setRGB(rgbR,rgbG,rgbB); delay(50);
}

// ==================== HTML ====================
#include "panel_robot.h"

// ==================== SERVER ====================
void handleRoot()  { server.send_P(200,"text/html",HTML); }

void handleCmd() {
  if(server.hasArg("expr")){currentMode=MODE_IDLE;runExpression(server.arg("expr").toInt());}
  if(server.hasArg("mode")){currentMode=(Mode)server.arg("mode").toInt();lastAuto=0;}
  if(server.hasArg("rgb")){
    String v=server.arg("rgb");int c1=v.indexOf(','),c2=v.lastIndexOf(',');
    rgbR=v.substring(0,c1).toInt();rgbG=v.substring(c1+1,c2).toInt();rgbB=v.substring(c2+1).toInt();
    setRGB(rgbR,rgbG,rgbB);
  }
  if(server.hasArg("bri")){rgbBrightness=server.arg("bri").toInt();setRGB(rgbR,rgbG,rgbB);}
  if(server.hasArg("eyecol")){
    String v=server.arg("eyecol");int c1=v.indexOf(','),c2=v.lastIndexOf(',');
    eyeColor=tft.color565(v.substring(0,c1).toInt(),v.substring(c1+1,c2).toInt(),v.substring(c2+1).toInt());
    drawEyes(0,0,eyeH,eyeH,0,eyeColor);
  }
  if(server.hasArg("eyeW")){eyeW=server.arg("eyeW").toInt();drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeH")){eyeH=server.arg("eyeH").toInt();drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeR")){eyeR=server.arg("eyeR").toInt();drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeG")){eyeG=server.arg("eyeG").toInt();drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("beep")){beep(server.arg("beep").toInt(),server.hasArg("bdur")?server.arg("bdur").toInt():200);}
  if(server.hasArg("melody")){playMelody(server.arg("melody").toInt());}
  if(server.hasArg("buzz_toggle")){buzzerEnabled=!buzzerEnabled;}
  if(server.hasArg("msg")){
    String txt=server.arg("msg");txt.trim();
    if(txt.length()>0){runExpression(1);showBubble(txt,4000);}
  }
  if(server.hasArg("phrase")){
    int idx=server.arg("phrase").toInt()%PHRASE_COUNT;
    runExpression(exprForPhrase[idx]);
    showBubble(String(phrases[idx]),3500);
  }
  if(server.hasArg("name")){
    String n=server.arg("name");n.trim();
    if(n.length()>0&&n.length()<24){n.toCharArray(robotName,24);}
    drawEyes(0,0,eyeH,eyeH,0,eyeColor);
  }
  if(server.hasArg("blink")){blink(eyeColor);expressIdle();}
  server.send(200,"text/plain","ok");
}

void handleStatus() {
  unsigned long s=(millis()-bootTime)/1000;
  connectedClients=WiFi.softAPgetStationNum();
  String j="{";
  j+="\"uptime\":\""+String(s/3600)+"h "+String((s%3600)/60)+"m "+String(s%60)+"s\",";
  j+="\"heap\":"+String(ESP.getFreeHeap()/1024)+",";
  j+="\"clients\":"+String(connectedClients)+",";
  j+="\"expressions\":"+String(exprCount)+",";
  j+="\"messages\":"+String(msgCount)+",";
  j+="\"mode\":"+String((int)currentMode)+",";
  j+="\"buzzer\":"+String(buzzerEnabled?1:0)+",";
  j+="\"eyeW\":"+String(eyeW)+",\"eyeH\":"+String(eyeH)+",\"eyeR\":"+String(eyeR)+",\"eyeG\":"+String(eyeG);
  j+="}";
  server.send(200,"application/json",j);
}

// ==================== SETUP ====================
void setup() {
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(RGB_R,OUTPUT);pinMode(RGB_G,OUTPUT);pinMode(RGB_B,OUTPUT);
  tft.initR(INITR_BLACKTAB);tft.setRotation(0);tft.fillScreen(ST7735_BLACK);
  setRGBRaw(0,0,80);
  bootScanline();
  beep(1800,80);delay(120);beep(2600,80);delay(120);beep(3400,80);delay(120);beep(4200,150);delay(200);
  WiFi.softAP(AP_SSID,AP_PASS);
  setRGBRaw(0,60,80);
  tft.setTextColor(0x07FF);tft.setTextSize(1);
  tft.setCursor(14,28);tft.print("ROBOT FACE");
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(8,46);tft.print("WiFi: RobotFace");
  tft.setCursor(8,60);tft.print("Pass: robot1234");
  tft.setTextColor(0x07FF);
  tft.setCursor(8,74);tft.print("192.168.4.1");
  beep(3500,150);delay(200);beep(4200,150);
  delay(2500);
  server.on("/",handleRoot);
  server.on("/cmd",handleCmd);
  server.on("/status",handleStatus);
  server.begin();
  bootTime=millis();
  setRGB(rgbR,rgbG,rgbB);
  blink(eyeColor);
  expressIdle();
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  unsigned long now=millis();
  if(bubbleActive&&now>bubbleUntil) clearBubble();
  switch(currentMode){
    case MODE_IDLE:
      if(now-lastAuto>4000){blink(eyeColor);expressIdle();lastAuto=now;}
      break;
    case MODE_RANDOM:
      if(now-lastAuto>3500){runExpression(random(1,9));lastAuto=now;}
      break;
    case MODE_PARTY:     partyTick();      break;
    case MODE_HYPNO:     hypnoTick();      break;
    case MODE_HEARTBEAT: heartbeatTick();  break;
    case MODE_DISCO:     discoTick();      break;
    case MODE_ALERT:     alertTick();      break;
    case MODE_RAINBOW:   rainbowEyesTick();break;
    case MODE_TALK:      talkTick();       break;
  }
}
