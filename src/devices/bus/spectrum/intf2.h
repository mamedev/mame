// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ZX Interface 2

**********************************************************************/


#ifndef MAME_BUS_SPECTRUM_INTF2_H
#define MAME_BUS_SPECTRUM_INTF2_H

#include "exp.h"
#include "softlist.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_intf2_device:
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_intf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool romcs() override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;

private:
	std::error_condition load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	required_ioport m_exp_line3;
	required_ioport m_exp_line4;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_INTF2, spectrum_intf2_device)


#endif /* MAME_BUS_SPECTRUM_INTF2_H */
