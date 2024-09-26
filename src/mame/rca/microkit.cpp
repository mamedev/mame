// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/*

    RCA COSMAC Microkit

    http://www.vintagecomputer.net/browse_thread.cfm?id=511

    Press CR or LF to get the * prompt.
    Commands (must be in UPPERcase):
    $Pxxxx - Jump to address xxxx
    ?Mxxxx yyyy - Dump memory starting at xxxx for yyyy bytes
    !Mxxxx yyzz... - Write data (yy etc) to memory xxxx onwards.

    There's no sound or storage facilities, therefore no software.

    The computer looks like a rack-mount metal box with a rudimentary front panel.
    - Buttons are: Reset; Load; Run program; Run Utility
    - There is a RUN LED.
    - It also has a power switch and lamp, and a fuse.
    - According to the schematic there are LEDs on every data, address and status line.

*****************************************************************************************/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "bus/rs232/rs232.h"
#include "machine/cdp1852.h"


namespace {

class microkit_state : public driver_device
{
public:
	microkit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_rs232(*this, "rs232")
	{ }

	void microkit(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	DECLARE_INPUT_CHANGED_MEMBER(runp_button);
	DECLARE_INPUT_CHANGED_MEMBER(runu_button);

private:
	int clear_r();
	void ram_w(offs_t offset, uint8_t data);
	uint8_t ram_r(offs_t offset);

	void microkit_io(address_map &map) ATTR_COLD;
	void microkit_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	std::unique_ptr<uint8_t[]> m_ram;
	uint8_t m_resetcnt = 0U;
	bool m_a15 = 1;
	required_device<cosmac_device> m_maincpu;
	required_memory_region m_rom;
	required_device<rs232_port_device> m_rs232;
};

void microkit_state::microkit_mem(address_map &map)
{
	map(0x0000, 0x01ff).rw(FUNC(microkit_state::ram_r),FUNC(microkit_state::ram_w));
	map(0x0200, 0x0fff).ram();
	map(0x8000, 0x81ff).rom().region("maincpu", 0);  // CDP1832.U2
	map(0x8c00, 0x8c1f).ram();   // CDP1824.U3 "Register Storage"
}

void microkit_state::microkit_io(address_map &map)
{
	map(0x05, 0x05).r("u4", FUNC(cdp1852_device::read)).w("u4", FUNC(cdp1852_device::write));   // user output port
	map(0x06, 0x06).r("u5", FUNC(cdp1852_device::read)).w("u5", FUNC(cdp1852_device::write));   // user input port
	map(0x07, 0x07).nopw(); // writes lots of zeros here
}

static INPUT_PORTS_START( microkit )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, microkit_state, reset_button, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RUN P") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, microkit_state, runp_button, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RUN U") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, microkit_state, runu_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(microkit_state::reset_button)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	m_resetcnt = 0;
}

INPUT_CHANGED_MEMBER(microkit_state::runp_button)
{
	m_a15 = 0;
	m_resetcnt = 0;
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(microkit_state::runu_button)
{
	m_a15 = 1;
	m_resetcnt = 0;
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

int microkit_state::clear_r()
{
	if (m_resetcnt < 0x20)
		m_resetcnt++;

	if (m_resetcnt < 0x10)
		return 0;
	else
	if (m_resetcnt == 0x1c)
		m_a15 = 0;

	return 1;
}

uint8_t microkit_state::ram_r(offs_t offset)
{
	if (m_a15)
		return m_rom->base()[offset];
	else
		return m_ram[offset];
}

void microkit_state::ram_w(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}

void microkit_state::machine_reset()
{
	m_resetcnt = 0;
	m_maincpu->reset();
}

void microkit_state::machine_start()
{
	// Register save state
	save_item(NAME(m_resetcnt));
	save_item(NAME(m_a15));
	m_ram = make_unique_clear<uint8_t[]>(0x200);
	save_pointer(NAME(m_ram), 0x200);
}

static DEVICE_INPUT_DEFAULTS_START( serial_keyb )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_MARK )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END


void microkit_state::microkit(machine_config &config)
{
	// basic machine hardware
	CDP1802(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &microkit_state::microkit_mem);
	m_maincpu->set_addrmap(AS_IO, &microkit_state::microkit_io);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(microkit_state::clear_r));
	m_maincpu->q_cb().set(m_rs232, FUNC(rs232_port_device::write_txd)).invert();

	/* video hardware */
	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF4);
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(serial_keyb));

	cdp1852_device &u4(CDP1852(config, "u4"));
	u4.mode_cb().set_constant(1); // output

	cdp1852_device &u5(CDP1852(config, "u5"));
	u5.mode_cb().set_constant(0); // input
}

ROM_START( microkit )
	ROM_REGION( 0x200, "maincpu", 0 )
	ROM_LOAD( "ut4.u2", 0x0000, 0x0200, CRC(ba642f8e) SHA1(8975a4c6a0bb699b53c753946aac54860054246a) )

	// These are bad, with heaps of bugs
	//ROM_REGION( 0x200, "maincpu", ROMREGION_INVERT )
	//ROM_LOAD( "3.2b", 0x000, 0x100, CRC(6799357e) SHA1(c46e3322b8b1b6534a7da04806be29fa265951b7) )
	//ROM_LOAD( "4.2a", 0x100, 0x100, CRC(27267bad) SHA1(838df9be2dc175584a1a6ee1770039118e49482e) )

ROM_END

} // anonymous namespace


COMP( 1975, microkit, 0, 0, microkit, microkit, microkit_state, empty_init, "RCA", "COSMAC Microkit", MACHINE_SUPPORTS_SAVE )
