// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * NABU PC FDC Card
 *
 *******************************************************************/

#include "emu.h"
#include "fdc.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

#include "formats/nabupc_dsk.h"


//**************************************************************************
//  NABU PC FDC DEVICE
//**************************************************************************
namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* NABU PC FDC Device */

class fdc_device : public device_t, public bus::nabupc::device_option_expansion_interface
{
public:
	// construction/destruction
	fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void ds_w(uint8_t data);

	static void floppy_formats(format_registration &fr);

	required_device<wd2797_device>              m_wd2797;
	required_device_array<floppy_connector, 2>  m_floppies;
};


static void nabu_fdc_drives(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  fdc_device - constructor
//-------------------------------------------------
fdc_device::fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NABUPC_OPTION_FDC, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	m_wd2797(*this, "fdc"),
	m_floppies(*this, "fdc:%u", 0)
{
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------
void fdc_device::device_add_mconfig(machine_config &config)
{
	WD2797(config, m_wd2797, 4_MHz_XTAL / 4).set_force_ready(true);
	FLOPPY_CONNECTOR(config, m_floppies[0], nabu_fdc_drives, "525dd", fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], nabu_fdc_drives, "525dd", fdc_device::floppy_formats);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void fdc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void fdc_device::device_reset()
{
	m_wd2797->set_floppy(nullptr);
	m_wd2797->dden_w(0);
}

//-------------------------------------------------
//  supported floppy formats
//-------------------------------------------------
void fdc_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_NABUPC_FORMAT);
}

//-------------------------------------------------
//  ds_w - write to drive select/density register
//-------------------------------------------------
void fdc_device::ds_w(uint8_t data)
{
	m_wd2797->dden_w(BIT(data, 0));

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 1)) floppy = m_floppies[0]->get_device();
	if (BIT(data, 2)) floppy = m_floppies[1]->get_device();

	m_wd2797->set_floppy(floppy);

	for (auto& fdd : m_floppies)
	{
		floppy_image_device *floppy = fdd->get_device();
		if (floppy)
		{
			floppy->mon_w((data & 6) ? 0 : 1);
		}
	}
}

uint8_t fdc_device::read(offs_t offset)
{
	uint8_t result;

	switch(offset) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		result = m_wd2797->read(offset);
		break;
	case 0x0F:
		result = 0x10;
		break;
	default:
		result = 0xFF;
	}
	return result;
}

void fdc_device::write(offs_t offset, uint8_t data)
{
	switch(offset) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		m_wd2797->write(offset, data);
		break;
	case 0x0F:
		ds_w(data);
		break;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NABUPC_OPTION_FDC, bus::nabupc::device_option_expansion_interface, fdc_device, "nabupc_option_fdc", "NABU PC Floppy Controller")
