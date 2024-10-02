// license:BSD-3-Clause
// copyright-holders:68bit
/**********************************************************************

    SWTPC SS30 DC5 Floppy Disk Controller

    See: http://www.swtpc.com/mholley/DC_5/DC5_Index.htm

    The DC5 features an extra control register compared with the DC4. This
    extra register can be enabled or disabled via a jumper setting. This
    register is write only, and reading back gives a version number of 0x02.

    This register can be useful for setting the clock rate, and particular
    clock rates are needed for difference floppy disk sizes and formats. The
    clock rates are control by bits bits 2 and 3 as follows: 0x00 gives 1MHz,
    0x04 gives 1.2MHz, 0x80 gives 2MHz, and 0xC0 gives 2.4MHz.

    This register also controls the single versus double density format
    selection, via bit 5, so 0x00 gives double density and 0x20 gives single
    density.

    Software is typically not written for this DC5 extended control register,
    but typically does not touch it. Thus is can be practical to work with a
    purely single or double density disk by setting that format in this
    control register. Most virtual disks appear to fit this pattern, rather
    than mixing single and double density in the one disk.

**********************************************************************/

#include "emu.h"
#include "dc5.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "formats/cp68_dsk.h"
#include "formats/fdos_dsk.h"
#include "formats/flex_dsk.h"
#include "formats/os9_dsk.h"
#include "formats/uniflex_dsk.h"

class ss50_dc5_device : public device_t, public ss50_card_interface
{
public:
	ss50_dc5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_DC5, tag, owner, clock)
		, ss50_card_interface(mconfig, *this)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_interrupt_select(*this, "INTERRUPT_SELECT")
		, m_address_mode(*this, "ADDRESS_MODE")
		, m_two_control_regs(*this, "TWO_CONTROL_REGS")
		, m_force_ready(*this, "FORCE_READY")
		, m_ctrl_reg_bit7_side_select(*this, "CTRL_REG_BIT7_SIDE_SELECT")
		, m_expected_clock(*this, "EXPECTED_CLOCK")
		, m_expected_density(*this, "EXPECTED_DENSITY")
		, m_expected_sectors(*this, "EXPECTED_SECTORS")
		, m_track_zero_expected_sectors(*this, "TRACK_ZERO_EXPECTED_SECTORS")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(force_ready_change);
	DECLARE_INPUT_CHANGED_MEMBER(ctrl_reg_bit7_side_select_change);
	DECLARE_INPUT_CHANGED_MEMBER(expected_clock_change);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint8_t register_read(offs_t offset) override;
	virtual void register_write(offs_t offset, uint8_t data) override;

private:
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void fdc_sso_w(int state);

	static void floppy_formats(format_registration &fr);
	uint8_t m_fdc_status;              // for floppy controller
	uint8_t m_control_register;
	uint8_t m_motor_timer_out;
	emu_timer *m_floppy_motor_timer;
	floppy_image_device *m_fdc_selected_floppy; // Current selected floppy.
	uint8_t m_fdc_side;            // Current floppy side.
	uint8_t m_fdc_clock_div;       // Current clock frequency divisor.
	uint8_t m_fdc_prog_clock_div;      // Programmed clock frequency divisor.

	TIMER_CALLBACK_MEMBER(floppy_motor_callback);

	void floppy_motor_trigger();
	void control_register_update();
	void validate_floppy_side(uint8_t cmd);
	uint8_t validate_fdc_dden(uint8_t dden);
	uint8_t validate_fdc_sso(uint8_t cmd);
	uint8_t validate_fdc_sector_size(uint8_t cmd);

	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_ioport m_interrupt_select;
	required_ioport m_address_mode;
	required_ioport m_two_control_regs;
	required_ioport m_force_ready;
	required_ioport m_ctrl_reg_bit7_side_select;
	required_ioport m_expected_clock;
	required_ioport m_expected_density;
	required_ioport m_expected_sectors;
	required_ioport m_track_zero_expected_sectors;
};


static INPUT_PORTS_START( dc5 )
	PORT_START("INTERRUPT_SELECT")
	PORT_DIPNAME(0x3, 0, "Interrupt select")
	PORT_DIPSETTING(0, "N/C")
	PORT_DIPSETTING(1, "IRQ")
	PORT_DIPSETTING(2, "NMI/FIRQ")

	PORT_START("ADDRESS_MODE")
	PORT_DIPNAME(0x1, 1, "Address mode")
	PORT_DIPSETTING(0, "4 address")
	PORT_DIPSETTING(1, "16 address")

	PORT_START("TWO_CONTROL_REGS")
	PORT_DIPNAME(0x1, 0, "Two control registers")
	PORT_DIPSETTING(0, "No, DC4 compatible")
	PORT_DIPSETTING(1, "Yes, DC5 extension")

	// The DC5 has two modes for controlling the FDC 'ready' input. One
	// mode forces the 'ready' line for a period triggered by the chip
	// select, along with the motors. The other mode detects index pulses
	// from the disk to control the 'ready' line. Flex2 for the 6800
	// generally needs the 'ready' line forced and Flex9 can use the index
	// pulse detection to determine if disks are present.
	PORT_START("FORCE_READY")
	PORT_CONFNAME(0x1, 0, "Force ready") PORT_CHANGED_MEMBER(DEVICE_SELF, ss50_dc5_device, force_ready_change, 0)
	PORT_CONFSETTING(0, DEF_STR(No))
	PORT_CONFSETTING(1, DEF_STR(Yes))

	// This config setting allows checking of the FDC clock rate and
	// overriding it to assist driver development. The DC5 supports
	// programming of the clock rate via the second control register, and
	// that setting can be validate using this setting. This setting also
	// sets overrides the initial clock rate which is otherwise 1MHz.
	//
	// Some common rates:
	// 5.25" single or double density  -  1.0MHz
	// 3.5" 'standard'  -  1.2MHz
	// 3.5" HD  -  2.0MHz
	// 8" 360rpm  -  2.4MHz
	PORT_START("EXPECTED_CLOCK")
	PORT_CONFNAME(0xf, 0, "FDC expected clock rate") PORT_CHANGED_MEMBER(DEVICE_SELF, ss50_dc5_device, expected_clock_change, 0)
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(12, "1.0 MHz")
	PORT_CONFSETTING(10, "1.2 MHz")
	PORT_CONFSETTING(6, "2.0 MHz")
	PORT_CONFSETTING(5, "2.4 MHz")

	// It is not uncommon to see drivers using bit 7 of the control
	// register as a side selection. This conflicts with the DC5, and
	// earlier controllers in this series, which use bit 7 to inhibit
	// drive selection. These drivers need to be corrected, but this
	// option helps identify this issue and work around it.
	PORT_START("CTRL_REG_BIT7_SIDE_SELECT")
	PORT_CONFNAME(0x1, 0, "Control register bit 7") PORT_CHANGED_MEMBER(DEVICE_SELF, ss50_dc5_device, ctrl_reg_bit7_side_select_change, 0)
	PORT_CONFSETTING(0, "Inhibits drive selection")
	PORT_CONFSETTING(1, "Erroneous side select")

	// Debug aid to hard code the density. The OS9 format can use single
	// density for track zero head zero. The FLEX format can use single
	// density for track zero on all heads, and many virtual disks 'fix'
	// the format to be purely double density and often without properly
	// implementing driver support for that. This setting is an aid to
	// report unexpected usage, and it attempts to correct that.
	PORT_START("EXPECTED_DENSITY")
	PORT_CONFNAME(0x7, 0, "Expected density")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(1, "single density") // Purely single density.
	PORT_CONFSETTING(2, "double density, with single density track zero head zero") // OS9 format
	PORT_CONFSETTING(3, "double density, with single density track zero all heads") // The default FLEX double density format.
	PORT_CONFSETTING(4, "double density") // Purely double density.

	// Debug aid, to check that the sector head or side is set as expected
	// for the sector ID being read for the FLEX floppy disk format. Many
	// FLEX disk images were developed for vitural machines that have
	// little regard for the actual number of heads and can work off the
	// sector ID and their drivers can set the head incorrectly. E.g. a
	// disk with 18 sectors per side might have a driver incorrectly switch
	// heads for sectors above 10. Another issue is that double density
	// disk images are often 'fixed' so that they are pure double density
	// without being single density on the first track, and the drivers
	// might still set the head based on track zero being single
	// density. This aid is not intended to be a substitute for fixing the
	// drivers but it does help work through the issues while patching the
	// disks.
	PORT_START("EXPECTED_SECTORS")
	PORT_CONFNAME(0xff, 0, "FLEX expected sectors per side")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(10, "10") // 5 1/4" single density 256B
	PORT_CONFSETTING(15, "15") // 8" single density 256B
	PORT_CONFSETTING(16, "16") // 5 1/4" double density 256B
	PORT_CONFSETTING(18, "18") // 5 1/4" double density 256B
	PORT_CONFSETTING(26, "26") // 8" double density 256B
	PORT_CONFSETTING(36, "36") // 3.5" 1.4M QD 256B
	// The track zero expected sectors if different from the above. FLEX
	// 6800 disks did format track zero in single density and if the
	// driver sticks to that and if using a double density disk then set a
	// single density size here.
	PORT_START("TRACK_ZERO_EXPECTED_SECTORS")
	PORT_CONFNAME(0xff, 0, "FLEX track zero expected sectors per side")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(10, "10") // 5 1/4" single density 256B
	PORT_CONFSETTING(15, "15") // 8" single density 256B
	PORT_CONFSETTING(16, "16") // 5 1/4" double density 256B
	PORT_CONFSETTING(18, "18") // 5 1/4" double density 256B
	PORT_CONFSETTING(26, "26") // 8" double density 256B
	PORT_CONFSETTING(36, "36") // 3.5" 1.4M QD 256B
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ss50_dc5_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dc5);
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void ss50_dc5_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_FLEX_FORMAT);
	fr.add(FLOPPY_CP68_FORMAT);
	fr.add(FLOPPY_FDOS_FORMAT);
	fr.add(FLOPPY_OS9_FORMAT);
	fr.add(FLOPPY_UNIFLEX_FORMAT);
}

static void flex_floppies(device_slot_interface &device)
{
	device.option_add("sssd35", FLOPPY_525_SSSD_35T); // 35 trks ss sd 5.25
	device.option_add("sssd",   FLOPPY_525_SSSD);     // 40 trks ss sd 5.25
	device.option_add("dssd35", FLOPPY_525_SD_35T);   // 35 trks ds sd 5.25
	device.option_add("dssd",   FLOPPY_525_SD);       // 40 trks ds sd 5.25
	device.option_add("ssdd",   FLOPPY_525_SSDD);     // 40 trks ss dd 5.25
	device.option_add("dd",     FLOPPY_525_DD);       // 40 trks ds dd 5.25
	device.option_add("ssqd",   FLOPPY_525_SSQD);     // 80 trks ss dd 5.25
	device.option_add("qd",     FLOPPY_525_QD);       // 80 trks ds dd 5.25
	device.option_add("8sssd",  FLOPPY_8_SSSD);       // 77 trks ss sd 8"
	device.option_add("8dssd",  FLOPPY_8_DSSD);       // 77 trks ds sd 8"
	device.option_add("8ssdd",  FLOPPY_8_SSDD);       // 77 trks ss dd 8"
	device.option_add("8dsdd",  FLOPPY_8_DSDD);       // 77 trks ds dd 8"
	device.option_add("35hd",   FLOPPY_35_HD);        // 1.44mb disk from swtpc emu (emulator only?)
}

void ss50_dc5_device::device_add_mconfig(machine_config &config)
{
	WD2797(config, m_fdc, 12_MHz_XTAL / 12); // divider is selectable
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, flex_floppies, "sssd35", ss50_dc5_device::floppy_formats).enable_sound(true);

	m_fdc->intrq_wr_callback().set(FUNC(ss50_dc5_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(ss50_dc5_device::fdc_drq_w));
	m_fdc->sso_wr_callback().set(FUNC(ss50_dc5_device::fdc_sso_w));
}

void ss50_dc5_device::device_reset()
{
	// Initialize to the expected rate if any, otherwise to 1MHz.
	m_fdc_prog_clock_div = 12;
	uint8_t expected_clock_div = m_expected_clock->read();
	uint8_t clock_div = expected_clock_div ? expected_clock_div : m_fdc_prog_clock_div;
	m_fdc->set_unscaled_clock(12_MHz_XTAL / clock_div);
	m_fdc_clock_div = clock_div;

	// The reset state of DDEN for the DC5 is one, so selecting single density.
	m_fdc->dden_w(1);
}

void ss50_dc5_device::device_start()
{
	m_fdc_status = 0;
	m_control_register = 0;
	m_fdc_side = 0;
	m_floppy_motor_timer = timer_alloc(FUNC(ss50_dc5_device::floppy_motor_callback), this);
	m_motor_timer_out = 0;
	m_fdc->set_force_ready(0);
	m_fdc_prog_clock_div = 12;

	save_item(NAME(m_fdc_status));
	save_item(NAME(m_control_register));
	save_item(NAME(m_motor_timer_out));
	save_item(NAME(m_fdc_side));
	save_item(NAME(m_fdc_clock_div));
	save_item(NAME(m_fdc_prog_clock_div));
}


INPUT_CHANGED_MEMBER(ss50_dc5_device::force_ready_change)
{
	control_register_update();
}

INPUT_CHANGED_MEMBER(ss50_dc5_device::ctrl_reg_bit7_side_select_change)
{
	control_register_update();
}

INPUT_CHANGED_MEMBER(ss50_dc5_device::expected_clock_change)
{
	uint8_t clock_div = newval ? newval : m_fdc_prog_clock_div;
	m_fdc->set_unscaled_clock(12_MHz_XTAL / clock_div);
	m_fdc_clock_div = clock_div;
}

/* Shared floppy support. */

void ss50_dc5_device::floppy_motor_trigger()
{
	m_floppy_motor_timer->adjust(attotime::from_msec(30000));
	if (!m_motor_timer_out)
	{
		m_motor_timer_out = 1;
		control_register_update();
	}
}

TIMER_CALLBACK_MEMBER(ss50_dc5_device::floppy_motor_callback)
{
	if (m_motor_timer_out)
	{
		m_motor_timer_out = 0;
		control_register_update();
	}
}

// On a FDC command write, check that the floppy side is as expected given
// the track and sector. This check is performed for the type II and III
// commands. The floppy side is modified if necessary.
void ss50_dc5_device::validate_floppy_side(uint8_t cmd)
{
	if ((cmd & 0xe1) == 0x80 || (cmd & 0xe0) == 0xa0 ||
		(cmd & 0xf9) == 0xc0 || (cmd & 0xf9) == 0xe0 ||
		(cmd & 0xf9) == 0xf0)
	{
		uint32_t expected_sectors = m_expected_sectors->read();
		uint32_t track_zero_expected_sectors = m_track_zero_expected_sectors->read();
		uint8_t sector = m_fdc->sector_r();
		uint8_t track = m_fdc->track_r();

		if (track_zero_expected_sectors && track == 0)
		{
			uint8_t expected_side = sector > track_zero_expected_sectors ? 1 : 0;

			if (m_fdc_side != expected_side)
			{
				logerror("%s Unexpected side %d for track %d sector %d expected side %d\n", machine().describe_context(), m_fdc_side, track, sector, expected_side);
				if (m_fdc_selected_floppy)
				{
					m_fdc_selected_floppy->ss_w(expected_side);
					m_fdc_side = expected_side;
				}
			}
		}
		else if (expected_sectors)
		{
			uint8_t expected_side = sector > expected_sectors ? 1 : 0;

			if (m_fdc_side != expected_side)
			{
				logerror("%s Unexpected side %d for track %d sector %d expected side %d\n", machine().describe_context(), m_fdc_side, track, sector, expected_side);
				if (m_fdc_selected_floppy)
				{
					m_fdc_selected_floppy->ss_w(expected_side);
					m_fdc_side = expected_side;
				}
			}
		}
	}
}

// Note the dden line is low for double density.
uint8_t ss50_dc5_device::validate_fdc_dden(uint8_t dden)
{
	uint8_t expected_density = m_expected_density->read();
	switch (expected_density)
	{
		case 1:
			// Single density.
			if (!dden)
				logerror("%s Unexpected DDEN %d for single density\n", machine().describe_context(), dden);
			return 1;
		case 2:
		{
			// Double density with track zero head zero single density.
			uint8_t track = m_fdc->track_r();

			if (track == 0 && m_fdc_side == 0)
			{
				// If this path is called on an update of the
				// second control register then the track need
				// not be accurate so this message might not
				// be accurate.
				if (!dden)
					logerror("%s Unexpected DDEN %d for single density track 0 head 0\n", machine().describe_context(), dden);
				return 1;
			}
			if (dden)
				logerror("%s Unexpected DDEN %d for double density\n", machine().describe_context(), dden);
			return 0;
		}
		case 3:
		{
			// Double density with track zero all heads single density.
			uint8_t track = m_fdc->track_r();

			if (track == 0)
			{
				// If this path is called on an update of the
				// second control register then the track need
				// not be accurate so this message might not
				// be accurate.
				if (!dden)
					logerror("%s Unexpected DDEN %d for single density track 0\n", machine().describe_context(), dden);
				return 1;
			}
			if (dden)
				logerror("%s Unexpected DDEN %d for double density\n", machine().describe_context(), dden);
			return 0;
		}
		case 4:
			// Pure double density.
			if (dden)
				logerror("%s Unexpected DDEN %d for double density\n", machine().describe_context(), dden);
			return 0;
		default:
			return dden;
	}
}

uint8_t ss50_dc5_device::validate_fdc_sso(uint8_t cmd)
{
	// The DC4 and DC5 invert the SSO output and wire it to the DDEN input
	// to allow selection of single or double density.

	// In the DC5 two-register mode the SSO pin does not control DDEN.
	uint32_t two_regs = m_two_control_regs->read();
	if (two_regs)
	{
		// If drivers are attempting to use SSO to control the
		// density then that might need correcting so report such
		// usage.
		logerror("%s Unexpected SSO flag set in two control registers mode\n", machine().describe_context());
		return cmd;
	}

	// The SSO is only loaded for type II and III commands.
	if ((cmd & 0xe1) == 0x80 ||
		(cmd & 0xe0) == 0xa0 ||
		(cmd & 0xf9) == 0xc0 ||
		(cmd & 0xf9) == 0xe0 ||
		(cmd & 0xf9) == 0xf0)
	{
		// The SSO output is inverted and input to DDEN.
		uint8_t dden = !BIT(cmd, 1);
		uint8_t sso = !validate_fdc_dden(dden);
		cmd = (cmd & 0xfd) | (sso << 1);
	}

	return cmd;
}

// The WD2797 supports an alternate interpretation of the sector size. Check
// that the flag is as expected for FLEX and return the corrected command if
// necessary.
uint8_t ss50_dc5_device::validate_fdc_sector_size(uint8_t cmd)
{
	if ((cmd & 0xe1) == 0x80 || (cmd & 0xe0) == 0xa0)
	{
		// Check that the sector length flag is set as expected.
		uint8_t sector_length_default = cmd & 0x08;
		if (sector_length_default != 0x08)
		{
			logerror("%s Unexpected sector length default %02x\n", machine().describe_context(), sector_length_default);
			// Patch the sector length flag.
			cmd |= 0x08;
		}
	}
	return cmd;
}

void ss50_dc5_device::fdc_intrq_w(int state)
{
	if (state)
		m_fdc_status |= 0x40;
	else
		m_fdc_status &= ~0x40;

	uint8_t selection = m_interrupt_select->read();

	if (selection == 1)
		write_irq(state);
	else if (selection == 2)
		write_firq(state);
}

void ss50_dc5_device::fdc_drq_w(int state)
{
	if (state)
		m_fdc_status |= 0x80;
	else
		m_fdc_status &= 0x7f;
}

void ss50_dc5_device::fdc_sso_w(int state)
{
	// The DC4 and DC5 invert the SSO output and wire it to the DDEN input
	// to allow selection of single or double density.

	// A DC5 extension. In two-register mode the SSO pin does not control
	// DDEN.
	uint32_t two_regs = m_two_control_regs->read();
	if (two_regs)
		return;

	// However there are a lot of boot loaders that will
	// not work properly with this, so allow other options.
	uint8_t expected_density = m_expected_density->read();
	switch (expected_density)
	{
		case 1:
			// Single density.
			if (state)
				logerror("Unexpected SSO %d for single density\n", state);
			m_fdc->dden_w(1);
			break;
		case 2:
		{
			// Double density with track zero head zero single density.
			uint8_t track = m_fdc->track_r();

			if (track == 0 && m_fdc_side == 0)
			{
				if (state)
					logerror("Unexpected SSO %d for single density track 0 head 0\n", state);
				m_fdc->dden_w(1);
			}
			else
			{
				if (!state)
					logerror("Unexpected SSO %d for double density\n", state);
				m_fdc->dden_w(0);
			}
			break;
		}
		case 3:
		{
			// Double density with track zero all heads single density.
			uint8_t track = m_fdc->track_r();

			if (track == 0)
			{
				if (state)
					logerror("Unexpected SSO %d for single density track 0\n", state);
				m_fdc->dden_w(1);
			}
			else
			{
				if (!state)
					logerror("Unexpected SSO %d for double density\n", state);
				m_fdc->dden_w(0);
			}
			break;
		}
		case 4:
			// Pure double density.
			if (!state)
				logerror("Unexpected SSO %d for double density\n", state);
			m_fdc->dden_w(0);
			break;
		default:
			m_fdc->dden_w(!state);
			break;
	}
}

// Called when state affected by the control register changes, when the
// control register is modified, or when the motor starts or stops.
void ss50_dc5_device::control_register_update()
{
	// The floppy drives do not gate the motor based on the select input,
	// so the motors run, or not, irrespective of the drive selection.
	uint8_t motor = m_motor_timer_out ? CLEAR_LINE : ASSERT_LINE;
	for (auto &floppy : m_floppy)
	{
		floppy_image_device *fd = floppy->get_device();
		if (fd)
			fd->mon_w(motor);
	}

	// The DC2, DC3, DC4, DC5 have a drive inhibit feature controlled by
	// bit 7, and the drives are de-selected if the motor timer has timed
	// out.
	uint8_t inhibit = BIT(m_control_register, 7);

	// However it is common to see drivers using bit 7 for side selection,
	// so offer any alternative interpretation as a work around.
	uint8_t bit7_side_select = m_ctrl_reg_bit7_side_select->read();

	floppy_image_device *floppy = nullptr;

	if ((bit7_side_select || !inhibit) && m_motor_timer_out)
		floppy = m_floppy[m_control_register & 3]->get_device();

	if (floppy != m_fdc_selected_floppy)
	{
		m_fdc->set_floppy(floppy);
		m_fdc_selected_floppy = floppy;
	}

	if (floppy)
	{
		// The DC3, DC4, DC5 have a side select feature that is
		// controlled by bit 6. The DC1 and DC2 do not have a floppy
		// side output so did not support double sided disks.
		uint8_t side = BIT(m_control_register, 6);

		if (bit7_side_select)
		{
			uint8_t bit7 = BIT(m_control_register, 7);
			if (bit7)
				logerror("%s Unexpected use of DC5 control register bit 7 for side selection.\n", machine().describe_context());
			// OR together the side selection bits. This appears
			// the most useful choice because drivers in error are
			// most likely just writing to the wrong bit rather
			// than inconsistently to both bits. This allows
			// incremental correction of drivers while spotting
			// remaining uses of bit 7 for side selection.
			side |= bit7;
		}
		floppy->ss_w(side);
		m_fdc_side = side;
	}

	// Force the FDC 'ready' line on if the motor line is asserted, but
	// only when this forced mode is enabled.
	m_fdc->set_force_ready(m_motor_timer_out && m_force_ready->read());
}


//-------------------------------------------------
//  register_read - read from a port register
//-------------------------------------------------

uint8_t ss50_dc5_device::register_read(offs_t offset)
{
	uint8_t address_mode = m_address_mode->read();

	// If the address mode is 16 bytes then subtract 4 and handle it as a
	// 4 byte address base.
	if (address_mode == 1) {
		if (offset < 4)
			return 0;
		offset -= 4;
	}

	if (offset > 7)
		return 0;

	if ((offset & 0x4) == 0x0)
	{
		uint32_t two_regs = m_two_control_regs->read();

		// Note: access to this control register does not trigger the
		// motor on and does not reset the motor timer.

		if (!two_regs || (offset & 2) == 0)
		{
			// Control register 8014 / e014 read.
			return m_fdc_status;
		}
		else
		{
			// DC5 extension, reads a version number.
			return 0x02;
		}
	}
	else
	{
		// FDC 8018-801b / e018-e01b
		offset &= 3;
		// Access to the FDC triggers the motor timer.
		if (!machine().side_effects_disabled())
			floppy_motor_trigger();

		uint8_t data = m_fdc->wd2797_device::read(offset);
		return data;
	}
}

//-------------------------------------------------
//  register_write - write to a port register
//-------------------------------------------------

void ss50_dc5_device::register_write(offs_t offset, uint8_t data)
{
	uint8_t address_mode = m_address_mode->read();

	// If the address mode is 16 bytes then subtract 4 and handle it as a
	// 4 byte address base.
	if (address_mode == 1) {
		if (offset < 4)
			return;
		offset -= 4;
	}

	if (offset > 7)
		return;

	if ((offset & 0x4) == 0x0)
	{
		// Control register(s) 8014-8017 E014-E017
		uint32_t two_regs = m_two_control_regs->read();

		// Note: access to this control register does not trigger the
		// motor on and and does not reset the motor timer.

		if (!two_regs || (offset & 2) == 0)
		{
			// Control register 8014 / e014 write.
			m_control_register = data;
			control_register_update();
		}

		if (two_regs && (offset & 2) == 2)
		{
			// DC5 extended control register 8016 / E016.

			// Osc20 and osc12 control a clock divider.
			uint8_t osc12 = BIT(data, 2);
			uint8_t osc20 = BIT(data, 3);
			uint8_t clock_div = 12;

			// Note: supporting 2.4MHz is here is an extension to
			// the published DC5 design, and appears necessary to
			// support 360 rpm 8" drives. The case in which osc20
			// and osc12 are high generates 2.0MHz in the
			// published design, and that is redundant and is used
			// here to generate 2.4MHz. The DC5 is an open design
			// using a PLD and this variation can be programmed
			// into the DC5 hardware. The 2.4MHz rate appears to
			// be outside the limit for the WD2797 which appears
			// to be 2.173MHz. TODO explore this further; were the
			// 8" drives run at 300rpm; is there an error in the
			// rates elsewhere?
			if (osc20 && osc12)
				clock_div = 5;
			else if (osc20)
				clock_div = 6;
			else if (osc12)
				clock_div = 10;

			// Note the programmed clock divisor, in case the
			// expected changes.
			m_fdc_prog_clock_div = clock_div;

			// Compare to the expected clock divisor if any.
			uint32_t expected_clock_div = m_expected_clock->read();
			if (expected_clock_div && clock_div != expected_clock_div)
			{
				logerror("%s Unexpected clock rate of %dHz expected %dHz.\n", machine().describe_context(), 12'000'000 / clock_div, 12'000'000 / expected_clock_div);
				clock_div = expected_clock_div;
			}

			if (clock_div != m_fdc_clock_div)
			{
				m_fdc->set_unscaled_clock(12_MHz_XTAL / clock_div);
				m_fdc_clock_div = clock_div;
			}

#if 0
			// Connects to the WD2979 5/8 pin.
			// TODO is this needed for the emulator?
			uint8_t five_or_eight = BIT(data, 4);
#endif

			// In this two-register mode the FDC SSO output pin
			// does not control DDEN. The bit 5 value is inverted
			// and input to DDEN.
			uint8_t dden = !BIT(data, 5);
			dden = validate_fdc_dden(dden);
			m_fdc->dden_w(dden);
		}
	}
	else
	{
		// FDC 8018-801b / e018-e01b
		offset &= 3;
		// Access to the FDC triggers the motor timer.
		floppy_motor_trigger();

		if (offset == 0) {
			validate_floppy_side(data);
			data = validate_fdc_sector_size(data);
			data = validate_fdc_sso(data);
		}

		m_fdc->wd2797_device::write(offset, data);
	}
}

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(SS50_DC5, ss50_card_interface, ss50_dc5_device, "ss50_dc5", "DC5 Floppy Disk Controller")
