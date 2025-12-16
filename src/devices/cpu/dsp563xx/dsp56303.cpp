// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 56303

#include "emu.h"
#include "dsp56303.h"

ROM_START( dsp56303 )
	ROM_REGION32_LE( 0xc0*4, "boot", 0 )
	ROM_LOAD( "boot-303.bin", 0, 0xc0*4, CRC(331218e3) SHA1(a1728cfaf31919f619a6c4d579f198c5a4bb9e44))
ROM_END

dsp56303_device::dsp56303_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	dsp563xx_device(mconfig, DSP56303, tag, owner, clock,
					address_map_constructor(FUNC(dsp56303_device::p_map), this),
					address_map_constructor(FUNC(dsp56303_device::x_map), this),
					address_map_constructor(FUNC(dsp56303_device::y_map), this))
{
}

void dsp56303_device::device_start()
{
	dsp563xx_device::device_start();
}

u32 dsp56303_device::get_reset_vector() const
{
	return (m_omr & 15) == 0 ? 0xc00000 : (m_omr & 15) == 8 ? 0x008000 : 0xff0000;
}

void dsp56303_device::device_reset()
{
	dsp563xx_device::device_reset();
}

void dsp56303_device::device_add_mconfig(machine_config &config)
{
	HI08(config, m_hi08);
}

const tiny_rom_entry *dsp56303_device::device_rom_region() const
{
	return ROM_NAME(dsp56303);
}


void dsp56303_device::p_map(address_map &map)
{
	map(0x000000, 0x000fff).ram();
	map(0xff0000, 0xff00bf).rom().region("boot", 0);
}

void dsp56303_device::x_map(address_map &map)
{
	map(0x000000, 0x0007ff).ram();
	map(0xffffc2, 0xffffc7).m(m_hi08, FUNC(hi08_device::map));
}

void dsp56303_device::y_map(address_map &map)
{
	map(0x000000, 0x0007ff).ram();
}

DEFINE_DEVICE_TYPE(DSP56303, dsp56303_device, "dsp56303", "Motorola DSP56303")
