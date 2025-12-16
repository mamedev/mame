// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 56364

#include "emu.h"
#include "dsp56364.h"

ROM_START( dsp56364 )
	ROM_REGION32_LE( 0xc0*4, "boot", 0 )
	ROM_LOAD( "boot-364.bin", 0, 0xc0*4, CRC(2b508786) SHA1(55cb609a20cda6be20bf050c643c9fb089f58229) )
ROM_END

dsp56364_device::dsp56364_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	dsp563xx_device(mconfig, DSP56364, tag, owner, clock,
					address_map_constructor(FUNC(dsp56364_device::p_map), this),
					address_map_constructor(FUNC(dsp56364_device::x_map), this),
					address_map_constructor(FUNC(dsp56364_device::y_map), this))
{
}

void dsp56364_device::device_start()
{
	dsp563xx_device::device_start();
}

u32 dsp56364_device::get_reset_vector() const
{
	// Technically, modes 0 and 8 are undocumented and unknown
	return (m_omr & 15) == 0 ? 0xc00000 : (m_omr & 15) == 8 ? 0x008000 : 0xff0000;
}

void dsp56364_device::device_reset()
{
	dsp563xx_device::device_reset();
}

const tiny_rom_entry *dsp56364_device::device_rom_region() const
{
	return ROM_NAME(dsp56364);
}

void dsp56364_device::device_add_mconfig(machine_config &config)
{
	DSP5636X_SHI(config, m_shi);
}


void dsp56364_device::p_map(address_map &map)
{
	map(0x000000, 0x0001ff).ram();
	map(0xff0000, 0xff00bf).rom().region("boot", 0);
}

void dsp56364_device::x_map(address_map &map)
{
	map(0x000000, 0x0003ff).ram();
	map(0xffff90, 0xffff94).m(m_shi, FUNC(dsp5636x_shi_device::map));
}

void dsp56364_device::y_map(address_map &map)
{
	map(0x000000, 0x0005ff).ram();
}

DEFINE_DEVICE_TYPE(DSP56364, dsp56364_device, "dsp56364", "Motorola DSP56364")
