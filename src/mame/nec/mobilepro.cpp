// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Skeleton driver for NEC MobilePro palmtops.

    Hardware
    --------

    MobilePro 450:

    - Maxim MAX3241CAI (RS-232 Transceiver)
    - NEC 23C32040LGY (2 * 4 MB Mask ROM)
    - NEC D30101GM-33 VR4101 (MIPS R4000 CPU)
    - NEC D42S16165LG5-A70DB-7 (2 * 2 MB DRAM)
    - Ricoh RF5C296 (PC Card Controller)
    - Texas Instruments TLV1543C (A/D Converter)
    - PC Card slot
    - RS-232 port

***************************************************************************/

#include "emu.h"

#include "cpu/mips/mips3.h"

#include "emupal.h"
#include "screen.h"


namespace {

class mobilepro_state : public driver_device
{
public:
	mobilepro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void mobilepro(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD { }

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	void prg_map(address_map &map) ATTR_COLD;
	void palette(palette_device &palette) const ATTR_COLD { }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
};

void mobilepro_state::prg_map(address_map &map)
{
	map(0x1f000000, 0x1f7fffff).rom().mirror(0x00800000).region("mask_rom_1_2", 0);
}

void mobilepro_state::mobilepro(machine_config &config)
{
	R4000LE(config, m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mobilepro_state::prg_map);

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(480, 240);
	m_screen->set_visarea(0, 480 - 1, 0, 240 - 1);
	m_screen->set_screen_update(FUNC(mobilepro_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(mobilepro_state::palette), 4);
}

// Windows CE Version 1.0 (Build 210-510)
ROM_START( mbpro450 )
	ROM_REGION64_LE(0x800000, "mask_rom_1_2", 0)
	ROM_LOAD16_BYTE("23c32040lgy-808.b1a3", 0x00000, 0x400000, CRC(efedc65d) SHA1(177b67ff76394f97d1876d5897580f3fbd21bfdc))
	ROM_LOAD16_BYTE("23c32040lgy-809.b1a4", 0x00001, 0x400000, CRC(af2870ca) SHA1(f3c5f29e968f3a81243109a120ce0bb5a31302f0))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT  CLASS            INIT         COMPANY  FULLNAME         FLAGS
COMP( 1996, mbpro450, 0,      0,      mobilepro,  0,     mobilepro_state, empty_init,  "NEC",   "MobilePro 450", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
