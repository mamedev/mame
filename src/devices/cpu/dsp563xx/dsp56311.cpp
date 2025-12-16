// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 56311

#include "emu.h"
#include "dsp56311.h"

ROM_START( dsp56311 )
	ROM_REGION32_LE( 0xc0*4, "boot", 0 )
	ROM_LOAD( "boot-311.bin", 0, 0xc0*4, CRC(0bf4780e) SHA1(09a704dfb8be9e5863cb0523994ce5b4fb225801) )
ROM_END

dsp56311_device::dsp56311_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	dsp563xx_device(mconfig, DSP56311, tag, owner, clock,
					address_map_constructor(FUNC(dsp56311_device::p_map), this),
					address_map_constructor(FUNC(dsp56311_device::x_map), this),
					address_map_constructor(FUNC(dsp56311_device::y_map), this))
{
}

void dsp56311_device::device_start()
{
	dsp563xx_device::device_start();
}

u32 dsp56311_device::get_reset_vector() const
{
	return (m_omr & 15) == 0 ? 0xc00000 : (m_omr & 15) == 8 ? 0x008000 : 0xff0000;
}

void dsp56311_device::device_reset()
{
	dsp563xx_device::device_reset();
}

void dsp56311_device::device_add_mconfig(machine_config &config)
{
	HI08(config, m_hi08);
}

const tiny_rom_entry *dsp56311_device::device_rom_region() const
{
	return ROM_NAME(dsp56311);
}


void dsp56311_device::p_map(address_map &map)
{
	map(0x000000, 0x007fff).ram();
	map(0xff0000, 0xff00bf).rom().region("boot", 0);
}

void dsp56311_device::x_map(address_map &map)
{
	map(0x000000, 0x00bfff).ram();
	map(0xffffc2, 0xffffc7).m(m_hi08, FUNC(hi08_device::map));
}

void dsp56311_device::y_map(address_map &map)
{
	map(0x000000, 0x00bfff).ram();
}

DEFINE_DEVICE_TYPE(DSP56311, dsp56311_device, "dsp56311", "Motorola DSP56311")
