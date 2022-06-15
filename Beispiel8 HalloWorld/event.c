/*
 * event.c
 *
 *  Created on: 28.04.2018
 *      Author: Admin
 */

#include <msp430.h>
#include "..\base.h"
#include "event.h"

LOCAL volatile TEvent event  = NO_EVENTS;
LOCAL volatile TEvent errflg = NO_EVENTS;

GLOBAL Void wait_for_event(Void) {
   _disable_interrupt();
   if (event NE NO_EVENTS) {
      _enable_interrupt();
      return;
   }
   _low_power_mode_3(); // do nothing
}

#pragma FUNC_ALWAYS_INLINE(is_event_error)
GLOBAL Bool is_event_error(Void) {
   return (errflg NE NO_EVENTS);
}

#pragma FUNC_ALWAYS_INLINE(set_event)
GLOBAL Void set_event(TEvent arg) {
   errflg |= event BAND arg;
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
