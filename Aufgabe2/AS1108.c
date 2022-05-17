/*
 * SPI_AS1108.c
 *
 *  Created on: 26.12.2018
 *      Author: Admin
 */


#include <msp430.h>
#include "../base.h"
#include "event.h"
#include "AS1108.h"

// Basis des Zahlensystems
// Einstellung zwischen 2 und 10 soll m�glich sein
#define BASE 10

// es sind geeignete Datenstrukturen f�r den Datenaustausch
// zwischen den Handlern festzulegen.

LOCAL Void AS1108_Write(UChar adr, UChar arg) {
   Char ch = UCA1RXBUF;   // dummy read, UCRXIFG := 0, UCOE := 0
   CLRBIT(P2OUT,  BIT3);  // Select aktivieren
   UCA1TXBUF = adr;       // Adresse ausgeben
   while (TSTBIT(UCA1IFG, UCRXIFG) EQ 0);
   ch = UCA1RXBUF;        // dummy read
   UCA1TXBUF = arg;       // Datum ausgeben
   while (TSTBIT(UCA1IFG, UCRXIFG) EQ 0);
   ch = UCA1RXBUF;        // dummy read
   SETBIT(P2OUT,  BIT3);  // Select deaktivieren
}


GLOBAL Void SPI_Init(Void) {
   // set up Pin 3 at Port 2 => CS output, High
   SETBIT(P2OUT,  BIT3);
   SETBIT(P2DIR,  BIT3);
   CLRBIT(P2SEL0, BIT3);
   CLRBIT(P2SEL1, BIT3);
   CLRBIT(P2REN,  BIT3);

   // set up Pins 4, 5 and 6 at Port 2
   CLRBIT(P2SEL0, BIT4 + BIT5 + BIT6);
   SETBIT(P2SEL1, BIT4 + BIT5 + BIT6);

   // set up Universal Serial Communication Interface A
   SETBIT(UCA1CTLW0, UCSWRST);        // UCA1 software reset

   // in �bereinstimung mit dem SPI-Timing-Diagramm von AS1108
   UCA1CTLW0_H = 0b10101001;          // clock phase: rising edge, ...
                                      // ... clock polarity: inactive low ...
                                      // ... MSB first, 8-bit data, SPI master mode, ...
                                      // ... 3-pin SPI, synchronus mode

   CLRBIT(UCA1CTLW0, UCSSEL1);        // select ACLK ...
   SETBIT(UCA1CTLW0, UCSSEL0);        // .. as source clock
   UCA1BRW = 0;                       // prescaler

   CLRBIT(UCA1CTLW0, UCSWRST);        // release the UCA0 for operation
}


// der Treiberbaustein AS1108 ist hier �ber die SPI-Schnittstelle zu initialisieren
GLOBAL Void AS1108_Init(Void) {
    // Shutdown
    AS1108_Write(0x0C, 0x81);
    // Decode mode on Code-B for all digits
    AS1108_Write(0x09, 0xFF);
    // Intensity on max
    AS1108_Write(0x0A, 0x06);
    // Scan limit on display all digits
    AS1108_Write(0x0B, 0x03);

    // Feature
    // AS1108_Write(0x0E, 0b00000000);
    // Display test on normal operation
    // AS1108_Write(0x0F, 0);

    // Init with 0
    //AS1108_Write(0x01, 0);
    //AS1108_Write(0x02, 0);
    //AS1108_Write(0x03, 0);
    //AS1108_Write(0x04, 0);

    set_event(EVENT_8);
    __low_power_mode_off_on_exit();
}

// ----------------------------------------------------------------------------
GLOBAL UInt steps_size = 0;

// der Button-Handler beinhaltet keine Zustandsmaschiene
GLOBAL Void Button_Handler(Void) {
    if (tst_event(EVENT_3)) {
        clr_event(EVENT_3);
        steps_size = 1;
        set_event(EVENT_7);
        __low_power_mode_off_on_exit();
    } else if (tst_event(EVENT_4)) {
        clr_event(EVENT_4);
        steps_size = 10;
        set_event(EVENT_7);
        __low_power_mode_off_on_exit();
    } else if (tst_event(EVENT_5)) {
        clr_event(EVENT_5);
        steps_size = 100;
        set_event(EVENT_7);
        __low_power_mode_off_on_exit();
    } else if (tst_event(EVENT_6)) {
        clr_event(EVENT_6);
        steps_size = 1000;
        set_event(EVENT_7);
        __low_power_mode_off_on_exit();
    }

}

// ----------------------------------------------------------------------------
GLOBAL Int value = 0;

// der Number-Handler beinhaltet keine Zustandsmaschiene
GLOBAL Void Number_Handler(Void) {
    if (tst_event(EVENT_7)) {
       clr_event(EVENT_7);
       if (TSTBIT(P2OUT, BIT7)) {
           if (value LT step_size) {
               value += 10000;
           }
           value -= steps_size;
       } else {
           value += steps_size;
       }
       set_event(EVENT_8);
       __low_power_mode_off_on_exit();
   }
}

// ----------------------------------------------------------------------------

// der AS1108_Hander beinhaltet eine Zustandsmaschine
GLOBAL Void AS1108_Handler(Void) {
    if (tst_event(EVENT_8)) {
        clr_event(EVENT_8);

        Int tmp = value;
        if (tmp GE 10000) {
            tmp -= 10000;
        }

        for (int i = 1; i < 5; i++) {
            // Better if these Ints are outside?
            Int to_check = 0;
            Int digit = 0;
            if (i EQ 1) {
                to_check = 1000;
            } else if (i EQ 2) {
                to_check = 100;
            } else if (i EQ 3) {
                to_check = 10;
            } else if (i EQ 4) {
                to_check = 1;
            }

            if (tmp EQ to_check) {
                digit = 1;
            } else if (tmp GT to_check) {
                for (int j = 0; j < 10; j++) {
                    if (tmp GE steps_size) {
                        digit++;
                        tmop -= steps_size;
                    } else {
                        break;
                    }
                }
            }
            AS1108_Write(i, digit);
        }
//           10823
//           10000
//
//           823 < 1000 -> 0
//
//           823 > 100
   }
}

