/*********************************************************************
 
	a2vulcan.c

	Applied Engineering Vulcan IDE controller

	Recognized drives by IDE features parameters:
	(# of cylinders is never checked, just heads, sectors, and the vendor specific at 0x0A)

	 H  S    Vendor specific #5
	 8, 33 + 0x69
	 2, 33 + 0x69
	 4, 26 + 0x69
	 5, 29 + (any)
	 7, 29 + 0x44
	 9, 29 + (any)
	 9, 36 + 0x44
	 9, 36 + 0xff
	 7, 34 + (any)
	 4, 17 + 0x55
	 4, 26 + 0x55
	 5, 17 + 0x55
	 6, 26 + 0x55
	 2, 28 + 0x36
	 4, 28 + 0x36
	 4, 28 + 0x67
	 4, 27 + 0x43
	 5, 17 + 0x26
	15, 32 + 0x43
	16, 38 + 0x94
	10, 17 + (any)

*********************************************************************/

#include "a2vulcan.h"
#include "includes/apple2.h"
#include "machine/idectrl.h"
#include "imagedev/harddriv.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_VULCAN = &device_creator<a2bus_vulcan_device>;

#define VULCAN_ROM_REGION  "vulcan_rom"
#define VULCAN_IDE_TAG     "vulcan_ide"

static MACHINE_CONFIG_FRAGMENT( vulcan )
	MCFG_IDE_CONTROLLER_ADD(VULCAN_IDE_TAG, ide_image_devices, "hdd", "hdd", false)
MACHINE_CONFIG_END

ROM_START( vulcan )
	ROM_REGION(0x4000, VULCAN_ROM_REGION, 0)
	ROM_LOAD( "ae vulcan rom v1.4.bin", 0x000000, 0x004000, CRC(798d5825) SHA1(1d668e856e33c6eeb10fe26975341afa8acb81f5) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_vulcanbase_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vulcan );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_vulcanbase_device::device_rom_region() const
{
	return ROM_NAME( vulcan );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_vulcanbase_device::a2bus_vulcanbase_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ide(*this, VULCAN_IDE_TAG)
{
}

a2bus_vulcan_device::a2bus_vulcan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	a2bus_vulcanbase_device(mconfig, A2BUS_VULCAN, "Applied Engineering Vulcan IDE controller", tag, owner, clock)
{
	m_shortname = "a2vulcan";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_vulcanbase_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	astring tempstring;
	m_rom = device().machine().root_device().memregion(this->subtag(tempstring, VULCAN_ROM_REGION))->base();

	// patch partition table check failure
//	m_rom[0x59e] = 0xea;
//	m_rom[0x59f] = 0xea;

	save_item(NAME(m_lastdata));
	save_item(NAME(m_ram));
	save_item(NAME(m_rombank));
	save_item(NAME(m_rambank));
}

void a2bus_vulcanbase_device::device_reset()
{
	m_rombank = m_rambank = 0;
	m_last_read_was_0 = false;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_vulcanbase_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
			m_lastdata = ide_controller_r(m_ide, 0x1f0+offset, 2);
//			printf("IDE: read %04x\n", m_lastdata);
			m_last_read_was_0 = true;
			return m_lastdata&0xff;

		case 1:
			if (m_last_read_was_0)
			{
				m_last_read_was_0 = false;
				return (m_lastdata>>8) & 0xff;
			}
			else
			{
				return ide_controller_r(m_ide, 0x1f0+offset, 1);
			}
			break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return ide_controller_r(m_ide, 0x1f0+offset, 1);

		default:
//			printf("Read @ C0n%x\n", offset);
			break;

	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_vulcanbase_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0:   
			m_lastdata = data;
			m_last_read_was_0 = true;
			break;
			  				 
		case 1:
			if (m_last_read_was_0)
			{
				m_last_read_was_0 = false;
				m_lastdata &= 0x00ff;
				m_lastdata |= (data << 8);
//				printf("IDE: write %04x\n", m_lastdata);
				ide_controller_w(m_ide, 0x1f0, 2, m_lastdata);
			}
			else
			{
				ide_controller_w(m_ide, 0x1f0+offset, 1, data);
			}
			break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
//			printf("%02x to IDE controller @ %x\n", data, offset);
			ide_controller_w(m_ide, 0x1f0+offset, 1, data);
			break;

		case 9:	// ROM bank
//			printf("%x (%x) to ROM bank\n", data, (data & 0xf) * 0x400);
			m_rombank = (data & 0xf) * 0x400;
			break;

		case 0xa: // RAM bank
//			printf("%x to RAM bank\n", data);
			m_rambank = (data & 7) * 0x400;
			break;

		default:
			printf("Write %02x @ C0n%x\n", data, offset);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_vulcanbase_device::read_cnxx(address_space &space, UINT8 offset)
{
	int slotimg = m_slot * 0x100;

	// ROM contains a CnXX image for each of slots 1-7 at 0x3400
	return m_rom[offset+slotimg+0x3400];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_vulcanbase_device::read_c800(address_space &space, UINT16 offset)
{
	offset &= 0x7ff;
	if (offset < 0x400)	// c800-cbff is banked RAM window, cc00-cfff is banked ROM window
	{
//		printf("read RAM @ %x (bank %x)\n", offset, m_rambank);
		return m_ram[offset + m_rambank];
	}

	return m_rom[(offset & 0x3ff)+m_rombank];
}

void a2bus_vulcanbase_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	offset &= 0x7ff;
	if (offset < 0x400)
	{
//		printf("%02x to RAM @ %x (bank %x)\n", data, offset, m_rambank);
		m_ram[offset + m_rambank] = data;
	}
}
