// license:BSD-3-Clause
// copyright-holders:hap
/****************************************************************************************

2018-09-15

Video 21 blackjack game
VIDEO-GAMES - LICH/GERMANY 1017a

NEC D8080AFC (i8080), 20.79MHz xtal, bank of 7 dips. 10x 4-bit proms, type F93453 (=82S137)
Video Ram = 7 x 2102 (bit 7 omitted). Main Ram = 2x MCM145101 (=M5101L).
The game has sound (there's a LM380N visible), looks like there's a bunch of TTL chips
involved, and a 555.

TODO:
- improve sound, it's definitely beeper pitch control, but sounds offtune
- identify all dips (7 total)
- confirm CPU clock

When booted, press Key out (mapped to W by default) to get it going.

*******************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/nvram.h"
#include "sound/beep.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"

#include "video21.lh"


namespace {

class video21_state : public driver_device
{
public:
	video21_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_beeper(*this, "beeper")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void video21(machine_config &config);
	int hopper_coinout_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<beep_device> m_beeper;
	output_finder<6> m_lamps;

	int m_hopper_motor = 0;
	int m_hopper_coin = 0;
	emu_timer *m_hopper_timer = nullptr;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_w(uint8_t data);
	void lamp1_w(uint8_t data);
	void lamp2_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(hopper_coinout);
};


uint32_t video21_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;

	for (uint8_t y = 0; y < 28; y++)
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 32; x++)
			{
				uint8_t chr = m_p_videoram[x+ma] & 0x7f;
				uint8_t gfx = m_p_chargen[(chr<<3) | ra ];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=32;
	}
	return 0;
}

TIMER_CALLBACK_MEMBER(video21_state::hopper_coinout)
{
	m_hopper_coin = param;

	if (m_hopper_motor || m_hopper_coin)
		m_hopper_timer->adjust(attotime::from_msec(100), m_hopper_coin ^ 1);
}

void video21_state::sound_w(uint8_t data)
{
	// beeper pitch
	m_beeper->set_state(data != 0xff); // FF is off
	m_beeper->set_clock(4 * data);
}

void video21_state::lamp1_w(uint8_t data)
{
	// d1-d3: coincounters
	machine().bookkeeping().coin_counter_w(0, data & 0x04); // coin in
	machine().bookkeeping().coin_counter_w(1, data & 0x08); // keeper coin out
	machine().bookkeeping().coin_counter_w(2, data & 0x02); // hopper coin out

	// d4: hopper motor
	if (!m_hopper_motor && BIT(data, 4))
		m_hopper_timer->adjust(attotime::from_msec(100), 1);
	m_hopper_motor = BIT(data, 4);

	// lamps:
	// d5: card/stop(which?)
	// d6: card/stop(which?)
	// d7: start
	for (int i = 0; i < 3; i++)
		m_lamps[i+0] = BIT(data, 7-i);
}

void video21_state::lamp2_w(uint8_t data)
{
	// lamps:
	// d5: bet
	// d6: accept win/double(which?)
	// d7: accept win/double(which?)
	for (int i = 0; i < 3; i++)
		m_lamps[i+3] = BIT(data, 7-i);
}


void video21_state::mem_map(address_map &map) {
	map(0x0000,0x0fff).rom().mirror(0x3000);
	map(0xe000,0xe3ff).ram().share("videoram");
	map(0xff00,0xffff).ram().share("nvram");
}

void video21_state::io_map(address_map &map) {
	map(0x02,0x02).w(FUNC(video21_state::sound_w));
	map(0x04,0x04).w(FUNC(video21_state::lamp1_w));
	map(0x08,0x08).w(FUNC(video21_state::lamp2_w));
	map(0x41,0x41).portr("IN41");
	map(0x42,0x42).portr("IN42");
	map(0x44,0x44).portr("IN44");
}


int video21_state::hopper_coinout_r()
{
	return m_hopper_coin;
}

static INPUT_PORTS_START( video21 )
	PORT_START("IN41")
	PORT_DIPNAME( 0x01, 0x01, "41b0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "41b1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "41b2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Win Mode" )
	PORT_DIPSETTING(    0x08, "Amusement" ) // winnings get added to credits
	PORT_DIPSETTING(    0x00, "Casino" ) // winnings go to coin out
	PORT_DIPNAME( 0x10, 0x10, "41b4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT ) // ?

	PORT_START("IN42")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Card")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(video21_state, hopper_coinout_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN44")
	PORT_DIPNAME( 0x01, 0x01, "44b0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "44b1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "44b2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double Or Nothing")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Accept Win")
	PORT_DIPNAME( 0x80, 0x80, "Max Bet" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x00, "2" )
INPUT_PORTS_END


static const gfx_layout video21_charlayout =
{
	8, 8,                   // 8 x 8 characters
	128,                    // 128 characters, but only the first 76 look useful
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_video21 )
	GFXDECODE_ENTRY( "chargen", 0x0000, video21_charlayout, 0, 1 )
GFXDECODE_END

void video21_state::machine_start()
{
	m_hopper_timer = timer_alloc(FUNC(video21_state::hopper_coinout), this);

	m_lamps.resolve();

	// zerofill/register for savestates
	m_hopper_motor = 0;
	m_hopper_coin = 0;

	save_item(NAME(m_hopper_motor));
	save_item(NAME(m_hopper_coin));
}

void video21_state::video21(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 20.79_MHz_XTAL / 10); // crystal confirmed but divisor unknown (divider appears to be 74LS160)
	m_maincpu->set_addrmap(AS_PROGRAM, &video21_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &video21_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::white());
	screen.set_raw(20.79_MHz_XTAL / 4, 330, 0, 32*8, 315, 0, 28*8); // parameters guessed
	screen.set_screen_update(FUNC(video21_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_video21);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}


ROM_START( video21 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "2",  0x0000, 0x0400, CRC(05585e39) SHA1(7eefb5d63b4499a303ecdcad6af5df9fe9c89205) )
	ROM_LOAD_NIB_LOW ( "3",  0x0000, 0x0400, CRC(b9134e96) SHA1(e7a8ff71f735add608d3c9dcc287ca37414debcb) )
	ROM_LOAD_NIB_HIGH( "4",  0x0400, 0x0400, CRC(8a6aa143) SHA1(16973106a95b17d8c4712db8aa7ef564751ae6d4) )
	ROM_LOAD_NIB_LOW ( "5",  0x0400, 0x0400, CRC(98c07d4d) SHA1(d3126b5484c67ecc7c44ddc25b48ff72ed8a734f) )
	ROM_LOAD_NIB_HIGH( "76", 0x0800, 0x0400, CRC(737c27f5) SHA1(55c7eb29b979d35633e5fb2c1c1ac3117901a0f0) )
	ROM_LOAD_NIB_LOW ( "78", 0x0800, 0x0400, CRC(c1081a2f) SHA1(24dc1d9afa4635c3114369c338903343a8b81d1a) )
	ROM_LOAD_NIB_HIGH( "77", 0x0c00, 0x0400, CRC(3a725b98) SHA1(efa3802025f1f99b45e56abd85dc3d1860d4734d) )
	ROM_LOAD_NIB_LOW ( "79", 0x0c00, 0x0400, CRC(044d1bfd) SHA1(5a57c1ab7eb7dd7ed05852e128b704c3b37a87b8) )

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD_NIB_HIGH( "29", 0x0000, 0x0400, CRC(2b70870d) SHA1(1f50a6976e1634020c78f10c1259e38f5e010a86) )
	ROM_LOAD_NIB_LOW ( "43", 0x0000, 0x0400, CRC(0ecb0aab) SHA1(7f3f1b93a5d38828ae3e97e5f8ef1a6a96dc798b) )
ROM_END

} // anonymous namespace


GAMEL(1980?, video21, 0, video21, video21, video21_state, empty_init, ROT0, "Video Games GmbH", "Video 21", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_video21)
