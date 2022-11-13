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

#define rf95_MAX_MESSAGE_LEN 128 //Taulle max message -> 128 octets.

uint8_t state, RxSeq, TxSeq, credit; //état courant
uint32_t attente; // Durée chien de garde
uint8_t txbuf[rf95_MAX_MESSAGE_LEN]; // tableau de trames à émettre de taille rf95_MAX_MESSAGE_LEN 
uint8_t rxbuf[rf95_MAX_MESSAGE_LEN];
uint8_t txbuflen = rf95_MAX_MESSAGE_LEN; //taille trame à émettre
uint8_t rxlen = rf95_MAX_MESSAGE_LEN;
char data_to_send[] = "Une tres tres longue phrase a transmettre !";
char str_out[255]; //String for data output on screen

#define E0 0 // Initialize sending
#define E1 1 // Set watchdog
#define E2 2 // Enable receiver
#define E3 3 // Wait for frames
#define E4 4 // Success (ACK and frame number)
#define E5 5 // Transmission error (watchdog expired)

#define TYPE_DATA 1
#define TYPE_ACK 2
#define TIMEOUT_ACK 4000 //Watchdog -> 4 sec

void setup (){
  //Serial.begin(115200);
  M5.begin();
  M5.Power.begin();
  if (!rf95.init())
    printString("rf95 init error\r");
  else
    printString("rf95 init OK\r");

  //config fréquence radio
  rf95.setTxPower(8);
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  rf95.setFrequency(867.7);

  state = E0; //état de départ

  delay(2000);

  TxSeq = 0; credit = 5;

  printString("initialization complete\r");
}

void loop () {
  switch (state) {
    case E0:
      snprintf(str_out, sizeof(str_out), "Sending %d : \r", TxSeq);
      printString(str_out);

      txbuf[0] = TYPE_DATA;
      txbuf[1] = TxSeq;

      memcpy(txbuf+2, data_to_send, sizeof(data_to_send)); //Merge frame prefix with payload

      rf95.send(txbuf, sizeof(data_to_send)+2); //Size of the frame = DATA_TYPE + ACK + Payload = payload size + 2 bytes
      rf95.waitPacketSent();

      credit--; //Decrement retry count

      state = E1;
      break;
      
    case E1:
      attente = millis() + TIMEOUT_ACK; //Start watchdog
      state = E2;
      break;

    case E2:
      rf95.setModeRx();
      state = E3;
      break;
    
    case E3:
      if (millis() > attente)
        { state = E5; } //Check for watchdog expiration
      else {
        if (rf95.recv(rxbuf, &rxlen))
        {
          if ((rxbuf[0] == TYPE_ACK) && (rxbuf[1] == TxSeq)) //Check if the frame is an ACK and the received code is matching the data frame number.
            { state = E4; }
          else state = E2;
        }
      }
      break;
    
    case E4:
      printString("ACK_RECEIVED\r");
      delay(2000);
      state = E0; TxSeq++; credit = 5;
      break;
    
    case E5:
      if (credit == 0)
      {
        printString("Timout\r");
        state = E0;
        credit = 5; TxSeq++;
        break;
      }
      else 
      {
      snprintf(str_out, sizeof(str_out), "Retry n. %d : \r", 5-credit);
      printString(str_out);
      state = E0;
      delay(2000); //Wait two sec before sending again.
      break;
      }

    default:
      state = E0;
      break;
  }
}
