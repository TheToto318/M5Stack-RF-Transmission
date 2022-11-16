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

uint8_t state, i, j, RxSeq, erreur, rang;
uint32_t attente; //full-duplex
uint8_t rxbuf[RF95_MAX_MESSAGE_LEN]; // Tableau de trames recues de taille RF95_MAX_MESSAGE_LEN
uint8_t txbuf[RF95_MAX_MESSAGE_LEN];
uint8_t rxbuflen = RF95_MAX_MESSAGE_LEN; //Taille max buffer 
uint8_t rxlen = RF95_MAX_MESSAGE_LEN; //Taille trame recue
uint8_t txlen = RF95_MAX_MESSAGE_LEN;
uint8_t FCSc[1];
uint8_t FCSr;
uint16_t Sr, SPr, Sc, SPc;
// int rxFrames; //Nbr de trames recues
char str_out[255];

#define E0 0    // Listen for messages
#define E1 1   //  Wait for messages
#define E2 2 // Print messages 
#define E3 3 // Return ACK
#define E4 4 //Correct error
#define E5 5 //Check for error

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
        state = E5;
        rxbuf[2] = 140; // Generate error on frame.
      }
      else {
          delay(2000);
          state = E0;
      }
    break;

    case E4:
      memset(str_out, 0, sizeof(str_out));
      // Serial.printf("\nlS[0] : %d\n", rxbuf[rxlen-4]);
      // Serial.printf("\nlS[1] : %d\n", rxbuf[rxlen-3]);
      // Serial.printf("\nlSP[0] : %d\n", rxbuf[rxlen-2]);
      // Serial.printf("\nlSP[1] : %d\n", rxbuf[rxlen-1]);
      Sr = rxbuf[rxlen-4] + rxbuf[rxlen-3] * 256;
      SPr = rxbuf[rxlen-2] + rxbuf[rxlen-1] * 256;

      Sc = 0; SPc = 0;

      for (i=0; i<=rxlen-6; i++)
      {
        Serial.printf("%d ", rxbuf[i]);
        Sc = Sc + rxbuf[i];
        SPc = SPc + rxbuf[i]*(i+1);
      }

      Serial.printf("\nSc: %d / SPc: %d\n", Sc, SPc);
      Serial.printf("\nSr: %d / SPr: %d\n", Sr, SPr);

      if (Sr != Sc)
      {
        printString("Correcting error\r");
        erreur = Sc-Sr;
        memset(str_out, 0, sizeof(str_out));
        snprintf(str_out, sizeof(str_out), "Error value = %d\r", erreur);
        printString(str_out);
        rang = (SPc-SPr) / erreur;
        Serial.printf("Rang error : %d\n", rang);
        memset(str_out, 0, sizeof(str_out));
        snprintf(str_out, sizeof(str_out), "Error index = %d\r", rang);

        if (rang !=0)
          {
            memset(str_out, 0, sizeof(str_out));
            snprintf(str_out, sizeof(str_out), "Correcting %d to %d\r", rxbuf[rang-1], rxbuf[rang-1]-erreur);
            printString(str_out);
            rxbuf[rang-1] = rxbuf[rang-1] - erreur;
            delay(3000);
            state = E2;
          }
          else
          { 
          if (SPr != SPc)
            {
              memset(str_out, 0, sizeof(str_out));
              snprintf(str_out, sizeof(str_out), "Error on redundancy\r");
              printString(str_out);
              delay(3000);
              state = E0;
            }
        }
      }
      else 
      {
        state = E2;
      }
    break;

    case E5:
      if (rxbuf[0] == TYPE_DATA)
        {
          for (i=0; i<=rxlen-1; i++)
          {
              Serial.printf("%d ", rxbuf[i]);
          } 
          Serial.println();
          FCSr = rxbuf[rxlen-5];
          //Serial.printf("rxlen : %d\n", rxlen);
          //Serial.printf("\nFCS Recus : %d\n", FCSr);
          FCSc[0] = 0;
          for (i=0; i<=rxlen-6; i++)
          {
              FCSc[0] = FCSc[0] ^ rxbuf[i];
          }
          //Serial.printf("\nFCS Calcule : %d\n", FCSc[0]);
          if (FCSc[0]==FCSr)
          {
            state = E2;
            //Serial.printf("E2 ok\n");
          }
          else { state = E4; }
        }
    break;



    case E2:
      if (RxSeq != rxbuf[1])
        {
          RxSeq = rxbuf[1];
          snprintf(str_out, sizeof(str_out), "Rx SEQ n. : %d, Taille :  %d octets\r", RxSeq, rxlen);
          printString(str_out);
          int h = 0;
          memset(str_out, 0, sizeof(str_out));
          for (j=2; j<=rxlen-6; j++) //Avoid the two first (DATA_TYPE and ACK) bytes to only show the payload
          {
            //Serial.printf("%d ", rxbuf[j]);
            str_out[h] = rxbuf[j]; //Geerate the payload string.
            h++;
          }
          printString(str_out);
          //Serial.printf("\n%s", str_out);
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
      txbuf[2] = txbuf[0] ^ txbuf[1];
      rf95.send(txbuf, 3);
      rf95.waitPacketSent();
      state = E0;
      break;
    
    default:
      break;
  }  
}
