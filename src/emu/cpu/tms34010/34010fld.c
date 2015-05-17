// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright Alex Pasadyn/Zsolt Vasvari
    Parts based on code by Aaron Giles

***************************************************************************/



/***************************************************************************
    FIELD WRITE FUNCTIONS
***************************************************************************/

void tms340x0_device::wfield_01(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x01,16);
}

void tms340x0_device::wfield_02(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x03,15);
}

void tms340x0_device::wfield_03(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x07,14);
}

void tms340x0_device::wfield_04(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x0f,13);
}

void tms340x0_device::wfield_05(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x1f,12);
}

void tms340x0_device::wfield_06(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x3f,11);
}

void tms340x0_device::wfield_07(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x7f,10);
}

void tms340x0_device::wfield_08(offs_t offset, UINT32 data)
{
	WFIELDMAC_8();
}

void tms340x0_device::wfield_09(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x1ff,8);
}

void tms340x0_device::wfield_10(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x3ff,7);
}

void tms340x0_device::wfield_11(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x7ff,6);
}

void tms340x0_device::wfield_12(offs_t offset, UINT32 data)
{
	WFIELDMAC(0xfff,5);
}

void tms340x0_device::wfield_13(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x1fff,4);
}

void tms340x0_device::wfield_14(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x3fff,3);
}

void tms340x0_device::wfield_15(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x7fff,2);
}

void tms340x0_device::wfield_16(offs_t offset, UINT32 data)
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

void tms340x0_device::wfield_17(offs_t offset, UINT32 data)
{
	WFIELDMAC(0x1ffff,0);
}

void tms340x0_device::wfield_18(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x3ffff,15);
}

void tms340x0_device::wfield_19(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x7ffff,14);
}

void tms340x0_device::wfield_20(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0xfffff,13);
}

void tms340x0_device::wfield_21(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x1fffff,12);
}

void tms340x0_device::wfield_22(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x3fffff,11);
}

void tms340x0_device::wfield_23(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x7fffff,10);
}

void tms340x0_device::wfield_24(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0xffffff,9);
}

void tms340x0_device::wfield_25(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x1ffffff,8);
}

void tms340x0_device::wfield_26(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x3ffffff,7);
}

void tms340x0_device::wfield_27(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x7ffffff,6);
}

void tms340x0_device::wfield_28(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0xfffffff,5);
}

void tms340x0_device::wfield_29(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x1fffffff,4);
}

void tms340x0_device::wfield_30(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x3fffffff,3);
}

void tms340x0_device::wfield_31(offs_t offset, UINT32 data)
{
	WFIELDMAC_BIG(0x7fffffff,2);
}

void tms340x0_device::wfield_32(offs_t offset, UINT32 data)
{
	WFIELDMAC_32();
}


const tms340x0_device::wfield_func tms340x0_device::s_wfield_functions[32] =
{
	&tms340x0_device::wfield_32, &tms340x0_device::wfield_01, &tms340x0_device::wfield_02, &tms340x0_device::wfield_03, &tms340x0_device::wfield_04, &tms340x0_device::wfield_05,
	&tms340x0_device::wfield_06, &tms340x0_device::wfield_07, &tms340x0_device::wfield_08, &tms340x0_device::wfield_09, &tms340x0_device::wfield_10, &tms340x0_device::wfield_11,
	&tms340x0_device::wfield_12, &tms340x0_device::wfield_13, &tms340x0_device::wfield_14, &tms340x0_device::wfield_15, &tms340x0_device::wfield_16, &tms340x0_device::wfield_17,
	&tms340x0_device::wfield_18, &tms340x0_device::wfield_19, &tms340x0_device::wfield_20, &tms340x0_device::wfield_21, &tms340x0_device::wfield_22, &tms340x0_device::wfield_23,
	&tms340x0_device::wfield_24, &tms340x0_device::wfield_25, &tms340x0_device::wfield_26, &tms340x0_device::wfield_27, &tms340x0_device::wfield_28, &tms340x0_device::wfield_29,
	&tms340x0_device::wfield_30, &tms340x0_device::wfield_31
};



/***************************************************************************
    FIELD READ FUNCTIONS (ZERO-EXTEND)
***************************************************************************/

UINT32 tms340x0_device::rfield_z_01(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x01,16);
	return ret;
}

UINT32 tms340x0_device::rfield_z_02(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x03,15);
	return ret;
}

UINT32 tms340x0_device::rfield_z_03(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x07,14);
	return ret;
}

UINT32 tms340x0_device::rfield_z_04(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x0f,13);
	return ret;
}

UINT32 tms340x0_device::rfield_z_05(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1f,12);
	return ret;
}

UINT32 tms340x0_device::rfield_z_06(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3f,11);
	return ret;
}

UINT32 tms340x0_device::rfield_z_07(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7f,10);
	return ret;
}

UINT32 tms340x0_device::rfield_z_08(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_8();
	return ret;
}

UINT32 tms340x0_device::rfield_z_09(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ff,8);
	return ret;
}

UINT32 tms340x0_device::rfield_z_10(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3ff,7);
	return ret;
}

UINT32 tms340x0_device::rfield_z_11(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7ff,6);
	return ret;
}

UINT32 tms340x0_device::rfield_z_12(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0xfff,5);
	return ret;
}

UINT32 tms340x0_device::rfield_z_13(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1fff,4);
	return ret;
}

UINT32 tms340x0_device::rfield_z_14(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3fff,3);
	return ret;
}

UINT32 tms340x0_device::rfield_z_15(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7fff,2);
	return ret;
}

UINT32 tms340x0_device::rfield_z_16(offs_t offset)
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

UINT32 tms340x0_device::rfield_z_17(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ffff,0);
	return ret;
}

UINT32 tms340x0_device::rfield_z_18(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffff,15);
	return ret;
}

UINT32 tms340x0_device::rfield_z_19(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffff,14);
	return ret;
}

UINT32 tms340x0_device::rfield_z_20(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffff,13);
	return ret;
}

UINT32 tms340x0_device::rfield_z_21(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffff,12);
	return ret;
}

UINT32 tms340x0_device::rfield_z_22(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffff,11);
	return ret;
}

UINT32 tms340x0_device::rfield_z_23(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffff,10);
	return ret;
}

UINT32 tms340x0_device::rfield_z_24(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xffffff,9);
	return ret;
}

UINT32 tms340x0_device::rfield_z_25(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1ffffff,8);
	return ret;
}

UINT32 tms340x0_device::rfield_z_26(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffffff,7);
	return ret;
}

UINT32 tms340x0_device::rfield_z_27(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffffff,6);
	return ret;
}

UINT32 tms340x0_device::rfield_z_28(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffffff,5);
	return ret;
}

UINT32 tms340x0_device::rfield_z_29(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffffff,4);
	return ret;
}

UINT32 tms340x0_device::rfield_z_30(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffffff,3);
	return ret;
}

UINT32 tms340x0_device::rfield_z_31(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffffff,2);
	return ret;
}

UINT32 tms340x0_device::rfield_32(offs_t offset)
{
	RFIELDMAC_32();
}


/***************************************************************************
    FIELD READ FUNCTIONS (SIGN-EXTEND)
***************************************************************************/

UINT32 tms340x0_device::rfield_s_01(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x01,16);
	return ((INT32)(ret << 31)) >> 31;
}

UINT32 tms340x0_device::rfield_s_02(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x03,15);
	return ((INT32)(ret << 30)) >> 30;
}

UINT32 tms340x0_device::rfield_s_03(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x07,14);
	return ((INT32)(ret << 29)) >> 29;
}

UINT32 tms340x0_device::rfield_s_04(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x0f,13);
	return ((INT32)(ret << 28)) >> 28;
}

UINT32 tms340x0_device::rfield_s_05(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1f,12);
	return ((INT32)(ret << 27)) >> 27;
}

UINT32 tms340x0_device::rfield_s_06(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3f,11);
	return ((INT32)(ret << 26)) >> 26;
}

UINT32 tms340x0_device::rfield_s_07(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7f,10);
	return ((INT32)(ret << 25)) >> 25;
}

UINT32 tms340x0_device::rfield_s_08(offs_t offset)
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

UINT32 tms340x0_device::rfield_s_09(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ff,8);
	return ((INT32)(ret << 23)) >> 23;
}

UINT32 tms340x0_device::rfield_s_10(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3ff,7);
	return ((INT32)(ret << 22)) >> 22;
}

UINT32 tms340x0_device::rfield_s_11(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7ff,6);
	return ((INT32)(ret << 21)) >> 21;
}

UINT32 tms340x0_device::rfield_s_12(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0xfff,5);
	return ((INT32)(ret << 20)) >> 20;
}

UINT32 tms340x0_device::rfield_s_13(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1fff,4);
	return ((INT32)(ret << 19)) >> 19;
}

UINT32 tms340x0_device::rfield_s_14(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x3fff,3);
	return ((INT32)(ret << 18)) >> 18;
}

UINT32 tms340x0_device::rfield_s_15(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x7fff,2);
	return ((INT32)(ret << 17)) >> 17;
}

UINT32 tms340x0_device::rfield_s_16(offs_t offset)
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

UINT32 tms340x0_device::rfield_s_17(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC(0x1ffff,0);
	return ((INT32)(ret << 15)) >> 15;
}

UINT32 tms340x0_device::rfield_s_18(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffff,15);
	return ((INT32)(ret << 14)) >> 14;
}

UINT32 tms340x0_device::rfield_s_19(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffff,14);
	return ((INT32)(ret << 13)) >> 13;
}

UINT32 tms340x0_device::rfield_s_20(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffff,13);
	return ((INT32)(ret << 12)) >> 12;
}

UINT32 tms340x0_device::rfield_s_21(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffff,12);
	return ((INT32)(ret << 11)) >> 11;
}

UINT32 tms340x0_device::rfield_s_22(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffff,11);
	return ((INT32)(ret << 10)) >> 10;
}

UINT32 tms340x0_device::rfield_s_23(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffff,10);
	return ((INT32)(ret << 9)) >> 9;
}

UINT32 tms340x0_device::rfield_s_24(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xffffff,9);
	return ((INT32)(ret << 8)) >> 8;
}

UINT32 tms340x0_device::rfield_s_25(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1ffffff,8);
	return ((INT32)(ret << 7)) >> 7;
}

UINT32 tms340x0_device::rfield_s_26(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3ffffff,7);
	return ((INT32)(ret << 6)) >> 6;
}

UINT32 tms340x0_device::rfield_s_27(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7ffffff,6);
	return ((INT32)(ret << 5)) >> 5;
}

UINT32 tms340x0_device::rfield_s_28(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0xfffffff,5);
	return ((INT32)(ret << 4)) >> 4;
}

UINT32 tms340x0_device::rfield_s_29(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x1fffffff,4);
	return ((INT32)(ret << 3)) >> 3;
}

UINT32 tms340x0_device::rfield_s_30(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x3fffffff,3);
	return ((INT32)(ret << 2)) >> 2;
}

UINT32 tms340x0_device::rfield_s_31(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_BIG(0x7fffffff,2);
	return ((INT32)(ret << 1)) >> 1;
}

const tms340x0_device::rfield_func tms340x0_device::s_rfield_functions[64] =
{
	&tms340x0_device::rfield_32  , &tms340x0_device::rfield_z_01, &tms340x0_device::rfield_z_02, &tms340x0_device::rfield_z_03, &tms340x0_device::rfield_z_04, &tms340x0_device::rfield_z_05,
	&tms340x0_device::rfield_z_06, &tms340x0_device::rfield_z_07, &tms340x0_device::rfield_z_08, &tms340x0_device::rfield_z_09, &tms340x0_device::rfield_z_10, &tms340x0_device::rfield_z_11,
	&tms340x0_device::rfield_z_12, &tms340x0_device::rfield_z_13, &tms340x0_device::rfield_z_14, &tms340x0_device::rfield_z_15, &tms340x0_device::rfield_z_16, &tms340x0_device::rfield_z_17,
	&tms340x0_device::rfield_z_18, &tms340x0_device::rfield_z_19, &tms340x0_device::rfield_z_20, &tms340x0_device::rfield_z_21, &tms340x0_device::rfield_z_22, &tms340x0_device::rfield_z_23,
	&tms340x0_device::rfield_z_24, &tms340x0_device::rfield_z_25, &tms340x0_device::rfield_z_26, &tms340x0_device::rfield_z_27, &tms340x0_device::rfield_z_28, &tms340x0_device::rfield_z_29,
	&tms340x0_device::rfield_z_30, &tms340x0_device::rfield_z_31,
	&tms340x0_device::rfield_32  , &tms340x0_device::rfield_s_01, &tms340x0_device::rfield_s_02, &tms340x0_device::rfield_s_03, &tms340x0_device::rfield_s_04, &tms340x0_device::rfield_s_05,
	&tms340x0_device::rfield_s_06, &tms340x0_device::rfield_s_07, &tms340x0_device::rfield_s_08, &tms340x0_device::rfield_s_09, &tms340x0_device::rfield_s_10, &tms340x0_device::rfield_s_11,
	&tms340x0_device::rfield_s_12, &tms340x0_device::rfield_s_13, &tms340x0_device::rfield_s_14, &tms340x0_device::rfield_s_15, &tms340x0_device::rfield_s_16, &tms340x0_device::rfield_s_17,
	&tms340x0_device::rfield_s_18, &tms340x0_device::rfield_s_19, &tms340x0_device::rfield_s_20, &tms340x0_device::rfield_s_21, &tms340x0_device::rfield_s_22, &tms340x0_device::rfield_s_23,
	&tms340x0_device::rfield_s_24, &tms340x0_device::rfield_s_25, &tms340x0_device::rfield_s_26, &tms340x0_device::rfield_s_27, &tms340x0_device::rfield_s_28, &tms340x0_device::rfield_s_29,
	&tms340x0_device::rfield_s_30, &tms340x0_device::rfield_s_31
};
