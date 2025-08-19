#include <M5StickCPlus.h>
#include <WiFi.h>

#define VERSION "0.9-beta"

enum UiState { HOME, SCAN, RESULTS, DETAILS, SPLASH };
UiState uiState = SPLASH;

int selectedNetwork = 0;
int totalNetworks = 0;

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

void drawSplash();
void drawHome();
void drawResults();
void drawDetails(int index);
void drawHeader();
void drawFooter(const char* text);

void setup() {
  M5.begin();
  M5.Axp.ScreenBreath(60);
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);

  RTC_TimeStruct.Hours   = 14;
  RTC_TimeStruct.Minutes = 32;
  RTC_TimeStruct.Seconds = 0;
  M5.Rtc.SetTime(&RTC_TimeStruct);

  RTC_DateStruct.WeekDay = 1;
  RTC_DateStruct.Month   = 8;
  RTC_DateStruct.Date    = 18;
  RTC_DateStruct.Year    = 2025;
  M5.Rtc.SetDate(&RTC_DateStruct);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  drawSplash();
  delay(2000);
  uiState = HOME;
  drawHome();
}

void loop() {
  M5.update();

  if (uiState == HOME) {
    if (M5.BtnA.wasPressed()) {
      uiState = SCAN;
      M5.Lcd.fillScreen(BLACK);
      drawHeader();
      drawFooter("Scanning...");
      totalNetworks = WiFi.scanNetworks();
      selectedNetwork = 0;
      uiState = RESULTS;
      drawResults();
    }
  }

  else if (uiState == RESULTS) {
    if (M5.BtnB.wasPressed()) {
      selectedNetwork++;
      if (selectedNetwork >= totalNetworks) selectedNetwork = 0;
      drawResults();
    }
    if (M5.BtnA.wasPressed()) {
      uiState = DETAILS;
      drawDetails(selectedNetwork);
    }
    if (M5.BtnB.pressedFor(1000)) {
      uiState = HOME;
      drawHome();
    }
  }

  else if (uiState == DETAILS) {
    if (M5.BtnA.wasPressed()) {
      uiState = RESULTS;
      drawResults();
    }
    if (M5.BtnB.wasPressed()) {
      uiState = HOME;
      drawHome();
    }
  }

  if (uiState != SPLASH) {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
      drawHeader();
      lastUpdate = millis();
    }
  }
}

void drawSplash() {
  M5.Lcd.fillScreen(BLACK);

  M5.Lcd.setCursor(40, 50);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(MAGENTA, BLACK);
  M5.Lcd.println("SeekerOS");

  M5.Lcd.setCursor(20, 75);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(YELLOW, BLACK);
  M5.Lcd.println("Beyond The Limits of Control");

  M5.Lcd.setCursor(65, 100);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.printf("Version: %s", VERSION);
}

void drawHeader() {
  M5.Lcd.fillRect(0, 0, 240, 20, MAGENTA);

  M5.Rtc.GetTime(&RTC_TimeStruct);
  char timeBuf[6];
  sprintf(timeBuf, "%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.setTextColor(WHITE, MAGENTA);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print(timeBuf);

  float vbat = M5.Axp.GetBatVoltage();
  int batX = 200;
  int batY = 4;
  int batW = 30;
  int batH = 12;

  M5.Lcd.drawRect(batX, batY, batW, batH, BLACK);
  M5.Lcd.drawRect(batX + batW, batY + 3, 3, 6, BLACK);

  int percent = map((int)(vbat * 100), 330, 420, 0, 100);
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  int level = (batW - 2) * percent / 100;

  uint16_t color = (percent > 20) ? WHITE : RED;
  M5.Lcd.fillRect(batX + 1, batY + 1, level, batH - 2, color);
}

void drawFooter(const char* text) {
  int footerHeight = 15;
  int y = M5.Lcd.height() - footerHeight;

  M5.Lcd.fillRect(0, y, M5.Lcd.width(), footerHeight, DARKGREY);

  M5.Lcd.setTextColor(WHITE, DARKGREY);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString(text, M5.Lcd.width() / 2, y + footerHeight / 2);
}

void drawHome() {
  M5.Lcd.fillScreen(BLACK);
  drawHeader();

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(40, 60);
  M5.Lcd.println("WiFi Scanner");

  drawFooter("Press M5 to Scan");
}

void drawResults() {
  M5.Lcd.fillScreen(BLACK);
  drawHeader();

  if (totalNetworks == 0) {
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.println("No networks found");
    drawFooter("Hold Side for Home");
    return;
  }

  String ssid = WiFi.SSID(selectedNetwork);
  int rssi = WiFi.RSSI(selectedNetwork);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(YELLOW, BLACK);
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.printf("> %s\n", ssid.c_str());
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.printf("Signal: %d dBm\n", rssi);

  drawFooter("Main=Details | Side=Next");
}

void drawDetails(int index) {
  M5.Lcd.fillScreen(BLACK);
  drawHeader();

  String ssid = WiFi.SSID(index);
  int32_t rssi = WiFi.RSSI(index);
  int32_t channel = WiFi.channel(index);
  wifi_auth_mode_t encryption = WiFi.encryptionType(index);

  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("%s\n\n", ssid.c_str());

  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Signal: %d dBm\n", rssi);
  M5.Lcd.printf("Channel: %d\n", channel);

  M5.Lcd.print("Encrypt: ");
  switch (encryption) {
    case WIFI_AUTH_OPEN: M5.Lcd.println("Open"); break;
    case WIFI_AUTH_WEP: M5.Lcd.println("WEP"); break;
    case WIFI_AUTH_WPA_PSK: M5.Lcd.println("WPA-PSK"); break;
    case WIFI_AUTH_WPA2_PSK: M5.Lcd.println("WPA2-PSK"); break;
    case WIFI_AUTH_WPA_WPA2_PSK: M5.Lcd.println("WPA/WPA2-PSK"); break;
    case WIFI_AUTH_WPA2_ENTERPRISE: M5.Lcd.println("WPA2-ENT"); break;
    default: M5.Lcd.println("Unknown"); break;
  }

  drawFooter("Main=Back | Side=Home");
}
