// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Picture Magic (codename JANUS) */
// http://segaretro.org/Sega_Picture_Magic

// this uses a Sega 32X PCB (not in a 32X case) attached to a stripped down 68k based board rather than a full Genesis / Megadrive
// it is likely the internal SH2 bios roms differ


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "emupal.h"
#include "screen.h"


namespace {

class segapm_state : public driver_device
{
public:
	segapm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void segapm(machine_config &config);

private:
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_segapm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void segapm_map(address_map &map) ATTR_COLD;
};


void segapm_state::video_start()
{
}

uint32_t segapm_state::screen_update_segapm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



void segapm_state::segapm_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	// A15100

	map(0xe00000, 0xe7ffff).ram();

}

static INPUT_PORTS_START( segapm )
INPUT_PORTS_END




void segapm_state::segapm(machine_config &config)
{
	M68000(config, m_maincpu, 8000000); // ??
	m_maincpu->set_addrmap(AS_PROGRAM, &segapm_state::segapm_map);

	// + 2 sh2s on 32x board

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(segapm_state::screen_update_segapm));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x200);
}



ROM_START( segapm ) // was more than one cartridge available? if so softlist them?
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "picture magic boot cart.bin", 0x00000, 0x80000, CRC(c9ab4e60) SHA1(9c4d4ab3e59c8acde86049a1ba3787aa03b549a3) ) // internal header is GOUSEI HENSYUU

	// todo, sh2 bios roms etc.
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME         FLAGS
CONS( 1996, segapm, 0,      0,      segapm,  segapm, segapm_state, empty_init, "Sega",  "Picture Magic", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
