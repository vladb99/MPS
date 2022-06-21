/*
 * UCA0.c
 *
 *  Created on: 10.04.2018
 *      Author: Admin
 */

#include <msp430.h>
#include "..\base.h"
#include "uca0.h"
#include "event.h"

//#define ACLK_614kHz_9600
#define SMCLK_1000kHz

GLOBAL Void UCA0_Init(Void) {

   // set up Universal Serial Communication Interface A
   SETBIT(UCA0CTLW0, UCSWRST);        // UCA0 software reset
   UCA0CTLW1 = 0x0002;                // deglitch time approximately 100 ns
   UCA0CTLW0_H = 0xC0;                // even parity, LSB first,  ...
                                      // ... 8-bit data,  One stop bit,
                                      // ... UART mode, Asynchronous mode
   SETBIT(UCA0CTLW0, UCBRKIE);        // receive break character interrupt enable
#ifdef ACLK_614kHz_9600               // only with LPM3
   SETBIT(UCA0CTLW0, UCSSEL__ACLK);   // select clock source: ACLK with 614.4 kHz
   UCA0BRW = 4;                       // set clock prescaler for 9600 baud
   UCA0MCTLW_L = 0;                   // first modulation stage
   UCA0MCTLW_H = 0x00;                // second modulation stage
   SETBIT(UCA0MCTLW, UCOS16);         // enable 16 times oversampling
#endif
#ifdef SMCLK_1000kHz                  // only with LPM1
   SETBIT(UCA0CTLW0, UCSSEL__SMCLK);  // select clock source: SMCLK with 1.0 MHz
   UCA0BRW = 3;                       // set clock prescaler for 19200 baud
   UCA0MCTLW_L = 4 << 4;              // first modulation stage
   UCA0MCTLW_H = 0x04;                // second modulation stage
   SETBIT(UCA0MCTLW, UCOS16);         // enable 16 times oversampling
#endif

   CLRBIT(P2SEL0, BIT1 + BIT0);       // set up Port 2: Pin0 => TXD, ...
   SETBIT(P2SEL1, BIT1 + BIT0);       // ... Pin1 <= RXD
   CLRBIT(P2REN,  BIT1 + BIT0);       // without pull up

   CLRBIT(UCA0CTLW0, UCSWRST);        // release the UCA0 for operation
   SETBIT(UCA0IE, UCRXIE);            // enable receive interrupt
}

/*
 * The UCRXIFG interrupt flag is set each time a character is received and loaded into UCAxRXBUF.
 * An interrupt request is generated if UCRXIE and GIE are also set. UCRXIFG and UCRXIE are reset
 * by a system reset PUC signal or when UCSWRST = 1. UCRXIFG is automatically reset when UCAxRXBUF
 * is read.
 */
#pragma vector = USCI_A0_VECTOR
__interrupt Void UCA0_ISR(Void) {
   switch (__even_in_range(UCA0IV, 0x08)) {
      case 0x02:  // Vector 2: Receive buffer full
         if (TSTBIT(UCA0STATW, UCBRK + UCRXERR)) {
            Char ch = UCA0RXBUF; // dummy read
         } else {
            UCA0TXBUF = UCA0RXBUF;
         }
         break;
      case 0x04:  // Vector 4: Transmit buffer empty
         break;
      case 0x06:  // Vector 6: Start bit received
         break;
      case 0x08:  // Vector 6: Transmit complete
         break;
      default:
         break;
   }
}
