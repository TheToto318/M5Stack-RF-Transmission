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

/*Interactive configuration variables*/
int txPower;
double frequency;
char str_out[255];
bool set_txPower;
bool set_ModemConfig;
bool set_Frequency;
bool setMode;
char *modemConfig[5] = {"Bw125Cr45Sf128", "Bw500Cr45Sf128", "Bw31_25Cr48Sf512", "Bw125Cr48Sf4096", "Bw125Cr45Sf2048"};
char *message_template[3] = {"Salut !", "Test d'emission du M5Stack !", "1234567890"};

/* Common constants */
#define rf95_MAX_MESSAGE_LEN 128 //Taulle max message -> 128 octets.
#define E0 0 // Initialize sending
#define E1 1 // Set watchdog
#define E2 2 // Enable receiver
#define E3 3 // Wait for frames
#define E4 4 // Success (ACK and frame number)
#define E5 5 // Transmission error (watchdog expired)
#define TxMode 6
#define RxMode 7
#define setup_rx 8
#define setup_tx_sender 9
#define setup_tx_receiver 10
#define setup_tx_message 11


#define TYPE_DATA 1
#define TYPE_ACK 2
#define TIMEOUT_ACK 4000 //Watchdog -> 4 sec

uint8_t TxAddr;
uint8_t RxAddr;

uint8_t mode;
int i_mes;
int i, j;

void sender();
void receiver();

/*Sender variables*/
uint8_t state_tx, RxSeq, TxSeq, credit, backoff, NewFrame, EIT; //état courant
uint32_t attente; // Durée chien de garde
uint8_t txbuf[rf95_MAX_MESSAGE_LEN]; // tableau de trames à émettre de taille rf95_MAX_MESSAGE_LEN 
uint8_t rxbuf[rf95_MAX_MESSAGE_LEN];
uint8_t rxlen = rf95_MAX_MESSAGE_LEN;
uint8_t FCS[1]; //Champ de controle d'un octet
uint16_t S; //Code correcteur d'erreur
uint16_t SP; 
uint8_t lS[2]; //S sur deux octets
uint8_t lSP[2];//SP sur deux octets

/*Receiver variables*/
uint8_t erreur, rang, state_rx;
uint8_t FCSc[1];
uint8_t FCSr;
uint16_t Sr, SPr, Sc, SPc;

void setup (){
    //Serial.begin(115200);
    M5.begin();
    M5.Power.begin();
    if (!rf95.init())
        printString("rf95 init error\r");
    else
        printString("rf95 init OK\r");
    txPower = 8;       //        default values
    frequency = 433.1; //
    set_txPower,set_ModemConfig, set_Frequency, setMode = false; //Start interactive setup
    /* ----------------- Set Tx Power interactively ---------------------------------*/
    printString("Set TX Power (default : 8 dBm)\r");
    printString("Btn A : +, Btn B : confirm , Btn C : -\r");
    while (set_txPower != true) {
       M5.update();
       sprintf(str_out, "Tx Power : %d dBm", txPower);
       M5.Lcd.drawString(str_out, xPos, yDraw, 2);                            // Use TFT Terminal library xPos and yDraw pointers to write text on the good position      
       if (txPower < 2) {                                                     //Avoid forbided power value
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
       if (M5.BtnA.wasPressed()) {                                             // Reduce power by pressing the A button
           txPower--;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
       } 
       if (M5.BtnA.pressedFor(500)) {                                           // Reduce power quickly (long press 500 ms)
           txPower = txPower - 3;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
           delay(500);
       }
       if (M5.BtnC.wasPressed()) {                                              // Incrase Power by pressing the C button 
           txPower++;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
       }
       if (M5.BtnC.pressedFor(500)) {                                           //Increase power quickly (long press 500ms)
           txPower = txPower + 3;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
           delay(500);
       }
       if (M5.BtnA.pressedFor(1000) && M5.BtnC.pressedFor(1000)) {              //Revert to default power value (A and B button pressed for 1sec)
           txPower = 8;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
           M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       }
       if (M5.BtnB.wasPressed()){                                               // Confirm choice and exit the loop
           set_txPower = true;
           printString("\r");
       }
    } 
    /*-------------------------Set Modem config interactively---------------------------------------*/
    printString("Select modem configuration : \r");
    i = 0;
    while (set_ModemConfig != true){                                //Browse into string array (modemConfig) to set up the modulation
        M5.update();
        sprintf(str_out, "%s", modemConfig[i]);
        M5.Lcd.drawString(str_out, xPos, yDraw, 2);
        if (M5.BtnA.wasPressed() && i != 0) {
            i--;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
        }
        if (M5.BtnA.wasPressed() && i == 0) {i = 0;}            //Avoid selecting non existent element in array
        if (M5.BtnC.wasPressed() && i != 4) {
            i++;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
        }
        if (M5.BtnC.wasPressed() && i == 4) {i = 4;}                
        if (M5.BtnA.pressedFor(1000) && M5.BtnC.pressedFor(1000)) {     //Default value
            i = 0;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
            M5.Lcd.drawString(str_out, xPos, yDraw, 2);
        }        
        if (M5.BtnB.wasPressed()){                                                  //Configure RF modulation and exit loop
            if (i == 0) {rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);}
            if (i == 1) {rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);}
            if (i == 2) {rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);}
            if (i == 3) {rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);}
            if (i == 4) {rf95.setModemConfig(RH_RF95::Bw125Cr45Sf2048);}
            set_ModemConfig = true;
            printString("\r");
        }
    }
    /*------------------------Set Frequency interactively--------------------------------------------*/
    printString("Select operating frequency\r");
    int canal = 0;
    while (set_Frequency != true) {
       M5.update();
       sprintf(str_out, "Frequency : %0.1f Mhz (canal %d)", frequency, canal);
       M5.Lcd.drawString(str_out, xPos, yDraw, 2);
       if (M5.BtnA.wasPressed() && canal == 0) {i = 0;}
       if (M5.BtnA.wasPressed() && canal != 0) {
           canal--;
           frequency = 433.1 + canal*0.1;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
       }
       if (M5.BtnC.wasPressed()) {
           canal++;
           frequency = 433.1 + canal*0.1;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
       }
       if (M5.BtnB.wasPressed()){
           rf95.setFrequency(frequency);
           printString("\r");
           set_Frequency = true;
       }
    }
    /*------------------------------------Select emission or reception mode------------------------------------------*/
    printString("Select mode : Rx = Btn A, Tx = Btn C\r");
    while (setMode != true) {
        M5.update();
        if (M5.BtnA.wasPressed()) {
            mode = RxMode; printString("Rx mode selected\r");
            RxSeq = 255; state_rx = setup_rx; setMode = true; //Switch to RxMode case, and to the setup state + exit the loop
            }
        if (M5.BtnC.wasPressed()) {
            mode = TxMode; printString("Tx mode selected;\r");
            TxSeq = 0; credit = 5; NewFrame = 1; state_tx = setup_tx_sender; setMode = true; //Switch to TxMode case and the setup state + exit the loop
            randomSeed(analogRead(3));
            }
    }
    printString("Initialization complete\r");
    delay(1000);
}


void loop() {
    switch (mode) {
        case RxMode:
            receiver();
        break;
        case TxMode:
            sender();
        break;
    }
}

void sender() {
switch (state_tx) {
    case setup_tx_sender:
        M5.update();
        sprintf(str_out, "Set your address : %d", TxAddr);
        M5.Lcd.drawString(str_out, xPos, yDraw, 2);
        if (M5.BtnA.wasPressed()) {
            TxAddr--;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK); 
        }
        if (M5.BtnC.wasPressed()) {
           TxAddr++;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
        }
        if (M5.BtnB.wasPressed()) {
            printString("\r");
            state_tx = setup_tx_receiver;
        }
        break;

    case setup_tx_receiver:
        M5.update();
        sprintf(str_out, "Set the receiver address : %d", RxAddr);
        M5.Lcd.drawString(str_out, xPos, yDraw, 2);
        if (M5.BtnA.wasPressed()) {
            RxAddr--;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK); 
        }
        if (M5.BtnC.wasPressed()) {
           RxAddr++;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
        }
        if (M5.BtnB.wasPressed()) {
            printString("\r");
            printString("Set a message to send : \r");
            state_tx = setup_tx_message; i_mes = 0;
        }
        break;	
    case setup_tx_message:
        M5.update();
        sprintf(str_out, "%s", message_template[i_mes]);
        M5.Lcd.drawString(str_out, xPos, yDraw, 2);
        if (M5.BtnA.wasPressed() && i_mes != 0) {
            i_mes--;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
        }
        if (M5.BtnA.wasPressed() && i_mes == 0) {i_mes = 0;}
        if (M5.BtnC.wasPressed() && i_mes != 2) {
            i_mes++;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
        }
        if (M5.BtnC.wasPressed() && i_mes == 2) {i_mes = 2;}  
        if (M5.BtnB.wasPressed()) {
            printString("\r");
            state_tx = E0;
        }               
        break;

    case E0:
      if (NewFrame == 1)
      {
        EIT = random(5, 100);
        delay(EIT);
        sprintf(str_out, "EIT %d : \r", EIT);
        printString(str_out);
      }
      
      sprintf(str_out, "New Frame ? : %d\r", NewFrame);
      printString(str_out);

      sprintf(str_out, "Sending %d : \r", TxSeq);
      printString(str_out);

      //Add sender and receiver address + data type and seq number.

      txbuf[0] = TxAddr;
      txbuf[1] = RxAddr;
      txbuf[2] = TYPE_DATA;
      txbuf[3] = TxSeq;

      memcpy(txbuf+4, message_template[i_mes], strlen(message_template[i_mes])); //Merge frame prefix with payload
  	  
      //Add detector error code
      FCS[0] = 0;
      for (i=0; i<=strlen(message_template[i_mes])+3; i++)
      {
        FCS[0] = FCS[0] ^ txbuf[i];
      }

      memcpy(txbuf+strlen(message_template[i_mes])+4, FCS, sizeof(FCS));

      //Add error correcting code
      S=0; SP=0;
      for (i=0; i<=strlen(message_template[i_mes])+3; i++)
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

      memcpy(txbuf+strlen(message_template[i_mes])+5, lS, sizeof(lS));
      memcpy(txbuf+strlen(message_template[i_mes])+7, lSP, sizeof(lSP));

      for (i=0; i<=strlen(message_template[i_mes])+8; i++)
      {
              Serial.printf("%d ", txbuf[i]);
      } 

      Serial.println();

      rf95.send(txbuf, strlen(message_template[i_mes])+9); //Size of the frame = DATA_TYPE + ACK + Payload = payload size + 2 bytes
      rf95.waitPacketSent();

      credit--; //Decrement retry count

      state_tx = E1;
      break;
      
    case E1:
      attente = millis() + TIMEOUT_ACK; //Start watchdog
      state_tx = E2;
      break;

    case E2:
      rf95.setModeRx();
      state_tx = E3;
      break;
    
    case E3:
      if (millis() > attente)
        { state_tx = E4; } //Check for watchdog expiration
      else {
        if (rf95.recv(rxbuf, &rxlen))
        {
          if (rxbuf[1] != TxAddr) {
            printString("Incorrect destination address\r");
            state_tx = E2;
          }
          
          if (rxbuf[2] != TYPE_ACK) {
            printString("Frame is not ACK type\r");
            state_tx = E2;
          }

          if (rxbuf[3] != TxSeq) {
            printString("Incorrect ACK sequence number\r");
            state_tx = E2;
          }

          if ((rxbuf[2] ^ rxbuf[3])!=rxbuf[4]) {
            printString("ACK frame corrupted, retrying...\r");
            state_tx = E4;
          }

          state_tx = E5;

          
          //rxbuf[2] = rxbuf[2] + 1; //simulate XOR error on ACK
        }
      }
      break;
    
    case E4:
      if (credit == 0)
      {
        state_tx = E0; NewFrame = 1;
        credit = 5; TxSeq++;
        break;
      }
      else 
      {
      sprintf(str_out, "Collision ? Retry n. %d : \r", 5-credit);
      printString(str_out);
      sprintf(str_out, "Backoff : %d : \r", backoff);
      printString(str_out);
      backoff = random(0,100);
      delay(backoff);
      state_tx = E0; NewFrame = 0;
      break;
      }

     case E5:
      printString("ACK_RECEIVED\r");
      state_tx = E0; TxSeq++; credit = 5;
      break;

    default:
      state_tx = E0;
      break;
  }
}

void receiver() {
    switch (state_rx) {
    case setup_rx:
        M5.update();
        sprintf(str_out, "Set your address... : %d", RxAddr);
        M5.Lcd.drawString(str_out, xPos, yDraw, 2);
        if (M5.BtnA.wasPressed()) {
            RxAddr--;
            M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK); 
        }
        if (M5.BtnC.wasPressed()) {
           RxAddr++;
           M5.Lcd.fillRect(xPos,yDraw,320,16,TFT_BLACK);
        }
        if (M5.BtnB.wasPressed()) {
            printString("\r");
            state_rx = E0;
        }
        break;    

    case E0: //Listen mode
      M5.Lcd.drawString("Listening...", xPos, yDraw, 2);
      rf95.setModeRx(); //Receive mode
      state_rx = E1;
    break;
  
    case E1: //When a frame is received
      rxlen = rf95_MAX_MESSAGE_LEN;
      if (rf95.recv(rxbuf, &rxlen)){
        printString("\r"); //Move TFT Terminal pointer
        state_rx = E2;
        rxbuf[4] = 140; // Generate error on frame.
      }
      else {
        delay(2000);
        state_rx = E0;
      }
    break;

    case E2: //Detecting error
      if (rxbuf[1]!=TxAddr)
      {
        state_rx = E0;
      }      
      if (rxbuf[2] == TYPE_DATA)
        {
          for (i=0; i<=rxlen-1; i++)
          {
              Serial.printf("%d ", rxbuf[i]);
          } 
          FCSr = rxbuf[rxlen-5];
          FCSc[0] = 0;
          for (i=0; i<=rxlen-6; i++)
          {
              FCSc[0] = FCSc[0] ^ rxbuf[i];
          }
          Serial.printf("\nFCSr : %d / FCSc : %d\n", FCSr, FCSc[0]);
          if (FCSc[0]==FCSr)
          {
            state_rx = E4;
          }
          else { state_rx = E3; }
        }
    break;

    case E3: //Correcting error 
      Sr = rxbuf[rxlen-4] + rxbuf[rxlen-3] * 256;
      SPr = rxbuf[rxlen-2] + rxbuf[rxlen-1] * 256;

      Sc = 0; SPc = 0;

      for (i=0; i<=rxlen-6; i++)
      {
        Sc = Sc + rxbuf[i];
        SPc = SPc + rxbuf[i]*(i+1);
      }

      Serial.printf("\nSc: %d / SPc: %d\n", Sc, SPc);
      Serial.printf("\nSr: %d / SPr: %d\n", Sr, SPr);

      if (Sr != Sc)
      {
        printString("Correcting error\r");
        erreur = Sc-Sr;
        rang = (SPc-SPr) / erreur;
        sprintf(str_out, "Error value : %d, index : %d\r", erreur, rang-1);
        printString(str_out);

        if (rang !=0)
          {
            sprintf(str_out, "Correcting %d to %d\r", rxbuf[rang-1], rxbuf[rang-1]-erreur);
            printString(str_out);
            rxbuf[rang-1] = rxbuf[rang-1] - erreur;
            state_rx = E4;
          }
          else
          { 
          if (SPr != SPc)
            {
              sprintf(str_out, "Error on redundancy\r");
              printString(str_out);
              state_rx = E0;
            }
        }
      }
      else 
      {
        state_rx = E4;
      }
    break;
    
    case E4: //Show the payload
      if (RxSeq != rxbuf[3])
        {
          RxSeq = rxbuf[3];
          sprintf(str_out, "Rx SEQ n. : %d, Taille :  %d octets\r", RxSeq, rxlen);
          printString(str_out);
          int h = 0;
          memset(str_out, 0, sizeof(str_out));
          for (j=4; j<=rxlen-6; j++) //Avoid the 4 first bytes (Source/Dest adddr, DATA_TYPE and RxSeq) and last five bytes of redundancy.
          {
            str_out[h] = rxbuf[j]; //Geerate the payload string.
            h++;
          }
          printString(str_out);
          printString("\r");
        }
      else 
      {
        printString("Already Received\r");
      }

      state_rx = E5;
      break;
    
    case E5: //Send ACK
      printString("Sending ACK\r");
      txbuf[0] = TxAddr;
      txbuf[1] = rxbuf[0];
      txbuf[2] = TYPE_ACK;
      txbuf[3] = rxbuf[3];
      txbuf[4] = txbuf[2] ^ txbuf[3]; //DATA_TYPE and TxSeq
      rf95.send(txbuf, 5);
      rf95.waitPacketSent();
      state_rx = E0;
      break;

    default:
      break;
  }  
}