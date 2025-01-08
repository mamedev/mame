// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "fp1000_exp.h"

#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(FP1000_EXP_SLOT, fp1000_exp_slot_device, "fp1000_exp_slot", "FP-1000/FP-1100 Expansion Slot")

fp1000_exp_slot_device::fp1000_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FP1000_EXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_fp1000_exp_interface>(mconfig, *this)
	, m_iospace(*this, finder_base::DUMMY_TAG, -1)
	, m_inta_cb(*this)
	, m_intb_cb(*this)
	, m_intc_cb(*this)
	, m_intd_cb(*this)
{
}

fp1000_exp_slot_device::~fp1000_exp_slot_device()
{
}

void fp1000_exp_slot_device::device_start()
{
}

void fp1000_exp_slot_device::device_config_complete()
{
	m_dev = get_card_device();
}

void fp1000_exp_slot_device::remap_cb()
{
	if (!m_main_enable || m_dev == nullptr)
	{
		LOG("%s: unmap\n", machine().describe_context());
		m_iospace->unmap_readwrite(0x0000, 0xfeff);
		m_iospace->unmap_readwrite(0xff00, 0xff7f);
	}
	else
	{
		LOG("%s: map\n", machine().describe_context());
		m_iospace->install_readwrite_handler(0xff00, 0xff7f, read8sm_delegate(*m_dev, FUNC(device_fp1000_exp_interface::id_r)), write8sm_delegate(*this, FUNC(fp1000_exp_slot_device::main_cs_w)));
		m_dev->remap_cb();
	}
}

void fp1000_exp_slot_device::select_w(bool enable)
{
	m_main_enable = enable;
	remap_cb();
}

void fp1000_exp_slot_device::main_cs_w(offs_t offset, u8 data)
{
	m_dev->cs_w(offset, data);
	m_dev->remap_cb();
}



device_fp1000_exp_interface::device_fp1000_exp_interface(const machine_config &mconfig, device_t &device)
   : device_interface(device, "fp1000exp")
{
	m_slot = dynamic_cast<fp1000_exp_slot_device *>(device.owner());
}

device_fp1000_exp_interface::~device_fp1000_exp_interface()
{
}

void device_fp1000_exp_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_fp1000_exp_interface::interface_post_start()
{
	m_slot->select_w(false);
}

// generic passthroughs
void device_fp1000_exp_interface::inta_w(int state) { m_slot->m_inta_cb(state); }
void device_fp1000_exp_interface::intb_w(int state) { m_slot->m_intb_cb(state); }
void device_fp1000_exp_interface::intc_w(int state) { m_slot->m_intc_cb(state); }
void device_fp1000_exp_interface::intd_w(int state) { m_slot->m_intd_cb(state); }


fp1000_exp_device::fp1000_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_fp1000_exp_interface(mconfig, *this)
{
}

void fp1000_exp_device::device_start()
{

}


// slot devices
#include "fp1060io.h"

void fp1000_exp_devices(device_slot_interface &device)
{
	device.option_add("fp1060io", FP1060IO);
	// TODO: other options, if any
}
