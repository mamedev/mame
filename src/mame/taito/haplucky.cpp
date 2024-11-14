// license:BSD-3-Clause
// copyright-holders:

/*
    Happy Lucky! (c) 1985 Taito
    Video lottery machine (as the titles screen says)

    J1100059A K1100139A CPU BOARD
    JII00060A K1100140A VIDEO BOARD

    Main components (CPU BOARD):

    2 x Z8400APS
    1 x DQ52B13H-250 (2 KB parallel EEPROM, blank on this PCB)
    1 x PC010SA
    1 x M5L8255AP-5
    1 x YM2149F
    1 x 8 MHz XTAL
    1 x 8-DIP switch bank

    Main components (VIDEO BOARD):
    1 x 24 MHz XTAL
    lots of TTLs and RAMs

    Possibly missing a ROM board.
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class haplucky_state : public driver_device
{
public:
	haplucky_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void haplucky(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint8_t m_sound_data = 0;
	uint8_t m_sound_status = 0x0f;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};


uint32_t haplucky_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void haplucky_state::machine_start()
{
	save_item(NAME(m_sound_data));
	save_item(NAME(m_sound_status));
}

void haplucky_state::main_map(address_map &map) // TODO: verify everything
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0); // TODO: There's only a 0x4000 ROM, but calls past it. Missing ROM board?
	map(0xe000, 0xe7ff).ram();
	map(0xf800, 0xffff).ram();
}

void haplucky_state::sound_map(address_map &map) // TODO: verify everything, but program is almost identical to the Super Dead Heat sub CPU one
{
	map(0x0000, 0xdfff).rom().region("audiocpu", 0);
	map(0x8000, 0x8000).w("dac", FUNC(dac_byte_interface::write));
	map(0xf800, 0xffff).ram();
}

void haplucky_state::sound_io_map(address_map &map) // TODO: verify everything, but program is almost identical to the Super Dead Heat sub CPU one
{
	map.global_mask(0xff);
	map(0xff, 0xff).lr8(NAME([this] () -> uint8_t { return m_sound_data; })).lw8(NAME([this] (uint8_t data) { m_sound_status = 0; }));
}

static INPUT_PORTS_START( haplucky )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


void haplucky_state::haplucky(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &haplucky_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(haplucky_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 8_MHz_XTAL / 2)); // divider not verified
	audiocpu.set_addrmap(AS_PROGRAM, &haplucky_state::sound_map);
	audiocpu.set_addrmap(AS_IO, &haplucky_state::sound_io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything to be verified
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(haplucky_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x800); // TODO: entries to be verified

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &ymsnd(YM2149(config, "ymsnd", 8_MHz_XTAL / 8)); // divider not verified
	ymsnd.port_a_read_callback().set([this] () { logerror("%s: PA read\n", machine().describe_context()); return u8(0xff); });
	ymsnd.port_a_write_callback().set([this] (u8 data) { logerror("%s: PA write %02X\n", machine().describe_context(), data); });
	ymsnd.port_b_read_callback().set([this] () { logerror("%s: PB read\n", machine().describe_context()); return u8(0xff); });
	ymsnd.port_b_write_callback().set([this] (u8 data) { logerror("%s: PB write %02X\n", machine().describe_context(), data); });
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.5); // divider not verified

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.5); // DAC type not verified
}


ROM_START( haplucky )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a58-01-1.ic24", 0x0000, 0x4000, CRC(6bddc224) SHA1(fb3783b4b97c1f9b454e5b8301897d8cfcc0b7b7) ) // MBM27128-25 on CPU board
	ROM_FILL( 0x400f, 0x01, 0xc9 ) // code jumps here early, return for now

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a58-02.ic5", 0x0000, 0x8000, CRC(cfffcffb) SHA1(4808930f6b8cc105d39ffef1525defbc25bd43f4) ) // MBM27256-25 on CPU board
	ROM_LOAD( "a58-03.ic6", 0x8000, 0x8000, CRC(470887e4) SHA1(69d9f907816c8c1b39b7c432b4196d04fcd7e365) ) // MBM27256-25 on CPU board

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "a40-09.ic26", 0x000, 0x104, CRC(733d1242) SHA1(3f99940f0c49023cbbd308eacfb5b102a52a68d2) ) // PAL16L8ACN, on video board
	ROM_LOAD( "a58-04.ic13", 0x200, 0x104, CRC(cbbe4e50) SHA1(a7b8e23f7d34b0f4b462cffc47a0c0a3d5611d31) ) // PAL16L8ACN, on CPU board
ROM_END

} // anonymous namespace


GAME( 1985, haplucky, 0, haplucky, haplucky, haplucky_state, empty_init, ROT0, "Taito Corporation", "Happy Lucky!", MACHINE_IS_SKELETON )
