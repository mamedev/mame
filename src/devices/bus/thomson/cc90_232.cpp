// license:BSD-3-Clause
// copyright-holders:Antoine Mine

#include "emu.h"
#include "cc90_232.h"

#include "bus/centronics/ctronics.h"
#include "machine/input_merger.h"
#include "machine/output_latch.h"

#define VERBOSE 0
#include "logmacro.h"

/* ------------ CC 90-232 I/O extension ------------ */

/* Features:
   - 6821 PIA
   - serial RS232: bit-banging?
   - parallel CENTRONICS: a printer (-prin) is emulated
   - usable on TO7(/70), MO5(E), MO5NR
     (TO8, TO9, MO6 normally use their built-in parallel printer interfaces instead)

   Note: it seems impossible to connect both a serial & a parallel device
   because the Data Transmit Ready bit is shared in an incompatible way!
*/

// device type definition
DEFINE_DEVICE_TYPE(CC90_232, cc90_232_device, "cc90_232", "Thomson CC 90-232 Communication Interface")

//-------------------------------------------------
//  cc90_232_device - constructor
//-------------------------------------------------

cc90_232_device::cc90_232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CC90_232, tag, owner, clock),
	thomson_extension_interface(mconfig, *this),
	m_pia_io(*this, "pia"),
	m_rs232(*this, "rs232"),
	m_last_low(0)
{
}

void cc90_232_device::rom_map(address_map &map)
{
}

void cc90_232_device::io_map(address_map &map)
{
	map(0x20, 0x23).rw(m_pia_io, FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
}

void cc90_232_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia_io);
	m_pia_io->readpa_handler().set(FUNC(cc90_232_device::porta_in));
	m_pia_io->writepa_handler().set(FUNC(cc90_232_device::porta_out));
	m_pia_io->writepb_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_pia_io->cb2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	m_pia_io->irqa_handler().set("piairq", FUNC(input_merger_device::in_w<0>));
	m_pia_io->irqb_handler().set("piairq", FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, "piairq").output_handler().set(FUNC(cc90_232_device::firq_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(cc90_232_device::write_rxd));
	m_rs232->cts_handler().set(FUNC(cc90_232_device::write_cts));
	m_rs232->dsr_handler().set(FUNC(cc90_232_device::write_dsr));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set(m_pia_io, FUNC(pia6821_device::cb1_w));
	centronics.busy_handler().set(FUNC(cc90_232_device::write_centronics_busy));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);
}


void cc90_232_device::device_start()
{
	m_rs232->write_dtr(0);
}

void cc90_232_device::porta_out(uint8_t data)
{
	int txd = (data >> 0) & 1;
	int rts = (data >> 1) & 1;

	LOG( "%s %f porta_out: txd=%i, rts=%i\n",  machine().describe_context(), machine().time().as_double(), txd, rts );

	m_rs232->write_txd(txd);
	m_rs232->write_rts(rts);
}



WRITE_LINE_MEMBER(cc90_232_device::write_rxd )
{
	m_rxd = state;
}

WRITE_LINE_MEMBER(cc90_232_device::write_dsr )
{
	if (!state) m_last_low = 0;

	m_dsr = state;
}

WRITE_LINE_MEMBER(cc90_232_device::write_cts )
{
	m_pia_io->ca1_w(state);
	m_cts = state;
}

WRITE_LINE_MEMBER(cc90_232_device::write_centronics_busy )
{
	if (!state) m_last_low = 1;

	m_centronics_busy = state;
}


uint8_t cc90_232_device::porta_in()
{
	LOG( "%s %f porta_in: select=%i cts=%i, dsr=%i, rd=%i\n", machine().describe_context(), machine().time().as_double(), m_centronics_busy, m_cts, m_dsr, m_rxd );

	/// HACK: without high impedance we can't tell whether a device is driving a line high or if it's being pulled up.
	/// so assume the last device to drive it low is active.
	int dsr;
	if (m_last_low == 0)
		dsr = m_dsr;
	else
		dsr = !m_centronics_busy;

	return (0x1f /* not required when converted to write_pa */) | (m_cts << 5) | (dsr << 6) | (m_rxd << 7);
}
