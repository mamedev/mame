// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood, Ryan Holtz, Stephh, Pierpaolo Prazzoli, Roberto Fresca
/*****************************************************************
* Status Triv Two driver by David Haywood, Ryan Holtz, and Stephh*
* Super Triv II driver by Ryan Holtz                             *
* Triv Quiz driver by Ryan Holtz                                 *
* Triv Four driver by Pierpaolo Prazzoli                         *
* Super Triv III driver by Pierpaolo Prazzoli                    *
* Hangman driver by Pierpaolo Prazzoli                           *
* Status Black Jack driver by Angelo Salese & Roberto Fresca     *
*                                                                *
******************************************************************
*                                                                *
* All notes by David Haywood unless otherwise noted              *
*                                                                *
* Thanks to Ryan Holtz for working out why statriv2 was crashing *
* in attract and fixing the questions so it actually asked more  *
* than one per category.                                         *
*                                                                *
* Game Speed too fast?                                           *
*                                                                *
* SJ: Fixed colours based on screen shots, they're probably OK,  *
*     but more shots would be good for verification              *
*                                                                *
* MG: Dave seems to think that the AY is hooked up wrong since   *
*     it generates lots of errors in error.log, even though the  *
*     sound seems to make sense. Can someone with a PCB stomach  *
*     the game long enough to verify one way or the other?       *
*                                                                *
* RF: Reworked the Status Fun Casino inputs. Lowered the CPU     *
*     clock to get it working properly. Added technical notes.   *
*                                                                *
******************************************************************

******************************************************************
*      U21 U22 DSW U17                                           *
*                       1982 status game corp                    *
*                         8085  12.4 MHz                         *
* 8910 crt5037 8255                                              *
*                                                                *
*         u36            u7 u8 u9                                *
*                                 5101    2114                   *
*                                 5101    2114                   *
*           2114 2114 2114 2114                                  *
*                                u12(pb)                         *
*                                                                *
* triv-quiz II (pb plugs into u12)                               *
*                                                                *
* 74244  74161 74161 74161 74161  74138 7402  74139              *
* u1 u2 u3 u4 u5 u6 u7 u8                                        *
*                                                                *
*                                                                *
******************************************************************

CTR5037 is also labelled as a TMS9937NL
DSW is a 4 position dipswitch

U17 is a socketted 74s288 (compatible with 82s123)
U21 is a soldered in 74s288 (compatible with 82s123)
U22 is a soldered in 74s287 (compatible with 82s129)

PROM use is unknown


Issues:
 * statusbj - very glitchy, bad video, seems to spin
 * hangman - keys are weird, spinner is busted
 *
quaquiz2 - no inputs, needs NVRAM

*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "video/tms9927.h"
#include "machine/nvram.h"


class statriv2_state : public driver_device
{
public:
	statriv2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms(*this, "tms"),
		m_videoram(*this, "videoram"),
		m_question_offset(*this, "question_offset"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
			{ }

	required_device<cpu_device> m_maincpu;
	required_device<tms9927_device> m_tms;
	required_shared_ptr<UINT8> m_videoram;
	tilemap_t *m_tilemap;
	required_shared_ptr<UINT8> m_question_offset;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	UINT8 m_question_offset_low;
	UINT8 m_question_offset_mid;
	UINT8 m_question_offset_high;
	UINT8 m_latched_coin;
	UINT8 m_last_coin;
	DECLARE_WRITE8_MEMBER(statriv2_videoram_w);
	DECLARE_READ8_MEMBER(question_data_r);
	DECLARE_READ8_MEMBER(laserdisc_io_r);
	DECLARE_WRITE8_MEMBER(laserdisc_io_w);
	DECLARE_CUSTOM_INPUT_MEMBER(latched_coin_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_hi_w);
	DECLARE_DRIVER_INIT(addr_xlh);
	DECLARE_DRIVER_INIT(addr_lhx);
	DECLARE_DRIVER_INIT(addr_lmh);
	DECLARE_DRIVER_INIT(addr_lmhe);
	DECLARE_DRIVER_INIT(addr_xhl);
	DECLARE_DRIVER_INIT(laserdisc);
	TILE_GET_INFO_MEMBER(horizontal_tile_info);
	TILE_GET_INFO_MEMBER(vertical_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(statriv2);
	DECLARE_VIDEO_START(vertical);
	UINT32 screen_update_statriv2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(statriv2_interrupt);
};


#define MASTER_CLOCK        12440000


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(statriv2_state::horizontal_tile_info)
{
	UINT8 *videoram = m_videoram;
	int code = videoram[0x400+tile_index];
	int attr = videoram[tile_index] & 0x3f;

	SET_TILE_INFO_MEMBER(0, code, attr, 0);
}

TILE_GET_INFO_MEMBER(statriv2_state::vertical_tile_info)
{
	UINT8 *videoram = m_videoram;
	int code = videoram[0x400+tile_index];
	int attr = videoram[tile_index] & 0x3f;

	SET_TILE_INFO_MEMBER(0, ((code & 0x7f) << 1) | ((code & 0x80) >> 7), attr, 0);
}



/*************************************
 *
 *  Video/palette start
 *
 *************************************/

PALETTE_INIT_MEMBER(statriv2_state, statriv2)
{
	int i;

	for (i = 0; i < 64; i++)
	{
		palette.set_pen_color(2*i+0, pal1bit(i >> 2), pal1bit(i >> 0), pal1bit(i >> 1));
		palette.set_pen_color(2*i+1, pal1bit(i >> 5), pal1bit(i >> 3), pal1bit(i >> 4));
	}
}

void statriv2_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(statriv2_state::horizontal_tile_info),this) ,TILEMAP_SCAN_ROWS, 8,15, 64,16);
}

VIDEO_START_MEMBER(statriv2_state,vertical)
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(statriv2_state::vertical_tile_info),this), TILEMAP_SCAN_ROWS, 8,8, 32,32);
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

WRITE8_MEMBER(statriv2_state::statriv2_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 statriv2_state::screen_update_statriv2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_tms->screen_reset())
		bitmap.fill(m_palette->black_pen(), cliprect);
	else
		m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

INTERRUPT_GEN_MEMBER(statriv2_state::statriv2_interrupt)
{
	UINT8 new_coin = ioport("COIN")->read();

	/* check the coin inputs once per frame */
	m_latched_coin |= new_coin & (new_coin ^ m_last_coin);
	m_last_coin = new_coin;

	device.execute().set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	device.execute().set_input_line(I8085_RST75_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  Question address computation
 *
 *************************************/

READ8_MEMBER(statriv2_state::question_data_r)
{
	const UINT8 *qrom = memregion("questions")->base();
	UINT32 qromsize = memregion("questions")->bytes();
	UINT32 address;

	if (m_question_offset_high == 0xff)
		m_question_offset[m_question_offset_low]++;

	address = m_question_offset[m_question_offset_low];
	address |= m_question_offset[m_question_offset_mid] << 8;
	if (m_question_offset_high != 0xff)
		address |= m_question_offset[m_question_offset_high] << 16;

	return (address < qromsize) ? qrom[address] : 0xff;
}



/*************************************
 *
 *  Coin input
 *
 *************************************/

CUSTOM_INPUT_MEMBER(statriv2_state::latched_coin_r)
{
	return m_latched_coin;
}


WRITE8_MEMBER(statriv2_state::ppi_portc_hi_w)
{
	data >>= 4;
	if (data != 0x0f)
		m_latched_coin = 0;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( statriv2_map, AS_PROGRAM, 8, statriv2_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4800, 0x48ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(statriv2_videoram_w) AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( statriv2_io_map, AS_IO, 8, statriv2_state )
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x28, 0x2b) AM_READ(question_data_r) AM_WRITEONLY AM_SHARE("question_offset")
	AM_RANGE(0xb0, 0xb1) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xb1, 0xb1) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0xc0, 0xcf) AM_DEVREADWRITE("tms", tms9927_device, read, write)
ADDRESS_MAP_END

#ifdef UNUSED_CODE
static ADDRESS_MAP_START( statusbj_io, AS_IO, 8, statriv2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0xb0, 0xb1) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xb1, 0xb1) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0xc0, 0xcf) AM_DEVREADWRITE("tms", tms9927_device, read, write)
ADDRESS_MAP_END
#endif



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( statusbj )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Deal")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hit")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stand")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Insurance")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Split")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, statriv2_state,latched_coin_r, "COIN")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
//  PORT_DIPSETTING(    0x30, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP switch?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( funcsino )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )     PORT_NAME("Draw")      PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1 / Horse 1 / Point")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2 / Horse 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3 / Horse 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4 / Horse 4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5 / Horse 5 / No Point")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stand")         PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Select Game")   PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, statriv2_state,latched_coin_r, "COIN")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, "DIP switch? 10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP switch? 20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP switch? 40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP switch? 80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( hangman )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Choose")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Go For It")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Spinner")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Keep High Scores" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( statriv2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Play All")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Play 10000")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button D")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Play 1000")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, statriv2_state,latched_coin_r, "COIN")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x20, 0x20, "Show Correct Answer" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( statriv4 )
	PORT_INCLUDE(statriv2)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( supertr2 )
	PORT_INCLUDE(statriv2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( quaquiz2 )
	PORT_INCLUDE(supertr2)

	PORT_MODIFY("IN0")
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(1) PORT_NAME("Button A")
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_IMPULSE(1) PORT_NAME("Button B")
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_IMPULSE(1) PORT_NAME("Button C")
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_IMPULSE(1) PORT_NAME("Button D")
	PORT_DIPNAME( 0x80, 0x00, "Port $20 bit 7" ) // coin3? if off it doesn't boot
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x02, 0x00, "Port $21 bit 1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Company Logo" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Port $21 bit 6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Initialize NVRAM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sextriv )
	PORT_INCLUDE(statriv2)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics decoding
 *
 *************************************/

static const gfx_layout horizontal_tiles_layout =
{
	8,15,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	16*8
};

static GFXDECODE_START( horizontal )
	GFXDECODE_ENTRY( "tiles", 0, horizontal_tiles_layout, 0, 64 )
GFXDECODE_END


static const gfx_layout vertical_tiles_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( vertical )
	GFXDECODE_ENTRY( "tiles", 0, vertical_tiles_layout, 0, 64 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( statriv2, statriv2_state )
	/* basic machine hardware */
	/* FIXME: The 8085A had a max clock of 6MHz, internally divided by 2! */
	MCFG_CPU_ADD("maincpu", I8085A, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(statriv2_map)
	MCFG_CPU_IO_MAP(statriv2_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", statriv2_state,  statriv2_interrupt)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	/* PPI 8255 group A & B set to Mode 0.
	 Port A, B and lower 4 bits of C set as Input.
	 High 4 bits of C set as Output */
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(statriv2_state, ppi_portc_hi_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/2, 384, 0, 320, 270, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(statriv2_state, screen_update_statriv2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("tms", TMS9927, MASTER_CLOCK/2)
	MCFG_TMS9927_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", horizontal)
	MCFG_PALETTE_ADD("palette", 2*64)
	MCFG_PALETTE_INIT_OWNER(statriv2_state, statriv2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( statriv2v, statriv2 )

	/* basic machine hardware */

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/2, 392, 0, 256, 262, 0, 256)

	MCFG_VIDEO_START_OVERRIDE(statriv2_state,vertical)
	MCFG_GFXDECODE_MODIFY("gfxdecode", vertical)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( funcsino, statriv2 )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(MASTER_CLOCK/2)  /* 3 MHz?? seems accurate */
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Black Jack (Status 1981)

12.44Mhz
                          5101  5101
8085
                2
                1
      8255                 2114
      9927                 2114
      8255                 2114
                           2114
*/

ROM_START( statusbj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "statusbj.1",   0x0000, 0x0800, CRC(d3faf340) SHA1(e03f7e3375a02a3bec07d9c7f4f2b1a711d4d1cc) )
	ROM_LOAD( "statusbj.2",   0x0800, 0x0800, CRC(3f1727af) SHA1(0df12626591fc70031a9d8615c37243813d67b70) )

	ROM_REGION( 0x800, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "statusbj.vid",   0x0000, 0x0800, CRC(99ade7a2) SHA1(98704ca3a9fcfc4590f850c8ae24445baaed6dfa) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "prom.u17", 0x0000, 0x0020, NO_DUMP ) /* Socketted */
	ROM_LOAD( "prom.u21", 0x0020, 0x0020, NO_DUMP ) /* Soldered in (Color?) */
	ROM_LOAD( "prom.u22", 0x0040, 0x0100, NO_DUMP ) /* Soldered in */
ROM_END

ROM_START( tripdraw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u7_td1.bin", 0x0000, 0x0800, CRC(7128ce7c) SHA1(3d46239456969c3af1817ef917e328c2819d6625) )
	ROM_LOAD( "u8_td2.bin", 0x0800, 0x0800, CRC(637f5f69) SHA1(97d30eaa65a4b44c28b2b5e5eed8aa9966c2bbb7) )

	ROM_REGION( 0x800, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "u36_td0.bin", 0x0000, 0x0800, CRC(2faa1942) SHA1(0205bf9eb86ef1c32f1ae959d0e02001393db3af) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "prom.u17", 0x0000, 0x0020, NO_DUMP ) /* Socketted */
	ROM_LOAD( "prom.u21", 0x0020, 0x0020, NO_DUMP ) /* Soldered in (Color?) */
	ROM_LOAD( "prom.u22", 0x0040, 0x0100, NO_DUMP ) /* Soldered in */
ROM_END

/*

Fun Casino
Status Game Corp., 1982

8085
xtal 12.44MHz
8255
TMS9937
AY3-8910
5101 RAM x2
2114 RAM x6
2732 EPROMs x4
74S287 PROM x1
74S288 PROM x2
4-position DSW x1

*/
ROM_START( funcsino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u7", 0x0000, 0x1000, CRC(e7380a81) SHA1(dbc9646a33bd61cdfab9f8a5ac7db996cfc0eaf9) )
	ROM_LOAD( "u8", 0x1000, 0x1000, CRC(89767226) SHA1(d948c139a876e516fe54b39fd0548a07537f8535) )
	ROM_LOAD( "u9", 0x2000, 0x1000, CRC(d663c0be) SHA1(70a5aa509815ce0992809f88c3bd55e130c03dc4) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "u36", 0x0000, 0x1000, CRC(79eaf78b) SHA1(9df5f90344bbb9f1d196f35d910bb09fe6f74aa1) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "prom.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted */
	ROM_LOAD( "prom.u21", 0x0020, 0x0020, CRC(e8f60d23) SHA1(2070b8201b75a13e416f597d6b2473d0027f420c) ) /* Soldered in (Color?) */
	ROM_LOAD( "prom.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( hangman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "main_hang_1b_2732.u7", 0x0000, 0x1000, CRC(0d71b2ad) SHA1(636e5e0e356c9c7de174a0b6a5663fedcb1e697f) )
	ROM_LOAD( "main_hang_2b_2732.u8", 0x1000, 0x1000, CRC(77533554) SHA1(fe97a412135b770ce7e85442507eac0c25b7256a) )
	ROM_LOAD( "main_hang_3b_2732.u9", 0x2000, 0x1000, CRC(daab4853) SHA1(b3b8ef051f4b04195cf7b25232f1af561c3ff7ba) )

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "main_hang_0b_2732.u36", 0x0000, 0x1000, CRC(e031dbf8) SHA1(ae6d34ad02b2d7deb1665d2420f8399a3ca88585) )

	ROM_REGION( 0x16000, "questions", 0 ) /* question data */
	ROM_LOAD( "aux_27c256.8", 0x00000, 0x8000, CRC(c7d76338) SHA1(40e88efd7e250ad867772258eb6dc3b225de781f) )
	ROM_LOAD( "aux_2764.1",   0x08000, 0x2000, CRC(a88563c8) SHA1(23cb169268ded6c81494197cfb9b34180667fc8c) )
	ROM_LOAD( "aux_2764.2",   0x0a000, 0x2000, CRC(4bddbe3c) SHA1(391012de04e8a3638fac6f173a81cf1f86d8f751) )
	ROM_LOAD( "aux_2764.3",   0x0c000, 0x2000, CRC(3ece427a) SHA1(ff09bf3734ecfa6d848037cca11b80adb8074e39) )
	ROM_LOAD( "aux_2764.4",   0x0e000, 0x2000, CRC(bb0efc98) SHA1(4317d1243f6aaf8178d19574645a43f9c2e42725) )
	ROM_LOAD( "aux_2764.5",   0x10000, 0x2000, CRC(8bebf907) SHA1(76d01b71e696b06cdf9d9c93839ef797c56b78db) )
	ROM_LOAD( "aux_2764.6",   0x12000, 0x2000, CRC(d1732f3b) SHA1(c4e862bd98f237e1d2ecad430226cba6aba4ebb8) )
	ROM_LOAD( "aux_2764.7",   0x14000, 0x2000, CRC(e51d45b8) SHA1(7cd0ced0245dbd55a225182e43b89d55d8d33197) )

	ROM_REGION( 0x0100, "nvram", 0 )
	ROM_LOAD( "hangman.nv", 0x0000, 0x0100, CRC(4cecee6f) SHA1(bd9fe7bea081c87033993f809ed0b2c727ab5e88) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( trivquiz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "triv1-1f.u8",  0x00000, 0x01000, CRC(da9a763a) SHA1(d7a60718a1aeadb247330c939e0ac487015b55b2) )
	ROM_LOAD( "triv1-2f.u9",  0x01000, 0x01000, CRC(270459fe) SHA1(1507a477fe7170d24788c880d43b0a3b08f35748) )
	ROM_LOAD( "triv1-3f.u10", 0x02000, 0x01000, CRC(103f4160) SHA1(487afaf243d144aaee8a2ea76105fba09181dfdb) )

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "triv1-0f.u7",  0x00000, 0x01000, CRC(af5f434a) SHA1(1e7ae7ad7ea697007a30f5ba89127802a835eddc) )

	ROM_REGION( 0x10000, "questions", 0 ) /* question data */
	ROM_LOAD( "qmt11.rom",    0x00000, 0x02000, CRC(82107565) SHA1(28d71340873330df7d15f1bc55cee78a9c7c31a6) )
	ROM_LOAD( "qmt12.rom",    0x02000, 0x02000, CRC(68667637) SHA1(df6ad3e624dcad57ce176912931660c6c1780369) )
	ROM_LOAD( "qmt13.rom",    0x04000, 0x02000, CRC(e0d01a68) SHA1(22bb2a8628a3764d733748e4f5f3bad881371a29) )
	ROM_LOAD( "qmt14.rom",    0x06000, 0x02000, CRC(68262b46) SHA1(faba97e5f6475e088554117e4b772e1631d740b2) )
	ROM_LOAD( "qmt15.rom",    0x08000, 0x02000, CRC(d1f39185) SHA1(e46120496e84e224bd15da0652e218cea85c170d) )
	ROM_LOAD( "qmt16.rom",    0x0a000, 0x02000, CRC(1d2ecf1d) SHA1(1d833b57bf4b3ccb3dc60307641ef9476289fe07) )
	ROM_LOAD( "qmt17.rom",    0x0c000, 0x02000, CRC(01840f9c) SHA1(d9b4f7f931657d4e16cf981d887508fd1db5e4c0) )
	ROM_LOAD( "qmt18.rom",    0x0e000, 0x02000, CRC(004a9480) SHA1(7adff194a1549fa42577f969706aab6bb6a58851) )

	ROM_REGION( 0x0100, "nvram", 0 )
	ROM_LOAD( "trivquiz.nv", 0x0000, 0x0100, CRC(bd07a964) SHA1(a1fe68d95c79ac99cfca8a468073b5c838e1cd49) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted, not verified the same! */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( statriv2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trivii1c.u7", 0x00000, 0x01000, CRC(89326d7b) SHA1(4878a3aabe51a4de5ff5927a0707b2d121ff87fa) )
	ROM_LOAD( "trivii2c.u8", 0x01000, 0x01000, CRC(6fd255f6) SHA1(13c75effda1db8eb3635d955ae11f37388f159aa) )
	ROM_LOAD( "trivii3c.u9", 0x02000, 0x01000, CRC(f666dc54) SHA1(757e0e621400d266771ea6db835305208457702f) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "trivii0c.u36", 0x00000, 0x01000, CRC(af5f434a) SHA1(1e7ae7ad7ea697007a30f5ba89127802a835eddc) )

	ROM_REGION( 0x10000, "questions", 0 ) /* question data */
	ROM_LOAD( "statuspb.u1", 0x00000, 0x02000, CRC(a50c0313) SHA1(f9bf84613e2ebb952a81a10ee1da49a37423b717) )
	ROM_LOAD( "statuspb.u2", 0x02000, 0x02000, CRC(0bc03294) SHA1(c4873cd065c9eb237b03a4195332b7629abac327) )
	ROM_LOAD( "statuspb.u3", 0x04000, 0x02000, CRC(d1732f3b) SHA1(c4e862bd98f237e1d2ecad430226cba6aba4ebb8) )
	ROM_LOAD( "statuspb.u4", 0x06000, 0x02000, CRC(e51d45b8) SHA1(7cd0ced0245dbd55a225182e43b89d55d8d33197) )
	ROM_LOAD( "statuspb.u5", 0x08000, 0x02000, CRC(b3e49c5d) SHA1(bc42ba21bb0d411c2f484076499484b104f0b4ea) )
	ROM_LOAD( "statuspb.u6", 0x0a000, 0x02000, CRC(7ee1cea0) SHA1(00ef768524e54890ebd1fdb3dd52d0080a18fc03) )
	ROM_LOAD( "statuspb.u7", 0x0c000, 0x02000, CRC(121d6976) SHA1(2e4da8f2c3620c8f46fd4951551b0747b3c38caf) )
	ROM_LOAD( "statuspb.u8", 0x0e000, 0x02000, CRC(5080df10) SHA1(b5cb0868d844bbb598159177fd5ce65ff3f18eda) )

	ROM_REGION( 0x0100, "nvram", 0 )
	ROM_LOAD( "statriv2.nv", 0x0000, 0x0100, CRC(3edb2bd1) SHA1(fdda1310f519054eb5e6ea498d27555f96b370eb) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted, not verified the same! */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( statriv2v )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "status.u7",    0x00000, 0x01000, CRC(4acc2060) SHA1(04f841fa7fba2231f312904dbd1d352fd2dbc287) )
	ROM_LOAD( "status.u8",    0x01000, 0x01000, CRC(f2de3867) SHA1(ec891d4aa4e8dc0780cf187d8b1548d7e00d4321) )
	ROM_LOAD( "status.u9",    0x02000, 0x01000, CRC(d70f5dbf) SHA1(1b21a6d9cc17c7cd03a43056070ab55f3c5d4c58))

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "status.u36",    0x00000, 0x00800, CRC(ae1d07c0) SHA1(2b657ba58e3ae7cceb8cf23cba3e1f0d20817933) ) // first half is garbage?
	ROM_CONTINUE(0x0000, 0x800)

	/* other roms were not from this set, missing sub-board?, but as the game is 'triv two' like the parent
	   it seems compatible with the same question board */

	ROM_REGION( 0x10000, "questions", 0 ) /* question data */
	ROM_LOAD( "statuspb.u1", 0x00000, 0x02000, CRC(a50c0313) SHA1(f9bf84613e2ebb952a81a10ee1da49a37423b717) )
	ROM_LOAD( "statuspb.u2", 0x02000, 0x02000, CRC(0bc03294) SHA1(c4873cd065c9eb237b03a4195332b7629abac327) )
	ROM_LOAD( "statuspb.u3", 0x04000, 0x02000, CRC(d1732f3b) SHA1(c4e862bd98f237e1d2ecad430226cba6aba4ebb8) )
	ROM_LOAD( "statuspb.u4", 0x06000, 0x02000, CRC(e51d45b8) SHA1(7cd0ced0245dbd55a225182e43b89d55d8d33197) )
	ROM_LOAD( "statuspb.u5", 0x08000, 0x02000, CRC(b3e49c5d) SHA1(bc42ba21bb0d411c2f484076499484b104f0b4ea) )
	ROM_LOAD( "statuspb.u6", 0x0a000, 0x02000, CRC(7ee1cea0) SHA1(00ef768524e54890ebd1fdb3dd52d0080a18fc03) )
	ROM_LOAD( "statuspb.u7", 0x0c000, 0x02000, CRC(121d6976) SHA1(2e4da8f2c3620c8f46fd4951551b0747b3c38caf) )
	ROM_LOAD( "statuspb.u8", 0x0e000, 0x02000, CRC(5080df10) SHA1(b5cb0868d844bbb598159177fd5ce65ff3f18eda) )

	ROM_REGION( 0x0100, "nvram", 0 )
	ROM_LOAD( "statriv2v.nv", 0x0000, 0x0100, CRC(3a9c7db7) SHA1(3d5a78beed26a73320f1f0748944b7fd87794bc7) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted, not verified the same! */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( statriv4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "triv4.u07",    0x00000, 0x01000, CRC(38800e01) SHA1(3d174c4194169eae7c033e8bb30bd02779170d42) )
	ROM_LOAD( "triv4.u08",    0x01000, 0x01000, CRC(7557e97e) SHA1(9096e7055b7a7579cc9206ad678063f9c882785b) )
	ROM_LOAD( "triv4.u09",    0x02000, 0x01000, CRC(7f1b2e1d) SHA1(12249335a1c7fed8912009051e400e216688bdbc) )

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "triv4.u36",    0x00000, 0x01000, CRC(af5f434a) SHA1(1e7ae7ad7ea697007a30f5ba89127802a835eddc) )

	ROM_REGION( 0x10000, "questions", 0 ) /* question data */
	ROM_LOAD( "triv4.u41",    0x00000, 0x02000, CRC(aed8eead) SHA1(a615786d11c879875e9b7d3c3593fe0334e79178) )
	ROM_LOAD( "triv4.u42",    0x02000, 0x02000, CRC(3354d389) SHA1(527e46e9276f4dfaad57a77f0b549d9d26c59226) )
	ROM_LOAD( "triv4.u43",    0x04000, 0x02000, CRC(de7513e8) SHA1(c2e38cb39aacf57edb27cf5ee0b0fd49a44befa3) )
	ROM_LOAD( "triv4.u44",    0x06000, 0x02000, CRC(b4293435) SHA1(5e2b96c19c4f5c63a5afa2de504d29fe64a4c908) )
	ROM_LOAD( "triv4.u45",    0x08000, 0x02000, CRC(0b082745) SHA1(73c375d1dd906f0cc1106eac1fba45c51c751f86) )
	ROM_LOAD( "triv4.u46",    0x0a000, 0x02000, CRC(fa53158a) SHA1(3814b60d999ad234f6c08ace2c84893fcb745a3c) )
	ROM_LOAD( "triv4.u47",    0x0c000, 0x02000, CRC(fddbb113) SHA1(a88a1afdb1be035fc71929ef0236b61b8403cc1b) )
	ROM_LOAD( "triv4.u48",    0x0e000, 0x02000, CRC(30ca8393) SHA1(dfb2f16f9b014d23793efe085be1ed75342c00dc) )

	ROM_REGION( 0x0100, "nvram", 0 )
	ROM_LOAD( "statriv4.nv", 0x0000, 0x0100, CRC(ab449099) SHA1(80fe9e07068a1034f8c0b233a7d37f6b40644be5) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted, verified */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( sextriv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sex.u7",       0x00000, 0x1000, CRC(f587bd69) SHA1(47ddc70c3cc75a22ba67833531aeeb409f8d8dc1) )
	ROM_LOAD( "sex.u8",       0x01000, 0x1000, CRC(2718c26d) SHA1(b614b4a102ae664c4a3be1e30e515454442de052) )
	ROM_LOAD( "sex.u9",       0x02000, 0x1000, CRC(f4f5a651) SHA1(a1e1fa96f7631b2274ef7fbe7e6e1aae1ee540c5) )

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "sex.u36",      0x00000, 0x1000, CRC(fe3fa087) SHA1(356b3fe62b4c600ff5a625bc3e1c53d8143f55df) )

	ROM_REGION( 0x10000, "questions", 0 ) /* question data */
	ROM_LOAD( "sex1.bin",     0x00000, 0x2000, CRC(7b55360b) SHA1(c38b1be8cb7c6c40e167c449c75b9ca0596affe9) )
	ROM_LOAD( "sex2.bin",     0x02000, 0x2000, CRC(a88563c8) SHA1(23cb169268ded6c81494197cfb9b34180667fc8c) )
	ROM_LOAD( "sex3.bin",     0x04000, 0x2000, CRC(da1e00a5) SHA1(d70b0a1ecaf7913cfbf3d218ff05e8511be6ab26) )
	ROM_LOAD( "sex4.bin",     0x06000, 0x2000, CRC(fcff262c) SHA1(b5c17c0285db4b5a6a19aa2d487a98df519bd1b9) )
	ROM_LOAD( "sex5.bin",     0x08000, 0x2000, CRC(b0b9cd9a) SHA1(b233d7522eab6fd4454209f8bdd91e6c7392d779) )
	ROM_LOAD( "sex6.bin",     0x0a000, 0x2000, CRC(02653058) SHA1(e830562d9b720f49fbb0079a00799958245e1d96) )
	ROM_LOAD( "sex7.bin",     0x0c000, 0x2000, CRC(4bddbe3c) SHA1(391012de04e8a3638fac6f173a81cf1f86d8f751) )
	ROM_LOAD( "sex8.bin",     0x0e000, 0x2000, CRC(d4221641) SHA1(d2c0f66c4fe3a77c73cdcc71bbd8c48342d29431) )

	ROM_REGION( 0x0100, "nvram", 0 )
	ROM_LOAD( "sextriv.nv", 0x0000, 0x0100, CRC(33fae98c) SHA1(11aaca7706460dbad750c11c794bbe20084a8ff6) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( quaquiz2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qquiz.u7",  0x00000, 0x01000, CRC(8e525abc) SHA1(b03500a2ccb8f0b038093460e38460d29bdf8af3) )
	ROM_LOAD( "qquiz.u8",  0x01000, 0x01000, CRC(2186ceb5) SHA1(c8f74026d18841ebcc4cfc85ba08e68e41e9d1d0) )
	ROM_LOAD( "qquiz.u9",  0x02000, 0x01000, CRC(6d815876) SHA1(275a76e791abd38bc9baf6626edcb6d78259ebc9) )
	ROM_LOAD( "qquiz.u10", 0x03000, 0x01000, CRC(714a9093) SHA1(9ca75565003bd14ca2f0b8882667fe577732a4da) )

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "qquiz.u36", 0x00000, 0x01000, CRC(468dca15) SHA1(4ace7b3cd233f826949b65e2ab71e94ac6a293a0) )

	ROM_REGION( 0x40000, "questions", ROMREGION_INVERT ) /* question data - inverted */
	ROM_LOAD( "gst.01",    0x00000, 0x08000, CRC(c0d83049) SHA1(94c750068e550cdaf4f6f5416bc7c160a759dd5a) )
	ROM_LOAD( "gst.02",    0x08000, 0x08000, CRC(b844743e) SHA1(4a75e4956c568bad70130a326c0fc691a11ff04c) )
	ROM_LOAD( "gst.03",    0x10000, 0x08000, CRC(4c734bc5) SHA1(48171494f183dec01732b2d6a0f2af0c1b173dba) )
	ROM_LOAD( "gst.04",    0x18000, 0x08000, CRC(8ddbeca6) SHA1(1e49fb7f1469c0476094d8538473b23ef0b64ac5) )
	ROM_LOAD( "gst.05",    0x20000, 0x08000, CRC(f1e07381) SHA1(3a5f075491840ed214490704453336512ecafc0d) )
	ROM_LOAD( "gst.06",    0x28000, 0x08000, CRC(18855c6d) SHA1(0454eaebc42838c75e7748f8e2c2eb5f58380f51) )
	ROM_LOAD( "gst.07",    0x30000, 0x08000, CRC(1270d5bd) SHA1(826162e37c233639b1f545f4d215a4bf9fcba065) )
	ROM_LOAD( "gst.08",    0x38000, 0x08000, CRC(64a54915) SHA1(13bbdff3617ec14595bc72891f56d327d76f539d) )

	ROM_REGION( 0x0100, "nvram", 0 )
	ROM_LOAD( "quaquiz2.nv", 0x0000, 0x0100, CRC(dad239cf) SHA1(c46380d7b673a1367f14364ae47cd46ebe080e1b) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted, not verified the same! */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( supertr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ast2-1d.rom", 0x00000, 0x01000, CRC(e9f0e271) SHA1(c2bae7d5ef04aed3ce14c403c70d2acc1831b763) )
	ROM_LOAD( "ast2-2d.rom", 0x01000, 0x01000, CRC(542ba813) SHA1(1ac063f3678d1295aa728ab7ac43165284b66836) )
	ROM_LOAD( "ast2-3d.rom", 0x02000, 0x01000, CRC(46c467b7) SHA1(2556ce6436112646d8ec3bcff7c32212c5296463) )
	ROM_LOAD( "ast2-4d.rom", 0x03000, 0x01000, CRC(11382c44) SHA1(6b611ad9e591b27d5cb239388e4d27e646be3028) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "ast2-0d.rom", 0x00000, 0x01000, CRC(a40f9201) SHA1(a87cfc3dbe5cff82926f5f8486c37fd3f4449135) )

	ROM_REGION( 0x40000, "questions", ROMREGION_INVERT ) /* question data - inverted */
	ROM_LOAD( "astq2-1.rom", 0x00000, 0x08000, CRC(4af390cb) SHA1(563c6210f2fcc8ee9b5112e2d6f522ddfca2ddea) )
	ROM_LOAD( "astq2-2.rom", 0x08000, 0x08000, CRC(91a7b4f6) SHA1(c8ff2e8475ae889be14086a04275df94efd66156) )
	ROM_LOAD( "astq2-3.rom", 0x10000, 0x08000, CRC(e6a50944) SHA1(e3fad344d4bedfd14f307445334903c35e745d9b) )
	ROM_LOAD( "astq2-4.rom", 0x18000, 0x08000, CRC(6f9f9cef) SHA1(b43d1a2a714764f46f038f85a8233bf811a877ae) )
	ROM_LOAD( "astq2-5.rom", 0x20000, 0x08000, CRC(a0c0f51e) SHA1(c61518ef53d5bec334062b6853663424853892b9) )
	ROM_LOAD( "astq2-6.rom", 0x28000, 0x08000, CRC(c0f61b5f) SHA1(65398f9d22fce95c4146a2cb8174edd6b336b9e4) )
	ROM_LOAD( "astq2-7.rom", 0x30000, 0x08000, CRC(72461937) SHA1(2f95a708b24f56d9b1293a88aa53eb4a32f89869) )
	ROM_LOAD( "astq2-8.rom", 0x38000, 0x08000, CRC(cd2674d5) SHA1(7fb6513172ffe8e3b9e0f4dc9ecdb42d954b1ff0) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted, not verified the same! */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( supertr3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "triv3.u07",    0x00000, 0x01000, CRC(f04a19d7) SHA1(f887ec976d9eb14329621ac75d6547fca6808bb3) )
	ROM_LOAD( "triv3.u08",    0x01000, 0x01000, CRC(543d5664) SHA1(58ee8b94964b567fc052f7c4df4517ee029046bd) )
	ROM_LOAD( "triv3.u09",    0x02000, 0x01000, CRC(047faed4) SHA1(e24c919434ad4e9a1059e34e6609a7271accd8f1) )
	ROM_LOAD( "triv3.u10",    0x03000, 0x01000, CRC(df4b81b5) SHA1(b1ab666c51b838c4176f8b314677d6ae129997d0) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "triv3.u36",    0x00000, 0x01000, CRC(79277b08) SHA1(e8de06809853e030d1ee29a788f9bc8ff7175af0) )

	ROM_REGION( 0x40000, "questions", ROMREGION_INVERT ) /* question data - inverted */
	ROM_LOAD( "triv3.u41",    0x00000, 0x08000, CRC(d62960c4) SHA1(d6f7dbdb016c14ca1cab5a0e965c9ae40dcbbc28) )
	ROM_LOAD( "triv3.u42",    0x08000, 0x08000, CRC(6d50fec9) SHA1(6edb3ed92781e8961eacc342c0bceeb052b81a3e) )
	ROM_LOAD( "triv3.u43",    0x10000, 0x08000, CRC(8c0a73de) SHA1(2a7175b7845b26b8d0d53279cd8793edee95d3a1) )
	ROM_LOAD( "triv3.u44",    0x18000, 0x08000, CRC(fec7e3d0) SHA1(6921386be4de06efb2d4c382733c2d22948fdf4f) )
	ROM_LOAD( "triv3.u45",    0x20000, 0x08000, CRC(b28d81dd) SHA1(d4a6026b437dcaf6881232b960b9e870754c9ec6) )
	ROM_LOAD( "triv3.u46",    0x28000, 0x08000, CRC(86cffc1f) SHA1(06557bcc51b415349e5f7440f753ef2f66dcfde2) )
	ROM_LOAD( "triv3.u47",    0x30000, 0x08000, CRC(f316803c) SHA1(31edb97bad7083ed32e0ee75256bc7d488fa234b) )
	ROM_LOAD( "triv3.u48",    0x38000, 0x08000, CRC(1a99b268) SHA1(6369c79f645962b4a2f85b18e9d93c3cc65defc1) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketted, verified */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( cstripxi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "csxirom1.bin", 0x0000, 0x1000, CRC(7821c604) SHA1(12217bcbbe534187b941ce74c10b57679d8d922c) )
	ROM_LOAD( "csxirom2.bin", 0x1000, 0x1000, CRC(67d8ee9b) SHA1(f13f4a55ecaa5d7b08c7ec3bad7e43379a9ca7db) )
	ROM_LOAD( "csxirom3.bin", 0x2000, 0x1000, CRC(97d81042) SHA1(fdc5519cb1891b3aff9d4c1ce9b41b29500e416c) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "csxirom0.bin", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "prom.u17", 0x0000, 0x0020, NO_DUMP ) /* Socketted */
	ROM_LOAD( "prom.u21", 0x0020, 0x0020, NO_DUMP ) /* Soldered in (Color?) */
	ROM_LOAD( "prom.u22", 0x0040, 0x0100, NO_DUMP ) /* Soldered in */

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cstripxi", 0, NO_DUMP )
ROM_END



/*************************************
 *
 *  Driver setup
 *
 *************************************/

/* question address is stored as L/H/X (low/high/don't care) */
DRIVER_INIT_MEMBER(statriv2_state,addr_lhx)
{
	m_question_offset_low = 0;
	m_question_offset_mid = 1;
	m_question_offset_high = 0xff;
}

/* question address is stored as X/L/H (don't care/low/high) */
DRIVER_INIT_MEMBER(statriv2_state,addr_xlh)
{
	m_question_offset_low = 1;
	m_question_offset_mid = 2;
	m_question_offset_high = 0xff;
}

/* question address is stored as X/H/L (don't care/high/low) */
DRIVER_INIT_MEMBER(statriv2_state,addr_xhl)
{
	m_question_offset_low = 2;
	m_question_offset_mid = 1;
	m_question_offset_high = 0xff;
}

/* question address is stored as L/M/H (low/mid/high) */
DRIVER_INIT_MEMBER(statriv2_state,addr_lmh)
{
	m_question_offset_low = 0;
	m_question_offset_mid = 1;
	m_question_offset_high = 2;
}

DRIVER_INIT_MEMBER(statriv2_state,addr_lmhe)
{
	/***************************************************\
	*                                                   *
	* Super Trivia has some really weird protection on  *
	* its question data. For some odd reason, the data  *
	* itself is stored normally. Just load the ROMs up  *
	* in a hex editor and OR everything with 0x40 to    *
	* get normal text. However, the game itself expects *
	* different data than what the question ROMs        *
	* contain. Here is some pseudocode for what the     *
	* game does for each character:                     *
	*                                                   *
	*     GetCharacter:                                 *
	*     In A,($28)             // Read character in   *
	*     Invert A               // Invert the bits     *
	*     AND A,$1F              // Put low 5 bits of   *
	*     B = Low 8 bits of addy // addy into high 8    *
	*     C = 0                  // bits of BC pair     *
	*     Call ArcaneFormula(BC) // Get XOR value       *
	*     XOR A,C                // Apply it            *
	*     Return                                        *
	*                                                   *
	*     ArcaneFormula(BC):                            *
	*     ShiftR BC,1                                   *
	*     DblShiftR BC,1                                *
	*     DblShiftR BC,1                                *
	*     DblShiftR BC,1                                *
	*     ShiftR BC,1                                   *
	*     Return                                        *
	*                                                   *
	* Essentially what ArcaneFormula does is to "fill   *
	* out" an entire 8 bit number from only five bits.  *
	* The way it does this is by putting bit 0 of the 5 *
	* bits into bit 0 of the 8 bits, putting bit 1 into *
	* bits 1 and 2, bit 2 into bits 3 and 4, bit 3 into *
	* bits 5 and 6, and finally, bit 4 into bit         *
	* position 7 of the 8-bit number. For example, for  *
	* a value of FA, these would be the steps to get    *
	* the XOR value:                                    *
	*                                                   *
	*                                 Address  XOR val  *
	*     1: Take original number     11111010 00000000 *
	*     2: AND with 0x1F            00011010 00000000 *
	*     3: Put bit 0 in bit 0       0001101- 00000000 *
	*     4: Double bit 1 in bits 1,2 000110-0 00000110 *
	*     5: Double bit 2 in bits 3,4 00011-10 00000110 *
	*     6: Double bit 3 in bits 5,6 0001-010 01100110 *
	*     7: Put bit 4 in bit 7       000-1010 11100110 *
	*                                                   *
	* Since XOR operations are symmetrical, to make the *
	* game end up receiving the correct value one only  *
	* needs to invert the value and XOR it with the     *
	* value derived from its address. The game will     *
	* then de-invert the value when it tries to invert  *
	* it, re-OR the value when it tries to XOR it, and  *
	* we wind up with nice, working questions. If       *
	* anyone can figure out a way to simplify the       *
	* formula I'm using, PLEASE DO SO!                  *
	*                                                   *
	*                                       - Ryan Holtz*
	*                                                   *
	\***************************************************/

	UINT8 *qrom = memregion("questions")->base();
	UINT32 length = memregion("questions")->bytes();
	UINT32 address;

	for (address = 0; address < length; address++)
		qrom[address] ^= BITSWAP8(address, 4,3,3,2,2,1,1,0);

	DRIVER_INIT_CALL(addr_lmh);
}


READ8_MEMBER(statriv2_state::laserdisc_io_r)
{
	UINT8 result = 0x00;
	if (offset == 1)
		result = 0x18;
	osd_printf_debug("%s:ld read ($%02X) = %02X\n", machine().describe_context(), 0x28 + offset, result);
	return result;
}

WRITE8_MEMBER(statriv2_state::laserdisc_io_w)
{
	osd_printf_debug("%s:ld write ($%02X) = %02X\n", machine().describe_context(), 0x28 + offset, data);
}

DRIVER_INIT_MEMBER(statriv2_state,laserdisc)
{
	address_space &iospace = m_maincpu->space(AS_IO);
	iospace.install_readwrite_handler(0x28, 0x2b, read8_delegate(FUNC(statriv2_state::laserdisc_io_r), this), write8_delegate(FUNC(statriv2_state::laserdisc_io_w), this));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1981, statusbj, 0,        statriv2,  statusbj, driver_device, 0,         ROT0, "Status Games", "Status Black Jack (V1.0c)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, funcsino, 0,        funcsino,  funcsino, driver_device, 0,         ROT0, "Status Games", "Status Fun Casino (V1.3s)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, tripdraw, 0,        statriv2,  funcsino, driver_device, 0,         ROT0, "Status Games", "Tripple Draw (V3.1 s)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1984, hangman,  0,        statriv2,  hangman, statriv2_state,  addr_lmh,  ROT0, "Status Games", "Hangman", MACHINE_SUPPORTS_SAVE )
GAME( 1984, trivquiz, 0,        statriv2,  statriv2, statriv2_state, addr_lhx,  ROT0, "Status Games", "Triv Quiz", MACHINE_SUPPORTS_SAVE )
GAME( 1984, statriv2, 0,        statriv2,  statriv2, statriv2_state, addr_xlh,  ROT0, "Status Games", "Triv Two", MACHINE_SUPPORTS_SAVE )
GAME( 1985, statriv2v,statriv2, statriv2v, statriv2, statriv2_state, addr_xlh,  ROT90,"Status Games", "Triv Two (Vertical)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, statriv4, 0,        statriv2,  statriv4, statriv2_state, addr_xhl,  ROT0, "Status Games", "Triv Four", MACHINE_SUPPORTS_SAVE )
GAME( 1985, sextriv,  0,        statriv2,  sextriv, statriv2_state,  addr_lhx,  ROT0, "Status Games", "Sex Triv", MACHINE_SUPPORTS_SAVE )
GAME( 1985, quaquiz2, 0,        statriv2,  quaquiz2, statriv2_state, addr_lmh,  ROT0, "Status Games", "Quadro Quiz II", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1986, supertr2, 0,        statriv2,  supertr2, statriv2_state, addr_lmhe, ROT0, "Status Games", "Super Triv II", MACHINE_SUPPORTS_SAVE )
GAME( 1988, supertr3, 0,        statriv2,  supertr2, statriv2_state, addr_lmh,  ROT0, "Status Games", "Super Triv III", MACHINE_SUPPORTS_SAVE )
GAME( 1990, cstripxi, 0,        statriv2,  funcsino, statriv2_state, laserdisc, ROT0, "Status Games", "Casino Strip XI", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
