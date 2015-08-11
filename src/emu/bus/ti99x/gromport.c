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
    own address counter, see grom.h).

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

    (Emulation detail: Take care when changing something in this emulation -
    this overrun is emulated by the sequence in which the devices on the datamux
    are executed.)

    Michael Zapf, July 2012

***************************************************************************/
#include "gromport.h"

#define VERBOSE 1
#define LOG logerror

#define GROM3_TAG "grom3"
#define GROM4_TAG "grom4"
#define GROM5_TAG "grom5"
#define GROM6_TAG "grom6"
#define GROM7_TAG "grom7"

#define CARTGROM_TAG "grom_contents"
#define CARTROM_TAG "rom_contents"
#define CARTROM2_TAG "rom2_contents"
#define GKRACKER_ROM_TAG "gkracker_rom"
#define GKRACKER_NVRAM_TAG "gkracker_nvram"

gromport_device::gromport_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:   bus8z_device(mconfig, GROMPORT, "Cartridge port", tag, owner, clock, "gromport", __FILE__),
		device_slot_interface(mconfig, *this),
		m_connector(NULL),
		m_reset_on_insert(true),
		m_console_ready(*this),
		m_console_reset(*this) { }

/* Only called for addresses 6000-7fff and GROM addresses (see datamux config) */
READ8Z_MEMBER(gromport_device::readz)
{
	if (m_connector != NULL)
		m_connector->readz(space, offset, value);
}

WRITE8_MEMBER(gromport_device::write)
{
	if (m_connector != NULL)
		m_connector->write(space, offset, data);
}

READ8Z_MEMBER(gromport_device::crureadz)
{
	if (m_connector != NULL)
		m_connector->crureadz(space, offset, value);
}

WRITE8_MEMBER(gromport_device::cruwrite)
{
	if (m_connector != NULL)
		m_connector->cruwrite(space, offset, data);
}

WRITE_LINE_MEMBER(gromport_device::ready_line)
{
	m_console_ready(state);
}

void gromport_device::device_start()
{
	m_console_ready.resolve();
	m_console_reset.resolve();
}

void gromport_device::device_reset()
{
	m_reset_on_insert = (ioport("CARTRESET")->read()==0x01);
}

void gromport_device::set_grom_base(UINT16 grombase, UINT16 grommask)
{
	m_grombase = grombase;
	m_grommask = grommask;
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

void gromport_device::device_config_complete()
{
	m_connector = static_cast<ti99_cartridge_connector_device*>(first_subdevice());
	set_grom_base(0x9800, 0xf800);
}

SLOT_INTERFACE_START( gromport )
	SLOT_INTERFACE("single", GROMPORT_SINGLE)
	SLOT_INTERFACE("multi", GROMPORT_MULTI)
	SLOT_INTERFACE("gkracker", GROMPORT_GK)
SLOT_INTERFACE_END


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

const device_type GROMPORT = &device_creator<gromport_device>;

/***************************************************************************
    Different versions of cartridge connections

    single: the standard console connector, one cartridge
    multi:  a multi-cart expander, up to 4 cartridges with software selection
    gkracker: GRAMKracker, a device with NVRAM which allows the user to copy
              the contents of the cartridge plugged into its slot into the NVRAM
              and to modify it.

***************************************************************************/

const device_type GROMPORT_SINGLE = &device_creator<single_conn_device>;
const device_type GROMPORT_MULTI = &device_creator<multi_conn_device>;
const device_type GROMPORT_GK = &device_creator<gkracker_device>;

ti99_cartridge_connector_device::ti99_cartridge_connector_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: bus8z_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_gromport(NULL)
{
}

WRITE_LINE_MEMBER( ti99_cartridge_connector_device::ready_line )
{
	m_gromport->ready_line(state);
}

void ti99_cartridge_connector_device::device_config_complete()
{
	m_gromport = static_cast<gromport_device*>(owner());
}

UINT16 ti99_cartridge_connector_device::grom_base()
{
	return m_gromport->get_grom_base();
}

UINT16 ti99_cartridge_connector_device::grom_mask()
{
	return m_gromport->get_grom_mask();
}

single_conn_device::single_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ti99_cartridge_connector_device(mconfig, GROMPORT_SINGLE, "Standard cartridge connector", tag, owner, clock, "single", __FILE__),
	m_cartridge(NULL)
{
}

READ8Z_MEMBER(single_conn_device::readz)
{
	// Pass through
	m_cartridge->readz(space, offset, value);
}

WRITE8_MEMBER(single_conn_device::write)
{
	// Pass through
	m_cartridge->write(space, offset, data);
}

READ8Z_MEMBER(single_conn_device::crureadz)
{
	// Pass through
	m_cartridge->crureadz(space, offset, value);
}

WRITE8_MEMBER(single_conn_device::cruwrite)
{
	// Pass through
	m_cartridge->cruwrite(space, offset, data);
}

void single_conn_device::device_start()
{
	m_cartridge = static_cast<ti99_cartridge_device*>(first_subdevice());
}

void single_conn_device::device_reset()
{
	m_cartridge->set_slot(0);
}

static MACHINE_CONFIG_FRAGMENT( single_slot )
	MCFG_DEVICE_ADD("cartridge", TI99CART, 0)
MACHINE_CONFIG_END

machine_config_constructor single_conn_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( single_slot );
}

/********************************************************/

/*
    The multi-cartridge extender

    This is a somewhat mythical device which was never available for the normal
    customer, but there are reports of the existence of such a device
    in development labs or demonstrations.

    The interesting thing about this is that the OS of the console
    fully supports this multi-cartridge extender, providing a selection
    option on the screen to switch between different plugged-in
    cartridges.

    The switching is possible by decoding address lines that are reserved
    for GROM access. GROMs are accessed via four separate addresses
    9800, 9802, 9C00, 9C02. The addressing scheme looks like this:

    1001 1Wxx xxxx xxM0        W = write(1), read(0), M = address(1), data(0)

    This leaves 8 bits (256 options) which are not decoded inside the
    console. As the complete address is routed to the port, some circuit
    just needs to decode the xxx lines and turn on the respective slot.

    One catch must be considered: Some cartridges contain ROMs which are
    directly accessed and not via ports. This means that the ROMs must
    be activated according to the slot that is selected.

    Another issue: Each GROM contains an own address counter and an ID.
    According to the ID the GROM only delivers data if the address counter
    is within the ID area (0 = 0000-1fff, 1=2000-3fff ... 7=e000-ffff).
    Thus it is essential that all GROMs stay in sync with their address
    counters. We have to route all address settings to all slots and their
    GROMs, even when the slot has not been selected before. The selected
    just shows its effect when data is read. In this case, only the
    data from the selected slot will be delivered.

    This may be considered as a design flaw within the complete cartridge system
    which eventually led to TI not manufacturing that device for the broad
    market.
*/

#define AUTO -1

multi_conn_device::multi_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ti99_cartridge_connector_device(mconfig, GROMPORT_MULTI, "Multi-cartridge extender", tag, owner, clock, "multi", __FILE__),
	m_active_slot(0),
	m_fixed_slot(0),
	m_next_free_slot(0)
{
}

/*
    Activates a slot in the multi-cartridge extender.
    Setting the slot is done by accessing the GROM ports using a
    specific address:

    slot 0:  0x9800 (0x9802, 0x9c00, 0x9c02)   : cartridge1
    slot 1:  0x9804 (0x9806, 0x9c04, 0x9c06)   : cartridge2
    ...
    slot 15: 0x983c (0x983e, 0x9c3c, 0x9c3e)   : cartridge16

    Scheme:

    1001 1Wxx xxxx xxM0 (M=mode; M=0: data, M=1: address; W=write)

    The following addresses are theoretically available, but the
    built-in OS does not use them; i.e. cartridges will not be
    included in the selection list, and their features will not be
    found by lookup, but they could be accessed directly by user
    programs.
    slot 16: 0x9840 (0x9842, 0x9c40, 0x9c42)
    ...
    slot 255:  0x9bfc (0x9bfe, 0x9ffc, 0x9ffe)

    Setting the GROM base should select one cartridge, but the ROMs in the
    CPU space must also be switched. As there is no known special mechanism
    we assume that by switching the GROM base, the ROM is automatically
    switched.

    Caution: This means that cartridges which do not have at least one
    GROM cannot be switched with this mechanism.

    We assume that the slot number is already calculated in the caller:
    slotnumber>=0 && slotnumber<=255

    NOTE: The OS will stop searching when it finds slots 1 and 2 empty.
    Interestingly, cartridge subroutines are found nevertheless, even when
    the cartridge is plugged into a higher slot.
*/
void multi_conn_device::set_slot(int slotnumber)
{
	if (VERBOSE>7)
		if (m_active_slot != slotnumber) LOG("multi_conn_device: Setting cartslot to %d\n", slotnumber);

	if (m_fixed_slot==AUTO)
		m_active_slot = slotnumber;
	else
		m_active_slot = m_fixed_slot;
}

int multi_conn_device::get_active_slot(bool changebase, offs_t offset)
{
	int slot;
	if (changebase)
	{
		if ((offset & grom_mask()) == grom_base())
		{
			set_slot((offset>>2) & 0x00ff);
		}
	}
	slot = m_active_slot;
	return slot;
}

void multi_conn_device::insert(int index, ti99_cartridge_device* cart)
{
	if (VERBOSE>3) LOG("multi_conn_device: insert slot %d\n", index);
	m_cartridge[index] = cart;
	m_gromport->cartridge_inserted();
}

void multi_conn_device::remove(int index)
{
	if (VERBOSE>3) LOG("multi_conn_device: remove slot %d\n", index);
	m_cartridge[index] = NULL;
}

READ8Z_MEMBER(multi_conn_device::readz)
{
	int slot = get_active_slot(true, offset);

	// If we have a GROM access, we need to send the read request to all
	// attached cartridges so the slot is irrelevant here. Each GROM
	// contains an internal address counter, and we must make sure they all stay in sync.
	if ((offset & grom_mask()) == grom_base())
	{
		for (int i=0; i < NUMBER_OF_CARTRIDGE_SLOTS; i++)
		{
			if (m_cartridge[i] != NULL)
			{
				UINT8 newval = *value;
				m_cartridge[i]->readz(space, offset, &newval, mem_mask);
				if (i==slot)
				{
					*value = newval;
				}
			}
		}
	}
	else
	{
		if (slot < NUMBER_OF_CARTRIDGE_SLOTS && m_cartridge[slot] != NULL)
		{
			m_cartridge[slot]->readz(space, offset, value, mem_mask);
		}
	}
}

WRITE8_MEMBER(multi_conn_device::write)
{
	int slot = get_active_slot(true, offset);

	// Same issue as above (read)
	// We don't have GRAM cartridges, anyway, so it's just used for setting the address.
	if ((offset & grom_mask()) == grom_base())
	{
		for (int i=0; i < NUMBER_OF_CARTRIDGE_SLOTS; i++)
		{
			if (m_cartridge[i] != NULL)
			{
				m_cartridge[i]->write(space, offset, data, mem_mask);
			}
		}
	}
	else
	{
		if (slot < NUMBER_OF_CARTRIDGE_SLOTS && m_cartridge[slot] != NULL)
		{
			//      LOG("try it on slot %d\n", slot);
			m_cartridge[slot]->write(space, offset, data, mem_mask);
		}
	}
}

READ8Z_MEMBER(multi_conn_device::crureadz)
{
	int slot = get_active_slot(false, offset);
	/* Sanity check. Higher slots are always empty. */
	if (slot >= NUMBER_OF_CARTRIDGE_SLOTS)
		return;

	if (m_cartridge[slot] != NULL)
	{
		m_cartridge[slot]->crureadz(space, offset, value);
	}
}

WRITE8_MEMBER(multi_conn_device::cruwrite)
{
	int slot = get_active_slot(true, offset);

	/* Sanity check. Higher slots are always empty. */
	if (slot >= NUMBER_OF_CARTRIDGE_SLOTS)
		return;

	if (m_cartridge[slot] != NULL)
	{
		m_cartridge[slot]->cruwrite(space, offset, data);
	}
}

void multi_conn_device::device_start()
{
	m_next_free_slot = 0;
	m_active_slot = 0;
	for (int i=0; i < NUMBER_OF_CARTRIDGE_SLOTS; i++)
	{
		m_cartridge[i] = NULL;
	}
}

void multi_conn_device::device_reset(void)
{
	m_active_slot = 0;
	m_fixed_slot = ioport("CARTSLOT")->read() - 1;
}

static MACHINE_CONFIG_FRAGMENT( multi_slot )
	MCFG_DEVICE_ADD("cartridge1", TI99CART, 0)
	MCFG_DEVICE_ADD("cartridge2", TI99CART, 0)
	MCFG_DEVICE_ADD("cartridge3", TI99CART, 0)
	MCFG_DEVICE_ADD("cartridge4", TI99CART, 0)
MACHINE_CONFIG_END

machine_config_constructor multi_conn_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( multi_slot );
}

INPUT_PORTS_START(multi_slot)
	PORT_START( "CARTSLOT" )
	PORT_DIPNAME( 0x0f, 0x00, "Multi-cartridge slot" )
		PORT_DIPSETTING(    0x00, "Auto" )
		PORT_DIPSETTING(    0x01, "Slot 1" )
		PORT_DIPSETTING(    0x02, "Slot 2" )
		PORT_DIPSETTING(    0x03, "Slot 3" )
		PORT_DIPSETTING(    0x04, "Slot 4" )
INPUT_PORTS_END

ioport_constructor multi_conn_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(multi_slot);
}

/**************************************************************************

    The GRAM Kracker was manufactured by Miller's Graphics and designed to
    fit into the cartridge slot.

    It offers one own cartridge slot at the top side and a row of switches
    at its front. It contains buffered SRAM circuits; the base version has
    56 KiB, and the extended version has 80 KiB.

    The operation of the GRAM Kracker is a bit complex and most likely
    bound to fail when you have no manual. Accordingly, this emulation is
    neither simpler nor more difficult to use.

    Concept of operation:

    Loader: The GRAM Kracker contains a small loader utility
    which allows you to dump cartridges and to load the contents into the
    SRAM of the GK. This loader utility is active when the switch 5 is put
    into "Loader on" state. The activated loader hides the TI BASIC
    interpreter in the console.

    Cartridges: When a cartridge is plugged into the GK the contents may be
    dumped and saved to disk by the loader. They cannot be directly copied
    into the GK because the memory locations are hidden by the cartridge.

    Loading the cartridge into the SRAM: With the cartridge unplugged, dumps
    can be loaded into the SRAM using the loader. This is one major use case
    of the GK, that is, to load dumps from disk, and in particular modified
    dumps. (There is no checksum, so contents may be freely changed.)

    Console dump: The GK is also able to dump the console GROMs and also to
    load them into the SRAM (only in the extended version). Due to a
    peculiarity of the TI console design it is possible to override the
    console GROMs with the contents in the cartridge slot.

    A standard procedure for use with the GK:

    Save cartridge:

    - Put switches to [Normal | OpSys | TI BASIC | W/P | Loader On]
    - Insert a disk image into disk drive 1
    - Plug in a cartridge
    - Reset the console (done automatically here)
    - Visit the option screen, press 1 for GRAM KRACKER
    - In the GK loader, select 2 for Save Module
    - Follow the on-screen instructions. Switches are set via the dip switch menu.
    - Enter a target file name
    - Saving is complete when the Save operation has been unmarked.

    Load cartridge:

    - Put switches to [Normal | OpSys | TI BASIC | W/P | Loader On]
    - Insert a disk image into disk drive 1
    - Make sure no cartridge is plugged in
    - Press 1 for GRAM KRACKER
    - Press 3 for Init Module space; follow instructions
    - Press 1 for Load Module; specify file name on disk
    - Loading is complete when the Load operation has been unmarked.

    Memory organisation:

    The console has three GROMs with 6 KiB size and occupying 8 KiB of address
    space each. These are called GROMs 0, 1, and 2. GROM 0 contains the common
    routines for the computer operation; GROMs 1 and 2 contain TI BASIC.

    Memory locations 6000-7fff are assigned to cartridge ROMs; in some
    cartridges, a second ROM bank can be used by writing a value to a special
    ROM access. This way, instead of 8 KiB we often have 16 KiB at these
    locations.

    Each cartridge can host up to 5 GROMs (called GROM 3, 4, 5, 6, and 7).
    As in the console, each one occupies 6 KiB in an 8 KiB window.

    The GRAM Kracker offers

    - a loader in an own GROM 1 (which hides the console GROM 1 when active,
    so we have no BASIC anymore). The contents of the loader must be found
    by the emulator in a file named ti99_gkracker.zip.

    - a complete set of 8 (simulated) GRAMs with full 8 KiB each (done by a
    simple addressing circuit); the basic version only offered GRAMs 3-7

    - 16 KiB of RAM memory space for the 6000-7fff area (called "bank 1" and "bank 2")

    Notes:

    - it is mandatory to turn off the loader when loading into GRAM 1, but only
    after prompted in the on-screen instructions, or the loader will crash
    - GRAM0 must be properly loaded if switch 2 is set to GRAM0 and the computer is reset
    - Switch 4 must not be in W/P position (write protect) when loading data
    into the GK (either other position will do).


***************************************************************************/
enum
{
	GK_OFF = 0,
	GK_NORMAL = 1,
	GK_GRAM0 = 0,
	GK_OPSYS = 1,
	GK_GRAM12 = 0,
	GK_TIBASIC = 1,
	GK_BANK1 = 0,
	GK_WP = 1,
	GK_BANK2 = 2,
	GK_LDON = 0,
	GK_LDOFF = 1
};

#define GKSWITCH1_TAG "GKSWITCH1"
#define GKSWITCH2_TAG "GKSWITCH2"
#define GKSWITCH3_TAG "GKSWITCH3"
#define GKSWITCH4_TAG "GKSWITCH4"
#define GKSWITCH5_TAG "GKSWITCH5"

gkracker_device::gkracker_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:   ti99_cartridge_connector_device(mconfig, GROMPORT_GK, "GRAMKracker", tag, owner, clock, "ti99_gkracker", __FILE__),
		device_nvram_interface(mconfig, *this),
		m_ram_page(0),
		m_grom_address(0),
		m_ram_ptr(NULL),
		m_grom_ptr(NULL),
		m_waddr_LSB(false),
		m_cartridge(NULL)
{
}

READ8Z_MEMBER(gkracker_device::readz)
{
	if ((offset & grom_mask()) == grom_base())
	{
		// Reads from the GRAM space of the GRAM Kracker.

		// The GK does not have a readable address counter, but the console
		// GROMs will keep our address counter up to date. That is
		// exactly what happens in the real machine.
		// (The console GROMs are not accessed here but directly via the datamux
		// so we can just return without doing anything)
		if ((offset & 0x0002)!=0) return;

		int id = ((m_grom_address & 0xe000)>>13)&0x07;
		switch (id)
		{
		case 0:
			// GRAM 0. Only return a value if switch 2 is in GRAM0 position.
			if (m_gk_switch[2]==GK_GRAM0)
				*value = m_ram_ptr[m_grom_address];
			break;
		case 1:
			// If the loader is turned on, return loader contents.
			if (m_gk_switch[5]==GK_LDON)
			{
				// The only ROM contained in the GK box is the loader
				// Adjust the address
				*value = m_grom_ptr[m_grom_address & 0x1fff];
			}
			else
			{
				// Loader off
				// GRAM 1. Only return a value if switch 3 is in GRAM12 position.
				// Otherwise, the console GROM 1 will respond (not here; it is the grom_device
				// whose output would then not be overwritten)
				if (m_gk_switch[3]==GK_GRAM12)
					*value = m_ram_ptr[m_grom_address];
			}
			break;
		case 2:
			// GRAM 2. Only return a value if switch 3 is in GRAM12 position.
			if (m_gk_switch[3]==GK_GRAM12)
				*value = m_ram_ptr[m_grom_address];
			break;
		default:
			// Cartridge space (0x6000 - 0xffff)
			// When a cartridge is installed, it overrides the GK contents
			// but only if it has GROMs
			bool guest_has_grom = false;

			if (m_cartridge != NULL)
			{
				guest_has_grom = m_cartridge->has_grom();
				// Note that we only have ONE real cartridge and the GK;
				// we need not access all slots.
				if (guest_has_grom)
				{
					m_cartridge->readz(space, offset, value, mem_mask); // read from guest
				}
			}
			if (!guest_has_grom && (m_gk_switch[1]==GK_NORMAL))
				*value = m_ram_ptr[m_grom_address]; // use the GK memory
		}

		// The GK GROM emulation does not wrap at 8K boundaries.
		m_grom_address = (m_grom_address + 1) & 0xffff;

		// Reset the write address flipflop.
		m_waddr_LSB = false;
	}
	else
	{
		if (m_cartridge != NULL)
		{
			// Read from the guest cartridge.
			m_cartridge->readz(space, offset, value, mem_mask);
		}
		else
		{
			// Reads from the RAM space of the GRAM Kracker.
			if (m_gk_switch[1] == GK_OFF) return; // just don't do anything
			switch (m_gk_switch[4])
			{
			// RAM is stored behind the GRAM area
			case GK_BANK1:
				*value = m_ram_ptr[offset+0x10000 - 0x6000];
				break;
			case GK_BANK2:
				*value = m_ram_ptr[offset+0x12000 - 0x6000];
				break;

			default:
				// Switch in middle position (WP, implies auto-select according to the page flag)
				if (m_ram_page==0)
					*value = m_ram_ptr[offset+0x10000 - 0x6000];
				else
					*value = m_ram_ptr[offset+0x12000 - 0x6000];
				break;
			}
		}
	}
	if (VERBOSE>8) LOG("gkracker_device: read %04x -> %02x\n", offset, *value);
}

WRITE8_MEMBER(gkracker_device::write)
{
	// write to the guest cartridge if present
	if (m_cartridge != NULL)
	{
		m_cartridge->write(space, offset, data, mem_mask);
	}

	if ((offset & grom_mask()) == grom_base())
	{
		// Write to the GRAM space of the GRAM Kracker.
		if ((offset & 0x0002)==0x0002)
		{
			// Set address
			if (m_waddr_LSB == true)
			{
				// Accept low address byte (second write)
				m_grom_address = (m_grom_address & 0xff00) | data;
				m_waddr_LSB = false;
				if (VERBOSE>8) LOG("gkracker_device: set grom address %04x\n", m_grom_address);
			}
			else
			{
				// Accept high address byte (first write)
				m_grom_address = (m_grom_address & 0x00ff) | (data << 8);
				m_waddr_LSB = true;
			}
		}
		else
		{
			// Write data byte to GRAM area.
			if (VERBOSE>7) LOG("gkracker_device: gwrite %04x(%04x) <- %02x\n", offset, m_grom_address, data);

			// According to manual:
			// Writing to GRAM 0: switch 2 set to GRAM 0 + Write protect switch (4) in 1 or 2 position
			// Writing to GRAM 1: switch 3 set to GRAM 1-2 + Loader off (5); write prot has no effect
			// Writing to GRAM 2: switch 3 set to GRAM 1-2 (write prot has no effect)
			// Writing to GRAM 3-7: switch 1 set to GK_NORMAL, no cartridge inserted
			// GK_NORMAL switch has no effect on GRAM 0-2
			int id = ((m_grom_address & 0xe000)>>13)&0x07;
			switch (id)
			{
			case 0:
				if (m_gk_switch[2]==GK_GRAM0 && m_gk_switch[4]!=GK_WP)
					m_ram_ptr[m_grom_address] = data;
				break;
			case 1:
				if (m_gk_switch[3]==GK_GRAM12 && m_gk_switch[5]==GK_LDOFF)
					m_ram_ptr[m_grom_address] = data;
				break;
			case 2:
				if (m_gk_switch[3]==GK_GRAM12)
					m_ram_ptr[m_grom_address] = data;
				break;
			default:
				if (m_gk_switch[1]==GK_NORMAL && m_cartridge == NULL)
					m_ram_ptr[m_grom_address] = data;
				break;
			}
			// The GK GROM emulation does not wrap at 8K boundaries.
			m_grom_address = (m_grom_address + 1) & 0xffff;

			// Reset the write address flipflop.
			m_waddr_LSB = false;
		}
	}
	else
	{
		// Write to the RAM space of the GRAM Kracker
		// (only if no cartridge is present)
		if (VERBOSE>7) LOG("gkracker_device: write %04x <- %02x\n", offset, data);
		if (m_cartridge == NULL)
		{
			if (m_gk_switch[1] == GK_OFF) return; // just don't do anything
			switch (m_gk_switch[4])
			{
			// RAM is stored behind the GRAM area
			case GK_BANK1:
				m_ram_ptr[offset+0x10000 - 0x6000] = data;
				break;

			case GK_BANK2:
				m_ram_ptr[offset+0x12000 - 0x6000] = data;
				break;

			default:
				// Switch in middle position (WP, implies auto-select according to the page flag)
				// This is handled like in Extended Basic (using addresses)
				m_ram_page = (offset >> 1) & 1;
				break;
			}
		}
	}
}

READ8Z_MEMBER( gkracker_device::crureadz )
{
	if (m_cartridge != NULL) m_cartridge->crureadz(space, offset, value);
}

WRITE8_MEMBER( gkracker_device::cruwrite )
{
	if (m_cartridge != NULL) m_cartridge->cruwrite(space, offset, data);
}

INPUT_CHANGED_MEMBER( gkracker_device::gk_changed )
{
	if (VERBOSE>7) LOG("gkracker_device: input changed %d - %d\n", (int)((UINT64)param & 0x07), newval);
	m_gk_switch[(UINT64)param & 0x07] = newval;
}

void gkracker_device::insert(int index, ti99_cartridge_device* cart)
{
	if (VERBOSE>3) LOG("gkracker_device: insert cartridge\n");
	m_cartridge = cart;
	// Switch 1 has a third location for resetting. We do the reset by default
	// here. It can be turned off in the configuration.
	m_gromport->cartridge_inserted();
}

void gkracker_device::remove(int index)
{
	if (VERBOSE>3) LOG("gkracker_device: remove cartridge\n");
	m_cartridge = NULL;
}

void gkracker_device::gk_install_menu(const char* menutext, int len, int ptr, int next, int start)
{
	const int base = 0x0000;
	m_ram_ptr[base + ptr] = (UINT8)((next >> 8) & 0xff);
	m_ram_ptr[base + ptr+1] = (UINT8)(next & 0xff);
	m_ram_ptr[base + ptr+2] = (UINT8)((start >> 8) & 0xff);
	m_ram_ptr[base + ptr+3] = (UINT8)(start & 0xff);

	m_ram_ptr[base + ptr+4] = (UINT8)(len & 0xff);
	memcpy(m_ram_ptr + base + ptr+5, menutext, len);
}

/*
    Define the default for the GRAM Kracker device. The memory is preset with
    some sample entries which shall indicate that the memory has been tested
    by the manufacturer.
*/
void gkracker_device::nvram_default()
{
	if (VERBOSE>3) LOG("gkracker_device: Creating default NVRAM\n");
	memset(m_ram_ptr, 0, 81920);

	m_ram_ptr[0x6000] = 0xaa;
	m_ram_ptr[0x6001] = 0x01;
	m_ram_ptr[0x6002] = 0x01;

	m_ram_ptr[0x6006] = 0x60;
	m_ram_ptr[0x6007] = 0x20;

	gk_install_menu("GROM 3 OK", 9, 0x60e0, 0, 0x6100);
	gk_install_menu("GROM 4 OK", 9, 0x60c0, 0x60e0, 0x6100);
	gk_install_menu("GROM 5 OK", 9, 0x60a0, 0x60c0, 0x6100);
	gk_install_menu("GROM 6 OK", 9, 0x6080, 0x60a0, 0x6100);
	gk_install_menu("PROM   OK", 9, 0x6060, 0x6080, 0x6100);
	gk_install_menu("RAMS   OK", 9, 0x6040, 0x6060, 0x6100);
	gk_install_menu("OPTION GRAMS OK", 15, 0x6020, 0x6040, 0x6100);

	m_ram_ptr[0x6100] = 0x0b;       // GPL EXIT
}

void gkracker_device::nvram_read(emu_file &file)
{
	int readsize = file.read(m_ram_ptr, 81920);
	if (VERBOSE>3) LOG("gkracker_device: reading NVRAM\n");
	// If we increased the size, fill the remaining parts with 0
	if (readsize < 81920)
	{
		memset(m_ram_ptr + readsize, 0, 81920-readsize);
	}
}

void gkracker_device::nvram_write(emu_file &file)
{
	if (VERBOSE>3) LOG("gkracker_device: writing NVRAM\n");
	file.write(m_ram_ptr, 81920);
}

void gkracker_device::device_start()
{
	m_ram_ptr = memregion(GKRACKER_NVRAM_TAG)->base();
	m_grom_ptr = memregion(GKRACKER_ROM_TAG)->base();
	m_cartridge = NULL;
	for (int i=1; i < 6; i++) m_gk_switch[i] = 0;
}

void gkracker_device::device_reset()
{
	m_gk_switch[1] = ioport(GKSWITCH1_TAG)->read();
	m_gk_switch[2] = ioport(GKSWITCH2_TAG)->read();
	m_gk_switch[3] = ioport(GKSWITCH3_TAG)->read();
	m_gk_switch[4] = ioport(GKSWITCH4_TAG)->read();
	m_gk_switch[5] = ioport(GKSWITCH5_TAG)->read();
	m_grom_address = 0; // for the GROM emulation
	m_ram_page = 0;
	m_waddr_LSB = false;
}

static MACHINE_CONFIG_FRAGMENT( gkracker_slot )
	MCFG_DEVICE_ADD("cartridge", TI99CART, 0)
MACHINE_CONFIG_END

/*
    The GRAMKracker ROM
*/
ROM_START( gkracker_rom )
	ROM_REGION(0x14000, GKRACKER_NVRAM_TAG, ROMREGION_ERASE00)
	ROM_REGION(0x2000, GKRACKER_ROM_TAG, 0)
	ROM_LOAD("gkracker.bin", 0x0000, 0x2000, CRC(86eaaf9f) SHA1(a3bd5257c63e190800921b52dbe3ffa91ad91113))
ROM_END

const rom_entry *gkracker_device::device_rom_region() const
{
	return ROM_NAME( gkracker_rom );
}

machine_config_constructor gkracker_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gkracker_slot );
}

INPUT_PORTS_START(gkracker)
	PORT_START( GKSWITCH1_TAG )
	PORT_DIPNAME( 0x01, 0x01, "GK switch 1" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gkracker_device, gk_changed, 1)
		PORT_DIPSETTING(    0x00, "GK Off" )
		PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )

	PORT_START( GKSWITCH2_TAG )
	PORT_DIPNAME( 0x01, 0x01, "GK switch 2" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gkracker_device, gk_changed, 2)
		PORT_DIPSETTING(    0x00, "GRAM 0" )
		PORT_DIPSETTING(    0x01, "Op Sys" )

	PORT_START( GKSWITCH3_TAG )
	PORT_DIPNAME( 0x01, 0x01, "GK switch 3" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gkracker_device, gk_changed, 3)
		PORT_DIPSETTING(    0x00, "GRAM 1-2" )
		PORT_DIPSETTING(    0x01, "TI BASIC" )

	PORT_START( GKSWITCH4_TAG )
	PORT_DIPNAME( 0x03, 0x01, "GK switch 4" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gkracker_device, gk_changed, 4)
		PORT_DIPSETTING(    0x00, "Bank 1" )
		PORT_DIPSETTING(    0x01, "W/P" )
		PORT_DIPSETTING(    0x02, "Bank 2" )

	PORT_START( GKSWITCH5_TAG )
	PORT_DIPNAME( 0x01, 0x00, "GK switch 5" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gkracker_device, gk_changed, 5)
		PORT_DIPSETTING(    0x00, "Loader On" )
		PORT_DIPSETTING(    0x01, "Loader Off" )
INPUT_PORTS_END

ioport_constructor gkracker_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gkracker);
}

/***************************************************************************
    Cartridge implementation

    Every cartridge is an instance of ti99_cartridge_device, implementing the
    device_image_interface. This means it is capable of loading cartridge
    data into its memory locations. All memory locations are organised as
    regions.

    The different cartridge versions are realised by different PCB instances.
    All PCBs are subclassed from ti99_cartridge_pcb.

***************************************************************************/
enum
{
	PCB_STANDARD=1,
	PCB_PAGED,
	PCB_MINIMEM,
	PCB_SUPER,
	PCB_MBX,
	PCB_PAGED379I,
	PCB_PAGEDCRU,
	PCB_GROMEMU
};

static const pcb_type pcbdefs[] =
{
	{ PCB_STANDARD, "standard" },
	{ PCB_PAGED, "paged" },
	{ PCB_MINIMEM, "minimem" },
	{ PCB_SUPER, "super" },
	{ PCB_MBX, "mbx" },
	{ PCB_PAGED379I, "paged379i" },
	{ PCB_PAGEDCRU, "pagedcru" },
	{ PCB_GROMEMU, "gromemu" },
	{ 0, NULL}
};

// Softlists do not support the cartridges with RAM yet
static const pcb_type sw_pcbdefs[] =
{
	{ PCB_STANDARD, "standard" },
	{ PCB_PAGED, "paged" },
	{ PCB_GROMEMU, "gromemu" },
	{ 0, NULL}
};

ti99_cartridge_device::ti99_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
:   bus8z_device(mconfig, TI99CART, "TI-99 cartridge", tag, owner, clock, "cartridge", __FILE__),
	device_image_interface(mconfig, *this),
	m_softlist(false),
	m_pcbtype(0),
	m_slot(0),
	m_pcb(NULL),
	m_connector(NULL),
	m_rpk(NULL)
{
}

void ti99_cartridge_device::prepare_cartridge()
{
	int rom2_length;

	UINT8* grom_ptr;

	memory_region *regg;
	memory_region *regr;
	memory_region *regr2;

	// Initialize some values.
	m_pcb->m_rom_page = 0;
	m_pcb->m_rom_ptr = NULL;
	m_pcb->m_rom2_ptr = NULL;
	m_pcb->m_ram_size = 0;
	m_pcb->m_ram_ptr = NULL;
	m_pcb->m_ram_page = 0;

	for (int i=0; i < 5; i++) m_pcb->m_grom[i] = NULL;

	m_pcb->m_grom_size = m_softlist? get_software_region_length("grom_socket") : m_rpk->get_resource_length("grom_socket");
	if (VERBOSE>6) LOG("gromport: grom_socket.size=0x%04x\n", m_pcb->m_grom_size);

	if (m_pcb->m_grom_size > 0)
	{
		regg = memregion(CARTGROM_TAG);
		grom_ptr = m_softlist? get_software_region("grom_socket") : m_rpk->get_contents_of_socket("grom_socket");
		memcpy(regg->base(), grom_ptr, m_pcb->m_grom_size);
		m_pcb->m_grom_ptr = regg->base();   // for gromemu
		m_pcb->m_grom_address = 0;          // for gromemu

		// Find the GROMs and keep their pointers
		m_pcb->set_grom_pointer(0, subdevice(GROM3_TAG));
		if (m_pcb->m_grom_size > 0x2000) m_pcb->set_grom_pointer(1, subdevice(GROM4_TAG));
		if (m_pcb->m_grom_size > 0x4000) m_pcb->set_grom_pointer(2, subdevice(GROM5_TAG));
		if (m_pcb->m_grom_size > 0x6000) m_pcb->set_grom_pointer(3, subdevice(GROM6_TAG));
		if (m_pcb->m_grom_size > 0x8000) m_pcb->set_grom_pointer(4, subdevice(GROM7_TAG));
	}

	m_pcb->m_rom_size = m_softlist? get_software_region_length("rom_socket") : m_rpk->get_resource_length("rom_socket");
	if (m_pcb->m_rom_size > 0)
	{
		if (VERBOSE>6) LOG("gromport: rom_socket.size=0x%04x\n", m_pcb->m_rom_size);
		regr = memregion(CARTROM_TAG);
		m_pcb->m_rom_ptr = m_softlist? get_software_region("rom_socket") : m_rpk->get_contents_of_socket("rom_socket");
		memcpy(regr->base(), m_pcb->m_rom_ptr, m_pcb->m_rom_size);
		// Set both pointers to the same region for now
		m_pcb->m_rom_ptr = m_pcb->m_rom2_ptr = regr->base();
	}

	rom2_length = m_softlist? get_software_region_length("rom2_socket") : m_rpk->get_resource_length("rom2_socket");
	if (rom2_length > 0)
	{
		// sizes do not differ between rom and rom2
		regr2 = memregion(CARTROM2_TAG);
		m_pcb->m_rom2_ptr = m_softlist? get_software_region("rom2_socket") : m_rpk->get_contents_of_socket("rom2_socket");
		memcpy(regr2->base(), m_pcb->m_rom2_ptr, rom2_length);
		m_pcb->m_rom2_ptr = regr2->base();
	}

	// NVRAM cartridges are not supported by softlists (we need to find a way to load the nvram contents first)
	if (!m_softlist)
	{
		m_pcb->m_ram_size = m_rpk->get_resource_length("ram_socket");
		if (m_pcb->m_ram_size > 0)
		{
			// TODO: Consider to use a region as well. If so, do not forget to memcpy.
			m_pcb->m_ram_ptr = m_rpk->get_contents_of_socket("ram_socket");
		}
	}
}

/*
    Find the index of the cartridge name. We assume the format
    <name><number>, i.e. the number is the longest string from the right
    which can be interpreted as a number. Subtract 1.
*/
int ti99_cartridge_device::get_index_from_tagname()
{
	const char *mytag = tag();
	int maxlen = strlen(mytag);
	int i;

	for (i=maxlen-1; i >=0; i--)
		if (mytag[i] < 48 || mytag[i] > 57) break;

	if (i==maxlen-1) return 0;
	return atoi(mytag+i+1)-1;
}

bool ti99_cartridge_device::has_grom()
{
	return m_pcb->m_grom_size>0;
}

UINT16 ti99_cartridge_device::grom_base()
{
	return m_connector->grom_base();
}

UINT16 ti99_cartridge_device::grom_mask()
{
	return m_connector->grom_mask();
}

bool ti99_cartridge_device::call_load()
{
	// File name is in m_basename
	// return true = error
	if (VERBOSE>8) LOG("cartridge_device: loading %s in slot %s\n", m_basename.c_str(), tag());

	if (m_softlist)
	{
		if (VERBOSE>7) LOG("using softlists\n");
		int i = 0;
		const char* pcb = get_feature("pcb");
		do
		{
			if (strcmp(pcb, sw_pcbdefs[i].name)==0)
			{
				m_pcbtype = sw_pcbdefs[i].id;
				break;
			}
			i++;
		} while (sw_pcbdefs[i].id != 0);
		if (VERBOSE>5) LOG("gromport.cartridge_device: Cartridge type is %s (%d)\n", pcb, m_pcbtype);
	}
	else
	{
		rpk_reader *reader = new rpk_reader(pcbdefs);
		try
		{
			m_rpk = reader->open(machine().options(), filename(), machine().system().name);
			m_pcbtype = m_rpk->get_type();
		}
		catch (rpk_exception& err)
		{
			LOG("gromport.cartridge_device: Failed to load cartridge '%s': %s\n", basename(), err.to_string());
			m_rpk = NULL;
			m_err = IMAGE_ERROR_INVALIDIMAGE;
			return true;
		}
	}

	switch (m_pcbtype)
	{
	case PCB_STANDARD:
		if (VERBOSE>6) LOG("gromport.cartridge_device: standard PCB\n");
		m_pcb = new ti99_standard_cartridge();
		break;
	case PCB_PAGED:
		if (VERBOSE>6) LOG("gromport.cartridge_device: paged PCB\n");
		m_pcb = new ti99_paged_cartridge();
		break;
	case PCB_MINIMEM:
		if (VERBOSE>6) LOG("gromport.cartridge_device: minimem PCB\n");
		m_pcb = new ti99_minimem_cartridge();
		break;
	case PCB_SUPER:
		if (VERBOSE>6) LOG("gromport.cartridge_device: superspace PCB\n");
		m_pcb = new ti99_super_cartridge();
		break;
	case PCB_MBX:
		if (VERBOSE>6) LOG("gromport.cartridge_device: MBX PCB\n");
		m_pcb = new ti99_mbx_cartridge();
		break;
	case PCB_PAGED379I:
		if (VERBOSE>6) LOG("gromport.cartridge_device: Paged379i PCB\n");
		m_pcb = new ti99_paged379i_cartridge();
		break;
	case PCB_PAGEDCRU:
		if (VERBOSE>6) LOG("gromport.cartridge_device: PagedCRU PCB\n");
		m_pcb = new ti99_pagedcru_cartridge();
		break;
	case PCB_GROMEMU:
		if (VERBOSE>6) LOG("gromport.cartridge_device: GromEmulation PCB\n");
		m_pcb = new ti99_gromemu_cartridge();
		break;
	}

	prepare_cartridge();
	m_pcb->set_cartridge(this);
	m_slot = get_index_from_tagname();
	m_connector->insert(m_slot, this);
	return false;
}

void ti99_cartridge_device::call_unload()
{
	if (VERBOSE>7) LOG("ti99_cartridge_device: unload\n");
	if (m_rpk != NULL)
	{
		m_rpk->close(); // will write NVRAM contents
		delete m_rpk;
	}

	delete m_pcb;
	m_pcb = NULL;
	m_connector->remove(m_slot);
}

void ti99_cartridge_device::set_slot(int i)
{
	m_slot = i;
}

bool ti99_cartridge_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	if (VERBOSE>8) LOG("ti99_cartridge_device: swlist = %s, swname = %s\n", swlist.list_name(), swname);
	load_software_part_region(*this, swlist, swname, start_entry);
	m_softlist = true;
	m_rpk = NULL;
	return true;
}

READ8Z_MEMBER(ti99_cartridge_device::readz)
{
	if (m_pcb != NULL) m_pcb->readz(space, offset, value);
}

WRITE8_MEMBER(ti99_cartridge_device::write)
{
	if (m_pcb != NULL) m_pcb->write(space, offset, data);
}

READ8Z_MEMBER(ti99_cartridge_device::crureadz)
{
	if (m_pcb != NULL) m_pcb->crureadz(space, offset, value);
}

WRITE8_MEMBER(ti99_cartridge_device::cruwrite)
{
	if (m_pcb != NULL) m_pcb->cruwrite(space, offset, data);
}

WRITE_LINE_MEMBER( ti99_cartridge_device::ready_line )
{
	m_connector->ready_line(state);
}

void ti99_cartridge_device::device_config_complete()
{
	update_names();
	m_softlist = false;
	m_connector = static_cast<ti99_cartridge_connector_device*>(owner());
}

static GROM_CONFIG(grom3_config)
{
	false, 3, CARTGROM_TAG, 0x0000, 0x1800, GROMFREQ
};
static GROM_CONFIG(grom4_config)
{
	false, 4, CARTGROM_TAG, 0x2000, 0x1800, GROMFREQ
};
static GROM_CONFIG(grom5_config)
{
	false, 5, CARTGROM_TAG, 0x4000, 0x1800, GROMFREQ
};
static GROM_CONFIG(grom6_config)
{
	false, 6, CARTGROM_TAG, 0x6000, 0x1800, GROMFREQ
};
static GROM_CONFIG(grom7_config)
{
	false, 7, CARTGROM_TAG, 0x8000, 0x1800, GROMFREQ
};

/*
    5 GROMs that may be contained in a cartridge
*/
static MACHINE_CONFIG_FRAGMENT( ti99_cartridge )
	MCFG_GROM_ADD( GROM3_TAG, grom3_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_cartridge_device, ready_line))
	MCFG_GROM_ADD( GROM4_TAG, grom4_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_cartridge_device, ready_line))
	MCFG_GROM_ADD( GROM5_TAG, grom5_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_cartridge_device, ready_line))
	MCFG_GROM_ADD( GROM6_TAG, grom6_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_cartridge_device, ready_line))
	MCFG_GROM_ADD( GROM7_TAG, grom7_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_cartridge_device, ready_line))
MACHINE_CONFIG_END

machine_config_constructor ti99_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti99_cartridge );
}

/*
    Memory area for one cartridge. For most cartridges we only need 8 KiB for
    ROM contents, but cartridges of the "paged379i" type have up to 128 KiB
    organised as selectable banks, so we must be sure there is enough space.
*/
ROM_START( cartridge_memory )
	ROM_REGION(0xa000, CARTGROM_TAG, ROMREGION_ERASE00)
	ROM_REGION(0x20000, CARTROM_TAG, ROMREGION_ERASE00)
	ROM_REGION(0x2000, CARTROM2_TAG, ROMREGION_ERASE00)
ROM_END

const rom_entry *ti99_cartridge_device::device_rom_region() const
{
	return ROM_NAME( cartridge_memory );
}

const device_type TI99CART = &device_creator<ti99_cartridge_device>;

/***************************************************************************
    Cartridge types
    Cartridges differ by the circuits on their PCB which hosts the ROMs.
    Some cartridges also have RAM, and some allow for switching between
    ROMs.

    Unlike in the previous implementation we do not model it as a full device.
***************************************************************************/

ti99_cartridge_pcb::ti99_cartridge_pcb()
{
}

UINT16 ti99_cartridge_pcb::grom_base()
{
	return m_cart->grom_base();
}

UINT16 ti99_cartridge_pcb::grom_mask()
{
	return m_cart->grom_mask();
}

void ti99_cartridge_pcb::set_cartridge(ti99_cartridge_device *cart)
{
	m_cart = cart;
}

READ8Z_MEMBER(ti99_cartridge_pcb::gromreadz)
{
	for (int i=0; i < 5; i++)
	{
		if (m_grom[i] != NULL)
		{
			m_grom[i]->readz(space, offset, value, mem_mask);
		}
	}
}

WRITE8_MEMBER(ti99_cartridge_pcb::gromwrite)
{
	for (int i=0; i < 5; i++)
	{
		if (m_grom[i] != NULL)
		{
			m_grom[i]->write(space, offset, data, mem_mask);
		}
	}
}

READ8Z_MEMBER(ti99_cartridge_pcb::readz)
{
	if ((offset & grom_mask())==grom_base())
		gromreadz(space, offset, value, mem_mask);
	else
	{
		if (m_rom_ptr!=NULL)
		{
			// For TI-99/8 we should plan for 16K cartridges. However, none was ever produced.
			// Well, forget about that.
			*value = m_rom_ptr[offset & 0x1fff];
			//      LOG("read cartridge rom space %04x = %02x\n", offset, *value);
		}
	}
}

WRITE8_MEMBER(ti99_cartridge_pcb::write)
{
	// LOG("write standard\n");
	if ((offset & grom_mask())==grom_base())
		gromwrite(space, offset, data, mem_mask);
	else
	{
		if (VERBOSE>5) LOG("cartridge_pcb_device: Cannot write to ROM space at %04x\n", offset);
	}
}

READ8Z_MEMBER(ti99_cartridge_pcb::crureadz)
{
}

WRITE8_MEMBER(ti99_cartridge_pcb::cruwrite)
{
}

inline void ti99_cartridge_pcb::set_grom_pointer(int number, device_t *dev)
{
	m_grom[number] = static_cast<ti99_grom_device*>(dev);
}

/*****************************************************************************
  Cartridge type: Paged (Extended Basic)
    This cartridge consists of GROM memory and 2 pages of standard ROM.
    The page is set by writing any value to a location in
    the address area, where an even word offset sets the page to 0 and an
    odd word offset sets the page to 1 (e.g. 6000 = bank 0, and
    6002 = bank 1).
******************************************************************************/

READ8Z_MEMBER(ti99_paged_cartridge::readz)
{
	if ((offset & grom_mask())==grom_base())
		gromreadz(space, offset, value, mem_mask);
	else
	{
		if (m_rom_page==0)
		{
			*value = m_rom_ptr[offset & 0x1fff];
		}
		else
		{
			*value = m_rom2_ptr[offset & 0x1fff];
		}
	}
}

WRITE8_MEMBER(ti99_paged_cartridge::write)
{
	// LOG("write standard\n");
	if ((offset & grom_mask())==grom_base())
		gromwrite(space, offset, data, mem_mask);

	else {
		m_rom_page = (offset >> 1) & 1;
	}
}

/*****************************************************************************
  Cartridge type: Mini Memory
    GROM: 6 KiB (occupies G>6000 to G>7800)
    ROM: 4 KiB (romfile is actually 8 K long, second half with zeros, 0x6000-0x6fff)
    persistent RAM: 4 KiB (0x7000-0x7fff)
******************************************************************************/

/* Read function for the minimem cartridge. */
READ8Z_MEMBER(ti99_minimem_cartridge::readz)
{
	if ((offset & grom_mask())==grom_base())
		gromreadz(space, offset, value, mem_mask);

	else
	{
		if ((offset & 0x1000)==0x0000)
		{
			if (m_rom_ptr!=NULL)    // Super-Minimem seems to have no ROM
			{
				*value = m_rom_ptr[offset & 0x0fff];
			}
		}
		else
		{
			*value = m_ram_ptr[offset & 0x0fff];
		}
	}
}

/* Write function for the minimem cartridge. */
WRITE8_MEMBER(ti99_minimem_cartridge::write)
{
	// LOG("write standard\n");
	if ((offset & grom_mask())==grom_base())
		gromwrite(space, offset, data, mem_mask);

	else
	{
		if ((offset & 0x1000)==0x0000)
		{
			if (VERBOSE>1) LOG("ti99: gromport: Write access to cartridge ROM at address %04x ignored", offset);
		}
		else
		{
			m_ram_ptr[offset & 0x0fff] = data;
		}
	}
}

/*****************************************************************************
    Cartridge type: SuperSpace II

    SuperSpace is intended as a user-definable blank cartridge containing
    buffered RAM. It has an Editor/Assembler GROM which helps the user to load
    the user program into the cartridge. If the user program has a suitable
    header, the console recognizes the cartridge as runnable, and
    assigns a number in the selection screen. Switching the RAM banks in this
    cartridge is achieved by setting CRU bits (the system serial interface).

    GROM:           Editor/Assembler GROM
    ROM:            none
    persistent RAM: 32 KiB (0x6000-0x7fff, 4 banks)
    Banking:        via CRU write
******************************************************************************/

/* Read function for the super cartridge. */
READ8Z_MEMBER(ti99_super_cartridge::readz)
{
	if ((offset & grom_mask())==grom_base())
		gromreadz(space, offset, value, mem_mask);
	else
	{
		if (m_ram_ptr != NULL)
		{
			*value = m_ram_ptr[(m_ram_page << 13) | (offset & 0x1fff)];
		}
	}
}

/* Write function for the super cartridge. */
WRITE8_MEMBER(ti99_super_cartridge::write)
{
	if ((offset & grom_mask())==grom_base())
		gromwrite(space, offset, data, mem_mask);
	else
	{
		m_ram_ptr[(m_ram_page << 13) | (offset & 0x1fff)] = data;
	}
}

READ8Z_MEMBER(ti99_super_cartridge::crureadz)
{
	// offset is the bit number. The CRU base address is already divided  by 2.

	// ram_page contains the bank number. We have a maximum of
	// 4 banks; the Super Space II manual says:
	//
	// Banks are selected by writing a bit pattern to CRU address >0800:
	//
	// Bank #   Value
	// 0        >02  = 0000 0010
	// 1        >08  = 0000 1000
	// 2        >20  = 0010 0000
	// 3        >80  = 1000 0000
	//
	// With the bank number (0, 1, 2, or 3) in R0:
	//
	// BNKSW   LI    R12,>0800   Set CRU address
	//         LI    R1,2        Load Shift Bit
	//         SLA   R0,1        Align Bank Number
	//         JEQ   BNKS1       Skip shift if Bank 0
	//         SLA   R1,0        Align Shift Bit
	// BNKS1   LDCR  R1,0        Switch Banks
	//         SRL   R0,1        Restore Bank Number (optional)
	//         RT

	// Our implementation in MESS always gets 8 bits in one go. Also, the address
	// is twice the bit number. That is, the offset value is always a multiple
	// of 0x10.

	if ((offset & 0xfff0) == 0x0800)
	{
		if (VERBOSE>2) LOG("ti99_super_cartridge: CRU accessed at %04x\n", offset);
		UINT8 val = 0x02 << (m_ram_page << 1);
		*value = (val >> ((offset - 0x0800)>>1)) & 0xff;
	}
}

WRITE8_MEMBER(ti99_super_cartridge::cruwrite)
{
	if ((offset & 0xfff0) == 0x0800)
	{
		if (VERBOSE>2) LOG("ti99_super_cartridge: CRU accessed at %04x\n", offset);
		if (data != 0)
			m_ram_page = (offset-0x0802)>>2;
	}
}

/*****************************************************************************
  Cartridge type: MBX
    GROM: up to 40 KiB
    ROM: up to 16 KiB (in up to 2 banks of 8KiB each)
    RAM: 1022 B (0x6c00-0x6ffd, overrides ROM in that area)
    ROM mapper: 6ffe

    TODO: Some MBX cartridges assume the presence of the MBX system
    (special user interface box with speech input/output)
    and will not run without it. This MBX hardware is not emulated yet.
******************************************************************************/

/* Read function for the mbx cartridge. */
READ8Z_MEMBER(ti99_mbx_cartridge::readz)
{
	if ((offset & grom_mask())==grom_base())
		gromreadz(space, offset, value, mem_mask);
	else
	{
		if ((offset & 0x1c00)==0x0c00)
		{
			// This is the RAM area which overrides any ROM. There is no
			// known banking behavior for the RAM, so we must assume that
			// there is only one bank.
			if (m_ram_ptr != NULL)
				*value = m_ram_ptr[offset & 0x03ff];
		}
		else
		{
			if (m_rom_ptr!=NULL)
				*value = m_rom_ptr[(offset & 0x1fff) | (m_rom_page<<13)];
		}
	}
}

/* Write function for the mbx cartridge. */
WRITE8_MEMBER(ti99_mbx_cartridge::write)
{
	if ((offset & grom_mask())==grom_base())
		gromwrite(space, offset, data, mem_mask);
	else
	{
		if (offset == 0x6ffe)
		{
			m_rom_page = data & 1;
			return;
		}

		if ((offset & 0x1c00)==0x0c00)
		{
			if (m_ram_ptr == NULL) return;
			m_ram_ptr[offset & 0x03ff] = data;
		}
	}
}

/*****************************************************************************
  Cartridge type: paged379i
    This cartridge consists of one 16 KiB, 32 KiB, 64 KiB, or 128 KiB EEPROM
    which is organised in 2, 4, 8, or 16 pages of 8 KiB each. The complete
    memory contents must be stored in one dump file.
    The pages are selected by writing a value to some memory locations. Due to
    using the inverted outputs of the LS379 latch, setting the inputs of the
    latch to all 0 selects the highest bank, while setting to all 1 selects the
    lowest. There are some cartridges (16 KiB) which are using this scheme, and
    there are new hardware developments mainly relying on this scheme.

    Writing to       selects page (16K/32K/64K/128K)
    >6000            1 / 3 / 7 / 15
    >6002            0 / 2 / 6 / 14
    >6004            1 / 1 / 5 / 13
    >6006            0 / 0 / 4 / 12
    >6008            1 / 3 / 3 / 11
    >600A            0 / 2 / 2 / 10
    >600C            1 / 1 / 1 / 9
    >600E            0 / 0 / 0 / 8
    >6010            1 / 3 / 7 / 7
    >6012            0 / 2 / 6 / 6
    >6014            1 / 1 / 5 / 5
    >6016            0 / 0 / 4 / 4
    >6018            1 / 3 / 3 / 3
    >601A            0 / 2 / 2 / 2
    >601C            1 / 1 / 1 / 1
    >601E            0 / 0 / 0 / 0

    The paged379i cartrige does not have any GROMs.
******************************************************************************/

/*
    Determines which bank to set, depending on the size of the ROM. This is
    some magic code that actually represents different PCB versions.
*/
int ti99_paged379i_cartridge::get_paged379i_bank(int rompage)
{
	int mask = 0;
	if (m_rom_size > 16384)
	{
		if (m_rom_size > 32768)
		{
			if (m_rom_size > 65536)
				mask = 15;
			else
				mask = 7;
		}
		else
			mask = 3;
	}
	else
		mask = 1;

	return rompage & mask;
}


/* Read function for the paged379i cartridge. */
READ8Z_MEMBER(ti99_paged379i_cartridge::readz)
{
	if ((offset & 0xe000)==0x6000)
		*value = m_rom_ptr[(m_rom_page<<13) | (offset & 0x1fff)];
}

/* Write function for the paged379i cartridge. Only used to set the bank. */
WRITE8_MEMBER(ti99_paged379i_cartridge::write)
{
	// Bits: 0110 0000 000b bbbx
	// x = don't care, bbbb = bank
	if ((offset & 0xffe0)==0x6000)
	{
		// Set bank
		m_rom_page = get_paged379i_bank(15 - ((offset>>1) & 15));
	}
}

/*****************************************************************************
  Cartridge type: pagedcru
    This cartridge consists of one 16 KiB, 32 KiB, or 64 KiB EEPROM which is
    organised in 2, 4, or 8 pages of 8 KiB each. We assume there is only one
    dump file of the respective size.
    The pages are selected by writing a value to the CRU. This scheme is
    similar to the one used for the SuperSpace cartridge, with the exception
    that we are using ROM only, and we can have up to 8 pages.

    Bank     Value written to CRU>0800
    0      >0002  = 0000 0000 0000 0010
    1      >0008  = 0000 0000 0000 1000
    2      >0020  = 0000 0000 0010 0000
    3      >0080  = 0000 0000 1000 0000
    4      >0200  = 0000 0010 0000 0000
    5      >0800  = 0000 1000 0000 0000
    6      >2000  = 0010 0000 0000 0000
    7      >8000  = 1000 0000 0000 0000

    No GROMs used in this type.
******************************************************************************/

/* Read function for the pagedcru cartridge. */
READ8Z_MEMBER(ti99_pagedcru_cartridge::readz)
{
	if ((offset & 0xe000)==0x6000)
		*value = m_rom_ptr[(m_rom_page<<13) | (offset & 0x1fff)];
}

/* Write function for the pagedcru cartridge. No effect. */
WRITE8_MEMBER(ti99_pagedcru_cartridge::write)
{
	return;
}

READ8Z_MEMBER(ti99_pagedcru_cartridge::crureadz)
{
	int page = m_rom_page;
	if ((offset & 0xf800)==0x0800)
	{
		int bit = (offset & 0x001e)>>1;
		if (bit != 0)
		{
			page = page-(bit/2);  // 4 page flags per 8 bits
		}
		*value = 1 << (page*2+1);
	}
}

WRITE8_MEMBER(ti99_pagedcru_cartridge::cruwrite)
{
	if ((offset & 0xf800)==0x0800)
	{
		int bit = (offset & 0x001e)>>1;
		if (data != 0 && bit > 0)
		{
			m_rom_page = (bit-1)/2;
		}
	}
}

/*****************************************************************************
  Cartridge type: GROM emulation/paged

  This cartridge offers GROM address space without real GROM circuits. The GROMs
  are emulated by a normal EPROM with a circuits that mimics GROM behavior.
  Each simulated GROM offers 8K (real GROMs only offer 6K).

  Some assumptions:
  - No readable address counter. This means the parallel console GROMs
    will deliver the address when reading.
  - No wait states. Reading is generally faster than with real GROMs.
  - No wrapping at 8K boundaries.
  - Two pages of ROM at address 6000

  If any of these fails, the cartridge will crash, so we'll see.

  Typical cartridges: RXB, Super Extended Basic

  For the sake of simplicity, we register GROMs like the other PCB types, but
  we implement special access methods for the GROM space.

  Still not working:
     rxb1002 (Set page to 1 (6372 <- 00), lockup)
     rxb237 (immediate reset)
     rxbv555 (repeating reset on Master Title Screen)
     superxb (lockup, fix: add RAM at 7c00)

  Super-MiniMemory is also included here. We assume a RAM area at addresses
  7000-7fff for this cartridge.

******************************************************************************/

READ8Z_MEMBER(ti99_gromemu_cartridge::readz)
{
	if ((offset & grom_mask())==grom_base())
		gromemureadz(space, offset, value, mem_mask);
	else
	{
		if (m_ram_ptr != NULL)
		{
			// Variant of the cartridge which emulates MiniMemory. We don't introduce
			// another type for this single cartridge.
			if ((offset & 0x1fff)==0x1000) {
				*value = m_ram_ptr[offset & 0x0fff];
				return;
			}
		}

		if (m_rom_ptr == NULL) return;
		if (m_rom_page==0)
		{
			*value = m_rom_ptr[offset & 0x1fff];
		}
		else
		{
			*value = m_rom2_ptr[offset & 0x1fff];
		}
	}
}

WRITE8_MEMBER(ti99_gromemu_cartridge::write)
{
	// LOG("write standard\n");
	if ((offset & grom_mask())==grom_base())
		gromemuwrite(space, offset, data, mem_mask);

	else {
		if (m_ram_ptr != NULL)
		{
			// Lines for Super-Minimem; see above
			if ((offset & 0x1fff)==0x1000) {
				m_ram_ptr[offset & 0x0fff] = data;
			}
			return; // no paging
		}

		m_rom_page = (offset >> 1) & 1;
	}
}

READ8Z_MEMBER(ti99_gromemu_cartridge::gromemureadz)
{
	// Similar to the GKracker implemented above, we do not have a readable
	// GROM address counter but use the one from the console GROMs.
	if ((offset & 0x0002)!=0) return;
	int id = ((m_grom_address & 0xe000)>>13)&0x07;
	if (id > 2) {
		// Cartridge space (0x6000 - 0xffff)
		*value = m_grom_ptr[m_grom_address-0x6000]; // use the GROM memory
	}

	// The GROM emulation does not wrap at 8K boundaries.
	m_grom_address = (m_grom_address + 1) & 0xffff;

	// Reset the write address flipflop.
	m_waddr_LSB = false;
}

WRITE8_MEMBER(ti99_gromemu_cartridge::gromemuwrite)
{
	// Set GROM address
	if ((offset & 0x0002)==0x0002) {
		if (m_waddr_LSB == true)
		{
			// Accept low address byte (second write)
			m_grom_address = (m_grom_address & 0xff00) | data;
			m_waddr_LSB = false;
			if (VERBOSE>8) LOG("ti99_gromemu_cartridge: set grom address %04x\n", m_grom_address);
		}
		else
		{
			// Accept high address byte (first write)
			m_grom_address = (m_grom_address & 0x00ff) | (data << 8);
			m_waddr_LSB = true;
		}
	}
	else {
		if (VERBOSE>2) LOG("ti99_gromemu_cartridge: ignoring write to GROM area at address %04x\n", m_grom_address);
	}
}

/****************************************************************************

    RPK loader

    RPK format support

    A RPK file ("rompack") contains a collection of dump files and a layout
    file that defines the kind of circuit board (PCB) used in the cartridge
    and the mapping of dumps to sockets on the board.

Example:
    <?xml version="1.0" encoding="utf-8"?>
    <romset>
        <resources>
            <rom id="gromimage" file="ed-assmg.bin" />
        </resources>
        <configuration>
            <pcb type="standard">
                <socket id="grom_socket" uses="gromimage"/>
            </pcb>
        </configuration>
    </romset>

DTD:
    <!ELEMENT romset (resources, configuration)>
    <!ELEMENT resources (rom|ram)+>
    <!ELEMENT rom EMPTY>
    <!ELEMENT ram EMPTY>
    <!ELEMENT configuration (pcb)>
    <!ELEMENT pcb (socket)+>
    <!ELEMENT socket EMPTY>
    <!ATTLIST romset version CDATA #IMPLIED>
    <!ATTLIST rom id ID #REQUIRED
    <!ATTLIST rom file CDATA #REQUIRED>
    <!ATTLIST rom crc CDATA #IMPLIED>
    <!ATTLIST rom sha1 CDATA #IMPLIED>
    <!ATTLIST ram id ID #REQUIRED>
    <!ATTLIST ram type (volatile|persistent) #IMPLIED>
    <!ATTLIST ram store (internal|external) #IMPLIED>
    <!ATTLIST ram file CDATA #IMPLIED>
    <!ATTLIST ram length CDATA #REQUIRED>
    <!ATTLIST pcb type CDATA #REQUIRED>
    <!ATTLIST socket id ID #REQUIRED>
    <!ATTLIST socket uses IDREF #REQUIRED>

****************************************************************************/

#include "unzip.h"
#include "xmlfile.h"

/****************************************
    RPK class
****************************************/
/*
    Constructor.
*/
rpk::rpk(emu_options& options, const char* sysname)
	:m_options(options)
	//,m_system_name(sysname)
{
	m_sockets.reset();
}

rpk::~rpk()
{
	if (VERBOSE>6) LOG("rpk: Destroy RPK\n");
}

/*
    Deliver the contents of the socket by name of the socket.
*/
UINT8* rpk::get_contents_of_socket(const char *socket_name)
{
	rpk_socket *socket = m_sockets.find(socket_name);
	if (socket==NULL) return NULL;
	return socket->get_contents();
}

/*
    Deliver the length of the contents of the socket by name of the socket.
*/
int rpk::get_resource_length(const char *socket_name)
{
	rpk_socket *socket = m_sockets.find(socket_name);
	if (socket==NULL) return 0;
	return socket->get_content_length();
}

void rpk::add_socket(const char* id, rpk_socket *newsock)
{
	m_sockets.append(id, *newsock);
}

/*-------------------------------------------------
    rpk_close - closes a rpk
    Saves the contents of the NVRAMs and frees all memory.
-------------------------------------------------*/

void rpk::close()
{
	// Save the NVRAM contents
	rpk_socket *socket = m_sockets.first();
	while (socket != NULL)
	{
		if (socket->persistent_ram())
		{
			image_battery_save_by_name(m_options, socket->get_pathname(), socket->get_contents(), socket->get_content_length());
		}
		socket->cleanup();
		socket = socket->m_next;
	}
}

/**************************************************************
    RPK socket (location in the PCB where a chip is plugged in;
    not a network socket)
***************************************************************/

rpk_socket::rpk_socket(const char* id, int length, UINT8* contents, const char *pathname)
: m_id(id), m_length(length), m_next(NULL), m_contents(contents), m_pathname(pathname)
{
}

rpk_socket::rpk_socket(const char* id, int length, UINT8* contents)
: m_id(id), m_length(length), m_next(NULL), m_contents(contents), m_pathname(NULL)
{
}

/*
    Locate a file in the ZIP container
*/
const zip_file_header* rpk_reader::find_file(zip_file *zip, const char *filename, UINT32 crc)
{
	const zip_file_header *header;
	for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
	{
		// We don't check for CRC == 0.
		if (crc != 0)
		{
			// if the CRC and name both match, we're good
			// if the CRC matches and the name doesn't, we're still good
			if (header->crc == crc)
				return header;
		}
		else
		{
			if (core_stricmp(header->filename, filename)==0)
			{
				return header;
			}
		}
	}
	return NULL;
}

/*
    Load a rom resource and put it in a pcb socket instance.
*/
rpk_socket* rpk_reader::load_rom_resource(zip_file* zip, xml_data_node* rom_resource_node, const char* socketname)
{
	const char* file;
	const char* crcstr;
	const char* sha1;
	zip_error ziperr;
	UINT32 crc;
	int length;
	UINT8* contents;
	const zip_file_header *header;

	// find the file attribute (required)
	file = xml_get_attribute_string(rom_resource_node, "file", NULL);
	if (file == NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "<rom> must have a 'file' attribute");

	if (VERBOSE>6) LOG("gromport/RPK: Loading ROM contents for socket '%s' from file %s\n", socketname, file);

	// check for crc
	crcstr = xml_get_attribute_string(rom_resource_node, "crc", NULL);
	if (crcstr==NULL)
	{
		// no CRC, just find the file in the RPK
		header = find_file(zip, file, 0);
	}
	else
	{
		crc = strtoul(crcstr, NULL, 16);
		header = find_file(zip, file, crc);
	}
	if (header == NULL) throw rpk_exception(RPK_INVALID_FILE_REF, "File not found or CRC check failed");

	length = header->uncompressed_length;

	// Allocate storage
	contents = global_alloc_array_clear(UINT8, length);
	if (contents==NULL) throw rpk_exception(RPK_OUT_OF_MEMORY);

	// and unzip file from the zip file
	ziperr = zip_file_decompress(zip, contents, length);
	if (ziperr != ZIPERR_NONE)
	{
		if (ziperr == ZIPERR_UNSUPPORTED) throw rpk_exception(RPK_ZIP_UNSUPPORTED);
		else throw rpk_exception(RPK_ZIP_ERROR);
	}

	// check for sha1
	sha1 = xml_get_attribute_string(rom_resource_node, "sha1", NULL);
	if (sha1 != NULL)
	{
		hash_collection actual_hashes;
		actual_hashes.compute((const UINT8 *)contents, length, hash_collection::HASH_TYPES_CRC_SHA1);

		hash_collection expected_hashes;
		expected_hashes.add_from_string(hash_collection::HASH_SHA1, sha1, strlen(sha1));

		if (actual_hashes != expected_hashes) throw rpk_exception(RPK_INVALID_FILE_REF, "SHA1 check failed");
	}

	// Create a socket instance
	return new rpk_socket(socketname, length, contents);
}

/*
    Load a ram resource and put it in a pcb socket instance.
*/
rpk_socket* rpk_reader::load_ram_resource(emu_options &options, xml_data_node* ram_resource_node, const char* socketname, const char* system_name)
{
	const char* length_string;
	const char* ram_type;
	const char* ram_filename;
	const char* ram_pname;
	unsigned int length;
	UINT8* contents;

	// find the length attribute
	length_string = xml_get_attribute_string(ram_resource_node, "length", NULL);
	if (length_string == NULL) throw rpk_exception(RPK_MISSING_RAM_LENGTH);

	// parse it
	char suffix = '\0';
	sscanf(length_string, "%u%c", &length, &suffix);
	switch(tolower(suffix))
	{
		case 'k': // kilobytes
			length *= 1024;
			break;

		case 'm':
			/* megabytes */
			length *= 1024*1024;
			break;

		case '\0':
			break;

		default:  // failed
			throw rpk_exception(RPK_INVALID_RAM_SPEC);
	}

	// Allocate memory for this resource
	contents = global_alloc_array_clear(UINT8, length);
	if (contents==NULL) throw rpk_exception(RPK_OUT_OF_MEMORY);

	if (VERBOSE>6) LOG("gromport/RPK: Allocating RAM buffer (%d bytes) for socket '%s'\n", length, socketname);

	ram_pname = NULL;

	// That's it for pure RAM. Now check whether the RAM is "persistent", i.e. NVRAM.
	// In that case we must load it from the NVRAM directory.
	// The file name is given in the RPK file; the subdirectory is the system name.
	ram_type = xml_get_attribute_string(ram_resource_node, "type", NULL);
	if (ram_type != NULL)
	{
		if (strcmp(ram_type, "persistent")==0)
		{
			// Get the file name (required if persistent)
			ram_filename = xml_get_attribute_string(ram_resource_node, "file", NULL);
			if (ram_filename==NULL)
			{
				global_free_array(contents);
				throw rpk_exception(RPK_INVALID_RAM_SPEC, "<ram type='persistent'> must have a 'file' attribute");
			}
			std::string ram_pathname = std::string(system_name).append(PATH_SEPARATOR).append(ram_filename);
			ram_pname = core_strdup(ram_pathname.c_str());
			// load, and fill rest with 00
			if (VERBOSE>6) LOG("gromport/RPK: Loading NVRAM contents from '%s'\n", ram_pname);
			image_battery_load_by_name(options, ram_pname, contents, length, 0x00);
		}
	}

	// Create a socket instance
	return new rpk_socket(socketname, length, contents, ram_pname);
}

/*-------------------------------------------------
    rpk_open - open a RPK file
    options - parameters from the settings; we need it only for the NVRAM directory
    system_name - name of the driver (also just for NVRAM handling)
-------------------------------------------------*/

rpk* rpk_reader::open(emu_options &options, const char *filename, const char *system_name)
{
	zip_error ziperr;

	const zip_file_header *header;
	const char *pcb_type;
	const char *id;
	const char *uses_name;
	const char *resource_name;

	zip_file* zipfile;

	std::vector<char> layout_text;
	xml_data_node *layout_xml = NULL;
	xml_data_node *romset_node;
	xml_data_node *configuration_node;
	xml_data_node *resources_node;
	xml_data_node *resource_node;
	xml_data_node *socket_node;
	xml_data_node *pcb_node;

	rpk_socket *newsock;

	int i;

	rpk *newrpk = new rpk(options, system_name);

	try
	{
		/* open the ZIP file */
		ziperr = zip_file_open(filename, &zipfile);
		if (ziperr != ZIPERR_NONE) throw rpk_exception(RPK_NOT_ZIP_FORMAT);

		/* find the layout.xml file */
		header = find_file(zipfile, "layout.xml", 0);
		if (header == NULL) throw rpk_exception(RPK_MISSING_LAYOUT);

		/* reserve space for the layout file contents (+1 for the termination) */
		layout_text.resize(header->uncompressed_length + 1);

		/* uncompress the layout text */
		ziperr = zip_file_decompress(zipfile, &layout_text[0], header->uncompressed_length);
		if (ziperr != ZIPERR_NONE)
		{
			if (ziperr == ZIPERR_UNSUPPORTED) throw rpk_exception(RPK_ZIP_UNSUPPORTED);
			else throw rpk_exception(RPK_ZIP_ERROR);
		}

		layout_text[header->uncompressed_length] = '\0';  // Null-terminate

		/* parse the layout text */
		layout_xml = xml_string_read(&layout_text[0], NULL);
		if (layout_xml == NULL) throw rpk_exception(RPK_XML_ERROR);

		// Now we work within the XML tree

		// romset is the root node
		romset_node = xml_get_sibling(layout_xml->child, "romset");
		if (romset_node==NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "document element must be <romset>");

		// resources is a child of romset
		resources_node = xml_get_sibling(romset_node->child, "resources");
		if (resources_node==NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "<romset> must have a <resources> child");

		// configuration is a child of romset; we're actually interested in ...
		configuration_node = xml_get_sibling(romset_node->child, "configuration");
		if (configuration_node==NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "<romset> must have a <configuration> child");

		// ... pcb, which is a child of configuration
		pcb_node = xml_get_sibling(configuration_node->child, "pcb");
		if (pcb_node==NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "<configuration> must have a <pcb> child");

		// We'll try to find the PCB type on the provided type list.
		pcb_type = xml_get_attribute_string(pcb_node, "type", NULL);
		if (pcb_type==NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "<pcb> must have a 'type' attribute");
		if (VERBOSE>6) LOG("gromport/RPK: Cartridge says it has PCB type '%s'\n", pcb_type);

		i=0;
		do
		{
			if (strcmp(pcb_type, m_types[i].name)==0)
			{
				newrpk->m_type = m_types[i].id;
				break;
			}
			i++;
		} while (m_types[i].id != 0);

		if (m_types[i].id==0) throw rpk_exception(RPK_UNKNOWN_PCB_TYPE);

		// Find the sockets and load their respective resource
		for (socket_node = pcb_node->child;  socket_node != NULL; socket_node = socket_node->next)
		{
			if (strcmp(socket_node->name, "socket")!=0) throw rpk_exception(RPK_INVALID_LAYOUT, "<pcb> element has only <socket> children");
			id = xml_get_attribute_string(socket_node, "id", NULL);
			if (id == NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "<socket> must have an 'id' attribute");
			uses_name = xml_get_attribute_string(socket_node, "uses", NULL);
			if (uses_name == NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "<socket> must have a 'uses' attribute");

			bool found = false;
			// Locate the resource node
			for (resource_node = resources_node->child; resource_node != NULL; resource_node = resource_node->next)
			{
				resource_name = xml_get_attribute_string(resource_node, "id", NULL);
				if (resource_name == NULL) throw rpk_exception(RPK_INVALID_LAYOUT, "resource node must have an 'id' attribute");

				if (strcmp(resource_name, uses_name)==0)
				{
					// found it
					if (strcmp(resource_node->name, "rom")==0)
					{
						newsock = load_rom_resource(zipfile, resource_node, id);
						newrpk->add_socket(id, newsock);
					}
					else
					{
						if (strcmp(resource_node->name, "ram")==0)
						{
							newsock = load_ram_resource(options, resource_node, id, system_name);
							newrpk->add_socket(id, newsock);
						}
						else throw rpk_exception(RPK_INVALID_LAYOUT, "resource node must be <rom> or <ram>");
					}
					found = true;
				}
			}
			if (!found) throw rpk_exception(RPK_INVALID_RESOURCE_REF, uses_name);
		}
	}
	catch (rpk_exception &exp)
	{
		newrpk->close();
		if (layout_xml != NULL)     xml_file_free(layout_xml);
		if (zipfile != NULL)        zip_file_close(zipfile);

		// rethrow the exception
		throw exp;
	}

	if (layout_xml != NULL)     xml_file_free(layout_xml);
	if (zipfile != NULL)        zip_file_close(zipfile);

	return newrpk;
}
