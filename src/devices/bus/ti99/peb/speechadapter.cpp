// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Speech synthesizer adapter device

    This adapter card made the sidecar Speech Synthesizer unit available to
    systems without a TI-99/4A console, like the SGCPU and the Geneve. The
    board must be removed from the sidecar unit and plugged on this simple
    adapter board, which is then put in a slot of the Peripheral Expansion Box.
    
                                      +-------+
                                +-----|-------|---+
                                |    Speech syn   |
                                |    board        |
                                |     _________   |
                                +-----+++++++++---+
         +----------------------------+|||||||+------------+
         |                             -------             |
         |        Adapter board                            |
         |                                                 |
      (((o LED              PEB slot connector             |
         +--------------|||||||||||||||||||||||||----------+                            
                        |||||||||||||||||||||||||
    
    Technical detail:
                        
    The SBE signal (Speech Block Enable), which is generated in the TI console,
    is not forwarded to the PEB. One of the tasks of this board is thus to 
    decode the mapped addresses once more and to activate the synthesizer 
    accordingly.
    
    A second issue for Geneve users is that the address extension bits (AMA,
    AMB, AMC) need to be decoded as well to avoid the synthesizer interfering
    with other memory access. This can be activated in the configuration. The
    default is on.
   
    Michael Zapf
    March 2025

*****************************************************************************/

#include "emu.h"
#include "speechadapter.h"

#include "speaker.h"
#include "bus/ti99/sidecar/speechsyn.h"

#define LOG_WARN        (1U << 1)    // Warnings
#define LOG_CONFIG      (1U << 2)
#define LOG_MEM         (1U << 3)
#define LOG_ADDR        (1U << 4)
#define LOG_READY       (1U << 5)

#define VERBOSE (LOG_CONFIG | LOG_WARN)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_SPEECHADAPTER, bus::ti99::peb::ti_speechsyn_adapter_device, "ti99_speechconn", "TI-99 Speech synthesizer adapter card")

namespace bus::ti99::peb {

#define PORT "conn"

/****************************************************************************/

ti_speechsyn_adapter_device::ti_speechsyn_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_SPEECHADAPTER, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_port(*this, PORT),
	m_dec_high(false)
{
}

void ti_speechsyn_adapter_device::readz(offs_t offset, uint8_t *value)
{
	if (m_port != nullptr)
	{
		LOGMASKED(LOG_MEM, "read %06x\n", offset);
		m_port->readz(offset, value);
	}
}

void ti_speechsyn_adapter_device::write(offs_t offset, uint8_t data)
{
	if (m_port != nullptr)
	{
		LOGMASKED(LOG_MEM, "write %06x\n", offset);
		m_port->write(offset, data);
	}
}

void ti_speechsyn_adapter_device::setaddress_dbin(offs_t offset, int state)
{
	// Valid access = not(DBIN and A5)
	bool reading = (state==ASSERT_LINE);
	
	// An access is valid when reading from 9000 and writing to 9400. 
	bool valid = (((offset & 0x0400)==0) == reading);
	bool sbe = false;
	
	// Recreate the SBE signal that is only available at the I/O port of the console
	if (m_dec_high)
		// We need to decode the AMA/AMB/AMC address extension lines
		sbe = ((offset & 0x7f801)==0x79000) && valid;
	else
		// No need to decode the extension lines
		sbe = ((offset & 0x0f801)==0x09000) && valid;

	if (sbe) LOGMASKED(LOG_ADDR, "set address = %04x, dbin = %d\n", offset, state);

	if (m_port != nullptr)
	{
		m_port->sbe(sbe);
		m_port->setaddress_dbin(offset, state);
	}
}

void ti_speechsyn_adapter_device::ready(int state)
{
	LOGMASKED(LOG_READY, "Incoming READY=%d from synthesizer %d\n", state);
	m_slot->set_ready(state);
}

void ti_speechsyn_adapter_device::device_start()
{
}

void ti_speechsyn_adapter_device::device_reset()
{
	m_dec_high = (ioport("AMADECODE")->read()!=0);
	LOGMASKED(LOG_CONFIG, "Speech adapter%s decoding the AMA/B/C lines.\n",  m_dec_high? "" : " not"); 
}

void ti_speechsyn_adapter_options(device_slot_interface &device)
{
	device.option_add("speechsyn", TI99_SPEECHSYN);
}

void ti_speechsyn_adapter_device::device_add_mconfig(machine_config& config)
{
	TI99_IOPORT(config, m_port, 0, ti_speechsyn_adapter_options, "speechsyn");
	m_port->ready_cb().set(FUNC(ti_speechsyn_adapter_device::ready));
}

INPUT_PORTS_START( ti99_speechadapter )
	PORT_START( "AMADECODE" )
	PORT_CONFNAME( 0x01, 0x01, "Decode AMA/AMB/AMC lines" )
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( 0x01, DEF_STR( On ))
INPUT_PORTS_END

ioport_constructor ti_speechsyn_adapter_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ti99_speechadapter);
}

} // end namespace bus::ti99::peb

