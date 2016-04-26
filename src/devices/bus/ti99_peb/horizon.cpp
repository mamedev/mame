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

*****************************************************************************/

#include "horizon.h"

#define RAMREGION "ram32k"
#define ROSREGION "ros8k"
#define NVRAMREGION "nvram"

#define MAXSIZE 16777216
#define ROSSIZE 8192

#define TRACE_CONFIG 0
#define TRACE_READ 0
#define TRACE_WRITE 0
#define TRACE_CRU 0

horizon_ramdisk_device::horizon_ramdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_HORIZON, "Horizon 4000 Ramdisk", tag, owner, clock,"ti99_horizon",__FILE__),
	device_nvram_interface(mconfig, *this),
	m_ram(*this, RAMREGION),
	m_nvram(*this, NVRAMREGION),
	m_ros(*this, ROSREGION),
	m_select6_value(0),
	m_select_all(0),
	m_page(0),
	m_cru_horizon(0),
	m_cru_phoenix(0),
	m_timode(false),
	m_32k_installed(false),
	m_split_mode(false),
	m_rambo_mode(false),
	m_hideswitch(false),
	m_use_rambo(false)
{
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void horizon_ramdisk_device::nvram_default()
{
	int size = 2097152*(1 << ioport("HORIZONSIZE")->read());
	memset(m_nvram->pointer(), 0,  size);
	memset(m_ros->pointer(), 0, ROSSIZE);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void horizon_ramdisk_device::nvram_read(emu_file &file)
{
	int size = 2097152*(1 << ioport("HORIZONSIZE")->read());

	// NVRAM plus ROS
	UINT8* buffer = global_alloc_array_clear<UINT8>(MAXSIZE + ROSSIZE);

	memset(m_nvram->pointer(), 0,  size);
	memset(m_ros->pointer(), 0, ROSSIZE);

	// We assume the last 8K is ROS
	int filesize = file.read(buffer, MAXSIZE+ROSSIZE);
	int nvramsize = filesize - ROSSIZE;

	// If there is a reasonable size
	if (nvramsize >= 0)
	{
		// Copy from buffer to NVRAM and ROS
		memcpy(m_nvram->pointer(), buffer, nvramsize);
		memcpy(m_ros->pointer(), buffer + nvramsize, ROSSIZE);
	}

	global_free_array(buffer);
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void horizon_ramdisk_device::nvram_write(emu_file &file)
{
	int nvramsize = 2097152*(1 << ioport("HORIZONSIZE")->read());

	UINT8* buffer = global_alloc_array_clear<UINT8>(nvramsize + ROSSIZE);
	memcpy(buffer, m_nvram->pointer(), nvramsize);
	memcpy(buffer + nvramsize, m_ros->pointer(), ROSSIZE);

	file.write(buffer, nvramsize + ROSSIZE);
}

READ8Z_MEMBER(horizon_ramdisk_device::readz)
{
	// 32K expansion
	// According to the manual, "this memory is not affected by the HIDE switch"
	if (m_32k_installed)
	{
		switch((offset & 0xe000)>>13)
		{
		case 1:  // 2000-3fff
			*value = m_ram->pointer()[offset & 0x1fff];
			return;
		case 5: // a000-bfff
			*value = m_ram->pointer()[(offset & 0x1fff) | 0x2000];
			return;
		case 6: // c000-dfff
			*value = m_ram->pointer()[(offset & 0x1fff) | 0x4000];
			return;
		case 7: // e000-ffff
			*value = m_ram->pointer()[(offset & 0x1fff) | 0x6000];
			return;
		default:
			break;
		}
	}

	if (m_hideswitch) return;

	// I think RAMBO mode does not need the card to be selected
	if (!m_selected && !m_rambo_mode) return;

	if (!m_rambo_mode)
	{
		if ((offset & m_select_mask) == m_select_value)
		{
			if ((offset & 0x1800) == 0x1800)
			{
				// NVRAM page of size 2 KiB
				*value = m_nvram->pointer()[(m_page << 11)|(offset & 0x07ff)];
				if (TRACE_READ) logerror("offset=%04x, page=%04x -> %02x\n", offset&0xffff,  m_page, *value);
			}
			else
			{
				// ROS
				*value = m_ros->pointer()[offset & 0x1fff];
				if (TRACE_READ) logerror("offset=%04x (ROS) -> %02x\n", offset&0xffff,  *value);
			}
		}
	}
	else
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			*value = m_ros->pointer()[offset & 0x1fff];
			if (TRACE_READ) logerror("offset=%04x (Rambo) -> %02x\n", offset&0xffff,  *value);
		}
		if ((offset & m_select_mask)==m_select6_value)
		{
			// In RAMBO mode the page numbers are multiples of 4
			// (encompassing 4 Horizon pages)
			// We clear away the rightmost two bits
			*value = m_nvram->pointer()[((m_page&0xfffc)<<11) | (offset & 0x1fff)];
			if (TRACE_READ) logerror("offset=%04x, page=%04x (Rambo) -> %02x\n", offset&0xffff,  m_page, *value);
		}
	}
}

WRITE8_MEMBER(horizon_ramdisk_device::write)
{
	// 32K expansion
	// According to the manual, "this memory is not affected by the HIDE switch"
	if (m_32k_installed)
	{
		switch((offset & 0xe000)>>13)
		{
		case 1:  // 2000-3fff
			m_ram->pointer()[offset & 0x1fff] = data;
			return;
		case 5: // a000-bfff
			m_ram->pointer()[(offset & 0x1fff) | 0x2000] = data;
			return;
		case 6: // c000-dfff
			m_ram->pointer()[(offset & 0x1fff) | 0x4000] = data;
			return;
		case 7: // e000-ffff
			m_ram->pointer()[(offset & 0x1fff) | 0x6000] = data;
			return;
		default:
			break;
		}
	}

	if (m_hideswitch) return;

	// I think RAMBO mode does not need the card to be selected
	if (!m_selected && !m_rambo_mode) return;

	if (!m_rambo_mode)
	{
		if ((offset & m_select_mask) == m_select_value)
		{
			if ((offset & 0x1800) == 0x1800)
			{
				// NVRAM page of size 2 KiB
				m_nvram->pointer()[(m_page << 11)|(offset & 0x07ff)] = data;
				if (TRACE_WRITE) logerror("offset=%04x, page=%04x <- %02x\n", offset&0xffff,  m_page, data);
			}
			else
			{
				// ROS
				m_ros->pointer()[offset & 0x1fff] = data;
				if (TRACE_WRITE) logerror("offset=%04x (ROS) <- %02x\n", offset&0xffff,  data);
			}
		}
	}
	else
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			m_ros->pointer()[offset & 0x1fff] = data;
			if (TRACE_WRITE) logerror("offset=%04x (Rambo) <- %02x\n", offset&0xffff,  data);
		}
		if ((offset & m_select_mask)==m_select6_value)
		{
			// In RAMBO mode the page numbers are multiples of 4
			// (encompassing 4 Horizon pages)
			// We clear away the rightmost two bits
			m_nvram->pointer()[((m_page&0xfffc)<<11) | (offset & 0x1fff)] = data;
			if (TRACE_WRITE) logerror("offset=%04x, page=%04x (Rambo) <- %02x\n", offset&0xffff,  m_page, data);
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
		if (TRACE_CRU) logerror("CRU write bit %d <- %d\n", bit, data);
		switch (bit)
		{
		case 0:
			m_selected = (data!=0);
			if (TRACE_CRU) logerror("Activate ROS = %d\n", m_selected);
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
				if (TRACE_CRU) logerror("RAMBO = %d\n", m_rambo_mode);
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

	m_cru_horizon = ioport("CRUHOR")->read();
	m_cru_phoenix = ioport("CRUPHOE")->read();

	m_32k_installed = (ioport("HORIZON32")->read()!=0);

	m_split_mode = (ioport("HORIZONDUAL")->read()!=0);
	m_timode = (ioport("HORIZONDUAL")->read()==1);

	m_rambo_mode = false;
	m_hideswitch = (ioport("HORIZONACT")->read()!=0);

	m_use_rambo = (ioport("RAMBO")->read()!=0);

	m_page = 0;
	m_selected = false;
}

INPUT_CHANGED_MEMBER( horizon_ramdisk_device::hs_changed )
{
	if (TRACE_CONFIG) logerror("hideswitch changed %d\n", newval);
	m_hideswitch = (newval!=0);
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
	PORT_DIPNAME( 0x01, 0x00, "Horizon hideswitch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, horizon_ramdisk_device, hs_changed, nullptr)
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
		PORT_CONFSETTING( 0x02, "8 MiB")
		PORT_CONFSETTING( 0x03, "16 MiB")

INPUT_PORTS_END

MACHINE_CONFIG_FRAGMENT( horizon )
	MCFG_RAM_ADD(NVRAMREGION)
	MCFG_RAM_DEFAULT_SIZE("16M")

	MCFG_RAM_ADD(ROSREGION)
	MCFG_RAM_DEFAULT_SIZE("8k")

	MCFG_RAM_ADD(RAMREGION)
	MCFG_RAM_DEFAULT_SIZE("32k")
MACHINE_CONFIG_END

machine_config_constructor horizon_ramdisk_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( horizon );
}

ioport_constructor horizon_ramdisk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(horizon);
}

const device_type TI99_HORIZON = &device_creator<horizon_ramdisk_device>;
