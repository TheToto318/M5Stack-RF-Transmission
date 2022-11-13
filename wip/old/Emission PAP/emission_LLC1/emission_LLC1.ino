#include <SPI.h>
#include <RH_RF95.h>
#include <stdio.h>
#include <string.h>
#include <M5Stack.h>

#define RFM95_CS 5 // Output pin -> lora module
#define RFM95_DI00 36 // Pin d'interruption connecté a la ligne d'interruption du module RFM DIO0 
RH_RF95 rf95(RFM95_CS, RFM95_DI00); //instance couche radio

#define rf95_MAX_MESSAGE_LEN 32

uint8_t state; //état courant
uint8_t txbuf[rf95_MAX_MESSAGE_LEN]; // tableau de trames à émettre de taille rf95_MAX_MESSAGE_LEN 
uint8_t txbuflen = rf95_MAX_MESSAGE_LEN; //taille trame à émettre
char data_to_send[] = "Hello World!";

#define EMISSION 0
#define DELAI 1

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

  state = EMISSION; //état de départ

  delay(2000);

  printString("initialization complete\r");
}

void loop () {
  switch (state) {
    case EMISSION:
      printString("EMISSION\r");

      //txbuf[0] = 0x0AA;
      //txbuf[1] = 0x055;

      rf95.send((uint8_t*)data_to_send, sizeof(data_to_send)); 
      rf95.waitPacketSent();

      state = DELAI;
      break;
      
    case DELAI:
      printString("WAIT\r");
      delay(1000);
      state = EMISSION;
      break;

    default:
      break;
  }
}
