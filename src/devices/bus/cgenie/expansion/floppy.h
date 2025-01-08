// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Floppy Controller Cartridge

***************************************************************************/

#ifndef MAME_BUS_CGENIE_EXPANSION_FLOPPY_H
#define MAME_BUS_CGENIE_EXPANSION_FLOPPY_H

#pragma once

#include "expansion.h"
#include "imagedev/floppy.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "bus/generic/slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> floppy_controller_device

class cgenie_fdc_device : public device_t, public device_cg_exp_interface
{
public:
	// construction/destruction
	cgenie_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(socket_load);

	void intrq_w(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(timer_callback);

	uint8_t irq_r();
	void select_w(uint8_t data);
	void command_w(uint8_t data);

	void mmio(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	required_device<wd2793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<generic_slot_device> m_socket;

	enum
	{
		IRQ_WDC = 0x40,
		IRQ_TIMER = 0x80
	};

	floppy_image_device *m_floppy;

	uint8_t m_irq_status;
};

// device type definition
DECLARE_DEVICE_TYPE(CGENIE_FDC, cgenie_fdc_device)

#endif // MAME_BUS_CGENIE_EXPANSION_FLOPPY_H
