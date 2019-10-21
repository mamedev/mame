// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam floppy disk controller emulation

**********************************************************************/

#ifndef MAME_BUS_ADAMNET_FDC_H
#define MAME_BUS_ADAMNET_FDC_H

#pragma once

#include "adamnet.h"
#include "cpu/m6800/m6801.h"
#include "formats/adam_dsk.h"
#include "imagedev/floppy.h"
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

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

private:
	required_device<m6801_cpu_device> m_maincpu;
	required_device<wd2793_device> m_fdc;
	required_device<floppy_connector> m_connector;
	floppy_image_device *m_floppy;
	required_shared_ptr<uint8_t> m_ram;
	required_ioport m_sw3;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );

	void adam_fdc_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ADAM_FDC, adam_fdc_device)


#endif // MAME_BUS_ADAMNET_FDC_H
