// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

/*************************************************************************************

    Yamaha XT446 : 32-voice polyphonic/multitimbral General MIDI/GS/XG tone modules

    Embedded version of the MU100B

**************************************************************************************/

#include "emu.h"

#include "xt446.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"

DEFINE_DEVICE_TYPE(XT446, xt446_device, "xt446", "Yamaha XT446 synth (embedded MU100B)")

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

ROM_START( xt446 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MU-100B v1.08 (Nov. 28, 1997)
	ROM_LOAD16_WORD_SWAP( "xu50710-m27c160.bin", 0x000000, 0x200000, CRC(4b10bd27) SHA1(12d7c6e1bce7974b34916e1bfa5057ab55867476) )

	ROM_REGION32_LE( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x0000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x0000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x0800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x0800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )
	ROM_LOAD32_WORD( "xt462a0.ic39", 0x1000000, 0x400000, CRC(cbf037da) SHA1(37449e741243305de38cb913b17041942ad334cd) )
	ROM_LOAD32_WORD( "xt463a0.ic38", 0x1000002, 0x400000, CRC(cce5f8d3) SHA1(bdca8c5158f452f2b5535c7d658c9b22c6d66048) )
ROM_END

xt446_device::xt446_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XT446, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_swp30(*this, "swp30")
{
}

const tiny_rom_entry *xt446_device::device_rom_region() const
{
	return ROM_NAME(xt446);
}

void xt446_device::device_start()
{
}

void xt446_device::device_reset()
{
}

void xt446_device::xt446_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x21ffff).ram(); // 128K work RAM
	map(0x400000, 0x401fff).m(m_swp30, FUNC(swp30_device::map));
}

void xt446_device::swp30_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("swp30",         0).mirror(0x200000);
	map(0x400000, 0x4fffff).rom().region("swp30",  0x800000).mirror(0x300000);
	map(0x800000, 0x9fffff).rom().region("swp30", 0x1000000).mirror(0x200000);
}

void xt446_device::device_add_mconfig(machine_config &config)
{
	H8S2655(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &xt446_device::xt446_map);
	m_maincpu->read_adc<0>().set_constant(0);
	m_maincpu->read_adc<1>().set_constant(0);
	m_maincpu->read_adc<2>().set_constant(0);
	m_maincpu->read_adc<3>().set_constant(0);
	m_maincpu->read_adc<4>().set_constant(0);
	m_maincpu->read_adc<5>().set_constant(0);
	m_maincpu->read_adc<6>().set_constant(0x200);
	m_maincpu->read_adc<7>().set_constant(0x3ff);
	m_maincpu->read_port1().set_constant(0);
	m_maincpu->write_port1().set_nop();
	m_maincpu->write_port2().set_nop();
	m_maincpu->read_port6().set_constant(0);
	m_maincpu->read_porta().set_constant(0);
	m_maincpu->write_portf().set_nop();

	SWP30(config, m_swp30);
	m_swp30->set_addrmap(AS_DATA, &xt446_device::swp30_map);
	m_swp30->add_route(0, *this, 1.0, 0);
	m_swp30->add_route(1, *this, 1.0, 1);
}

