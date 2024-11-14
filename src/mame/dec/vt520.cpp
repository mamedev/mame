// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

        DEC VT520

        02/07/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/mc68681.h"
#include "machine/ram.h"
#include "emupal.h"
#include "screen.h"


namespace {

class vt520_state : public driver_device
{
public:
	vt520_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "maincpu")
	{ }

	void vt520(machine_config &config);
	void vt420(machine_config &config);

private:
	uint8_t vt520_some_r();
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_vt520(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_region_ptr<uint8_t> m_rom;
	void vt520_io(address_map &map) ATTR_COLD;
	void vt520_mem(address_map &map) ATTR_COLD;
};


void vt520_state::vt520_mem(address_map &map)
{
	map(0x0000, 0xffff).bankrw("bank1");
}

/*
    On the board there is TC160G41AF (1222) custom chip
    doing probably all video/uart logic
    there is 43.320MHz xtal near by
*/

uint8_t vt520_state::vt520_some_r()
{
	//bit 5 0
	//bit 6 1
	return 0x40;
}

void vt520_state::vt520_io(address_map &map)
{
	map.unmap_value_high();
	map(0x7ffb, 0x7ffb).r(FUNC(vt520_state::vt520_some_r));
}

/* Input ports */
static INPUT_PORTS_START( vt520 )
INPUT_PORTS_END


void vt520_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.unmap_write(0x0000, 0xffff);
	membank("bank1")->set_base(&m_rom[m_rom.length() - 0x10000]);
}

void vt520_state::video_start()
{
}

uint32_t vt520_state::screen_update_vt520(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vt520_state::vt420(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, XTAL(43'320'000) / 3); // SCN8031HCFN40 (divider not verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &vt520_state::vt520_mem);
	m_maincpu->set_addrmap(AS_IO, &vt520_state::vt520_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(802, 480);
	screen.set_visarea(0, 802-1, 0, 480-1);
	screen.set_screen_update(FUNC(vt520_state::screen_update_vt520));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

void vt520_state::vt520(machine_config &config)
{
	/* basic machine hardware */
	I80C32(config, m_maincpu, XTAL(20'000'000)); // Philips P80C32IBPN
	m_maincpu->set_addrmap(AS_PROGRAM, &vt520_state::vt520_mem);
	m_maincpu->set_addrmap(AS_IO, &vt520_state::vt520_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(802, 480);
	screen.set_visarea(0, 802-1, 0, 480-1);
	screen.set_screen_update(FUNC(vt520_state::screen_update_vt520));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	// On the board there are two M5M44256BJ-7 chips
	// Which are DRAM 256K x 4bit
	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256K");
}

/**************************************************************************************************************

DEC VT420.
Chips: SCN8031HCFN40, SCN2681TC1N40, M5M4464AP, HM62256LFP-10T, TC23SC070AT, TC531001CP-1815
Crystals: 43.320

***************************************************************************************************************/

ROM_START( vt420 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v14", "Version 1.4")
	ROMX_LOAD( "23-202e9.e2",    0x00000, 0x20000, CRC(ca6cfb18) SHA1(2e0d3c16e04808bc6a45a0fc032b597458e6dd85), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v13", "Version 1.3")
	ROMX_LOAD( "23-068e9-00.e2", 0x00000, 0x20000, CRC(22c3f93b) SHA1(b212911c41e4dba2e09d91fdd1f72d6c7536b0af), ROM_BIOS(1) )
ROM_END

ROM_START( vt520 )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-010ed-00.e20", 0x0000, 0x80000, CRC(2502cc22) SHA1(0437c3107412f69e09d050fef003f2a81d8a3163)) // "(C)DEC94 23-010ED-00 // 9739 D" dumped from a VT520-A4 model
ROM_END

} // anonymous namespace


/* Driver */

COMP( 1990, vt420, 0, 0, vt420, vt520, vt520_state, empty_init, "Digital Equipment Corporation", "VT420 Video Terminal", MACHINE_IS_SKELETON )
//COMP( 1993, vt510, 0, 0, vt520, vt520, vt520_state, empty_init, "Digital Equipment Corporation", "VT510 Video Terminal",  MACHINE_IS_SKELETON)
COMP( 1994, vt520, 0, 0, vt520, vt520, vt520_state, empty_init, "Digital Equipment Corporation", "VT520 Video Terminal",  MACHINE_IS_SKELETON)
//COMP( 1994, vt525, 0, 0, vt520, vt520, vt520_state, empty_init, "Digital Equipment Corporation", "VT525 Video Terminal",  MACHINE_IS_SKELETON)
