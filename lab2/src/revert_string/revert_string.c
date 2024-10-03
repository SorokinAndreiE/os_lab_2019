#include "revert_string.h"

void RevertString(char *str)
{
	int len = 0;
	len=strlen(str);
	int i,j=0;
	for (i=0;i<=len/2;i++){
		*(str + len - i) = *(str + i);  
		*(str + i) = *(str + len - i - 1);

	}
	for (i = len / 2; i <= len; i++)
        *(str + i) = *(str + i + 1);
        
    //Устанавливаем символ завершения строки
    *(str + len) = '\0';
}

