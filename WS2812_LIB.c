#include <htc.h>
#include "WS2812_LIB.h"
#define _XTAL_FREQ 19660800

unsigned char ledByte[NUMLEDS*3];		//Single array, 3 bytes per LED
unsigned char index = 0;

void initLED(void){
    TRISC0 = 0;
    RC0 = 0;
}

void writeLED(void) {
    GIE = 0;                     //Time sensitive code
    unsigned char Temp = 0;
    for(unsigned int i = 0; i<NUMLEDS*3; i++){ 	//Loop all the bytes
        Temp = 0x00;             //clear temp
        Temp = ledByte[i];	 //Place current byte to temporary
        for(unsigned char j = 0; j<8; j++){	//Loop all the bits
         if(Temp&0x80)		 //if msb is 1
        {
            asm("BSF 07h, 0");   //Set RC0 (1cycle)
            asm("NOP");          //Wait 1cycle
            asm("NOP");          //Wait 1cycle
            asm("NOP");          //Wait 1cycle
            asm("BCF 07h, 0");   //Reset RC0 (1cycle)
        }
        else
        {
            asm("BSF 07h, 0");  //Set RC0 (1cycle)
            asm("NOP");         //Wait 1cycle
            asm("BCF 07h, 0");  //Reset RC0 (1cycle)
        }
        Temp = Temp << 1;       //Shift left
        }
    }
    __delay_us(50);
    GIE = 1;
}

void setLED(unsigned char led, unsigned char R, unsigned char G, unsigned char B) {
	//1 gezamelijke array met 3 indexen per led
	//led0 wordt dan ledByte[0]=G,ledByte[1]=R,ledByte[2]=B
	//led1 1*3 = 3, ledByte[3]=G,.....
    if(led<NUMLEDS){
	index = led * 3;
	ledByte[index] = G;
	ledByte[index+1] = R;
	ledByte[index+2] = B;
    }
}

void AllOff(void){
    /* set all leds to white */
    for(unsigned int a = 0; a<NUMLEDS;a++){
    setLED(a,0,0,0);
    }
    writeLED(); //write data to LED's
}