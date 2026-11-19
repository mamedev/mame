// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  CDR FDC-880H soft-sectored floppy controller

    Supports up to 4 floppy drives, both 5.25" and 8" drives.

    Registers (from appendix A-6 of the FDC-880H hardware manual)

    Port 3Bh write - control latch
      bit 7    1 = interrupts enabled
      bit 6    1 = single density, 0 = double density
      bit 5    1 = 5.25" drive, 0 = 8" drive
      bit 4    1 = operate, 0 = reset (FD1797 master reset)
      bit 3-0  drive select: 0 = none, 1/2/4/8 = drive 0/1/2/3

    Port 3Bh read - status
      bit 7    0 = interrupt pending
      bit 6    1 = side 0, 0 = side 1
      bit 5    1 = 8" drive, 0 = 5.25" drive
      bit 4    1 = operate, 0 = reset
      bit 3-0  Fh = no drive, Eh/Dh/Bh/7h = drive 0/1/2/3

    Ports 3Ch-3Fh - FD1797 command/status, track, sector and data registers

    The board requires the CDR86 I/O decoder PROM.

    5.25" drives have no READY output, so in 5.25" mode READY comes from the head
    load timer instead.

    BIOS error codes (from page A1 of the FDC-880H software manual)

    Disk errors are reported as ERR DR SC TK TP CD: error code, drive, sector,
    track, disk type and command type. The error code is the FD1797 status:
      bit 7    drive not ready
      bit 6    write protect
      bit 5    record type (read) or write fault
      bit 4    seek error or record not found
      bit 3    CRC error
      bit 2    lost data
      bit 1    index or data request
      bit 0    busy
      FFh      no drive connected for the drive number, or no disk in the drive

    Disk type:
      bit 7    0 = double sided, 1 = single sided
      bit 6    0 = double density, 1 = single density
      bit 5    0 = 8", 1 = 5.25"
      bit 4-2  unused
      bit 1    5.25" format: 0 = normal, 1 = extended
      bit 0    sector size: 0 = normal, 1 = extended
      FFh      inactive drive
      FEh      active drive, non-standard disk

    Command type:
      0Ah  restore            12h  seek, no verify     1Eh  seek
      C4h  read address       88h  read sector         A8h  write sector
      F4h  write track        E4h  read track

****************************************************************************/

#include "emu.h"

#include "cdr_fdc_880h.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

#include "formats/flopimg.h"

#include <bit>

#define LOG_REG   (1U << 1)    // Shows register setup
#define LOG_LINES (1U << 2)    // Show control lines
#define LOG_DRIVE (1U << 3)    // Show drive select
#define LOG_ERR   (1U << 4)    // log errors
#define LOG_WAIT  (1U << 5)    // wait mode
#define LOG_DATA  (1U << 6)    // data read/writes
#define LOG_SETUP (1U << 7)    // setup

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGDRIVE(...)      LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGERR(...)        LOGMASKED(LOG_ERR, __VA_ARGS__)
#define LOGWAIT(...)       LOGMASKED(LOG_WAIT, __VA_ARGS__)
#define LOGDATA(...)       LOGMASKED(LOG_DATA, __VA_ARGS__)
#define LOGSETUP(...)      LOGMASKED(LOG_SETUP, __VA_ARGS__)

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

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	/// Bits set in cmd_ControlPort_c
	static constexpr u8 ctrl_EnableIntReq_c    = 7;
	static constexpr u8 ctrl_SetFMRecording_c  = 6;
	static constexpr u8 ctrl_DriveType_c       = 5;
	static constexpr u8 ctrl_Mode_c            = 4;

	static constexpr XTAL MASTER_CLOCK         = XTAL(4'000'000);
	static constexpr XTAL FIVE_IN_CLOCK        = MASTER_CLOCK / 4;
	static constexpr XTAL EIGHT_IN_CLOCK       = MASTER_CLOCK / 2;

	required_device<fd1797_device>             m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;

	bool m_installed;
	bool m_irq_allowed;

	bool m_irq;
	bool m_drq;

	bool m_double_density;
	bool m_five_in_drive;
	bool m_mode_operate;
	bool m_side;
	bool m_hld;
	s8   m_drive;

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

	void cmd_w(u8 val);
	void data_w(u8 val);
	u8 data_r();
	u8 status_r();

	void set_irq(int state);
	void set_drq(int state);
	void set_side(int state);
	void drive_ready_w(int state);
	void set_hld(int state);
	void update_intr();
	void update_fdc();
	void update_ready();

	// wait states are only generated when the interrupt/transfer mode is enabled
	bool should_assert_wait_state() const { return m_irq_allowed && !m_drq && !m_irq; }
};


cdr_fdc_880h_device::cdr_fdc_880h_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H89BUS_CDR_FDC_880H, tag, owner, clock)
	, device_h89bus_right_card_interface(mconfig, *this)
	, m_fdc(*this, "cdr_fdc_880h")
	, m_floppies(*this, "cdr_fdc_880h:%u", 0U)
{
}

void cdr_fdc_880h_device::cmd_w(u8 val)
{
	LOGREG("%s: val: 0x%02x\n", FUNCNAME, val);

	u8 const drive = BIT(val, 0, 4);

	// the lowest set select bit wins, none selected = -1
	m_drive = drive ? std::countr_zero(drive) : -1;

	m_mode_operate = bool(BIT(val, ctrl_Mode_c));
	m_five_in_drive = bool(BIT(val, ctrl_DriveType_c));
	m_double_density = !bool(BIT(val, ctrl_SetFMRecording_c));
	m_irq_allowed = bool(BIT(val, ctrl_EnableIntReq_c));

	LOGLINES("%s: intrq allowed: %d\n", FUNCNAME, m_irq_allowed);

	update_intr();

	LOGDRIVE("%s: floppydrive: %d, 5.25 in: %d\n", FUNCNAME, m_drive, m_five_in_drive);

	update_fdc();
}

void cdr_fdc_880h_device::update_fdc()
{
	m_fdc->dden_w(!m_double_density);

	m_fdc->set_floppy((m_drive >= 0) ? m_floppies[m_drive]->get_device() : nullptr);

	update_ready();

	// Clearing the mode bit holds the FD1797 in reset, which also forces
	// SSO low. Releasing it starts a restore on the selected drive, which the
	// ROM and BIOS cut short with a Force Interrupt.
	m_fdc->mr_w(m_mode_operate);

	for (auto &elem : m_floppies)
	{
		floppy_image_device *const floppy = elem->get_device();

		// 8" drive motors run continuously, they are started in device_reset()
		if (floppy && floppy->get_form_factor() != floppy_image::FF_8)
		{
			floppy->mon_w(m_drive == -1);
		}
	}

	m_fdc->set_clock(m_five_in_drive ? FIVE_IN_CLOCK : EIGHT_IN_CLOCK);
}

// The FD1797 READY input combines the 8" connector's READY line (J3-22) with
// the head load timer, which supplies READY in 5.25" mode since 5.25" drives
// have no READY output. A 5.25" drive selected in 8" mode is therefore not
// ready, which FORMAT9 relies on to check the drive type.
void cdr_fdc_880h_device::update_ready()
{
	floppy_image_device *const floppy = (m_drive >= 0) ? m_floppies[m_drive]->get_device() : nullptr;

	// in 5.25" mode, the head load timer output, approximated as following HLD with no delay
	bool const ready = m_five_in_drive
			? m_hld
			: (floppy && (floppy->get_form_factor() == floppy_image::FF_8) && !floppy->ready_r());

	m_fdc->ready_w(ready);
}

void cdr_fdc_880h_device::set_hld(int state)
{
	m_hld = bool(state);

	update_ready();
}

void cdr_fdc_880h_device::drive_ready_w(int state)
{
	// the selected drive's READY changed (spin up, disk change)
	update_ready();
}

void cdr_fdc_880h_device::data_w(u8 val)
{
	LOGDATA("%s: val: 0x%02x\n", FUNCNAME, val);

	if (should_assert_wait_state())
	{
		LOGWAIT("%s: wait state\n", FUNCNAME);

		set_slot_wait(ASSERT_LINE);
		return;
	}

	m_fdc->data_w(val);
}

void cdr_fdc_880h_device::write(offs_t offset, u8 data)
{
	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, data);

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			// write not supported on these addresses
			LOGERR("%s: Unexpected port write reg: %d\n", FUNCNAME, offset);
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
	// drive select bits are active low
	u8 data = (m_drive >= 0) ? (~(1 << m_drive) & 0x0f) : 0x0f;

	data |= m_mode_operate ? 0x10 : 0;
	data |= m_five_in_drive ? 0 : 0x20;
	data |= m_side ? 0 : 0x40;
	data |= m_irq ? 0 : 0x80;

	return data;
}

u8 cdr_fdc_880h_device::data_r()
{
	u8 data = 0;

	if (should_assert_wait_state())
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

	LOGDATA("%s: data: 0x%02x\n", FUNCNAME, data);

	return data;
}

u8 cdr_fdc_880h_device::read(offs_t offset)
{
	// undecoded ports float high
	u8 value = 0xff;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			// read not supported on these addresses
			LOGERR("%s: Unexpected port read reg: %d\n", FUNCNAME, offset);
			break;
		case 3: // controller status port
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
	m_installed = false;

	save_item(NAME(m_installed));
	save_item(NAME(m_irq_allowed));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
	save_item(NAME(m_double_density));
	save_item(NAME(m_five_in_drive));
	save_item(NAME(m_mode_operate));
	save_item(NAME(m_side));
	save_item(NAME(m_hld));
	save_item(NAME(m_drive));
}

void cdr_fdc_880h_device::device_reset()
{
	if (!m_installed)
	{
		h89bus::addr_ranges  addr_ranges = h89bus().get_address_ranges(h89bus::IO_CASS);

		if (addr_ranges.size() == 1)
		{
			h89bus::addr_range range = addr_ranges.front();

			h89bus().install_io_device(range.first, range.second,
				read8sm_delegate(*this, FUNC(cdr_fdc_880h_device::read)),
				write8sm_delegate(*this, FUNC(cdr_fdc_880h_device::write)));
		}
		else
		{
			LOGSETUP("%s: No address specified\n", FUNCNAME);
		}

		m_installed = true;
	}

	// control latch is cleared on reset
	m_irq_allowed    = false;
	m_irq            = false;
	m_drq            = false;
	m_double_density = true;
	m_five_in_drive  = false;
	m_mode_operate   = false;
	m_side           = false;
	m_hld            = false;
	m_drive          = -1;

	set_slot_int5(CLEAR_LINE);
	set_slot_wait(CLEAR_LINE);

	for (auto &elem : m_floppies)
	{
		floppy_image_device *const floppy = elem->get_device();
		if (floppy && floppy->get_form_factor() == floppy_image::FF_8)
		{
			// turn on motor of all installed 8" floppies
			floppy->mon_w(0);
		}
	}

	update_fdc();
}

void cdr_fdc_880h_device::device_post_load()
{
	// the FDC doesn't save its selected floppy, and device clocks aren't saved
	m_fdc->set_floppy((m_drive >= 0) ? m_floppies[m_drive]->get_device() : nullptr);
	m_fdc->set_clock(m_five_in_drive ? FIVE_IN_CLOCK : EIGHT_IN_CLOCK);
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
	m_fdc->sso_wr_callback().set(FUNC(cdr_fdc_880h_device::set_side));
	m_fdc->hld_wr_callback().set(FUNC(cdr_fdc_880h_device::set_hld));
	m_fdc->ready_wr_callback().set(FUNC(cdr_fdc_880h_device::drive_ready_w));
	m_fdc->set_external_ready(true);

	for (auto &floppy : m_floppies)
	{
		FLOPPY_CONNECTOR(config, floppy, cdr_floppies, "5_ds_qd", floppy_image_device::default_mfm_floppy_formats);
		floppy->enable_sound(true);
	}
}

void cdr_fdc_880h_device::set_irq(int state)
{
	LOGLINES("set irq, allowed: %d state: %d\n", m_irq_allowed, state);

	m_irq = bool(state);

	if (m_irq)
	{
		set_slot_wait(CLEAR_LINE);
	}

	update_intr();
}

void cdr_fdc_880h_device::set_drq(int state)
{
	LOGLINES("set drq, allowed: %d state: %d\n", m_irq_allowed, state);

	m_drq = bool(state);

	if (m_drq)
	{
		set_slot_wait(CLEAR_LINE);
	}

	update_intr();
}

void cdr_fdc_880h_device::set_side(int state)
{
	m_side = bool(state);

	// SSO is a single line on the drive cable, shared by all drives
	for (auto &elem : m_floppies)
	{
		floppy_image_device *const floppy = elem->get_device();
		if (floppy)
		{
			floppy->ss_w(state);
		}
	}
}

void cdr_fdc_880h_device::update_intr()
{
	// When interrupts are enabled, both INTRQ and DRQ interrupt the CPU.
	set_slot_int5(m_irq_allowed && (m_irq || m_drq));
}
}

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_CDR_FDC_880H, device_h89bus_right_card_interface, cdr_fdc_880h_device, "cdr_fdc_880h", "CDR FDC-880H Soft-sectored Controller");
