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

char index = 0;
signed char buffer[4];
Bool is_buffer_set = FALSE;

GLOBAL Void UCA0_Init(Void) {

   // set up Universal Serial Communication Interface A
   SETBIT(UCA0CTLW0, UCSWRST);        // UCA0 software reset
   UCA0CTLW1 = 0x0002;                // deglitch time approximately 100 ns
   UCA0CTLW0_H = 0xC0;                // even parity, LSB first,  ...
                                      // ... 8-bit data,  One stop bit,
                                      // ... UART mode, Asynchronous mode
   SETBIT(UCA0CTLW0, UCBRKIE);        // receive break character interrupt enable
   SETBIT(UCA0CTLW0, UCSSEL__ACLK);   // select clock source: ACLK with 614.4 kHz

   UCA0BRW = 2;                       // set clock prescaler for 14400 baud
   UCA0MCTLW_L = 10<<4;               // first modulation stage
   UCA0MCTLW_H = 0xB7;                // second modulation stage
   SETBIT(UCA0MCTLW, UCOS16);         // enable 16 times oversampling

   CLRBIT(P2SEL0, BIT1 + BIT0);       // set up Port 2: Pin0 => TXD, ...
   SETBIT(P2SEL1, BIT1 + BIT0);       // ... Pin1 <= RXD
   CLRBIT(P2REN,  BIT1 + BIT0);       // without pull up
   CLRBIT(UCA0CTLW0, UCSWRST);        // release the UCA0 for operation
   SETBIT(UCA0IE, UCRXIE);            // enable receive interrupt
}

#pragma vector = USCI_A0_VECTOR
__interrupt Void UCA0_ISR(Void) {
    register ch;
   switch (__even_in_range(UCA0IV, 0x04)) {
      case 0x02:  // Vector 2: Receive buffer full
         if (TSTBIT(UCA0STATW, UCBRK + UCRXERR)) {
            ch = UCA0RXBUF; // dummy read
            set_error_code(BR_ERROR);
         } else if (TSTBIT(UCA0STATW, UCFE + UCPE + UCOE)) {
             ch = UCA0RXBUF; // dummy read
             set_error_code(FOP_ERROR);
         } else {
             ch = UCA0RXBUF;
             if ((ch NE ENTER) AND (NOT(between(0x30, ch, 0x39)))) {
                 set_error_code(C_ERROR);
             } else if ((between(0x30, ch, 0x39) AND index EQ 4) OR (ch EQ ENTER AND index NE 4)) {
                 set_error_code(BU_ERROR);
             } else {
                 if (between(0x30, ch, 0x39)) {
                     buffer[index] = ch;
                     index++;
                     set_blink_muster(BY_RX);
                 } else if (ch EQ ENTER) {
                     index = 0;
                     //error_code = BY_RX;
                     set_blink_muster(BY_RX);
                     is_buffer_set = TRUE;
                     set_event(EVENT_DIGI);
                 }
             }
         }
         __low_power_mode_off_on_exit(); // restore Active Mode on return
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

Void set_error_code(Char code) {
    if (code NE BY_RX) {
        index = 0;
    }
    set_blink_muster(code);
}

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
