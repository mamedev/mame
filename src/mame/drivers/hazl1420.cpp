// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Hazeltine 1420 terminal.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mcs48/mcs48.h"
//#include "machine/i8243.h"
//#include "machine/ins8250.h"
//#include "video/dp8350.h"
#include "screen.h"

class hazl1420_state : public driver_device
{
public:
	hazl1420_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void hazl1420(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void prog_map(address_map &map);
	void io_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mcs48_cpu_device> m_maincpu;
	//required_device_array<i8243_device, 2> m_ioexp;
};

void hazl1420_state::prog_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("maincpu", 0);
	map(0x800, 0xfff).rom().region("maincpu_ea", 0x800);
}

void hazl1420_state::io_map(address_map &map)
{
}

void hazl1420_state::machine_start()
{
}

u32 hazl1420_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START(hazl1420)
	// DIP switches are on access panel above keyboard
	// "SW1" and "SW2" are not actual names

	PORT_START("SW1")
	PORT_DIPNAME(0x01, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x1c, 0x18, "Baud Rate") PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(0x00, "110")
	PORT_DIPSETTING(0x04, "300")
	PORT_DIPSETTING(0x08, "600")
	PORT_DIPSETTING(0x0c, "1200")
	PORT_DIPSETTING(0x10, "1800")
	PORT_DIPSETTING(0x14, "2400")
	PORT_DIPSETTING(0x18, "4800")
	PORT_DIPSETTING(0x1c, "9600")
	PORT_DIPNAME(0x20, 0x00, "Lead-In") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x00, "ESC")
	PORT_DIPSETTING(0x20, "~")
	PORT_DIPNAME(0xc0, 0xc0, "Parity") PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(0x00, "Odd")
	PORT_DIPSETTING(0x40, "Even")
	PORT_DIPSETTING(0x80, "1")
	PORT_DIPSETTING(0xc0, "0")

	PORT_START("SW2")
	PORT_DIPNAME(0x01, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Cursor") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x00, "Wraparound")
	PORT_DIPSETTING(0x02, "No Wrap")
	PORT_DIPNAME(0x04, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "Font") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x08, "Upper/Lower Case")
	PORT_DIPSETTING(0x00, "Upper Case Only")
	PORT_DIPNAME(0x10, 0x10, "Communication Mode") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00, "Half Duplex")
	PORT_DIPSETTING(0x10, "Full Duplex")
	PORT_DIPNAME(0x20, 0x20, "Automatic LF/CR") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x20, "Auto LF")
	PORT_DIPSETTING(0x00, "Carriage Return")
	PORT_DIPNAME(0x40, 0x40, "On Line") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

void hazl1420_state::hazl1420(machine_config &config)
{
	I8049(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hazl1420_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &hazl1420_state::io_map);

	//I8243(config, m_ioexp[0]);
	//I8243(config, m_ioexp[1]);

	//INS8250(config, "ace", 1'843'200);

	//DP8350(config, "crtc", 10.92_MHz_XTAL);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.92_MHz_XTAL, 700, 0, 560, 260, 0, 240);
	screen.set_screen_update(FUNC(hazl1420_state::screen_update));
}

ROM_START(hazl1420)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("8049h.u19", 0x0000, 0x0800, CRC(81beb6de) SHA1(f272d1277f100af92384a4c4cec2c9db9424b603))

	ROM_REGION(0x1000, "maincpu_ea", 0)
	ROM_LOAD("2716.u10", 0x0000, 0x0800, CRC(7c40ba24) SHA1(7575225adf1a06d66b079efcf0f4f9ee77fbddd4))
	ROM_LOAD("8316.u11", 0x0800, 0x0800, CRC(1c112f09) SHA1(fa4973e99c6d66809cffef009c4869787089a774))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("8316.u23", 0x0000, 0x0800, NO_DUMP)
ROM_END

COMP(1979, hazl1420, 0, 0, hazl1420, hazl1420, hazl1420_state, empty_init, "Hazeltine", "1420 Video Display Terminal", MACHINE_IS_SKELETON)
