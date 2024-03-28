// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    GROM port - the cartridge port of the TI-99/4, TI-99/4A, and
    TI-99/8 console.

    The name refers to the main intended application scenario, that is,
    to host cartridges with GROMs. The second, wider port of the console is
    called I/O port and connects to the Peripheral Expansion System
    (see peribox.h).

          LEFT

  /RESET  1||2   GND
      D7  3||4   CRUCLK
      D6  5||6   CRUIN
      D5  7||8   A15/CRUOUT
      D4  9||10  A13
      D3 11||12  A12
      D2 13||14  A11
      D1 15||16  A10
      D0 17||18  A9
     +5V 19||20  A8
     /GS 21||22  A7
     A14 23||24  A3
    DBIN 25||26  A6
  GRMCLK 27||28  A5
     -5V 29||30  A4
   READY 31||32  /WE
     GND 33||34  /ROMG
     GND 35||36  GND

         RIGHT

    Address bus line ordering, according to TI convention, is A0 (MSB) ... A15 (LSB).
    A0, A1, and A2 are not delivered to the port but decoded in the console:

    /ROMG is asserted for A0/A1/A2 = 011 (addresses 6000 - 7fff)
    /GS is asserted for A0...A5 = 100110 (addresses 9800 - 9bff)

    This means that a maximum of 8 KiB of direct memory space can be accessed.
    The /GS line is used to enable GROM circuits on the board (serial ROMs with
    own address counter, see tmc0430.h).

    When a cartridge is inserted the /RESET line is pulled to ground, which
    via a R/C component pulls down the /RESET input of the timer circuit for
    a short time, which in turn resets the CPU. In order to dump cartridges,
    a common procedure was to tape the /RESET line of the cartridge. However,
    inserting a cartridge without resetting often caused so much data bus noise
    that the console usually locked up.

    ----------------

    The TI-99/4A computer was strictly designed for cartridge usage. The basic
    console had only little directly accessible RAM and offered no ways to
    write machine language programs; cartridges were intended to add various
    capabilities, or just for running games.

    Beside the seemingly simple handling, Texas Instruments had own intentions
    behind their cartridge strategy. With only 8 KiB of direct access memory, a
    major part of the cartridge code had to be stored in GROMs, which had to be
    licensed from Texas Instruments. Thus they kept firm control over all
    software development.

    Over the years, and with the increasingly difficult market situations,
    TI's policies seem to have changed. This may be the reason that the built-in
    operating system actually allowed for running ROM-only cartridges until TI
    clipped out this part in the OS, banning cartridges without GROMs. Consoles
    with this modification were produced in 1983, TI's last year in the home
    computer business.

    Although only 8 KiB were available for direct addressing, clever techniques
    were invented by third-party manufacturers. The first extension was utilized
    by TI themselves in the Extended Basic cartridge which offers two banks
    of ROM contents. Switching between the banks is achieved by writing a value
    to the ROM space at 6000 or 6002. Later, cartridges with much more memory
    space were created, up to the Super Space II cartridge with 128 KiB of
    buffered SRAM.

    ----------------

    From the console case layout the GROM port was intended for a single
    cartridge only. Although never officially released, the operating system
    of the TI console supported a multi-cartridge extender with software
    switching. There were also extenders based on hardware switching (like
    the Navarone Widget).

    This emulation offers both variants as slot options:

    -gromport single   : default single cartridge connector
    -gromport multi    : software-switchable 4-slot cartridge extender
    -gromport gkracker : GRAM Kracker

    The last option enables another popular device, the GRAM Kracker. This is
    a device to be plugged into the cartridge slot with five manual switches
    at the front and an own cartridge slot at its top. It contains buffered
    SRAM, a built-in ROM, and a GROM simulator which simulates GROM behaviour
    when accessing the buffered RAM. Its main use is to provide editable
    storage for the read-only cartridge contents. Cartridges can be plugged
    into its slot; their contents can be read and written to disk, modified as
    needed, and loaded into the buffered RAM. Even the console GROMs can be
    copied into the device; despite running in parallel, the GROM simulator
    is able to override the console GROMs, thus allowing the user to install
    a customized OS.

***************************************************************************/
#include "emu.h"
#include "gromport.h"
#include "emuopts.h"
#include "image.h"
#include "softlist.h"

#include "singleconn.h"
#include "multiconn.h"
#include "gkracker.h"

#define LOG_WARN         (1U << 1)   // Warnings
#define LOG_READ         (1U << 2)   // Reading
#define LOG_WRITE        (1U << 3)   // Writing

#define VERBOSE (LOG_WARN)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_GROMPORT, bus::ti99::gromport::gromport_device, "gromport", "TI-99 Cartridge port")

namespace bus::ti99::gromport {

gromport_device::gromport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:   device_t(mconfig, TI99_GROMPORT, tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_connector(nullptr),
		m_reset_on_insert(true),
		m_console_ready(*this),
		m_console_reset(*this),
		m_mask(0x1fff)
{ }

/*
    Reading via the GROM port. Only 13 address lines are passed through
    on the TI-99/4A, and 14 lines on the TI-99/8.
*/
void gromport_device::readz(offs_t offset, uint8_t *value)
{
	if (m_connector != nullptr)
	{
		m_connector->readz(offset & m_mask, value);
		if (m_romgq) LOGMASKED(LOG_READ, "Read %04x -> %02x\n", offset | 0x6000, *value);
	}
}

/*
    Writing via the GROM port. Only 13 address lines are passed through
    on the TI-99/4A, and 14 lines on the TI-99/8.
*/
void gromport_device::write(offs_t offset, uint8_t data)
{
	if (m_connector != nullptr)
	{
		if (m_romgq) LOGMASKED(LOG_WRITE, "Write %04x <- %02x\n", offset | 0x6000, data);
		m_connector->write(offset & m_mask, data);
	}
}

void gromport_device::crureadz(offs_t offset, uint8_t *value)
{
	if (m_connector != nullptr)
		m_connector->crureadz(offset, value);
}

void gromport_device::cruwrite(offs_t offset, uint8_t data)
{
	if (m_connector != nullptr)
		m_connector->cruwrite(offset, data);
}

void gromport_device::ready_line(int state)
{
	m_console_ready(state);
}

/*
    Asserted when the console addresses cartridge rom.
*/
void gromport_device::romgq_line(int state)
{
	m_romgq = state;
	if (m_connector != nullptr)
		m_connector->romgq_line(state);
}

void gromport_device::gclock_in(int state)
{
	if (m_connector != nullptr)
		m_connector->gclock_in(state);
}

/*
    Combined GROM control lines.
*/
void gromport_device::set_gromlines(line_state mline, line_state moline, line_state gsq)
{
	if (m_connector != nullptr)
		m_connector->set_gromlines(mline, moline, gsq);
}

void gromport_device::device_start()
{

	save_item(NAME(m_romgq));
}

void gromport_device::device_reset()
{
	m_reset_on_insert = (ioport("CARTRESET")->read()==0x01);
}

/*
    Shall we reset the console when a cartridge has been inserted?
    This is triggered by the cartridge by pulling down /RESET via a capacitor.
    Accordingly, when we put a tape over the /RESET contact we can avoid the
    reset, which is useful when we want to swap the cartridges while a program
    is runnning.
*/
void gromport_device::cartridge_inserted()
{
	if (m_reset_on_insert)
	{
		m_console_reset(ASSERT_LINE);
		m_console_reset(CLEAR_LINE);
	}
}

/*
    Find out whether the GROMs in the cartridge are idle. In that case,
    cut the clock line.
*/
bool gromport_device::is_grom_idle()
{
	if (m_connector != nullptr)
		return m_connector->is_grom_idle();
	else
		return false;
}

void gromport_device::device_config_complete()
{
	if (subdevices().first() != nullptr)
	{
		m_connector = dynamic_cast<cartridge_connector_device*>(subdevices().first());
		if (m_connector == nullptr)
			throw emu_fatalerror("gromport_device: cartridge connector not found");
	}
}

INPUT_PORTS_START(gromport)
	PORT_START( "CARTRESET" )
	PORT_CONFNAME( 0x01, 0x01, "RESET on cartridge insert" )
		PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
		PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor gromport_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gromport);
}

/***************************************************************************
    Different versions of cartridge connections

    single: the standard console connector, one cartridge
    multi:  a multi-cart expander, up to 4 cartridges with software selection
    gkracker: GRAMKracker, a device with NVRAM which allows the user to copy
              the contents of the cartridge plugged into its slot into the NVRAM
              and to modify it.

***************************************************************************/

cartridge_connector_device::cartridge_connector_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_gromport(nullptr)
{
}

void  cartridge_connector_device::ready_line(int state)
{
	m_gromport->ready_line(state);
}

void cartridge_connector_device::device_config_complete()
{
	m_gromport = dynamic_cast<gromport_device*>(owner());
	// owner is the empty_state during -listxml, so this will be nullptr
}

} // end namespace bus::ti99::gromport

void ti99_gromport_options(device_slot_interface &device)
{
	device.option_add("single", TI99_GROMPORT_SINGLE);
	device.option_add("multi", TI99_GROMPORT_MULTI);
	device.option_add("gkracker", TI99_GROMPORT_GK);
}

void ti99_gromport_options_998(device_slot_interface &device)
{
	device.option_add("single", TI99_GROMPORT_SINGLE);
	device.option_add("multi", TI99_GROMPORT_MULTI);
}
