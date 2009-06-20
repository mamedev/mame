/*
    Atmel 8-bit AVR simulator

    (Skeleton)

    Written by MooglyGuy
*/

#pragma once

#ifndef __AVR8_H__
#define __AVR8_H__

enum
{
    AVR8_SREG = 1,
    AVR8_PC,
    AVR8_R0,
    AVR8_R1,
    AVR8_R2,
    AVR8_R3,
    AVR8_R4,
    AVR8_R5,
    AVR8_R6,
    AVR8_R7,
    AVR8_R8,
    AVR8_R9,
    AVR8_R10,
    AVR8_R11,
    AVR8_R12,
    AVR8_R13,
    AVR8_R14,
    AVR8_R15,
    AVR8_R16,
    AVR8_R17,
    AVR8_R18,
    AVR8_R19,
    AVR8_R20,
    AVR8_R21,
    AVR8_R22,
    AVR8_R23,
    AVR8_R24,
    AVR8_R25,
    AVR8_R26,
    AVR8_R27,
    AVR8_R28,
    AVR8_R29,
    AVR8_R30,
    AVR8_R31,
    AVR8_X,
    AVR8_Y,
    AVR8_Z,
};

CPU_GET_INFO( avr8 );
#define CPU_AVR8 CPU_GET_INFO_NAME( avr8 )

CPU_DISASSEMBLE( avr8 );

#endif /* __AVR8_H__ */
