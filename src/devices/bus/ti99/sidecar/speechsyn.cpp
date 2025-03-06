// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
    
    TI-99 Speech synthesizer

    This is the emulation Speech Synthesizer, which plugs into the I/O port of
    the TI console. Even though the sidecar expansion concept was largely
    abandoned by the introduction of the Peripheral Expansion Box with the 4A,
    the Speech Synthesizer was sold as a sidecar expansion until the end.
    
    Typical setup:
                        Speech Synthesizer
                        sidecar
                        v  
     +----------------+---+----------
     |   TI-99/4(A)   |   |    PEB connection cable
     +------------+---+   +----------
     | oooooooooo |   |---+
     | oooooooooo |   |
     +-----------------
    
    Also as a common modification, users removed the board inside the sidecar
    and placed it on an adapter to go into the PEB and thus to become
    available for the Geneve or SGCPU. See bus/ti99/peb/speechadapter.cpp.

    The sidecar offers a flippable lid where vocabulary expansion modules were 
    supposed to be plugged in, but those have never seen daylight. In most of 
    the units, including mine, no connector can be found under the lid, so the 
    decision to drop this idea must have come early. 

    Technical details:
    
    The Voice Synthesis Processor (VSP) used for the TI Speech Synthesizer 
    is the CD2501E, aka TMS5200 (internal name TMC0285), a predecessor of the 
    TMS5220 which was used in many other commercial products. 
    Two TMS6100 circuits hold the standard vocabulary, mainly used from Extended
    Basic programs.
    
    The interaction with the TMS5200 relies completely on the READY*
    line; the INT* line is not connected.

    The VSP delivers a READY* signal, which needs to be inverted for the rest
    of the TI system. The board of the Speech Synthesizer uses a simple 
    transistor for this purpose.
    
    Michael Zapf
    March 2025

*****************************************************************************/

#include "emu.h"
#include "speechsyn.h"
#include "speaker.h"

#define LOG_WARN        (1U << 1)   // Warnings
#define LOG_CONFIG      (1U << 2)
#define LOG_MEM         (1U << 3)
#define LOG_ADDR        (1U << 4)
#define LOG_READY       (1U << 5)

#define VERBOSE (LOG_GENERAL | LOG_WARN)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_SPEECHSYN, bus::ti99::sidecar::ti_speech_synthesizer_device, "ti99_speech", "TI-99 Speech Synthesizer")

namespace bus::ti99::sidecar {

#define VSP "tms5200"
#define PORT "extport"

/*
    Constructor called from subclasses.
*/
ti_speech_synthesizer_device::ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: 	bus::ti99::internal::ioport_attached_device(mconfig, TI99_SPEECHSYN, tag, owner, clock),
		m_vsp(*this, VSP),
		m_port(*this, PORT),
		m_reading(false),
		m_sbe(CLEAR_LINE),
		m_ext_ready(ASSERT_LINE),
		m_ssyn_ready(ASSERT_LINE)
{
}

/*
    Memory read
*/

void ti_speech_synthesizer_device::readz(offs_t offset, uint8_t *value)
{
	if (m_sbe)
	{
		if (machine().side_effects_disabled()) return;

		*value = m_vsp->status_r() & 0xff;
		LOGMASKED(LOG_MEM, "read value = %02x\n", *value);
		// We should clear the lines at this point. The TI-99/4A clears the
		// lines by setting the address bus to a different value, but the
		// Geneve may behave differently. This may not 100% reflect the real
		// situation, but it ensures a safe processing.
		m_vsp->combined_rsq_wsq_w(~0);
	}

	// Pass through to the external port
	if (m_port != nullptr)
		m_port->readz(offset, value);
}
/*
    Memory write
*/
void ti_speech_synthesizer_device::write(offs_t offset, uint8_t data)
{
	if (m_sbe)
	{
		if (machine().side_effects_disabled()) return;
		LOGMASKED(LOG_MEM, "write value = %02x\n", data);
		m_vsp->data_w(data);
		// Note that we must NOT clear the lines here. Find the lines in the
		// READY callback below.
	}

	// Pass through to the external port
	if (m_port != nullptr)
		m_port->write(offset, data);
}

void ti_speech_synthesizer_device::setaddress_dbin(offs_t offset, int state)
{
	if (m_sbe)
	{
		m_reading = (state==ASSERT_LINE);
		LOGMASKED(LOG_ADDR, "set address = %04x, dbin = %d\n", offset, state);

		// Caution: In the current tms5220 emulation, care must be taken
		// to clear one line before asserting the other line, or otherwise
		// both RS* and WS* are active, which is illegal.
		// Alternatively, we'll use the combined settings method

		m_vsp->combined_rsq_wsq_w(m_reading ? ~tms5220_device::RS : ~tms5220_device::WS);
	}
	else
	{
		// If other address, turn off RS* and WS* (negative logic!)
		m_vsp->combined_rsq_wsq_w(~0);
		
		// Pass through to the external port
		if (m_port != nullptr)
			m_port->setaddress_dbin(offset, state);
	}
}

void ti_speech_synthesizer_device::sbe(int state)
{
	m_sbe = (state==ASSERT_LINE);
	// Not forwarded to the external port
}

void ti_speech_synthesizer_device::crureadz(offs_t offset, uint8_t *value)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->crureadz(offset, value);
}

void ti_speech_synthesizer_device::cruwrite(offs_t offset, uint8_t data)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->cruwrite(offset, data);
}

void ti_speech_synthesizer_device::memen_in(int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->memen_in(state);
}

void ti_speech_synthesizer_device::msast_in(int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->msast_in(state);
}

void ti_speech_synthesizer_device::clock_in(int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->clock_in(state);
}

void ti_speech_synthesizer_device::reset_in(int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->reset_in(state);
}

/*
	Forward the incoming interrupt to the console
*/
void ti_speech_synthesizer_device::extint(int state)
{
	set_extint(state);
}

void ti_speech_synthesizer_device::extready(int state)
{
	LOGMASKED(LOG_READY, "Incoming READY=%d from external port\n", state);
	m_ext_ready = (line_state)state;
	set_ready(m_ext_ready & m_ssyn_ready);
}

void ti_speech_synthesizer_device::speech_ready(int state)
{
	// Invert the READY* signal	
	m_ssyn_ready = (state==0)? ASSERT_LINE : CLEAR_LINE; 
	LOGMASKED(LOG_READY, "SSyn READY = %d\n", (state==0));
	
	if ((state==0) && !m_reading)
		// Clear the lines only when we are done with writing.
		m_vsp->combined_rsq_wsq_w(~0);

	set_ready(m_ext_ready & m_ssyn_ready);
}

void ti_speech_synthesizer_device::device_start()
{
	save_item(NAME(m_reading));
	save_item(NAME(m_sbe));
}

void ti_speech_synthesizer_device::device_reset()
{
	m_reading = false;
	m_sbe = CLEAR_LINE;
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

	TI99_IOPORT(config, m_port, 0, ti99_ioport_options_evpc1, nullptr);
	m_port->extint_cb().set(FUNC(ti_speech_synthesizer_device::extint));
	m_port->ready_cb().set(FUNC(ti_speech_synthesizer_device::extready));
}

const tiny_rom_entry *ti_speech_synthesizer_device::device_rom_region() const
{
	return ROM_NAME( ti99_speech );
}

} // end namespace bus::ti99::sidecar
