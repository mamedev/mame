// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Z-37 Floppy controller

    This was an option for both the Heathkit H8 and H89 computer systems.

****************************************************************************/

#include "emu.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

#include "z37_fdc.h"

#define LOG_REG (1U << 1)    // Shows register setup
#define LOG_LINES (1U << 2)  // Show control lines
#define LOG_DRIVE (1U << 3)  // Show drive select
#define LOG_FUNC (1U << 4)   // Function calls
#define LOG_ERR (1U << 5)    // log errors
#define LOG_SETUP (1U << 6)  // setup

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGDRIVE(...)      LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGERR(...)        LOGMASKED(LOG_ERR, __VA_ARGS__)
#define LOGSETUP(...)      LOGMASKED(LOG_SETUP, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


h89bus_z37_device::h89bus_z37_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H89BUS_Z37, tag, owner, 0)
	, device_h89bus_right_card_interface(mconfig, *this)
	, m_fdc(*this, "z37_fdc")
	, m_floppies(*this, "z37_fdc:%u", 0U)
	, m_intr_cntrl(*this, finder_base::DUMMY_TAG)
{
}

void h89bus_z37_device::ctrl_w(u8 val)
{
	bool motor_on = bool(BIT(val, ctrl_MotorsOn_c));

	m_irq_allowed = bool(BIT(val, ctrl_EnableIntReq_c));
	m_drq_allowed = bool(BIT(val, ctrl_EnableDrqInt_c));
	m_fdc->dden_w(BIT(~val, ctrl_SetMFMRecording_c));

	LOGREG("%s: motor on: %d, intrq allowed: %d, drq allowed: %d\n",
		FUNCNAME, motor_on, m_irq_allowed, m_drq_allowed);

	if (m_drq_allowed)
	{
		m_intr_cntrl->block_interrupts(1);
	}
	else
	{
		m_intr_cntrl->block_interrupts(0);
		m_intr_cntrl->set_drq(0);
	}

	if (BIT(val, ctrl_Drive_0_c))
	{
		m_fdc->set_floppy(m_floppies[0]->get_device());
		LOGDRIVE("Drive selected: 0\n");
	}
	else if (BIT(val, ctrl_Drive_1_c))
	{
		m_fdc->set_floppy(m_floppies[1]->get_device());
		LOGDRIVE("Drive selected: 1\n");
	}
	else if (BIT(val, ctrl_Drive_2_c))
	{
		m_fdc->set_floppy(m_floppies[2]->get_device());
		LOGDRIVE("Drive selected: 2\n");
	}
	else if (BIT(val, ctrl_Drive_3_c))
	{
		m_fdc->set_floppy(m_floppies[3]->get_device());
		LOGDRIVE("Drive selected: 3\n");
	}
	else
	{
		m_fdc->set_floppy(nullptr);
		LOGDRIVE("Drive selected: none\n");
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

void h89bus_z37_device::intf_w(u8 val)
{
	m_access_track_sector = bool(BIT(val, if_SelectSectorTrack_c));

	LOGREG("access track/sector: %d\n", m_access_track_sector);
}

void h89bus_z37_device::cmd_w(u8 val)
{
	m_access_track_sector ? m_fdc->sector_w(val) : m_fdc->cmd_w(val);
}

u8 h89bus_z37_device::stat_r()
{
	return m_access_track_sector ? m_fdc->sector_r() : m_fdc->status_r();
}

void h89bus_z37_device::data_w(u8 val)
{
	m_access_track_sector ? m_fdc->track_w(val) : m_fdc->data_w(val);
}

u8 h89bus_z37_device::data_r()
{
	return m_access_track_sector ? m_fdc->track_r() : m_fdc->data_r();
}

void h89bus_z37_device::write(offs_t offset, u8 data)
{
	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, data);

	switch (offset)
	{
		case 0:
			ctrl_w(data);
			break;
		case 1:
			intf_w(data);
			break;
		case 2:
			cmd_w(data);
			break;
		case 3:
			data_w(data);
			break;
	}
}

u8 h89bus_z37_device::read(offs_t offset)
{
	// default return for the h89
	u8 value = 0xff;

	switch (offset)
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

	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, value);

	return value;
}

void h89bus_z37_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
	save_item(NAME(m_irq_allowed));
	save_item(NAME(m_drq_allowed));
	save_item(NAME(m_access_track_sector));
}

void h89bus_z37_device::device_reset()
{
	if (!m_installed)
	{
		h89bus::addr_ranges  addr_ranges = h89bus().get_address_ranges(h89bus::IO_CASS);

		if (addr_ranges.size() == 1)
		{
			h89bus::addr_range range = addr_ranges.front();

			LOGSETUP("%s: addr: 0x%04x-0x%04x\n", FUNCNAME, range.first, range.second);

			h89bus().install_io_device(range.first, range.second,
				read8sm_delegate(*this, FUNC(h89bus_z37_device::read)),
				write8sm_delegate(*this, FUNC(h89bus_z37_device::write)));
		}
		else
		{
			LOGERR("%s: no address provided for device\n", FUNCNAME);
		}

		m_installed = true;
	}

	m_irq_allowed         = false;
	m_drq_allowed         = false;
	m_access_track_sector = false;

	m_intr_cntrl->set_irq(0);
	m_intr_cntrl->set_drq(0);
	m_intr_cntrl->block_interrupts(0);
}

static void z37_floppies(device_slot_interface &device)
{
	// H-17-1 -- SS 48tpi
	device.option_add("ssdd", FLOPPY_525_SSDD);
	// SS 96tpi
	device.option_add("ssqd", FLOPPY_525_SSQD);
	// DS 48tpi
	device.option_add("dd",   FLOPPY_525_DD);
	// H-17-4 / H-17-5 -- DS 96tpi
	device.option_add("qd",   FLOPPY_525_QD);
}

void h89bus_z37_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(h89bus_z37_device::set_irq));
	m_fdc->drq_wr_callback().set(FUNC(h89bus_z37_device::set_drq));
	// Z-89-37 schematics show the ready line tied high.
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, m_floppies[0], z37_floppies, "qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[0]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], z37_floppies, "qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[1]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], z37_floppies, "qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[2]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], z37_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	m_floppies[3]->enable_sound(true);
}

void h89bus_z37_device::set_irq(int state)
{
	LOGLINES("set irq, allowed: %d state: %d\n", m_irq_allowed, state);

	m_intr_cntrl->set_irq(m_irq_allowed ? state : CLEAR_LINE);
}

void h89bus_z37_device::set_drq(int state)
{
	LOGLINES("set drq, allowed: %d state: %d\n", m_drq_allowed, state);

	m_intr_cntrl->set_drq(m_drq_allowed ? state : CLEAR_LINE);
}

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_Z37, device_h89bus_right_card_interface, h89bus_z37_device, "h89_z37", "Heathkit Z-37 Floppy Disk Controller");
