// license:BSD-3-Clause
// copyright-holders:

/*
bootleg handheld with standard NES game library
probably emulation based as the games have a brief animated loading graphic
and the game data appears to be compressed in the ROM

SoC / CPU is an unmarked 48-pin chip, architecture unknown, there is what looks like native code
at 0x4000

has T1FF signature at the start of the ROM
 
other similar devices are S+Core, but the code in the ROM doesn't appear to disassemble as such
*/

#include "emu.h"

#include "screen.h"
#include "speaker.h"


namespace {

class gc71_nes_bootleg_state : public driver_device
{
public:
	gc71_nes_bootleg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen")
	{
	}

	void gc71hh(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
};

u32 gc71_nes_bootleg_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gc71_nes_bootleg_state::machine_start()
{
}

static INPUT_PORTS_START(gc71hh)
INPUT_PORTS_END


void gc71_nes_bootleg_state::gc71hh(machine_config &config)
{
	// unknown CPU

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(FUNC(gc71_nes_bootleg_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( gc71hh )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "p25q64sh.u6", 0x000000, 0x800000, CRC(465ccc0a) SHA1(b2dd18fbad05f9011f53c54173c8956771bf616f) )
ROM_END

} // anonymous namespace

// GC71D-TYPE-C-250819-V3 on PCB, no manufacturer information present on box or unit
GAME( 2025, gc71hh, 0, gc71hh, gc71hh, gc71_nes_bootleg_state, empty_init, ROT0, "<unknown>", "Mini Handled Game Console GC71", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
