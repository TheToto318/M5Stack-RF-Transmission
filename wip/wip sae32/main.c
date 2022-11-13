#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
    char data_to_send[] = "Hello World!";
    uint8_t final_str[32];
    int i;

    final_str[0] = 1;
    final_str[1] = 55;

    //memcpy(final_str, txbuf_prefix, sizeof(txbuf_prefix));
    //memcpy(final_str+sizeof(txbuf_prefix), (uint8_t*)data_to_send, sizeof(data_to_send));

    memcpy(final_str+2, data_to_send, sizeof(data_to_send));


    for (i=0; i<=sizeof(final_str); i++)
    {
        printf("%c ", final_str[i]);
    }


    printf("\nTaille txbuf : %d\n", sizeof(final_str));
    //printf("Taille data : %d\n", sizeof(data_to_send));
}
