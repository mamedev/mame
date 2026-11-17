// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 card cage emulation

    The daisy chain runs through the slots in physical order. A board
    that uses no interrupts straps IEIN to IEOUT, and an empty slot
    breaks the chain for every slot after it.

**********************************************************************/

#include "emu.h"
#include "slot.h"

#include "cage.h"
#include "fdc.h"
#include "hdc.h"
#include "hdi.h"
#include "m68k.h"
#include "mem68k.h"
#include "ram.h"
#include "vidkb.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(TRS80M2_BUS, trs80m2_bus_device, "trs80m2_bus", "Tandy Model II bus")
DEFINE_DEVICE_TYPE(TRS80M2_SLOT, trs80m2_slot_device, "trs80m2_slot", "Tandy Model II card cage slot")


//**************************************************************************
//  BUS
//**************************************************************************

trs80m2_bus_device::trs80m2_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TRS80M2_BUS, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_page(*this, "page"),
	m_write_int(*this),
	m_write_nmi(*this),
	m_write_kbirq(*this),
	m_write_xferrq(*this),
	m_write_useirq(*this),
	m_write_page(*this),
	m_tap_installed(false),
	m_claimed(0),
	m_sources(0),
	m_int(0),
	m_nmi(0),
	m_kbirq(0),
	m_xferrq(0),
	m_useirq(0),
	m_ff(0),
	m_lowp(0)
{
}

void trs80m2_bus_device::page_map(address_map &map)
{
	map(0x8000, 0xffff).view(m_page);

	for (u32 i = 0; i < PAGE_ENTRIES; i++)
		m_page[i];
}

void trs80m2_bus_device::device_start()
{
	save_item(NAME(m_int));
	save_item(NAME(m_nmi));
	save_item(NAME(m_kbirq));
	save_item(NAME(m_xferrq));
	save_item(NAME(m_useirq));
	save_item(NAME(m_ff));
	save_item(NAME(m_lowp));
}

void trs80m2_bus_device::device_reset()
{
	if (!m_tap_installed)
	{
		m_tap_installed = true;
		m_ff_tap = io().install_write_tap(0xff, 0xff, "page_ff", [this] (offs_t offset, uint8_t &data, uint8_t mem_mask) { ff_w(offset, data, mem_mask); });
	}

	m_ff = 0;
	update_page();
}

void trs80m2_bus_device::ff_w(offs_t offset, uint8_t &data, uint8_t mem_mask)
{
	m_ff = data;
	update_page();
}

void trs80m2_bus_device::lowp_w(int state)
{
	m_lowp = state ? 1 : 0;
	update_page();
}

void trs80m2_bus_device::update_page()
{
	u32 const index = entry(m_lowp, m_ff);

	m_page.select(index);
	m_write_page(index);
}

void trs80m2_bus_device::install_page_ram(int page, offs_t end, uint8_t *ram)
{
	for (u32 i = 0; i < PAGE_ENTRIES; i++)
	{
		if (entry_page(i) != page)
			continue;

		offs_t const top = entry_vram(i) ? std::min<offs_t>(end, 0xf7ff) : end;
		m_page[i].install_ram(0x8000, top, ram);

		if (top >= 0x8000)
			claim_window(i);
	}
}

bool trs80m2_bus_device::wait(int cycles, const void *tag)
{
	if (machine().side_effects_disabled() || machine().scheduler().currently_executing() != m_cpu.target())
		return false;

	return m_cpu->access_before_delay(cycles, tag);
}

attotime trs80m2_bus_device::local_time() const
{
	return m_cpu->local_time();
}

void trs80m2_bus_device::m68k_map(address_space &space)
{
	for (device_trs80m2_card_interface *card : m_cards)
		card->m68k_map(space);
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_trs80m2_card_interface::device_trs80m2_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "trs80m2"),
	m_slot(nullptr)
{
}

void device_trs80m2_card_interface::interface_pre_start()
{
	m_slot = dynamic_cast<trs80m2_slot_device *>(device().owner());

	if (!m_slot->started())
		throw device_missing_dependencies();

	m_slot->bus().add_card(*this);
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

void device_trs80m2_card_interface::m68k_map(address_space &space)
{
}


//**************************************************************************
//  SLOT
//**************************************************************************

trs80m2_slot_device::trs80m2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TRS80M2_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_trs80m2_card_interface>(mconfig, *this),
	device_z80daisy_interface(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG),
	m_card(nullptr),
	m_source(0)
{
}

void trs80m2_slot_device::device_start()
{
	m_card = get_card_device();
	m_source = m_bus->add_source();
}

int trs80m2_slot_device::z80daisy_irq_state()
{
	return m_card ? m_card->z80daisy_irq_state() : Z80_DAISY_IEO;
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


//**************************************************************************
//  SLOT OPTIONS
//**************************************************************************

void trs80m2_cards(device_slot_interface &device)
{
	device.option_add("fdc", TRS80M2_FDC);
	device.option_add("vidkb", TRS80M2_VIDKB);
	device.option_add("vidkb12", TRS80M12_VIDKB);
	device.option_add("ram32k", TRS80M2_RAM32K);
	device.option_add("ram64k", TRS80M2_RAM64K);
	device.option_add("hdi", TRS80M2_HDI);
	device.option_add("hdc", TRS80M2_HDC);
	device.option_add("m16", TRS80M2_M68K_M16);
	device.option_add("m16b", TRS80M2_M68K_M16B);
	device.option_add("t6000", TRS80M2_M68K_T6000);
	device.option_add("128k", TRS80M2_MEM128K);
	device.option_add("256k", TRS80M2_MEM256K);
	device.option_add("512k", TRS80M2_MEM512K);
	device.option_add("1m", TRS80M2_MEM1M);
}

void trs80m12_cards(device_slot_interface &device)
{
	device.option_add("vidkb", TRS80M2_VIDKB);
	device.option_add("vidkb12", TRS80M12_VIDKB);
	device.option_add("hdi", TRS80M2_HDI);
	device.option_add("hdc", TRS80M2_HDC);
	device.option_add("m16", TRS80M2_M68K_M16);
	device.option_add("m16b", TRS80M2_M68K_M16B);
	device.option_add("t6000", TRS80M2_M68K_T6000);
	device.option_add("128k", TRS80M2_MEM128K);
	device.option_add("256k", TRS80M2_MEM256K);
	device.option_add("512k", TRS80M2_MEM512K);
	device.option_add("1m", TRS80M2_MEM1M);
}

void trs80m12_j0_cards(device_slot_interface &device)
{
	device.option_add("vidkb", TRS80M2_VIDKB);
	device.option_add("vidkb12", TRS80M12_VIDKB);
	device.option_add("cage", TRS80M12_CAGE);
}
