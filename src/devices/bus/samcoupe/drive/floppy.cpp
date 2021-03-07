// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Internal floppy drive for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "floppy.h"
#include "formats/coupedsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_FLOPPY, sam_floppy_device, "sam_floppy", "SAM Coupe Internal Floppy")

void sam_floppy_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_MGT_FORMAT);
}

static void samcoupe_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_floppy_device::device_add_mconfig(machine_config &config)
{
	WD1772(config, m_fdc, 24_MHz_XTAL / 3);
	FLOPPY_CONNECTOR(config, "fdc:0", samcoupe_floppies, "35dd", sam_floppy_device::floppy_formats);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_floppy_device - constructor
//-------------------------------------------------

sam_floppy_device::sam_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_FLOPPY, tag, owner, clock),
	device_samcoupe_drive_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_drive(*this, "fdc:0")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_floppy_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t sam_floppy_device::read(offs_t offset)
{
	floppy_image_device *floppy = m_drive->get_device();

	if (floppy)
		floppy->ss_w(BIT(offset, 2));

	m_fdc->set_floppy(floppy);

	return m_fdc->read(offset & 0x03);
}

void sam_floppy_device::write(offs_t offset, uint8_t data)
{
	floppy_image_device *floppy = m_drive->get_device();

	if (floppy)
		floppy->ss_w(BIT(offset, 2));

	m_fdc->set_floppy(floppy);

	m_fdc->write(offset & 0x03, data);
}
