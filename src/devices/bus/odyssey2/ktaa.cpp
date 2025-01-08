// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli, hap
/******************************************************************************

Homebrew KTAA(Kill the Attacking Aliens) cartridge emulation

Bankswitched ROM with page size of 3KB.

******************************************************************************/

#include "emu.h"
#include "ktaa.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class o2_ktaa_device : public device_t, public device_o2_cart_interface
{
public:
	o2_ktaa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override;
	virtual u8 read_rom0c(offs_t offset) override { return read_rom04(offset + 0x800); }

	virtual void write_p1(u8 data) override { m_bank = data & m_bank_mask; }

private:
	u32 m_page_size = 0;
	u8 m_bank_mask = 0;
	u8 m_bank = 0;
};

o2_ktaa_device::o2_ktaa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_KTAA, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this)
{ }

void o2_ktaa_device::device_start()
{
	save_item(NAME(m_bank));
}

void o2_ktaa_device::cart_init()
{
	bool err = false;

	if (m_rom_size & (m_rom_size - 1))
	{
		// freely released binary file is 12KB
		err = m_rom_size != 0xc00 && m_rom_size != 0xc00*2 && m_rom_size != 0xc00*4;
		m_page_size = 0xc00;
	}
	else
	{
		// actual ROM is 16KB(27C128), first 1KB of each 4KB block is empty
		err = m_rom_size < 0x1000;
		m_page_size = 0x1000;
	}

	if (err)
		fatalerror("o2_ktaa_device: ROM size must be multiple of 3KB or 4KB\n");

	m_bank_mask = (m_rom_size / m_page_size) - 1;
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

u8 o2_ktaa_device::read_rom04(offs_t offset)
{
	offset += m_page_size - 0xc00;
	return m_rom[offset + m_bank * m_page_size];
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(O2_ROM_KTAA, device_o2_cart_interface, o2_ktaa_device, "o2_ktaa", "Videopac+ KTAA Cartridge")
