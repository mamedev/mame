// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Skeleton driver for Philips Velo 1 / 500 palmtops.

    Hardware
    --------

    Velo 1:

    - Linear Technology LTC1350CG (3-Driver/5-Receiver EIA/TIA-562 Transceiver)
    - Mitsubishi M5M4V16165CTP (2 * 2 MB DRAM)
    - Motorola S25C8Q-SPH
    - Philips PR31500ABC "Poseidon embedded processor" (MIPS R3000A CPU)
    - Samsung Electronics A4640S100 (2 * 4 MB Mask ROM with marking "SEC PMCG Velo1.0 / Copyright 1997 Philips Elec. and Microsoft Corp")
    - Miniature Card ROM / RAM slots
    - Type II PC Card V-Module slot

***************************************************************************/

#include "emu.h"

#include "cpu/mips/mips1.h"

#include "emupal.h"
#include "screen.h"


namespace {

class velo1_state : public driver_device
{
public:
	velo1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void velo1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD { }

private:
	required_device<mips1_device_base> m_maincpu;
	required_device<screen_device> m_screen;

	void prg_map(address_map &map) ATTR_COLD;
	void palette(palette_device &palette) const ATTR_COLD { }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
};

void velo1_state::prg_map(address_map &map)
{
	map(0x1f000000, 0x1f7fffff).rom().mirror(0x00800000).region("mask_rom_1_2", 0);
}

void velo1_state::velo1(machine_config &config)
{
	R3000A(config, m_maincpu, 40_MHz_XTAL, 4096, 1024);
	m_maincpu->set_addrmap(AS_PROGRAM, &velo1_state::prg_map);
	m_maincpu->set_endianness(ENDIANNESS_LITTLE);

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(480, 240);
	m_screen->set_visarea(0, 480 - 1, 0, 240 - 1);
	m_screen->set_screen_update(FUNC(velo1_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(velo1_state::palette), 4);
}

// Windows CE Version 1.0 (Build 713)
// Pin-compatible with MX23L6422
ROM_START( pvelo1 )
	ROM_REGION32_LE(0x800000, "mask_rom_1_2", 0)
	ROM_LOAD32_WORD_SWAP("pmcg_velo1.0_hi_16.a4640s100-001.bin", 0x00000, 0x400000, CRC(fa19fdda) SHA1(be1234d0959bd133b6c0fb77a8c5ab1114c083b4))
	ROM_LOAD32_WORD_SWAP("pmcg_velo1.0_low_16.a4640s100-002.bin", 0x000002, 0x400000, CRC(b1aeec75) SHA1(b78a02741f9804455eab6d68b734f5ad4299623c))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT  CLASS        INIT         COMPANY     FULLNAME  FLAGS
COMP( 1997, pvelo1, 0,       0,      velo1,   0,     velo1_state, empty_init,  "Philips",  "Velo 1", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
