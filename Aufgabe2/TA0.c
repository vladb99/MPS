/*
 * TA0.c
 *
 *  Created on: 22.04.2018
 *      Author: Admin
 */

#include <msp430.h>
#include "../base.h"
#include "TA0.h"
#include "event.h"
#include "GPIO.h"

#define CNTMAX 4
// 1/800Hz * 1000 = 1.25ms
// 250ms / 1.25ms = 200
#define BTNMAX 6
#define TIMER_COUNT 200

/*
 * Man soll sich eine geeignete Datenstruktur �berlegen,
 * die eine laufzeiteffiziente Ausf�hrung der ISR erm�glicht.
 */

typedef struct {
    const UChar * const port;
    const UChar mask;
    const TEvent msg;
    UInt * const pcnt; // Hysterese Wert,
} TButton;

LOCAL struct {
    UInt idx;
    UInt cnt[BTNMAX];
    const TButton *ptr;
} button;

LOCAL const TButton btn[BTNMAX] = {
    {(UChar *)(&P1IN), BIT0, EVENT_BTN1, &button.cnt[0]},
    {(UChar *)(&P1IN), BIT1, EVENT_BTN2, &button.cnt[1]},
    {(UChar *)(&P3IN), BIT0, EVENT_BTN3, &button.cnt[2]},
    {(UChar *)(&P3IN), BIT1, EVENT_BTN4, &button.cnt[3]},
    {(UChar *)(&P3IN), BIT2, EVENT_BTN5, &button.cnt[4]},
    {(UChar *)(&P3IN), BIT3, EVENT_BTN6, &button.cnt[5]}
};

// 10ms Schritte
LOCAL const UChar blink_musters[] = {
        8, 2, 0,
        3, 3, 0,
        1, 1, 0,
        2, 8, 0,
        2, 2, 2, 8, 0,
        2, 2, 2, 2, 2, 8, 0
};

LOCAL const UChar * const map[] = {
    &blink_musters[0],
    &blink_musters[3],
    &blink_musters[6],
    &blink_musters[9],
    &blink_musters[12],
    &blink_musters[17]
};

static UChar step_count;
static UChar array_index;
static UChar cnt_led;
static UChar pattern_index;
static UChar current_pattern_value;
static UChar pattern_index_new;
//static const struct Button btn1 = {P1IN, BIT1, EVENT_BTN1};

GLOBAL Void set_blink_muster(UInt arg) {
/*
 * Die Funktion muss so erweitert werden,
 * dass ein Blinkmuster selektiert wird.
 * Diese L�sung h�ngt stark von der gew�hlten
 * Datenstruktur ab.
 */
    pattern_index_new = arg;
}

// Der Timer A0 ist bereits initialisiert
GLOBAL Void TA0_Init(Void) {
    button.idx = 0;
    button.ptr = &btn[0];

    for (UChar i=0; i LE BTNMAX-1; i++) {
        button.cnt[i] = 0;
    }

    step_count = TIMER_COUNT;
    array_index = 0;
    cnt_led = 0;
    pattern_index = 0;
    current_pattern_value = 0;
    pattern_index_new = 0;

    //int *cheat_x = btn1->portid;
    //*cheat_x = P1IN;
    //btn1.portid = P1IN;

    // Folie 27
    TA0CCR0 = 0;                              // stopp Timer A
    CLRBIT(TA0CTL, TAIFG);                    // clear overflow flag
    CLRBIT(TA0CCR0, CCIFG);                   // clear CCI flag
    TA0CTL  = TASSEL__ACLK + MC__UP + ID__8;  // set up Timer A
    TA0EX0  = TAIDEX_7;                       // set up expansion register
    // 9600 / 800Hz = 12
    TA0CCR0 = 12;                           // set up CCR0 for 10 ms
    SETBIT(TA0CTL, TACLR);                    // clear and start Timer
    SETBIT(TA0CCTL0, CCIE);                   // enable Timer A interrupt

    // Set LED2 once
    SETBIT(P1OUT, BIT2);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt Void TA0_ISR(Void) {
    //static volatile UChar test = 0;
    // Timer logic for 250ms step
    if (--step_count EQ 0) {
        //test = *(*(P + pattern_index) + array_index);
        step_count = TIMER_COUNT;
        current_pattern_value = *(*(map + pattern_index) + array_index);
        cnt_led++;

        // LED2 Flash logic
        if (cnt_led EQ current_pattern_value) {
            TGLBIT(P1OUT, BIT2);
            array_index++;
            cnt_led = 0;
            if (*(*(map + pattern_index) + array_index) EQ 0) {
                array_index = 0;
                pattern_index = pattern_index_new;
            }
        }
    }

    // Debouncing
    register UInt * const pcnt = button.ptr->pcnt;
    if (TSTBIT(*button.ptr->port, button.ptr->mask)) {
        if (*pcnt LT CNTMAX) {
            if  (++(*pcnt) EQ CNTMAX) {
                set_event(button.ptr->msg);
                __low_power_mode_off_on_exit();
            }
        }
    } else {
        if (*pcnt GT 0) {
            (*pcnt)--;
        }
    }

    button.ptr++;
    button.idx++;
    if (button.idx GT BTNMAX-1) {
        button.idx = 0;
        button.ptr = &btn[0];
    }
}
