// license:BSD-3-Clause
// copyright-holders:
/*
Play Sonic 4 by SegaSA / Sonic

This is a multi-game system. Up to 4 JAMMA PCBs can be connected and the player can decide which game to play.
The system offers digital counters for time of play and credits (configurable via dips), and game statistics.

The PCB doesn't seem to have any markings.
Main components are:
Z8400AB1 main CPU
3x 6116 RAMs
8-dip bank
4-dip bank
8 MHz XTAL (near main CPU)
20 MHz XTAL
lots of TTL
4x digital counters
lots of wires

There's a very small riser PCB marked 1B-2001-241 with a couple of TTL  and a slightly bigger one marked 1B-2001-238 with 3 TTL.

Notes:
- The outputs at I/O 0x00-0x1f are four 8-bit addressable latches (74LS259) using D0.
- The cabinet has two LED displays above the monitor: credits (2 digits) and time units (3 digits).
  They're driven through a BCD bus (inverted, probably by the ULN2003 on the 1B-2001-241 riser)
  plus one latch strobe per digit. One credit buys 150 time units.
- The upper 1 KiB of the battery backed RAM (0x8c00-0x8fff) holds the configuration and
  statistics. Writing to it needs I/O 0x0f set, the program sets it around every access.
- The menu video (and probably the game PCBs' video) is selected with one relay per source.
  Game PCB video can't be emulated, so the screen is black while a game PCB is selected.
  Without credits, the menu periodically cycles through the active game PCBs' video as attract.
- With a blank NVRAM the system asks to "program the game PCB names" in test mode. To get to
  the game selection menu, turn on the test switch, choose 'PROGRAMAR PLACA JUEGO', pick a PCB,
  set it to 'PLACA SI ACTIVADA', return to the main test menu and turn the test switch off.
- The boot self-test (ROM / RAM checks with delay loops) takes almost a minute.

TODO:
- Colors come from three 18-pin chips with scratched-off markings (probably 1Kx4 PROMs, one
  per RGB channel, each feeding a two-resistor DAC) addressed by the pixel data and the upper
  six bits of the attribute byte. They aren't dumped.
- I/O 0x00 is cleared around every video RAM access and set again afterwards (display enable?).
- I/O 0x09 is only set during the boot delay loops (game PCB reset?).
- Screen raw parameters are guessed. The program counts 60 NMIs per second.
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/nvram.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#include "plsonic4.lh"


namespace {

class plsonic4_state : public driver_device
{
public:
	plsonic4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_outlatch(*this, "outlatch%u", 0U),
		m_videoram(*this, "videoram"),
		m_nvram(*this, "nvram"),
		m_proms(*this, "proms"),
		m_digits(*this, "digit%u", 0U)
	{ }

	void plsonic4(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<ls259_device, 4> m_outlatch;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_nvram;
	required_region_ptr<uint8_t> m_proms;
	output_finder<5> m_digits;

	tilemap_t *m_tilemap = nullptr;

	void palette_init(palette_device &palette) const ATTR_COLD;
	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	void videoram_w(offs_t offset, uint8_t data);
	void nvram_w(offs_t offset, uint8_t data);
	void nmi_enable_w(int state);
	void update_digits();

	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void plsonic4_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(plsonic4_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void plsonic4_state::palette_init(palette_device &palette) const
{
	// TODO: placeholder, the color PROMs aren't dumped
	for (int i = 0; i < palette.entries(); i++)
		palette.set_pen_color(i, pal2bit(m_proms[i]), pal2bit(m_proms[0x400 + i]), pal2bit(m_proms[0x800 + i]));
}

TILE_GET_INFO_MEMBER(plsonic4_state::tile_info)
{
	// attribute bits 0-1 are the tile bank, bits 2-7 go to the color PROMs
	uint8_t const attr = m_videoram[tile_index * 2 + 1];
	int const code = m_videoram[tile_index * 2] | (attr & 0x03) << 8;

	tileinfo.set(0, code, attr >> 2, 0);
}

uint32_t plsonic4_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_outlatch[1]->q6_r())
		m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(screen.palette().black_pen(), cliprect); // a game PCB is selected

	return 0;
}

void plsonic4_state::vblank_w(int state)
{
	if (state && m_outlatch[1]->q0_r())
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void plsonic4_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset >> 1);
}

void plsonic4_state::nvram_w(offs_t offset, uint8_t data)
{
	if (m_outlatch[1]->q7_r())
		m_nvram[0x400 + offset] = data;
	else
		logerror("%s: write to protected NVRAM %04x = %02x\n", machine().describe_context(), 0x8c00 + offset, data);
}

void plsonic4_state::nmi_enable_w(int state)
{
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void plsonic4_state::update_digits()
{
	// each digit follows the BCD bus while its strobe output is high and holds it when it goes low,
	// values above 9 blank the digit
	static constexpr uint8_t PATTERNS[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

	uint8_t const display = m_outlatch[3]->output_state();
	uint8_t const bcd = ~display & 0x0f;
	uint8_t const strobes =
			(m_outlatch[0]->q4_r() << 0) |  // credits tens
			(BIT(display, 7) << 1) |        // credits units
			(BIT(display, 6) << 2) |        // time hundreds
			(BIT(display, 5) << 3) |        // time tens
			(BIT(display, 4) << 4);         // time units

	for (int i = 0; i < 5; i++)
	{
		if (BIT(strobes, i))
			m_digits[i] = (bcd < 10) ? PATTERNS[bcd] : 0;
	}
}


void plsonic4_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8fff).ram().share(m_nvram);
	map(0x8c00, 0x8fff).w(FUNC(plsonic4_state::nvram_w));
	map(0x9000, 0x97ff).ram().w(FUNC(plsonic4_state::videoram_w)).share(m_videoram);
	map(0x9800, 0x99ff).nopw(); // the screen clear routine writes past the end of video RAM
}

void plsonic4_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("DSW");
	map(0x00, 0x07).w(m_outlatch[0], FUNC(ls259_device::write_d0));
	map(0x08, 0x0f).w(m_outlatch[1], FUNC(ls259_device::write_d0));
	map(0x10, 0x17).w(m_outlatch[2], FUNC(ls259_device::write_d0));
	map(0x18, 0x1f).w(m_outlatch[3], FUNC(ls259_device::write_d0));
}


static INPUT_PORTS_START( plsonic4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	// The test mode shows the switch settings and numbers them as below (switches are on when the bit is set)
	PORT_START("IN2") // coins + 4 dip bank
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) // counted as a coin 1 insertion
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SWA:!2,!1,!4,!3") // per credit
	PORT_DIPSETTING(    0x00, "2:00" )
	PORT_DIPSETTING(    0x20, "2:10" )
	PORT_DIPSETTING(    0x10, "2:20" )
	PORT_DIPSETTING(    0x30, "2:30" )
	PORT_DIPSETTING(    0x80, "2:40" )
	PORT_DIPSETTING(    0xa0, "2:50" )
	PORT_DIPSETTING(    0x90, "3:00" )
	PORT_DIPSETTING(    0xb0, "3:10" )
	PORT_DIPSETTING(    0x40, "3:30" )
	PORT_DIPSETTING(    0x60, "3:40" )
	PORT_DIPSETTING(    0x50, "4:00" )
	PORT_DIPSETTING(    0x70, "4:10" )
	PORT_DIPSETTING(    0xc0, "4:30" )
	PORT_DIPSETTING(    0xe0, "5:00" )
	PORT_DIPSETTING(    0xd0, "5:30" )
	PORT_DIPSETTING(    0xf0, "6:00" )

	PORT_START("DSW") // setting both coin slots to 'Disabled' gives free play
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWB:!2,!1,!4,!3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0f, "Disabled" ) // 'inhibido'
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWB:!6,!5,!8,!7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xf0, "Disabled" ) // 'inhibido'
INPUT_PORTS_END


static GFXDECODE_START( gfx_plsonic4 )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x3_planar, 0, 64 )
GFXDECODE_END


void plsonic4_state::plsonic4(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &plsonic4_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &plsonic4_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	LS259(config, m_outlatch[0]);
	// Q0: TODO, cleared around video RAM accesses
	// Q1: coin pulse to the selected game PCB
	// Q2: P1 start pulse to the selected game PCB
	// Q3: P2 start pulse to the selected game PCB
	m_outlatch[0]->q_out_cb<4>().set([this] (int) { update_digits(); }); // credits tens strobe
	m_outlatch[0]->q_out_cb<5>().set([this] (int state) { machine().bookkeeping().coin_counter_w(0, state); });
	m_outlatch[0]->q_out_cb<6>().set([this] (int state) { machine().bookkeeping().coin_counter_w(1, state); });
	// Q7: unused

	LS259(config, m_outlatch[1]);
	m_outlatch[1]->q_out_cb<0>().set(FUNC(plsonic4_state::nmi_enable_w));
	// Q1: TODO, only set during the boot delay loops
	// Q2-Q5: video from game PCB 1-4
	// Q6: menu video (read by screen_update)
	// Q7: NVRAM write enable (read by nvram_w)

	LS259(config, m_outlatch[2]);
	// Q0/Q2/Q4/Q6: game PCB 4/3/2/1, only used by the test mode when timing a PCB's boot
	// Q1/Q3/Q5/Q7: game PCB 4/3/2/1, set together with the video relays when a game is played

	LS259(config, m_outlatch[3]);
	m_outlatch[3]->parallel_out_cb().set([this] (uint8_t) { update_digits(); }); // Q0-Q3: BCD bus, Q4-Q7: digit strobes

	// video hardware
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(20_MHz_XTAL / 4, 320, 0, 256, 262, 16, 240); // TODO: guessed, should give 60 Hz
	screen.set_screen_update(FUNC(plsonic4_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(plsonic4_state::vblank_w));

	PALETTE(config, "palette", FUNC(plsonic4_state::palette_init), 64 * 8);
	GFXDECODE(config, m_gfxdecode, "palette", gfx_plsonic4);
}


ROM_START( plsonic4 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "segasa_m-12_play_sonic_4_1.bin", 0x0000, 0x8000, CRC(f7fb2259) SHA1(4525ad6c38b12e5abf6f57ed16963a4ce48f3c5d) ) // second half is almost empty

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "segasa_m-12_play_sonic_4_2.bin", 0x0000, 0x2000, CRC(58b2b6a0) SHA1(6271f83a0c7858add286e4faaf5999916debcb70) )
	ROM_LOAD( "segasa_m-12_play_sonic_4_3.bin", 0x2000, 0x2000, CRC(d124045b) SHA1(a6e258582a80b411e718df87927a240ff9c59b2d) )
	ROM_LOAD( "segasa_m-12_play_sonic_4_4.bin", 0x4000, 0x2000, CRC(3db5dd0a) SHA1(c9c17a5c696f2ded8362fab2658913cca630665d) )

	ROM_REGION( 0xc00, "proms", 0 ) // 18-pin chips with scratched-off markings, type and size are guessed
	ROM_LOAD( "r.bin", 0x000, 0x400, NO_DUMP )
	ROM_LOAD( "g.bin", 0x400, 0x400, NO_DUMP )
	ROM_LOAD( "b.bin", 0x800, 0x400, NO_DUMP )
ROM_END

} // anonymous namespace


GAMEL( 1991, plsonic4, 0, plsonic4, plsonic4, plsonic4_state, empty_init, ROT0, "SegaSA / Sonic", "Play Sonic 4", MACHINE_NO_SOUND_HW | MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING, layout_plsonic4 )
