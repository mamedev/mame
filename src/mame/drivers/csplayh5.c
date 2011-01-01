/***************************************************************************

    'High Rate DVD' HW (c) 1998 Nichibutsu

    preliminary driver by Angelo Salese

    TODO:
    - fix h8 CPU core bugs, it trips various unhandled opcodes
    - Implement DVD routing and YUV decoding;
    - V9958 timings are screwed, some places have missing gfxs.
    - game timings seem busted either, could be due of missing DVD hook-up

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"
#include "video/v9938.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "sound/dac.h"
#include "sound/3812intf.h"
#include "cpu/z80/z80daisy.h"
#include "machine/nvram.h"
#include "cpu/h83002/h8.h"

static bitmap_t *vdp0_bitmap;

#define USE_H8 0

// from MSX2 driver, may be not accurate for this HW
#define MSX2_XBORDER_PIXELS		16
#define MSX2_YBORDER_PIXELS		28
#define MSX2_TOTAL_XRES_PIXELS		256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS		212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS	8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS	14 * 2

static void csplayh5_vdp0_interrupt(running_machine *machine, int i)
{
	/* this is not used as the v9938 interrupt callbacks are broken
       interrupts seem to be fired quite randomly */
}

static VIDEO_START( csplayh5 )
{
	vdp0_bitmap = machine->primary_screen->alloc_compatible_bitmap();
	v9938_init (machine, 0, *machine->primary_screen, vdp0_bitmap, MODEL_V9958, 0x20000, csplayh5_vdp0_interrupt);
	v9938_reset(0);
}

static VIDEO_UPDATE( csplayh5 )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	copybitmap(bitmap, vdp0_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

static UINT16 mux_data;

static READ16_HANDLER( csplayh5_mux_r )
{
	switch(mux_data)
	{
		case 0x01: return input_port_read(space->machine, "KEY0");
		case 0x02: return input_port_read(space->machine, "KEY1");
		case 0x04: return input_port_read(space->machine, "KEY2");
		case 0x08: return input_port_read(space->machine, "KEY3");
		case 0x10: return input_port_read(space->machine, "KEY4");
	}

	return 0xffff;
}

static WRITE16_HANDLER( csplayh5_mux_w )
{
	mux_data = (~data & 0x1f);
}

static WRITE16_HANDLER( csplayh5_sound_w )
{
	soundlatch_w(space, 0, ((data >> 8) & 0xff));
}


static ADDRESS_MAP_START( csplayh5_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("DSW") AM_WRITE(csplayh5_sound_w)
	AM_RANGE(0x200200, 0x200201) AM_READWRITE(csplayh5_mux_r,csplayh5_mux_w)
	AM_RANGE(0x200400, 0x200401) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x200600, 0x200601) AM_READWRITE8(v9938_0_vram_r, v9938_0_vram_w,0xff)
	AM_RANGE(0x200602, 0x200603) AM_READWRITE8(v9938_0_status_r, v9938_0_command_w,0xff)
	AM_RANGE(0x200604, 0x200605) AM_WRITE8(v9938_0_palette_w,0xff)
	AM_RANGE(0x200606, 0x200607) AM_WRITE8(v9938_0_register_w,0xff)

	AM_RANGE(0x800000, 0xbfffff) AM_ROM AM_REGION("blit_gfx",0) // GFX ROM routes here

	AM_RANGE(0xc80000, 0xcfffff) AM_RAM AM_SHARE("nvram") // work RAM

	AM_RANGE(0xfffc00, 0xffffff) AM_READWRITE(tmp68301_regs_r, tmp68301_regs_w)	// TMP68301 Registers
ADDRESS_MAP_END

#if USE_H8
static READ16_HANDLER( test_r )
{
	return space->machine->rand();
}

static ADDRESS_MAP_START( csplayh5_sub_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM

	AM_RANGE(0x04002a, 0x04002b) AM_READ(test_r)
	AM_RANGE(0x040036, 0x040037) AM_READ(test_r)

	AM_RANGE(0x078000, 0x07ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x080000, 0x0fffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( csplayh5_sub_io_map, ADDRESS_SPACE_IO, 16 )

ADDRESS_MAP_END
#endif


/*
sound HW is identical to Niyanpai
*/

/* TMPZ84C011 PIO emulation */
static UINT8 pio_dir[5], pio_latch[5];

#define SIGNED_DAC	0		// 0:unsigned DAC, 1:signed DAC
#if SIGNED_DAC
#define DAC_WRITE	dac_signed_w
#else
#define DAC_WRITE	dac_w
#endif

static void csplayh5_soundbank_w(running_machine *machine, int data)
{
	UINT8 *SNDROM = machine->region("audiocpu")->base();

	memory_set_bankptr(machine, "bank1", &SNDROM[0x08000 + (0x8000 * (data & 0x03))]);
}

static READ8_HANDLER( csplayh5_sound_r )
{
	return soundlatch_r(space, 0);
}

static WRITE8_HANDLER( csplayh5_soundclr_w )
{
	soundlatch_clear_w(space, 0, 0);
}

static READ8_HANDLER( tmpz84c011_pio_r )
{
	int portdata;

	switch (offset)
	{
		case 0:			/* PA_0 */
			portdata = 0xff;
			break;
		case 1:			/* PB_0 */
			portdata = 0xff;
			break;
		case 2:			/* PC_0 */
			portdata = 0xff;
			break;
		case 3:			/* PD_0 */
			portdata = csplayh5_sound_r(space, 0);
			break;
		case 4:			/* PE_0 */
			portdata = 0xff;
			break;

		default:
			logerror("%s: TMPZ84C011_PIO Unknown Port Read %02X\n", cpuexec_describe_context(space->machine), offset);
			portdata = 0xff;
			break;
	}

	return portdata;
}

static WRITE8_HANDLER( tmpz84c011_pio_w)
{
	switch (offset)
	{
		case 0:			/* PA_0 */
			csplayh5_soundbank_w(space->machine, data & 0x03);
			break;
		case 1:			/* PB_0 */
			DAC_WRITE(space->machine->device("dac2"), 0, data);
			break;
		case 2:			/* PC_0 */
			DAC_WRITE(space->machine->device("dac1"), 0, data);
			break;
		case 3:			/* PD_0 */
			break;
		case 4:			/* PE_0 */
			if (!(data & 0x01)) csplayh5_soundclr_w(space, 0, 0);
			break;

		default:
			logerror("%s: TMPZ84C011_PIO Unknown Port Write %02X, %02X\n", cpuexec_describe_context(space->machine), offset, data);
			break;
	}
}


/* CPU interface */
static READ8_HANDLER( tmpz84c011_0_pa_r )	{ return (tmpz84c011_pio_r(space,0) & ~pio_dir[0]) | (pio_latch[0] & pio_dir[0]); }
static READ8_HANDLER( tmpz84c011_0_pb_r )	{ return (tmpz84c011_pio_r(space,1) & ~pio_dir[1]) | (pio_latch[1] & pio_dir[1]); }
static READ8_HANDLER( tmpz84c011_0_pc_r )	{ return (tmpz84c011_pio_r(space,2) & ~pio_dir[2]) | (pio_latch[2] & pio_dir[2]); }
static READ8_HANDLER( tmpz84c011_0_pd_r )	{ return (tmpz84c011_pio_r(space,3) & ~pio_dir[3]) | (pio_latch[3] & pio_dir[3]); }
static READ8_HANDLER( tmpz84c011_0_pe_r )	{ return (tmpz84c011_pio_r(space,4) & ~pio_dir[4]) | (pio_latch[4] & pio_dir[4]); }

static WRITE8_HANDLER( tmpz84c011_0_pa_w )	{ pio_latch[0] = data; tmpz84c011_pio_w(space, 0, data); }
static WRITE8_HANDLER( tmpz84c011_0_pb_w )	{ pio_latch[1] = data; tmpz84c011_pio_w(space, 1, data); }
static WRITE8_HANDLER( tmpz84c011_0_pc_w )	{ pio_latch[2] = data; tmpz84c011_pio_w(space, 2, data); }
static WRITE8_HANDLER( tmpz84c011_0_pd_w )	{ pio_latch[3] = data; tmpz84c011_pio_w(space, 3, data); }
static WRITE8_HANDLER( tmpz84c011_0_pe_w )	{ pio_latch[4] = data; tmpz84c011_pio_w(space, 4, data); }

static READ8_HANDLER( tmpz84c011_0_dir_pa_r )	{ return pio_dir[0]; }
static READ8_HANDLER( tmpz84c011_0_dir_pb_r )	{ return pio_dir[1]; }
static READ8_HANDLER( tmpz84c011_0_dir_pc_r )	{ return pio_dir[2]; }
static READ8_HANDLER( tmpz84c011_0_dir_pd_r )	{ return pio_dir[3]; }
static READ8_HANDLER( tmpz84c011_0_dir_pe_r )	{ return pio_dir[4]; }

static WRITE8_HANDLER( tmpz84c011_0_dir_pa_w )	{ pio_dir[0] = data; }
static WRITE8_HANDLER( tmpz84c011_0_dir_pb_w )	{ pio_dir[1] = data; }
static WRITE8_HANDLER( tmpz84c011_0_dir_pc_w )	{ pio_dir[2] = data; }
static WRITE8_HANDLER( tmpz84c011_0_dir_pd_w )	{ pio_dir[3] = data; }
static WRITE8_HANDLER( tmpz84c011_0_dir_pe_w )	{ pio_dir[4] = data; }

static ADDRESS_MAP_START( csplayh5_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( csplayh5_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ctc", z80ctc_r, z80ctc_w)
	AM_RANGE(0x50, 0x50) AM_READWRITE(tmpz84c011_0_pa_r, tmpz84c011_0_pa_w)
	AM_RANGE(0x51, 0x51) AM_READWRITE(tmpz84c011_0_pb_r, tmpz84c011_0_pb_w)
	AM_RANGE(0x52, 0x52) AM_READWRITE(tmpz84c011_0_pc_r, tmpz84c011_0_pc_w)
	AM_RANGE(0x30, 0x30) AM_READWRITE(tmpz84c011_0_pd_r, tmpz84c011_0_pd_w)
	AM_RANGE(0x40, 0x40) AM_READWRITE(tmpz84c011_0_pe_r, tmpz84c011_0_pe_w)
	AM_RANGE(0x54, 0x54) AM_READWRITE(tmpz84c011_0_dir_pa_r, tmpz84c011_0_dir_pa_w)
	AM_RANGE(0x55, 0x55) AM_READWRITE(tmpz84c011_0_dir_pb_r, tmpz84c011_0_dir_pb_w)
	AM_RANGE(0x56, 0x56) AM_READWRITE(tmpz84c011_0_dir_pc_r, tmpz84c011_0_dir_pc_w)
	AM_RANGE(0x34, 0x34) AM_READWRITE(tmpz84c011_0_dir_pd_r, tmpz84c011_0_dir_pd_w)
	AM_RANGE(0x44, 0x44) AM_READWRITE(tmpz84c011_0_dir_pe_r, tmpz84c011_0_dir_pe_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("ymsnd", ym3812_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( csplayh5 )
	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, "DSWA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "DSWB" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )			// COIN2
	PORT_DIPNAME( 0x0004, 0x0004, "SYSA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "SYSB" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END

static GFXDECODE_START( csplayh5 )
GFXDECODE_END

static Z80CTC_INTERFACE( ctc_intf )
{
	0,							/* timer disables */
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0),/* interrupt handler */
	DEVCB_LINE(z80ctc_trg3_w),	/* ZC/TO0 callback ctc1.zc0 -> ctc1.trg3 */
	DEVCB_NULL,					/* ZC/TO1 callback */
	DEVCB_NULL					/* ZC/TO2 callback */
};


static MACHINE_RESET( csplayh5 )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int i;

	// initialize TMPZ84C011 PIO
	for (i = 0; i < 5; i++)
	{
		pio_dir[i] = pio_latch[i] = 0;
		tmpz84c011_pio_w(space, i, 0);
	}
}

static INTERRUPT_GEN( scanline_irq )
{
	v9938_set_sprite_limit(0, 0);
	v9938_set_resolution(0, RENDER_HIGH);
	v9938_interrupt(device->machine, 0);
}

static INTERRUPT_GEN( csplayh5_irq )
{
	cpu_set_input_line_and_vector(device, 1, HOLD_LINE,0x100/4);
}

static const z80_daisy_config daisy_chain_sound[] =
{
	{ "ctc" },
	{ NULL }
};


static MACHINE_CONFIG_START( csplayh5, driver_device )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,16000000) /* TMP68301-16 */
	MCFG_CPU_PROGRAM_MAP(csplayh5_map)
	MCFG_CPU_VBLANK_INT("screen", csplayh5_irq )
	MCFG_CPU_PERIODIC_INT(scanline_irq,262*60) // unknown timing

#if USE_H8
	MCFG_CPU_ADD("subcpu", H83002, 16000000)	/* unknown clock */
	MCFG_CPU_PROGRAM_MAP(csplayh5_sub_map)
	MCFG_CPU_IO_MAP(csplayh5_sub_io_map)
#endif

	MCFG_CPU_ADD("audiocpu", Z80, 8000000)	/* TMPZ84C011, unknown clock */
	MCFG_CPU_CONFIG(daisy_chain_sound)
	MCFG_CPU_PROGRAM_MAP(csplayh5_sound_map)
	MCFG_CPU_IO_MAP(csplayh5_sound_io_map)

	MCFG_Z80CTC_ADD("ctc", 8000000, ctc_intf)

	MCFG_MACHINE_RESET(csplayh5)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen",RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, MSX2_TOTAL_YRES_PIXELS)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MCFG_PALETTE_LENGTH(512)
	MCFG_PALETTE_INIT( v9958 )

	MCFG_VIDEO_START(csplayh5)
	MCFG_VIDEO_UPDATE(csplayh5)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static DRIVER_INIT( csplayh5 )
{
	UINT16 *MAINROM = (UINT16 *)machine->region("maincpu")->base();
	UINT8 *SNDROM = machine->region("audiocpu")->base();

	// initialize sound rom bank
	csplayh5_soundbank_w(machine, 0);

	/* patch DVD comms check */
	MAINROM[0x4cb4/2] = 0x6018;

	/* patch sound program */
	SNDROM[0x0213] = 0x00;			// DI -> NOP
}

ROM_START( csplayh5 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(980bf3b0) SHA1(89da7354552f30aaa9d46442972c060b4b0f8979) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(81ca49a4) SHA1(601b6802ab85be61f45a64f5b4c7e1f1ae5ee887) )

	ROM_REGION( 0x20000, "subcpu", 0 ) // ???
	ROM_LOAD16_WORD_SWAP( "daughter.bin",   0x00000, 0x20000, CRC(36135792) SHA1(1b9c50bd02df8227b228b35cc485efd5a13ec639) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(0b920806) SHA1(95f50ebfb296ba29aaa8079a41f5362cb9e879cc) )

	ROM_REGION( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00000, 0x80000, CRC(895b5e1f) SHA1(9398ee95d391f74d62fe641cb75311f31d4d1c8d) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00001, 0x80000, CRC(113d7e96) SHA1(f3fb9c719544417a6a018b82f07c65bf73de21ff) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "dvd" )
	DISK_IMAGE( "csplayh5", 0, SHA1(ce4883ce1351ce5299e41bfbd9a5ae8078b82b8c) )
ROM_END

// 1998
// 01 : Mahjong Gal-pri (World Gal-con Grandprix) : Nichibutsu/Just&Just
// 02 : Sengoku Mahjong Kurenai Otome-tai : Nichibutsu/Just&Just
// 03 : Jyunai - Manatsu no First Kiss : Nichibutsu/eic
/* 04 */ GAME( 1998, csplayh5,  0,   csplayh5,  csplayh5,  csplayh5, ROT0, "Nichibutsu", "Mahjong Hanafuda Cosplay Tengoku 5 (Japan)", GAME_NOT_WORKING )
// 05 : Jyunai2 - White Love Story : Nichibutsu/eic
// 06 : Mahjong Mogitate : Nichibutsu/Just&Just/NVS/Astro System/AV Japan

// 1999
// 07 : Mahjong Maina - Kairakukan he Youkoso : Sphinx/Just&Just
// 08 : Renai Mahjong Idol Gakuen : Nichibutsu/eic
// 09 : BiKiNikko - Okinawa de Ippai Shityaimashita : Nichibutsu/eic
// 10 : Mahjong Hanafuda Cospure Tengoko 6 - Jyunai hen : Nichibutsu/eic
// 11 : The Nanpa : Nichibutsu/Love Factory/eic
// 12 : PokoaPoka Onsen de CHU - Bijin 3 Simai ni kiwotukete : Nichibutsu/eic
// 13 : Cospure Tengoku 7 - Super Co-gal Grandprix : Nichibutsu/eic
// 14 : Ai-mode - Pet Shiiku : Nichibutsu/eic

// 2000
// 15 : Fu-dol : Nichbutsu/eic
// 16 : Nurete Mitaino... - Net Idol Hen : Nichibutsu/Love Factory
// 17 : Tuugakuro no Yuuwaku : Nichibutsu/Love Factory/Just&Just
// 18 : Toraretyattano - AV Kantoku Hen : Nichibutsu/Love Factory/M Friend

// 2001
// 19 : Konnano Hajimete! : Nichibutsu/Love Factory
// 20 : Uwasa no Deai-kei Sight : Nichibutsu/Love Factory/eic

