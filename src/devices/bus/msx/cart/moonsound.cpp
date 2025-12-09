// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

TODO:
- Properly hook up correct SRAM sizes for different moonsound compatible
  cartridges. (Original moonsound has 128KB SRAM)

**********************************************************************************/

#include "emu.h"
#include "moonsound.h"

#include "sound/ymopl.h"

#include "speaker.h"


#define VERBOSE 0
#include "logmacro.h"

namespace {

class msx_cart_moonsound_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_moonsound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_MOONSOUND, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_ymf278b(*this, "ymf278b")
	{ }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void irq_w(int state);
	void write_ymf278b_pcm(offs_t offset, u8 data);
	u8 read_ymf278b_pcm(offs_t offset);
	u8 read_c0();
	void ymf278b_map(address_map &map) ATTR_COLD;

	required_device<ymf278b_device> m_ymf278b;
};

void msx_cart_moonsound_device::ymf278b_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x3fffff).ram();  // 2MB sram for testing
}

void msx_cart_moonsound_device::device_add_mconfig(machine_config &config)
{
	// The moonsound cartridge has a separate stereo output.
	SPEAKER(config, "speaker", 2).front();

	YMF278B(config, m_ymf278b, 33.8688_MHz_XTAL);
	m_ymf278b->set_addrmap(0, &msx_cart_moonsound_device::ymf278b_map);
	m_ymf278b->irq_handler().set(FUNC(msx_cart_moonsound_device::irq_w));
	m_ymf278b->add_route(0, "speaker", 0.50, 0);
	m_ymf278b->add_route(1, "speaker", 0.50, 1);
	m_ymf278b->add_route(2, "speaker", 0.40, 0);
	m_ymf278b->add_route(3, "speaker", 0.40, 1);
	m_ymf278b->add_route(4, "speaker", 0.50, 0);
	m_ymf278b->add_route(5, "speaker", 0.50, 1);
}

ROM_START(msx_cart_moonsound)
	ROM_REGION(0x400000, "ymf278b", 0)
	ROM_LOAD("yrw801.rom", 0x0, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca))
ROM_END

const tiny_rom_entry *msx_cart_moonsound_device::device_rom_region() const
{
	return ROM_NAME(msx_cart_moonsound);
}

void msx_cart_moonsound_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x7e, 0x7f, emu::rw_delegate(*this, FUNC(msx_cart_moonsound_device::read_ymf278b_pcm)), emu::rw_delegate(*this, FUNC(msx_cart_moonsound_device::write_ymf278b_pcm)));
	io_space().install_readwrite_handler(0xc4, 0xc7, emu::rw_delegate(m_ymf278b, FUNC(ymf278b_device::read)), emu::rw_delegate(m_ymf278b, FUNC(ymf278b_device::write)));
	io_space().install_read_handler(0xc0, 0xc0, emu::rw_delegate(*this, FUNC(msx_cart_moonsound_device::read_c0)));
}

void msx_cart_moonsound_device::device_reset()
{
}

void msx_cart_moonsound_device::irq_w(int state)
{
	LOG("moonsound: irq state %d\n", state);
	irq_out(state);
}

void msx_cart_moonsound_device::write_ymf278b_pcm(offs_t offset, u8 data)
{
	LOG("moonsound: write 0x%02x, data 0x%02x\n", 0x7e + offset, data);
	m_ymf278b->write(4 + offset, data);
}

u8 msx_cart_moonsound_device::read_ymf278b_pcm(offs_t offset)
{
	LOG("moonsound: read 0x%02x\n", 0x7e + offset);
	return m_ymf278b->read(4 + offset);
}

// For detecting presence of moonsound cartridge
u8 msx_cart_moonsound_device::read_c0()
{
	LOG("moonsound: read 0xc0\n");
	return 0x00;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MOONSOUND, msx_cart_interface, msx_cart_moonsound_device, "msx_moonsound", "MSX Cartridge - MoonSound")
