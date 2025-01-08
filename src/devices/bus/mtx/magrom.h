// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Martin Allcorn's Games ROM

**********************************************************************/


#ifndef MAME_BUS_MTX_MAGROM_H
#define MAME_BUS_MTX_MAGROM_H

#include "exp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mtx_magrom_device

class mtx_magrom_device : public device_t, public device_mtx_exp_interface
{
public:
	// construction/destruction
	mtx_magrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void bankswitch(uint8_t data) override;

private:
	memory_bank_creator m_bank;
	required_ioport m_jumpers;

	void page_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(MTX_MAGROM, mtx_magrom_device)


#endif // MAME_BUS_MTX_MAGROM_H
