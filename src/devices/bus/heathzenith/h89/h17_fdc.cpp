// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-17 Floppy controller

    This was an option for both the Heathkit H8 and H89 computer systems.

  TODO
    - define hard-sectored disk format
    - incoming floppy data should drive receive clock of the ami 2350.

****************************************************************************/

#include "emu.h"
#include "h17_fdc.h"

#include "imagedev/floppy.h"
#include "machine/s2350.h"


#define LOG_REG   (1U << 1) // Register setup
#define LOG_LINES (1U << 2) // Control lines
#define LOG_DRIVE (1U << 3) // Drive select
#define LOG_FUNC  (1U << 4) // Function calls

//#define VERBOSE (LOG_GENERAL | LOG_REG | LOG_LINES | LOG_DRIVE | LOG_FUNC)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGDRIVE(...)      LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

class heath_h17_fdc_device : public device_t, public device_h89bus_right_card_interface
{
public:
	heath_h17_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto floppy_ram_wp_cb() { return m_floppy_ram_wp.bind(); }

	virtual void write(u8 select_lines, u8 offset, u8 data) override;
	virtual u8 read(u8 select_lines, u8 offset) override;

	[[maybe_unused]] void side_select_w(int state);

protected:
	static constexpr u8 MAX_FLOPPY_DRIVES = 3;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void ctrl_w(u8 val);
	u8   floppy_status_r();

	void set_floppy(floppy_image_device *floppy);
	void step_w(int state);
	void dir_w(int state);
	void set_motor(bool motor_on);

	void sync_character_received(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(tx_timer_cb);

	devcb_write_line m_floppy_ram_wp;

	required_device<s2350_device> m_s2350;
	required_device_array<floppy_connector, MAX_FLOPPY_DRIVES> m_floppies;
	required_device<timer_device> m_tx_timer;

	bool m_motor_on;
	bool m_write_gate;
	bool m_sync_char_received;
	u8   m_step_direction;
	u8   m_side;

	floppy_image_device *m_floppy;

	/// write bit control port
	static constexpr u8 CTRL_WRITE_GATE       = 0;
	static constexpr u8 CTRL_DRIVE_SELECT_0   = 1;
	static constexpr u8 CTRL_DRIVE_SELECT_1   = 2;
	static constexpr u8 CTRL_DRIVE_SELECT_2   = 3;
	static constexpr u8 CTRL_MOTOR_ON         = 4; // Controls all the drives
	static constexpr u8 CTRL_DIRECTION        = 5; // (0 = out)
	static constexpr u8 CTRL_STEP_COMMAND     = 6; // (Active high)
	static constexpr u8 CTRL_WRITE_ENABLE_RAM = 7; // 0 - write protected

	// USRT clock
	static constexpr XTAL USRT_BASE_CLOCK = XTAL(12'288'000) / 6 / 16;
	static constexpr u32  USRT_TX_CLOCK   = USRT_BASE_CLOCK.value();
};


heath_h17_fdc_device::heath_h17_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_H_17_FDC, tag, owner, 0),
	device_h89bus_right_card_interface(mconfig, *this),
	m_floppy_ram_wp(*this),
	m_s2350(*this, "s2350"),
	m_floppies(*this, "floppy%u", 0U),
	m_tx_timer(*this, "tx_timer"),
	m_floppy(nullptr)
{
}

void heath_h17_fdc_device::write(u8 select_lines, u8 offset, u8 data)
{
	if (!(select_lines & h89bus_device::H89_FLPY))
	{
		return;
	}

	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, data);

	switch (offset)
	{
	case 0: // data port
		m_s2350->transmitter_holding_reg_w(data);
		break;
	case 1: // fill character
		m_s2350->transmit_fill_reg_w(data);
		break;
	case 2: // sync port
		m_s2350->receiver_sync_reg_w(data);
		break;
	case 3: // control port
		ctrl_w(data);
		break;
	}
}

void heath_h17_fdc_device::set_floppy(floppy_image_device *floppy)
{
	if (m_floppy == floppy)
	{
		return;
	}

	LOGDRIVE("%s: selecting new drive\n", FUNCNAME);

	m_floppy = floppy;

	// set any latched signals
	{
		m_floppy->ss_w(m_side);
	}
}

void heath_h17_fdc_device::side_select_w(int state)
{
	m_side = BIT(state, 0);

	if (m_floppy)
	{
		m_floppy->ss_w(m_side);
	}
}

void heath_h17_fdc_device::dir_w(int state)
{
	if (m_floppy)
	{
		LOGFUNC("%s: step dir: 0x%02x\n", FUNCNAME, state);

		m_floppy->dir_w(state);
	}
}

void heath_h17_fdc_device::step_w(int state)
{
	if (m_floppy)
	{
		LOGFUNC("%s: step dir: 0x%02x\n", FUNCNAME, state);

		m_floppy->stp_w(state);
	}
}

void heath_h17_fdc_device::set_motor(bool motor_on)
{
	if (m_motor_on == motor_on)
	{
		return;
	}

	m_motor_on = motor_on;

	for (auto &elem : m_floppies)
	{
		floppy_image_device *floppy = elem->get_device();
		if (floppy)
		{
			LOGFUNC("%s: motor: %d\n", FUNCNAME, motor_on);

			floppy->mon_w(!motor_on);
		}
	}
}

void heath_h17_fdc_device::ctrl_w(u8 val)
{
	m_write_gate = bool(BIT(val, CTRL_WRITE_GATE));

	set_motor(bool(BIT(val, CTRL_MOTOR_ON)));

	if (BIT(val, CTRL_DRIVE_SELECT_0))
	{
		LOGFUNC("%s: set drive 0\n", FUNCNAME);

		set_floppy(m_floppies[0]->get_device());
	}
	else if (BIT(val, CTRL_DRIVE_SELECT_1))
	{
		LOGFUNC("%s: set drive 1\n", FUNCNAME);

		set_floppy(m_floppies[1]->get_device());
	}
	else if (BIT(val, CTRL_DRIVE_SELECT_2))
	{
		LOGFUNC("%s: set drive 2\n", FUNCNAME);

		set_floppy(m_floppies[2]->get_device());
	}
	else
	{
		LOGFUNC("%s: set drive none\n", FUNCNAME);

		set_floppy(nullptr);
	}

	dir_w(!BIT(val, CTRL_DIRECTION));

	step_w(!BIT(val, CTRL_STEP_COMMAND));

	m_floppy_ram_wp(BIT(val, CTRL_WRITE_ENABLE_RAM));
}

u8 heath_h17_fdc_device::read(u8 select_lines, u8 offset)
{
	if (!(select_lines & h89bus_device::H89_FLPY))
	{
		return 0;
	}

	u8 val = 0;

	switch (offset)
	{
	case 0: // data port
		val = m_s2350->receiver_output_reg_r();
		break;
	case 1: // status port
		val = m_s2350->status_word_r();
		break;
	case 2: // sync port
		val = m_s2350->receiver_sync_search();
		break;
	case 3: // floppy status port
		val = floppy_status_r();
		break;
	}

	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, val);

	return val;
}

u8 heath_h17_fdc_device::floppy_status_r()
{
	u8 val = 0;

	// statuses from the floppy drive
	if (m_floppy)
	{
		// index/sector hole
		val |= m_floppy->idx_r() ? 0x00 : 0x01;

		// track 0
		val |= m_floppy->trk00_r() ? 0x00 : 0x02;

		// disk is write-protected
		val |= m_floppy->wpt_r() ? 0x00 : 0x04;
	}
	else
	{
		LOGREG("%s: no drive selected\n", FUNCNAME);
	}

	// status from USRT
	val |= m_sync_char_received ? 0x08 : 0x00;

	LOGFUNC("%s: val: 0x%02x\n", FUNCNAME, val);

	return val;
}

void heath_h17_fdc_device::device_start()
{
	save_item(NAME(m_motor_on));
	save_item(NAME(m_write_gate));
	save_item(NAME(m_sync_char_received));
	save_item(NAME(m_step_direction));
	save_item(NAME(m_side));
}

void heath_h17_fdc_device::device_reset()
{
	LOGFUNC("%s\n", FUNCNAME);

	m_motor_on           = false;
	m_write_gate         = false;
	m_sync_char_received = false;

	m_tx_timer->adjust(attotime::from_hz(USRT_TX_CLOCK), 0, attotime::from_hz(USRT_TX_CLOCK));
}

static void h17_floppies(device_slot_interface &device)
{
	// H-17-1
	device.option_add("ssdd", FLOPPY_525_SSDD);

	// Future plans - test and verify higher capacity drives with LLC's BIOS-80 for CP/M and an HUG's enhanced HDOS driver
	//  - FLOPPY_525_SSQD
	//  - FLOPPY_525_DD
	//  - FLOPPY_525_QD (H-17-4)
}

TIMER_DEVICE_CALLBACK_MEMBER(heath_h17_fdc_device::tx_timer_cb)
{
	m_s2350->tcp_w();
}

void heath_h17_fdc_device::device_add_mconfig(machine_config &config)
{
	S2350(config, m_s2350, 0);
	m_s2350->sync_character_received_cb().set(FUNC(heath_h17_fdc_device::sync_character_received));

	for (int i = 0; i < MAX_FLOPPY_DRIVES; i++)
	{
		// TODO -> add (and define) heath hard-sectored floppy formats.
		FLOPPY_CONNECTOR(config, m_floppies[i], h17_floppies, "ssdd", floppy_image_device::default_fm_floppy_formats);
		m_floppies[i]->enable_sound(true);
	}

	TIMER(config, m_tx_timer).configure_generic(FUNC(heath_h17_fdc_device::tx_timer_cb));
}

void heath_h17_fdc_device::sync_character_received(int state)
{
	LOGFUNC("%s: state: %d\n", FUNCNAME, state);

	m_sync_char_received = bool(!BIT(state, 0));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_H_17_FDC, device_h89bus_right_card_interface, heath_h17_fdc_device, "h89_h17_fdc", "Heath H-17 Hard-sectored Controller (H-88-1)");
