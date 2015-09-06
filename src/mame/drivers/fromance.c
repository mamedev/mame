// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi. Bryan McPhail, Nicola Salmoria, Aaron Giles
/******************************************************************************

    Game Driver for Video System Mahjong series.

    Idol-Mahjong Final Romance (Japan)
    (c)1991 Video System Co.,Ltd.

    Nekketsu Mahjong Sengen! AFTER 5 (Japan)
    (c)1991 Video System Co.,Ltd.

    Mahjong Daiyogen (Japan)
    (c)1990 Video System Co.,Ltd.

    Mahjong Fun Club - Idol Saizensen (Japan)
    (c)1989 Video System Co.,Ltd.

    Mahjong Natsu Monogatari (Mahjong Summer Story) (Japan)
    (c)1989 Video System Co.,Ltd.

    Natsuiro Mahjong (Mahjong Summer Story) (Japan)
    (c)1989 Video System Co.,Ltd.

    Idol-Mahjong Housoukyoku (Japan)
    (c)1988 System Service Co.,Ltd.

    Rettou Juudan Nekkyoku Janshi - Higashi Nippon Hen (Japan)
    (c)1988 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Nicola Salmoria, Aaron Giles

******************************************************************************/
/******************************************************************************
Memo:

- 2player's input is not supported.

- Communication between MAIN CPU and SUB CPU can be wrong.

Notes:

idolmj tries to read at 0x9e89 (happened to me when I've declared rii'chi),
with the following code:

790A: 3A 89 9E      ld   a,($9E89)
790D: BB            cp   e
790E: 20 01         jr   nz,$7911
7910: 0C            inc  c
7911: 7B            ld   a,e
7912: FD 6E 07      ld   l,(iy+$07)
7915: FD 66 08      ld   h,(iy+$08)
7918: 11 80 03      ld   de,$0380
791B: 19            add  hl,de
791C: C5            push bc
791D: 46            ld   b,(hl)
791E: 23            inc  hl
791F: BE            cp   (hl)
7920: 20 01         jr   nz,$7923
7922: 0C            inc  c
7923: 10 F9         djnz $791E
7925: 79            ld   a,c
7926: C1            pop  bc
7927: 4F            ld   c,a
7928: E1            pop  hl
7929: 10 D3         djnz $78FE
792B: FD 71 33      ld   (iy+$33),c
792E: C9            ret

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/2413intf.h"
#include "sound/msm5205.h"
#include "includes/fromance.h"


/*************************************
 *
 *  Master/slave communication
 *
 *************************************/

READ8_MEMBER(fromance_state::fromance_commanddata_r)
{
	return m_commanddata;
}


TIMER_CALLBACK_MEMBER(fromance_state::deferred_commanddata_w)
{
	m_commanddata = param;
	m_directionflag = 1;
}


WRITE8_MEMBER(fromance_state::fromance_commanddata_w)
{
	/* do this on a timer to let the slave CPU synchronize */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(fromance_state::deferred_commanddata_w),this), data);
}


READ8_MEMBER(fromance_state::fromance_busycheck_main_r)
{
	/* set a timer to force synchronization after the read */
	machine().scheduler().synchronize();

	if (!m_directionflag)
		return 0x00;        // standby
	else
		return 0xff;        // busy
}


READ8_MEMBER(fromance_state::fromance_busycheck_sub_r)
{
	if (m_directionflag)
		return 0xff;        // standby
	else
		return 0x00;        // busy
}


WRITE8_MEMBER(fromance_state::fromance_busycheck_sub_w)
{
	m_directionflag = 0;
}



/*************************************
 *
 *  Slave CPU ROM banking
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_rombank_w)
{
	membank("bank1")->set_entry(data);
}



/*************************************
 *
 *  ADPCM interface
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_adpcm_reset_w)
{
	m_adpcm_reset = (data & 0x01);
	m_vclk_left = 0;

	m_msm->reset_w(!(data & 0x01));
}


WRITE8_MEMBER(fromance_state::fromance_adpcm_w)
{
	m_adpcm_data = data;
	m_vclk_left = 2;
}


WRITE_LINE_MEMBER(fromance_state::fromance_adpcm_int)
{
	/* skip if we're reset */
	if (!m_adpcm_reset)
		return;

	/* clock the data through */
	if (m_vclk_left)
	{
		m_msm->data_w(m_adpcm_data >> 4);
		m_adpcm_data <<= 4;
		m_vclk_left--;
	}

	/* generate an NMI if we're out of data */
	if (!m_vclk_left)
		m_subcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}



/*************************************
 *
 *  Input handlers
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_portselect_w)
{
	m_portselect = data;
}


READ8_MEMBER(fromance_state::fromance_keymatrix_r)
{
	int ret = 0xff;

	if (m_portselect & 0x01)
		ret &= ioport("KEY1")->read();
	if (m_portselect & 0x02)
		ret &= ioport("KEY2")->read();
	if (m_portselect & 0x04)
		ret &= ioport("KEY3")->read();
	if (m_portselect & 0x08)
		ret &= ioport("KEY4")->read();
	if (m_portselect & 0x10)
		ret &= ioport("KEY5")->read();

	return ret;
}



/*************************************
 *
 *  Coin counters
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_coinctr_w)
{
	//
}



/*************************************
 *
 *  Master CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( nekkyoku_main_map, AS_PROGRAM, 8, fromance_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("SERVICE") AM_WRITE(fromance_portselect_w)
	AM_RANGE(0xf001, 0xf001) AM_READ(fromance_keymatrix_r) AM_WRITENOP
	AM_RANGE(0xf002, 0xf002) AM_READ_PORT("COIN") AM_WRITE(fromance_coinctr_w)
	AM_RANGE(0xf003, 0xf003) AM_READWRITE(fromance_busycheck_main_r, fromance_commanddata_w)
	AM_RANGE(0xf004, 0xf004) AM_READ_PORT("DSW2")
	AM_RANGE(0xf005, 0xf005) AM_READ_PORT("DSW1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( fromance_main_map, AS_PROGRAM, 8, fromance_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0x9e89, 0x9e89) AM_READNOP         // unknown (idolmj)
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("SERVICE") AM_WRITE(fromance_portselect_w)
	AM_RANGE(0xe001, 0xe001) AM_READ(fromance_keymatrix_r)
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("COIN") AM_WRITE(fromance_coinctr_w)
	AM_RANGE(0xe003, 0xe003) AM_READWRITE(fromance_busycheck_main_r, fromance_commanddata_w)
	AM_RANGE(0xe004, 0xe004) AM_READ_PORT("DSW2")
	AM_RANGE(0xe005, 0xe005) AM_READ_PORT("DSW1")
ADDRESS_MAP_END



/*************************************
 *
 *  Slave CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( nekkyoku_sub_map, AS_PROGRAM, 8, fromance_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xefff) AM_READWRITE(fromance_videoram_r, fromance_videoram_w)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(fromance_paletteram_r, fromance_paletteram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fromance_sub_map, AS_PROGRAM, 8, fromance_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(fromance_paletteram_r, fromance_paletteram_w)
	AM_RANGE(0xd000, 0xffff) AM_READWRITE(fromance_videoram_r, fromance_videoram_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Slave CPU port handlers
 *
 *************************************/

static ADDRESS_MAP_START( nekkyoku_sub_io_map, AS_IO, 8, fromance_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITE(fromance_crtc_data_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(fromance_crtc_register_w)
	AM_RANGE(0x12, 0x12) AM_READNOP             // unknown
	AM_RANGE(0xe0, 0xe0) AM_WRITE(fromance_rombank_w)
	AM_RANGE(0xe1, 0xe1) AM_READ(fromance_busycheck_sub_r) AM_WRITE(fromance_gfxreg_w)
	AM_RANGE(0xe2, 0xe5) AM_WRITE(fromance_scroll_w)
	AM_RANGE(0xe6, 0xe6) AM_READWRITE(fromance_commanddata_r, fromance_busycheck_sub_w)
	AM_RANGE(0xe7, 0xe7) AM_WRITE(fromance_adpcm_reset_w)
	AM_RANGE(0xe8, 0xe8) AM_WRITE(fromance_adpcm_w)
	AM_RANGE(0xe9, 0xea) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( idolmj_sub_io_map, AS_IO, 8, fromance_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITE(fromance_crtc_data_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(fromance_crtc_register_w)
	AM_RANGE(0x12, 0x12) AM_READNOP             // unknown
	AM_RANGE(0x20, 0x20) AM_WRITE(fromance_rombank_w)
	AM_RANGE(0x21, 0x21) AM_READ(fromance_busycheck_sub_r) AM_WRITE(fromance_gfxreg_w)
	AM_RANGE(0x22, 0x25) AM_WRITE(fromance_scroll_w)
	AM_RANGE(0x26, 0x26) AM_READWRITE(fromance_commanddata_r, fromance_busycheck_sub_w)
	AM_RANGE(0x27, 0x27) AM_WRITE(fromance_adpcm_reset_w)
	AM_RANGE(0x28, 0x28) AM_WRITE(fromance_adpcm_w)
	AM_RANGE(0x29, 0x2a) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fromance_sub_io_map, AS_IO, 8, fromance_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITE(fromance_crtc_data_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(fromance_crtc_register_w)
	AM_RANGE(0x12, 0x12) AM_READNOP             // unknown
	AM_RANGE(0x20, 0x20) AM_WRITE(fromance_rombank_w)
	AM_RANGE(0x21, 0x21) AM_READ(fromance_busycheck_sub_r) AM_WRITE(fromance_gfxreg_w)
	AM_RANGE(0x22, 0x25) AM_WRITE(fromance_scroll_w)
	AM_RANGE(0x26, 0x26) AM_READWRITE(fromance_commanddata_r, fromance_busycheck_sub_w)
	AM_RANGE(0x27, 0x27) AM_WRITE(fromance_adpcm_reset_w)
	AM_RANGE(0x28, 0x28) AM_WRITE(fromance_adpcm_w)
	AM_RANGE(0x2a, 0x2b) AM_DEVWRITE("ymsnd", ym2413_device, write)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mahjong_panel )
	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( nekkyoku )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE( mahjong_panel )
INPUT_PORTS_END


static INPUT_PORTS_START( idolmj )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )   // MEMORY RESET
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)    // TEST
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )      // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Voices" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE( mahjong_panel )
INPUT_PORTS_END


static INPUT_PORTS_START( fromance )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )   // MEMORY RESET
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)    // TEST
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )      // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE( mahjong_panel )
INPUT_PORTS_END


static INPUT_PORTS_START( nmsengen )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )   // MEMORY RESET
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)    // TEST
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )      // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE( mahjong_panel )
INPUT_PORTS_END


static INPUT_PORTS_START( daiyogen )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )   // MEMORY RESET
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)    // TEST
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )      // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE( mahjong_panel )
INPUT_PORTS_END


static INPUT_PORTS_START( mfunclub )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )   // MEMORY RESET
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)    // TEST
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )      // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Voices" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE( mahjong_panel )
INPUT_PORTS_END


static INPUT_PORTS_START( mjnatsu )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )   // MEMORY RESET
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)    // TEST
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )      // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Voices" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE( mahjong_panel )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout bglayout =
{
	8,4,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32 },
	16*8
};


static GFXDECODE_START( fromance )
	GFXDECODE_ENTRY( "gfx1", 0, bglayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, bglayout,   0, 128 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_START_MEMBER(fromance_state,fromance)
{
	UINT8 *ROM = memregion("sub")->base();

	membank("bank1")->configure_entries(0, 0x100, &ROM[0x10000], 0x4000);


	save_item(NAME(m_directionflag));
	save_item(NAME(m_commanddata));
	save_item(NAME(m_portselect));

	save_item(NAME(m_adpcm_reset));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_vclk_left));

	/* video-related elements are saved in video_start */
}

MACHINE_RESET_MEMBER(fromance_state,fromance)
{
	int i;

	m_directionflag = 0;
	m_commanddata = 0;
	m_portselect = 0;

	m_adpcm_reset = 0;
	m_adpcm_data = 0;
	m_vclk_left = 0;

	m_flipscreen_old = -1;
	m_scrollx_ofs = 0x159;
	m_scrolly_ofs = 0x10;

	m_selected_videoram = m_selected_paletteram = 0;
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly[0] = 0;
	m_scrolly[1] = 0;
	m_gfxreg = 0;
	m_flipscreen = 0;
	m_crtc_register = 0;

	for (i = 0; i < 0x10; i++)
		m_crtc_data[i] = 0;
}

static MACHINE_CONFIG_START( nekkyoku, fromance_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)     /* 6.00 Mhz ? */
	MCFG_CPU_PROGRAM_MAP(nekkyoku_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fromance_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,12000000/2)     /* 6.00 Mhz ? */
	MCFG_CPU_PROGRAM_MAP(nekkyoku_sub_map)
	MCFG_CPU_IO_MAP(nekkyoku_sub_io_map)

	MCFG_MACHINE_START_OVERRIDE(fromance_state,fromance)
	MCFG_MACHINE_RESET_OVERRIDE(fromance_state,fromance)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromance_state, screen_update_fromance)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fromance)
	MCFG_PALETTE_ADD("palette", 1024)

	MCFG_VIDEO_START_OVERRIDE(fromance_state,nekkyoku)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 12000000/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(fromance_state, fromance_adpcm_int)) /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 KHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( idolmj, fromance_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)     /* 6.00 Mhz ? */
	MCFG_CPU_PROGRAM_MAP(fromance_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fromance_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,12000000/2)     /* 6.00 Mhz ? */
	MCFG_CPU_PROGRAM_MAP(fromance_sub_map)
	MCFG_CPU_IO_MAP(idolmj_sub_io_map)

	MCFG_MACHINE_START_OVERRIDE(fromance_state,fromance)
	MCFG_MACHINE_RESET_OVERRIDE(fromance_state,fromance)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromance_state, screen_update_fromance)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fromance)
	MCFG_PALETTE_ADD("palette", 2048)

	MCFG_VIDEO_START_OVERRIDE(fromance_state,fromance)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 12000000/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(fromance_state, fromance_adpcm_int)) /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 KHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( fromance, fromance_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)     /* 6.00 Mhz ? */
	MCFG_CPU_PROGRAM_MAP(fromance_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fromance_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,12000000/2)     /* 6.00 Mhz ? */
	MCFG_CPU_PROGRAM_MAP(fromance_sub_map)
	MCFG_CPU_IO_MAP(fromance_sub_io_map)

	MCFG_MACHINE_START_OVERRIDE(fromance_state,fromance)
	MCFG_MACHINE_RESET_OVERRIDE(fromance_state,fromance)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromance_state, screen_update_fromance)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fromance)
	MCFG_PALETTE_ADD("palette", 2048)

	MCFG_VIDEO_START_OVERRIDE(fromance_state,fromance)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(fromance_state, fromance_adpcm_int)) /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 KHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( nekkyoku )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "1-ic1a.bin",  0x000000, 0x008000, CRC(bb52d959) SHA1(1dfeb108879978dbcc1398e64b26c36505bee6d0) )
	ROM_LOAD( "2-ic2a.bin",  0x008000, 0x008000, CRC(61848d8b) SHA1(72048c53e4364544ca8a79e213db9d02b7b4778f) )

	ROM_REGION( 0x410000, "sub", 0 )
	ROM_LOAD( "3-ic3a.bin",  0x000000, 0x008000, CRC(a13da011) SHA1(601180f65ba42b7b1b6b058c0eccf88af1b430ca) )
	ROM_LOAD( "ic4a.bin",    0x010000, 0x080000, CRC(1cc4d31b) SHA1(6ea8ec3d6b3bbffbbab3460e9c5dae74195eafc6) )
	ROM_LOAD( "ic5a.bin",    0x090000, 0x080000, CRC(8b0945a1) SHA1(c52f77d817c687afa0cd93e7725cdf2021158a11) )
	ROM_LOAD( "ic6a.bin",    0x110000, 0x080000, CRC(d5615e1d) SHA1(ffee1a240045636db5d03345faadb9991b51f2c9) )
	ROM_LOAD( "4-ic7a.bin",  0x190000, 0x008000, CRC(e259cfbb) SHA1(d92a71e5f840b338aa2080a6b5872e23c7b6146f) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ic11a.bin",   0x000000, 0x080000, CRC(2bc2b1d0) SHA1(1c516b058a640ca07b065f4c55959b63a11ca015) )
	ROM_LOAD( "ic12a.bin",   0x080000, 0x040000, CRC(cac93dc0) SHA1(e47f547ac83ac005e787a2c9f802101d251d4a66) )
	ROM_LOAD( "6-ic13a.bin", 0x0c0000, 0x008000, CRC(84830e34) SHA1(e3af89938b1e122909b13faf0022b6b60afb1f3f) )
	ROM_FILL(                0x0c8000, 0x038000, 0xff )
	ROM_FILL(                0x100000, 0x100000, 0xff )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic8a.bin",    0x000000, 0x080000, CRC(599790d8) SHA1(4e4ade1a89d6cb93b0808867883d70c4c7ed78dd) )
	ROM_LOAD( "ic9a.bin",    0x080000, 0x040000, CRC(78c1906f) SHA1(54459e0120ec58a962d3f4a1287e68d2fbb28be9) )
	ROM_LOAD( "5-ic10a.bin", 0x0c0000, 0x008000, CRC(2e78515f) SHA1(397985c082ffc0df07cd44d54e4fef909c30a4f1) )
	ROM_FILL(                0x0c8000, 0x038000, 0xff )
	ROM_FILL(                0x100000, 0x100000, 0xff )
ROM_END


ROM_START( idolmj )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "3-13g.bin", 0x000000, 0x008000, CRC(910e9e7a) SHA1(5d577549ca25def14fbc6db682afda105244b7c1) )

	ROM_REGION( 0x410000, "sub", 0 )
	ROM_LOAD( "5-13e.bin", 0x000000, 0x008000, CRC(cda33264) SHA1(88ac345fccce82fafc346675726b1b806ecab1bc) )
	ROM_LOAD( "18e.bin",   0x010000, 0x080000, CRC(7ee5aaf3) SHA1(f1aaa512a475bfb56250ef4350d4b17e6165c5f7) )
	ROM_LOAD( "17e.bin",   0x090000, 0x080000, CRC(38055f94) SHA1(7724e099efda2a255ccc7989f2584fc543e77061) )
	ROM_LOAD( "4-14e.bin", 0x190000, 0x010000, CRC(84d80b43) SHA1(7d6310d313b953ed460a6891316845b64bffdd31) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "6e.bin",    0x000000, 0x080000, CRC(51dadedd) SHA1(ce0188cce457759130d70b71b2877a7eb98824e4) )
	ROM_LOAD( "2-8e.bin",  0x080000, 0x008000, CRC(a1a62c4c) SHA1(c946ad5dab918e0a7951c7eca88f1b32bf96e895) )
	ROM_FILL(              0x088000, 0x008000, 0xff )
	ROM_FILL(              0x090000, 0x170000, 0xff )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "3e.bin",    0x000000, 0x080000, CRC(eff9b562) SHA1(895d70be5efd746e15393bb138e6c6f7d39b2e54) )
	ROM_LOAD( "1-1e.bin",  0x080000, 0x008000, CRC(abf03c62) SHA1(0ad88ffe3f06f493978f292154894415ed38f797) )
	ROM_FILL(              0x088000, 0x008000, 0xff )
	ROM_FILL(              0x090000, 0x170000, 0xff )
ROM_END


ROM_START( mjnatsu )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "3-ic70.bin", 0x000000, 0x008000, CRC(543eb9e1) SHA1(cfe1d33bdf6541e2207465a941f342be21b69f7d) )

	ROM_REGION( 0x410000, "sub", 0 )
	ROM_LOAD( "4-ic47.bin", 0x000000, 0x008000, CRC(27a61dc7) SHA1(e12e36eefb7cbce8e1383b7c1b57793cde2cd6cd) )
	ROM_LOAD( "ic87.bin",   0x010000, 0x080000, CRC(caec9310) SHA1(a92be0116d7e682763e387230a51daedbe749c26) )
	ROM_LOAD( "ic78.bin",   0x090000, 0x080000, CRC(2b291006) SHA1(5360158ee9e50bd956f149ce0e58920d4d294b42) )
	ROM_LOAD( "ic72.bin",   0x110000, 0x020000, CRC(42464fba) SHA1(b16da303f3e16f47322d00f7a27d2004be63f611) )
	ROM_LOAD( "5-ic48.bin", 0x210000, 0x010000, CRC(d3c06cd9) SHA1(53c10670557d72a87f2a289f22ea2749d8d35976) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ic58.bin",   0x000000, 0x080000, CRC(257a8075) SHA1(c741a4f10cbec8e6d641d4dc2b36749277bc9548) )
	ROM_LOAD( "ic63.bin",   0x080000, 0x020000, CRC(b54c7d3a) SHA1(6be26b7d2cdf884fc140c8ed44865784d107583a) )
	ROM_LOAD( "1-ic74.bin", 0x0a0000, 0x008000, CRC(fbafa46b) SHA1(3867180f8a3cb3becf78023d3bbc2bb66580a13d) )
	ROM_FILL(               0x0a8000, 0x008000, 0xff )
	ROM_FILL(               0x0b0000, 0x150000, 0xff )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic59.bin",   0x000000, 0x080000, CRC(03983ac7) SHA1(1e14ec3c614b227a6362f2a3a7582ac7a4f58ee3) )
	ROM_LOAD( "ic64.bin",   0x080000, 0x040000, CRC(9bd8e855) SHA1(a445dd7634958f69e58aed2581513e7a583ff66b) )
	ROM_FILL(               0x0c0000, 0x140000, 0xff )
ROM_END


ROM_START( natsuiro )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "3-ic70.bin", 0x000000, 0x008000, CRC(543eb9e1) SHA1(cfe1d33bdf6541e2207465a941f342be21b69f7d) )

	ROM_REGION( 0x410000, "sub", 0 )
	ROM_LOAD( "m4.bin",     0x000000, 0x008000, CRC(8d37cc3f) SHA1(90fee63b5864002bec95c61003532e743d351244) )
	ROM_LOAD( "ic87.bin",   0x010000, 0x080000, CRC(caec9310) SHA1(a92be0116d7e682763e387230a51daedbe749c26) )
	ROM_LOAD( "ic78.bin",   0x090000, 0x080000, CRC(2b291006) SHA1(5360158ee9e50bd956f149ce0e58920d4d294b42) )
	ROM_LOAD( "ic72.bin",   0x110000, 0x020000, CRC(42464fba) SHA1(b16da303f3e16f47322d00f7a27d2004be63f611) )
	ROM_LOAD( "m5.bin",     0x210000, 0x010000, CRC(cd1ea5f7) SHA1(26965663485c7311c7a421dba0921fdfb8392df9) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ic58.bin",   0x000000, 0x080000, CRC(257a8075) SHA1(c741a4f10cbec8e6d641d4dc2b36749277bc9548) )
	ROM_LOAD( "ic63.bin",   0x080000, 0x020000, CRC(b54c7d3a) SHA1(6be26b7d2cdf884fc140c8ed44865784d107583a) )
	ROM_LOAD( "m1.bin",     0x0a0000, 0x008000, CRC(fbafa46b) SHA1(3867180f8a3cb3becf78023d3bbc2bb66580a13d) )
	ROM_FILL(               0x0a8000, 0x008000, 0xff )
	ROM_FILL(               0x0b0000, 0x150000, 0xff )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic59.bin",   0x000000, 0x080000, CRC(03983ac7) SHA1(1e14ec3c614b227a6362f2a3a7582ac7a4f58ee3) )
	ROM_LOAD( "ic64.bin",   0x080000, 0x040000, CRC(9bd8e855) SHA1(a445dd7634958f69e58aed2581513e7a583ff66b) )
	ROM_LOAD( "m2.bin",     0x0c0000, 0x008000, CRC(61129677) SHA1(a1789b4e23b46c61c822eb02fdfa65b7d0a34ce8) )
	ROM_FILL(               0x0c8000, 0x138000, 0xff )
ROM_END


ROM_START( mfunclub )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "3.70",        0x000000, 0x008000, CRC(e6f76ca3) SHA1(2f4292e50770c3325c1573781cb21940d73e8fb1) )

	ROM_REGION( 0x410000, "sub", 0 )
	ROM_LOAD( "4.47",        0x000000, 0x008000, CRC(d71ee0e3) SHA1(87a5bc53bc2f027fc644427f34141826de9f48a5) )
	ROM_LOAD( "586.87",      0x010000, 0x080000, CRC(e197af4a) SHA1(e68530bf8dbf9cb0913b36879f1ec0756f202b4e) )
	ROM_LOAD( "587.78",      0x090000, 0x080000, CRC(08ff39c3) SHA1(d1f6011c9d08820eef3bb80abcf2fdb9acd9d967) )
	ROM_LOAD( "5.57",        0x290000, 0x010000, CRC(bf988bde) SHA1(6e965f26f7c72b4ede954bd9941db282dfe0e6a9) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "584.58",      0x000000, 0x080000, CRC(d65af431) SHA1(0006846631619af41bfd4a14563c16966ea1573b) )
	ROM_LOAD( "lh634a14.63", 0x080000, 0x080000, CRC(cdda9b9e) SHA1(ffb2ca1233d5e969abc9da2faa0252ba8e779b42) )
	ROM_FILL(                0x100000, 0x080000, 0xff )
	ROM_LOAD( "1.74",        0x180000, 0x008000, CRC(5b0b2efc) SHA1(4524d9e8b6c059e0cca31d9f0d677cb6bd09561d) )
	ROM_FILL(                0x188000, 0x078000, 0xff )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "585.59",      0x000000, 0x080000, CRC(58ce0937) SHA1(729e1b906454145b3d069717f2e058ab41750db1) )
	ROM_FILL(                0x080000, 0x100000, 0xff )
	ROM_LOAD( "2.75",        0x180000, 0x010000, CRC(4dd4f786) SHA1(06f398cffc54ac9e7c42eddc1d46e00c0a64d9c7) )
	ROM_FILL(                0x190000, 0x070000, 0xff )
ROM_END


ROM_START( daiyogen )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "n1-ic70.bin", 0x000000, 0x008000, CRC(29af632b) SHA1(9a55cc7a82dc2735be6310de27521ff0f5c352bd) )

	ROM_REGION( 0x130000, "sub", 0 )
	ROM_LOAD( "n2-ic47.bin", 0x000000, 0x008000, CRC(8896604c) SHA1(305a42c6c770645d6dcb5297325dbb3c7e337443) )
	ROM_LOAD( "ic87.bin",    0x010000, 0x080000, CRC(4f86ffe2) SHA1(159952b6be9800b117b70d992ef86d72fdac1802) )
	ROM_LOAD( "ic78.bin",    0x090000, 0x080000, CRC(ae52bccd) SHA1(591bf4736807b004a2576c220066985d3218e0a1) )
	ROM_LOAD( "7-ic72.bin",  0x110000, 0x020000, CRC(30279296) SHA1(6943fd16c1b87f5000c89c380091dc922866e5ac) )

	ROM_REGION( 0x0c0000, "gfx1", 0 )
	ROM_LOAD( "ic58.bin",    0x000000, 0x080000, CRC(8cf3d5f5) SHA1(26470a9b02fd5017cbb64ceee00abb51c58b97d3) )
	ROM_LOAD( "ic63.bin",    0x080000, 0x040000, CRC(64611070) SHA1(203aa055c318a9bc6cd6f56a6fbd2ff12e4e4d5a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "ic59.bin",    0x000000, 0x080000, CRC(715f2f8c) SHA1(a1206bc21556c2644688f6332b9a57c7188ea7fa) )
	ROM_LOAD( "ic64.bin",    0x080000, 0x080000, CRC(e5a41864) SHA1(73b25fe87e347feabe7256ced80a696c377d690c) )
ROM_END


ROM_START( nmsengen )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "3-ic70.bin",   0x000000, 0x008000, CRC(4e6edbbb) SHA1(890f93569a6dec6bf7c917a3db4f268c6ec64564) )

	ROM_REGION( 0x410000, "sub", 0 )
	ROM_LOAD( "4-ic47.bin",   0x000000, 0x008000, CRC(d31c596e) SHA1(6063805b93c39054ad4804d6d476b8370c4249ca) )
	ROM_LOAD( "vsj-ic87.bin", 0x010000, 0x100000, CRC(d3e8bd73) SHA1(a8c8f3c589e561cb79ac586209369ae192507212) )
	ROM_LOAD( "j5-ic72.bin",  0x210000, 0x020000, CRC(db937253) SHA1(35fe382393e05911e7d801fff49b5427c6337ba3) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "vsk-ic63.bin", 0x000000, 0x100000, CRC(f95f9c67) SHA1(1826b52ad404ca44d79fa869aa5a974299861f40) )
	ROM_LOAD( "ic58.bin",     0x100000, 0x040000, CRC(c66dcf18) SHA1(a64f2fb478a8b3d08d0157059c9d4ab30d43c9bf) )
	ROM_FILL(                 0x140000, 0x080000, 0xff )
	ROM_LOAD( "1-ic68.bin",   0x1c0000, 0x020000, CRC(a944a8d6) SHA1(1dfaa34c6c406c7216aa989268a0d8e680f419e2) )
	ROM_FILL(                 0x1e0000, 0x020000, 0xff )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "vsh-ic64.bin", 0x000000, 0x100000, CRC(f546ffaf) SHA1(bb4babc2dffc36fb321d64cd4a28c3089f3ee366) )
	ROM_LOAD( "vsg-ic59.bin", 0x100000, 0x080000, CRC(25bae018) SHA1(ba00718d473d1fbaec8d18cc481ed146657ef14f) )
	ROM_LOAD( "ic69.bin",     0x180000, 0x040000, CRC(dc867ccd) SHA1(69745b9f6aecb92fea8e70b0cec4895dea444c04) )
	ROM_LOAD( "2-ic75.bin",   0x1c0000, 0x020000, CRC(e2fad82e) SHA1(3ab41e8e7674f8d12b0a0f518c5a54af8afa1231) )
	ROM_FILL(                 0x1e0000, 0x020000, 0xff )
ROM_END


ROM_START( fromance )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "2-ic70.bin", 0x000000, 0x008000, CRC(a0866e26) SHA1(019a8dfaa54dd397f642622d7ed847b7147a61f7) )

	ROM_REGION( 0x410000, "sub", 0 )
	ROM_LOAD( "1-ic47.bin", 0x000000, 0x008000, CRC(ac859917) SHA1(86808c06fba4a3320298d38815e8e5d32ad29182) )
	ROM_LOAD( "ic87.bin",   0x010000, 0x100000, CRC(bb0d224e) SHA1(020a5b09f58f1c18cae89c8da019562ebed16710) )
	ROM_LOAD( "ic78.bin",   0x110000, 0x040000, CRC(ba2dba83) SHA1(ca744b4ddd2200d29a5d4efe6a3378d7cfe28a36) )
	ROM_LOAD( "5-ic72.bin", 0x210000, 0x020000, CRC(377cd57c) SHA1(7e81ca5b898f5ffe24068be1e7cd0a57a3ca68b8) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ic63.bin",   0x000000, 0x100000, CRC(faa9cdf3) SHA1(194dfaf04dc960fa1d575ad494a333cc7e7d221a) )
	ROM_FILL(               0x100000, 0x0c0000, 0xff )
	ROM_LOAD( "4-ic68.bin", 0x1c0000, 0x020000, CRC(9b35cea3) SHA1(e99df84c57e6d473681b7622ad6a28fce63f1128) )
	ROM_FILL(               0x1e0000, 0x020000, 0xff )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic64.bin",   0x000000, 0x100000, CRC(23b9a484) SHA1(4299b689eb1da3d7e69c604eb28651ebfd4dad09) )
	ROM_FILL(               0x100000, 0x080000, 0xff )
	ROM_LOAD( "ic69.bin",   0x180000, 0x040000, CRC(d06a0fc0) SHA1(ea645b77f671ad71049e4db547b2c4a22e12d6c3) )
	ROM_LOAD( "3-ic75.bin", 0x1c0000, 0x020000, CRC(bb314e78) SHA1(58ce4f9c94e45b05543289643b848fbdcb9ab473) )
	ROM_FILL(               0x1e0000, 0x020000, 0xff )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1988, nekkyoku,  0,       nekkyoku, nekkyoku, driver_device, 0, ROT0, "Video System Co.", "Rettou Juudan Nekkyoku Janshi - Higashi Nippon Hen (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1988, idolmj,    0,       idolmj,   idolmj, driver_device,   0, ROT0, "System Service", "Idol-Mahjong Housoukyoku (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mjnatsu,   0,       fromance, mjnatsu, driver_device,  0, ROT0, "Video System Co.", "Mahjong Natsu Monogatari (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, natsuiro,  mjnatsu, fromance, mjnatsu, driver_device,  0, ROT0, "Video System Co.", "Natsuiro Mahjong (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mfunclub,  0,       fromance, mfunclub, driver_device, 0, ROT0, "Video System Co.", "Mahjong Fun Club - Idol Saizensen (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, daiyogen,  0,       fromance, daiyogen, driver_device, 0, ROT0, "Video System Co.", "Mahjong Daiyogen (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, nmsengen,  0,       fromance, nmsengen, driver_device, 0, ROT0, "Video System Co.", "Nekketsu Mahjong Sengen! AFTER 5 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, fromance,  0,       fromance, fromance, driver_device, 0, ROT0, "Video System Co.", "Idol-Mahjong Final Romance (Japan)", MACHINE_SUPPORTS_SAVE )
