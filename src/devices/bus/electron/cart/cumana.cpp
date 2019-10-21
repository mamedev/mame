// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana Floppy Disk System

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_FDsystem.html

    TODO:
    - add floppy format CDFS, and find original utilities disc
    - confirm whether DRQ and INTRQ are connected
    - add spare ROM slot

**********************************************************************/


#include "emu.h"
#include "cumana.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_CUMANA, electron_cumana_device, "electron_cumana", "Cumana Floppy Disk System")


//-------------------------------------------------
//  MACHINE_DRIVER( cumana )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(electron_cumana_device::floppy_formats)
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_ACORN_ADFS_OLD_FORMAT
FLOPPY_FORMATS_END0

void cumana_floppies(device_slot_interface &device)
{
	device.option_add("35dd",  FLOPPY_35_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_cumana_device::device_add_mconfig(machine_config &config)
{
	/* fdc */
	FD1793(config, m_fdc, DERIVED_CLOCK(1, 16)); // TODO: Not known whether DRQ and INTRQ are connected
	FLOPPY_CONNECTOR(config, m_floppy0, cumana_floppies, "525qd", electron_cumana_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, cumana_floppies, nullptr, electron_cumana_device::floppy_formats).enable_sound(true);

	/* rtc */
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::irq_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_cumana_device - constructor
//-------------------------------------------------

electron_cumana_device::electron_cumana_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_CUMANA, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_floppy0(*this, "fdc:0")
	, m_floppy1(*this, "fdc:1")
	, m_rtc(*this, "rtc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_cumana_device::device_start()
{
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_cumana_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
			data = m_fdc->read(offset & 0x03);
			break;
		case 0x98:
		case 0x9c:
			data = m_rtc->read(BIT(offset, 2));
			break;
		}
	}
	else if (oe)
	{
		switch (romqa)
		{
		case 0:
			if (offset < 0x3800)
			{
				data = m_rom[offset & 0x3fff];
			}
			else
			{
				data = m_nvram[offset & 0x07ff];
			}
			break;
		case 1:
			// TODO: rom slot not implemented
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_cumana_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
			m_fdc->write(offset & 0x03, data);
			break;
		case 0x94:
			wd1793_control_w(data);
			break;
		case 0x98:
		case 0x9c:
			m_rtc->write(BIT(offset, 2), data);
			break;
		}
	}
	else if (oe)
	{
		if (romqa == 0 && offset >= 0x3800)
		{
			m_nvram[offset & 0x07ff] = data;
		}
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void electron_cumana_device::wd1793_control_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// bit 1, 2: drive select
	if (BIT(data, 1)) floppy = m_floppy0->get_device();
	if (BIT(data, 2)) floppy = m_floppy1->get_device();
	m_fdc->set_floppy(floppy);

	// bit 0: side select
	if (floppy)
		floppy->ss_w(BIT(data, 0));

	// bit 3: density
	m_fdc->dden_w(BIT(data, 3));

	// bit 4: motor on
	if (floppy)
		floppy->mon_w(!BIT(data, 4));

	// bit 5: head load
}
