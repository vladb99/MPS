/*
 * event.c
 *
 *  Created on: 28.04.2018
 *      Author: Admin
 */

#include <msp430.h>
#include "../base.h"
#include "event.h"

LOCAL volatile TEvent event  = NO_EVENTS;
LOCAL volatile TEvent errflg = NO_EVENTS;

/*
 * Um die Auslastung der Mikrokontrollers messen zu k�nnen,
 * ben�tugt man zwei Zeiten: die Dauer von Interrupt-Service-Routinen
 * und die Zeit, die die CPU in der while-Schleife der Fuunktion main
 * verbringt. Diese Zeiten kann man mit einem Osziloskopen messen,
 * wenn man am Eval-Board zwei Messpunkte (z.B. Pins) hat.
 */
GLOBAL Void init_event(Void) {
   // set up Pin 4 at Port 3 => output, main time
   CLRBIT(P3OUT,  BIT4);
   SETBIT(P3DIR,  BIT4);
   CLRBIT(P3SEL0, BIT4);
   CLRBIT(P3SEL1, BIT4);
   CLRBIT(P3REN,  BIT4);

   // set up Pin 5 at Port 3 => output, ISR time
   CLRBIT(P3OUT,  BIT5);
   SETBIT(P3DIR,  BIT5);
   CLRBIT(P3SEL0, BIT5);
   CLRBIT(P3SEL1, BIT5);
   CLRBIT(P3REN,  BIT5);
}

GLOBAL Void wait_for_event(Void) {
   _disable_interrupt();
   if (event EQ NO_EVENTS) {
      CLRBIT(P3OUT,  BIT4);
      _low_power_mode_3();
      SETBIT(P3OUT,  BIT4);
   }
   _enable_interrupt();
}

#pragma FUNC_ALWAYS_INLINE(is_event_error)
GLOBAL Bool is_event_error(Void) {
   return (errflg NE NO_EVENTS);
}

#pragma FUNC_ALWAYS_INLINE(set_event)
GLOBAL Void set_event(TEvent arg) {
   errflg  |= event BAND arg;
   TGLBIT(event, arg);
}

#pragma FUNC_ALWAYS_INLINE(clr_event)
GLOBAL Void clr_event(TEvent arg) {
   TGLBIT(event, arg);
}

#pragma FUNC_ALWAYS_INLINE(tst_event)
GLOBAL Bool tst_event(TEvent arg) {
   return TSTBIT(event, arg);
}
