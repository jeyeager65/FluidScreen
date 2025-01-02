#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "JogJoystick.h"
#include <ESPRotary.h>
#include "menu.h"
#include "qrcode.h"
#include "FluidScreenSettings.h"

void setupColors(FluidScreenSettings& settings);
void drawConfig();
void drawPreview();

#include "CaptivePortal.h"
#include "SPIFFS.h"

bool  wsDisconnected = false;

// Rotary Encoder Settings
volatile int encoderCounter = 0;
int encoderPrevCounter;
volatile int encoderMaxCount = 20;
int buttonState;
unsigned long lastButtonTime = 0;


const byte ENCODER_CLK_PIN = 25;
const byte ENCODER_DT_PIN = 26;
const byte ENCODER_SW_PIN = 27;

const int tftWidth = 320;
const int tftHeight = 240;

// Info Screen
String messageCurrent;
String messagePrevious;


ESPRotary menuEncoder;
hw_timer_t *timer = NULL;

void IRAM_ATTR handleLoop() {
  menuEncoder.loop();
}

// Joystick Canvas
GFXcanvas16 joystickCanvas(120, 120);
GFXcanvas16 jsInfoCanvas(100, 100);

// Joystick Pins
const int JOYSTICK_X_PIN = 33;
const int JOYSTICK_Y_PIN = 32;

// Display TFT Pins
const int TFT_CS_PIN = 5;
const int TFT_DC_PIN = 16;
const int TFT_RST_PIN = 17;
const int TFT_BL_PIN = 22;

unsigned long lastDrawTime = 0;
unsigned long drawDelay = 50;

unsigned long lastMacroInfoTime = 0;
unsigned long macroInfoDelay = 200;

unsigned long lastJogTime = 0;
unsigned long jogDelay = 200;
bool jogActive = false;

// Colors
// RGB565 Colors - https://learn.adafruit.com/adafruit-gfx-graphics-library/coordinate-system-and-units
// Converter - https://barth-dev.de/online/rgb565-color-picker/
uint16_t COLOR_PRIMARY   = 0x7800;        // #7a0000
uint16_t COLOR_SECONDARY = 0xA800;        // #ad0000
uint16_t COLOR_BG        = 0x0000;        // #000000
uint16_t COLOR_BG_ALT    = 0x4A69;        // #4d4d4d
uint16_t COLOR_BG_PANEL  = 0x31A6;        // #363636
uint16_t COLOR_BORDER    = 0xD6BA;        // #d6d6d6
uint16_t COLOR_SELECT    = 0xF800;        // #ff0000
uint16_t COLOR_TEXT      = 0xFFFF;        
uint16_t COLOR_QR        = 0x0000;
uint16_t COLOR_QR_BG     = 0xFFFF;

uint16_t COLOR_STATUS_DEFAULT    = 0x7BEF;
uint16_t COLOR_STATUS_IDLE       = 0x7BEF;
uint16_t COLOR_STATUS_JOG        = 0x000F;
uint16_t COLOR_STATUS_RUN        = 0x03E0;
uint16_t COLOR_STATUS_HOLD       = 0xFFE0;
uint16_t COLOR_STATUS_HOME       = 0x03EF;
uint16_t COLOR_STATUS_ALARM      = 0xF800;
uint16_t COLOR_STATUS_DISCONNECT = 0x780F;
uint16_t COLOR_STATUS_CONFIG     = 0xFB80;

// Display Values
String grblState = "UNKNOWN";
String grblStatePrev = "UNKNOWN";
String mPosX = " ", mPosY = " ", mPosZ = " ";
String mPosXPrev = "*", mPosYPrev = "*", mPosZPrev = "*";
String lastMessage;

JogJoystick joystick(JOYSTICK_X_PIN, JOYSTICK_Y_PIN);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);

WebSocketsClient webSocket;

void initWiFi() 
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(CurrentSettings.Connection.SSID, CurrentSettings.Connection.Password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

String convert(uint8_t *str, int length)
{
  return String((char *)str, length);
}

void setupTextArea(int x, int y, int width, int height, bool drawBorder = false, int leftMargin = 0, int lineHeight = 30, int negYOffset = 10, uint16_t bgColor = COLOR_BG) {
  tft.setCursor(x+leftMargin, y+lineHeight-negYOffset);
  tft.fillRect(x, y, width, height, bgColor);
  
  if(drawBorder) {
    tft.drawRect(x, y, width, height, COLOR_BORDER);
  }
  
  tft.setTextColor(COLOR_TEXT);
  tft.setFont(&FreeSans9pt7b);
}

void drawSubMenu(const String* subMenu, const int subMenuSize, int subMenuSelected) {
  int menuHeight = 24;

  tft.fillRect(0, 240-(menuHeight*2), 320, menuHeight, COLOR_SECONDARY);
  tft.drawRect(0, 240-(menuHeight*2), 320, menuHeight, COLOR_BORDER);

  int itemSize = 320/subMenuSize;
  int rightX = 0;
  for(int j = 0; j < subMenuSize; j++) {
    tft.setCursor(j*itemSize, 234-menuHeight);
    tft.setFont(&FreeSans9pt7b);
    if(j == subMenuSelected) {
      tft.fillRect((j*itemSize)+1,240-(menuHeight*2)+1,itemSize-1,menuHeight-2,COLOR_SELECT);
    }
    tft.setTextColor(COLOR_TEXT);

    int16_t x1, y1, xCenterStart;
    uint16_t width, height;
    tft.getTextBounds(subMenu[j], j*itemSize, 234-menuHeight, &x1, &y1, &width, &height);
    xCenterStart = x1 + (width/2);

    tft.setCursor((j*itemSize) + ((itemSize-width)/2), 234-menuHeight);
    tft.print(subMenu[j]);
    tft.drawLine((j+1)*itemSize, 240-(menuHeight*2), (j+1)*itemSize, 240-menuHeight, COLOR_BORDER);
  }
}

void drawMenu() {
  int menuHeight = 24;

  tft.fillRect(0, 240-menuHeight, 320, menuHeight, COLOR_PRIMARY);
  tft.drawRect(0, 240-menuHeight, 320, menuHeight, COLOR_BORDER);

  int itemSize = 320/menuSize;
  
  for(int i = 0; i < menuSize; i++) {
    tft.setCursor(i*itemSize, 234);
    tft.setFont(&FreeSans9pt7b);
    if(i == menuSelected) {
      tft.fillRect((i*itemSize)+1,240-menuHeight+1,itemSize-1,menuHeight-2,COLOR_SELECT);
    }
    tft.setTextColor(COLOR_TEXT);

    // Center Text
    int16_t x1, y1, xCenterStart;
    uint16_t width, height;
    tft.getTextBounds(menu[i], i*itemSize, 234, &x1, &y1, &width, &height);
    xCenterStart = x1 + (width/2);

    tft.setCursor(x1 + ((itemSize-width)/2), 234);
    tft.print(menu[i]);
    tft.drawLine((i+1)*itemSize, 240-menuHeight, (i+1)*itemSize, 240, COLOR_BORDER);
  }

  // Draw submenu
  tft.fillRect(0, 240-(menuHeight*2), 320, menuHeight, COLOR_PRIMARY);
  tft.drawRect(0, 240-(menuHeight*2), 320, menuHeight, COLOR_BORDER);

  if(menu[menuSelected] == "Jog") {
    drawSubMenu(jogMenu, jogMenuSize, jogMenuSelected);
  }
  else if(menu[menuSelected] == "Home") {
    drawSubMenu(homeMenu, homeMenuSize, homeMenuSelected);
  }
  else if(menu[menuSelected] == "Macro") {
    drawSubMenu(macroMenu, macroMenuSize, macroMenuSelected);
  }
  else if(menu[menuSelected] == "Other") {
    drawSubMenu(otherMenu, otherMenuSize, otherMenuSelected);
  }
  else if(menu[menuSelected] == "Main") {
    drawSubMenu(controlMenu, controlMenuSize, controlMenuSelected);
  }
}

void drawMessage() {
  int16_t x1, y1;
  uint16_t width, height;
  const char * label = "Last Message";

  setupTextArea(0,55,320,125);

  tft.fillRoundRect(0,55+5, 320, 125-5, 5, COLOR_BG_ALT);
  tft.drawRoundRect(0,55+5, 320, 125-5, 5, COLOR_BORDER);
  tft.setCursor(10, 55+7);
  tft.setFont();
  tft.getTextBounds(label, tft.getCursorX(), tft.getCursorY(), &x1, &y1, &width, &height);
  tft.fillRect(x1-2, y1, width+4, height-1, COLOR_BG);
  tft.print(label);

  // Clean Message
  if(lastMessage.startsWith("[MSG:")) {
    int startIndex = lastMessage.indexOf(":", 5)+2;
    int tempStartIndex = lastMessage.indexOf(",", startIndex+1);
    if(tempStartIndex >= 0) { startIndex = tempStartIndex+2; }
    int endIndex = lastMessage.lastIndexOf("]");
    lastMessage = lastMessage.substring(startIndex, endIndex);
  }

  lastMessage.trim();

  // Break at spaces
  bool includeSpace = false;
  char * ptr;
  int length = lastMessage.length();
  char buffer[length];
  sprintf(buffer, "%s", lastMessage.c_str());

  tft.setFont(&FreeSans9pt7b);
  
  tft.setCursor(2,60+30-8+2);
  tft.setTextWrap(true);

  ptr = strtok(buffer, " ");
  while (ptr != NULL)
  {
    tft.getTextBounds(ptr, tft.getCursorX(), tft.getCursorY(), &x1, &y1, &width, &height);

    if(x1 + width > 320-10) {
      tft.println("");
      tft.setCursor(2, tft.getCursorY());
      includeSpace = false;
    }

    if(includeSpace) {
      tft.print(" ");
    }
    
    tft.printf("%s", ptr);
    ptr = strtok(NULL, " ");
    includeSpace = true;
  }
}

void drawStatus(String status, bool preventDuplicate = true) {
  bool isHold = status.startsWith("Hold");
  
  grblState = status;
  if(preventDuplicate && grblState == grblStatePrev) return;

  grblStatePrev = grblState;

  tft.setCursor(0, 0+30-7);
  tft.setTextColor(COLOR_TEXT);

  uint16_t statusColor = COLOR_STATUS_DEFAULT;
  if(status == "Idle") statusColor = COLOR_STATUS_IDLE;
  else if(status == "Jog") statusColor = COLOR_STATUS_JOG;
  else if(status == "Run") statusColor = COLOR_STATUS_RUN;
  else if(status.startsWith("Hold")) statusColor = COLOR_STATUS_HOLD;
  else if(status == "Home") statusColor = COLOR_STATUS_HOME;
  else if(status == "Alarm") statusColor = COLOR_STATUS_ALARM;
  else if(status == "Disconnected") statusColor = COLOR_STATUS_DISCONNECT;
  else if(status == "Configuration") statusColor = COLOR_STATUS_CONFIG;

  tft.fillRect(0, 0, 320, 30, statusColor);
  tft.drawRect(0, 0, 320, 30, COLOR_BORDER);

  // Override Font
  tft.setFont(&FreeSansBold12pt7b);
  
  status.toUpperCase();

  int16_t x1, y1, xCenterStart;
  uint16_t width, height;
  tft.getTextBounds(status, 0, 0, &x1, &y1, &width, &height);
  xCenterStart = x1 + (width/2);
  tft.setCursor(x1 + ((320-width)/2), 0+30-7);

  tft.printf("%s", status);

  // Auto-select Resume when Hold occurs
  if(isHold) {
    menuSelected = 0; // Main
    controlMenuSelected = 2; // Resume
    menuLevel = 1;
    encoderMaxCount = controlMenuSize-1;
    encoderPrevCounter = -1;
    encoderCounter = controlMenuSelected;
    menuEncoder.setUpperBound(encoderMaxCount);
    menuEncoder.resetPosition(encoderCounter);
  }
}

void drawPosition(String mPosX, String mPosY, String mPosZ) {
  if(mPosX != mPosXPrev) 
  { 
    setupTextArea(0,30-1,320/3,20, true, 2, 20, 5, COLOR_BG_PANEL); 
    tft.printf("X: %s", mPosX); 
  }
  
  if(mPosY != mPosYPrev)
  { 
    setupTextArea(0+(320/3)*1,30-1,320/3,20, true, 2, 20, 5, COLOR_BG_PANEL);
    tft.printf("Y: %s", mPosY); 
  }
  
  if(mPosZ != mPosZPrev)
  { 
    setupTextArea(0+(320/3)*2,30-1,320-((320/3)*2),20, true, 2, 20, 5, COLOR_BG_PANEL);
    tft.printf("Z: %s", mPosZ); 
  }

  mPosXPrev = mPosX;
  mPosYPrev = mPosY;
  mPosZPrev = mPosZ;
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
      drawStatus("Disconnected");
      wsDisconnected = true;
			break;
		case WStype_CONNECTED:
			Serial.printf("[WSc] Connected to url: %s\n", payload);
      webSocket.setReconnectInterval(86400000); // Disable reconnect - set to 24 hours

			// send message to server when Connected
      webSocket.sendTXT("Connected\n");
      webSocket.sendTXT("$Report/Interval=100\n");
      
			break;
		case WStype_TEXT:
			Serial.printf("[WSc] get text: %s\n", payload);
			break;
		case WStype_BIN:
    {
			Serial.printf("[WSc] get binary string: %s\n", payload);

      String temp = convert(payload, length);
      if(temp.startsWith("<")) // Process Status Line
      {
        unsigned int pos = temp.indexOf("|");
        String status = temp.substring(1, pos);
        drawStatus(status);

        //<Idle|MPos:99.820,563.900,42.215|FS:0,0>
        pos = pos + 6; // "|MPos:";

        unsigned int pos2 = temp.indexOf("|", pos);
        String mpos = temp.substring(pos, pos2);

        mPosX = mpos.substring(0, mpos.indexOf(","));
        mPosY = mpos.substring(mpos.indexOf(",")+1, mpos.lastIndexOf(","));
        mPosZ = mpos.substring(mpos.lastIndexOf(",")+1);

        tft.setCursor(0, 30+30-8);
        tft.setTextColor(COLOR_TEXT);
        tft.setFont(&FreeSans9pt7b);

        drawPosition(mPosX, mPosY, mPosZ);
      }
      else if(length > 4 && temp.startsWith("[MSG")) // Ignore "ok" for now
      {
        lastMessage = convert(payload, length);
        if((menu[menuSelected] != "Jog" || jogMenuSelected == 0))
        {
          if(lastMessage.indexOf("websocket auto report") >= 0) {
            lastMessage = "Ready";
          }
          drawMessage();
        }
      }
			break;
    }
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}
}

void initWebSocket() 
{
  Serial.println("Initializing WebSocket");
  wsDisconnected = false;
  webSocket.begin(CurrentSettings.Connection.Address, CurrentSettings.Connection.Port);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(500);
}

void rotate(ESPRotary& r) {
  encoderCounter = r.getPosition();
  
  if(menuLevel == 0) {
    menuSelected = encoderCounter;
  }
  else if(menuLevel == 1) {
    if(menu[menuSelected] == "Jog") {
      jogMenuSelected = encoderCounter;
    }
    else if(menu[menuSelected] == "Home") {
      homeMenuSelected = encoderCounter;
    }
    else if(menu[menuSelected] == "Macro") {
      macroMenuSelected = encoderCounter;
    }
    else if(menu[menuSelected] == "Other") {
      otherMenuSelected = encoderCounter;
    }
    else if(menu[menuSelected] == "Main") {
      controlMenuSelected = encoderCounter;
    }
  }
}

void drawQrCode(String content) {
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, content.c_str());

  tft.fillRoundRect(195,75,320-195,125,5,COLOR_QR_BG);
  tft.drawRoundRect(195,75,320-195,125,5,COLOR_BORDER);
  
  int mult = (125-25)/qrcode.size;
  int size = qrcode.size * mult;
  int xStart = 320 - size - ((125 - size) / 2); // (320 - size) / 2;
  int yStart = 75 + ((125 - size) / 2);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if(qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(xStart + (x*mult), yStart + (y*mult), mult, mult, COLOR_QR);
      }
    }
  }
}

void setupColors(FluidScreenSettings& settings) {
  COLOR_PRIMARY   = settings.ThemeColors.Primary;
  COLOR_SECONDARY = settings.ThemeColors.Secondary;
  COLOR_BG        = settings.ThemeColors.Background;
  COLOR_BG_ALT    = settings.ThemeColors.BackgroundAlt;
  COLOR_BG_PANEL  = settings.ThemeColors.BackgroundPanel;
  COLOR_BORDER    = settings.ThemeColors.Border;
  COLOR_SELECT    = settings.ThemeColors.Selected;
  COLOR_TEXT      = settings.ThemeColors.Text;
  COLOR_QR        = settings.ThemeColors.QrCode;
  COLOR_QR_BG     = settings.ThemeColors.QrCodeBg;
  COLOR_STATUS_DEFAULT    = settings.StatusColors.Default;
  COLOR_STATUS_IDLE       = settings.StatusColors.Idle;
  COLOR_STATUS_JOG        = settings.StatusColors.Jog;
  COLOR_STATUS_RUN        = settings.StatusColors.Run;
  COLOR_STATUS_HOLD       = settings.StatusColors.Hold;
  COLOR_STATUS_HOME       = settings.StatusColors.Home;
  COLOR_STATUS_ALARM      = settings.StatusColors.Alarm;
  COLOR_STATUS_DISCONNECT = settings.StatusColors.Disconnect;
  COLOR_STATUS_CONFIG     = settings.StatusColors.Configuration;
}

void drawConfig() {
  setupTextArea(0,0,320,240);

  drawStatus("Configuration", false);

  tft.drawRoundRect(0,40,320-140, 240-40, 5, COLOR_BORDER);
  tft.setCursor(5, 60);
  tft.setFont(&FreeSans9pt7b);
  tft.println("Scan QR code or");
  tft.setCursor(5, tft.getCursorY());
  tft.println("connect to the");
  tft.setCursor(5, tft.getCursorY());
  tft.println("FluidScreen");
  tft.setCursor(5, tft.getCursorY());
  tft.println("network.");

  drawQrCode("WIFI:S:FluidScreen;;");
}

void drawPreview() {
  setupTextArea(0,0,320,240);

  drawStatus("Configuration", false);

  mPosXPrev = "";
  mPosYPrev = "";
  mPosZPrev = "";
  drawPosition("0000.000", "0000.000", "0000.000");

  lastMessage = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec iaculis efficitur porta.";
  drawMessage();

  drawMenu();
}

void setup() {
  Serial.begin(115200);
  
  // I don't know why but without this delay, I can't read preferences
  delay(100);
  
  settingsSetup();
  setupColors(CurrentSettings);

  if(CurrentSettings.IsConfigMode) {
    captivePortalSetup();

    ledcSetup(0 /* LEDChannel */, 5000 /* freq */, 8 /* resolution */);
    ledcAttachPin(TFT_BL_PIN, 0 /* LEDChannel */);
    ledcWrite(0 /* LEDChannel */, 127); /* 0-255 */

    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(COLOR_BG);

    drawConfig();
    return;
  }

  joystickCanvas.setTextWrap(false);
  jsInfoCanvas.setTextWrap(false);

  menuEncoder.begin(ENCODER_CLK_PIN, ENCODER_DT_PIN, 4);
  menuEncoder.setChangedHandler(rotate);
  menuEncoder.setLowerBound(0);

  encoderMaxCount = menuSize - 1;
  menuEncoder.setUpperBound(encoderMaxCount);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &handleLoop, true);
  timerAlarmWrite(timer, 10000, true); // every 0.1 seconds
  timerAlarmEnable(timer);

  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);

  ledcSetup(0 /* LEDChannel */, 5000 /* freq */, 8 /* resolution */);
  ledcAttachPin(TFT_BL_PIN, 0 /* LEDChannel */);
  ledcWrite(0 /* LEDChannel */, 127); /* 0-255 */

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(COLOR_BG);

  drawMenu();
  joystick.init();

  drawStatus("Initializing");
  lastMessage = "Connecting to Wifi Network ";
  lastMessage.concat(CurrentSettings.Connection.SSID);
  drawMessage();
  initWiFi();
  lastMessage = "Connecting to FluidNC at ";
  lastMessage.concat(CurrentSettings.Connection.Address);
  lastMessage.concat(":");
  lastMessage.concat(CurrentSettings.Connection.Port);
  drawMessage();
  initWebSocket();
}

void home(String axis) {
  Serial.printf("Home - %s - %s\n", axis, grblState);
  if(grblState == "Idle") {
    lastMessage = "Homing " + axis;
    drawMessage();
    if(axis == "All") {
      webSocket.sendTXT("$H\n");
    } else if(axis == "X") {
      webSocket.sendTXT("$HX\n");
    } else if(axis == "Y") {
      webSocket.sendTXT("$HY\n");
    } else if(axis == "Z") {
      webSocket.sendTXT("$HZ\n");
    }
  }
}

void zero(String axis) {
  Serial.printf("Home - %s - %s\n", axis, grblState);
  if(grblState == "Idle") {
    lastMessage = "Zero " + axis;
    drawMessage();
    if(axis == "All") {
      webSocket.sendTXT("G92 X0 Y0 Z0\n");
    } else if(axis == "XY") {
      webSocket.sendTXT("G92 X0 Y0\n");
    } else if(axis == "X") {
      webSocket.sendTXT("G92 X0\n");
    } else if(axis == "Y") {
      webSocket.sendTXT("G92 Y0\n");
    } else if(axis == "Z") {
      webSocket.sendTXT("G92 Z0\n");
    }
  }
}

void runMacro(String number) {
  Serial.printf("Macro - %s\n", number);
  
  // Set to run macro, not just display info
  webSocket.sendTXT("#<_screenMacroInfoOnly>=0\n");

  // Set specific macro number to run
  String command = "#<_screenMacro>=" + number + "\n";
  webSocket.sendTXT(command);

  webSocket.sendTXT("$SD/Run=_screenmacros.gcode\n");
}

void handleButtonPress()
{
  Serial.println("Button Pressed!");

  if(menuLevel == 0) {
    menuLevel = 1;

    if(menu[menuSelected] == "Jog") {
      jogMenuSelected = 0;
      encoderMaxCount = jogMenuSize-1;
    }
    else if(menu[menuSelected] == "Home") {
      homeMenuSelected = 0;
      encoderMaxCount = homeMenuSize-1;
    }
    else if(menu[menuSelected] == "Macro") {
      macroMenuSelected = 0;
      encoderMaxCount = macroMenuSize-1;
    }
    else if(menu[menuSelected] == "Other") {
      otherMenuSelected = 0;
      encoderMaxCount = otherMenuSize-1;
    }
    else if(menu[menuSelected] == "Main") {
      controlMenuSelected = 0;
      encoderMaxCount = controlMenuSize-1;
    }

    menuEncoder.setUpperBound(encoderMaxCount);
    menuEncoder.resetPosition(0);

    encoderPrevCounter = 0;
    encoderCounter = 0;
  } else if(menuLevel == 1) {
    bool goBack = false;
    // Handle Back
    if(menu[menuSelected] == "Jog") {
      if(jogMenuSelected == 0) {
        jogMenuSelected = -1;
        goBack = true;
      }
    }
    else if(menu[menuSelected] == "Home") {
      if(homeMenuSelected == 0) {
        homeMenuSelected = -1;
        goBack = true;
      } else if(homeMenuSelected > 0) {
        home(homeMenu[homeMenuSelected]);
      }
    }
    else if(menu[menuSelected] == "Macro") {
      if(macroMenuSelected == 0) {
        macroMenuSelected = -1;
        goBack = true;
      } else {
        runMacro(macroMenu[macroMenuSelected]);
      }
    }
    else if(menu[menuSelected] == "Other") {
      if(otherMenuSelected == 0) {
        otherMenuSelected = -1;
        goBack = true;
      } else if(otherMenuSelected > 0) {
        if(otherMenu[otherMenuSelected] == "Conn") {
          if(wsDisconnected)
          {
            lastMessage = "Reconnecting to WebSocket";
            drawMessage();
            initWebSocket();
          } else {
            lastMessage = "WebSocket Already Connected";
            drawMessage();
          }
        } 
        else if(otherMenu[otherMenuSelected] == "Setup") {
          lastMessage = "Rebooting in 5 Seconds into Setup Mode";
          drawMessage();
          setConfigMode(true);
          delay(5000);
          ESP.restart();
        }
      }
    }
    else if(menu[menuSelected] == "Main") {
      if(controlMenuSelected == 0) {
        controlMenuSelected = -1;
        goBack = true;
      } else {
        lastMessage = "Command: " + controlMenu[controlMenuSelected];
        drawMessage();
        if(controlMenu[controlMenuSelected] == "Hold") {
          webSocket.sendTXT("!\n");
        } else if(controlMenu[controlMenuSelected] == "Resume") {
          webSocket.sendTXT("~\n");
        } else if(controlMenu[controlMenuSelected] == "Unlock") {
          webSocket.sendTXT("$X\n");
        } else if(controlMenu[controlMenuSelected] == "Reset") {
          webSocket.sendTXT(0x18);
        } else if(controlMenu[controlMenuSelected] == "Restart") {
          webSocket.sendTXT(0x24);
        }
      }
    }
    
    if(goBack) 
    {
      menuLevel = 0;
      encoderMaxCount = menuSize-1;
      encoderPrevCounter = -1;
      encoderCounter = menuSelected;
      menuEncoder.setUpperBound(encoderMaxCount);
      menuEncoder.resetPosition(encoderCounter);
      return;
    }
  }

  drawMenu();
}

int calcJogTimeMs(int feedrateMmMin, float distance) {
	int a = 200;
	float s = distance;
	float v = (float)feedrateMmMin / 60;
	float dt = s / v;
	int N = 32;
	float T = dt * N;
	float ms = (dt * 1000); 

	float test = pow(v, 2) / (2 * a * (N-1));
	if(ms < test) {
		ms = test;
	}

  // Subtract time to allow for program loop
  ms -= 25;

  // If first jog command, shorten the time
  if(!jogActive) {
    ms -= 75;
  }

	if(ms < 10) {
		ms = 10;
	}

	return (int)ms;
}

void loop() {
  if(CurrentSettings.IsConfigMode) {
    captivePortalLoop();
    return;
  }

  if(!wsDisconnected) { webSocket.loop(); }

  // Process joystick if Jog menu active
  if(menu[menuSelected] == "Jog" && menuLevel == 1) {
    joystick.loop();
  }

  int buttonStateCurr = digitalRead(ENCODER_SW_PIN);

  if((millis() - lastButtonTime) > 100) {
    if(buttonStateCurr != buttonState) {
      buttonState = buttonStateCurr;

      if(buttonState == LOW) {
        handleButtonPress();
      }
      lastButtonTime = millis();
    }
  }

  // Redraw Menu if Encoder Changed
  if (encoderPrevCounter != encoderCounter) {
    encoderPrevCounter = encoderCounter;
    
     if(menu[menuSelected] == "Jog" && menuLevel == 1) {
       setupTextArea(0,55,320,125);

       // If on Back, redraw last message
       if(jogMenuSelected == 0) {
        drawMessage();
       }
     } else if(menu[menuSelected] == "Macro" && menuLevel == 1) {
        if(macroMenuSelected > 0) {
          // Debounce getting macro info
          if((millis() - lastMacroInfoTime) > macroInfoDelay) {
            // Set to only display macro info
            webSocket.sendTXT("#<_screenMacroInfoOnly>=1\n");

            // Set specific macro number to get info for
            String command = "#<_screenMacro>=" + macroMenu[macroMenuSelected] + "\n";
            
            webSocket.sendTXT(command);
            webSocket.sendTXT("$SD/Run=_screenmacros.gcode\n");
            lastMacroInfoTime = millis();
          }
          else {
            encoderPrevCounter = -1; // Reset counter so it gets rechecked
            lastMessage = "Loading...";
            drawMessage();
          }
        } else {
          // Set message
          lastMessage = "Highlight a macro number to view details.";
          drawMessage();
        }
     }

    drawMenu();
  }

  if(menu[menuSelected] == "Jog" && menuLevel == 1) 
  {
    Position position = joystick.getPosition();
    // Feedrate - base on max axis
    int maxFeedrate = 0;
    int feedrate = 0;
    
    if(millis() - lastDrawTime > drawDelay) {

      if(jogMenu[jogMenuSelected] == "XY") {
        maxFeedrate = 4000;
        feedrate = maxFeedrate * ((float)abs(position.x)/100);
        if(abs(position.x) < abs(position.y)) {
          feedrate = maxFeedrate * ((float)abs(position.y)/100);
        }

        jsInfoCanvas.fillScreen(COLOR_BG);
        jsInfoCanvas.setFont(&FreeSans9pt7b);
        jsInfoCanvas.setTextColor(COLOR_TEXT);
        jsInfoCanvas.setCursor(0, 12);
        jsInfoCanvas.printf("X: %i%%\n", position.x);
        jsInfoCanvas.printf("Y: %i%%\n", position.y*-1); // Invert Y sign
        jsInfoCanvas.printf("F: %i\n", feedrate);
        tft.drawRGBBitmap(0, 60, jsInfoCanvas.getBuffer(), jsInfoCanvas.width(), jsInfoCanvas.height());

        joystickCanvas.fillScreen(COLOR_BG);
        joystickCanvas.fillRoundRect(0, 0, 120, 120, 5, COLOR_BG_PANEL);
        joystickCanvas.drawRoundRect(0, 0, 120, 120, 5, COLOR_BORDER);
        joystickCanvas.fillCircle(120/2, 120/2, 45, COLOR_BG_ALT);
        joystickCanvas.drawCircle(120/2, 120/2, 45, COLOR_BORDER);
        joystickCanvas.drawLine(120/2, 0, 120/2, 120, COLOR_BORDER);
        joystickCanvas.drawLine(0, 120/2, 120, 120/2, COLOR_BORDER);
        joystickCanvas.setFont();
        joystickCanvas.setTextColor(COLOR_TEXT);
        joystickCanvas.setCursor((120/2)+2, 5);
        joystickCanvas.print("Y+");
        joystickCanvas.setCursor((120/2)+2, 120-11);
        joystickCanvas.print("Y-");
        joystickCanvas.setCursor(2, (120/2)-8);
        joystickCanvas.print("X-");
        joystickCanvas.setCursor(120-12, (120/2)-8);
        joystickCanvas.print("X+");

        int centerX = 120/2;
        int centerY = 120/2;
        int joyPosX = centerX + (position.x*((float)45/100));
        int joyPosY = centerY + (position.y*((float)45/100));

        joystickCanvas.fillCircle(joyPosX, joyPosY, 10, COLOR_PRIMARY);
        joystickCanvas.drawCircle(joyPosX, joyPosY, 10, COLOR_BORDER);
        joystickCanvas.fillCircle(joyPosX, joyPosY, 2, COLOR_BORDER);

        tft.drawRGBBitmap((320/2)-(joystickCanvas.width()/2), 60, joystickCanvas.getBuffer(), joystickCanvas.width(), joystickCanvas.height());
      }
      else if(jogMenu[jogMenuSelected] == "X") {
        maxFeedrate = 4000;
        feedrate = maxFeedrate * ((float)abs(position.x)/100);

        jsInfoCanvas.fillScreen(COLOR_BG);
        jsInfoCanvas.setFont(&FreeSans9pt7b);
        jsInfoCanvas.setTextColor(COLOR_TEXT);
        jsInfoCanvas.setCursor(0, 12);
        jsInfoCanvas.printf("X: %i%%\n", position.x);
        jsInfoCanvas.printf("F: %i\n", feedrate);
        tft.drawRGBBitmap(0, 60, jsInfoCanvas.getBuffer(), jsInfoCanvas.width(), jsInfoCanvas.height());
        
        joystickCanvas.fillScreen(COLOR_BG);
        joystickCanvas.fillRoundRect(0, 0, 120, 120, 5, COLOR_BG_PANEL);
        joystickCanvas.drawRoundRect(0, 0, 120, 120, 5, COLOR_BORDER);

        joystickCanvas.fillCircle(27, 120/2, 12, COLOR_BG_ALT);
        joystickCanvas.drawCircle(27, 120/2, 12, COLOR_BORDER);
        joystickCanvas.fillCircle(120-27, 120/2, 12, COLOR_BG_ALT);
        joystickCanvas.drawCircle(120-27, 120/2, 12, COLOR_BORDER);
        joystickCanvas.fillRect(27, (120/2)-12, 120-(27*2), 12*2, COLOR_BG_ALT);
        joystickCanvas.drawLine(27, (120/2)-12, 120-27, (120/2)-12, COLOR_BORDER);
        joystickCanvas.drawLine(27, (120/2)+12, 120-27, (120/2)+12, COLOR_BORDER);
        
        joystickCanvas.drawLine(120/2, 40, 120/2, 80, COLOR_BORDER);
        joystickCanvas.drawLine(0, 120/2, 120, 120/2, COLOR_BORDER);
        joystickCanvas.setFont();
        joystickCanvas.setTextColor(COLOR_TEXT);
        joystickCanvas.setCursor(2, (120/2)-8);
        joystickCanvas.print("X-");
        joystickCanvas.setCursor(120-12, (120/2)-8);
        joystickCanvas.print("X+");

        int centerX = 120/2;
        int centerY = 120/2;
        int joyPosX = centerX + (position.x*((float)45/100));
        int joyPosY = centerY;

        joystickCanvas.fillCircle(joyPosX, joyPosY, 10, COLOR_PRIMARY);
        joystickCanvas.drawCircle(joyPosX, joyPosY, 10, COLOR_BORDER);
        joystickCanvas.fillCircle(joyPosX, joyPosY, 2, COLOR_BORDER);

        tft.drawRGBBitmap((320/2)-(joystickCanvas.width()/2), 60, joystickCanvas.getBuffer(), joystickCanvas.width(), joystickCanvas.height());
      }
      else if(jogMenu[jogMenuSelected] == "Y") {
        maxFeedrate = 4000;
        feedrate = maxFeedrate * ((float)abs(position.y)/100);

        jsInfoCanvas.fillScreen(COLOR_BG);
        jsInfoCanvas.setFont(&FreeSans9pt7b);
        jsInfoCanvas.setTextColor(COLOR_TEXT);
        jsInfoCanvas.setCursor(0, 12);
        jsInfoCanvas.printf("Y: %i%%\n", position.y*-1); // Invert Y sign
        jsInfoCanvas.printf("F: %i\n", feedrate);
        tft.drawRGBBitmap(0, 60, jsInfoCanvas.getBuffer(), jsInfoCanvas.width(), jsInfoCanvas.height());

        joystickCanvas.fillScreen(COLOR_BG);
        joystickCanvas.fillRoundRect(0, 0, 120, 120, 5, COLOR_BG_PANEL);
        joystickCanvas.drawRoundRect(0, 0, 120, 120, 5, COLOR_BORDER);
        
        joystickCanvas.fillCircle(120/2, 27, 12, COLOR_BG_ALT);
        joystickCanvas.drawCircle(120/2, 27, 12, COLOR_BORDER);
        joystickCanvas.fillCircle(120/2, 120-27, 12, COLOR_BG_ALT);
        joystickCanvas.drawCircle(120/2, 120-27, 12, COLOR_BORDER);
        joystickCanvas.fillRect((120/2)-12, 27, 12*2, 120-(27*2), COLOR_BG_ALT);
        joystickCanvas.drawLine((120/2)-12, 27, (120/2)-12, 120-27, COLOR_BORDER);
        joystickCanvas.drawLine((120/2)+12, 27, (120/2)+12, 120-27, COLOR_BORDER);

        joystickCanvas.drawLine(120/2, 0, 120/2, 120, COLOR_BORDER);
        joystickCanvas.drawLine(40, 120/2, 80, 120/2, COLOR_BORDER);

        joystickCanvas.setFont();
        joystickCanvas.setTextColor(COLOR_TEXT);
        joystickCanvas.setCursor((120/2)+2, 5);
        joystickCanvas.print("Y+");
        joystickCanvas.setCursor((120/2)+2, 120-11);
        joystickCanvas.print("Y-");

        int centerX = 120/2;
        int centerY = 120/2;
        int joyPosX = centerX;
        int joyPosY = centerY + (position.y*((float)45/100));

        joystickCanvas.fillCircle(joyPosX, joyPosY, 10, COLOR_PRIMARY);
        joystickCanvas.drawCircle(joyPosX, joyPosY, 10, COLOR_BORDER);
        joystickCanvas.fillCircle(joyPosX, joyPosY, 2, COLOR_BORDER);

        tft.drawRGBBitmap((320/2)-(joystickCanvas.width()/2), 60, joystickCanvas.getBuffer(), joystickCanvas.width(), joystickCanvas.height());
      }
      else if(jogMenu[jogMenuSelected] == "Z") {
        maxFeedrate = 1200;
        feedrate = maxFeedrate * ((float)abs(position.y)/100);

        jsInfoCanvas.fillScreen(COLOR_BG);
        jsInfoCanvas.setFont(&FreeSans9pt7b);
        jsInfoCanvas.setTextColor(COLOR_TEXT);
        jsInfoCanvas.setCursor(0, 12);
        jsInfoCanvas.printf("Z: %i%%\n", position.y*-1); // Invert Y sign
        jsInfoCanvas.printf("F: %i\n", feedrate);
        tft.drawRGBBitmap(0, 60, jsInfoCanvas.getBuffer(), jsInfoCanvas.width(), jsInfoCanvas.height());

        joystickCanvas.fillScreen(COLOR_BG);
        joystickCanvas.fillRoundRect(0, 0, 120, 120, 5, COLOR_BG_PANEL);
        joystickCanvas.drawRoundRect(0, 0, 120, 120, 5, COLOR_BORDER);
        
        joystickCanvas.fillCircle(120/2, 27, 12, COLOR_BG_ALT);
        joystickCanvas.drawCircle(120/2, 27, 12, COLOR_BORDER);
        joystickCanvas.fillCircle(120/2, 120-27, 12, COLOR_BG_ALT);
        joystickCanvas.drawCircle(120/2, 120-27, 12, COLOR_BORDER);
        joystickCanvas.fillRect((120/2)-12, 27, 12*2, 120-(27*2), COLOR_BG_ALT);
        joystickCanvas.drawLine((120/2)-12, 27, (120/2)-12, 120-27, COLOR_BORDER);
        joystickCanvas.drawLine((120/2)+12, 27, (120/2)+12, 120-27, COLOR_BORDER);

        joystickCanvas.drawLine(120/2, 0, 120/2, 120, COLOR_BORDER);
        joystickCanvas.drawLine(40, 120/2, 80, 120/2, COLOR_BORDER);

        joystickCanvas.setFont();
        joystickCanvas.setTextColor(COLOR_TEXT);
        joystickCanvas.setCursor((120/2)+2, 5);
        joystickCanvas.print("Z+");
        joystickCanvas.setCursor((120/2)+2, 120-11);
        joystickCanvas.print("Z-");

        int centerX = 120/2;
        int centerY = 120/2;
        int joyPosX = centerX;
        int joyPosY = centerY + (position.y*((float)45/100));

        joystickCanvas.fillCircle(joyPosX, joyPosY, 10, COLOR_PRIMARY);
        joystickCanvas.drawCircle(joyPosX, joyPosY, 10, COLOR_BORDER);
        joystickCanvas.fillCircle(joyPosX, joyPosY, 2, COLOR_BORDER);

        tft.drawRGBBitmap((320/2)-(joystickCanvas.width()/2), 60, joystickCanvas.getBuffer(), joystickCanvas.width(), joystickCanvas.height());
      }

      lastDrawTime = millis();
    }
    
    if((millis() - lastJogTime) > jogDelay) {

      float jogX = position.x * (0.1f);
      float jogY = position.y * (0.1f) * -1; // Invert Y

      // Prevent displaying -0.000
      if(abs(jogX) < 0.001) { jogX = 0; }
      if(abs(jogY) < 0.001) { jogY = 0; }

      if(feedrate == 0) {
        // Don't do anything
      }
      else if(jogMenu[jogMenuSelected] == "XY") {
        float distance = sqrt(pow(jogX, 2) + pow(jogY, 2));

        int jogTimeMs = 0;

        if(jogX != 0 || jogY != 0) {
          jogTimeMs = calcJogTimeMs(feedrate, distance);
          Serial.printf("$J=G91 G21 X%.3f Y%.3f F%i - Time: %ims\n", jogX, jogY, feedrate, jogTimeMs);
          char jogCmd[255];
          sprintf(jogCmd, "$J=G91 G21 X%.3f Y%.3f F%i\n", jogX, jogY, feedrate);
          webSocket.sendTXT(jogCmd);
          jogActive = true;
          jogDelay = jogTimeMs;
          lastJogTime = millis();
        } else if(jogActive) {
          Serial.println("Cancel Jog");
          webSocket.sendTXT(0x85);
          jogActive = false;
          jogDelay = 200;
        }
      }
      else if(jogMenu[jogMenuSelected] == "X") {
        float distance = abs(jogX);

        int jogTimeMs = 0;

        if(jogX != 0) {
          jogTimeMs = calcJogTimeMs(feedrate, distance);
          Serial.printf("$J=G91 G21 X%.3f F%i - Time: %ims\n", jogX, feedrate, jogTimeMs);
          char jogCmd[255];
          sprintf(jogCmd, "$J=G91 G21 X%.3f F%i\n", jogX, feedrate);
          webSocket.sendTXT(jogCmd);
          jogActive = true;
          jogDelay = jogTimeMs;
          lastJogTime = millis();
        } else if(jogActive) {
          Serial.println("Cancel Jog");
          webSocket.sendTXT(0x85);
          jogActive = false;
          jogDelay = 200;
        }
      }
      else if(jogMenu[jogMenuSelected] == "Y") {
        float distance = abs(jogY);

        int jogTimeMs = 0;

        if(jogY != 0) {
          jogTimeMs = calcJogTimeMs(feedrate, distance);
          Serial.printf("$J=G91 G21 Y%.3f F%i - Time: %ims\n", jogY, feedrate, jogTimeMs);
          char jogCmd[255];
          sprintf(jogCmd, "$J=G91 G21 Y%.3f F%i\n", jogY, feedrate);
          webSocket.sendTXT(jogCmd);
          jogActive = true;
          jogDelay = jogTimeMs;
          lastJogTime = millis();
        } else if(jogActive) {
          Serial.println("Cancel Jog");
          webSocket.sendTXT(0x85);
          jogActive = false;
          jogDelay = 200;
        }
      }
      else if(jogMenu[jogMenuSelected] == "Z") {
        float distance = abs(jogY);

        int jogTimeMs = 0;

        if(jogY != 0) {
          jogTimeMs = calcJogTimeMs(feedrate, distance);
          Serial.printf("$J=G91 G21 Z%.3f F%i - Time: %ims\n", jogY, feedrate, jogTimeMs);
          char jogCmd[255];
          sprintf(jogCmd, "$J=G91 G21 Z%.3f F%i\n", jogY, feedrate);
          webSocket.sendTXT(jogCmd);
          jogActive = true;
          jogDelay = jogTimeMs;
          lastJogTime = millis();
        } else if(jogActive) {
          Serial.println("Cancel Jog");
          webSocket.sendTXT(0x85);
          jogActive = false;
          jogDelay = 200;
        }
      }
    }
  }
}