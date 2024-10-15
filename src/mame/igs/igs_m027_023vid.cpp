// license:BSD-3-Clause
// copyright-holders:

/*
IGS ARM7 (IGS027A) based mahjong / gambling platform(s),
with IGS customs normally used on PGM hardware.
Contrary to proper PGM hardware, these don't have a M68K.

Main components for the PCB-0457-03-GS are:
- IGS 027A (ARM7-based MCU)
- 33 MHz XTAL
- IGS 023 graphics chip
- ICS2115V Wavefront sound chip
- IGS 026B I/O chip
- 3 banks of 8 DIP switches

*/

#include "emu.h"

#include "igs027a.h"
#include "mahjong.h"
#include "pgmcrypt.h"

#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"

#include "sound/ics2115.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "endianness.h"

#include <algorithm>


namespace {

class igs_m027_023vid_state : public driver_device
{
public:
	igs_m027_023vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_external_rom(*this, "user1"),
		m_nvram(*this, "nvram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void m027_023vid(machine_config &config) ATTR_COLD;

	void init_mxsqy() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_region_ptr<u32> m_external_rom;
	required_shared_ptr<u32> m_nvram;

	required_device<igs027a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	u32 m_xor_table[0x100];

	u32 external_rom_r(offs_t offset);

	void xor_table_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void m027_map(address_map &map) ATTR_COLD;
};

void igs_m027_023vid_state::machine_start()
{
	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);

	save_item(NAME(m_xor_table));
}


u32 igs_m027_023vid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027_023vid_state::m027_map(address_map &map)
{
	map(0x0800'0000, 0x0807'ffff).r(FUNC(igs_m027_023vid_state::external_rom_r)); // Game ROM

	map(0x1800'0000, 0x1800'7fff).ram().mirror(0x0000f'8000).share(m_nvram);

	map(0x5000'0000, 0x5000'03ff).umask32(0x0000'00ff).w(FUNC(igs_m027_023vid_state::xor_table_w)); // uploads XOR table to external ROM here
}


/***************************************************************************

    Input Ports

***************************************************************************/

INPUT_PORTS_START( base )
	PORT_START("CLEARMEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("PORTB") // buttons?
	PORT_DIPNAME( 0x01, 0x01, "PORTB")
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

	PORT_START("PORTC") // buttons?
	PORT_DIPNAME( 0x01, 0x01, "PORTC")
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
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/


u32 igs_m027_023vid_state::external_rom_r(offs_t offset)
{
	return m_external_rom[offset] ^ m_xor_table[offset & 0x00ff];
}


void igs_m027_023vid_state::xor_table_w(offs_t offset, u8 data)
{
	m_xor_table[offset] = (u32(data) << 24) | (u32(data) << 8);
}


void igs_m027_023vid_state::m027_023vid(machine_config &config)
{
	IGS027A(config, m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027_023vid_state::m027_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(igs_m027_023vid_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::BLACK).set_format(palette_device::xRGB_555, 0x1200/2);

	// PGM video

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ICS2115(config, "ics", 33_MHz_XTAL);
}


/***************************************************************************

    ROMs Loading

***************************************************************************/


ROM_START( mxsqy )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "a8_027a.u41", 0x00000, 0x4000, CRC(f9ada8c4) SHA1(0715fdc3d15ae2d1af4e9c7d25f6410ae7c22d42) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "igs_m2401.u39", 0x000000, 0x80000, CRC(32e69540) SHA1(e5bc44700ba965fae433c3b39afa95bc753e3f2a) )

	ROM_REGION( 0x80000, "tiles",  0 )
	ROM_LOAD( "igs_l2405.u38", 0x00000, 0x80000, CRC(2f20eade) SHA1(aa11d26cb51483af5fdd4b181dff0f222baeaaff) )

	ROM_REGION16_LE( 0x400000, "sprcol", 0 )
	ROM_LOAD( "igs_l2404.u23", 0x000000, 0x400000, CRC(dc8ff7ae) SHA1(4609b5543d8bea7a8dea4e744f81c407688a96ee) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION16_LE( 0x400000, "sprmask", 0 )
	ROM_LOAD( "igs_m2403.u22", 0x000000, 0x400000, CRC(53940332) SHA1(3c703cbdc51dfb100f3ce10452a81091305dee01) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "igs_s2402.u21", 0x000000, 0x400000, CRC(a3e3b2e0) SHA1(906e5839ab62e570d9716e01b49e5b067e041269) )
ROM_END


void igs_m027_023vid_state::init_mxsqy()
{
	luckycrs_decrypt(machine());
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 200?, mxsqy, 0, m027_023vid, base, igs_m027_023vid_state, init_mxsqy, ROT0, "IGS", "Ming Xing San Que Yi (China)", MACHINE_IS_SKELETON )
