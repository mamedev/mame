// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    RetroClinic DataCentre - USB, IDE and RAMdisc for the BBC Model B, B+, and Master 128

    http://www.retroclinic.com/acorn/datacentre/datacentre.htm

    TODO:
    - implement VNC2 USB controller

**********************************************************************/


#include "emu.h"
#include "datacentre.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_DATACENTRE, bbc_datacentre_device, "bbc_datacentre", "RetroClinic DataCentre");


//-------------------------------------------------
//  INPUT_PORTS( datacentre )
//-------------------------------------------------

INPUT_PORTS_START(datacentre)
	PORT_START("LINKS")
	PORT_CONFNAME(0x01, 0x00, "JP1: IDE Power")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
	PORT_CONFNAME(0x02, 0x00, "JP2: IDE Interrupt Enable")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x02, DEF_STR(Yes))
	PORT_CONFNAME(0x04, 0x00, "JP3: NVRAM Write Protect")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x04, DEF_STR(Yes))

	PORT_START("NVREST")
	PORT_CONFNAME(0x01, 0x00, "Import NVRAM Restore Utility") PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_datacentre_device, import_nvrest, 0)
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_datacentre_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(datacentre);
}

//-------------------------------------------------
//  ROM( datacentre )
//-------------------------------------------------

ROM_START(datacentre)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("ramfs104.rom", 0x0000, 0x4000, CRC(2e52e331) SHA1(d3c501fb29d35c2d144b164fa3ceb5a3b65d2850))

	/* initialised NVRAM */
	ROM_REGION(0x10000, "nvram", 0)
	ROM_LOAD("eeprom.bin", 0x0000, 0x10000, CRC(eb22a606) SHA1(7d791234216c63a32b7181ff09c2f5fbdfc7f1a0))

	/* NVRAM restore utility */
	ROM_REGION(0x32000, "nvrest", 0)
	ROM_LOAD("nvrest_1.01.ssd", 0x00000, 0x32000, CRC(54b49e63) SHA1(524826112e436728ba54e73f3f6b25c7b43abd69))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_datacentre_device::device_add_mconfig(machine_config &config)
{
	/* compact flash hard drive */
	ATA_INTERFACE(config, m_ide).options(ata_devices, "hdd", "hdd", false);
	m_ide->irq_handler().set(FUNC(bbc_datacentre_device::irq_w));

	I2C_24C512(config, m_nvram); // 24LC512

	/* import floppy images - delayed to allow RAMFS to initialise before import */
	QUICKLOAD(config, "import0", "ssd,dsd,img", attotime::from_seconds(1)).set_load_callback(FUNC(bbc_datacentre_device::quickload_cb<0>));
	QUICKLOAD(config, "import1", "ssd,dsd,img", attotime::from_seconds(1)).set_load_callback(FUNC(bbc_datacentre_device::quickload_cb<1>));
	QUICKLOAD(config, "import2", "ssd,dsd,img", attotime::from_seconds(1)).set_load_callback(FUNC(bbc_datacentre_device::quickload_cb<2>));
	QUICKLOAD(config, "import3", "ssd,dsd,img", attotime::from_seconds(1)).set_load_callback(FUNC(bbc_datacentre_device::quickload_cb<3>));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_datacentre_device::device_rom_region() const
{
	return ROM_NAME(datacentre);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_ide_device - constructor
//-------------------------------------------------

bbc_datacentre_device::bbc_datacentre_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, BBC_DATACENTRE, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_links(*this, "LINKS")
	, m_ide(*this, "ide")
	, m_nvram(*this, "nvram")
	, m_nvrest(*this, "nvrest")
	, m_page_ram(0)
	, m_ide_data(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_datacentre_device::device_start()
{
	/* define 1mb ram */
	m_ram = std::make_unique<uint8_t[]>(0x100000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x100000);
	save_item(NAME(m_page_ram));
	save_item(NAME(m_ide_data));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_datacentre_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xf8)
	{
	case 0x40:
		if (offset & 0x07)
		{
			data = m_ide->cs0_r(offset & 0x07, 0xff);
		}
		else
		{
			m_ide_data = m_ide->cs0_r(offset & 0x07);
			data = m_ide_data & 0xff;
		}
		break;
	case 0x48:
		if (offset & 0x04)
		{
			data = m_ide->cs1_r(offset & 0x07, 0xff);
		}
		else
		{
			data = m_ide_data >> 8;
		}
		break;

	case 0xf8:
		switch (offset & 0x07)
		{
		case 0x00:
			/* VNC Data Port */
			logerror("fred_r: VNC Data Port not implemented\n");
			break;
		case 0x01:
			/* VNC Control Port */
			logerror("fred_r: VNC Control Port not implemented\n");
			break;

		case 0x02:
			/* NVRAM Data Port */
			data = m_nvram->read_sda();
			break;

		case 0x06:
			/* PAGE RAM MSB - This register does not have feedback */
			break;
		case 0x07:
			/* PAGE RAM LSB - LSB 8 bits of page ram register */
			data = m_page_ram & 0xff;
			break;
		}
		break;
	}

	return data;
}

void bbc_datacentre_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf8)
	{
	case 0x40:
		if (offset & 0x07)
		{
			m_ide->cs0_w(offset & 0x07, data, 0xff);
		}
		else
		{
			m_ide_data = (m_ide_data & 0xff00) | data;
			m_ide->cs0_w(offset & 0x07, m_ide_data);
		}
		break;
	case 0x48:
		if (offset & 0x04)
		{
			m_ide->cs1_w(offset & 0x07, data, 0xff);
		}
		else
		{
			m_ide_data = data << 8;
		}
		break;

	case 0xf8:
		switch (offset & 0x07)
		{
		case 0x00:
			/* VNC Data Port */
			logerror("fred_w: VNC Data Port not implemented\n");
			break;
		case 0x01:
			/* VNC Control Port */
			logerror("fred_w: VNC Control Port not implemented\n");
			break;

		case 0x02:
			/* NVRAM Data Port */
			m_nvram->write_sda(BIT(data, 0));
			break;
		case 0x03:
			/* NVRAM Data Direction Register */
			break;
		case 0x04:
			/* NVRAM Clock */
			m_nvram->write_scl(BIT(data, 0));
			break;
		case 0x05:
			/* NVRAM Write Protect (Issue 1 boards only) */
			logerror("fred_w: NVRAM Write Protect (Issue 1 boards only) not implemented\n");
			break;

		case 0x06:
			/* PAGE RAM MSB - MSB 4 bits of page ram register */
			m_page_ram = (m_page_ram & 0x00ff) | (data << 8);
			break;
		case 0x07:
			/* PAGE RAM LSB - LSB 8 bits of page ram register */
			m_page_ram = (m_page_ram & 0xff00) | data;
			break;
		}
		break;
	}
}

uint8_t bbc_datacentre_device::jim_r(offs_t offset)
{
	return m_ram[((m_page_ram & 0x0fff) << 8) | offset];
}

void bbc_datacentre_device::jim_w(offs_t offset, uint8_t data)
{
	m_ram[((m_page_ram & 0x0fff) << 8) | offset] = data;
}

WRITE_LINE_MEMBER(bbc_datacentre_device::irq_w)
{
	if (BIT(m_links->read(), 1))
	{
		m_slot->irq_w(state);
	}
}


INPUT_CHANGED_MEMBER(bbc_datacentre_device::import_nvrest)
{
	if (newval)
	{
		/* import NVRAM restore utility to RAM drive 0 */
		memcpy(m_ram.get() + 0x1000, m_nvrest->base(), 0x32000);
	}
	else
	{
		/* clear RAM drive 0 */
		memset(m_ram.get() + 0x1000, 0, 0x200);
		m_ram[0x1106] = 0x03; // sector count MSB
		m_ram[0x1107] = 0x20; // sector count LSB
	}
}


//-------------------------------------------------
//  QUICKLOAD_LOAD_MEMBER( quickload_cb )
//-------------------------------------------------

template<int Drive>
QUICKLOAD_LOAD_MEMBER(bbc_datacentre_device::quickload_cb)
{
	/* simulate *IMPORT from USB to RAMFS */
	if (image.is_filetype("ssd") || image.is_filetype("img") || image.is_filetype("dsd"))
	{
		uint32_t ram_addr = (Drive * 0x40000) | 0x1000;
		offs_t offset = 0;

		/* import tracks */
		for (int i = 0; i < 80; i++)
		{
			image.fread(m_ram.get() + ram_addr + offset, 0xa00);
			if (image.is_filetype("dsd"))
			{
				image.fread(m_ram.get() + ((ram_addr + offset) ^ 0x80000), 0xa00);
			}
			offset += 0xa00;
		}
	}
	else
	{
		image.seterror(image_error::INVALIDIMAGE, "Invalid filetype, must be SSD, DSD, or IMG");
		return image_init_result::FAIL;
	}

	return image_init_result::PASS;
}
