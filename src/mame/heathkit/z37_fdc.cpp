// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Z-37 Floppy controller

    This was an option for both the Heathkit H8 and H89 computer systems.

****************************************************************************/

#include "emu.h"

#include "z37_fdc.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(HEATH_Z37_FDC, heath_z37_fdc_device, "heath_z37_fdc", "Heath H/Z-37 Soft-sectored Controller");

heath_z37_fdc_device::heath_z37_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, HEATH_Z37_FDC, tag, owner, 0),
	m_fd_irq_cb(*this),
	m_drq_cb(*this),
	m_block_interrupt_cb(*this),
	m_fdc(*this, "z37_fdc"),
	m_floppies(*this, "z37_fdc:%u", 0U)
{
}


void heath_z37_fdc_device::ctrl_w(u8 val)
{
	bool motor_on = bool(BIT(val, ctrl_MotorsOn_c));

	m_intrq_allowed = bool(BIT(val, ctrl_EnableIntReq_c));
	m_drq_allowed = bool(BIT(val, ctrl_EnableDrqInt_c));
	m_fdc->dden_w(BIT(val, ctrl_SetMFMRecording_c) ? CLEAR_LINE : ASSERT_LINE);

	if (m_drq_allowed)
	{
		m_block_interrupt_cb(ASSERT_LINE);
	}
	else
	{
		m_block_interrupt_cb(CLEAR_LINE);
		m_drq_cb(CLEAR_LINE);
	}

	if (BIT(val, ctrl_Drive_0_c))
	{
		m_fdc->set_floppy(m_floppies[0]->get_device());
	}
	else if (BIT(val, ctrl_Drive_1_c))
	{
		m_fdc->set_floppy(m_floppies[1]->get_device());
	}
	else if (BIT(val, ctrl_Drive_2_c))
	{
		m_fdc->set_floppy(m_floppies[2]->get_device());
	}
	else if (BIT(val, ctrl_Drive_3_c))
	{
		m_fdc->set_floppy(m_floppies[3]->get_device());
	}
	else
	{
		m_fdc->set_floppy(nullptr);
	}

	for (auto &elem : m_floppies)
	{
		floppy_image_device *floppy = elem->get_device();
		if (floppy)
		{
			floppy->mon_w(!motor_on);
		}
	}
}

void heath_z37_fdc_device::intf_w(u8 val)
{
	m_access_track_sector = bool(BIT(val, if_SelectSectorTrack_c));
}

void heath_z37_fdc_device::cmd_w(u8 val)
{
	m_access_track_sector ? m_fdc->sector_w(val) : m_fdc->cmd_w(val);
}

u8 heath_z37_fdc_device::stat_r()
{
	return m_access_track_sector ? m_fdc->sector_r() : m_fdc->status_r();
}

void heath_z37_fdc_device::data_w(u8 val)
{
	m_access_track_sector ? m_fdc->track_w(val) : m_fdc->data_w(val);
}

u8 heath_z37_fdc_device::data_r()
{
	return m_access_track_sector ? m_fdc->track_r() : m_fdc->data_r();
}

void heath_z37_fdc_device::write(offs_t reg, u8 val)
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
		cmd_w(val);
		break;
	case 3:
		data_w(val);
		break;
	}
}

u8 heath_z37_fdc_device::read(offs_t reg)
{
	// default return for the h89
	u8 value = 0xff;

	switch (reg)
	{
	case 0:
	case 1:
		// read not supported on these addresses
		break;
	case 2:
		value = stat_r();
		break;
	case 3:
		value = data_r();
		break;
	}

	return value;
}

void heath_z37_fdc_device::device_start()
{
	save_item(NAME(m_intrq_allowed));
	save_item(NAME(m_drq_allowed));
	save_item(NAME(m_access_track_sector));

	m_intrq_allowed = false;
	m_drq_allowed = false;
	m_access_track_sector = false;
}

void heath_z37_fdc_device::device_reset()
{
	m_intrq_allowed = false;
	m_drq_allowed = false;
	m_access_track_sector = false;

	m_fd_irq_cb(CLEAR_LINE);
	m_drq_cb(CLEAR_LINE);
	m_block_interrupt_cb(CLEAR_LINE);
}

static void z37_floppies(device_slot_interface &device)
{
	// H-17-1
	device.option_add("ssdd", FLOPPY_525_SSDD);
	// SS 96tpi
	device.option_add("ssqd", FLOPPY_525_SSQD);
	// DS 48tpi
	device.option_add("dd", FLOPPY_525_DD);
	// H-17-4 / H-17-5 -- DS 96tpi
	device.option_add("qd", FLOPPY_525_QD);
}

void heath_z37_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(heath_z37_fdc_device::set_irq));
	m_fdc->drq_wr_callback().set(FUNC(heath_z37_fdc_device::set_drq));

	FLOPPY_CONNECTOR(config, m_floppies[0], z37_floppies, "qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[0]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], z37_floppies, "qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[1]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], z37_floppies, "qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[2]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], z37_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	m_floppies[3]->enable_sound(true);
}

void heath_z37_fdc_device::set_irq(u8 data)
{
	m_fd_irq_cb(m_intrq_allowed ? data : CLEAR_LINE);
}

void heath_z37_fdc_device::set_drq(u8 data)
{
	m_drq_cb(m_drq_allowed ? data : CLEAR_LINE);
}
