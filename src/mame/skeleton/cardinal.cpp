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
#include "bus/rs232/rs232.h"
#include "machine/eepromser.h"
#include "sound/spkrdev.h"
#include "video/crt9028.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cardinal_state : public driver_device
{
public:
	cardinal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_eeprom(*this, "eeprom")
		, m_vtlc(*this, "vtlc")
		, m_speaker(*this, "speaker")
		, m_rs232(*this, "rs232")
		, m_address_select(false)
	{
	}

	void cardinal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 p1_r();
	void p1_w(u8 data);

	u8 vtlc_r();
	void vtlc_w(u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
	void ram_map(address_map &map) ATTR_COLD;

	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<crt9028_device> m_vtlc;
	required_device<speaker_sound_device> m_speaker;
	required_device<rs232_port_device> m_rs232;

	bool m_address_select;
};


void cardinal_state::machine_start()
{
	save_item(NAME(m_address_select));
}

u8 cardinal_state::p1_r()
{
	return 0x9f | (m_eeprom->do_read() << 5) | (0 /*m_rs232->cts_r()*/ << 6);
}

void cardinal_state::p1_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->clk_write(BIT(data, 3));

	m_address_select = BIT(data, 1);

	m_speaker->level_w(!BIT(data, 2));
	m_rs232->write_rts(BIT(data, 7));
}

u8 cardinal_state::vtlc_r()
{
	return m_vtlc->read(m_address_select);
}

void cardinal_state::vtlc_w(u8 data)
{
	m_vtlc->write(m_address_select, data);
}

void cardinal_state::prog_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("program", 0);
}

void cardinal_state::ext_map(address_map &map)
{
	map(0, 0).mirror(0xffff).rw(FUNC(cardinal_state::vtlc_r), FUNC(cardinal_state::vtlc_w));
}

void cardinal_state::ram_map(address_map &map)
{
	map(0x000, 0x7ff).ram();
}


static INPUT_PORTS_START(cardinal)
	PORT_START("P3")
	PORT_DIPNAME(0x10, 0x00, "Keyboard Baud Rate")
	PORT_DIPSETTING(0x10, "300")
	PORT_DIPSETTING(0x00, "600")
	PORT_BIT(0xef, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_600)
DEVICE_INPUT_DEFAULTS_END


void cardinal_state::cardinal(machine_config &config)
{
	i8031_device &maincpu(I8031(config, "maincpu", 7.3728_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &cardinal_state::prog_map);
	maincpu.set_addrmap(AS_IO, &cardinal_state::ext_map);
	maincpu.port_in_cb<1>().set(FUNC(cardinal_state::p1_r));
	maincpu.port_out_cb<1>().set(FUNC(cardinal_state::p1_w));
	maincpu.port_in_cb<3>().set_ioport("P3");

	EEPROM_93C06_16BIT(config, m_eeprom);

	CRT9028_000(config, m_vtlc, 10.92_MHz_XTAL);
	m_vtlc->set_screen("screen");
	m_vtlc->set_addrmap(0, &cardinal_state::ram_map);
	m_vtlc->vsync_callback().set_inputline("maincpu", MCS51_INT0_LINE).invert();

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.05);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	rs232_port_device &kb(RS232_PORT(config, "kb", default_rs232_devices, "keyboard"));
	kb.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));
	kb.rxd_handler().set_inputline("maincpu", MCS51_INT1_LINE).invert();
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

} // anonymous namespace


COMP(1984, cardinal, 0, 0, cardinal, cardinal, cardinal_state, empty_init, "Standard Microsystems", "Cardinal Video Terminal", MACHINE_IS_SKELETON)
