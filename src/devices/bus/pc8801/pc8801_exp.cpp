// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    PC-8801 EXPansion bus

    TODO:
    - Extend for PC-88VA EXPansion bus
      (can potentially access 16-bit iospace rather than 8 at very least);
    - Transparent support for on-board ROMs;
    - Implement device_mixer_interface for sound cards using it (GH #10746);

**************************************************************************************************/

#include "emu.h"
#include "pc8801_exp.h"

DEFINE_DEVICE_TYPE(PC8801_EXP_SLOT, pc8801_exp_slot_device, "pc8801_exp_slot", "PC-8801 Expansion Slot")

pc8801_exp_slot_device::pc8801_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC8801_EXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_pc8801_exp_interface>(mconfig, *this)
	, m_iospace(*this, finder_base::DUMMY_TAG, -1)
	, m_int3_cb(*this)
	, m_int4_cb(*this)
	, m_int5_cb(*this)
{
}

pc8801_exp_slot_device::~pc8801_exp_slot_device()
{
}

void pc8801_exp_slot_device::device_start()
{
}

device_pc8801_exp_interface::device_pc8801_exp_interface(const machine_config &mconfig, device_t &device)
   : device_interface(device, "pc8801exp")
{
	m_slot = dynamic_cast<pc8801_exp_slot_device *>(device.owner());
}

device_pc8801_exp_interface::~device_pc8801_exp_interface()
{
}

void device_pc8801_exp_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_pc8801_exp_interface::interface_post_start()
{
	m_slot->install_io_device(*this, &device_pc8801_exp_interface::io_map);
}

// generic passthroughs to INT* lines
// NB: clients are responsible to handle irq masking just like base HW if available
void device_pc8801_exp_interface::int3_w(int state) { m_slot->m_int3_cb(state); }
void device_pc8801_exp_interface::int4_w(int state) { m_slot->m_int4_cb(state); }
void device_pc8801_exp_interface::int5_w(int state) { m_slot->m_int5_cb(state); }

pc8801_exp_device::pc8801_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pc8801_exp_interface(mconfig, *this)
{
}

void pc8801_exp_device::device_start()
{

}



// slot devices
#include "gsx8800.h"
#include "pcg8100.h"
#include "pc8801_23.h"
#include "jmbx1.h"
#include "hmb20.h"

void pc8801_exp_devices(device_slot_interface &device)
{
	device.option_add("sbii", PC8801_23);

	device.option_add("pcg8100", PCG8100);

	device.option_add("jmbx1", JMBX1);
	device.option_add("hmb20", HMB20);
	device.option_add("gsx8800", GSX8800);
}
