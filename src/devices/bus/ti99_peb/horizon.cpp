// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Horizon Ramdisk

    This emulation realizes the latest development, the HRD 4000, which could
    host up to 16 MiB of SRAM. Real cards rarely had more than 1.5 MiB since
    the SRAM used on the card is rather expensive.

    The SRAM is buffered with a battery pack. Also, there is an option for
    an additional 32 KiB of unbuffered memory.

    The driver (ROS) of the ramdisk is stored in another buffered 8 KiB SRAM.

    The Horizon RAMdisk comes with a disk containing the ROS and a configuration
    program (CFG). The latest version is ROS 8.14.

    Technical details:

    In the tradition (Horizon) mode, memory is organized as 2 KiB pages. The
    pages are selected via CRU bits and visible in the address area 5800 - 5fff.
    The area 4000-57ff is occupied by the ROS. As with all peripheral cards,
    the 4000-5fff area requires a CRU bit to be set (usually bit 0 of this
    card's CRU base).

    Next releases of the HRD included new modes. The RAMBO (RAM Block operator)
    mode gathers four pages to a single 8 KiB page that is visible in the
    area 6000-7fff (cartridge space). Note that due to a possible design glitch,
    each RAMBO page n covers Horizon pages 4n, 4n+2, 4n+1, 4n+3 in this sequence.
    We emulate this by swapping two CRU lines.

    The RAMDisk may be split in two separate drives, which is called the
    Phoenix extension. This is particularly important for use in the Geneve.
    As a bootable drive, the RAMdisk must not
    exceed 256 KiB; consequently, the RAM area is split, and one part realizes
    the boot drive while the other is still available for data. Also, there
    is a mechanism for selecting the parts of the card: The TI setting allows
    to select two CRU addresses, one for each part. In the Geneve mode, only
    one CRU address is used (1400 or 1600), and the part is selected by the
    fact that one disk uses CRU bits higher than 8, while the other uses the
    bits lower than 8.

    The card is able to handle 128K*8 and 512K*8 SRAM chips, allowing a total
    of 16 MiB memory space. Unfortunately, a bug causes the configuration
    program to crash when used with more than 2 MiB. Although the card was
    quite popular, this bug was not found because most cards were sold with
    less than 2 MiB onboard. As the community is still alive we can hope for
    a fix for this problem; so we make the size configurable.

    Michael Zapf
    February 2012

*****************************************************************************/

#include "horizon.h"

#define RAMREGION "ram"
#define NVRAMREGION "nvram"
#define ROSREGION "ros"

// Paged RAM is max 16 MiB; behind we add the 8 KiB for the buffered RAM for the ROS
#define MAXRAM_SIZE 16777216+8192

#define VERBOSE 1
#define LOG logerror

horizon_ramdisk_device::horizon_ramdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_HORIZON, "Horizon 4000 Ramdisk", tag, owner, clock,"ti99_horizon",__FILE__),
	device_nvram_interface(mconfig, *this), m_ram(nullptr), m_nvram(nullptr), m_ros(nullptr), m_select6_value(0), m_select_all(0), m_page(0), m_cru_horizon(0), m_cru_phoenix(0), m_timode(false), m_32k_installed(false), m_split_mode(false), m_rambo_mode(false), m_killswitch(false), m_use_rambo(false)
{
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

int horizon_ramdisk_device::get_size()
{
	int size = 8192 + 2097152*(1 << ioport("HORIZONSIZE")->read());
	if (VERBOSE>2) LOG("horizon: size = %d\n", size);
	return size;
}

void horizon_ramdisk_device::nvram_default()
{
	memset(m_nvram, 0, get_size());
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void horizon_ramdisk_device::nvram_read(emu_file &file)
{
	int size = get_size();
	int readsize = file.read(m_nvram, size);
	// If we increased the size, fill the remaining parts with 0
	if (readsize < size)
	{
		memset(m_nvram + readsize, 0, size-readsize);
	}
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void horizon_ramdisk_device::nvram_write(emu_file &file)
{
	int size = get_size();
	file.write(m_nvram, size);
}

READ8Z_MEMBER(horizon_ramdisk_device::readz)
{
	// 32K expansion
	if (m_32k_installed)
	{
		switch((offset & 0xe000)>>13)
		{
		case 1:  // 2000-3fff
			*value = m_ram[offset & 0x1fff];
			return;
		case 5: // a000-bfff
			*value = m_ram[(offset & 0x1fff) | 0x2000];
			return;
		case 6: // c000-dfff
			*value = m_ram[(offset & 0x1fff) | 0x4000];
			return;
		case 7: // e000-ffff
			*value = m_ram[(offset & 0x1fff) | 0x6000];
			return;
		default:
			break;
		}
	}

	if (m_killswitch) return;

	// I think RAMBO mode does not need the card to be selected
	if (!m_selected && !m_rambo_mode) return;

	if (!m_rambo_mode)
	{
		if ((offset & m_select_mask) == m_select_value)
		{
			if ((offset & 0x1800) == 0x1800)
			{
				// NVRAM page of size 2 KiB
				*value = m_nvram[(m_page << 11)|(offset & 0x07ff)];
				if (VERBOSE>5) LOG("horizon: offset=%04x, page=%04x -> %02x\n", offset&0xffff,  m_page, *value);
			}
			else
			{
				// ROS
				*value = m_ros[offset & 0x1fff];
				if (VERBOSE>5) LOG("horizon: offset=%04x -> %02x\n", offset&0xffff,  *value);
			}
		}
	}
	else
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			*value = m_ros[offset & 0x1fff];
			if (VERBOSE>5) LOG("horizon: offset=%04x (Rambo) -> %02x\n", offset&0xffff,  *value);
		}
		if ((offset & m_select_mask)==m_select6_value)
		{
			// In RAMBO mode the page numbers are multiples of 4
			// (encompassing 4 Horizon pages)
			// We clear away the rightmost two bits
			*value = m_nvram[((m_page&0xfffc)<<11) | (offset & 0x1fff)];
			if (VERBOSE>5) LOG("horizon: offset=%04x, page=%04x (Rambo) -> %02x\n", offset&0xffff,  m_page, *value);
		}
	}
}

WRITE8_MEMBER(horizon_ramdisk_device::write)
{
	// 32K expansion
	if (m_32k_installed)
	{
		switch((offset & 0xe000)>>13)
		{
		case 1:  // 2000-3fff
			m_ram[offset & 0x1fff] = data;
			return;
		case 5: // a000-bfff
			m_ram[(offset & 0x1fff) | 0x2000] = data;
			return;
		case 6: // c000-dfff
			m_ram[(offset & 0x1fff) | 0x4000] = data;
			return;
		case 7: // e000-ffff
			m_ram[(offset & 0x1fff) | 0x6000] = data;
			return;
		default:
			break;
		}
	}

	if (m_killswitch) return;

	// I think RAMBO mode does not need the card to be selected
	if (!m_selected && !m_rambo_mode) return;

	if (!m_rambo_mode)
	{
		if ((offset & m_select_mask) == m_select_value)
		{
			if ((offset & 0x1800) == 0x1800)
			{
				// NVRAM page of size 2 KiB
				m_nvram[(m_page << 11)|(offset & 0x07ff)] = data;
				if (VERBOSE>5) LOG("horizon: offset=%04x, page=%04x <- %02x\n", offset&0xffff,  m_page, data);
			}
			else
			{
				// ROS
				m_ros[offset & 0x1fff] = data;
				if (VERBOSE>5) LOG("horizon: offset=%04x <- %02x\n", offset&0xffff,  data);
			}
		}
	}
	else
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			m_ros[offset & 0x1fff] = data;
			if (VERBOSE>5) LOG("horizon: offset=%04x (Rambo) <- %02x\n", offset&0xffff,  data);
		}
		if ((offset & m_select_mask)==m_select6_value)
		{
			// In RAMBO mode the page numbers are multiples of 4
			// (encompassing 4 Horizon pages)
			// We clear away the rightmost two bits
			m_nvram[((m_page&0xfffc)<<11) | (offset & 0x1fff)] = data;
			if (VERBOSE>5) LOG("horizon: offset=%04x, page=%04x (Rambo) <- %02x\n", offset&0xffff,  m_page, data);
		}
	}
}

READ8Z_MEMBER(horizon_ramdisk_device::crureadz)
{
	// There is no CRU read operation for the Horizon.
	return;
}

void horizon_ramdisk_device::setbit(int& page, int pattern, bool set)
{
	if (set)
	{
		page |= pattern;
	}
	else
	{
		page &= ~pattern;
	}
}

WRITE8_MEMBER(horizon_ramdisk_device::cruwrite)
{
	int size = ioport("HORIZONSIZE")->read();
	int split_bit = size + 10;
	int splitpagebit = 0x0200 << size;

	if (((offset & 0xff00)==m_cru_horizon)||((offset & 0xff00)==m_cru_phoenix))
	{
		int bit = (offset >> 1) & 0x0f;
		if (VERBOSE>5) LOG("horizon: CRU write bit %d <- %d\n", bit, data);
		switch (bit)
		{
		case 0:
			m_selected = (data!=0);
			if (VERBOSE>4) LOG("horizon: Activate ROS = %d\n", m_selected);
			break;
		case 1:
			// Swap the lines so that the access with RAMBO is consistent
			if (!m_rambo_mode) setbit(m_page, 0x0002, data!=0);
			break;
		case 2:
			// Swap the lines so that the access with RAMBO is consistent
			if (!m_rambo_mode) setbit(m_page, 0x0001, data!=0);
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			setbit(m_page, 0x0001 << (bit-1), data!=0);
			break;
		case 14:
			break;
		case 15:
			if (m_use_rambo)
			{
				m_rambo_mode = (data != 0);
				if (VERBOSE>4) LOG("horizon: RAMBO = %d\n", m_rambo_mode);
			}
			break;

		default:   // bits 10-13
			if (bit != split_bit || !m_split_mode)
			{
				if (bit <= split_bit) setbit(m_page, 0x0200<<(bit-10), data!=0);
			}
			break;
		}

		if (m_split_mode)
		{
			if (m_timode)
			{
				// In TI mode, switch between both RAMDisks using the CRU address
				setbit(m_page, splitpagebit, ((offset & 0xff00)==m_cru_phoenix));
			}
			else
			{
				// In Geneve mode, switch between both RAMdisks by
				// using the bit number of the last CRU access
				setbit(m_page, splitpagebit, (bit>7));
			}
		}
	}
}

void horizon_ramdisk_device::device_start(void)
{
	m_nvram = memregion(NVRAMREGION)->base();
	m_ram = memregion(RAMREGION)->base();
	m_ros = m_nvram + MAXRAM_SIZE - 8192;
	m_cru_horizon = 0;
	m_cru_phoenix = 0;
}

void horizon_ramdisk_device::device_reset(void)
{
	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
		m_select6_value = 0x176000;
		m_select_all = 0x170000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
		m_select6_value = 0x76000;
		m_select_all = 0x70000;
	}

	m_ros = m_nvram + get_size()-8192;
	m_cru_horizon = ioport("CRUHOR")->read();
	m_cru_phoenix = ioport("CRUPHOE")->read();

	m_32k_installed = (ioport("HORIZON32")->read()!=0);

	m_split_mode = (ioport("HORIZONDUAL")->read()!=0);
	m_timode = (ioport("HORIZONDUAL")->read()==1);

	m_rambo_mode = false;
	m_killswitch = (ioport("HORIZONACT")->read()!=0);

	m_use_rambo = (ioport("RAMBO")->read()!=0);

	m_page = 0;
	m_selected = false;
}

INPUT_CHANGED_MEMBER( horizon_ramdisk_device::ks_changed )
{
	if (VERBOSE>5) LOG("horizon: killswitch changed %d\n", newval);
	m_killswitch = (newval!=0);
}

/*
    Input ports for the Horizon
*/
INPUT_PORTS_START( horizon )
	PORT_START( "CRUHOR" )
	PORT_DIPNAME( 0x1f00, 0x1200, "Horizon CRU base" )
		PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x1000, "1000" )
		PORT_DIPSETTING(    0x1200, "1200" )
		PORT_DIPSETTING(    0x1400, "1400" )
		PORT_DIPSETTING(    0x1500, "1500" )
		PORT_DIPSETTING(    0x1600, "1600" )
		PORT_DIPSETTING(    0x1700, "1700" )

	PORT_START( "CRUPHOE" )
	PORT_DIPNAME( 0x1f00, 0x0000, "Phoenix CRU base" )
		PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x1400, "1400" )
		PORT_DIPSETTING(    0x1600, "1600" )

	PORT_START( "HORIZONDUAL" )
	PORT_DIPNAME( 0x03, 0x00, "Horizon ramdisk split" )
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, "TI mode" )
		PORT_DIPSETTING(    0x02, "Geneve mode" )

	PORT_START( "HORIZONACT" )
	PORT_DIPNAME( 0x01, 0x00, "Horizon killswitch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, horizon_ramdisk_device, ks_changed, 1)
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_START( "HORIZON32" )
	PORT_CONFNAME( 0x01, 0x00, "Horizon 32 KiB upgrade" )
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( 0x01, DEF_STR( On ))

	PORT_START( "RAMBO" )
	PORT_CONFNAME( 0x01, 0x01, "Horizon RAMBO" )
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( 0x01, DEF_STR( On ))

	PORT_START( "HORIZONSIZE" )
	PORT_CONFNAME( 0x03, 0x00, "Horizon size" )
		PORT_CONFSETTING( 0x00, "2 MiB")
		PORT_CONFSETTING( 0x01, "4 MiB")
		PORT_CONFSETTING( 0x03, "16 MiB")

INPUT_PORTS_END

ROM_START( horizon )
	ROM_REGION(MAXRAM_SIZE, NVRAMREGION, 0)
	ROM_FILL(0x0000, MAXRAM_SIZE, 0x00)
	ROM_REGION(0x8000, RAMREGION, 0)
	ROM_FILL(0x0000, 0x8000, 0x00)
ROM_END

const rom_entry *horizon_ramdisk_device::device_rom_region() const
{
	return ROM_NAME( horizon );
}

ioport_constructor horizon_ramdisk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(horizon);
}

const device_type TI99_HORIZON = &device_creator<horizon_ramdisk_device>;
