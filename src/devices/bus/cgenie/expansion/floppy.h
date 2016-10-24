// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Floppy Controller Cartridge

***************************************************************************/

#pragma once

#ifndef __CGENIE_EXPANSION_FLOPPY_H__
#define __CGENIE_EXPANSION_FLOPPY_H__

#include "emu.h"
#include "expansion.h"
#include "machine/wd_fdc.h"
#include "bus/generic/slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> floppy_controller_device

class cgenie_fdc_device : public device_t, public device_expansion_interface
{
public:
	// construction/destruction
	cgenie_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_ADDRESS_MAP(mmio, 8);

	void timer_callback(timer_device &timer, void *ptr, int32_t param);

	image_init_result device_image_load_socket_load(device_image_interface &image);

	void intrq_w(int state);
	uint8_t irq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	DECLARE_FLOPPY_FORMATS(floppy_formats);


protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<fd1793_t> m_fdc;
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
extern const device_type CGENIE_FDC;

#endif // __CGENIE_EXPANSION_FLOPPY_H__
