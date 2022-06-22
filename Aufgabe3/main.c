#include <msp430.h> 
#include "../base.h"
#include "TA0.h"
#include "event.h"
#include "AS1108.h"
#include "UCA0.h"

/**
 * main.c
 */

LOCAL Void CS_Init(Void);
LOCAL Void Port_Init(Void);

GLOBAL Void main(Void) {
   // stop watchdog timer
   WDTCTL = WDTPW + WDTHOLD;

   CS_Init();     // set up Clock System
   Port_Init();   // set up LED ports
   SPI_Init();    // set up SPI
   TA0_Init();    // set up BTN Ports and Timer A0
   UCA0_Init();   // set up UART

   AS1108_Init();

   while(TRUE) {
      wait_for_event();

      if (tst_event(EVENT_SETDIGITS)) {
          clr_event(EVENT_SETDIGITS);

          digits[0] = buffer[3] - 0x30;
          digits[1] = buffer[2] - 0x30;
          digits[2] = buffer[1] - 0x30;
          digits[3] = buffer[0] - 0x30;
          set_event(EVENT_DIGI);
      }

      if (tst_event(EVENT_BTN2)) {
         clr_event(EVENT_BTN2);
         TGLBIT(P2OUT, BIT7);
      }

      // wenn die drei Handler korrekt implementiert sind,
      // kann man ihre Reihnefolge hier beliebig �ndern
      Button_Handler();
      Number_Handler();
      AS1108_Handler();

      if (tst_event(EVENT_SHOWTERM)) {
          clr_event(EVENT_SHOWTERM);
          // TODO optimize!
          const Char hex_digits[7] = {0x30 + digits[3], 0x30 + digits[2], 0x30 + digits[1], 0x30 + digits[0], 0x0D, 0x0A, 0x00};
          UCA0_printf(hex_digits);
      }

      // im Falle eines Event-Errors leuchtet die LED dauerhaft
      if (is_event_error()) {
         SETBIT(P1OUT, BIT2);
      }
   }
}

LOCAL Void CS_Init(Void) {
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

   // set up Pin 2 at Port 1 => output, LED2
   CLRBIT(P1OUT,  BIT2);
   SETBIT(P1DIR,  BIT2);
   CLRBIT(P1SEL0, BIT2);
   CLRBIT(P1SEL1, BIT2);
   CLRBIT(P1REN,  BIT2);
}
