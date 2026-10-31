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

const char* AP_SSID = "BuddyMax";
const char* AP_PASS = "buddy1234";
WebServer server(80);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ==================== EYE CONFIG ====================
int eyeW  = 28;
int eyeH  = 38;
int eyeR  = 7;
int eyeG  = 10;
int eyeOX = 0;   // eye center X offset
int eyeOY = 0;   // eye center Y offset

// ==================== PERSONALITY ====================
// 0 = Buddy (round/friendly), 1 = Robot (square/mechanical)
int personality = 0;

// ==================== STATE ====================
enum Mode {
  MODE_IDLE, MODE_RANDOM, MODE_PARTY, MODE_HYPNO,
  MODE_HEARTBEAT, MODE_DISCO, MODE_ALERT, MODE_RAINBOW, MODE_TALK
};
Mode currentMode = MODE_IDLE;
int  currentExpr = 0;

int  rgbR=0, rgbG=180, rgbB=180;
int  rgbBrightness = 100;
bool buzzerEnabled = true;

unsigned long autoInterval  = 4000; // ms between auto actions in IDLE
unsigned long blinkInterval = 4000;
unsigned long lastAuto      = 0;
unsigned long lastBlink     = 0;
unsigned long bootTime      = 0;
unsigned long exprCount     = 0;
unsigned long msgCount      = 0;

uint16_t eyeColor = ST7735_CYAN;
uint16_t bgColor  = ST7735_BLACK;

char robotName[24] = "BUDDY";

String        bubbleText  = "";
bool          bubbleActive = false;
unsigned long bubbleUntil  = 0;

// ==================== RGB ====================
void setRGB(int r, int g, int b) {
  float s = rgbBrightness / 100.0;
  analogWrite(RGB_R,(int)(r*s));
  analogWrite(RGB_G,(int)(g*s));
  analogWrite(RGB_B,(int)(b*s));
}
void setRGBRaw(int r, int g, int b) {
  analogWrite(RGB_R,r); analogWrite(RGB_G,g); analogWrite(RGB_B,b);
}

// ==================== BUZZER ====================
void beep(int freq, int dur) { if(buzzerEnabled) tone(BUZZER_PIN,freq,dur); }

void playMelody(int id) {
  if(!buzzerEnabled) return;
  switch(id) {
    case 0:{int n[]={2637,3136,3520,3951,4186};for(int i=0;i<5;i++){tone(BUZZER_PIN,n[i],100);delay(130);}break;}
    case 1:{int n[]={3136,3136,0,3136,0,2637,3136,0,4186,0,0,0,2093};
            int d[]={150,150,150,150,150,150,150,150,300,150,150,150,300};
            for(int i=0;i<13;i++){if(n[i])tone(BUZZER_PIN,n[i],d[i]);else noTone(BUZZER_PIN);delay(d[i]+20);}break;}
    case 2:{int n[]={2093,1976,1760,1568};for(int i=0;i<4;i++){tone(BUZZER_PIN,n[i],300);delay(350);}break;}
    case 3:{for(int i=0;i<4;i++){tone(BUZZER_PIN,4500,80);delay(120);tone(BUZZER_PIN,3000,80);delay(120);}break;}
    case 4:{int n[]={3500,4000,4500,4000,4500};for(int i=0;i<5;i++){tone(BUZZER_PIN,n[i],60);delay(80);}break;}
    case 5:{for(int i=0;i<8;i++){tone(BUZZER_PIN,2000+random(2500),40+random(80));delay(60+random(60));}break;}
    case 6:{// Imperial March snippet
            int n[]={3136,3136,3136,2489,0,3729,3136,0,2489,0,3136,0};
            int d[]={300,300,300,200,100,200,300,100,200,100,400,200};
            for(int i=0;i<12;i++){if(n[i])tone(BUZZER_PIN,n[i],d[i]);else noTone(BUZZER_PIN);delay(d[i]+30);}break;}
  }
}

// ==================== CORE EYE DRAW ====================
void drawEyesCore(int shiftX, int shiftY, int hL, int hR, int brow, uint16_t col) {
  tft.fillScreen(bgColor);
  // Name label
  tft.setTextSize(1);
  tft.setTextColor(tft.color565(80,80,80));
  int nx = 64 - (strlen(robotName)*3);
  tft.setCursor(nx, 2); tft.print(robotName);

  int cx = (tft.width()/2)  + shiftX + eyeOX;
  int cy = (tft.height()/2) + shiftY + eyeOY;
  int lx = cx - eyeW - (eyeG/2), ly = cy - (hL/2);
  int rx = cx + (eyeG/2),        ry = cy - (hR/2);

  if(hL>2) tft.fillRoundRect(lx,ly,eyeW,hL,eyeR,col);
  else     tft.drawFastHLine(lx,cy,eyeW,col);
  if(hR>2) tft.fillRoundRect(rx,ry,eyeW,hR,eyeR,col);
  else     tft.drawFastHLine(rx,cy,eyeW,col);

  if(brow>0){
    tft.fillTriangle(lx-2,ly-2,lx+eyeW+2,ly-2,lx+eyeW+2,ly+brow,bgColor);
    tft.fillTriangle(rx-2,ry-2,rx+eyeW+2,ry-2,rx-2,ry+brow,bgColor);
  } else if(brow<0){
    int d=-brow;
    tft.fillTriangle(lx-2,ly-2,lx+eyeW+2,ly-2,lx-2,ly+d,bgColor);
    tft.fillTriangle(rx-2,ry-2,rx+eyeW+2,ry-2,rx+eyeW+2,ry+d,bgColor);
  }
}

void drawEyes(int sx,int sy,int hL,int hR,int brow,uint16_t col){
  drawEyesCore(sx,sy,hL,hR,brow,col);
}

void blink(uint16_t col) {
  int step = max(4, eyeH/8);
  for(int h=eyeH;h>=0;h-=step){drawEyes(0,0,h,h,0,col);delay(10);}
  for(int h=0;h<=eyeH;h+=step){drawEyes(0,0,h,h,0,col);delay(10);}
}

void bootScanline() {
  for(int y=0;y<tft.height();y+=3){tft.drawFastHLine(0,y,tft.width(),eyeColor);delay(10);}
  delay(80); tft.fillScreen(bgColor);
}

// ==================== SPEECH BUBBLE ====================
void showBubble(String text, int durMs) {
  bubbleText=text; bubbleActive=true; bubbleUntil=millis()+durMs; msgCount++;
  int bx=4,by=2,bw=120,bh=22;
  tft.fillRoundRect(bx,by,bw,bh,4,ST7735_WHITE);
  tft.drawRoundRect(bx,by,bw,bh,4,ST7735_BLACK);
  tft.fillTriangle(14,by+bh,20,by+bh,14,by+bh+6,ST7735_WHITE);
  tft.drawLine(14,by+bh+6,14,by+bh,ST7735_BLACK);
  tft.drawLine(14,by+bh+6,20,by+bh,ST7735_BLACK);
  tft.setTextColor(ST7735_BLACK); tft.setTextSize(1);
  String disp=text.substring(0,min((int)text.length(),18));
  for(int i=1;i<=(int)disp.length();i++){
    tft.fillRect(bx+3,by+7,bw-6,10,ST7735_WHITE);
    tft.setCursor(bx+4,by+8); tft.print(disp.substring(0,i)); delay(45);
  }
}

void clearBubble(){ bubbleActive=false; drawEyes(0,0,eyeH,eyeH,0,eyeColor); }

// ==================== EXPRESSIONS ====================
void expressIdle()    { setRGB(rgbR,rgbG,rgbB); drawEyes(0,0,eyeH,eyeH,0,eyeColor); }

void expressHappy() {
  exprCount++; setRGB(0,220,0); eyeColor=ST7735_GREEN;
  beep(3200,80);delay(120);beep(3800,80);delay(120);beep(4200,120);
  drawEyes(0,0,eyeH,eyeH,0,ST7735_GREEN);delay(300);
  blink(ST7735_GREEN);
  drawEyes(10,0,eyeH,eyeH,0,ST7735_GREEN);delay(400);
  drawEyes(-10,0,eyeH,eyeH,0,ST7735_GREEN);delay(400);
  drawEyes(0,0,eyeH,eyeH,0,ST7735_GREEN);
}
void expressAngry() {
  exprCount++; setRGB(255,0,0); eyeColor=ST7735_RED; beep(2400,300);
  drawEyes(0,4,eyeH,eyeH,14,ST7735_RED);delay(200);
  for(int i=0;i<3;i++){drawEyes(-6,4,eyeH,eyeH,14,ST7735_RED);delay(100);drawEyes(6,4,eyeH,eyeH,14,ST7735_RED);delay(100);}
  drawEyes(0,4,eyeH,eyeH,14,ST7735_RED);
}
void expressSad() {
  exprCount++; setRGB(0,0,200); eyeColor=ST7735_BLUE; beep(2000,500);
  drawEyes(0,0,eyeH,eyeH,-10,ST7735_BLUE);delay(300);
  drawEyes(0,6,eyeH,eyeH,-10,ST7735_BLUE);delay(700);
  drawEyes(0,0,eyeH,eyeH,-10,ST7735_BLUE);
}
void expressSurprised() {
  exprCount++; setRGB(220,220,0); eyeColor=ST7735_YELLOW;
  beep(4000,80);delay(100);beep(4500,80);
  drawEyes(0,0,eyeH+10,eyeH+10,0,ST7735_YELLOW);delay(700);
  blink(ST7735_YELLOW);
  drawEyes(0,0,eyeH+10,eyeH+10,0,ST7735_YELLOW);
}
void expressCurious() {
  exprCount++; setRGB(200,100,0); eyeColor=ST7735_CYAN; beep(3000,100);
  drawEyes(0,0,eyeH+6,eyeH-12,-8,ST7735_CYAN);delay(300);
  drawEyes(6,0,eyeH+6,eyeH-12,-8,ST7735_CYAN);delay(400);
  drawEyes(0,0,eyeH+6,eyeH-12,-8,ST7735_CYAN);
}
void expressWink() {
  exprCount++; setRGB(0,180,180); eyeColor=ST7735_CYAN; beep(3500,80);
  drawEyes(0,0,eyeH,0,0,ST7735_CYAN);delay(300);
  drawEyes(0,0,eyeH,eyeH,0,ST7735_CYAN);
}
void expressLove() {
  exprCount++; setRGB(255,20,80);
  uint16_t pink=tft.color565(255,100,150); eyeColor=pink;
  beep(3500,80);delay(100);beep(4000,80);delay(100);beep(4500,120);
  drawEyes(0,0,eyeH,eyeH,0,pink);delay(300);
  tft.fillCircle(37,30,4,pink);tft.fillCircle(43,30,4,pink);
  tft.fillTriangle(33,32,47,32,40,40,pink);
  tft.fillCircle(85,30,4,pink);tft.fillCircle(91,30,4,pink);
  tft.fillTriangle(81,32,95,32,88,40,pink);
  delay(1200); drawEyes(0,0,eyeH,eyeH,0,pink);
}
void expressDizzy() {
  exprCount++; setRGB(180,0,180); eyeColor=ST7735_MAGENTA; beep(2000,400);
  for(int i=0;i<4;i++){
    drawEyes(random(-10,10),random(-6,6),eyeH+random(-10,10),eyeH+random(-10,10),random(-8,8),ST7735_MAGENTA);
    delay(150);
  }
  drawEyes(0,0,eyeH,eyeH,0,ST7735_MAGENTA);
}
void expressSleepy() {
  exprCount++; setRGB(0,0,150); eyeColor=tft.color565(100,100,255); beep(1800,400);
  drawEyes(0,0,eyeH/2,eyeH/2,0,eyeColor);delay(500);
  for(int h=eyeH/2;h>=0;h-=4){drawEyes(0,0,h,h,0,eyeColor);delay(55);}
  delay(700);
  for(int h=0;h<=eyeH;h+=4){drawEyes(0,0,h,h,0,eyeColor);delay(55);}
}
void expressConfused() {
  exprCount++; setRGB(180,100,0); eyeColor=ST7735_YELLOW; beep(2600,200);
  drawEyes(0,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);delay(400);
  drawEyes(-4,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);delay(300);
  drawEyes(4,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);delay(300);
  drawEyes(0,0,eyeH+6,eyeH-10,-6,ST7735_YELLOW);
}
void expressExcited() {
  exprCount++; setRGB(255,180,0); eyeColor=tft.color565(255,200,0);
  beep(4000,60);
  for(int i=0;i<3;i++){
    drawEyes(0,-4,eyeH+8,eyeH+8,0,eyeColor);delay(120);
    drawEyes(0,4,eyeH+8,eyeH+8,0,eyeColor);delay(120);
  }
  drawEyes(0,0,eyeH+8,eyeH+8,0,eyeColor);
  beep(4500,80);delay(100);beep(4000,80);
}

void runExpression(int e) {
  switch(e){
    case 1:  expressHappy();    break;
    case 2:  expressAngry();    break;
    case 3:  expressSad();      break;
    case 4:  expressSurprised();break;
    case 5:  expressCurious();  break;
    case 6:  expressWink();     break;
    case 7:  expressLove();     break;
    case 8:  expressDizzy();    break;
    case 9:  expressSleepy();   break;
    case 10: expressConfused(); break;
    case 11: expressExcited();  break;
    default: expressIdle();     break;
  }
}

// ==================== PERSONALITY PRESETS ====================
void applyPersonality(int p) {
  personality=p;
  if(p==0){ eyeW=28;eyeH=38;eyeR=7;eyeG=10;eyeColor=ST7735_CYAN;strcpy(robotName,"BUDDY"); }
  else     { eyeW=30;eyeH=30;eyeR=4;eyeG=8; eyeColor=0x07FF;    strcpy(robotName,"UNIT-01"); }
  drawEyes(0,0,eyeH,eyeH,0,eyeColor);
}

// ==================== TALK ====================
const char* buddyPhrases[]={"Hello there!","I am BUDDY","Beep boop!","How are you?","I feel great!","Whoa!","Zzz...","I like you!","Error 404","Lets party!","Ouch!","Hehe ;)","So excited!","I am dizzy","Miss you!"};
const char* robotPhrases[]={"ONLINE","SCANNING...","BEEP BOOP","HELLO HUMAN","PROCESSING","LOW BATTERY","REBOOTING","ALERT!","ALL CLEAR","UNIT OK","CHARGING","STANDBY","INITIALIZING","CALIBRATING","READY"};
const int   phraseExpr[]  ={1,0,8,5,1,4,9,7,2,1,3,6,11,8,7};
const int   PHRASE_COUNT  =15;
int talkIdx=0;

void talkTick(){
  static unsigned long lastTalk=0;
  if(millis()-lastTalk>4000){
    const char** ph=(personality==0)?buddyPhrases:robotPhrases;
    runExpression(phraseExpr[talkIdx]);
    showBubble(String(ph[talkIdx]),3000);
    talkIdx=(talkIdx+1)%PHRASE_COUNT;
    lastTalk=millis();
  }
}

// ==================== SPECIAL MODES ====================
void hsvToRGB(int hue,int &r,int &g,int &b){
  float h=hue/60.0;int i=(int)h;float f=h-i;
  int q=(int)(255*(1-f)),t=(int)(255*f);
  r=g=b=0;
  switch(i%6){case 0:r=255;g=t;break;case 1:r=q;g=255;break;case 2:g=255;b=t;break;
              case 3:g=q;b=255;break;case 4:r=t;b=255;break;case 5:r=255;b=q;break;}
}
void partyTick(){
  static int hue=0; hue=(hue+15)%360; int r,g,b; hsvToRGB(hue,r,g,b);
  float sc=rgbBrightness/100.0;
  setRGBRaw((int)(r*sc),(int)(g*sc),(int)(b*sc));
  beep(2000+random(2000),60);
  drawEyes(random(-8,8),random(-4,4),eyeH+random(-8,8),eyeH+random(-8,8),0,tft.color565(r,g,b));
  delay(300);
}
void hypnoTick(){
  static int phase=0,dir=1; phase+=dir*4;
  if(phase>=16||phase<=-16)dir=-dir;
  drawEyes(0,0,eyeH+phase,eyeH+phase,0,tft.color565(0,180+phase*4,180-phase*4));
  setRGB(0,100+phase*4,180-phase*4); delay(40);
}
void heartbeatTick(){
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
void discoTick(){
  uint16_t cols[]={ST7735_RED,ST7735_GREEN,ST7735_BLUE,ST7735_YELLOW,ST7735_CYAN,ST7735_MAGENTA,ST7735_WHITE};
  drawEyes(random(-6,6),0,eyeH,eyeH,0,cols[random(0,7)]);
  float sc=rgbBrightness/100.0;
  setRGBRaw((int)(random(256)*sc),(int)(random(256)*sc),(int)(random(256)*sc));
  beep(1500+random(3000),40); delay(140);
}
void alertTick(){
  static bool on=false; on=!on;
  setRGBRaw(on?255:0,0,0);
  drawEyes(0,0,eyeH,eyeH,14,on?ST7735_RED:tft.color565(60,0,0));
  if(on)beep(4500,60); delay(200);
}
void rainbowTick(){
  static int hue=0; hue=(hue+8)%360; int r,g,b; hsvToRGB(hue,r,g,b);
  eyeColor=tft.color565(r,g,b);
  drawEyes(0,0,eyeH,eyeH,0,eyeColor);
  setRGB(rgbR,rgbG,rgbB); delay(50);
}

// ==================== HTML ====================
#include "panel_max.h"

// ==================== SERVER ====================
void handleRoot(){ server.send_P(200,"text/html",HTML); }

void handleCmd(){
  if(server.hasArg("expr")){currentMode=MODE_IDLE;runExpression(server.arg("expr").toInt());}
  if(server.hasArg("mode")){currentMode=(Mode)server.arg("mode").toInt();lastAuto=0;}
  if(server.hasArg("personality")){applyPersonality(server.arg("personality").toInt());}
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
  if(server.hasArg("bgcol")){
    String v=server.arg("bgcol");int c1=v.indexOf(','),c2=v.lastIndexOf(',');
    bgColor=tft.color565(v.substring(0,c1).toInt(),v.substring(c1+1,c2).toInt(),v.substring(c2+1).toInt());
    drawEyes(0,0,eyeH,eyeH,0,eyeColor);
  }
  if(server.hasArg("eyeW")){eyeW=constrain(server.arg("eyeW").toInt(),8,50);drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeH")){eyeH=constrain(server.arg("eyeH").toInt(),6,58);drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeR")){eyeR=constrain(server.arg("eyeR").toInt(),0,20);drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeG")){eyeG=constrain(server.arg("eyeG").toInt(),2,32);drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeOX")){eyeOX=constrain(server.arg("eyeOX").toInt(),-20,20);drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("eyeOY")){eyeOY=constrain(server.arg("eyeOY").toInt(),-20,20);drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  if(server.hasArg("autoInt")){autoInterval=server.arg("autoInt").toInt();}
  if(server.hasArg("blinkInt")){blinkInterval=server.arg("blinkInt").toInt();}
  if(server.hasArg("beep")){beep(server.arg("beep").toInt(),server.hasArg("bdur")?server.arg("bdur").toInt():200);}
  if(server.hasArg("melody")){playMelody(server.arg("melody").toInt());}
  if(server.hasArg("buzz_toggle")){buzzerEnabled=!buzzerEnabled;}
  if(server.hasArg("msg")){
    String t=server.arg("msg");t.trim();
    if(t.length()>0){runExpression(1);showBubble(t,4000);}
  }
  if(server.hasArg("phrase")){
    int idx=server.arg("phrase").toInt()%PHRASE_COUNT;
    const char** ph=(personality==0)?buddyPhrases:robotPhrases;
    runExpression(phraseExpr[idx]);
    showBubble(String(ph[idx]),3500);
  }
  if(server.hasArg("name")){
    String n=server.arg("name");n.trim();
    if(n.length()>0&&n.length()<24){n.toCharArray(robotName,24);}
    drawEyes(0,0,eyeH,eyeH,0,eyeColor);
  }
  if(server.hasArg("blink")){blink(eyeColor);expressIdle();}
  if(server.hasArg("scanline")){bootScanline();drawEyes(0,0,eyeH,eyeH,0,eyeColor);}
  server.send(200,"text/plain","ok");
}

void handleStatus(){
  unsigned long s=(millis()-bootTime)/1000;
  String j="{";
  j+="\"uptime\":\""+String(s/3600)+"h "+String((s%3600)/60)+"m "+String(s%60)+"s\",";
  j+="\"heap\":"+String(ESP.getFreeHeap()/1024)+",";
  j+="\"clients\":"+String(WiFi.softAPgetStationNum())+",";
  j+="\"expressions\":"+String(exprCount)+",";
  j+="\"messages\":"+String(msgCount)+",";
  j+="\"mode\":"+String((int)currentMode)+",";
  j+="\"buzzer\":"+String(buzzerEnabled?1:0)+",";
  j+="\"personality\":"+String(personality)+",";
  j+="\"eyeW\":"+String(eyeW)+",\"eyeH\":"+String(eyeH)+",";
  j+="\"eyeR\":"+String(eyeR)+",\"eyeG\":"+String(eyeG)+",";
  j+="\"eyeOX\":"+String(eyeOX)+",\"eyeOY\":"+String(eyeOY);
  j+="}";
  server.send(200,"application/json",j);
}

// ==================== SETUP ====================
void setup(){
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(RGB_R,OUTPUT);pinMode(RGB_G,OUTPUT);pinMode(RGB_B,OUTPUT);
  tft.initR(INITR_BLACKTAB);tft.setRotation(0);tft.fillScreen(ST7735_BLACK);
  setRGBRaw(0,0,80);
  beep(2000,80);delay(120);beep(2800,80);delay(120);beep(3600,80);delay(120);beep(4200,150);delay(200);
  WiFi.softAP(AP_SSID,AP_PASS);
  setRGBRaw(0,100,0);
  tft.setTextColor(ST7735_GREEN);tft.setTextSize(1);
  tft.setCursor(8,24);tft.print("BUDDY MAX");
  tft.setTextColor(ST7735_CYAN);
  tft.setCursor(8,40);tft.print("WiFi: BuddyMax");
  tft.setCursor(8,54);tft.print("Pass: buddy1234");
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(8,68);tft.print("192.168.4.1");
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
void loop(){
  server.handleClient();
  unsigned long now=millis();
  if(bubbleActive&&now>bubbleUntil) clearBubble();
  switch(currentMode){
    case MODE_IDLE:
      if(now-lastBlink>blinkInterval){blink(eyeColor);expressIdle();lastBlink=now;}
      break;
    case MODE_RANDOM:
      if(now-lastAuto>autoInterval){runExpression(random(1,12));lastAuto=now;}
      break;
    case MODE_PARTY:     partyTick();     break;
    case MODE_HYPNO:     hypnoTick();     break;
    case MODE_HEARTBEAT: heartbeatTick(); break;
    case MODE_DISCO:     discoTick();     break;
    case MODE_ALERT:     alertTick();     break;
    case MODE_RAINBOW:   rainbowTick();   break;
    case MODE_TALK:      talkTick();      break;
  }
}
