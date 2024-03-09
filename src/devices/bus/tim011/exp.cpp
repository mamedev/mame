// license:BSD-3-Clause
// copyright-holders:Marko Solajic, Miodrag Milanovic
/**********************************************************************

        TIM-011 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"

DEFINE_DEVICE_TYPE(TIM011_EXPANSION_SLOT, bus::tim011::exp_slot_device, "tim011_exp_slot", "TIM-011 Expansion port")

namespace bus::tim011 {

/***********************************************************************
    CARD INTERFACE
***********************************************************************/

device_exp_interface::device_exp_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "tim011exp")
{
	m_slot = dynamic_cast<exp_slot_device *>(device.owner());
}

/***********************************************************************
    SLOT DEVICE
***********************************************************************/

exp_slot_device::exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TIM011_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_exp_interface>(mconfig, *this),
	m_io(*this, finder_base::DUMMY_TAG, -1),
	m_int_handler(*this),
	m_nmi_handler(*this)
{
}

/*----------------------------------
  device_t implementation
----------------------------------*/

void exp_slot_device::device_start()
{
}

} // namespace bus::tim011

#include "aycard.h"

void tim011_exp_devices(device_slot_interface &device)
{
	device.option_add("ay", TIM011_AYCARD);
}
