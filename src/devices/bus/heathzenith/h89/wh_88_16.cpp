// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heath WH-88-16 16K RAM expansion

  The CPU board holds at most 48K. This card adds the last 16K for a 64K
  system: the primary memory decoder selects it with RD6 when the CPU
  board's memory size jumpers are set for 64K.

  It plugs into a left slot. Heath's instructions install it in P503,
  but with a longer cable it works in any of P501 - P503.

****************************************************************************/

#include "emu.h"

#include "wh_88_16.h"


namespace {

class wh_88_16_device : public device_t, public device_h89bus_left_card_interface
{
public:
	wh_88_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// device_h89bus_left_card_interface implementation
	virtual u8 mem_select_lines() const override ATTR_COLD { return h89bus::MEM_PRI_RD6; }
	virtual u8 *mem_select(u8 select_lines, offs_t addr) override ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	memory_share_creator<u8> m_ram;
};


wh_88_16_device::wh_88_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H89BUS_WH_88_16, tag, owner, clock),
	device_h89bus_left_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x4000, ENDIANNESS_LITTLE)
{
}

u8 *wh_88_16_device::mem_select(u8 select_lines, offs_t addr)
{
	// A13 - A0 address the 16K
	return (select_lines & h89bus::MEM_PRI_RD6) ? &m_ram[addr & 0x3fff] : nullptr;
}

void wh_88_16_device::device_start()
{
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_WH_88_16, device_h89bus_left_card_interface, wh_88_16_device, "h89_wh_88_16", "Heath WH-88-16 16K RAM")
