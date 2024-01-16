// license:BSD-3-Clause
// copyright-holders:David Haywood

// P/ECE

#include "emu.h"

#include "cpu/s1c33/s1c33.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class aquaplus_piece_state : public driver_device
{
public:
	aquaplus_piece_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void aquaplus_piece(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<s1c33_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

void aquaplus_piece_state::machine_start()
{
}

void aquaplus_piece_state::machine_reset()
{
}

static INPUT_PORTS_START( aquaplus_piece )
INPUT_PORTS_END

void aquaplus_piece_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
}

u32 aquaplus_piece_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void aquaplus_piece_state::aquaplus_piece(machine_config &config)
{
	S1C33(config, m_maincpu, 24000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &aquaplus_piece_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128, 88);
	screen.set_visarea(0, 128-1, 0, 88-1);
	screen.set_screen_update(FUNC(aquaplus_piece_state::screen_update));

	PALETTE(config, m_palette).set_entries(4);

	SPEAKER(config, "speaker").front_center();
}

ROM_START( piece )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	// these are from the update package, it is unclear if anything needs to be added to them
	// in order to boot
	// they are also not full ROM size, do they map at 0, or does something else live there?
	ROM_SYSTEM_BIOS( 0, "v120_2mb", "Version 1.20 (2MB)" )
	ROMX_LOAD("120_all_2mb.bin", 0x00000, 0x7e000, CRC(f9dbea00) SHA1(112a9dbd01ebef485a164b0020c3677b9f8bbc83), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v120", "Version 1.20" )
	ROMX_LOAD("120_all.bin",     0x00000, 0x7e000, CRC(e9c5769e) SHA1(b01f17b3b027a96e6421c40b758b3ed61cbb9997), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v118", "Version 1.18" )
	ROMX_LOAD("118_all.bin",     0x00000, 0x7e000, CRC(235978fb) SHA1(4116ef2876f3e5c042b51a358a0bb870679bfeb7), ROM_BIOS(2) )
ROM_END

} // anonymous namespace


CONS( 2001, piece,      0,       0,      aquaplus_piece, aquaplus_piece, aquaplus_piece_state, empty_init, "Aquaplus", "P/ECE", MACHINE_IS_SKELETON )
