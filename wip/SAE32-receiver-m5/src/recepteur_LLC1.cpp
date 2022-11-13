#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <stdio.h>
#include <string.h>
#include <M5Stack.h>
#include "TFT_Terminal.h"

#define RFM95_CS 5 // Output pin -> lora module
#define RFM95_DI00 36 // Pin d'interruption connecté a la ligne d'interruption du module RFM DIO0 
RH_RF95 rf95(RFM95_CS, RFM95_DI00); //instance couche radio

#define RF95_MAX_MESSAGE_LEN 128

uint8_t state, i, RxSeq;
uint32_t attente; //full-duplex
uint8_t rxbuf[RF95_MAX_MESSAGE_LEN]; // Tableau de trames recues de taille RF95_MAX_MESSAGE_LEN
uint8_t txbuf[RF95_MAX_MESSAGE_LEN];
uint8_t rxbuflen = RF95_MAX_MESSAGE_LEN; //Taille max buffer 
uint8_t rxlen = RF95_MAX_MESSAGE_LEN; //Taille trame recue
uint8_t txlen = RF95_MAX_MESSAGE_LEN;
// int rxFrames; //Nbr de trames recues
char str_out[255];

#define E0 0    // Listen for messages
#define E1 1   //  Wait for messages
#define E2 2 // Print messages 
#define E3 3 // Return ACK

#define TYPE_DATA 1
#define TYPE_ACK 2

void setup (){
  Serial.begin(115200);
  M5.begin();
  M5.Power.begin();
  if (!rf95.init())
    printString("rf95 init error\r");
  else
    printString("rf95 init OK\r");

  //config fréquence radio
  rf95.setTxPower(8); //8dbm
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //changement de frequences possible et de modulation
  rf95.setFrequency(867.7);
  
  printString("initialization complete\r");
  
  delay(2000);

  state = E0;
  RxSeq = 255; //Valeur précédente à 0 -> sur 8 bits non signés c'est 255
}

void loop (){
  int j;
  switch (state) {
    case E0:
      printString("ECOUTE\r");
      rf95.setModeRx(); //mode reception
      state = E1;
    break;

    case E1:
      rxlen = RF95_MAX_MESSAGE_LEN;
      if (rf95.recv(rxbuf, &rxlen)){
        if (rxbuf[0] == TYPE_DATA)
        {
          state = E2;
        }
        else 
          state = E0;
        }
    break;

    case E2:
      if (RxSeq != rxbuf[1])
        {
          RxSeq = rxbuf[1];
          snprintf(str_out, sizeof(str_out), "Rx SEQ n. : %d, Taille :  %d octets\r", RxSeq, rxlen);
          printString(str_out);
          int h = 0;
          for (j=2; j<rxlen; j++) //Avoid the two first (DATA_TYPE and ACK) bytes to only show the payload
          {
            //Serial.printf("%d ", rxbuf[j]);
            str_out[h] = rxbuf[j]; //Geerate the payload string.
            h++;
          }
          //Serial.println();
          printString(str_out);
          printString("\r");
        }
      else 
      {
        printString("Already Received\r");
      }

      state = E3;
      break;
    
    case E3:
      printString("Sending ACK\r");
      txbuf[0] = TYPE_ACK;
      txbuf[1] = rxbuf[1];
      rf95.send(txbuf, 2);
      rf95.waitPacketSent();
      state = E0;
      break;
    
    default:
      break;
  }  
}
