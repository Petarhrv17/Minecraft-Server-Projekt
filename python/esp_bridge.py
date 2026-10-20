import time
import sys
import re
import subprocess
import serial
import serial.tools.list_ports
from mcstatus import JavaServer

# ================= KONFIGURATION =================
MC_HOST = "127.0.0.1"
MC_PORT = 25565
BAUD_RATE = 115200

# RCON Konfiguration (optional)
RCON_PORT = 25575
RCON_PASS = "esp123"
# =================================================

def find_esp_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = p.description.lower()
        if "cp210" in desc or "ch340" in desc or "uart" in desc or "serial" in desc:
            return p.device
    if ports:
        return ports[-1].device
    return None

def is_docker_mc_running():
    """
    Prueft, ob auf dem System ein Minecraft- oder Reviactyl-Server-Container laeuft.
    """
    try:
        res = subprocess.run(
            ["docker", "ps", "--format", "{{.Image}} {{.Status}}"],
            capture_output=True,
            text=True,
            timeout=2
        )
        output = res.stdout.lower()
        # Prueft ob ein typischer Minecraft Server Container laeuft
        if "minecraft" in output or "paper" in output or "mc" in output:
            return True
    except Exception:
        pass
    return False

def get_rcon_tps():
    try:
        from mcrcon import MCRcon
        with MCRcon(MC_HOST, RCON_PASS, port=RCON_PORT, timeout=0.8) as mcr:
            resp = mcr.command("tps")
            match = re.search(r'([0-9]{1,2}\.[0-9]{1,2})', resp)
            if match:
                return match.group(1)
    except Exception:
        pass
    return "20.0"

def main():
    print("==================================================")
    print("  MINECRAFT ESP OLED & LED BRIDGE (USB-SERIELL)   ")
    print("==================================================")
    
    ser = None
    last_port = None
    was_online = False

    while True:
        port = find_esp_port()
        if not port:
            print("[INFO] Kein ESP an den USB-Ports gefunden. Warte...")
            time.sleep(3)
            continue

        if ser is None or port != last_port:
            try:
                print(f"[VERBINDUNG] Oeffne Seriellen Port {port} mit {BAUD_RATE} Baud...")
                ser = serial.Serial(port, BAUD_RATE, timeout=1)
                last_port = port
                print(f"[OK] Erfolgreich mit {port} verbunden!\n")
                time.sleep(2)
            except Exception as e:
                print(f"[FEHLER] Konnte {port} nicht oeffnen: {e}")
                time.sleep(2)
                continue

        # 1. Pruefen, ob der Server auf Port 25565 antwortet
        try:
            server = JavaServer.lookup(f"{MC_HOST}:{MC_PORT}")
            status = server.status()
            online = status.players.online
            max_players = status.players.max
            ping = int(round(status.latency))
            tps = get_rcon_tps()

            # Status: ONLINE -> GRUENE LED
            data_str = f"DATA,{online},{max_players},{tps},{ping}\n"
            ser.write(data_str.encode('utf-8'))
            print(f"[ONLINE] Spieler: {online}/{max_players} | TPS: {tps} | Ping: {ping}ms [GRUENE LED]")
            was_online = True

        except Exception:
            # 2. Server antwortet nicht auf Port 25565
            container_runs = is_docker_mc_running()

            if container_runs:
                # Container laeuft noch: Wenn er gerade online war -> faehrt er runter, sonst startet er
                if was_online:
                    state_msg = "STOPPING\n"
                    print("[STOPPING] Server faehrt herunter... [GELBE LED]")
                else:
                    state_msg = "STARTING\n"
                    print("[STARTING] Server startet... [GELBE LED]")
            else:
                was_online = False
                state_msg = "OFFLINE\n"
                print("[OFFLINE] Server ist komplett gestoppt. [ROTE LED]")

            try:
                ser.write(state_msg.encode('utf-8'))
            except Exception:
                pass

        time.sleep(1.5)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nBeendet durch Benutzer.")
