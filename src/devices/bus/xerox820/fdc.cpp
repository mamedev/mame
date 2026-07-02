// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II FD1797 floppy controller daughterboard

    The main-board floppy controller personality of the 9R80758: an
    FD1797 with two Shugart drives at Z80 ports 0x10-0x13.  Unlike the
    first-generation 820's FD1771 the FD1797 data bus is not inverted.
    Drive select, side and density come from the system PIO / latch on
    the main board; INTRQ and DRQ return to the main board's /HALT-gated
    /NMI generator.

**********************************************************************/

#include "emu.h"
#include "fdc.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

#include "formats/flopimg.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> xerox820_fdc_device

class xerox820_fdc_device : public device_t, public device_xerox820_dbslot_card_interface
{
public:
	xerox820_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_xerox820_dbslot_card_interface implementation
	virtual uint8_t io_r(offs_t offset) override;
	virtual void io_w(offs_t offset, uint8_t data) override;
	virtual void drvsel_w(uint8_t data) override;
	virtual void density_w(int fm) override;
	virtual void box_select_w(uint8_t data) override;
	virtual bool media_8inch() override { return m_8n5; }
	virtual bool media_twosided() override { return m_400_460; }
	virtual uint8_t personality() override { return 0x01; } // FD1797 floppy controller

protected:
	xerox820_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const char *default_floppy, bool single_floppy);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void prime_rate(floppy_image_device *floppy);

	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;

	const char *const m_default_floppy;
	const bool m_single_floppy; // 16/8 RX024 box: the 5.25" drive is permanently cabled to floppy0

	bool m_8n5;       // 8"/5.25" media sense (PA4)
	bool m_400_460;   // double-sided media sense (PA5)
};


// ======================> xerox820_fdc5_device (5.25" drive complement)

class xerox820_fdc5_device : public xerox820_fdc_device
{
public:
	xerox820_fdc5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> xerox820_fdc_box5_device (16/8 RX024 5.25" floppy box)

class xerox820_fdc_box5_device : public xerox820_fdc_device
{
public:
	xerox820_fdc_box5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(XEROX820_FDC, device_xerox820_dbslot_card_interface, xerox820_fdc_device, "xerox820_fdc", "Xerox 820-II FD1797 floppy controller (8\")")
DEFINE_DEVICE_TYPE_PRIVATE(XEROX820_FDC5, device_xerox820_dbslot_card_interface, xerox820_fdc5_device, "xerox820_fdc5", "Xerox 820-II FD1797 floppy controller (5.25\")")
DEFINE_DEVICE_TYPE_PRIVATE(XEROX820_FDC_BOX5, device_xerox820_dbslot_card_interface, xerox820_fdc_box5_device, "xerox820_fdc_box5", "Xerox 16/8 RX024 5.25\" floppy box")


static void xerox820_floppies(device_slot_interface &device)
{
	device.option_add("sa400", FLOPPY_525_SSSD_35T); // Shugart SA-400, 35 trk drive
	device.option_add("sa400l", FLOPPY_525_SSSD);    // Shugart SA-400, 40 trk drive
	device.option_add("sa450", FLOPPY_525_DD);       // Shugart SA-450
	device.option_add("sa800", FLOPPY_8_SSDD);       // Shugart SA-800
	device.option_add("sa850", FLOPPY_8_DSDD);       // Shugart SA-850
}


//**************************************************************************
//  xerox820_fdc_device
//**************************************************************************

xerox820_fdc_device::xerox820_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: xerox820_fdc_device(mconfig, XEROX820_FDC, tag, owner, clock, "sa850", false)
{
}

xerox820_fdc_device::xerox820_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const char *default_floppy, bool single_floppy)
	: device_t(mconfig, type, tag, owner, clock)
	, device_xerox820_dbslot_card_interface(mconfig, *this)
	, m_fdc(*this, "fd1797")
	, m_floppy0(*this, "fd1797:0")
	, m_floppy1(*this, "fd1797:1")
	, m_default_floppy(default_floppy)
	, m_single_floppy(single_floppy)
	, m_8n5(false)
	, m_400_460(false)
{
}

xerox820_fdc5_device::xerox820_fdc5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: xerox820_fdc_device(mconfig, XEROX820_FDC5, tag, owner, clock, "sa450", false)
{
}

xerox820_fdc_box5_device::xerox820_fdc_box5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: xerox820_fdc_device(mconfig, XEROX820_FDC_BOX5, tag, owner, clock, "sa450", true)
{
}


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void xerox820_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set([this](int state) { m_slot->intrq_w(state); });
	m_fdc->drq_wr_callback().set([this](int state) { m_slot->drq_w(state); });
	m_fdc->sso_wr_callback().set_nop(); // SSO unconnected; side select is PIO-driven (system port bit 2)
	FLOPPY_CONNECTOR(config, m_floppy0, xerox820_floppies, m_default_floppy, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, xerox820_floppies, m_default_floppy, floppy_image_device::default_mfm_floppy_formats);
}


//-------------------------------------------------
//  device_start / device_reset
//-------------------------------------------------

void xerox820_fdc_device::device_start()
{
	save_item(NAME(m_8n5));
	save_item(NAME(m_400_460));
}

void xerox820_fdc_device::device_reset()
{
	m_fdc->reset();

	// The FD1797 data rate must match the installed drive size.  drvsel_w only
	// updates the clock when the form factor *changes* (and m_8n5 starts at 5.25",
	// so a 5.25" drive would never trigger it) -> prime the rate + sense
	// unconditionally from the connected drive's form factor here (8" = 2 MHz,
	// 5.25" = 1 MHz).
	if (floppy_image_device *fd = m_floppy0->get_device())
	{
		m_8n5 = (fd->get_form_factor() == floppy_image::FF_8);
		m_fdc->set_unscaled_clock(m_8n5 ? 16_MHz_XTAL / 8 : 16_MHz_XTAL / 16);
		fd->mon_w(0); // spindle motor on (drive ready for the boot's RESTORE)
	}
}


//-------------------------------------------------
//  prime_rate - update the data rate when the form factor changes
//-------------------------------------------------

void xerox820_fdc_device::prime_rate(floppy_image_device *floppy)
{
	bool const eight = (floppy->get_form_factor() == floppy_image::FF_8);

	if (m_8n5 != eight)
	{
		m_8n5 = eight;
		m_fdc->set_unscaled_clock(eight ? 16_MHz_XTAL / 8 : 16_MHz_XTAL / 16);
	}
}


//-------------------------------------------------
//  io_r / io_w - Z80 ports 0x10-0x13
//-------------------------------------------------

uint8_t xerox820_fdc_device::io_r(offs_t offset)
{
	return m_fdc->read(offset & 0x03);
}

void xerox820_fdc_device::io_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset & 0x03, data);
}


//-------------------------------------------------
//  density_w - density select (1 = FM/single)
//-------------------------------------------------

void xerox820_fdc_device::density_w(int fm)
{
	m_fdc->dden_w(fm);
}



//-------------------------------------------------
//  drvsel_w - system PIO port A: drive select / side
//-------------------------------------------------

void xerox820_fdc_device::drvsel_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	// 16/8 RX024 box: the 5.25" drive is permanently cabled to the FD1797 and
	// its box controller routes the drive, so the system-PIO drive-select bits
	// 0-1 do NOT mux it (the v5.0 monitor in fact drives them with unrelated
	// keyboard-ROM banking traffic).  The loadable WDVR driver samples READY
	// before it issues its own select, so floppy0 must already be attached and
	// spinning.  Pin the FDC to floppy0 (motor kept on below).
	if (m_single_floppy)
		floppy = m_floppy0->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		prime_rate(floppy);
		m_400_460 = !floppy->twosid_r();
		floppy->mon_w(0);

		// The v5.0 monitor's keyboard-ROM banking writes this port with garbage
		// in the low bits (0x3F/0xBF, bits 4 & 5 set) that would flip the head to
		// the empty side 1 of a single-sided disk.  Honor the driver's side select
		// and ignore the banking writes (bits 4 & 5 set) on the single-floppy box;
		// other configurations select the side normally.
		if (m_single_floppy)
		{
			if ((data & 0x30) != 0x30)
				floppy->ss_w(BIT(data, 2));
		}
		else
			floppy->ss_w(BIT(data, 2));
	}
}


//-------------------------------------------------
//  box_select_w - 16/8 RX024 box select latch (raw I/O-bus write)
//-------------------------------------------------

void xerox820_fdc_device::box_select_w(uint8_t data)
{
	// On the 16/8 the system PIO keeps D0-D2 as inputs, so it cannot drive
	// drive-select/side.  The RX024 box's own latch listens to the I/O-bus
	// write directly: pin the box's single floppy, motor on, side from bit 2
	// (unless the banking-write bits 4 & 5 are set).
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
	{
		floppy->mon_w(0);
		if ((data & 0x30) != 0x30)
			floppy->ss_w(BIT(data, 2));
	}
}


