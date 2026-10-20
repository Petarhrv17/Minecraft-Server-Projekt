@echo off
chcp 65001 >nul
title Minecraft ESP Display Bridge
cd /d "%~dp0"

echo ========================================================
echo             MINECRAFT ESP DISPLAY BRIDGE
echo ========================================================
echo.
echo Starte Verbindung zum ESP ueber USB...
echo.

python esp_bridge.py

pause
