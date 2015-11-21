// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/****************************************************************************

Some Dynax games using the first version of their blitter

driver by Nicola Salmoria, blitter support based on work by Luca Elia

CPU:    Z80-A
Sound:  YM2203C
        M5205
OSC:    20.0000MHz
Video:  HD46505SP

---------------------------------------
Year + Game                 Board
---------------------------------------
87 Hana Yayoi               D0208298L1
87 Hana Fubuki              D0602048
87 Untouchable              D0806298
---------------------------------------

Notes:
- In service mode, press "analyzer" (0) and "test" (F1) to see a gfx test

- hnfubuki doesn't have a service mode dip, press "analyzer" instead

- untoucha doesn't have it either; press "test" during boot for one kind
  of service menu, "analyzer" at any other time for another menu (including
  dip switch settings)

TODO:
- dips/inputs for all games

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "machine/nvram.h"
#include "includes/hnayayoi.h"


READ8_MEMBER(hnayayoi_state::keyboard_0_r)
{
	int res = 0x3f;
	int i;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };

	for (i = 0; i < 5; i++)
	{
		if (~m_keyb & (1 << i))
			res &= ioport(keynames[i])->read();
	}

	return res;
}

READ8_MEMBER(hnayayoi_state::keyboard_1_r)
{
	/* Player 2 not supported */
	return 0x3f;
}

WRITE8_MEMBER(hnayayoi_state::keyboard_w)
{
	m_keyb = data;
}


WRITE8_MEMBER(hnayayoi_state::adpcm_data_w)
{
	m_msm->data_w(data);
}

WRITE8_MEMBER(hnayayoi_state::adpcm_vclk_w)
{
	m_msm->vclk_w(data & 1);
}

WRITE8_MEMBER(hnayayoi_state::adpcm_reset_w)
{
	m_msm->reset_w(data & 1);
}

WRITE8_MEMBER(hnayayoi_state::adpcm_reset_inv_w)
{
	m_msm->reset_w(~data & 1);
}


static ADDRESS_MAP_START( hnayayoi_map, AS_PROGRAM, 8, hnayayoi_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hnayayoi_io_map, AS_IO, 8, hnayayoi_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ymsnd", ym2203_device, write)
	AM_RANGE(0x02, 0x03) AM_DEVREAD("ymsnd", ym2203_device, read)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW3")
	AM_RANGE(0x06, 0x06) AM_WRITE(adpcm_data_w)
//  AM_RANGE(0x08, 0x08) AM_WRITENOP // CRT Controller
//  AM_RANGE(0x09, 0x09) AM_WRITENOP // CRT Controller
	AM_RANGE(0x0a, 0x0a) AM_WRITE(dynax_blitter_rev1_start_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(dynax_blitter_rev1_clear_w)
	AM_RANGE(0x23, 0x23) AM_WRITE(adpcm_vclk_w)
	AM_RANGE(0x24, 0x24) AM_WRITE(adpcm_reset_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(keyboard_w)
	AM_RANGE(0x41, 0x41) AM_READ(keyboard_0_r)
	AM_RANGE(0x42, 0x42) AM_READ(keyboard_1_r)
	AM_RANGE(0x43, 0x43) AM_READ_PORT("COIN")
	AM_RANGE(0x60, 0x61) AM_WRITE(hnayayoi_palbank_w)
	AM_RANGE(0x62, 0x67) AM_WRITE(dynax_blitter_rev1_param_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hnfubuki_map, AS_PROGRAM, 8, hnayayoi_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8000, 0xfeff) AM_ROM
	AM_RANGE(0xff00, 0xff01) AM_DEVWRITE("ymsnd", ym2203_device, write)
	AM_RANGE(0xff02, 0xff03) AM_DEVREAD("ymsnd", ym2203_device, read)
	AM_RANGE(0xff04, 0xff04) AM_READ_PORT("DSW3")
	AM_RANGE(0xff06, 0xff06) AM_WRITE(adpcm_data_w)
//  AM_RANGE(0xff08, 0xff08) AM_WRITENOP // CRT Controller
//  AM_RANGE(0xff09, 0xff09) AM_WRITENOP // CRT Controller
	AM_RANGE(0xff0a, 0xff0a) AM_WRITE(dynax_blitter_rev1_start_w)
	AM_RANGE(0xff0c, 0xff0c) AM_WRITE(dynax_blitter_rev1_clear_w)
	AM_RANGE(0xff23, 0xff23) AM_WRITE(adpcm_vclk_w)
	AM_RANGE(0xff24, 0xff24) AM_WRITE(adpcm_reset_inv_w)
	AM_RANGE(0xff40, 0xff40) AM_WRITE(keyboard_w)
	AM_RANGE(0xff41, 0xff41) AM_READ(keyboard_0_r)
	AM_RANGE(0xff42, 0xff42) AM_READ(keyboard_1_r)
	AM_RANGE(0xff43, 0xff43) AM_READ_PORT("COIN")
	AM_RANGE(0xff60, 0xff61) AM_WRITE(hnayayoi_palbank_w)
	AM_RANGE(0xff62, 0xff67) AM_WRITE(dynax_blitter_rev1_param_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( untoucha_map, AS_PROGRAM, 8, hnayayoi_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( untoucha_io_map, AS_IO, 8, hnayayoi_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("ymsnd", ym2203_device, control_port_w)
	AM_RANGE(0x11, 0x11) AM_DEVREAD("ymsnd", ym2203_device, status_port_r)
//  AM_RANGE(0x12, 0x12) AM_WRITENOP // CRT Controller
	AM_RANGE(0x13, 0x13) AM_WRITE(adpcm_data_w)
	AM_RANGE(0x14, 0x14) AM_READ_PORT("COIN")
	AM_RANGE(0x15, 0x15) AM_READ(keyboard_1_r)
	AM_RANGE(0x16, 0x16) AM_READ(keyboard_0_r)  // bit 7 = blitter busy flag
	AM_RANGE(0x17, 0x17) AM_WRITE(keyboard_w)
	AM_RANGE(0x18, 0x19) AM_WRITE(hnayayoi_palbank_w)
	AM_RANGE(0x1a, 0x1f) AM_WRITE(dynax_blitter_rev1_param_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(dynax_blitter_rev1_clear_w)
	AM_RANGE(0x28, 0x28) AM_WRITE(dynax_blitter_rev1_start_w)
	AM_RANGE(0x31, 0x31) AM_WRITE(adpcm_vclk_w)
	AM_RANGE(0x32, 0x32) AM_WRITE(adpcm_reset_inv_w)
	AM_RANGE(0x50, 0x50) AM_DEVWRITE("ymsnd", ym2203_device, write_port_w)
	AM_RANGE(0x51, 0x51) AM_DEVREAD("ymsnd", ym2203_device, read_port_r)
//  AM_RANGE(0x52, 0x52) AM_WRITENOP // CRT Controller
ADDRESS_MAP_END

static INPUT_PORTS_START( hf_keyboard )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( hnayayoi )
	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* DSW3 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )   // blitter busy flag
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN")  /* COIN */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)   /* Test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )   /* Analizer (Statistics) */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  /* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( hf_keyboard )
INPUT_PORTS_END

static INPUT_PORTS_START( hnfubuki )
	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* DSW3 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )   // blitter busy flag
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN")  /* COIN */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)   /* Test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )   /* Analizer (Statistics) */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  /* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( hf_keyboard )
INPUT_PORTS_END


static INPUT_PORTS_START( untoucha )
	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN")  /* COIN */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR(Test)) PORT_CODE(KEYCODE_F1) /* Test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )   /* Analizer (Statistics) */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  /* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("KEY0")  /* P1 keyboard */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flip Flop") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



WRITE_LINE_MEMBER(hnayayoi_state::irqhandler)
{
	popmessage("irq");
//  m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void hnayayoi_state::machine_start()
{
	save_item(NAME(m_palbank));
	save_item(NAME(m_blit_layer));
	save_item(NAME(m_blit_dest));
	save_item(NAME(m_blit_src));
	save_item(NAME(m_keyb));
}

void hnayayoi_state::machine_reset()
{
	/* start with the MSM5205 reset */
	m_msm->reset_w(1);

	m_palbank = 0;
	m_blit_layer = 0;
	m_blit_dest = 0;
	m_blit_src = 0;
	m_keyb = 0;
}


static MACHINE_CONFIG_START( hnayayoi, hnayayoi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 20000000/4 )        /* 5 MHz ???? */
	MCFG_CPU_PROGRAM_MAP(hnayayoi_map)
	MCFG_CPU_IO_MAP(hnayayoi_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", hnayayoi_state,  irq0_line_hold)
	MCFG_CPU_PERIODIC_INT_DRIVER(hnayayoi_state, nmi_line_pulse,  8000)


	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(hnayayoi_state, screen_update_hnayayoi)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 20000000/8)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(hnayayoi_state, irqhandler))
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 0.80)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hnfubuki, hnayayoi )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hnfubuki_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( untoucha, hnayayoi )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(untoucha_map)
	MCFG_CPU_IO_MAP(untoucha_io_map)

	MCFG_VIDEO_START_OVERRIDE(hnayayoi_state,untoucha)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( hnayayoi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "021.4a",     0x00000, 0x08000, CRC(d9734da4) SHA1(a2c8f5113c8136bea990c282d60f67b2793f9a2c) )
	ROM_LOAD( "022.3a",     0x08000, 0x08000, CRC(e6be5af4) SHA1(cdc56705ba0d191930f892618512cb687975ecbb) )

	ROM_REGION( 0x38000, "gfx1", 0 )    /* blitter data */
	ROM_LOAD( "023.8f",     0x00000, 0x08000, CRC(81ae7317) SHA1(9e37dad046420138b4655d0692fe4bac3a8e09de) )
	ROM_LOAD( "024.9f",     0x08000, 0x08000, CRC(413ab77a) SHA1(4b44d2a76c37f25f126e3759ab61fadba02e2b55) )
	ROM_LOAD( "025.10f",    0x10000, 0x08000, CRC(56d16426) SHA1(38162f2a240ce6828232d4280120acc576f71200) )
	ROM_LOAD( "026.12f",    0x18000, 0x08000, CRC(a99779d9) SHA1(1c4586c5070ec9646440cafefa8cf8a550ad17bd) )
	ROM_LOAD( "027.8d",     0x20000, 0x08000, CRC(209c149a) SHA1(4b7232f1b8b7f88fd8e53b719baff85afce72541) )
	ROM_LOAD( "028.9d",     0x28000, 0x08000, CRC(6981b043) SHA1(9d8f54945bce4ef149290ff34ef6e4532f4e6e8f) )
	ROM_LOAD( "029.10d",    0x30000, 0x08000, CRC(a266f1eb) SHA1(525d8c350cdad2461cb814e4eaa785e84b6fc529) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "r.16b",      0x0000, 0x0100, CRC(b6e9ac04) SHA1(f3ec1c285d40e6189ad8722fab202d2b877f7f75) )
	ROM_LOAD( "g.17b",      0x0100, 0x0100, CRC(a595f310) SHA1(6bd16897d3abcceae76a613a1296992c2e9e1a37) )
	ROM_LOAD( "b.17c",      0x0200, 0x0100, CRC(e33bd9ea) SHA1(47395511d7bd44a27fa55d43094c67c5551cdfbf) )
ROM_END

ROM_START( hnfubuki )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s1.s2c",     0x00000, 0x10000, CRC(afe3179c) SHA1(fdfba1e7073318f9782d628f3c7dd0d9c84cbeea) )

	ROM_REGION( 0x40000, "gfx1", 0 )    /* blitter data */
	ROM_LOAD( "062.8f",     0x00000, 0x10000, CRC(0d96a540) SHA1(1cadf19d8fd48962acb0e45a50431fabd6f13672) )
	ROM_LOAD( "063.9f",     0x10000, 0x10000, CRC(14250093) SHA1(8459024ebe5f8c3fa146e3303a155c2cf5c487b3) )
	ROM_LOAD( "064.10f",    0x20000, 0x10000, CRC(41546fb9) SHA1(3c6028c19aa65dcb7ccfc01c223c2cba36cc9bb4) )
	ROM_LOAD( "0652.12f",   0x30000, 0x10000, CRC(e7b54ea3) SHA1(157306db5f3e3cc652fcdb7b42d33f0b93eda07b) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "r-16b",      0x0000, 0x0100, CRC(e6fd8f5d) SHA1(fb9dedd46c4aaa707d4f7d2614227435eea0bca3) )
	ROM_LOAD( "g-17b",      0x0100, 0x0100, CRC(3f425f67) SHA1(3b0c6585d74871638681749f7162b0780318645c) )
	ROM_LOAD( "b-17c",      0x0200, 0x0100, CRC(d1f912e5) SHA1(0e0b61498ef5e4685754c0786b7288156b8a56ce) )
ROM_END

ROM_START( untoucha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4.10b",     0x00000, 0x10000, CRC(4df04e41) SHA1(4d5232c2f383640394d85417aa973f92c78184c9) )

	ROM_REGION( 0x90000, "gfx1", 0 )    /* blitter data */
	ROM_LOAD( "081.10f",    0x00000, 0x10000, CRC(36ba990d) SHA1(10b2865f1d19c01cc898029a23489f47ade2ce86) )
	ROM_LOAD( "082.12f",    0x10000, 0x10000, CRC(2beb6277) SHA1(ea57970051c674800a9bedd581d734bd9beaa894) )
	ROM_LOAD( "083.13f",    0x20000, 0x10000, CRC(c3fed8ff) SHA1(405a6563ff7420686063e04fb99dfe6f0f7378dc) )
	ROM_LOAD( "084.14f",    0x30000, 0x10000, CRC(10de3aae) SHA1(c006f58f03f261ed104da870e944e3e3bed3ecfe) )
	ROM_LOAD( "085.16f",    0x40000, 0x10000, CRC(527e5879) SHA1(3a126e64890157f346320923427889ab24a39fcc) )
	ROM_LOAD( "086.10h",    0x50000, 0x10000, CRC(be3f0a2e) SHA1(46b52e05c340d32de49769ebf69c9f4f8c1baf1e) )
	ROM_LOAD( "087.12h",    0x60000, 0x10000, CRC(35e072b7) SHA1(e15d56fbc7ec71b21a22a12121ab6d2bfdfc530d) )
	ROM_LOAD( "088.13h",    0x70000, 0x10000, CRC(742cf3c0) SHA1(542fe62feea752e57895c20932865d53953e4945) )
	ROM_LOAD( "089.14h",    0x80000, 0x10000, CRC(ff497db1) SHA1(c786d1b6d07c889e6fc1f812d7dac50674eae356) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "08r.9f",     0x0000, 0x0100, CRC(308e65b4) SHA1(73ec6df141502270148d898376f0ed3211436f04) )
	ROM_LOAD( "08g.8f",     0x0100, 0x0100, CRC(349c3de3) SHA1(ffbbc3a049a749cea5867f575ac77cfd8f5299b1) )
	ROM_LOAD( "08b.7f",     0x0200, 0x0100, CRC(2007435a) SHA1(2fae6f349b36f700e65ae9b980adce691bdc6dde) )
ROM_END


DRIVER_INIT_MEMBER(hnayayoi_state,hnfubuki)
{
	UINT8 *rom = memregion("gfx1")->base();
	int len = memregion("gfx1")->bytes();
	int i, j;

	/* interestingly, the blitter data has a slight encryption */

	/* swap address bits 4 and 5 */
	for (i = 0; i < len; i += 0x40)
	{
		for (j = 0; j < 0x10; j++)
		{
			UINT8 t = rom[i + j + 0x10];
			rom[i + j + 0x10] = rom[i + j + 0x20];
			rom[i + j + 0x20] = t;
		}
	}

	/* swap data bits 0 and 1 */
	for (i = 0; i < len; i++)
	{
		rom[i] = BITSWAP8(rom[i],7,6,5,4,3,2,0,1);
	}
}


GAME( 1987, hnayayoi, 0,        hnayayoi, hnayayoi, driver_device, 0,        ROT0, "Dyna Electronics", "Hana Yayoi (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, hnfubuki, hnayayoi, hnfubuki, hnfubuki, hnayayoi_state, hnfubuki, ROT0, "Dynax", "Hana Fubuki [BET] (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, untoucha, 0,        untoucha, untoucha, driver_device, 0,        ROT0, "Dynax", "Untouchable (Japan)", MACHINE_SUPPORTS_SAVE )
