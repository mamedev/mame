// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    United Disk Memories DDFS FDC

    Microware DDFS FDC

**********************************************************************/


#include "emu.h"
#include "microware.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MICROWARE, bbc_microware_device, "bbc_microware", "Microware DDFS FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( microware )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_microware_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_FSD_FORMAT
FLOPPY_FORMATS_END

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

ROM_START( microware )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("udm200")
	ROM_SYSTEM_BIOS(0, "ddfs090", "Microware DDFS 0.90")
	ROMX_LOAD("microware_ddfs090.rom", 0x0000, 0x4000, CRC(700d50e5) SHA1(6834e46cb15354003d553e6c2bdb4ed76b47a465), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "udm200", "UDM DDFS 2.00")
	ROMX_LOAD("udm_ddfs200.rom", 0x0000, 0x4000, CRC(1b4708a2) SHA1(0f37bcc73a758657cfe58c19f0cc92be9107e767), ROM_BIOS(1))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_microware_device::device_add_mconfig(machine_config &config)
{
	WD2793(config, m_fdc, DERIVED_CLOCK(1, 8)); // Replay advert suggests Type R8272 UDM DFS
	m_fdc->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_microware_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
}

const tiny_rom_entry *bbc_microware_device::device_rom_region() const
{
	return ROM_NAME( microware );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_microware_device - constructor
//-------------------------------------------------

bbc_microware_device::bbc_microware_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MICROWARE, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_fdc(*this, "wd2793")
	, m_floppy0(*this, "wd2793:0")
	, m_floppy1(*this, "wd2793:1")
	, m_drive_control(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_microware_device::device_start()
{
	save_item(NAME(m_drive_control));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_microware_device::read)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_fdc->read(offset & 0x03);
	}
	else
	{
		data = m_drive_control;
	}
	return data;
}

WRITE8_MEMBER(bbc_microware_device::write)
{
	if (offset & 0x04)
	{
		m_fdc->write(offset & 0x03, data);
	}
	else
	{
		floppy_image_device *floppy = nullptr;

		m_drive_control = data;
		logerror("fdc: Drive control %02x \n", data);
		// bit 0: drive select
		switch (BIT(data, 0))
		{
		case 0: floppy = m_floppy0->get_device(); break;
		case 1: floppy = m_floppy1->get_device(); break;
		}
		m_fdc->set_floppy(floppy);

		// bit 1: side select
		if (floppy)
			floppy->ss_w(BIT(data, 1));

		// bit 2: density
		m_fdc->dden_w(BIT(data, 2));

		// bit 3: always set ???

		// bit 4: ??? interrupt
	}
}

WRITE_LINE_MEMBER(bbc_microware_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
}
