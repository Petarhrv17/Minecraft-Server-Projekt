# Minecraft-Server-Projekt
Dieses Repository enthält ein Hardware-Monitoring-System für einen in Docker betriebenen Reviactyl-Minecraft-Server. Über eine serielle Python-Brücke werden Live-Daten wie Spieleranzahl, TPS und Ping in Echtzeit an einen ESP32 übertragen, der diese auf einem SSD1306-OLED-Display und über Status-LEDs visualisiert.


# Reviactyl Minecraft ESP32 Status Monitor

Ein Hardware- und Software-Monitoring-System fuer einen containerisierten Minecraft-Server unter Reviactyl (Docker). 

Ueber eine serielle Python-Bruecke werden Serverzustaende und Telemetriedaten in Echtzeit an einen ESP32-Mikrocontroller uebertragen. Dieser visualisiert die Daten auf einem 0.96 Zoll SSD1306 OLED-Display sowie ueber drei Status-LEDs.

---

## Features

* **Echtzeit-Telemetrie:** Anzeige von aktiver Spieleranzahl, maximalen Slots, Server-Performance (TPS) und Netzwerk-Latenz (Ping in ms).
* **LED-Statuslogik:**
  * **Rot:** Server ist offline oder nicht erreichbar.
  * **Gelb:** Server startet (STARTING), stoppt (STOPPING) oder ist stark ausgelastet (10 oder mehr Spieler online).
  * **Gruen:** Server ist online und betriebsbereit (unter 10 Spieler online).
* **Heartbeat-Indikator:** Ein pulsierendes Pixelmuster auf dem Display signalisiert den laufenden Datenempfang ueber die Schnittstelle.
* **Timeout-Erkennung:** Automatischer Hinweis auf dem Bildschirm, falls die serielle Verbindung unterbrochen oder das Python-Skript gestoppt wird.

---

## Systemarchitektur

1. **Server-Infrastruktur:**
   * Minecraft-Server in Docker, verwaltet ueber das Reviactyl-Panel und den Reviactyl-Agenten.
   * Netzwerkzugriff ueber Tailscale.

2. **Python Bridge (`python/esp_bridge.py`):**
   * Fragt den Serverstatus periodisch lokal ueber das Netzwerk ab.
   * Sendet formatierte Status-Strings ueber die serielle USB-Schnittstelle (115200 Baud) an den ESP32.
   * Schnellstart unter Windows ueber `python/start_esp_display.bat`.

3. **ESP32 Firmware (`arduino/display.ino`):**
   * Verarbeitet serielle Datenpakete (DATA, STARTING, STOPPING, OFFLINE).
   * Rendert UI-Elemente via I2C auf dem SSD1306-OLED.
   * Steuert die GPIOs fuer die drei Status-LEDs.

---

## Hardware-Verkabelung

| Komponente | Pin am Bauteil | ESP32 GPIO / Pin | Anmerkung |
| :--- | :--- | :--- | :--- |
| **OLED (SSD1306)** | SDA | **GPIO 21** | I2C Data |
| | SCL | **GPIO 22** | I2C Clock |
| | VCC | **3.3V** | Spannungsversorgung |
| | GND | **GND** | Masse |
| **Gruene LED** | Anode (+) | **GPIO 18** | Vorwiderstand (220 bis 330 Ohm) an Pin, Kathode an GND |
| **Gelbe LED** | Anode (+) | **GPIO 19** | Vorwiderstand (220 bis 330 Ohm) an Pin, Kathode an GND |
| **Rote LED** | Anode (+) | **GPIO 23** | Vorwiderstand (220 bis 330 Ohm) an Pin, Kathode an GND |

---

## Repository-Struktur

```text
├── arduino/
│   └── display.ino
├── python/
│   ├── esp_bridge.py
│   └── start_esp_display.bat
├── docker/
│   └── compose.yml
└── README.md
```

---

## Inbetriebnahme

1. **Firmware aufspielen:**
   * Die Datei `arduino/display.ino` in der Arduino IDE oeffnen.
   * Die Bibliotheken `Adafruit SSD1306` und `Adafruit GFX` installieren.
   * Das passende Board sowie den COM-Port auswaehlen und den Sketch hochladen.

2. **Minecraft-Server starten:**
   * Den Server ueber die Weboberflaeche von Reviactyl starten.

3. **Python Bridge starten:**
   * Die Datei `python/start_esp_display.bat` ausfuehren oder im Terminal `python esp_bridge.py` aufrufen.
