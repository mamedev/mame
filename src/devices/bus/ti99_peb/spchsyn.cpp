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

#include "spchsyn.h"
#include "sound/wave.h"
#include "machine/spchrom.h"

#define TRACE_MEM 0
#define TRACE_ADDR 0
#define TRACE_READY 0

/****************************************************************************/

ti_speech_synthesizer_device::ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_SPEECH, "TI-99 Speech synthesizer (on adapter card)", tag, owner, clock, "ti99_speech", __FILE__),
	m_vsp(nullptr), m_reading(false), m_sbe(false)
{
}

/*
    Memory read
*/

READ8Z_MEMBER( ti_speech_synthesizer_device::readz )
{
	if (space.debugger_access()) return;

	if (m_sbe)
	{
		*value = m_vsp->status_r(space, 0, 0xff) & 0xff;
		if (TRACE_MEM) logerror("read value = %02x\n", *value);
		// We should clear the lines at this point. The TI-99/4A clears the
		// lines by setting the address bus to a different value, but the
		// Geneve may behave differently. This may not 100% reflect the real
		// situation, but it ensures a safe processing.
		m_vsp->combined_rsq_wsq_w(space, 0, ~0);
	}
}

/*
    Memory write
*/
WRITE8_MEMBER( ti_speech_synthesizer_device::write )
{
	if (space.debugger_access()) return;

	if (m_sbe)
	{
		if (TRACE_MEM) logerror("write value = %02x\n", data);
		m_vsp->data_w(space, 0, data);
		// Note that we must NOT clear the lines here. Find the lines in the
		// READY callback below.
	}
}

SETADDRESS_DBIN_MEMBER( ti_speech_synthesizer_device::setaddress_dbin )
{
	// 1001 00xx xxxx xxx0   DBIN=1
	// 1001 01xx xxxx xxx0   DBIN=0
	// 1111 1000 0000 0001    mask
	m_space = &space;
	m_reading = (state==ASSERT_LINE);

	bool valid = (((offset & 0x0400)==0) == m_reading);
	m_sbe = ((offset & m_select_mask)==m_select_value) && valid;

	if (m_sbe)
	{
		if (TRACE_ADDR) logerror("set address = %04x, dbin = %d\n", offset, state);

		// Caution: In the current tms5220 emulation, care must be taken
		// to clear one line before asserting the other line, or otherwise
		// both RS* and WS* are active, which is illegal.
		// Alternatively, we'll use the combined settings method

		m_vsp->combined_rsq_wsq_w(space, 0, m_reading? ~RS : ~WS);
	}
	else
		// If other address, turn off RS* and WS* (negative logic!)
		m_vsp->combined_rsq_wsq_w(space, 0, ~0);
}

/****************************************************************************/

WRITE_LINE_MEMBER( ti_speech_synthesizer_device::speech_ready )
{
	// The TMS5200 implementation uses TRUE/FALSE, not ASSERT/CLEAR semantics
	// and we have to adapt a /READY to a READY line.
	// The real synthesizer board uses a transistor for that purpose.
	m_slot->set_ready((state==0)? ASSERT_LINE : CLEAR_LINE);
	if (TRACE_READY) logerror("READY = %d\n", (state==0));

	if ((state==0) && !m_reading)
		// Clear the lines only when we are done with writing.
		m_vsp->combined_rsq_wsq_w(*m_space, 0, ~0);
}

void ti_speech_synthesizer_device::device_start()
{
}

void ti_speech_synthesizer_device::device_config_complete()
{
	m_vsp = subdevice<cd2501e_device>("speechsyn");

	// Need to configure the speech ROM for inverse bit order
	speechrom_device* mem = subdevice<speechrom_device>("vsm");
	mem->set_reverse_bit_order(true);
}

void ti_speech_synthesizer_device::device_reset()
{
	if (m_genmod)
	{
		m_select_mask = 0x1ff801;
		m_select_value = 0x179000;
	}
	else
	{
		m_select_mask = 0x7f801;
		m_select_value = 0x79000;
	}

	m_reading = false;
	m_sbe = false;
}

MACHINE_CONFIG_FRAGMENT( ti99_speech )
	MCFG_DEVICE_ADD("vsm", SPEECHROM, 0)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speechsyn", CD2501E, 640000L)
	MCFG_TMS52XX_READYQ_HANDLER(WRITELINE(ti_speech_synthesizer_device, speech_ready))
	MCFG_TMS52XX_SPEECHROM("vsm")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( ti99_speech )
	ROM_REGION(0x8000, "vsm", 0)
	ROM_LOAD("cd2325a.u2a", 0x0000, 0x4000, CRC(1f58b571) SHA1(0ef4f178716b575a1c0c970c56af8a8d97561ffe)) // at location u2, bottom of stack
	ROM_LOAD("cd2326a.u2b", 0x4000, 0x4000, CRC(65d00401) SHA1(a367242c2c96cebf0e2bf21862f3f6734b2b3020)) // at location u2, top of stack
ROM_END

machine_config_constructor ti_speech_synthesizer_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti99_speech );
}

const rom_entry *ti_speech_synthesizer_device::device_rom_region() const
{
	return ROM_NAME( ti99_speech );
}
const device_type TI99_SPEECH = &device_creator<ti_speech_synthesizer_device>;
