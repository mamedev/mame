// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 12/16B/6000 card cage (26-6017) emulation

    Motherboard 8897701 (AX-9369) with seven slots, J1 at the bottom,
    plugged into the main board's video/keyboard connector. The daisy
    chain continues from the main board into J1, then J2 and upwards.

**********************************************************************/

#include "emu.h"
#include "cage.h"


DEFINE_DEVICE_TYPE(TRS80M12_CAGE, trs80m12_cage_device, "trs80m12_cage", "Tandy Model 12 card cage")


trs80m12_cage_device::trs80m12_cage_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TRS80M12_CAGE, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_slots(*this, "j%u", 1U)
{
}

void trs80m12_cage_device::device_add_mconfig(machine_config &config)
{
	trs80m2_slot_device *const parent = dynamic_cast<trs80m2_slot_device *>(owner());

	for (int i = 0; i < 7; i++)
	{
		TRS80M2_SLOT(config, m_slots[i], DERIVED_CLOCK(1, 1), trs80m12_cards, nullptr);

		if (parent)
			m_slots[i]->bus_finder().set_tag(parent->bus_finder());
	}
}

void trs80m12_cage_device::device_start()
{
}

int trs80m12_cage_device::z80daisy_irq_state()
{
	int state = 0;

	for (auto &slot : m_slots)
	{
		int const slot_state = slot->z80daisy_irq_state();

		state |= slot_state;

		if (slot_state & (Z80_DAISY_INT | Z80_DAISY_IEO))
			break;
	}

	return state;
}

int trs80m12_cage_device::z80daisy_irq_ack()
{
	for (auto &slot : m_slots)
	{
		int const slot_state = slot->z80daisy_irq_state();

		if (slot_state & Z80_DAISY_INT)
			return slot->z80daisy_irq_ack();

		if (slot_state & Z80_DAISY_IEO)
			break;
	}

	return -1;
}

void trs80m12_cage_device::z80daisy_irq_reti()
{
	for (auto &slot : m_slots)
	{
		if (slot->z80daisy_irq_state() & Z80_DAISY_IEO)
		{
			slot->z80daisy_irq_reti();
			break;
		}
	}
}
