// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

TODO:
- Properly hook up correct SRAM sizes for different moonsound compatible
  cartridges. (Original moonsound has 128KB SRAM)

**********************************************************************************/

#include "emu.h"
#include "moonsound.h"

#include "speaker.h"


#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MSX_CART_MOONSOUND, msx_cart_moonsound_device, "msx_moonsound", "MSX Cartridge - MoonSound")


msx_cart_moonsound_device::msx_cart_moonsound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_MOONSOUND, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_ymf278b(*this, "ymf278b")
{
}


void msx_cart_moonsound_device::ymf278b_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x3fffff).ram();  // 2MB sram for testing
}


void msx_cart_moonsound_device::device_add_mconfig(machine_config &config)
{
	// The moonsound cartridge has a separate stereo output.
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	YMF278B(config, m_ymf278b, YMF278B_STD_CLOCK);
	m_ymf278b->set_addrmap(0, &msx_cart_moonsound_device::ymf278b_map);
	m_ymf278b->irq_handler().set(FUNC(msx_cart_moonsound_device::irq_w));
	m_ymf278b->add_route(0, "lspeaker", 0.50);
	m_ymf278b->add_route(1, "rspeaker", 0.50);
	m_ymf278b->add_route(2, "lspeaker", 0.40);
	m_ymf278b->add_route(3, "rspeaker", 0.40);
	m_ymf278b->add_route(4, "lspeaker", 0.40);
	m_ymf278b->add_route(5, "rspeaker", 0.40);
}


ROM_START( msx_cart_moonsound )
	ROM_REGION(0x400000, "ymf278b", 0)
	ROM_LOAD("yrw801.rom", 0x0, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca))
ROM_END


const tiny_rom_entry *msx_cart_moonsound_device::device_rom_region() const
{
	return ROM_NAME( msx_cart_moonsound );
}


void msx_cart_moonsound_device::device_start()
{
	m_out_irq_cb.resolve_safe();

	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_readwrite_handler(0x7e, 0x7f, read8_delegate(FUNC(msx_cart_moonsound_device::read_ymf278b_pcm), this), write8_delegate(FUNC(msx_cart_moonsound_device::write_ymf278b_pcm), this));
	space.install_readwrite_handler(0xc4, 0xc7, read8_delegate(FUNC(msx_cart_moonsound_device::read_ymf278b_fm), this), write8_delegate(FUNC(msx_cart_moonsound_device::write_ymf278b_fm), this));
	space.install_read_handler(0xc0, 0xc0, read8_delegate(FUNC(msx_cart_moonsound_device::read_c0), this));
}


void msx_cart_moonsound_device::device_reset()
{
}


WRITE_LINE_MEMBER(msx_cart_moonsound_device::irq_w)
{
	LOG("moonsound: irq state %d\n", state);
	m_out_irq_cb(state);
}


WRITE8_MEMBER(msx_cart_moonsound_device::write_ymf278b_fm)
{
	LOG("moonsound: write 0x%02x, data 0x%02x\n", 0xc4 + offset, data);
	m_ymf278b->write(offset, data);
}


READ8_MEMBER(msx_cart_moonsound_device::read_ymf278b_fm)
{
	LOG("moonsound: read 0x%02x\n", 0xc4 + offset);
	return m_ymf278b->read(offset);
}


WRITE8_MEMBER(msx_cart_moonsound_device::write_ymf278b_pcm)
{
	LOG("moonsound: write 0x%02x, data 0x%02x\n", 0x7e + offset, data);
	m_ymf278b->write(4 + offset, data);
}


READ8_MEMBER(msx_cart_moonsound_device::read_ymf278b_pcm)
{
	LOG("moonsound: read 0x%02x\n", 0x7e + offset);
	return m_ymf278b->read(4 + offset);
}


// For detecting presence of moonsound cartridge
READ8_MEMBER(msx_cart_moonsound_device::read_c0)
{
	LOG("moonsound: read 0xc0\n");
	return 0x00;
}
