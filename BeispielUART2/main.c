#include <msp430.h> 
#include <ctype.h>
#include "..\base.h"
#include "event.h"
#include "TA0.h"
#include "UCA0.h"


LOCAL Void CS_Init(Void);
LOCAL Void Port_Init(Void);


/**
 * main.c
 */

LOCAL Void CS_Init(Void);

LOCAL const char msg[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ >>>>> 0123456789 >>>>> 0123456789 >>>>>> HalloWorld!\r\n";

GLOBAL Void main(Void) {

   // stop watchdog timer
   WDTCTL = WDTPW + WDTHOLD;

   CS_Init();   // set up Clock System
   Port_Init(); // set up Port
   TA0_Init();  // set up Timer A
   UCA0_Init(); // set up UART

   while(TRUE) {
      wait_for_event();
      if (tst_event(EVENT_IMA)) {
         TGLBIT(P2OUT, BIT7);
         clr_event(EVENT_IMA);
      }

      if (tst_event(EVENT_RXD)) {
         clr_event(EVENT_RXD);
         UCA0_printf(msg);
      }
   }
}

LOCAL Void CS_Init(Void) {
   // set up Clock System
    CSCTL0 = CSKEY;                                       // enable clock system
    CSCTL1 = DCOFSEL_3;                                   // DCO frequency = 8,0 MHz
    CSCTL2 = SELA__XT1CLK + SELS__DCOCLK + SELM__DCOCLK;  // select clock sources
    CSCTL3 = DIVA__8      + DIVS__8      + DIVM__8;       // set frequency divider
    // XT2 disabled, XT1: HF mode,  low power, no bypass
    CSCTL4 = XT2OFF + XTS + XT1DRIVE_0;
    CSCTL0_H = 0;                                         // disable clock system
}

LOCAL Void Port_Init(Void) {
   // set up Pin 7 at Port 2 => output, LED1
   CLRBIT(P2OUT,  BIT7);
   SETBIT(P2DIR,  BIT7);
   CLRBIT(P2SEL0, BIT7);
   CLRBIT(P2SEL1, BIT7);
   CLRBIT(P2REN,  BIT7);
}
