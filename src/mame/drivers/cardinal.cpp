// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Standard Microsystems Cardinal.

    This is a generic video display terminal controller on a Eurocard-sized
    PCB. It provides space for two RS-232-C connectors and pin headers for
    a serial keyboard and monitor.

    All video timing and display signal generation, including the character
    set, is integrated within Standard Microsystems' CRT9028 Video Terminal
    Logic Controller.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
//#include "bus/rs232/rs232.h"
#include "machine/eepromser.h"
//#include "video/crt9028.h"
#include "screen.h"

class cardinal_state : public driver_device
{
public:
	cardinal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_eeprom(*this, "eeprom")
		, m_address_select(false)
	{
	}

	void cardinal(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 p1_r();
	void p1_w(u8 data);

	u8 vtlc_r();
	void vtlc_w(u8 data);

	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<eeprom_serial_93cxx_device> m_eeprom;
	//required_device<crt9028_device> m_vtlc;

	bool m_address_select;
};


void cardinal_state::machine_start()
{
	save_item(NAME(m_address_select));
}

u32 cardinal_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 cardinal_state::p1_r()
{
	return 0xdf | (m_eeprom->do_read() << 5);
}

void cardinal_state::p1_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->clk_write(BIT(data, 3));

	m_address_select = BIT(data, 1);
}

u8 cardinal_state::vtlc_r()
{
	return 0xff;
}

void cardinal_state::vtlc_w(u8 data)
{
	logerror("%s: Writing %02X to CRT9028 %s register\n", machine().describe_context(), data, m_address_select ? "address" : "data");
}

void cardinal_state::prog_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("program", 0);
}

void cardinal_state::ext_map(address_map &map)
{
	map(0xff00, 0xff00).mirror(0xff).rw(FUNC(cardinal_state::vtlc_r), FUNC(cardinal_state::vtlc_w));
}


static INPUT_PORTS_START(cardinal)
INPUT_PORTS_END


void cardinal_state::cardinal(machine_config &config)
{
	i8031_device &maincpu(I8031(config, "maincpu", 7.3728_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &cardinal_state::prog_map);
	maincpu.set_addrmap(AS_IO, &cardinal_state::ext_map);
	maincpu.port_in_cb<1>().set(FUNC(cardinal_state::p1_r));
	maincpu.port_out_cb<1>().set(FUNC(cardinal_state::p1_w));

	EEPROM_93C06_16BIT(config, m_eeprom);

	//CRT9028_000(config, m_vtlc, 10.92_MHz_XTAL);
	//m_vtlc->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.92_MHz_XTAL, 700, 0, 560, 260, 0, 240);
	screen.set_screen_update(FUNC(cardinal_state::screen_update));
}


// STANDARD MICROSYSTEMS CORP. CARDINAL © 1984
// ASSY NO.710.015 REV
// Complete IC list:
// * TI SN74LS240N (A1)
// * NS NMC9306N (A2)
// * Intel P8031AH (A3)
// * SMC CRT9028-000 (A4)
// * Hitachi HM6116LP-4 (A5)
// * TI SN74LS373N (A6)
// * TI MC1489A/75189AN (A7)
// * 2732 EPROM in 28-pin socket (A8)
// * TI MC1488/75188N (A9)
// Oscillators: 7.3728 (Y1), 10.920MHz (Y2)
ROM_START(cardinal)
	ROM_REGION(0x1000, "program", 0)
	ROM_LOAD("smc_8031_crt9028.bin", 0x0000, 0x1000, CRC(486705d0) SHA1(39f3fd80a72756b3267d771202cf917060eb04e1))
ROM_END


COMP(1984, cardinal, 0, 0, cardinal, cardinal, cardinal_state, empty_init, "Standard Microsystems", "Cardinal Video Terminal", MACHINE_IS_SKELETON)
