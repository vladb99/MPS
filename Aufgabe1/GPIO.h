/*
 * GPIO.h
 *
 *  Created on: 03.06.2019
 *      Author: Admin
 */

#ifndef GPIO_H_
#define GPIO_H_

#define GPIO(base, offset)  ((UChar *)((base) + (offset)))

#define PORT1    0x0200
#define PORT2    0x0201
#define PORT3    0x0220
#define PORT4    0x0221

#define PxIN     0x00
#define PxOUT    0x02
#define PxDIR    0x04
#define PxREN    0x06
#define PxSEL0   0x0A
#define PxSEL1   0x0C

#endif /* GPIO_H_ */
