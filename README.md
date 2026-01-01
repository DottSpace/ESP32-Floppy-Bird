# ESP32-Flappy-Bird

A basilar Flappy Bird clone for ESP32 streaming via web.

<img width="727" height="760" alt="Schermata del 2026-01-01 19-57-42" src="https://github.com/user-attachments/assets/606c96b7-0bbc-4c1d-bece-8bf5dedb2092" />

By connecting to the ESP32 WiFi and then entering its IP in your browser, you can connect to the ESP32 and play Flappy Bird live.

## How to play

1. Get an ESP32 and connect it to your computer.  
2. Open your favorite IDE.  
3. If you use PlatformIO, the procedure is:  
   - Open **Visual Studio Code**.  
   - Install the **PlatformIO** extension.  
   - Click on the alien face icon on the left to create a new project.  
   - Select **Arduino** as the framework and **ESP32 Dev Kit** as the board (even if your ESP32 is not exactly a Dev Kit, select Dev Kit anyway).  
4. Open the `src` folder and paste the `main.cpp` code.  
5. Make the same for the `platformio.ini` file.  
6. Build and upload the project to your ESP32.  
7. Connect to the ESP32 WiFi and open the browser at its IP to start playing.
