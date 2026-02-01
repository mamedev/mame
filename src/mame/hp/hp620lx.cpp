// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Skeleton driver for Hewlett-Packard 620LX palmtop.

***************************************************************************/

#include "emu.h"

#include "cpu/sh/sh4.h"

#include "emupal.h"
#include "screen.h"


namespace {

class hp620lx_state : public driver_device
{
public:
	hp620lx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void hp620lx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD { }

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	void hp620lx_map(address_map &map) ATTR_COLD;
	void hp620lx_palette(palette_device &palette) const ATTR_COLD { }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
};

void hp620lx_state::hp620lx_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom().region("mask_rom_1_2", 0);
}

void hp620lx_state::hp620lx(machine_config &config)
{
	SH7709(config, m_maincpu, 75_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp620lx_state::hp620lx_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 240);
	m_screen->set_visarea(0, 640 - 1, 0, 240 - 1);
	m_screen->set_screen_update(FUNC(hp620lx_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(hp620lx_state::hp620lx_palette), 256);
}

// Model: F1250A
// PCB markings: F1250-80071 Rev 1
// Dumped as MX23L6410, removed second half of each ROM (pin A21 is not connected).
ROM_START( hp620lx )
	ROM_REGION64_LE(0x800000, "mask_rom_1_2", 0)
	ROM_LOAD32_WORD("1818-7457.q05h0403-abj.lhme5bwv.u1", 0x000000, 0x400000, CRC(304857e8) SHA1(907edcf71df4a8629d77b6a0df3e49cc3c581d66))
	ROM_LOAD32_WORD("1818-7458.q05h0403-abj.lhme5bww.u2", 0x000002, 0x400000, CRC(8a871962) SHA1(bee0d75a388a17f5fad615c69314766c7cebc791))

	ROM_REGION64_LE(0x800000, "mask_rom_3_4", 0)
	ROM_LOAD32_WORD("1818-7459.q05h0403-abj.lhme5bwy.u3", 0x000000, 0x400000, CRC(e4e7484e) SHA1(352bd4077ef60d64dfd62f4c4ecb6f77f2fbe983))
	ROM_LOAD32_WORD("1818-7460.q05h0403-abj.lhme5bwz.u4", 0x000002, 0x400000, CRC(da2d3323) SHA1(a59e1188d1ddcb7758ed06ed9fad3fd077aa89c5))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS          INIT         COMPANY             FULLNAME    FLAGS
COMP( 1998, hp620lx, 0,       0,      hp620lx, 0,     hp620lx_state, empty_init,  "Hewlett-Packard",  "HP 620LX", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
