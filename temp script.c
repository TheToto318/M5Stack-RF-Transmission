#include <stdio.h>
#include <string.h>


char strToASCII (char str[])
{
  char strASCII[32];
  int init_size = strlen(str);

  for (int i = 0; i < init_size; i++)
  {
    snprintf(strASCII[i], 32, "%d", str[i]); /* Convert the character to integer, in this case the character's ASCII equivalent */ 
  }
  return strASCII;
}



int main()
{
	char test[] = "coucou";
	
	testASCII = strToASCII(test);
	printf("%s", testASCII);
	
}