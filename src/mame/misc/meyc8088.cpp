// license:BSD-3-Clause
// copyright-holders:hap, Roberto Fresca
// thanks-to:Darrell Hal Smith, Kevin Mullins
/****************************************************************

  Meyco 8088 based hardware

  i8088 CPU @ 5MHz (15MHz XTAL + i8284A clock generator),
  3 x 8KB EPROM (max 4), 3 x 8KB RAM (max 4), 2KB battery RAM,
  2 x i8155, optional i8251A + RS232 for factory debug

  To initialize battery RAM, go into Meter Read mode (F1 -> 9),
  and then press the Meter Read + Reset buttons (9 + 0).

  If a game is not turned off properly, eg. exiting MAME
  in mid-game, it may run faulty on the next boot.
  Enable the Night Switch to prevent this.

  TODO:
  - coincounters/hopper
  - correct CPU speed (currently underclocked)

****************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/i8155.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "gldarrow.lh"


namespace {

class meyc8088_state : public driver_device
{
public:
	meyc8088_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_vram(*this, "vram"),
		m_heartbeat(*this, "heartbeat"),
		m_switches(*this, "C%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void meyc8088(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_vram;
	required_device<timer_device> m_heartbeat;

	required_ioport_array<4> m_switches;

	output_finder<16> m_lamps;

	uint8_t m_status = 0;
	uint8_t m_common = 0;

	void drive_w(uint8_t data);
	void video5_flip_w(uint8_t data);
	uint8_t video5_flip_r();
	void screen_flip_w(uint8_t data);
	uint8_t screen_flip_r();
	uint8_t input_r();
	uint8_t status_r();
	void lights1_w(uint8_t data);
	void lights2_w(uint8_t data);
	void common_w(uint8_t data);

	void meyc8088_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(heartbeat_callback);
	void meyc8088_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Video

***************************************************************************/

/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:
  (even and uneven pins were switched around in the schematics)

  bit 7 -- N/C
        -- 820 ohm resistor  -- GREEN
        --                   -- GREEN
        -- 820 ohm resistor  -- BLUE
        --                   -- BLUE
        -- 820 ohm resistor  -- RED
        --                   -- RED
  bit 0 -- N/C

  plus 330 ohm pullup resistors on all lines

***************************************************************************/

static const res_net_decode_info meyc8088_decode_info =
{
	1,      // there may be two proms needed to construct color
	0, 31,  // start/end
	//  R,   G,   B,
	{   0,   0,   0, },     // offsets
	{   1,   5,   3, },     // shifts
	{0x03,0x03,0x03, }      // masks
};

static const res_net_info meyc8088_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_OPEN_COL,
	{
		{ RES_NET_AMP_NONE, 330, 0, 2, { 1, 820 } },
		{ RES_NET_AMP_NONE, 330, 0, 2, { 1, 820 } },
		{ RES_NET_AMP_NONE, 330, 0, 2, { 1, 820 } }
	}
};

void meyc8088_state:: meyc8088_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, meyc8088_decode_info, meyc8088_net_info);
	palette.set_pen_colors(0, rgb);
}

uint32_t meyc8088_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t v[5];
	v[4] = m_status << 2 & 0x10; // video5: color prom d4

	if (~m_status & 2)
	{
		// screen off
		bitmap.fill(v[4]);
		return 0;
	}

	for (offs_t offs = 0x800; offs < 0x4000; offs+=2)
	{
		uint8_t y = (offs-0x800) >> 6;
		uint8_t x = (offs-0x800) << 2;

		v[0] = m_vram[offs|0x0000]; // video1: color prom d0
		v[1] = m_vram[offs|0x0001]; // video2: color prom d1
		v[2] = m_vram[offs|0x4000]; // video3: color prom d2
		v[3] = m_vram[offs|0x4001]; // video4: color prom d3

		for (int i = 0; i < 8; i++)
			bitmap.pix(y, x | i) = ((v[0] << i) >> 7 & 1) | ((v[1] << i) >> 6 & 2) | ((v[2] << i) >> 5 & 4) | ((v[3] << i) >> 4 & 8) | v[4];
	}

	return 0;
}

void meyc8088_state::screen_vblank(int state)
{
	// LC255(200ns pulse) rising edge asserts INTR at start and end of vblank
	// INTA wired back to INTR to clear it, vector is hardwired to $20
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x20); // I8088
}


/***************************************************************************

  I/O

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(meyc8088_state::heartbeat_callback)
{
	m_status |= 0x20;
}

void meyc8088_state::drive_w(uint8_t data)
{
	// drivers go into high-impedance state ~100ms after write (LS374 /OC)
	m_status &= ~0x20;
	m_heartbeat->adjust(attotime::from_msec(100));

	// d0-d3: DC counter drivers
	// d4-d7: AC motor drivers
}

// switch screen on/off on $b4000 access
uint8_t meyc8088_state::screen_flip_r()
{
	m_status ^= 2;
	return 0;
}

void meyc8088_state::screen_flip_w(uint8_t data)
{
	m_status ^= 2;
}

// switch video5 (color prom d4) on/off on $b5000 access
uint8_t meyc8088_state::video5_flip_r()
{
	m_status ^= 4;
	return 0;
}

void meyc8088_state::video5_flip_w(uint8_t data)
{
	m_status ^= 4;
}


void meyc8088_state::meyc8088_map(address_map &map)
{
	map(0x00000, 0x007ff).ram().share("nvram");
	map(0x70000, 0x77fff).ram().share("vram");
	map(0xb0000, 0xb00ff).rw("i8155_2", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xb0800, 0xb0807).rw("i8155_2", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xb1000, 0xb10ff).rw("i8155_1", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xb1800, 0xb1807).rw("i8155_1", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xb2000, 0xb2000).w(FUNC(meyc8088_state::drive_w));
	map(0xb3000, 0xb3000).noprw(); // i8251A data (debug related, unpopulated on sold boards)
	map(0xb3800, 0xb3800).noprw(); // "
	map(0xb4000, 0xb4000).rw(FUNC(meyc8088_state::screen_flip_r), FUNC(meyc8088_state::screen_flip_w));
	map(0xb5000, 0xb5000).rw(FUNC(meyc8088_state::video5_flip_r), FUNC(meyc8088_state::video5_flip_w));
	map(0xf8000, 0xfffff).rom();
}


uint8_t meyc8088_state::input_r()
{
	uint8_t ret = 0xff;

	// multiplexed switch inputs
	if (~m_common & 1) ret &= m_switches[0]->read(); // bit switches
	if (~m_common & 2) ret &= m_switches[1]->read(); // control switches
	if (~m_common & 4) ret &= m_switches[2]->read(); // light switches
	if (~m_common & 8) ret &= m_switches[3]->read(); // light switches

	return ret;
}

uint8_t meyc8088_state::status_r()
{
	// d0: /CR2
	// d1: screen on
	// d2: video5
	// d3: N/C
	// d4: battery ok
	// d5: /drive on
	return (m_status & 0x27) | 0x18;
}


void meyc8088_state::lights1_w(uint8_t data)
{
	// lite 1-8
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(~data, i);
}

void meyc8088_state::lights2_w(uint8_t data)
{
	// lite 9-16
	for (int i = 0; i < 8; i++)
		m_lamps[i + 8] = BIT(~data, i);
}

void meyc8088_state::common_w(uint8_t data)
{
	// d0: /CR2
	m_status = (m_status & ~1) | (data & 1);

	// d1: battery on
	m_status = (m_status & ~0x10) | (data << 3 & 0x10);

	// d2-d5: /common
	m_common = data >> 2 & 0xf;
}

void meyc8088_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_status));
	save_item(NAME(m_common));
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( gldarrow )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) // coin4
	PORT_BIT( 0x78, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hopper coin switch?

	PORT_START("C0")
	PORT_DIPNAME( 0x03, 0x00, "Payout Percentage" )     PORT_DIPLOCATION("BSW:1,2")
	PORT_DIPSETTING(    0x03, "85%")
	PORT_DIPSETTING(    0x02, "88%")
	PORT_DIPSETTING(    0x01, "90%")
	PORT_DIPSETTING(    0x00, "93%")
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )          PORT_DIPLOCATION("BSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bonus Award" )           PORT_DIPLOCATION("BSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("BSW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )          PORT_DIPLOCATION("BSW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )          PORT_DIPLOCATION("BSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "BSW:8" )

	PORT_START("C1")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Read")
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("C2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

void meyc8088_state::meyc8088(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, (XTAL(15'000'000) / 3) * 0.95); // NOTE: underclocked to prevent errors on diagnostics, MAME i8088 cycle timing is probably inaccurate
	m_maincpu->set_addrmap(AS_PROGRAM, &meyc8088_state::meyc8088_map);

	i8155_device &i8155_1(I8155(config, "i8155_1", XTAL(15'000'000) / (3*1)));
	// all ports set to input
	i8155_1.in_pa_callback().set(FUNC(meyc8088_state::input_r));
	i8155_1.in_pb_callback().set_ioport("SW");
	i8155_1.in_pc_callback().set(FUNC(meyc8088_state::status_r));
	// i8251A trigger txc/rxc (debug related, unpopulated on sold boards)

	i8155_device &i8155_2(I8155(config, "i8155_2", XTAL(15'000'000) / (3*32)));
	// all ports set to output
	i8155_2.out_pa_callback().set(FUNC(meyc8088_state::lights2_w));
	i8155_2.out_pb_callback().set(FUNC(meyc8088_state::lights1_w));
	i8155_2.out_pc_callback().set(FUNC(meyc8088_state::common_w));
	i8155_2.out_to_callback().set("dac", FUNC(dac_bit_interface::write));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, m_heartbeat).configure_generic(FUNC(meyc8088_state::heartbeat_callback));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(15'000'000)/3, 320, 0, 256, 261, 0, 224);
	screen.set_screen_update(FUNC(meyc8088_state::screen_update));
	screen.screen_vblank().set(FUNC(meyc8088_state::screen_vblank));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(meyc8088_state::meyc8088_palette), 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}


ROM_START( gldarrow )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "3.14h",   0x0fa000, 0x002000, CRC(a4acc6df) SHA1(b25f2cf8154932834100615e2e9c44ef47a15fea) )
	ROM_LOAD( "2.13h",   0x0fc000, 0x002000, CRC(595e380d) SHA1(6f8e58f646106d33cb651d97ca6a1133f7b05373) )
	ROM_LOAD( "1.12h",   0x0fe000, 0x002000, CRC(71bd0e39) SHA1(15345f5726cd33ecb1b2da05f2852b6cc3ac7747) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom.2c", 0x00, 0x20, CRC(2839bb14) SHA1(c9acdb3ae00c2f9344aedaf77c0f4e860a3184fc) ) // M3-7602-5 color prom
ROM_END

} // anonymous namespace


GAMEL( 1984, gldarrow, 0, meyc8088, gldarrow, meyc8088_state, empty_init, ROT0, "Meyco Games, Inc.", "Golden Arrow (Standard G8-03)", MACHINE_SUPPORTS_SAVE, layout_gldarrow )
