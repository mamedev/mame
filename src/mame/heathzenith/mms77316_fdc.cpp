// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Magnolia Microsystems 77316 soft-sectored floppy controller


****************************************************************************/

#include "emu.h"

#include "mms77316_fdc.h"

#define LOG_REG   (1U << 1)    // Shows register setup
#define LOG_LINES (1U << 2)    // Show control lines
#define LOG_DRIVE (1U << 3)    // Show drive select
#define LOG_FUNC  (1U << 4)    // Function calls
#define LOG_ERR   (1U << 5)    // log errors
#define LOG_BURST (1U << 6)    // burst mode

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGDRIVE(...)      LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGERR(...)        LOGMASKED(LOG_ERR, __VA_ARGS__)
#define LOGBURST(...)      LOGMASKED(LOG_BURST, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(MMS77316_FDC, mms77316_fdc_device, "mms77316_fdc", "Magnolia MicroSystems 77316 Soft-sectored Controller");

mms77316_fdc_device::mms77316_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, MMS77316_FDC, tag, owner, 0),
	m_irq_cb(*this),
	m_drq_cb(*this),
	m_fdc(*this, "mms_fdc"),
	m_floppies(*this, "mms_fdc:%u", 0U)
{
}

void mms77316_fdc_device::ctrl_w(u8 val)
{
	u8 floppy_drive = BIT(val, 0, 3);
	u8 five_in_drv  = bool(BIT(val, ctrl_525DriveSel_c));

	m_drq_count     = 0;
	m_irq_allowed   = bool(BIT(val, ctrl_EnableIntReq_c));
	m_drq_allowed   = m_irq_allowed && bool(BIT(val, ctrl_EnableDrqInt_c));

	m_fdc->dden_w(BIT(val, ctrl_SetMFMRecording_c));

	if (m_irq_allowed)
	{
		m_irq_cb(m_irq);
		m_drq_cb(m_drq);
	}
	else
	{
		m_irq_cb(CLEAR_LINE);
		m_drq_cb(CLEAR_LINE);
	}

	LOGREG("%s: floppydrive: %d\n", FUNCNAME, floppy_drive);

	m_fdc->set_floppy(m_floppies[floppy_drive]->get_device());

	LOGREG("%s: intrq allowed: %d, drq allowed: %d\n", FUNCNAME, m_irq_allowed, m_drq_allowed);

	m_fdc->set_clock(five_in_drv ? FIVE_IN_CLOCK : EIGHT_IN_CLOCK);

	for (int i = 4; i < 8; i++)
	{
		auto elem = m_floppies[i];
		if (elem)
		{
			floppy_image_device *floppy = elem->get_device();
			if (floppy)
			{
				// turn on all installed 5" floppies
				floppy->mon_w(!five_in_drv);
			}
		}
	}
}

void mms77316_fdc_device::data_w(u8 val)
{
	if (burstMode())
	{
		// TODO add wait states in burst mode, currently blocked on Z80 properly supporting wait states
		LOGBURST("%s: burst_mode: %d\n", FUNCNAME, val);
	}

	m_fdc->data_w(val);
}

void mms77316_fdc_device::write(offs_t reg, u8 val)
{
	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, val);

	switch (reg)
	{
	case 0:
		ctrl_w(val);
		break;
	case 1:
	case 2:
	case 3:
		LOGFUNC("%s: Unexpected port write reg: %d val: 0x%02x\n", FUNCNAME, reg, val);
		break;
	case 4:
		m_fdc->cmd_w(val);
		break;
	case 5:
		m_fdc->track_w(val);
		break;
	case 6:
		m_fdc->sector_w(val);
		break;
	case 7:
		data_w(val);
		break;
	}
}

u8 mms77316_fdc_device::data_r()
{
	if (burstMode())
	{
		// TODO add wait states in burst mode, currently blocked on Z80 properly supporting wait states
		LOGBURST("%s: burst_mode\n", FUNCNAME);
	}

	return m_fdc->data_r();
}

u8 mms77316_fdc_device::read(offs_t reg)
{
	// default return for the h89
	u8 value = 0xff;

	switch (reg)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		// read not supported on these addresses
		LOGERR("%s: Unexpected port read reg: %d\n", FUNCNAME, reg);
		break;
	case 4:
		value = m_fdc->status_r();
		break;
	case 5:
		value = m_fdc->track_r();
		break;
	case 6:
		value = m_fdc->sector_r();
		break;
	case 7:
		value = data_r();
		break;
	}

	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, value);

	return value;
}

void mms77316_fdc_device::device_start()
{
	save_item(NAME(m_irq_allowed));
	save_item(NAME(m_drq_allowed));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
	save_item(NAME(m_drq_count));
}

void mms77316_fdc_device::device_reset()
{
	m_irq_allowed = false;
	m_drq_allowed = false;
	m_irq         = false;
	m_drq_count   = 0;

	m_irq_cb(0);
	m_drq_cb(0);
	for (int i = 0; i < 4; i++)
	{
		auto elem = m_floppies[i];
		if (elem)
		{
			floppy_image_device *floppy = elem->get_device();
			if (floppy)
			{
				// turn on motor of all installed 8" floppies
				floppy->mon_w(0);
			}
		}
	}
}

static void mms_5_in_floppies(device_slot_interface &device)
{
	// H-17-1 -- SS 48tpi
	device.option_add("5_ssdd", FLOPPY_525_SSDD);
	// SS 96tpi
	device.option_add("5_ssqd", FLOPPY_525_SSQD);
	// DS 48tpi
	device.option_add("5_dd",   FLOPPY_525_DD);
	// H-17-4 / H-17-5 -- DS 96tpi
	device.option_add("5_qd",   FLOPPY_525_QD);
}

static void mms_8_in_floppies(device_slot_interface &device)
{
	// 8" DSDD
	device.option_add("8_sssd", FLOPPY_8_SSSD);
	// 8" SSDD
	device.option_add("8_dssd", FLOPPY_8_DSSD);
	// 8" DSDD
	device.option_add("8_ssdd", FLOPPY_8_SSDD);
	// 8" SSDD
	device.option_add("8_dsdd", FLOPPY_8_DSDD);
}

void mms77316_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, EIGHT_IN_CLOCK);
	m_fdc->intrq_wr_callback().set(FUNC(mms77316_fdc_device::set_irq));
	m_fdc->drq_wr_callback().set(FUNC(mms77316_fdc_device::set_drq));

	// 8" Floppy drives
	FLOPPY_CONNECTOR(config, m_floppies[0], mms_8_in_floppies, "8_dsdd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[0]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], mms_8_in_floppies, "8_dsdd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[1]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], mms_8_in_floppies, "8_dsdd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[2]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], mms_8_in_floppies, "8_dsdd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[3]->enable_sound(true);

	// 5" Floppy drives
	FLOPPY_CONNECTOR(config, m_floppies[4], mms_5_in_floppies, "5_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[4]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[5], mms_5_in_floppies, "5_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[5]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[6], mms_5_in_floppies, "5_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[6]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[7], mms_5_in_floppies, "5_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[7]->enable_sound(true);
}

void mms77316_fdc_device::set_irq(int state)
{
	LOGLINES("set irq, allowed: %d state: %d\n", m_irq_allowed, state);

	if (state)
	{
		m_drq_count = 0;
	}

	m_irq = state;

	m_irq_cb(m_irq_allowed ? m_irq : CLEAR_LINE);
}

void mms77316_fdc_device::set_drq(int state)
{
	bool drq_allowed = false;

	LOGLINES("set drq, allowed: %d state: %d\n", m_drq_allowed, state);
	m_drq = state;

	if (m_drq)
	{
		// even if drq bit is not set, trigger if the first one after an IRQ.
		drq_allowed = m_drq_allowed || (m_drq_count++ == 0);
	}

	m_drq_cb(drq_allowed ? m_drq : CLEAR_LINE);
}
