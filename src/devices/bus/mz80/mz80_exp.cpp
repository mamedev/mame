// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Sharp MZ-80 I/O Expansion board

https://www.sharpmz.org/hwindex.htm

TODO:
- preliminary, just enough to have a base to map in MZ based machines;

**************************************************************************************************/

#include "emu.h"
#include "mz80_exp.h"

DEFINE_DEVICE_TYPE(MZ80_EXP_SLOT, mz80_exp_slot_device, "mz80_exp_slot", "MZ-80 Expansion Slot")

mz80_exp_slot_device::mz80_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MZ80_EXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_mz80_exp_interface>(mconfig, *this)
	, m_iospace(*this, finder_base::DUMMY_TAG, -1)
{
}

mz80_exp_slot_device::~mz80_exp_slot_device()
{
}

void mz80_exp_slot_device::device_start()
{
}

device_mz80_exp_interface::device_mz80_exp_interface(const machine_config &mconfig, device_t &device)
   : device_interface(device, "mz80exp")
{
	m_slot = dynamic_cast<mz80_exp_slot_device *>(device.owner());
}

device_mz80_exp_interface::~device_mz80_exp_interface()
{
}

void device_mz80_exp_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_mz80_exp_interface::interface_post_start()
{
	m_slot->install_io_device(*this, &device_mz80_exp_interface::io_map);
}

mz80_exp_device::mz80_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mz80_exp_interface(mconfig, *this)
{
}

void mz80_exp_device::device_start()
{

}


// slot devices

#include "mz1e30.h"
#include "mz1e35.h"
#include "mz1r37.h"

void mz2500_exp_devices(device_slot_interface &device)
{
	device.option_add("mz1e30", MZ1E30);
	device.option_add("mz1e35", MZ1E35);
	device.option_add("mz1r37", MZ1R37);
}
