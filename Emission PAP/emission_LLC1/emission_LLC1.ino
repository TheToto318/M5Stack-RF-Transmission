#include <SPI.h>
#include <RH_RF95.h>
#include <stdio.h>
#include <string.h>

#define RFM95_CS 5 // Output pin -> lora module
#define RFM95_DI00 36 // Pin d'interruption connecté a la ligne d'interruption du module RFM DIO0 
RH_RF95 rf95(RFM95_CS, RFM95_DI00); //instance couche radio

#define rf95_MAX_MESSAGE_LEN 32

uint8_t state; //état courant
uint8_t txbuf[rf95_MAX_MESSAGE_LEN]; // tableau de trames à émettre de taille rf95_MAX_MESSAGE_LEN 
uint8_t txbuflen = rf95_MAX_MESSAGE_LEN; //taille trame à émettre 

#define EMISSION 0
#define DELAI 1

char strToASCII (char str)
{
  char strASCII[rf95_MAX_MESSAGE_LEN];
  int init_size = strlen(str);

  for (int i = 0; i < init_size; i++)
  {
    snprintf(strASCII[i], sizeof(str), "%d", str[i]);/* Convert the character to integer, in this case the character's ASCII equivalent */ 
  }
  return strASCII;
}

void setup (){
  Serial.begin(115200);
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

      txbuf[0] = 0x0AA;
      txbuf[1] = 0x055;

      rf95.send(txbuf, 2); 
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
