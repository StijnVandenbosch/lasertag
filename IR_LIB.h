#define NUL_PULS 10 //10 periodes van 28µs
#define EEN_PULS 40 //20 periodes van 28µs
#define NUL_MIN 1  //minimale tijd van NUL 1 keer TMR1H @ 9600Hz
#define NUL_MAX 8  //maximale tijd van NUL 4 keer TMR1H @ 9600Hz
#define EEN_MIN 10  //minimale tijd van EEN 5 keer TMR1H @ 9600Hz
#define EEN_MAX 18  //maximale tijd van EEn 8 keer TMR1H @ 9600Hz
#define START_MIN 15  //minimale tijd van EEN 5 keer TMR1H @ 9600Hz
#define START_MAX 31  //maximale tijd van EEn 8 keer TMR1H @ 9600Hz

#define IRPIN RC1
#define IRTRIS TRISC1

#define muzzle RD1

void IR_INIT(void);
void IR_Handler(void);
void IR_Send(void);
void IR_ONE(void);
void IR_ZERO(void);
void IR_START(void);