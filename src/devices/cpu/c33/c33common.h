// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Common Epson C33 family stuff
*/

#ifndef MAME_CPU_C33_C33COMMON_H
#define MAME_CPU_C33_C33COMMON_H

#pragma once


class c33_cpu_common
{
public:
	// state indices
	enum
	{
		C33_R0 = 1, C33_R1, C33_R2, C33_R3, C33_R4, C33_R5, C33_R6, C33_R7,
		C33_R8, C33_R9, C33_R10, C33_R11, C33_R12, C33_R13, C33_R14, C33_R15,
		C33_PSR, C33_SP,
		C33_ALR, C33_AHR,
		C33_LCO, C33_LSA, C33_LEA,
		C33_SOR, C33_TTBR, C33_DP, C33_DBBR,
		C33_USP, C33_SSP
	};
};

#endif // MAME_CPU_C33_C33COMMON_H
