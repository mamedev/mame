// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  CDR FDC-880H soft-sectored floppy controller

    Supports up to 4 floppy drives, both 5.25" and 8" drives.

****************************************************************************/

#include "emu.h"

#include "cdr_fdc_880h.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

#define LOG_REG   (1U << 1)    // Shows register setup
#define LOG_LINES (1U << 2)    // Show control lines
#define LOG_DRIVE (1U << 3)    // Show drive select
#define LOG_FUNC  (1U << 4)    // Function calls
#define LOG_ERR   (1U << 5)    // log errors
#define LOG_WAIT  (1U << 6)    // wait mode
#define LOG_DATA  (1U << 7)    // data read/writes

#define VERBOSE (0xff)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGDRIVE(...)      LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGERR(...)        LOGMASKED(LOG_ERR, __VA_ARGS__)
#define LOGWAIT(...)       LOGMASKED(LOG_WAIT, __VA_ARGS__)
#define LOGDATA(...)       LOGMASKED(LOG_DATA, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace {

class cdr_fdc_880h_device : public device_t, public device_h89bus_right_card_interface
{
public:
	cdr_fdc_880h_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void write(u8 select_lines, u8 offset, u8 data) override;
	virtual u8 read(u8 select_lines, u8 offset) override;

	// The controller has two 16L8 PALs which are not dumped (z15 and z20 from schematics).
	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void cmd_w(u8 val);
	void data_w(u8 val);
	u8 data_r();
	u8 status_r();

	void set_irq(int state);
	void set_drq(int state);

private:
	/// Bits set in cmd_ControlPort_c
	static constexpr u8 ctrl_EnableIntReq_c    = 7;
	static constexpr u8 ctrl_SetMFMRecording_c = 6;
	static constexpr u8 ctrl_DriveType_c       = 5;
	static constexpr u8 ctrl_Mode_c            = 4;

	static constexpr XTAL MASTER_CLOCK         = XTAL(4'000'000);
	static constexpr XTAL FIVE_IN_CLOCK        = MASTER_CLOCK / 4;
	static constexpr XTAL EIGHT_IN_CLOCK       = MASTER_CLOCK / 2;

	required_device<fd1797_device>             m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;

	bool                                       m_irq_allowed;

	bool                                       m_irq;
	bool                                       m_drq;

	bool                                       m_double_density;
	bool                                       m_five_in_drive;
	bool                                       m_mode_operate;
	s8                                         m_drive;
};


cdr_fdc_880h_device::cdr_fdc_880h_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_CDR_FDC_880H, tag, owner, 0),
	device_h89bus_right_card_interface(mconfig, *this),
	m_fdc(*this, "cdr_fdc_880h"),
	m_floppies(*this, "cdr_fdc_880h:%u", 0U)
{
}

void cdr_fdc_880h_device::cmd_w(u8 val)
{
	LOGREG("%s: val: %d\n", FUNCNAME, val);

	u8 drive = BIT(val, 0, 3);

	if (drive & 0x01)
	{
		m_drive = 0;
	}
	else if (drive & 0x02)
	{
		m_drive = 1;
	}
	else if (drive & 0x04)
	{
		m_drive = 2;
	}
	else if (drive & 0x08)
	{
		m_drive = 3;
	}
	else
	{
		m_drive = -1;
	}
	m_mode_operate = bool(BIT(val, ctrl_Mode_c));
	m_five_in_drive = bool(BIT(val, ctrl_DriveType_c));
	m_double_density = !bool(BIT(val, ctrl_SetMFMRecording_c));
	m_irq_allowed = bool(BIT(val, ctrl_EnableIntReq_c));

	m_fdc->dden_w(!m_double_density);

	LOGLINES("%s: intrq allowed: %d\n", FUNCNAME, m_irq_allowed);

	set_slot_int5(m_irq_allowed ? m_irq : CLEAR_LINE);

	LOGDRIVE("%s: floppydrive: %d, 5.25 in: %d\n", FUNCNAME, m_drive, m_five_in_drive);

	if (m_drive >= 0)
	{
		m_fdc->set_floppy(m_floppies[m_drive]->get_device());
	}

	for (auto &elem : m_floppies)
	{
		floppy_image_device *floppy = elem->get_device();
		if (floppy)
		{
			floppy->mon_w(m_drive == -1);
		}
	}

	m_fdc->set_clock(m_five_in_drive ? FIVE_IN_CLOCK : EIGHT_IN_CLOCK);
}

void cdr_fdc_880h_device::data_w(u8 val)
{
	LOGDATA("%s: val: %d\n", FUNCNAME, val);

	if (!m_drq && !m_irq)
	{
		LOGWAIT("%s: wait state\n", FUNCNAME);

		set_slot_wait(ASSERT_LINE);
		return;
	}

	m_fdc->data_w(val);
}

void cdr_fdc_880h_device::write(u8 select_lines, u8 offset, u8 data)
{
	if (!(select_lines & h89bus_device::H89_CASS))
	{
		return;
	}

	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, data);

	switch (offset)
	{
	case 0:
	case 1:
	case 2:
		// read not supported on these addresses
		LOGERR("%s: Unexpected port read reg: %d\n", FUNCNAME, offset);
		break;
	case 3: // Select Port
		cmd_w(data);
		break;
	case 4: // Command Port
		m_fdc->cmd_w(data);
		break;
	case 5: // Track Port
		m_fdc->track_w(data);
		break;
	case 6: // Sector Port
		m_fdc->sector_w(data);
		break;
	case 7: // Data Port
		data_w(data);
		break;
	}
}

u8 cdr_fdc_880h_device::status_r()
{
	u8 data = 0;

	switch(m_drive)
	{
	case 0:
		data |= 0x0e;
		break;
	case 1:
		data |= 0x0d;
		break;
	case 2:
		data |= 0x0b;
		break;
	case 3:
		data |= 0x07;
		break;
	case -1:
	default:
		data |= 0x0f;
		break;
	}
	data |= m_mode_operate ? 0x10 : 0;
	data |= m_five_in_drive ? 0 : 0x20;
	data |= m_double_density ? 0x40 : 0;
	data |= m_irq ? 0 : 0x80;

	return data;
}

u8 cdr_fdc_880h_device::data_r()
{
	u8 data = 0;

	if (!m_drq && !m_irq)
	{
		LOGWAIT("%s: wait state\n", FUNCNAME);

		if (!machine().side_effects_disabled())
		{
			set_slot_wait(ASSERT_LINE);
		}
	}
	else
	{
		data = m_fdc->data_r();
	}

	LOGDATA("%s: data: %d\n", FUNCNAME, data);

	return data;
}

u8 cdr_fdc_880h_device::read(u8 select_lines, u8 offset)
{
	if (!(select_lines & h89bus_device::H89_CASS))
	{
		return 0;
	}

	u8 value = 0;

	switch (offset)
	{
	case 0:
	case 1:
	case 2:
		// read not supported on these addresses
		LOGERR("%s: Unexpected port read reg: %d\n", FUNCNAME, offset);
		break;
	case 3: // ??
		value = status_r();
		break;
	case 4: // Drive Status Port
		value = m_fdc->status_r();
		break;
	case 5: // Track Port
		value = m_fdc->track_r();
		break;
	case 6: // Sector Port
		value = m_fdc->sector_r();
		break;
	case 7: // Data Port
		value = data_r();
		break;
	}

	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, value);

	return value;
}

void cdr_fdc_880h_device::device_start()
{
	save_item(NAME(m_irq_allowed));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
}

void cdr_fdc_880h_device::device_reset()
{
	m_irq_allowed = false;
	m_irq         = false;

	set_slot_int5(CLEAR_LINE);
	set_slot_wait(CLEAR_LINE);

	for (auto &elem : m_floppies)
	{
		if (elem)
		{
			floppy_image_device *const floppy = elem->get_device();
			if (floppy)
			{
				// turn on motor of all installed 8" floppies
				floppy->mon_w(0);
			}
		}
	}
}

static void cdr_floppies(device_slot_interface &device)
{
	// 5.25" SS 48tpi
	device.option_add("5_ss_dd", FLOPPY_525_SSDD);
	// 5.25" SS 96tpi
	device.option_add("5_ss_qd", FLOPPY_525_SSQD);
	// 5.25" DS 48tpi
	device.option_add("5_ds_dd", FLOPPY_525_DD);
	// 5.25" DS 96tpi
	device.option_add("5_ds_qd", FLOPPY_525_QD);

	// 8" SSSD
	device.option_add("8_ss_sd", FLOPPY_8_SSSD);
	// 8" DSSD
	device.option_add("8_ds_sd", FLOPPY_8_DSSD);
	// 8" SSDD
	device.option_add("8_ss_dd", FLOPPY_8_SSDD);
	// 8" DSDD
	device.option_add("8_ds_dd", FLOPPY_8_DSDD);
}

void cdr_fdc_880h_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, EIGHT_IN_CLOCK);
	m_fdc->intrq_wr_callback().set(FUNC(cdr_fdc_880h_device::set_irq));
	m_fdc->drq_wr_callback().set(FUNC(cdr_fdc_880h_device::set_drq));

	FLOPPY_CONNECTOR(config, m_floppies[0], cdr_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[0]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], cdr_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[1]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], cdr_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[2]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], cdr_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[3]->enable_sound(true);
}

void cdr_fdc_880h_device::set_irq(int state)
{
	LOGLINES("set irq, allowed: %d state: %d\n", m_irq_allowed, state);

	m_irq = state;

	if (m_irq)
	{
		set_slot_wait(CLEAR_LINE);
	}

	set_slot_int5(m_irq_allowed ? m_irq : CLEAR_LINE);
}

void cdr_fdc_880h_device::set_drq(int state)
{
	LOGLINES("set drq state: %d\n", state);

	m_drq = state;

	if (m_drq)
	{
		set_slot_wait(CLEAR_LINE);
	}

}
}

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_CDR_FDC_880H, device_h89bus_right_card_interface, cdr_fdc_880h_device, "cdr_fdc_880h", "CDR FDC-880H Soft-sectored Controller");
