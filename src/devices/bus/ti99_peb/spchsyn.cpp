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

#define TMS5220_ADDRESS_MASK 0x3FFFFUL  /* 18-bit mask for tms5220 address */

#define VERBOSE 1
#define LOG logerror

/****************************************************************************/

ti_speech_synthesizer_device::ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_SPEECH, "TI-99 Speech synthesizer (on adapter card)", tag, owner, clock, "ti99_speech", __FILE__), m_vsp(nullptr), m_read_mode(false)
{
}

/*
    Memory read
*/

READ8Z_MEMBER( ti_speech_synthesizer_device::readz )
{
	if (space.debugger_access()) return;

	if ((offset & m_select_mask)==m_select_value)
	{
		*value = m_vsp->status_r(space, offset, 0xff) & 0xff;
		if (VERBOSE>4) LOG("spchsyn: read value = %02x\n", *value);
		// We should clear the lines at this point. The TI-99/4A clears the
		// lines by setting the address bus to a different value, but the
		// Geneve may behave differently. This may not 100% reflect the real
		// situation, but it ensures a safe processing.
		m_vsp->rsq_w(TRUE);
		m_vsp->wsq_w(TRUE);
	}
}

/*
    Memory write
*/
WRITE8_MEMBER( ti_speech_synthesizer_device::write )
{
	if (space.debugger_access()) return;

	if ((offset & m_select_mask)==(m_select_value | 0x0400))
	{
		if (VERBOSE>4) LOG("spchsyn: write value = %02x\n", data);
		m_vsp->data_w(space, offset, data);
		// Note that we must NOT clear the lines here. Find the lines in the
		// READY callback below.
	}
}

SETADDRESS_DBIN_MEMBER( ti_speech_synthesizer_device::setaddress_dbin )
{
	if ((offset & m_select_mask & ~0x0400)==m_select_value)
	{
		if (VERBOSE>4) LOG("spchsyn: set address = %04x, dbin = %d\n", offset, state);
		m_read_mode = (state==ASSERT_LINE);
		bool readop = (offset & 0x0400)==0;

		if (m_read_mode != readop)
		{
			// reset all; this is not a valid access
			m_vsp->rsq_w(TRUE);
			m_vsp->wsq_w(TRUE);
		}
		else
		{
			if (readop)
			{
				// Caution: We MUST first clear (TRUE) one line to avoid
				// both RS* and WS* be asserted (otherwise tms5220 will report "illegal")
				m_vsp->wsq_w(TRUE);
				m_vsp->rsq_w(FALSE);
			}
			else
			{
				m_vsp->rsq_w(TRUE);
				m_vsp->wsq_w(FALSE);
			}
		}
	}
	else
	{
		// If other address, turn off RS* and WS* (negative logic!)
		m_vsp->rsq_w(TRUE);
		m_vsp->wsq_w(TRUE);
		return;
	}
}

/****************************************************************************/

WRITE_LINE_MEMBER( ti_speech_synthesizer_device::speech_ready )
{
	// The TMS5200 implementation uses TRUE/FALSE, not ASSERT/CLEAR semantics
	// and we have to adapt a /READY to a READY line.
	// The real synthesizer board uses a transistor for that purpose.
	m_slot->set_ready((state==0)? ASSERT_LINE : CLEAR_LINE);
	if (VERBOSE>5) LOG("spchsyn: READY = %d\n", (state==0));

	if ((state==0) && !m_read_mode)
	{
		// Clear the lines only when we are done with writing.
		m_vsp->rsq_w(TRUE);
		m_vsp->wsq_w(TRUE);
	}
}

void ti_speech_synthesizer_device::device_start()
{
	m_read_mode = false;
}

void ti_speech_synthesizer_device::device_config_complete()
{
	m_vsp = subdevice<cd2501e_device>("speechsyn");
}

void ti_speech_synthesizer_device::device_reset()
{
	if (VERBOSE>5) LOG("spchsyn: reset\n");
	if (m_genmod)
	{
		m_select_mask = 0x1ffc01;
		m_select_value = 0x179000;
	}
	else
	{
		m_select_mask = 0x7fc01;
		m_select_value = 0x79000;
	}
	m_read_mode = false;
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
	// Note: the following line is actually wrong; the speech roms in the ti 99/4a and 99/8 are two VSM roms labeled CD2325A and CD2326A, and contain the same data as the following line rom does, but with the byte bit order reversed. This bit ordering issue needs to be fixed elsewhere in the code here before the original/real roms can be used.
	ROM_LOAD_OPTIONAL("spchrom.bin", 0x0000, 0x8000, CRC(58b155f7) SHA1(382292295c00dff348d7e17c5ce4da12a1d87763)) /* system speech ROM */
	// correct lines are:
	// ROM_LOAD_OPTIONAL("cd2325a.u2a", 0x0000, 0x4000, CRC(1f58b571) SHA1(0ef4f178716b575a1c0c970c56af8a8d97561ffe)) // at location u2, bottom of stack
	// ROM_LOAD_OPTIONAL("cd2326a.u2b", 0x4000, 0x4000, CRC(65d00401) SHA1(a367242c2c96cebf0e2bf21862f3f6734b2b3020)) // at location u2, top of stack
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
