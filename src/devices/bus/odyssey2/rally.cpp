// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Videopac+ 55/58/59/60 cartridge emulation

Bankswitched ROM via latch, either 16KB/2*8KB, or 4KB+8KB.

Used in:
- #55: Neutron Star
- #58: Air Battle
- #59: Helicopter Rescue
- #60: Trans American Rally

#55 and #58 also work on the G7000.

******************************************************************************/

#include "emu.h"
#include "rally.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class o2_rally_device : public device_t, public device_o2_cart_interface
{
public:
	o2_rally_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override;
	virtual u8 read_rom0c(offs_t offset) override { return read_rom04(offset + 0x400); }

	virtual void write_p1(u8 data) override { m_control = data; }
	virtual void io_write(offs_t offset, u8 data) override;

private:
	u8 m_control = 0;
	u8 m_bank = 0;
};

o2_rally_device::o2_rally_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_RALLY, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this)
{ }

void o2_rally_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_bank));
}

void o2_rally_device::cart_init()
{
	if (m_rom_size & (m_rom_size - 1) || m_rom_size < 0x800)
		fatalerror("o2_rally_device: ROM size must be 2^x\n");
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

u8 o2_rally_device::read_rom04(offs_t offset)
{
	// P10 enables latch output
	u8 bank = (m_control & 1) ? ~0 : m_bank;
	return m_rom[(offset + bank * 0x800) & (m_rom_size - 1)];
}

void o2_rally_device::io_write(offs_t offset, u8 data)
{
	if (offset & 0x80 && ~m_control & 0x10)
		m_bank = data;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(O2_ROM_RALLY, device_o2_cart_interface, o2_rally_device, "o2_rally", "Videopac+ 60 Cartridge")
