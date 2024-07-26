// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

FP-1060I/O Expansion Slot

List of known devices:
- FP-1020FD2 (FDC uPD765, 2x 5.25 floppy DSDD, id = 0x04)
- FP-1021FD1 (FDC, 1x 5.25 floppy FP-200 compatible 70kb (?))
- FP-1022FD1 (FDC, 1x 5.25)
- FP-1023FD1 (Same as 1020 with 1x drive?)
- FP-1024FD2 (FDC, 2x floppy 2HD)
- FP-1030 (RAMPACK, id = 0x01)
- FP-1031 (ROMPACK, id = 0x00)
\- One of the ROMPACKs has undumped Test Mode (cfr. page 94 of service manual)
- FP-1032K (Kanji ROM pack, unknown id)
- FP-1035RS (RS-232C, id = 0x02)

**************************************************************************************************/

#include "emu.h"
#include "fp1060io_exp.h"


DEFINE_DEVICE_TYPE(FP1060IO_EXP_SLOT, fp1060io_exp_slot_device, "fp1060io_exp_slot", "FP-1060I/O Expansion Slot")

fp1060io_exp_slot_device::fp1060io_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FP1060IO_EXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_fp1060io_exp_interface>(mconfig, *this)
	, m_inta_cb(*this)
	, m_intb_cb(*this)
	, m_intc_cb(*this)
	, m_intd_cb(*this)
{
}

fp1060io_exp_slot_device::~fp1060io_exp_slot_device()
{
}

void fp1060io_exp_slot_device::device_start()
{
}

void fp1060io_exp_slot_device::device_config_complete()
{
	m_dev = get_card_device();
}


device_fp1060io_exp_interface::device_fp1060io_exp_interface(const machine_config &mconfig, device_t &device)
   : device_interface(device, "fp1060ioexp")
{
	m_slot = dynamic_cast<fp1060io_exp_slot_device *>(device.owner());
}

device_fp1060io_exp_interface::~device_fp1060io_exp_interface()
{
}

void device_fp1060io_exp_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_fp1060io_exp_interface::interface_post_start()
{
	// Dynamic mapping, shouldn't need anything from here
}

void device_fp1060io_exp_interface::inta_w(int state) { m_slot->m_inta_cb(state); }
void device_fp1060io_exp_interface::intb_w(int state) { m_slot->m_intb_cb(state); }
void device_fp1060io_exp_interface::intc_w(int state) { m_slot->m_intc_cb(state); }
void device_fp1060io_exp_interface::intd_w(int state) { m_slot->m_intd_cb(state); }


fp1060io_exp_device::fp1060io_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_fp1060io_exp_interface(mconfig, *this)
{
}

void fp1060io_exp_device::device_start()
{

}


#include "fp1020fd.h"
#include "fp1030_rampack.h"

void fp1060io_slot_devices(device_slot_interface &device)
{
	device.option_add("fdcpack", FP1020FD);
	device.option_add("rampack", FP1030_RAMPACK);
}

