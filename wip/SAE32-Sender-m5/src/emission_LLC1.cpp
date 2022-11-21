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

uint8_t state, RxSeq, TxSeq, credit, backoff, NewFrame, EIT; //état courant
uint32_t attente; // Durée chien de garde
uint8_t txbuf[rf95_MAX_MESSAGE_LEN]; // tableau de trames à émettre de taille rf95_MAX_MESSAGE_LEN 
uint8_t rxbuf[rf95_MAX_MESSAGE_LEN];
uint8_t txbuflen = rf95_MAX_MESSAGE_LEN; //taille trame à émettre
uint8_t rxlen = rf95_MAX_MESSAGE_LEN;
uint8_t FCS[1]; //Champ de controle d'un octet
uint16_t S; //Code correcteur d'erreur
uint16_t SP; 
uint8_t lS[2]; //S sur deux octets
uint8_t lSP[2];//SP sur deux octets


int i; //Index
char data_to_send[] = "Salut !";
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

#define MyAddr 1
#define ReAddr 0

void setup (){
  //Serial.begin(115200);
  M5.begin();
  M5.Power.begin();
  if (!rf95.init())
    printString("rf95 init error\r");
  else
    printString("rf95 init OK\r");

  //config fréquence radio
  rf95.setTxPower(8); //8dbm
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); 
  rf95.setFrequency(867.7); //137 to 1020 Mhz

  state = E0; //état de départ

  delay(2000);

  TxSeq = 0; credit = 5;

  NewFrame = 1;
  randomSeed(analogRead(3));

  printString("initialization complete\r");
}

void loop () {
  switch (state) {
    case E0:
      if (NewFrame == 1)
      {
        EIT = random(5, 100);
        delay(EIT);
        snprintf(str_out, sizeof(str_out), "EIT %d : \r", EIT);
        printString(str_out);
      }
      
      snprintf(str_out, sizeof(str_out), "New Frame ? : %d\r", NewFrame);
      printString(str_out);

      snprintf(str_out, sizeof(str_out), "Sending %d : \r", TxSeq);
      printString(str_out);

      //Add sender and receiver address + data type and seq number.

      txbuf[0] = MyAddr;
      txbuf[1] = ReAddr;
      txbuf[2] = TYPE_DATA;
      txbuf[3] = TxSeq;

      memcpy(txbuf+4, data_to_send, sizeof(data_to_send)); //Merge frame prefix with payload
  	  
      //Add detector error code
      FCS[0] = 0;
      for (i=0; i<=sizeof(data_to_send)+2; i++)
      {
        FCS[0] = FCS[0] ^ txbuf[i];
      }

      memcpy(txbuf+sizeof(data_to_send)+3, FCS, sizeof(FCS));

      //Add error correcting code
      S=0; SP=0;
      for (i=0; i<=sizeof(data_to_send)+2; i++)
      {
        Serial.printf("%d ", txbuf[i]);
        S = S + txbuf[i];
        SP = SP + txbuf[i]*(i+1);
      }

      Serial.printf("\nS = %d\n", S);
      Serial.printf("SP = %d\n", SP);

      lS[0] = S & 0x00FF;
      lS[1] = (S & 0xFF00) >>8;

      lSP[0] = SP & 0x00FF;
      lSP[1] = (SP & 0xFF00) >>8;

      memcpy(txbuf+sizeof(data_to_send)+4, lS, sizeof(lS));
      memcpy(txbuf+sizeof(data_to_send)+6, lSP, sizeof(lSP));

      for (i=0; i<=sizeof(data_to_send)+7; i++)
      {
              Serial.printf("%d ", txbuf[i]);
      } 

      Serial.println();

      rf95.send(txbuf, sizeof(data_to_send)+8); //Size of the frame = DATA_TYPE + ACK + Payload = payload size + 2 bytes
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
          if (rxbuf[1] != MyAddr) {
            printString("Incorrect destination address\r");
            state = E2;
          }
          
          if (rxbuf[2] != TYPE_ACK) {
            printString("Frame is not ACK type\r");
            state = E2;
          }

          if (rxbuf[3] != TxSeq) {
            printString("Incorrect ACK sequence number\r");
            state = E2;
          }

          if ((rxbuf[2] ^ rxbuf[3])!=rxbuf[4]) {
            printString("ACK frame corrupted, retrying...\r");
            state = E5;
          }

          state = E4;

          
          //rxbuf[2] = rxbuf[2] + 1; //simulate XOR error on ACK
        }
      }
      break;
    
    case E4:
      printString("ACK_RECEIVED\r");
      state = E0; TxSeq++; credit = 5;
      break;
    
    case E5:
      if (credit == 0)
      {
        state = E0; NewFrame = 1;
        credit = 5; TxSeq++;
        break;
      }
      else 
      {
      snprintf(str_out, sizeof(str_out), "Collision ? Retry n. %d : \r", 5-credit);
      printString(str_out);
      snprintf(str_out, sizeof(str_out), "Backoff : %d : \r", backoff);
      printString(str_out);
      backoff = random(0,100);
      delay(backoff);
      state = E0; NewFrame = 0;
      break;
      }

    default:
      state = E0;
      break;
  }
}
