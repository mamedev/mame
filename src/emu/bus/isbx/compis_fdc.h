// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis Floppy Disk Controller (119 106/1) emulation

**********************************************************************/

#pragma once

#ifndef __COMPIS_FDC__
#define __COMPIS_FDC__

#include "emu.h"
#include "isbx.h"
#include "formats/cpis_dsk.h"
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
	compis_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER( fdc_irq );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_isbx_card_interface overrides
	virtual UINT8 mcs0_r(address_space &space, offs_t offset);
	virtual void mcs0_w(address_space &space, offs_t offset, UINT8 data);
	virtual UINT8 mdack_r(address_space &space, offs_t offset);
	virtual void mdack_w(address_space &space, offs_t offset, UINT8 data);
	virtual void opt0_w(int state);
	virtual void opt1_w(int state);

private:
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};


// device type definition
extern const device_type COMPIS_FDC;


#endif
