/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright (C) Alex Pasadyn/Zsolt Vasvari 1998
    Parts based on code by Aaron Giles

***************************************************************************/

#include "debugger.h"
#include "tms34010.h"
#include "34010ops.h"


/***************************************************************************
    FIELD WRITE FUNCTIONS
***************************************************************************/

static void wfield_01(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x01,16);
}

static void wfield_02(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x03,15);
}

static void wfield_03(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x07,14);
}

static void wfield_04(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x0f,13);
}

static void wfield_05(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x1f,12);
}

static void wfield_06(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x3f,11);
}

static void wfield_07(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x7f,10);
}

static void wfield_08(offs_t offset,UINT32 data)
{
	WFIELDMAC_8;
}

static void wfield_09(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x1ff,8);
}

static void wfield_10(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x3ff,7);
}

static void wfield_11(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x7ff,6);
}

static void wfield_12(offs_t offset,UINT32 data)
{
	WFIELDMAC(0xfff,5);
}

static void wfield_13(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x1fff,4);
}

static void wfield_14(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x3fff,3);
}

static void wfield_15(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x7fff,2);
}

static void wfield_16(offs_t offset,UINT32 data)
{
	if (offset & 0x0f)
	{
		WFIELDMAC(0xffff,1);
	}
	else
	{
		TMS34010_WRMEM_WORD(TOBYTE(offset),data);
	}
}

static void wfield_17(offs_t offset,UINT32 data)
{
	WFIELDMAC(0x1ffff,0);
}

static void wfield_18(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x3ffff,15);
}

static void wfield_19(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x7ffff,14);
}

static void wfield_20(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0xfffff,13);
}

static void wfield_21(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x1fffff,12);
}

static void wfield_22(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x3fffff,11);
}

static void wfield_23(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x7fffff,10);
}

static void wfield_24(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0xffffff,9);
}

static void wfield_25(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x1ffffff,8);
}

static void wfield_26(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x3ffffff,7);
}

static void wfield_27(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x7ffffff,6);
}

static void wfield_28(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0xfffffff,5);
}

static void wfield_29(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x1fffffff,4);
}

static void wfield_30(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x3fffffff,3);
}

static void wfield_31(offs_t offset,UINT32 data)
{
	WFIELDMAC_BIG(0x7fffffff,2);
}

static void wfield_32(offs_t offset,UINT32 data)
{
	WFIELDMAC_32;
}


void (*tms34010_wfield_functions[32])(offs_t offset,UINT32 data) =
{
	wfield_32, wfield_01, wfield_02, wfield_03, wfield_04, wfield_05,
	wfield_06, wfield_07, wfield_08, wfield_09, wfield_10, wfield_11,
	wfield_12, wfield_13, wfield_14, wfield_15, wfield_16, wfield_17,
	wfield_18, wfield_19, wfield_20, wfield_21, wfield_22, wfield_23,
	wfield_24, wfield_25, wfield_26, wfield_27, wfield_28, wfield_29,
	wfield_30, wfield_31
};



/***************************************************************************
    FIELD READ FUNCTIONS (ZERO-EXTEND)
***************************************************************************/

static UINT32 rfield_z_01(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x01,16);
	return ret;
}

static UINT32 rfield_z_02(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x03,15);
	return ret;
}

static UINT32 rfield_z_03(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x07,14);
	return ret;
}

static UINT32 rfield_z_04(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x0f,13);
	return ret;
}

static UINT32 rfield_z_05(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1f,12);
	return ret;
}

static UINT32 rfield_z_06(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3f,11);
	return ret;
}

static UINT32 rfield_z_07(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7f,10);
	return ret;
}

static UINT32 rfield_z_08(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_8;
	return ret;
}

static UINT32 rfield_z_09(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ff,8);
	return ret;
}

static UINT32 rfield_z_10(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3ff,7);
	return ret;
}

static UINT32 rfield_z_11(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7ff,6);
	return ret;
}

static UINT32 rfield_z_12(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0xfff,5);
	return ret;
}

static UINT32 rfield_z_13(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1fff,4);
	return ret;
}

static UINT32 rfield_z_14(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3fff,3);
	return ret;
}

static UINT32 rfield_z_15(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7fff,2);
	return ret;
}

static UINT32 rfield_z_16(offs_t offset)
{
	UINT32 ret;
	if (offset & 0x0f)
	{
		RFIELDMAC(0xffff,1);
	}

	else
		ret = TMS34010_RDMEM_WORD(TOBYTE(offset));
	return ret;
}

static UINT32 rfield_z_17(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ffff,0);
	return ret;
}

static UINT32 rfield_z_18(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffff,15);
	return ret;
}

static UINT32 rfield_z_19(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffff,14);
	return ret;
}

static UINT32 rfield_z_20(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffff,13);
	return ret;
}

static UINT32 rfield_z_21(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffff,12);
	return ret;
}

static UINT32 rfield_z_22(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffff,11);
	return ret;
}

static UINT32 rfield_z_23(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffff,10);
	return ret;
}

static UINT32 rfield_z_24(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xffffff,9);
	return ret;
}

static UINT32 rfield_z_25(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1ffffff,8);
	return ret;
}

static UINT32 rfield_z_26(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffffff,7);
	return ret;
}

static UINT32 rfield_z_27(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffffff,6);
	return ret;
}

static UINT32 rfield_z_28(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffffff,5);
	return ret;
}

static UINT32 rfield_z_29(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffffff,4);
	return ret;
}

static UINT32 rfield_z_30(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffffff,3);
	return ret;
}

static UINT32 rfield_z_31(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffffff,2);
	return ret;
}

static UINT32 rfield_32(offs_t offset)
{
	RFIELDMAC_32;
}


/***************************************************************************
    FIELD READ FUNCTIONS (SIGN-EXTEND)
***************************************************************************/

static UINT32 rfield_s_01(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x01,16);
	return ((INT32)(ret << 31)) >> 31;
}

static UINT32 rfield_s_02(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x03,15);
	return ((INT32)(ret << 30)) >> 30;
}

static UINT32 rfield_s_03(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x07,14);
	return ((INT32)(ret << 29)) >> 29;
}

static UINT32 rfield_s_04(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x0f,13);
	return ((INT32)(ret << 28)) >> 28;
}

static UINT32 rfield_s_05(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1f,12);
	return ((INT32)(ret << 27)) >> 27;
}

static UINT32 rfield_s_06(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3f,11);
	return ((INT32)(ret << 26)) >> 26;
}

static UINT32 rfield_s_07(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7f,10);
	return ((INT32)(ret << 25)) >> 25;
}

static UINT32 rfield_s_08(offs_t offset)
{
	UINT32 ret;
	if (offset & 0x07)
	{
		RFIELDMAC(0xff,9);
	}

	else
		ret = TMS34010_RDMEM(TOBYTE(offset));
	return (INT32)(INT8)ret;
}

static UINT32 rfield_s_09(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ff,8);
	return ((INT32)(ret << 23)) >> 23;
}

static UINT32 rfield_s_10(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3ff,7);
	return ((INT32)(ret << 22)) >> 22;
}

static UINT32 rfield_s_11(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7ff,6);
	return ((INT32)(ret << 21)) >> 21;
}

static UINT32 rfield_s_12(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0xfff,5);
	return ((INT32)(ret << 20)) >> 20;
}

static UINT32 rfield_s_13(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1fff,4);
	return ((INT32)(ret << 19)) >> 19;
}

static UINT32 rfield_s_14(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3fff,3);
	return ((INT32)(ret << 18)) >> 18;
}

static UINT32 rfield_s_15(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7fff,2);
	return ((INT32)(ret << 17)) >> 17;
}

static UINT32 rfield_s_16(offs_t offset)
{
	UINT32 ret;
	if (offset & 0x0f)
	{
		RFIELDMAC(0xffff,1);
	}

	else
	{
		ret = TMS34010_RDMEM_WORD(TOBYTE(offset));
	}

	return (INT32)(INT16)ret;
}

static UINT32 rfield_s_17(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ffff,0);
	return ((INT32)(ret << 15)) >> 15;
}

static UINT32 rfield_s_18(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffff,15);
	return ((INT32)(ret << 14)) >> 14;
}

static UINT32 rfield_s_19(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffff,14);
	return ((INT32)(ret << 13)) >> 13;
}

static UINT32 rfield_s_20(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffff,13);
	return ((INT32)(ret << 12)) >> 12;
}

static UINT32 rfield_s_21(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffff,12);
	return ((INT32)(ret << 11)) >> 11;
}

static UINT32 rfield_s_22(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffff,11);
	return ((INT32)(ret << 10)) >> 10;
}

static UINT32 rfield_s_23(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffff,10);
	return ((INT32)(ret << 9)) >> 9;
}

static UINT32 rfield_s_24(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xffffff,9);
	return ((INT32)(ret << 8)) >> 8;
}

static UINT32 rfield_s_25(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1ffffff,8);
	return ((INT32)(ret << 7)) >> 7;
}

static UINT32 rfield_s_26(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffffff,7);
	return ((INT32)(ret << 6)) >> 6;
}

static UINT32 rfield_s_27(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffffff,6);
	return ((INT32)(ret << 5)) >> 5;
}

static UINT32 rfield_s_28(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffffff,5);
	return ((INT32)(ret << 4)) >> 4;
}

static UINT32 rfield_s_29(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffffff,4);
	return ((INT32)(ret << 3)) >> 3;
}

static UINT32 rfield_s_30(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffffff,3);
	return ((INT32)(ret << 2)) >> 2;
}

static UINT32 rfield_s_31(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffffff,2);
	return ((INT32)(ret << 1)) >> 1;
}

UINT32 (*tms34010_rfield_functions[64])(offs_t offset) =
{
	rfield_32  , rfield_z_01, rfield_z_02, rfield_z_03, rfield_z_04, rfield_z_05,
	rfield_z_06, rfield_z_07, rfield_z_08, rfield_z_09, rfield_z_10, rfield_z_11,
	rfield_z_12, rfield_z_13, rfield_z_14, rfield_z_15, rfield_z_16, rfield_z_17,
	rfield_z_18, rfield_z_19, rfield_z_20, rfield_z_21, rfield_z_22, rfield_z_23,
	rfield_z_24, rfield_z_25, rfield_z_26, rfield_z_27, rfield_z_28, rfield_z_29,
	rfield_z_30, rfield_z_31,
	rfield_32  , rfield_s_01, rfield_s_02, rfield_s_03, rfield_s_04, rfield_s_05,
	rfield_s_06, rfield_s_07, rfield_s_08, rfield_s_09, rfield_s_10, rfield_s_11,
	rfield_s_12, rfield_s_13, rfield_s_14, rfield_s_15, rfield_s_16, rfield_s_17,
	rfield_s_18, rfield_s_19, rfield_s_20, rfield_s_21, rfield_s_22, rfield_s_23,
	rfield_s_24, rfield_s_25, rfield_s_26, rfield_s_27, rfield_s_28, rfield_s_29,
	rfield_s_30, rfield_s_31
};

