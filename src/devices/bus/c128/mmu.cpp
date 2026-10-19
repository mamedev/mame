// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 128 MMU socket emulation

**********************************************************************/

#include "emu.h"
#include "mmu.h"

#include "supercpu128_mmu.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C128_MMU_SLOT, c128_mmu_slot_device, "c128_mmu_slot", "C128 MMU socket")
DEFINE_DEVICE_TYPE(C128_MMU_MOS8722, c128_mmu_mos8722_device, "c128_mmu_mos8722", "C128 MOS 8722 MMU")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_c128_mmu_interface::device_c128_mmu_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "c128mmu"),
	m_slot(dynamic_cast<c128_mmu_slot_device *>(device.owner()))
{
}



//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

c128_mmu_slot_device::c128_mmu_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C128_MMU_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_c128_mmu_interface>(mconfig, *this),
	m_write_z80en(*this),
	m_write_fsdir(*this),
	m_read_game(*this, 1),
	m_read_exrom(*this, 1),
	m_read_sense40(*this, 1),
	m_write_port(*this),
	m_write_drive(*this),
	m_read_caps(*this, 1),
	m_card(nullptr)
{
}

void c128_mmu_slot_device::device_start()
{
	m_card = get_card_device();

	if (!m_card)
		throw emu_fatalerror("%s: the MMU socket must not be empty\n", tag());
}



//**************************************************************************
//  MOS 8722 MMU
//**************************************************************************

c128_mmu_mos8722_device::c128_mmu_mos8722_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c128_mmu_mos8722_device(mconfig, C128_MMU_MOS8722, tag, owner, clock)
{
}

c128_mmu_mos8722_device::c128_mmu_mos8722_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_c128_mmu_interface(mconfig, *this),
	m_mmu(*this, "u7")
{
}

void c128_mmu_mos8722_device::device_add_mconfig(machine_config &config)
{
	MOS8722(config, m_mmu, DERIVED_CLOCK(1, 1));
	m_mmu->z80en().set(*m_slot, FUNC(c128_mmu_slot_device::z80en_w));
	m_mmu->fsdir().set(*m_slot, FUNC(c128_mmu_slot_device::fsdir_w));
	m_mmu->game().set(*m_slot, FUNC(c128_mmu_slot_device::game_r));
	m_mmu->exrom().set(*m_slot, FUNC(c128_mmu_slot_device::exrom_r));
	m_mmu->sense40().set(*m_slot, FUNC(c128_mmu_slot_device::sense40_r));
}

uint8_t c128_mmu_mos8722_device::read(offs_t offset, uint8_t data)
{
	return m_mmu->read(offset, data);
}

void c128_mmu_mos8722_device::write(offs_t offset, uint8_t data)
{
	m_mmu->write(offset, data);
}

offs_t c128_mmu_mos8722_device::ta_r(offs_t offset, int aec, int *ms0, int *ms1, int *ms2, int *ms3, int *cas0, int *cas1)
{
	return m_mmu->ta_r(offset, aec, ms0, ms1, ms2, ms3, cas0, cas1);
}

int c128_mmu_mos8722_device::fsdir_r()
{
	return m_mmu->fsdir_r();
}



//**************************************************************************
//  CARDS
//**************************************************************************

void c128_mmu_cards(device_slot_interface &device)
{
	device.option_add("mos8722", C128_MMU_MOS8722);
	device.option_add("supercpu128", CMD_MMU_SUPERADAPTER);
}
