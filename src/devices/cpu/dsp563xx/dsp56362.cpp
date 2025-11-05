// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 56362

#include "emu.h"
#include "dsp56362.h"

ROM_START( dsp56362 )
	ROM_REGION32_LE( 0xc0*4, "boot", 0 )
	ROM_LOAD( "boot-362.bin", 0, 0xc0*4, CRC(fa69796b) SHA1(c18cbc0fb01ad423f99eb6421a36f3915e37e843) )
ROM_END

dsp56362_device::dsp56362_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	dsp563xx_device(mconfig, DSP56362, tag, owner, clock,
					address_map_constructor(FUNC(dsp56362_device::p_map), this),
					address_map_constructor(FUNC(dsp56362_device::x_map), this),
					address_map_constructor(FUNC(dsp56362_device::y_map), this))
{
}

void dsp56362_device::device_start()
{
	dsp563xx_device::device_start();
}

u32 dsp56362_device::get_reset_vector() const
{
	return m_omr == 0 ? 0xc00000 : m_omr == 8 ? 0x008000 : 0xff0000;
}

void dsp56362_device::device_reset()
{
	dsp563xx_device::device_reset();
}

const tiny_rom_entry *dsp56362_device::device_rom_region() const
{
	return ROM_NAME(dsp56362);
}

void dsp56362_device::device_add_mconfig(machine_config &config)
{
	HI08(config, m_hi08);
	DSP5636X_SHI(config, m_shi);
}


void dsp56362_device::p_map(address_map &map)
{
	map(0x000000, 0x000bff).ram();
	map(0xff0000, 0xff00bf).rom().region("boot", 0);
}

void dsp56362_device::x_map(address_map &map)
{
	map(0x000000, 0x0015ff).ram();
	map(0xffff90, 0xffff94).m(m_shi, FUNC(dsp5636x_shi_device::map));
	map(0xffffc2, 0xffffc7).m(m_hi08, FUNC(hi08_device::map));
}

void dsp56362_device::y_map(address_map &map)
{
	map(0x000000, 0x0015ff).ram();
}

DEFINE_DEVICE_TYPE(DSP56362, dsp56362_device, "dsp56362", "Motorola DSP56362")
