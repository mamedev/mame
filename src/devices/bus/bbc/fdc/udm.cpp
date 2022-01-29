// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microware / United Disk Memories DDFS FDC

**********************************************************************/


#include "emu.h"
#include "udm.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_UDM, bbc_udm_device, "bbc_udm", "United Disk Memories DDFS FDC")


//-------------------------------------------------
//  FLOPPY_FORMATS( udm )
//-------------------------------------------------

void bbc_udm_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
	fr.add(FLOPPY_ACORN_DSD_FORMAT);
	fr.add(FLOPPY_FSD_FORMAT);
}

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}


//-------------------------------------------------
//  ROM( udm )
//-------------------------------------------------

ROM_START( udm )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_SYSTEM_BIOS(0, "ddfs310", "UDM DDFS 3.10")
	ROMX_LOAD("udm_ddfs310.rom", 0x0000, 0x4000, CRC(55851c2d) SHA1(c5b6557fa4dbfa651c8ecc5b1da93b615c3aa905), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ddfs200", "UDM DDFS 2.00")
	ROMX_LOAD("udm_ddfs200.rom", 0x0000, 0x4000, CRC(1b4708a2) SHA1(0f37bcc73a758657cfe58c19f0cc92be9107e767), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ddfs098", "UDM DDFS 0.98")
	ROMX_LOAD("udm_ddfs098.rom", 0x0000, 0x4000, CRC(2119f9ad) SHA1(bfb9404b34de3b489db73d886300f37081db2482), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "ddfs090", "Microware DDFS 0.90")
	ROMX_LOAD("microware_ddfs090.rom", 0x0000, 0x4000, CRC(700d50e5) SHA1(6834e46cb15354003d553e6c2bdb4ed76b47a465), ROM_BIOS(3))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_udm_device::device_add_mconfig(machine_config &config)
{
	WD2793(config, m_fdc, DERIVED_CLOCK(1, 8));
	m_fdc->drq_wr_callback().set(FUNC(bbc_udm_device::drq_w));
	m_fdc->intrq_wr_callback().set(FUNC(bbc_udm_device::intrq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_udm_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
}

const tiny_rom_entry *bbc_udm_device::device_rom_region() const
{
	return ROM_NAME( udm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_udm_device - constructor
//-------------------------------------------------

bbc_udm_device::bbc_udm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_UDM, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_fdc(*this, "wd2793")
	, m_floppy(*this, "wd2793:%u", 0)
	, m_fdc_ie(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_udm_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_udm_device::read(offs_t offset)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_fdc->read(offset & 0x03);
	}
	else
	{
		data = 0xfe;
	}
	return data;
}

void bbc_udm_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		m_fdc->write(offset & 0x03, data);
	}
	else
	{
		floppy_image_device *floppy = nullptr;

		// bit 0: drive select
		floppy = m_floppy[BIT(data, 0)]->get_device();
		m_fdc->set_floppy(floppy);

		// bit 1: side select
		if (floppy)
			floppy->ss_w(!BIT(data, 1));

		// bit 2: density
		m_fdc->dden_w(!BIT(data, 2));

		// bit 3: master reset
		m_fdc->mr_w(BIT(data, 3));

		// bit 4: interrupt enable
		m_fdc_ie = BIT(data, 4);
	}
}

WRITE_LINE_MEMBER(bbc_udm_device::intrq_w)
{
	m_slot->intrq_w((state && m_fdc_ie) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(bbc_udm_device::drq_w)
{
	m_slot->drq_w((state && m_fdc_ie) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(bbc_udm_device::motor_w)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
}
