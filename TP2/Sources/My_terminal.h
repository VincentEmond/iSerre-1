/*
 * My_terminal.h
 *
 *  Created on: 4 oct. 2016
 *      Author: Julien
 */

#ifndef SOURCES_MY_TERMINAL_H_
#define SOURCES_MY_TERMINAL_H_

#include "Console.h"
#include "stdlib.h"

void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    char temp;
    while (start < end)
    {
    	temp = *(str+start);
    	*(str+start) = *(str+end);
    	*(str+end) = temp;
        start++;
        end--;
    }
}

char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}

void Terminal_ReadChar_NonBlocking(char* c);

bool Terminal_KeyPressed()
{
	// TODO : implement
	return true;
}

void Terminal_ReadChar(char* value)
{
	*value = GETCHAR();
}

void Terminal_SendChar(char value)
{
	PUTCHAR(value);
}

/*
 * Send a character string to Console output.
 * Use it instead of PRINTF if you only want to print string and not number
 */
void Terminal_SendStr(char* str)
{
	char* ptr = str;
	while(ptr)
	{
		PUTCHAR(*ptr);
		ptr++;
	}
}

void Terminal_Cls()
{
	Terminal_SendStr("\033[2J");
}

void Terminal_CRLF()
{
	Terminal_SendStr("\n\r");
}

void Terminal_CursorHome()
{
	Terminal_SendStr("\x1B[H");
}

void Terminal_SendNum(int number, int base)
{
	char temp[20];
	Terminal_SendStr(itoa(number, temp, base));
}

/*
 * Read unsigned 32-bits integer from stdio
 * The entered characters are send one by one to stdio
 */
uint32_t getuint32();

#endif /* SOURCES_MY_TERMINAL_H_ */
