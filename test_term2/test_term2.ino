
#include <M5Stack.h>

void setup() {
    M5.begin();
    M5.Power.begin();
    
    termInit();
    
    M5.Lcd.setTextFont(4);
    
}

void loop() {
            char str[255] = "Bonjour le monde ! ";
            char c;
            printString(str);
            strcpy(str,"Hello World!\r");
            printString(str);
            strcpy(str,"Phrase numero 3 beaucoup plus longue qui va a la ligne toute seule !");
            printString(str);
            termPutchar('\r'); // aller à la ligne 
            printString("Coucou !!!"); 
            termPutchar('A');
            termPutchar('B');
            c='C';
            termPutchar(c);
            termPutchar('\r'); // aller à la ligne           
            delay(10000);
    
}
