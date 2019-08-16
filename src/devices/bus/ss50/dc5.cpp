// license:BSD-3-Clause
// copyright-holders:68bit
/**********************************************************************

    SWTPC SS30 DC5 Floppy Disk Controller

    See: http://www.swtpc.com/mholley/DC_5/DC5_Index.htm

**********************************************************************/

#include "emu.h"
#include "dc5.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "formats/flex_dsk.h"

class ss50_dc5_device : public device_t, public ss50_card_interface
{
public:
	ss50_dc5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_DC5, tag, owner, clock)
		, ss50_card_interface(mconfig, *this)
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy2(*this, "fdc:2")
		, m_floppy3(*this, "fdc:3")
		, m_clock(*this, "clock")
		, m_interrupt_select(*this, "interrupt_select")
		, m_address_mode(*this, "address_mode")
		, m_two_control_regs(*this, "two_control_regs")
		, m_expected_density(*this, "expected_density")
		, m_expected_sectors(*this, "expected_sectors")
		, m_track_zero_expected_sectors(*this, "track_zero_expected_sectors")
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t register_read(offs_t offset) override;
	virtual void register_write(offs_t offset, uint8_t data) override;

private:
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_sso_w );

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	uint8_t m_fdc_status;              // for floppy controller
	int m_floppy_motor_on;
	emu_timer *m_floppy_motor_timer;
	floppy_image_device *m_fdc_floppy; // Current selected floppy.
	uint8_t m_fdc_side;                // Current floppy side.
	uint32_t m_fdc_clock;		   // Current clock frequency.

	TIMER_CALLBACK_MEMBER(floppy_motor_callback);

	void floppy_motor_trigger();

	void validate_floppy_side(uint8_t cmd);
	uint8_t validate_fdc_sector_size(uint8_t cmd);

	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_ioport m_clock;
	required_ioport m_interrupt_select;
	required_ioport m_address_mode;
	required_ioport m_two_control_regs;
	required_ioport m_expected_density;
	required_ioport m_expected_sectors;
	required_ioport m_track_zero_expected_sectors;
};


static INPUT_PORTS_START( dc5 )
	PORT_START("interrupt_select")
	PORT_DIPNAME(0xf, 0, "Interrupt select")
	PORT_DIPSETTING(0, "N/C")
	PORT_DIPSETTING(1, "IRQ")
	PORT_DIPSETTING(2, "NMI/FIRQ")

	PORT_START("address_mode")
	PORT_DIPNAME(0xf, 1, "Address mode")
	PORT_DIPSETTING(0, "4 address")
	PORT_DIPSETTING(1, "16 address")

	PORT_START("two_control_regs")
	PORT_DIPNAME(0xf, 0, "Two control registers")
	PORT_DIPSETTING(0, "No, DC4 compatible")
	PORT_DIPSETTING(1, "Yes, DC5 extension")

	// single or double density 5.24"  -  1.0MHz
	// 'standard' 3.5"  -  1.2MHz
	// 3.5" hd  -  2.0MHz
	// 8" 360rpm  -  2.4MHz
	PORT_START("clock")
	PORT_DIPNAME(0xffffff, 1000000, "FDC clock")
	PORT_DIPSETTING(1000000, "1.0 MHz")
	PORT_DIPSETTING(1200000, "1.2 MHz")
	PORT_DIPSETTING(2000000, "2.0 MHz")
	PORT_DIPSETTING(2400000, "2.4 MHz")

	// Debug aid to hard code the density. The FLEX format uses single
	// density for track zero. Many virtual disks 'fix' the format to be
	// purely double density and often without properly implement driver
	// support for that. This setting is an aid to report unexpected
	// usage, and it attempts to correct that.
	PORT_START("expected_density")
	PORT_CONFNAME(0xff, 0, "Expected density")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(1, "single density") // Purely single density.
	PORT_CONFSETTING(2, "double density, with single density track zero") // The default FLEX double density format.
	PORT_CONFSETTING(3, "double density") // Purely double density.

	// Debug aid, to check that the sector head or side is set as expected
	// for the sector ID being read for the FLEX floppy disk format. Many
	// FLEX disk images were developed for vitural machines that have
	// little regard for the actual head and can work off the sector ID
	// and their drivers can set the head incorrectly. E.g. a disk with 18
	// sectors per side might have a drive set to switch heads for sectors
	// above 10. Another issue is that double density disk images are
	// often 'fixed' so that they are pure double density without being
	// single density onthe first track, and the drivers might still set
	// the head based track zero being single density. This aid is not
	// intended to be a substitute for fixing the drivers but it does help
	// work through the issues while patching the disks.
	PORT_START("expected_sectors")
	PORT_CONFNAME(0xff, 0, "Expected sectors per side")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(10, "10") // 5 1/4" single density 256B
	PORT_CONFSETTING(15, "15") // 8" single density 256B
	PORT_CONFSETTING(18, "18") // 5 1/4" double density 256B
	PORT_CONFSETTING(26, "26") // 8" double density 256B
	PORT_CONFSETTING(36, "36") // 3.5" 1.4M QD 256B
	// The track zero expected sectors if different from the above. FLEX
	// 6800 disks did format track zero in single density and if the
	// driver sticks to that and if using a double density disk then set a
	// single density size here.
	PORT_START("track_zero_expected_sectors")
	PORT_CONFNAME(0xff, 0, "Track zero expected sectors per side")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(10, "10") // 5 1/4" single density 256B
	PORT_CONFSETTING(15, "15") // 8" single density 256B
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

FLOPPY_FORMATS_MEMBER( ss50_dc5_device::floppy_formats )
	FLOPPY_FLEX_FORMAT
FLOPPY_FORMATS_END

// todo: implement floppy controller cards as slot devices and do this properly
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
	WD2797(config, m_fdc, 1_MHz_XTAL);
	FLOPPY_CONNECTOR(config, "fdc:0", flex_floppies, "sssd35", ss50_dc5_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", flex_floppies, "sssd35", ss50_dc5_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", flex_floppies, "sssd35", ss50_dc5_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", flex_floppies, "sssd35", ss50_dc5_device::floppy_formats).enable_sound(true);

	m_fdc->intrq_wr_callback().set(FUNC(ss50_dc5_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(ss50_dc5_device::fdc_drq_w));
	m_fdc->sso_wr_callback().set(FUNC(ss50_dc5_device::fdc_sso_w));
}

void ss50_dc5_device::device_reset()
{
	uint32_t clock = m_clock->read();
	m_fdc->set_unscaled_clock(clock);
	m_fdc_clock = clock;
}

void ss50_dc5_device::device_start()
{
	m_fdc_status = 0;
	m_fdc_side = 0;
	m_floppy_motor_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ss50_dc5_device::floppy_motor_callback),this));
	m_floppy_motor_on = 0;
}


/* Shared floppy support. */

void ss50_dc5_device::floppy_motor_trigger()
{
	m_floppy0->get_device()->mon_w(CLEAR_LINE);
	m_floppy1->get_device()->mon_w(CLEAR_LINE);
	m_floppy2->get_device()->mon_w(CLEAR_LINE);
	m_floppy3->get_device()->mon_w(CLEAR_LINE);
	m_floppy_motor_timer->adjust(attotime::from_msec(30000));
	m_floppy_motor_on = 1;
}

TIMER_CALLBACK_MEMBER(ss50_dc5_device::floppy_motor_callback)
{
	m_floppy0->get_device()->mon_w(ASSERT_LINE);
	m_floppy1->get_device()->mon_w(ASSERT_LINE);
	m_floppy2->get_device()->mon_w(ASSERT_LINE);
	m_floppy3->get_device()->mon_w(ASSERT_LINE);
	m_floppy_motor_on = 0;
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
				if (m_fdc_floppy)
				{
					m_fdc_floppy->ss_w(expected_side);
					m_fdc_side = expected_side;
				}
			}
		}

		if (expected_sectors)
		{
			uint8_t expected_side = sector > expected_sectors ? 1 : 0;

			if (m_fdc_side != expected_side)
			{
				if (m_fdc_floppy)
				{
					m_fdc_floppy->ss_w(expected_side);
					m_fdc_side = expected_side;
				}
			}
		}
	}
}

// The WD2797 supports an alternate interpretation of the sector size. Check
// that the flag is as expected and return the corrected command if necessary.
uint8_t ss50_dc5_device::validate_fdc_sector_size(uint8_t cmd)
{
	if ((cmd & 0xe1) == 0x80 || (cmd & 0xe0) == 0xa0)
	{
		// Check that the sector size L flag is set, as expected.
		uint8_t sector_length_default = cmd & 0x08;
		if (!sector_length_default)
		{
			cmd |= 0x08;
		}
	}
	return cmd;
}

WRITE_LINE_MEMBER( ss50_dc5_device::fdc_intrq_w )
{
	if (state)
		m_fdc_status |= 0x40;
	else
		m_fdc_status &= ~0x40;

	uint8_t selection = m_interrupt_select->read();

	if (selection == 1)
	{
		write_irq(state);
	}
	else if (selection == 2)
	{
		write_firq(state);
	}
}

WRITE_LINE_MEMBER( ss50_dc5_device::fdc_drq_w )
{
	if (state)
		m_fdc_status |= 0x80;
	else
		m_fdc_status &= 0x7f;
}

WRITE_LINE_MEMBER( ss50_dc5_device::fdc_sso_w )
{
		// The DC4 and DC5 invert the SSO output and wire it to the
		// DDEN input to allow selection of single or double density.

		// A DC5 extension. In two-register mode the SSO pin does not
		// control DDEN.
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
				m_fdc->dden_w(1);
				break;
			case 2:
			{
				// Double density with track zero single density.
				uint8_t track = m_fdc->track_r();

				if (track == 0)
				{
					m_fdc->dden_w(1);
				}
				else
				{
					m_fdc->dden_w(0);
				}
				break;
			}
			case 3:
				// Pure double density.
				m_fdc->dden_w(0);
				break;
			default:
				m_fdc->dden_w(!state);
				break;
		}
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

		if (!two_regs || (offset & 2) == 0)
		{
			// Control register 8014 / e014 write.
			floppy_image_device *floppy = nullptr;
			uint8_t drive = data & 3;

			// The DC2, DC3, DC4, DC5 have a drive inhibit feature
			// controlled by bit 7. Have not seen software use that?

			if (drive == 0) floppy = m_floppy0->get_device();
			if (drive == 1) floppy = m_floppy1->get_device();
			if (drive == 2) floppy = m_floppy2->get_device();
			if (drive == 3) floppy = m_floppy3->get_device();

			if (floppy != m_fdc_floppy)
			{
				m_fdc->set_floppy(floppy);
				m_fdc_floppy = floppy;
			}

			if (floppy)
			{
				// The DC3, DC4, DC5 have a side select feature that
				// is controlled by bit 6. The DC1 and DC2 do not have
				// a floppy side output.
				uint8_t side = BIT(data, 6);
				floppy->ss_w(side);
				m_fdc_side = side;
			}
		}

		if (two_regs && (offset & 2) == 2)
		{
			// DC5 extended control register 8016 / E016.
#if 0
			// TODO DC5 clocking extensions.
			// Osc20 and osc12 control a clock divider.
			uint8_t osc12 = BIT(data, 2);
			uint8_t osc20 = BIT(data, 3);
			uint32_t clock = 1000000;
			if (osc20)
				clock = 2000000;
			else if (osc12)
				clock = 1200000;
			m_fdc->set_unscaled_clock(clock);
			m_fdc_clock = clock;

			// Connects to the WD2979 5/8 pin.
			// TODO is this needed for the emulator?
			uint8_t five_or_eight = BIT(data, 4);
#endif
			// In this two-register mode the FDC SSO output pin
			// does not control DDEN.
			uint8_t dden = BIT(data, 5);
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
		}

		m_fdc->wd2797_device::write(offset, data);
	}
}

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(SS50_DC5, ss50_card_interface, ss50_dc5_device, "ss50_dc5", "DC5 Floppy Disk Controller")
