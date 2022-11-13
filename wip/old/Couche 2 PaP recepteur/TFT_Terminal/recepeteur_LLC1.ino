#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 5 // Output pin -> lora module
#define RFM95_DI00 36 // Pin d'interruption connecté a la ligne d'interruption du module RFM DIO0 
RH_RF95 rf95(RFM95_CS, RFM95_DI00); //instance couche radio

#define RF95_MAX_MESSAGE_LEN 32

int state;
uint8_t rxbuf[RF95_MAX_MESSAGE_LEN]; // Tableau de trames recues de taille RF95_MAX_MESSAGE_LEN
uint8_t rxbuflen = RF95_MAX_MESSAGE_LEN; //Taille max buffer 
uint8_t rxlen = RF95_MAX_MESSAGE_LEN; //Taille trame recue
int rxFrames; //Nbr de trames recues
int j;

//#define ECOUTE 1    //
//#define TEST-RX 2   // 3 états -> schéma automate
//#define AFFICHAGE 3 // 

char str[255];

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
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //changement de frequences possible et de modulation
  rf95.setFrequency(867.7);
  
  printString("initialization complete\r");
  state = 1;
  rxFrames = 0;
  delay(2000);
}

void loop (){
  switch (state) {
    case 1:
      printString("ECOUTE\r");
      rf95.setModeRx(); //mode reception
      state = 2;
    break;
  
    case 2:
      printString("Checking for messages\r");
      delay(2000);
      if (rf95.available()){
        state = 3;
      }
    break;
    case 3:
      if (rf95.recv(rxbuf, &rxlen)){
        rxFrames++;
        snprintf(str, sizeof(str), "Trame[%d] de %d octets\r", rxFrames, rxlen); 
        printString(str);
        for (j=0; j<rxlen; j++){
          snprintf(str, sizeof(str), "%02x\r", rxbuf[j]);
          printString(str);
        }
      }
      state = 1;
    break;
    default:
    break;
  }  
}
