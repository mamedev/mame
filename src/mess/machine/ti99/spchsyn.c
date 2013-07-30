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

#define REAL_TIMING 0

/****************************************************************************/

ti_speech_synthesizer_device::ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_SPEECH, "TI-99 Speech synthesizer (on adapter card)", tag, owner, clock, "ti99_speech", __FILE__)
{
}

/*
    Comments on real timing in the TMS5200 emulation

    Real timing means that the synthesizer clears the /READY line (puts high)
    whenever a read or write access is in progress. This is done by setting
    /RS and /WS lines, according to the read or write operation. The /READY line
    is asserted again after some time. Real timing is used once the tms5220_wsq_w
    and tms5220_rsq_w are called.

    Within the TI systems, the /RS and /WS lines are controlled directly by the
    address bus. There is currently no way to insert wait states between
    the address setting and the data bus sampling, since this is an atomic
    operation in the emulator (read_byte). It would be necessary to somehow
    announce the pending read before it actually happens so that devices
    like this VSP may insert wait states before the read.

    The TMS5220 implementation assumes that wait states are respected and
    therefore delivers bad values when queried too early. It uses a latch that
    gets the new value after some time has expired.

    To fix this we have to modify the RS/WS methods in TMS5200 to immediately
    set the status (after updating the sound status, or the status will be
    outdated too early).

    Also note that the /RS and /WS lines must be cleared (put to high) when the
    read is done. Again, this is not possible with the current implementation.
    So we do this in the ready callback.

    On the bottom line we will stay with the not-REAL_TIMING for now and wait
    for the core to allow for split read accesses.
*/


/*
    Memory read
*/
#if REAL_TIMING
// ======  This is the version with real timing =======
READ8Z_MEMBER( ti_speech_synthesizer_device::readz )
{
	if ((offset & m_select_mask)==m_select_value)
	{
		m_vsp->wsq_w(TRUE);
		m_vsp->rsq_w(FALSE);
		*value = m_vsp->read(offset) & 0xff;
		if (VERBOSE>4) LOG("spchsyn: read value = %02x\n", *value);
	}
}

/*
    Memory write
*/
WRITE8_MEMBER( ti_speech_synthesizer_device::write )
{
	if ((offset & m_select_mask)==(m_select_value | 0x0400))
	{
		m_vsp->rsq_w(m_vsp, TRUE);
		m_vsp->wsq_w(m_vsp, FALSE);
		if (VERBOSE>4) LOG("spchsyn: write value = %02x\n", data);
		m_vsp->write(offset, data);
	}
}

#else
// ======  This is the version without real timing =======

READ8Z_MEMBER( ti_speech_synthesizer_device::readz )
{
	if ((offset & m_select_mask)==m_select_value)
	{
		machine().device("maincpu")->execute().adjust_icount(-(18+3));      /* this is just a minimum, it can be more */
		*value = m_vsp->status_r(space, offset, 0xff) & 0xff;
		if (VERBOSE>4) LOG("spchsyn: read value = %02x\n", *value);
	}
}

/*
    Memory write
*/
WRITE8_MEMBER( ti_speech_synthesizer_device::write )
{
	if ((offset & m_select_mask)==(m_select_value | 0x0400))
	{
		machine().device("maincpu")->execute().adjust_icount(-(54+3));      /* this is just an approx. minimum, it can be much more */

		/* RN: the stupid design of the tms5220 core means that ready is cleared */
		/* when there are 15 bytes in FIFO.  It should be 16.  Of course, if */
		/* it were the case, we would need to store the value on the bus, */
		/* which would be more complex. */
		if (!m_vsp->readyq_r())
		{
			attotime time_to_ready = attotime::from_double(m_vsp->time_to_ready());
			int cycles_to_ready = machine().device<cpu_device>("maincpu")->attotime_to_cycles(time_to_ready);
			if (VERBOSE>8) LOG("spchsyn: time to ready: %f -> %d\n", time_to_ready.as_double(), (int) cycles_to_ready);

			machine().device("maincpu")->execute().adjust_icount(-cycles_to_ready);
			machine().scheduler().timer_set(attotime::zero, FUNC_NULL);
		}
		if (VERBOSE>4) LOG("spchsyn: write value = %02x\n", data);
		m_vsp->data_w(space, offset, data);
	}
}
#endif

/****************************************************************************/

WRITE_LINE_MEMBER( ti_speech_synthesizer_device::speech_ready )
{
	// The TMS5200 implementation uses TRUE/FALSE, not ASSERT/CLEAR semantics
	// and we have to adapt a /READY to a READY line.
	// The real synthesizer board uses a transistor for that purpose.
	m_slot->set_ready((state==0)? ASSERT_LINE : CLEAR_LINE);
	if (VERBOSE>5) LOG("spchsyn: READY = %d\n", (state==0));

#if REAL_TIMING
	// Need to do that here (see explanations above)
	if (state==0)
	{
		m_vsp->rsq_w(TRUE);
		m_vsp->wsq_w(TRUE);
	}
#endif
}

void ti_speech_synthesizer_device::device_start()
{
}

void ti_speech_synthesizer_device::device_config_complete()
{
	m_vsp = subdevice<cd2501e_device>("speechsyn");
}

void ti_speech_synthesizer_device::device_reset()
{
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
