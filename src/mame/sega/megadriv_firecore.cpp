// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Firecore / RedKid 'Mega Drive' and 'Genesis' bootleg/clone based systems.

    Firecore offers some extended video modes over a regular Mega Drive
    The YM/FM implementation in the chip has a number of flaws, MAME's older
    YM/FM core was closer, but still 'too good' in many situations.  Some
    software written for Firecore relies on these flaws to sound correct.

*/

#include "emu.h"
#include "megadriv.h"


namespace {


class megadriv_firecore_state : public md_ctrl_state
{
public:
	megadriv_firecore_state(const machine_config& mconfig, device_type type, const char* tag) :
		md_ctrl_state(mconfig, type, tag),
		m_bank(0),
		m_externalbank(0),
		m_romsize(0x400000),
		m_rom(*this, "maincpu")
	{ }

	void megadriv_firecore_3button_ntsc(machine_config &config);
	void megadriv_firecore_3button_pal(machine_config &config);
	void megadriv_firecore_6button_ntsc(machine_config &config);

	void init_atgame40();
	void init_dcat();
	void init_mdhh100();
	void init_sarc110();
	void init_reactmd();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint16_t read(offs_t offset);

	void megadriv_firecore_map(address_map &map) ATTR_COLD;

	void bank_high_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void bank_low_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void bank_upper_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void b01036_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	int m_bank;
	int m_externalbank;
	int m_romsize;

	required_region_ptr<uint16_t> m_rom;
};

uint16_t megadriv_firecore_state::read(offs_t offset)
{
	return m_rom[(((m_externalbank | m_bank) >> 1) + offset) & (m_romsize - 1)];
}

void megadriv_firecore_state::bank_high_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data &= 0x7f;
	mem_mask &= 0x7f;

	m_bank = (m_bank & 0xff80ffff) | (data & mem_mask) << 16;
	logerror("%s: bank_high_w bank is now %08x\n", machine().describe_context(), m_bank);
}

void megadriv_firecore_state::bank_low_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_bank = (m_bank & 0xffff0000) | (data & mem_mask);
	logerror("%s: bank_low_w bank is now %08x\n", machine().describe_context(), m_bank);
}

void megadriv_firecore_state::bank_upper_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// this is handled differently to the other writes, probably some external logic
	// rather than the same banking
	// written before bank_high and bank_low
	m_bank |= 0x800000;
	logerror("%s: bank_upper_w (%04x %04x) bank is now %08x\n", machine().describe_context(), data, mem_mask, m_bank);
}

void megadriv_firecore_state::b01036_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// all games in atgame40 that fail to display anything write 0x0001 here
	// could be coincidence, but could also be enabling the alt display mode?
	logerror("%s: b01036_w %04x %04x (for games with no display?)\n", machine().describe_context(), data, mem_mask);
}

void megadriv_firecore_state::megadriv_firecore_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).r(FUNC(megadriv_firecore_state::read)); // Cartridge Program ROM

	map(0xa10104, 0xa10105).w(FUNC(megadriv_firecore_state::bank_upper_w)); // read and written

	map(0xb01028, 0xb01029).w(FUNC(megadriv_firecore_state::bank_low_w));
	map(0xb0102a, 0xb0102b).w(FUNC(megadriv_firecore_state::bank_high_w));

	map(0xb01036, 0xb01037).w(FUNC(megadriv_firecore_state::b01036_w));
}

// controller is wired directly into unit, no controller slots
static INPUT_PORTS_START( firecore_3button )
	PORT_INCLUDE( md_common )

	// TODO: how do the MENU buttons on the two controllers work?
INPUT_PORTS_END


INPUT_PORTS_START( mympac )
	PORT_START("PAD1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Coin Button 1") // not coin slots
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Coin Button 2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PAD2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( firecore_6button )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1") // Extra buttons for Joypad 1 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(1) PORT_NAME("%p Mode")

	PORT_MODIFY("PAD2") // Extra buttons for Joypad 2 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(2) PORT_NAME("%p Mode")
INPUT_PORTS_END

static INPUT_PORTS_START( msi_6button )
	PORT_INCLUDE( firecore_6button )

	PORT_MODIFY("PAD2") // no 2nd pad
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET") // RESET button on controller to the left of START
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) PORT_NAME("Reset")
INPUT_PORTS_END

void megadriv_firecore_state::machine_start()
{
	md_ctrl_state::machine_start();
	m_vdp->stop_timers();
	save_item(NAME(m_bank));
	save_item(NAME(m_externalbank));
	save_item(NAME(m_romsize));
}

void megadriv_firecore_state::machine_reset()
{
	m_bank = 0;
	md_ctrl_state::machine_reset();
}

void megadriv_firecore_state::megadriv_firecore_3button_ntsc(machine_config &config)
{
	md_ntsc(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_firecore_state::megadriv_firecore_map);
}

void megadriv_firecore_state::megadriv_firecore_3button_pal(machine_config &config)
{
	md_pal(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_firecore_state::megadriv_firecore_map);
}

void megadriv_firecore_state::megadriv_firecore_6button_ntsc(machine_config &config)
{
	megadriv_firecore_3button_ntsc(config);

	ctrl1_6button(config);
	ctrl2_6button(config);
}


ROM_START( matet )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "tetrismyarcade_s29gl032n90tfi04_0001227e.bin", 0x000000, 0x400000, CRC(09b5af89) SHA1(85e506923fd803f05cc8f579f37331b608fea744) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( mateta )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "testris_s29gl032m90tfir4_0001227e.bin", 0x000000, 0x400000, CRC(656ffc77) SHA1(da7ca2d4c2bff3e583f5ad30aa4fe722691a03d9) )
	ROM_IGNORE(0x100)
ROM_END


ROM_START( mypac )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "en29lb160bb.bin", 0x000000, 0x200000, CRC(d741a601) SHA1(a8d89034458b14c5cea83980be5400b82081b274) )
ROM_END

ROM_START( mypaca )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "pacmanarcade_s29gl064n90tfi04_0001227e.bin", 0x000000, 0x800000, CRC(41495033) SHA1(219f0bd38b8a646ca43c9679aeed02c121467cd7) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( mympac )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "m29w640ft.bin", 0x000000, 0x800000, CRC(d6ceda9e) SHA1(c897f8d5661fea0c030daf9c5e92524eb4e71d52) )
ROM_END

ROM_START( mygalag )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "galaga_standup_s29al016d70tfi02_00012249.bin", 0x000000, 0x200000, CRC(8f3d2e05) SHA1(8f6a54e5a8ee55e7a6cae3e72b8e70c4eee2c1ef) )
ROM_END

ROM_START( mygalaga )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "galaga_s29jl032h70tfi01_0001227e.bin", 0x000000, 0x400000, CRC(e775089a) SHA1(0938afa8e92a8c77b4fb86e0ec044fbb2b572570) )
ROM_END

ROM_START( mysinv )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "spaceinvaders_s29gl0640n90tfi04_0001227e.bin", 0x000000, 0x800000, CRC(55e001d1) SHA1(1eaa377bf78a0f1f492565a9f38b2f7d60d0e440) )
	ROM_IGNORE(0x100)
ROM_END



/*

As the atgame40 is not running on standard MegaDrive hardware quite a few of these games rely
on an unsupported video mode (they still play sounds and resond to inputs / do colour fades)

00 Menu                             located at 00800000  WORKS
--
01 Air Hockey                       located at 0000c800  WORKS
02 Black Sheep                      located at 0002935a  BOOTS - NO DISPLAY
03 Bomber                           located at 000b6f5a  WORKS
04 Bottle Taps Race                 located at 000d6f5a  WORKS
05 Brain Switch                     located at 000ec760  BOOTS - NO DISPLAY
06 Bulls and Cows                   located at 00800000  BOOTS - NO DISPLAY
07 Cannon                           located at 0013e360  WORKS
08 Checker                          located at 0016301e  WORKS
09 Chess                            located at 001aa232  WORKS
10 Colour Puzzle                    located at 001f0a32  BOOTS - NO DISPLAY
11 Cross The Road                   located at 008a2800  BOOTS - NO DISPLAY
12 Curling 2010                     located at 00953486  BOOTS - NO DISPLAY
13 Fight or Lose                    located at 009f002e  WORKS
14 Fire Fly Glow                    located at 00a09ede  BOOTS - NO DISPLAY
15 Fish Story                       located at 00aa2ede  BOOTS - NO DISPLAY
16 Flash Memory                     located at 00285632  BOOTS - NO DISPLAY
17 Formula Challenge                located at 00310232  BOOTS - NO DISPLAY
18 Hexagons                         located at 00394a32  WORKS
19 Jacks Pea                        located at 003b4a32  BOOTS - NO DISPLAY
20 Jewel Magic                      located at 00b4a874  BOOTS - NO DISPLAY
21 Logic Dial                       located at 0040b4ac  BOOTS - NO DISPLAY
22 Table Magic                      located at 00be8f8c  BOOTS - NO DISPLAY
23 Mahjong                          located at 0049e8ac  WORKS
24 Match Eleven                     located at 004ae8ac  BOOTS - NO DISPLAY
25 Mega Brain Switch                located at 005390ac  BOOTS - NO DISPLAY
26 Memory                           located at 0058a0ac  WORKS
27 Memory Match                     located at 00c8538c  BOOTS - NO DISPLAY
28 Mirror Mirror                    located at 00d2178c  BOOTS - NO DISPLAY
29 Mr Balls                         located at 0059a0ac  WORKS
30 Navel Power                      located at 005c0a08  WORKS
31 Panic Lift                       located at 00d86f8c  BOOTS - NO DISPLAY
32 Reaction Match                   located at 00e2038c  BOOTS - NO DISPLAY
33 Snake                            located at 005dbbd4  WORKS
34 Space Hunter                     located at 005ebbd4  BOOTS - NO DISPLAY
35 Spider                           located at 006817d4  WORKS
36 Sudoku Quiz                      located at 0069efb4  BOOTS - NO DISPLAY
37 Treasure Hunt                    located at 00eb9b8c  BOOTS - NO DISPLAY
38 UFO Sighting                     located at 00f5938c  BOOTS - NO DISPLAY
39 Warehouse Keeper                 located at 00730fb4  WORKS
40 Whack a Wolf                     located at 00740fb4  BOOTS - NO DISPLAY
*/

ROM_START( atgame40 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "40bonusgamesin1.bin", 0x800000, 0x800000, CRC(4eba6e83) SHA1(b8edf1b6ecb70a136b551f1454ba8afa45bd8bc1) )
	ROM_CONTINUE(0x000000, 0x800000)
ROM_END


ROM_START(dcat16)
	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "mg6025.u1", 0x0000,  0x800000, CRC(5453d673) SHA1(b9f8d849cbed81fe73525229f4897ccaeeb7a833) )
	ROM_RELOAD(0x800000,0x800000)
ROM_END


ROM_START(mahg156)
	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "md156.u3", 0x0000,  0x8000000, CRC(665fc68c) SHA1(6b765f96716c4a0abf3d27252ec82be6b0d9a985) )
//  the Megadrive ROMs for the most part appear to be hacked versions of the games / old scene dumps
//  some are region locked to differing regions (not all games present in ROM appear on the menu)
//  ROM_COPY( "maincpu", 0x0000000, 0, 0x080000) // FORGOTTEN WORLDS
//  ROM_COPY( "maincpu", 0x0080000, 0, 0x080000) // FIRE PRO WRESTLING
//  ROM_COPY( "maincpu", 0x0100000, 0, 0x080000) // GHOST BUSTERS
//  ROM_COPY( "maincpu", 0x0180000, 0, 0x080000) // DICK TRACY
//  ROM_COPY( "maincpu", 0x0200000, 0, 0x080000) // DEVIL CRASH
//  ROM_COPY( "maincpu", 0x0280000, 0, 0x080000) // DECAP ATTACK
//  ROM_COPY( "maincpu", 0x0300000, 0, 0x080000) // DARWIN 4081
//  ROM_COPY( "maincpu", 0x0380000, 0, 0x080000) // CRACK DOWN
//  ROM_COPY( "maincpu", 0x0400000, 0, 0x080000) // CAPTAIN PLANET
//  ROM_COPY( "maincpu", 0x0480000, 0, 0x080000) // CALIFORNIA GAMES
//  ROM_COPY( "maincpu", 0x0500000, 0, 0x080000) // CADASH
//  ROM_COPY( "maincpu", 0x0580000, 0, 0x080000) // BOOGIE WOOGIE BOWLING
//  ROM_COPY( "maincpu", 0x0600000, 0, 0x080000) // BIMINI RUN
//  ROM_COPY( "maincpu", 0x0700000, 0, 0x080000) // BATTLE TOADS
//  ROM_COPY( "maincpu", 0x0780000, 0, 0x080000) // TROUBLE SHOOTER
//  ROM_COPY( "maincpu", 0x0800000, 0, 0x080000) // BURNING FORCE
//  ROM_COPY( "maincpu", 0x0880000, 0, 0x080000) // FAERY TALE ADVENTURE
//  ROM_COPY( "maincpu", 0x0900000, 0, 0x080000) // E-SWAT
//  ROM_COPY( "maincpu", 0x0980000, 0, 0x080000) // ELEMENTAL MASTER
//  ROM_COPY( "maincpu", 0x0a00000, 0, 0x080000) // EA HOCKEY
//  ROM_COPY( "maincpu", 0x0a80000, 0, 0x080000) // DARK CASTLE
//  ROM_COPY( "maincpu", 0x0b00000, 0, 0x080000) // CYBORG JUSTICE (CENSOR)
//  ROM_COPY( "maincpu", 0x0b80000, 0, 0x080000) // LITTLE MERMAID
//  ROM_COPY( "maincpu", 0x0c00000, 0, 0x080000) // DORAEMON
//  ROM_COPY( "maincpu", 0x0c80000, 0, 0x080000) // SONIC
//  ROM_COPY( "maincpu", 0x0d00000, 0, 0x080000) // WANI WANI WORLD
//  ROM_COPY( "maincpu", 0x0d80000, 0, 0x080000) // GOLDEN AXE 2
//  etc.
ROM_END

ROM_START(mdhh100)
	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "s29gl01gp11tfir1.u13", 0x0000,  0x8000000, CRC(564ab33a) SHA1(e455aaa9ed6f302d1ebe55b5202f983af612c415) )
ROM_END

ROM_START( msi_sf2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "29lv320.bin", 0x000000, 0x400000, CRC(465b12f0) SHA1(7a058f6feb4f08f56ae0f7369c2ca9a9fe2ed40e) )
ROM_END

ROM_START( reactmd )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 ) // this contains the MD games and main boot menu
	ROM_LOAD16_WORD_SWAP( "reactormd.bin", 0x0000, 0x2000000, CRC(fe9664a4) SHA1(d475b524f576c9d1d90aed20c7467cc652396baf) )

	ROM_REGION( 0x4000000, "sunplus", ROMREGION_ERASE00 ) // this contains the SunPlus games
	ROM_LOAD16_WORD_SWAP( "reactor_md_sunplus-full.bin", 0x0000, 0x4000000, CRC(843aa58c) SHA1(07cdc6d4aa0057939c145ece01a9aca73c7f1f2b) )
	ROM_IGNORE(0x4000000) // the 2nd half of the ROM can't be accessed by the PCB (address line tied low) (contains garbage? data)
ROM_END

ROM_START( sarc110 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) // Mega Drive part
	ROM_LOAD16_WORD_SWAP( "superarcade.bin", 0x000000, 0x1000000, CRC(be732867) SHA1(3857b2fbddd6a548c81caf64122e47a0df079be5) )

	ROM_REGION( 0x400000, "mainrom", 0 ) // VT02/03 part
	ROM_LOAD( "ic1.prg", 0x00000, 0x400000, CRC(de76f71f) SHA1(ff6b37a76c6463af7ae901918fc008b4a2863951) )
ROM_END

ROM_START( sarc110a )
	ROM_REGION( 0x1000000, "maincpu", 0 ) // Mega Drive part
	ROM_LOAD16_WORD_SWAP( "superarcade.bin", 0x000000, 0x1000000, CRC(be732867) SHA1(3857b2fbddd6a548c81caf64122e47a0df079be5) )

	ROM_REGION( 0x400000, "mainrom", 0 ) // VT02/03 part
	ROM_LOAD( "ic1_ver2.prg", 0x00000, 0x400000, CRC(b97a0dc7) SHA1(bace32d73184df914113de5336e29a7a6f4c03fa) )
ROM_END

void megadriv_firecore_state::init_atgame40()
{
	m_romsize = memregion("maincpu")->bytes();
	init_megadrie();
}

void megadriv_firecore_state::init_sarc110()
{
	m_romsize = memregion("maincpu")->bytes();
	m_externalbank = 0x800000;
	init_megadrie();
}

void megadriv_firecore_state::init_reactmd()
{
	m_romsize = memregion("maincpu")->bytes();
	m_externalbank = 0x1800000;
	init_megadrie();

	// decryption of the SunPlus part
	uint16_t *ROM = (uint16_t*)memregion("sunplus")->base();
	int size = memregion("sunplus")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 15, 13, 14, 12,  7,  6,  5,  4,
									 11, 10, 9,  8,   3,  1,  2,  0);

		ROM[i] = ROM[i] ^ 0xa5a5;
	}
}

void megadriv_firecore_state::init_dcat()
{
	m_romsize = memregion("maincpu")->bytes();
	init_megadriv();
}

void megadriv_firecore_state::init_mdhh100()
{
	m_romsize = memregion("maincpu")->bytes();
	m_externalbank = 0x7800000;
	init_megadriv();
}


} // anonymous namespace

// Games below have a device at b0102x which appears to either be able to select ROM base on a byte boundary
// OR maybe are running from RAM instead of ROM (with an auto-copy at the start?) with that being a DMA operation.

// Technically this is a MD type cartridge, but it doesn't seem to be designed for use with a standard MD as it contains
// nothing but the 16Mbyte ROM and a 5v to 3.3v converter yet the code clearly requires some extensive banking logic.
// Testing it on a real MD shows nothing, not even the menu.
//
// We don't seem to emulate the system it's designed for, so for now just treat it as its own thing (which may become
// the basis of a driver for that console)
//
// due to differences in the SoC compared to real MD hardware (including sound + new video modes) these have been left
// as NOT WORKING for now although some games run to a degree

CONS( 2021, mypac,     0,        0, megadriv_firecore_3button_ntsc,  mympac, megadriv_firecore_state, init_megadriv,           "dreamGEAR", "My Arcade Pac-Man (DGUNL-4198, Pocket Player Pro)", MACHINE_NOT_WORKING | ROT270 )
CONS( 2021, mypaca,    mypac,    0, megadriv_firecore_3button_ntsc,  mympac, megadriv_firecore_state, init_megadriv,           "dreamGEAR", "My Arcade Pac-Man (DGUNL-4194, Micro Player Pro)", MACHINE_NOT_WORKING | ROT270 )

CONS( 2021, mympac,    0,        0, megadriv_firecore_3button_ntsc,  mympac, megadriv_firecore_state, init_megadriv,           "dreamGEAR", "My Arcade Ms. Pac-Man (DGUNL-7010, Pocket Player Pro)", MACHINE_NOT_WORKING | ROT270 )

// menu uses unsupported extended mode
CONS( 2021, mygalag,   0,        0, megadriv_firecore_3button_ntsc,  mympac, megadriv_firecore_state, init_megadriv,           "dreamGEAR", "My Arcade Galaga (DGUNL-4195, Micro Player Pro)", MACHINE_NOT_WORKING | ROT270 )
CONS( 2021, mygalaga,  mygalag,  0, megadriv_firecore_3button_ntsc,  mympac, megadriv_firecore_state, init_megadriv,           "dreamGEAR", "My Arcade Galaga (DGUNL-4199, Pocket Player Pro)", MACHINE_NOT_WORKING | ROT270 )

CONS( 2021, mysinv,    0,        0, megadriv_firecore_3button_ntsc,  mympac, megadriv_firecore_state, init_megadriv,           "dreamGEAR", "My Arcade Space Invaders (DGUNL-7006, Pocket Player Pro)", MACHINE_NOT_WORKING | ROT90 )

CONS( 2012, atgame40,  0,        0, megadriv_firecore_3button_pal,   firecore_3button, megadriv_firecore_state, init_atgame40, "AtGames",   "40 Bonus Games in 1 (AtGames)", MACHINE_NOT_WORKING)

CONS( 2021, matet,     0,        0, megadriv_firecore_3button_ntsc,  firecore_3button, megadriv_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Tetris (DGUNL-7028, Pocket Player Pro)", MACHINE_NOT_WORKING)
CONS( 2021, mateta,    matet,    0, megadriv_firecore_3button_ntsc,  firecore_3button, megadriv_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Tetris (DGUNL-7025, Micro Player Pro)", MACHINE_NOT_WORKING)

// has an SD card slot?
CONS( 200?, dcat16,    0,        0, megadriv_firecore_3button_ntsc,  firecore_3button, megadriv_firecore_state, init_dcat,     "Firecore",  "D-CAT16 (Mega Drive handheld)",  MACHINE_NOT_WORKING )
// seems to be based on the AT games units, requires custom mode for menu?
CONS( 201?, mahg156,   0,        0, megadriv_firecore_3button_ntsc,  firecore_3button, megadriv_firecore_state, init_mdhh100,  "<unknown>", "Mini Arcade Handheld Game Console 2.8 Inch Screen Built in 156 Retro Games (Mega Drive handheld)",  MACHINE_NOT_WORKING )
// game-boy like handheld, pink in colour, 6 button controller (+ home select, start, vol buttons)
CONS( 201?, mdhh100,   0,        0, megadriv_firecore_3button_ntsc,  firecore_3button, megadriv_firecore_state, init_mdhh100,  "<unknown>", "unknown 100-in-1 handheld (Mega Drive based)",  MACHINE_NOT_WORKING )

// From a European unit but NTSC? - code is hacked from original USA Genesis game with region check still intact? (does the clone hardware always identify as such? or does the bypassed boot code skip the check?)
CONS( 2018, msi_sf2,   0,        0, megadriv_firecore_6button_ntsc,  msi_6button,      megadriv_firecore_state, init_megadriv, "MSI / Capcom / Sega", "Street Fighter II: Special Champion Edition (MSI Plug & Play) (Europe)", 0)

// Two systems in one unit - Firecore Genesis and SunPlus
CONS( 2009, reactmd,   0,        0, megadriv_firecore_3button_pal,   firecore_3button, megadriv_firecore_state, init_reactmd,  "AtGames / Sega / Waixing", "Reactor MD (Firecore/SunPlus hybrid, PAL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Two systems in one unit - Firecore Genesis and VT02/VT03
CONS( 200?, sarc110,   0,        0, megadriv_firecore_3button_pal,   firecore_3button, megadriv_firecore_state, init_sarc110,  "<unknown>", "Super Arcade 101-in-1 (Firecore/VT hybrid, set 1)", MACHINE_NOT_WORKING)
CONS( 200?, sarc110a,  sarc110,  0, megadriv_firecore_3button_pal,   firecore_3button, megadriv_firecore_state, init_sarc110,  "<unknown>", "Super Arcade 101-in-1 (Firecore/VT hybrid, set 2)", MACHINE_NOT_WORKING)
