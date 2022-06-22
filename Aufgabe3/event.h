/*
 * event.h
 *
 *  Created on: 28.04.2018
 *      Author: Admin
 */

#include "../base.h"

// Ein Ereignis (Event) ist eine �nderung einer (physikalischen) Gr��e,
// die f�r das System relevant ist und auf die das System reagieren sollte.

#ifndef EVENT_H_
#define EVENT_H_

typedef unsigned int TEvent;

#define NO_EVENTS   0x0000
#define EVENT_1     0x0001
#define EVENT_2     0x0002
#define EVENT_3     0x0004
#define EVENT_4     0x0008
#define EVENT_5     0x0010
#define EVENT_6     0x0020
#define EVENT_7     0x0040
#define EVENT_8     0x0080
#define EVENT_9     0x0100
#define EVENT_10    0x0200
#define EVENT_11    0x0400
#define EVENT_12    0x0800
#define EVENT_13    0x1000
#define EVENT_14    0x2000
#define EVENT_15    0x4000
#define EVENT_16    0x8000
#define ALL_EVENTS  0xFFFF

#define EVENT_IMA   EVENT_1  // I am alive
#define EVENT_BTN1  EVENT_2  // click on button 1
#define EVENT_BTN2  EVENT_3  // click on button 2
#define EVENT_BTN3  EVENT_4  // click on button 3
#define EVENT_BTN4  EVENT_5  // click on button 4
#define EVENT_BTN5  EVENT_6  // click on button 5
#define EVENT_BTN6  EVENT_7  // click on button 6
#define EVENT_DIGI  EVENT_8  // update digital
#define EVENT_7LED  EVENT_9  // update LEDs
#define EVENT_SHOWTERM  EVENT_10  // show digits in terminal

EXTERN Void wait_for_event(Void);
EXTERN Bool is_event_error(Void);
EXTERN Void set_event(TEvent);
EXTERN Void clr_event(TEvent);
EXTERN Bool tst_event(TEvent);

#endif /* EVENT_H_ */
