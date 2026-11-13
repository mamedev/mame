// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 12/16B/6000 card cage (26-6017) emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_CAGE_H
#define MAME_BUS_TRS80M2_CAGE_H

#pragma once

#include "slot.h"


class trs80m12_cage_device : public device_t, public device_trs80m2_card_interface
{
public:
	trs80m12_cage_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_trs80m2_card_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<trs80m2_slot_device, 7> m_slots;
};


DECLARE_DEVICE_TYPE(TRS80M12_CAGE, trs80m12_cage_device)

#endif // MAME_BUS_TRS80M2_CAGE_H
