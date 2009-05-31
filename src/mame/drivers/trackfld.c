/***************************************************************************

Konami games memory map (preliminary)

Based on drivers from Juno First emulator by Chris Hardy (chrish@kcbbs.gen.nz)

Track'n'Field

MAIN BOARD:
0000-17ff RAM
1800-183f Sprite RAM Pt 1
1C00-1C3f Sprite RAM Pt 2
3800-3bff Color RAM
3000-33ff Video RAM
6000-ffff ROM
1200-12ff IO

***************************************************************************/

#include "driver.h"
#include "trackfld.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "machine/konami1.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"
#include "sound/dac.h"
#include "sound/msm5205.h"

#define MASTER_CLOCK		XTAL_18_432MHz
#define SOUND_CLOCK			XTAL_14_31818MHz
#define VLM_CLOCK			XTAL_3_579545MHz


static UINT8 hyprolyb_adpcm_ready;
static UINT8 hyprolyb_adpcm_busy;
static UINT8 hyprolyb_vck_ready;


static WRITE8_HANDLER( coin_w )
{
	coin_counter_w(offset,data & 1);
}

static WRITE8_HANDLER( questions_bank_w )
{
	if (!(data & 0x01))
		memory_set_bank(space->machine, 1, 0);
	else if (!(data & 0x02))
		memory_set_bank(space->machine, 1, 1);
	else if (!(data & 0x04))
		memory_set_bank(space->machine, 1, 2);
	else if (!(data & 0x08))
		memory_set_bank(space->machine, 1, 3);
	else if (!(data & 0x10))
		memory_set_bank(space->machine, 1, 4);
	else if (!(data & 0x20))
		memory_set_bank(space->machine, 1, 5);
	else if (!(data & 0x40))
		memory_set_bank(space->machine, 1, 6);
	else if (!(data & 0x80))
		memory_set_bank(space->machine, 1, 7);
}

static WRITE8_HANDLER( hyprolyb_adpcm_w )
{
	soundlatch2_w(space, offset, data);
	hyprolyb_adpcm_ready = 0x80;
}

static READ8_HANDLER( hyprolyb_adpcm_busy_r )
{
	return hyprolyb_adpcm_busy ? 0x10 : 0x00;
}

static WRITE8_DEVICE_HANDLER( hyprolyb_msm_data_w )
{
	msm5205_data_w(device, data);
	hyprolyb_adpcm_busy = ~data & 0x80;
}

static READ8_DEVICE_HANDLER( hyprolyb_msm_vck_r )
{
	UINT8 old = hyprolyb_vck_ready;
	hyprolyb_vck_ready = 0x00;
	return old;
}

static READ8_HANDLER( hyprolyb_adpcm_ready_r )
{
	return hyprolyb_adpcm_ready;
}

static READ8_HANDLER( hyprolyb_adpcm_data_r )
{
	hyprolyb_adpcm_ready = 0x00;
	return soundlatch2_r(space, offset);
}



static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x007f) AM_WRITE(watchdog_reset_w)		/* AFE */
	AM_RANGE(0x1080, 0x1080) AM_MIRROR(0x0078) AM_WRITE(trackfld_flipscreen_w)	/* FLIP */
	AM_RANGE(0x1081, 0x1081) AM_MIRROR(0x0078) AM_WRITE(konami_sh_irqtrigger_w)	/* 26 */ /* cause interrupt on audio CPU */
	AM_RANGE(0x1082, 0x1082) AM_MIRROR(0x0078) AM_WRITENOP 						/* 25 */
	AM_RANGE(0x1083, 0x1084) AM_MIRROR(0x0078) AM_WRITE(coin_w)					/* 24, 23 */
	AM_RANGE(0x1085, 0x1085) AM_MIRROR(0x0078) AM_WRITENOP 						/* CN3.2 */
	AM_RANGE(0x1086, 0x1086) AM_MIRROR(0x0078) AM_WRITENOP 						/* CN3.4 */
	AM_RANGE(0x1087, 0x1087) AM_MIRROR(0x0078) AM_WRITE(interrupt_enable_w)		/* INT */
	AM_RANGE(0x1100, 0x1100) AM_MIRROR(0x007f) AM_WRITE(soundlatch_w)			/* 32 */
	AM_RANGE(0x1200, 0x1200) AM_MIRROR(0x007f) AM_READ_PORT("DSW2")
	AM_RANGE(0x1280, 0x1280) AM_MIRROR(0x007c) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1281, 0x1281) AM_MIRROR(0x007c) AM_READ_PORT("IN0")
	AM_RANGE(0x1282, 0x1282) AM_MIRROR(0x007c) AM_READ_PORT("IN1")
	AM_RANGE(0x1283, 0x1283) AM_MIRROR(0x007c) AM_READ_PORT("DSW1")
	/* not used according to schems: AM_RANGE(0x1300, 0x1300) AM_MIRROR(0x007f) AM_READ_PORT("DSW3") */
	AM_RANGE(0x1800, 0x183f) AM_RAM AM_BASE(&spriteram_2)
	AM_RANGE(0x1840, 0x185f) AM_RAM AM_BASE(&trackfld_scroll)
	AM_RANGE(0x1860, 0x1bff) AM_RAM
	AM_RANGE(0x1c00, 0x1c3f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x1c40, 0x1c5f) AM_RAM AM_BASE(&trackfld_scroll2)
	AM_RANGE(0x1c60, 0x1fff) AM_RAM
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(trackfld_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x3800, 0x3fff) AM_RAM_WRITE(trackfld_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( reaktor_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	/* all usual addresses +0x8000 */
	AM_RANGE(0x9000, 0x9000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x9080, 0x9080) AM_WRITE(trackfld_flipscreen_w)
	AM_RANGE(0x9081, 0x9081) AM_WRITE(konami_sh_irqtrigger_w)  /* cause interrupt on audio CPU */
	AM_RANGE(0x9083, 0x9084) AM_WRITE(coin_w)
	AM_RANGE(0x9087, 0x9087) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x9100, 0x9100) AM_WRITE(soundlatch_w)
	AM_RANGE(0x9200, 0x9200) AM_READ_PORT("DSW2")
	AM_RANGE(0x9280, 0x9280) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x9281, 0x9281) AM_READ_PORT("IN0")
	AM_RANGE(0x9282, 0x9282) AM_READ_PORT("IN1")
	AM_RANGE(0x9283, 0x9283) AM_READ_PORT("DSW1")
	AM_RANGE(0x9800, 0x983f) AM_RAM AM_BASE(&spriteram_2)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE(&trackfld_scroll)
	AM_RANGE(0x9860, 0x9bff) AM_RAM
	AM_RANGE(0x9c00, 0x9c3f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x9c40, 0x9c5f) AM_RAM AM_BASE(&trackfld_scroll2)
	AM_RANGE(0x9c60, 0x9fff) AM_RAM
	AM_RANGE(0xa800, 0xabff) AM_RAM
	AM_RANGE(0xac00, 0xafff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM_WRITE(trackfld_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xb800, 0xbfff) AM_RAM_WRITE(trackfld_colorram_w) AM_BASE(&colorram)
ADDRESS_MAP_END

/* Reaktor reads / writes some I/O ports, no idea what they're connected to, if anything */
static ADDRESS_MAP_START( reaktor_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP
	AM_RANGE(0x01, 0x01) AM_NOP
	AM_RANGE(0x02, 0x02) AM_WRITENOP
	AM_RANGE(0x03, 0x03) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( mastkin_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x1000, 0x1000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x10b0, 0x10b0) AM_WRITE(trackfld_flipscreen_w)
	AM_RANGE(0x10b1, 0x10b1) AM_WRITE(konami_sh_irqtrigger_w)
	AM_RANGE(0x1083, 0x1084) AM_WRITE(coin_w)
	AM_RANGE(0x1087, 0x1087) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x1100, 0x1100) AM_WRITE(soundlatch_w)
	AM_RANGE(0x1200, 0x1200) AM_READ_PORT("DSW2")
	AM_RANGE(0x1280, 0x1280) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1281, 0x1281) AM_READ_PORT("IN0")
//  AM_RANGE(0x1282, 0x1282) AM_READ_PORT("IN1") /* unused */
	AM_RANGE(0x1283, 0x1283) AM_READ_PORT("DSW1")
	AM_RANGE(0x1800, 0x183f) AM_RAM AM_BASE(&spriteram_2)
	AM_RANGE(0x1840, 0x185f) AM_RAM AM_BASE(&trackfld_scroll)
	AM_RANGE(0x1860, 0x1bff) AM_RAM
	AM_RANGE(0x1c00, 0x1c3f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x1c40, 0x1c5f) AM_RAM AM_BASE(&trackfld_scroll2)
	AM_RANGE(0x1c60, 0x1fff) AM_RAM
	AM_RANGE(0x2800, 0x2bff) AM_RAM
	AM_RANGE(0x2c00, 0x2fff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(trackfld_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x3800, 0x3fff) AM_RAM_WRITE(trackfld_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( wizzquiz_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READWRITE(watchdog_reset_r, watchdog_reset_w)
	AM_RANGE(0x1080, 0x1080) AM_WRITE(trackfld_flipscreen_w)
	AM_RANGE(0x1081, 0x1081) AM_WRITE(konami_sh_irqtrigger_w)  /* cause interrupt on audio CPU */
	AM_RANGE(0x1083, 0x1084) AM_WRITE(coin_w)
	AM_RANGE(0x1087, 0x1087) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x1100, 0x1100) AM_WRITE(soundlatch_w)
	AM_RANGE(0x1200, 0x1200) AM_READ_PORT("DSW2")
	AM_RANGE(0x1280, 0x1280) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1281, 0x1281) AM_READ_PORT("IN0")
	AM_RANGE(0x1282, 0x1282) AM_READ_PORT("IN1")
	AM_RANGE(0x1283, 0x1283) AM_READ_PORT("DSW1")
	AM_RANGE(0x1800, 0x183f) AM_RAM AM_BASE(&spriteram_2)
	AM_RANGE(0x1840, 0x185f) AM_RAM AM_BASE(&trackfld_scroll)
	AM_RANGE(0x1860, 0x1bff) AM_RAM
	AM_RANGE(0x1c00, 0x1c3f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x1c40, 0x1c5f) AM_RAM AM_BASE(&trackfld_scroll2)
	AM_RANGE(0x1c60, 0x1fff) AM_RAM
	AM_RANGE(0x2800, 0x2bff) AM_RAM
	AM_RANGE(0x2c00, 0x2fff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x3000, 0x37ff) AM_READ(SMH_RAM) AM_WRITE(trackfld_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x3800, 0x3fff) AM_READ(SMH_RAM) AM_WRITE(trackfld_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(questions_bank_w)
	AM_RANGE(0x6000, 0xdfff) AM_READ(SMH_BANK(1))
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1fff) AM_READ(trackfld_sh_timer_r)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1fff) AM_WRITE(konami_SN76496_latch_w)
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x1fff) AM_DEVWRITE("sn", konami_SN76496_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1ff8) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x1ff8) AM_NOP			/* watch dog ?; reaktor reads here */
	AM_RANGE(0xe002, 0xe002) AM_MIRROR(0x1ff8) AM_DEVREAD("vlm", trackfld_speech_r)
	AM_RANGE(0xe003, 0xe003) AM_MIRROR(0x1ff8) AM_MASK(0x0380) AM_DEVWRITE("vlm", trackfld_sound_w)
	AM_RANGE(0xe004, 0xe004) AM_MIRROR(0x1ff8) AM_DEVWRITE("vlm", vlm5030_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hyprolyb_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1fff) AM_READ(trackfld_sh_timer_r)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1fff) AM_WRITE(konami_SN76496_latch_w)
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x1fff) AM_DEVWRITE("sn", konami_SN76496_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1ff8) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x1ff8) AM_NOP			/* watch dog ?; reaktor reads here */
	AM_RANGE(0xe002, 0xe002) AM_MIRROR(0x1ff8) AM_READ(hyprolyb_adpcm_busy_r)
	AM_RANGE(0xe003, 0xe003) AM_MIRROR(0x1ff8) AM_WRITENOP
	AM_RANGE(0xe004, 0xe004) AM_MIRROR(0x1ff8) AM_WRITE(hyprolyb_adpcm_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hyprolyb_adpcm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READ(hyprolyb_adpcm_data_r)
	AM_RANGE(0x1001, 0x1001) AM_READ(hyprolyb_adpcm_ready_r)
	AM_RANGE(0x1002, 0x1002) AM_DEVWRITE("msm", hyprolyb_msm_data_w)
	AM_RANGE(0x1003, 0x1003) AM_DEVREAD("msm", hyprolyb_msm_vck_r)
		// on init:
		//    $1003 = $00
		//    $1002 = $FF
		//    $1003 = $34
		//    $1001 = $36
		//    $1002 = $80
		// loops while ($1003) & 0x80 == 0
		// 1002 = ADPCM data written (low 4 bits)
		//
		// $1003 & $80 (in) = 5205 DRQ
		// $1002 & $0f (out) = 5205 data
		// $1001 & $80 (in) = sound latch request
		// $1000 (in) = sound latch data
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( trackfld )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_PLAYER(3) //PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(3) //PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(3) //PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin B" )
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, "After Last Event" )
	PORT_DIPSETTING(    0x02, "Game Over" )
	PORT_DIPSETTING(    0x00, "Game Continues" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0x10, 0x10, "World Records" )
	PORT_DIPSETTING(    0x10, "Don't Erase" )
	PORT_DIPSETTING(    0x00, "Erase on Reset" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( atlantol )
	PORT_INCLUDE( trackfld )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(3) //PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(3) //PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(3) //PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Italian ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mastkin )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Timer Speed" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) )	// "Damage"
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )				//   0x03
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )			//   0x07
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )				//   0x0b
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )			//   0x0f
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x00, "Internal speed" )		// Check code at 0x8576
	PORT_DIPSETTING(    0x20, "Slow" )				//   0x0c00
	PORT_DIPSETTING(    0x00, "Fast" )				//   0x0a00
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		// Stored at 0x284e but not read back
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )		// Cocktail Mode, not used

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
//  PORT_DIPSETTING(    0x0b, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x0d, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
//  PORT_DIPSETTING(    0xb0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xd0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wizzquiz )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 - C")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 - B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 - A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Set")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 - C")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 - B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 - A")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )	PORT_CONDITION("DSW1", 0xf0, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "No Coin A" )				PORT_CONDITION("DSW1", 0xf0, PORTCOND_NOTEQUALS, 0x00)
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )	PORT_CONDITION("DSW1", 0x0f, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "No Coin B" )				PORT_CONDITION("DSW1", 0x0f, PORTCOND_NOTEQUALS, 0x00)
	/* No Coin X = coin slot X open (coins produce sound), but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "8,000 Points" )
	PORT_DIPSETTING(    0x04, "10,000 Points" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" ) /* Listed as "not used" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "not used" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "not used" */
	PORT_DIPNAME( 0x40, 0x40, "Show Correct Answer" )	PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Limit" )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "99 Credits" )
	PORT_DIPSETTING(    0x00, "9 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START( reaktor )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	/* controls seem to be shared by both players */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused

	PORT_START("IN1")
  	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // probably unused

	PORT_START("DSW1")
    PORT_DIPNAME( 0x01,   0x01, "Pricing" )
    PORT_DIPSETTING(      0x01, "10p / 25c per play" )
    PORT_DIPSETTING(      0x00, "20p / 50c per play" )
    PORT_DIPNAME( 0x02,   0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04,   0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10,   0x10, "Coinage Type" )
    PORT_DIPSETTING(      0x10, "English (10p / 20p)" )
    PORT_DIPSETTING(      0x00, "American (25c / 50c)" )
    PORT_DIPNAME( 0x60,   0x20, DEF_STR( Lives ) )
    PORT_DIPSETTING(      0x60, "2" )
    PORT_DIPSETTING(      0x40, "3" )
    PORT_DIPSETTING(      0x20, "4" )
    PORT_DIPSETTING(      0x00, "5" )
    PORT_DIPNAME( 0x80,   0x80, DEF_STR( Bonus_Life ) )
    PORT_DIPSETTING(      0x80, "20000" )
    PORT_DIPSETTING(      0x00, "30000" )

	PORT_START("DSW2")
    PORT_DIPNAME( 0x01,   0x01, "Game Orientation" )
    PORT_DIPSETTING(      0x01, "For Vertical Monitor" )
    PORT_DIPSETTING(      0x00, "For Horizontal Monitor" )
    PORT_DIPNAME( 0x02,   0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04,   0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08,   0x08, DEF_STR( Free_Play ) )
    PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10,   0x00, "Wipe Highscores" ) // it doesn't have NVRAM does it?
    PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x10, DEF_STR( On ) )
    PORT_DIPNAME( 0x20,   0x20, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40,   0x40, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80,   0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( trackfld )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END


static void adpcm_vck_callback(const device_config *device)
{
	hyprolyb_vck_ready = 0x80;
}

static const msm5205_interface msm5205_config =
{
	adpcm_vck_callback,	/* VCK function */
	MSM5205_S96_4B		/* 4 kHz */
};



static MACHINE_DRIVER_START( trackfld )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, MASTER_CLOCK/6/2)	/* a guess for now */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, SOUND_CLOCK/4)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(trackfld)
	MDRV_PALETTE_LENGTH(16*16+16*16)

	MDRV_PALETTE_INIT(trackfld)
	MDRV_VIDEO_START(trackfld)
	MDRV_VIDEO_UPDATE(trackfld)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("sn", SN76496, SOUND_CLOCK/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("vlm", VLM5030, VLM_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/* same as the original, but uses ADPCM instead of VLM5030 */
/* also different memory handlers do handle that */
static MACHINE_DRIVER_START( hyprolyb )
	MDRV_IMPORT_FROM(trackfld)

	MDRV_CPU_MODIFY("audiocpu")
	MDRV_CPU_PROGRAM_MAP(hyprolyb_sound_map)

	MDRV_CPU_ADD("adpcm", M6802, SOUND_CLOCK/8)	/* unknown clock */
	MDRV_CPU_PROGRAM_MAP(hyprolyb_adpcm_map)

	/* sound hardware */
	MDRV_SOUND_REMOVE("vlm")

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mastkin )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(trackfld)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(mastkin_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( wizzquiz )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(trackfld)
	// right cpu?
	MDRV_CPU_REPLACE("maincpu",M6800,2048000)		/* 1.400 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(wizzquiz_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( reaktor )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(trackfld)
	MDRV_CPU_REPLACE("maincpu",Z80,MASTER_CLOCK/6)
	MDRV_CPU_PROGRAM_MAP(reaktor_map)
	MDRV_CPU_IO_MAP(reaktor_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( trackfld )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "a01_e01.bin",  0x6000, 0x2000, CRC(2882f6d4) SHA1(f7ddae2c5412a2849efd7f9629e92a5b0328e7cb) )
	ROM_LOAD( "a02_e02.bin",  0x8000, 0x2000, CRC(1743b5ee) SHA1(31301031a525f893c31461f634350f01a9492ef4) )
	ROM_LOAD( "a03_k03.bin",  0xa000, 0x2000, CRC(6c0d1ee9) SHA1(380ab2162153a61910a6fe5b6d091ca9451ad4fd) )
	ROM_LOAD( "a04_e04.bin",  0xc000, 0x2000, CRC(21d6c448) SHA1(6c42cc76302485954a31520bdd08469fa948c72f) )
	ROM_LOAD( "a05_e05.bin",  0xe000, 0x2000, CRC(f08c7b7e) SHA1(50e65d9b0ea37d2afb2dfdf1f3e1378e3290bc81) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c2_d13.bin",   0x0000, 0x2000, CRC(95bf79b6) SHA1(ea9135acd7ad162c19c5cdde356e69792d61b675) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "c11_d06.bin",  0x0000, 0x2000, CRC(82e2185a) SHA1(1da9ea20e7af0b49c62fb39834a7ec686491af04) )
	ROM_LOAD( "c12_d07.bin",  0x2000, 0x2000, CRC(800ff1f1) SHA1(33d73b18903e3e6bfb30f1a06db4b8105d4040d8) )
	ROM_LOAD( "c13_d08.bin",  0x4000, 0x2000, CRC(d9faf183) SHA1(4448b6242790783d37acf50704d597af5878c2ab) )
	ROM_LOAD( "c14_d09.bin",  0x6000, 0x2000, CRC(5886c802) SHA1(884a12a8f63600da4f23b29be6dbaacef37add20) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "h16_e12.bin",  0x0000, 0x2000, CRC(50075768) SHA1(dfff92c0f59dd3d8d3d6256944bfd48792cef6a9) )
	ROM_LOAD( "h15_e11.bin",  0x2000, 0x2000, CRC(dda9e29f) SHA1(0f41cde82bb60c3f1591ee14dc3cff4642bbddc1) )
	ROM_LOAD( "h14_e10.bin",  0x4000, 0x2000, CRC(c2166a5c) SHA1(5ba25900e653ce4edcf35f1fbce758a327a715ce) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "361b16.f1",    0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b16",   0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.e15",   0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */

	ROM_REGION( 0x10000, "vlm", 0 )	/* 64k for speech rom */
	ROM_LOAD( "c9_d15.bin",   0x0000, 0x2000, CRC(f546a56b) SHA1(caee3d8546eb7a75ce2a578c6a1a630246aec6b8) )
ROM_END

ROM_START( trackflc )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "f01.1a",       0x6000, 0x2000, CRC(4e32b360) SHA1(cafd4b9ef5548d31d894610dfd2288425d29ed58) )
	ROM_LOAD( "f02.2a",       0x8000, 0x2000, CRC(4e7ebf07) SHA1(266110e5195ab1e374724536b82ec4da35123dc7) )
	ROM_LOAD( "l03.3a",       0xa000, 0x2000, CRC(fef4c0ea) SHA1(c34a0f001de8c06fdb617e20dc335ad99e15df05) )
	ROM_LOAD( "f04.4a",       0xc000, 0x2000, CRC(73940f2d) SHA1(31e0db23ebcf634605f8c232606079ad75e27a66) )
	ROM_LOAD( "f05.5a",       0xe000, 0x2000, CRC(363fd761) SHA1(2b4868813b62c2b7d122e2cb238803eb4687b002) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c2_d13.bin",   0x0000, 0x2000, CRC(95bf79b6) SHA1(ea9135acd7ad162c19c5cdde356e69792d61b675) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "c11_d06.bin",  0x0000, 0x2000, CRC(82e2185a) SHA1(1da9ea20e7af0b49c62fb39834a7ec686491af04) )
	ROM_LOAD( "c12_d07.bin",  0x2000, 0x2000, CRC(800ff1f1) SHA1(33d73b18903e3e6bfb30f1a06db4b8105d4040d8) )
	ROM_LOAD( "c13_d08.bin",  0x4000, 0x2000, CRC(d9faf183) SHA1(4448b6242790783d37acf50704d597af5878c2ab) )
	ROM_LOAD( "c14_d09.bin",  0x6000, 0x2000, CRC(5886c802) SHA1(884a12a8f63600da4f23b29be6dbaacef37add20) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "h16_e12.bin",  0x0000, 0x2000, CRC(50075768) SHA1(dfff92c0f59dd3d8d3d6256944bfd48792cef6a9) )
	ROM_LOAD( "h15_e11.bin",  0x2000, 0x2000, CRC(dda9e29f) SHA1(0f41cde82bb60c3f1591ee14dc3cff4642bbddc1) )
	ROM_LOAD( "h14_e10.bin",  0x4000, 0x2000, CRC(c2166a5c) SHA1(5ba25900e653ce4edcf35f1fbce758a327a715ce) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "361b16.f1",    0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b16",   0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.e15",   0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */

	ROM_REGION( 0x10000, "vlm", 0 )	/* 64k for speech rom */
	ROM_LOAD( "c9_d15.bin",   0x0000, 0x2000, CRC(f546a56b) SHA1(caee3d8546eb7a75ce2a578c6a1a630246aec6b8) )
ROM_END

ROM_START( hyprolym ) /* GX361 */
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "361-d01.a01", 0x6000, 0x2000, CRC(82257fb7) SHA1(4a5038292e582d5c3b5f2d82b01c57ccb24f3095) )
	ROM_LOAD( "361-d02.a02", 0x8000, 0x2000, CRC(15b83099) SHA1(79827590d74f20c9a95723e06b05af2b15c34f5f) )
	ROM_LOAD( "361-d03.a03", 0xa000, 0x2000, CRC(e54cc960) SHA1(7c448c174675271d548ffcf0297ec7a2ae646985) )
	ROM_LOAD( "361-d04.a04", 0xc000, 0x2000, CRC(d099b1e8) SHA1(0472991ad6caef41ec6b8ec8bf3d9d07584a57cc) )
	ROM_LOAD( "361-d05.a05", 0xe000, 0x2000, CRC(974ff815) SHA1(11512df2008a79ba44bbb84bd70885f187113211) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c2_d13.bin",   0x0000, 0x2000, CRC(95bf79b6) SHA1(ea9135acd7ad162c19c5cdde356e69792d61b675) ) /* 361-d13.c03 */

	ROM_REGION( 0x8000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "c11_d06.bin",  0x0000, 0x2000, CRC(82e2185a) SHA1(1da9ea20e7af0b49c62fb39834a7ec686491af04) ) /* 361-d06.c11 */
	ROM_LOAD( "c12_d07.bin",  0x2000, 0x2000, CRC(800ff1f1) SHA1(33d73b18903e3e6bfb30f1a06db4b8105d4040d8) ) /* 361-d07.c12 */
	ROM_LOAD( "c13_d08.bin",  0x4000, 0x2000, CRC(d9faf183) SHA1(4448b6242790783d37acf50704d597af5878c2ab) ) /* 361-d08.c13 */
	ROM_LOAD( "c14_d09.bin",  0x6000, 0x2000, CRC(5886c802) SHA1(884a12a8f63600da4f23b29be6dbaacef37add20) ) /* 361-d09.c14 */

	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "361-d12.h16", 0x0000, 0x2000, CRC(768bb63d) SHA1(effc46615c389245e5a4aac18292e1d764ff0e46) )
	ROM_LOAD( "361-d11.h15", 0x2000, 0x2000, CRC(3af0e2a8) SHA1(450f35fd7e45ecc88ee80bf57499b2e9f06f6487) )
	ROM_LOAD( "h14_e10.bin",  0x4000, 0x2000, CRC(c2166a5c) SHA1(5ba25900e653ce4edcf35f1fbce758a327a715ce) ) /* 361-d10.h14 */

	ROM_REGION( 0x0220, "proms", 0 ) /* Prom names = 361-b16.f01 / 361-b17.b16 / 361-b18.e15 */
	ROM_LOAD( "361b16.f1",    0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b16",   0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.e15",   0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */

	ROM_REGION( 0x10000, "vlm", 0 )	/* 64k for speech rom */
	ROM_LOAD( "c9_d15.bin",   0x0000, 0x2000, CRC(f546a56b) SHA1(caee3d8546eb7a75ce2a578c6a1a630246aec6b8) ) /* 361-d15.c09 */
ROM_END

ROM_START( hyprolyb )
    /* These ROM's are located on the CPU/Video Board */
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "1.a1",         0x6000, 0x2000, CRC(9aee2d5a) SHA1(81f151459f1113b5f2f76ddc140bf86676f778e4) )
	ROM_LOAD( "2.a2",         0x8000, 0x2000, CRC(15b83099) SHA1(79827590d74f20c9a95723e06b05af2b15c34f5f) )
	ROM_LOAD( "3.a4",         0xa000, 0x2000, CRC(2d6fc308) SHA1(1ff95384670e40d560703f2238998a8e154aa4cf) )
	ROM_LOAD( "4.a5",         0xc000, 0x2000, CRC(d099b1e8) SHA1(0472991ad6caef41ec6b8ec8bf3d9d07584a57cc) )
	ROM_LOAD( "5.a7",         0xe000, 0x2000, CRC(974ff815) SHA1(11512df2008a79ba44bbb84bd70885f187113211) )

    /* These ROM's are located on the Sound Board */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c2_d13.bin",   0x0000, 0x2000, CRC(95bf79b6) SHA1(ea9135acd7ad162c19c5cdde356e69792d61b675) )

    /* These ROM's are located on the Sound Board */
	ROM_REGION( 0x10000, "adpcm", 0 )	/*  64k for the 6802 which plays ADPCM samples */
	ROM_LOAD( "2764.1",       0x8000, 0x2000, CRC(a4cddeb8) SHA1(057981ad3b04239662bb19342e9ec14b0dab2351) )
	ROM_LOAD( "2764.2",       0xa000, 0x2000, CRC(e9919365) SHA1(bd11d6e3ee2c6e698159c2768e315389d666107f) )
	ROM_LOAD( "2764.3",       0xc000, 0x2000, CRC(c3ec42e1) SHA1(048a95726c4f031552e629c3788952c1bc5e7251) )
	ROM_LOAD( "2764.4",       0xe000, 0x2000, CRC(76998389) SHA1(499189b0e20296af88712199b93b958655083608) )

    /* These ROM's are located on the CPU/Video Board */
	ROM_REGION( 0x8000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "6.a18",       0x0000, 0x2000, CRC(82e2185a) SHA1(1da9ea20e7af0b49c62fb39834a7ec686491af04) )
	ROM_LOAD( "7.a19",       0x2000, 0x2000, CRC(800ff1f1) SHA1(33d73b18903e3e6bfb30f1a06db4b8105d4040d8) )
	ROM_LOAD( "8.a21",       0x4000, 0x2000, CRC(d9faf183) SHA1(4448b6242790783d37acf50704d597af5878c2ab) )
	ROM_LOAD( "9.a22",       0x6000, 0x2000, CRC(5886c802) SHA1(884a12a8f63600da4f23b29be6dbaacef37add20) )

    /* These ROM's are located on the CPU/Video Board */
	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "12.h22",      0x0000, 0x2000, CRC(768bb63d) SHA1(effc46615c389245e5a4aac18292e1d764ff0e46) )
	ROM_LOAD( "11.h21",      0x2000, 0x2000, CRC(3af0e2a8) SHA1(450f35fd7e45ecc88ee80bf57499b2e9f06f6487) )
	ROM_LOAD( "10.h19",      0x4000, 0x2000, CRC(c2166a5c) SHA1(5ba25900e653ce4edcf35f1fbce758a327a715ce) )

    /* These PROM's are located on the CPU/Video Board */
	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "361b16.e1",   0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b15",  0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.f22",  0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */

    /* These PAL's are located on the CPU/Video Board */
	ROM_REGION( 0x0002, "plds", ROMREGION_DISPOSE )
    ROM_LOAD( "pal16l8.bin", 0x0000, 0x0001, NO_DUMP ) /* Located at 4E. */
    ROM_LOAD( "pal16l8.bin", 0x0000, 0x0001, NO_DUMP ) /* Located at 6E. */
ROM_END

ROM_START( atlantol )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "atl37", 0x00000, 0x10000, CRC(aca8da51) SHA1(50e96fd8496ed32e11eb43bcbfd468ce566caa47) )
	ROM_CONTINUE(      0x00000, 0x10000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "atl35", 0x00000, 0x10000, CRC(03331597) SHA1(74a6e20cb0cadc17500b9046d621be252839de98) )
	ROM_CONTINUE(      0x00000, 0x10000 )

	ROM_REGION( 0x10000, "adpcm", 0 )	/*  64k for the 6802 which plays ADPCM samples */
	ROM_LOAD( "atl36", 0x00000, 0x10000, CRC(0bae8489) SHA1(fbaeac99733f9c46b0b8d9a601c57df4004e2044) )
	ROM_CONTINUE(      0x00000, 0x10000 )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "atl38", 0x00000, 0x20000, CRC(dbbcbcda) SHA1(df84fd73425bac2dcde0b650369ed1ff105f729f) )
	ROM_LOAD( "atl39", 0x20000, 0x20000, CRC(d08f067f) SHA1(077446bf3269dba2881e745434f1581f3a901d99) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "atl40", 0x00000, 0x20000, CRC(c915f53a) SHA1(5983fa68f8a494fe0c71e8dae79b45eee178bbcd) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "361b16.f1",    0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b16",   0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.e15",   0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */
ROM_END

ROM_START( mastkin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mk3",          0x8000, 0x2000, CRC(9f80d6ae) SHA1(724321d8c3e32d679f8170dfef6555d0179f9d20) )
	ROM_LOAD( "mk4",          0xa000, 0x2000, CRC(99f361e7) SHA1(8706e5c393325c5a89d32388991bc48fa4102779) )
	ROM_LOAD( "mk5",          0xe000, 0x2000, CRC(143d76ce) SHA1(5e5c450e891a11980fb514453f28ffc74a2730ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mk1",          0x0000, 0x2000, CRC(95bf79b6) SHA1(ea9135acd7ad162c19c5cdde356e69792d61b675) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mk6",          0x0000, 0x2000, CRC(18fbe047) SHA1(d2c6aeb1dfd9751f4db386944d144e293001b497) )
	ROM_LOAD( "mk7",          0x2000, 0x2000, CRC(47dee791) SHA1(9c2d5c2ef1e2e8f329160a1c536119b078803347) )
	ROM_LOAD( "mk8",          0x4000, 0x2000, CRC(9c091ead) SHA1(fce50c9d260f20873289921926bd632d6d49ef15) )
	ROM_LOAD( "mk9",          0x6000, 0x2000, CRC(5c8ed3fe) SHA1(a878fcd547aad5388fef9fe2825c1122444c216d) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "mk12",         0x0000, 0x2000, CRC(8b1a19cf) SHA1(9f75f69828eeaeb2d0dcf20fb80425546124b21e) )
	ROM_LOAD( "mk11",         0x2000, 0x2000, CRC(1a56d24d) SHA1(e64b8a9bdbcf6d2d583ded0750d5f48721785459) )
	ROM_LOAD( "mk10",         0x4000, 0x2000, CRC(e7d05634) SHA1(e7532749fe9b955ba221517807888b34a7754db7) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom.1",       0x0000, 0x0020, NO_DUMP ) /* palette */
	ROM_LOAD( "prom.3",       0x0020, 0x0100, NO_DUMP ) /* sprite lookup table */
	ROM_LOAD( "prom.2",       0x0120, 0x0100, NO_DUMP ) /* char lookup table */

	ROM_REGION( 0x10000, "vlm", 0 )	/* 64k for speech rom */
	ROM_LOAD( "mk2",          0x0000, 0x2000, CRC(f546a56b) SHA1(caee3d8546eb7a75ce2a578c6a1a630246aec6b8) )
ROM_END

ROM_START( wizzquiz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pros.rom",     0xe000, 0x2000, CRC(4c858841) SHA1(78858bd4021d19415cd5f0db21b508880b298c1d) )

	ROM_REGION( 0x40000, "user1", 0 )    /* questions data */
	ROM_LOAD( "sn1.rom",      0x02000, 0x6000, CRC(0ae28676) SHA1(40dbf84b710a8d92939fb698f0393023462f6b23) )
	ROM_CONTINUE(             0x00000, 0x2000 )
	ROM_LOAD( "sn2.rom",      0x0a000, 0x6000, CRC(f2b7374a) SHA1(c0afcca551523748dd236254a0765ffd949a7f6d) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_LOAD( "tvmov1.rom",   0x12000, 0x6000, CRC(921f551d) SHA1(2077ee5f29689ac46c932b74e63a482adcdc7670) )
	ROM_CONTINUE(             0x10000, 0x2000 )
	ROM_LOAD( "tvmov2.rom",   0x1a000, 0x6000, CRC(1ed44df6) SHA1(871a53340ad396ff96a5c57f1c7fcb0cd5931301) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_LOAD( "sport1.rom",   0x22000, 0x6000, CRC(3b7f2ce4) SHA1(f655995961db6782c477b46e4c2478e367ff0d44) )
	ROM_CONTINUE(             0x20000, 0x2000 )
	ROM_LOAD( "sport2.rom",   0x2a000, 0x6000, CRC(14dbfa23) SHA1(71a0124de99c7d4401cf24facc9460360e34c904) )
	ROM_CONTINUE(             0x28000, 0x2000 )
	ROM_LOAD( "pop1.rom",     0x32000, 0x6000, CRC(61f60def) SHA1(3a3508d2cc48654643d16dc607e6957d8e8b0270) )
	ROM_CONTINUE(             0x30000, 0x2000 )
	ROM_LOAD( "pop2.rom",     0x3a000, 0x6000, CRC(5a5b41cd) SHA1(31b2cdc74925b4666820d1d6febcb1358312bbdf) )
	ROM_CONTINUE(             0x38000, 0x2000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "zandz.2c",     0x0000, 0x2000, CRC(3daca93a) SHA1(743c2b787aeb2c893ea476efc95d92e33b9bd159) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "rom.11c",      0x0000, 0x2000, CRC(87d060d4) SHA1(22da2dfaf71d78a4789ca34c27571733ab65ea30) )
	ROM_LOAD( "rom.14c",      0x2000, 0x2000, CRC(5bff1607) SHA1(20c4b74c93511f9cafd6e3f2d048baad3a3a8aa4) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "rom.16h",      0x0000, 0x2000, CRC(e6728bda) SHA1(8bd029af5136b0ed6c0087989c69f0b1c23305fb) )
	ROM_LOAD( "rom.15h",      0x2000, 0x2000, CRC(9c067ef4) SHA1(2a66beee4fa76d40ca18637c0061b196d3873df3) )
	ROM_LOAD( "rom.14h",      0x4000, 0x2000, CRC(3bbad920) SHA1(f5c491f37aa6855181c62fe6bb2975c7d011cc72) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "361b16.f1",    0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b16",   0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.e15",   0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */

	ROM_REGION( 0x10000, "vlm", ROMREGION_ERASE00 )	/* 64k for speech rom */
	/* not used */
ROM_END

ROM_START( wizzquza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic9_a1.bin",   0xe000, 0x2000, CRC(608e1ff3) SHA1(f3350a3367df59ec1780bb22c7a6a227e7b10d5e) )

	ROM_REGION( 0x40000, "user1", 0 )    /* questions data */
	ROM_LOAD( "ic1_q06.bin",  0x02000, 0x6000, CRC(c62f25b1) SHA1(22694716b2675dd0c725ce788bb0ffe7a1808cf6) )
	ROM_CONTINUE(             0x00000, 0x2000 )
	ROM_LOAD( "ic2_q28.bin",  0x0a000, 0x6000, CRC(2bd00476) SHA1(88ed9d26909873c52273290686b4783563edfb61) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_LOAD( "ic3_q27.bin",  0x12000, 0x6000, CRC(46d28aaf) SHA1(af19b166eabdab59712eb755ae3d83545ea7db62) )
	ROM_CONTINUE(             0x10000, 0x2000 )
	ROM_LOAD( "ic4_q23.bin",  0x1a000, 0x6000, CRC(3f46f702) SHA1(f41a9ea5a47f2677cea8ad55847860a955521374) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_LOAD( "ic5_q26.bin",  0x22000, 0x6000, CRC(9d130515) SHA1(bfc32219d4d4eaca4efa02c3c46125144c8cd286) )
	ROM_CONTINUE(             0x20000, 0x2000 )
	ROM_LOAD( "ic6_q09.bin",  0x2a000, 0x6000, CRC(636f89b4) SHA1(0b9b471e52fff343f9c7e7b1212f03aba52839f2) )
	ROM_CONTINUE(             0x28000, 0x2000 )
	ROM_LOAD( "ic7_q15.bin",  0x32000, 0x6000, CRC(b35332b1) SHA1(18c5cf3cc6fb6d1fe6d672d745d22b2498d8324e) )
	ROM_CONTINUE(             0x30000, 0x2000 )
	ROM_LOAD( "ic8_q19.bin",  0x3a000, 0x6000, CRC(8d152da0) SHA1(8404256775b6236d80869f5023d912aa9ebb6582) )
	ROM_CONTINUE(             0x38000, 0x2000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "02c.bin",      0x0000, 0x2000, CRC(3daca93a) SHA1(743c2b787aeb2c893ea476efc95d92e33b9bd159) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "11c.bin",      0x0000, 0x2000, CRC(87d060d4) SHA1(22da2dfaf71d78a4789ca34c27571733ab65ea30) )
	ROM_LOAD( "14c.bin",      0x2000, 0x2000, CRC(5bff1607) SHA1(20c4b74c93511f9cafd6e3f2d048baad3a3a8aa4) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "16h.bin",      0x0000, 0x2000, CRC(e6728bda) SHA1(8bd029af5136b0ed6c0087989c69f0b1c23305fb) )
	ROM_LOAD( "15h.bin",      0x2000, 0x2000, CRC(9c067ef4) SHA1(2a66beee4fa76d40ca18637c0061b196d3873df3) )
	ROM_LOAD( "14h.bin",      0x4000, 0x2000, CRC(3bbad920) SHA1(f5c491f37aa6855181c62fe6bb2975c7d011cc72) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "361b16.f1",    0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b16",   0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.e15",   0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */

	ROM_REGION( 0x10000, "vlm", ROMREGION_ERASE00 )	/* 64k for speech rom */
	/* not used */
ROM_END

ROM_START( reaktor )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "prog3.bin",  0x0000, 0x8000, CRC(8ba956fa) SHA1(8085b85da1b81f5d9e0da80fcfec44d70f59c208) )

	/* most of these were 27128 roms, but they have identical halves, 2764 chips could have been used
       instead, and one was actually used for rom 12c.  I'm not cutting the others because this is the
       form in which they were found */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2c.bin",   0x0000, 0x2000, CRC(105a8beb) SHA1(4bd9a0076fece8dc9a830e76a60fbcefe08940f7) )
	ROM_CONTINUE(0x0000,0x2000)

	ROM_REGION( 0x8000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "11c.bin",  0x0000, 0x2000, CRC(d24553fa) SHA1(ad4f2dd6c7236f33841bc59ce150a212fbe871cd) )
	ROM_CONTINUE(0x0000,0x2000)
	ROM_LOAD( "14c.bin",  0x4000, 0x2000, CRC(4d0ab831) SHA1(2009b263fff3fd512a055fef23e667e76af1c584) )
	ROM_CONTINUE(0x4000,0x2000)
	ROM_LOAD( "12c.bin",  0x2000, 0x2000, CRC(d0d39e66) SHA1(769fb526f6cd4b016fcfe9d08710fdb456cb4e47) )

	ROM_LOAD( "15c.bin",  0x6000, 0x2000, CRC(bf1e608d) SHA1(ad5f16c091439358bbece9bc50e5979d44e85980) )
	ROM_CONTINUE(0x6000,0x2000)

	ROM_REGION( 0x6000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "16h.bin",  0x0000, 0x2000, CRC(cb062c3b) SHA1(4a1c1a662dec26cb49310de596e1e1416d101d5d) )
	ROM_CONTINUE(0x0000,0x2000)
	ROM_LOAD( "15h.bin",  0x2000, 0x2000, CRC(df83e659) SHA1(435523f3747c5aaf0a2d3a826766cb9b9ebb821e) )
	ROM_CONTINUE(0x2000,0x2000)
	ROM_LOAD( "14h.bin",  0x4000, 0x2000, CRC(5ca53215) SHA1(650338a95465b61d9388bede716053523855eeee) )
	ROM_CONTINUE(0x4000,0x2000)

	/* Proms, and speech rom (unused?) are unchanged from the original */
	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "361b16.f1",    0x0000, 0x0020, CRC(d55f30b5) SHA1(4d6a851f4886778307f75771645078b97ad55f5f) ) /* palette */
	ROM_LOAD( "361b17.b16",   0x0020, 0x0100, CRC(d2ba4d32) SHA1(894b5cedf01ba9225a0d6215291857e455b84903) ) /* sprite lookup table */
	ROM_LOAD( "361b18.e15",   0x0120, 0x0100, CRC(053e5861) SHA1(6740a62cf7b6938a4f936a2fed429704612060a5) ) /* char lookup table */

	ROM_REGION( 0x10000, "vlm", 0 )	/* 64k for speech rom */
	ROM_LOAD( "c9_d15.bin",   0x0000, 0x2000, CRC(f546a56b) SHA1(caee3d8546eb7a75ce2a578c6a1a630246aec6b8) )
ROM_END

static DRIVER_INIT( trackfld )
{
	konami1_decode(machine, "maincpu");
}

static DRIVER_INIT( atlantol )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT8 *rom = memory_region(machine, "maincpu");
	UINT8 *decrypt;
	int A;

	/* "konami1" encrypted opcodes */
	decrypt = konami1_decode(machine, "maincpu");

	/* not encrypted opcodes */
	for (A = 0;A < 0x6000;A++)
		decrypt[A] = rom[A];

	memory_set_decrypted_region(space, 0x0000, 0xffff, decrypt);

	memory_install_write8_handler(space, 0x0800, 0x0800, 0, 0, atlantol_gfxbank_w);
	memory_install_write8_handler(space, 0x1000, 0x1000, 0, 0, (write8_space_func)SMH_NOP);

	/* unmapped areas read as ROM */
	memory_install_read8_handler(space, 0x0000, 0x11ff, 0, 0, (read8_space_func)SMH_BANK(10));
	memory_install_read8_handler(space, 0x1380, 0x17ff, 0, 0, (read8_space_func)SMH_BANK(11));
	memory_install_read8_handler(space, 0x2000, 0x27ff, 0, 0, (read8_space_func)SMH_BANK(12));
	memory_install_read8_handler(space, 0x4000, 0x5fff, 0, 0, (read8_space_func)SMH_BANK(13));
	memory_set_bankptr(machine, 10, &rom[0x0000]);
	memory_set_bankptr(machine, 11, &rom[0x1380]);
	memory_set_bankptr(machine, 12, &rom[0x2000]);
	memory_set_bankptr(machine, 13, &rom[0x4000]);
}

static DRIVER_INIT( mastkin )
{
	UINT8 *prom = memory_region(machine, "proms");
	int i;

	/* build a fake palette so the screen won't be all black */
	for (i = 0; i < 0x20; i++)
	{
		prom[i] = i * 4;
	}

	/* build a fake lookup table since we don't have the color PROMs */
	for (i = 0; i < 0x0200; i++)
	{
		if ((i & 0x0f) == 0)
			prom[i + 0x20] = 0;
		else
			prom[i + 0x20] = (i + i / 16) & 0x0f;
	}
}

static DRIVER_INIT( wizzquiz )
{
	UINT8 *ROM = memory_region(machine, "maincpu") + 0xe000;
	int i;

	/* decrypt program rom */
	for( i = 0; i < 0x2000; i++ )
		ROM[i] = BITSWAP8(ROM[i],0,1,2,3,4,5,6,7);

	ROM = memory_region(machine, "user1");

	/* decrypt questions roms */
	for( i = 0; i < 0x40000; i++ )
		ROM[i] = BITSWAP8(ROM[i],0,1,2,3,4,5,6,7);

	memory_configure_bank(machine, 1, 0, 8, ROM, 0x8000);
}


GAME( 1983, trackfld, 0,        trackfld, trackfld, trackfld, ROT0,  "Konami", "Track & Field", 0 )
GAME( 1983, trackflc, trackfld, trackfld, trackfld, trackfld, ROT0,  "Konami (Centuri license)", "Track & Field (Centuri)", 0 )
GAME( 1983, hyprolym, trackfld, trackfld, trackfld, trackfld, ROT0,  "Konami", "Hyper Olympic", 0 )
GAME( 1983, hyprolyb, trackfld, hyprolyb, trackfld, trackfld, ROT0,  "bootleg", "Hyper Olympic (bootleg)", GAME_IMPERFECT_SOUND )
GAME( 1996, atlantol, trackfld, hyprolyb, atlantol,	atlantol, ROT0,  "bootleg", "Atlant Olimpic", 0 )
GAME( 1988, mastkin,  0,        mastkin,  mastkin,  mastkin,  ROT0,  "Du Tech", "The Masters of Kin", GAME_WRONG_COLORS )
GAME( 1985, wizzquiz, 0,        wizzquiz, wizzquiz, wizzquiz, ROT0,  "Konami", "Wizz Quiz (Konami version)", 0 )
GAME( 1985, wizzquza, wizzquiz, wizzquiz, wizzquiz, wizzquiz, ROT0,  "Zilec - Zenitone", "Wizz Quiz (version 4)", 0 )
GAME( 1987, reaktor,  0,        reaktor,  reaktor,  0,        ROT90, "Zilec", "Reaktor (Track & Field conversion)", 0 )
