// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************

 Sega AI Expansion slot emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "segaai_exp.h"

DEFINE_DEVICE_TYPE(SEGAAI_EXP_SLOT, segaai_exp_slot_device, "segaai_exp_slot", "Sega AI Expansion Slot")


segaai_exp_interface::segaai_exp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "segaai_exp")
	, m_slot(dynamic_cast<segaai_exp_slot_device *>(device.owner()))
{
}

segaai_exp_interface::~segaai_exp_interface()
{
}



segaai_exp_slot_device::segaai_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGAAI_EXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<segaai_exp_interface>(mconfig, *this)
	, m_mem_space(*this, finder_base::DUMMY_TAG, -1)
	, m_io_space(*this, finder_base::DUMMY_TAG, -1)
{
}

segaai_exp_slot_device::~segaai_exp_slot_device()
{
}


// slot interfaces
#include "soundbox.h"

void segaai_exp(device_slot_interface &device)
{
	device.option_add("soundbox", SEGAAI_SOUNDBOX);
}

