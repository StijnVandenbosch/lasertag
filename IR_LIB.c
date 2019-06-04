#include <htc.h>
#include <stdio.h>
#include <stdlib.h>
#include <htc.h>
#include "IR_LIB.h"
#include "globals.h"
#define _XTAL_FREQ 19660800

void IR_INIT(void){
    TRISB0 = 1;      //INTE als input
    T1CON &= 0x94;   //fosc/4 : no gate : stop timer
    T1CON |= 0x10;   //pre /2
    INTEDG = 0;      //falling edge interrupt
    TMR1IE = 1;      //TMR1 interrupt enable
    INTCON = 0xD0;   //GIE : PEIE : INTE
    
    TMR1L = 0;      //clear timer register
    TMR1H = 0;      //clear timer register
    IRTRIS = 0;     //IR output pin output
}

void IR_Send(void){
    TMR1IE = 0; //disable timer
    INTE = 0;   //disable edge interrupt
    unsigned int temp = outData;//out data to temp int
    IR_START();
    for(char i=0; i<16; i++){   //loop trough all data
            if(temp&0x8000){    //check msb
                IR_ONE();   //one pulse
            }
            else{
                IR_ZERO();  //zero pulse
            }
            temp <<= 1;     //shift data
    }
    TMR1IE = 1; //enable timer
    INTE = 1;   //enable edge interrupt
}

void IR_ONE(void){
    /* generate long ir pulse and flash muzzle */
    for(char i = 0;i<35;i++){
        IRPIN = 1;
        muzzle = 1;
        __delay_us(15);
        IRPIN = 0;
        muzzle = 0;
        __delay_us(15);
    }
    __delay_us(300);    //time between pulses
}

void IR_ZERO(void){
    /* generate short ir pulse and flash muzzle */
    for(char i = 0;i<10;i++){
        IRPIN = 1;
        muzzle = 1;
        __delay_us(15);
        IRPIN = 0;
        muzzle = 0;
        __delay_us(15);
    }
    __delay_us(300);    //time between pulses
}

void IR_START(void){
  for(char i = 0;i<52;i++){
        IRPIN = 1;
        muzzle = 1;
        __delay_us(15);
        IRPIN = 0;
        muzzle = 0;
        __delay_us(15);
    }
  __delay_us(300);    //time between pulses
}