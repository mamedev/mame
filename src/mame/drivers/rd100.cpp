// license:BSD-3-Clause
// copyright-holders:AJR
/********************************************************************************

    Data RD100

    Little is known about this system except for a few PCB pictures. No
    manuals, schematic or circuit description have been found.

    The RD100 was apparently sold in France under the "Superkit" brand. There
    appear to have been several versions. Earlier models had 7-segment LEDs
    and rudimentary keyboards. The model dumped here is apparently the K32K,
    which had a 16x2 character LCD display, a QWERTY keyboard and non-numeric
    keypad, Centronics and RS-232 ports, and an extension board for prototyping.

*********************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


class rd100_state : public driver_device
{
public:
	rd100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void rd100(machine_config &config);

private:
	DECLARE_MACHINE_RESET(rd100);
	HD44780_PIXEL_UPDATE(pixel_update);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};


void rd100_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	//map(0x8608, 0x860f).rw("timer", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x8640, 0x8643).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8680, 0x8683).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8700, 0x8701).rw("hd44780", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x8800, 0xffff).rom().region("roms", 0x800);
}

/* Input ports */
static INPUT_PORTS_START( rd100 )
INPUT_PORTS_END

HD44780_PIXEL_UPDATE(rd100_state::pixel_update)
{
	if (pos < 16)
		bitmap.pix16(line * 8 + y, pos * 6 + x) = state;
}

void rd100_state::rd100(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 4_MHz_XTAL); // MC6809P???
	m_maincpu->set_addrmap(AS_PROGRAM, &rd100_state::mem_map);

	PIA6821(config, "pia1");

	PIA6821(config, "pia2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 16);
	screen.set_visarea(0, 16*6-1, 0, 16-1);
	screen.set_palette("palette");

	hd44780_device &hd44780(HD44780(config, "hd44780"));
	hd44780.set_lcd_size(2, 16);
	hd44780.set_pixel_update_cb(FUNC(rd100_state::pixel_update), this);

	PALETTE(config, "palette").set_entries(2);
}

ROM_START( rd100 )
	ROM_REGION( 0x8000, "roms", 0 )
	ROM_LOAD( "pak3-01.bin",  0x0000, 0x8000, CRC(cf5bbf01) SHA1(0673f4048d700b84c30781af23fbeabe0b994306) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1989, rd100, 0,      0,      rd100,   rd100, rd100_state, empty_init, "Data R.D.", "RD100",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
