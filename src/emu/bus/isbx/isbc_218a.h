// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ISBX 218a with ISBC configuration

**********************************************************************/

#pragma once

#ifndef __ISBC_218A__
#define __ISBC_218A__

#include "emu.h"
#include "isbx.h"
#include "formats/pc_dsk.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_fdc_device

class isbc_218a_device : public device_t,
							public device_isbx_card_interface
{
public:
	// construction/destruction
	isbc_218a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	virtual UINT8 mcs1_r(address_space &space, offs_t offset);
	virtual void mcs1_w(address_space &space, offs_t offset, UINT8 data);
	virtual UINT8 mdack_r(address_space &space, offs_t offset);
	virtual void mdack_w(address_space &space, offs_t offset, UINT8 data);
	virtual void opt0_w(int state);

private:
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;

	bool m_reset, m_motor;
};


// device type definition
extern const device_type ISBC_218A;


#endif
