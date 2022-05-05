/*
 * TA0.c
 *
 *  Created on: 22.04.2018
 *      Author: Admin
 */

#include <msp430.h>
#include "../base.h"
#include "event.h"

#define LED2MAX 50                           // for 500 ms period

GLOBAL Void TA0_Init(Void) {
   TA0CCR0 = 0;                              // stopp Timer A
   CLRBIT(TA0CTL, TAIFG);                    // clear overflow flag
   CLRBIT(TA0CCR0, CCIFG);                   // clear CCI flag
   TA0CTL  = TASSEL__ACLK + MC__UP + ID__8;  // set up Timer A
   TA0EX0  = TAIDEX_7;                       // set up expansion register
   TA0CCR0 = 2*48;                            // set up CCR0 for 10 ms
   SETBIT(TA0CTL, TACLR);                    // clear and start Timer
   SETBIT(TA0CCTL0, CCIE);                   // enable Timer A interrupt
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt Void TA0_ISR(Void) {
   static volatile UInt LED2cnt = LED2MAX;   // LED2 counter
   if (--LED2cnt EQ 0) {
      LED2cnt = LED2MAX;                     // set up counter
      set_event(EVENT_IMA);                  // set up event
      __low_power_mode_off_on_exit();        // restore Active Mode on return
   }
   if (TSTBIT(P1IN, BIT0 + BIT1)) {
      SETBIT(P1OUT, BIT2);
   } else {
      CLRBIT(P1OUT, BIT2);
   }
}
