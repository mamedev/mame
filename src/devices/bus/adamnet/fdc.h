// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam floppy disk controller emulation

**********************************************************************/

#pragma once

#ifndef __ADAM_FDC__
#define __ADAM_FDC__

#include "emu.h"
#include "adamnet.h"
#include "cpu/m6800/m6800.h"
#include "formats/adam_dsk.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_fdc_device

class adam_fdc_device :  public device_t,
							public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	uint8_t data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

	required_device<cpu_device> m_maincpu;
	required_device<wd2793_t> m_fdc;
	required_device<floppy_image_device> m_floppy0;
	floppy_image_device *m_floppy;
	required_shared_ptr<uint8_t> m_ram;
	required_ioport m_sw3;
};


// device type definition
extern const device_type ADAM_FDC;


#endif
