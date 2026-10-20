#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LED Pins definieren
#define PIN_LED_GREEN  18
#define PIN_LED_YELLOW 19
#define PIN_LED_RED    23

int onlinePlayers = 0;
int maxPlayers = 0;
String tps = "20.0";
int ping = 0;
unsigned long lastUpdate = 0;
bool heartbeat = false;

void setLeds(bool red, bool yellow, bool green) {
  digitalWrite(PIN_LED_RED, red ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, yellow ? HIGH : LOW);
  digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
}

void drawHeader() {
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(16, 2);
  display.print("MINECRAFT SERVER");

  if (heartbeat) {
    display.fillRect(122, 3, 4, 6, SSD1306_BLACK);
  }
}

void drawScreenState(const char* title, const char* subtitle) {
  display.clearDisplay();
  drawHeader();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(15, 24);
  display.print(title);
  display.setCursor(15, 42);
  display.print(subtitle);
  display.display();
}

void drawOnlineScreen() {
  display.clearDisplay();
  drawHeader();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 20);
  display.print("Spieler: ");
  display.print(onlinePlayers);
  display.print(" / ");
  display.print(maxPlayers);

  display.setCursor(0, 34);
  display.print("TPS:     ");
  display.print(tps);

  display.setCursor(0, 48);
  display.print("Latenz:  ");
  display.print(ping);
  display.print(" ms");

  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  // Initialer LED-Test
  setLeds(true, true, true);
  delay(300);
  setLeds(false, true, false);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      while (true) delay(100);
    }
  }

  display.clearDisplay();
  drawScreenState("Starte Verbindung", "Warte auf Daten...");
}

void loop() {
  if (Serial.available() > 0) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    // 1. ONLINE-Status mit Live-Werten
    if (line.startsWith("DATA,")) {
      int idx1 = line.indexOf(',');
      int idx2 = line.indexOf(',', idx1 + 1);
      int idx3 = line.indexOf(',', idx2 + 1);
      int idx4 = line.indexOf(',', idx3 + 1);

      if (idx1 != -1 && idx2 != -1 && idx3 != -1 && idx4 != -1) {
        onlinePlayers = line.substring(idx1 + 1, idx2).toInt();
        maxPlayers    = line.substring(idx2 + 1, idx3).toInt();
        tps           = line.substring(idx3 + 1, idx4);
        ping          = line.substring(idx4 + 1).toInt();
        
        lastUpdate = millis();
        heartbeat  = !heartbeat;

        // LED-Logik nach Spieleranzahl:
        // Bei 10 oder mehr Spielern -> GELB
        // Bei weniger als 10 Spielern -> GRUEN
        if (onlinePlayers >= 10) {
          setLeds(false, true, false); // Gelb AN (Rot/Gruen AUS)
        } else {
          setLeds(false, false, true); // Gruen AN (Rot/Gelb AUS)
        }

        drawOnlineScreen();
      }
    } 
    // 2. STARTING-Status (Server bootet)
    else if (line == "STARTING") {
      lastUpdate = millis();
      setLeds(false, true, false); // Gelb AN
      drawScreenState("[ SERVER STARTET ]", "Bitte warten...");
    }
    // 3. STOPPING-Status (Server faehrt herunter)
    else if (line == "STOPPING") {
      lastUpdate = millis();
      setLeds(false, true, false); // Gelb AN
      drawScreenState("[ SERVER STOPPT ]", "Wird beendet...");
    }
    // 4. OFFLINE-Status
    else if (line == "OFFLINE") {
      lastUpdate = millis();
      setLeds(true, false, false); // Rot AN
      drawScreenState("[ SERVER OFFLINE ]", "Nicht aktiv");
    }
  }

  // Timeout-Check (falls Python-Skript beendet wird oder USB trennt)
  if (millis() - lastUpdate > 6000 && lastUpdate != 0) {
    setLeds(false, true, false);
    drawScreenState("Verbindungsverlust", "Warte auf Skript");
  }

  delay(20);
}
