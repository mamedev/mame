// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Videopac 31/40 cartridge emulation

4KB ROM (program ROM is fixed, data ROM is 'bankswitched' via P11)

Used in:
- #31: Musician
- #40: 4 in 1 Row

******************************************************************************/

#include "emu.h"
#include "4in1.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class o2_4in1_device : public device_t, public device_o2_cart_interface
{
public:
	o2_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override { return m_rom[offset + 0x400]; }
	virtual u8 read_rom0c(offs_t offset) override { return m_rom[offset + 0xc00]; }

	virtual void write_p1(u8 data) override { m_control = data; }
	virtual void write_p2(u8 data) override { m_bank = data; }
	virtual u8 io_read(offs_t offset) override;

private:
	u8 m_control = 0;
	u8 m_bank = 0;
};

o2_4in1_device::o2_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_4IN1, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this)
{ }

void o2_4in1_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_bank));
}

void o2_4in1_device::cart_init()
{
	if (m_rom_size != 0x1000)
		fatalerror("o2_4in1_device: ROM size must be 4KB\n");
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

u8 o2_4in1_device::io_read(offs_t offset)
{
	if (m_control & 2)
		return m_rom[m_bank << 8 | offset];
	else
		return 0xff;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(O2_ROM_4IN1, device_o2_cart_interface, o2_4in1_device, "o2_4in1", "Videopac 40 Cartridge")
