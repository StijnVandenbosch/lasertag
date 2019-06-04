/* 
 * File:   MP3_Player.c
 * Author: Stijn v
 *
 * Created on 19 oktober 2018, 19:06
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <htc.h>
#include "HITECH_LCD.h"
#define _XTAL_FREQ 19660800

__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF & CPD_OFF & BOREN_ON & IESO_ON & FCMEN_ON & LVP_OFF);
__CONFIG(BOR4V_BOR21V & WRT_OFF);

#define START_BYTE 0x7E
#define VERSION 0xFF
#define DATA_LEN 0x06
#define END_BYTE 0xEF

void sendCMD(unsigned char CMD, int VAL, char FEEDBACK);
void setVOL(unsigned char VOL);
void playNUM(unsigned char NUM);
void playSTOP(void);
void waitForStop(void);
void startUP(void);
void teamSelect(void);
void startGame(void);
void fire(void);
void reload(void);
void error(void);

unsigned char ADs = 0;
unsigned char Ammo = 0;

void main(void){
    /* UART settings */
    SPBRGH = 511 >> 8;      // Baudrate high register -> 8 MSB of SPBRG -> 0x00
    SPBRG = 511 & 0xFF;     // Baudrate low register  -> 8 LSB of SPBRG -> 0x7F
    TRISC6 = 0;             // RC6 is TX -> output
    TRISC7 = 1;             // RC7 is RX -> input
    BAUDCTL = 0x08;         // Non inverted TX data, 16bit baud gen
    RCSTA = 0x90;           // Enable serial, enable receiver
    TXSTA = 0x24;           // Send 8bits, enable transmitter, high speed, asynchronous mode
    /* PORT settings */
    TRISD = 0x00;           // PORTD output
    PORTD = 0x00;           // PORTD low
    TRISA = 0x00;           // PORTA output
    PORTA = 0x00;           // PORTA low
    TRISB = 0x0F;           // RB0-RB3 input
    PORTB = 0x00;           // PORTB low
    TRISC0 = 0;
    /* LCD init */
    ANSEL=0x00;             // No analog function
    ANSELH = 0x00;          // No analog function
    LCD_Start();            // Start the LCD
    LCD_Clear();            // Clear the LCD
    __delay_ms(5000);
    startUP();              // Welkom screen and sound
    //PORTD = 0xFF;           // All LED's on
    teamSelect();           // Team select screen
    startGame();            // Start game screen

    LCD_Clear();            // Clear the LCD
    LCD_Cursor(0,0);
    LCD_PrintString("S5 to FIRE");//Write on lcd
    LCD_ClearLine(1);       // Clear line 1
    Ammo = 20;              // Reset ammo
    LCD_Cursor(0,1);
    LCD_PrintString("Ammo: ");//Print ammo
    LCD_PrintNumber(Ammo);

    while(1){
        if(RB1){            // when RB1 is high
            if(Ammo != 0){  // Is ammo available
                fire();     // Flash leds and play sound
                Ammo--;     // Decrese ammo
                LCD_ClearLine(1);//Clear line 1
                LCD_Cursor(0,1);
                LCD_PrintString("Ammo: ");//print ammo
                LCD_PrintNumber(Ammo);
                __delay_ms(120);//debounce
            }
            else{           //Ammo is empty
                error();    //Error sound
            }
        }
        else if(RB0){       // when RB0 is high
            Ammo = 20;      // Reset ammo
            reload();       // PLay reload sound
            LCD_ClearLine(1);  //Clear line 1
            LCD_Cursor(0,1);
            LCD_PrintString("Ammo: ");//print ammp
            LCD_PrintNumber(Ammo);
        }
     }
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

void startUP(void){
    LCD_Clear();            //Clear the LCD
    LCD_Cursor(0,0);
    LCD_PrintString("Laser TAG");//Print on LCD
    setVOL(20); //set volume to 20, max 30
    playNUM(1); //play number 1
    waitForStop(); //wait for module to send feedback
    
}

void waitForStop(void){
    while(!RCIF)continue; //Wait for data
    unsigned char dataIn[10];
    dataIn[3]=0;
    while(dataIn[3] != 0x3D){
        for(char i = 0; i<10; i++){
        while(!RCIF)continue;//wait for data
        dataIn[i]=RCREG;  //Read data
        }
    }
}

void teamSelect(void){
    LCD_Clear();    //Clear LCD
    LCD_Cursor(0,0);
    LCD_PrintString("Kies team");//Print on LCD
    LCD_Cursor(0,1);
    LCD_PrintString("<= team1    team2 =>");
    while(!(RB1 || RB2))continue;//While not selected team
    if(RB1){    //if RB1 was high
        setVOL(27);//Set volume 27
        playNUM(2);//Play number 2
    }
    else{       //else RB2
        setVOL(27);//Set volume 27
        playNUM(3);//Play number 2
    }
    waitForStop();//Wait for stop
}

void startGame(void){
    LCD_Clear();//Clear LCD
    LCD_Cursor(0,0);
    LCD_PrintString("ready");//Print on LCD
    LCD_Cursor(0,1);
    LCD_PrintString("S1 to start game");
    while(!RB0)continue;//While RB0 is not high
    setVOL(27);//Set volume 27
    playNUM(4);//Play number 4
    waitForStop();//Wait for stop
}

void fire(void){
    setVOL(27); //Set volume 27
    playNUM(5); //Play number 5
    PORTD = 0x00;//All leds off
    __delay_ms(5);
    PORTD = 0xFF;//All leds on
    __delay_ms(5);
    waitForStop();//Wait for stop
    
}

void reload(void){
    setVOL(27); //Set volume 27
    playNUM(6); //PLay number 6
    waitForStop();//Wait for stop
}

void error(void){
    setVOL(27); //Set volume 27
    playNUM(7); //Play number 7
    waitForStop();//Wait for stop
}