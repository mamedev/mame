// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Horizon Ramdisk

    This emulation realizes the latest development, the HRD 4000B, which can
    host up to 16 MiB of SRAM. The SRAM is buffered with a battery pack. Also,
    there is an option for an additional 32 KiB of unbuffered memory.

    The driver (ROS) of the ramdisk is stored in another buffered SRAM.
    Originally set to be 8 KiB, the 4000B offers four banks of 8 KiB storage
    for the driver.

    The Horizon RAMdisk comes with a disk containing the ROS and a configuration
    program (CFG).

    Technical details:

    The card is able to handle 128K*8 and 512K*8 SRAM chips, allowing a total
    of 16 MiB memory space. The card may be equipped with 0 to 32 SRAM chips;
    in this implementation, a multiple of 4 can be selected in that range.

    The Horizon 4000(B) implements the AMA decoding, unlike the the earlier
    models.

    Sockets must be populated contiguously from 0 to 15. Only when all sockets
    are populated, further SRAM circuits may be soldered on top of the bottom
    layer chips, again contiguously from 0 to 15.

    The respective CS* lines of the SRAM chips on the top layer must be
    connected with separate wires to the selector chip.

    === Normal mode ===

    In the normal (Horizon) mode, memory is organized as 2 KiB pages. The
    pages are selected via CRU bits and visible in the address area 5800 - 5fff.
    The area 4000-57ff is occupied by the ROS. As with most peripheral cards,
    the 4000-5fff area requires CRU bit 0 to be set.

    CRU bits:

       15     14      13     12  11  10   9   8   7   6       1    0
    +-----------------------------------------------------------------+
    | RAMBO | DSR  |  SRAM  | B | B | B | B | P | P | P ... | P | Ena |  512Kx8
    |       | bank |  Layer |   Bank (0-15) |   Page (0-255)    | ble |  RAM
    +-----------------------------------------------------------------+

       15     14    13  12    11   10   9   8   7   6         1    0
    +-----------------------------------------------------------------+
    | RAMBO | DSR  | - | - | SRAM | B | B | B | B | P | ... | P | Ena |  128Kx8
    |       | bank |   |   | Layer|  Bank (0-15)  |  Page(0-63) | ble |  RAM
    +-----------------------------------------------------------------+

    The difference between layer, banks, and pages results from the board
    design. Logically, the layer, bank, and page information can be seen as a
    13-bit page number for 512Kx8 RAMs or a 11-bit page number for 128Kx8 SRAMs.

    The 8-bit page number results from the capacity of a 512Kx8 chip (256 pages
    of 2 KiB), while the 6-bit number applies to 128Kx8 chips (64 pages of
    2 KiB). The bank selects the chip socket. The layer allows for selecting
    the second layer of chips, piggy-backed on the bottom layer.

    === RAMBO ===

    The RAMBO (RAM Block operator) mode gathers four pages to a single 8 KiB
    page in the area 6000-7fff (cartridge space). Due to an arguable design
    glitch in the real hardware (swapped A3/A4 lines), the four pages do not
    appear in their original order but as the base number plus 0 / 2 / 1 / 3.
    This can only be noticed when working in both modes (writing to pages in the
    normal mode, then mapping the group of pages via RAMBO).

    The above CRU bit chart also applies here, but bits 1 and 2 (least signi-
    ficant page number bits) are ignored.

    === Phoenix ===

    Using jumper JP2 and a DIP switch setting, the card can be configured to the
    Phoenix mode. This mode was originally intended to allow the Geneve to boot
    from the Horizon card, because the early boot ROMs did not support a boot
    drive larger than 256 KiB. Consequently, the RAM area is split, and one part
    realizes the boot drive while the other is still available for data.

    The Phoenix configuration splits the card in the middle, assigning the
    lower 8 sockets (bottom M0-M7, top M16-M23) to the Phoenix part, and the
    upper 8 sockets (bottom M8-M15, top M24-M31) to the normal part.

    The most significant bank selection bit (12 or 10) is ignored, because this
    bit determines the choice of upper and lower part. The selection is done
    in this way instead:

    In "TI mode", two separate CRU base addresses must be selected by the DIP
    switches. When an access occurs on the normal CRU address, the bit is set,
    while an access to the Phoenix CRU address resets the bit. Accesses occur
    when the page is set.

    In the "Geneve mode", only one CRU address is used. A CRU write access
    to bits 8-15 sets the bit to 0, and a CRU write access to bits 0-7 sets
    the bit to 1. Since the bits keep their semantics, care must be taken in
    which order the bits are written to. For this reason, the Phoenix mode is
    rarely used.

    Later investigations proved that the GeneveOS could be loaded normally from
    the large ramdisk, and the Phoenix mode became obsolete. It is included for
    test purposes and for the sake of completeness.

    The "Split mode" for TI and Geneve can be used without splitting the RAM
    space, i.e. a Phoenix address may be set without JP2 set to split. In that
    case, only the DSR RAM is split in two halves (if its size is 32KiB).

    References

    [1] Software Developer's Guide to the HRD4000B, Sept. 2020
    [2] Schematics for the HRD4000B
    [3] Horizon 4000 Ramdisk Construction Guide, Bud Mills Services, 1992

    Original design of the hardware (2000) by Horizon Computer, Ltd., 1986
    Extensions (3000, 4000) by Bud Mills Services, 1989, 1992
    HRD 4000B by Jim Fetzner et al., 2019

    Michael Zapf
    November 2020

*****************************************************************************/

#include "emu.h"
#include "horizon.h"

#define LOG_WARN        (1U<<1)   // Warnings
#define LOG_CONFIG      (1U<<2)   // Configuration
#define LOG_32K         (1U<<3)   // 32K optional RAM r/w
#define LOG_DSR         (1U<<4)   // DSR
#define LOG_RAM         (1U<<5)   // RAM chips
#define LOG_ORAM        (1U<<6)   // outside of RAM chips
#define LOG_CRU         (1U<<7)   // CRU
#define LOG_PAGE        (1U<<8)   // Page access

#define VERBOSE ( LOG_GENERAL | LOG_CONFIG | LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_HORIZON, bus::ti99::peb::horizon_ramdisk_device, "ti99_horizon", "Horizon 4000B RAMdisk")

namespace bus::ti99::peb {

#define CRULATCH1_TAG "u4_latch"
#define CRULATCH2_TAG "u3_latch"

#define DSRRAM_TAG "u9_dsrram"
#define OPT32K_TAG "m32_ram"
#define RAM16M_TAG "m31_m0_ram"

horizon_ramdisk_device::horizon_ramdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, TI99_HORIZON, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_ram(*this, RAM16M_TAG),
	m_dsrram(*this, DSRRAM_TAG),
	m_optram(*this, OPT32K_TAG),
	m_crulatch_u4(*this, CRULATCH1_TAG),
	m_crulatch_u3(*this, CRULATCH2_TAG),
	m_32k_installed(true),
	m_phoenix_accessed(false),
	m_dsr32k(true),
	m_128kx8(true),
	m_geneve_mode(false),
	m_phoenix_split(false),
	m_hideswitch(false),
	m_rambo_supported(false),
	m_modified(false),
	m_page(0),
	m_bank(0),
	m_ramsize(0),
	m_cru_base_horizon(0),
	m_cru_base_phoenix(0)
{
}

void horizon_ramdisk_device::readz(offs_t offset, uint8_t *value)
{
	read_write(offset, value, false);
}

void horizon_ramdisk_device::write(offs_t offset, uint8_t data)
{
	read_write(offset, &data, true);
}

/*
    Read from or write to the card. This can involve the
    a) optional 32K RAM (M32)
    b) DSR RAM (U9)
    c) set of RAM chips (M0-M31)
*/
void horizon_ramdisk_device::read_write(offs_t offset, uint8_t *value, bool write)
{
	// If AMA/B/C != 111, do not allow access. This "AMA decoding" was done
	// by default by TI cards, but third-party cards did not comply in all cases.

	// HRD4000(B) used this decoding (U13); earlier Horizon cards did not
	// implement it
	if ((offset & 0x70000) != 0x70000) return;

	// 32K expansion
	// According to [3], this memory is not affected by the HIDE switch
	// Also, it cannot be mapped out by a CRU bit
	//
	//     offset       a14  a13
	// ----------------
	// |   E000       |  0   0
	// ----------------
	// |   C000       |  0   1
	// ----------------
	// |   A000       |  1   0
	// ----------------
	// |   2000       |  1   1
	// ----------------
	//
	// a14=/c*/e, a13=/a*/e    (pins of the memory chip, not the address bus)

	int sect = (offset>>13) & 0x07;  // 8K sections in the 64K address space
	int offset8k = (offset & 0x1fff);
	int offset2k = (offset & 0x07ff);

	if (m_32k_installed)
	{
		// Decoding by U12 and U11
		int a14 = ((sect != 6) && (sect != 7))? 0x4000 : 0;  // a14: 11x
		int a13 = ((sect != 5) && (sect != 7))? 0x2000 : 0;  // a13: 1x1

		if ((sect == 1) || (sect == 5) || (sect == 6) || (sect == 7))
		{
			if (write)
				m_optram->write(offset8k | a14 | a13, *value);
			else
				*value = m_optram->read(offset8k | a14 | a13);

			LOGMASKED(LOG_32K, "offset=%04x (32K) %s %02x\n", offset&0xffff, write? "<-" : "->", *value);
			return;
		}
	}

	// DSR RAM is not affected by SW2 either.
	// CRU bit 0 must be set to 1 to activate it.
	// Decoding by U12 and U7
	int b_a = offset & 0x1800;
	if (m_rambo_supported && m_crulatch_u3->q7_r()) b_a &= 0x0f00;  // clear A3 (RAMBO mode)

	bool dsr = m_crulatch_u4->q0_r() && (sect == 2);

	if (dsr && (b_a != 0x1800))
	{
		// DSR access
		// All cards 2000-4000 have 8K RAM here
		// The 4000B supports up to 32K; 4 banks can be selected
		int a13 = (m_crulatch_u3->q6_r())? 0x2000 : 0;
		int a14 = (!m_phoenix_accessed)? 0x4000 : 0;

		if (write)
			m_dsrram->write(offset8k | (m_dsr32k? (a13 | a14) : 0), *value);
		else
			*value = m_dsrram->read(offset8k | (m_dsr32k? (a13 | a14) : 0));

		LOGMASKED(LOG_DSR, "offset=%04x (DSR) %s %02x\n", offset&0xffff, write? "<-" : "->", *value);
		return;
	}

	// Lower part of U7
	if ((dsr && (b_a == 0x1800)) || ((sect == 3) && m_rambo_supported &&  m_crulatch_u3->q7_r()))
	{
		// Access to a selected RAM
		// Either in normal mode (on 5800) or in RAMBO mode (6000-7fff)
		int page = m_page;
		int ramaddress = offset2k;

		// RAMBO mode
		// If we are here in RAMBO mode, we are accessing 6000-7fff,
		// because for sect 2 (4000-5fff), b_a was changed to be != 1800
		if (m_rambo_supported && m_crulatch_u3->q7_r())
		{
			// Replace the last two bits of the page number by address bits A3 and A4
			// ...xx...........
			// According to the specs, the lines must be swapped
			// Hence, the 2k page number sequence in the 6000 space is
			// base + 0 2 1 3
			int a3_4 = (((offset & 0x0800)>>10) | ((offset & 0x1000)>>12));
			page = (page & 0xfc) | a3_4;
		}

		// Just debugging stuff
		if (page != m_current_page || m_bank != m_current_bank)
		{
			LOGMASKED(LOG_PAGE, "Bank %02x, page %02x\n", m_bank, page);
			m_current_bank = m_bank;
			m_current_page = page;
		}

		if (m_128kx8)
			// bb bbbp pppp p... .... ....
			ramaddress |= ((page << 11) | (m_bank << 17));
		else
			// bbbb bppp pppp p... .... ....
			ramaddress |= ((page << 11) | (m_bank << 19));

		if (ramaddress < m_ramsize)
		{
			if (write)
			{
				m_modified = true;
				m_ram->write(ramaddress, *value);
			}
			else
				*value = m_ram->read(ramaddress);

			LOGMASKED(LOG_RAM, "offset=%04x, page=%02x, bank=%02x %s %02x\n", offset&0xffff, page, m_bank, write? "<-" : "->", *value);
		}
		else
			LOGMASKED(LOG_ORAM, "offset=%04x, page=%02x, bank=%02x %s outside of RAM space\n", offset&0xffff, page, m_bank, write? "write" : "read");
	}
}

/*
    CRU read operation. There is not such read feature on the Horizon.
*/
void horizon_ramdisk_device::crureadz(offs_t offset, uint8_t *value)
{
	return;
}

/*
    CRU write operation.
*/
void horizon_ramdisk_device::cruwrite(offs_t offset, uint8_t data)
{
	// Horizon and Phoenix set to off = ff00 will never match
	if (((offset & 0x1f00) != m_cru_base_horizon) && ((offset & 0x1f00) != m_cru_base_phoenix))
		return;

	int bit = (offset >> 1) & 0x0f;

	// Set the latch U5
	// For Geneve mode, it is set when a bit 8-15 is accessed; for the TI mode
	// it is set for an access to the Phoenix CRU area
	m_phoenix_accessed = m_geneve_mode? (bit >= 8) : ((offset & 0x1f00) == m_cru_base_phoenix);

	// Set the latches U4 and U3 (unless the SW2 hideswitch is turned on)
	if (m_hideswitch==false)
	{
		if (bit < 8)
		{
			m_crulatch_u4->write_bit(bit & 7, data & 1);
		}
		else
		{
			m_crulatch_u3->write_bit(bit & 7, data & 1);
		}
		get_address_prefix();
	}
}

/*
    Translate latch settings to page and bank.
    Also called after savestate load.
    This method is used to avoid reading all settings every time that a memory
    access occurs.
*/
void horizon_ramdisk_device::get_address_prefix()
{
	u8 latch1 = m_crulatch_u4->output_state();
	u8 latch2 = m_crulatch_u3->output_state();

	// Latch 2   Latch 1
	// ..lbbbbp  ppppppp.   512k
	// ....lbbb  bpppppp.   128k
	// l=layer b=bank p=page

	if (m_128kx8)
	{
		m_page = (latch1 >> 1) & 0x3f;
		m_bank = ((latch1 >> 7) & 0x01) | ((latch2 << 1) & 0x1e);
	}
	else
	{
		m_page = ((latch1 >> 1) & 0x7f) | ((latch2 << 7) & 0x80);
		m_bank = (latch2 >> 1) & 0x1f;
	}

	// Phoenix jumper JP2
	if (m_phoenix_split)
	{
		m_bank &= 0x08;  // Clear the D bit
		if (!m_phoenix_accessed) m_bank |= 0x08;
	}
}

void horizon_ramdisk_device::device_start(void)
{
	machine().save().register_postload(save_prepost_delegate(FUNC(horizon_ramdisk_device::get_address_prefix),this));
}

void horizon_ramdisk_device::device_reset(void)
{
	m_cru_base_horizon = ioport("CRUHOR")->read();
	m_cru_base_phoenix = ioport("CRUPHOE")->read();
	m_32k_installed = (ioport("OPT32K")->read()!=0);
	m_phoenix_split = (ioport("PHOENIX")->read()!=0);
	m_hideswitch = (ioport("HIDESW2")->read()!=0);
	m_dsr32k = (ioport("DSRSIZE")->read()!=0);
	m_128kx8 = (ioport("CHIPTYPE")->read()==0);
	m_geneve_mode = (ioport("MODE")->read()!=0);
	m_rambo_supported = (ioport("RAMBO")->read()!=0);

	int dsrsize = 0;
	get_mem_size(m_ramsize, dsrsize);
	LOGMASKED(LOG_CONFIG, "Horizon card memory: %d bytes\n", m_ramsize);
}

INPUT_CHANGED_MEMBER( horizon_ramdisk_device::hs_changed )
{
	if (param == 0)
	{
		LOGMASKED(LOG_CONFIG, "Hideswitch changed to %d\n", newval);
		m_hideswitch = (newval!=0);
	}
	else
	{
		LOGMASKED(LOG_CONFIG, "Phoenix split setting changed to %d\n", newval);
		m_phoenix_split = (newval!=0);
	}
}

/*
    NVRAM support
    The size of the file is num_chips * mem_per_chip + dsr_size
    The contents of the RAM banks are stored first, then the
    DSR chip contents are appended

    0                 end
    ____________________
    |   RAM      | DSR |
    --------------------
*/
void horizon_ramdisk_device::nvram_default()
{
	int ramsize, dsrsize;
	get_mem_size(ramsize, dsrsize);

	if (ramsize > 0) memset(m_ram->pointer(), 0,  ramsize);
	memset(m_dsrram->pointer(), 0, dsrsize);
}


bool horizon_ramdisk_device::nvram_read(util::read_stream &file)
{
	int ramsize, dsrsize;
	get_mem_size(ramsize, dsrsize);

	// NVRAM plus ROS, according to the current configuration
	auto buffer = make_unique_clear<uint8_t []>(ramsize + dsrsize);

	if (ramsize > 0) memset(m_ram->pointer(), 0,  ramsize);
	memset(m_dsrram->pointer(), 0, dsrsize);

	// Read complete file, at most ramsize+dsrsize
	// Mind that the configuration may have changed
	size_t filesize;
	if (file.read(&buffer[0], ramsize + dsrsize, filesize))
		return false;
	int nvramsize = int(filesize) - dsrsize;

	// At least the DSR must be complete
	if (nvramsize >= 0)
	{
		// Copy from buffer to NVRAM and ROS
		if (nvramsize > 0) memcpy(m_ram->pointer(), &buffer[0], nvramsize);
		memcpy(m_dsrram->pointer(), &buffer[nvramsize], dsrsize);

		return true;
	}

	return false;
}

bool horizon_ramdisk_device::nvram_write(util::write_stream &file)
{
	int ramsize, dsrsize;
	get_mem_size(ramsize, dsrsize);

	// NVRAM plus ROS, according to the current configuration
	auto buffer = make_unique_clear<uint8_t []>(ramsize + dsrsize);

	memcpy(&buffer[0], m_ram->pointer(), ramsize);
	memcpy(&buffer[ramsize], m_dsrram->pointer(), dsrsize);

	// Store both parts in one file
	size_t filesize;
	return !file.write(buffer.get(), ramsize + dsrsize, filesize) && filesize == ramsize + dsrsize;
}

bool horizon_ramdisk_device::nvram_can_write()
{
	// Do not save if nothing was written. This is helpful to avoid loss of the
	// contents when the settings were found to be different, and the emulation
	// has to be restarted after restoring the settings.
	return m_modified;
}


void horizon_ramdisk_device::get_mem_size(int& ramsize, int& dsrsize)
{
	int chipsize = 128*1024*((ioport("CHIPTYPE")->read()*3)+1);
	ramsize = (ioport("CHIPCOUNT")->read()*4) * chipsize;
	dsrsize = ((ioport("DSRSIZE")->read()*3)+1)*8192;
}

/*
    Input ports for the Horizon
*/
INPUT_PORTS_START( horizon )
	PORT_START( "CRUHOR" )
	PORT_DIPNAME( 0xff00, 0x1200, "SW1 Horizon CRU base" )
		PORT_DIPSETTING(    0xff00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x1000, "1000" )     // 1100 and 1300 not available
		PORT_DIPSETTING(    0x1200, "1200" )
		PORT_DIPSETTING(    0x1400, "1400" )
		PORT_DIPSETTING(    0x1500, "1500" )
		PORT_DIPSETTING(    0x1600, "1600" )
		PORT_DIPSETTING(    0x1700, "1700" )

	PORT_START( "CRUPHOE" )
	PORT_DIPNAME( 0xff00, 0xff00, "SW1 Phoenix CRU base" )
		PORT_DIPSETTING(    0xff00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x1400, "1400" )    // must not be used when selected for Horizon
		PORT_DIPSETTING(    0x1600, "1600" )

	PORT_START( "MODE" )
	PORT_DIPNAME( 0x01, 0x00, "JP4 Split mode" )
		PORT_DIPSETTING(    0x00, "TI mode" )
		PORT_DIPSETTING(    0x01, "Geneve mode" )

	PORT_START( "HIDESW2" )
	PORT_DIPNAME( 0x01, 0x00, "SW2 Hideswitch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, horizon_ramdisk_device, hs_changed, 0)
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_START( "PHOENIX" )
	PORT_DIPNAME( 0x01, 0x00, "JP2 Phoenix split" ) PORT_CHANGED_MEMBER(DEVICE_SELF, horizon_ramdisk_device, hs_changed, 1)
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )

// --------------------------------------------------------------------

	PORT_START( "CHIPTYPE" )
	PORT_CONFNAME( 0x01, 0x00, "Memory circuit type" )
		PORT_CONFSETTING( 0x00, "128Kx8")
		PORT_CONFSETTING( 0x01, "512Kx8")

	PORT_START( "CHIPCOUNT" )
	PORT_CONFNAME( 0x0f, 0x04, "Memory circuit count" )
		PORT_CONFSETTING( 0x00, "0")
		PORT_CONFSETTING( 0x01, "4")
		PORT_CONFSETTING( 0x02, "8")
		PORT_CONFSETTING( 0x03, "12")
		PORT_CONFSETTING( 0x04, "16")
		PORT_CONFSETTING( 0x05, "20")
		PORT_CONFSETTING( 0x06, "24")
		PORT_CONFSETTING( 0x07, "28")
		PORT_CONFSETTING( 0x08, "32")

	PORT_START( "DSRSIZE" )
	PORT_CONFNAME( 0x01, 0x00, "DSR memory size" )
		PORT_CONFSETTING( 0x00, "8 KiB" )
		PORT_CONFSETTING( 0x01, "32 KiB" )

	PORT_START( "OPT32K" )
	PORT_CONFNAME( 0x01, 0x00, "Optional 32 KiB memory" )
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( 0x01, DEF_STR( On ))

	PORT_START( "RAMBO" )
	PORT_CONFNAME( 0x01, 0x00, "RAMBO support" )
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( 0x01, DEF_STR( On ))

INPUT_PORTS_END

void horizon_ramdisk_device::device_add_mconfig(machine_config &config)
{
	// It could make sense to use buffered_ram (BUFF_RAM) for the 16M RAM and
	// the DSR RAM which has its own NVRAM handler. However, we want to have
	// a configurable emulation, and the config switches are not available
	// before the NVRAM handlers of the subdevices kick in. It could be done
	// if the NVRAM files always use the same (maximum) size.

	RAM(config, RAM16M_TAG).set_default_size("16M");
	RAM(config, DSRRAM_TAG).set_default_size("32K");
	RAM(config, OPT32K_TAG).set_default_size("32K").set_default_value(0);

	// CRU latches
	LS259(config, m_crulatch_u4);
	LS259(config, m_crulatch_u3);
}

ioport_constructor horizon_ramdisk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(horizon);
}

} // end namespace bus::ti99::peb
