// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Z-37 Floppy controller

    This was an option for both the Heathkit H8 and H89 computer systems.

****************************************************************************/

#include "emu.h"

#include "z37_fdc.h"

DEFINE_DEVICE_TYPE(HEATH_Z37_FDC, heath_z37_fdc_device, "heath_z37_fdc", "Heath H/Z-37 Soft-sectored Controller");

heath_z37_fdc_device::heath_z37_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, HEATH_Z37_FDC, tag, owner, 0)
		, m_raise_irq_cb(*this)
		, m_raise_drq_cb(*this)
		, m_block_interrupt_cb(*this)
		, m_fdc(*this, "z37_fdc")
		, m_floppies(*this, "z37_fdc:%u", 0U)
		, m_control_reg(0)
		, m_interface_reg(0)
		, m_intrq_allowed(false)
		, m_drq_allowed(false)
		, m_access_track_sector(false)
		, m_floppy(nullptr)
{
}


void heath_z37_fdc_device::ctrl_w(uint8_t val)
{
	m_control_reg = val;

	m_intrq_allowed = bool(BIT(val, ctrl_EnableIntReq_c));
	m_drq_allowed = bool(BIT(val, ctrl_EnableDrqInt_c));
	m_fdc->dden_w(BIT(val, ctrl_SetMFMRecording_c));
	m_fdc->mr_w(BIT(val, ctrl_MotorsOn_c));

	if (BIT(val, ctrl_Drive_0_c))
	{
		m_floppy = m_floppies[0]->get_device();
	}
	if (BIT(val, ctrl_Drive_1_c))
	{
		m_floppy = m_floppies[1]->get_device();
	}
	if (BIT(val, ctrl_Drive_2_c))
	{
		m_floppy = m_floppies[2]->get_device();
	}
	if (BIT(val, ctrl_Drive_3_c))
	{
		m_floppy = m_floppies[3]->get_device();
	}

	m_fdc->set_floppy(m_floppy);
}

uint8_t heath_z37_fdc_device::ctrl_r()
{
	return m_control_reg;
}

void heath_z37_fdc_device::intf_w(uint8_t val)
{

	m_access_track_sector = bool(BIT(val, if_SelectSectorTrack_c));
}

uint8_t heath_z37_fdc_device::intf_r()
{
	return m_interface_reg;
}

void heath_z37_fdc_device::stat_w(uint8_t val)
{
	if (m_access_track_sector)
	{
		m_fdc->sector_w(val);
	}
	else
	{
		m_fdc->cmd_w(val);
	}
}

uint8_t heath_z37_fdc_device::stat_r()
{
	if (m_access_track_sector)
	{
		return m_fdc->sector_r();
	}
	else
	{
		return m_fdc->status_r();
	}
}

void heath_z37_fdc_device::data_w(uint8_t val)
{
	if (m_access_track_sector)
	{
		m_fdc->track_w(val);
	}
	else
	{
		m_fdc->data_w(val);
	}
}

uint8_t heath_z37_fdc_device::data_r()
{
	if (m_access_track_sector)
	{
		return m_fdc->track_r();
	}
	else
	{
		return m_fdc->data_r();
	}
}

void heath_z37_fdc_device::write(offs_t reg, uint8_t val)
{
	switch (reg)
	{
	case 0:
		ctrl_w(val);
		break;
	case 1:
		intf_w(val);
		break;
	case 2:
		stat_w(val);
		break;
	case 3:
		data_w(val);
		break;
	}
}

uint8_t heath_z37_fdc_device::read(offs_t reg)
{
	switch (reg)
	{
	case 0:
		return ctrl_r();
	case 1:
		return intf_r();
	case 2:
		return stat_r();
	case 3:
		return data_r();
	}
	return 0xff;
}

void heath_z37_fdc_device::device_start()
{

	save_item(NAME(m_control_reg));
	save_item(NAME(m_interface_reg));
	save_item(NAME(m_intrq_allowed));
	save_item(NAME(m_drq_allowed));
	save_item(NAME(m_access_track_sector));

	m_control_reg = 0;
	m_interface_reg = 0;

	m_intrq_allowed = false;
	m_drq_allowed = false;
	m_access_track_sector = false;

}

static void z37_floppies(device_slot_interface &device)
{
	// H-17-1 - density is a function of the controller, and this one support both ?
	device.option_add("sssd", FLOPPY_525_SSSD);
	device.option_add("ssdd", FLOPPY_525_SSDD);
	// SS 96tpi
	device.option_add("ssqd", FLOPPY_525_SSQD);
	// DS 48tpi
	device.option_add("dd", FLOPPY_525_DD);
	// DS 96tpi
	device.option_add("qd", FLOPPY_525_QD);
}

void heath_z37_fdc_device::device_add_mconfig(machine_config &config)
{

	FD1797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(heath_z37_fdc_device::raise_irq));
	m_fdc->drq_wr_callback().set(FUNC(heath_z37_fdc_device::raise_drq));

	FLOPPY_CONNECTOR(config, m_floppies[0], z37_floppies, "ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], z37_floppies, "ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[2], z37_floppies, "qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[3], z37_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

}

void heath_z37_fdc_device::device_resolve_objects()
{
	m_raise_irq_cb.resolve_safe();
	m_raise_drq_cb.resolve_safe();
}

void heath_z37_fdc_device::raise_irq(uint8_t data)
{
	if (m_intrq_allowed) {
		m_raise_irq_cb(data);
	}
}

void heath_z37_fdc_device::raise_drq(uint8_t data)
{
	if (m_drq_allowed) {
		m_raise_drq_cb(data);
	}
}
