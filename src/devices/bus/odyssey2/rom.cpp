// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli, hap
/******************************************************************************

Standard cartridges emulation, optionally bankswitched up to 8KB.

******************************************************************************/

#include "emu.h"
#include "rom.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class o2_rom_device : public device_t, public device_o2_cart_interface
{
public:
	o2_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override;
	virtual u8 read_rom0c(offs_t offset) override { return read_rom04(offset + 0x400); }

	virtual void write_p1(u8 data) override { m_bank = data & 3; }

private:
	u32 m_cart_mask = 0;
	u8 m_bank = 0;
};

o2_rom_device::o2_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_STD, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this)
{ }

void o2_rom_device::device_start()
{
	save_item(NAME(m_bank));
}

void o2_rom_device::cart_init()
{
	m_cart_mask = (1 << (31 - count_leading_zeros_32(m_rom_size))) - 1;
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

u8 o2_rom_device::read_rom04(offs_t offset)
{
	offset = (offset + m_bank * 0x800) & m_cart_mask;
	return (offset < m_rom_size) ? m_rom[offset] : 0xff;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(O2_ROM_STD, device_o2_cart_interface, o2_rom_device, "o2_rom", "Odyssey 2 Standard Cartridge")
