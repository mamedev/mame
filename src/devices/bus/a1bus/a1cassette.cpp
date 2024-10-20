// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cassette.c

    Apple I Cassette Interface

*********************************************************************/

#include "emu.h"
#include "a1cassette.h"

#include "imagedev/cassette.h"

#include "speaker.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define CASSETTE_ROM_REGION "casrom"

ROM_START( cassette )
	// 256-byte cassette interface ROM, in two 82s129 or mmi6301 256x4 proms at locations 3 and 4 on the cassette interface daughtercard (they are labeled "MMI 6301-IJ // 7623L // APPLE-A3" and "MMI 6301-IJ // 7623L // APPLE-A4")
	ROM_REGION(0x100, CASSETTE_ROM_REGION, 0)
	ROM_LOAD_NIB_HIGH( "apple-a3.3",    0x0000, 0x0100, CRC(6eae8f52) SHA1(71906932727ef70952ef6afe6b08708df15cd67d) )
	ROM_LOAD_NIB_LOW( "apple-a4.4",    0x0000, 0x0100, CRC(94efa977) SHA1(851f3bd6863859a1a6909179a5e5bf744b3d807e) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a1bus_cassette_device:
	public device_t,
	public device_a1bus_card_interface
{
public:
	// construction/destruction
	a1bus_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t cassette_r(offs_t offset);
	void cassette_w(offs_t offset, uint8_t data);

protected:
	a1bus_cassette_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;


	void cassette_toggle_output();

	optional_device<cassette_image_device> m_cassette;

private:
	required_region_ptr<uint8_t> m_rom;
	int m_cassette_output_flipflop;
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

/* sound output */

void a1bus_cassette_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("apple1_cass");
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.08);
}

const tiny_rom_entry *a1bus_cassette_device::device_rom_region() const
{
	return ROM_NAME( cassette );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a1bus_cassette_device::a1bus_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a1bus_cassette_device(mconfig, A1BUS_CASSETTE, tag, owner, clock)
{
}

a1bus_cassette_device::a1bus_cassette_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a1bus_card_interface(mconfig, *this)
	, m_cassette(*this, "cassette")
	, m_rom(*this, CASSETTE_ROM_REGION)
	, m_cassette_output_flipflop(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1bus_cassette_device::device_start()
{
	install_device(0xc000, 0xc0ff, read8sm_delegate(*this, FUNC(a1bus_cassette_device::cassette_r)), write8sm_delegate(*this, FUNC(a1bus_cassette_device::cassette_w)));
	install_bank(0xc100, 0xc1ff, &m_rom[0]);

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

uint8_t a1bus_cassette_device::cassette_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
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
			return m_rom[(offset & ~1)];
		else
			return m_rom[offset];
	}
}

void a1bus_cassette_device::cassette_w(offs_t offset, uint8_t data)
{
	/* Writes toggle the output flip-flop in the same way that reads
	   do; other than that they have no effect.  Any repeated accesses
	   to the cassette I/O address range can be used to write data to
	   cassette, and the cassette ROM always uses reads to do this.
	   However, we still have to handle writes, since they may be done
	   by user code. */

	cassette_toggle_output();
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A1BUS_CASSETTE, device_a1bus_card_interface, a1bus_cassette_device, "a1cass", "Apple I cassette board")
