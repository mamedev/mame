// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ISBX 218a with ISBC configuration

**********************************************************************/

#ifndef MAME_BUS_ISBX_ISBC_218A_H
#define MAME_BUS_ISBX_ISBC_218A_H

#pragma once

#include "isbx.h"
#include "imagedev/floppy.h"
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
	isbc_218a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_isbx_card_interface overrides
	virtual uint8_t mcs0_r(offs_t offset) override;
	virtual void mcs0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mcs1_r(offs_t offset) override;
	virtual void mcs1_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mdack_r(offs_t offset) override;
	virtual void mdack_w(offs_t offset, uint8_t data) override;
	virtual void opt0_w(int state) override;

private:
	void fdc_irq(int state);
	void fdc_drq(int state);
	static void floppy_formats(format_registration &fr);

	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;

	bool m_reset, m_motor, m_fd8;
};


// device type definition
DECLARE_DEVICE_TYPE(ISBC_218A, isbc_218a_device)


#endif // MAME_BUS_ISBX_ISBC_218A_H
