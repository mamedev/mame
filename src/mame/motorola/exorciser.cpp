// license:BSD-3-Clause
// copyright-holders:68bit
/***************************************************************************

Motorola M6800 EXORciser I (M68SDT)

Note The EXORciser II (M68SDT II) was distinctly different, using dual 64k
address maps (a user map and a supervisor map) and other improvements, and
used EXBUG 2.

To boot MDOS using the ROM disk boot press 'X' until the EXBUG prompt is seen,
and then enter "MAID" without a LF or CR, and then at the "*" prompt enter
E800;G without a LF or CR.

EXBUG 1.1 and 1.2 commands:
X
  General escape command, exits to the prompt.
MAID
  Switches to the MAID (Motorola Active Interface Debug) mode.
LOAD
  Load S19 format binary object tapes. At the "SGL/CONT" prompt press S to
  load a single program, or press C to load multiple programs and the abort
  button when finished, or press X to abort.
VERF
  Verifies memory from tape data. At the "SGL/CONT" prompt press S to verify a
  single program, or press C to verify multiple programs and the abort button
  when finished, or press X to abort.
SRCH
  Searches for a S0 header and prints it and prompts CONT/LOAD/VERF and then
  press C to continue searching, or press L to switch to the LOAD function, or
  press V to switch to the VERF function, or press X to abort.
PNCH
  Prompts for a begin and an end address and then for a header of 6 characters
  and then prints "EXEC" and waits for Y and then outputs the memory range in
  S19 format, or press X to abort.
PRNT
  Prints a memory dump. Prompts for a begin and an end address then prints
  "EXEC" and waits for Y and then dumps the memory in a readable format, or
  press X to abort.
TERM (EXBUG 1.2 extension)
  Prompts for a 15 bit hex number which is used as a terminal output delay
  between characters.
S10.
S30.
S120
S240 (EXBUG 1.2 extension)
  Select different communication protocols for the tape drive.


MAID (Motorola Active Interface Debug) mode has a '*' prompt and the
following commands:
X
  General escape command, exits the command or MAID mode.
xxxx/
  Memory examine and change.
    xx LF   - enter new value, and advance to the next address
    LF      - advance to the next address
    ^       - previous address
    xxxx;O  - calculate a branch offset from the current to the given address
              printing INVL if out of range.
$R
  Register display and change. LF advances to the next register.
xxxx;V
  Insert a software breakpoint at the given address, up to 8 breakpoints.
  These are stored starting at address $ff4f.
$V
  Prints the address of the 8 breakpoints.
xxxx;U
  Removes the software breakpoint at the given address.
;U
  Removes all software breakpoints.
$M
  Prints the current search mask and prompts for a new search mask and address
  range for use by the ;W search command.
xx;W
  Searches the memory range set by the $M command for a byte matching the
  given xx masked with the mask set by the $M command, printing all
  matches. The test is (byte^xx) & mask == 0.
$T
  Prompts for an end address, to trace this address when code is next run.
;T
  Deactivates the trace end address set by the $T command.
$S
  Prompts for a stop address, which it sets and activates. This is a hardware
  breakpoint that activates when the address is touched.
;S
  Deactivates the stop address, as set by the $S command.
;G
  Execute the user's program from the auto start memory location.
xxxx;G
  Execute the user's program from the given address.
;P
  Continue executing from the current program counter address.
nn;P
  Continue executing from the current program counter address, skipping the
  given number of software breakpoints. This does not appear to be reliable
  at least not in the emulator?
N
  Trace one instruction.
;N
xxxx;N
  Trace the next instruction or the given number of instructions.
#nnnnn=
  Converts the given decimal number to hex.
#@nnn=
  Converts the given octal number to hex.
#$xxxx=
  Converts the given hex number to decimal.


For all tape formats the PNCH command sends code 0x12, aka DC2, to start tape
recording and code 0x14, aka DC4, to stop tape recording. For tape format
S120, the codes 0x10, 0x30, 0x12 are sent to start tape recording, and the
codes 0x14, 0x10, 0x39 are sent to stop tape recording.

For tape formats S10 and S30 the ACIA RTS line is used for tape motor control
when reading from the tape, but not when writing to the tape, and the code
0x11, aka DC1, is sent to start tape playback, and the code 0x13, aka DC3, is
sent to stop tape playback.

For tape format S120, the ACIA RTS line is not used for tape motor control,
rather the codes 0x10, 0x37 are sent to start playback from the tape, and the
code 0x13 is sent to stop playback.

References:

"M6800 EXORciser User's Guide.", second edition, Motorola, 1975.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/exorterm.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/pty.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/terminal.h"
#include "cpu/m6800/m6800.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"
#include "machine/m68sfdc.h"

#include "imagedev/printer.h"

#include "formats/mdos_dsk.h"


namespace {

class exorciser_state : public driver_device
{
public:
	exorciser_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_mainirq(*this, "mainirq")
		, m_mainnmi(*this, "mainnmi")
		, m_maincpu_clock(*this, "MAINCPU_CLOCK")
		, m_abort_key(*this, "ABORT_KEY")
		, m_pia_dbg(*this, "pia_dbg")
		, m_acia(*this, "acia")
		, m_brg(*this, "brg")
		, m_rs232_baud(*this, "RS232_BAUD")
		, m_rs232_config(*this, "RS232_CONFIG")
		, m_pia_lpt(*this, "pia_lpt")
		, m_printer(*this, "printer")
		, m_acia_prn(*this, "acia_prn")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "floppy%u", 0U)
	{ }

	void exorciser(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(maincpu_clock_change);
	void abort_key_w(int state);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void dbg_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	void irq_line_w(int state);
	u8 m_irq;
	address_space *m_banked_space;
	u8 main_r(offs_t offset);
	void main_w(offs_t offset, u8 data);
	u8 prom_r(offs_t offset);

	required_device<m6800_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device<input_merger_device> m_mainirq;
	required_device<input_merger_device> m_mainnmi;
	required_ioport m_maincpu_clock;
	required_ioport m_abort_key;
	required_device<pia6821_device> m_pia_dbg;
	required_device<acia6850_device> m_acia;
	required_device<mc14411_device> m_brg;
	required_ioport m_rs232_baud;
	required_ioport m_rs232_config;
	required_device<pia6821_device> m_pia_lpt;
	required_device<printer_image_device> m_printer;
	required_device<acia6850_device> m_acia_prn;
	required_device<m68sfdc_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;

	// RS232 bit rate generator clocks
	void write_f1_clock(int state);
	void write_f3_clock(int state);
	[[maybe_unused]] void write_f5_clock(int state);
	void write_f7_clock(int state);
	void write_f8_clock(int state);
	void write_f9_clock(int state);
	[[maybe_unused]] void write_f11_clock(int state);
	void write_f13_clock(int state);

	u8 m_restart_count;

	emu_timer *m_trace_timer;
	TIMER_CALLBACK_MEMBER(assert_trace);

	void pia_dbg_pa_w(u8 data);
	int pia_dbg_ca1_r();
	void pia_dbg_pb_w(u8 data);
	void pia_dbg_ca2_w(int state);
	void pia_dbg_cb2_w(int state);
	u16 m_stop_address;
	u8 m_stop_enabled;

	void pia_lpt_pa_w(u8 data);
	void pia_lpt_ca2_w(int state);
	uint8_t pia_lpt_pb_r();
	uint8_t m_printer_data;
	uint8_t m_printer_data_ready;

	static void exorciser_rs232_devices(device_slot_interface &device);

	static void floppy_formats(format_registration &fr);
};

void exorciser_state::dbg_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(exorciser_state::main_r), FUNC(exorciser_state::main_w));
}

void exorciser_state::mem_map(address_map &map)
{
	// User RAM
	map(0x0000, 0xe800).ram();

	// Disk driver code.
	map(0xe800, 0xebff).rom().region("68fdc2", 0);

	// Disk driver unit
	map(0xec00, 0xec07).rw(m_fdc, FUNC(m68sfdc_device::read), FUNC(m68sfdc_device::write));

	// Line printer
	map(0xec10, 0xec13).rw(m_pia_lpt, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// Serial printer.
	map(0xec26, 0xec27).rw(m_acia_prn, FUNC(acia6850_device::read), FUNC(acia6850_device::write));

	// EXBUG
	map(0xf000, 0xfbff).rom().region("exbug", 0);

	map(0xfcf4, 0xfcf5).mirror(0x0002).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xfcf8, 0xfcfb).rw(m_pia_dbg, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// Small PROM with the restart vector and ACIA settings.
	map(0xfcfc, 0xfcff).r(FUNC(exorciser_state::prom_r));

	// EXBUG RAM
	map(0xff00, 0xffff).ram();
}

/* Input ports */
static INPUT_PORTS_START( exorciser )

	PORT_START("ABORT_KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Abort") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, exorciser_state, abort_key_w)

	// The EXORciser I supported 1MHz, and the EXORciser II also supported
	// 1.5 and 2.0MHz.
	PORT_START("MAINCPU_CLOCK")
	PORT_CONFNAME(0xffffff, 1000000, "CPU clock") PORT_CHANGED_MEMBER(DEVICE_SELF, exorciser_state, maincpu_clock_change, 0)
	PORT_CONFSETTING(1000000, "1.0 MHz")
	PORT_CONFSETTING(2000000, "1.5 MHz")
	PORT_CONFSETTING(4000000, "2.0 MHz")

	PORT_START("RS232_BAUD")
	PORT_CONFNAME(0xff, 1, "RS232 Baud Rate")
	PORT_CONFSETTING(0x80, "110")
	PORT_CONFSETTING(0x40, "150")
	PORT_CONFSETTING(0x20, "300")
	PORT_CONFSETTING(0x10, "600")
	PORT_CONFSETTING(0x08, "1200")
	PORT_CONFSETTING(0x04, "2400")
	PORT_CONFSETTING(0x02, "4800")
	PORT_CONFSETTING(0x01, "9600")

	PORT_START("RS232_CONFIG")
	PORT_CONFNAME(0x7F, 0x75, "RS232 Config")
	PORT_CONFSETTING(0x61, "7 data bits, 2 stop bits, even parity")
	PORT_CONFSETTING(0x65, "7 data bits, 2 stop bits, odd parity")
	PORT_CONFSETTING(0x69, "7 data bits, 1 stop bits, even parity")
	PORT_CONFSETTING(0x6d, "7 data bits, 1 stop bits, odd parity")
	PORT_CONFSETTING(0x71, "8 data bits, 2 stop bits, no parity")
	PORT_CONFSETTING(0x75, "8 data bits, 1 stop bit, no parity")
	PORT_CONFSETTING(0x79, "8 data bits, 1 stop bit, even parity")
	PORT_CONFSETTING(0x7d, "8 data bits, 1 stop bit, odd parity")

INPUT_PORTS_END

INPUT_CHANGED_MEMBER(exorciser_state::maincpu_clock_change)
{
	m_maincpu->set_clock(newval);
}


void exorciser_state::write_f1_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 0))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}

	m_acia_prn->write_txc(state);
	m_acia_prn->write_rxc(state);
}

void exorciser_state::write_f3_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 1))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorciser_state::write_f5_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 2))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorciser_state::write_f7_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 3))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorciser_state::write_f8_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 4))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorciser_state::write_f9_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 5))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorciser_state::write_f11_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 6))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorciser_state::write_f13_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 7))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

u8 exorciser_state::main_r(offs_t offset)
{
	if (offset == m_stop_address && m_stop_enabled &&
		!machine().side_effects_disabled())
	{
		m_pia_dbg->cb1_w(CLEAR_LINE);
		m_pia_dbg->cb1_w(ASSERT_LINE);
		m_pia_dbg->cb1_w(CLEAR_LINE);
		// The PIA does not connect to the NMI, rather
		// this triggers some logic that triggers the NMI.
		// The stop-on-address works with just this.
		// TODO This logic might have some delay etc?
		m_mainnmi->in_w<0>(1);
		m_mainnmi->in_w<0>(0);
	}
	if (offset < 0xfffc)
		return m_banked_space->read_byte(offset);
	else if (m_restart_count < 2)
	{
		// The MAME debugger appears to read here on reset to
		// initialize the PC, so it is not possible to distinguish a
		// normal and debug read, so disable this path after the first
		// two reads irrespective of side effects being enabled.
		m_restart_count++;
		if (offset == 0xfffe)
			return 0xf0;
		if (offset == 0xffff)
			return 0x00;
		return 0;
	}
	return m_banked_space->read_byte(offset);
}

void exorciser_state::main_w(offs_t offset, u8 data)
{
	m_banked_space->write_byte(offset, data);
}


// The PROM occupies four addresses 0xfcfc to 0xfcff, decoding A0 and A1. It is
// used to supply the restart address, the first two reads which will be from
// 0xfffe and 0xffff are redirected to this PROM.
//
// The EXBUG firmware reads 0xfcfd to obtain some ACIA configuation bits. EXBUG
// 1.1 masks out all bits except 0x75, and EXBUG 1.2 masks out all bits except
// 0x7f.
//
// The A2 input comes from a circuit that selects the number of stop bits,
// however this might have only been effective at 150 baud. These are config
// options here, as if that small PROM was configured to the desired serial
// protocol settings. Since EXBUG 1.1 masks more of these bits is has less
// effective options.
//
// Input A3 is connected to the /IRQ line and has the effect of setting high
// bits in the byte read from 0xfcfd to allow probing of the /IRQ line. This
// feature does not appear to be used by EXBUG.
//
// Input A4 is a static level selectable via jumpers and when clear all reads
// from 0xfcfc to 0xfcff appear to return zero in the default PROM.
u8 exorciser_state::prom_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return 0;
	case 1: {
		u8 byte = m_rs232_config->read();
		if (!m_irq)
			byte |= 0xf0;
		return byte;
	}
	case 2:
		// Restart vector
		return 0xf0;
	case 3:
		return 0x00;
	}

	return 0;
}


void exorciser_state::pia_dbg_pa_w(u8 data)
{
	m_stop_address = (m_stop_address & 0xff00) | data;
}

void exorciser_state::abort_key_w(int state)
{
	m_pia_dbg->ca1_w(!state);
	m_mainnmi->input_merger_device::in_w<2>(state);
}

int exorciser_state::pia_dbg_ca1_r()
{
	return !m_abort_key->read();
}

void exorciser_state::pia_dbg_pb_w(u8 data)
{
	m_stop_address = (m_stop_address & 0x00ff) | (data << 8);
}

void exorciser_state::pia_dbg_ca2_w(int state)
{
	m_stop_enabled = !state;
}


TIMER_CALLBACK_MEMBER(exorciser_state::assert_trace)
{
	m_mainnmi->input_merger_device::in_w<1>(ASSERT_LINE);
}

// Note the trace timer delay is actually 11 cycles, but is stretched to 16
// cycles here to get it working. This is necessary because of inaccurate
// cycle timing in the 6800 emulation, so change the delay to 11 cycles when
// the cycle emulation is more accurate.
void exorciser_state::pia_dbg_cb2_w(int state)
{
	if (state)
	{
		// The trace timer needs to scale with the CPU clock.
		uint32_t maincpu_clock = m_maincpu_clock->read();
		if (!maincpu_clock)
			maincpu_clock = 10000000;

		m_trace_timer->adjust(attotime::from_ticks(16, maincpu_clock));
	}
	else
		m_mainnmi->input_merger_device::in_w<1>(CLEAR_LINE);
}



void exorciser_state::pia_lpt_pa_w(u8 data)
{
	// External parallel printer data output.
	m_printer_data = data;
}

void exorciser_state::pia_lpt_ca2_w(int state)
{
	// External parallel printer data ready.

	// Trigger on the falling edge.
	if (m_printer_data_ready == 1 && state == 0)
	{
		m_printer->output(m_printer_data);
		// Toggle the printer busy line as the software waits for a
		// low to high transition.
		m_pia_lpt->ca1_w(CLEAR_LINE);
		m_pia_lpt->ca1_w(ASSERT_LINE);
		m_pia_lpt->ca1_w(CLEAR_LINE);
	}
	m_printer_data_ready = state;
}


uint8_t exorciser_state::pia_lpt_pb_r()
{
	// The printer driver expects the low two bits to be 01 for a printer
	// attempt to succeed.
	return 1;
}


void exorciser_state::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add(FLOPPY_MDOS_FORMAT);
}


static void mdos_floppies(device_slot_interface &device)
{
	device.option_add("8sssd",  FLOPPY_8_SSSD);       // 77 trks ss sd 8"
	device.option_add("8dssd",  FLOPPY_8_DSSD);       // 77 trks ds sd 8"
}


void exorciser_state::irq_line_w(int state)
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, state);
	m_irq = state;
}

void exorciser_state::machine_reset()
{
	uint32_t maincpu_clock = m_maincpu_clock->read();
	if (maincpu_clock)
		m_maincpu->set_clock(maincpu_clock);

	m_brg->rsa_w(0);
	m_brg->rsb_w(1);

	m_restart_count = 0;

	m_fdc->set_floppies_4(m_floppy[0], m_floppy[1], m_floppy[2], m_floppy[3]);

	m_irq = 1;
	m_stop_address = 0x0000;
	m_stop_enabled = 0;

	m_printer_data = 0;
	m_printer_data_ready = 1;
	m_pia_lpt->ca1_w(CLEAR_LINE);

}

void exorciser_state::machine_start()
{
	m_banked_space = &subdevice<address_map_bank_device>("bankdev")->space(AS_PROGRAM);

	save_item(NAME(m_restart_count));
	save_item(NAME(m_irq));
	save_item(NAME(m_stop_address));
	save_item(NAME(m_stop_enabled));
	save_item(NAME(m_printer_data));
	save_item(NAME(m_printer_data_ready));

	m_trace_timer = timer_alloc(FUNC(exorciser_state::assert_trace), this);
}

static DEVICE_INPUT_DEFAULTS_START(exorterm)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(printer)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

void exorciser_state::exorciser_rs232_devices(device_slot_interface &device)
{
	device.option_add("exorterm155", SERIAL_TERMINAL_EXORTERM155);
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("pty", PSEUDO_TERMINAL);
}

void exorciser_state::exorciser(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &exorciser_state::dbg_map);

	ADDRESS_MAP_BANK(config, m_bankdev, 0);
	m_bankdev->set_endianness(ENDIANNESS_BIG);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(16);
	m_bankdev->set_addrmap(AS_PROGRAM, &exorciser_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set(FUNC(exorciser_state::irq_line_w));
	INPUT_MERGER_ANY_HIGH(config, m_mainnmi).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	MC14411(config, m_brg, XTAL(1'843'200));
	m_brg->out_f<1>().set(FUNC(exorciser_state::write_f1_clock));
	m_brg->out_f<3>().set(FUNC(exorciser_state::write_f3_clock));
	m_brg->out_f<7>().set(FUNC(exorciser_state::write_f7_clock));
	m_brg->out_f<8>().set(FUNC(exorciser_state::write_f8_clock));
	m_brg->out_f<9>().set(FUNC(exorciser_state::write_f9_clock));
	m_brg->out_f<13>().set(FUNC(exorciser_state::write_f13_clock));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", exorciser_state::exorciser_rs232_devices, "exorterm155"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.set_option_device_input_defaults("exorterm155", DEVICE_INPUT_DEFAULTS_NAME(exorterm));

	PIA6821(config, m_pia_dbg);
	m_pia_dbg->writepa_handler().set(FUNC(exorciser_state::pia_dbg_pa_w));
	m_pia_dbg->readca1_handler().set(FUNC(exorciser_state::pia_dbg_ca1_r));
	m_pia_dbg->writepb_handler().set(FUNC(exorciser_state::pia_dbg_pb_w));
	m_pia_dbg->ca2_handler().set(FUNC(exorciser_state::pia_dbg_ca2_w));
	m_pia_dbg->cb2_handler().set(FUNC(exorciser_state::pia_dbg_cb2_w));

	// MEX68PI Parallel printer port
	PIA6821(config, m_pia_lpt);
	m_pia_lpt->writepa_handler().set(FUNC(exorciser_state::pia_lpt_pa_w));
	m_pia_lpt->ca1_w(0); // External parallel printer busy input.
	m_pia_lpt->ca2_handler().set(FUNC(exorciser_state::pia_lpt_ca2_w));
	m_pia_lpt->readpb_handler().set(FUNC(exorciser_state::pia_lpt_pb_r));

	PRINTER(config, m_printer, 0);

	// MEX6850? Serial printer port
	ACIA6850(config, m_acia_prn, 0);
	m_acia_prn->txd_handler().set("rs232_prn", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232_prn(RS232_PORT(config, "rs232_prn", default_rs232_devices, "printer"));
	rs232_prn.rxd_handler().set(m_acia_prn, FUNC(acia6850_device::write_rxd));
	rs232_prn.set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(printer));

	M68SFDC(config, m_fdc, 0);
	m_fdc->irq_handler().set(m_mainirq, FUNC(input_merger_device::in_w<0>));
	m_fdc->nmi_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<3>));

	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, mdos_floppies, "8dssd", exorciser_state::floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( exorciser )
	ROM_REGION( 0x0400, "68fdc2", 0 )
	// Later MDOS versions support four double sided disk drives, but these
	// ROMs only support two singled sided disk drives. These ROMs are hard
	// coded for 26 sectors per track and up to 2002 sectors fill in the 77
	// tracks. Both these ROM versions appear to have largely compatible
	// entry points. The MDOS equate file calls this the Disk EROM.
	//
	// Various MDOS commands, such a 'format' and 'dosgen' probe this ROM
	// at 0xebfe and 0xebff and use the contents to select between
	// operating modes and features. There are clearly some ROMs supported
	// by MDOS that are not represented here. If 0xebff has 'E' then the
	// MDOS FORMAT command errors out. If 0xebff has 'C' then the 'write
	// enabled' input is interpreted as being an active low write protect
	// input, otherwise it is interpreted as being an active low write
	// enabled input. If 0xebff has 'C' or 0xebfe has 0x11 or 0x12 then
	// disks are formatted single sided otherwise PA5 is consulted.
	//
	// The MDOS code, such as DOSGEN, expects the byte at $000d to have bit
	// 7 set for single sided and clear for double sided disks, yet these
	// ROMs clear this bit. For DOSGEN this can be worked around with these
	// ROMs by locking out the sectors that would have been allocated
	// beyond the single sided disk extent, so lock out 0x7d0 to 0xfa0. The
	// FORMAT command does not look at this bit so is not affected by this
	// issue. Clearly neither of these ROMs was intended to work cleanly
	// with MDOS 3 even using single sided disks.
	//
	// So it would be great if a ROM can be found that supports double
	// sided disks, but if not then there appears to be enough known to
	// write a ROM to work with MDOS. The ROM is tightly written already,
	// and it is not clear how well double sided support was implemented,
	// but the ROM accepts logical sector numbers and to work with both
	// single and double sided disks it may need to firstly try a double
	// sided operation and if that fails then to fall back to try a single
	// sided operation and to set the 0x000d flag appropriately. Note that
	// the FORMAT command does not bother to set the 'side' in the address
	// marks, so the ROM should ignore that or expect it to be
	// zero. Support also needs to be added for four drives which is just a
	// matter of setting the 'select 2' output based on bit 1 of 0x0000 and
	// updating the bounds check.
	//
	// The code compression techniques used in this disk driver code are
	// edifying, generally reusing code well. For example, flags are used
	// to make subtle changes in code paths to reuse many paths. Test or
	// comparison instructions are used to probe I/O addresses without
	// destroying an accumulator to reduce register pressure.
	//
	// Good use is made of instructions with overlapping
	// interpretations. For example, placing small instructions in the
	// operands of other instructions where they become effectively a NOP
	// and this saves on branching. Two byte instructions such as loading
	// an accumulator are placed in the 16 bit immediate argument of a
	// three byte instruction, or one byte instructions such as a shift or
	// rotate are placed in the immediate argument of a two byte
	// instruction.
	//
	// This version supports two single sided disk drives. The use of the
	// 'step' and 'direction' lines is conventional. It interprets a high
	// on the 'write enabled' input as an active low write protected
	// input. It drives a serial printer using a 6850 ACIA at $ec26-ec27,
	// which is configured for 8 bits, no parity, and 1 stop bit, and
	// clocked at 9600 baud here, and it supports XON-XOFF software flow
	// control. The last byte of this ROM has been patched from 0x00 to
	// 0x43 to work with the MDOS feature detection code.
	//
	ROM_LOAD("diskeromv2.bin", 0x0000, 0x0400, CRC(09b1e724) SHA1(59b54ded1f9a7266c1f12da37f700b4b478b84bc))

	// This version supports two single sided disk drives. It toggles the
	// 'step' line low to step towards track 0 and toggles the 'direction'
	// line low to step away from track 0. It interprets a high on the
	// 'write enabled' input as an active low write enabled input rather
	// than an active low write protected input. It drives a parallel
	// printer using a 6821 PIA at $ec10-ex13. The last two bytes of this
	// PROM are 0x07 and 0x50 ('P') - these codes are probed by some MDOS
	// commands to select different operation and features.
	//ROM_LOAD("diskeromv1.bin", 0x0000, 0x0400, CRC(87bf9b0d) SHA1(dbcce885d21b418a08812234d1581ac101f32536))

	ROM_REGION( 0x0c00, "exbug", 0 )
	// EXBUG 1.2 adds support for the 'S240' command another punch tape
	// protocol, and a 'TERM' command which accepts a 15 bit hex number as
	// a terminal output delay between characters.
	ROM_DEFAULT_BIOS("exbug12")
	ROM_SYSTEM_BIOS(0, "exbug12", "EXBUG version 1.2")
	ROMX_LOAD("exbug12.bin", 0x0000, 0x0c00, CRC(c002759b) SHA1(0f63d75148e82fb0b9ce7d64b91464523937e0b7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "exbug11", "EXBUG version 1.1")
	ROMX_LOAD("exbug11.bin", 0x0000, 0x0c00, CRC(5a5db110) SHA1(14f3e14ed809f9ec30b8189e5506ed911127de34), ROM_BIOS(1))

ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                                     FULLNAME      FLAGS
COMP( 1975, exorciser,  0,      0,      exorciser,   exorciser, exorciser_state, empty_init, "Motorola", "M6800 EXORciser (M68SDT)", MACHINE_NO_SOUND_HW )
