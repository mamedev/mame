// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cassette.c

    Apple I Cassette Interface

*********************************************************************/

#include "a1cassette.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define CASSETTE_ROM_REGION "casrom"

const device_type A1BUS_CASSETTE = &device_creator<a1bus_cassette_device>;

/* sound output */

MACHINE_CONFIG_FRAGMENT( cassette )
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
	MCFG_CASSETTE_INTERFACE("apple1_cass")
MACHINE_CONFIG_END

ROM_START( cassette )
	/* 256-byte cassette interface ROM, in two 82s129 or mmi6301 256x4 proms at locations 3 and 4 on the cassette interface daughtercard (they are labeled "MMI 6301-IJ // 7623L // APPLE-A3" and "MMI 6301-IJ // 7623L // APPLE-A4") */
	ROM_REGION(0x100, CASSETTE_ROM_REGION, 0)
	ROM_LOAD_NIB_HIGH( "apple-a3.3",    0x0000, 0x0100, CRC(6eae8f52) SHA1(71906932727ef70952ef6afe6b08708df15cd67d) )
	ROM_LOAD_NIB_LOW( "apple-a4.4",    0x0000, 0x0100, CRC(94efa977) SHA1(851f3bd6863859a1a6909179a5e5bf744b3d807e) )
ROM_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a1bus_cassette_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cassette );
}

const rom_entry *a1bus_cassette_device::device_rom_region() const
{
	return ROM_NAME( cassette );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a1bus_cassette_device::a1bus_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A1BUS_CASSETTE, "Apple I cassette board", tag, owner, clock, "a1cass", __FILE__),
		device_a1bus_card_interface(mconfig, *this),
		m_cassette(*this, "cassette")
{
}

a1bus_cassette_device::a1bus_cassette_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_a1bus_card_interface(mconfig, *this),
		m_cassette(*this, "cassette")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1bus_cassette_device::device_start()
{
	set_a1bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(CASSETTE_ROM_REGION).c_str())->base();

	install_device(0xc000, 0xc0ff, read8_delegate(FUNC(a1bus_cassette_device::cassette_r), this), write8_delegate(FUNC(a1bus_cassette_device::cassette_w), this));
	install_bank(0xc100, 0xc1ff, 0, 0, (char *)"bank_a1cas", m_rom);

	save_item(NAME(m_cassette_output_flipflop));
}

void a1bus_cassette_device::device_reset()
{
	m_cassette_output_flipflop = 0;
}

/*****************************************************************************
**  Cassette interface I/O
**
**  The Apple I's cassette interface was a small card that plugged
**  into the expansion connector on the motherboard.  (This was a
**  slot-type connector, separate from the motherboard's edge
**  connector, but with the same signals.)  The cassette interface
**  provided separate cassette input and output jacks, some very
**  simple interface hardware, and 256 bytes of ROM containing the
**  cassette I/O code.
**
**  The interface was mostly software-controlled.  The only hardware
**  was an output flip-flop for generating the cassette output signal,
**  a National Semiconductor LM311 voltage comparator for generating a
**  digital signal from the analog cassette input, an input
**  signal-level LED, and some gates to control the interface logic
**  and address decoding.  The cassette ROM code did most of the work
**  of generating and interpreting tape signals.  It also contained
**  its own mini-monitor for issuing tape read and write commands.
**
**  The cassette interface was assigned to the $C000-$CFFF block of
**  addresses, although it did not use most of the space in that
**  block.  Addresses were mapped as follows:
**
**      $C000-$C0FF:  Cassette I/O space.
**                    Any access here toggles the output signal.
**          $C000-$C07F:  Cassette output only; input disabled.
**                        Mirrors $C100-$C17F on reads.
**          $C080-$C0FF:  Cassette input and output.
**                        When input low, mirrors $C180-$C1FF on reads.
**                        When input high, both odd and even addresses
**                        mirror even ROM addresses $C180-$C1FE.
**      $C100-$C1FF:  Cassette ROM code.
**
**  Note the peculiar addressing scheme.  Data was written simply
**  through repeated accesses, rather than by writing to an address.
**  Data was read by reading an odd input address and comparing the
**  ROM byte returned to detect signal changes.
**
**  The standard tape signal was a simple square wave, although this
**  was often greatly distorted by the cassette recorder.  A single
**  tape record consisted of a 10-second 800-Hz leader, followed by a
**  single short square-wave cycle used as a sync bit, followed by the
**  tape data.  The data was encoded using a single square-wave cycle
**  for each bit; "1" bits were at 1000 Hz, "0" bits at 2000 Hz.  (All
**  of these frequencies are approximate and could vary due to
**  differences in recorder speed.)  Each byte was written starting
**  from the most significant bit; bytes were written from low to high
**  addresses.  No error detection was provided.  Multiple records
**  could be placed on a single tape.
*****************************************************************************/

/* The cassette output signal for writing tapes is generated by a
   flip-flop which is toggled to produce the output waveform.  Any
   access to the cassette I/O range, whether a read or a write,
   toggles this flip-flop. */
void a1bus_cassette_device::cassette_toggle_output()
{
	m_cassette_output_flipflop = !m_cassette_output_flipflop;
	m_cassette->output(m_cassette_output_flipflop ? 1.0 : -1.0);
}

READ8_MEMBER(a1bus_cassette_device::cassette_r)
{
	cassette_toggle_output();

	if (offset <= 0x7f)
	{
		/* If the access is to address range $C000-$C07F, the cassette
		   input signal is ignored .  In this case the value read
		   always comes from the corresponding cassette ROM location
		   in $C100-$C17F. */

		return m_rom[offset];
	}
	else
	{
		/* For accesses to address range $C080-$C0FF, the cassette
		   input signal is enabled.  If the signal is low, the value
		   read comes from the corresponding cassette ROM location in
		   $C180-$C1FF.  If the signal is high, the low bit of the
		   address is masked before the corresponding cassette ROM
		   location is accessed; e.g., a read from $C081 would return
		   the ROM byte at $C180.  The cassette ROM routines detect
		   changes in the cassette input signal by repeatedly reading
		   from $C081 and comparing the values read. */

		/* (Don't try putting a non-zero "noise threshhold" here,
		   because it can cause tape header bits on real cassette
		   images to be misread as data bits.) */
		if (m_cassette->input() > 0.0)
			return m_rom[0xc100 + (offset & ~1)];
		else
			return m_rom[0xc100 + offset];
	}
}

WRITE8_MEMBER(a1bus_cassette_device::cassette_w)
{
	/* Writes toggle the output flip-flop in the same way that reads
	   do; other than that they have no effect.  Any repeated accesses
	   to the cassette I/O address range can be used to write data to
	   cassette, and the cassette ROM always uses reads to do this.
	   However, we still have to handle writes, since they may be done
	   by user code. */

	cassette_toggle_output();
}
