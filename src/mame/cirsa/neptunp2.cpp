// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina
/***************************************************************************

    Unidesa/Cirsa "960606-5" hardware.
    Skeleton driver.

****************************************************************************

The "960606-5" PCB is used at least on the following games:

  Unidesa/Cirsa Blue Swamp Land
  Unidesa/Cirsa Corsarios
  Unidesa/Cirsa Dinópolis
  Unidesa/Cirsa Euro Bingo 7 (1000)
  Unidesa/Cirsa Euro Bingo 7
  Unidesa/Cirsa Euro Lucky
  Unidesa/Cirsa Far West
  Unidesa/Cirsa Filón
  Unidesa/Cirsa Gladiadores
  Unidesa/Cirsa La Perla del Caribe
  Unidesa/Cirsa Las Llaves del Tesoro
  Unidesa/Cirsa Legend
  Unidesa/Cirsa Max Money
  Unidesa/Cirsa Megatrón
  Unidesa/Cirsa Megatrón Salón
  Unidesa/Cirsa Millenium
  Unidesa/Cirsa Mini Genio
  Unidesa/Cirsa Mini Guay Plus
  Unidesa/Cirsa Mini Joker
  Unidesa/Cirsa Monsters Manía
  Unidesa/Cirsa Multi Points
  Unidesa/Cirsa Nevada
  Unidesa/Cirsa Rock 'n' Roll
  Unidesa/Cirsa Saloon
  Unidesa/Cirsa Super Sevens
  Unidesa/Cirsa Secreto de la Pirámide
  Unidesa/Cirsa Vikingos
  Unidesa/Cirsa Vulcano

The same hardware from Unidesa/Cirsa was also used on some games from
"Europea de Investigaciones Electrónicas S.A.":

  Unidesa/Cirsa/Europea Atlantis
  Unidesa/Cirsa/Europea Bingo Lotto
  Unidesa/Cirsa/Europea Charleston
  Unidesa/Cirsa/Europea Extra Cash
  Unidesa/Cirsa/Europea Mississippi Casino
  Unidesa/Cirsa/Europea Oklahoma Express
  Unidesa/Cirsa/Europea Ruleta de la Fortuna

This hardware was also used in several fruit machines released for the UK market by B. Gaming Technology Ltd.
B. Gaming Technology (BGT) was a brand of International Amusement Machine Corporation (IAMC), which was a
member of the CIRSA Corporation.
BGT was was formed in 1995 in Cannock, Staffordshire, to develop and supply gaming machines for the UK market.


 CIRSA / UNIDESA 960606-5 CPU BOARD
 _________________________________________________________________
 |        ________                                  ____________  |
 |__      |ULN2003|                                 | ........  | |
 || |__ _  __________________                       | ____      | |
 ||P||P||| |OTP 27C8000 or  |   _______    ________ | X9313     |<- CB1 (CS4)
 ||1||1||| |27C4001_-_SOUND_|   |OKI   |   |S1 DIPS||           | |
 || ||5|P9 __________________   |MSM6376   |_______|| _________ | |
 ||_||_|   |    27C8000 or  |   |______|   _________| |PAT_PLD_(PAL16L8 or equivalent)
 |         |27C4001_-_SOUND_|              |S2 DIPS|| ····      | |
 |__                                       |_______||___________| |
 ||P|      __________________  __________________    ____    BATT |
 ||7|      |27C801 or       |  |RAM MS62256-79  |    8583P   3V6  |
 ||_|      |27C4001_________|  |________________|           179mAh|
 |__       __________________  __________________                 |
 ||P|__    |27C801 or       |  |MS628512        |    ____         |
 ||1||P|   |27C4001_________|  |NOT_POPULATED___|   X24C16        |
 ||1||18              _______                                     |
 || |__               |CPLD |    ________    ____________         |
 ||_||P|              |PD   |    |_75189_|   | CIRSA     |     __ |
 |   |17              |XC9536  NOT POPULATED | 38302 or  |     |P||
 |   |_|     XTAL                ________    | 38304     |     |2||
 |__         36.8640MHz          |_75188_|   |           |     | ||
 ||P|      ___________         NOT POPULATED |___________|     | ||
 ||3|      |CPU       |      ________          ________   ____ |_||
 ||_|      |80C188XL  |      |7407___|         |7406___| LM393    |
 |         |          |      ________          ________           |
 |__       |          |      |74HC14_|         |74HC00_|          |
 ||P|      |__________|      ________          ________           |
 ||8|        ________        |74HC14_|         |74HC14_|          |
 || |        |74HC14_|       ________                             |
 || |      ____________      |74HCT08|       ____________         |
 ||_| __   | CIRSA     |     ________        | CIRSA     |        |
 |    |P|  | 38302 or  |     |74HCT14|       | 38302 or  |        |
 |__  |1|  | 38304     |     ________        | 38304     |        |
 ||P| |3|  | MASTER    |     |74HCT14|       |           |        |
 ||5| __   |___________|                     |___________|        |
 || | |P|                                                         |
 || | |4|    ________        ________           ________          |
 ||_| | |    |ULN2064        |ULN2064           |74LS145 <- NOT POPULATED
 |    | |    ________        ________         __________          |
 |    | |    |ULN2064        |ULN2064         |UDN2580A| <- NOT POPULATED
 |    |_|   __________       ________         __________          |
 |          |___P14___|      |ULN2064         |_ARRAY__| <- NOT POPULATED
 |                          NOT POPULATED                 ______  |
 |_________________________________________________________P19____|

P4, P13, P15, P16 and P19 are unused.
Games without video support have the P8 without a connector soldered in.

The CS4 security counters module is a black plastic box with an anti-tamper sticker for
auditing the machine financials. It sits on top of the PAT security PAL and the X9313
using two connectors (4-pin on one side and 8-pin on the other).
It's just a small PCB with a SEEPROM:
 _______________________
 |: ···                 |
 |:  _____      _____  ·|
 |: TLP521-2   24C16WP ·|
 |:                    ·|
 |______________________|

According with fhe Spanish laws (at that time) for slot machines, it was mandatory to destroy
the program ROMs and the security counters module for retiring the machines from service. That's
the reason why most games on this driver are missing these ROMs.

The service manual contains the complete PCB schematics:
https://media.recreativas.org/manuales/201909/cirsa-unidesa-carta-control-960606-5-manual.pdf

Games can support video thanks to an additional PCB (called "IS040302-3 VGA SOC-Legacy PCB"),
connected to P8:
            _________
 ___________|VGA HD15|_______________________________________
 |          |________|     :::::::::::::: <- Conn to 960606-5 PCB
 |                        ___  ___  ___  ___                 |
 |             _____     |__| |__| |__| |__|                 |
 |             |    |                        ______  ·       |
 |             |____|             ___40.000  |XILINX ·       |
 |            ADV7123   ________  |_|MHz     |     | ·       |
 |                      |XILINX | XT1  ____  |_____| ·       |
 |                      |SPARTAN|      |   | XC9536XV        |
 |                      |XC3S400|      |SRAM     ____  ____  |
 |                      |_______|      |   |     |   | |   | |
 |                                     |___|     |___| |___| |
 |                                    71V124  LT1963A LT1963A|
 |  ______________________________________                   |
 |  |_SIMM_3__BACKGROUND_A_______________|                   |
 |  ______________________________________                   |
 |  |_SIMM_2__BACKGROUND_B_______________|             ____  |
 |  ______________________________________             |   | |
 |  |_SIMM_1__WINDOW_A___________________|             |___| |
 |  ______________________________________           LT1963A |
 |  |_SIMM_0__WINDOW_B___________________|                   |
 |___________________________________________________________|

 Each SIMM contains two AM29LV128/256 flash chips (or compatible), and the
 SIMMs PCBs are labeled as "IS040103-2 AMD/FUJITSU SIMM FLASH 3.3V 16bits".

 On every game (with or without video), a Samsung VFD display (1x16) can be connected
 to connector P14 for diagnostics and configuration.
 Sometimes this display is externally exposed so it shows game texts and messages to the players.

 Most games (with or without video) have their serial number hardcoded on the program ROMs.

*/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "emupal.h"
#include "screen.h"
#include "sound/okim6376.h"
#include "speaker.h"


namespace {

class neptunp2_state : public driver_device
{
public:
	neptunp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_okim6376(*this, "oki")
	{ }

	void neptunp2_video(machine_config &config);
	void neptunp2_no_video(machine_config &config);

protected:
	// driver_device overrides
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t test_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void neptunp2_io(address_map &map) ATTR_COLD;
	void neptunp2_video_map(address_map &map) ATTR_COLD;
	void neptunp2_no_video_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<okim6376_device> m_okim6376;
};

static INPUT_PORTS_START(c960606)
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

void neptunp2_state::video_start()
{
}

uint32_t neptunp2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

uint8_t neptunp2_state::test_r()
{
	return machine().rand();
}

void neptunp2_state::neptunp2_no_video_map(address_map &map)
{
	map(0x00000, 0xbffff).rom();
	map(0xe0000, 0xeffff).ram();

	map(0xdb004, 0xdb007).ram();
	map(0xdb00c, 0xdb00f).ram();

	map(0xff806, 0xff806).r(FUNC(neptunp2_state::test_r));
	map(0xff810, 0xff810).r(FUNC(neptunp2_state::test_r));
	map(0xff812, 0xff812).r(FUNC(neptunp2_state::test_r));

	map(0xff980, 0xff980).nopw();

	map(0xffff0, 0xfffff).rom();
}

void neptunp2_state::neptunp2_video_map(address_map &map)
{
	neptunp2_no_video_map(map);

	map(0xd0000, 0xd7fff).ram(); //videoram
}

void neptunp2_state::neptunp2_io(address_map &map)
{
}


static INPUT_PORTS_START( neptunp2 )
INPUT_PORTS_END

#if 0
static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,3),  // 1024 characters
	3,  // 3 bits per pixel
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },    // The bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // Every char takes 8 consecutive bytes
};
#endif

static GFXDECODE_START( gfx_neptunp2 )
//  GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 8 )
GFXDECODE_END

void neptunp2_state::neptunp2_no_video(machine_config &config)
{
	// Basic machine hardware
	I80188(config, m_maincpu, 36.864_MHz_XTAL); // N80C188-20 AMD
	m_maincpu->set_addrmap(AS_PROGRAM, &neptunp2_state::neptunp2_no_video_map);
	m_maincpu->set_addrmap(AS_IO, &neptunp2_state::neptunp2_io);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6376(config, "oki", 36.864_MHz_XTAL/32).add_route(ALL_OUTPUTS, "mono", 1.0); // Frequency divisor is a guess
}

void neptunp2_state::neptunp2_video(machine_config &config)
{
	neptunp2_no_video(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &neptunp2_state::neptunp2_video_map);
	m_maincpu->set_vblank_int("screen", FUNC(neptunp2_state::irq0_line_hold));

	// Video hardware (probably wrong values, as the video board outputs VGA resolution)
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(neptunp2_state::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_neptunp2);
	PALETTE(config, "palette").set_entries(512);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

/***************************************************************************
Games on Cirsa "960606-5" PCB with additional video board
***************************************************************************/

// It's unclear what exact video board is used on this game
ROM_START( neptunp2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u2.bin",   0x000000, 0x100000, CRC(4fbb06d1) SHA1(6490cd3b96b3b61f48fcb843772bd787605ab76f) )

	ROM_REGION( 0x100000, "prg_data", 0 ) //dunno how this maps ...
	ROM_LOAD( "u3.bin",   0x000000, 0x100000, CRC(3c1746e2) SHA1(a7fd59f5397ce1653848e15f16399b537f3a1ea7) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "u14.bin",  0x000000, 0x100000, CRC(a2de1156) SHA1(58b325b720057e8d7105fe3a87ac2c0109afad84) )
	ROM_LOAD( "u15.bin",  0x100000, 0x100000, CRC(8de6d4de) SHA1(121e7507ef57074bc7ad0bf69556f26c84c4e236) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "flash_roms", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "eeprom.u10", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pal16l8.u6", 0x000, 0x104, NO_DUMP )
ROM_END

/* '61509960606-5 PCB (exactly the same as '960606-5', but with better quality connectors) and 'CB1 (CS4)' security counters module. CPLD labeled as 'PD11'.
   There's a small piggyback PCB with a LS14 connected to the 75188 and 75189 sockets (usually not populated on other games).
   Uses Cirsa IS040302-3 VGA SOC-Legacy PCB for video and Cirsa 615092000401-3 PCB for reels control (20MHz xtal + PIC16F76 as main CPU).
*/
ROM_START( perlacrb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "c.la_perla_del_caribe_600_cvb-0092-e_a_v1.0_cat_b082.u2", 0x000000, 0x100000, CRC(73f38d0a) SHA1(ca198e26a057ab7ce3479fff1287ff265306ab1f) )

	ROM_REGION( 0x100000, "prg_data", 0 )
	ROM_LOAD( "c.la_perla_del_caribe_600_cvb-0092-e_b_v1.0_cat_b082.u3", 0x000000, 0x100000, CRC(b306d9be) SHA1(29e2d289770bec9c89f3938ecb8b8747477c3860) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "s-436_otp.u14", 0x000000, 0x100000, CRC(52c08401) SHA1(2cd5110bb433996f6afdf48e68c65554d393dd14) )
	ROM_LOAD( "s-437_otp.u15", 0x100000, 0x100000, CRC(23cc1ab1) SHA1(d1fddb8c742a356703993cee35dc3f5d6ee3d6ea) )

	ROM_REGION( 0x8000400, "gfx", 0 )
	// SIMM 0, "Window B", not present
	// SIMM 1, "Window A", dated 15/May/2008
	ROM_LOAD( "la_perla_del_caribe_bq_graf.-es_w1-1-2_v_g-1f019.u1",  0x0000000, 0x2000100, CRC(1e613ecf) SHA1(7036855b29f796a936aac5084acd6a49bc614dd8) )
	ROM_LOAD( "la_perla_del_caribe_bq_graf.-es_w1-1-2_v_g-1f019.u2",  0x2000100, 0x2000100, CRC(229c83e3) SHA1(647c11b55ded105ec21bc57aad7b65575966ff9c) )
	// SIMM 2, "Background B", not present
	// SIMM 3, "Background A", dated 15/May/2008
	ROM_LOAD( "la_perla_del_caribe_bq_graf.-es_bg1-2-2_v_g-3f019.u1", 0x4000200, 0x2000100, CRC(2dd9db7f) SHA1(a6fc4ebaf536933bc901699c21d65ad1eb7baaad) )
	ROM_LOAD( "la_perla_del_caribe_bq_graf.-es_bg1-2-2_v_g-3f019.u2", 0x6000300, 0x2000100, CRC(c391c42c) SHA1(a3416f6ed0de7898cf7205fc88499cc27eb9471d) )

	// Reels PCB 2000401-3
	ROM_REGION( 0x2000, "reels", 0 )
	ROM_LOAD( "pic16f76.u11", 0x0000, 0x2000, NO_DUMP ) // 8KB internal ROM, undumped

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, CRC(554805fa) SHA1(a79a1617c70e02d8100e6f38062a9aa15141c73c) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4_st24c16.bin", 0x000, 0x800, CRC(16fb7d4f) SHA1(e795731d041bbfd21270d774f3c1d7d4e91c4a15) ) // Probably contains operator data

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat063-1_gal16v8d.u6", 0x000, 0x104, NO_DUMP ) // "PAT-063/1", protected
ROM_END


/***************************************************************************
Games on Cirsa "960606-5" PCB without video
***************************************************************************/

// '960606-5 PCB and 'CB1 (CS4)' security counters module. CPLD labeled as 'PD03'. This is mechanical.
ROM_START( ccorsario )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "c_corsarios_50_b-2057-a_a_6.0d_b-0082.u2",   0x000000, 0x080000, CRC(3a68e622) SHA1(0fddf47915b1eb584eb9fc1a2ca611582629ace1) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "cirsa_corsarios_b-2057-a_s127_1.0_b-82.u14", 0x000000, 0x080000, CRC(f15ccc6b) SHA1(08f2f0129634075297d4a6b9697ba5bd0c8455ce) )
	ROM_LOAD( "cirsa_corsarios_b-2057-a_s128_1.0_b-82.u15", 0x080000, 0x080000, CRC(061dc7c8) SHA1(a9c8da9c2e7cecd8800974ce70546cc60391cfe8) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, CRC(0212ae8f) SHA1(d1db767bd4314894e0cbf8063290fe3260646843) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat_062_tibpal16l8-25cn.u6", 0x000, 0x104, NO_DUMP ) // "PAT 062", protected
ROM_END

// '960606-5 PCB and 'CB1 (CS4)' security counters module. CPLD labeled as 'PD03'. This is mechanical.
ROM_START( ccorsarioa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "c_corsarios_50_b-hm0023_b_5.01_cat_b-0082.u2", 0x000000, 0x080000, CRC(b262a36d) SHA1(6c403e5418cde12da87148a44084c7a8866c5005) )
	ROM_LOAD( "c_corsarios_50_b-hm0023_b_5.01_cat_b-0082.u3", 0x080000, 0x080000, CRC(fda606f4) SHA1(5f17fe5139e15c738b0c6e354f43221a3a60d807) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "cirsa_corsario_50_hm0023_s127_1.0_b-82.u14",   0x000000, 0x080000, CRC(f15ccc6b) SHA1(08f2f0129634075297d4a6b9697ba5bd0c8455ce) )
	ROM_LOAD( "cirsa_corsario_50_hm0023_s128_1.0_b-82.u15",   0x080000, 0x080000, CRC(061dc7c8) SHA1(a9c8da9c2e7cecd8800974ce70546cc60391cfe8) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, CRC(4254a4e1) SHA1(8d3aeb29a63bbdb8de8c33806af73a6aba910e30) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat_062_tibpal16l8-25cn.u6", 0x000, 0x104, NO_DUMP ) // "PAT 062", protected
ROM_END

// '960606-5 PCB and 'CB1 (CS4)' security counters module. CPLD labeled as 'PD18'. This is mechanical.
ROM_START( charles )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "charles1_catalana_b-h240151_v2.1_b-256_europea_iesa.u2", 0x000000, 0x100000, CRC(41df7dc2) SHA1(b7045bb52f098022e07bd7f303e247e9390348ff) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s-1.0_otp_char.u14", 0x000000, 0x100000, CRC(4b10093e) SHA1(872a4b7f4abcb302163a2ca77149599c1d338c1b) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24c16.u10", 0x000, 0x800, CRC(9ec16fc3) SHA1(dc4dccc766aceb4fc6a009ffc45a7dbdf6d16d32) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat_063_tibpal16l8-25cn.u6", 0x000, 0x104, NO_DUMP ) // "PAT 063/1", protected
ROM_END

/* '960606-5 PCB and 'CB1 (CS4)' security counters module. CPLD labeled as 'PD18'. This is mechanical.
   A complete manual with schematics can be downloaded from https://www.recreativas.org/manuales
   There's a newer model running on CPU PCB 2060608-3 (different ROMs, undumped). */
ROM_START( gladiador )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cirsa_gladiadores_b-m240891_v1.0_caa_b-82.u2", 0x000000, 0x100000, CRC(a34148d1) SHA1(5401cd1b6b7ec00c0ed9579fe24ecee71834d219) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s-431_otp.u14", 0x000000, 0x100000, CRC(5d0d36ec) SHA1(3cafe09fc8d802abb0197c1ed89d3ad07f9a67f7) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24c16.u10", 0x000, 0x800, CRC(a38693a9) SHA1(bad306f3e50dbb60cbf59a09f1e4720576bb3983) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "225c_b-h240891_cs4_24c16w.bin", 0x000, 0x800, CRC(fd418bd7) SHA1(c257b11288aa70ae986fcf8cbdfcbc4bc83dbd45) ) // Dump for machine with serial 09-01000 (the SEEPROM is probably tied to the serial number)

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat_063_tibpal16l8-25cn.u6", 0x000, 0x104, NO_DUMP ) // "PAT 063/1", protected

	// Two reels PCBs 615092000401-3
	ROM_REGION( 0x2000, "reels", 0)
	ROM_LOAD( "pic16f76_lower.u11", 0x000, 0x2000, NO_DUMP ) // 8KB internal ROM, undumped
	ROM_LOAD( "pic16f76_upper.u11", 0x000, 0x2000, NO_DUMP ) // 8KB internal ROM, undumped

	// "Lower Additional Game" PCB 615032060623-2
	ROM_REGION( 0x4000, "lower", 0)
	ROM_LOAD( "pic16f630.u11", 0x000, 0x4000, NO_DUMP ) // 16KB internal ROM, undumped
ROM_END

/* '61509960606-5 PCB (exactly the same as '960606-5', but with better quality connectors) and 'CB1 (CS4)' security counters module.
   CPLD labeled as 'PD18'. There's a sticker on the PCB with the date '26/01/2007'.
   There's a small piggyback PCB with a LS14 connected to the 75188 and 75189 sockets (usually not populated on other games).
   This model has the Samsung VFD display (1x16) exposed to the player (on other games it's usually hidden, just for operator use). */
ROM_START( mltpoints )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "c_multi_points_ro_v1.0_caa_03bf9e68_checksum.u2", 0x000000, 0x100000, CRC(26c5a62c) SHA1(e376eb84a75e4ac0b3beb5b1bd0aaf7bd0c3b3cc) )
	// U3 not used

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "s-427_otp.u14", 0x000000, 0x100000, CRC(2cc39293) SHA1(7e19ef8ad2b95e30c169c87387f14a7350e4aff8) )
	// U15 not used

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat.u6", 0x000, 0x104, NO_DUMP )
ROM_END

// '960606-5 PCB and 'CB1 (CS4)' security counters module. It was found with most sockets unpopulated. This is mechanical.
ROM_START( rockroll )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u2", 0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x100000, "prg_data", 0 )
	ROM_LOAD( "u3", 0x000000, 0x100000, NO_DUMP ) // it's also possible it wasn't ever populated

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "c.rock_n_roll_b-2103_6219_otp_b-82_m27c801.u14", 0x000000, 0x100000, CRC(963d184b) SHA1(8ad8b3215d3fc513dfae27bea2ed2ae9939c0f02) )
	ROM_LOAD( "u15",                                            0x100000, 0x100000, NO_DUMP ) // it's also possible it wasn't ever populated

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, CRC(fee6b8e4) SHA1(cb0ddd23e0decda540f22ebb455c91c2aabc60fd) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat_063_tibpal16l8-25cn.u6", 0x000, 0x104, NO_DUMP ) // "PAT 063", protected
ROM_END

// '960606-5 PCB and 'CB1 (CS4)' security counters module. It was found with most sockets unpopulated. This is mechanical.
ROM_START( unk960606 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u2", 0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x100000, "prg_data", 0 )
	ROM_LOAD( "u3", 0x000000, 0x100000, NO_DUMP ) // it's also possible it wasn't ever populated

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "s284_otp_m27c801.u14", 0x000000, 0x100000, CRC(87325ae4) SHA1(6d77f1933f0aab29371795e8fc7bef9bd05cafea) )
	ROM_LOAD( "u15",                  0x100000, 0x100000, NO_DUMP ) // it's also possible it wasn't ever populated

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, CRC(5e2d52ac) SHA1(98bc7a668ee23de4184bdef23fbceda0c1987cd7) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pal16l8.u6", 0x000, 0x104, NO_DUMP )
ROM_END

/* '960606-5 PCB and 'CB1 (CS4)' security counters module. CPLD labeled as 'PD18'.
   It was found with most sockets unpopulated. This is mechanical.
   Two stickers on the PCB:
    - "DOGOR  18/01 00537  C.P.U.  960606.27"
    - "050.594  Europea"
*/
ROM_START( unk960606b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u2", 0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x100000, "prg_data", 0 )
	ROM_LOAD( "u3", 0x000000, 0x100000, NO_DUMP ) // it's also possible it wasn't ever populated

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "s-1.0_otp_char_m27c801.u14", 0x000000, 0x100000, CRC(4b10093e) SHA1(872a4b7f4abcb302163a2ca77149599c1d338c1b) )
	ROM_LOAD( "u15",                        0x100000, 0x100000, NO_DUMP ) // it's also possible it wasn't ever populated

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, CRC(1daca43c) SHA1(28ab94799dcb4bc6889e32311e93af5e9ac2fb90) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat_063_tibpal16l8-25cn.u6", 0x000, 0x104, NO_DUMP ) // "PAT 063", protected
ROM_END

ROM_START( bg_barmy )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "barmyarmy_ndp16", 0x0000, 0x080000, CRC(ae488f48) SHA1(c417a3d1a79a0ca54ade2d9a4f6d70467e6c5cb4) )
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "barmyarmy_sound1", 0x000000, 0x080000, CRC(3530d77c) SHA1(c7a42f698090fcd9644f9929b92935cf85183d23) )
	ROM_LOAD( "barmyarmy_sound2", 0x080000, 0x080000, CRC(48d4c2f3) SHA1(71e64e3e76a55275484a7c72ce2a17232b27a4eb) )
ROM_END

// '960606-5 PCB and 'CB1 (CS4)' security counters module.
ROM_START( bg_dbells )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "sldb.28c4.073nkyly014.u2", 0x0000, 0x080000, CRC(e0c855da) SHA1(1a12dee87705f2b9c4413fbad2a7bff89860ef19) )
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "073snd1.u14", 0x000000, 0x080000, CRC(0ad1e49a) SHA1(e737936d377a46e2c0bf8eb856aa738f9f9c9675) )
	ROM_LOAD( "073snd2.u15", 0x080000, 0x080000, CRC(5feecf8c) SHA1(5e85e3180087faca5f4e6e6cb7deed1a37360ac2) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24c16.u10", 0x000, 0x800, CRC(0cba036f) SHA1(d5bfe96d32e97e820b22a36cd8fee08cfe812a69) )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat.u6", 0x000, 0x104, NO_DUMP )
ROM_END

ROM_START( bg_ddb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "nkyky_0-15_5", 0x0000, 0x080000, CRC(ac4a5094) SHA1(db4eab0be63e5daddca603af290debd8e929757e) )
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	/* There were sound ROMs in the 'CoinWorld Ding Dong Bells' set which might belong here, otherwise
	   ROMs are probably missing */

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat.u6", 0x000, 0x104, NO_DUMP )
ROM_END

ROM_START( bg_max )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "max_a_million_v014", 0x0000, 0x080000, CRC(32fe9c3b) SHA1(77519657e6e478b3cd1bf2ad2aecc6e191abe554) )
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// probably missing

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat.u6", 0x000, 0x104, NO_DUMP )
ROM_END

ROM_START( bg_maxa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "max_a_million_v114", 0x0000, 0x080000, CRC(a66851e9) SHA1(733ec52fa01615e740ebd40fba4a88efe9d9f24f) )
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// probably missing

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "counters", 0 )
	ROM_LOAD( "cirsa_cs-4.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat.u6", 0x000, 0x104, NO_DUMP )
ROM_END

} // Anonymous namespace


// Video games on Cirsa "960606-5" CPU PCB + "IS040302-3" VGA SOC-Legacy PCB (or similar video PCB)
GAME( 2003,  neptunp2,   0,         neptunp2_video,    neptunp2, neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "Neptune's Pearls 2",                                MACHINE_IS_SKELETON ) // Year from legal registry date
GAME( 2008,  perlacrb,   0,         neptunp2_video,    neptunp2, neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "La Perla del Caribe (V1.0, Catalonia)",             MACHINE_IS_SKELETON )

// Screenless games on Cirsa "960606-5" CPU PCB
GAME( 1999,  ccorsario,  0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "Corsarios (Cirsa slot machine, V6.0D)",             MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from sticker on PCB
GAME( 1999,  ccorsarioa, ccorsario, neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "Corsarios (Cirsa slot machine, V5.10D, Catalonia)", MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from sticker on PCB
GAME( 2002?, charles,    0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa/Europea", "Charleston (V2.1, Catalonia)",                      MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from sticker on PCB
GAME( 2008,  gladiador,  0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "Gladiadores (V1.0, CAA)",                           MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from manual
GAME( 2006,  mltpoints,  0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "Multi Points (V1.0, CAA)",                          MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from manual
GAME( 1999,  rockroll,   0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "Rock 'n' Roll",                                     MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from parts' manual and sticker on PCB
GAME( 2001?, unk960606,  0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa",         "unknown 960606-5 based machine (set 1)",            MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from sticker on PCB
GAME( 2001?, unk960606b, 0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa/Europea", "unknown 960606-5 based machine (set 2)",            MACHINE_IS_SKELETON_MECHANICAL ) // Year taken from sticker on PCB

// B. Gaming Technology Ltd. (BGT) fruit machines on Cirsa "960606-5" CPU PCB
GAME( 1997,  bg_barmy,   0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa/B. Gaming Technology", "Barmy Army",                            MACHINE_IS_SKELETON_MECHANICAL )
GAME( 2000,  bg_dbells,  0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa/B. Gaming Technology", "Dancing Bells",                         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1999,  bg_ddb,     0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa/B. Gaming Technology", "Ding Dong Bells (B Gaming Technology)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 2000,  bg_max,     0,         neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa/B. Gaming Technology", "Max A Million (set 1)",                 MACHINE_IS_SKELETON_MECHANICAL )
GAME( 2000,  bg_maxa,    bg_max,    neptunp2_no_video, c960606,  neptunp2_state, empty_init, ROT0, "Unidesa/Cirsa/B. Gaming Technology", "Max A Million (set 2)",                 MACHINE_IS_SKELETON_MECHANICAL )
