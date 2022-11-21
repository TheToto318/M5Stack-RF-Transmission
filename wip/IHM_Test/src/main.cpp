#include <Arduino.h>
#include <SPI.h>
//#include <RH_RF95.h>
#include <stdio.h>
#include <string.h>
#include <M5Stack.h>
#include "TFT_Terminal.h"

int txPower;
double frequency;
char str_out[255];
bool set_txPower;
bool set_ModemConfig;
bool set_Frequency;
enum RH_RF95{Bw125Cr45Sf128 = 0};

void setup (){
    //Serial.begin(115200);
    M5.begin();
    M5.Power.begin();
    txPower = 8;
    frequency = 867.7;
    set_txPower,set_ModemConfig, set_Frequency = false;
    /* ----------------- Set Tx Power interactively ---------------------------------*/
    printString("Set TX Power (default : 8 dBm)\r");
    printString("Btn A : +, Btn B : confirm , Btn C : -\r");
    while (set_txPower != true) {
       M5.update();
       sprintf(str_out, "Tx Power : %d dBm", txPower);
       M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       if (txPower < 2) {
           txPower = 2;
           sprintf(str_out, "Tx Power : %d dBm (Min atteint !)", txPower);
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       }
       if (txPower > 20) {
           txPower = 20;
           sprintf(str_out, "Tx Power : %d dBm (Max atteint !)", txPower);
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       }
       if (M5.BtnA.wasPressed()) {
           txPower--;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
       } 
       if (M5.BtnA.pressedFor(500)) {
           txPower = txPower - 3;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
           delay(500);
       }
       if (M5.BtnC.wasPressed()) {
           txPower++;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
       }
       if (M5.BtnC.pressedFor(500)) {
           txPower = txPower + 3;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
           delay(500);
       }
       if (M5.BtnA.pressedFor(1000) && M5.BtnC.pressedFor(1000)) {
           txPower = 8;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       }
       if (M5.BtnB.wasPressed()){
           set_txPower = true;
           printString("\r");
       }
    } 
    /*-------------------------Set Modem config interactively---------------------------------------*/
    printString("Select modem configuration : \r");
    while (set_ModemConfig != true){
        M5.update();
        if (M5.BtnB.wasPressed()){
            set_ModemConfig = true;
            printString("\r");
        } 
    }
    /*------------------------Set Frequency interactively--------------------------------------------*/
    printString("Select operating frequency\r");
    while (set_Frequency != true) {
       M5.update();
       sprintf(str_out, "Frequency : %0.1f Mhz", frequency);
       M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       if (frequency < 137.0) {
           txPower = 137.0;
           sprintf(str_out, "Frequency : %0.1f Mhz (Min atteint !)", frequency);
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       }
       if (frequency > 1020.0) {
           txPower = 1020.0;
           sprintf(str_out, "Frequency : %0.1f Mhz (Max atteint !)", frequency);
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       }
       if (M5.BtnA.wasPressed()) {
           frequency = frequency - 0.1;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
       }
       if (M5.BtnA.pressedFor(500)) {
           frequency = frequency - 0.5;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
           delay(500);
       }
       if (M5.BtnA.pressedFor(2000)) {
           frequency = frequency - 10;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
           delay(500);
       }
       if (M5.BtnA.pressedFor(4000)) {
           frequency = frequency - 100;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
           delay(500);
       }
    }
}

void loop() {
    
}