// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for ADDS 2020 terminal.

    The PCB has a 68-pin PLCC ASIC at the top right corner, next to the 36.000 (Y4), 24.0000 (Y5), 4.9152 MHz (Y6) XTALs:

        ©NCR 1986
        006-0130001 PT
        200-87700
        M823270 8701A

    This presumably does the video encoding, and possibly also contains the character graphics (not present in any dumped ROMs).

***********************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/scn_pci.h"
#include "video/scn2674.h"
#include "screen.h"

namespace {

class adds2020_state : public driver_device
{
public:
	adds2020_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_epci(*this, "epci")
		, m_avdc(*this, "avdc")
		, m_ram(*this, "nvram")
	{
	}

	void adds2020(machine_config &config);

private:
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	u8 char_r(offs_t offset);

	void unknown_0000_w(u8 data);
	void unknown_9800_w(offs_t offset, u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<scn_pci_device> m_epci;
	required_device<scn2674_device> m_avdc;
	required_shared_ptr<u8> m_ram;
};

SCN2674_DRAW_CHARACTER_MEMBER(adds2020_state::draw_character)
{
}

u8 adds2020_state::char_r(offs_t offset)
{
	return m_ram[offset & 0x1fff];
}

void adds2020_state::unknown_0000_w(u8 data)
{
	logerror("%s: Writing %02Xh to 0000h\n", machine().describe_context(), data);
}

void adds2020_state::unknown_9800_w(offs_t offset, u8 data)
{
	logerror("%s: Writing %02Xh to %04Xh\n", machine().describe_context(), data, offset + 0x9800);
}

void adds2020_state::prog_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("firmware", 0);
}

void adds2020_state::ext_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(adds2020_state::unknown_0000_w));
	map(0x4000, 0x5fff).ram().share("nvram");
	map(0x6000, 0x67ff).mirror(0x1800).ram(); // code suggests one HW variant may have a full 8K here
	map(0x8000, 0x8000).select(6).lr8([this](offs_t offset) { return m_epci->read(offset >> 1); }, "epci_r");
	map(0x8001, 0x8001).select(6).lw8([this](offs_t offset, u8 data) { m_epci->write(offset >> 1, data); }, "epci_w");
	map(0x8800, 0x8807).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x9800, 0x9801).w(FUNC(adds2020_state::unknown_9800_w));
}

void adds2020_state::char_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x7ff).ram();
}

static INPUT_PORTS_START(adds2020)
INPUT_PORTS_END

void adds2020_state::adds2020(machine_config &config)
{
	I8031(config, m_maincpu, 10.92_MHz_XTAL); // P8031AH
	m_maincpu->set_addrmap(AS_PROGRAM, &adds2020_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &adds2020_state::ext_map);

	INPUT_MERGER_ANY_HIGH(config, "mainint").output_handler().set_inputline(m_maincpu, MCS51_INT1_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1); // NEC D4464C-15L + 3.5V battery (only first 3K is actually saved)

	SCN2661B(config, m_epci, 4.9152_MHz_XTAL);
	m_epci->rxrdy_handler().set("mainint", FUNC(input_merger_device::in_w<0>));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	//screen.set_raw(24_MHz_XTAL, 960, 0, 800, 361, 0, 338);
	screen.set_raw(36_MHz_XTAL, 1440, 0, 1188, 361, 0, 338);
	screen.set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	SCN2674(config, m_avdc, 36_MHz_XTAL / 9); // SCN2674B
	m_avdc->set_character_width(9); // cell width guessed (10 in 80-column mode?)
	m_avdc->set_screen("screen");
	m_avdc->set_addrmap(0, &adds2020_state::char_map);
	m_avdc->set_display_callback(FUNC(adds2020_state::draw_character));
	m_avdc->mbc_char_callback().set(FUNC(adds2020_state::char_r));
	m_avdc->breq_callback().set_inputline(m_maincpu, MCS51_T0_LINE);
	m_avdc->intr_callback().set("mainint", FUNC(input_merger_device::in_w<1>));

	// TODO: speaker
}

ROM_START(adds2020)
	ROM_REGION(0xc000, "firmware", ROMREGION_ERASEFF)
	ROM_LOAD("2020_v2.5_ua5.bin", 0x0000, 0x4000, CRC(2d9e3397) SHA1(9aec8fdfe064618c20b4ba85dd8b71a44e29bbd3))
	ROM_LOAD("2020_v2.5_ua6.bin", 0x4000, 0x2000, CRC(f4cbda6f) SHA1(efe03a15d7eab4038de1e8118c6891d2078166fd))
	ROM_LOAD("2020_v2.5_ua8.bin", 0x8000, 0x4000, CRC(4a8ac850) SHA1(ce02a0e904ed07930a891360f1e5779fbee8eaac))
ROM_END

} // anonymous namespace

COMP(1986, adds2020, 0, 0, adds2020, adds2020, adds2020_state, empty_init, "Applied Digital Data Systems", "ADDS 2020", MACHINE_IS_SKELETON)
