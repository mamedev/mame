// license:BSD-3-Clause
// copyright-holder:FelipeSanches
//
// Control ID x628
//
// This is a fingerprint reader device
//
// TODO: Emulate the other CPU board (supposedly runs the Linux kernel on
//       a dedicated SoC targeting image-processing typically used on
//       fingerprint readers). The SoC is labelled ZKSoftware ZK6001
//       and someone online suggested it may be a rebranded device from
//       Ingenic, so likely a MIPS32 or MIPS64.
//
//       It has a 32Mb flash-rom and an ethernet controller
//       (model is Realtek RTL8201BL)
//
//       While the 8051 board has a tiny buzzer (and a battery-backed RAM)
//       the other PCB interfaces with an audio speaker.
//
//       There's also an RJ45 connector (ethernet port) as well as a
//       DB9 connector which seems to be used for a serial interface.
//
//       Finally, there are 2 LEDs, a 128x64 LCD with blueish backlight
//       and a keypad with the following layout:
//
//       1  2  3   ESC
//       4  5  6   MENU
//       7  8  9   UP-ARROW
//       .  0  OK  DOWN-ARROW

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/nt7534.h"
#include "emupal.h"
#include "screen.h"


namespace {

class controlidx628_state : public driver_device
{
public:
	controlidx628_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_lcdc(*this, "nt7534")
	{ }

	void controlidx628(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void p0_w(uint8_t data);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();
	uint8_t p3_r();
	void p3_w(uint8_t data);
	void controlidx628_palette(palette_device &palette) const;

	void io_map(address_map &map) ATTR_COLD;

	required_device<nt7534_device> m_lcdc;

	uint8_t m_p0_data = 0xff;
	uint8_t m_p1_data = 0xff;
	uint8_t m_p3_data = 0xff;
};


void controlidx628_state::machine_start()
{
	m_p0_data = 0xff;
	m_p1_data = 0xff;
	m_p3_data = 0xff;

	save_item(NAME(m_p0_data));
	save_item(NAME(m_p1_data));
	save_item(NAME(m_p3_data));
}

/*************************
* Memory map information *
*************************/

void controlidx628_state::io_map(address_map &map)
{
	map(0x8000, 0xffff).ram();
}


void controlidx628_state::p0_w(uint8_t data)
{
	m_p0_data = data;
}

uint8_t controlidx628_state::p1_r()
{
	// P1.1 is used for serial I/O; P1.4 and P1.5 are also used bidirectionally
	return 0xcd;
}

void controlidx628_state::p1_w(uint8_t data)
{
	if ((BIT(m_p1_data, 6) == 0) && (BIT(data, 6) == 1)) // on raising-edge of bit 6
	{
		m_lcdc->write(BIT(data, 7), m_p0_data);
	}
	// P1.0 is also used as a serial I/O clock
	m_p1_data = data;
}

uint8_t controlidx628_state::p2_r()
{
	// Low nibble used for input
	return 0xf0;
}

uint8_t controlidx628_state::p3_r()
{
	// P3.3 (INT1) and P3.4 (T0) used bidirectionally
	return 0xff;
}

void controlidx628_state::p3_w(uint8_t data)
{
	m_p3_data = data;
}

/*************************
*      Input ports       *
*************************/

//static INPUT_PORTS_START( controlidx628 )
//INPUT_PORTS_END

void controlidx628_state::controlidx628_palette(palette_device &palette) const
{
	// These colors were selected from a photo of the display
	// using the color-picker in Inkscape:
	palette.set_pen_color(0, rgb_t(0x06, 0x61, 0xee));
	palette.set_pen_color(1, rgb_t(0x00, 0x23, 0x84));
}

/*************************
*     Machine Driver     *
*************************/

void controlidx628_state::controlidx628(machine_config &config)
{
	// basic machine hardware
	at89s52_device &maincpu(AT89S52(config, "maincpu", XTAL(11'059'200)));
	maincpu.set_addrmap(AS_IO, &controlidx628_state::io_map);
	maincpu.port_out_cb<0>().set(FUNC(controlidx628_state::p0_w));
	maincpu.port_in_cb<1>().set(FUNC(controlidx628_state::p1_r));
	maincpu.port_out_cb<1>().set(FUNC(controlidx628_state::p1_w));
	maincpu.port_in_cb<2>().set(FUNC(controlidx628_state::p2_r));
	maincpu.port_in_cb<3>().set(FUNC(controlidx628_state::p3_r));
	maincpu.port_out_cb<3>().set(FUNC(controlidx628_state::p3_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(132, 65);
	screen.set_visarea(3, 130, 0, 63);
	screen.set_screen_update("nt7534", FUNC(nt7534_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(controlidx628_state::controlidx628_palette), 2);

	NT7534(config, m_lcdc);
}


/*************************
*        Rom Load        *
*************************/

ROM_START( cidx628 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "controlid_x628.u1",   0x0000, 0x2000, CRC(500d79b4) SHA1(5522115f2da622db389e067fcdd4bccb7aa8561a) )
ROM_END

} // anonymous namespace


COMP(200?, cidx628, 0, 0, controlidx628, 0, controlidx628_state, empty_init, "ControlID", "X628", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
