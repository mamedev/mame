/******************************************************************************
    Atari 400/800

    ANTIC video controller

    Juergen Buchmueller, June 1998
******************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)	/* x */
#endif

ANTIC antic;

/**************************************************************
 *
 * Reset ANTIC
 *
 **************************************************************/

void antic_reset(void)
{
	/* reset the ANTIC read / write registers */
	memset(&antic.r, 0, sizeof(antic.r));
	memset(&antic.w, 0, sizeof(antic.w));
	antic.r.antic00 = 0xff;
	antic.r.antic01 = 0xff;
	antic.r.antic02 = 0xff;
	antic.r.antic03 = 0xff;
	antic.r.antic04 = 0xff;
	antic.r.antic05 = 0xff;
	antic.r.antic06 = 0xff;
	antic.r.antic07 = 0xff;
	antic.r.antic08 = 0xff;
	antic.r.antic09 = 0xff;
	antic.r.antic0a = 0xff;
	antic.r.penh	= 0x00;
	antic.r.penv	= 0x00;
	antic.r.antic0e = 0xff;
	antic.r.nmist	= 0x1f;
}

/**************************************************************
 *
 * Read ANTIC hardware registers
 *
 **************************************************************/
READ8_HANDLER ( atari_antic_r )
{
	UINT8 data = 0xff;

	switch (offset & 15)
	{
	case  0: /* nothing */
		data = antic.r.antic00;
		break;
	case  1: /* nothing */
		data = antic.r.antic01;
		break;
	case  2: /* nothing */
		data = antic.r.antic02;
		break;
	case  3: /* nothing */
		data = antic.r.antic03;
		break;
	case  4: /* nothing */
		data = antic.r.antic04;
		break;
	case  5: /* nothing */
		data = antic.r.antic05;
		break;
	case  6: /* nothing */
		data = antic.r.antic06;
		break;
	case  7: /* nothing */
		data = antic.r.antic07;
		break;
	case  8: /* nothing */
		data = antic.r.antic08;
		break;
	case  9: /* nothing */
		data = antic.r.antic09;
		break;
	case 10: /* WSYNC read */
		cpunum_spinuntil_trigger(0, TRIGGER_HSYNC);
		antic.w.wsync = 1;
		data = antic.r.antic0a;
		break;
	case 11: /* vert counter (scanline / 2) */
		data = antic.r.vcount = antic.scanline >> 1;
		break;
	case 12: /* light pen horz pos */
		data = antic.r.penh;
		break;
	case 13: /* light pen vert pos */
		data = antic.r.penv;
		break;
	case 14: /* NMI enable */
		data = antic.r.antic0e;
		break;
	case 15: /* NMI status */
		data = antic.r.nmist;
		break;
	}
	return data;
}

/**************************************************************
 *
 * Write ANTIC hardware registers
 *
 **************************************************************/

WRITE8_HANDLER ( atari_antic_w )
{
	int temp;

	switch (offset & 15)
	{
	case  0:
		if( data == antic.w.dmactl )
			break;
		LOG(("ANTIC 00 write DMACTL $%02X\n", data));
		antic.w.dmactl = data;
		switch (data & 3)
		{
			case 0: antic.pfwidth =  0; break;
			case 1: antic.pfwidth = 32; break;
			case 2: antic.pfwidth = 40; break;
			case 3: antic.pfwidth = 48; break;
		}
		break;
	case  1:
		if( data == antic.w.chactl )
			break;
		LOG(("ANTIC 01 write CHACTL $%02X\n", data));
		antic.w.chactl = data;
		antic.chand = (data & 1) ? 0x00 : 0xff;
		antic.chxor = (data & 2) ? 0xff : 0x00;
		break;
	case  2:
		LOG(("ANTIC 02 write DLISTL $%02X\n", data));
		antic.w.dlistl = data;
		temp = (antic.w.dlisth << 8) + antic.w.dlistl;
		antic.dpage = temp & DPAGE;
		antic.doffs = temp & DOFFS;
		break;
	case  3:
		LOG(("ANTIC 03 write DLISTH $%02X\n", data));
		antic.w.dlisth = data;
		temp = (antic.w.dlisth << 8) + antic.w.dlistl;
		antic.dpage = temp & DPAGE;
		antic.doffs = temp & DOFFS;
		break;
	case  4:
		if( data == antic.w.hscrol )
			break;
		LOG(("ANTIC 04 write HSCROL $%02X\n", data));
		antic.w.hscrol = data & 15;
		break;
	case  5:
		if( data == antic.w.vscrol )
			break;
		LOG(("ANTIC 05 write VSCROL $%02X\n", data));
		antic.w.vscrol = data & 15;
		break;
	case  6:
		if( data == antic.w.pmbasl )
			break;
		LOG(("ANTIC 06 write PMBASL $%02X\n", data));
		/* antic.w.pmbasl = data; */
		break;
	case  7:
		if( data == antic.w.pmbash )
			break;
		LOG(("ANTIC 07 write PMBASH $%02X\n", data));
		antic.w.pmbash = data;
		antic.pmbase_s = (data & 0xfc) << 8;
		antic.pmbase_d = (data & 0xf8) << 8;
		break;
	case  8:
		if( data == antic.w.chbasl )
			break;
		LOG(("ANTIC 08 write CHBASL $%02X\n", data));
		/* antic.w.chbasl = data; */
		break;
	case  9:
		if( data == antic.w.chbash )
			break;
		LOG(("ANTIC 09 write CHBASH $%02X\n", data));
		antic.w.chbash = data;
		break;
	case 10: /* WSYNC write */
		LOG(("ANTIC 0A write WSYNC  $%02X\n", data));
		cpunum_spinuntil_trigger(0, TRIGGER_HSYNC);
		antic.w.wsync = 1;
		break;
	case 11:
		if( data == antic.w.antic0b )
			break;
		LOG(("ANTIC 0B write ?????? $%02X\n", data));
		antic.w.antic0b = data;
		break;
	case 12:
		if( data == antic.w.antic0c )
			break;
		LOG(("ANTIC 0C write ?????? $%02X\n", data));
		antic.w.antic0c = data;
		break;
	case 13:
		if( data == antic.w.antic0d )
			break;
		LOG(("ANTIC 0D write ?????? $%02X\n", data));
		antic.w.antic0d = data;
		break;
	case 14:
		if( data == antic.w.nmien )
			break;
		LOG(("ANTIC 0E write NMIEN  $%02X\n", data));
		antic.w.nmien  = data;
		break;
	case 15:
		LOG(("ANTIC 0F write NMIRES $%02X\n", data));
		antic.r.nmist = 0x1f;
		antic.w.nmires = data;
		break;
	}
}

/*************  ANTIC mode 00: *********************************
 * generate 1-8 empty scanlines
 ***************************************************************/
void antic_mode_0_xx(VIDEO *video)
{
	PREPARE();
	memset(dst, PBK, HWIDTH*4);
	POST();
}

/*************  ANTIC mode 01: *********************************
 * display list jump, eventually wait for vsync
 ***************************************************************/


/*************  ANTIC mode 02: *********************************
 * character mode 8x8:2 (32/40/48 byte per line)
 ***************************************************************/
#define MODE2(s) COPY4(dst, antic.pf_21[video->data[s]])

void antic_mode_2_32(VIDEO *video)
{
	PREPARE_TXT2(32);
	REP32(MODE2);
	POST_TXT(32);
}
void antic_mode_2_40(VIDEO *video)
{
	PREPARE_TXT2(40);
	REP40(MODE2);
	POST_TXT(40);
}
void antic_mode_2_48(VIDEO *video)
{
	PREPARE_TXT2(48);
	REP48(MODE2);
	POST_TXT(48);
}

/*************  ANTIC mode 03: *********************************
 * character mode 8x10:2 (32/40/48 byte per line)
 ***************************************************************/
#define MODE3(s) COPY4(dst, antic.pf_21[video->data[s]])

void antic_mode_3_32(VIDEO *video)
{
	PREPARE_TXT3(32);
	REP32(MODE3);
	POST_TXT(32);
}
void antic_mode_3_40(VIDEO *video)
{
	PREPARE_TXT3(40);
	REP40(MODE3);
	POST_TXT(40);
}
void antic_mode_3_48(VIDEO *video)
{
	PREPARE_TXT3(48);
	REP48(MODE3);
	POST_TXT(48);
}

/*************  ANTIC mode 04: *********************************
 * character mode 8x8:4 multi color (32/40/48 byte per line)
 ***************************************************************/
#define MODE4(s) COPY4(dst, antic.pf_x10b[video->data[s]])

void antic_mode_4_32(VIDEO *video)
{
	PREPARE_TXT45(32,0);
	REP32(MODE4);
	POST_TXT(32);
}
void antic_mode_4_40(VIDEO *video)
{
	PREPARE_TXT45(40,0);
	REP40(MODE4);
	POST_TXT(40);
}
void antic_mode_4_48(VIDEO *video)
{
	PREPARE_TXT45(48,0);
	REP48(MODE4);
	POST_TXT(48);
}

/*************  ANTIC mode 05: *********************************
 * character mode 8x16:4 multi color (32/40/48 byte per line)
 ***************************************************************/
#define MODE5(s) COPY4(dst, antic.pf_x10b[video->data[s]])

void antic_mode_5_32(VIDEO *video)
{
	PREPARE_TXT45(32,1);
	REP32(MODE5);
	POST_TXT(32);
}
void antic_mode_5_40(VIDEO *video)
{
	PREPARE_TXT45(40,1);
	REP40(MODE5);
	POST_TXT(40);
}
void antic_mode_5_48(VIDEO *video)
{
	PREPARE_TXT45(48,1);
	REP48(MODE5);
	POST_TXT(48);
}

/*************  ANTIC mode 06: *********************************
 * character mode 16x8:5 single color (16/20/24 byte per line)
 ***************************************************************/
#define MODE6(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

void antic_mode_6_32(VIDEO *video)
{
	PREPARE_TXT67(16,0);
	REP16(MODE6);
	POST_TXT(16);
}
void antic_mode_6_40(VIDEO *video)
{
	PREPARE_TXT67(20,0);
	REP20(MODE6);
	POST_TXT(20);
}
void antic_mode_6_48(VIDEO *video)
{
	PREPARE_TXT67(24,0);
	REP24(MODE6);
	POST_TXT(24);
}

/*************  ANTIC mode 07: *********************************
 * character mode 16x16:5 single color (16/20/24 byte per line)
 ***************************************************************/
#define MODE7(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

void antic_mode_7_32(VIDEO *video)
{
	PREPARE_TXT67(16,1);
	REP16(MODE7);
	POST_TXT(16);
}
void antic_mode_7_40(VIDEO *video)
{
	PREPARE_TXT67(20,1);
	REP20(MODE7);
	POST_TXT(20);
}
void antic_mode_7_48(VIDEO *video)
{
	PREPARE_TXT67(24,1);
	REP24(MODE7);
	POST_TXT(24);
}

/*************  ANTIC mode 08: *********************************
 * graphics mode 8x8:4 (8/10/12 byte per line)
 ***************************************************************/
#define MODE8(s) COPY16(dst, antic.pf_210b4[video->data[s]],antic.pf_210b4[video->data[s]+1],antic.pf_210b4[video->data[s]+2],antic.pf_210b4[video->data[s]+3])

void antic_mode_8_32(VIDEO *video)
{
	PREPARE_GFX8(8);
	REP08(MODE8);
	POST_GFX(8);
}
void antic_mode_8_40(VIDEO *video)
{
	PREPARE_GFX8(10);
	REP10(MODE8);
	POST_GFX(10);
}
void antic_mode_8_48(VIDEO *video)
{
	PREPARE_GFX8(12);
	REP12(MODE8);
	POST_GFX(12);
}

/*************  ANTIC mode 09: *********************************
 * graphics mode 4x4:2 (8/10/12 byte per line)
 ***************************************************************/
#define MODE9(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

void antic_mode_9_32(VIDEO *video)
{
	PREPARE_GFX9BC(16);
	REP16(MODE9);
	POST_GFX(16);
}
void antic_mode_9_40(VIDEO *video)
{
	PREPARE_GFX9BC(20);
	REP20(MODE9);
	POST_GFX(20);
}
void antic_mode_9_48(VIDEO *video)
{
	PREPARE_GFX9BC(24);
	REP24(MODE9);
	POST_GFX(24);
}

/*************  ANTIC mode 0A: *********************************
 * graphics mode 4x4:4 (16/20/24 byte per line)
 ***************************************************************/
#define MODEA(s) COPY8(dst, antic.pf_210b2[video->data[s]], antic.pf_210b2[video->data[s]+1])

void antic_mode_a_32(VIDEO *video)
{
	PREPARE_GFXA(16);
	REP16(MODEA);
	POST_GFX(16);
}
void antic_mode_a_40(VIDEO *video)
{
	PREPARE_GFXA(20);
	REP20(MODEA);
	POST_GFX(20);
}
void antic_mode_a_48(VIDEO *video)
{
	PREPARE_GFXA(24);
	REP24(MODEA);
	POST_GFX(24);
}

/*************  ANTIC mode 0B: *********************************
 * graphics mode 2x2:2 (16/20/24 byte per line)
 ***************************************************************/
#define MODEB(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

void antic_mode_b_32(VIDEO *video)
{
	PREPARE_GFX9BC(16);
	REP16(MODEB);
	POST_GFX(16);
}
void antic_mode_b_40(VIDEO *video)
{
	PREPARE_GFX9BC(20);
	REP20(MODEB);
	POST_GFX(20);
}
void antic_mode_b_48(VIDEO *video)
{
	PREPARE_GFX9BC(24);
	REP24(MODEB);
	POST_GFX(24);
}

/*************  ANTIC mode 0C: *********************************
 * graphics mode 2x1:2 (16/20/24 byte per line)
 ***************************************************************/
#define MODEC(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

void antic_mode_c_32(VIDEO *video)
{
	PREPARE_GFX9BC(16);
	REP16(MODEC);
	POST_GFX(16);
}
void antic_mode_c_40(VIDEO *video)
{
	PREPARE_GFX9BC(20);
	REP20(MODEC);
	POST_GFX(20);
}
void antic_mode_c_48(VIDEO *video)
{
	PREPARE_GFX9BC(24);
	REP24(MODEC);
	POST_GFX(24);
}

/*************  ANTIC mode 0D: *********************************
 * graphics mode 2x2:4 (32/40/48 byte per line)
 ***************************************************************/
#define MODED(s) COPY4(dst, antic.pf_x10b[video->data[s]])

void antic_mode_d_32(VIDEO *video)
{
	PREPARE_GFXDE(32);
	REP32(MODED);
	POST_GFX(32);
}
void antic_mode_d_40(VIDEO *video)
{
	PREPARE_GFXDE(40);
	REP40(MODED);
	POST_GFX(40);
}
void antic_mode_d_48(VIDEO *video)
{
	PREPARE_GFXDE(48);
	REP48(MODED);
	POST_GFX(48);
}

/*************  ANTIC mode 0E: *********************************
 * graphics mode 2x1:4 (32/40/48 byte per line)
 ***************************************************************/
#define MODEE(s) COPY4(dst, antic.pf_x10b[video->data[s]])

void antic_mode_e_32(VIDEO *video)
{
	PREPARE_GFXDE(32);
	REP32(MODEE);
	POST_GFX(32);
}
void antic_mode_e_40(VIDEO *video)
{
	PREPARE_GFXDE(40);
	REP40(MODEE);
	POST_GFX(40);
}
void antic_mode_e_48(VIDEO *video)
{
	PREPARE_GFXDE(48);
	REP48(MODEE);
	POST_GFX(48);
}

/*************  ANTIC mode 0F: *********************************
 * graphics mode 1x1:2 (32/40/48 byte per line)
 ***************************************************************/
#define MODEF(s) COPY4(dst, antic.pf_1b[video->data[s]])

void antic_mode_f_32(VIDEO *video)
{
	PREPARE_GFXF(32);
	REP32(MODEF);
	POST_GFX(32);
}
void antic_mode_f_40(VIDEO *video)
{
	PREPARE_GFXF(40);
	REP40(MODEF);
	POST_GFX(40);
}
void antic_mode_f_48(VIDEO *video)
{
	PREPARE_GFXF(48);
	REP48(MODEF);
	POST_GFX(48);
}

