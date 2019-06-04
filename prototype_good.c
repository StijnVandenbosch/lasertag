/* 
 * File:   Main.c
 * Author: Stijn Vandenbosch
 *
 * Created on 22 november 2018, 15:07
 * Last edit on 22 february 2019 17:09
 */

#include <stdio.h>
#include <stdlib.h>
#include <htc.h>
#include <string.h>
#include "IR_LIB.h"
#include "WS2812_LIB.h"
#include "prototypes.h"
#include "MP3_LIB.h"
#include "ESP01_LIB.h"
#include "globals.h"
#define _XTAL_FREQ 19660800

__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_ON & MCLRE_ON & CP_OFF & CPD_OFF & BOREN_ON & IESO_OFF & FCMEN_ON & LVP_OFF);
__CONFIG(BOR4V_BOR40V & WRT_OFF);

#define serverIP "192.168.0.70"
#define requestLEN "60" //aantal karakters in request string
#define request "GET /SITEV2/esp/getData.php HTTP/1.0\r\nHost: 192.168.0.70\r\n\r\n"

/*
 * Smartled op RC0
 * IR ontvanger op RB0
 * IR zender op RC1
 * trigger/confirm op RB1
 * reload op RB2
 * mode op RB3
 */

#define mode (!RB3)     //normally closed mode switch
#define reload (!RB2)   //normally closed reload switch
#define trigger RB1     //normally open trigger switch

#define hit RD0         //hit LED on RD0

#define minutMulti 60000 //60000millisecondes zijn 1 minuut

bit setup = 0;          //setup type
bit time = 0;           //has time passed 

void main (void){
    ANSEL = 0x00;       //no analog function
    ANSELH = 0x00;      //no analog function
    TRISA = 0x00;       //PORTA als output
    PORTA = 0x00;       //PORTA laag
    TRISB |= 0b00001110;//RB1, RB2, RB3 input
    TRISB &=~0b00100000;//RB5 output
    TRISD = 0x10;       //PORTD output (RD4 input)
    PORTD = 0x00;       //PORTD laag
    IRPIN = 0;          //IR led uit
    beep();             //korte beep
    IR_INIT();          //initialise IR receive and send
    initLED();          //initialise smartled
    T2CON = 0x3D;       //pre /4 | post /8 | timer on
    PR2 = 154;          //generate interrupt when TMR2 = 192
    __delay_ms(1000);   //wacht 1s - audio module moet starten
    configUART(1);      //configure UART @ 9600bps
    setVOL(20);         //set volume to 20
    playNUM(1);         //welkom message
    waitForStop(10000); //wait for playing to stop
    setVOL(20);         //set volume to 20
    playNUM(2);         //play setup select
    waitForStop(10000); //wait for playing to stop
    setupSelect();      //select setup mode
    if(!setup){         //manueel
        outData = 0x0000;
        selectTeam();   //set team in output IR data
        /* coundown */
        setVOL(20);         //set volume to 20
        playNUM(11);        //play trigger to start
        waitForStop(10000); //wait for audio to stop
        configUART(0);      //baud 115200bps for ESP01
        while(!trigger);    //wait for trigger press
        /* wait 10s with beeps */
        for(char i=0; i<10; i++){
            beeps();    //beep 0.5s
            __delay_ms(500);
        }
    }
    else{               //automatisch
        TMR2IE = 1;   //enable TMR2 interrupt PEIE is enabled in IR_INIT
        beep();         //start with beep
        /*
         * WDT werkt op 31kHz interne klok
         * Met prescaler op 65536 -> 2.11s timeout
         * Als die tijd bereikt wordt, reset deze de microcontroler
         */
        /* WDT telt van 0 tot 65535 */
        WDTPS3 = 1;
        WDTPS2 = 0;
        WDTPS1 = 1;
        WDTPS0 = 1;

        configUART(0);                  //UART @ 115200bps
        SWDTEN = 0;                     //start WDT
        __delay_ms(1500);               //wait 1.5s, ESP8266-01 startup
        writeString("AT+CWMODE=1\r\n"); //Station mode
        waitFor("OK\r\n",5000);         //wait for OK
        asm("clrwdt");                  //clear wdt
        //writeString("AT+CWJAP_CUR=\"telenet-99FEDF4\",\"yM8ak4khxywZ\"\r\n");//connect to AP
        writeString("AT+CWJAP_CUR=\"LaboEA-ICT\",\"SJS2900Schoten$153\"\r\n");//connect to AP
        waitFor("OK\r\n",10000);        //wait for response OK
        asm("clrwdt");                  //clear wdt
        writeString("AT+CIPSTART=\"TCP\",\"192.168.0.70\",80\r\n");//connect to server
        waitFor("OK\r\n",5000);         //wait for response OK
        asm("clrwdt");                  //clear wdt
        writeString("AT+CIPSEND=61\r\n");//setup send to server
        waitFor("> ",5000);             //wait for response
        asm("clrwdt");                  //clear wdt
        writeString(request);           //send request
        writeString("\r\n");            //einde van request
        waitFor("+IPD",5000);           //wait for +IPD
        waitFor("\r\n\r\n",5000);       //wait for response
        asm("clrwdt");                  //clear wdt
        /* data from server format: ID,TEAM,AMMO,HEALTH,GAMEMODE */
        char temp = '\0';   //temp char is null
        waitFor("  ",2000);
        temp = getByte();   //get byte(char) | cijfer 1
        myId = temp-'0';    //ascii naar decimaal
        temp = getByte();   //get byte(char) | comma
        temp = getByte();   //get byte(char) | cijfer 1
        myTeam = temp-'0';  //ascii naar decimaal
        temp = getByte();   //get byte(char) | comma
        temp = getByte();   //get byte(char) | cijfer 1
        myAmmo = temp-'0';  //ascii naar decimaal
        temp = getByte();   //get byte(char) | cijfer 2/comma
        if(temp != ','){    //als karakter geen comma is -> nog een digit
            myAmmo = (10*myAmmo)+(temp-'0'); //ascii naar decimaal
            temp = getByte();   //get byte(char) | cijfer 3/comma
            if(temp != ','){    //als karakter geen comma is -> nog een digit
                myAmmo = (10*myAmmo)+(temp-'0'); //ascii naar decimaal
            }
        }
        temp = getByte();   //get byte(char) | cijfer 1
        myLife = temp - '0';//ascii naar decimaal
        temp = getByte();   //get byte(char) | cijfer 2/comma
        if(temp != ','){    //als karakter geen comma is -> nog een digit
            myLife = (10*myLife)+(temp-'0'); //ascii naar decimaal
            temp = getByte();   //get byte(char) | cijfer 3/comma
            if(temp != ','){    //als karakter geen comma is -> nog een digit
                myLife = (10*myLife)+(temp-'0'); //ascii naar decimaal
            }
        }
        temp = getByte();   //get byte(char) | cijfer 1
        gameMode = temp-'0';//ascii naar decimaal
        temp = getByte();   //get byte(char) | comma
        temp = getByte();   //get byte(char) | cijfer 1
        myTime = temp-'0';  //ascii naar decimaal
        temp = getByte();   //get byte(char) | cijfer 2/comma
        if(temp != ','){    //als karakter geen comma is -> nog een digit
            myTime = (10*myTime)+(temp-'0'); //ascii naar decimaal
        }
        outData = 0x0000;
        outData |= ((myId&0x07) << 11);    //zet id in ir code
        outData |= ((myTeam&0x0F) << 8);   //zet team in ir code
        /* give leds team color
         * num leds = player
         */
        switch(myTeam){     //check team
            case 1:         //rood
                for(unsigned char i = 0; i<myId; i++){
                    setLED(i,20,0,0);  //red
                }
                break;
            case 2:         //blauw
                for(unsigned char i = 0; i<myId; i++){
                    setLED(i,0,0,20);  //blue
                }
                break;
            default:        //fout
                setLED(0,20,20,20); //licht wit
                break;
        }
        writeLED();         //stuur data naar leds
        SWDTEN = 0;         //disable wdt
        configUART(1);      //baud 9600bps for dfplayer
        setVOL(20);         //set volume to 20
        playNUM(5);         //play settings received
        waitForStop(10000); //wait for audio to end
        setVOL(20);         //set volume to 20
        playNUM(19);        //play wait
        waitForStop(5000); //wait for play to stop
        configUART(0);
        while(!mode)continue;   //wacht 
    }
    TMR2IE = 0;
    unsigned int copy = ((outData>>8)&0x00FF);
    outData |= copy;
    unsigned char ammo = 0;//local ammo 
    while(1){
        time = 0;       //gen stop van programma
        TMR1ON = 1;     //start TMR1
        INTE = 1;       //external interrupt on
        ammo = myAmmo;  //zet ammo op instelling
        life = myLife;  //zet levens op intstelling
        timeOut = (myTime * minutMulti)+ millis ;   //bereken timeOut van spel
        updateLED(ammo,life);           //zet ammo en life op leds
        TMR2IE = 1;                     //enable timer 2 interrupt
        while(!time){                   //loop zolang tijd niet voorbij is
            if(trigger && life && ammo){//schieten kan als je niet dood bent
                configUART(1);          //baud 9600bps for dfplayer
                setVOL(20);             //set volume to 20
                playNUM(21);            //play fire
                IR_Send();              //ir schot
                waitForStop(2000);      //wait for play to stop
                configUART(1);          //baud 1152000bps for esp
                ammo--;                 //decrement ammo
                updateLED(ammo,life);   //update ammo en life
            }
            else if(trigger && (!ammo || !life)){  //proberen schieten zonder kogels of als dood
                configUART(1);          //9600bps
                setVOL(20);             //set volume to 20
                playNUM(15);            //play no ammo error
                waitForStop(10000);     //wait for play to stop
                configUART(0);          //115200bps
            }
            else if(reload){            //herladen / (nieuw leven)
                ammo = myAmmo;          //momentele ammo is max ammo
                beeps();                //beep soung 1s
                configUART(1);          //9600bps
                setVOL(20);             //set volume to 20
                playNUM(14);            //play reloading
                waitForStop(10000);     //wait for play to stop
                configUART(0);          //115200bps
                if(!life){  //als gamemode niet FFA is
                    life = myLife;      //momentele levens is max levens
                    TMR1ON = 1;         //herstart TMR1
                    INTE = 1;           //herstart external interrupt
                }
                updateLED(ammo,life);   //update ammo en life
            }
            if(IRIF){                   //nieuwe ontvangen data
               /*
                * met gamemode wordt momenteel geen rekening meer gehouden
                * damage wordt geregistreerd waneer inkomend signaal niet van zelfde team komt
                */
               if(IRIF == 1){           //damage
                   hit = 1;
                   beep();
                   configUART(1);      //baud 9600bps for dfplayer
                   setVOL(20);         //set volume to 20
                   playNUM(12);        //play hit
                   waitForStop(2000); //wait for play to stop
                   hit = 0;
                }
               else{                    //killed
                    if(setup){          //als auto setup was geselecteerd
                        configUART(0);  //baud 115200bps for esp01
                        __delay_ms(1);
                        log(myId,myTeam,'K',inPlayer);  //log kill
                        __delay_ms(50); //switch delay
                        configUART(1);  //baud 9600bps for dfplayer
                    }
                    hit = 1;
                    configUART(1);      //baud 9600bps for dfplayer
                    setVOL(20);         //set volume to 20
                    playNUM(13);        //play dead
                    waitForStop(3000);  //wait for play to stop
                    hit = 0;
                    TMR1ON = 0;         //TMR1 uit
                    INTE = 0;           //external interrupt uit
                }
                updateLED(ammo,life);   //update ammo en life
                IRIF = 0;               //clear 'interrupt' flag
            }
        } //end of game
        beeps();                //long beep
        configUART(1);          //baud 9600bps for dfplayer
        setVOL(20);             //set volume to 20
        playNUM(20);            //play gameover
        waitForStop(5000);      //wait for play to stop
        TMR2IE = 0;             //timer2 interrupt uit
        hit = 1;                //hit aan
        life = 0;               //clear levens
        ammo = 0;               //clear ammo
        updateLED(ammo,life); //update ammo en life
        while(!trigger || !reload)continue; //w8 op trigger en reload
        hit = 0;                //hit uit
        beeps();                //long beep
    }   //end of main loop
}

void interrupt isr(void){           //interrupt routine
    static unsigned char BitCount = 0;
    if(TMR1IF){  //als TMR1 interrupt wanneer geen bit wordt ontvangen
        TMR1IF = 0;
        TMR1ON = 0;
        INTEDG = 0;
        TMR1L = 0x00; //clear timer register
        TMR1H = 0x00; //clear timer register
        inData = 0x0000;
        BitCount = 0; //reset bit count
    }
    else if(INTF){          //als edge interrupt
        RA0 = 1;
        INTF = 0;
        if(!TMR1ON){        //start van bit
            TMR1L = 0x00;   //clear timer register
            TMR1H = 0x00;   //clear timer register
            TMR1ON = 1;     //zet timer aan
            INTEDG = !INTEDG; //rising edge
        }
        else{               // end of bit
            TMR1ON = 0;     //stop timer
            if((BitCount == 0)&&((TMR1H>START_MIN) && (TMR1H<START_MAX))){ //wacht op startbit ?
                BitCount = 1;   //data kan ontvangen worden
                inData = 0x0000;//clear data
            }
            else if(BitCount >= 1){
                if(TMR1H>NUL_MIN && TMR1H<NUL_MAX){
                    inData <<= 1;//shift data
                    TMR1L = 0x00;//clear timer register
                    TMR1H = 0x00;//clear timer register
                    INTEDG = 0;  //falling edge
                    BitCount++;  //increment bit count
                }
                else if(TMR1H>EEN_MIN && TMR1H<EEN_MAX){
                    inData <<= 1;//shift data
                    inData |= 1; //maak bit 1
                    TMR1L = 0x00;//clear timer register
                    TMR1H = 0x00;//clear timer register
                    INTEDG = 0;  //faling edge
                    BitCount++;  //increment bit count
                }
            }
            else{
                BitCount = 0; //clear bitcount bij fout
            }
            TMR1L = 0x00;    //clear timer register
            TMR1H = 0x00;    //clear timer register
            INTEDG = 0;      //falling edge
        }
        if(BitCount == 17){  //alle 16 bits zijn ontvangen
            /*
             * indata 16bits
             * XPPPPTTT DDDDDDDD
             */
            inPlayer = 0;   //reset inPLayer
            inTeam = 0;     //reset inTeam
            inPlayer = (((inData&0x7800)>>11)&0xFF);    //krijg id van inkomende speler
            inTeam = (((inData&0x0700)>>8)&0xFF);       //krijg team van inkomende speler
            if(((inData&0xFF00)>>8)==(inData&0x00FF)){
                if(inTeam != myTeam){       //niet hetzelfde team en niet FFA
                    if(15 >= life){   //damage is groter of gelijk aan levens
                        TMR1ON = 0; //stop TMR1
                        INTE = 0;   //stop external interrupt
                        life = 0;   //geen levens
                        IRIF = 2;   //je bent dood
                    }
                    else{             //damage is kleiner dan levens
                        life -= 15;   //trek damage van leven af
                        IRIF = 1;     //damage
                    }
                }
            }
            BitCount = 0;      //reset bitcount
        }
    }
    else if(TMR2IF){
        TMR2IF = 0;                 //clear interrupt flag
        millis++;                   //increment millis from ESP_LIB
        T2CON |= 0b00111000;        //reset postscaler
        T2CON &= 0b00111111;        //reset postscaler
        if(myTime != 0 && time != 1){
            if((millis >= timeOut)){
                TMR1ON = 0; //stop TMR1
                INTE = 0;   //stop external interrupt
                time = 1;
            }
        }
    }
}

void configUART(char speed){
    if(speed == 0){
    /*  UART configs  */
    // Baud rate 115200 w/ xtal 19660800: ((19660800/115200)/4)-1 = 42
    SPBRGH = 42 >> 8;   // Baudrate high register -> 8 MSB of SPBRG 
    SPBRG = 42 & 0xFF;  // Baudrate low register  -> 8 LSB of SPBRG 
    TRISC6 = 0;         // RC6 is TX -> output
    TRISC7 = 1;         // RC7 is RX -> input
    BAUDCTL = 0x08;     // Non inverted TX data, 16bit baud gen
    RCSTA = 0x90;       // Enable serial, enable receiver
    TXSTA = 0x24;       // Send 8bits, enable transmitter, high speed, asynchronous mode
    RD2 = 0;            // Select MUX to 0
    __delay_ms(2);
    }
    else{
    /*  UART configs  */
    // Baud rate 9600 w/ xtal 19660800: ((19660800/9600)/4)-1 = 511
    SPBRGH = 511 >> 8;  // Baudrate high register -> 8 MSB of SPBRG 
    SPBRG = 511 & 0xFF; // Baudrate low register  -> 8 LSB of SPBRG 
    TRISC6 = 0;         // RC6 is TX -> output
    TRISC7 = 1;         // RC7 is RX -> input
    BAUDCTL = 0x08;     // Non inverted TX data, 16bit baud gen
    RCSTA = 0x90;       // Enable serial, enable receiver
    TXSTA = 0x24;       // Send 8bits, enable transmitter, high speed, asynchronous mode
    RD2 = 1;            // Select MUX to 1
    __delay_ms(2);
    }
}

void beep(void){        //short beep
    for(char i=0; i<50; i++){   //loop 50 times
        /* square wave of 714Hz*/
        RB5 = 1;
        __delay_us(700);
        RB5 = 0;
        __delay_us(700);
    }
}

void beeps(void){       //beep for 0.5s
    for(char i=0; i<250; i++){  //loop 250 times
        /* square wave of 1000 Hz*/
        RB5 = 1;
        __delay_ms(1);
        RB5 = 0;
        __delay_ms(1);
    }
}

void selectTeam(void){
    configUART(1);      //baud 9600bps for dfplayer
    setVOL(20);         //set volume to 20
    playNUM(6);         //play team select
    waitForStop(10000);      //wait for play to stop
    while(!reload && !mode);    //wait for button to be pressed
    if(reload){         //pressed on reload
        myTeam = 1;     //set local team
        outData |= 0b0000000100000000; //set team in ir data
        setVOL(20);     //set volume to 20
        playNUM(8);     //play red selected
        setLED(0,20,0,0);       //first led red
    }
    else{               //pressed on mode
        myTeam = 2;     //set local team
        outData |= 0b0000001000000000; //set team in ir data
        setVOL(20);     //set volume to 20
        playNUM(7);     //play blue selected
        setLED(0,0,0,20);       //first led blue
    }
    beep();             //beep sound
    writeLED();         //give color to leds
    waitForStop(10000);      //wait for audio to stop playing
    __delay_ms(250);    //no doubble press
}


void setupSelect(void){
    while(!reload && !mode);    //wait for button to be pressed
    if(mode){       //pressed on mode
        setup = 0;  //manual mode
        setVOL(20); //set volume to 20
        playNUM(3); //play manual selected
    }
    else{
        setup = 1;  //auto mode
        setVOL(20); //set volume to 20
        playNUM(4); //play auto selected
    }
    beep();         //beep sound
    waitForStop(10000);  //wait for play to stop
    __delay_ms(250);//no doubble press
}

void log(unsigned char id,unsigned char team,char type,unsigned char kby){
    configUART(0);      //baud 115200bps for ESP01
    char buf[88];       //request buffer
    char bufb[4];       //request length buffer
    sprintf(buf,"GET /SITEV2/esp/regDead.php?N=%d&T=%d&Y=%c&K=%d HTTP/1.0\r\nHost: 192.168.0.70\r\n\r\n",id,team,type,kby);
    sprintf(bufb,"%d",strlen(buf));//create string
    writeString("AT+CIPSTART=\"TCP\",\"192.168.0.70\",80\r\n");//connect to server
    //waitFor("OK\r\n",2000);              //wait for response OK
    __delay_ms(20);
    writeString("AT+CIPSEND=");//setup send to server
    writeString(bufb);  //send request string length
    writeString("\r\n");//end command
    //waitFor("> ",2000);              //wait for response
    __delay_ms(20);
    writeString(buf);   //send request string
    writeString("\r\n");//end request string
}

void updateLED(unsigned char am,unsigned char li){
    if(am == 0){    //if no ammo
        /* 10 first LEDs purple */
        for(unsigned char i = 0; i<9; i++){
            setLED(i,10,0,10);
        }
    }
    else{           //ammo available
        /* scale ammo to 10 leds*/
        unsigned char x = map(am,1,myAmmo,0,NUMLEDS/2);
        /* if led is in range -> red */
        for(unsigned char i = 0; i<x; i++){
                setLED(i,20,0,0);
        }
        /* if led is not in range -> off*/
        for(unsigned char i = NUMLEDS/2; i>x; i--){
            setLED(i,0,0,0);
        }
    }
    if(li == 0){    //if no lives
        /* 10 last LEDs purple */
        for(unsigned char i = NUMLEDS/2; i<NUMLEDS; i++){
            setLED(i,20,0,20);
        }
    }
    else{           //ammo available
        /* scale life to 10 leds */
        unsigned char y = map(li,1,myLife,NUMLEDS/2,NUMLEDS);
        /* if led is in range -> green */
        for(unsigned char i = NUMLEDS/2; i<y; i++){
            setLED(i,0,20,0);
        }
        /* if led is not in range -> off*/
        for(unsigned char i = NUMLEDS; i>y; i--){
            setLED(i,0,0,0);
        }   
    }
    writeLED();     //write data to LEDs
}

/* scaling function like arduino*/
unsigned int map(unsigned int in, unsigned int inm, unsigned int inM, unsigned int outm, unsigned int outM)
{
    return (in - inm) * (outM - outm) / (inM - inm) + outm;
}