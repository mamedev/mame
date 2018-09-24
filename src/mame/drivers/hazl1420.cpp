// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Hazeltine 1420 terminal.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "machine/i8243.h"
#include "machine/ins8250.h"
//#include "video/dp8350.h"
#include "screen.h"

class hazl1420_state : public driver_device
{
public:
	hazl1420_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_ioexp(*this, "ioexp%u", 0U)
		, m_screen(*this, "screen")
	{
	}

	void hazl1420(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void p1_w(u8 data);
	u8 p2_r();
	void p2_w(u8 data);

	void prog_map(address_map &map);
	void io_map(address_map &map);
	void bank_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mcs48_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device_array<i8243_device, 2> m_ioexp;
	required_device<screen_device> m_screen;
};

void hazl1420_state::p1_w(u8 data)
{
	m_ioexp[0]->cs_w((data & 0xc0) == 0x80 ? 0 : 1);
	m_ioexp[1]->cs_w((data & 0xc0) == 0xc0 ? 0 : 1);
}

u8 hazl1420_state::p2_r()
{
	u8 result = m_screen->vblank() ? 0xf0 : 0xe0;
	result |= m_ioexp[0]->p2_r() & m_ioexp[1]->p2_r();
	return result;
}

void hazl1420_state::p2_w(u8 data)
{
	m_bankdev->set_bank(data & 0x0f);
}

void hazl1420_state::prog_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("maincpu", 0);
}

void hazl1420_state::io_map(address_map &map)
{
	map(0x00, 0xff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
}

void hazl1420_state::bank_map(address_map &map)
{
	map(0x000, 0x7ff).ram().share("videoram");
	map(0x800, 0x807).mirror(0x10).rw("ace", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0xc48, 0xc48).ram();
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

	PORT_START("UNUSED")
	PORT_DIPNAME(0x1, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x1, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x2, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))

	PORT_START("INP4")
	PORT_DIPNAME(0x7, 0x6, "Baud Rate") PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(0x0, "110")
	PORT_DIPSETTING(0x1, "300")
	PORT_DIPSETTING(0x2, "600")
	PORT_DIPSETTING(0x3, "1200")
	PORT_DIPSETTING(0x4, "1800")
	PORT_DIPSETTING(0x5, "2400")
	PORT_DIPSETTING(0x6, "4800")
	PORT_DIPSETTING(0x7, "9600")
	PORT_DIPNAME(0x8, 0x0, "Lead-In") PORT_DIPLOCATION("SW1:6") // not verified
	PORT_DIPSETTING(0x0, "ESC")
	PORT_DIPSETTING(0x8, "~")

	PORT_START("INP5")
	PORT_DIPNAME(0x3, 0x3, "Parity") PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(0x0, "Odd")
	PORT_DIPSETTING(0x1, "Even")
	PORT_DIPSETTING(0x2, "1")
	PORT_DIPSETTING(0x3, "0")
	PORT_DIPNAME(0x4, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x4, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INP6")
	PORT_DIPNAME(0x1, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x1, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x2, 0x2, "On Line") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x4, 0x4, "Automatic LF/CR") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x4, "Auto LF")
	PORT_DIPSETTING(0x0, "Carriage Return")
	PORT_DIPNAME(0x8, 0x8, "Communication Mode") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x0, "Half Duplex")
	PORT_DIPSETTING(0x8, "Full Duplex")

	PORT_START("INP7")
	PORT_DIPNAME(0x1, 0x1, "Font") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x1, "Upper/Lower Case")
	PORT_DIPSETTING(0x0, "Upper Case Only")
	PORT_DIPNAME(0x2, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x4, 0x0, "Cursor") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x0, "Wraparound")
	PORT_DIPSETTING(0x4, "No Wrap")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

void hazl1420_state::hazl1420(machine_config &config)
{
	I8049(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hazl1420_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &hazl1420_state::io_map);
	m_maincpu->p1_out_cb().set(FUNC(hazl1420_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(hazl1420_state::p2_r));
	m_maincpu->p2_out_cb().set(FUNC(hazl1420_state::p2_w));
	m_maincpu->p2_out_cb().append(m_ioexp[0], FUNC(i8243_device::p2_w));
	m_maincpu->p2_out_cb().append(m_ioexp[1], FUNC(i8243_device::p2_w));
	m_maincpu->prog_out_cb().set(m_ioexp[0], FUNC(i8243_device::prog_w));
	m_maincpu->prog_out_cb().append(m_ioexp[1], FUNC(i8243_device::prog_w));
	m_maincpu->t1_in_cb().set("ace", FUNC(ins8250_device::intrpt_r));

	ADDRESS_MAP_BANK(config, m_bankdev);
	m_bankdev->set_addrmap(0, &hazl1420_state::bank_map);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(12);
	m_bankdev->set_stride(0x100);

	I8243(config, m_ioexp[0]);
	m_ioexp[0]->p4_in_cb().set_ioport("INP4");
	m_ioexp[0]->p5_in_cb().set_ioport("INP5");

	I8243(config, m_ioexp[1]);
	m_ioexp[1]->p6_in_cb().set_ioport("INP6");
	m_ioexp[1]->p7_in_cb().set_ioport("INP7");

	INS8250(config, "ace", 2'764'800);

	//DP8350(config, "crtc", 10.92_MHz_XTAL);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.92_MHz_XTAL, 700, 0, 560, 260, 0, 240);
	screen.set_screen_update(FUNC(hazl1420_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
}

ROM_START(hazl1420)
	ROM_REGION(0x0800, "maincpu_internal", 0)
	// This internal ROM seems to belong to some earlier program revision
	ROM_LOAD("8049h.u19", 0x0000, 0x0800, CRC(81beb6de) SHA1(f272d1277f100af92384a4c4cec2c9db9424b603))

	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("2716.u10", 0x0000, 0x0800, CRC(7c40ba24) SHA1(7575225adf1a06d66b079efcf0f4f9ee77fbddd4))
	ROM_LOAD("8316.u11", 0x0800, 0x0800, CRC(1c112f09) SHA1(fa4973e99c6d66809cffef009c4869787089a774))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("8316.u23", 0x0000, 0x0800, NO_DUMP)
ROM_END

COMP(1979, hazl1420, 0, 0, hazl1420, hazl1420, hazl1420_state, empty_init, "Hazeltine", "1420 Video Display Terminal", MACHINE_IS_SKELETON)
