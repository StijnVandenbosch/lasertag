#include <htc.h>
#define _XTAL_FREQ 19660800

#define START_BYTE 0x7E
#define VERSION 0xFF
#define DATA_LEN 0x06
#define END_BYTE 0xEF
#define nBUSY RD4

void sendCMD(unsigned char CMD, int VAL, char FEEDBACK);
void setVOL(unsigned char VOL);
void playNUM(unsigned char NUM);
void playSTOP(void);
//void waitForStop(void);
void startMP3(void);
void checkBusy(void);
void waitForStop(int timeout);