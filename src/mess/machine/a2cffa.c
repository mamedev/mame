/*********************************************************************

    a2cffa.c

    Implementation of Rich Dreher's IDE/CompactFlash card
    for the Apple II.

    http://www.dreher.net/

*********************************************************************/

#include "a2cffa.h"
#include "includes/apple2.h"
#include "machine/idectrl.h"
#include "imagedev/harddriv.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_A2CFFA	1

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_CFFA2 = &device_creator<a2bus_cffa2_device>;
const device_type A2BUS_CFFA2_6502 = &device_creator<a2bus_cffa2_6502_device>;

#define CFFA2_ROM_REGION  "cffa2_rom"
#define CFFA2_IDE_TAG     "cffa2_ide"

static const ide_config ide_intf = 
{
	NULL, 
	NULL, 
	0
};

MACHINE_CONFIG_FRAGMENT( cffa2 )
    MCFG_IDE_CONTROLLER_ADD(CFFA2_IDE_TAG, ide_intf, ide_image_devices, "hdd", "hdd", false)
MACHINE_CONFIG_END

ROM_START( cffa2 )
	ROM_REGION(0x1000, CFFA2_ROM_REGION, 0)
    ROM_LOAD( "cffa20eec02.bin", 0x000000, 0x001000, CRC(fb3726f8) SHA1(080ff88f19de22328e162954ee2b51ee65f9d5cd) )
ROM_END

ROM_START( cffa2_6502 )
	ROM_REGION(0x1000, CFFA2_ROM_REGION, 0)
    ROM_LOAD( "cffa20ee02.bin", 0x000000, 0x001000, CRC(3ecafce5) SHA1(d600692ed9626668233a22a48236af639410cb7b) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_cffa2000_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cffa2 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_cffa2000_device::device_rom_region() const
{
	return ROM_NAME( cffa2 );
}

const rom_entry *a2bus_cffa2_6502_device::device_rom_region() const
{
	return ROM_NAME( cffa2_6502 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_cffa2000_device::a2bus_cffa2000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, type, name, tag, owner, clock),
    device_a2bus_card_interface(mconfig, *this),
    m_ide(*this, CFFA2_IDE_TAG)
{
}

a2bus_cffa2_device::a2bus_cffa2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    a2bus_cffa2000_device(mconfig, A2BUS_CFFA2, "CFFA2000 Compact Flash (65C02 firmware, www.dreher.net)", tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
	m_shortname = "a2cffa2";
}

a2bus_cffa2_6502_device::a2bus_cffa2_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    a2bus_cffa2000_device(mconfig, A2BUS_CFFA2, "CFFA2000 Compact Flash (6502 firmware, www.dreher.net)", tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
	m_shortname = "a2cffa02";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_cffa2000_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	astring tempstring;
	m_rom = device().machine().root_device().memregion(this->subtag(tempstring, CFFA2_ROM_REGION))->base();

	// patch default setting so slave device is enabled
	m_rom[0x801] = 4;

	save_item(NAME(m_lastdata));
	save_item(NAME(m_writeprotect));
	save_item(NAME(m_eeprom));
}

void a2bus_cffa2000_device::device_reset()
{
	m_writeprotect = true;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_cffa2000_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
			return m_lastdata>>8;

		case 3:
			m_writeprotect = false;
			break;

		case 4:
			m_writeprotect = true;
			break;

		case 8:
			m_lastdata = ide_controller_r(m_ide, 0x1f0+offset-8, 2);
			return m_lastdata & 0xff;

		case 9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			return ide_controller_r(m_ide, 0x1f0+offset-8, 1);
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_cffa2000_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0:
			m_lastdata &= 0x00ff;
			m_lastdata |= data<<8;
			break;

		case 3:
			m_writeprotect = false;
			break;

		case 4:
			m_writeprotect = true;
			break;

		case 8:
			m_lastdata &= 0xff00;
			m_lastdata |= data;
			ide_controller_w(m_ide, 0x1f0+offset-8, 2, m_lastdata);
			break;

		case 9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			ide_controller_w(m_ide, 0x1f0+offset-8, 1, data);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_cffa2000_device::read_cnxx(address_space &space, UINT8 offset)
{
    int slotimg = m_slot * 0x100;

    // ROM contains a CnXX image for each of slots 1-7
    return m_eeprom[offset+slotimg];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_cffa2000_device::read_c800(address_space &space, UINT16 offset)
{
    return m_eeprom[offset+0x800];
}

void a2bus_cffa2000_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	if (!m_writeprotect)
	{
//      printf("Write %02x to EEPROM at %x (PC=%x)\n", data, offset, cpu_get_pc(&space.device()));
		m_eeprom[offset + 0x800] = data;
	}
}

// NVRAM device virtual overrides to provide saving/loading of settings changes
void a2bus_cffa2_device::nvram_default()
{
	memcpy(m_eeprom, m_rom, 0x1000);
}

void a2bus_cffa2_device::nvram_read(emu_file &file)
{
	file.read(m_eeprom, 0x1000);
}

void a2bus_cffa2_device::nvram_write(emu_file &file)
{
	file.write(m_eeprom, 0x1000);
}

void a2bus_cffa2_6502_device::nvram_default()
{
	memcpy(m_eeprom, m_rom, 0x1000);
}

void a2bus_cffa2_6502_device::nvram_read(emu_file &file)
{
	file.read(m_eeprom, 0x1000);
}

void a2bus_cffa2_6502_device::nvram_write(emu_file &file)
{
	file.write(m_eeprom, 0x1000);
}

