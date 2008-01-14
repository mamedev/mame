/******************************************************************************

    Game Driver for Nichibutsu Mahjong series.

    Niyanpai
    (c)1996 Nihon Bussan Co.,Ltd.

    Musoubana
    (c)1995 Nihon Bussan Co.,Ltd. / Yubis Co.,Ltd.

    Mahjong 4P Simasyo
    (c)1994 SPHINX/AV JAPAN

    Mahjong Housoukyoku Honbanchuu
    (c)199? Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/12/23 -

******************************************************************************/
/******************************************************************************
Memo:

- TMP68301 emulation is not implemented (machine/m68kfmly.c, .h does nothing).

- niyanpai's 2p start does not mean 2p simultaneous or exchanging play.
  Simply uses controls for 2p side.

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Screen flip is not perfect.

******************************************************************************/

#include "driver.h"
#include "machine/m68kfmly.h"
#include "machine/z80ctc.h"
#include "nb1413m3.h"
#include "sound/dac.h"
#include "sound/3812intf.h"
#include "cpu/z80/z80daisy.h"


#define SIGNED_DAC	0		// 0:unsigned DAC, 1:signed DAC
#if SIGNED_DAC
#define DAC_0_WRITE	DAC_0_signed_data_w
#define DAC_1_WRITE	DAC_1_signed_data_w
#else
#define DAC_0_WRITE	DAC_0_data_w
#define DAC_1_WRITE	DAC_1_data_w
#endif



extern VIDEO_UPDATE( niyanpai );
extern VIDEO_START( niyanpai );

extern READ16_HANDLER( niyanpai_palette_r );
extern WRITE16_HANDLER( niyanpai_palette_w );

extern READ16_HANDLER( niyanpai_blitter_0_r );
extern READ16_HANDLER( niyanpai_blitter_1_r );
extern READ16_HANDLER( niyanpai_blitter_2_r );
extern WRITE16_HANDLER( niyanpai_blitter_0_w );
extern WRITE16_HANDLER( niyanpai_blitter_1_w );
extern WRITE16_HANDLER( niyanpai_blitter_2_w );
extern WRITE16_HANDLER( niyanpai_clut_0_w );
extern WRITE16_HANDLER( niyanpai_clut_1_w );
extern WRITE16_HANDLER( niyanpai_clut_2_w );
extern WRITE16_HANDLER( niyanpai_clutsel_0_w );
extern WRITE16_HANDLER( niyanpai_clutsel_1_w );
extern WRITE16_HANDLER( niyanpai_clutsel_2_w );


static int musobana_inputport;
static int musobana_outcoin_flag;


static void niyanpai_soundbank_w(int data)
{
	UINT8 *SNDROM = memory_region(REGION_CPU2);

	memory_set_bankptr(1, &SNDROM[0x08000 + (0x8000 * (data & 0x03))]);
}

static int niyanpai_sound_r(int offset)
{
	return soundlatch_r(0);
}

static WRITE16_HANDLER( niyanpai_sound_w )
{
	soundlatch_w(0, ((data >> 8) & 0xff));
}

static void niyanpai_soundclr_w(int offset, int data)
{
	soundlatch_clear_w(0, 0);
}


/* TMPZ84C011 PIO emulation */
static UINT8 pio_dir[5], pio_latch[5];

static int tmpz84c011_pio_r(int offset)
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
			portdata = niyanpai_sound_r(0);
			break;
		case 4:			/* PE_0 */
			portdata = 0xff;
			break;

		default:
			logerror("PC %04X: TMPZ84C011_PIO Unknown Port Read %02X\n", activecpu_get_pc(), offset);
			portdata = 0xff;
			break;
	}

	return portdata;
}

static void tmpz84c011_pio_w(int offset, int data)
{
	switch (offset)
	{
		case 0:			/* PA_0 */
			niyanpai_soundbank_w(data & 0x03);
			break;
		case 1:			/* PB_0 */
			DAC_1_WRITE(0, data);
			break;
		case 2:			/* PC_0 */
			DAC_0_WRITE(0, data);
			break;
		case 3:			/* PD_0 */
			break;
		case 4:			/* PE_0 */
			if (!(data & 0x01)) niyanpai_soundclr_w(0, 0);
			break;

		default:
			logerror("PC %04X: TMPZ84C011_PIO Unknown Port Write %02X, %02X\n", activecpu_get_pc(), offset, data);
			break;
	}
}

/* CPU interface */
static READ8_HANDLER( tmpz84c011_0_pa_r )	{ return (tmpz84c011_pio_r(0) & ~pio_dir[0]) | (pio_latch[0] & pio_dir[0]); }
static READ8_HANDLER( tmpz84c011_0_pb_r )	{ return (tmpz84c011_pio_r(1) & ~pio_dir[1]) | (pio_latch[1] & pio_dir[1]); }
static READ8_HANDLER( tmpz84c011_0_pc_r )	{ return (tmpz84c011_pio_r(2) & ~pio_dir[2]) | (pio_latch[2] & pio_dir[2]); }
static READ8_HANDLER( tmpz84c011_0_pd_r )	{ return (tmpz84c011_pio_r(3) & ~pio_dir[3]) | (pio_latch[3] & pio_dir[3]); }
static READ8_HANDLER( tmpz84c011_0_pe_r )	{ return (tmpz84c011_pio_r(4) & ~pio_dir[4]) | (pio_latch[4] & pio_dir[4]); }

static WRITE8_HANDLER( tmpz84c011_0_pa_w )	{ pio_latch[0] = data; tmpz84c011_pio_w(0, data); }
static WRITE8_HANDLER( tmpz84c011_0_pb_w )	{ pio_latch[1] = data; tmpz84c011_pio_w(1, data); }
static WRITE8_HANDLER( tmpz84c011_0_pc_w )	{ pio_latch[2] = data; tmpz84c011_pio_w(2, data); }
static WRITE8_HANDLER( tmpz84c011_0_pd_w )	{ pio_latch[3] = data; tmpz84c011_pio_w(3, data); }
static WRITE8_HANDLER( tmpz84c011_0_pe_w )	{ pio_latch[4] = data; tmpz84c011_pio_w(4, data); }

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


static void ctc0_interrupt(int state)
{
	cpunum_set_input_line(1, 0, state);
}

static z80ctc_interface ctc_intf =
{
	1,					/* clock */
	0,					/* timer disables */
	ctc0_interrupt,		/* interrupt handler */
	z80ctc_0_trg3_w,	/* ZC/TO0 callback ctc1.zc0 -> ctc1.trg3 */
	0,					/* ZC/TO1 callback */
	0,					/* ZC/TO2 callback */
};

static void tmpz84c011_init(running_machine *machine)
{
	// initialize the CTC
	ctc_intf.baseclock = cpunum_get_clock(1);
	z80ctc_init(0, &ctc_intf);
}

static MACHINE_RESET( niyanpai )
{
	int i;

	// initialize TMPZ84C011 PIO
	for (i = 0; i < 5; i++)
	{
		pio_dir[i] = pio_latch[i] = 0;
		tmpz84c011_pio_w(i, 0);
	}
}

static DRIVER_INIT( niyanpai )
{
	UINT8 *MAINROM = memory_region(REGION_CPU1);
	UINT8 *SNDROM = memory_region(REGION_CPU2);

	// main program patch (USR0 -> IRQ LEVEL1)
	MAINROM[(25 * 4) + 0] = MAINROM[(64 * 4) + 0];
	MAINROM[(25 * 4) + 1] = MAINROM[(64 * 4) + 1];
	MAINROM[(25 * 4) + 2] = MAINROM[(64 * 4) + 2];
	MAINROM[(25 * 4) + 3] = MAINROM[(64 * 4) + 3];

	// sound program patch
	SNDROM[0x0213] = 0x00;			// DI -> NOP

	// initialize TMPZ84C011 PIO and CTC
	tmpz84c011_init(machine);

	// initialize sound rom bank
	niyanpai_soundbank_w(0);

	// initialize out coin flag (musobana)
	musobana_outcoin_flag = 1;
}


static READ16_HANDLER( niyanpai_dipsw_r )
{
	UINT8 dipsw_a, dipsw_b;

	dipsw_a = (((readinputport(0) & 0x01) << 7) | ((readinputport(0) & 0x02) << 5) |
			   ((readinputport(0) & 0x04) << 3) | ((readinputport(0) & 0x08) << 1) |
			   ((readinputport(0) & 0x10) >> 1) | ((readinputport(0) & 0x20) >> 3) |
			   ((readinputport(0) & 0x40) >> 5) | ((readinputport(0) & 0x80) >> 7));

	dipsw_b = (((readinputport(1) & 0x01) << 7) | ((readinputport(1) & 0x02) << 5) |
			   ((readinputport(1) & 0x04) << 3) | ((readinputport(1) & 0x08) << 1) |
			   ((readinputport(1) & 0x10) >> 1) | ((readinputport(1) & 0x20) >> 3) |
			   ((readinputport(1) & 0x40) >> 5) | ((readinputport(1) & 0x80) >> 7));

	return ((dipsw_a << 8) | dipsw_b);
}

static READ16_HANDLER( niyanpai_inputport_0_r )
{
	return ((readinputport(3) << 8) | (readinputport(4) << 0));
}

static READ16_HANDLER( niyanpai_inputport_1_r )
{
	return ((readinputport(2) << 8) | 0xff);
}

static READ16_HANDLER( musobana_inputport_0_r )
{
	int portdata;

	switch ((musobana_inputport ^ 0xff00) >> 8)
	{
		case 0x01:	portdata = ((readinputport(3) << 8) | (readinputport(8))); break;
		case 0x02:	portdata = ((readinputport(4) << 8) | (readinputport(9))); break;
		case 0x04:	portdata = ((readinputport(5) << 8) | (readinputport(10))); break;
		case 0x08:	portdata = ((readinputport(6) << 8) | (readinputport(11))); break;
		case 0x10:	portdata = ((readinputport(7) << 8) | (readinputport(12))); break;
		default:	portdata = (((readinputport(3) << 8) | (readinputport(8)))) & (((readinputport(4) << 8) | (readinputport(9)))) & (((readinputport(5) << 8) | (readinputport(10)))) & (((readinputport(6) << 8) | (readinputport(11)))) & (((readinputport(7) << 8) | (readinputport(12)))); break;
	}

	return (portdata);
}

static READ16_HANDLER( musobana_inputport_1_r )
{
	// tmp68301_parallel_interface[0x05]
	//  bit 0   coin counter
	//  bit 2   motor on
	//  bit 3   coin lock

	if (tmp68301_parallel_interface_r(0x0005, 0x00ff) & 0x0004) musobana_outcoin_flag ^= 1;
	else musobana_outcoin_flag = 1;

	return (((readinputport(2) & 0xdf) | ((musobana_outcoin_flag & 0x01) << 5)) << 8);
}

static WRITE16_HANDLER ( musobana_inputport_w )
{
	musobana_inputport = data;
}


static ADDRESS_MAP_START( niyanpai_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x040000, 0x040fff) AM_READ(MRA16_RAM)

	AM_RANGE(0x0a0000, 0x0a08ff) AM_READ(niyanpai_palette_r)
	AM_RANGE(0x0a0900, 0x0a11ff) AM_READ(MRA16_RAM)				// palette work ram?

	AM_RANGE(0x0bf800, 0x0bffff) AM_READ(MRA16_RAM)

	AM_RANGE(0x240400, 0x240403) AM_READ(niyanpai_blitter_0_r)
	AM_RANGE(0x240600, 0x240603) AM_READ(niyanpai_blitter_1_r)
	AM_RANGE(0x240800, 0x240803) AM_READ(niyanpai_blitter_2_r)

	AM_RANGE(0x280000, 0x280001) AM_READ(niyanpai_dipsw_r)
	AM_RANGE(0x280200, 0x280201) AM_READ(niyanpai_inputport_0_r)
	AM_RANGE(0x280400, 0x280401) AM_READ(niyanpai_inputport_1_r)

	AM_RANGE(0xfffc00, 0xfffc0f) AM_READ(tmp68301_address_decoder_r)
	AM_RANGE(0xfffc80, 0xfffc9f) AM_READ(tmp68301_interrupt_controller_r)
	AM_RANGE(0xfffd00, 0xfffd0f) AM_READ(tmp68301_parallel_interface_r)
	AM_RANGE(0xfffd80, 0xfffdaf) AM_READ(tmp68301_serial_interface_r)
	AM_RANGE(0xfffe00, 0xfffe4f) AM_READ(tmp68301_timer_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( niyanpai_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x040000, 0x040fff) AM_WRITE(MWA16_RAM) AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)

	AM_RANGE(0x0a0000, 0x0a08ff) AM_WRITE(niyanpai_palette_w)
	AM_RANGE(0x0a0900, 0x0a11ff) AM_WRITE(MWA16_RAM)			// palette work ram?

	AM_RANGE(0x0bf800, 0x0bffff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x200000, 0x200001) AM_WRITE(niyanpai_sound_w)

	AM_RANGE(0x200200, 0x200201) AM_WRITE(MWA16_NOP)			// unknown
	AM_RANGE(0x240000, 0x240009) AM_WRITE(MWA16_NOP)			// unknown
	AM_RANGE(0x240200, 0x2403ff) AM_WRITE(MWA16_NOP)			// unknown

	AM_RANGE(0x240400, 0x24041f) AM_WRITE(niyanpai_blitter_0_w)
	AM_RANGE(0x240420, 0x24043f) AM_WRITE(niyanpai_clut_0_w)

	AM_RANGE(0x240600, 0x24061f) AM_WRITE(niyanpai_blitter_1_w)
	AM_RANGE(0x240620, 0x24063f) AM_WRITE(niyanpai_clut_1_w)

	AM_RANGE(0x240800, 0x24081f) AM_WRITE(niyanpai_blitter_2_w)
	AM_RANGE(0x240820, 0x24083f) AM_WRITE(niyanpai_clut_2_w)

	AM_RANGE(0x240a00, 0x240a01) AM_WRITE(niyanpai_clutsel_0_w)
	AM_RANGE(0x240c00, 0x240c01) AM_WRITE(niyanpai_clutsel_1_w)
	AM_RANGE(0x240e00, 0x240e01) AM_WRITE(niyanpai_clutsel_2_w)

	AM_RANGE(0xfffc00, 0xfffc0f) AM_WRITE(tmp68301_address_decoder_w)
	AM_RANGE(0xfffc80, 0xfffc9f) AM_WRITE(tmp68301_interrupt_controller_w)
	AM_RANGE(0xfffd00, 0xfffd0f) AM_WRITE(tmp68301_parallel_interface_w)
	AM_RANGE(0xfffd80, 0xfffdaf) AM_WRITE(tmp68301_serial_interface_w)
	AM_RANGE(0xfffe00, 0xfffe4f) AM_WRITE(tmp68301_timer_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( musobana_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x040000, 0x040fff) AM_READ(MRA16_RAM)

	AM_RANGE(0x0a0000, 0x0a08ff) AM_READ(niyanpai_palette_r)
	AM_RANGE(0x0a0900, 0x0a11ff) AM_READ(MRA16_RAM)				// palette work ram?

	AM_RANGE(0x0a8000, 0x0a87ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x0bf800, 0x0bffff) AM_READ(MRA16_RAM)

	AM_RANGE(0x240400, 0x240403) AM_READ(niyanpai_blitter_0_r)
	AM_RANGE(0x240600, 0x240603) AM_READ(niyanpai_blitter_1_r)
	AM_RANGE(0x240800, 0x240803) AM_READ(niyanpai_blitter_2_r)

	AM_RANGE(0x280000, 0x280001) AM_READ(niyanpai_dipsw_r)
	AM_RANGE(0x280200, 0x280201) AM_READ(musobana_inputport_0_r)
	AM_RANGE(0x280400, 0x280401) AM_READ(musobana_inputport_1_r)

	AM_RANGE(0xfffc00, 0xfffc0f) AM_READ(tmp68301_address_decoder_r)
	AM_RANGE(0xfffc80, 0xfffc9f) AM_READ(tmp68301_interrupt_controller_r)
	AM_RANGE(0xfffd00, 0xfffd0f) AM_READ(tmp68301_parallel_interface_r)
	AM_RANGE(0xfffd80, 0xfffdaf) AM_READ(tmp68301_serial_interface_r)
	AM_RANGE(0xfffe00, 0xfffe4f) AM_READ(tmp68301_timer_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( musobana_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x040000, 0x040fff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x0a0000, 0x0a08ff) AM_WRITE(niyanpai_palette_w)
	AM_RANGE(0x0a0900, 0x0a11ff) AM_WRITE(MWA16_RAM)			// palette work ram?

	AM_RANGE(0x0a8000, 0x0a87ff) AM_WRITE(MWA16_RAM) AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x0bf800, 0x0bffff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x200000, 0x200001) AM_WRITE(niyanpai_sound_w)

	AM_RANGE(0x200200, 0x200201) AM_WRITE(musobana_inputport_w)	// inputport select
	AM_RANGE(0x240000, 0x240009) AM_WRITE(MWA16_NOP)			// unknown
	AM_RANGE(0x240200, 0x2403ff) AM_WRITE(MWA16_NOP)			// unknown

	AM_RANGE(0x240400, 0x24041f) AM_WRITE(niyanpai_blitter_0_w)
	AM_RANGE(0x240420, 0x24043f) AM_WRITE(niyanpai_clut_0_w)

	AM_RANGE(0x240600, 0x24061f) AM_WRITE(niyanpai_blitter_1_w)
	AM_RANGE(0x240620, 0x24063f) AM_WRITE(niyanpai_clut_1_w)

	AM_RANGE(0x240800, 0x24081f) AM_WRITE(niyanpai_blitter_2_w)
	AM_RANGE(0x240820, 0x24083f) AM_WRITE(niyanpai_clut_2_w)

	AM_RANGE(0x240a00, 0x240a01) AM_WRITE(niyanpai_clutsel_0_w)
	AM_RANGE(0x240c00, 0x240c01) AM_WRITE(niyanpai_clutsel_1_w)
	AM_RANGE(0x240e00, 0x240e01) AM_WRITE(niyanpai_clutsel_2_w)

	AM_RANGE(0xfffc00, 0xfffc0f) AM_WRITE(tmp68301_address_decoder_w)
	AM_RANGE(0xfffc80, 0xfffc9f) AM_WRITE(tmp68301_interrupt_controller_w)
	AM_RANGE(0xfffd00, 0xfffd0f) AM_WRITE(tmp68301_parallel_interface_w)
	AM_RANGE(0xfffd80, 0xfffdaf) AM_WRITE(tmp68301_serial_interface_w)
	AM_RANGE(0xfffe00, 0xfffe4f) AM_WRITE(tmp68301_timer_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mhhonban_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x040000, 0x040fff) AM_READ(MRA16_RAM)

	AM_RANGE(0x060000, 0x0608ff) AM_READ(niyanpai_palette_r)
	AM_RANGE(0x060900, 0x0611ff) AM_READ(MRA16_RAM)				// palette work ram?
	AM_RANGE(0x07f800, 0x07ffff) AM_READ(MRA16_RAM)

	AM_RANGE(0x0a8000, 0x0a87ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x0bf000, 0x0bffff) AM_READ(MRA16_RAM)

	AM_RANGE(0x240400, 0x240403) AM_READ(niyanpai_blitter_0_r)
	AM_RANGE(0x240600, 0x240603) AM_READ(niyanpai_blitter_1_r)
	AM_RANGE(0x240800, 0x240803) AM_READ(niyanpai_blitter_2_r)

	AM_RANGE(0x280000, 0x280001) AM_READ(niyanpai_dipsw_r)
	AM_RANGE(0x280200, 0x280201) AM_READ(musobana_inputport_0_r)
	AM_RANGE(0x280400, 0x280401) AM_READ(musobana_inputport_1_r)

	AM_RANGE(0xfffc00, 0xfffc0f) AM_READ(tmp68301_address_decoder_r)
	AM_RANGE(0xfffc80, 0xfffc9f) AM_READ(tmp68301_interrupt_controller_r)
	AM_RANGE(0xfffd00, 0xfffd0f) AM_READ(tmp68301_parallel_interface_r)
	AM_RANGE(0xfffd80, 0xfffdaf) AM_READ(tmp68301_serial_interface_r)
	AM_RANGE(0xfffe00, 0xfffe4f) AM_READ(tmp68301_timer_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mhhonban_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x040000, 0x040fff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x060000, 0x0608ff) AM_WRITE(niyanpai_palette_w)
	AM_RANGE(0x060900, 0x0611ff) AM_WRITE(MWA16_RAM)			// palette work ram?
	AM_RANGE(0x07f800, 0x07ffff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x0a8000, 0x0a87ff) AM_WRITE(MWA16_RAM) AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x0bf000, 0x0bffff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x200000, 0x200001) AM_WRITE(niyanpai_sound_w)

	AM_RANGE(0x200200, 0x200201) AM_WRITE(musobana_inputport_w)	// inputport select
	AM_RANGE(0x240000, 0x240009) AM_WRITE(MWA16_NOP)			// unknown
	AM_RANGE(0x240200, 0x2403ff) AM_WRITE(MWA16_NOP)			// unknown

	AM_RANGE(0x240400, 0x24041f) AM_WRITE(niyanpai_blitter_0_w)
	AM_RANGE(0x240420, 0x24043f) AM_WRITE(niyanpai_clut_0_w)

	AM_RANGE(0x240600, 0x24061f) AM_WRITE(niyanpai_blitter_1_w)
	AM_RANGE(0x240620, 0x24063f) AM_WRITE(niyanpai_clut_1_w)

	AM_RANGE(0x240800, 0x24081f) AM_WRITE(niyanpai_blitter_2_w)
	AM_RANGE(0x240820, 0x24083f) AM_WRITE(niyanpai_clut_2_w)

	AM_RANGE(0x240a00, 0x240a01) AM_WRITE(niyanpai_clutsel_0_w)
	AM_RANGE(0x240c00, 0x240c01) AM_WRITE(niyanpai_clutsel_1_w)
	AM_RANGE(0x240e00, 0x240e01) AM_WRITE(niyanpai_clutsel_2_w)

	AM_RANGE(0xfffc00, 0xfffc0f) AM_WRITE(tmp68301_address_decoder_w)
	AM_RANGE(0xfffc80, 0xfffc9f) AM_WRITE(tmp68301_interrupt_controller_w)
	AM_RANGE(0xfffd00, 0xfffd0f) AM_WRITE(tmp68301_parallel_interface_w)
	AM_RANGE(0xfffd80, 0xfffdaf) AM_WRITE(tmp68301_serial_interface_w)
	AM_RANGE(0xfffe00, 0xfffe4f) AM_WRITE(tmp68301_timer_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_READ(MRA8_ROM)
	AM_RANGE(0x7800, 0x7fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x8000, 0xffff) AM_READ(MRA8_BANK1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x10, 0x13) AM_READ(z80ctc_0_r)
	AM_RANGE(0x50, 0x50) AM_READ(tmpz84c011_0_pa_r)
	AM_RANGE(0x51, 0x51) AM_READ(tmpz84c011_0_pb_r)
	AM_RANGE(0x52, 0x52) AM_READ(tmpz84c011_0_pc_r)
	AM_RANGE(0x30, 0x30) AM_READ(tmpz84c011_0_pd_r)
	AM_RANGE(0x40, 0x40) AM_READ(tmpz84c011_0_pe_r)
	AM_RANGE(0x54, 0x54) AM_READ(tmpz84c011_0_dir_pa_r)
	AM_RANGE(0x55, 0x55) AM_READ(tmpz84c011_0_dir_pb_r)
	AM_RANGE(0x56, 0x56) AM_READ(tmpz84c011_0_dir_pc_r)
	AM_RANGE(0x34, 0x34) AM_READ(tmpz84c011_0_dir_pd_r)
	AM_RANGE(0x44, 0x44) AM_READ(tmpz84c011_0_dir_pe_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x10, 0x13) AM_WRITE(z80ctc_0_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(tmpz84c011_0_pa_w)
	AM_RANGE(0x51, 0x51) AM_WRITE(tmpz84c011_0_pb_w)
	AM_RANGE(0x52, 0x52) AM_WRITE(tmpz84c011_0_pc_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(tmpz84c011_0_pd_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(tmpz84c011_0_pe_w)
	AM_RANGE(0x54, 0x54) AM_WRITE(tmpz84c011_0_dir_pa_w)
	AM_RANGE(0x55, 0x55) AM_WRITE(tmpz84c011_0_dir_pb_w)
	AM_RANGE(0x56, 0x56) AM_WRITE(tmpz84c011_0_dir_pc_w)
	AM_RANGE(0x34, 0x34) AM_WRITE(tmpz84c011_0_dir_pd_w)
	AM_RANGE(0x44, 0x44) AM_WRITE(tmpz84c011_0_dir_pe_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(YM3812_control_port_0_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(YM3812_write_port_0_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( niyanpai )
	PORT_START	/* (0) DIPSW-A */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START	/* (1) DIPSW-B */
	PORT_DIPNAME( 0x01, 0x00, "Nudity" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (2) PORT 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )			// COIN2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )			// CREDIT CLEAR
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )			// START2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )		// ANALYZER
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )			// START1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )		// ?
 	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )					// TEST

	PORT_START	/* (3) PLAYER-1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* (4) PLAYER-2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( musobana )

	// I don't have manual for this game.

	PORT_START	/* (0) DIPSW-A */
	PORT_DIPNAME( 0x03, 0x03, "Game Out" )
	PORT_DIPSETTING(    0x03, "90% (Easy)" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x00, "60% (Hard)" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Game Type" )
	PORT_DIPSETTING(    0x80, "Medal Type" )
	PORT_DIPSETTING(    0x00, "Credit Type" )

	PORT_START	/* (1) DIPSW-B */
	PORT_DIPNAME( 0x03, 0x03, "Bet Min" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Score Pool" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (2) PORT 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )			// COIN2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )			// CREDIT CLEAR
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 )		// MEMORY RESET
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )		// ANALYZER
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )			// OUT COIN
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
 	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )					// TEST

	NBMJCTRL_PORT1	/* (3)  PORT 1-1 */
	NBMJCTRL_PORT2	/* (4)  PORT 1-2 */
	NBMJCTRL_PORT3	/* (5)  PORT 1-3 */
	NBMJCTRL_PORT4	/* (6)  PORT 1-4 */
	NBMJCTRL_PORT5	/* (7)  PORT 1-5 */

	NBMJCTRL_PORT6	/* (8)  PORT 2-1 */
	NBMJCTRL_PORT7	/* (9)  PORT 2-2 */
	NBMJCTRL_PORT8	/* (10) PORT 2-3 */
	NBMJCTRL_PORT9	/* (11) PORT 2-4 */
	NBMJCTRL_PORT10	/* (12) PORT 2-5 */
INPUT_PORTS_END

static INPUT_PORTS_START( 4psimasy )

	// I don't have manual for this game.

	PORT_START	/* (0) DIPSW-A */
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (1) DIPSW-B */
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Option Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (2) PORT 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )			// COIN2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )			// CREDIT CLEAR
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 )		// MEMORY RESET
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )		// ANALYZER
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )			// OUT COIN
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
 	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )					// TEST

	NBMJCTRL_PORT1	/* (3)  PORT 1-1 */
	NBMJCTRL_PORT2	/* (4)  PORT 1-2 */
	NBMJCTRL_PORT3	/* (5)  PORT 1-3 */
	NBMJCTRL_PORT4	/* (6)  PORT 1-4 */
	NBMJCTRL_PORT5	/* (7)  PORT 1-5 */

	NBMJCTRL_PORT6	/* (8)  PORT 2-1 */
	NBMJCTRL_PORT7	/* (9)  PORT 2-2 */
	NBMJCTRL_PORT8	/* (10) PORT 2-3 */
	NBMJCTRL_PORT9	/* (11) PORT 2-4 */
	NBMJCTRL_PORT10	/* (12) PORT 2-5 */
INPUT_PORTS_END

static INPUT_PORTS_START( mhhonban )

	// I don't have manual for this game.

	PORT_START	/* (0) DIPSW-A */
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (1) DIPSW-B */
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Option Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (2) PORT 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )			// COIN2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )			// CREDIT CLEAR
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 )		// MEMORY RESET
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )		// ANALYZER
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )			// OUT COIN
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
 	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )					// TEST

	NBMJCTRL_PORT1	/* (3)  PORT 1-1 */
	NBMJCTRL_PORT2	/* (4)  PORT 1-2 */
	NBMJCTRL_PORT3	/* (5)  PORT 1-3 */
	NBMJCTRL_PORT4	/* (6)  PORT 1-4 */
	NBMJCTRL_PORT5	/* (7)  PORT 1-5 */

	NBMJCTRL_PORT6	/* (8)  PORT 2-1 */
	NBMJCTRL_PORT7	/* (9)  PORT 2-2 */
	NBMJCTRL_PORT8	/* (10) PORT 2-3 */
	NBMJCTRL_PORT9	/* (11) PORT 2-4 */
	NBMJCTRL_PORT10	/* (12) PORT 2-5 */
INPUT_PORTS_END


static INTERRUPT_GEN( niyanpai_interrupt )
{
	cpunum_set_input_line(0, 1, HOLD_LINE);
}

static const struct z80_irq_daisy_chain daisy_chain_sound[] =
{
	{ z80ctc_reset, z80ctc_irq_state, z80ctc_irq_ack, z80ctc_irq_reti, 0 },	/* device 0 = CTC_1 */
	{ 0, 0, 0, 0, -1 }		/* end mark */
};


static MACHINE_DRIVER_START( niyanpai )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, 12288000/2)	/* TMP68301, 6.144 MHz */
	MDRV_CPU_PROGRAM_MAP(niyanpai_readmem,niyanpai_writemem)
	MDRV_CPU_VBLANK_INT(niyanpai_interrupt,1)

	MDRV_CPU_ADD(Z80, 8000000/1)					/* TMPZ84C011, 8.00 MHz */
	MDRV_CPU_CONFIG(daisy_chain_sound)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem, sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport, sound_writeport)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_RESET(niyanpai)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(1024, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 240-1)
	MDRV_PALETTE_LENGTH(768)

	MDRV_VIDEO_START(niyanpai)
	MDRV_VIDEO_UPDATE(niyanpai)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( musobana )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(niyanpai)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(musobana_readmem, musobana_writemem)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mhhonban )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(niyanpai)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mhhonban_readmem, mhhonban_writemem)
MACHINE_DRIVER_END


ROM_START( niyanpai )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "npai_01.bin", 0x00000, 0x20000, CRC(a904e8a1) SHA1(77865d7b48cac96af1e3cac4a702f7de4b5ee82b) )
	ROM_LOAD16_BYTE( "npai_02.bin", 0x00001, 0x20000, CRC(244f9d6f) SHA1(afde18f32c4879a66c0707671d783c21c54cffa4) )

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "npai_03.bin", 0x000000, 0x20000, CRC(d154306b) SHA1(3375568a6d387d850b8996b8bad3d0220de13993) )

	ROM_REGION( 0x400000, REGION_GFX1, 0 ) /* gfx */
	ROM_LOAD( "npai_04.bin", 0x000000, 0x80000, CRC(bec845b5) SHA1(2b00b4fd0bdda84cdc08933e593afdd91dde8d07) )
	ROM_LOAD( "npai_05.bin", 0x080000, 0x80000, CRC(3300ce07) SHA1(dc2eeb804aaf0aeb6cfee1844260ea24c3164bd9) )
	ROM_LOAD( "npai_06.bin", 0x100000, 0x80000, CRC(448e4e39) SHA1(63ca27f76a23235d3538d7f6c18dcc309e0f1f1c) )
	ROM_LOAD( "npai_07.bin", 0x180000, 0x80000, CRC(2ad47e55) SHA1(dbda82e654a85b0d5303bffa3005aaf78bdf0d28) )
	ROM_LOAD( "npai_08.bin", 0x200000, 0x80000, CRC(2ff980a0) SHA1(055addac657a5f7ec37ba85385834805c7aa0402) )
	ROM_LOAD( "npai_09.bin", 0x280000, 0x80000, CRC(74037ee3) SHA1(d975e6af962b9c62304ac15adab46c0ce972194b) )
	ROM_LOAD( "npai_10.bin", 0x300000, 0x80000, CRC(d35a9af6) SHA1(9a41aeea84c59b194bd122e2f102476834303302) )
	ROM_LOAD( "npai_11.bin", 0x380000, 0x80000, CRC(0748eb73) SHA1(63849f6625928646238a76748fd7903cee3ece2e) )
ROM_END

ROM_START( musobana )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "1.209", 0x00000, 0x20000, CRC(574929a1) SHA1(70ea96c3aa8a3512176b719de0928470541d85cb) )
	ROM_LOAD16_BYTE( "2.208", 0x00001, 0x20000, CRC(12734fda) SHA1(46241efe4266ad6426eb31db757ae4852c70c25d) )

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "3.804",  0x000000, 0x20000, CRC(0be8f2ce) SHA1(c1ee8907c03f615fbc42654a3c37387714761560) )

	ROM_REGION( 0x500000, REGION_GFX1, 0 ) /* gfx */
	ROM_LOAD( "4.102",  0x000000, 0x80000, CRC(1b5dcff8) SHA1(afc44d8a381e1f6059e8e29d415799f863ba8528) )
	ROM_LOAD( "5.103",  0x080000, 0x80000, CRC(dd69b24a) SHA1(2d1986f2b24877cfb4df9c32d76e4c4aada11420) )
	ROM_LOAD( "6.104",  0x100000, 0x80000, CRC(e898f3a2) SHA1(4d5002105b3a20f962a0f31c7703e16fcd4970aa) )
	ROM_LOAD( "7.105",  0x180000, 0x80000, CRC(812cb79a) SHA1(f905663f8656270c4cdda4a8547c57e9f3e1093b) )
	ROM_LOAD( "8.106",  0x200000, 0x80000, CRC(20285661) SHA1(503650148e07af9c34b22ae60b4a10c253f694aa) )
	ROM_LOAD( "9.107",  0x280000, 0x80000, CRC(91dfb28b) SHA1(4d673957533e6ef155765b71e3b0010455f2968b) )
	ROM_LOAD( "10.108", 0x300000, 0x80000, CRC(5c8e2300) SHA1(0e36d32d679f80d76a4c947e65d56ae449b03966) )
	ROM_LOAD( "11.109", 0x380000, 0x80000, CRC(12894ba4) SHA1(dcaef30283dfb5b97cc8a48247888434ffa7ec81) )
	ROM_LOAD( "12.202", 0x400000, 0x80000, CRC(d02957e0) SHA1(54f5e4724cc31030bc2185ac9d00fa57cc5c2255) )
	ROM_LOAD( "13.203", 0x480000, 0x80000, CRC(240ffce3) SHA1(8b357e962af8081bd01603ce1a3be6d2a7a4ed2a) )
ROM_END

ROM_START( 4psimasy )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "1.209", 0x00000, 0x20000, CRC(28dda353) SHA1(3d4738189a7b8b8b0434b3e58550572c3ce74b42) )
	ROM_LOAD16_BYTE( "2.208", 0x00001, 0x20000, CRC(3679c9fb) SHA1(74a940c3c95723680a63a281f194ef4bbe3dc58a) )

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "3.804",  0x000000, 0x20000, CRC(bd644726) SHA1(1f8e12a081657d6e1dd9c896056d1ffd977dfe95) )

	ROM_REGION( 0x400000, REGION_GFX1, 0 ) /* gfx */
	ROM_LOAD( "4.102",  0x000000, 0x80000, CRC(66c96d20) SHA1(2e8c6876c52fdc9afda4c29c84568942e3fe7fb8) )
	ROM_LOAD( "5.103",  0x080000, 0x80000, CRC(d8787e7d) SHA1(85a69f69da25159e0f7f75370ffaa3b8cb754eb0) )
	ROM_LOAD( "6.104",  0x100000, 0x80000, CRC(ad68defc) SHA1(fe6e0fd88dfbb20e13efb8ab80bc41c19963e6d7) )
	ROM_LOAD( "7.105",  0x180000, 0x80000, CRC(daceaa7b) SHA1(6b8ab028653f2546ab56860d81a3f5a8bbd6dede) )
	ROM_LOAD( "8.106",  0x200000, 0x80000, CRC(469032ce) SHA1(8ea743646acf0e2e0860ca092b0a45a43d19333f) )
	ROM_LOAD( "9.107",  0x280000, 0x80000, CRC(fa87a29e) SHA1(14c33f66efcbd4cbb8de16ace609711cc87e9ece) )
	ROM_LOAD( "10.108", 0x300000, 0x80000, CRC(f5608b85) SHA1(96f817ead285fefd5d1ebb1d4dd1f79125b110be) )
	ROM_LOAD( "11.109", 0x380000, 0x80000, CRC(1659f8d0) SHA1(daa5a2c5c7b5dc362581268fe98897164ccaa316) )
ROM_END

ROM_START( mhhonban )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "u209.bin", 0x00000, 0x20000, CRC(121c861f) SHA1(70a6b695998904dccb8791ea5d9acbf7484bd812) )
	ROM_LOAD16_BYTE( "u208.bin", 0x00001, 0x20000, CRC(d6712d0b) SHA1(a384c8f508ec6885bccb989d150cfd7f36a6898d) )

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "u804.bin",  0x000000, 0x20000, CRC(48407507) SHA1(afd24d16d487fd2b6548d967e2f1ae122e2633a2) )

	ROM_REGION( 0x300000, REGION_GFX1, 0 ) /* gfx */
	ROM_LOAD( "u102.bin",  0x000000, 0x80000, CRC(385b51aa) SHA1(445e365e762e60d6189d84608459f7d18fff859f) )
	ROM_LOAD( "u103.bin",  0x080000, 0x80000, CRC(1b85c6f4) SHA1(f8417b2526a8b51e52117d7d2690ce70af5e90fa) )
	ROM_LOAD( "u104.bin",  0x100000, 0x80000, CRC(0f091b1d) SHA1(f53425524a22ab0be241dc4303be7e1403989f3a) )
	ROM_LOAD( "u105.bin",  0x180000, 0x80000, CRC(20b39ac9) SHA1(b32b56c52cc6b79000588ad2cc8bfa533d7203f6) )
	ROM_LOAD( "u106.bin",  0x200000, 0x80000, CRC(11f42938) SHA1(f7cdc21cdefa8476090cc6e5b87b220b001fbeb1) )
	ROM_LOAD( "u107.bin",  0x280000, 0x80000, CRC(efc85912) SHA1(b9d523fd5f9ce879e2ed6916c89940be1d738a1c) )
ROM_END


GAME( 1996, niyanpai, 0, niyanpai, niyanpai, niyanpai, ROT0, "Nichibutsu", "Niyanpai (Japan)", 0 )
GAME( 1995, musobana, 0, musobana, musobana, niyanpai, ROT0, "Nichibutsu/Yubis", "Musoubana (Japan)", 0 )
GAME( 1994, 4psimasy, 0, musobana, 4psimasy, niyanpai, ROT0, "SPHINX/AV JAPAN", "Mahjong 4P Simasyo (Japan)", 0 )
GAME( 199?, mhhonban, 0, mhhonban, mhhonban, niyanpai, ROT0, "Nichibutsu?", "Mahjong Housoukyoku Honbanchuu (Japan)", 0 )
