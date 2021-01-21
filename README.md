## Example Summary
The goal of this project was to create a CHIP-8 gamesystem using the Texas Instruments MSP432 microcontroller.
We would start by using the Texas Instruments Educational BoosterPack MII as the user interface for our game system.
The game is displayed on the BoosterPack's colored 128x128 matrix LCD. Using MSP Graphics Library, 
the MSP432 sends data to the LCD's controller through SPI communication to display the data.
## BoosterPack Requirements

This program requires the [Educational BoosterPack MKII (BOOSTXL-EDUMKII)](http://www.ti.com/tool/BOOSTXL-EDUMKII) paired with the MSP-EXP432P401R LaunchPad

## Example Usage

You will need to change the file "chip8.c" in the loadgame portion of the script to your specific file system. The program is configured with the game so that spaceinvaders is the first game booted up, change the game by editing the loadgame function. Games will run but you might need to edit the controls for your specific game. Off the bat space invders, astro dodge and breakout will work without configuration. 

## Application Design Details

* No-RTOS
  * Clock System is configured with MCLK = 48 MHz.
  
