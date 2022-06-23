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

// fifth element is a dont care
signed char digits[] = {0, 0, 0, 0, 0};
UChar selected;
UChar i;

Char button_events[4] = {EVENT_BTN3, EVENT_BTN4, EVENT_BTN5, EVENT_BTN6};
Char button_index = 0;

Char hex_digits[7] = {0, 0, 0, 0, 0x0D, 0x0A, 0x00};

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

    //hex_digits = {0x30 + digits[3], 0x30 + digits[2], 0x30 + digits[1], 0x30 + digits[0], 0x0D, 0x0A, 0x00};
    set_event(EVENT_DIGI);
}

// ----------------------------------------------------------------------------
// der Button-Handler beinhaltet keine Zustandsmaschiene
GLOBAL Void Button_Handler(Void) {
    Char event = button_events[button_index];
    if (tst_event(event)) {
        clr_event(event);
        selected = button_index;
        set_event(EVENT_15);
    }
    button_index++;
    if (button_index == 4) {
        button_index = 0;
    }
}

// ----------------------------------------------------------------------------
// der Number-Handler beinhaltet keine Zustandsmaschiene
GLOBAL Void Number_Handler(Void) {
    if (tst_event(EVENT_15)) {
       clr_event(EVENT_15);
       digits[selected] += 1;
       if (TSTBIT(P2OUT, BIT7)) {
           digits[selected] -= 2;
       }
       set_event(EVENT_DIGI);
   }
}

// ----------------------------------------------------------------------------
// Datentyp eines konstanten Funktionspointers
typedef Void (* VoidFunc)(Void);

// Funktionsprototypen
LOCAL Void State0(Void);
LOCAL Void State1(Void);
LOCAL Void State2(Void);
LOCAL Void State3(Void);


// lokale Zustandsvariable
LOCAL VoidFunc state = State0;

LOCAL Void State0(Void) {
    if (tst_event(EVENT_DIGI)) {
        i = 1;
        state = State1;
    }
}

LOCAL Void State1(Void) {
    if (i <= 4) {
        UChar index = i - 1;
        if (digits[index] > BASE-1) {
            digits[index] = 0;
            digits[i]++;
        } else if (digits[index] < 0) {
            digits[index] = BASE-1;
            digits[i]--;
        }

        if (is_buffer_set EQ TRUE) {
            digits[index] = buffer[3 - index] - 0x30;
        }

        hex_digits[3 - index] = 0x30 + digits[index];

        Char ch = UCA1RXBUF;   // dummy read, UCRXIFG := 0, UCOE := 0
        CLRBIT(P2OUT,  BIT3);  // Select aktivieren
        UCA1TXBUF = i;       // Adresse ausgeben

        state = State2;
    } else {
        UCA0_printf(hex_digits);

        is_buffer_set = FALSE;
        clr_event(EVENT_DIGI);
        state = State0;
    }
}

LOCAL Void State2(Void) {
    if (NOT(TSTBIT(UCA1IFG, UCRXIFG) EQ 0)) {
        Char ch = UCA1RXBUF;        // dummy read
        UCA1TXBUF = digits[i-1];       // Datum ausgeben
        state = State3;
    }
}

LOCAL Void State3(Void) {
    if (NOT(TSTBIT(UCA1IFG, UCRXIFG) EQ 0)) {
        Char ch = UCA1RXBUF;        // dummy read
        SETBIT(P2OUT,  BIT3);  // Select deaktivieren
        i++;
        state = State1;
    }
}

// der AS1108_Hander beinhaltet eine Zustandsmaschine
GLOBAL Void AS1108_Handler(Void) {
    (*state)();
}
