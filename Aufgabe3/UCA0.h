/*
 * UCA0.h
 *
 *  Created on: 10.04.2018
 *      Author: Admin
 */

#include "../base.h"
#include "TA0.h"
#include "AS1108.h"

#ifndef UCA0_H_
#define UCA0_H_

#define ENTER 0x0D
#define ZERO 0x30
#define NINE 0x39
#define BR_ERROR 0
#define FOP_ERROR 1
#define C_ERROR 2
#define BU_ERROR 3
#define BY_RX 4
#define NO_ERROR 4

EXTERN Void UCA0_Init(Void);
EXTERN Int UCA0_printf(const Char * str);
EXTERN Void set_error_code(Char code);

EXTERN signed char buffer[4];
EXTERN Char error_code;
EXTERN Bool is_buffer_set;

#endif /* UCA0_H_ */
