// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Epson C33 shared helper functions and constants
*/
#ifndef MAME_CPU_C33_C33HELPERS_IPP
#define MAME_CPU_C33_C33HELPERS_IPP

#pragma once


enum
{
	PSR_BIT_N  =  0,
	PSR_BIT_Z  =  1,
	PSR_BIT_V  =  2,
	PSR_BIT_C  =  3,
	PSR_BIT_IE =  4,
	PSR_BIT_DS =  6,
	PSR_BIT_MO =  7,
	PSR_BIT_IL =  8,
	PSR_BIT_SV = 12,
	PSR_BIT_ME = 13,
	PSR_BIT_DE = 14,
	PSR_BIT_S  = 15,
	PSR_BIT_HC = 16,
	PSR_BIT_LC = 17,
	PSR_BIT_SE = 20,
	PSR_BIT_OC = 21,
	PSR_BIT_SW = 22,
	PSR_BIT_RC = 24,
	PSR_BIT_PM = 28,
	PSR_BIT_LM = 29,
	PSR_BIT_RM = 30,
	PSR_BIT_HE = 31
};

enum : u32
{
	PSR_MASK_N  = u32(0x1) << PSR_BIT_N,
	PSR_MASK_Z  = u32(0x1) << PSR_BIT_Z,
	PSR_MASK_V  = u32(0x1) << PSR_BIT_V,
	PSR_MASK_C  = u32(0x1) << PSR_BIT_C,
	PSR_MASK_IE = u32(0x1) << PSR_BIT_IE,
	PSR_MASK_DS = u32(0x1) << PSR_BIT_DS,
	PSR_MASK_MO = u32(0x1) << PSR_BIT_MO,
	PSR_MASK_IL = u32(0xf) << PSR_BIT_IL,
	PSR_MASK_SV = u32(0x1) << PSR_BIT_SV,
	PSR_MASK_ME = u32(0x1) << PSR_BIT_ME,
	PSR_MASK_DE = u32(0x1) << PSR_BIT_DE,
	PSR_MASK_S  = u32(0x1) << PSR_BIT_S,
	PSR_MASK_HC = u32(0x1) << PSR_BIT_HC,
	PSR_MASK_LC = u32(0x1) << PSR_BIT_LC,
	PSR_MASK_SE = u32(0x1) << PSR_BIT_SE,
	PSR_MASK_OC = u32(0x1) << PSR_BIT_OC,
	PSR_MASK_SW = u32(0x1) << PSR_BIT_SW,
	PSR_MASK_RC = u32(0xf) << PSR_BIT_RC,
	PSR_MASK_PM = u32(0x1) << PSR_BIT_PM,
	PSR_MASK_LM = u32(0x1) << PSR_BIT_LM,
	PSR_MASK_RM = u32(0x1) << PSR_BIT_RM,
	PSR_MASK_HE = u32(0x1) << PSR_BIT_HE
};

#endif // MAME_CPU_C33_C33HELPERS_IPP
