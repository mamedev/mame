// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 card cage slot emulation

**********************************************************************/

#include "emu.h"
#include "slot.h"

#include "hdc.h"


DEFINE_DEVICE_TYPE(TRS80M2_SLOT, trs80m2_slot_device, "trs80m2_slot", "Tandy Model II card cage slot")


device_trs80m2_card_interface::device_trs80m2_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "trs80m2"),
	m_slot(nullptr)
{
}

void device_trs80m2_card_interface::interface_pre_start()
{
	m_slot = dynamic_cast<trs80m2_slot_device *>(device().owner());
}

int device_trs80m2_card_interface::z80daisy_irq_state()
{
	return 0;
}

int device_trs80m2_card_interface::z80daisy_irq_ack()
{
	return -1;
}

void device_trs80m2_card_interface::z80daisy_irq_reti()
{
}


trs80m2_slot_device::trs80m2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TRS80M2_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_trs80m2_card_interface>(mconfig, *this),
	device_z80daisy_interface(mconfig, *this),
	m_io(*this, finder_base::DUMMY_TAG, -1),
	m_write_int(*this),
	m_write_drq(*this),
	m_card(nullptr)
{
}

void trs80m2_slot_device::device_start()
{
	m_card = get_card_device();
}

int trs80m2_slot_device::z80daisy_irq_state()
{
	return m_card ? m_card->z80daisy_irq_state() : 0;
}

int trs80m2_slot_device::z80daisy_irq_ack()
{
	return m_card ? m_card->z80daisy_irq_ack() : -1;
}

void trs80m2_slot_device::z80daisy_irq_reti()
{
	if (m_card)
		m_card->z80daisy_irq_reti();
}


void trs80m2_cards(device_slot_interface &device)
{
	device.option_add("hdc", TRS80M2_HDC);
}
