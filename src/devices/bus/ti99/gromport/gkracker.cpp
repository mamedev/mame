// license:LGPL-2.1+
// copyright-holders:Michael Zapf
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
#include "emu.h"
#include "gkracker.h"

#define LOG_WARN         (1U<<1)   // Warnings
#define LOG_CHANGE       (1U<<2)   // Cartridge change
#define LOG_GKRACKER     (1U<<3)   // Gram Kracker operation

#define VERBOSE ( LOG_WARN )
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_GROMPORT_GK, bus::ti99::gromport::ti99_gkracker_device,         "ti99_gkracker",  "Miller's Graphics GRAM Kracker")

namespace bus::ti99::gromport {

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

#define GKRACKER_NVRAM_TAG "gkracker_nvram"
#define GKRACKER_ROM_TAG "gkracker_rom"

ti99_gkracker_device::ti99_gkracker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:   cartridge_connector_device(mconfig, TI99_GROMPORT_GK, tag, owner, clock),
		device_nvram_interface(mconfig, *this),
		m_romspace_selected(false),
		m_ram_page(0),
		m_grom_address(0),
		m_ram_ptr(nullptr),
		m_grom_ptr(nullptr),
		m_waddr_LSB(false),
		m_cartridge(nullptr)
{
}

WRITE_LINE_MEMBER(ti99_gkracker_device::romgq_line)
{
	m_romspace_selected = (state==ASSERT_LINE);
	// Propagate to the guest
	if (m_cartridge != nullptr) m_cartridge->romgq_line(state);
}

/*
    Combined select lines
*/
void ti99_gkracker_device::set_gromlines(line_state mline, line_state moline, line_state gsq)
{
	m_grom_selected = (gsq==ASSERT_LINE);
	if (m_cartridge != nullptr) m_cartridge->set_gromlines(mline, moline, gsq);
}

WRITE_LINE_MEMBER(ti99_gkracker_device::gclock_in)
{
	if (m_cartridge != nullptr) m_cartridge->gclock_in(state);
}

/*
    Check whether the GROMs are idle.
*/
bool ti99_gkracker_device::is_grom_idle()
{
	return (m_cartridge != nullptr)? m_cartridge->is_grom_idle() : false;
}

void ti99_gkracker_device::readz(offs_t offset, uint8_t *value)
{
	if (m_grom_selected)
	{
		// Reads from the GRAM space of the GRAM Kracker.
		int id = ((m_grom_address & 0xe000)>>13)&0x07;

		// The GK does not have a readable address counter, but the console
		// GROMs and the GROMs of the guest cartridge will keep our address
		// counter up to date.
		if ((offset & 0x0002)==0)
		{
			// Reading data
			if    (((id==0) && (m_gk_switch[2]==GK_GRAM0))
				|| ((id==1) && (m_gk_switch[5]==GK_LDOFF) && (m_gk_switch[3]==GK_GRAM12))
				|| ((id==2) && (m_gk_switch[3]==GK_GRAM12))
				|| ((id>=3) && (m_gk_switch[1]==GK_NORMAL)))
				*value = m_ram_ptr[m_grom_address];

			if ((id==1) && (m_gk_switch[5]==GK_LDON))
				*value = m_grom_ptr[m_grom_address & 0x1fff];

			// The GK GROM emulation does not wrap at 8K boundaries.
			m_grom_address = (m_grom_address + 1) & 0xffff;

			// Reset the write address flipflop.
			m_waddr_LSB = false;
			LOGMASKED(LOG_GKRACKER, "GROM read -> %02x\n", *value);
		}
	}

	if (m_romspace_selected)
	{
		// Reads from the RAM space of the GRAM Kracker.

		// RAM is stored behind the GRAM area
		// Note that offset is 0000...1fff
		// When switch in middle position (WP) do bank select according to page flag
		if (m_gk_switch[1] == GK_NORMAL)
		{
			int base = ((m_gk_switch[4]==GK_BANK1) || ((m_gk_switch[4]==GK_WP) && (m_ram_page==0)))? 0x10000 : 0x12000;
			*value = m_ram_ptr[offset | base];
			LOGMASKED(LOG_GKRACKER, "Read %04x -> %02x\n", offset | 0x6000, *value);
		}
	}

	// If the guest has GROMs or ROMs they will override the GK contents
	if (m_cartridge != nullptr)
	{
		// For debugging
		uint8_t val1 = *value;

		// Read from the guest cartridge.
		m_cartridge->readz(offset, value);
		if (val1 != *value)
			LOGMASKED(LOG_GKRACKER, "Read (from guest) %04x -> %02x\n", offset, *value);
	}
}

void ti99_gkracker_device::write(offs_t offset, uint8_t data)
{
	// write to the guest cartridge if present
	if (m_cartridge != nullptr)
	{
		m_cartridge->write(offset, data);
	}

	if (m_grom_selected)
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
				LOGMASKED(LOG_GKRACKER, "Set GROM address %04x\n", m_grom_address);
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
			LOGMASKED(LOG_GKRACKER, "GROM write %04x(%04x) <- %02x\n", offset, m_grom_address, data);

			// According to manual:
			// Writing to GRAM 0: switch 2 set to GRAM 0 + Write protect switch (4) in 1 or 2 position
			// Writing to GRAM 1: switch 3 set to GRAM 1-2 + Loader off (5); write prot has no effect
			// Writing to GRAM 2: switch 3 set to GRAM 1-2 (write prot has no effect)
			// Writing to GRAM 3-7: switch 1 set to GK_NORMAL, no cartridge inserted
			// GK_NORMAL switch has no effect on GRAM 0-2

			int id = ((m_grom_address & 0xe000)>>13)&0x07;

			if    ((id==0 && m_gk_switch[2]==GK_GRAM0 && m_gk_switch[4]!=GK_WP)
				|| (id==1 && m_gk_switch[3]==GK_GRAM12 && m_gk_switch[5]==GK_LDOFF)
				|| (id==2 && m_gk_switch[3]==GK_GRAM12)
				|| (id>=3 && m_gk_switch[1]==GK_NORMAL))
				m_ram_ptr[m_grom_address] = data;

			// The GK GROM emulation does not wrap at 8K boundaries.
			m_grom_address = (m_grom_address + 1) & 0xffff;

			// Reset the write address flipflop.
			m_waddr_LSB = false;
		}
	}

	if (m_romspace_selected)
	{
		// Write to the RAM space of the GRAM Kracker
		LOGMASKED(LOG_GKRACKER, "Write %04x <- %02x\n", offset | 0x6000, data);

		if (m_gk_switch[1] == GK_NORMAL)
		{
			if (m_gk_switch[4]==GK_BANK1) m_ram_ptr[offset | 0x10000] = data;
			else if (m_gk_switch[4]==GK_BANK2) m_ram_ptr[offset | 0x12000] = data;
			// Switch in middle position (WP, implies auto-select according to the page flag)
			// This is handled like in Extended Basic (using addresses)
			else m_ram_page = (offset >> 1) & 1;
		}
	}
}

void ti99_gkracker_device::crureadz(offs_t offset, uint8_t *value)
{
	if (m_cartridge != nullptr) m_cartridge->crureadz(offset, value);
}

void ti99_gkracker_device::cruwrite(offs_t offset, uint8_t data)
{
	if (m_cartridge != nullptr) m_cartridge->cruwrite(offset, data);
}

INPUT_CHANGED_MEMBER( ti99_gkracker_device::gk_changed )
{
	LOGMASKED(LOG_GKRACKER, "Input changed %d - %d\n", int(param & 0x07), newval);
	m_gk_switch[param & 0x07] = newval;
}

void ti99_gkracker_device::insert(int index, ti99_cartridge_device* cart)
{
	LOGMASKED(LOG_CHANGE, "Insert cartridge\n");
	m_cartridge = cart;
	// Switch 1 has a third location for resetting. We do the reset by default
	// here. It can be turned off in the configuration.
	m_gromport->cartridge_inserted();
}

void ti99_gkracker_device::remove(int index)
{
	LOGMASKED(LOG_CHANGE, "Remove cartridge\n");
	m_cartridge = nullptr;
}

void ti99_gkracker_device::gk_install_menu(const char* menutext, int len, int ptr, int next, int start)
{
	const int base = 0x0000;
	m_ram_ptr[base + ptr] = (uint8_t)((next >> 8) & 0xff);
	m_ram_ptr[base + ptr+1] = (uint8_t)(next & 0xff);
	m_ram_ptr[base + ptr+2] = (uint8_t)((start >> 8) & 0xff);
	m_ram_ptr[base + ptr+3] = (uint8_t)(start & 0xff);

	m_ram_ptr[base + ptr+4] = (uint8_t)(len & 0xff);
	memcpy(m_ram_ptr + base + ptr+5, menutext, len);
}

/*
    Define the default for the GRAM Kracker device. The memory is preset with
    some sample entries which shall indicate that the memory has been tested
    by the manufacturer.
*/
void ti99_gkracker_device::nvram_default()
{
	LOGMASKED(LOG_GKRACKER, "Creating default NVRAM\n");
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

bool ti99_gkracker_device::nvram_read(util::read_stream &file)
{
	size_t readsize;
	if (file.read(m_ram_ptr, 81920, readsize))
		return false;
	LOGMASKED(LOG_GKRACKER, "Reading NVRAM\n");
	// If we increased the size, fill the remaining parts with 0
	if (readsize < 81920)
	{
		memset(m_ram_ptr + readsize, 0, 81920-readsize);
	}
	return true;
}

bool ti99_gkracker_device::nvram_write(util::write_stream &file)
{
	LOGMASKED(LOG_GKRACKER, "Writing NVRAM\n");
	size_t writesize;
	return !file.write(m_ram_ptr, 81920, writesize) && writesize == 81920;
}

void ti99_gkracker_device::device_start()
{
	m_ram_ptr = memregion(GKRACKER_NVRAM_TAG)->base();
	m_grom_ptr = memregion(GKRACKER_ROM_TAG)->base();
	m_cartridge = nullptr;
	for (int i=1; i < 6; i++) m_gk_switch[i] = 0;
	save_pointer(NAME(m_gk_switch),6);
	save_item(NAME(m_romspace_selected));
	save_item(NAME(m_ram_page));
	save_item(NAME(m_grom_address));
	save_item(NAME(m_waddr_LSB));
}

void ti99_gkracker_device::device_reset()
{
	m_gk_switch[1] = ioport(GKSWITCH1_TAG)->read();
	m_gk_switch[2] = ioport(GKSWITCH2_TAG)->read();
	m_gk_switch[3] = ioport(GKSWITCH3_TAG)->read();
	m_gk_switch[4] = ioport(GKSWITCH4_TAG)->read();
	m_gk_switch[5] = ioport(GKSWITCH5_TAG)->read();
	m_grom_address = 0; // for the GROM emulation
	m_ram_page = 0;
	m_waddr_LSB = false;
	m_grom_selected = false;
}

/*
    The GRAMKracker ROM
*/
ROM_START( gkracker_rom )
	ROM_REGION(0x14000, GKRACKER_NVRAM_TAG, ROMREGION_ERASE00)
	ROM_REGION(0x2000, GKRACKER_ROM_TAG, 0)
	ROM_LOAD("gkracker.bin", 0x0000, 0x2000, CRC(86eaaf9f) SHA1(a3bd5257c63e190800921b52dbe3ffa91ad91113))
ROM_END

const tiny_rom_entry *ti99_gkracker_device::device_rom_region() const
{
	return ROM_NAME( gkracker_rom );
}

void ti99_gkracker_device::device_add_mconfig(machine_config &config)
{
	TI99_CART(config, "cartridge", 0);
}

INPUT_PORTS_START(gkracker)
	PORT_START( GKSWITCH1_TAG )
	PORT_DIPNAME( 0x01, 0x01, "GK switch 1" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti99_gkracker_device, gk_changed, 1)
		PORT_DIPSETTING(    0x00, "GK Off" )
		PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )

	PORT_START( GKSWITCH2_TAG )
	PORT_DIPNAME( 0x01, 0x01, "GK switch 2" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti99_gkracker_device, gk_changed, 2)
		PORT_DIPSETTING(    0x00, "GRAM 0" )
		PORT_DIPSETTING(    0x01, "Op Sys" )

	PORT_START( GKSWITCH3_TAG )
	PORT_DIPNAME( 0x01, 0x01, "GK switch 3" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti99_gkracker_device, gk_changed, 3)
		PORT_DIPSETTING(    0x00, "GRAM 1-2" )
		PORT_DIPSETTING(    0x01, "TI BASIC" )

	PORT_START( GKSWITCH4_TAG )
	PORT_DIPNAME( 0x03, 0x01, "GK switch 4" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti99_gkracker_device, gk_changed, 4)
		PORT_DIPSETTING(    0x00, "Bank 1" )
		PORT_DIPSETTING(    0x01, "W/P" )
		PORT_DIPSETTING(    0x02, "Bank 2" )

	PORT_START( GKSWITCH5_TAG )
	PORT_DIPNAME( 0x01, 0x00, "GK switch 5" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti99_gkracker_device, gk_changed, 5)
		PORT_DIPSETTING(    0x00, "Loader On" )
		PORT_DIPSETTING(    0x01, "Loader Off" )
INPUT_PORTS_END

ioport_constructor ti99_gkracker_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gkracker);
}

} // end namespace bus::ti99::gromport

