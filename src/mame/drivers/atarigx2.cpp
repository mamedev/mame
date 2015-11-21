// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari GX2 hardware

    driver by Aaron Giles

    Games supported:
        * Space Lords (1992)
        * Moto Frenzy (1992)
        * Road Riot's Revenge Rally (1993)

    Known bugs:
        * protection devices unknown

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/atarirle.h"
#include "includes/atarigx2.h"



/*************************************
 *
 *  Initialization
 *
 *************************************/

void atarigx2_state::update_interrupts()
{
	m_maincpu->set_input_line(4, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(5, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_RESET_MEMBER(atarigx2_state,atarigx2)
{
	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 8);
}



/*************************************
 *
 *  I/O read dispatch.
 *
 *************************************/

READ32_MEMBER(atarigx2_state::special_port2_r)
{
	int temp = ioport("SERVICE")->read();
	temp ^= 0x0008;     /* A2D.EOC always high for now */
	return (temp << 16) | temp;
}


READ32_MEMBER(atarigx2_state::special_port3_r)
{
	int temp = ioport("SPECIAL")->read();
	return (temp << 16) | temp;
}



READ32_MEMBER(atarigx2_state::a2d_data_r)
{
	/* otherwise, assume it's hydra */
	switch (offset)
	{
		case 0:
			return (ioport("A2D0")->read() << 24) | (ioport("A2D1")->read() << 8);
		case 1:
			return (ioport("A2D2")->read() << 24) | (ioport("A2D3")->read() << 8);
	}

	return 0;
}


WRITE32_MEMBER(atarigx2_state::latch_w)
{
	/*
	    D13 = 68.DISA
	    D12 = ERASE
	    D11 = /MOGO
	    D8  = VCR
	    D5  = /XRESET
	    D4  = /SNDRES
	    D3  = CC.L
	    D0  = CC.R
	*/

	logerror("latch_w(%08X) & %08X\n", data, mem_mask);

	/* upper byte */
	if (ACCESSING_BITS_24_31)
	{
		/* bits 13-11 are the MO control bits */
		m_rle->control_write(space, offset, (data >> 27) & 7);
	}

	/* lower byte */
	if (ACCESSING_BITS_16_23)
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, (data & 0x100000) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE32_MEMBER(atarigx2_state::mo_command_w)
{
	COMBINE_DATA(m_mo_command);
	if (ACCESSING_BITS_0_15)
		m_rle->command_write(space, offset, ((data & 0xffff) == 2) ? ATARIRLE_COMMAND_CHECKSUM : ATARIRLE_COMMAND_DRAW);
}



/*************************************
 *
 *  Protection?
 *
 *************************************/


WRITE32_MEMBER(atarigx2_state::atarigx2_protection_w)
{
	{
		int pc = space.device().safe_pcbase();
//      if (pc == 0x11cbe || pc == 0x11c30)
//          logerror("%06X:Protection W@%04X = %04X  (result to %06X)\n", pc, offset, data, space.device().state().state_int(M68K_A2));
//      else
		if (ACCESSING_BITS_16_31)
			logerror("%06X:Protection W@%04X = %04X\n", pc, offset * 4, data >> 16);
		else
			logerror("%06X:Protection W@%04X = %04X\n", pc, offset * 4 + 2, data);
	}

	COMBINE_DATA(&m_protection_base[offset]);

	if (ACCESSING_BITS_16_31)
	{
		m_last_write = m_protection_base[offset] >> 16;
		m_last_write_offset = offset*2;
	}
	if (ACCESSING_BITS_0_15)
	{
		m_last_write = m_protection_base[offset] & 0xffff;
		m_last_write_offset = offset*2+1;
	}
}

/********************

The following pseudocode reproduces almost exactly (there are just 3 or 4 bits of divergence)
the 0x200 values in the table below below under the "initialization" region. The function
ftest4 must be called by throwing away the 17 lower bits on the first element of the pair
(  ftest4(lookup_table[i][0]>>17)  ). In other words, the 16 bits value in the RHS is
function of m_last_write_offset (see the protection code below the table). It's unclear how
this should be generalized to cover other regions of the table, and the scarcity of data
in those regions precludes applying the same kind of analysis.

int parameters3[16][2] = {
    {0x003a, 0x0032},
    {0x0076, 0x0064},
    {0x00ee, 0x00c8},
    {0x01dc, 0x0190},
    {0x01b0, 0x012A},
    {0x0168, 0x005C},
    {0x00d8, 0x00b8},
    {0x01b0, 0x0172},
    {0x016a, 0x00ee},
    {0x00de, 0x01de},
    {0x01be, 0x01b6},
    {0x0174, 0x0164},
    {0x00e2, 0x00c0},
    {0x01c6, 0x0180},
    {0x0186, 0x010a},
    {0x0104, 0x001c}
};

// every output bit is a linear function on the input bits except by
// a quadratic term always involving bit #0 of the input
UINT32 ftest4(UINT32 num)
{
    UINT32 accum = 0;

    for (int i=0; i<16; ++i)
    {
        int b1 = weight(num&parameters3[i][0])&1;  // <- linear function (parameters3[i][0] acts as a mask determining which bits are added)
        int b2 = weight(num&parameters3[i][1])&1;  // <- idem
        b2 &= BIT(num, 0);                         // quadratic term
        int b3 = b1 ^ b2;

        accum ^= (b3 << i);
    }

    return accum;
}

*********************/

READ32_MEMBER(atarigx2_state::atarigx2_protection_r)
{
	static const UINT32 lookup_table[][2] =
	{
		// sprite flipping
		{ 0x0000e54f, 0<<11 },
		{ 0x00024602, 5<<11 },
		{ 0x0004ec02, 7<<11 },
		{ 0x00064ddb, 7<<11 },
		{ 0x00086016, 0<<11 },
		{ 0x000ad909, 1<<11 },
		{ 0x000cf3cd, 3<<11 },
		{ 0x000e0a0f, 3<<11 },

		{ 0x00001b23, 0<<11 },
		{ 0x0002948b, 5<<11 },
		{ 0x0004a826, 7<<11 },
		{ 0x0006f6eb, 7<<11 },
		{ 0x00085031, 0<<11 },
		{ 0x000a798f, 1<<11 },
		{ 0x000c708e, 3<<11 },
		{ 0x000e1bf8, 3<<11 },

		{ 0x00000241, 0<<11 },
		{ 0x00020892, 5<<11 },
		{ 0x0004e0dc, 7<<11 },
		{ 0x00066288, 7<<11 },
		{ 0x0008294a, 0<<11 },
		{ 0x000a31dc, 1<<11 },
		{ 0x000c4413, 3<<11 },
		{ 0x000e31b6, 3<<11 },

		{ 0x000097c1, 0<<11 },
		{ 0x00020e6b, 5<<11 },
		{ 0x00042f77, 7<<11 },
		{ 0x00068256, 7<<11 },
		{ 0x0008317f, 0<<11 },
		{ 0x000af594, 1<<11 },
		{ 0x000c0a72, 3<<11 },
		{ 0x000ea856, 3<<11 },

		{ 0x0000ebd6, 0<<11 },
		{ 0x0002e22c, 5<<11 },
		{ 0x0004bca8, 7<<11 },
		{ 0x000688dd, 7<<11 },
		{ 0x00088556, 0<<11 },
		{ 0x000ad37f, 1<<11 },
		{ 0x000ca6d4, 3<<11 },
		{ 0x000e88c3, 3<<11 },

		{ 0x00007d37, 0<<11 },
		{ 0x00027b45, 5<<11 },
		{ 0x00045981, 7<<11 },
		{ 0x0006d53c, 7<<11 },
		{ 0x0008223e, 0<<11 },
		{ 0x000aaee3, 1<<11 },
		{ 0x000cf4c8, 3<<11 },
		{ 0x000e1064, 3<<11 },


		// initialization

		{ 0x00000241, 0x0000 },
		{ 0x0002F1CC, 0x0000 },
		{ 0x00043996, 0x7707 },
		{ 0x0006451F, 0x3096 },
		{ 0x0008BA02, 0xee0e },
		{ 0x000A78F4, 0x612c },
		{ 0x000CB890, 0x9909 },
		{ 0x000EF8A2, 0x51ba },
		{ 0x00105C79, 0x076d },
		{ 0x00121D13, 0xc419 },
		{ 0x00148CB8, 0x706a },
		{ 0x00162C65, 0xf48f },
		{ 0x0018FABB, 0xe963 },
		{ 0x001A2A33, 0xa535 },
		{ 0x001CF0DD, 0x9e64 },
		{ 0x001E8C14, 0x95a3 },
		{ 0x0020F239, 0x0edb },
		{ 0x00228E87, 0x8832 },
		{ 0x0024B022, 0x79dc },
		{ 0x00261D55, 0xb8a4 },
		{ 0x002889AB, 0xe0d5 },
		{ 0x002A935B, 0xe91e },
		{ 0x002C7BAE, 0x97d2 },
		{ 0x002E39D1, 0xd988 },
		{ 0x0030E44E, 0x09b6 },
		{ 0x00321765, 0x4c2b },
		{ 0x003487FF, 0x7eb1 },
		{ 0x003658C2, 0x7cbd },
		{ 0x00381E83, 0xe7b8 },
		{ 0x003A77CD, 0x2d07 },
		{ 0x003CCAE4, 0x90bf },
		{ 0x003EC0CB, 0x1d91 },
		{ 0x00409494, 0x1db7 },
		{ 0x004289EA, 0x1064 },
		{ 0x004458F1, 0x6ab0 },
		{ 0x00468029, 0x20f2 },
		{ 0x0048491E, 0xf3b9 },
		{ 0x004AB99D, 0x7148 },
		{ 0x004C1C9F, 0x84be },
		{ 0x004E2764, 0x41de },
		{ 0x00507ED9, 0x1ada },
		{ 0x005215B4, 0xd47d },
		{ 0x00547C85, 0x6ddd },
		{ 0x0056B74A, 0xe4eb },
		{ 0x005852C4, 0xf4d4 },
		{ 0x005AC949, 0xb551 },
		{ 0x005CD796, 0x83d3 },
		{ 0x005E9A08, 0x85c7 },
		{ 0x0060F0A5, 0x136c },
		{ 0x00629CD1, 0x9856 },
		{ 0x0064D3BB, 0x646b },
		{ 0x0066DF91, 0xa8c0 },
		{ 0x0068E892, 0xfd62 },
		{ 0x006AA1A7, 0xf97a },
		{ 0x006C2D39, 0x8a65 },
		{ 0x006E5928, 0xc9ec },
		{ 0x0070E2F1, 0x1401 },
		{ 0x00720C3A, 0x5c4f },
		{ 0x0074EBF3, 0x6306 },
		{ 0x00763113, 0x6cd9 },
		{ 0x00788C2A, 0xfa0f },
		{ 0x007A9E4B, 0x3d63 },
		{ 0x007C021D, 0x8d08 },
		{ 0x007E5B2C, 0x0df5 },
		{ 0x008016C3, 0x3b6e },
		{ 0x008211C5, 0x20c8 },
		{ 0x0084BED3, 0x4c69 },
		{ 0x00867F09, 0x105e },
		{ 0x00888CA5, 0xd560 },
		{ 0x008A7AA6, 0x41e4 },
		{ 0x008CE2C3, 0xa267 },
		{ 0x008EBA54, 0x7172 },
		{ 0x0090F0D1, 0x3c03 },
		{ 0x0092635E, 0xe4d1 },
		{ 0x0094AE35, 0x4b04 },
		{ 0x00969B70, 0xd447 },
		{ 0x00989794, 0xd20d },
		{ 0x009AC1CD, 0x85fd },
		{ 0x009CA4AA, 0xa50a },
		{ 0x009E265A, 0xb56b },
		{ 0x00A0D6FC, 0x35b5 },
		{ 0x00A24182, 0xa8fa },
		{ 0x00A405F7, 0x42b2 },
		{ 0x00A6C9BE, 0x986c },
		{ 0x00A8E888, 0xdbbb },
		{ 0x00AAA365, 0xc9d6 },
		{ 0x00AC9993, 0xacbc },
		{ 0x00AE3F72, 0xf940 },
		{ 0x00B03288, 0x32d8 },
		{ 0x00B2F67D, 0x6ce3 },
		{ 0x00B440B2, 0x45df },
		{ 0x00B67DA8, 0x5c75 },
		{ 0x00B89694, 0xdcd6 },
		{ 0x00BA29D0, 0x0dcf },
		{ 0x00BCED90, 0xabd1 },
		{ 0x00BE9524, 0x3d59 },
		{ 0x00C0F2B3, 0x26d9 },
		{ 0x00C240C2, 0x30ac },
		{ 0x00C4BF06, 0x51de },
		{ 0x00C68352, 0x003a },
		{ 0x00C868F8, 0xc8d7 },
		{ 0x00CAA9AF, 0x5180 },
		{ 0x00CC25DE, 0xbfd0 },
		{ 0x00CE8F3F, 0x6116 },
		{ 0x00D00D11, 0x21b4 },
		{ 0x00D216C8, 0xf4b5 },
		{ 0x00D4EF51, 0x56b3 },
		{ 0x00D61327, 0xc423 },
		{ 0x00D802EB, 0xcfba },
		{ 0x00DA2BD6, 0x9599 },
		{ 0x00DCFDB5, 0xb8bd },
		{ 0x00DECE09, 0xa50f },
		{ 0x00E037C5, 0x2802 },
		{ 0x00E2ECA7, 0xb89e },
		{ 0x00E40330, 0x5f05 },
		{ 0x00E6ACE5, 0x8808 },
		{ 0x00E8390C, 0xc60c },
		{ 0x00EAB30C, 0xd9b2 },
		{ 0x00ECB932, 0xb10b },
		{ 0x00EEEBB1, 0xe924 },
		{ 0x00F0B8CF, 0x2f6f },
		{ 0x00F23F66, 0x7c87 },
		{ 0x00F440E4, 0x5868 },
		{ 0x00F63401, 0x4c11 },
		{ 0x00F86F6F, 0xc161 },
		{ 0x00FAAD62, 0x1dab },
		{ 0x00FC0C91, 0xb666 },
		{ 0x00FE8189, 0x2d3d },
		{ 0x01000712, 0x76dc },
		{ 0x010208E2, 0x4190 },
		{ 0x01044C60, 0x01db },
		{ 0x0106254D, 0x7106 },
		{ 0x01081649, 0x98d2 },
		{ 0x010AB069, 0x20bc },
		{ 0x010C2BE2, 0xefd5 },
		{ 0x010E1B1E, 0x102a },
		{ 0x0110C888, 0x71b1 },
		{ 0x0112506A, 0x8589 },
		{ 0x0114E064, 0x06b6 },
		{ 0x01161632, 0xb51f },
		{ 0x0118A299, 0x9fbf },
		{ 0x011A94DB, 0xe4a5 },
		{ 0x011C0E71, 0xe8b8 },
		{ 0x011E9EF3, 0xd433 },
		{ 0x012041FA, 0x7807 },
		{ 0x0122D26A, 0xc9a2 },
		{ 0x01246AE1, 0x0f00 },
		{ 0x0126B6A8, 0xf934 },
		{ 0x012836BC, 0x9609 },
		{ 0x012AEDE2, 0xa88e },
		{ 0x012CF8F2, 0xe10e },
		{ 0x012E7530, 0x9818 },
		{ 0x0130CE06, 0x7f6a },
		{ 0x0132490B, 0x0dbb },
		{ 0x0134DCBE, 0x086d },
		{ 0x01363A41, 0x3d2d },
		{ 0x0138835D, 0x9164 },
		{ 0x013A1970, 0x6c97 },
		{ 0x013CCB58, 0xe663 },
		{ 0x013E514E, 0x5c01 },
		{ 0x01401717, 0x6b6b },
		{ 0x01428305, 0x51f4 },
		{ 0x0144A4CC, 0x1c6c },
		{ 0x014675E6, 0x6162 },
		{ 0x0148EBBC, 0x8565 },
		{ 0x014AED25, 0x30d8 },
		{ 0x014C860E, 0xf262 },
		{ 0x014EE316, 0x004e },
		{ 0x01509004, 0x6c06 },
		{ 0x0152C1C0, 0x95ed },
		{ 0x0154F096, 0x1b01 },
		{ 0x0156ED9B, 0xa57b },
		{ 0x01587C24, 0x8208 },
		{ 0x015A5DA7, 0xf4c1 },
		{ 0x015CFF4E, 0xf50f },
		{ 0x015E69E6, 0xc457 },
		{ 0x0160A24B, 0x65b0 },
		{ 0x0162CDF1, 0xd9c6 },
		{ 0x0164AD72, 0x12b7 },
		{ 0x01660309, 0xe950 },
		{ 0x0168E65E, 0x8bbe },
		{ 0x016A31D4, 0xb8ea },
		{ 0x016C3361, 0xfcb9 },
		{ 0x016E8E02, 0x887c },
		{ 0x01703638, 0x62dd },
		{ 0x0172EF1D, 0x1ddf },
		{ 0x0174E345, 0x15da },
		{ 0x0176029F, 0x2d49 },
		{ 0x01782EF0, 0x8cd3 },
		{ 0x017A7D43, 0x7cf3 },
		{ 0x017CA201, 0xfbd4 },
		{ 0x017E0360, 0x4c65 },
		{ 0x0180E8D8, 0x4db2 },
		{ 0x01820C2E, 0x6158 },
		{ 0x0184D83C, 0x3ab5 },
		{ 0x01867A8F, 0x51ce },
		{ 0x0188B382, 0xa3bc },
		{ 0x018AD630, 0x0074 },
		{ 0x018C7A47, 0xd4bb },
		{ 0x018EE93A, 0x30e2 },
		{ 0x019039FF, 0x4adf },
		{ 0x01925D77, 0xa541 },
		{ 0x0194C33D, 0x3dd8 },
		{ 0x019624B0, 0x95d7 },
		{ 0x01984FAA, 0xa4d1 },
		{ 0x019AF889, 0xc46d },
		{ 0x019C6DE5, 0xd3d6 },
		{ 0x019E06D4, 0xf4fb },
		{ 0x01A04BB4, 0x4369 },
		{ 0x01A2827B, 0xe96a },
		{ 0x01A4490A, 0x346e },
		{ 0x01A68ADA, 0xd9fc },
		{ 0x01A8D44B, 0xad67 },
		{ 0x01AAEB4A, 0x8846 },
		{ 0x01AC9BE5, 0xda60 },
		{ 0x01AE44F3, 0xb8d0 },
		{ 0x01B04E07, 0x4404 },
		{ 0x01B27E24, 0x2d73 },
		{ 0x01B47297, 0x3303 },
		{ 0x01B6E409, 0x1de5 },
		{ 0x01B8FFBC, 0xaa0a },
		{ 0x01BA5CD2, 0x4c5f },
		{ 0x01BCDF00, 0xdd0d },
		{ 0x01BEF748, 0x7cc9 },
		{ 0x01C0A6B1, 0x5005 },
		{ 0x01C2030E, 0x713c },
		{ 0x01C46FE5, 0x2702 },
		{ 0x01C65493, 0x41aa },
		{ 0x01C840BC, 0xbe0b },
		{ 0x01CA8B3E, 0x1010 },
		{ 0x01CCD7C4, 0xc90c },
		{ 0x01CE1A9C, 0x2086 },
		{ 0x01D015E6, 0x5768 },
		{ 0x01D272E4, 0xb525 },
		{ 0x01D4B256, 0x206f },
		{ 0x01D6CB39, 0x8583 },
		{ 0x01D867DA, 0xb966 },
		{ 0x01DA73ED, 0xd409 },
		{ 0x01DC6735, 0xce61 },
		{ 0x01DEAA0E, 0xe49f },
		{ 0x01E01B49, 0x5ede },
		{ 0x01E20CFD, 0xf90e },
		{ 0x01E415C3, 0x29d9 },
		{ 0x01E60EA5, 0xc998 },
		{ 0x01E87809, 0xb0d0 },
		{ 0x01EA455D, 0x9822 },
		{ 0x01EC5A0B, 0xc7d7 },
		{ 0x01EEAFDE, 0xa8b4 },
		{ 0x01F0A050, 0x59b3 },
		{ 0x01F2A751, 0x3d17 },
		{ 0x01F4EEF5, 0x2eb4 },
		{ 0x01F643FC, 0x0d81 },
		{ 0x01F8B9A0, 0xb7bd },
		{ 0x01FAA071, 0x5c3b },
		{ 0x01FC0340, 0xc0ba },
		{ 0x01FECF88, 0x6cad },
		{ 0x0200ED43, 0xedb8 },
		{ 0x02028222, 0x8320 },
		{ 0x020412A4, 0x9abf },
		{ 0x0206FC35, 0xb3b6 },
		{ 0x020870E9, 0x03b6 },
		{ 0x020AA719, 0xe20c },
		{ 0x020CBEAE, 0x74b1 },
		{ 0x020E50D4, 0xd29a },
		{ 0x0210BCA5, 0xead5 },
		{ 0x02127B20, 0x4739 },
		{ 0x02149187, 0x9dd2 },
		{ 0x021609C1, 0x77af },
		{ 0x02180B25, 0x04db },
		{ 0x021A24CA, 0x2615 },
		{ 0x021C1E83, 0x73dc },
		{ 0x021EEEF9, 0x1683 },
		{ 0x022042DD, 0xe363 },
		{ 0x02226C65, 0x0b12 },
		{ 0x0224347F, 0x9464 },
		{ 0x02266615, 0x3b84 },
		{ 0x02280966, 0x0d6d },
		{ 0x022A1FEB, 0x6a3e },
		{ 0x022CCA04, 0x7a6a },
		{ 0x022E9490, 0x5aa8 },
		{ 0x0230185F, 0xe40e },
		{ 0x023297E3, 0xcf0b },
		{ 0x02345336, 0x9309 },
		{ 0x02364610, 0xff9d },
		{ 0x0238C709, 0x0a00 },
		{ 0x023A559B, 0xae27 },
		{ 0x023CC2C8, 0x7d07 },
		{ 0x023E23E3, 0x9eb1 },
		{ 0x0240EEA8, 0xf00f },
		{ 0x0242C980, 0x9344 },
		{ 0x02448D9B, 0x8708 },
		{ 0x02464411, 0xa3d2 },
		{ 0x0248A96F, 0x1e01 },
		{ 0x024A6EB8, 0xf268 },
		{ 0x024C025A, 0x6906 },
		{ 0x024E7D2A, 0xc2fe },
		{ 0x0250DCBA, 0xf762 },
		{ 0x02526DCA, 0x575d },
		{ 0x0254063E, 0x8065 },
		{ 0x0256CEDB, 0x67cb },
		{ 0x0258106A, 0x196c },
		{ 0x025A39AC, 0x3671 },
		{ 0x025CB664, 0x6e6b },
		{ 0x025E5CEB, 0x06e7 },
		{ 0x02609C06, 0xfed4 },
		{ 0x0262E82F, 0x1b76 },
		{ 0x0264702B, 0x89d3 },
		{ 0x026638E8, 0x2be0 },
		{ 0x02680576, 0x10da },
		{ 0x026A06A9, 0x7a5a },
		{ 0x026CDF6E, 0x67dd },
		{ 0x026EEF02, 0x4acc },
		{ 0x0270E35B, 0xf9b9 },
		{ 0x02729A9A, 0xdf6f },
		{ 0x0274E451, 0x8ebe },
		{ 0x0276D343, 0xeff9 },
		{ 0x027819F8, 0x17b7 },
		{ 0x027A0956, 0xbe43 },
		{ 0x027C0DB0, 0x60b0 },
		{ 0x027E5D80, 0x8ed5 },
		{ 0x02806C72, 0xd6d6 },
		{ 0x0282A80F, 0xa3e8 },
		{ 0x0284E5EC, 0xa1d1 },
		{ 0x02867DCD, 0x937e },
		{ 0x02882667, 0x38d8 },
		{ 0x028AEBCD, 0xc2c4 },
		{ 0x028C668D, 0x4fdf },
		{ 0x028E5840, 0xf252 },
		{ 0x0290CB93, 0xd1bb },
		{ 0x02921C8F, 0x67f1 },
		{ 0x02944E2B, 0xa6bc },
		{ 0x02961785, 0x5767 },
		{ 0x0298E2C3, 0x3fb5 },
		{ 0x029ABF47, 0x06dd },
		{ 0x029CED9E, 0x48b2 },
		{ 0x029E5173, 0x364b },
		{ 0x02A0004A, 0xd80d },
		{ 0x02A25437, 0x2bda },
		{ 0x02A49B8D, 0xaf0a },
		{ 0x02A664A0, 0x1b4c },
		{ 0x02A8AE40, 0x3603 },
		{ 0x02AAE51F, 0x4af6 },
		{ 0x02AC80FA, 0x4104 },
		{ 0x02AE9687, 0x7a60 },
		{ 0x02B0CD25, 0xdf60 },
		{ 0x02B2BCB3, 0xefc3 },
		{ 0x02B4CD8D, 0xa967 },
		{ 0x02B6FFFC, 0xdf55 },
		{ 0x02B81F78, 0x316e },
		{ 0x02BA0E8B, 0x8eef },
		{ 0x02BC1A89, 0x4669 },
		{ 0x02BEBA0D, 0xbe79 },
		{ 0x02C0587B, 0xcb61 },
		{ 0x02C2901E, 0xb38c },
		{ 0x02C4F808, 0xbc66 },
		{ 0x02C65F98, 0x931a },
		{ 0x02C89C62, 0x256f },
		{ 0x02CA03F9, 0xd2a0 },
		{ 0x02CCA4C0, 0x5268 },
		{ 0x02CEF1DD, 0xe236 },
		{ 0x02D0707E, 0xcc0c },
		{ 0x02D222FE, 0x7795 },
		{ 0x02D4F00E, 0xbb0b },
		{ 0x02D679FC, 0x4703 },
		{ 0x02D89581, 0x2202 },
		{ 0x02DADF2F, 0x16b9 },
		{ 0x02DCFBCE, 0x5505 },
		{ 0x02DE2E69, 0x262f },
		{ 0x02E069E7, 0xc5ba },
		{ 0x02E20F8F, 0x3bbe },
		{ 0x02E42E77, 0xb2bd },
		{ 0x02E6969F, 0x0b28 },
		{ 0x02E82D59, 0x2bb4 },
		{ 0x02EA14FC, 0x5a92 },
		{ 0x02EC879C, 0x5cb3 },
		{ 0x02EEDEB1, 0x6a04 },
		{ 0x02F0A9CB, 0xc2d7 },
		{ 0x02F226F7, 0xffa7 },
		{ 0x02F40DF5, 0xb5d0 },
		{ 0x02F608E7, 0xcf31 },
		{ 0x02F87608, 0x2cd9 },
		{ 0x02FA507C, 0x9e8b },
		{ 0x02FC2A3F, 0x58de },
		{ 0x02FEC003, 0xae1d },
		{ 0x03007959, 0x9b64 },
		{ 0x0302181A, 0xc2b0 },
		{ 0x03045C87, 0xec63 },
		{ 0x0306F6E4, 0xf226 },
		{ 0x0308E603, 0x756a },
		{ 0x030A1562, 0xa39c },
		{ 0x030CF677, 0x026d },
		{ 0x030E56FD, 0x930a },
		{ 0x0310731C, 0x9c09 },
		{ 0x0312CA4E, 0x06a9 },
		{ 0x03148639, 0xeb0e },
		{ 0x0316140E, 0x363f },
		{ 0x0318EB2F, 0x7207 },
		{ 0x031A13D3, 0x6785 },
		{ 0x031C8940, 0x0500 },
		{ 0x031E5AF7, 0x5713 },
		{ 0x0320557E, 0x95bf },
		{ 0x0322982F, 0x4a82 },
		{ 0x0324B60C, 0xe2b8 },
		{ 0x0326A26C, 0x7a14 },
		{ 0x0328D0F4, 0x7bb1 },
		{ 0x032A9258, 0x2bae },
		{ 0x032CEAAE, 0x0cb6 },
		{ 0x032EF0B3, 0x1b38 },
		{ 0x0330E489, 0x92d2 },
		{ 0x03322BC0, 0x8e9b },
		{ 0x0334E8E2, 0xe5d5 },
		{ 0x0336CB76, 0xbe0d },
		{ 0x03380957, 0x7cdc },
		{ 0x033AEEA9, 0xefb7 },
		{ 0x033C0D68, 0x0bdb },
		{ 0x033E26ED, 0xdf21 },
		{ 0x0340E5BA, 0x86d3 },
		{ 0x0342445C, 0xd2d4 },
		{ 0x03444287, 0xf1d4 },
		{ 0x03463A60, 0xe242 },
		{ 0x0348B680, 0x68dd },
		{ 0x034A68AD, 0xb3f8 },
		{ 0x034C283D, 0x1fda },
		{ 0x034EF68D, 0x836e },
		{ 0x03509056, 0x81be },
		{ 0x03524E05, 0x16cd },
		{ 0x0354B3B5, 0xf6b9 },
		{ 0x0356A11D, 0x265b },
		{ 0x0358049C, 0x6fb0 },
		{ 0x035AD934, 0x77e1 },
		{ 0x035C07CD, 0x18b7 },
		{ 0x035E9F07, 0x4777 },
		{ 0x0360959C, 0x8808 },
		{ 0x0362957A, 0x5ae6 },
		{ 0x036400C5, 0xff0f },
		{ 0x0366E520, 0x6a70 },
		{ 0x0368E728, 0x6606 },
		{ 0x036A321D, 0x3bca },
		{ 0x036C0BF2, 0x1101 },
		{ 0x036EC1A9, 0x0b5c },
		{ 0x037034E7, 0x8f65 },
		{ 0x0372D30B, 0x9eff },
		{ 0x0374056C, 0xf862 },
		{ 0x0376AB8C, 0xae69 },
		{ 0x03786234, 0x616b },
		{ 0x037AF5F5, 0xffd3 },
		{ 0x037CDD2B, 0x166c },
		{ 0x037EE8D2, 0xcf45 },
		{ 0x03805144, 0xa00a },
		{ 0x03826A5C, 0xe278 },
		{ 0x03840DE5, 0xd70d },
		{ 0x03869747, 0xd2ee },
		{ 0x03888E63, 0x4e04 },
		{ 0x038A12F2, 0x8354 },
		{ 0x038C597B, 0x3903 },
		{ 0x038EB704, 0xb3c2 },
		{ 0x03902359, 0xa767 },
		{ 0x0392FB74, 0x2661 },
		{ 0x0394521A, 0xd060 },
		{ 0x03962F46, 0x16f7 },
		{ 0x0398F591, 0x4969 },
		{ 0x039A6F1A, 0x474d },
		{ 0x039CA694, 0x3e6e },
		{ 0x039E823F, 0x77db },
		{ 0x03A02FF5, 0xaed1 },
		{ 0x03A253D2, 0x6a4a },
		{ 0x03A4BE72, 0xd9d6 },
		{ 0x03A65896, 0x5adc },
		{ 0x03A8BBAD, 0x40df },
		{ 0x03AA21D8, 0x0b66 },
		{ 0x03AC77E9, 0x37d8 },
		{ 0x03AEF0FB, 0x3bf0 },
		{ 0x03B0DB09, 0xa9bc },
		{ 0x03B211E7, 0xae53 },
		{ 0x03B4DF01, 0xdebb },
		{ 0x03B667F3, 0x9ec5 },
		{ 0x03B827ED, 0x47b2 },
		{ 0x03BA1787, 0xcf7f },
		{ 0x03BCE3A9, 0x30b5 },
		{ 0x03BE34A1, 0xffe9 },
		{ 0x03C02323, 0xbdbd },
		{ 0x03C2370F, 0xf21c },
		{ 0x03C4EBDC, 0xcaba },
		{ 0x03C62A95, 0xc28a },
		{ 0x03C8301E, 0x53b3 },
		{ 0x03CA4EB5, 0x9330 },
		{ 0x03CCE18C, 0x24b4 },
		{ 0x03CE1646, 0xa3a6 },
		{ 0x03D07157, 0xbad0 },
		{ 0x03D2B0B7, 0x3605 },
		{ 0x03D42FFF, 0xcdd7 },
		{ 0x03D6269B, 0x0693 },
		{ 0x03D88BB0, 0x54de },
		{ 0x03DAE167, 0x5729 },
		{ 0x03DCA760, 0x23d9 },
		{ 0x03DECF00, 0x67bf },
		{ 0x03E02963, 0xb366 },
		{ 0x03E28258, 0x7a2e },
		{ 0x03E4FCA3, 0xc461 },
		{ 0x03E6263F, 0x4ab8 },
		{ 0x03E8EB02, 0x5d68 },
		{ 0x03EA0B78, 0x1b02 },
		{ 0x03EC5550, 0x2a6f },
		{ 0x03EEBE0C, 0x2b94 },
		{ 0x03F0806F, 0xb40b },
		{ 0x03F2A949, 0xbe37 },
		{ 0x03F41B2F, 0xc30c },
		{ 0x03F67713, 0x8ea1 },
		{ 0x03F89F8E, 0x5a05 },
		{ 0x03FA5016, 0xdf1b },
		{ 0x03FCDDE6, 0x2d02 },
		{ 0x03FE5B4D, 0xef8d },

		{ 0x02003AC0, 0x00d0 },
		{ 0x0202D4BC, 0x7fb1 },
		{ 0x02042B3E, 0x7f6f },
		{ 0x0206D2C0, 0x7f2c },
		{ 0x0208EB82, 0x7ee9 },
		{ 0x020A0B9A, 0x76c9 },
		{ 0x020C4137, 0x6e88 },
		{ 0x020EFA40, 0x6647 },
		{ 0x02101507, 0x5e07 },
		{ 0x0212020F, 0x55e6 },
		{ 0x0214C3DE, 0x4da5 },
		{ 0x02163152, 0x4164 },
		{ 0x02187A11, 0x3944 },
		{ 0x021A314B, 0x3103 },
		{ 0x021C0C30, 0x28c2 },
		{ 0x021E757D, 0x20a2 },

		{ 0x01C0F05D, 0x0000 },
		{ 0x01C24084, 0x562c },
		{ 0x01C4B60C, 0xb5d2 },
		{ 0x01C6C25D, 0xc98e },
		{ 0x01C874E5, 0xcd87 },
		{ 0x01CAB35C, 0x31a7 },
		{ 0x01CCABF7, 0xc8e7 },
		{ 0x01CE60F5, 0xad4e },
		{ 0x01D06E82, 0x34e8 },
		{ 0x01D20210, 0x190c },
		{ 0x01D44FFD, 0x4461 },
		{ 0x01D6FCCC, 0x9cc3 },
		{ 0x01D8CD46, 0xa842 },
		{ 0x01DAA446, 0x886b },
		{ 0x01DC764D, 0x0000 },
		{ 0x01DE667F, 0x5235 },
		{ 0x0180CF8D, 0x0000 },
		{ 0x01820D53, 0x7fff },
		{ 0x0184E62A, 0x7ffa },
		{ 0x018693FD, 0x7ff5 },
		{ 0x0188D142, 0x7ff0 },
		{ 0x018AF821, 0x7feb },
		{ 0x018CDE00, 0x7fc9 },
		{ 0x018ED3E3, 0x7f87 },
		{ 0x01904654, 0x7f65 },
		{ 0x01921444, 0x7f22 },
		{ 0x0194851C, 0x7ee0 },
		{ 0x019615A2, 0x7a60 },
		{ 0x019815A4, 0x69c0 },
		{ 0x019AF59F, 0x64e0 },
		{ 0x019C7C68, 0x4c00 },
		{ 0x019E3D7A, 0x3c00 },
		{ 0x01A0D5A9, 0x03e0 },
		{ 0x01A22C96, 0x0080 },
		{ 0x01A4F15D, 0x1c0b },
		{ 0x01A6A9DD, 0x45ae },
		{ 0x01A85152, 0x6f7f },
		{ 0x01AA1429, 0x673d },
		{ 0x01AC153D, 0x5ada },
		{ 0x01AED816, 0x5298 },
		{ 0x01B037B8, 0x4635 },
		{ 0x01B2626F, 0x3df3 },
		{ 0x01B4EFE1, 0x3190 },
		{ 0x01B66047, 0x294e },
		{ 0x01B8A17F, 0x1ceb },
		{ 0x01BAE8F8, 0x14a9 },
		{ 0x01BC381B, 0x0846 },
		{ 0x01BEB317, 0x0003 },

		{ 0x07A0D7BD, 0x0000 },
		{ 0x07A2FDF7, 0x7c00 },
		{ 0x07A414B0, 0x0360 },
		{ 0x07A6A375, 0x001f },
		{ 0x07A85770, 0x7fe0 },
		{ 0x07AA2F6E, 0x7fff },
		{ 0x07AC384C, 0x541d },
		{ 0x07AEDFF1, 0x037f },
		{ 0x07B0F8D9, 0x0000 },
		{ 0x07B2EAAF, 0x7fe7 },
		{ 0x07B4750F, 0x7f48 },
		{ 0x07B6346C, 0x7ea9 },
		{ 0x07B8DAC0, 0x7dea },
		{ 0x07BC6350, 0x03e0 },

		{ 0x014064E7, 0x0000 },
		{ 0x014251E1, 0x7ffa },
		{ 0x01443AA9, 0x7f8d },
		{ 0x0146488B, 0x7fff },
		{ 0x014829A2, 0x7ff0 },
		{ 0x014AA74B, 0x7fd9 },
		{ 0x014C2EFA, 0x7ffc },
		{ 0x014E1287, 0x7f4f },
		{ 0x0150E842, 0x7f68 },
		{ 0x0152158D, 0x7ee3 },
		{ 0x0154B670, 0x7e41 },
		{ 0x0156D95E, 0x7e48 },
		{ 0x01584FF1, 0x7da0 },
		{ 0x015A6F2A, 0x7f99 },
		{ 0x015CA37A, 0x76cc },
		{ 0x015ECB80, 0x6f17 },
		{ 0x01602196, 0x6ad3 },
		{ 0x0162D660, 0x5e92 },
		{ 0x0164A29B, 0x624e },
		{ 0x0166DD13, 0x6208 },
		{ 0x01684D54, 0x71a1 },
		{ 0x016A2FF4, 0x6108 },
		{ 0x016CFDFD, 0x64e0 },
		{ 0x016E05A0, 0x4586 },
		{ 0x0170B432, 0x4e2e },
		{ 0x0172E5C7, 0x41ac },
		{ 0x0174CDBB, 0x3107 },
		{ 0x01769E44, 0x3880 },
		{ 0x0178FAD3, 0x3567 },
		{ 0x017A602C, 0x20c3 },
		{ 0x017C096A, 0x0800 },
		{ 0x017E1488, 0x7fff },

		{ 0x0BE0F70E, 0x4cb3 },
		{ 0x0BE2E350, 0x7fff },
		{ 0x0BE4F722, 0x7ffe },
		{ 0x0BE68643, 0x7ffb },
		{ 0x0BE8CB3E, 0x7ff9 },
		{ 0x0BEAABDC, 0x7ff6 },
		{ 0x0BEC000F, 0x7ff4 },
		{ 0x0BEEC3C1, 0x7bd1 },
		{ 0x0BF0C04A, 0x7bcf },
		{ 0x0BF2EB06, 0x77ae },
		{ 0x0BF45561, 0x77ad },
		{ 0x0BF66384, 0x738d },
		{ 0x0BF80566, 0x6f6c },
		{ 0x0BFA6EB1, 0x6f6b },
		{ 0x0BFCF260, 0x6b4a },
		{ 0x0BFE60B7, 0x6b4a },
		{ 0x0C000F4B, 0x6729 },
		{ 0x0C02E81E, 0x6308 },
		{ 0x0C04CF07, 0x6307 },
		{ 0x0C060E25, 0x5ee6 },
		{ 0x0C0839B6, 0x5ac6 },
		{ 0x0C0ABF8D, 0x5ac5 },
		{ 0x0C0C35A0, 0x56a4 },
		{ 0x0C0ECA49, 0x56a0 },
		{ 0x0C10B601, 0x56a0 },
		{ 0x0C12175F, 0x56a0 },
		{ 0x0C14C4BE, 0x5280 },
		{ 0x0C161897, 0x5280 },
		{ 0x0C18651C, 0x4e60 },
		{ 0x0C1AD23F, 0x4e60 },
		{ 0x0C1CB033, 0x4a40 },
		{ 0x0C1EDA23, 0x4a40 },
		{ 0x0C20A71B, 0x4a40 },
		{ 0x0C22DAF3, 0x4620 },
		{ 0x0C24EE1E, 0x4620 },
		{ 0x0C26D154, 0x4201 },
		{ 0x0C280468, 0x4201 },
		{ 0x0C2A34A7, 0x3de1 },
		{ 0x0C2CA338, 0x3de1 },
		{ 0x0C2E71F4, 0x3de1 },
		{ 0x0C30E165, 0x39c1 },
		{ 0x0C32BB85, 0x39c1 },
		{ 0x0C340203, 0x35a1 },
		{ 0x0C363F4A, 0x35a1 },
		{ 0x0C382D53, 0x3181 },
		{ 0x0C3AAA4B, 0x3181 },
		{ 0x0C3C4602, 0x2d61 },
		{ 0x0C3E888C, 0x2d61 },
		{ 0x0C4082BF, 0x2d61 },
		{ 0x0C42D415, 0x2d61 },
		{ 0x0C445792, 0x2941 },
		{ 0x0C46D209, 0x2941 },
		{ 0x0C485FF6, 0x2941 },
		{ 0x0C4AF2EC, 0x2521 },
		{ 0x0C4CDEFD, 0x2521 },
		{ 0x0C4E5AB9, 0x2521 },
		{ 0x0C50B991, 0x2521 },
		{ 0x0C52B6EF, 0x2101 },
		{ 0x0C5435E9, 0x2101 },
		{ 0x0C56A383, 0x1ce1 },
		{ 0x0C58C376, 0x18c1 },
		{ 0x0C5AF004, 0x1080 },
		{ 0x0C5CD659, 0x0c60 },
		{ 0x0C5E7C17, 0x0000 },

		// in-game demo

		{ 0x0740E366, 0x0000 },
		{ 0x07426EEA, 0x03e0 },
		{ 0x07485D97, 0x0000 },
		{ 0x074A20B4, 0x3863 },
		{ 0x074CFCFA, 0x5c21 },
		{ 0x074E37F7, 0x7c00 },
		{ 0x07509EC6, 0x7e2b },
		{ 0x0752736A, 0x14a6 },
		{ 0x07540D3B, 0x318c },
		{ 0x07567A70, 0x4a53 },
		{ 0x075861B3, 0x6739 },
		{ 0x075A48D6, 0x7fff },
		{ 0x075C4AD8, 0x03e0 },
		{ 0x075E7452, 0x294a },

		{ 0x07BA4193, 0x0045 },
		{ 0x07BECBD7, 0x0000 },

		{ 0x03005940, 0x00d0 },
		{ 0x0302E2B3, 0x7f75 },
		{ 0x03047625, 0x7b33 },
		{ 0x030665F4, 0x72b3 },
		{ 0x0308310F, 0x6eaf },
		{ 0x030A0FBE, 0x666d },
		{ 0x030CE5BD, 0x5e2b },
		{ 0x030E624D, 0x5a0a },
		{ 0x03104644, 0x55a9 },
		{ 0x03124604, 0x49a8 },
		{ 0x031420BD, 0x4d67 },
		{ 0x0316579A, 0x3d46 },
		{ 0x03187A4E, 0x3105 },
		{ 0x031AF2C9, 0x24c5 },
		{ 0x031CA091, 0x20a5 },
		{ 0x031E5980, 0x1881 },

		{ 0x0400C0C3, 0x00d0 },
		{ 0x040261D4, 0x7ef5 },
		{ 0x0404D01A, 0x7ad3 },
		{ 0x04065D42, 0x72b1 },
		{ 0x0408E974, 0x6e8f },
		{ 0x040AE9DF, 0x662e },
		{ 0x040CA223, 0x61ec },
		{ 0x040E0187, 0x5dcb },
		{ 0x04102BA8, 0x55a9 },
		{ 0x0412A48D, 0x4d88 },
		{ 0x04142A16, 0x4568 },
		{ 0x041629A5, 0x3d27 },
		{ 0x0418D2E6, 0x3105 },
		{ 0x041A091D, 0x24c6 },
		{ 0x041C0EAF, 0x1ca6 },
		{ 0x041E314B, 0x1881 },

		{ 0x04201D60, 0x0045 },
		{ 0x04229C56, 0x7f75 },
		{ 0x042477E4, 0x7b33 },
		{ 0x04263191, 0x72b3 },
		{ 0x0428AF11, 0x6eae },
		{ 0x042ACCBF, 0x666d },
		{ 0x042C0E5B, 0x5e2b },
		{ 0x042E0363, 0x5e0e },
		{ 0x0430D171, 0x59ed },
		{ 0x04322692, 0x51ac },
		{ 0x0434A314, 0x4d67 },
		{ 0x04366B22, 0x3d46 },
		{ 0x04383EFE, 0x3105 },
		{ 0x043A0272, 0x20a7 },
		{ 0x043C0D9F, 0x1487 },
		{ 0x043E2D92, 0x1881 },

		{ 0x0702BB8D, 0x2c28 },
		{ 0x0704470B, 0x3048 },
		{ 0x07067B2D, 0x3048 },
		{ 0x070840B7, 0x3048 },
		{ 0x070A58B4, 0x3469 },
		{ 0x070C99D1, 0x3469 },
		{ 0x070EDEF2, 0x386a },
		{ 0x071044AF, 0x3c8a },
		{ 0x0712EB75, 0x3c8a },
		{ 0x0714DB75, 0x3449 },
		{ 0x0716C8A4, 0x2c27 },
		{ 0x0718346B, 0x2406 },
		{ 0x071AFE61, 0x2406 },
		{ 0x071C771A, 0x2807 },
		{ 0x071E50A7, 0x2827 },


		// troid screen

		{ 0x07C0B2D7, 0x0019 },
		{ 0x07C222F6, 0x7fff },
		{ 0x07C43550, 0x7b5f },
		{ 0x07C6221A, 0x7fff },
		{ 0x07C8CEF1, 0x6ede },
		{ 0x07CA4B46, 0x7fdd },
		{ 0x07CC3A84, 0x7f7d },
		{ 0x07CEFD3D, 0x5e9a },
		{ 0x07D09D44, 0x7ffa },
		{ 0x07D20DDC, 0x7b5a },
		{ 0x07D46F47, 0x7f79 },
		{ 0x07D6E3E5, 0x72f9 },
		{ 0x07D8666A, 0x7f18 },
		{ 0x07DAF254, 0x4e17 },
		{ 0x07DC2DD1, 0x5e75 },
		{ 0x07DE0A51, 0x7ff5 },
		{ 0x07E0ADE9, 0x62d5 },
		{ 0x07E2206B, 0x7f55 },
		{ 0x07E4C323, 0x7b35 },
		{ 0x07E681E9, 0x7ef4 },
		{ 0x07E8AFE1, 0x5614 },
		{ 0x07EADE06, 0x76d4 },
		{ 0x07EC7D8D, 0x49f4 },
		{ 0x07EE3EA8, 0x7274 },
		{ 0x07F06F90, 0x7f71 },
		{ 0x07F234C6, 0x41b0 },
		{ 0x07F4320A, 0x5a10 },
		{ 0x07F6F668, 0x7ed0 },
		{ 0x07F869F3, 0x7ab0 },
		{ 0x07FA1140, 0x7f50 },
		{ 0x07FCC28A, 0x6a6f },
		{ 0x07FE3235, 0x5e6f },
		{ 0x08003FD7, 0x720f },
		{ 0x0802DE06, 0x418d },
		{ 0x0804C73E, 0x5a0c },
		{ 0x0806F643, 0x6e0c },
		{ 0x0808188C, 0x55ac },
		{ 0x080A636C, 0x7ecb },
		{ 0x080C4160, 0x310b },
		{ 0x080E39BD, 0x7a6b },
		{ 0x0810AD3A, 0x6a6b },
		{ 0x0812C4F2, 0x20ab },
		{ 0x0814DE48, 0x3d2b },
		{ 0x08163D93, 0x632a },
		{ 0x08181879, 0x458a },
		{ 0x081A8813, 0x5689 },
		{ 0x081C9B42, 0x59e9 },
		{ 0x081E708E, 0x5589 },
		{ 0x08202692, 0x6dc8 },
		{ 0x082280AF, 0x1ca8 },
		{ 0x0824397F, 0x5127 },
		{ 0x08267601, 0x3d27 },
		{ 0x0828F05D, 0x4167 },
		{ 0x082A0A0B, 0x3e47 },
		{ 0x082C881E, 0x2ce7 },
		{ 0x082E1ACC, 0x25a5 },
		{ 0x08304151, 0x5105 },
		{ 0x0832C78D, 0x28a5 },
		{ 0x08346959, 0x1c85 },
		{ 0x08362656, 0x6504 },
		{ 0x083856FE, 0x14e4 },
		{ 0x083A4A7E, 0x3ca4 },
		{ 0x083CCC66, 0x1843 },
		{ 0x083E9C6C, 0x0001 },

		// atari screen

		{ 0x0BC0C176, 0x0000 },
		{ 0x0BC2894F, 0x7774 },
		{ 0x0BC45928, 0x66d2 },
		{ 0x0BC6C3C0, 0x5e8d },
		{ 0x0BC8C31C, 0x7bdd },
		{ 0x0BCA6F93, 0x62b9 },
		{ 0x0BCC2CA7, 0x5a92 },
		{ 0x0BCE9F86, 0x522c },
		{ 0x0BD0644B, 0x3dab },
		{ 0x0BD2A1BC, 0x41f2 },
		{ 0x0BD47C0F, 0x41f8 },
		{ 0x0BD6C568, 0x20e6 },
		{ 0x0BD8368A, 0x2951 },
		{ 0x0BDAF5DE, 0x2da4 },
		{ 0x0BDC37CE, 0x1cc9 },
		{ 0x0BDE6895, 0x14a5 },

		{ 0xffffffff, 0xffff }
	};

	UINT32 result = m_protection_base[offset];

	if (offset == 0x300)
		result |= 0x80000000;
	if (offset == 0x3f0)
	{
		UINT32 tag = (m_last_write_offset << 17) | m_last_write;
		int i = 0;

		while (lookup_table[i][0] != 0xffffffff)
		{
			if (tag == lookup_table[i][0])
			{
				result = lookup_table[i][1] << 16;
				break;
			}
			i++;
		}

		if (lookup_table[i][0] == 0xffffffff)
		{
			if (m_last_write_offset*2 >= 0x700 && m_last_write_offset*2 < 0x720)
				result = machine().rand() << 16;
			else
				result = 0xffff << 16;
			logerror("%06X:Unhandled protection R@%04X = %04X\n", space.device().safe_pcbase(), offset, result);
		}
	}

	if (ACCESSING_BITS_16_31)
		logerror("%06X:Protection R@%04X = %04X\n", space.device().safe_pcbase(), offset * 4, result >> 16);
	else
		logerror("%06X:Protection R@%04X = %04X\n", space.device().safe_pcbase(), offset * 4 + 2, result);
	return result;
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, atarigx2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0xc80000, 0xc80fff) AM_RAM
	AM_RANGE(0xca0000, 0xca0fff) AM_READWRITE(atarigx2_protection_r, atarigx2_protection_w) AM_SHARE("protection_base")
	AM_RANGE(0xd00000, 0xd1ffff) AM_READ(a2d_data_r)
	AM_RANGE(0xd20000, 0xd20fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0xff00ff00)
	AM_RANGE(0xd40000, 0xd40fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xd72000, 0xd75fff) AM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xd76000, 0xd76fff) AM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xd78000, 0xd78fff) AM_RAM AM_SHARE("rle")
	AM_RANGE(0xd7a200, 0xd7a203) AM_WRITE(mo_command_w) AM_SHARE("mo_command")
	AM_RANGE(0xd70000, 0xd7ffff) AM_RAM
	AM_RANGE(0xd80000, 0xd9ffff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xe06000, 0xe06003) AM_DEVWRITE8("jsa", atari_jsa_iiis_device, main_command_w, 0xff000000)
	AM_RANGE(0xe08000, 0xe08003) AM_WRITE(latch_w)
	AM_RANGE(0xe0c000, 0xe0c003) AM_WRITE16(video_int_ack_w, 0xffffffff)
	AM_RANGE(0xe0e000, 0xe0e003) AM_WRITENOP//watchdog_reset_w },
	AM_RANGE(0xe80000, 0xe80003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xe82000, 0xe82003) AM_READ(special_port2_r)
	AM_RANGE(0xe82004, 0xe82007) AM_READ(special_port3_r)
	AM_RANGE(0xe86000, 0xe86003) AM_DEVREAD8("jsa", atari_jsa_iiis_device, main_response_r, 0xff000000)
	AM_RANGE(0xff8000, 0xffffff) AM_RAM
ADDRESS_MAP_END




/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( spclords )
	PORT_START("P1_P2")     /* 68.SW (A1=0,1) */
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )                       /* RED button */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)       /* Right thumb */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)       /* Right trigger */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)       /* Throttle reverse */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )                       /* BLUE button */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)       /* Left thumb */
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)       /* Left trigger */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)       /* Throttle forward */
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)       /* Throttle button */
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SERVICE")      /* 68.STATUS (A2=0) */
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* A2D.EOC */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // /AUDIRQ
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // /AUDFULL
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")     /* 68.STATUS (A2=1) */
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XIRQ */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XFULL */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SERVICER */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SER.L */
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* A2D @ 0xD00000 */
	PORT_BIT ( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D1")      /* A2D @ 0xD00002 */
	PORT_BIT ( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D2")      /* A2D @ 0xD00004 */
	PORT_BIT ( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D3")      /* A2D @ 0xD00006 */
	PORT_BIT ( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( motofren )
	PORT_START("P1_P2")     /* 68.SW (A1=0,1) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )                       /* Start/fire */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)       /* AUX3 */
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)       /* AUX2 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)       /* AUX1 */
	PORT_BIT( 0xf0000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")       /* 68.STATUS (A2=0) */
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED )   /* +5V */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* A2D.EOC */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // /AUDIRQ
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // /AUDFULL
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")       /* 68.STATUS (A2=1) */
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XIRQ */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XFULL */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SERVICER */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SER.L */
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* A2D @ 0xD00000 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("A2D1")      /* A2D @ 0xD00002 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D2")      /* A2D @ 0xD00004 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("A2D3")      /* A2D @ 0xD00006 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rrreveng )
	PORT_START("P1_P2")     /* 68.SW (A1=0,1) */
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0000fe00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_A) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_S) PORT_PLAYER(1)
	PORT_BIT( 0x30000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")       /* 68.STATUS (A2=0) */
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* A2D.EOC */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // /AUDIRQ
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // /AUDFULL
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")       /* 68.STATUS (A2=1) */
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XIRQ */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XFULL */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SERVICER */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SER.L */
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_SPECIAL )  /* +5V */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* A2D @ 0xD00000 */
	PORT_BIT ( 0x00ff, 0x0010, IPT_PEDAL ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D1")      /* A2D @ 0xD00002 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D2")      /* A2D @ 0xD00004 */
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D3")      /* A2D @ 0xD00006 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,3),
	5,
	{ 0, 0, 1, 2, 3 },
	{ RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+4, 0, 4, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+12, 8, 12 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout pftoplayout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+4, 0, 0, 0, 0 },
	{ 3, 2, 1, 0, 11, 10, 9, 8 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};

static GFXDECODE_START( atarigx2 )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout, 0x000, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, anlayout, 0x000, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, pftoplayout, 0x000, 64 )
GFXDECODE_END

static const atari_rle_objects_config modesc_0x200 =
{
	0,          /* left clip coordinate */
	0,          /* right clip coordinate */
	0x200,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x01f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0,0x0e00,0,0,0,0,0,0 }}, /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};

static const atari_rle_objects_config modesc_0x400 =
{
	0,          /* left clip coordinate */
	0,          /* right clip coordinate */
	0x400,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x03f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0,0x0e00,0,0,0,0,0,0 }}, /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( atarigx2, atarigx2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, ATARI_CLOCK_14MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_MACHINE_RESET_OVERRIDE(atarigx2_state,atarigx2)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", atarigx2)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)

	MCFG_TILEMAP_ADD_CUSTOM("playfield", "gfxdecode", 2, atarigx2_state, get_playfield_tile_info, 8,8, atarigx2_playfield_scan, 128,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, atarigx2_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64,32, 0)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a pair of GALs to determine H and V parameters */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(atarigx2_state, screen_update_atarigx2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(atarigx2_state,atarigx2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_ATARI_JSA_IIIS_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("SERVICE", 6)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( atarigx2_0x200, atarigx2 )
	MCFG_ATARIRLE_ADD("rle", modesc_0x200)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( atarigx2_0x400, atarigx2 )
	MCFG_ATARIRLE_ADD("rle", modesc_0x400)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( spclords )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "main0rc.095",  0x000000, 0x020000, CRC(82ddf575) SHA1(e2821b2f576694bce4590cd03b944b7991ecbc27) )
	ROM_LOAD32_BYTE( "main1rc.095",  0x000001, 0x020000, CRC(69d64819) SHA1(e9cb99b0ba2a0e23e7699a61130e5e8a4b632db4) )
	ROM_LOAD32_BYTE( "main2rc.095",  0x000002, 0x020000, CRC(49d30630) SHA1(2d0f2abe5d17b4cf575f80687502fac33c7f3206) )
	ROM_LOAD32_BYTE( "main3rc.095",  0x000003, 0x020000, CRC(3872424c) SHA1(db08ad9386dfe8fa4e2a83a2505118a636247279) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x10000, 0x4000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )
	ROM_CONTINUE(           0x04000, 0xc000 )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41b", 0x000000, 0x80000, CRC(02ce7e07) SHA1(a48a6930c8ca4e2d0e4bc77a558730fa790be3b5) )
	ROM_LOAD16_BYTE( "136095.40b", 0x000001, 0x80000, CRC(abb80720) SHA1(0e02688454f59b90d38d548808f794294f5c9e7e) )
	ROM_LOAD16_BYTE( "136095.43b", 0x100000, 0x80000, CRC(26526345) SHA1(30ef83f63aca3a846dfc2828e3147208e3e350ca) )
	ROM_LOAD16_BYTE( "136095.42b", 0x100001, 0x80000, CRC(c7a163df) SHA1(deb2c1fe57b6673d98ea9880a16e26751cc6e3a5) )
	ROM_LOAD16_BYTE( "136095.45b", 0x200000, 0x80000, CRC(53d01714) SHA1(6ef584eb6c723d4843dad9e00da132e65be5ce25) )
	ROM_LOAD16_BYTE( "136095.44b", 0x200001, 0x80000, CRC(60a16e4d) SHA1(13629b7039e77a654e22d7efb3bc01413b7ec187) )
	ROM_LOAD16_BYTE( "136095.47b", 0x300000, 0x80000, CRC(41c873a3) SHA1(87c4e35a198ba5f73a545ccd176e9c5d454d194a) )
	ROM_LOAD16_BYTE( "136095.46b", 0x300001, 0x80000, CRC(e885aece) SHA1(f02d2028b74ad26447e4d4c9d4d2d8136e0be8f5) )
	ROM_LOAD16_BYTE( "136095.49b", 0x400000, 0x80000, CRC(7af90faf) SHA1(14629961f70ed969525d9aba49b9637115d26f44) )
	ROM_LOAD16_BYTE( "136095.48b", 0x400001, 0x80000, CRC(6c553406) SHA1(b5a596dc69620d935eab9d9afc45377ad98ba77e) )
	ROM_LOAD16_BYTE( "136095.51b", 0x500000, 0x80000, CRC(97541074) SHA1(f9f75bfc4af9587f4a9630ad93d9cd0efd89e4f4) )
	ROM_LOAD16_BYTE( "136095.50b", 0x500001, 0x80000, CRC(a1c11ae8) SHA1(53fb2f376aae0aa346f9f911d6d8a73753c67d6e) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( spclordsb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136095.21b", 0x00000, 0x20000, CRC(2ba99ce2) SHA1(5d8d138698c29838a85da1721c3400c666a14e18) )
	ROM_LOAD32_BYTE( "136095.22b", 0x00001, 0x20000, CRC(631c5009) SHA1(6b2ea907087e411579f55dff60724ba33afa8a06) )
	ROM_LOAD32_BYTE( "136095.23b", 0x00002, 0x20000, CRC(bc64ab63) SHA1(999851a39123f6a01cb83d97ea744e12590b6e7e) )
	ROM_LOAD32_BYTE( "136095.24b", 0x00003, 0x20000, CRC(7284a01a) SHA1(afa866c97b4c3df7fda3c196072231096beaa0db) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x10000, 0x4000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )
	ROM_CONTINUE(           0x04000, 0xc000 )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41b", 0x000000, 0x80000, CRC(02ce7e07) SHA1(a48a6930c8ca4e2d0e4bc77a558730fa790be3b5) )
	ROM_LOAD16_BYTE( "136095.40b", 0x000001, 0x80000, CRC(abb80720) SHA1(0e02688454f59b90d38d548808f794294f5c9e7e) )
	ROM_LOAD16_BYTE( "136095.43b", 0x100000, 0x80000, CRC(26526345) SHA1(30ef83f63aca3a846dfc2828e3147208e3e350ca) )
	ROM_LOAD16_BYTE( "136095.42b", 0x100001, 0x80000, CRC(c7a163df) SHA1(deb2c1fe57b6673d98ea9880a16e26751cc6e3a5) )
	ROM_LOAD16_BYTE( "136095.45b", 0x200000, 0x80000, CRC(53d01714) SHA1(6ef584eb6c723d4843dad9e00da132e65be5ce25) )
	ROM_LOAD16_BYTE( "136095.44b", 0x200001, 0x80000, CRC(60a16e4d) SHA1(13629b7039e77a654e22d7efb3bc01413b7ec187) )
	ROM_LOAD16_BYTE( "136095.47b", 0x300000, 0x80000, CRC(41c873a3) SHA1(87c4e35a198ba5f73a545ccd176e9c5d454d194a) )
	ROM_LOAD16_BYTE( "136095.46b", 0x300001, 0x80000, CRC(e885aece) SHA1(f02d2028b74ad26447e4d4c9d4d2d8136e0be8f5) )
	ROM_LOAD16_BYTE( "136095.49b", 0x400000, 0x80000, CRC(7af90faf) SHA1(14629961f70ed969525d9aba49b9637115d26f44) )
	ROM_LOAD16_BYTE( "136095.48b", 0x400001, 0x80000, CRC(6c553406) SHA1(b5a596dc69620d935eab9d9afc45377ad98ba77e) )
	ROM_LOAD16_BYTE( "136095.51b", 0x500000, 0x80000, CRC(97541074) SHA1(f9f75bfc4af9587f4a9630ad93d9cd0efd89e4f4) )
	ROM_LOAD16_BYTE( "136095.50b", 0x500001, 0x80000, CRC(a1c11ae8) SHA1(53fb2f376aae0aa346f9f911d6d8a73753c67d6e) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( spclordsg )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "german0.095",  0x000000, 0x020000, CRC(5a885f8e) SHA1(ec7ef0d76320099a6f3fad80c9ec404ce8602557) )
	ROM_LOAD32_BYTE( "german1.095",  0x000001, 0x020000, CRC(56f8d517) SHA1(4bcd2d368d48e7492a739aa3041a40e1518b8c94) )
	ROM_LOAD32_BYTE( "german2.095",  0x000002, 0x020000, CRC(9527df10) SHA1(c18434c1f40fa23a6cc78df7104c7e2e6888d189) )
	ROM_LOAD32_BYTE( "german3.095",  0x000003, 0x020000, CRC(0aaaad66) SHA1(382b859be652d7d83319907d354d294643cef2b4) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x10000, 0x4000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )
	ROM_CONTINUE(           0x04000, 0xc000 )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41a", 0x000000, 0x80000, CRC(5f9743ee) SHA1(fa521572f8dd2eda566f90d1345adba3d0b8c48f) )
	ROM_LOAD16_BYTE( "136095.40a", 0x000001, 0x80000, CRC(99b26863) SHA1(21682771d310c73d4431dde5e72398a69a6f3d53) )
	ROM_LOAD16_BYTE( "136095.43a", 0x100000, 0x80000, CRC(9c0e09a5) SHA1(039dc52318935f686230f57a7b39b9c62280cbf9) )
	ROM_LOAD16_BYTE( "136095.42a", 0x100001, 0x80000, CRC(523bbb39) SHA1(635e859e634f6ef0ba26775653bd38a5ca9ddbbc) )
	ROM_LOAD16_BYTE( "136095.45a", 0x200000, 0x80000, CRC(ac9bf600) SHA1(7da51103c419c7e09992bea67b5413bd2c9a0bb6) )
	ROM_LOAD16_BYTE( "136095.44a", 0x200001, 0x80000, CRC(58949b04) SHA1(c05cc0b691e110532a04e5de43564ddef6c65769) )
	ROM_LOAD16_BYTE( "136095.47a", 0x300000, 0x80000, CRC(675fee50) SHA1(2acbebce99d84ba6029baba48aa0271acb465c1c) )
	ROM_LOAD16_BYTE( "136095.46a", 0x300001, 0x80000, CRC(c948c7b6) SHA1(c5fc684bc8b38370221fe126e6d1461533908cfd) )
	ROM_LOAD16_BYTE( "136095.49a", 0x400000, 0x80000, CRC(04af9a77) SHA1(a985ef7617480e3606068369a1ddb7d90739aeb2) )
	ROM_LOAD16_BYTE( "136095.48a", 0x400001, 0x80000, CRC(5ad113aa) SHA1(71bb40520578447bef1eb78bb60f97a12720ecd9) )
	ROM_LOAD16_BYTE( "136095.51a", 0x500000, 0x80000, CRC(4635c534) SHA1(7261508052e3b17a552b43bc3d4ad7cd2d1f6af9) )
	ROM_LOAD16_BYTE( "136095.50a", 0x500001, 0x80000, CRC(94bde47d) SHA1(dde8f0184a2d7e9f7eb961af2d9d016399ec18fc) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( spclordsa )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136095.21a", 0x00000, 0x20000, CRC(fe8edb0b) SHA1(ae50a637df476c62f8194577cdca2677f9b5cbd0) )
	ROM_LOAD32_BYTE( "136095.22a", 0x00001, 0x20000, CRC(c2d2867b) SHA1(481fe54d6cd8698bfd2776e2af6f51332304b7ba) )
	ROM_LOAD32_BYTE( "136095.23a", 0x00002, 0x20000, CRC(20a0e443) SHA1(54597342901d6b38dddbe754f41ceeddcc4e5289) )
	ROM_LOAD32_BYTE( "136095.24a", 0x00003, 0x20000, CRC(d3f0439c) SHA1(f9245f448b77187b4cd5d9436b5caebd2800be5d))

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x10000, 0x4000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )
	ROM_CONTINUE(           0x04000, 0xc000 )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41a", 0x000000, 0x80000, CRC(5f9743ee) SHA1(fa521572f8dd2eda566f90d1345adba3d0b8c48f) )
	ROM_LOAD16_BYTE( "136095.40a", 0x000001, 0x80000, CRC(99b26863) SHA1(21682771d310c73d4431dde5e72398a69a6f3d53) )
	ROM_LOAD16_BYTE( "136095.43a", 0x100000, 0x80000, CRC(9c0e09a5) SHA1(039dc52318935f686230f57a7b39b9c62280cbf9) )
	ROM_LOAD16_BYTE( "136095.42a", 0x100001, 0x80000, CRC(523bbb39) SHA1(635e859e634f6ef0ba26775653bd38a5ca9ddbbc) )
	ROM_LOAD16_BYTE( "136095.45a", 0x200000, 0x80000, CRC(ac9bf600) SHA1(7da51103c419c7e09992bea67b5413bd2c9a0bb6) )
	ROM_LOAD16_BYTE( "136095.44a", 0x200001, 0x80000, CRC(58949b04) SHA1(c05cc0b691e110532a04e5de43564ddef6c65769) )
	ROM_LOAD16_BYTE( "136095.47a", 0x300000, 0x80000, CRC(675fee50) SHA1(2acbebce99d84ba6029baba48aa0271acb465c1c) )
	ROM_LOAD16_BYTE( "136095.46a", 0x300001, 0x80000, CRC(c948c7b6) SHA1(c5fc684bc8b38370221fe126e6d1461533908cfd) )
	ROM_LOAD16_BYTE( "136095.49a", 0x400000, 0x80000, CRC(04af9a77) SHA1(a985ef7617480e3606068369a1ddb7d90739aeb2) )
	ROM_LOAD16_BYTE( "136095.48a", 0x400001, 0x80000, CRC(5ad113aa) SHA1(71bb40520578447bef1eb78bb60f97a12720ecd9) )
	ROM_LOAD16_BYTE( "136095.51a", 0x500000, 0x80000, CRC(4635c534) SHA1(7261508052e3b17a552b43bc3d4ad7cd2d1f6af9) )
	ROM_LOAD16_BYTE( "136095.50a", 0x500001, 0x80000, CRC(94bde47d) SHA1(dde8f0184a2d7e9f7eb961af2d9d016399ec18fc) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( motofren )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-moto0.23e", 0x000000, 0x020000, CRC(2c6ec446) SHA1(d83fee26b384e6fd783104746e6560504ae43ca6) )
	ROM_LOAD32_BYTE( "136094-moto1.23j", 0x000001, 0x020000, CRC(e7163e7b) SHA1(7ea8a7a63bd1befee4cf9e708949fca7f06572c1) )
	ROM_LOAD32_BYTE( "136094-moto2.37e", 0x000002, 0x020000, CRC(6b1c7626) SHA1(b318a5856bcbd6a8fc7eb92e4b9a576b8c16cbf3) )
	ROM_LOAD32_BYTE( "136094-moto3.37j", 0x000003, 0x020000, CRC(44c3cd2a) SHA1(a16046586cbaa000e056115c92b5f22bf49869ad) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x10000, 0x4000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( motofrenmd )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-0221a.23e", 0x00000, 0x20000, CRC(134e9ff0) SHA1(801b817bf49b4317a7518192025a878b9cd13f7f) )
	ROM_LOAD32_BYTE( "136094-0222a.23j", 0x00001, 0x20000, CRC(f6df65c7) SHA1(0a2092a509ae8c61e3f55c30c47bf39c71e2aa6e) )
	ROM_LOAD32_BYTE( "136094-0223a.37e", 0x00002, 0x20000, CRC(cdb04a4a) SHA1(ee342bdb5654e8b841b1f60e46d1bcae7c4e5cd2) )
	ROM_LOAD32_BYTE( "136094-0224a.37j", 0x00003, 0x20000, CRC(f3a9949f) SHA1(d3fa68fc63c505dd4c9d0e0c7f0625cc24ac9571) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080b.12c", 0x10000, 0x4000, CRC(5e542608) SHA1(8a10b5fac6ac120c7aae2edaa12413c9b8345d87) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* Although verified, the manual states the label codes as 136094-0030 through 136094-0032 */
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )

	ROM_REGION( 0x10000, "clarn", 0 )   /* ADSP2105 (40 MHz) CPU code for communications / Game Link with another PCB */
	ROM_LOAD( "136094-0071a.1j",  0x00000, 0x10000, CRC(089bc0a4) SHA1(677f95aac18fecfc6067d93f488999775889be4c) )
ROM_END


#ifdef UNUSED_DEFINITION
ROM_START( motofrei )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-motoi0.23e",   0x000000, 0x020000, CRC(a2ed9656) SHA1(7473400ee26e72d8ca51dd1f84a6c3bf0c5a72e9) )
	ROM_LOAD32_BYTE( "136094-motoi1.23j",   0x000001, 0x020000, CRC(5ded2f8d) SHA1(df146f110abf3d53f1c968baac1a6fc6e1871aa0) )
	ROM_LOAD32_BYTE( "136094-motoi2.37e",   0x000002, 0x020000, CRC(7a26217f) SHA1(1271a000e2976480a3b959609a5597498886be4f) )
	ROM_LOAD32_BYTE( "136094-motoi3.37j",   0x000003, 0x020000, CRC(ff5ca6ad) SHA1(1e26db56940ce1db819d2179f4ce3962e0b5b732) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x10000, 0x4000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END
#endif

#ifdef UNUSED_DEFINITION
ROM_START( motofreg )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-motog0.23e",   0x000000, 0x020000, CRC(1b205eed) SHA1(e5bcabd6b0b8e7f06e9be0f7f66c86d09840d876) )
	ROM_LOAD32_BYTE( "136094-motog1.23j",   0x000001, 0x020000, CRC(f28e6634) SHA1(2d5d151cbbebdb8691b01398e0be6a08b2bc65ac) )
	ROM_LOAD32_BYTE( "136094-motog2.37e",   0x000002, 0x020000, CRC(01400d54) SHA1(cd539497465857a804b5bc228bb0c93afd1e684e) )
	ROM_LOAD32_BYTE( "136094-motog3.37j",   0x000003, 0x020000, CRC(c467c136) SHA1(9407bdf65ee6261e30227e6b87e2a35da8ee124e) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x10000, 0x4000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END
#endif

#ifdef UNUSED_DEFINITION
ROM_START( motofmdg )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-mdg0.23e", 0x000000, 0x020000, CRC(5839b940) SHA1(0b331869e938a7e344d4a514a8b70d26f5d1fc14) )
	ROM_LOAD32_BYTE( "136094-mdg1.23j", 0x000001, 0x020000, CRC(c46a4104) SHA1(76543fefff535938f11ba7b68e97a786d35f5b82) )
	ROM_LOAD32_BYTE( "136094-mdg2.37e", 0x000002, 0x020000, CRC(0b8bfe6e) SHA1(7220032a07928fd8a887c63ffcab4ec526733cae) )
	ROM_LOAD32_BYTE( "136094-mdg3.37j", 0x000003, 0x020000, CRC(1dcd0d09) SHA1(0f6801694498688ed94588ac4b828ac56f3a16ec) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x10000, 0x4000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END
#endif

ROM_START( motofrenft )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-ft0.23e", 0x000000, 0x020000, CRC(99158754) SHA1(46e73a465aceb147e1ca1bd982448079880cc47e) )
	ROM_LOAD32_BYTE( "136094-ft1.23j", 0x000001, 0x020000, CRC(33c4e205) SHA1(8a223481cfe2aa45a815c6a18017a14502e929b3) )
	ROM_LOAD32_BYTE( "136094-ft2.37e", 0x000002, 0x020000, CRC(30eb94bb) SHA1(b7a2b41570d2110aaedea8a3b9d120af31671bbd) )
	ROM_LOAD32_BYTE( "136094-ft3.37j", 0x000003, 0x020000, CRC(a92e05e3) SHA1(354b6bbb058d10c4da55cb58bf05eae83350ba08) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x10000, 0x4000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( motofrenmf )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-ftmd0.23e", 0x000000, 0x020000, CRC(9be0803e) SHA1(b5e3029ef43adfeafd5979d4ee49a3eb62efd629) )
	ROM_LOAD32_BYTE( "136094-ftmd1.23j", 0x000001, 0x020000, CRC(2a5e9b18) SHA1(af671680047678f86614e23439ac1f0420528343) )
	ROM_LOAD32_BYTE( "136094-ftmd2.37e", 0x000002, 0x020000, CRC(769223fc) SHA1(acfafae3d81a6a3a4ff82c6381590ac31ad80f23) )
	ROM_LOAD32_BYTE( "136094-ftmd3.37j", 0x000003, 0x020000, CRC(96382cc0) SHA1(ba2b6b105c552077767d1185886761fce3ec2885) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x10000, 0x4000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( rrreveng )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "revenge.23e", 0x00000, 0x20000, CRC(3ade13a6) SHA1(672dd0800d6a1cf6cbb2adcebe452a0df71b3236) ) /* Test menu shows 06SEP1994 14:25:13 */
	ROM_LOAD32_BYTE( "revenge.23j", 0x00001, 0x20000, CRC(aff623d5) SHA1(3ad419deb2f40d62f5a6803035c5d08fe82833f4) )
	ROM_LOAD32_BYTE( "revenge.37e", 0x00002, 0x20000, CRC(b5e2a3e2) SHA1(b6ad6d03120ad6699af31d09474b82979ead65bb) )
	ROM_LOAD32_BYTE( "revenge.37j", 0x00003, 0x20000, CRC(6c7f114b) SHA1(2b9a627ec0a211da8080ea33a5486367b043952a) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "rralpl.2d", 0x000000, 0x80000, CRC(00488dad) SHA1(604f08a219db0438dcbf21337ebd497f353bd812) ) /* playfield, planes 0-1 */
	ROM_LOAD( "rralpm.5d", 0x080000, 0x80000, CRC(ade27447) SHA1(641fdca97a4b08251e111425d8467e4640433df7) ) /* playfield, planes 2-3 */
	ROM_LOAD( "rralph.8d", 0x100000, 0x80000, CRC(ef04f04e) SHA1(e518133096978c4a0152253231625c385a84530f) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "alpha.13n", 0x000000, 0x20000, CRC(f2efbd66) SHA1(d5339f0b3de7a102d659f7459b5f4800cab31829) ) /* alphanumerics */

	ROM_REGION16_BE( 0x500000, "rle", 0 )
	ROM_LOAD16_BYTE( "mo0h.31n",    0x000000, 0x80000, CRC(fc2755d5) SHA1(e24319161307efbb3828b21a6250869051f1ccc1) )
	ROM_LOAD16_BYTE( "mo0l.31l",    0x000001, 0x80000, CRC(f9f6bfe3) SHA1(c7e1479bb86646691d5ca7ee9127553cfd86571e) )
	ROM_LOAD16_BYTE( "rrmo1h.33n",  0x100000, 0x80000, CRC(c7a48389) SHA1(a263aa4829cb243440ebe0496dd4f0158d97e2cc) )
	ROM_LOAD16_BYTE( "rrmo1l.33l",  0x100001, 0x80000, CRC(085a67c1) SHA1(0449abb5fec9014bd0ad14ce4802f726a90bc48c) )
	ROM_LOAD16_BYTE( "rrmo2h.35n",  0x200000, 0x80000, CRC(aea35aff) SHA1(2e02d2e877d356f9fb07e8b55ce252a148192d59) )
	ROM_LOAD16_BYTE( "rrmo2l.35l",  0x200001, 0x80000, CRC(b256d6d6) SHA1(0f0a05e3e58a00662f99988a050f2f5a11592d29) )
	ROM_LOAD16_BYTE( "mo3h.37n",    0x300000, 0x80000, CRC(563baaeb) SHA1(e356aa2048c6addefa62d10a17fe5736afa54b16) )
	ROM_LOAD16_BYTE( "mo3l.37l",    0x300001, 0x80000, CRC(e0cf3396) SHA1(544863e5c18cce002ced8f96d5dfaedffdcfff1e) )
	ROM_LOAD16_BYTE( "revenge.31t", 0x400000, 0x80000, CRC(086fb896) SHA1(5ca3aea3a52e73a1054c88759d957709c2ad22a2) )
	ROM_LOAD16_BYTE( "revenge.31r", 0x400001, 0x80000, CRC(518fdd7c) SHA1(ccee646efb178aa3720e75524646e20d18b27694) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) ) /* 74S472AN BPROM */

	ROM_REGION( 0x0600, "pals", 0 ) /* none of these have been verified as good */
	ROM_LOAD( "136094-0019b.3n",  0x0000, 0x0157, CRC(598d5009) SHA1(9804f05fbf1b9324f8c3937e0953da02870d988b) ) /* GAL20V8A */
	ROM_LOAD( "136094-0010a.5n",  0x0000, 0x0117, CRC(87ff6393) SHA1(df1f0a5450485598c0ef7fa4981cc0e40a6a5073) ) /* GAL16V8A */
	ROM_LOAD( "136094-0011b.5r",  0x0000, 0x0117, CRC(832671eb) SHA1(85232128a4b03c4e3dffb4f2e6381a89f4f9aac5) ) /* GAL16V8A */
	ROM_LOAD( "136094-0009a.7n",  0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0012a.7r",  0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0007a.12l", 0x0000, 0x0117, CRC(145b1474) SHA1(f1983732c36a444d38aeba94adaffa305d4c0398) ) /* GAL16V8A */
	ROM_LOAD( "136094-0006a.13r", 0x0000, 0x0117, CRC(d5c84926) SHA1(22d2821ed77ad070163e3d188b1412f8d8d52977) ) /* GAL16V8A */
	ROM_LOAD( "136094-0008a.17l", 0x0000, 0x0117, CRC(b85ab18d) SHA1(eabcc2e54c2b6bc393603a31d22418edf60593ad) ) /* GAL16V8A, hand written "ROAD 2" over label */
	ROM_LOAD( "136094-0014a.22r", 0x0000, 0x0117, CRC(9dc3831d) SHA1(553c289801eb1e15118bc045ddca226343e6a623) ) /* GAL16V8A */
	ROM_LOAD( "136094-0016a.23r", 0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0015a.23s", 0x0000, 0x0117, CRC(9404e122) SHA1(fb1db0fdb10ddeb7247dd254b3e725b9ef85097b) ) /* GAL16V8A */
	ROM_LOAD( "136094-0013a.24c", 0x0000, 0x0117, CRC(11934654) SHA1(a230c4e9abc190a62872961d60f9f96dedb273cd) ) /* GAL16V8A */
	ROM_LOAD( "136094-0018a.24j", 0x0000, 0x0117, CRC(9def4158) SHA1(11c168e2c16046e1213786c065906455fdb5a63c) ) /* GAL16V8A */
	ROM_LOAD( "136094-0017a.25c", 0x0000, 0x0117, CRC(76d8fa5b) SHA1(5fcb7b75f37f918331d99422ea3f0ea202665d5e) ) /* GAL16V8A */

	/* all roms above are from this PCB however the sound board was missing - assumed to be the same */

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "rr65snd.bin", 0x10000, 0x4000, CRC(d78429da) SHA1(a4d36d74986f08c793f15f2e67cb97a8c91c5e90) )
	ROM_CONTINUE(            0x04000, 0xc000 )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "rralpc0.bin",  0x00000, 0x80000, CRC(1f7b6ecf) SHA1(1787a2e89618e1338d70a54684dbc7d44c5f5559) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "rralpc1.bin",  0x00000, 0x80000, CRC(7ccd26d7) SHA1(1a74bdc66482896f5b9795d27383aa993e5fbaa4) )
ROM_END


ROM_START( rrrevenga ) /* Same program roms as the set below, but shares more roms with the most current version (parent) */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "rrprghh.23e", 0x00000, 0x20000, CRC(d2903e9d) SHA1(8782cd6ee39e2159b9ebc68ecdc3ecefcdeb8623) ) /* Test menu shows 27JAN1994 17:02:20 */
	ROM_LOAD32_BYTE( "rrprghl.23j", 0x00001, 0x20000, CRC(1afd500c) SHA1(6d24087a839e5e7d9c764026a9f3089e52785cdb) )
	ROM_LOAD32_BYTE( "rrprglh.37e", 0x00002, 0x20000, CRC(2b03a6fc) SHA1(7c95a0307b854bd37fd327ff1af1b69aa60fb2fd) )
	ROM_LOAD32_BYTE( "rrprgll.37j", 0x00003, 0x20000, CRC(acf078da) SHA1(3506e105d3b208864ce12ab20e6250cb3a0005d6) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "rr65snd.bin", 0x10000, 0x4000, CRC(d78429da) SHA1(a4d36d74986f08c793f15f2e67cb97a8c91c5e90) )
	ROM_CONTINUE(            0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "rralpl.2d", 0x000000, 0x80000, CRC(00488dad) SHA1(604f08a219db0438dcbf21337ebd497f353bd812) ) /* playfield, planes 0-1 */
	ROM_LOAD( "rralpm.5d", 0x080000, 0x80000, CRC(ade27447) SHA1(641fdca97a4b08251e111425d8467e4640433df7) ) /* playfield, planes 2-3 */
	ROM_LOAD( "rralph.8d", 0x100000, 0x80000, CRC(ef04f04e) SHA1(e518133096978c4a0152253231625c385a84530f) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "alpha.13n", 0x000000, 0x20000, CRC(f2efbd66) SHA1(d5339f0b3de7a102d659f7459b5f4800cab31829) ) /* alphanumerics */

	ROM_REGION16_BE( 0x500000, "rle", 0 )
	ROM_LOAD16_BYTE( "mo0h.31n",   0x000000, 0x80000, CRC(fc2755d5) SHA1(e24319161307efbb3828b21a6250869051f1ccc1) )
	ROM_LOAD16_BYTE( "mo0l.31l",   0x000001, 0x80000, CRC(f9f6bfe3) SHA1(c7e1479bb86646691d5ca7ee9127553cfd86571e) )
	ROM_LOAD16_BYTE( "rrmo1h.33n", 0x100000, 0x80000, CRC(c7a48389) SHA1(a263aa4829cb243440ebe0496dd4f0158d97e2cc) )
	ROM_LOAD16_BYTE( "rrmo1l.33l", 0x100001, 0x80000, CRC(085a67c1) SHA1(0449abb5fec9014bd0ad14ce4802f726a90bc48c) )
	ROM_LOAD16_BYTE( "rrmo2h.35n", 0x200000, 0x80000, CRC(aea35aff) SHA1(2e02d2e877d356f9fb07e8b55ce252a148192d59) )
	ROM_LOAD16_BYTE( "rrmo2l.35l", 0x200001, 0x80000, CRC(b256d6d6) SHA1(0f0a05e3e58a00662f99988a050f2f5a11592d29) )
	ROM_LOAD16_BYTE( "mo3h.37n",   0x300000, 0x80000, CRC(563baaeb) SHA1(e356aa2048c6addefa62d10a17fe5736afa54b16) )
	ROM_LOAD16_BYTE( "mo3l.37l",   0x300001, 0x80000, CRC(e0cf3396) SHA1(544863e5c18cce002ced8f96d5dfaedffdcfff1e) )
	ROM_LOAD16_BYTE( "mo4h.31t",   0x400000, 0x80000, CRC(af6a027e) SHA1(08038bddb6aa7e97f013f9d3e508f5501821e460) )
	ROM_LOAD16_BYTE( "mo4l.31r",   0x400001, 0x80000, CRC(9ebc5369) SHA1(ffd8418b328d99aa44fb1aed1db1aa6ac715c644) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "rralpc0.bin",  0x00000, 0x80000, CRC(1f7b6ecf) SHA1(1787a2e89618e1338d70a54684dbc7d44c5f5559) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "rralpc1.bin",  0x00000, 0x80000, CRC(7ccd26d7) SHA1(1a74bdc66482896f5b9795d27383aa993e5fbaa4) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) ) /* 74S472AN BPROM */

	ROM_REGION( 0x0600, "pals", 0 ) /* none of these have been verified as good */
	ROM_LOAD( "136094-0019b.3n",  0x0000, 0x0157, CRC(598d5009) SHA1(9804f05fbf1b9324f8c3937e0953da02870d988b) ) /* GAL20V8A */
	ROM_LOAD( "136094-0010a.5n",  0x0000, 0x0117, CRC(87ff6393) SHA1(df1f0a5450485598c0ef7fa4981cc0e40a6a5073) ) /* GAL16V8A */
	ROM_LOAD( "136094-0011b.5r",  0x0000, 0x0117, CRC(832671eb) SHA1(85232128a4b03c4e3dffb4f2e6381a89f4f9aac5) ) /* GAL16V8A */
	ROM_LOAD( "136094-0009a.7n",  0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0012a.7r",  0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0007a.12l", 0x0000, 0x0117, CRC(145b1474) SHA1(f1983732c36a444d38aeba94adaffa305d4c0398) ) /* GAL16V8A */
	ROM_LOAD( "136094-0006a.13r", 0x0000, 0x0117, CRC(d5c84926) SHA1(22d2821ed77ad070163e3d188b1412f8d8d52977) ) /* GAL16V8A */
	ROM_LOAD( "136094-0008a.17l", 0x0000, 0x0117, CRC(b85ab18d) SHA1(eabcc2e54c2b6bc393603a31d22418edf60593ad) ) /* GAL16V8A, hand written "ROAD 2" over label */
	ROM_LOAD( "136094-0014a.22r", 0x0000, 0x0117, CRC(9dc3831d) SHA1(553c289801eb1e15118bc045ddca226343e6a623) ) /* GAL16V8A */
	ROM_LOAD( "136094-0016a.23r", 0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0015a.23s", 0x0000, 0x0117, CRC(9404e122) SHA1(fb1db0fdb10ddeb7247dd254b3e725b9ef85097b) ) /* GAL16V8A */
	ROM_LOAD( "136094-0013a.24c", 0x0000, 0x0117, CRC(11934654) SHA1(a230c4e9abc190a62872961d60f9f96dedb273cd) ) /* GAL16V8A */
	ROM_LOAD( "136094-0018a.24j", 0x0000, 0x0117, CRC(9def4158) SHA1(11c168e2c16046e1213786c065906455fdb5a63c) ) /* GAL16V8A */
	ROM_LOAD( "136094-0017a.25c", 0x0000, 0x0117, CRC(76d8fa5b) SHA1(5fcb7b75f37f918331d99422ea3f0ea202665d5e) ) /* GAL16V8A */
ROM_END


ROM_START( rrrevengb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "rrprghh.23e", 0x00000, 0x20000, CRC(d2903e9d) SHA1(8782cd6ee39e2159b9ebc68ecdc3ecefcdeb8623) )
	ROM_LOAD32_BYTE( "rrprghl.23j", 0x00001, 0x20000, CRC(1afd500c) SHA1(6d24087a839e5e7d9c764026a9f3089e52785cdb) )
	ROM_LOAD32_BYTE( "rrprglh.37e", 0x00002, 0x20000, CRC(2b03a6fc) SHA1(7c95a0307b854bd37fd327ff1af1b69aa60fb2fd) )
	ROM_LOAD32_BYTE( "rrprgll.37j", 0x00003, 0x20000, CRC(acf078da) SHA1(3506e105d3b208864ce12ab20e6250cb3a0005d6) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "rr65snd.bin", 0x10000, 0x4000, CRC(d78429da) SHA1(a4d36d74986f08c793f15f2e67cb97a8c91c5e90) )
	ROM_CONTINUE(            0x04000, 0xc000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "rralpl.2d", 0x000000, 0x80000, CRC(00488dad) SHA1(604f08a219db0438dcbf21337ebd497f353bd812) ) /* playfield, planes 0-1 */
	ROM_LOAD( "rralpm.5d", 0x080000, 0x80000, CRC(ade27447) SHA1(641fdca97a4b08251e111425d8467e4640433df7) ) /* playfield, planes 2-3 */
	ROM_LOAD( "rralph.8d", 0x100000, 0x80000, CRC(ef04f04e) SHA1(e518133096978c4a0152253231625c385a84530f) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "rralalph.13n", 0x000000, 0x20000, CRC(7ca93790) SHA1(5e2f069be4b15d63f418c8693e8550eb0ae22381) ) /* alphanumerics */

	ROM_REGION16_BE( 0x500000, "rle", 0 )
	ROM_LOAD16_BYTE( "rrmo0h.31n", 0x000000, 0x80000, CRC(9b7e0315) SHA1(856804e89f586a4e777da5f47dc29c4e34175f44) )
	ROM_LOAD16_BYTE( "rrmo0l.31l", 0x000001, 0x80000, CRC(10478697) SHA1(9682e82cfbdc20f63d5c49303265c598a334c15b) )
	ROM_LOAD16_BYTE( "rrmo1h.33n", 0x100000, 0x80000, CRC(c7a48389) SHA1(a263aa4829cb243440ebe0496dd4f0158d97e2cc) )
	ROM_LOAD16_BYTE( "rrmo1l.33l", 0x100001, 0x80000, CRC(085a67c1) SHA1(0449abb5fec9014bd0ad14ce4802f726a90bc48c) )
	ROM_LOAD16_BYTE( "rrmo2h.35n", 0x200000, 0x80000, CRC(aea35aff) SHA1(2e02d2e877d356f9fb07e8b55ce252a148192d59) )
	ROM_LOAD16_BYTE( "rrmo2l.35l", 0x200001, 0x80000, CRC(b256d6d6) SHA1(0f0a05e3e58a00662f99988a050f2f5a11592d29) )
	ROM_LOAD16_BYTE( "rrmo3h.37n", 0x300000, 0x80000, CRC(e02549d7) SHA1(8b373d4969c77a14b15588f8a7e2977277fc9752) )
	ROM_LOAD16_BYTE( "rrmo3l.37l", 0x300001, 0x80000, CRC(8c81b537) SHA1(16cf8804687590aedf9020f9619bd14240f3b628) )
	ROM_LOAD16_BYTE( "rrmo4h.31t", 0x400000, 0x80000, CRC(12bf3e11) SHA1(37b1a7fe0b50202030f5c1938b95a449bbd51add) )
	ROM_LOAD16_BYTE( "rrmo4l.31r", 0x400001, 0x80000, CRC(a80175f6) SHA1(db621902fdfa99ec532713f4314c6cbb8353a773) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "rralpc0.bin",  0x00000, 0x80000, CRC(1f7b6ecf) SHA1(1787a2e89618e1338d70a54684dbc7d44c5f5559) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "rralpc1.bin",  0x00000, 0x80000, CRC(7ccd26d7) SHA1(1a74bdc66482896f5b9795d27383aa993e5fbaa4) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) ) /* 74S472AN BPROM */

	ROM_REGION( 0x0600, "pals", 0 ) /* none of these have been verified as good */
	ROM_LOAD( "136094-0019b.3n",  0x0000, 0x0157, CRC(598d5009) SHA1(9804f05fbf1b9324f8c3937e0953da02870d988b) ) /* GAL20V8A */
	ROM_LOAD( "136094-0010a.5n",  0x0000, 0x0117, CRC(87ff6393) SHA1(df1f0a5450485598c0ef7fa4981cc0e40a6a5073) ) /* GAL16V8A */
	ROM_LOAD( "136094-0011b.5r",  0x0000, 0x0117, CRC(832671eb) SHA1(85232128a4b03c4e3dffb4f2e6381a89f4f9aac5) ) /* GAL16V8A */
	ROM_LOAD( "136094-0009a.7n",  0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0012a.7r",  0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0007a.12l", 0x0000, 0x0117, CRC(145b1474) SHA1(f1983732c36a444d38aeba94adaffa305d4c0398) ) /* GAL16V8A */
	ROM_LOAD( "136094-0006a.13r", 0x0000, 0x0117, CRC(d5c84926) SHA1(22d2821ed77ad070163e3d188b1412f8d8d52977) ) /* GAL16V8A */
	ROM_LOAD( "136094-0008a.17l", 0x0000, 0x0117, CRC(b85ab18d) SHA1(eabcc2e54c2b6bc393603a31d22418edf60593ad) ) /* GAL16V8A, hand written "ROAD 2" over label */
	ROM_LOAD( "136094-0014a.22r", 0x0000, 0x0117, CRC(9dc3831d) SHA1(553c289801eb1e15118bc045ddca226343e6a623) ) /* GAL16V8A */
	ROM_LOAD( "136094-0016a.23r", 0x0000, 0x0117, CRC(87d3a1d6) SHA1(33437f6b39b263a3064b34da41a7eed922036a56) ) /* GAL16V8A */
	ROM_LOAD( "136094-0015a.23s", 0x0000, 0x0117, CRC(9404e122) SHA1(fb1db0fdb10ddeb7247dd254b3e725b9ef85097b) ) /* GAL16V8A */
	ROM_LOAD( "136094-0013a.24c", 0x0000, 0x0117, CRC(11934654) SHA1(a230c4e9abc190a62872961d60f9f96dedb273cd) ) /* GAL16V8A */
	ROM_LOAD( "136094-0018a.24j", 0x0000, 0x0117, CRC(9def4158) SHA1(11c168e2c16046e1213786c065906455fdb5a63c) ) /* GAL16V8A */
	ROM_LOAD( "136094-0017a.25c", 0x0000, 0x0117, CRC(76d8fa5b) SHA1(5fcb7b75f37f918331d99422ea3f0ea202665d5e) ) /* GAL16V8A */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(atarigx2_state,spclords)
{
	m_playfield_base = 0x000;
}


DRIVER_INIT_MEMBER(atarigx2_state,motofren)
{
	m_playfield_base = 0x400;
/*
L/W=!68.A23*!E.A22*!E.A21                                       = 000x xxxx = 000000-1fffff
   +68.A23*E.A22*E.A21*68.A20*68.A19*68.A18*68.A17              = 1111 111x = fe0000-ffffff

68.XIO=68.A23*E.A22*E.A21*!68.A20*!68.A18*!68.A17*!68.A16       = 1110 x000 = e00000-e0ffff, e80000-e8ffff
    +68.A23*E.A22*E.A21*!68.A20*!68.A19                         = 1110 0xxx = e00000-e7ffff

68.ROM=!68.A23*68.AS*!E.A22*!E.A21                              = 000x xxxx = 000000-1fffff

68.EXT=68.A23*68.AS*E.A22*!E.A21*!68.A20                        = 1100 xxxx = c00000-cfffff

68.WRAM=68.A23*68.AS*E.A22*E.A21*68.A20*68.A19*68.A18*68.A17    = 1111 111x = fe0000-ffffff

XMEM=68.A23*E.A22*!E.A21*68.A20                                 = 1101 xxxx = d00000-dfffff

68.W1=68.A23*E.A22*!E.A21*68.A20*!68.A18                        = 1101 x0xx = d00000-d3ffff, d80000-dbffff

68.W0=68.A23*E.A22*!E.A21*68.A20*!68.A18                        = 1101 x0xx = d00000-d3ffff, d80000-dbffff
    +68.A23*E.A22*!E.A21*68.A20*!68.A19                         = 1101 0xxx = d00000-d7ffff
    +68.A23*E.A22*!E.A21*!68.A20*68.A19                         = 1100 1xxx = c80000-cfffff
    +!68.A23*!E.A22*!E.A21                                      = 000x xxxx = 000000-1fffff
*/
}

READ32_MEMBER(atarigx2_state::rrreveng_prot_r)
{
	return 0;
}

DRIVER_INIT_MEMBER(atarigx2_state,rrreveng)
{
	m_playfield_base = 0x000;

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xca0fc0, 0xca0fc3, read32_delegate(FUNC(atarigx2_state::rrreveng_prot_r),this));
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, spclords,  0,        atarigx2_0x400, spclords, atarigx2_state, spclords, ROT0, "Atari Games", "Space Lords (rev C)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, spclordsb, spclords, atarigx2_0x400, spclords, atarigx2_state, spclords, ROT0, "Atari Games", "Space Lords (rev B)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, spclordsg, spclords, atarigx2_0x400, spclords, atarigx2_state, spclords, ROT0, "Atari Games", "Space Lords (rev A, German)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, spclordsa, spclords, atarigx2_0x400, spclords, atarigx2_state, spclords, ROT0, "Atari Games", "Space Lords (rev A)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )

GAME( 1992, motofren,   0,        atarigx2_0x200, motofren, atarigx2_state, motofren, ROT0, "Atari Games", "Moto Frenzy", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, motofrenmd, motofren, atarigx2_0x200, motofren, atarigx2_state, motofren, ROT0, "Atari Games", "Moto Frenzy (Mini Deluxe)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, motofrenft, motofren, atarigx2_0x200, motofren, atarigx2_state, motofren, ROT0, "Atari Games", "Moto Frenzy (Field Test Version)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, motofrenmf, motofren, atarigx2_0x200, motofren, atarigx2_state, motofren, ROT0, "Atari Games", "Moto Frenzy (Mini Deluxe Field Test Version)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )

GAME( 1993, rrreveng,  0,        atarigx2_0x400, rrreveng, atarigx2_state, rrreveng, ROT0, "Atari Games", "Road Riot's Revenge (prototype, Sep 06, 1994)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1993, rrrevenga, rrreveng, atarigx2_0x400, rrreveng, atarigx2_state, rrreveng, ROT0, "Atari Games", "Road Riot's Revenge (prototype, Jan 27, 1994, set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1993, rrrevengb, rrreveng, atarigx2_0x400, rrreveng, atarigx2_state, rrreveng, ROT0, "Atari Games", "Road Riot's Revenge (prototype, Jan 27, 1994, set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
