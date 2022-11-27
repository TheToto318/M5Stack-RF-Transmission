#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main()
{
    char *modemConfig[5] = {"Bw125Cr45Sf128", "Bw500Cr45Sf128", "Bw31_25Cr48Sf512", "Bw125Cr48Sf4096", "Bw125Cr45Sf2048"};
    char data_to_send[] = "co";
    uint8_t final_str[32];
    int i;
    uint8_t FCS[1];
    uint16_t S; //Code correcteur d'erreur
    uint16_t SP;
    uint8_t lS[2]; //S sur deux octets
    uint8_t lSP[2];//SP sur deux octets

    final_str[0] = 1;
    final_str[1] = 2;
    final_str[2] = 3;
    final_str[3] = 4;

    printf("Taille data debut : %d\n", sizeof(data_to_send));
    printf("Taille data debut (strlen) : %d\n", strlen(modemConfig[0]));


    //memcpy(final_str, txbuf_prefix, sizeof(txbuf_prefix));
    //memcpy(final_str+sizeof(txbuf_prefix), (uint8_t*)data_to_send, sizeof(data_to_send));

    memcpy(final_str+4, data_to_send, strlen(data_to_send));

    printf("Taille FCS : %d\n", sizeof(FCS));

    for (i=0; i<=strlen(data_to_send)+1; i++)
    {
        printf("%d ", final_str[i]);
    }

    printf("\n");

    FCS[0] = 0;
    for (i=0; i<=strlen(data_to_send)+3; i++)
    {
        printf("%d ", final_str[i]);
        FCS[0] = FCS[0] ^ final_str[i];
    }

    memcpy(final_str+strlen(data_to_send)+4, FCS, sizeof(FCS));

    printf("\nFCS = %d\n", FCS[0]);

    for (i=0; i<=strlen(data_to_send)+4; i++)
    {
        printf("%d ", final_str[i]);
    }
    printf("\n");
    //Add error correcting code
      S=0; SP=0;
      for (i=0; i<=strlen(data_to_send)+3; i++)
      {
        printf("%d ", final_str[i]);
        S = S + final_str[i];
        SP = SP + final_str[i]*(i+1);
      }

      lS[0] = S & 0x00FF;
      lS[1] = (S & 0xFF00) >>8;

      printf("\nS0 = %d\n", lS[0]);
      printf("S1 = %d\n", lS[1]);

      lSP[0] = SP & 0x00FF;
      lSP[1] = (SP & 0xFF00) >>8;

      printf("\nSP0 = %d\n", lSP[0]);
      printf("SP1 = %d\n", lSP[1]);

      memcpy(final_str+strlen(data_to_send)+5, lS, sizeof(lS));

      printf("\n");

    for (i=0; i<=strlen(data_to_send)+6; i++)
    {
        printf("%d ", final_str[i]);
    }

    printf("\n");

    memcpy(final_str+strlen(data_to_send)+7, lSP, sizeof(lSP));


    for (i=0; i<=strlen(data_to_send)+8; i++)
    {
        printf("%d ", final_str[i]);
    }


    printf("\nTaille txbuf : %d\n", sizeof(final_str));
    printf("Taille data complete : %d\n", strlen(data_to_send)+9);
}
