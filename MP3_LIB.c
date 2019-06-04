#include "MP3_LIB.h"
#include "ESP01_LIB.h"

void startMP3(void){
	/* UART settings */
    SPBRGH = 511 >> 8;      // Baudrate high register -> 8 MSB of SPBRG -> 0x00
    SPBRG = 511 & 0xFF;     // Baudrate low register  -> 8 LSB of SPBRG -> 0x7F
    TRISC6 = 0;             // RC6 is TX -> output
    TRISC7 = 1;             // RC7 is RX -> input
    BAUDCTL = 0x08;         // Non inverted TX data, 16bit baud gen
    RCSTA = 0x90;           // Enable serial, enable receiver
    TXSTA = 0x24;           // Send 8bits, enable transmitter, high speed, asynchronous mode
    __delay_ms(1000);       // Wait for module to start
}

void sendCMD(unsigned char CMD, int VAL, char FEEDBACK){
    //Checksum formula: -(VERSION+LEN+CMD+FEEDBACK+VALH+VALL)
    int checksum = -(VERSION + DATA_LEN + CMD + FEEDBACK + (VAL >>8) + (VAL & 0xFF));
    //10 bytes to send, found in datasheet
    unsigned char data[10]={START_BYTE,VERSION,DATA_LEN,0,0,0,0,0,0,END_BYTE};
    data[3]=CMD;
    data[4]=FEEDBACK;
    data[5]=VAL >> 8;
    data[6]=VAL & 0xFF;
    data[7]=checksum >> 8;
    data[8]=checksum & 0xFF;
    //send all 10 bytes
    for(char i = 0; i<10; i++){
        while(!TXIF)continue;//Wait for previous data to be send
        TXREG = data[i];     //Write byte to UART
    }
}

void setVOL(unsigned char VOL){
    sendCMD(0x06,VOL,0);    //set volume, no feedback
}

void playNUM(unsigned char NUM){
    sendCMD(0x03,NUM,0);    //Play requested number
}

void playSTOP(void){
    sendCMD(0x16,0,0);      //Stop playing
    __delay_ms(200);
}

void waitForStop(int timeout){
    //busy is active low
    /* wait for module to start en then stop */
    unsigned long temp = millis;
    while(nBUSY && millis < (temp + timeout) )continue;
    __delay_ms(1);
    while(!nBUSY && millis < (temp + timeout) )continue;
    __delay_ms(250);
}
