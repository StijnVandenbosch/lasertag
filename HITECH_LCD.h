//LCDDisplay0: //Macro functie declaraties

void FCD_LCDDisplay0_RawSend(char in, char mask);
void LCD_Start();
void LCD_Clear();
void LCD_PrintASCII(char Character);
void LCD_Command(char in);
void LCD_Cursor(char x, char y);
void LCD_PrintNumber(short Number);
void LCD_PrintString(const unsigned char* String);
void LCD_ScrollDisplay(char Direction, char Num_Positions);
void LCD_ClearLine(char Line);
void LCD_RAM_Write(char nIdx, char d0, char d1, char d2, char d3, char d4, char d5, char d6, char d7);