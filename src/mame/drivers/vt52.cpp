// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for DEC VT50 terminal family.

    The VT50 "DECscope" was DEC's first video terminal to contain a CPU of
    sorts, with TTL logic spanning two boards executing custom microcode.
    It displayed 12 lines of 80-column text, using a standard character
    generator that only contained uppercase letters and symbols.

    The VT52 used the same case and most of the same circuitry as the VT50,
    but quickly displaced it by supporting 24 lines of text and a full ASCII
    character generator (on a board of its own). VT50 and VT52 each had
    minor variants differing in keyboard function and printer availability.

    The VT55 was a graphical terminal ostensibly in the same family as the
    VT50 and VT52. Its hardware commonalities and differences are unknown.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/vt50/vt50.h"
#include "machine/ay31015.h"
#include "screen.h"

class vt52_state : public driver_device
{
public:
	vt52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
	{
	}

	void vt52(machine_config &mconfig);

protected:
	virtual void machine_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void rom_1k(address_map &map);
	void ram_2k(address_map &map);

	required_device<vt5x_cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
};

void vt52_state::machine_start()
{
	m_uart->write_swe(0);
}

u32 vt52_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vt52_state::rom_1k(address_map &map)
{
	map(00000, 01777).rom().region("program", 0);
}

void vt52_state::ram_2k(address_map &map)
{
	map(00000, 03777).ram();
}

static INPUT_PORTS_START(vt52)
INPUT_PORTS_END

void vt52_state::vt52(machine_config &mconfig)
{
	VT52_CPU(mconfig, m_maincpu, 13.824_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt52_state::rom_1k);
	m_maincpu->set_addrmap(AS_DATA, &vt52_state::ram_2k);

	AY51013(mconfig, m_uart); // TR1402 or equivalent

	screen_device &screen(SCREEN(mconfig, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(13.824_MHz_XTAL, 900, 0, 720, 256, 0, 192);
	screen.set_screen_update(FUNC(vt52_state::screen_update));
}

ROM_START(vt52)
	ROM_REGION(0x400, "program", 0) // bipolar PROMs
	ROM_LOAD_NIB_LOW( "23-124a9.e29", 0x000, 0x200, CRC(3f5f3b92) SHA1(244c3100f277da3fce5513a92529a2c3e26a80b4))
	ROM_LOAD_NIB_HIGH("23-125a9.e26", 0x000, 0x200, CRC(b2a670c9) SHA1(fa8dd031dcafe4facff41e79603bdb388a6df928))
	ROM_LOAD_NIB_LOW( "23-126a9.e37", 0x200, 0x200, CRC(4883a600) SHA1(c5d9b0c21493065c75b4a7d52d5bd47f9851dfe7))
	ROM_LOAD_NIB_HIGH("23-127a9.e21", 0x200, 0x200, CRC(56c1c0d6) SHA1(ab0eb6e7bbafcc3d28481b62de3d3490f01c0174))

	ROM_REGION(0x400, "chargen", 0) // 2608 character generator
	ROM_LOAD("23-002b4.e1", 0x000, 0x400, NO_DUMP)
ROM_END

COMP(1975, vt52, 0, 0, vt52, vt52, vt52_state, empty_init, "DEC", "VT52", MACHINE_IS_SKELETON)
