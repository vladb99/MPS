/*
 * UCA0.c
 *
 *  Created on: 10.04.2018
 *      Author: Admin
 */

#include <msp430.h>
#include "../base.h"
#include "uca0.h"
#include "event.h"

//#define WITH_BUSY_WAITING
#define WITH_INTERRUPT

#ifdef WITH_INTERRUPT
LOCAL const Char EndOfTExt = '\0';
LOCAL const Char * ptr = &EndOfTExt;
#endif

GLOBAL Void UCA0_Init(Void) {

   // set up Universal Serial Communication Interface A
   SETBIT(UCA0CTLW0, UCSWRST);        // UCA0 software reset
   UCA0CTLW1 = 0x0002;                // deglitch time approximately 100 ns
   UCA0CTLW0_H = 0xC0;                // even parity, LSB first,  ...
                                      // ... 8-bit data,  One stop bit,
                                      // ... UART mode, Asynchronous mode
   SETBIT(UCA0CTLW0, UCBRKIE);        // receive break character interrupt enable
   SETBIT(UCA0CTLW0, UCSSEL__ACLK);   // select clock source: ACLK with 614.4 kHz
   UCA0BRW = 4;                       // set clock prescaler for 9600 baud
   UCA0MCTLW_L = 0;                   // first modulation stage
   UCA0MCTLW_H = 0x00;                // second modulation stage
   SETBIT(UCA0MCTLW, UCOS16);         // enable 16 times oversampling
   CLRBIT(P2SEL0, BIT1 + BIT0);       // set up Port 2: Pin0 => TXD, ...
   SETBIT(P2SEL1, BIT1 + BIT0);       // ... Pin1 <= RXD
   CLRBIT(P2REN,  BIT1 + BIT0);       // without pull up
   CLRBIT(UCA0CTLW0, UCSWRST);        // release the UCA0 for operation
   SETBIT(UCA0IE, UCRXIE);            // enable receive interrupt
}

#pragma vector = USCI_A0_VECTOR
__interrupt Void UCA0_ISR(Void) {
   switch (__even_in_range(UCA0IV, 0x04)) {
      case 0x02:  // Vector 2: Receive buffer full
         if (TSTBIT(UCA0STATW, UCBRK + UCRXERR)) {
            Char ch = UCA0RXBUF; // dummy read
         } else {
            if (UCA0RXBUF EQ '?') {
               set_event(EVENT_RXD);
               __low_power_mode_off_on_exit(); // restore Active Mode on return
            }
         }
         break;
#ifdef WITH_INTERRUPT
      case 0x04:  // Vector 4: Transmit buffer empty
         if (*ptr NE '\0') {
            UCA0TXBUF = *ptr++;
         } else {
            ptr = &EndOfTExt;
            CLRBIT(UCA0IE, UCTXIE);   // disable transmit interrupt
         }
#endif
      default:
         break;
   }
}

#ifdef WITH_BUSY_WAITING
GLOBAL Int UCA0_printf(const Char * str) {
   if (str EQ NULL) {
      return -1;
   }
   while (*str NE '\0') {
      UCA0TXBUF = *str++;
      // ohne while-Schleife funktioniert die Ausgabe nicht
      while (TSTBIT(UCA0IFG, UCTXIFG) EQ 0) ;
   }
   return 0;
}
#endif

#ifdef WITH_INTERRUPT
GLOBAL Int UCA0_printf(const Char * str) {
   if (str EQ NULL) {
      return -1;
   }
   if (*ptr EQ '\0') {
      ptr = str;
      SETBIT(UCA0IFG, UCTXIFG); // set UCTXIFG
      SETBIT(UCA0IE,  UCTXIE);  // enable transmit interrupt
      return 0;
   }
   return -1;
}
#endif
