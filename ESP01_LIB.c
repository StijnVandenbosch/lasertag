/*
 * Library created by Stijn Vandenbosch
 *  // used to communicate with esp8266 modules with AT commands
 * date: 08/02/2019
 *
 * UART settings calculated for 19660800Hz XTAL
 * HITECH C compiler
 */
#include "ESP01_LIB.h"

void writeChar(char* e){ //stuur een char
    while(!TXIF)continue;
    TXREG = e;
}

unsigned char waitFor(char* string, unsigned int timeOut) {
    unsigned long time = millis;
    unsigned char progress = 0; // count of correct received characters
    char temp;                  // char to store received
    do {
        temp = getByte();       // get character
        if (temp == string[progress]) { //check if character is correct
            progress++;         //update progress
        }
        else {                  //if char is wrong
            progress = 0;       //reset progress
        }
    } while (string[progress] != '\0' && temp != 0 && (millis <= (time+timeOut)));
    if(string[progress] == '\0') return 1;
    else return 0;
}

char getByte() //function that waits for data and returns received char
{
    /*
     * OERR and FERR of the RCSTA reg tell you if there was a problem with reading
     * the OERR bit is high when the receive buffer was full and the byte was lost
     * the FERR bit is high when there was a framing error (each byte in buffer seperately)
     * the receiver needs to be resetted to clear the OERR bit
     */
    if(OERR || FERR){   //if there was a read error
        CREN = 0;       //turn receive off
        CREN = 1;       //turn receive on
        return 0;       //return 0 -> error
    }
    else{
        while(!RCIF);  // wait for data
        return RCREG;  //return received byte
    }
}

void writeString(const char* pointer){// Function w/ pointer to string
    for(unsigned char i = 0; pointer[i] != '\0';i++){// Loop while not end of string -> \0
        while(!TXIF)continue;   // Wait for previous byte to be sent
        TXREG = pointer[i];     // Send current character byte
    }
}

char* esp_getIP(char type){
    char temp;              //store received char
    char outS[16];          //full size of IP in string format
    char count = 0;         //keeps track of received correct characters
    writeString("AT+CIFSR\r\n"); //request all IP's
    if(type == STA){        //if want station IP
      waitFor("STAIP,",2000);    //look for STAIP,
    }
    else if(type == AP){    //else want AP IP
      waitFor("APIP,",2000);     //look for APIP,
    }
    temp = getByte();       //get the incomming byte
    while(temp != '\r'){    //while no carriage return
        if(temp<58 && temp>47){ //ascii value's for numbers
          outS[count]= temp;//store character
          count++;          //update count
        }
        else if(temp == '.'){//if received was a point
          outS[count]=temp; //store characters
          count++;          //update count
        }
        temp = getByte();   //get the incomming byte
    }
    outS[count]='\0';       //last character end needs to be NULL for string
    waitFor("OK\r\n",2000);      //wait for OK
    return outS;            //return pointer to received string
}

void sendData(char* Id, int data){
    char buf[10];            //buffer for data
    char buf2[4];            //buffer for stringlen
    sprintf(buf,"%d",data);  //create string
    sprintf(buf2,"%d",strlen(buf));//create string
    writeString("AT+CIPSEND="); //start send
    writeChar(Id);
    writeString(",");
    writeString(buf2);       //size of string
    writeString("\r\n");     //confirm
    waitFor("> ",2000);           //ready to receive
    writeString(buf);        //send string
    waitFor("OK\r\n",2000);        //wait for response (can be error)
}

void sendString(char* Id, char* string){
    char buf[95];            //buffer for string
    char buf2[4];            //buffer for stringlen
    sprintf(buf,"%s",string);//create string
    sprintf(buf2,"%d",strlen(buf));//create string
    writeString("AT+CIPSEND="); //start send
    writeChar(Id);           //connection to send to
    writeString(",");
    writeString(buf2);       //size of string
    writeString("\r\n");     //confirm
    waitFor("> ",2000);           //ready to receive
    writeString(buf);        //send string
    waitFor("OK\r\n",2000);       //wait for response (can be error)
}

void esp_init(void){
    /*
     * TMR1 op fosc/04, prescaler 1:1
     * en preload van 60647 -> 0.995 ms
     */
    /* Timer1 instellingen */
    TMR1GE = 0; //timer on als TMR1ON = 1;
    T1CKPS0 = 0;//1:1 prescaler
    T1CKPS1 = 0;//1:1 prescaler
    T1OSCEN = 0;//no lp oscillator
    TMR1CS = 0; //use fosc/4
    TMR1L = 60647 & 0xFF;   //preload
    TMR1H = 60647 >> 8;     //preload
    TMR1ON = 1; //Timer1 aan
    TMR1IE = 1; //enable interrupt
}

//void millisTimer(void){
//    if(TMR1IF){
//        TMR1IF = 0;       //clear interrrupt flag
//        TMR1ON = 0;
//        TMR1L = 60647 & 0xFF;   //preload
//        TMR1H = 60647 >> 8;     //preload
//        TMR1ON = 1;
//        millis++;         //increse millis
//        RD1 = 1;        
//    }
//} 


