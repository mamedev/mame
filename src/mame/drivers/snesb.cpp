// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Peter Ferrie,Stephane Humbert
/***************************************************************************

 Arcade games (hacks of console games) running on SNES hardware.

 Driver (based on nss.cpp) by Tomasz Slanina

    Supported games:
    - Fatal Fury Special
    - Final Fight 2
    - Final Fight 3
    - Ghost Chaser Densei (2 sets)
    - Gundam Wing: Endless Duel (2 sets)
    - Iron (bootleg of Iron Commando)
    - Killer Instinct
    - Legend
    - Rushing Beat Shura
    - Sonic Blast Man II Special Turbo (2 sets)
    - Venom & Spider-Man - Separation Anxiety
    - Wild Guns

TODO:

 - all games : (re)add PORT_DIPLOCATION
 - kinstb   : fix gfx glitches, missing texts
 - ffight2b : remove hack for starting credits (RAM - mainly 0x7eadce where credits are stored - is filled with 0x55,
   so you are awarded 55 credits on a hard reset)
 - sblast2b : dipswitches
 - sblast2b : pressing start during gameplay changes the character used. Intentional?
 - sblast2ba: needs decryption, seems to use a slightly different scheme and ROM size doesn't match the one from the original SNES game
 - denseib,2: fix gfx glitches, missing texts
 - legendsb : unknown dipswitches
 - rushbets : dipswitches (stored at memory locations $785006 and $785008)
 - venom    : gfx glitches on second level
 - wldgunsb : dipswitches
 - wldgunsb : sometimes continue counter doesn't start from '9', verify if protection is involved.

***************************************************************************

  Killer Instinct PCB Info:
  --------------------------

    PQFP 100(?)pin chip marked "SP-BE0"
    PQFP 100(?)pin chip marked "SP-BH0"
    PQFP 100(?)pin chip marked "SP-AF0"
    Lattice pLSI 1024-60LJ B604S03
    6116 SRAM    x2
    AS7C256 SRAM x8
    jumper pack (12)
    dsw8         x2
    Xtal 24.576 MHz
    Xtal 21.47727 MHz
    volume pot
    27c801       x4
    two empty eprom sockets

    It's SNES version of KI with few mods (removed copyright messages,
    extra code for coin input, etc).

    256 bytes of RAM (mapped to reserved area) are shared with some
    device (probably Lattice PLD) used for handling coin inputs and dips

    Data lines of eproms are bitswapped.

***************************************************************************

  Final Fight 2 PCB layout:
  ------------------------

 |----------------------------------------------------------------------------|
 | |-----------|                                                              |
 | |           |        21.47727 MHz      24.576 MHz                          |
 | | Lattice   |                                                              |
 | | pLSI      |               |--------|   |--------|          HM65256       |
 | | 1024-60LJ |  |--------|   |        |   |        |                        |
 | |           |  |        |   | 86A621 |   | 86A537 |                        |
 | |-----------|  | 86A623 |   |  JDCF  |   |  JDCF  |                        |
 |                |  JDCF  |   |        |   |        |          D42832C       |
 |    ff2_1.u8    |        |   |--------|   |--------|                        |
 |                |--------|                                                  |
 |                             |--------|   |--------|                        |
 |    ff2_2.u7    |--------|   |        |   |        |          KM62256       |
 |                |        |   | 86A617 |   | 86A618 |                        |
 |                | 86A540 |   |  JDCF  |   |  JDKF  |                        |
 |    ff2_3.u6    |  JDKF  |   |        |   |        |                        |
 |                |        |   |--------|   |--------|          KM62256       |
 |                |--------|                                                  |
 |     GL324                  D41464C     D41464C                             |
 |                                                                            |
 |                            D41464C     D41464C         DSW2      DSW1      |
 |                                                                            |
 |                              7414        74245        74245     74245      |
 |                                                                            |
 |    uPC1242H       VR1       GD4021B     GD4021B      GD4021B   GD4021B     |
 |                                                                            |
 |                                                                            |
 |               |---|              JAMMA                 |---|               |
 |---------------|   |------------------------------------|   |---------------|

***************************************************************************

Iron PCB (same as Final Fight 2?)
 ______________________________________________________________________________________________
|                                                                                              |
|     _____________              XTAL1                    XTAL2                                |
|    |             |             21.47727Mhz              24.576Mhz          _______           |
|    |             |                                                        |86A619 |          |
|    |   LATTICE   |                                                        |_______|          |
|    |pLSL1024_60LJ|                                                                           |
|    |   B611S01   |                                                         _______________   |
|    |             |                       _________        _________       |               |  |
|    |             |         ______       | 86A621  |      | 86A537  |      |HM65256BLP_12  |  |
|    |             |        |      |      |  JDCF   |      |  JDCF   |      |   01002990    |  |
|    |_____________|        |86A623|      |         |      |         |      |_______________|  |
|                           | JDCF |      |_________|      |_________|       _______________   |
|  ___________________      |      |                                        |               |  |
| |4.C11              |     |      |                                        |HM65256BLP_12  |  |
| |                   |     |______|                                        |   01002990    |  |
| |AM27C020           |                                                     |_______________|  |
| |___________________|                      ______           ______                           |
|  ___________________                      |      |         |      |        _______________   |
| |5.C10              |      ______         |86A617|         |86A618|       |               |  |
| |                   |     |      |        | JDCF |         | JDCF |       | KM62256BLP_10 |  |
| |27C4001            |     |86A540|        |      |         |      |       |  210Y  KOREA  |  |
| |___________________|     | JDKF |        |      |         |      |       |_______________|  |
|  ___________________      |      |        |______|         |______|        _______________   |
| |6.C09              |     |      |                                        |               |  |
| |                   |     |______|                                        | KM62256BLP_10 |  |
| |27C4001            |                                                     |  210Y  KOREA  |  |
| |___________________|                  ________     ________              |_______________|  |
|                                       |D41464C |   |D41464C |                                |
|  _______                              |________|   |________|                                |
| | GL324 |                                                        ________    ________        |
| |_______|                              ________     ________    |  DIP1  |  |  DIP2  |       |
|                                       |D41464C |   |D41464C |   |1      8|  |1      8|       |
|                                       |________|   |________|   |________|  |________|       |
|                                                                                              |
|                                           ______   _________    _________    _________       |
|                                          |74LS14| |74LS245N |  |74LS245N |  |74LS245B |      |
|                                          |______| |_________|  |_________|  |_________|      |
|                                                                                              |
|                                           ______     ______      ______       ______         |
|                                          |GD4021|   |CD4021|    |CD4021|     |CD4021|        |
|                                          |______|   |______|    |______|     |______|        |
|                                                                                              |
|                  _____ 1                                           28 _____                  |
|                 |     || | | | | | | | | | | | | | | | | | | | | | | |     |                 |
|                 |     || | | | | | | | | | | | | | | | | | | | | | | |     |                 |
|_________________|     |______________________________________________|     |_________________|

***************************************************************************/

#include "emu.h"
#include "includes/snes.h"
#include "speaker.h"


namespace {

class snesb_state : public snes_state
{
public:
	snesb_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram%u", 1U)
	{ }

	void base(machine_config &config);
	void endless(machine_config &config);
	void extrainp(machine_config &config);
	void ffight2b(machine_config &config);
	void kinstb(machine_config &config);
	void rushbets(machine_config &config);
	void sblast2b(machine_config &config);
	void venom(machine_config &config);
	void wldgunsb(machine_config &config);

	void init_iron();
	void init_denseib();
	void init_denseib2();
	void init_kinstb();
	void init_sblast2b();
	void init_ffight2b();
	void init_ffight3b();
	void init_endless();
	void init_legendsb();
	void init_rushbets();
	void init_venom();
	void init_wldgunsb();

private:
	optional_shared_ptr_array<int8_t, 2> m_shared_ram;
	uint8_t m_cnt;
	uint8_t prot_cnt_r();
	uint8_t sb2b_6a6xxx_r(offs_t offset);
	uint8_t sb2b_7xxx_r(offs_t offset);
	uint8_t endless_580xxx_r(offs_t offset);
	uint8_t endless_800b_r(offs_t offset);
	uint8_t rushbets_75axxx_r(offs_t offset);
	uint8_t wldgunsb_722262_r();
	uint8_t wldgunsb_723364_r();
	uint8_t wldgunsb_721197_r();
	uint8_t wldgunsb_72553b_r();
	uint8_t wldgunsb_72443a_r();

	DECLARE_MACHINE_RESET(ffight2b);
	void snesb_map(address_map &map);
	void spc_map(address_map &map);
	void endless_map(address_map &map);
	void extrainp_map(address_map &map);
	void kinstb_map(address_map &map);
	void rushbets_map(address_map &map);
	void sblast2b_map(address_map &map);
	void venom_map(address_map &map);
	void wldgunsb_map(address_map &map);
};


uint8_t snesb_state::prot_cnt_r()
{
	// protection check
	return ++m_cnt;
}


// Sonic Blast Man II Special Turbo

uint8_t snesb_state::sb2b_6a6xxx_r(offs_t offset)
{
	// protection checks
	switch (offset)
	{
		case 0x26f: return 0xb1;
		case 0x3e0: return 0x9e;
		case 0x5c8: return 0xf4;
		case 0x94b: return 0x3a;
		case 0xd1a: return 0xc5;
		case 0xfb7: return 0x47;
	}

	logerror("Unknown protection read %x @ %x\n", offset, m_maincpu->pc());

	return 0;
}

uint8_t snesb_state::sb2b_7xxx_r(offs_t offset) // handler to read boot code
{
	return m_maincpu->space(AS_PROGRAM).read_byte(0xc07000 + offset);
}


// Endless Duel
uint8_t snesb_state::endless_580xxx_r(offs_t offset)
{
	// protection checks
	switch (offset)
	{
		case 0x2bc: return 0xb4;
		case 0x36a: return 0x8a;
		case 0x7c1: return 0xd9;
		case 0x956: return 0xa5;
		case 0xe83: return 0x6b;
	}

	logerror("Unknown protection read %x @ %x\n", offset, m_maincpu->pc());

	return 0;
}

uint8_t snesb_state::endless_800b_r(offs_t offset)
{
	// work around missing content
	if (!offset)
	{
		return 0x50;
	}

	return 0xe8;
}


// Rushing Beat Shura
uint8_t snesb_state::rushbets_75axxx_r(offs_t offset)
{
// protection checks
	switch (offset)
	{
		case 0xf49: return 0xe3;
		case 0x05a: return 0xf4;
		case 0x16b: return 0x05;
		case 0x27c: return 0x16;
		case 0x38d: return 0x27;
	}

	logerror("Unknown protection read %x @ %x\n", offset, m_maincpu->pc());

	return 0;
}


// Wild Guns
// these read with a LDA but discard the upper 8-bit values

uint8_t snesb_state::wldgunsb_722262_r() // POST
{
	// PC 2e2f6
	return 0x2b;
}

uint8_t snesb_state::wldgunsb_723364_r() // POST
{
	// PC b983
	return 0x93;
}

uint8_t snesb_state::wldgunsb_721197_r() // in-game
{
	// PC 2e30c
	return 0xe4;
}

uint8_t snesb_state::wldgunsb_72553b_r() // in-game
{
	// PC 2e216
	return 0xbf;
}

uint8_t snesb_state::wldgunsb_72443a_r() // in-game
{
	// PC 2e322
	return 0x66;
}


void snesb_state::snesb_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(snesb_state::snes_r_bank1), FUNC(snesb_state::snes_w_bank1));
	map(0x7e0000, 0x7fffff).ram().share(m_wram);                 // 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM
	map(0x800000, 0xffffff).rw(FUNC(snesb_state::snes_r_bank2), FUNC(snesb_state::snes_w_bank2));    // Mirror and ROM
}

void snesb_state::spc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("aram");
}

void snesb_state::extrainp_map(address_map &map)
{
	snesb_map(map);

	map(0x770071, 0x770071).portr("DSW1");
	map(0x770073, 0x770073).portr("DSW2");
	map(0x770079, 0x770079).portr("COIN");
}

void snesb_state::kinstb_map(address_map &map)
{
	extrainp_map(map);

	map(0x781000, 0x7810ff).ram().share(m_shared_ram[0]);
}

void snesb_state::sblast2b_map(address_map &map)
{
	extrainp_map(map);

	map(0x007000, 0x007fff).r(FUNC(snesb_state::sb2b_7xxx_r));
	map(0x6a6000, 0x6a6fff).r(FUNC(snesb_state::sb2b_6a6xxx_r));
	map(0x75bd37, 0x75bd37).r(FUNC(snesb_state::prot_cnt_r));
}

void snesb_state::endless_map(address_map &map)
{
	venom_map(map);

	map(0x00800b, 0x00800c).r(FUNC(snesb_state::endless_800b_r));
	map(0x580000, 0x580fff).r(FUNC(snesb_state::endless_580xxx_r));
	map(0x624b7f, 0x624b7f).r(FUNC(snesb_state::prot_cnt_r));
}

void snesb_state::rushbets_map(address_map &map)
{
	snesb_map(map);

	map(0x5b8e3c, 0x5b8e3c).r(FUNC(snesb_state::prot_cnt_r));
	map(0x75a000, 0x75afff).r(FUNC(snesb_state::rushbets_75axxx_r));
	map(0x770071, 0x770071).portr("DSW1");
	map(0x770079, 0x770079).portr("COIN");
	map(0x785000, 0x78500f).ram().share(m_shared_ram[0]);
}

void snesb_state::venom_map(address_map &map)
{
	extrainp_map(map);

	map(0x781000, 0x781021).ram().share(m_shared_ram[0]);
	map(0x781200, 0x781221).ram().share(m_shared_ram[1]);
}

void snesb_state::wldgunsb_map(address_map &map)
{
	snesb_map(map);

	map(0x721197, 0x721197).r(FUNC(snesb_state::wldgunsb_721197_r));
	map(0x722262, 0x722262).r(FUNC(snesb_state::wldgunsb_722262_r));
	map(0x723363, 0x723363).r(FUNC(snesb_state::wldgunsb_723364_r));
	map(0x72443a, 0x72443a).r(FUNC(snesb_state::wldgunsb_72443a_r));
	map(0x72553b, 0x72553b).r(FUNC(snesb_state::wldgunsb_72553b_r));
	map(0x770071, 0x770071).portr("DSW1");
	map(0x770072, 0x770072).portr("DSW2");
	map(0x770079, 0x770079).portr("COIN");
	map(0x781000, 0x781021).ram().share(m_shared_ram[0]);
	map(0x7bf45b, 0x7bf45b).r(FUNC(snesb_state::prot_cnt_r));
}


static INPUT_PORTS_START( snes_common )

	PORT_START("SERIAL1_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL1_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

#if SNES_LAYER_DEBUG
	PORT_START("DEBUG1")
	PORT_CONFNAME( 0x03, 0x00, "Select BG1 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x01, "BG1B (lower) only" )
	PORT_CONFSETTING(    0x02, "BG1A (higher) only" )
	PORT_CONFNAME( 0x0c, 0x00, "Select BG2 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x04, "BG2B (lower) only" )
	PORT_CONFSETTING(    0x08, "BG2A (higher) only" )
	PORT_CONFNAME( 0x30, 0x00, "Select BG3 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "BG3B (lower) only" )
	PORT_CONFSETTING(    0x20, "BG3A (higher) only" )
	PORT_CONFNAME( 0xc0, 0x00, "Select BG4 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x40, "BG4B (lower) only" )
	PORT_CONFSETTING(    0x80, "BG4A (higher) only" )

	PORT_START("DEBUG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 1") PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 2") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 3") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 4") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Objects") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Main/Sub") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Color Math") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Windows") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_START("DEBUG3")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mosaic") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x70, 0x00, "Select OAM priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "OAM0 only" )
	PORT_CONFSETTING(    0x20, "OAM1 only" )
	PORT_CONFSETTING(    0x30, "OAM2 only" )
	PORT_CONFSETTING(    0x40, "OAM3 only" )
	PORT_CONFNAME( 0x80, 0x00, "Draw sprite in reverse order" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DEBUG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 0 draw") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 1 draw") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 2 draw") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 3 draw") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 4 draw") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 5 draw") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 6 draw") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 7 draw") PORT_TOGGLE
#endif
INPUT_PORTS_END


// verified from 5A22 code
static INPUT_PORTS_START( kinstb )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "0 (Easiest)" )               // "EASY" (0 star)
	PORT_DIPSETTING(    0x01, "1" )                         // (1 star)
	PORT_DIPSETTING(    0x02, "2" )                         // (2 stars)
	PORT_DIPSETTING(    0x03, "3" )                         // (3 stars)
	PORT_DIPSETTING(    0x04, "4" )                         // (4 stars)
	PORT_DIPSETTING(    0x05, "5" )                         // "HARD" (5 stars)
	PORT_DIPSETTING(    0x06, "6" )                         // undefined
	PORT_DIPSETTING(    0x07, "7" )                         // undefined
	PORT_DIPSETTING(    0x08, "8" )                         // undefined
	PORT_DIPSETTING(    0x09, "9" )                         // undefined
	PORT_DIPSETTING(    0x0a, "10" )                        // undefined
	PORT_DIPSETTING(    0x0b, "11" )                        // undefined
	PORT_DIPSETTING(    0x0c, "12" )                        // undefined
	PORT_DIPSETTING(    0x0d, "13" )                        // undefined
	PORT_DIPSETTING(    0x0e, "14" )                        // undefined
	PORT_DIPSETTING(    0x0f, "15 (Hardest)" )              // undefined
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0f, "15 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0e, "14 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0d, "13 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, "12 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0b, "11 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0a, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x09, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

// verified from 5A22 code
static INPUT_PORTS_START( ffight2b )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            // duplicate setting
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )       // "GAME LEVEL"
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          // "EXPERT"
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "100k 300k 200k+" )
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

// verified from 5A22 code
static INPUT_PORTS_START( iron )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            // duplicate setting
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       // "LEVEL"
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )           // "MEDIUM"
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )             // duplicate setting
	PORT_DIPNAME( 0x04, 0x04, "Suffered Damages" )          // code at 0x(8)082d0
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "More" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )            // table at 0x(8)3ffda (4 * 1 word) gives 02 03 04 05 (add 1) but extra LSRA before TAY at 0x(8)3ffcf
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "769 (Bug)" )
	PORT_DIPSETTING(    0x00, "1025 (Bug)" )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

// verified from 5A22 code
static INPUT_PORTS_START( denseib )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            // duplicate setting
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Difficulty ) )       // "RANK"
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )             // duplicate setting
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Battle" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Suffered Damages" )          // code at 0x(8)0f810
	PORT_DIPSETTING(    0x07, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x06, "x1.5" )
	PORT_DIPSETTING(    0x05, "x2.5" )
	PORT_DIPSETTING(    0x04, "x3.5" )
	PORT_DIPSETTING(    0x03, "x4.5" )
	PORT_DIPSETTING(    0x02, "x5.5" )
	PORT_DIPSETTING(    0x01, "x6.5" )
	PORT_DIPSETTING(    0x00, "x7.5" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

// verified from 5A22 code
static INPUT_PORTS_START( sblast2b )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            // duplicate setting
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Difficulty ) )       // "LEVEL"
	PORT_DIPSETTING(    0x38, "0 (Easiest)" )               // "NORMAL"
	PORT_DIPSETTING(    0x30, "1" )                         // "HARD"
	PORT_DIPSETTING(    0x28, "2" )                         // undefined
	PORT_DIPSETTING(    0x20, "3" )                         // undefined
	PORT_DIPSETTING(    0x18, "4" )                         // undefined
	PORT_DIPSETTING(    0x10, "5" )                         // undefined
	PORT_DIPSETTING(    0x08, "6" )                         // undefined
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )               // undefined
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x04, "Power" )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


static INPUT_PORTS_START( endless )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            // duplicate setting
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Difficulty ) )       // "LEVEL"
	PORT_DIPSETTING(    0x38, "0 (Easiest)" )               // "EASY"
	PORT_DIPSETTING(    0x30, "1" )                         // "NORMAL"
	PORT_DIPSETTING(    0x28, "2" )                         // "HARD"
	PORT_DIPSETTING(    0x20, "3" )                         // undefined
	PORT_DIPSETTING(    0x18, "4" )                         // undefined
	PORT_DIPSETTING(    0x10, "5" )                         // undefined
	PORT_DIPSETTING(    0x08, "6" )                         // undefined
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )               // undefined
	PORT_DIPNAME( 0xc0, 0xc0, "Time" )                      // "TIME"
	PORT_DIPSETTING(    0xc0, "99" )                        // "LIMIT"
	PORT_DIPSETTING(    0x80, "60" )                        // undefined
	PORT_DIPSETTING(    0x40, "30" )                        // undefined
	PORT_DIPSETTING(    0x00, "Infinite" )                  // "NO LIMIT"

	PORT_START("DSW2")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( rushbets )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            // duplicate setting
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x38, "0" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x28, "2" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( venom )
	PORT_INCLUDE(snes_common)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            // duplicate setting
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )

	// The game code has been hacked to use only 3 buttons (the arcade panel) as a result many moves are not even possible and some buttons have multiple purposes compared to the original game
	PORT_MODIFY("SERIAL1_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("SERIAL2_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( wldgunsb )
	PORT_INCLUDE( venom )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

void snesb_state::base(machine_config &config)
{
	// basic machine hardware
	_5A22(config, m_maincpu, 3580000 * 6);   // 2.68Mhz, also 3.58Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::snesb_map);

	// audio CPU, runs at 24.576 MHz / 12 = 2.048 MHz
	S_SMP(config, m_soundcpu, XTAL(24'576'000) / 12);
	m_soundcpu->set_addrmap(AS_DATA, &snesb_state::spc_map);
	m_soundcpu->dsp_io_read_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_r));
	m_soundcpu->dsp_io_write_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_w));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC);
	m_screen->set_video_attributes(VIDEO_VARIABLE_WIDTH);
	m_screen->set_screen_update(FUNC(snes_state::screen_update));

	SNES_PPU(config, m_ppu, MCLK_NTSC);
	m_ppu->open_bus_callback().set([this] { return snes_open_bus_r(); }); // lambda because overloaded function name
	m_ppu->set_screen("screen");

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	S_DSP(config, m_s_dsp, XTAL(24'576'000) / 12);
	m_s_dsp->set_addrmap(0, &snesb_state::spc_map);
	m_s_dsp->add_route(0, "lspeaker", 1.00);
	m_s_dsp->add_route(1, "rspeaker", 1.00);
}

void snesb_state::extrainp(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::extrainp_map);
}

void snesb_state::kinstb(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::kinstb_map);
}

MACHINE_RESET_MEMBER( snesb_state, ffight2b )
{
	address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
	snes_state::machine_reset();

	// Hack: avoid starting with 55 credits. It's either a work RAM init fault or MCU clears it by his own, hard to tell ...
	cpu0space.write_byte(0x7eadce, 0x00);
}

void snesb_state::ffight2b(machine_config &config)
{
	extrainp(config);

	MCFG_MACHINE_RESET_OVERRIDE( snesb_state, ffight2b )
}

void snesb_state::sblast2b(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::sblast2b_map);
}

void snesb_state::endless(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::endless_map);
}

void snesb_state::rushbets(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::rushbets_map);
}

void snesb_state::venom(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::venom_map);
}

void snesb_state::wldgunsb(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snesb_state::wldgunsb_map);
}

void snesb_state::init_kinstb()
{
	uint8_t *rom = memregion("user3")->base();

	for (uint32_t i = 0; i < 0x400000; i++)
	{
		rom[i] = bitswap<8>(rom[i], 5, 0, 6, 1, 7, 4, 3, 2);
	}

	init_snes_hirom();
}

void snesb_state::init_ffight2b()
{
	uint8_t *rom = memregion("user3")->base();

	for (uint32_t i = 0; i < 0x140000; i++)
	{
		rom[i] = rom[i] ^ 0xff;

		if (i < 0x10000)
			rom[i] = bitswap<8>(rom[i], 3, 1, 6, 4, 7, 0, 2, 5);
		else if (i < 0x20000)
			rom[i] = bitswap<8>(rom[i],3, 7, 0, 5, 1, 6, 2, 4);
		else if (i < 0x30000)
			rom[i] = bitswap<8>(rom[i], 1, 7, 6, 4, 5, 2, 3, 0);
		else if (i < 0x40000)
			rom[i] = bitswap<8>(rom[i], 0, 3, 2, 5, 4, 6, 7, 1);
		else if (i < 0x140000)
			rom[i] = bitswap<8>(rom[i], 6, 4, 0, 5, 1, 3, 2, 7);
	}

	// boot vector
	rom[0x7ffd] = 0x89;
	rom[0x7ffc] = 0x54;

	init_snes();
}

void snesb_state::init_ffight3b()
{
	uint8_t *rom = memregion("user3")->base();

	for (int i = 0; i < 0x300000; i++)
	{
		if (i < 0x80000)
			rom[i] = bitswap<8>(rom[i], 7, 4, 2, 0, 3, 5, 6, 1) ^ 0xff;
		else if (i < 0x280000)
			rom[i] = bitswap<8>(rom[i], 0, 5, 1, 3, 2, 7, 6, 4);
		else
			rom[i] = bitswap<8>(rom[i], 4, 7, 0, 2, 5, 3, 1, 6) ^ 0xff;
	}

	// boot vector. TODO: this is the same as the console version, but needs to be verified
	rom[0xfffc] = 0x00;
	rom[0xfffd] = 0xfe;

	// patch out protection
	rom[0xfe33] = 0x5c;
	rom[0xfe34] = 0x00;
	rom[0xfe35] = 0x00;
	rom[0xfe36] = 0xc0;
	rom[0xfeab] = 0x60;

	init_snes_hirom();
}

void snesb_state::init_iron()
{
	uint8_t *rom = memregion("user3")->base();

	for (uint32_t i = 0; i < 0x140000; i++)
	{
		if (i < 0x80000)
			rom[i] = bitswap<8>(rom[i] ^ 0xff, 2, 7, 1, 6, 3, 0, 5, 4);
		else
			rom[i] = bitswap<8>(rom[i], 6, 3, 0, 5, 1, 4, 7, 2);
	}

	init_snes();
}

void snesb_state::init_denseib()
{
	uint8_t *rom = memregion("user3")->base();

	for (uint32_t i = 0; i < 0x200000; i++)
	{
		rom[i] = rom[i] ^ 0xff;
		switch (i >> 16)
		{
			case 0x00: rom[i] = bitswap<8>(rom[i], 1, 7, 0, 6, 3, 4, 5, 2); break;
			case 0x01: rom[i] = bitswap<8>(rom[i], 3, 4, 7, 2, 0, 6, 5, 1); break;
			case 0x02: rom[i] = bitswap<8>(rom[i], 5, 4, 2, 1, 7, 0, 6, 3); break;
			case 0x03: rom[i] = bitswap<8>(rom[i], 0, 1, 3, 7, 2, 6, 5, 4); break;

			default:   rom[i] = bitswap<8>(rom[i], 4, 5, 1, 0, 2, 3, 7, 6); break;
		}
	}

	// boot vector
	rom[0xfffc] = 0x40;
	rom[0xfffd] = 0xf7;

	init_snes_hirom();
}

void snesb_state::init_denseib2()
{
	uint8_t *src = memregion("user7")->base();
	uint8_t *dst = memregion("user3")->base();

	static const uint8_t address_tab_high[0x40] = {
		0x0b, 0x1d, 0x05, 0x15, 0x09, 0x19, 0x04, 0x13, 0x02, 0x1f, 0x07, 0x17, 0x0d, 0x11, 0x0a, 0x1a,
		0x14, 0x0e, 0x18, 0x06, 0x1e, 0x01, 0x10, 0x0c, 0x1b, 0x0f, 0x16, 0x00, 0x12, 0x08, 0x1c, 0x03,
		0x2b, 0x3d, 0x25, 0x35, 0x29, 0x39, 0x24, 0x33, 0x22, 0x3f, 0x27, 0x37, 0x2d, 0x31, 0x2a, 0x3a,
		0x34, 0x2e, 0x38, 0x26, 0x3e, 0x21, 0x30, 0x2c, 0x3b, 0x2f, 0x36, 0x20, 0x32, 0x28, 0x3c, 0x23
	};

	static const uint8_t address_tab_low[0x40] = {
		0x14, 0x1d, 0x11, 0x3c, 0x0a, 0x29, 0x2d, 0x2e, 0x30, 0x32, 0x16, 0x36, 0x05, 0x25, 0x26, 0x37,
		0x20, 0x21, 0x27, 0x28, 0x33, 0x34, 0x23, 0x12, 0x1e, 0x1f, 0x3b, 0x24, 0x2c, 0x35, 0x38, 0x39,
		0x3d, 0x0c, 0x2a, 0x0d, 0x22, 0x18, 0x19, 0x1a, 0x03, 0x08, 0x04, 0x3a, 0x0b, 0x0f, 0x15, 0x17,
		0x1b, 0x13, 0x00, 0x1c, 0x2b, 0x01, 0x06, 0x2f, 0x07, 0x09, 0x02, 0x31, 0x10, 0x0e, 0x3f, 0x3e
	};

	static const uint8_t data_high[16] = {
		0x03, 0x04, 0x85, 0x01, 0x81, 0x87, 0x07, 0x05, 0x86, 0x00, 0x02, 0x82, 0x84, 0x83, 0x06, 0x80
	};

	static const uint8_t data_low[16] = {
		0x30, 0x40, 0x58, 0x10, 0x18, 0x78, 0x70, 0x50, 0x68, 0x00, 0x20, 0x28, 0x48, 0x38, 0x60, 0x08
	};

	for (int i = 0; i < 0x200000; i++)
	{
		int j = (address_tab_high[i >> 15] << 15) + (i & 0x7fc0) + address_tab_low[i & 0x3f];

		dst[i] = data_high[src[j] >> 4] | data_low[src[j] & 0xf];

		if (i >= 0x00000 && i < 0x10000)
			dst[i] = bitswap<8>(dst[i], 2, 1, 3, 0, 7, 4, 5, 6) ^ 0xff;

		if (i >= 0x10000 && i < 0x20000)
			dst[i] = bitswap<8>(dst[i], 1, 7, 4, 5, 6, 0, 3, 2);

		if (i >= 0x20000 && i < 0x30000)
			dst[i] = bitswap<8>(dst[i], 0, 2, 6, 7, 5, 3, 4, 1) ^ 0xff;

		if (i >= 0x30000 && i < 0x40000)
			dst[i] = bitswap<8>(dst[i], 6, 5, 0, 3, 1, 7, 2, 4) ^ 0xff;
	}

	// boot vector
	dst[0xfffc] = 0x40;
	dst[0xfffd] = 0xf7;

	init_snes_hirom();
}

void snesb_state::init_legendsb()
{
	uint8_t *rom = memregion("user3")->base();

	for (int i = 0; i < 0x100000; i++)
	{
		uint8_t val = rom[i] ^ 0xff;

		if (i < 0x10000)
			rom[i] = bitswap<8>(val, 6, 5, 4, 2, 1, 0, 3, 7);
		else if (i < 0x20000)
			rom[i] = bitswap<8>(val, 6, 1, 3, 5, 2, 0, 7, 4);
		else if (i < 0x30000)
			rom[i] = bitswap<8>(val, 2, 6, 3, 0, 4, 5, 7, 1);
		else if (i < 0x40000)
			rom[i] = bitswap<8>(val, 5, 4, 2, 7, 0, 3, 6, 1);
		else
			rom[i] = bitswap<8>(val, 3, 6, 0, 5, 1, 4, 7, 2);
	}

	// boot vector
	rom[0x7ffc] = 0x19;
	rom[0x7ffd] = 0x80;

	init_snes();
}

void snesb_state::init_sblast2b()
{
	uint8_t *src = memregion("user7")->base();
	uint8_t *dst = memregion("user3")->base();

	static const uint8_t address_tab_high[48] = {
		0x0b, 0x1d, 0x05, 0x15, 0x09, 0x19, 0x04, 0x13, 0x02, 0x1f, 0x07, 0x17, 0x0d, 0x11, 0x0a, 0x1a,
		0x14, 0x0e, 0x18, 0x06, 0x1e, 0x01, 0x10, 0x0c, 0x1b, 0x0f, 0x16, 0x00, 0x12, 0x08, 0x1c, 0x03,
		0x2b, 0x2e, 0x25, 0x26, 0x29, 0x21, 0x24, 0x2c, 0x22, 0x2f, 0x27, 0x20, 0x2d, 0x28, 0x2a, 0x23
	};

	static const uint8_t address_tab_low[64] = {
		0x14, 0x1d, 0x11, 0x3c, 0x0a, 0x29, 0x2d, 0x2e, 0x30, 0x32, 0x16, 0x36, 0x05, 0x25, 0x26, 0x37,
		0x20, 0x21, 0x27, 0x28, 0x33, 0x34, 0x23, 0x12, 0x1e, 0x1f, 0x3b, 0x24, 0x2c, 0x35, 0x38, 0x39,
		0x3d, 0x0c, 0x2a, 0x0d, 0x22, 0x18, 0x19, 0x1a, 0x03, 0x08, 0x04, 0x3a, 0x0b, 0x0f, 0x15, 0x17,
		0x1b, 0x13, 0x00, 0x1c, 0x2b, 0x01, 0x06, 0x2f, 0x07, 0x09, 0x02, 0x31, 0x10, 0x0e, 0x3f, 0x3e
	};

	static const uint8_t data_high[16] = {
		0x44, 0x01, 0x51, 0x40, 0x50, 0x55, 0x45, 0x41, 0x15, 0x00, 0x04, 0x14, 0x11, 0x54, 0x05, 0x10
	};

	static const uint8_t data_low[16] = {
		0x88, 0x02, 0x2a, 0x08, 0x28, 0xaa, 0x8a, 0x0a, 0xa2, 0x00, 0x80, 0xa0, 0x22, 0xa8, 0x82, 0x20
	};

	for (int i = 0; i < 0x180000; i++)
	{
		int j = (address_tab_high[i >> 15] << 15) + (i & 0x7fc0) + address_tab_low[i & 0x3f];

		dst[i] = data_high[src[j] >> 4] | data_low[src[j] & 0xf];

		if (i >= 0x00000 && i < 0x10000)
			dst[i] = bitswap<8>(dst[i], 6, 3, 5, 4, 2, 0, 7, 1) ^ 0xff;

		if (i >= 0x10000 && i < 0x20000)
			dst[i] = bitswap<8>(dst[i], 4, 0, 7, 6, 3, 1, 2, 5) ^ 0xff;

		if (i >= 0x20000 && i < 0x30000)
			dst[i] = bitswap<8>(dst[i], 5, 7, 6, 1, 4, 3, 0, 2);

		if (i >= 0x30000 && i < 0x40000)
			dst[i] = bitswap<8>(dst[i], 3, 1, 2, 0, 5, 6, 4, 7) ^ 0xff;
	}

	//  boot vector
	dst[0xfffc] = 0xc0;
	dst[0xfffd] = 0x7a;

	save_item(NAME(m_cnt));

	init_snes_hirom();
}

void snesb_state::init_endless()
{
	uint8_t *src = memregion("user7")->base();
	uint8_t *dst = memregion("user3")->base();

	static const uint8_t address_tab_high[0x40] = {
		0x3b, 0x1d, 0x35, 0x15, 0x39, 0x19, 0x34, 0x13, 0x32, 0x1f, 0x37, 0x17, 0x3d, 0x11, 0x3a, 0x1a,
		0x14, 0x3e, 0x18, 0x36, 0x1e, 0x31, 0x10, 0x3c, 0x1b, 0x3f, 0x16, 0x30, 0x12, 0x38, 0x1c, 0x33,
		0x2b, 0x0d, 0x25, 0x05, 0x29, 0x09, 0x24, 0x03, 0x22, 0x0f, 0x27, 0x07, 0x2d, 0x01, 0x2a, 0x0a,
		0x04, 0x2e, 0x08, 0x26, 0x0e, 0x21, 0x00, 0x2c, 0x0b, 0x2f, 0x06, 0x20, 0x02, 0x28, 0x0c, 0x23
	};

	static const uint8_t address_tab_low[0x40] = {
		0x14, 0x1d, 0x11, 0x3c, 0x0a, 0x29, 0x2d, 0x2e, 0x30, 0x32, 0x16, 0x36, 0x05, 0x25, 0x26, 0x37,
		0x20, 0x21, 0x27, 0x28, 0x33, 0x34, 0x23, 0x12, 0x1e, 0x1f, 0x3b, 0x24, 0x2c, 0x35, 0x38, 0x39,
		0x3d, 0x0c, 0x2a, 0x0d, 0x22, 0x18, 0x19, 0x1a, 0x03, 0x08, 0x04, 0x3a, 0x0b, 0x0f, 0x15, 0x17,
		0x1b, 0x13, 0x00, 0x1c, 0x2b, 0x01, 0x06, 0x2f, 0x07, 0x09, 0x02, 0x31, 0x10, 0x0e, 0x3f, 0x3e
	};

	static const uint8_t data_high[16] = {
		0x88, 0x38, 0x10, 0x98, 0x90, 0x00, 0x08, 0x18, 0x20, 0xb8, 0xa8, 0xa0, 0x30, 0x80, 0x28, 0xb0
	};

	static const uint8_t data_low[16] = {
		0x41, 0x46, 0x02, 0x43, 0x03, 0x00, 0x40, 0x42, 0x04, 0x47, 0x45, 0x05, 0x06, 0x01, 0x44, 0x07
	};

	for (int i = 0; i < 0x200000; i++)
	{
		int j = (address_tab_high[i >> 15] << 15) + (i & 0x7fc0) + address_tab_low[i & 0x3f];

		dst[i] = data_high[src[j] >> 4] | data_low[src[j] & 0xf];

		if (i >= 0x00000 && i < 0x10000)
			dst[i] = bitswap<8>(dst[i], 2, 3, 4, 1, 7, 0, 6, 5);

		if (i >= 0x10000 && i < 0x20000)
			dst[i] = bitswap<8>(dst[i], 1, 5, 6, 0, 2, 4, 7, 3) ^ 0xff;

		if (i >= 0x20000 && i < 0x30000)
			dst[i] = bitswap<8>(dst[i], 3, 0, 1, 6, 4, 5, 2, 7);

		if (i >= 0x30000 && i < 0x40000)
			dst[i] = bitswap<8>(dst[i], 0, 4, 2, 3, 5, 6, 7, 1) ^ 0xff;
	}

	//  boot vector
	dst[0x7ffc] = 0x00;
	dst[0x7ffd] = 0x80;

	save_item(NAME(m_cnt));

	init_snes();
}

void snesb_state::init_rushbets()
{
	uint8_t *src = memregion("user7")->base();
	uint8_t *dst = memregion("user3")->base();

	static const uint8_t address_tab_high[32] = {
		0x0b, 0x1d, 0x05, 0x15, 0x09, 0x19, 0x04, 0x13, 0x02, 0x1f, 0x07, 0x17, 0x0d, 0x11, 0x0a, 0x1a,
		0x14, 0x0e, 0x18, 0x06, 0x1e, 0x01, 0x10, 0x0c, 0x1b, 0x0f, 0x16, 0x00, 0x12, 0x08, 0x1c, 0x03
	};

	static const uint8_t address_tab_low[64] = {
		0x14, 0x1d, 0x11, 0x3c, 0x0a, 0x29, 0x2d, 0x2e, 0x30, 0x32, 0x16, 0x36, 0x05, 0x25, 0x26, 0x37,
		0x20, 0x21, 0x27, 0x28, 0x33, 0x34, 0x23, 0x12, 0x1e, 0x1f, 0x3b, 0x24, 0x2c, 0x35, 0x38, 0x39,
		0x3d, 0x0c, 0x2a, 0x0d, 0x22, 0x18, 0x19, 0x1a, 0x03, 0x08, 0x04, 0x3a, 0x0b, 0x0f, 0x15, 0x17,
		0x1b, 0x13, 0x00, 0x1c, 0x2b, 0x01, 0x06, 0x2f, 0x07, 0x09, 0x02, 0x31, 0x10, 0x0e, 0x3f, 0x3e
	};

	static const uint8_t data_high[16] = {
		0x84, 0x10, 0x92, 0x80, 0x82, 0x96, 0x94, 0x90, 0x16, 0x00, 0x04, 0x06, 0x12, 0x86, 0x14, 0x02
	};

	static const uint8_t data_low[16] = {
		0x28, 0x01, 0x61, 0x20, 0x60, 0x69, 0x29, 0x21, 0x49, 0x00, 0x08, 0x48, 0x41, 0x68, 0x09, 0x40
	};

	for (int i = 0; i < 0x200000; i++)
	{
		int j = (address_tab_high[(i >> 15) & 0x1f] << 15) + (i & 0x107fc0) + address_tab_low[i & 0x3f];

		dst[i] = data_high[src[j] >> 4] | data_low[src[j] & 0xf];

		if (i >= 0x00000 && i < 0x10000)
			dst[i] = bitswap<8>(dst[i], 0, 7, 6, 3, 5, 4, 1, 2) ^ 0xff;

		if (i >= 0x10000 && i < 0x20000)
			dst[i] = bitswap<8>(dst[i], 2, 1, 3, 7, 6, 5, 4, 0) ^ 0xff;

		if (i >= 0x20000 && i < 0x30000)
			dst[i] = bitswap<8>(dst[i], 4, 6, 0, 2, 7, 3, 5, 1);

		if (i >= 0x30000 && i < 0x40000)
			dst[i] = bitswap<8>(dst[i], 5, 4, 7, 1, 0, 6, 2, 3) ^ 0xff;
	}

	// boot vector
	dst[0xfffc] = 0xec;
	dst[0xfffd] = 0x80;

	save_item(NAME(m_cnt));

	init_snes_hirom();
}

void snesb_state::init_venom()
{
	uint8_t *src = memregion("user7")->base();
	uint8_t *dst = memregion("user3")->base();

	static uint8_t address_tab_high[0x60] = {
		0x00, 0x11, 0x02, 0x13, 0x04, 0x15, 0x06, 0x17, 0x08, 0x19, 0x0a, 0x1b, 0x0c, 0x1d, 0x0e, 0x1f,
		0x20, 0x31, 0x22, 0x33, 0x24, 0x35, 0x26, 0x37, 0x28, 0x39, 0x2a, 0x3b, 0x2c, 0x3d, 0x2e, 0x3f,
		0x10, 0x01, 0x12, 0x03, 0x14, 0x05, 0x16, 0x07, 0x18, 0x09, 0x1a, 0x0b, 0x1c, 0x0d, 0x1e, 0x0f,
		0x30, 0x21, 0x32, 0x23, 0x34, 0x25, 0x36, 0x27, 0x38, 0x29, 0x3a, 0x2b, 0x3c, 0x2d, 0x3e, 0x2f,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f
	};

	static uint8_t address_tab_low[0x40] = {
		0x14, 0x1d, 0x11, 0x3c, 0x0a, 0x29, 0x2d, 0x2e, 0x30, 0x32, 0x16, 0x36, 0x05, 0x25, 0x26, 0x37,
		0x20, 0x21, 0x27, 0x28, 0x33, 0x34, 0x23, 0x12, 0x1e, 0x1f, 0x3b, 0x24, 0x2c, 0x35, 0x38, 0x39,
		0x3d, 0x0c, 0x2a, 0x0d, 0x22, 0x18, 0x19, 0x1a, 0x03, 0x08, 0x04, 0x3a, 0x0b, 0x0f, 0x15, 0x17,
		0x1b, 0x13, 0x00, 0x1c, 0x2b, 0x01, 0x06, 0x2f, 0x07, 0x09, 0x02, 0x31, 0x10, 0x0e, 0x3f, 0x3e
	};

	static const uint8_t data_high[16] = {
		0x60, 0x45, 0x04, 0x64, 0x24, 0x00, 0x40, 0x44, 0x01, 0x65, 0x61, 0x21, 0x05, 0x20, 0x41, 0x25
	};

	static const uint8_t data_low[16] = {
		0x0a, 0x92, 0x80, 0x8a, 0x88, 0x00, 0x02, 0x82, 0x10, 0x9a, 0x1a, 0x18, 0x90, 0x08, 0x12, 0x98
	};

	for (int i = 0; i < 0x300000; i++)
	{
		int j = (address_tab_high[i >> 15] << 15) + (i & 0x7fc0) + address_tab_low[i & 0x3f];

		dst[i] = data_high[src[j] >> 4] | data_low[src[j] & 0xf];

		if (i >= 0x00000 && i < 0x10000)
			dst[i] = bitswap<8>(dst[i], 6, 7, 0, 3, 1, 4, 2, 5) ^ 0xff;

		if (i >= 0x10000 && i < 0x20000)
			dst[i] = bitswap<8>(dst[i], 0, 1, 4, 5, 3, 7, 6, 2);

		if (i >= 0x20000 && i < 0x30000)
			dst[i] = bitswap<8>(dst[i], 1, 3, 2, 6, 5, 4, 0, 7) ^ 0xff;

		if (i >= 0x30000 && i < 0x40000)
			dst[i] = bitswap<8>(dst[i], 4, 0, 7, 6, 2, 1, 5, 3);
	}

	// boot vector
	dst[0x7ffc] = 0x98;
	dst[0x7ffd] = 0xff;

	init_snes();
}

void snesb_state::init_wldgunsb()
{
	uint8_t *src = memregion("user7")->base();
	uint8_t *dst = memregion("user3")->base();

	static uint8_t address_tab_high[0x20] = {
		0x0b, 0x1d, 0x05, 0x15, 0x09, 0x19, 0x04, 0x13, 0x02, 0x1f, 0x07, 0x17, 0x0d, 0x11, 0x0a, 0x1a,
		0x14, 0x0e, 0x18, 0x06, 0x1e, 0x01, 0x10, 0x0c, 0x1b, 0x0f, 0x16, 0x00, 0x12, 0x08, 0x1c, 0x03
	};

	static uint8_t address_tab_low[0x40] = {
		0x14, 0x1d, 0x11, 0x3c, 0x0a, 0x29, 0x2d, 0x2e, 0x30, 0x32, 0x16, 0x36, 0x05, 0x25, 0x26, 0x37,
		0x20, 0x21, 0x27, 0x28, 0x33, 0x34, 0x23, 0x12, 0x1e, 0x1f, 0x3b, 0x24, 0x2c, 0x35, 0x38, 0x39,
		0x3d, 0x0c, 0x2a, 0x0d, 0x22, 0x18, 0x19, 0x1a, 0x03, 0x08, 0x04, 0x3a, 0x0b, 0x0f, 0x15, 0x17,
		0x1b, 0x13, 0x00, 0x1c, 0x2b, 0x01, 0x06, 0x2f, 0x07, 0x09, 0x02, 0x31, 0x10, 0x0e, 0x3f, 0x3e
	};

	static const uint8_t data_low[16] = {
		0x30, 0xa8, 0x80, 0xb0, 0x90, 0x00, 0x20, 0xa0, 0x08, 0xb8, 0x38, 0x18, 0x88, 0x10, 0x28, 0x98
	};

	static const uint8_t data_high[16] = {
		0x05, 0x43, 0x40, 0x45, 0x44, 0x00, 0x01, 0x41, 0x02, 0x47, 0x07, 0x06, 0x42, 0x04, 0x03, 0x46
	};

	for (int i = 0; i < 0x100000; i++)
	{
		int j = (address_tab_high[i >> 15] << 15) + (i & 0x7fc0) + address_tab_low[i & 0x3f];

		dst[i] = data_high[src[j] >> 4] | data_low[src[j] & 0xf];

		if (i >= 0x00000 && i < 0x10000)
			dst[i] = bitswap<8>(dst[i], 3, 1, 7, 0, 6, 4, 5, 2) ^ 0xff;

		if (i >= 0x10000 && i < 0x20000)
			dst[i] = bitswap<8>(dst[i], 4, 7, 3, 2, 0, 1, 6, 5);

		if (i >= 0x20000 && i < 0x30000)
			dst[i] = bitswap<8>(dst[i], 6, 0, 7, 1, 4, 3, 5, 2) ^ 0xff;

		if (i >= 0x30000 && i < 0x40000)
			dst[i] = bitswap<8>(dst[i], 0, 2, 6, 4, 1, 5, 7, 3);
	}

	// boot vector
	dst[0x7ffc] = 0x40;
	dst[0x7ffd] = 0x80;

	save_item(NAME(m_cnt));

	init_snes();
}


ROM_START( kinstb )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD( "1.u14", 0x000000, 0x100000, CRC(70889919) SHA1(1451714cbdacb7f6ced2bc7afa478ad7264cf3b7) )
	ROM_LOAD( "2.u15", 0x100000, 0x100000, CRC(e4a5d1da) SHA1(6ae566bd2f740a251d7a81b8ebb92a651cfaac8d) )
	ROM_LOAD( "3.u16", 0x200000, 0x100000, CRC(7a40f7dd) SHA1(cebe632e8d2d68d0619077cc1e931af73c9a723b) )
	ROM_LOAD( "4.u17", 0x300000, 0x100000, CRC(3d7564c1) SHA1(392b513991897668d5dd469ac84a34f785895774) )
ROM_END

ROM_START( ffight2b )
	ROM_REGION( 0x140000, "user3", 0 )
	ROM_LOAD( "ff2_3.u6",  0x000000, 0x008000, CRC(343bf582) SHA1(cc6b7219bb2fe61f0b377b606ad28b0e5a78be0b) )
	ROM_CONTINUE(          0x088000, 0x008000 )
	ROM_CONTINUE(          0x010000, 0x008000 )
	ROM_CONTINUE(          0x098000, 0x008000 )
	ROM_CONTINUE(          0x020000, 0x008000 )
	ROM_CONTINUE(          0x0a8000, 0x008000 )
	ROM_CONTINUE(          0x030000, 0x008000 )
	ROM_CONTINUE(          0x0b8000, 0x008000 )
	ROM_CONTINUE(          0x040000, 0x008000 )
	ROM_CONTINUE(          0x0c8000, 0x008000 )
	ROM_CONTINUE(          0x050000, 0x008000 )
	ROM_CONTINUE(          0x0d8000, 0x008000 )
	ROM_CONTINUE(          0x060000, 0x008000 )
	ROM_CONTINUE(          0x0e8000, 0x008000 )
	ROM_CONTINUE(          0x070000, 0x008000 )
	ROM_CONTINUE(          0x0f8000, 0x008000 )
	ROM_LOAD( "ff2_2.u7",  0x080000, 0x008000, CRC(b2078ae5) SHA1(e7bc3ad26ed672707d0dcfcaff238aad74986532) )
	ROM_CONTINUE(          0x008000, 0x008000 )
	ROM_CONTINUE(          0x090000, 0x008000 )
	ROM_CONTINUE(          0x018000, 0x008000 )
	ROM_CONTINUE(          0x0a0000, 0x008000 )
	ROM_CONTINUE(          0x028000, 0x008000 )
	ROM_CONTINUE(          0x0b0000, 0x008000 )
	ROM_CONTINUE(          0x038000, 0x008000 )
	ROM_CONTINUE(          0x0c0000, 0x008000 )
	ROM_CONTINUE(          0x048000, 0x008000 )
	ROM_CONTINUE(          0x0d0000, 0x008000 )
	ROM_CONTINUE(          0x058000, 0x008000 )
	ROM_CONTINUE(          0x0e0000, 0x008000 )
	ROM_CONTINUE(          0x068000, 0x008000 )
	ROM_CONTINUE(          0x0f0000, 0x008000 )
	ROM_CONTINUE(          0x078000, 0x008000 )
	ROM_LOAD( "ff2_1.u8",  0x100000, 0x040000, CRC(ea315ac1) SHA1(a85de091882d35bc77dc99677511828ff7c20350) )
ROM_END

ROM_START( ffight3b ) // CS101P049-1 PCB
	ROM_REGION( 0x300000, "user3", ROMREGION_ERASEFF )
	ROM_LOAD( "801.u6",  0x000000, 0x080000, CRC(b6c637a7) SHA1(8ab041b9d7ab4318002b11bb876bce8f9764f644) )
	ROM_CONTINUE(        0x280000, 0x080000)
	ROM_LOAD( "801.u7",  0x100000, 0x100000, CRC(efbdd541) SHA1(85c7a674bd976414e916b87239571615d255d7eb) )
	ROM_LOAD( "801.u8",  0x200000, 0x080000, CRC(6e2f7309) SHA1(ad5f37d79590c4bc4b1d33432595eb9d53f1bb90) )
	ROM_CONTINUE(        0x080000, 0x080000)
ROM_END

ROM_START( iron )
	ROM_REGION( 0x140000, "user3", 0 )
	ROM_LOAD( "6.c09.bin", 0x000000, 0x080000, CRC(50ea1457) SHA1(092f9a0e34deeb090b8c88553be3b1596ded60ef) )
	ROM_LOAD( "5.c10.bin", 0x080000, 0x080000, CRC(0c3a0b5b) SHA1(1e8ab860689137e0e94731f1af2cfc561492b5bd) )
	ROM_LOAD( "4.c11.bin", 0x100000, 0x040000, CRC(2aa417c7) SHA1(24b375e5bbd4be5dcd31b63ea98fbbadd53d543e) )
ROM_END

ROM_START( denseib )
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )
	ROM_LOAD( "dj.u14", 0x000000, 0x0080000, CRC(487ded13) SHA1(624edce30fe2f2d750bcb49c609ceb511b2279b1) )
	ROM_LOAD( "dj.u15", 0x080000, 0x0080000, CRC(5932a440) SHA1(6048372268a097b08d9f56ad30f083267d798165) )
	ROM_LOAD( "dj.u16", 0x100000, 0x0080000, CRC(7cb71fd7) SHA1(7673e9dcaabe804e2d637e67eabca1683dad4245) )
	ROM_LOAD( "dj.u17", 0x180000, 0x0080000, CRC(de29dd89) SHA1(441aefbc7ee64515ee66431ef504e76dc8dc5ca3) )
ROM_END

ROM_START( denseib2 )
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "user7", 0 )
	ROM_LOAD( "u31.bin", 0x000000, 0x080000, CRC(834723a8) SHA1(3f56bba5017f77147e7d52618678f1e2eff4991b) )
	ROM_LOAD( "u32.bin", 0x080000, 0x080000, CRC(9748e86b) SHA1(68a62e0961d735602ae6ebd1aca5990c588ccbb1) )
	ROM_LOAD( "u33.bin", 0x100000, 0x080000, CRC(abcc6b61) SHA1(ef90f23b674f6dd36b3d60c9c395a1d4bc853798) )
	ROM_LOAD( "u34.bin", 0x180000, 0x080000, CRC(0a16ac96) SHA1(ddc11009d4b35a151aa7e357346f3ac109e112ef) )
ROM_END

ROM_START( sblast2b )
	ROM_REGION( 0x180000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x180000, "user7", 0 )
	ROM_LOAD( "1.bin", 0x000000, 0x0080000, CRC(bea10c40) SHA1(d9cc65267b9b57145d714f2c17b436c1fb21513f) )
	ROM_LOAD( "2.bin", 0x080000, 0x0080000, CRC(57d2b6e9) SHA1(1a7b347101f67b254e2f86294d501b0669431644) )
	ROM_LOAD( "3.bin", 0x100000, 0x0080000, CRC(9e63a5ce) SHA1(1d18606fbb28b55a921fc37e1af1aff4caae9003) )
ROM_END

ROM_START( sblast2ba) // all 27c4000
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "user7", 0 )
	ROM_LOAD( "u14", 0x000000, 0x080000, CRC(d2bdc126) SHA1(fa8f03b73f2f9b7a159699b764e2c46b5f8a8190) )
	ROM_LOAD( "u15", 0x080000, 0x080000, CRC(50f9acb1) SHA1(a86bc98f81dc2c9443fbcd9b5f4880b8d5851ed6) )
	ROM_LOAD( "u16", 0x100000, 0x080000, CRC(2a7f40f3) SHA1(e0db49969880af1edbadd8bc5a1bc59a55777d23) )
	ROM_LOAD( "u17", 0x180000, 0x080000, CRC(09817173) SHA1(e2ad9e7e1a95cde9ee973647dbd5df83b524978b) )
ROM_END

ROM_START( legendsb )
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "u37_0", 0x000000, 0x080000, BAD_DUMP CRC(44101f23) SHA1(7563886598b290faa616397f7e87a56e2f984b79) ) // U37 ROM is bad, was unable to get stable reads
	ROM_LOAD( "u37_1", 0x000000, 0x080000, BAD_DUMP CRC(d2e835bb) SHA1(0620e099f43cde95d6b4b210eef13abbff5f40e9) )
	ROM_LOAD( "u37_2", 0x000000, 0x008000, BAD_DUMP CRC(1bc6f429) SHA1(eb4e1a483d2aa545a1ba33243afd9693ee5bebd0) )
	ROM_CONTINUE(      0x088000, 0x008000 )
	ROM_CONTINUE(      0x010000, 0x008000 )
	ROM_CONTINUE(      0x098000, 0x008000 )
	ROM_CONTINUE(      0x020000, 0x008000 )
	ROM_CONTINUE(      0x0a8000, 0x008000 )
	ROM_CONTINUE(      0x030000, 0x008000 )
	ROM_CONTINUE(      0x0b8000, 0x008000 )
	ROM_CONTINUE(      0x040000, 0x008000 )
	ROM_CONTINUE(      0x0c8000, 0x008000 )
	ROM_CONTINUE(      0x050000, 0x008000 )
	ROM_CONTINUE(      0x0d8000, 0x008000 )
	ROM_CONTINUE(      0x060000, 0x008000 )
	ROM_CONTINUE(      0x0e8000, 0x008000 )
	ROM_CONTINUE(      0x070000, 0x008000 )
	ROM_CONTINUE(      0x0f8000, 0x008000 )
	ROM_LOAD( "u36",   0x080000, 0x008000, CRC(c33a5362) SHA1(537b1b7ef22baa289523fac8f9843db155408c56) )
	ROM_CONTINUE(      0x008000, 0x008000 )
	ROM_CONTINUE(      0x090000, 0x008000 )
	ROM_CONTINUE(      0x018000, 0x008000 )
	ROM_CONTINUE(      0x0a0000, 0x008000 )
	ROM_CONTINUE(      0x028000, 0x008000 )
	ROM_CONTINUE(      0x0b0000, 0x008000 )
	ROM_CONTINUE(      0x038000, 0x008000 )
	ROM_CONTINUE(      0x0c0000, 0x008000 )
	ROM_CONTINUE(      0x048000, 0x008000 )
	ROM_CONTINUE(      0x0d0000, 0x008000 )
	ROM_CONTINUE(      0x058000, 0x008000 )
	ROM_CONTINUE(      0x0e0000, 0x008000 )
	ROM_CONTINUE(      0x068000, 0x008000 )
	ROM_CONTINUE(      0x0f0000, 0x008000 )
	ROM_CONTINUE(      0x078000, 0x008000 )
ROM_END

ROM_START( endless )
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "user7", 0 )
	ROM_LOAD( "endlessduel.unknownposition1", 0x000000, 0x80000, CRC(e49acd29) SHA1(ac137261fe7a7691738ac812bea9591256eb9038) )
	ROM_LOAD( "endlessduel.unknownposition2", 0x080000, 0x80000, CRC(ad2052f9) SHA1(d61382e3d93eb0bff45fb534cec0ce5ae3626165) )
	ROM_LOAD( "endlessduel.unknownposition3", 0x100000, 0x80000, CRC(30d06d7a) SHA1(17c617d94abb10c3bdf9d51013b116f4ef4debe8) )
	ROM_LOAD( "endlessduel.unknownposition4", 0x180000, 0x80000, CRC(9a9493ad) SHA1(82ee4fce9cc2014cb8404fd43eebb7941cdb9ac1) )
ROM_END

ROM_START( endlessa )
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "user7", 0 )
	ROM_LOAD( "gundam wing endless duel.c23", 0x000000, 0x80000, CRC(e49acd29) SHA1(ac137261fe7a7691738ac812bea9591256eb9038) )
	ROM_LOAD( "gundam wing endless duel.c20", 0x080000, 0x80000, CRC(cf22a554) SHA1(86a31c83a1d28038c334949e82182c07010ccb3c) )
	ROM_LOAD( "gundam wing endless duel.c22", 0x100000, 0x80000, CRC(30d06d7a) SHA1(17c617d94abb10c3bdf9d51013b116f4ef4debe8) )
	ROM_LOAD( "gundam wing endless duel.c21", 0x180000, 0x80000, CRC(0cc430c0) SHA1(fb36359e7e3919244c47a6da43f31dc2a79fcba6) )
ROM_END

ROM_START( rushbets )
	ROM_REGION( 0x200000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "user7", 0 )
	ROM_LOAD( "ic19.bin", 0x000000, 0x80000, CRC(8aa0ad59) SHA1(83facb65c53ade99f1f057a8de27bee4a9c2efd8) )
	ROM_LOAD( "ic20.bin", 0x080000, 0x80000, CRC(a8afe28b) SHA1(16d1c4f957804d22dc05a97c56ae10c408dbc1f2) )
	ROM_LOAD( "ic21.bin", 0x100000, 0x80000, CRC(2f6e8711) SHA1(fe4030ef3445594455fe93e374a41e9ba2147bf6) )
	ROM_LOAD( "ic22.bin", 0x180000, 0x80000, CRC(95a234d2) SHA1(31a556c8ed395f61ba198631ee086c18cc740792) )
ROM_END

ROM_START( venom )
	ROM_REGION( 0x300000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x300000, "user7", 0 )
	ROM_LOAD( "u31.bin", 0x000000, 0x0100000, CRC(d1034a76) SHA1(541dd92197ca2e4eb686e426c840aad847d02be8) )
	ROM_LOAD( "u32.bin", 0x100000, 0x0100000, CRC(fbe865b0) SHA1(25467a6faa912bf180c5dd7aecee77c3b5f207f8) )
	ROM_LOAD( "u33.bin", 0x200000, 0x0080000, CRC(ed874ca2) SHA1(cfc90b38ea2eea07e990f0b72d7c1af2a7076beb) )
	ROM_LOAD( "u34.bin", 0x280000, 0x0080000, CRC(7a09c9e0) SHA1(794965d5501ec0e21f1f3a8cb8fd66f913d42760) )
ROM_END

ROM_START( wldgunsb )
	ROM_REGION( 0x100000, "user3", ROMREGION_ERASEFF )

	ROM_REGION( 0x100000, "user7", 0 )
	ROM_LOAD( "c19.bin", 0x000000, 0x080000, CRC(59df0dc8) SHA1(d18b7f204ad4e0fcd64c2e2a25d60b64930419e7) )
	ROM_LOAD( "c20.bin", 0x080000, 0x080000, CRC(62ae4acb) SHA1(62aa320bcc7eeedb00c70baa909ac0230256c9a4) )
ROM_END

} // Anonymous namespace


GAME( 199?, kinstb,       0,        kinstb,       kinstb,   snesb_state, init_kinstb,    ROT0, "bootleg",  "Killer Instinct (SNES bootleg)",                         MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, ffight2b,     0,        ffight2b,     ffight2b, snesb_state, init_ffight2b,  ROT0, "bootleg",  "Final Fight 2 (SNES bootleg)",                           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 199?, ffight3b,     0,        extrainp,     ffight2b, snesb_state, init_ffight3b,  ROT0, "bootleg",  "Final Fight 3 (SNES bootleg)",                           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // based on beta version? protection isn't figured out
GAME( 1996, iron,         0,        extrainp,     iron,     snesb_state, init_iron,      ROT0, "bootleg",  "Iron (SNES bootleg)",                                    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, denseib,      0,        extrainp,     denseib,  snesb_state, init_denseib,   ROT0, "bootleg",  "Ghost Chaser Densei (SNES bootleg, set 1)",              MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, denseib2,     denseib,  extrainp,     denseib,  snesb_state, init_denseib2,  ROT0, "bootleg",  "Ghost Chaser Densei (SNES bootleg, set 2)",              MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sblast2b,     0,        sblast2b,     sblast2b, snesb_state, init_sblast2b,  ROT0, "bootleg",  "Sonic Blast Man II Special Turbo (SNES bootleg, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sblast2ba,    sblast2b, base,         sblast2b, snesb_state, empty_init,     ROT0, "bootleg",  "Sonic Blast Man II Special Turbo (SNES bootleg, set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // needs to be descrambled
GAME( 1996, endless,      0,        endless,      endless,  snesb_state, init_endless,   ROT0, "bootleg",  "Gundam Wing: Endless Duel (SNES bootleg, set 1)",        MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, endlessa,     endless,  endless,      endless,  snesb_state, init_endless,   ROT0, "bootleg",  "Gundam Wing: Endless Duel (SNES bootleg, set 2)",        MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, legendsb,     0,        extrainp,     kinstb,   snesb_state, init_legendsb,  ROT0, "bootleg",  "Legend (SNES bootleg)",                                  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rushbets,     0,        rushbets,     rushbets, snesb_state, init_rushbets,  ROT0, "bootleg",  "Rushing Beat Shura (SNES bootleg)",                      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, venom,        0,        venom,        venom,    snesb_state, init_venom,     ROT0, "bootleg",  "Venom & Spider-Man - Separation Anxiety (SNES bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, wldgunsb,     0,        wldgunsb,     wldgunsb, snesb_state, init_wldgunsb,  ROT0, "bootleg",  "Wild Guns (SNES bootleg)",                               MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // based off Japanese version
