// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II FD1797 floppy controller daughterboard

**********************************************************************/

#ifndef MAME_BUS_XEROX820_FDC_H
#define MAME_BUS_XEROX820_FDC_H

#pragma once

#include "dbslot.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


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


// device type declarations
DECLARE_DEVICE_TYPE(XEROX820_FDC, xerox820_fdc_device)
DECLARE_DEVICE_TYPE(XEROX820_FDC5, xerox820_fdc5_device)
DECLARE_DEVICE_TYPE(XEROX820_FDC_BOX5, xerox820_fdc_box5_device)

#endif // MAME_BUS_XEROX820_FDC_H
