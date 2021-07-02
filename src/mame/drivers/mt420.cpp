// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Micro-Term Model 420 terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/scn_pci.h"
#include "video/scn2674.h"
#include "screen.h"

namespace {

class mt420_state : public driver_device
{
public:
	mt420_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
	{
	}

	void mt420(machine_config &config);

private:
	u8 c000_r();
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void vram_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};

SCN2674_DRAW_CHARACTER_MEMBER(mt420_state::draw_character)
{
}


u8 mt420_state::c000_r()
{
	return machine().rand() & 0x80;
}

void mt420_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x9000, 0x9000).nopw();
	map(0xc000, 0xc000).r(FUNC(mt420_state::c000_r)).nopw();
	map(0xe000, 0xefff).ram();
	map(0xeff8, 0xefff).rw("avdc", FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0xf000, 0xf7ff).ram();
}

void mt420_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xef).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xf0, 0xf3).rw("aci", FUNC(scn2641_device::read), FUNC(scn2641_device::write));
}

void mt420_state::vram_map(address_map &map)
{
	map(0x0000, 0x3fff).noprw();
}

static INPUT_PORTS_START(mt420)
INPUT_PORTS_END

void mt420_state::mt420(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mt420_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mt420_state::io_map);

	INPUT_MERGER_ANY_HIGH(config, "mainint").output_handler().set_inputline(m_maincpu, 0);

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL)); // MC2681
	duart.irq_cb().set("mainint", FUNC(input_merger_device::in_w<0>));
	duart.outport_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(5);
	duart.outport_cb().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	duart.outport_cb().append("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(3);

	scn2641_device &aci(SCN2641(config, "aci", 3.6864_MHz_XTAL));
	aci.intr_handler().set("mainint", FUNC(input_merger_device::in_w<1>));

	EEPROM_93C46_16BIT(config, "eeprom").do_callback().set("duart", FUNC(scn2681_device::ip6_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(9.87768_MHz_XTAL, 612, 0, 480, 269, 0, 250);
	//screen.set_raw(15.30072_MHz_XTAL, 948, 0, 792, 269, 0, 250);
	screen.set_screen_update("avdc", FUNC(scn2674_device::screen_update));

	scn2674_device &avdc(SCN2674(config, "avdc", 9.87768_MHz_XTAL / 6));
	//avdc.set_clock(15.30072_MHz_XTAL / 6);
	avdc.set_character_width(6);
	avdc.set_display_callback(FUNC(mt420_state::draw_character));
	avdc.set_addrmap(0, &mt420_state::vram_map);
	avdc.set_screen("screen");
}


/**************************************************************************************************************

Micro-Term Model 420.
Chips: Z80, MC2681P, SCN2674, 2x CDM6264E3, TMM2016BP-12, SCN2641, NMC9345N. Undumped PAL10L8NC at U18 and PROM (N82S129N) at U41.
Crystals: 3.6864, 15.30072 (hard to read), 9.87768

***************************************************************************************************************/

ROM_START(mt420)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "1910_m.p._r1.9.u8",   0x0000, 0x8000, CRC(e79154e9) SHA1(7c3f22097b931986c921bf731de98a1d0536aec9) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "mt420cg_rev2.1.u44",  0x0000, 0x0fe0, CRC(7950e485) SHA1(1f03525958464bbe861d2e78f07cc5264e17c0e8) ) // incomplete?
ROM_END

} // anonymous namespace

COMP(1986, mt420, 0, 0, mt420, mt420, mt420_state, empty_init, "Micro-Term", "Micro-Term 420", MACHINE_IS_SKELETON)
