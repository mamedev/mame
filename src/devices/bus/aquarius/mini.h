// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius Mini Expander

**********************************************************************/


#ifndef MAME_BUS_AQUARIUS_MINI_H
#define MAME_BUS_AQUARIUS_MINI_H

#include "slot.h"
#include "machine/input_merger.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class aquarius_mini_device:
	public device_t,
	public device_aquarius_cartridge_interface
{
public:
	// construction/destruction
	aquarius_mini_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

protected:
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_aquarius_cartridge_interface overrides
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mreq_ce_r(offs_t offset) override;
	virtual void mreq_ce_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	required_device<input_merger_device> m_irqs;
	required_device_array<aquarius_cartridge_slot_device, 2> m_exp;
	required_device<ay8910_device> m_ay;

	uint8_t m_ctrl_input[2];
};


// device type definition
DECLARE_DEVICE_TYPE(AQUARIUS_MINI, aquarius_mini_device)


#endif /* MAME_BUS_AQUARIUS_MINI_H */
