// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    PC-8801 EXPansion bus

	List of known cards:
	\- NEC PC-8801-10
		(MIDI interface);
	\- NEC PC-8801-11
		("Sound Board", single YM2203C OPN, single joy port, mono out);
	\- NEC PC-8801-12
		(Modem board, full duplex 300bps);
	\- NEC PC-8801-13
		(Parallel I/F board);
	\- NEC PC-8801-17 / -18
		(VTR capture card "Video art board" / "Video digitizing unit", 16-bit color);
	\- NEC PC-8801-21
		(CMT i/f board);
	\- NEC PC-8801-22
		("Multi board B", upgrades a FH to MH and FA to MA (?));
	\- NEC PC-8801-23 & -24 & -25
		("Sound Board 2", single YM2608 OPNA, single joy port, stereo out.
		-24 is the internal FH / MH version, -25 is the internal FE / FE2 with YM2608B.
		Standard and on main board instead for FA / MA and onward);
	\- NEC PC-8801-30 & -31
		(-31 is CD-ROM SCSI i/f, -30 is virtually same to PC Engine CD-ROM² drive)
	\- NEC PC-8864
		(Network board mapping at $a0-$a3)
	\- HAL PCG-8100
		(PCG and 3x DAC_1BIT at I/O $01, $02. PIT at $0c-$0f)
	\- HAL GSX-8800
		(2x PSG at I/O $a0-$a3, mono out. Has goofy extra connector on top and a couple jumpers,
		 guess it may cascade with another board for 2x extra PSGs at $a4-$a7);
	\- HIBIKI-8800
		(YM2151 OPM + YM3802-X MIDI controller, stereo out, has own internal XTAL @ 4MHz.
		 Has an undumped PAL/PROM labeled "HAL-881");
	\- HAL HMB-20
		(same as HIBIKI-8800 board?)
	\- JMB-X1
		("Sound Board X", 2x OPM + 1x SSG. Used by NRTDRV, more info at GH #8709);

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

void pc8801_exp_slot_device::device_resolve_objects()
{
	m_int3_cb.resolve_safe();
	m_int4_cb.resolve_safe();
	m_int5_cb.resolve_safe();
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
WRITE_LINE_MEMBER( device_pc8801_exp_interface::int3_w ) { m_slot->m_int3_cb(state); }
WRITE_LINE_MEMBER( device_pc8801_exp_interface::int4_w ) { m_slot->m_int4_cb(state); }
WRITE_LINE_MEMBER( device_pc8801_exp_interface::int5_w ) { m_slot->m_int5_cb(state); }

pc8801_exp_device::pc8801_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, type, tag, owner, clock)
	, device_pc8801_exp_interface(mconfig, *this)
{
}

void pc8801_exp_device::device_start()
{
	
}



// slot devices
#include "pcg8100.h"
#include "pc8801_23.h"
#include "jmbx1.h"

void pc8801_exp_devices(device_slot_interface &device)
{
	device.option_add("sbii", PC8801_23);

	device.option_add("pcg8100", PCG8100);
	device.option_add("jmbx1", JMBX1);
}
