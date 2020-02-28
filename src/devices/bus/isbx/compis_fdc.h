// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis Floppy Disk Controller (119 106/1) emulation

**********************************************************************/

#ifndef MAME_BUS_ISBX_COMPIS_FDC_H
#define MAME_BUS_ISBX_COMPIS_FDC_H

#pragma once

#include "isbx.h"
#include "formats/cpis_dsk.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_fdc_device

class compis_fdc_device : public device_t,
							public device_isbx_card_interface
{
public:
	// construction/destruction
	compis_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// device_isbx_card_interface overrides
	virtual uint8_t mcs0_r(offs_t offset) override;
	virtual void mcs0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mdack_r(offs_t offset) override;
	virtual void mdack_w(offs_t offset, uint8_t data) override;
	virtual void opt0_w(int state) override;
	virtual void opt1_w(int state) override;

private:
	DECLARE_WRITE_LINE_MEMBER( fdc_irq );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};


// device type definition
DECLARE_DEVICE_TYPE(COMPIS_FDC, compis_fdc_device)


#endif // MAME_BUS_ISBX_COMPIS_FDC_H
