// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood, Ryan Holtz, Pierpaolo Prazzoli, Roberto Fresca,Stephane Humbert
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

U17 is a socketed 74s288 (compatible with 82s123)
U21 is a soldered in 74s288 (compatible with 82s123)
U22 is a soldered in 74s287 (compatible with 82s129)

PROM use is unknown


Issues:
 * statusbj - very glitchy, bad video, seems to spin
 * hangman - keys are weird, spinner is busted
 * quaquiz2 - no inputs, needs NVRAM

*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/tms9927.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class statriv2_state : public driver_device
{
public:
	statriv2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_tms(*this, "tms")
		, m_videoram(*this, "videoram")
		, m_question_offset(*this, "question_offset")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void statriv2(machine_config &config);
	void statusbj(machine_config &config);
	void funcsino(machine_config &config);
	void tripdraw(machine_config &config);
	void statriv2v(machine_config &config);

	void init_addr_xlh();
	void init_addr_lhx();
	void init_addr_lmh();
	void init_addr_lmhe();
	void init_addr_xhl();
	void init_laserdisc();

	int latched_coin_r();

private:
	required_device<cpu_device> m_maincpu;
	required_device<tms9927_device> m_tms;
	required_shared_ptr<uint8_t> m_videoram;
	tilemap_t *m_tilemap = nullptr;
	optional_shared_ptr<uint8_t> m_question_offset;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	uint8_t m_question_offset_low = 0;
	uint8_t m_question_offset_mid = 0;
	uint8_t m_question_offset_high = 0;
	uint8_t m_latched_coin = 0;
	uint8_t m_last_coin = 0;

	void statriv2_videoram_w(offs_t offset, uint8_t data);
	uint8_t question_data_r();
	void ppi_portc_hi_w(uint8_t data);

	TILE_GET_INFO_MEMBER(horizontal_tile_info);
	TILE_GET_INFO_MEMBER(vertical_tile_info);
	virtual void video_start() override ATTR_COLD;
	void statriv2_palette(palette_device &palette) const;
	void check_coin_status();
	DECLARE_VIDEO_START(vertical);
	uint32_t screen_update_statriv2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(statriv2_interrupt);
	INTERRUPT_GEN_MEMBER(tripdraw_interrupt);

	void statriv2_map(address_map &map) ATTR_COLD;
	void statusbj_io_map(address_map &map) ATTR_COLD;
	void statriv2_io_map(address_map &map) ATTR_COLD;
};


#define MASTER_CLOCK        12440000


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(statriv2_state::horizontal_tile_info)
{
	int code = m_videoram[0x400 + tile_index];
	int attr = m_videoram[tile_index] & 0x3f;

	tileinfo.set(0, code, attr, 0);
}

TILE_GET_INFO_MEMBER(statriv2_state::vertical_tile_info)
{
	int code = m_videoram[0x400 + tile_index];
	int attr = m_videoram[tile_index] & 0x3f;

	tileinfo.set(0, ((code & 0x7f) << 1) | ((code & 0x80) >> 7), attr, 0);
}



/*************************************
 *
 *  Video/palette start
 *
 *************************************/

void statriv2_state::statriv2_palette(palette_device &palette) const
{
	for (int i = 0; i < 64; i++)
	{
		palette.set_pen_color(2*i + 0, pal1bit(i >> 2), pal1bit(i >> 0), pal1bit(i >> 1));
		palette.set_pen_color(2*i + 1, pal1bit(i >> 5), pal1bit(i >> 3), pal1bit(i >> 4));
	}
}

void statriv2_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(statriv2_state::horizontal_tile_info)), TILEMAP_SCAN_ROWS, 8, 15, 64, 16);
}

VIDEO_START_MEMBER(statriv2_state,vertical)
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(statriv2_state::vertical_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

void statriv2_state::statriv2_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t statriv2_state::screen_update_statriv2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

void statriv2_state::check_coin_status()
{
	uint8_t new_coin = ioport("COIN")->read();

	/* check the coin inputs once per frame */
	m_latched_coin |= new_coin & (new_coin ^ m_last_coin);
	m_last_coin = new_coin;
}

INTERRUPT_GEN_MEMBER(statriv2_state::statriv2_interrupt)
{
	if (ioport("COIN"))
		check_coin_status();

	device.execute().set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	device.execute().set_input_line(I8085_RST75_LINE, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(statriv2_state::tripdraw_interrupt)
{
	check_coin_status();

	device.execute().set_input_line(I8085_RST55_LINE, ASSERT_LINE);
	device.execute().set_input_line(I8085_RST55_LINE, CLEAR_LINE);
}

/*************************************
 *
 *  Question address computation
 *
 *************************************/

uint8_t statriv2_state::question_data_r()
{
	const uint8_t *qrom = memregion("questions")->base();
	uint32_t qromsize = memregion("questions")->bytes();
	uint32_t address;

	if (m_question_offset_high == 0xff && !machine().side_effects_disabled())
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

int statriv2_state::latched_coin_r()
{
	return m_latched_coin;
}


void statriv2_state::ppi_portc_hi_w(uint8_t data)
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

void statriv2_state::statriv2_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x4800, 0x48ff).ram().share("nvram");
	map(0xc800, 0xcfff).ram().w(FUNC(statriv2_state::statriv2_videoram_w)).share("videoram");
}

void statriv2_state::statusbj_io_map(address_map &map)
{
	map(0x20, 0x23).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb0, 0xb1).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xb1, 0xb1).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xc0, 0xcf).rw(m_tms, FUNC(tms9927_device::read), FUNC(tms9927_device::write));
}

void statriv2_state::statriv2_io_map(address_map &map)
{
	statusbj_io_map(map);
	map(0x28, 0x2b).r(FUNC(statriv2_state::question_data_r)).writeonly().share("question_offset");
}


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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(statriv2_state::latched_coin_r))
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("Deal")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Draw")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1 / Horse 1 / Point")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2 / Horse 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3 / Horse 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4 / Horse 4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5 / Horse 5 / No Point")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )                       PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Select Game")   PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(statriv2_state::latched_coin_r))
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

static INPUT_PORTS_START( tripdraw )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("Deal")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Draw")      PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1 / Lo")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3 / Double")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5 / Hi")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )                       PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(statriv2_state::latched_coin_r))
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

static INPUT_PORTS_START( bigcsino ) // flyer shows 8 buttons on the cabinet
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Play")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )                         PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Select Joker Poker / Discard 1 / Horse 1 / Point")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Select Blackjack / Discard 2 / Horse 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Select Baccarat / Discard 3 / Horse 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Select Craps / Discard 4 / Horse 4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Select Race Time / Discard 5 / Horse 5 / No Point")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(statriv2_state::latched_coin_r))
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Retain Backup Memory" )
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(statriv2_state::latched_coin_r))
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

static INPUT_PORTS_START( bbchall )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button D")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
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

static GFXDECODE_START( gfx_horizontal )
	GFXDECODE_ENTRY( "tiles", 0, horizontal_tiles_layout, 0, 64 )
GFXDECODE_END


static GFXDECODE_START( gfx_vertical )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 64 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void statriv2_state::statriv2(machine_config &config)
{
	/* basic machine hardware */
	/* FIXME: The 8085A had a max clock of 6MHz, internally divided by 2! */
	I8085A(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &statriv2_state::statriv2_map);
	m_maincpu->set_addrmap(AS_IO, &statriv2_state::statriv2_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(statriv2_state::statriv2_interrupt));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi(I8255A(config, "ppi8255"));
	/* PPI 8255 group A & B set to Mode 0.
	 Port A, B and lower 4 bits of C set as Input.
	 High 4 bits of C set as Output */
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.in_pc_callback().set_ioport("IN2");
	ppi.out_pc_callback().set(FUNC(statriv2_state::ppi_portc_hi_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK/2, 384, 0, 320, 270, 0, 240);
	screen.set_screen_update(FUNC(statriv2_state::screen_update_statriv2));
	screen.set_palette(m_palette);

	TMS9927(config, m_tms, MASTER_CLOCK/2/8).set_char_width(8);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_horizontal);
	PALETTE(config, m_palette, FUNC(statriv2_state::statriv2_palette), 2*64);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "aysnd", MASTER_CLOCK/8).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void statriv2_state::statusbj(machine_config &config)
{
	statriv2(config);
	m_maincpu->set_addrmap(AS_IO, &statriv2_state::statusbj_io_map); // no question data
}

void statriv2_state::statriv2v(machine_config &config)
{
	statriv2(config);

	/* basic machine hardware */

	subdevice<screen_device>("screen")->set_raw(MASTER_CLOCK/2, 392, 0, 256, 262, 0, 256);

	MCFG_VIDEO_START_OVERRIDE(statriv2_state, vertical)
	m_gfxdecode->set_info(gfx_vertical);
}

void statriv2_state::funcsino(machine_config &config)
{
	statusbj(config);

	/* basic machine hardware */

	m_maincpu->set_clock(MASTER_CLOCK/2);  /* 3 MHz?? seems accurate */
}

void statriv2_state::tripdraw(machine_config &config)
{
	statusbj(config);

	/* basic machine hardware */
	m_maincpu->set_vblank_int("screen", FUNC(statriv2_state::tripdraw_interrupt));
}


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
	ROM_LOAD( "prom.u17", 0x0000, 0x0020, NO_DUMP ) /* Socketed */
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
	ROM_LOAD( "prom.u17", 0x0000, 0x0020, NO_DUMP ) /* Socketed */
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
	ROM_LOAD( "prom.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed */
	ROM_LOAD( "prom.u21", 0x0020, 0x0020, CRC(e8f60d23) SHA1(2070b8201b75a13e416f597d6b2473d0027f420c) ) /* Soldered in (Color?) */
	ROM_LOAD( "prom.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( bigcsino ) // 5 in 1 of: Joker Poker, Blackjack, Baccarat, Craps and Race Time
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bc1k.u7",  0x0000, 0x1000, CRC(51f35f51) SHA1(3c1b9b613402e178d0d8752bf025e5d7fc9f1081) )
	ROM_LOAD( "bc2k.u8",  0x1000, 0x1000, CRC(8102d1eb) SHA1(e2ffa04d705b82b19e978429851690426cb4b474) )
	ROM_LOAD( "bc3k.u9",  0x2000, 0x1000, CRC(fdfb304e) SHA1(b000d7533c1613fb253b9f48143cc4add73aa23c) )
	ROM_LOAD( "bc4k.u10", 0x3000, 0x1000, CRC(521347c3) SHA1(539ce6e93bf6db2ca2a8fa22d0aef9933f8cd825) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "bc0k.u36", 0x0000, 0x1000, CRC(baf66e19) SHA1(4b78571e9370453e96e64c08d69b92b4d64e1a41) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "82s123.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed */
	ROM_LOAD( "82s123.u21", 0x0020, 0x0020, CRC(e8f60d23) SHA1(2070b8201b75a13e416f597d6b2473d0027f420c) ) /* Soldered in (Color?) */
	ROM_LOAD( "82s123.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, not verified the same! */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, not verified the same! */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, not verified the same! */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, verified */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( statriv5se )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr-4-sped1a.u7",    0x00000, 0x01000, CRC(d6e0b97d) SHA1(251292948d045fbcf7e883f1aeaf03c9ad03dc86) )
	ROM_LOAD( "tr-4-sped2a.u8",    0x01000, 0x01000, CRC(debcb949) SHA1(116550b9c4ab17bd10875b52f9a1b25c7eb2c8ea) )
	ROM_LOAD( "tr-4-sped3a.u9",    0x02000, 0x01000, CRC(e59e6a1f) SHA1(fc25b3195ef19ecee3bba2b530c38dbbed828326) )

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "tr-4-sped0a.u36",    0x00000, 0x01000, CRC(8e57d527) SHA1(ddb2719fd5d3e8c476e4f10158a9d83e0a759aa4) )

	ROM_REGION( 0x10000, "questions", 0 ) /* question data, not dumped  */
	ROM_LOAD( "triv5.u41",    0x00000, 0x02000, NO_DUMP )
	ROM_LOAD( "triv5.u42",    0x02000, 0x02000, NO_DUMP )
	ROM_LOAD( "triv5.u43",    0x04000, 0x02000, NO_DUMP )
	ROM_LOAD( "triv5.u44",    0x06000, 0x02000, NO_DUMP )
	ROM_LOAD( "triv5.u45",    0x08000, 0x02000, NO_DUMP )
	ROM_LOAD( "triv5.u46",    0x0a000, 0x02000, NO_DUMP )
	ROM_LOAD( "triv5.u47",    0x0c000, 0x02000, NO_DUMP )
	ROM_LOAD( "triv5.u48",    0x0e000, 0x02000, NO_DUMP )

	ROM_REGION( 0x0140, "proms", 0 ) // not dumped for this set, probably same as statriv4
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, verified */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, BAD_DUMP CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, BAD_DUMP CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, not verified the same! */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

// Dumper's note: I got the pcb with Triv Quiz (trivquiz) question rom pcb, but I don't think that's the correct one.
// If you use Triv Quiz question data the game will boot to playable state

ROM_START( supertr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u07.bin",  0x00000, 0x01000, CRC(6573d17c) SHA1(0ec4a572312393c9efbf580f015bcf418d867079) )
	ROM_LOAD( "u08.bin",  0x01000, 0x01000, CRC(734f7e0a) SHA1(42b4ec0396a7150b78901e0279fc7904abd94d06) )
	ROM_LOAD( "u09.bin",  0x02000, 0x01000, CRC(41b9bb46) SHA1(d440ec19c9962b3e35c50e2cc1ba0c218c05c0e1) )
	ROM_LOAD( "u10.bin",  0x03000, 0x01000, CRC(2f7b2207) SHA1(2c58fde128824fb553a6ec5336cadbf194ec81b7) )

	ROM_REGION( 0x1000,  "tiles", ROMREGION_INVERT )
	ROM_LOAD( "u36.bin", 0x00000, 0x01000, CRC(01f30203) SHA1(b902845af0e4d96446550539596354d9962d78be) )

	ROM_REGION( 0x40000, "questions", 0 )
	ROM_LOAD( "q1.rom", 0x00000, 0x08000, NO_DUMP )
	ROM_LOAD( "q2.rom", 0x08000, 0x08000, NO_DUMP )
	ROM_LOAD( "q3.rom", 0x10000, 0x08000, NO_DUMP )
	ROM_LOAD( "q4.rom", 0x18000, 0x08000, NO_DUMP )
	ROM_LOAD( "q5.rom", 0x20000, 0x08000, NO_DUMP )
	ROM_LOAD( "q6.rom", 0x28000, 0x08000, NO_DUMP )
	ROM_LOAD( "q7.rom", 0x30000, 0x08000, NO_DUMP )
	ROM_LOAD( "q8.rom", 0x38000, 0x08000, NO_DUMP )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, verified */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, BAD_DUMP CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, not verified the same! */
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
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, verified */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( nsupertr3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astiii-1.u7",    0x00000, 0x01000, CRC(f04a19d7) SHA1(f887ec976d9eb14329621ac75d6547fca6808bb3) )
	ROM_LOAD( "astiii-2.u8",    0x01000, 0x01000, CRC(543d5664) SHA1(58ee8b94964b567fc052f7c4df4517ee029046bd) )
	ROM_LOAD( "astiii-3.u9",    0x02000, 0x01000, CRC(047faed4) SHA1(e24c919434ad4e9a1059e34e6609a7271accd8f1) )
	ROM_LOAD( "astiii-4.u10",   0x03000, 0x01000, CRC(df4b81b5) SHA1(b1ab666c51b838c4176f8b314677d6ae129997d0) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "triv3.u36",    0x00000, 0x01000, CRC(79277b08) SHA1(e8de06809853e030d1ee29a788f9bc8ff7175af0) )

	ROM_REGION( 0x40000, "questions", ROMREGION_INVERT ) /* question data - inverted */
	ROM_LOAD( "astqiii-1.bin",    0x00000, 0x08000, CRC(d62960c4) SHA1(d6f7dbdb016c14ca1cab5a0e965c9ae40dcbbc28) )
	ROM_LOAD( "astqiii-2.bin",    0x08000, 0x08000, CRC(fdd5a792) SHA1(aeff26f919abc5bbeb1903c674a5d59f5e7aed27) ) // different from set supertrv3
	ROM_LOAD( "astqiii-3.bin",    0x10000, 0x08000, CRC(8c0a73de) SHA1(2a7175b7845b26b8d0d53279cd8793edee95d3a1) )
	ROM_LOAD( "astqiii-4.bin",    0x18000, 0x08000, CRC(fec7e3d0) SHA1(6921386be4de06efb2d4c382733c2d22948fdf4f) )
	ROM_LOAD( "astqiii-5.bin",    0x20000, 0x08000, CRC(bfe9c98d) SHA1(19e39780cb78eb1bd4448d423939ea1e125ac8d3) ) // different from set supertrv3
	ROM_LOAD( "astqiii-6.bin",    0x28000, 0x08000, CRC(86cffc1f) SHA1(06557bcc51b415349e5f7440f753ef2f66dcfde2) )
	ROM_LOAD( "astqiii-7.bin",    0x30000, 0x08000, CRC(f316803c) SHA1(31edb97bad7083ed32e0ee75256bc7d488fa234b) )
	ROM_LOAD( "astqiii-8.bin",    0x38000, 0x08000, CRC(c2141a9e) SHA1(7e6a32b5b49d53936192eb87bf8bd7a5977d7597) ) // different from set supertrv3

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, CRC(63b8a63e) SHA1(d59ad84edd583f7befce73b79e12dfb58a204c4f) ) /* Socketed, verified */
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, CRC(853d6172) SHA1(4aaab0faeaa1a07ee883fbed021f8dcd7e0ba549) ) /* Soldered in (Color?) */
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, CRC(0421b8e0) SHA1(8b786eed86397a1463ad37b9b011edf83d76dd63) ) /* Soldered in */
ROM_END

ROM_START( bbchall ) // ROMs came from a blister, Baby Boom Challenge title found in bbu2.2b
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bbu.1b",   0x0000, 0x1000, CRC(d329bd36) SHA1(165fa6d4c99efac3e563d60005ee7e56cdf49252) )
	ROM_LOAD( "bbu.2b",   0x1000, 0x1000, CRC(f5a0e022) SHA1(0224267cc500a45da86c881907c25a521b360f68) )
	ROM_LOAD( "bbu.3b",   0x2000, 0x1000, CRC(e83ca1c9) SHA1(dd71c5e44f881b2a6067998882605abb0037182d) )
	ROM_LOAD( "bbu.4b",   0x3000, 0x1000, CRC(dbad980c) SHA1(f4ec5f51185eeda7ed59b8cb7da9a293fe701389) )
	//ROM_FILL(0x00bf, 0x0001, 0xc2) // HACK for testing! jz to jnz to skip "wrong satellite board" message. Alternatively via debugger: bpset 00bf, go, do PC=00d1, go

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "bbu.0b",   0x0000, 0x1000, CRC(4d2f5878) SHA1(8f6b7278e43c1c505580fd9a02f4bdec51dc2284) )

	ROM_REGION( 0x40000, "questions", 0 )
	ROM_LOAD( "q1.rom", 0x00000, 0x08000, NO_DUMP )
	ROM_LOAD( "q2.rom", 0x08000, 0x08000, NO_DUMP )
	ROM_LOAD( "q3.rom", 0x10000, 0x08000, NO_DUMP )
	ROM_LOAD( "q4.rom", 0x18000, 0x08000, NO_DUMP )
	ROM_LOAD( "q5.rom", 0x20000, 0x08000, NO_DUMP )
	ROM_LOAD( "q6.rom", 0x28000, 0x08000, NO_DUMP )
	ROM_LOAD( "q7.rom", 0x30000, 0x08000, NO_DUMP )
	ROM_LOAD( "q8.rom", 0x38000, 0x08000, NO_DUMP )

	ROM_REGION( 0x0140, "proms", 0 ) // no PROMs in the blister, but should exist
	ROM_LOAD( "prom1", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "prom2", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "prom3", 0x0040, 0x0100, NO_DUMP )
ROM_END

// Casino Strip games were marketed by Status and Quantum. There were poker and shooting gallery versions
// and there were versions for Pioneer and Sony LD players. Thus every disc in the series could have many different ROM versions.
// The following romset names are built this way: csn_abc where:
// cs = Casino Strip n = disc number a = distributor (s for Status, q for Quantum) b = game type (p for poker, s for shooting gallery)
// c = LD player type (p for Pioneer, s for Sony).
// The series comprises 16 discs (I through XII, The Laser Shuffle, Private Eyes / All Stars, Vivid 1, Vivid 2).

// The following sets were sourced through the Dragon's Lair Project and are confirmed working on real hardware.

ROM_START( cs1_spp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_1_spp.u7", 0x0000, 0x1000, CRC(4f47a25d) SHA1(104cf9b7e4489b94df1aa699cde561e4464d527b) )
	ROM_LOAD( "cs_1_spp.u8", 0x1000, 0x1000, CRC(39dc3bfe) SHA1(f1752cfc3472abf23417d2e52d6b8c7c026017e8) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_1_spp.u36", 0x0000, 0x1000, CRC(f39ee880) SHA1(341fa53689d5d41c66091fe41548cc82b37f0802) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_1_spp", 0, NO_DUMP )
ROM_END

ROM_START( cs2_sps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_2_sps.u7", 0x0000, 0x1000, CRC(119d5891) SHA1(239528daf6591b74c40b7a8a44c11a854b4812e7) )
	ROM_LOAD( "cs_2_sps.u8", 0x1000, 0x1000, CRC(9109c676) SHA1(c4d45467c2f9e435491f89afc743bff4ec7037c8) )
	ROM_LOAD( "cs_2_sps.u9", 0x2000, 0x1000, CRC(8dccb3d1) SHA1(d638a394e01c9baf8f06c91298d49bbb8186a8d0) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_2_sps.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_2_sps", 0, NO_DUMP )
ROM_END

ROM_START( cs3_qps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_3_qps.u7", 0x0000, 0x1000, CRC(26a16431) SHA1(9847c0f63d773be22bad302bfd863ea4afc726ec) )
	ROM_LOAD( "cs_3_qps.u8", 0x1000, 0x1000, CRC(619661b9) SHA1(52d7d90efbbc23cc553fc9fab5cc7a6185686321) )
	ROM_LOAD( "cs_3_qps.u9", 0x2000, 0x1000, CRC(30a4930c) SHA1(a739d502c5dca21f986c4257464ce553ab8a03c6) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_3_qps.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_3_qps", 0, NO_DUMP )
ROM_END

ROM_START( cs5_spp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_5_spp.u7", 0x0000, 0x1000, CRC(26a16431) SHA1(9847c0f63d773be22bad302bfd863ea4afc726ec) )
	ROM_LOAD( "cs_5_spp.u8", 0x1000, 0x1000, CRC(619661b9) SHA1(52d7d90efbbc23cc553fc9fab5cc7a6185686321) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_5_spp.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_5_spp", 0, NO_DUMP )
ROM_END

ROM_START( cs5_ssp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_5_ssp.u7", 0x0000, 0x1000, CRC(bd2f35ea) SHA1(49ea3e92dc912f43774d0c0c7738ac3053d0b886) )
	ROM_LOAD( "cs_5_ssp.u8", 0x1000, 0x1000, CRC(2f77ca20) SHA1(83e8ccf66085d8452e2098345517e98f6c5fa2c4) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_5_ssp.u36", 0x0000, 0x1000, CRC(e31560ee) SHA1(d065ded3d5820f2179131f60fc8510ddef7718a6) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_5_ssp", 0, NO_DUMP )
ROM_END

ROM_START( cs6_ssp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_6_ssp.u7", 0x0000, 0x1000, CRC(120f08c2) SHA1(a45cb33e19a7e3aab8283cdc492892a334fcecc8) )
	ROM_LOAD( "cs_6_ssp.u8", 0x1000, 0x1000, CRC(6533cfb6) SHA1(b4704fd00aee60ade5ba5a93289f142f9be5af0c) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_6_ssp.u36", 0x0000, 0x1000, CRC(e31560ee) SHA1(d065ded3d5820f2179131f60fc8510ddef7718a6) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_6_ssp", 0, NO_DUMP )
ROM_END

ROM_START( cs8_ssp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_8_ssp.u7", 0x0000, 0x1000, CRC(13515129) SHA1(03dbf6e7c07cbae5363621cb9e0f92d7abd372f6) )
	ROM_LOAD( "cs_8_ssp.u8", 0x1000, 0x1000, CRC(7c26bfab) SHA1(be3322707a5e37572974cc0880397fc3c9c473bd) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_8_ssp.u36", 0x0000, 0x1000, CRC(e31560ee) SHA1(d065ded3d5820f2179131f60fc8510ddef7718a6) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_8_ssp", 0, NO_DUMP )
ROM_END

ROM_START( cs8_sps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_8_sps.u7", 0x0000, 0x1000, CRC(3cb451ce) SHA1(75e19da654f0a3d9af7a8fc70a8940bee6c575a2) )
	ROM_LOAD( "cs_8_sps.u8", 0x1000, 0x1000, CRC(cfd965f0) SHA1(1ec6a0d853d5ad17eca749444ce52c71c90e0940) )
	ROM_LOAD( "cs_8_sps.u9", 0x2000, 0x1000, CRC(0feeb474) SHA1(21f2a064958e3af4c777fa3614bd6322ad3c3fe6) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_8_sps.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_8_sps", 0, NO_DUMP )
ROM_END

ROM_START( cs9_qps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_9_qps.u7", 0x0000, 0x1000, CRC(26a16431) SHA1(9847c0f63d773be22bad302bfd863ea4afc726ec) )
	ROM_LOAD( "cs_9_qps.u8", 0x1000, 0x1000, CRC(619661b9) SHA1(52d7d90efbbc23cc553fc9fab5cc7a6185686321) )
	ROM_LOAD( "cs_9_qps.u9", 0x2000, 0x1000, CRC(f7dd64fe) SHA1(35c52fcb058ca3f492ed2de2f1563296eb6b8ff7) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_9_qps.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_9_qps", 0, NO_DUMP )
ROM_END

ROM_START( cs11_ssp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_11_ssp.u7", 0x0000, 0x1000, CRC(f51ad73e) SHA1(6e3b88d3c4493b3410965c837bbb4056282be755) )
	ROM_LOAD( "cs_11_ssp.u8", 0x1000, 0x1000, CRC(d9715d46) SHA1(81a3b733025381542f0cff49cb7f208c66f024c1) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_11_ssp.u36", 0x0000, 0x1000, CRC(e31560ee) SHA1(d065ded3d5820f2179131f60fc8510ddef7718a6) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_11_ssp", 0, NO_DUMP )
ROM_END

ROM_START( cs11_sps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_11_sps.u7", 0x0000, 0x1000, CRC(7821c604) SHA1(12217bcbbe534187b941ce74c10b57679d8d922c) )
	ROM_LOAD( "cs_11_sps.u8", 0x1000, 0x1000, CRC(67d8ee9b) SHA1(f13f4a55ecaa5d7b08c7ec3bad7e43379a9ca7db) )
	ROM_LOAD( "cs_11_sps.u9", 0x2000, 0x1000, CRC(97d81042) SHA1(fdc5519cb1891b3aff9d4c1ce9b41b29500e416c) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_11_sps.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_11_sps", 0, NO_DUMP )
ROM_END

ROM_START( cs11_sps2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs_11_sps2.u7", 0x0000, 0x1000, CRC(b076330c) SHA1(110ed3862dad6feed4678247177525207075a100) )
	ROM_LOAD( "cs_11_sps2.u8", 0x1000, 0x1000, CRC(b77bfb85) SHA1(711e2123fa600b48d4f278abf0dac4c09013d433) )
	ROM_LOAD( "cs_11_sps2.u9", 0x2000, 0x1000, CRC(d2a69ca2) SHA1(a945e8513f7ccd73fc3eef3daa6c7fcf8a41d61c) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cs_11_sps2.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_11_sps2", 0, NO_DUMP )
ROM_END

ROM_START( cspe_qps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cspe_qps.u7", 0x0000, 0x1000, CRC(26a16431) SHA1(9847c0f63d773be22bad302bfd863ea4afc726ec) )
	ROM_LOAD( "cspe_qps.u8", 0x1000, 0x1000, CRC(8697e664) SHA1(41cc8b8c183896f7a0ccae704f582448a7cb5411) )
	ROM_LOAD( "cspe_qps.u9", 0x2000, 0x1000, CRC(c73ee6b4) SHA1(8212db7d0fb68603b1e6883ba5a69d7b4ce04da5) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cspe_qps.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cspe_qps", 0, NO_DUMP )
ROM_END

ROM_START( csv1_qps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "csv1_qps.u7", 0x0000, 0x1000, CRC(a721d48d) SHA1(ec321f4cf4c3d93ba283a568d11ddc3155d982df) )
	ROM_LOAD( "csv1_qps.u8", 0x1000, 0x1000, CRC(3b503f12) SHA1(79b4e23eaab1e3c6435c28d7f73f3b621903b572) )
	ROM_LOAD( "csv1_qps.u9", 0x2000, 0x1000, CRC(c7ac1c23) SHA1(adbed3a3f9eea4aaf1c9e8276b9f30641d23c9da) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "csv1_qps.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("csv1_qps", 0, NO_DUMP )
ROM_END

// the following sets aren't currently verified on real hardware. Versions came from labels / auctions / title screen.

ROM_START( cs1_spp2 ) // believed to be poker because of the tiles ROM, Status because of title screen, Pioneer because of ROM label.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astrip-1g.u7", 0x0000, 0x1000, CRC(4f47a25d) SHA1(104cf9b7e4489b94df1aa699cde561e4464d527b) )
	ROM_LOAD( "astrip-2g.u8", 0x1000, 0x1000, CRC(39dc3bfe) SHA1(f1752cfc3472abf23417d2e52d6b8c7c026017e8) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "cstrip-0b.u36", 0x0000, 0x1000, CRC(d1968e69) SHA1(37fc2f70602c796db8dc1daa47277e6ef11d8846) )

	ROM_REGION( 0x0140, "proms", 0 ) // not present in the romset but PCB pic seems to show them
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cstrip", 0, NO_DUMP )
ROM_END

ROM_START( cs6_sps ) // believed to be poker because of the tiles ROM, Status because of title screen, Sony because of ROM label
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sastrip-vi_1-9.5.u7", 0x0000, 0x1000, CRC(e66a5def) SHA1(b9e033819051b888208c170597ecfec6041f5153) )
	ROM_LOAD( "sastrip-vi_2-9.5.u8", 0x1000, 0x1000, CRC(ba20f780) SHA1(02aa92c83977b4541566e6902a42da12d461494c) )
	ROM_LOAD( "sastrip-vi_3-9.5.u9", 0x2000, 0x1000, CRC(ec9cb9b0) SHA1(e927a929b7b6e724ea8a4dd77163696243de0715) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "sastrip-vi_0-9.5.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 ) // not present in the romset
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs6_sps", 0, NO_DUMP )
ROM_END

ROM_START( cs8_spp ) // believed to be poker because of the tiles ROM, Status because of title screen, Pioneer because of ROM label
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strip-viii-1b.u7", 0x0000, 0x1000, CRC(2408c1ec) SHA1(315c11145369cfb2c8050bfda30c0452ce5ba666) )
	ROM_LOAD( "strip-viii-2b.u8", 0x1000, 0x1000, CRC(94b1b070) SHA1(326ea9231ab060c09beebb9a954d036e23b1979c) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "strip-viii-0b.u36", 0x0000, 0x1000, CRC(f39ee880) SHA1(341fa53689d5d41c66091fe41548cc82b37f0802) )

	ROM_REGION( 0x0140, "proms", 0 ) // not present in the romset and no PCB pic available, but should be there
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cstripviii", 0, NO_DUMP )
ROM_END

ROM_START( cs9_spp ) // believed to be poker because of the tiles ROM, Status because of title screen, Pioneer because of ROM label
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astrip-ix-1.u7", 0x0000, 0x1000, CRC(62e7aa72) SHA1(c3a4c0550eee4765af205bd854270757b222892f) )
	ROM_LOAD( "astrip-ix-2.u8", 0x1000, 0x1000, CRC(c43066fe) SHA1(252597fcd1411b653acc99fd3f03e69cc1fbdaf6) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "strip-0.u36", 0x0000, 0x1000, CRC(f39ee880) SHA1(341fa53689d5d41c66091fe41548cc82b37f0802) )

	ROM_REGION( 0x0140, "proms", 0 ) // not present in the romset and no PCB pic available, but should be there
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cstripix", 0, NO_DUMP )
ROM_END

ROM_START( cs10_sps ) // believed to be poker because of the tiles ROM, Status because of title screen, Sony because of ROM label
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sastrip-x_1-9.5.u7", 0x0000, 0x1000, CRC(e66a5def) SHA1(b9e033819051b888208c170597ecfec6041f5153) )
	ROM_LOAD( "sastrip-x_2-9.5.u8", 0x1000, 0x1000, CRC(ba20f780) SHA1(02aa92c83977b4541566e6902a42da12d461494c) )
	ROM_LOAD( "sastrip-x_3-9.5.u9", 0x2000, 0x1000, CRC(c44358fe) SHA1(0f023dafe716987ba54b65976ad75b2020f3d6ec) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "sastrip_0.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 ) // not present in the romset
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs10_sps", 0, NO_DUMP )
ROM_END

ROM_START( cs12_sps ) // believed to be poker because of the tiles ROM, Status because of title screen, Sony because of ROM label. Was marked as XII but maincpu ROMs are definitely XI.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sonystrip-xi_1-4.1.u7", 0x0000, 0x1000, CRC(b076330c) SHA1(110ed3862dad6feed4678247177525207075a100) )
	ROM_LOAD( "sonystrip-xi_2-4.1.u8", 0x1000, 0x1000, CRC(b77bfb85) SHA1(711e2123fa600b48d4f278abf0dac4c09013d433) )
	ROM_LOAD( "sonystrip-xi_3-4.5.u9", 0x2000, 0x1000, CRC(1d32a970) SHA1(37a7c3526b9bc3ec41dfd4f037f2ea9dc7077d68) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "sastrip-xii_0.u36", 0x0000, 0x1000, CRC(4c9d995e) SHA1(a262d2124f65aa86b0fecee6976b6591fd370d55) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "dm74s288.u17", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s288.u21", 0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "dm74s282.u22", 0x0040, 0x0100, NO_DUMP )

	DISK_REGION( "laserdisc")
	DISK_IMAGE_READONLY("cs_12_sps", 0, NO_DUMP )
ROM_END


/*************************************
 *
 *  Driver setup
 *
 *************************************/

/* question address is stored as L/H/X (low/high/don't care) */
void statriv2_state::init_addr_lhx()
{
	m_question_offset_low = 0;
	m_question_offset_mid = 1;
	m_question_offset_high = 0xff;
}

/* question address is stored as X/L/H (don't care/low/high) */
void statriv2_state::init_addr_xlh()
{
	m_question_offset_low = 1;
	m_question_offset_mid = 2;
	m_question_offset_high = 0xff;
}

/* question address is stored as X/H/L (don't care/high/low) */
void statriv2_state::init_addr_xhl()
{
	m_question_offset_low = 2;
	m_question_offset_mid = 1;
	m_question_offset_high = 0xff;
}

/* question address is stored as L/M/H (low/mid/high) */
void statriv2_state::init_addr_lmh()
{
	m_question_offset_low = 0;
	m_question_offset_mid = 1;
	m_question_offset_high = 2;
}

void statriv2_state::init_addr_lmhe()
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

	uint8_t *qrom = memregion("questions")->base();
	uint32_t length = memregion("questions")->bytes();
	uint32_t address;

	for (address = 0; address < length; address++)
		qrom[address] ^= bitswap<8>(address, 4,3,3,2,2,1,1,0);

	init_addr_lmh();
}

void statriv2_state::init_laserdisc()
{
	address_space &iospace = m_maincpu->space(AS_IO);
	iospace.install_readwrite_handler(0x28, 0x2b,
		read8_delegate(*this, NAME([this] (address_space &space, offs_t offset, uint8_t mem_mask) -> uint8_t
		{
			uint8_t result = 0x00;
			if (offset == 1)
				result = 0x18;
			logerror("%s:ld read ($%02X) = %02X\n", machine().describe_context(), 0x28 + offset, result);
			return result;
		})),
		write8_delegate(*this, NAME([this] (address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
		{
			logerror("%s:ld write ($%02X) = %02X\n", machine().describe_context(), 0x28 + offset, data);
		}))
	);
}

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1981, statusbj,   0,        statusbj,  statusbj, statriv2_state, empty_init,     ROT0,  "Status Games",       "Status Black Jack (V1.0c)",             MACHINE_SUPPORTS_SAVE )
GAME( 1981, funcsino,   0,        funcsino,  funcsino, statriv2_state, empty_init,     ROT0,  "Status Games",       "Status Fun Casino (V1.3s)",             MACHINE_SUPPORTS_SAVE )
GAME( 1981, tripdraw,   0,        tripdraw,  tripdraw, statriv2_state, empty_init,     ROT0,  "Status Games",       "Tripple Draw (V3.1 s)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1984, bigcsino,   0,        statusbj,  bigcsino, statriv2_state, empty_init,     ROT0,  "Status Games",       "Big Casino",                            MACHINE_SUPPORTS_SAVE )
GAME( 1984, hangman,    0,        statriv2,  hangman,  statriv2_state, init_addr_lmh,  ROT0,  "Status Games",       "Hangman",                               MACHINE_SUPPORTS_SAVE )
GAME( 1984, trivquiz,   0,        statriv2,  statriv2, statriv2_state, init_addr_lhx,  ROT0,  "Status Games",       "Triv Quiz",                             MACHINE_SUPPORTS_SAVE )
GAME( 1984, statriv2,   0,        statriv2,  statriv2, statriv2_state, init_addr_xlh,  ROT0,  "Status Games",       "Triv Two",                              MACHINE_SUPPORTS_SAVE )
GAME( 1985, statriv2v,  statriv2, statriv2v, statriv2, statriv2_state, init_addr_xlh,  ROT90, "Status Games",       "Triv Two (Vertical)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1985, statriv4,   0,        statriv2,  statriv4, statriv2_state, init_addr_xhl,  ROT0,  "Status Games",       "Triv Four",                             MACHINE_SUPPORTS_SAVE )
GAME( 1985, statriv5se, statriv4, statriv2,  statriv4, statriv2_state, init_addr_xhl,  ROT0,  "Status Games",       "Triv Five Special Edition",             MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // missing questions' ROMs
GAME( 1985, sextriv,    0,        statriv2,  sextriv,  statriv2_state, init_addr_lhx,  ROT0,  "Status Games",       "Sex Triv",                              MACHINE_SUPPORTS_SAVE )
GAME( 1985, quaquiz2,   0,        statriv2,  quaquiz2, statriv2_state, init_addr_lmh,  ROT0,  "Status Games",       "Quadro Quiz II",                        MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1985, supertr,    0,        statriv2,  supertr2, statriv2_state, init_addr_lhx,  ROT0,  "Status Games",       "Super Triv Quiz I",                     MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // missing questions' ROMs
GAME( 1986, bbchall,    0,        statriv2,  bbchall,  statriv2_state, empty_init,     ROT0,  "Status Games",       "Baby Boom Challenge",                   MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // wrong satellite board message at startup. Also missing questions' ROMs.
GAME( 1986, supertr2,   0,        statriv2,  supertr2, statriv2_state, init_addr_lmhe, ROT0,  "Status Games",       "Super Triv II",                         MACHINE_SUPPORTS_SAVE )
GAME( 1988, supertr3,   0,        statriv2,  supertr2, statriv2_state, init_addr_lmh,  ROT0,  "Status Games",       "Super Triv III",                        MACHINE_SUPPORTS_SAVE )
GAME( 1988, nsupertr3,  supertr3, statriv2,  supertr2, statriv2_state, init_addr_lmh,  ROT0,  "Status Games",       "New Super Triv III",                    MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // new questions don't appear correctly, coinage problems
// The following Casino Strip sets don't show the version on screen (at least without the laserdisc video). It was taken from the rom labels / from the Dragon's Lair Project archive.
GAME( 1984, cs1_spp,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip I (Poker version, for Pioneer LD, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1984, cs1_spp2,  cs1_spp,   statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip I (Poker version, for Pioneer LD, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs2_sps,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip II (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1992, cs3_qps,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Quantum Industries", "Casino Strip III (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1985, cs5_spp,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip V (Poker version, for Pioneer LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1985, cs5_ssp,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip V (Shooting Game version, for Pioneer LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs6_sps,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip VI (Poker version, for Sony LD)",   MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1985, cs6_ssp,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip VI (Shooting Game version, for Pioneer LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1986, cs8_ssp,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip VIII (Shooting Game version, for Pioneer LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1985, cs8_spp,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip VIII (Poker version, for Pioneer LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs8_sps,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip VIII (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1992, cs9_qps,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Quantum Industries", "Casino Strip IX (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1985, cs9_spp,   0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip IX (Poker version, for Pioneer LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs10_sps,  0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip X (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs11_ssp,  0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip XI (Shooting Game version, for Pioneer LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs11_sps,  0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip XI (Poker version, for Sony LD, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs11_sps2, cs11_sps,  statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip XI (Poker version, for Sony LD, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1988, cs12_sps,  0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Status Games",       "Casino Strip XII (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1996, cspe_qps,  0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Quantum Industries", "Casino Strip Private Eyes / All Start (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, csv1_qps,  0,         statusbj,  funcsino, statriv2_state, init_laserdisc, ROT0,  "Quantum Industries", "Casino Strip Vivid 1 (Poker version, for Sony LD)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
