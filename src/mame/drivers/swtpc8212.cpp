// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Southwest Technical Products video terminal.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/ins8250.h"
#include "video/mc6845.h"
#include "screen.h"


class swtpc8212_state : public driver_device
{
public:
	swtpc8212_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
	{
	}

	void swtpc8212(machine_config &mconfig);

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};

MC6845_UPDATE_ROW(swtpc8212_state::update_row)
{
}


void swtpc8212_state::mem_map(address_map &map)
{
	map(0x0000, 0x007f).ram();
	map(0x0080, 0x0083).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0088, 0x0088).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0089, 0x0089).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0090, 0x0097).rw("uart", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x0098, 0x009b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4800, 0x4fff).ram();
	map(0xb800, 0xbfff).rom().region("program", 0);
	map(0xc000, 0xc7ff).mirror(0x3800).rom().region("program", 0x800);
}


static INPUT_PORTS_START(swtpc8212)
	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x00, DEF_STR(Unknown))
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

void swtpc8212_state::swtpc8212(machine_config &config)
{
	M6802(config, m_maincpu, 1.8432_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &swtpc8212_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);

	pia6821_device &pia0(PIA6821(config, "pia0"));
	pia0.irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	pia0.irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	PIA6821(config, "pia1");

	ins8250_device &uart(INS8250(config, "uart", 1.8432_MHz_XTAL));
	uart.out_int_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(17.0748_MHz_XTAL, 918, 0, 738, 310, 0, 280);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 17.0748_MHz_XTAL / 9));
	crtc.set_char_width(9);
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_update_row_callback(FUNC(swtpc8212_state::update_row));
}


ROM_START( swtpc8212 ) // MC6802P, 2xMC6821P, INS8250N, MCM66750, MC6845P, bank of 8 dips, crystals 17.0748 (video), 1.8432 (cpu/uart). On the back is a 25-pin RS-232 port, and a 25-pin printer port.
	ROM_REGION( 0x1000, "program", 0 )
	ROM_LOAD( "8224g_ver.1.1_6oct80.ic1", 0x0000, 0x0800, CRC(7d7f3c21) SHA1(f7e6e20b36a1c724a4e348bc784d0b7b5fb462a3) )
	ROM_LOAD( "8224g_ver.1.1_6oct80.ic2", 0x0800, 0x0800, CRC(2b118c22) SHA1(5fa031c834c7c582d5715764941499fcef51f477) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "grafix_8x12_22aug80.bin",  0x0000, 0x0800, CRC(a525ed65) SHA1(813d2e85ddb258c5b032b959e695ad33200cbcc4) )
ROM_END

COMP(1980, swtpc8212, 0, 0, swtpc8212, swtpc8212, swtpc8212_state, empty_init, "Southwest Technical Products", "SWTPC 8212 Video Terminal", MACHINE_IS_SKELETON)
