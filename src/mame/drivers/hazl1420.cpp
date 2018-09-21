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
