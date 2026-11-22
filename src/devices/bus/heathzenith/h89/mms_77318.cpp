// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Magnolia Microsystems 77318 128K RAM board (10-77318-2)

  Two banks of 64K, selected by RD6 (bank A) and RD7 (bank B). It plugs into
  P503 and takes the place of the 16K expansion, with the 48K on the CPU
  board. With a long enough cable to the control board it fits in any of
  P501 - P503. The bank select control board (10-77318-1) in the U517 socket
  gives the extra 112K as two 56K pages; with a Heath PROM it is a 16K
  expansion.

  A15 - A0 address a bank. CTL from the control board inverts the RAMs' A0
  line, which carries A6 and A13, so the control board's PROM can move an 8K
  block to the other half of its 16K. Following the schematics, the pages are
    page 0 (the standard 64K)  bank A, 0xc000 - 0xffff
    page 1                     bank B, 0x0000 - 0xdfff
    page 2                     bank A, 0x0000 - 0xbfff, and bank B,
                               0xe000 - 0xffff (CTL) for 0xc000 - 0xdfff

****************************************************************************/

#include "emu.h"

#include "mms_77318.h"


namespace {

class mms_77318_device : public device_t, public device_h89bus_left_card_interface
{
public:
	mms_77318_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// device_h89bus_left_card_interface implementation
	virtual u8 mem_select_lines() const override ATTR_COLD { return h89bus::MEM_PRI_RD6 | h89bus::MEM_PRI_RD7; }
	virtual u8 *mem_select(u8 select_lines, offs_t addr) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	memory_share_creator<u8> m_ram;
};


mms_77318_device::mms_77318_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H89BUS_MMS_77318, tag, owner, clock),
	device_h89bus_left_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x20000, ENDIANNESS_LITTLE)
{
}

u8 *mms_77318_device::mem_select(u8 select_lines, offs_t addr)
{
	// CTL inverts A6 as well, but bank B's top 8K is only reached with CTL, so that can't be
	// seen and only A13 is inverted
	offs_t const bank_addr = ((select_lines & h89bus::MEM_PRI_CTL) ? (addr ^ 0x2000) : addr) & 0xffff;

	// the control board never selects both banks, which would both drive the data bus
	if (select_lines & h89bus::MEM_PRI_RD6)
		return &m_ram[bank_addr];

	if (select_lines & h89bus::MEM_PRI_RD7)
		return &m_ram[0x10000 | bank_addr];

	return nullptr;
}

void mms_77318_device::device_start()
{
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MMS_77318, device_h89bus_left_card_interface, mms_77318_device, "h89_mms_77318", "Magnolia Microsystems 77318 128K RAM")
