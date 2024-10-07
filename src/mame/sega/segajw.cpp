// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

============================================================================

SEGA GOLDEN POKER SERIES "JOKER'S WILD" (REV.B)
(c) SEGA

MAIN CPU  : 68000 Z-80
CRTC      : HITACHI HD63484 (24KHz OUTPUT)
SOUND     : YM3438

14584B.EPR  ; MAIN BOARD  IC20 EPR-14584B (27C1000 MAIN-ODD)
14585B.EPR  ; MAIN BOARD  IC22 EPR-14585B (27C1000 MAIN-EVEN)
14586.EPR   ; MAIN BOARD  IC26 EPR-14586  (27C4096 BG)
14587A.EPR  ; SOUND BOARD IC51 EPR-14587A (27C1000 SOUND)

------------------------------------------------------------------

***************************************************************************/

/*
Also seem to be running on the same/similar hardware:
* Deuce's Wild (http://topline.royalflush.jp/modules/contents/?%A5%DE%A5%B7%A5%F3%A5%C7%A1%BC%A5%BF%A5%D9%A1%BC%A5%B9%2F%A5%D3%A5%C7%A5%AA%A5%DD%A1%BC%A5%AB%A1%BC%2FSEGA%2FDEUCE%27S_WILD)
* Draw Poker (http://topline.royalflush.jp/modules/contents/?%A5%DE%A5%B7%A5%F3%A5%C7%A1%BC%A5%BF%A5%D9%A1%BC%A5%B9%2F%A5%D3%A5%C7%A5%AA%A5%DD%A1%BC%A5%AB%A1%BC%2FSEGA%2FDRAW_POKER)
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "315_5296.h"
#include "sound/ymopn.h"
#include "video/hd63484.h"
#include "video/ramdac.h"
#include "screen.h"
#include "speaker.h"

#include "segajw.lh"

namespace {

class segajw_state : public driver_device
{
public:
	segajw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_lamps(*this, "lamp%u", 0U)
		, m_towerlamps(*this, "towerlamp%u", 0U)
	{ }

	void segajw(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_drop_start);
	ioport_value coin_sensors_r();
	int hopper_sensors_r();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t coin_counter_r();
	void coin_counter_w(uint8_t data);
	void hopper_w(uint8_t data);
	void lamps1_w(uint8_t data);
	void lamps2_w(uint8_t data);
	void coinlockout_w(uint8_t data);

	void ramdac_map(address_map &map) ATTR_COLD;
	void segajw_audiocpu_io_map(address_map &map) ATTR_COLD;
	void segajw_audiocpu_map(address_map &map) ATTR_COLD;
	void segajw_hd63484_map(address_map &map) ATTR_COLD;
	void segajw_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	output_finder<16> m_lamps;
	output_finder<3> m_towerlamps;

	uint64_t      m_coin_start_cycles = 0;
	uint64_t      m_hopper_start_cycles = 0;
	uint8_t       m_coin_counter = 0;
};


uint8_t segajw_state::coin_counter_r()
{
	return m_coin_counter ^ 0xff;
}

void segajw_state::coin_counter_w(uint8_t data)
{
	m_coin_counter = data;
}

void segajw_state::hopper_w(uint8_t data)
{
	m_hopper_start_cycles = data & 0x02 ? 0 : m_maincpu->total_cycles();
}

void segajw_state::lamps1_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
}

void segajw_state::lamps2_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_lamps[8 + i] = BIT(data, i);
}

void segajw_state::coinlockout_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, data & 1);

	for (int i = 0; i < 3; i++)
		m_towerlamps[i] = BIT(data, 3 + i);
}

INPUT_CHANGED_MEMBER( segajw_state::coin_drop_start )
{
	if (newval && !m_coin_start_cycles)
		m_coin_start_cycles = m_maincpu->total_cycles();
}

int segajw_state::hopper_sensors_r()
{
	uint8_t data = 0;

	// if the hopper is active simulate the coin-out sensor
	if (m_hopper_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_hopper_start_cycles);

		if (diff > attotime::from_msec(100))
			data |= 0x01;

		if (diff > attotime::from_msec(200))
			m_hopper_start_cycles = m_maincpu->total_cycles();
	}

	return data;
}

ioport_value segajw_state::coin_sensors_r()
{
	uint8_t data = 0;

	// simulates the passage of coins through multiple sensors
	if (m_coin_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_coin_start_cycles);

		if (diff > attotime::from_msec(20) && diff < attotime::from_msec(100))
			data |= 0x01;
		if (diff > attotime::from_msec(80) && diff < attotime::from_msec(200))
			data |= 0x02;
		if (diff <= attotime::from_msec(100))
			data |= 0x04;

		if (diff > attotime::from_msec(200))
			m_coin_start_cycles = 0;
	}

	return data;
}

void segajw_state::segajw_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0x080000, 0x080003).rw("hd63484", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));

	map(0x180000, 0x180001).portr("DSW0");
	map(0x180005, 0x180005).r("soundlatch2", FUNC(generic_latch_8_device::read)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0x180008, 0x180009).portr("DSW1");
	map(0x18000a, 0x18000b).portr("DSW3");
	map(0x18000c, 0x18000d).portr("DSW2");

	map(0x1a0000, 0x1a001f).rw("io1a", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);

	map(0x1c0000, 0x1c001f).rw("io1c", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);

	map(0x280001, 0x280001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x280003, 0x280003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x280005, 0x280005).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0xff0000, 0xffffff).ram().share("nvram");
}

void segajw_state::segajw_audiocpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xffff).ram();
}

void segajw_state::segajw_audiocpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x83).rw("ymsnd", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0xc0, 0xc0).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

void segajw_state::segajw_hd63484_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0x80000, 0xbffff).rom().region("gfx1", 0);
}


static INPUT_PORTS_START( segajw )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("1 Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )      PORT_NAME("Max Bet")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )      PORT_NAME("Deal / Draw")

	PORT_START("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )      PORT_NAME("Double")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )      PORT_NAME("Change")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reset")     PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Meter")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Last Game")   PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("M-Door")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("D-Door")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )       PORT_READ_LINE_MEMBER(segajw_state, hopper_sensors_r)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Hopper Full")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Hopper Fill")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(segajw_state, coin_sensors_r)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN1") // start the coin drop sequence (see coin_sensors_r)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )   PORT_CHANGED_MEMBER(DEVICE_SELF, segajw_state, coin_drop_start, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, "Progressive" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x0001, "Normal" )
	PORT_DIPSETTING(    0x0000, "Progressive" )
	PORT_DIPNAME( 0x0002, 0x0000, "Double Down" )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x0002, "With D.D" )
	PORT_DIPSETTING(    0x0000, "Without D.D" )
	PORT_DIPNAME( 0x0004, 0x0000, "Draw Cards" )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x0004, "Display" )
	PORT_DIPSETTING(    0x0000, "No Display" )
	PORT_DIPNAME( 0x0008, 0x0000, "Color Change" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x0008, "Change" )
	PORT_DIPSETTING(    0x0000, "No Change" )
	PORT_DIPNAME( 0x0010, 0x0000, "Odds Table" )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x0010, "No Display" )
	PORT_DIPSETTING(    0x0000, "Display" )
	PORT_DIPNAME( 0x0060, 0x0000, "Play Mode" )     PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x0000, "Coin" )
	PORT_DIPSETTING(    0x0020, "Coin/Credit" )
	// PORT_DIPSETTING(    0x0040, "Coin/Credit" )
	PORT_DIPSETTING(    0x0060, "Credit" )
	PORT_DIPNAME( 0x0080, 0x0000, "Best Choice" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x0080, "Display" )
	PORT_DIPSETTING(    0x0000, "No Display" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Denomination" )  PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x0000, "$25" )
	PORT_DIPSETTING(    0x0001, "$5" )
	PORT_DIPSETTING(    0x0002, "$1" )
	PORT_DIPSETTING(    0x0003, "50c" )
	PORT_DIPSETTING(    0x0004, "25c" )
	PORT_DIPSETTING(    0x0005, "10c" )
	PORT_DIPSETTING(    0x0006, "5c" )
	PORT_DIPSETTING(    0x0007, "Medal" )
	PORT_DIPNAME( 0x0008, 0x0000, "Max. Pay" )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x0008, "500" )
	PORT_DIPSETTING(    0x0000, "1000" )
	PORT_DIPNAME( 0x0010, 0x0000, "Max. Credit" )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x0010, "5000" )
	PORT_DIPSETTING(    0x0000, "1000" )
	PORT_DIPNAME( 0x0020, 0x0000, "$1200" )         PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x0020, "At. Pay" )
	PORT_DIPSETTING(    0x0000, "Credit/At. Pay" )
	PORT_DIPNAME( 0x00c0, 0x0000, "Max. Bet" )      PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x0000, "10" )
	PORT_DIPSETTING(    0x00c0, "5" )
	PORT_DIPSETTING(    0x0040, "3" )
	PORT_DIPSETTING(    0x0080, "1" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, "Meter" )         PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x0001, "Nevada" )
	PORT_DIPSETTING(    0x0000, "New Jersey" )
	PORT_DIPNAME( 0x0002, 0x0002, "Card Face" )     PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x0002, "Changeable" )
	PORT_DIPSETTING(    0x0000, "Original" )
	PORT_DIPNAME( 0x0004, 0x0004, "Card Back" )     PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x0004, "Changeable" )
	PORT_DIPSETTING(    0x0000, "Original" )
	PORT_DIPUNUSED( 0x0008, 0x0008)                 PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNUSED( 0x0010, 0x0010)                 PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNUSED( 0x0020, 0x0020)                 PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNUSED( 0x0040, 0x0040)                 PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNUSED( 0x0080, 0x0080)                 PORT_DIPLOCATION("SW3:8")

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0000, "Jumper 1" )      PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "Jumper 2" )      PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Jumper 3" )      PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "Jumper 4" )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Jumper 5" )      PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Jumper 6" )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Jumper 7" )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Jumper 8" )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END


void segajw_state::machine_start()
{
	m_lamps.resolve();
	m_towerlamps.resolve();

	m_coin_start_cycles = 0;
	m_hopper_start_cycles = 0;

	save_item(NAME(m_coin_start_cycles));
	save_item(NAME(m_hopper_start_cycles));
	save_item(NAME(m_coin_counter));
}


void segajw_state::machine_reset()
{
	m_coin_counter = 0xff;
}

void segajw_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void segajw_state::segajw(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &segajw_state::segajw_map);
	m_maincpu->set_vblank_int("screen", FUNC(segajw_state::irq4_line_hold));

	Z80(config, m_audiocpu, 4000000); // unknown clock
	m_audiocpu->set_addrmap(AS_PROGRAM, &segajw_state::segajw_audiocpu_map);
	m_audiocpu->set_addrmap(AS_IO, &segajw_state::segajw_audiocpu_io_map);

	config.set_maximum_quantum(attotime::from_hz(2000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	sega_315_5296_device &io1a(SEGA_315_5296(config, "io1a", 0)); // unknown clock
	io1a.out_pa_callback().set(FUNC(segajw_state::coin_counter_w));
	io1a.out_pb_callback().set(FUNC(segajw_state::lamps1_w));
	io1a.out_pc_callback().set(FUNC(segajw_state::lamps2_w));
	io1a.out_pd_callback().set(FUNC(segajw_state::hopper_w));
	io1a.in_pf_callback().set(FUNC(segajw_state::coin_counter_r));

	sega_315_5296_device &io1c(SEGA_315_5296(config, "io1c", 0)); // unknown clock
	io1c.in_pa_callback().set_ioport("IN0");
	io1c.in_pb_callback().set_ioport("IN1");
	io1c.in_pc_callback().set_ioport("IN2");
	io1c.in_pd_callback().set_ioport("IN3");
	io1c.out_pg_callback().set(FUNC(segajw_state::coinlockout_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update("hd63484", FUNC(hd63484_device::update_screen));
	screen.set_size(720, 480);
	screen.set_visarea(0, 720-1, 0, 448-1);
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(16);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, "palette"));
	ramdac.set_addrmap(0, &segajw_state::ramdac_map);

	hd63484_device &hd63484(HD63484(config, "hd63484", 8000000));
	hd63484.set_addrmap(0, &segajw_state::segajw_hd63484_map); // unknown clock

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, "soundlatch2");

	ym3438_device &ymsnd(YM3438(config, "ymsnd", 8000000));   // unknown clock
	ymsnd.irq_handler().set_inputline("maincpu", 5);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( segajw )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "14584b.epr",   0x00001, 0x20000, CRC(d3a6d63d) SHA1(ce9d4769b7514294a91af1dfd7cd10ee40b3572c) )
	ROM_LOAD16_BYTE( "14585b.epr",   0x00000, 0x20000, CRC(556d0a62) SHA1(d2def433a511cbdebbe2cd0c8e51fc8c4ff1ed7b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "14587a.epr",   0x00000, 0x20000, CRC(66163b6c) SHA1(88e994bcad86c58dc730a93b48226e9296df7667) )

	ROM_REGION16_BE( 0x80000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "14586.epr",   0x00000, 0x80000, CRC(daeb0616) SHA1(17a8bb7137ad46a7c3ac07d22cbc4430e76e2f71) )
ROM_END

} // Anonymous namespace


GAMEL( 1991, segajw, 0, segajw,  segajw, segajw_state, empty_init, ROT0, "Sega", "Joker's Wild (Rev. B)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_segajw )
