// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    multibus.cpp

    Intel Multibus

*********************************************************************/

#include "emu.h"
#include "multibus.h"
#include "isbc202.h"

// device type definition
DEFINE_DEVICE_TYPE(MULTIBUS_SLOT, multibus_slot_device, "multibus_slot", "Intel Multibus slot")

multibus_slot_device::multibus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , MULTIBUS_SLOT , tag , owner , clock)
	, device_single_card_slot_interface<device_multibus_interface>(mconfig , *this)
	, m_irq_cb(*this)
	, m_irqa_cb(*this)
{
	option_reset();
	option_add("isbc202" , ISBC202);
	set_default_option(nullptr);
	set_fixed(false);
}

multibus_slot_device::~multibus_slot_device()
{
}

void multibus_slot_device::install_io_rw(address_space& space)
{
	device_multibus_interface *card = get_card_device();
	if (card) {
		card->install_io_rw(space);
	}
}

void multibus_slot_device::install_mem_rw(address_space& space)
{
	device_multibus_interface *card = get_card_device();
	if (card) {
		card->install_mem_rw(space);
	}
}

void multibus_slot_device::device_start()
{
	m_irq_cb.resolve_all_safe();
	m_irqa_cb.resolve_safe();
}

device_multibus_interface::device_multibus_interface(const machine_config &mconfig , device_t &device)
	: device_interface(device , "multibus")
{
}

device_multibus_interface::~device_multibus_interface()
{
}

WRITE_LINE_MEMBER( multibus_slot_device::irq0_w ) { m_irq_cb[0](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irq1_w ) { m_irq_cb[1](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irq2_w ) { m_irq_cb[2](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irq3_w ) { m_irq_cb[3](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irq4_w ) { m_irq_cb[4](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irq5_w ) { m_irq_cb[5](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irq6_w ) { m_irq_cb[6](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irq7_w ) { m_irq_cb[7](state); }
WRITE_LINE_MEMBER( multibus_slot_device::irqa_w ) { m_irqa_cb(state); }
