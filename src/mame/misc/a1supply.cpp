// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 A-1 Supply discrete hardware games

 TV 21 (197?)
 TV 21 III (197?)
 TV Poker (197?)

***************************************************************************/


#include "emu.h"

#include "cpu/mcs40/mcs40.h"

#include "screen.h"
#include "speaker.h"

namespace {

class a1supply_state : public driver_device
{
public:
	a1supply_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	{
	}

	void a1supply(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void rom_map(address_map &map) ATTR_COLD;
};


void a1supply_state::machine_start()
{
}

void a1supply_state::machine_reset()
{
}

void a1supply_state::video_start()
{
}

void a1supply_state::rom_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0x0000); // TODO: to be verified
}

void a1supply_state::a1supply(machine_config &config)
{
	// basic machine hardware
	i4040_cpu_device &cpu(I4040(config, "maincpu", 4_MHz_XTAL / 7)); // P4201A divides the incoming clock by seven to get the multi-phase clock
	cpu.set_rom_map(&a1supply_state::rom_map);

	// video hardware
	// SCREEN(config, "screen", SCREEN_TYPE_RASTER); // TODO

	// sound hardware
	// TODO: netlist
}


/***************************************************************************

 Game driver(s)

 ***************************************************************************/


ROM_START( tv21 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "aw.1c", 0x0000, 0x0800, CRC(7a1d2705) SHA1(622fbccbbf9fc80d62a5dd6d143b24852385914b) )
	ROM_LOAD( "aw.3c", 0x0800, 0x0800, BAD_DUMP CRC(f1e8ba9e) SHA1(605db3fdbaff4ba13729371ad0c4fbab3889378e) ) // FIXED BITS (00000000)?

	ROM_LOAD( "aw.43", 0x1000, 0x0200, CRC(b23759c7) SHA1(6903b8cc9fa711b985afd52582237e66d97d3262) )
	ROM_LOAD( "aw.45", 0x1200, 0x0200, CRC(6acefe3e) SHA1(6cf751df41c26eb0375770742d3bfc318c084b11) )
	ROM_LOAD( "aw.63", 0x1400, 0x0200, CRC(a022fbe7) SHA1(625283f1cd7fbd21bcd17912cbd455404282bef8) )
	ROM_LOAD( "aw.73", 0x1600, 0x0200, CRC(34e3082d) SHA1(4daf28cfee41c2fd9711a5b5365bf322cf2fe8cd) )

	ROM_LOAD( "aw.12", 0x1800, 0x0020, CRC(490c782a) SHA1(6c5455ece13f200079924e5d3af3f6b6ee8ab3ef) )
	ROM_LOAD( "aw.22", 0x1820, 0x0020, CRC(80d03096) SHA1(39e60a7acaf019c0738e2048efbef6dd566426bc) )
	ROM_LOAD( "aw.41", 0x1840, 0x0020, CRC(8b2e1b4d) SHA1(efc374c8919496211b8587a9f6da15d13c801213) )
	ROM_LOAD( "aw.65", 0x1860, 0x0020, CRC(a54ace38) SHA1(05d8ec79566310b18d14c04a5216288e15575908) )
ROM_END


// A-1 SUPPLY,INC
// T.V. 21 III
// ASSY-510005
// Intel D4289 2312A
// Intel D4040 2036A
// Intel D4201A 2334A
// Intel P4002-1 x2 (empty socket right next to these 2)
ROM_START( tv21_3 )
	ROM_REGION( 0x1060, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "750135.9",    0x0000, 0x0800, CRC(c821464c) SHA1(5334e6011ff8cd76b6215af05e697e4538921260) ) // 2716 looking chip FIXED BITS (xx000000) BADADDR             ---xxxxxxxx
	ROM_LOAD( "750030.42",   0x0800, 0x0200, CRC(d8595357) SHA1(44805f2b3dad8e764dda246ed19d328927679062) ) // 82s141
	ROM_LOAD( "750026a.47",  0x0a00, 0x0200, CRC(165f590a) SHA1(d4d001ac710d28b983f8f5ce4a2e9364c2e73179) ) // 82s141
	ROM_LOAD( "700122.36",   0x0c00, 0x0100, CRC(29f99d7e) SHA1(e9c15507ca976b8ab9bb3f070eb9e1f9c3fc185c) ) // 82s129ba
	ROM_LOAD( "700121.37",   0x0d00, 0x0100, CRC(29b22045) SHA1(5017fa45909b243f5b84a523d8098320936e0c6a) ) // 82s129ba
	ROM_LOAD( "700123.38",   0x0e00, 0x0100, CRC(53c03cbe) SHA1(f28f4ce4c79fbf4407c27e094540d0e2ff794093) ) // 82s129ba
	ROM_LOAD( "700124.11",   0x0f00, 0x0020, CRC(a4a7d564) SHA1(fd625d431ca00fec129b85526839cd8e4f7d7091) ) // 82s23n, same contents as tvpoker's 90902100.20
	ROM_LOAD( "700119a.23",  0x0f20, 0x0020, CRC(51d2e42e) SHA1(144e5c7dbc034893e66ef8385fc1f839c862bf29) ) // PROM1-8256-5B 7644
	ROM_LOAD( "7500116a.24", 0x0f40, 0x0020, CRC(ebed85b7) SHA1(b62f099c3e6350cf88f9c70750d227c82c8c4608) ) // 82s23n
	ROM_LOAD( "7500115a.25", 0x0f60, 0x0020, CRC(b663d121) SHA1(ddf09dd624ad3cddc8f10406bdb22cf746361571) ) // 82s23n
	ROM_LOAD( "700117a.41",  0x0f80, 0x0040, NO_DUMP ) // PROM1-0512-5B 7645, equivalent to 74186
	ROM_LOAD( "700120.46",   0x0fc0, 0x0040, CRC(0e68a616) SHA1(cff8fd5a5ec28e9acbda116f69ad6cf251571eb4) ) // PROM1-0512-5B 7704, "
	ROM_LOAD( "700118.51",   0x1000, 0x0040, CRC(57bdc886) SHA1(890e3a0c48aa17a8d8bc1838264264c0bcc3bc1b) ) // PROM1-0512-5B 7704, "
	ROM_LOAD( "750029.53",   0x1040, 0x0020, CRC(d8c22608) SHA1(170e6f552fc013fec6903e45e2c7ec07e44d725c) ) // 82s23n, same contents as tvpoker's 90204100.69
ROM_END


// T.V. POKER
// ASSY-510075
// A-1 SUPPLY RENO, NV. U.S.A
// 9.072 Crystal
// 4.000 Crystal closest to Intel P4040
// full component list available
// 2 PCBs have been dumped. The PROM dumps match but for 62. ROM names are taken from 1st PCB. 2nd PCB has chips stamped differently, noted in the comments
// on the 2nd PCB, under the PROM at location 59 there was a printed label with: "(C) Copyright SIRCOMA 1979"
ROM_START( tvpoker )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "90202000.48", 0x0000, 0x0200, CRC(92bafcb3) SHA1(53598477c12e725c2aaaba1065e8a51f52e821ca) ) // 1st PCB: on riser board '510422 74S474 TO 74S472' - 7649 read as 82s147, 2nd PCB: no riser board, mmi6341-1j stamped 750144A,
	ROM_LOAD( "90201700.59", 0x0200, 0x0200, CRC(cf7d7d7f) SHA1(d6a892cd9f1b817ac189c50c94081c948ea9e3e0) ) // 1st PCB: on riser board '510422 74S474 TO 74S472' - 7649 read as 82s147, 2nd PCB: no riser board, m3-7641m-5 stamped 750143

	ROM_LOAD( "90100600.26", 0x0400, 0x0100, CRC(4b301446) SHA1(5020d03678b8a193a06d658ea6088cdcc55ebf35) ) // 1st PCB: 7611 read as 82s129, 2nd PCB: dm74s287j stamped 740141
	ROM_LOAD( "90101300.36", 0x0500, 0x0100, CRC(40ac3596) SHA1(1c1a4b5278b9fdbe467a6abbd9d5ed4edbc7b49b) ) // 1st PCB: 82s129n, 2nd PCB: mmi5301-1j stamped 750355
	ROM_LOAD( "90100700.38", 0x0600, 0x0100, CRC(95945f9f) SHA1(b83bcee3df787577a3b0651c554e075b28246e31) ) // 1st PCB: 7611 read as 82s129, 2nd PCB: dm74s287j stamped 740142
	ROM_LOAD( "90101300.39", 0x0700, 0x0100, CRC(40ac3596) SHA1(1c1a4b5278b9fdbe467a6abbd9d5ed4edbc7b49b) ) // 1st PCB: 82s129n, 2nd PCB: mmi5301-1j stamped 750355
	ROM_LOAD( "90100500.68", 0x0800, 0x0100, CRC(d3e64864) SHA1(89bf6a2f3a8840331bf14bd4345f88c463efcc29) ) // 1st PCB: 7611 read as 82s129, 2nd PCB: dm74s287j stamped 750140

	ROM_LOAD( "750098.17",   0x0900, 0x0020, CRC(8b2e1b4d) SHA1(efc374c8919496211b8587a9f6da15d13c801213) ) // same PROM type and stamp on both PCBs
	ROM_LOAD( "90902100.20", 0x0920, 0x0020, CRC(a4a7d564) SHA1(fd625d431ca00fec129b85526839cd8e4f7d7091) ) // 1st PCB: 7602 read as 82s23, 2nd PCB: n82s23n stamped 750124, soldered, wasn't dumped
	ROM_LOAD( "90902200.21", 0x0940, 0x0020, CRC(80d03096) SHA1(39e60a7acaf019c0738e2048efbef6dd566426bc) ) // 1st PCB: 7602 read as 82s23, 2nd PCB: n82s23n stamped 750099A
	ROM_LOAD( "90902300.22", 0x0960, 0x0020, CRC(490c782a) SHA1(6c5455ece13f200079924e5d3af3f6b6ee8ab3ef) ) // 1st PCB: 7602 read as 82s23, 2nd PCB: mmi6330-1j stamped 750105
	// the following (62) was the only dump that didn't match between the two PCBs, and only for 0x00, which had 0xe8 on 1st PCB and 0x00 on 2nd PCB. Given rest of data, 2nd PCB dump seems more probable and is loaded here
	ROM_LOAD( "90100800.62", 0x0980, 0x0020, CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) ) // 1st PCB: 7602 read as 82s23, 2nd PCB: mmi6331-1j stamped 750150
	ROM_LOAD( "90204100.69", 0x09a0, 0x0020, CRC(d8c22608) SHA1(170e6f552fc013fec6903e45e2c7ec07e44d725c) ) // 1st PCB: 7602 read as 82s23, 2nd PCB: n82s23n stamped 750029
	ROM_LOAD( "74288.71",    0x09c0, 0x0020, CRC(fea65356) SHA1(4f336dfa33a3920aef3f3eb68239c64e0fc0fed5) ) // on riser board 'RGB MOD', 1st PCB: 74288, 2nd PCB: im5610cpe

	ROM_LOAD( "90101900.11", 0x09e0, 0x0020, NO_DUMP ) // 1st PCB: unknown chip type - 8304 C29094, 2nd PCB: missing
	ROM_LOAD( "90101000.12", 0x0a00, 0x0020, NO_DUMP ) // 1st PCB: unknown chip type - 8248 C29093, 2nd PCB: missing
ROM_END

} // Anonymous namespace


GAME( 197?, tv21,    0, a1supply, 0, a1supply_state, empty_init, ROT0, "A-1 Supply", "T.V. 21",     MACHINE_IS_SKELETON )
GAME( 197?, tv21_3,  0, a1supply, 0, a1supply_state, empty_init, ROT0, "A-1 Supply", "T.V. 21 III", MACHINE_IS_SKELETON )
GAME( 197?, tvpoker, 0, a1supply, 0, a1supply_state, empty_init, ROT0, "A-1 Supply", "T.V. Poker",  MACHINE_IS_SKELETON )
