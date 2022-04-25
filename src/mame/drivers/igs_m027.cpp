// license:BSD-3-Clause
// copyright-holders:Xing Xing
/* IGS ARM7 (IGS027A) based Mahjong / Gambling platform(s)
 Driver by Xing Xing

 These games use the IGS027A processor.

 This is an ARM7 with Internal ROM. (Also used on later PGM games)

 In some cases the first part of the Internal ROM is execute only, and
 cannot be read out with a trojan.  It hasn't been confirmed if these
 games make use of that feature.

 To emulate these games the Internal ROM will need dumping
 There are at least 20 other games on this and similar platforms.

 Many of these also seem to have a 80C51 based MCU, also covered by
 a holographic sticker, this appears to be unprotected but has only been
 read for a few sets, it probably either acts as a secondary protection
 device or as a main CPU instructing the ARM.

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/nvram.h"
#include "machine/pgmcrypt.h"
#include "video/igs017_igs031.h"
#include "screen.h"


namespace {

class igs_m027_state : public driver_device
{
public:
	igs_m027_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_igs_mainram(*this, "igs_mainram"),
		m_maincpu(*this, "maincpu"),
		m_igs017_igs031(*this, "igs017_igs031")
	{ }

	void igs_mahjong(machine_config &config);
	void amazonia(machine_config &config);

	void init_sdwx();
	void init_chessc2();
	void init_lhzb4();
	void init_mgfx();
	void init_lhzb3();
	void init_gonefsh2();
	void init_sddz();
	void init_hauntedh();
	void init_zhongguo();
	void init_klxyj();
	void init_slqz3();
	void init_fruitpar();
	void init_amazonia();
	void init_amazoni2();
	void init_qlgs();

private:
	optional_shared_ptr<u32> m_igs_mainram;
	required_device<cpu_device> m_maincpu;
	required_device<igs017_igs031_device> m_igs017_igs031;

	virtual void video_start() override;
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void sdwx_gfx_decrypt();
	void pgm_create_dummy_internal_arm_region();
	void igs_mahjong_map(address_map &map);
};



/***************************************************************************

    Video

    0x38001000, 0x380017ff      CG_CONTROL,8 byte per object, 0x100 in total
    0x38001800, 0x380019ff      PALETTE RAM,2 byte per color, 0x100 in total
    0x38004000, 0x38005FFF      TX Video RAM????????1E00??????512x240??????
    0x38006000, 0x38007FFF      BG Video RAM????????1E00??????512x240??????

***************************************************************************/



/* CGLayer */
#if 0
void igs_m027_state::igs_cg_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_igs_cg_videoram[offset]);
	//if(data!=0)
	logerror("PC(%08X) CG @%x = %x!\n",m_maincpu->pc(),offset ,m_igs_cg_videoram[offset]);

	/*
	ROM:08020520                 DCW 0x3E                                           ddd1        y
	ROM:08020522                 DCW 0x29                                           ddd2        x
	ROM:08020524                 DCD 0x190BB6                                   ddd3        n
	ROM:08020528                 DCW 0xC                                            ddd4        Y
	ROM:0802052A                 DCW 0xA6                                           ddd5        X

	(ddd5+?)??10bit
	ddd2??9bit
	(ddd4+?)??11bit
	ddd1??8bit
	ddd3??10bit

	8060a4a6 2642ed8f
	A6A46080 8FED4226

	XXXX-XXXX
	XXxx-xxxx
	xxxY-YYYY
	YYYY-YYyy

	yyyy-yynn
	nnnn-nnnn
	*/
}
#endif



void igs_m027_state::video_start()
{
	m_igs017_igs031->video_start();
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027_state::igs_mahjong_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom(); /* Internal ROM */
	map(0x08000000, 0x0807ffff).rom().region("user1", 0);/* Game ROM */
	map(0x10000000, 0x100003ff).ram().share("igs_mainram");// main ram for asic?
	map(0x18000000, 0x18007fff).ram();

	map(0x38000000, 0x38007fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)); // guess based on below

	map(0x38009000, 0x38009003).ram();     //??????????????  oki 6295

	map(0x70000200, 0x70000203).ram();     //??????????????
	map(0x50000000, 0x500003ff).nopw(); // uploads xor table to external rom here
	map(0xf0000000, 0xF000000f).nopw(); // magic registers
}



/***************************************************************************

    Common functions

***************************************************************************/

/***************************************************************************

    Code Decryption

***************************************************************************/
#if 0
static const u8 sdwx_tab[] =
{
	0x49,0x47,0x53,0x30,0x30,0x35,0x35,0x52,0x44,0x34,0x30,0x32,0x30,0x36,0x32,0x31,
	0x8A,0xBB,0x20,0x67,0x97,0xA5,0x20,0x45,0x6B,0xC0,0xE8,0x0C,0x80,0xFB,0x49,0xAA,
	0x1E,0xAC,0x29,0xF2,0xB9,0x9F,0x01,0x4A,0x8D,0x5F,0x95,0x96,0x78,0xC3,0xF6,0x65,
	0x17,0xBD,0xB6,0x5B,0x25,0x5F,0x6B,0xDE,0x10,0x2E,0x67,0x05,0xDC,0xAC,0xB6,0xBD,
	0x3D,0x20,0x58,0x3D,0xF0,0xA8,0xC0,0xAD,0x5B,0x82,0x8D,0x12,0x65,0x97,0x87,0x7D,
	0x97,0x49,0xDD,0x74,0x74,0x7E,0x9D,0xA1,0x15,0xED,0x75,0xB9,0x09,0xA8,0xA8,0xB0,
	0x6B,0xEA,0x54,0x1B,0x45,0x23,0xE2,0xE5,0x25,0x42,0xCE,0x36,0xFE,0x42,0x99,0xA0,
	0x41,0xF8,0x0B,0x8C,0x3C,0x1B,0xAE,0xE4,0xB2,0x94,0x87,0x02,0xBC,0x08,0x17,0xD9,
	0xE0,0xA4,0x93,0x63,0x6F,0x28,0x5F,0x4A,0x24,0x36,0xD1,0xDA,0xFA,0xDD,0x23,0x26,
	0x4E,0x61,0xB9,0x7A,0x36,0x4D,0x95,0x01,0x20,0xBC,0x18,0xB7,0xAF,0xE4,0xFB,0x92,
	0xD2,0xE3,0x8E,0xEC,0x26,0xCE,0x2F,0x34,0x8F,0xF7,0x0D,0xD6,0x11,0x7F,0x1F,0x68,
	0xF4,0x1D,0x5F,0x16,0x19,0x2D,0x4C,0x4F,0x96,0xFC,0x9F,0xB0,0x99,0x53,0x4C,0x32,
	0x7B,0x41,0xBC,0x90,0x23,0x2E,0x4A,0xFC,0x9E,0x1D,0xFC,0x02,0xFC,0x41,0x83,0xBC,
	0x6D,0xC4,0x75,0x37,0x9D,0xD3,0xC9,0x26,0x4D,0xED,0x93,0xC6,0x32,0x6D,0x02,0x11,
	0x12,0x56,0x97,0x26,0x1D,0x5F,0xA7,0xF8,0x89,0x3F,0x14,0x36,0x72,0x3B,0x48,0x7B,
	0xF1,0xED,0x72,0xB7,0x7A,0x56,0x05,0xDE,0x7B,0x27,0x6D,0xCF,0x33,0x4C,0x14,0x86,
};
#endif



void igs_m027_state::sdwx_gfx_decrypt()
{
	int i;
	unsigned rom_size = 0x80000;
	u8 *src = (u8 *) (memregion("igs017_igs031:tilemaps")->base());
	std::vector<u8> result_data(rom_size);

	for (i=0; i<rom_size; i++)
		result_data[i] = src[bitswap<24>(i, 23,22,21,20,19,18,17,16,15,14,13,12,11,8,7,6,10,9,5,4,3,2,1,0)];

	for (i=0; i<rom_size; i+=0x200)
	{
		memcpy(src+i+0x000,&result_data[i+0x000],0x80);
		memcpy(src+i+0x080,&result_data[i+0x100],0x80);
		memcpy(src+i+0x100,&result_data[i+0x080],0x80);
		memcpy(src+i+0x180,&result_data[i+0x180],0x80);
	}
}

/***************************************************************************

    Protection & I/O

***************************************************************************/







/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( sdwx )
INPUT_PORTS_END

static INPUT_PORTS_START( amazonia )
	PORT_START("DSW1")
// Credits proportion
	PORT_DIPNAME( 0x03, 0x03, "Proporcao Credito" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x03, "10" )
// (Oponent's ?) credits proportion
	PORT_DIPNAME( 0x0c, 0x0c, "Proporcao Credito Ele" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x0c, "10" )
// Game Percentage
	PORT_DIPNAME( 0x70, 0x70, "Porcentagem Jogo" ) PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x40, "60%" )
	PORT_DIPSETTING(    0x20, "65%" )
	PORT_DIPSETTING(    0x60, "70%" )
	PORT_DIPSETTING(    0x10, "75%" )
	PORT_DIPSETTING(    0x50, "80%" )
	PORT_DIPSETTING(    0x30, "85%" )
	PORT_DIPSETTING(    0x70, "90%" )
// Payment System
	PORT_DIPNAME( 0x80, 0x80, "Sistema de Pagamento" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Normal" )
	PORT_DIPSETTING(    0x80, "Auto" )


	PORT_START("DSW2")
// Demo Song
	PORT_DIPNAME( 0x01, 0x01, "Demonstracao Musica" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
// End of Game
	PORT_DIPNAME( 0x02, 0x02, "Fim do Sistema" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, "10000" )
// Background color
	PORT_DIPNAME( 0x04, 0x04, "Cor do Fundo" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "Preto" ) // Black
	PORT_DIPSETTING(    0x04, "Cor" ) // Coloured
// Double Percentage
	PORT_DIPNAME( 0x18, 0x18, "Porcentagem Dobrar" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPSETTING(    0x08, "90%" )
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x18, "90%" )
// Language
	PORT_DIPNAME( 0x20, 0x20, "Idioma" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, "Espanhol" ) // Spanish
	PORT_DIPSETTING(    0x20, "Portugues" ) // Portuguese
// Credit Mode
	PORT_DIPNAME( 0x40, 0x40, "Credit Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "COIN" )
	PORT_DIPSETTING(    0x40, "KEYIN" )
// Panel Mode
	PORT_DIPNAME( 0x80, 0x80, "Panel Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "36+10" )
	PORT_DIPSETTING(    0x80, "28" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/




WRITE_LINE_MEMBER(igs_m027_state::vblank_irq)
{
	if (state)
		m_maincpu->pulse_input_line(ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
}


void igs_m027_state::igs_mahjong(machine_config &config)
{
	ARM7(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027_state::igs_mahjong_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);


	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update("igs017_igs031", FUNC(igs017_igs031_device::screen_update));
	screen.set_palette("igs017_igs031:palette");
	screen.screen_vblank().set(FUNC(igs_m027_state::vblank_irq));

	IGS017_IGS031(config, m_igs017_igs031, 0);
	m_igs017_igs031->set_text_reverse_bits();
	//m_igs017_igs031->set_i8255_tag("ppi8255");

	// 82C55? (accessed through igs017/igs031 area like igs017.cpp?)

	/* sound hardware */
	// OK6295
}


void igs_m027_state::amazonia(machine_config &config)
{
	ARM7(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027_state::igs_mahjong_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update("igs017_igs031", FUNC(igs017_igs031_device::screen_update));
	screen.set_palette("igs017_igs031:palette");
	screen.screen_vblank().set(FUNC(igs_m027_state::vblank_irq));

	IGS017_IGS031(config, m_igs017_igs031, 0);
	m_igs017_igs031->set_text_reverse_bits();
	//m_igs017_igs031->set_i8255_tag("ppi8255");

	// 82C55? (accessed through igs017/igs031 area like igs017.cpp?)

	/* sound hardware */
	// OK6295
}

/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

Mahjong Shuang Long Qiang Zhu 3
IGS, 1999

PCB Layout

IGS PCB-0239-11-EE
|--------------------------------------------|
|  DSW2 DSW1       U9             U18        |
|      22MHz           IGS031                |
|                                PAL         |
|                                            |
|                                            |
|                                            |
|          62256                  IGS027A    |
|                                 55857G     |
|                U29                         |
|    8255                                    |
|                                            |
|                  62256                     |
|                                            |
|ULN2004                                     |
|       M6295                                |
|                                            |
|                                    RESET_SW|
|TDA1519C        U26                BATTERY  |
|--------------------------------------------|

***************************************************************************/

ROM_START( slqz3 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A type G ARM based MCU */
	ROM_LOAD( "slqz3_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "u29", 0x000000, 0x200000, CRC(215fed1e) SHA1(c85d8695e0be1044ac206118c3fc0ddc7063aaf6) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "u9",  0x000000, 0x080000, CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "u18", 0x000000, 0x400000, CRC(81428f18) SHA1(9fb19c8a79cc3443642f4b044e04735df2cb45be) ) // FIXED BITS (xxxxxxxx0xxxxxxx)


	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "u26", 0x000000, 0x200000, CRC(84bc2f3e) SHA1(49dcf5eaa39accd5c6bf01782fd4221298cb43ed) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END




/***************************************************************************

Fruit Paradise
IGS

PCB Layout
----------

IGS PCB-0331-02-FG
|--------------------------------------------|
|PC817                   7805       W4102.U28|
|ULN2004 ULN2004   TDA1020  VOL        M6295 |
|ULN2004       PAL    62257          3.6VBATT|
|ULN2004  82C55   22MHz                      |
|ULN2004                                     |
|8                 V214.U23                  |
|L                                |--------| |
|I  PC817(x20)                    |IGS027A | |
|N                   |--------|   |55857G  | |
|E      M4101.U13    |        |   |--------| |
|R                   | IGS031 |              |
|                    |        |              |
|       TEXT.U12     |--------|              |
|DSW1                                        |
|DSW2 ULN2004                       61256    |
|DSW3              PC817(x13)   PC817 PC817  |
|       |--|         JAMMA             |--|  |
|-------|  |---------------------------|  |--|

***************************************************************************/

ROM_START( fruitpar )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A type G ARM based MCU */
	ROM_LOAD( "fruitpar_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "fruit_paradise_v214.u23", 0x00000, 0x80000, CRC(e37bc4e0) SHA1(f5580e6007dc60f32efd3b3e7e64c5ee446ede8a) )

	ROM_REGION( 0x080000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "paradise_text.u12", 0x000000, 0x080000, CRC(bdaa4407) SHA1(845eead0902c81290c2b5d7543ac9dfda375fdd1) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_m4101.u13",     0x000000, 0x400000, CRC(84899398) SHA1(badac65af6e03c490798f4368eb2b15db8c590d0) ) // FIXED BITS (xxxxxxx0xxxxxxxx)


	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_w4102.u28", 0x00000, 0x80000, CRC(558cab25) SHA1(0280b37a14589329f0385c048e5742b9e89bd587) )
ROM_END



/***************************************************************************

Amazonia King
IGS

IGS PCB-0367-00-FG-1

  - IGS 027A
  - IGS 031
  - IGS A2107
  - IGS T2105: Character Generator ROM
  - K668 (qfp44) == OKI6225
  - 82C55

***************************************************************************/

ROM_START( amazonia )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A type G ARM based MCU */
	ROM_LOAD( "amazonia_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "amazonia_v-104br.u23", 0x00000, 0x80000, CRC(103d465e) SHA1(68d088f24171e27c0a9b0660f81d3334f730637a) )

	ROM_REGION( 0x480000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "igs_t2105_cg_v110.u12", 0x000000, 0x80000, CRC(1d4be260) SHA1(6374c61735144b3ff54d5e490f26adac4a10b14d) )

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_a2107_cg_v110.u13", 0x000000, 0x400000,CRC(d8dadfd7) SHA1(b40a46d56ff46d91e3377be8616c3eed321f7db4) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "amazonia_cg.u11", 0x400000, 0x80000, CRC(2ac2cfd1) SHA1(f8750a4727ddabf1415dab6eaa4a72e60e86e7f1) )       // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2102.u28", 0x00000, 0x80000, CRC(90dda82d) SHA1(67fbc1e8d76b85e124136e2f1df09c8b6c5a8f97) )
ROM_END

ROM_START( amazonkp )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A type G ARM based MCU */
	ROM_LOAD( "amazonia_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ak_plus_v-204br.u23", 0x00000, 0x80000, CRC(e71f6272) SHA1(1717cc4dad9858f1a54988b7459631de8bac8ebd) )

	ROM_REGION( 0x480000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "igs_t2105.u12", 0x000000, 0x80000, CRC(1d4be260) SHA1(6374c61735144b3ff54d5e490f26adac4a10b14d) )

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_a2107.u13",      0x000000, 0x400000,CRC(d8dadfd7) SHA1(b40a46d56ff46d91e3377be8616c3eed321f7db4) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "ak_plus_ext_cg.u11", 0x400000, 0x80000, CRC(26796bc0) SHA1(bd259fbd05834de3d90af87235f13b467a492fed) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2102.u37", 0x00000, 0x80000, CRC(90dda82d) SHA1(67fbc1e8d76b85e124136e2f1df09c8b6c5a8f97) ) // this came dumped with 4 identical quarters, 1 quarter matches the ROM from the amazonia set
ROM_END

/*
Amazonia King II by IGS 2004 ( International Game System )
Patented by EAGO.

U12 is a 27c240  labeled ( AKII TEXT ) ( text )
U13 is a 27c160  labeled ( AKII CG ) ( Grafics)
U23 is a 27c4096 labeled ( AKII_V-202br ) ( Program version Brazil )
U28 is a 29F4000 labeled (AKII SP) ( Sound Program )
U17 is a ATF16V8B-15P labeled ( FG-1 ) (read protected)
U10 is a IGS 003c Dip 40 pin ( Maybe 8255 ? )
U24 is a IGS031 QFP with 208 pin
U32 is a IGS027a QFP with 120 pin ( Encrypted ARM, internal code, stamped P9 A/K II )
Crystal Frequency = 22.000 Mhz
Sound Processor ( U6295 )
*/

ROM_START( amazoni2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "sdwx_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "27c4096_akii_b-202br.u23", 0x000000, 0x80000, CRC(7147b43c) SHA1(29a4a20867595650918c4ab892ddb71440bd3f4b) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "akii_text.u12", 0x000000, 0x80000, CRC(60b415ac) SHA1(b4475b0ba1e70504cac9ac05078873df0b16495b) )

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "u13_27c160_akii_cg.u13", 0x000000, 0x200000, CRC(254bd84f) SHA1(091ecda792c4c4a7bb039b2c708788ef87fdaf86) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )  // m6295 samples
	ROM_LOAD( "akii_sp.u28", 0x00000, 0x80000, CRC(216b5418) SHA1(b7bc24ced0ccb5476c974420aa506c13b971fc9f) )
ROM_END


// Games with prg at u16
// text at u24
// cg at u25
// samples at u2

ROM_START( sdwx )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "sdwx_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "prg.u16", 0x000000, 0x80000, CRC(c94ef6a8) SHA1(69f2f356e05206b0866a9020253d9a112b56316c) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u24", 0x000000, 0x80000, CRC(60b415ac) SHA1(b4475b0ba1e70504cac9ac05078873df0b16495b) )

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg.u25", 0x000000, 0x200000, CRC(709b9a42) SHA1(18c4b8e159b29c168f5cafb437fe6eb123672471) )

	ROM_REGION( 0x80000, "oki", 0 ) // m6295 samples
	ROM_LOAD( "sp.u2", 0x00000, 0x80000, CRC(216b5418) SHA1(b7bc24ced0ccb5476c974420aa506c13b971fc9f) )
ROM_END

ROM_START( klxyj )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "klxyj_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "klxyj_104.u16", 0x000000, 0x80000, CRC(8cb9bdc2) SHA1(5a13d0ff6488a938617a9ea89e7cf607539a1f49) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "klxyj_text.u24", 0x000000, 0x80000, CRC(22dcebd0) SHA1(0383f017135230d020d12c8c6cc3aeb136fe9106) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "a4202.u25", 0x000000, 0x400000, CRC(97a68f85) SHA1(177c8c23fd0d585b24a71359ede005ac9a2e4d4d) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "w4201.u2", 0x00000, 0x100000, CRC(464f11ab) SHA1(56e45bd31f667fc30387fcd4c940a94819b7ef0f) )
ROM_END


// Games with prg at u9
// text at u17
// cg at u18
// samples at u14


ROM_START( lhzb3 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "lhzb3_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb3_104.u9", 0x000000, 0x80000, CRC(70d61846) SHA1(662b59702ef6f26129de6b16346786df92f99097) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "lhzb3_text.u17", 0x000000, 0x80000,CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2401.u18", 0x000000, 0x400000,  CRC(81428f18) SHA1(9fb19c8a79cc3443642f4b044e04735df2cb45be) )

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "s2402.u14", 0x00000, 0x100000, CRC(56083fe2) SHA1(62afd651809bf5e639bfda6e5579dbf4b903b664) )
ROM_END





/*

Zhong Guo Chu Da D
IGS, 2000

PCB Layout
----------

IGS PCB NO-0267
|------------------------------------------|
|M2601.U17  PAL |-------|           RESET  |
|    M2603.U18  |       |                  |
|               |IGS027A|                  |
|               |       |         BATT_3.6V|
|               |-------|                  |
|                          W24257          |
|J                                S2602.U14|
|A         |-------|                       |
|M T2604.U9|       |                       |
|M         |IGS031 |  P2600.U10            |
|A         |       |                 M6295 |
|          |-------|                       |
|                                          |
|       22MHz     W24257                   |
|                                          |
|                        8255         VOL  |
|    DSW1(8)                               |
|        DSW2(8)             LM7805        |
|                                 UPC1242H |
|------------------------------------------|
Notes:
      W24257     - Winbond 32kx8 SRAM (SOJ28)
      Custom ICs -
                  IGS027A - ARM7/9? based CPU (QFP120, labelled 'J8')
                  IGS033  - likey GFX processor. Appears to be linked to the 3.6V battery. However,
                  the battery was dead and the PCB still works, so maybe the battery is not used? (QFP208)
      ROMs -
            P2600.U10 - 27C4096 EPROM, Main program
            M2601.U17 - 32MBit DIP42 MaskROM, read as 27C322, GFX (stamped 'IMAGE')
            M2603.U18 - 4MBit DIP40 EPROM, read as 27C4096, GFX (stamped 'IMAGE')
            S2602.U14 - 8MBit DIP32 MaskROM, read as MX27C8000, Oki M6295 sound data (stamped 'SPEECH')
            T2604.U9  - 4MBit DIP40 MaskROM, read as 27C4096, GFX (stamped 'TEXT')

*/

ROM_START( zhongguo )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "zhongguo_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "p2600.u10", 0x000000, 0x80000, CRC(9ad34135) SHA1(54717753d1296efe49946369fd4a27181f19dbc0) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "t2604.u9", 0x000000, 0x80000, CRC(5401a52d) SHA1(05b47a4b39939c1d5904e3fbd5cc56d6ee9b7953) )

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2601.u17", 0x000000, 0x400000, CRC(89736e3f) SHA1(6a22e2eb10d2c740cf21640c43a8caf4c72d3be7) )
	ROM_LOAD( "m2603.u18", 0x400000, 0x080000, CRC(fb2e91a8) SHA1(29b2f0ce3749539cbe4cfb5c40b240cc7f6147f1) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "s2602.u14", 0x00000, 0x100000, CRC(f137028c) SHA1(0e4114222820bca2f7026fa653e2b96a489a0183) )
ROM_END



ROM_START( mgfx )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "mgfx_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "mgfx_101.u10", 0x000000, 0x80000, CRC(897c88a1) SHA1(0f7a7808b9503ff28ad32c0b8e071cb24cff59b1) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "mgfx_text.u9", 0x000000, 0x80000, CRC(e41e7768) SHA1(3d0add7c75c23533309e799fd8853c815e6f811c) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "mgfx_ani.u17", 0x000000, 0x400000, CRC(9fc75f4d) SHA1(acb600739dcf252a5210e28ec96d749573061b27) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "mgfx_sp.u14", 0x00000, 0x100000, CRC(9bb28fc8) SHA1(6368753c29607f2d212d68c5cca3f10aa069649b) )
ROM_END


/*


Gone Fishing II
IGS PCB-0388-05-FW

   +--------------------------------------------+
+--+               8-Liner Connecter            +---+
|                                                   |
|    +---------------+                            +-+
|    |   IGS 0027A   |     +------+               |
+-+  |    Plug-in    |     | IGS  |               +-+
  |  |   Daughter    |     | 025  |                P|
+-+  |     Card      |     +------+                r|
|    +---------------+ +---+                       i|
|J                     |   |                       n|
|A   +---+ +---+ +---+ |   |                       t|
|M   |   | |   | |   | | U |                      +-+
|M   |   | |   | |   | | 1 |                +---+ |
|A   | U | | U | | U | | 2 |                |   | +-+
|    | 1 | | 1 | | 1 | |   |         +----+ | U |   |
|C   | 5 | | 7 | | 4 | |   |         |Oki | | 1 |   |
|o   |   | |   | | * | |   |         |6295| | 3 |   |
|n   |   | |   | |   | +---+         +----+ |   |   |
|n   |   | |   | |   |                      +---+   |
|e   +---+ |   | +---+                              |
|c         +---+                                    |
|t                        62257                     |
|e                                                  |
|r       +-------+                                  |
|        |       |                                  |
|        |  IGS  |                                  |
|        |  031  |     61256                        |
+-+      |       |          PAL     V3021           |
  |      +-------+                                  |
+-+                                      X1    SW4  |
|                                                   |
| JP11                        SW3 SW2 SW1      BT1  |
|                                                   |
+---------------------------------------------------+


U12 - Program rom   - 27C4096
U15 - Text graphics - 27C4096
U17 - Char graphics - 27C160
U23 - Sound samples - 27C040

SW1-SW3 are unpopulated
U14* Not used (27C4096) or else it's U16 and 27C160 type EPROM

   X1 - 32.768kHZ OSC
V3021 - Micro Electronic Ultra Low Power 1-Bit 32kHz RTC (Real Time Clock)
  PAL - ATF22V10C at U26 labeled FW U26
  BT1 - 3.6V battery
  SW4 - Toggle switch
 JP11 - 4 Pin header (HD4-156)

IGS 025  - Custom programmed A8B1723(?)
IGS 0027 - Custom programmed ARM9

*/



ROM_START( gonefsh2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "gonefsh2_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "gfii_v-904uso.u12", 0x000000, 0x80000, CRC(ef0f6735) SHA1(0add92599b0989f3e50dc64e32ce234b4bd87d33) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "gfii_text.u15", 0x000000, 0x80000, CRC(b48118fd) SHA1(e718d23ce5f7f41ab94df2d05cdd3adbf27eef89) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "gfii_cg.u17", 0x000000, 0x200000, CRC(2568359c) SHA1(f1f240246e53496bf624c84f7cae3edb9675579f) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "gfii_sp.u13", 0x00000, 0x080000, CRC(61da1d58) SHA1(0a79578f0daf15f0efe2b0eeac59a60d8372a644) )
ROM_END

/*


Chess Challenge II

IGS PCB-0388-04-FW

   +--------------------------------------------+
+--+               8-Liner Connecter            +---+
|                                                   |
|    +---------------+                            +-+
|    |   IGS 0027A   |     +------+               |
+-+  |    Plug-in    |     | IGS  |               +-+
  |  |   Daughter    |     | 025  |                P|
+-+  |     Card      |     +------+                r|
|    +---------------+ +---+                       i|
|J                     |   |                       n|
|A   +---+ +---+ +---+ |   |                       t|
|M   |   | |   | |   | | U |                      +-+
|M   |   | |   | |   | | 1 |                +---+ |
|A   | U | | U | | U | | 2 |                |   | +-+
|    | 1 | | 1 | | 1 | |   |         +----+ | U |   |
|C   | 5 | | 7 | | 4 | |   |         |Oki | | 1 |   |
|o   |   | |   | | * | |   |         |6295| | 3 |   |
|n   |   | |   | |   | +---+         +----+ |   |   |
|n   |   | |   | |   |                      +---+   |
|e   +---+ |   | +---+                              |
|c         +---+                                    |
|t                        62257                     |
|e                                                  |
|r       +-------+                                  |
|        |       |                                  |
|        |  IGS  |                                  |
|        |  031  |     61256                        |
+-+      |       |          PAL     V3021           |
  |      +-------+                                  |
+-+                                      X1    SW4  |
|                                                   |
| JP11                        SW3 SW2 SW1      BT1  |
|                                                   |
+---------------------------------------------------+


U12 - Program rom   - 27C4096
U15 - Text graphics - 27C4096
U17 - Char graphics - 27C160
U23 - Sound samples - 27C040

SW1-SW3 are unpopulated
U14* Not used (27C4096) or else it's U16 and 27C160 type EPROM

   X1 - 32.768K OSC
V3021 - Micro Electronic Ultra Low Power 1-Bit 32kHz RTC (Real Time Clock)
  PAL - ATF22V10C at U26 labeled FW U26
  BT1 - 3.6V battery
  SW4 - Toggle switch
 JP11 - 4 Pin header (HD4-156)

IGS 025  - Custom programmed A8B1723(?)
IGS 0027 - Custom programmed ARM9

*/

ROM_START( chessc2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "chessc2_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ccii_v-707uso.u12", 0x000000, 0x80000, CRC(5937b67b) SHA1(967b3adf6f5bf92d63ec460d595e473898a78372) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "ccii_text.u15", 0x000000, 0x80000, CRC(25fed033) SHA1(b321c4994f609906597c3f7d5cdfc2dca63cd340) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "ccii_cg.u17", 0x000000, 0x200000, CRC(47e45157) SHA1(4459799a4a6c30a2d0a3ad9ac54e92b62221e10b) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "ccii_sp.u13", 0x00000, 0x080000,  CRC(220a7b71) SHA1(7dab7baa97c20b83763cf46ef0a6e5e8c4d6a348) )
ROM_END



// prg at u34
// text at u15
// cg at u32 / u12
// samples at u3

ROM_START( haunthig )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "haunthig_igs027a", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'H2'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "hauntedhouse_ver-101us.u34", 0x000000, 0x80000, CRC(4bf045d4) SHA1(78c848fd69961df8d9b75f92ad57c3534fbf08db) )

	ROM_REGION( 0x10000, "plcc", 0 )
	ROM_LOAD( "hauntedhouse.u17", 0x000000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked J9, not read protected?

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "haunted-h_text.u15", 0x000000, 0x80000, CRC(c23f48c8) SHA1(0cb1b6c61611a081ae4a3c0be51812045ff632fe) )

	// are these PGM-like sprites?
	ROM_REGION( 0x800000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "haunted-h_cg.u32",  0x000000, 0x400000, CRC(e0ea10e6) SHA1(e81be78fea93e72d4b1f4c0b58560bda46cf7948) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "haunted-h_ext.u12", 0x400000, 0x400000, CRC(662eb883) SHA1(831ebe29e1e7a8b2c2fff7fbc608975771c3486c) ) // FIXED BITS (xxxxxxxx0xxxxxxx)


	ROM_REGION( 0x200000, "samples", 0 ) // samples, but not OKI? possibly ICS?
	ROM_LOAD( "haunted-h_sp.u3", 0x00000, 0x200000,  CRC(fe3fcddf) SHA1(ac57ab6d4e4883747c093bd19d0025cf6588cb2c) )
ROM_END


// Games with prg at u17
// text at u27
// cg at u28
// samples at u4 (or u5?)

ROM_START( sddz )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "sddz_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ddz_218cn.u17", 0x000000, 0x80000, CRC(3cfe38d5) SHA1(9c7f82ecffbc22879583519d5f753bb35e973ee3) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "ddz_text.u27", 0x000000, 0x80000, CRC(520dc392) SHA1(0ab2620f20af8253806b6ff4e1d9d77a694da17c) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "ddz_ani.u28", 0x000000, 0x400000, CRC(72487508) SHA1(9f4bbc858960ddaae403e4a3330b2345f6fd6cb3))

	ROM_REGION( 0x200000, "samples", 0 ) // samples, but not OKI? possibly ICS?
	ROM_LOAD( "ddz_sp.u4", 0x00000, 0x200000, CRC(7ef65d95) SHA1(345c587cd449d6d06908e9687480be76b2cb2d28) )
ROM_END

ROM_START( lhzb4 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "lhzb4_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb4_104.u17", 0x000000, 0x80000, CRC(6f349bbb) SHA1(54cf895889ef0f208637ba732ede696ca3603ee0) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "lhzb4_text.u27", 0x000000, 0x80000, CRC(8488b039) SHA1(59bc9eccba810fcac2a53866b2da1e71bfd8a6e7) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "a05501.u28", 0x000000, 0x400000, CRC(f78b3714) SHA1(c73d8e50b04126bc4f91783384713624ed133ee2) )

	ROM_REGION( 0x200000, "samples", 0 ) // samples, but not OKI? possibly ICS?
	ROM_LOAD( "w05502.u5", 0x00000, 0x200000, CRC(467f677e) SHA1(63927c0d606176c0e22db89ea3a9777ed702abbd) )
ROM_END

// Que Long Gao Shou (Master on Mahjong Dragon) (IGS, 1999) - PCB-0489-17-FM-1 (IGS027A, M6295, IGS031, 8255, Battery)
ROM_START( qlgs )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal rom of IGS027A type G ARM based MCU
	ROM_LOAD( "qlgs_igs027a", 0x00000, 0x4000, NO_DUMP ) // has a 'DJ-2 U17' and a 'C3' sticker

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s-501cn.u17", 0x000000, 0x200000, CRC(c80b61c0) SHA1(4e9920beb85fd559620f3136ea52ab6532657b1f) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text_u26.u26", 0x000000, 0x200000, CRC(4cd44ba8) SHA1(49f73233d466f196ee62bfca0c3e1042f38ee340) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg_u28.u28", 0x000000, 0x400000, CRC(b34e22a0) SHA1(60c7efb9a0112745c5f9934a04578f6ce5071976) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "sp_u5.u5", 0x00000, 0x200000, CRC(6049b892) SHA1(f87285a288bd3fd169080045f70ff15181661582) ) // 11xxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

ROM_START( extradrw ) // IGS PCB 0326-05-DV-1
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "extradrw_igs027a", 0x00000, 0x4000, NO_DUMP ) // has a 'E1' sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg?
	ROM_LOAD( "u21", 0x00000, 0x80000, CRC(c1641b14) SHA1(bd2525a5b38d4d8a39e99e43ef62e1d2fd3c044d) ) // 1ST AND 2ND HALF IDENTICAL, label not readable

	ROM_REGION( 0x280000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "u12",           0x000000, 0x200000, CRC(642247fb) SHA1(69c01c3551551120a3786522b28a80621a0d5082) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF, label not readable
	ROM_LOAD( "igs m3001.u4",  0x000000, 0x080000, CRC(d161f8f7) SHA1(4b495197895fd805979c5d5c5a4b7f07a68f4171) ) // label barely readable

	ROM_REGION( 0x100000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "u3", 0x000000, 0x80000, CRC(97227767) SHA1(c6a1916c0df1aceafbd488ecace5794390058c49) ) // FIXED BITS (xxxxxxx0xxxxxxxx), label not readable

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "igs s3002.u18", 0x00000, 0x200000, CRC(48601c32) SHA1(8ef3bad80931f4b1badf0598463e15508602f104) ) // BADADDR   --xxxxxxxxxxxxxxxxxxx
ROM_END

void igs_m027_state::pgm_create_dummy_internal_arm_region()
{
	u16 *temp16 = (u16 *)memregion("maincpu")->base();

	// fill with RX 14
	int i;
	for (i=0;i<0x4000/2;i+=2)
	{
		temp16[i] = 0xff1e;
		temp16[i+1] = 0xe12f;

	}

	// jump straight to external area
	temp16[(0x0000)/2] = 0xd088;
	temp16[(0x0002)/2] = 0xe59f;
	temp16[(0x0004)/2] = 0x0680;
	temp16[(0x0006)/2] = 0xe3a0;
	temp16[(0x0008)/2] = 0xff10;
	temp16[(0x000a)/2] = 0xe12f;
	temp16[(0x0090)/2] = 0x0400;
	temp16[(0x0092)/2] = 0x1000;
}


/*
void igs_m027_state::init_igs_m027()
{
    pgm_create_dummy_internal_arm_region(machine());
}
*/

void igs_m027_state::init_sdwx()
{
	sdwx_decrypt(machine());
	sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_klxyj()
{
	klxyj_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_chessc2()
{
	chessc2_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_hauntedh()
{
	hauntedh_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}



void igs_m027_state::init_lhzb4()
{
	lhzb4_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_mgfx()
{
	mgfx_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_lhzb3()
{
	lhzb3_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_sddz()
{
	sddz_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_gonefsh2()
{
	gonefsh2_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_zhongguo()
{
	zhongguo_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_slqz3()
{
	slqz3_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_fruitpar()
{
	fruitpar_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_amazonia()
{
	amazonia_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_amazoni2()
{
	amazoni2_decrypt(machine());
	//sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_qlgs()
{
	//qlgs_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}


} // Anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1999, slqz3,     0,        igs_mahjong, sdwx, igs_m027_state, init_slqz3,    ROT0, "IGS", "Mahjong Shuang Long Qiang Zhu 3 (China, VS107C)", MACHINE_IS_SKELETON )
GAME( 1999, qlgs,      0,        igs_mahjong, sdwx, igs_m027_state, init_qlgs,     ROT0, "IGS", "Que Long Gao Shou", MACHINE_IS_SKELETON )
GAME( 1999, amazonia,  0,        amazonia,amazonia, igs_m027_state, init_amazonia, ROT0, "IGS", "Amazonia King (V104BR)", MACHINE_IS_SKELETON )
GAME( 1999, amazonkp,  amazonia, amazonia,amazonia, igs_m027_state, init_amazonia, ROT0, "IGS", "Amazonia King Plus (V204BR)", MACHINE_IS_SKELETON )
GAME( 200?, fruitpar,  0,        igs_mahjong, sdwx, igs_m027_state, init_fruitpar, ROT0, "IGS", "Fruit Paradise (V214)", MACHINE_IS_SKELETON )
GAME( 2002, sdwx,      0,        igs_mahjong, sdwx, igs_m027_state, init_sdwx,     ROT0, "IGS", "Sheng Dan Wu Xian", MACHINE_IS_SKELETON ) // aka Christmas 5 Line? (or Amazonia King II, shares roms at least?)
GAME( 2002, amazoni2,  0,        igs_mahjong, sdwx, igs_m027_state, init_amazoni2, ROT0, "IGS", "Amazonia King II (V202BR)", MACHINE_IS_SKELETON )
GAME( 200?, sddz,      0,        igs_mahjong, sdwx, igs_m027_state, init_sddz,     ROT0, "IGS", "Super Dou Di Zhu",  MACHINE_IS_SKELETON )
GAME( 2000, zhongguo,  0,        igs_mahjong, sdwx, igs_m027_state, init_zhongguo, ROT0, "IGS", "Zhong Guo Chu Da D",  MACHINE_IS_SKELETON )
GAME( 200?, lhzb3,     0,        igs_mahjong, sdwx, igs_m027_state, init_lhzb3,    ROT0, "IGS", "Long Hu Zhengba III", MACHINE_IS_SKELETON ) // 龙虎争霸Ⅲ
GAME( 200?, lhzb4,     0,        igs_mahjong, sdwx, igs_m027_state, init_lhzb4,    ROT0, "IGS", "Long Hu Zhengba 4", MACHINE_IS_SKELETON ) // 龙虎争霸4
GAME( 200?, klxyj,     0,        igs_mahjong, sdwx, igs_m027_state, init_klxyj,    ROT0, "IGS", "Kuai Le Xi You Ji",  MACHINE_IS_SKELETON )
GAME( 2000, mgfx,      0,        igs_mahjong, sdwx, igs_m027_state, init_mgfx,     ROT0, "IGS", "Man Guan Fu Xing",   MACHINE_IS_SKELETON )
GAME( 200?, gonefsh2,  0,        igs_mahjong, sdwx, igs_m027_state, init_gonefsh2, ROT0, "IGS", "Gone Fishing 2",   MACHINE_IS_SKELETON )
GAME( 2002, chessc2,   0,        igs_mahjong, sdwx, igs_m027_state, init_chessc2,  ROT0, "IGS", "Chess Challenge II",   MACHINE_IS_SKELETON )
GAME( 200?, haunthig,  0,        igs_mahjong, sdwx, igs_m027_state, init_hauntedh, ROT0, "IGS", "Haunted House (IGS)",   MACHINE_IS_SKELETON )
GAME( 200?, extradrw,  0,        igs_mahjong, sdwx, igs_m027_state, init_qlgs,     ROT0, "IGS", "Extra Draw",   MACHINE_IS_SKELETON )
