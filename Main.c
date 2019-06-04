/* 
 * File:   Main.c
 * Author: Stijn Vandenbosch
 *
 * Created on 22 november 2018, 15:07
 */

#include <stdio.h>
#include <stdlib.h>
#include <htc.h>
#include <string.h>
#include "HITECH_LCD.h"
#include "IR_LIB.h"
#include "WS2812_LIB.h"
#include "prototypes.h"
#define _XTAL_FREQ 19660800

__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_ON & MCLRE_ON & CP_OFF & CPD_OFF & BOREN_ON & IESO_ON & FCMEN_ON & LVP_OFF);
__CONFIG(BOR4V_BOR40V & WRT_OFF);

#define STA 1
#define AP 2

/*
 * Smartled op RC0
 * IR ontvanger op RB0
 * IR zender op RC1
 * trigger/confirm op RB1
 * links op RB2
 * rechts op RB3
 */

unsigned char myId = 0;
unsigned char myTeam = 0;
unsigned char myWeapon = 0;
unsigned char myAmmo = 0;
unsigned char myHealth = 0;
unsigned char gameMode = 0;
bit semi;
bit mode;

void main (void){
    ANSEL = 0x00;   //no analog function
    ANSELH = 0x00;  //no analog function
    TRISA = 0x00;   //PORTA als input
    TRISB |= 0b00001110; //RB1, RB2, RB3 input
    TRISB &=~0b00100000; //RB5 output
    configUART();   //configure UART @ 115200bps
    initLED();
    setLED(0,100,100,100);
    writeLED();
    LCD_Start();    //start LCD
    LCD_Clear();    //clear LCD
    setupSelect();
    IR_INIT();      //initialise IR receive and send
    initLED();      //initialise smartled
    if(mode = 1){
       startup();      //startup message and connect to wifi
    }
    else{
    selectTeam();   //set team in output IR data
    selectPlayer(); //set player in output IR data
    }
    setFireMode();  //set fire mode for rifle
    countDown();    //10s countdown
    unsigned int oldData = 1;   //var om constante LCD update te voorkomen
    unsigned char ammo = myAmmo;    //standaard 20 kogels
    unsigned char life = myHealth;   //standaard 100 levenspunten
    unsigned char inTeam = 0;
    unsigned char inPlayer = 0;
    unsigned char inWeapon = 0;
    LCD_ClearLine(0);
    LCD_Cursor(0,0);
    LCD_PrintNumber(ammo);
    while(1){
        if(RB1){    //schieten
            if(!semi && ammo > 0){  //single shot
                IR_Send();
                beep();
                ammo--;
                LCD_ClearLine(0);
                LCD_Cursor(0,0);
                LCD_PrintNumber(ammo);
            }
            else if(ammo >= 3){
                for(char c=0; c<3;c++){ //semi shot
                    IR_Send();
                    __delay_ms(2);
                    beep();
                    ammo--;
                    LCD_ClearLine(0);
                    LCD_Cursor(0,0);
                    LCD_PrintNumber(ammo);
                }
            }
            else{
               for(char c=0; c<ammo;c++){ //semi shot
                    IR_Send();
                    __delay_ms(2);
                    beep();
                    ammo--;
                    LCD_ClearLine(0);
                    LCD_Cursor(0,0);
                    LCD_PrintNumber(ammo);
                }
            }
            __delay_ms(50);
        }
        else if(!life){   //als levens = 0
            LCD_Clear();
            LCD_Cursor(0,0);
            LCD_PrintString("Je bent dood");
        }
        if(RB2){    //herladen
            ammo = myAmmo;
            LCD_ClearLine(0);
            LCD_Cursor(0,0);
            LCD_PrintNumber(ammo);
            beeps();
        }
        if(inData != oldData){
           LCD_ClearLine(1);
           LCD_Cursor(0,1);
           LCD_PrintString("D:");
           LCD_PrintNumber((inData>>16)&0x01);
           LCD_PrintString(" P:");
           inPlayer = (inData>>11)&0x0F;
           LCD_PrintNumber(inPlayer);
           LCD_ClearLine(2);
           LCD_Cursor(0,2);
           LCD_PrintString("T:");
           inTeam = (inData>>8)&0x07;
           LCD_PrintNumber(inTeam);
           LCD_PrintString(" D:");
           inWeapon = inData&0xFF;
           LCD_PrintNumber(inWeapon);
           oldData=inData;
        }
        /*if(full){
          inPlayer = (inData>>11)&0x0F;
          inTeam = (inData>>8)&0x07;
          inWeapon = inData&0xFF;
          if(inTeam != myTeam){
               life -= inWeapon;
           }
        }*/
    }
}

void interrupt isr(void){
    IR_Handler();
}

void configUART(void){
    /*  UART configs  */
    // Baud rate 38400 w/ xtal 19660800: ((19660800/115200)/4)-1 = 42
    SPBRGH = 42 >> 8;   // Baudrate high register -> 8 MSB of SPBRG -> 0x00
    SPBRG = 42 & 0xFF;  // Baudrate low register  -> 8 LSB of SPBRG -> 0x2A
    TRISC6 = 0;         // RC6 is TX -> output
    TRISC7 = 1;         // RC7 is RX -> input
    BAUDCTL = 0x08;     // Non inverted TX data, 16bit baud gen
    RCSTA = 0x90;       // Enable serial, enable receiver
    TXSTA = 0x24;       // Send 8bits, enable transmitter, high speed, asynchronous mode
}

void beep(void){
    for(char i=0; i<50; i++){
        RB5 = 1;
        __delay_us(700);
        RB5 = 0;
        __delay_us(700);
    }
}

void beeps(void){
    for(char i=0; i<250; i++){
        RB5 = 1;
        __delay_ms(1);
        RB5 = 0;
        __delay_ms(1);
    }
}

char getByte(void) //function that waits and returns received char
{
    while(!RCIF);  // wait for data
    return RCREG;  //return received byte
}

void waitFor(char* string) {
    unsigned char progress = 0; // count of correct received characters
    char temp;  // char to store received
    do {
        temp = getByte(); // get character
        if (temp == string[progress]) { //check if character is correct
            progress++;   //update progress
        }
        else { //if char is wrong
            progress = 0; //reset progress
        }
    } while (string[progress] != '\0');
}

void writeString(const char* pointer){// Function w/ pointer to string
    for(unsigned char i = 0; pointer[i] != '\0';i++){// Loop while not end of string -> \0
        while(!TXIF)continue;   // Wait for previous byte to be sent
        TXREG = pointer[i];     // Send current character byte
    }
}

void startup(void){
    beep();
    /*WDT telt tot 65536*/
    WDTPS3 = 1;
    WDTPS2 = 0;
    WDTPS1 = 1;
    WDTPS0 = 1;
    PSA = 1;    //prescaler voor WDT: /8
    PS2 = 1;
    PS1 = 0;
    PS0 = 0;
    SWDTEN = 1;         //start WDT
    LCD_Cursor(0,0);
    LCD_PrintString("GIPLaser Tag");
    LCD_Cursor(0,1);
    LCD_PrintString("Stijn Vandenbosch");
    __delay_ms(2000);
    LCD_Clear();
    LCD_Cursor(0,0);
    LCD_PrintString("Please wait!");
    LCD_Cursor(0,1);
    LCD_PrintString("starting WiFi...");
    __delay_ms(1500);   //wait 1.5s, ESP8266-01 startup
    //writeString("AT+CWDHCP=1,0\r\n"); //enable dhcp
    //waitFor("OK\r\n");
    writeString("AT+CWMODE=1\r\n"); //Station mode
    waitFor("OK\r\n");       
    writeString("AT+CWQAP\r\n");//connect to AP
    waitFor("OK\r\n");              //wait for response OK//wait for response OK
    asm("clrwdt");
    writeString("AT+CWJAP=\"telenet-99FEDF4\",\"yM8ak4khxywZ\"\r\n");//connect to AP
    waitFor("OK\r\n");              //wait for response OK
    asm("clrwdt");
    getData();
    SWDTEN = 0;
    //writeString("AT+CIPSTART=\"TCP\",\"192.168.4.1\",8102\r\n");
    //waitFor("OK\r\n");
    LCD_ClearLine(1);
    LCD_Cursor(0,1);
    LCD_PrintString("succes...");
    __delay_ms(1000);   //wait 2s
    LCD_Clear();
    LCD_Cursor(0,0);
    LCD_PrintString("ID:");
    LCD_PrintNumber(myId);
    LCD_PrintString(" Team:");
    LCD_PrintNumber(myTeam);
    LCD_Cursor(0,1);
    LCD_PrintString("Ammo:");
    LCD_PrintNumber(myAmmo);
    LCD_PrintString(" Health:");
    LCD_PrintNumber(myHealth);
    LCD_Cursor(0,2);
    LCD_PrintString("GameMode:");
    LCD_PrintNumber(gameMode);
    __delay_ms(5000);
    LCD_Clear();        //clear LCD
}

void selectTeam(void){
    LCD_Cursor(0,0);
    LCD_PrintString("Select team");
    LCD_Cursor(0,1);
    LCD_PrintString("<= RED        BLUE=>");
    while(!RB2 && !RB3);
    beep();
    if(RB2){
        myTeam = 1; //set team
        outData |= 0b0000000100000000; //set team
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: RED");
        setLED(0,255,0,0);
    }
    else{
        myTeam = 2; //qet team
        outData |= 0b0000001000000000; //set team
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: BLUE");
        setLED(0,0,0,255);
    }
    writeLED();
    __delay_ms(250);    //no doubble press
    LCD_Clear();
}

void selectPlayer(void){
    LCD_Cursor(0,0);
    LCD_PrintString("Select team");
    LCD_Cursor(0,1);
    LCD_PrintString("<=ALPHA       BETA=>");
    while(!RB2 && !RB3);
    beep();
    if(RB2){
        myId = 1;
        outData |= 0b0000100000000000; //set player
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: ALPHA");
    }
    else{
        myId = 2;
        outData |= 0b0001000000000000; //set player
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: BETA");
    }
    __delay_ms(250);    //no doubble press
}

void setFireMode(void){
    LCD_Cursor(0,0);
    LCD_PrintString("Select team");
    LCD_Cursor(0,1);
    LCD_PrintString("<=Single     Semi=>");
    while(!RB2 && !RB3);
    beep();
    if(RB2){
        semi = 0;
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: Single");
    }
    else{
        semi = 1;
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: Semi");
    }
    __delay_ms(250);    //no doubble press
}

void countDown(void){
    LCD_Clear();
    LCD_Cursor(0,0);
    LCD_PrintString("Countdown");
    for(char i=0; i<10; i++){
        beeps();
        __delay_ms(500);
    }
    LCD_Clear();
}

char* esp_getIP(char type){
    char temp;      //store received char
    char outS[16];  //full size of IP in string format
    char count = 0; //keeps track of received correct characters
    writeString("AT+CIFSR\r\n"); //request all IP's
    if(type == STA){    //if want station IP
      waitFor("STAIP,");//look for STAIP,
    }
    else if(type == AP){//else want AP IP
      waitFor("APIP,"); //look for APIP,
    }
    temp = getByte();  //get the incomming byte
    while(temp != '\r'){   //while no carriage return
        if(temp<58 && temp>47){ //ascii value's for numbers
          outS[count]= temp; //store character
          count++;  //update count
        }
        else if(temp == '.'){//if received was a point
          outS[count]=temp;  //store characters
          count++;  //update count
        }
        temp = getByte();  //get the incomming byte
    }
    outS[count]='\0';   //last character end needs to be NULL for string
    waitFor("OK\r\n");  //wait for OK
    return outS;        //return pointer to received string
}

void getData(void){
    writeString("AT+CIPSTART=\"TCP\",\"192.168.0.162\",80\r\n");
    waitFor("OK\r\n");
    writeString("AT+CIPSEND=54\r\n");
    waitFor("OK\r\n");
    waitFor("> ");
    writeString("GET /esp/getData.php HTTP/1.0\r\nHost: 192.168.0.162\r\n\r\n\r\n");
    waitFor("+IPD");
    waitFor("\r\n\r\n");
    myId = getByte() - '0';
    getByte();  //,
    myTeam = getByte() - '0';
    getByte();  //,
    myAmmo = getByte() - '0';   //digit 1;
    char temp = getByte();
    if(temp != ','){
        myAmmo = (10*myAmmo)+(temp -'0');
        temp = getByte();
        if(temp != ','){
            myAmmo = (10*myAmmo)+(temp-'0');
        }
    }
    myHealth = getByte-'0';
    temp = getByte();
    if(temp != ','){
        myHealth = (10*myHealth)+(temp-'0');
        temp = getByte();
        if(temp != ','){
            myHealth = (10*myHealth)+(temp-'0');
        }
    }
    gameMode = getByte() - '0';
}

void setupSelect(void){
    LCD_Cursor(0,0);
    LCD_PrintString("Select setup");
    LCD_Cursor(0,1);
    LCD_PrintString("<=WiFi      Manual=>");
    while(!RB2 && !RB3);
    beep();
    if(RB2){
        mode = 1;
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: wifi");
    }
    else{
        mode = 0;
        LCD_Clear();
        LCD_Cursor(0,0);
        LCD_PrintString("Selected: manual");
    }
    __delay_ms(250);    //no doubble press
}

//void sendData(char* Id, int data){
//        char buf[10];            //buffer for data
//        char buf2[4];            //buffer for stringlen
//        sprintf(buf,"%d",data);  //create string
//        sprintf(buf2,"%d",strlen(buf));//create string
//        writeString("AT+CIPSEND="); //start send
//        writeChar(Id);        //connection to send to
//        writeString(",");
//        writeString(buf2);       //size of string
//        writeString("\r\n");     //confirm
//        waitFor("> ");           //ready to receive
//        writeString(buf);        //send string
//        waitFor("OK\r\n");       //wait for response
//}
//
//void sendString(char* Id, char* string){
//        char buf[70];            //buffer for string
//        char buf2[4];            //buffer for stringlen
//        sprintf(buf,"%s",string);//create string
//        sprintf(buf2,"%d",strlen(buf));//create string
//        writeString("AT+CIPSEND="); //start send
//        writeChar(Id);        //connection to send to
//        writeString(",");
//        writeString(buf2);       //size of string
//        writeString("\r\n");     //confirm
//        waitFor("> ");           //ready to receive
//        writeString(buf);        //send string
//        waitFor("OK\r\n");       //wait for response
//}

void writeChar(char* e){ //stuur een char
    while(!TXIF)continue;
    TXREG = e;
}

//#define HIT 0
//#define DEAD 1
//
//void sendToBase(char type){
//    char out = 5;
//    switch(type){
//        case HIT:
//            out = 1;
//            break;
//        case DEAD:
//            out = 2;
//            break;
//    }
//    SWDTEN = 1; //start watchdog
//    writeString("AT+CIPSTART=\"TCP\",\"192.168.159.11\",8102\r\n"); //connect to homebase
//    waitFor("OK\r\n"); //wacht op response
//    writeString("AT+CIPSEND=1\r\n");
//    waitFor("> ");
//    writeChar(out); //send hit or dead
//    writeString("\r\n");
//    waitFor("OK\r\n");
//    SWDTEN = 0; //stop watchdog
//}

