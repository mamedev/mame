// license:GPL-2.0+
// copyright-holders:smf
/*
  |--|-------------------------|--|
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  | PC Card slot 3 (bottom) |  |
  |  | PC Card slot 4 (top)    |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |                         |  |
  |  |-------------------------|  |
  |                               |
  |                               |
  |                               |
  |                               |
  |                               |
  |                               |
  |                               |
  |                               |
  |                               |
  |  74LS245   74LS245   74LS245  |
  |                               |
  |  74LS245   74LS245   74LS245  |
  |                               |
  |                               |
  | BD-8 BD-8 BD-8 BD-8 BD-8 BD-8 |
  |                               |
  |                               |
  |                               |
  |                               |
  |                               |
  |    CN1           CN2          |
  |-------------------------------|
  Notes:
        CN1 68 pin VHDCI connector
        CN2 (unpopulated)

  Requires ADAPTER PC Card PN 0000088797 & cable
*/

#include "emu.h"
#include "konami_dual.h"
#include "linflash.h"

konami_dual_pccard_device::konami_dual_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KONAMI_DUAL_PCCARD, tag, owner, clock),
	device_pccard_interface(mconfig, *this),
	m_slot(*this, "slot%u", 3U)
{
}

void konami_dual_pccard_device::device_add_mconfig(machine_config &config)
{
	for (size_t i = 0; i < m_slot.size(); i++)
	{
		PCCARD_SLOT(config, m_slot[i]);
		m_slot[i]->option_add("16mb", FUJITSU_16MB_FLASH_CARD);
		m_slot[i]->option_add("32mb", FUJITSU_32MB_FLASH_CARD);
		m_slot[i]->option_add("id245p01", ID245P01);
		m_slot[i]->option_add("pm24276", PM24276);
		m_slot[i]->set_default_option("32mb");
	}
}

void konami_dual_pccard_device::device_start()
{
	m_cd1_cb(0);
	m_cd2_cb(0);
}

uint16_t konami_dual_pccard_device::read_memory(offs_t offset, uint16_t mem_mask)
{
	return m_slot[BIT(offset, 24)]->read_memory(BIT(offset, 0, 24), mem_mask);
}

void konami_dual_pccard_device::write_memory(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_slot[BIT(offset, 24)]->write_memory(BIT(offset, 0, 24), data, mem_mask);
}

DEFINE_DEVICE_TYPE(KONAMI_DUAL_PCCARD, konami_dual_pccard_device, "konami_dual_pccard", "Konami Dual PC Card Adapter (PWB0000085455)")
