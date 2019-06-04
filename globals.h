/* 
 * File:   globals.h
 * Author: Stijn v
 *
 * Created on 17 april 2019, 11:34
 */

unsigned int inData = 0x0000;   //2bytes of incomming data
unsigned int outData = 0x0000;	//2bytes of outgoing data
unsigned char inTeam = 0;       //inkomend teamnummer
unsigned char inPlayer = 0;     //id van inkomende speler
unsigned char inWeapon = 0;     //damage van inkomende speler
unsigned char myId = 0;         //mijn speler id
unsigned char myWeapon = 50;    //damage van mijn levens
unsigned char myAmmo = 20;      //standaard 20 kogels
unsigned char myLife = 100;     //standaard 100 levenspunten
unsigned char myTime = 0;       //standaard geen tijdslimiet
unsigned char myTeam = 1;       //stanaard team 1
unsigned char life = 100;       //momentele levens is mijn maximale levens
unsigned char gameMode = 1;     //gamemode tead deathmatch
unsigned char IRIF = 0;         //software created interrupt flag 
