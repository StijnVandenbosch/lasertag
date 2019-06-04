//LCD driver - bewerkte copy van FC4 LCD functies
/*******************************************************************
Start()
The start macro must be called once to initialise the LCD display before any other  LCD component macros are called.

Clear()
This macro clears the display of any previous output

Command(BYTE in)
Send the command signal value in to the LCD display.
Command can be used to configure the display and to send data and instructions directly to the LCD.
Please refer to the LCD datasheet for details of command signals and suitable values.

Cursor(x, y)
This macro positions the cursor at a given x and y coordinate. The PrintASCII and PrintNumber macros use the cursor position to decide where to print the character.

PrintNumber(BYTE number, or INT number)
Prints the number supplied. E.g. passing in the number 34 will cause "34" to be displayed on the LCD. 
Once the character has been printed, the cursor position is automatically advanced.
Also compatible with INTegers.

PrintASCII(BYTE Character)

Prints the ASCII characters at the current cursor position. Once the characters have been printed, the cursor position is automatically advanced.
Characters can be either an ASCII character code, a string of one or more characters surrounded by double quotes, or a standard ASCII character surrounded by single quotes.
Please note: Due to the way alphanumeric characters are handled in Flowcode only upper case characters can be used in single quotes.

PrintString(STRING String)
Prints the String at the current cursor position. Once the characters have been printed, the cursor position is automatically advanced.
The String is printed until a NULL string termination character is encountered (the end of the assigned characters in the string).

For Example:
Str1[20] is set to "Hello"
PrintString(Str1) will print "Hello", just 5 characters not 20 as the string is terminated after the Hello.

ScrollDisplay(BYTE Direction, BYTE Num_Positions)
Scrolls the display data left or right by a number of positions specified by the Num_Positions variable.
If the Direction byte is 0, 'l' or 'L' then the LCD data will scroll to the left.
If the Direction byte is 1, 'r' or 'R' then the LCD data will scroll to the right.

ClearLine(BYTE Line_Number)
Clears an entire line of the display and then returns the cursor back to the start of the line.

RAM_Write(BYTE nIdx, BYTE D0, BYTE D1, BYTE D2, BYTE D3, BYTE D4, BYTE D5, BYTE D6, BYTE D7)
Modifies the internal memory of the LCD device to allow for up to 8 customised characters to be created and stored into the device memory. 
Custom characters are referenced by using a nIdx parameter of 0 - 7 to specify the customised character you wish to edit.

The Dx bytes define the 8 data columns that are associated with the custom character.
To print out the custom charater on the display use the PrintASCII macro using the number as the Character parameter.
NOTE. This macro does not simulate.

The LCD display uses ASCII characters when simulating, however the actual display on a hardware LCD display depends on the character table for that particular display.
A list of LCD characters can be found on the introduction page.

Examples
PrintString( "Hello" ) prints out the letters Hello
PrintASCII("H") prints out the letter H
PrintASCII( 'H' ) prints out the letter H
PrintASCII( '=' ) prints out the character =
PrintASCII( 72 ) also prints out the letter H - 72 is the ASCII character code for H
PrintASCII( 104 ) prints out the letter h - 104 is the ASCII character code for h




********************************************************************/
/**** Example van LCD configuratie ****
Elke pin van de LCD is aan een afzonderlijke pin en poort toe te wijzen
Dit geeft iets meer werk tijdens de configuratie, maar is wel veel flexibeler
Dit is een voorbeeld waarbij alle pins van de LCD aan PORTB hangen

	#define LCD_918722_PORT0    PORTB
	#define LCD_918722_TRIS0    TRISB
	#define LCD_918722_PORT1    PORTB
	#define LCD_918722_TRIS1    TRISB
	#define LCD_918722_PORT2    PORTB
	#define LCD_918722_TRIS2     TRISB
	#define LCD_918722_PORT3    PORTB
	#define LCD_918722_TRIS3     TRISB
	#define LCD_918722_PORT4    PORTB
	#define LCD_918722_TRIS4     TRISB
	#define LCD_918722_PORT5	PORTB
	#define LCD_918722_TRIS5     TRISB
	#define LCD_918722_BIT0    	0
	#define LCD_918722_BIT1    	1
	#define LCD_918722_BIT2    	2
	#define LCD_918722_BIT3    	3
	#define LCD_918722_RS      	4
	#define LCD_918722_E       	5
	#define LCD_918722_ROWCNT	2
	#define LCD_918722_COLCNT	16
************************************************************/
#include <htc.h>
#define _XTAL_FREQ  19660800     // oscillator frequency for _delay()

//LINK LCD pins to MICROCONTROLLER PINS - CHANGE AS YOU WHISH!
	#define LCD_918722_PORT0    	PORTA
	#define LCD_918722_TRIS0    	TRISA
	#define LCD_918722_PORT1    	PORTA
	#define LCD_918722_TRIS1    	TRISA
	#define LCD_918722_PORT2    	PORTA
	#define LCD_918722_TRIS2     	TRISA
	#define LCD_918722_PORT3    	PORTA
	#define LCD_918722_TRIS3     	TRISA
	#define LCD_918722_PORT4    	PORTA
	#define LCD_918722_TRIS4     	TRISA
	#define LCD_918722_PORT5   		PORTA
	#define LCD_918722_TRIS5     	TRISA
	#define LCD_918722_BIT0    		0
	#define LCD_918722_BIT1    		1
	#define LCD_918722_BIT2    		2
	#define LCD_918722_BIT3    		3
	#define LCD_918722_RS      		5
	#define LCD_918722_E       		4
	#define LCD_918722_ROWCNT	4
	#define LCD_918722_COLCNT	20

// Define Clearbit and setbit instructions for HITECH C compiler

#define clear_bit( reg, bitNumb )		((reg) &= ~(1 << (bitNumb)))
#define set_bit( reg, bitNumb )		((reg) |= (1 << (bitNumb)))
#define test_bit( reg, bitNumb )		((reg) & (1 << (bitNumb)))

//LCDDisplay0: //Macro implementaties
void FCD_LCDDisplay0_RawSend(char in, char mask)
{
		unsigned char pt;

		clear_bit(LCD_918722_PORT0, LCD_918722_BIT0);
		clear_bit(LCD_918722_PORT1, LCD_918722_BIT1);
		clear_bit(LCD_918722_PORT2, LCD_918722_BIT2);
		clear_bit(LCD_918722_PORT3, LCD_918722_BIT3);
		clear_bit(LCD_918722_PORT4, LCD_918722_RS);
		clear_bit(LCD_918722_PORT5, LCD_918722_E);
		pt = ((in >> 4) & 0x0f);
		if (pt & 0x01)
		    set_bit(LCD_918722_PORT0, LCD_918722_BIT0);
		if (pt & 0x02)
		    set_bit(LCD_918722_PORT1, LCD_918722_BIT1);
		if (pt & 0x04)
		    set_bit(LCD_918722_PORT2, LCD_918722_BIT2);
		if (pt & 0x08)
		    set_bit(LCD_918722_PORT3, LCD_918722_BIT3);
		if (mask)
		    set_bit(LCD_918722_PORT4, LCD_918722_RS);
		__delay_us(120);
		set_bit (LCD_918722_PORT5, LCD_918722_E);
		__delay_us(120);
		clear_bit (LCD_918722_PORT5, LCD_918722_E);
		pt = (in & 0x0f);
		__delay_us(120);
		clear_bit(LCD_918722_PORT0, LCD_918722_BIT0);
		clear_bit(LCD_918722_PORT1, LCD_918722_BIT1);
		clear_bit(LCD_918722_PORT2, LCD_918722_BIT2);
		clear_bit(LCD_918722_PORT3, LCD_918722_BIT3);
		clear_bit(LCD_918722_PORT4, LCD_918722_RS);
		clear_bit(LCD_918722_PORT5, LCD_918722_E);
		if (pt & 0x01)
		    set_bit(LCD_918722_PORT0, LCD_918722_BIT0);
		if (pt & 0x02)
		    set_bit(LCD_918722_PORT1, LCD_918722_BIT1);
		if (pt & 0x04)
		    set_bit(LCD_918722_PORT2, LCD_918722_BIT2);
		if (pt & 0x08)
		    set_bit(LCD_918722_PORT3, LCD_918722_BIT3);
		if (mask)
		    set_bit(LCD_918722_PORT4, LCD_918722_RS);
		__delay_us(120);
		set_bit (LCD_918722_PORT5, LCD_918722_E);
		__delay_us(120);
		clear_bit (LCD_918722_PORT5, LCD_918722_E);
		__delay_us(120);
}

void LCD_Start()
{
	
		clear_bit(LCD_918722_TRIS0, LCD_918722_BIT0);
		clear_bit(LCD_918722_TRIS1, LCD_918722_BIT1);
		clear_bit(LCD_918722_TRIS2, LCD_918722_BIT2);
		clear_bit(LCD_918722_TRIS3, LCD_918722_BIT3);
		clear_bit(LCD_918722_TRIS4, LCD_918722_RS);
		clear_bit(LCD_918722_TRIS5, LCD_918722_E);

		__delay_ms(12);

		FCD_LCDDisplay0_RawSend(0x33, 0);
		__delay_ms(2);
		FCD_LCDDisplay0_RawSend(0x33, 0);
		__delay_ms(2);
		FCD_LCDDisplay0_RawSend(0x32, 0);
		__delay_ms(2);
		FCD_LCDDisplay0_RawSend(0x2c, 0);
		__delay_ms(2);
		FCD_LCDDisplay0_RawSend(0x06, 0);
		__delay_ms(2);
		FCD_LCDDisplay0_RawSend(0x0c, 0);
		__delay_ms(2);

		//clear the display
		FCD_LCDDisplay0_RawSend(0x01, 0);
		__delay_ms(2);
		FCD_LCDDisplay0_RawSend(0x02, 0);
		__delay_ms(2);

}

void LCD_Clear()
{
	
		FCD_LCDDisplay0_RawSend(0x01, 0);
		__delay_ms(2);
		FCD_LCDDisplay0_RawSend(0x02, 0);
		__delay_ms(2);

}

void LCD_PrintASCII(char Character)
{
	
		FCD_LCDDisplay0_RawSend(Character, 0x10);

}

void LCD_Command(char in)
{
	
		FCD_LCDDisplay0_RawSend(in, 0);
		__delay_ms(2);

}

void LCD_Cursor(char x, char y)
{
	
	  #if (LCD_918722_ROWCNT == 1)
	    y=0x80;
	  #endif

	  #if (LCD_918722_ROWCNT == 2)
		if (y==0)
			y=0x80;
		else
			y=0xc0;
	  #endif

	  #if (LCD_918722_ROWCNT == 4)
		if (y==0)
			y=0x80;
		else if (y==1)
			y=0xc0;

		#if (LCD_918722_COLCNT == 16)
			else if (y==2)
				y=0x90;
			else
				y=0xd0;
		#endif

		#if (LCD_918722_COLCNT == 20)
			else if (y==2)
				y=0x94;
			else
				y=0xd4;
		#endif
	  #endif

		FCD_LCDDisplay0_RawSend(y+x, 0);
		__delay_ms(2);

}

void LCD_PrintNumber(short Number)
{
	
		short tmp_int;
		char tmp_byte;
		if (Number < 0)
		{
			FCD_LCDDisplay0_RawSend('-', 0x10);
			Number = 0 - Number;
		}

		tmp_int = Number;
		if (Number >= 10000)
		{
			tmp_byte = tmp_int / 10000;
			FCD_LCDDisplay0_RawSend('0' + tmp_byte, 0x10);

			while (tmp_byte > 0)
			{
				tmp_int = tmp_int - 10000;
				tmp_byte--;
			}
		}
		if (Number >= 1000)
		{
			tmp_byte = tmp_int / 1000;
			FCD_LCDDisplay0_RawSend('0' + tmp_byte, 0x10);

			while (tmp_byte > 0)
			{
				tmp_int = tmp_int - 1000;
				tmp_byte--;
			}
		}
		if (Number >= 100)
		{
			tmp_byte = tmp_int / 100;
			FCD_LCDDisplay0_RawSend('0' + tmp_byte, 0x10);

			while (tmp_byte > 0)
			{
				tmp_int = tmp_int - 100;
				tmp_byte--;
			}
		}
		if (Number >= 10)
		{
			tmp_byte = tmp_int / 10;
			FCD_LCDDisplay0_RawSend('0' + tmp_byte, 0x10);

			while (tmp_byte > 0)
			{
				tmp_int = tmp_int - 10;
				tmp_byte--;
			}
		}
		FCD_LCDDisplay0_RawSend('0' + tmp_int, 0x10);

}

void LCD_PrintString(const unsigned char* String)
{
	
				while (*String != 0)
				{
				FCD_LCDDisplay0_RawSend(*String, 0x10);
				String++;
				}

}

void LCD_ScrollDisplay(char Direction, char Num_Positions)
{
	
		char cmd = 0;
		char count;

		//Choose the direction
		switch (Direction)
		{
			case 0:
			case 'l':
			case 'L':

				cmd = 0x18;
				break;

			case 1:
			case 'r':
			case 'R':

				cmd = 0x1C;
				break;

			default:
				break;
		}

		//If direction accepted then scroll the specified amount
		if (cmd)
		{
			for (count = 0; count < Num_Positions; count++)
				LCD_Command(cmd);
		}

}

void LCD_ClearLine(char Line)
{
	
		char count;
		char rowcount;

		//Define number of columns per line
		#if (LCD_918722_ROWCNT == 1)
			rowcount=80;
		#endif

		#if (LCD_918722_ROWCNT == 2)
			rowcount=40;
		#endif

		#if (LCD_918722_ROWCNT == 4)
			#if (LCD_918722_COLCNT == 16)
				rowcount=16;
			#endif
			#if (LCD_918722_COLCNT == 20)
				rowcount=20;
			#endif
		#endif

		//Start at beginning of the line
		LCD_Cursor (0, Line);

		//Send out spaces to clear line
		for (count = 0; count < rowcount; count++)
			FCD_LCDDisplay0_RawSend(' ', 0x10);

		//Move back to the beginning of the line.
		LCD_Cursor (0, Line);

}

void LCD_RAM_Write(char nIdx, char d0, char d1, char d2, char d3, char d4, char d5, char d6, char d7)
{
	   //set CGRAM address
	   FCD_LCDDisplay0_RawSend(64 + (nIdx << 3), 0);
	   __delay_ms(2);

	   //write CGRAM data
	   FCD_LCDDisplay0_RawSend(d0, 0x10);
	   FCD_LCDDisplay0_RawSend(d1, 0x10);
	   FCD_LCDDisplay0_RawSend(d2, 0x10);
	   FCD_LCDDisplay0_RawSend(d3, 0x10);
	   FCD_LCDDisplay0_RawSend(d4, 0x10);
	   FCD_LCDDisplay0_RawSend(d5, 0x10);
	   FCD_LCDDisplay0_RawSend(d6, 0x10);
	   FCD_LCDDisplay0_RawSend(d7, 0x10);

	   //Clear the display
	   FCD_LCDDisplay0_RawSend(0x01, 0);
	   __delay_ms(2);
	   FCD_LCDDisplay0_RawSend(0x02, 0);
	   __delay_ms(2);
}