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
	cgenie_fdc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_ADDRESS_MAP(mmio, 8);

	TIMER_DEVICE_CALLBACK_MEMBER(timer_callback);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(socket_load);

	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_READ8_MEMBER(irq_r);
	DECLARE_WRITE8_MEMBER(select_w);
	DECLARE_WRITE8_MEMBER(command_w);

	DECLARE_FLOPPY_FORMATS(floppy_formats);


protected:
	virtual const rom_entry *device_rom_region() const override;
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

	UINT8 m_irq_status;
};

// device type definition
extern const device_type CGENIE_FDC;

#endif // __CGENIE_EXPANSION_FLOPPY_H__
