// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS Floppy Disc Controller Board

    Part No. CMS 0015-1

**********************************************************************/

#include "emu.h"
#include "fdc.h"

#include "formats/acorn_dsk.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


namespace {

class cms_fdc_device : public device_t, public device_acorn_bus_interface
{
public:
	cms_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, CMS_FDC, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_fdc(*this, "wd1770")
		, m_floppy(*this, "wd1770:%u", 0)
	{
	}

	static void floppy_formats(format_registration &fr);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;

	uint8_t wd1770_state_r();
	void wd1770_control_w(uint8_t data);
};


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void cms_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
	fr.add(FLOPPY_ACORN_DSD_FORMAT);
}

static void cms_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cms_fdc_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, m_floppy[0], cms_floppies, "35dd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], cms_floppies, "35dd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], cms_floppies, nullptr, floppy_formats).enable_sound(true);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cms_fdc_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xfc50, 0xfc5f, emu::rw_delegate(*this, FUNC(cms_fdc_device::wd1770_state_r)), emu::rw_delegate(*this, FUNC(cms_fdc_device::wd1770_control_w)));
	space.install_readwrite_handler(0xfc40, 0xfc4f, emu::rw_delegate(*m_fdc, FUNC(wd1770_device::read)), emu::rw_delegate(*m_fdc, FUNC(wd1770_device::write)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t cms_fdc_device::wd1770_state_r()
{
	uint8_t data = 0x3f;

	data |= m_fdc->intrq_r() << 6;
	data |= m_fdc->drq_r() << 7;

	return data;
}

void cms_fdc_device::wd1770_control_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// bit 0, 1, 2: drive select
	if (BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (BIT(data, 1)) floppy = m_floppy[1]->get_device();
	if (BIT(data, 2)) floppy = m_floppy[2]->get_device();
	m_fdc->set_floppy(floppy);

	// bit 3: side select
	if (floppy)
		floppy->ss_w(BIT(data, 3));

	// bit 7: density ??
	m_fdc->dden_w(BIT(data, 7));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(CMS_FDC, device_acorn_bus_interface, cms_fdc_device, "cms_fdc", "CMS Floppy Disc Controller Board")
