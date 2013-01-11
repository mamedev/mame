	/**************************************************************************\
	*                      Alpha8201 Emulator                                  *
	*                                                                          *
	*                    Copyright Tatsuyuki Satoh                             *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	\**************************************************************************/

#pragma once

#ifndef __ALPH8201_H__
#define __ALPH8201_H__

enum
{
	ALPHA8201_PC=1,
	ALPHA8201_SP,
	ALPHA8201_RB,
	ALPHA8201_MB,
//
	ALPHA8201_CF,
	ALPHA8201_ZF,
//
	ALPHA8201_IX0,
	ALPHA8201_IX1,
	ALPHA8201_IX2,
	ALPHA8201_LP0,
	ALPHA8201_LP1,
	ALPHA8201_LP2,
	ALPHA8201_A,
	ALPHA8201_B,
//
	ALPHA8201_R0,ALPHA8201_R1,ALPHA8201_R2,ALPHA8201_R3,
	ALPHA8201_R4,ALPHA8201_R5,ALPHA8201_R6,ALPHA8201_R7
};

DECLARE_LEGACY_CPU_DEVICE(ALPHA8201, alpha8201);
DECLARE_LEGACY_CPU_DEVICE(ALPHA8301, alpha8301);

CPU_DISASSEMBLE( alpha8201 );

#endif  /* __ALPH8201_H__ */
