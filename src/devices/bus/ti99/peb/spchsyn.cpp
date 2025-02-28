// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Speech synthesizer

    We emulate the Speech Synthesizer plugged onto a P-Box adapter. The original
    Speech Synthesizer device was provided as a box to be plugged into the
    right side of the console. In order to be used with Geneve and SGCPU, the
    speech synthesizer must be moved into the Peripheral Box.

    The Speech Synthesizer used for the TI was the CD2501E, AKA TMS5200,
    (internal name TMC0285), a predecessor of the TMS5220 which was used in
    other commercial products.

    Note that this adapter also contains the speech roms.

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "spchsyn.h"

#include "speaker.h"

#define LOG_WARN        (1U << 1)    // Warnings
#define LOG_CONFIG      (1U << 2)
#define LOG_MEM         (1U << 3)
#define LOG_ADDR        (1U << 4)
#define LOG_READY       (1U << 5)

#define VERBOSE (LOG_CONFIG | LOG_WARN)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_SPEECH, bus::ti99::peb::ti_speech_synthesizer_device, "ti99_speech", "TI-99 Speech synthesizer (on adapter card)")

namespace bus::ti99::peb {

/****************************************************************************/

ti_speech_synthesizer_device::ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_SPEECH, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_vsp(*this, "vsp"),
	m_reading(false),
	m_sbe(false),
	m_dec_high(false)
{
}

/*
    Memory read
*/

void ti_speech_synthesizer_device::readz(offs_t offset, uint8_t *value)
{
	if (machine().side_effects_disabled()) return;

	if (m_sbe)
	{
		*value = m_vsp->status_r() & 0xff;
		LOGMASKED(LOG_MEM, "read value = %02x\n", *value);
		// We should clear the lines at this point. The TI-99/4A clears the
		// lines by setting the address bus to a different value, but the
		// Geneve may behave differently. This may not 100% reflect the real
		// situation, but it ensures a safe processing.
		m_vsp->combined_rsq_wsq_w(~0);
	}
}

/*
    Memory write
*/
void ti_speech_synthesizer_device::write(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled()) return;

	if (m_sbe)
	{
		LOGMASKED(LOG_MEM, "write value = %02x\n", data);
		m_vsp->data_w(data);
		// Note that we must NOT clear the lines here. Find the lines in the
		// READY callback below.
	}
}

void ti_speech_synthesizer_device::setaddress_dbin(offs_t offset, int state)
{
	// 1001 00xx xxxx xxx0   DBIN=1
	// 1001 01xx xxxx xxx0   DBIN=0
	// 1111 1000 0000 0001    mask
	m_reading = (state==ASSERT_LINE);

	bool valid = (((offset & 0x0400)==0) == m_reading);

	if (m_dec_high)
		m_sbe = ((offset & 0x7f801)==0x79000) && valid;
	else
		m_sbe = ((offset & 0x0f801)==0x09000) && valid;

	if (m_sbe)
	{
		LOGMASKED(LOG_ADDR, "set address = %04x, dbin = %d\n", offset, state);

		// Caution: In the current tms5220 emulation, care must be taken
		// to clear one line before asserting the other line, or otherwise
		// both RS* and WS* are active, which is illegal.
		// Alternatively, we'll use the combined settings method

		m_vsp->combined_rsq_wsq_w(m_reading ? ~tms5220_device::RS : ~tms5220_device::WS);
	}
	else
		// If other address, turn off RS* and WS* (negative logic!)
		m_vsp->combined_rsq_wsq_w(~0);
}

/****************************************************************************/

void ti_speech_synthesizer_device::speech_ready(int state)
{
	// The TMS5200 implementation uses true/false, not ASSERT/CLEAR semantics
	// and we have to adapt a /READY to a READY line.
	// The real synthesizer board uses a transistor for that purpose.
	m_slot->set_ready((state==0)? ASSERT_LINE : CLEAR_LINE);
	LOGMASKED(LOG_READY, "READY = %d\n", (state==0));

	if ((state==0) && !m_reading)
		// Clear the lines only when we are done with writing.
		m_vsp->combined_rsq_wsq_w(~0);
}

void ti_speech_synthesizer_device::device_start()
{
	save_item(NAME(m_reading));
	save_item(NAME(m_sbe));
}

void ti_speech_synthesizer_device::device_reset()
{
	m_reading = false;
	m_sbe = false;
	m_dec_high = (ioport("AMADECODE")->read()!=0);
}

ROM_START( ti99_speech )
	ROM_REGION(0x8000, "vsm", 0)
	ROM_LOAD("cd2325a.u2a", 0x0000, 0x4000, CRC(1f58b571) SHA1(0ef4f178716b575a1c0c970c56af8a8d97561ffe)) // at location u2, bottom of stack
	ROM_LOAD("cd2326a.u2b", 0x4000, 0x4000, CRC(65d00401) SHA1(a367242c2c96cebf0e2bf21862f3f6734b2b3020)) // at location u2, top of stack
ROM_END

void ti_speech_synthesizer_device::device_add_mconfig(machine_config& config)
{
	SPEAKER(config, "speech_out").front_center();
	CD2501E(config, m_vsp, 640000L);

	m_vsp->ready_cb().set(FUNC(ti_speech_synthesizer_device::speech_ready));
	m_vsp->add_route(ALL_OUTPUTS, "speech_out", 0.50);

	TMS6100(config, "vsm", 0);
	m_vsp->m0_cb().set("vsm", FUNC(tms6100_device::m0_w));
	m_vsp->m1_cb().set("vsm", FUNC(tms6100_device::m1_w));
	m_vsp->addr_cb().set("vsm", FUNC(tms6100_device::add_w));
	m_vsp->data_cb().set("vsm", FUNC(tms6100_device::data_line_r));
	m_vsp->romclk_cb().set("vsm", FUNC(tms6100_device::clk_w));
}

INPUT_PORTS_START( ti99_speech )
	PORT_START( "AMADECODE" )
	PORT_CONFNAME( 0x01, 0x01, "Decode AMA/AMB/AMC lines" )
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( 0x01, DEF_STR( On ))
INPUT_PORTS_END

ioport_constructor ti_speech_synthesizer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ti99_speech);
}

const tiny_rom_entry *ti_speech_synthesizer_device::device_rom_region() const
{
	return ROM_NAME( ti99_speech );
}

} // end namespace bus::ti99::peb

