/* 
 * File:   ESP01_LIB.h
 * Author: Stijn v
 *
 * Created on 8 februari 2019, 19:05
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <htc.h>

void esp_init(void);
char getByte(void);
unsigned char waitFor(char* string, unsigned int timeOut);
char* esp_getIP(char type);
void writeString(const char* pointer);
void sendData(char* Id, int data);
void sendString(char* Id, char* string);
void writeChar(char* e);
//void millisTimer(void);

/*
 * De millis variabele houdt de tijd bij in ms vanaf het programma gestart is.
 * -> unsigned long = max 4294967295 => +- 50 dagen
 */
volatile unsigned long millis = 0;
unsigned long timeOut = 0;

#define STA 1
#define AP 2

