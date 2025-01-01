// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Magnolia Microsystems 77316 DD soft-sectored floppy controller

    Supported upto 8 floppy drives
    - 4 8" drives
    - 4 5.25" drives

****************************************************************************/

#include "emu.h"

#include "mms77316_fdc.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

#define LOG_REG   (1U << 1)    // Shows register setup
#define LOG_LINES (1U << 2)    // Show control lines
#define LOG_DRIVE (1U << 3)    // Show drive select
#define LOG_FUNC  (1U << 4)    // Function calls
#define LOG_ERR   (1U << 5)    // log errors
#define LOG_BURST (1U << 6)    // burst mode
#define LOG_DATA  (1U << 7)    // data read/writes

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGDRIVE(...)      LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGERR(...)        LOGMASKED(LOG_ERR, __VA_ARGS__)
#define LOGBURST(...)      LOGMASKED(LOG_BURST, __VA_ARGS__)
#define LOGDATA(...)       LOGMASKED(LOG_DATA, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

class mms77316_fdc_device : public device_t, public device_h89bus_right_card_interface
{
public:
	mms77316_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void write(u8 select_lines, u8 offset, u8 data) override;
	virtual u8 read(u8 select_lines, u8 offset) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void ctrl_w(u8 val);
	void data_w(u8 val);
	u8 data_r();

	void set_irq(int state);
	void set_drq(int state);

	// Burst mode was required for a 2 MHz Z80 to handle 8" DD data rates.
	// The typical irq/drq was too slow, this utilizes wait states to read the
	// WD1797 data port once the drq line is high.
	inline bool burst_mode_r() { return !m_drq_allowed; }

private:
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 8> m_floppies;

	bool m_irq_allowed;
	bool m_drq_allowed;

	bool m_irq;
	bool m_drq;
	u32 m_drq_count;

	/// Bits set in cmd_ControlPort_c
	static constexpr u8 ctrl_525DriveSel_c = 2;
	static constexpr u8 ctrl_EnableIntReq_c = 3;
	static constexpr u8 ctrl_EnableDrqInt_c = 5;
	static constexpr u8 ctrl_SetMFMRecording_c = 6;

	static constexpr XTAL MASTER_CLOCK = XTAL(8'000'000);
	static constexpr XTAL FIVE_IN_CLOCK = MASTER_CLOCK / 8;
	static constexpr XTAL EIGHT_IN_CLOCK = MASTER_CLOCK / 4;
};

mms77316_fdc_device::mms77316_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_MMS77316, tag, owner, 0),
	device_h89bus_right_card_interface(mconfig, *this),
	m_fdc(*this, "mms_fdc"),
	m_floppies(*this, "mms_fdc:%u", 0U)
{
}

void mms77316_fdc_device::ctrl_w(u8 val)
{
	LOGREG("%s: val: %d\n", FUNCNAME, val);

	u8 floppy_drive = BIT(val, 0, 3);
	u8 five_in_drv  = bool(BIT(val, ctrl_525DriveSel_c));

	m_drq_count     = 0;
	m_irq_allowed   = bool(BIT(val, ctrl_EnableIntReq_c));
	m_drq_allowed   = m_irq_allowed && bool(BIT(val, ctrl_EnableDrqInt_c));

	m_fdc->dden_w(BIT(val, ctrl_SetMFMRecording_c));

	LOGLINES("%s: intrq allowed: %d, drq allowed: %d\n", FUNCNAME, m_irq_allowed, m_drq_allowed);

	if (m_irq_allowed)
	{
		set_slot_fdcirq(m_irq);
		set_slot_fdcdrq(m_drq);
	}
	else
	{
		set_slot_fdcirq(CLEAR_LINE);
		set_slot_fdcdrq(CLEAR_LINE);
	}

	LOGDRIVE("%s: floppydrive: %d, 5.25 in: %d\n", FUNCNAME, floppy_drive, five_in_drv);

	m_fdc->set_floppy(m_floppies[floppy_drive]->get_device());

	m_fdc->set_clock(five_in_drv ? FIVE_IN_CLOCK : EIGHT_IN_CLOCK);

	for (int i = 4; i < 8; i++)
	{
		auto elem = m_floppies[i];
		if (elem)
		{
			floppy_image_device *floppy = elem->get_device();
			if (floppy)
			{
				// set motor for installed 5" drives
				floppy->mon_w(!five_in_drv);
			}
		}
	}
}

void mms77316_fdc_device::data_w(u8 val)
{
	LOGDATA("%s: val: %d\n", FUNCNAME, val);

	if (burst_mode_r() && !m_drq && !m_irq)
	{
		LOGBURST("%s: burst_mode_r\n", FUNCNAME);

		set_slot_wait(ASSERT_LINE);
		return;
	}

	m_fdc->data_w(val);
}

void mms77316_fdc_device::write(u8 select_lines, u8 reg, u8 val)
{
	if (!(select_lines & h89bus_device::H89_GPP))
	{
		return;
	}

	if (reg != 2) LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, val);

	switch (reg)
	{
	case 0:
		ctrl_w(val);
		break;
	case 1:
	case 2:
	case 3:
		LOGERR("%s: Unexpected port write reg: %d val: 0x%02x\n", FUNCNAME, reg, val);
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
	u8 data = 0;

	if (burst_mode_r() && !m_drq && !m_irq)
	{
		LOGBURST("%s: burst_mode setting wait state\n", FUNCNAME);

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

u8 mms77316_fdc_device::read(u8 select_lines, u8 reg)
{
	if (!(select_lines & h89bus_device::H89_GPP))
	{
		return 0;
	}

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

	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, value);

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

	set_slot_fdcirq(CLEAR_LINE);
	set_slot_fdcdrq(CLEAR_LINE);
	set_slot_wait(CLEAR_LINE);

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
	device.option_add("5_ss_dd", FLOPPY_525_SSDD);
	// SS 96tpi
	device.option_add("5_ss_qd", FLOPPY_525_SSQD);
	// DS 48tpi
	device.option_add("5_ds_dd", FLOPPY_525_DD);
	// H-17-4 / H-17-5 -- DS 96tpi
	device.option_add("5_ds_qd", FLOPPY_525_QD);
}

static void mms_8_in_floppies(device_slot_interface &device)
{
	// 8" DSDD
	device.option_add("8_ss_sd", FLOPPY_8_SSSD);
	// 8" SSDD
	device.option_add("8_ds_sd", FLOPPY_8_DSSD);
	// 8" DSDD
	device.option_add("8_ss_dd", FLOPPY_8_SSDD);
	// 8" SSDD
	device.option_add("8_ds_dd", FLOPPY_8_DSDD);
}

void mms77316_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, EIGHT_IN_CLOCK);
	m_fdc->intrq_wr_callback().set(FUNC(mms77316_fdc_device::set_irq));
	m_fdc->drq_wr_callback().set(FUNC(mms77316_fdc_device::set_drq));

	// 8" Floppy drives
	FLOPPY_CONNECTOR(config, m_floppies[0], mms_8_in_floppies, "8_ds_dd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[0]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], mms_8_in_floppies, "8_ds_dd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[1]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], mms_8_in_floppies, "8_ds_dd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[2]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], mms_8_in_floppies, "8_ds_dd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[3]->enable_sound(true);

	// 5" Floppy drives
	FLOPPY_CONNECTOR(config, m_floppies[4], mms_5_in_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[4]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[5], mms_5_in_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[5]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[6], mms_5_in_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[6]->enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[7], mms_5_in_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
	m_floppies[7]->enable_sound(true);
}

void mms77316_fdc_device::set_irq(int state)
{
	LOGLINES("set irq, allowed: %d state: %d\n", m_irq_allowed, state);

	m_irq = state;

	if (m_irq)
	{
		set_slot_wait(CLEAR_LINE);
		m_drq_count = 0;
	}

	set_slot_fdcirq(m_irq_allowed ? m_irq : CLEAR_LINE);
}

void mms77316_fdc_device::set_drq(int state)
{
	LOGLINES("set drq, allowed: %d state: %d\n", m_drq_allowed, state);

	m_drq = state;

	if (burst_mode_r())
	{
		LOGBURST("%s: in burst mode drq: %d, m_drq_count: %d\n", FUNCNAME, m_drq, m_drq_count);

		if (m_drq)
		{
			set_slot_wait(CLEAR_LINE);
		}

		set_slot_fdcdrq(m_drq_count == 0 ? m_drq : CLEAR_LINE);
	}
	else
	{
		set_slot_fdcdrq(m_drq_allowed ? m_drq : CLEAR_LINE);
	}
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MMS77316, device_h89bus_right_card_interface, mms77316_fdc_device, "mms77316_fdc", "Magnolia MicroSystems 77316 Soft-sectored Controller");
