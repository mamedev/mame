// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Robbbert
/***************************************************************************

DEC DCT11-EM (Evaluation Module)

2010-12-03 Skeleton driver.

System currently works, but with some hacks to replace the unemulated DC319
DLART (Digital Link Asynchronous Receiver/Transmitter).


TODO:
- user LED (it's there but it doesn't work)
- DLART device to be emulated
- hookups between DLART (and its DL terminal), UART and remaining interrupts


****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/terminal.h"
#include "dct11em.lh"


namespace {

class dct11em_state : public driver_device
{
public:
	dct11em_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_uart(*this, "uart")
		, m_terminal(*this, "terminal")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_led(*this, "led0")
	{ }

	void dct11em(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(halt_button);
	DECLARE_INPUT_CHANGED_MEMBER(int_button);

private:
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	void porta_w(u8);
	void portc_w(u8);
	u8 portc_r();
	void irq_encoder(u8, bool);
	u8 dlart_r(offs_t);
	void dlart_w(offs_t, u8);
	void kbd_put(u8 data);

	u8 m_term_data = 0U;
	u8 m_seg_lower = 0U;
	u8 m_seg_upper = 0U;
	u8 m_portc = 0U;
	u16 m_irqs = 0U;
	bool m_dlart_maintmode = 0;

	void mem_map(address_map &map) ATTR_COLD;

	required_device<t11_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<i8251_device> m_uart;
	required_device<generic_terminal_device> m_terminal;
	required_ioport_array<5> m_io_keyboard;
	output_finder<12> m_digits;
	output_finder<> m_led;
};

void dct11em_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();   // 2x 6116
	map(0x1000, 0x1fff).noprw(); // the ram test reads/writes here even though there's no ram
	map(0x2000, 0x2fff).ram();   // expansion sockets, 2x 6116
	map(0xa000, 0xdfff).rom();
	map(0xff20, 0xff27).nopr().lw8(NAME([this] (offs_t offset, u8 data) { m_ppi->write(offset>>1, data); }));
	map(0xff28, 0xff2b).nopr().lw8(NAME([this] (offs_t offset, u8 data) { m_uart->write(offset>>1, data); }));
	map(0xff60, 0xff67).lr8(NAME([this] (offs_t offset) { return m_ppi->read(offset>>1); }));
	map(0xff68, 0xff6b).lr8(NAME([this] (offs_t offset) { return m_uart->read(offset>>1); }));
	//map(0xff70, 0xff7f).   // DC319 DLART unemulated device - uart to terminal
	map(0xff70, 0xff7f).rw(FUNC(dct11em_state::dlart_r), FUNC(dct11em_state::dlart_w));
}

// dummy routines to pass the DLART test and to talk to the ascii terminal
u8 dct11em_state::dlart_r(offs_t offset)
{
	offset >>= 1;
	switch (offset)
	{
		case 0:
			if (m_term_data)
				return 0xff;
			else
				return 0xfd;
		case 1:
			{
				u8 ret = m_term_data;
				irq_encoder(3, 0); // Release IRQ3
				m_term_data = 0;
				return ret;
			}
		default:
			return 0xfd;
	}
}

void dct11em_state::dlart_w(offs_t offset, u8 data)
{
	offset >>= 1;
	switch (offset)
	{
		case 3:
			if (m_dlart_maintmode)
				m_term_data = data;
			else
				m_terminal->write(data);
			break;
		case 2:
			m_dlart_maintmode = BIT(data, 4);
			break;
		default:
			break;
	}
}

void dct11em_state::porta_w(u8 data)
{
	m_seg_lower = data;
	if (BIT(m_portc, 3))
		m_seg_upper = data;
}

void dct11em_state::portc_w(u8 data)
{
	data &= 15;
	m_portc = data;
	if (BIT(data, 3))
	{
		m_seg_upper = m_seg_lower;
		irq_encoder(10, 0); // Release IRQ10
	}
	if (data < 6)
	{
		m_digits[data] = m_seg_lower;
		m_digits[data+6] = m_seg_upper;
	}
	m_led = (data!=9);
}

u8 dct11em_state::portc_r()
{
	if (m_portc < 5)
		return m_io_keyboard[m_portc]->read();
	else
		return 0;
}

/*
 * interrupts (p. 101)
 *
 * IRQ  CPx  Pri Vec Device
 * ---  ---  --- --- ------
 * 15   LLLL 7   140 DLART receiver break
 * 11   LHLL 6   100 External interrupt
 * 10   LHLH 6   104 Keypad/LED scanning
 * 7    HLLL 5   120 8251 receiver
 * 6    HLLH 5   124 8251 transmitter
 * 3    HHLL 4   060 DLART receiver
 * 2    HHLH 4   064 DLART transmitter */
 void dct11em_state::irq_encoder(u8 irq, bool state)
{
	if (state)
		m_irqs |= (1 << irq);
	else
		m_irqs &= ~(1 << irq);

	int i;
	for (i = 15; i > 0; i--)
		if (BIT(m_irqs, i))
			break;

	m_maincpu->set_input_line(t11_device::CP3_LINE, BIT(i, 3) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP2_LINE, BIT(i, 2) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP1_LINE, BIT(i, 1) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP0_LINE, BIT(i, 0) ? ASSERT_LINE : CLEAR_LINE);
}

void dct11em_state::machine_reset()
{
	m_irqs = 0;
}

void dct11em_state::machine_start()
{
	m_digits.resolve();
	m_led.resolve();

	save_item(NAME(m_seg_lower));
	save_item(NAME(m_seg_upper));
	save_item(NAME(m_portc));
	save_item(NAME(m_irqs));
	save_item(NAME(m_term_data));
	save_item(NAME(m_dlart_maintmode));
}


/* Input ports */
static INPUT_PORTS_START( dct11em )
	PORT_START("X0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CLR")       PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1/GOLED")   PORT_CODE(KEYCODE_1) // Go with LEDs
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4/PROTEC")  PORT_CODE(KEYCODE_4) // Release protection
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7/PC")      PORT_CODE(KEYCODE_7)
	PORT_START("X1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0/CANCEL")  PORT_CODE(KEYCODE_0) // Cancel breakpoints
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2/CONSOL")  PORT_CODE(KEYCODE_2) // Start console
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5")         PORT_CODE(KEYCODE_5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8/PS")      PORT_CODE(KEYCODE_8)
	PORT_START("X2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EXA/ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3/BAUD")    PORT_CODE(KEYCODE_3) // Set baud rates
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6/SP")      PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9/WP")      PORT_CODE(KEYCODE_9)
	PORT_START("X3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ADV")       PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("BAC")       PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("REG")       PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ADR")       PORT_CODE(KEYCODE_MINUS) // Address
	PORT_START("X4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("SST")       PORT_CODE(KEYCODE_S) // Single-step
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("GO")        PORT_CODE(KEYCODE_X)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("BPT")       PORT_CODE(KEYCODE_B) // Breakpoint
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("FNC")       PORT_CODE(KEYCODE_LALT) // Function
	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("HALT")      PORT_CODE(KEYCODE_H) PORT_CHANGED_MEMBER(DEVICE_SELF, dct11em_state, halt_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INT")       PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, dct11em_state, int_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(dct11em_state::halt_button)
{
	m_maincpu->set_input_line(t11_device::HLT_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(dct11em_state::int_button)
{
	m_maincpu->set_input_line(t11_device::PF_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

void dct11em_state::kbd_put(u8 data)
{
	m_term_data = data;
	irq_encoder(3, 1); // Assert IRQ3
}

void dct11em_state::dct11em(machine_config &config)
{
	/* basic machine hardware */
	T11(config, m_maincpu, 7'500'000); // 7.5MHz XTAL
	m_maincpu->set_initial_mode(0x1403);  /* according to specs */
	m_maincpu->set_addrmap(AS_PROGRAM, &dct11em_state::mem_map);

	config.set_default_layout(layout_dct11em);

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(dct11em_state::porta_w));  // segments
	// port B - expansion interface
	m_ppi->in_pc_callback().set(FUNC(dct11em_state::portc_r));   // keyboard
	m_ppi->out_pc_callback().set(FUNC(dct11em_state::portc_w));  // digits

	I8251(config, m_uart, 2'457'600 / 8);
	m_uart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart->rxrdy_handler().set([this] (bool state) { irq_encoder(7, state); });
	m_uart->txrdy_handler().set([this] (bool state) { irq_encoder(6, state); });

	clock_device &inta_clock(CLOCK(config, "inta_clock", 614'400 / 768)); // 800Hz, from DLART pin 25
	inta_clock.signal_handler().set([this] (bool state) { if (state) irq_encoder(10, 1); }); // Assert IRQ10

	//clock_device &dlart_clock(CLOCK(config, "dlart_clock", 2'457'600 / 4)); --> to DLART CLK pin 32

	clock_device &uart_clock(CLOCK(config, "uart_clock", 2'457'600 / 32));   // from DLART pin 34
	uart_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr)); // connection to host mainframe, does nothing for us
	rs232.rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set(m_uart, FUNC(i8251_device::write_cts));

	GENERIC_TERMINAL(config, m_terminal, 0); // Main terminal for now
	m_terminal->set_keyboard_callback(FUNC(dct11em_state::kbd_put));
}

/* ROM definition */
ROM_START( dct11em )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// Highest address line inverted
	ROM_LOAD16_BYTE( "23-213e4.e53", 0x8000, 0x2000, CRC(bdd82f39) SHA1(347deeff77596b67eee27a39a9c40075fcf5c10d))
	ROM_LOAD16_BYTE( "23-214e4.e45", 0x8001, 0x2000, CRC(b523dae8) SHA1(cd1a64a2bce9730f7a9177d391663919c7f56073))
	ROM_COPY("maincpu", 0x8000, 0xc000, 0x2000)
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                          FULLNAME    FLAGS */
COMP( 1983, dct11em, 0,      0,      dct11em, dct11em, dct11em_state, empty_init, "Digital Equipment Corporation", "DCT11-EM", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
