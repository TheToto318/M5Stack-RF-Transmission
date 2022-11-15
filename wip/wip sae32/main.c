#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
    char data_to_send[] = "co";
    uint8_t final_str[32];
    int i;
    uint8_t FCS[1];

    final_str[0] = 1;
    final_str[1] = 0;

    printf("Taille data debut : %d\n", sizeof(data_to_send));

    //memcpy(final_str, txbuf_prefix, sizeof(txbuf_prefix));
    //memcpy(final_str+sizeof(txbuf_prefix), (uint8_t*)data_to_send, sizeof(data_to_send));

    memcpy(final_str+2, data_to_send, sizeof(data_to_send));

    printf("Taille FCS : %d\n", sizeof(FCS));

    for (i=0; i<=sizeof(data_to_send); i++)
    {
        printf("%d ", final_str[i]);
    }

    FCS[0] = 0;
    for (i=0; i<=sizeof(data_to_send); i++)
    {
        FCS[0] = FCS[0] ^ final_str[i];
    }

    memcpy(final_str+sizeof(data_to_send)+1, FCS, sizeof(FCS));

    printf("\nFCS = %d\n", FCS[0]);

    for (i=0; i<=sizeof(data_to_send)+1; i++)
    {
        printf("%d ", final_str[i]);
    }


    printf("\nTaille txbuf : %d\n", sizeof(final_str));
    printf("Taille data : %d\n", sizeof(data_to_send));
}
