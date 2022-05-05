#include <msp430.h> /* --> msp430fr5729.h */
#include "../base.h"

// die LED am Pin 7 vom Port 2 soll mit
// einer Frequenz von ca. 1 Hz blinken

// die Verz�gerung wird meistens
// experimentell bestimmt
#define DLY 480000

//#define NOBITMACROS

LOCAL Void exec(Void);

/**
 * main.c
 */
GLOBAL Int main(Void) {
   // stop watchdog timer
   WDTCTL = WDTPW + WDTHOLD;

   exec();
   return 0;
}

LOCAL Void exec(Void) {

    #ifdef NOBITMACROS
    P2OUT  &= ~BIT7;
    P2DIR  |=  BIT7;
    P2SEL0 &= ~BIT7;
    P2SEL1 &= ~BIT7;
    P2REN  &= ~BIT7;

    while(TRUE) {
        __delay_cycles(DLY);
       P2OUT |= BIT7;
       __delay_cycles(DLY);
       P2OUT &= ~BIT7;
    }
    #else
    CLRBIT(P2OUT,   BIT7);
    SETBIT(P2DIR,   BIT7);
    CLRBIT(P2SEL0,  BIT7);
    CLRBIT(P2SEL1,  BIT7);
    CLRBIT(P2REN,   BIT7);

    while(TRUE) {
        __delay_cycles(DLY);
        TGLBIT(P2OUT, BIT7);
    }
    #endif
}
