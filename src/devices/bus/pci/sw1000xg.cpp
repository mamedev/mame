// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "sw1000xg.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(SW1000XG, sw1000xg_device, "sw1000xg", "Yamaha SW1000XG")

sw1000xg_device::sw1000xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ymp21_device(mconfig, SW1000XG, tag, owner, clock),
	  m_maincpu(*this, "maincpu"),
	  m_swp30(*this, "swp30")
{
	set_ids(0x10731000, 0x00, 0x040100, 0x10731000);
}

void sw1000xg_device::device_start()
{
	ymp21_device::device_start();
}

void sw1000xg_device::device_reset()
{
	ymp21_device::device_reset();

	m_maincpu->set_input_line(0, ASSERT_LINE);
}

void sw1000xg_device::h8_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x200000, 0x21ffff).ram();
	map(0x400000, 0x401fff).m(m_swp30, FUNC(swp30_device::map));
}

void sw1000xg_device::swp30_map(address_map &map)
{
	//  map(0x000000, 0x1fffff).rom().region("swp30",         0).mirror(0x200000);
	//  map(0x400000, 0x4fffff).rom().region("swp30",  0x800000).mirror(0x300000);
	//  map(0x800000, 0x9fffff).rom().region("swp30", 0x1000000).mirror(0x200000);
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

// a0 cards had firmware 1.00.00
// b0 cards are known to exist but not dumped yet
// c0 cards are not proven existing
// d0 cards started with 1.04.00 and the firmware was upgradeable from the PC

ROM_START( sw1000xg )
	ROM_REGION(  0x100000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v166")
	ROM_SYSTEM_BIOS( 0, "v166", "v1.06.06" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "1.06.06_xv561d0.ic102", 0x000000, 0x100000, CRC(6734482c) SHA1(dff42c0f0ad0f0ec9587cd2a53ba2b3836b1d587) )
	ROM_SYSTEM_BIOS( 1, "v161", "v1.06.01" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "1.06.01_xv561d0.ic102", 0x000000, 0x100000, CRC(c788aa9e) SHA1(d0861e9604a8e92f6adfc451e97d62d49d40f033) )
	ROM_SYSTEM_BIOS( 2, "v152", "v1.05.02" )
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "1.05.02_xv561d0.ic102", 0x000000, 0x100000, CRC(9152a052) SHA1(20759f4047f8dc3cb21efef538f78393d7a68521) )
	ROM_SYSTEM_BIOS( 3, "v150", "v1.05.00" )
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "1.05.00_xv561d0.ic102", 0x000000, 0x100000, CRC(36e199df) SHA1(efe2d3000b49a4a93705fee5ffd5456953ef5a22) )
	ROM_SYSTEM_BIOS( 4, "v140", "v1.04.00" )
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "1.04.00_xv561d0.ic102", 0x000000, 0x100000, CRC(9f5bfec8) SHA1(4ced122c8cac7a9c022d86fffe121042ad4be6a2) )
	ROM_SYSTEM_BIOS( 5, "v100", "v1.00.00" )
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "1.00.00_xv561a0.ic102", 0x000000, 0x100000, CRC(9b6d8700) SHA1(0bb6c60572b9ed1628fa9e6678e03bd1e47f2a06) )

	ROM_REGION( 0x1400000, "swp30", 0 )
	ROM_LOAD32_WORD( "xv389a0.ic122", 0x0000000, 0x800000, CRC(92e11205) SHA1(72ec39e03e4e5ddf6137faa798c0ec3c23905855) )
	ROM_LOAD32_WORD( "xv390a0.ic121", 0x0000002, 0x800000, CRC(cbc80585) SHA1(1903ed7f1292e6117a448fec19e2b712f3815964) )
	ROM_LOAD32_WORD( "xt445a0-828.ic124", 0x0800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic123", 0x0800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )

ROM_END

const tiny_rom_entry *sw1000xg_device::device_rom_region() const
{
	return ROM_NAME(sw1000xg);
}

void sw1000xg_device::device_add_mconfig(machine_config &config)
{
	H83002(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sw1000xg_device::h8_map);

	SPEAKER(config, "speaker", 2).front();

	SWP30(config, m_swp30);
	m_swp30->set_addrmap(AS_DATA, &sw1000xg_device::swp30_map);
	m_swp30->add_route(0, "speaker", 1.0, 0);
	m_swp30->add_route(1, "speaker", 1.0, 1);
}

