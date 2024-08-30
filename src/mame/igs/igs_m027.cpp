// license:BSD-3-Clause
// copyright-holders: Xing Xing, David Haywood

/* IGS ARM7 (IGS027A) based Mahjong / Gambling platform(s)
 Driver by Xing Xing

 These games use the IGS027A processor.

 This is an ARM7 with Internal ROM. (Also used on later PGM games)

 In some cases the first part of the Internal ROM is execute only, and
 cannot be read out with a trojan.  It hasn't been confirmed if these
 games make use of that feature.

 To emulate these games the Internal ROM will need dumping
 There are at least 20 other games on this and similar platforms.

*/

#include "emu.h"

#include "igs017_igs031.h"
#include "pgmcrypt.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "sound/okim6295.h"

#include "screen.h"
#include "speaker.h"

namespace {

class igs_m027_state : public driver_device
{
public:
	igs_m027_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_igs_mainram(*this, "igs_mainram"),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi8255"),
		m_igs017_igs031(*this, "igs017_igs031"),
		m_screen(*this, "screen"),
		m_oki(*this, "oki"),
		m_dsw(*this, "DSW%u", 1U)
	{ }

	void igs_mahjong(machine_config &config);

	void init_sdwx();
	void init_chessc2();
	void init_lhzb4();
	void init_lhzb3();
	void init_gonefsh2();
	void init_sddz();
	void init_zhongguo();
	void init_klxyj();
	void init_slqz3();
	void init_fruitpar();
	void init_oceanpar();
	void init_amazonia();
	void init_amazoni2();
	void init_qlgs();
	void init_mgzz();
	void init_mgcs3();
	void init_jking02();
	void init_lhdmg();
	void init_lhdmgp();
	void init_lthy();
	void init_luckycrs();
	void init_olympic5();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	optional_shared_ptr<u32> m_igs_mainram;
	required_device<cpu_device> m_maincpu;
	optional_device<i8255_device> m_ppi;
	required_device<igs017_igs031_device> m_igs017_igs031;
	required_device<screen_device> m_screen;
	required_device<okim6295_device> m_oki;
	required_ioport_array<3> m_dsw;

	u32 unk_r();
	u32 unk2_r();
	u32 lhdmg_unk2_r();
	void unk2_w(u32 data);

	void dsw_io_select_w(u32 data);

	u8 ppi_porta_r();

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void pgm_create_dummy_internal_arm_region();
	void igs_mahjong_map(address_map &map);

	u32 m_dsw_io_select;
	u32 m_unk2_write_count;
};

void igs_m027_state::video_start()
{
	m_igs017_igs031->video_start();
}

void igs_m027_state::machine_start()
{
	m_dsw_io_select = 7;
	m_unk2_write_count = 0;

	save_item(NAME(m_dsw_io_select));
	save_item(NAME(m_unk2_write_count));
}

/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027_state::igs_mahjong_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom(); // Internal ROM
	map(0x08000000, 0x0807ffff).rom().region("user1", 0); // Game ROM
	map(0x10000000, 0x100003ff).ram().share("igs_mainram"); // main RAM for ASIC?
	map(0x18000000, 0x18007fff).ram();

	map(0x38000000, 0x38007fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));

	map(0x38008000, 0x38008003).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x000000ff);

	map(0x38009000, 0x38009003).r(FUNC(igs_m027_state::unk_r));

	map(0x40000008, 0x4000000b).w(FUNC(igs_m027_state::unk2_w));
	map(0x4000000c, 0x4000000f).r(FUNC(igs_m027_state::unk2_r));
	map(0x40000018, 0x4000001b).w(FUNC(igs_m027_state::dsw_io_select_w));

	map(0x70000200, 0x70000203).ram(); // ??????????????
	map(0x50000000, 0x500003ff).nopw(); // uploads XOR table to external ROM here
	map(0xf0000000, 0xf000000f).nopw(); // magic registers
}

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( base )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("PORTB") // buttons?
	PORT_DIPNAME( 0x01, 0x01, "PORTB")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PORTC") // buttons?
	PORT_DIPNAME( 0x01, 0x01, "PORTC")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jking02 )
	PORT_INCLUDE(base)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2" ) // can cause a coin error (sets different inputs?)
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x00, "Show Odds" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Show Title" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x20, "Background Type" )
	PORT_DIPSETTING(    0x20, "Jungle Style" )
	PORT_DIPSETTING(    0x10, "Casino Style" )
	PORT_DIPSETTING(    0x00, "Casino Style (duplicate 1)" )
	PORT_DIPSETTING(    0x30, "Casino Style (duplicate 2)" )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // shows dipswitches
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) // maybe service coin?

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // maybe bet?

	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) // maybe start?
INPUT_PORTS_END

static INPUT_PORTS_START( qlgs )
	PORT_INCLUDE(base)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x00, "Link Mode" )
	PORT_DIPSETTING(    0x04, "Linked" )
	PORT_DIPSETTING(    0x00, "Standalone" )
INPUT_PORTS_END

static INPUT_PORTS_START( amazonia )
	PORT_INCLUDE(base)

	PORT_MODIFY("DSW1")
// Credits proportion
	PORT_DIPNAME( 0x03, 0x03, "Proporcao Credito" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x03, "10" )
// (Opponent's ?) credits proportion
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


	PORT_MODIFY("DSW2")
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
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(igs_m027_state::interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->pulse_input_line(ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time()); // source?

	if (scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->pulse_input_line(ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time()); // vbl?
}

u8 igs_m027_state::ppi_porta_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 3; i++)
		if (!BIT(m_dsw_io_select, i))
			data &= m_dsw[i]->read();

	return data;
}

void igs_m027_state::dsw_io_select_w(u32 data)
{
	m_dsw_io_select = data;
}


// IO? maybe serial?

void igs_m027_state::unk2_w(u32 data)
{
	logerror("%s: unk2_w %08x\n", machine().describe_context(), data);
	m_unk2_write_count++;
}

u32 igs_m027_state::unk2_r()
{
	logerror("%s: unk2_r\n", machine().describe_context());

	// slqz3 boot check
	if (m_unk2_write_count & 1)
		return 0xffffffff;
	else
		return 0xffffffef;
}

u32 igs_m027_state::lhdmg_unk2_r()
{
	logerror("%s: lhdmg_unk2_r\n", machine().describe_context());

	if (m_dsw_io_select & 1)
		return 0xffffffff;
	else
		return 0xffffffff ^ 0x400000;
}

u32 igs_m027_state::unk_r()
{
	// this is accessed as a byte, lower 2 bytes are read?
	// slqz3 reads test switch in here? writes to the address look like key matrix?
	logerror("%s: unk_r\n", machine().describe_context());
	return 0xffffffff;
}


void igs_m027_state::igs_mahjong(machine_config &config)
{
	ARM7(config, m_maincpu, 22000000); // Jungle King 2002 has a 22Mhz Xtal, what about the others?
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027_state::igs_mahjong_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update("igs017_igs031", FUNC(igs017_igs031_device::screen_update));
	m_screen->set_palette("igs017_igs031:palette");

	TIMER(config, "scantimer").configure_scanline(FUNC(igs_m027_state::interrupt), "screen", 0, 1);

	I8255A(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(igs_m027_state::ppi_porta_r));
	m_ppi->in_pb_callback().set_ioport("PORTB");
	m_ppi->in_pc_callback().set_ioport("PORTC");


	IGS017_IGS031(config, m_igs017_igs031, 0);
	m_igs017_igs031->set_text_reverse_bits();
	m_igs017_igs031->set_i8255_tag("ppi8255");

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
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
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "s11_027a.bin", 0x00000, 0x4000, CRC(abb8ef8b) SHA1(b8912fe38dc2ff3b1a718e9fe3c76eae30aad7dc) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "u29", 0x000000, 0x200000, CRC(215fed1e) SHA1(c85d8695e0be1044ac206118c3fc0ddc7063aaf6) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "u9",  0x000000, 0x080000, CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

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
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "q5_027a.bin", 0x00000, 0x4000, CRC(df756ac3) SHA1(5b5d2a7f6363260166e3411d1571056cc30a5e56) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "fruit_paradise_v214.u23", 0x00000, 0x80000, CRC(e37bc4e0) SHA1(f5580e6007dc60f32efd3b3e7e64c5ee446ede8a) )

	ROM_REGION( 0x080000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "paradise_text.u12", 0x000000, 0x080000, CRC(bdaa4407) SHA1(845eead0902c81290c2b5d7543ac9dfda375fdd1) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_m4101.u13",     0x000000, 0x400000, CRC(84899398) SHA1(badac65af6e03c490798f4368eb2b15db8c590d0) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_w4102.u28", 0x00000, 0x80000, CRC(558cab25) SHA1(0280b37a14589329f0385c048e5742b9e89bd587) )
ROM_END

ROM_START( oceanpar ) // IGS PCB-0331-02-FG
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "oceanpar_igs027a", 0x00000, 0x4000, NO_DUMP ) // possibly B1

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ocean_paradise_v105us.u23", 0x00000, 0x80000, CRC(e6eb66c3) SHA1(f6c1e31ccddc8ebb8218f52b5c0d97f0797b2e84) )

	ROM_REGION( 0x080000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "ocean_paradise_text.u12", 0x000000, 0x080000, CRC(bdaa4407) SHA1(845eead0902c81290c2b5d7543ac9dfda375fdd1) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_m4101.u13", 0x000000, 0x400000, CRC(84899398) SHA1(badac65af6e03c490798f4368eb2b15db8c590d0) ) // FIXED BITS (xxxxxxx0xxxxxxxx), same as fruitpar

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_w4102.u28", 0x00000, 0x80000, CRC(558cab25) SHA1(0280b37a14589329f0385c048e5742b9e89bd587) ) // same as fruitpar
ROM_END

ROM_START( oceanpara ) // IGS PCB-0331-01-FG
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "b1_igs027a", 0x00000, 0x4000, NO_DUMP ) // stickered B1

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ocean_paradise_v101us.u23", 0x00000, 0x80000, CRC(4f2bf87a) SHA1(559c8728632336ba84f455ac22b6e514967c644b) )

	ROM_REGION( 0x080000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "ocean_paradise_text.u12", 0x000000, 0x080000, CRC(bdaa4407) SHA1(845eead0902c81290c2b5d7543ac9dfda375fdd1) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_m4101.u13", 0x000000, 0x400000, CRC(84899398) SHA1(badac65af6e03c490798f4368eb2b15db8c590d0) ) // FIXED BITS (xxxxxxx0xxxxxxxx), same as fruitpar

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_w4102.u28", 0x00000, 0x80000, CRC(558cab25) SHA1(0280b37a14589329f0385c048e5742b9e89bd587) ) // same as fruitpar
ROM_END

// supposedly a reskin of fruitpar / oceanpar, runs on a slightly different PCB (type not readable, seems same as amazonkp)
ROM_START( luckycrs )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "v21_igs027a", 0x00000, 0x4000, NO_DUMP ) // stickered V21

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "luckycross_v-106sa.u23", 0x00000, 0x80000, CRC(5716de00) SHA1(ff68fa93c6801c78f910452c08c5a9c1a089261d) ) // 27C4002

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "luckycross_text_u12.u12", 0x00000, 0x80000, CRC(c03aa300) SHA1(612ab40d507da7614ec288dd2e95aaca4e497e1b) ) // 27C4002

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_k4101_u13.u13",         0x000000, 0x400000, CRC(84899398) SHA1(badac65af6e03c490798f4368eb2b15db8c590d0) ) // 27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "luckycross_ext_cg_u11.u11", 0x400000, 0x080000, CRC(997cb3cb) SHA1(6ae955abe7888135f9543ddf73ff84c23baf15f1) ) // 27C4002, FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "igs_w4102_u37.u37", 0x000000, 0x200000, CRC(114b44ee) SHA1(728ffee34c64edcfef3a46e8be97db60da8a90dc) ) // 27C160, 11xxxxxxxxxxxxxxxxxxx = 0xFF
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
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "amazonia_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "amazonia_v-104br.u23", 0x00000, 0x80000, CRC(103d465e) SHA1(68d088f24171e27c0a9b0660f81d3334f730637a) )

	ROM_REGION( 0x480000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "igs_t2105_cg_v110.u12", 0x000000, 0x80000, CRC(1d4be260) SHA1(6374c61735144b3ff54d5e490f26adac4a10b14d) )

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_a2107_cg_v110.u13", 0x000000, 0x400000, CRC(d8dadfd7) SHA1(b40a46d56ff46d91e3377be8616c3eed321f7db4) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "amazonia_cg.u11",       0x400000, 0x080000, CRC(2ac2cfd1) SHA1(f8750a4727ddabf1415dab6eaa4a72e60e86e7f1) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2102.u28", 0x00000, 0x80000, CRC(90dda82d) SHA1(67fbc1e8d76b85e124136e2f1df09c8b6c5a8f97) )
ROM_END

ROM_START( amazonkp )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "amazonia_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ak_plus_v-204br.u23", 0x00000, 0x80000, CRC(e71f6272) SHA1(1717cc4dad9858f1a54988b7459631de8bac8ebd) )

	ROM_REGION( 0x480000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "igs_t2105.u12", 0x000000, 0x80000, CRC(1d4be260) SHA1(6374c61735144b3ff54d5e490f26adac4a10b14d) )

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_a2107.u13",      0x000000, 0x400000, CRC(d8dadfd7) SHA1(b40a46d56ff46d91e3377be8616c3eed321f7db4) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "ak_plus_ext_cg.u11", 0x400000, 0x080000, CRC(26796bc0) SHA1(bd259fbd05834de3d90af87235f13b467a492fed) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

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
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "p9_igs027a", 0x00000, 0x4000, NO_DUMP ) // stamped P9

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "27c4096_akii_b-202br.u23", 0x000000, 0x80000, CRC(7147b43c) SHA1(29a4a20867595650918c4ab892ddb71440bd3f4b) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "akii_text.u12", 0x000000, 0x80000, CRC(60b415ac) SHA1(b4475b0ba1e70504cac9ac05078873df0b16495b) )

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "u13_27c160_akii_cg.u13", 0x000000, 0x200000, CRC(254bd84f) SHA1(091ecda792c4c4a7bb039b2c708788ef87fdaf86) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )  // m6295 samples
	ROM_LOAD( "akii_sp.u28", 0x00000, 0x80000, CRC(216b5418) SHA1(b7bc24ced0ccb5476c974420aa506c13b971fc9f) )
ROM_END

ROM_START( jking02 ) // PCB-0367-05-FG-1
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "j6_027a.bin", 0x0000, 0x4000, CRC(69e241f0) SHA1(1ae0aabb217c67ee6e7126f3f0f90c8b3e051888) ) // J6 holographic sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "j_k_2002_v-209us.u23", 0x00000, 0x80000, CRC(ef6b652b) SHA1(ee5c2cef2c7cbcd4a70e05c01295e964ca5e45d1) ) // 27C4096

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "jungle_king_02_u12_text.u12", 0x00000, 0x80000, CRC(22dcebd0) SHA1(0383f017135230d020d12c8c6cc3aeb136fe9106) ) // M27C4002

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "jungle_king_02_u13_a4202.u13", 0x000000, 0x400000, CRC(97a68f85) SHA1(177c8c23fd0d585b24a71359ede005ac9a2e4d4d) ) // 27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "jungle_king_02_u11_cg.u11",    0x400000, 0x080000, CRC(3c43da58) SHA1(7fbc34905587a36b6514ac781a16f12345129184) ) // M27C4002, FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "igs_w4201_speech_v103.u28", 0x000000, 0x200000, CRC(fb72d4b5) SHA1(c4f434fb20ac3df8d08aaf62f1dfad03f6f619ef) ) // M27C160, 1xxxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

ROM_START( olympic5 ) // PCB type not readable, layout almost identical to PCB-0367-05-FG-1
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "o2_igs027a", 0x00000, 0x4000, NO_DUMP ) // stickered O2

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "olympic_5_v-112us.u23", 0x00000, 0x80000, CRC(27743107) SHA1(ddb8fc3645b0d8f8b7348c180951ca212b3a2c03) ) // MX27C4096

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "olympic_5_text.u12", 0x00000, 0x80000, CRC(60b415ac) SHA1(b4475b0ba1e70504cac9ac05078873df0b16495b) ) // MX27C4096

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "olympic_5_cg.u13", 0x000000, 0x200000, CRC(6803f95b) SHA1(2d3d4194bb4efaf24c42c47c027068f396b08e7e) ) // M27C160, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u11 not populated

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "olympic_5_sp.u28", 0x000000, 0x200000, CRC(7a2b5441) SHA1(a100daa3534c06c0fd40d9bab25983efe9dd446d) ) // 27C160, contains 4 times the same data as the one in olympic5a
ROM_END

ROM_START( olympic5a )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "o2_igs027a", 0x00000, 0x4000, NO_DUMP ) // stickered O2?

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "olympic_5_v-107us.u23", 0x00000, 0x80000, CRC(3bcd4dd9) SHA1(08e49d9a5045e52a7eb60113f0c7ed25b30474c2) ) // MX27C4096

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "olympic_5_text.u12", 0x00000, 0x80000, CRC(60b415ac) SHA1(b4475b0ba1e70504cac9ac05078873df0b16495b) ) // MX27C4096

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "olympic_5_cg.u13", 0x000000, 0x200000, CRC(6803f95b) SHA1(2d3d4194bb4efaf24c42c47c027068f396b08e7e) ) // M27C160, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u11 not populated

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "olympic_5_sp.u28", 0x00000, 0x80000, CRC(216b5418) SHA1(b7bc24ced0ccb5476c974420aa506c13b971fc9f) ) // MX27C4000
ROM_END

// Games with prg at u16
// text at u24
// cg at u25
// samples at u2

ROM_START( sdwx )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "sdwx_igs027a", 0x00000, 0x4000, NO_DUMP ) // unknown sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "prg.u16", 0x000000, 0x80000, CRC(c94ef6a8) SHA1(69f2f356e05206b0866a9020253d9a112b56316c) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "text.u24", 0x000000, 0x80000, CRC(60b415ac) SHA1(b4475b0ba1e70504cac9ac05078873df0b16495b) )

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg.u25", 0x000000, 0x200000, CRC(709b9a42) SHA1(18c4b8e159b29c168f5cafb437fe6eb123672471) )

	ROM_REGION( 0x80000, "oki", 0 ) // m6295 samples
	ROM_LOAD( "sp.u2", 0x00000, 0x80000, CRC(216b5418) SHA1(b7bc24ced0ccb5476c974420aa506c13b971fc9f) )
ROM_END

ROM_START( klxyj )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "klxyj_igs027a", 0x00000, 0x4000, NO_DUMP ) // unknown sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "klxyj_104.u16", 0x000000, 0x80000, CRC(8cb9bdc2) SHA1(5a13d0ff6488a938617a9ea89e7cf607539a1f49) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "klxyj_text.u24", 0x000000, 0x80000, CRC(22dcebd0) SHA1(0383f017135230d020d12c8c6cc3aeb136fe9106) )

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
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "lhzb3_igs027a", 0x00000, 0x4000, NO_DUMP ) // unknown sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb3_104.u9", 0x000000, 0x80000, CRC(70d61846) SHA1(662b59702ef6f26129de6b16346786df92f99097) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "lhzb3_text.u17", 0x000000, 0x80000,CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2401.u18", 0x000000, 0x400000,  CRC(81428f18) SHA1(9fb19c8a79cc3443642f4b044e04735df2cb45be) )

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "s2402.u14", 0x00000, 0x100000, CRC(56083fe2) SHA1(62afd651809bf5e639bfda6e5579dbf4b903b664) )
ROM_END


/*********************************************************************************

Long Hu Da Man Guan, IGS 1999
Long Hu Da Man Guan Plus, IGS 1999

Both boards are identical and use the same mask ROMs, only with changed program EPROM.

PCB Layout
----------

IGS PCB NO-0240-03
|-----------------------------------------|
|              SW1  SW2          M2403.U17|
|1                                        |
|8  82C55                        M2401.U18|
|W                                        |
|A      22MHz                 UPC29L33    |
|Y                                PAL     |
|                   IGS031                |
|                                         |
|          61256                  IGS027A |
|                                         |
|                  27C4096.U9             |
|                                         |
|1                     HM62256            |
|0 T518B                                  |
|W                                        |
|A  7805                                  |
|Y                                     SW3|
|UPC1242  VOL   K668   S2402.U14   BATTERY|
|-----------------------------------------|
Notes:
       K668 - Oki M6295 clone. Clock 1.000MHz [22/22]
      SW1/2 - 8-Position DIP Switch
        SW3 - Reset / NVRAM Clear
      61256 - EliteMT LP61256 32kBx8-bit SRAM (SOJ28)
    HM62256 - Hitachi HM62256 32kBx8-bit SRAM (SOP28)
      T518B - Reset IC
    IGS027A - ARM7 CPU with internal ROM.
              lhdmg - Sticker: B6
              lhdmgplus - Sticker: B4

*********************************************************************************/

ROM_START( lhdmg ) // appears to be a different edition of lhzb3 and lthy (GFX and sound ROM match)
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "b6_igs027a", 0x00000, 0x4000, CRC(75645f8c) SHA1(738fba64a906f4f10e78e332ad30b8da9dc86b21) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhdmg_prg.u9", 0x000000, 0x80000, CRC(3b3a77ac) SHA1(c1c40e02d04dc701aa65b7e255b9a928cbecdb8d) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "m2403.u17", 0x000000, 0x80000, CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2401.u18", 0x000000, 0x400000, CRC(81428f18) SHA1(9fb19c8a79cc3443642f4b044e04735df2cb45be) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s2402.u14", 0x000000, 0x100000, CRC(56083fe2) SHA1(62afd651809bf5e639bfda6e5579dbf4b903b664) )
ROM_END

ROM_START( lhdmgp ) // appears to be a different edition of lhzb3 and lthy (GFX and sound ROM match)
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "b4_igs027a", 0x00000, 0x4000, CRC(6fd48959) SHA1(75cb6fc6ea3c36805d1a61536e2f2476942c0c49) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhdmg_plus_prg.u9", 0x000000, 0x80000, CRC(77dd7855) SHA1(f04995ee34ef9245dcf3d66fcf111fa377394f92) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "m2403.u17", 0x000000, 0x80000, CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2401.u18", 0x000000, 0x400000, CRC(81428f18) SHA1(9fb19c8a79cc3443642f4b044e04735df2cb45be) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s2402.u14", 0x000000, 0x100000, CRC(56083fe2) SHA1(62afd651809bf5e639bfda6e5579dbf4b903b664) )
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
                  IGS031  - likely GFX processor. Appears to be linked to the 3.6V battery. However,
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
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "j8_igs027a", 0x00000, 0x4000, CRC(95c51462) SHA1(818aad8ac25355950ba00438d0eecf58f0a17d8a) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "p2600.u10", 0x000000, 0x80000, CRC(9ad34135) SHA1(54717753d1296efe49946369fd4a27181f19dbc0) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "t2604.u9", 0x000000, 0x80000, CRC(5401a52d) SHA1(05b47a4b39939c1d5904e3fbd5cc56d6ee9b7953) )

	ROM_REGION( 0x480000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2601.u17", 0x000000, 0x400000, CRC(89736e3f) SHA1(6a22e2eb10d2c740cf21640c43a8caf4c72d3be7) )
	ROM_LOAD( "m2603.u18", 0x400000, 0x080000, CRC(fb2e91a8) SHA1(29b2f0ce3749539cbe4cfb5c40b240cc7f6147f1) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "s2602.u14", 0x00000, 0x100000, CRC(f137028c) SHA1(0e4114222820bca2f7026fa653e2b96a489a0183) )
ROM_END



ROM_START( mgzz )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "f1_027a.bin", 0x00000, 0x4000, CRC(4b270edb) SHA1(3b4821f7cb785056809c121e6508348df123bfa1) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "mgfx_101.u10", 0x000000, 0x80000, CRC(897c88a1) SHA1(0f7a7808b9503ff28ad32c0b8e071cb24cff59b1) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "mgfx_text.u9", 0x000000, 0x80000, CRC(e41e7768) SHA1(3d0add7c75c23533309e799fd8853c815e6f811c) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "mgfx_ani.u17", 0x000000, 0x400000, CRC(9fc75f4d) SHA1(acb600739dcf252a5210e28ec96d749573061b27) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "mgfx_sp.u14", 0x00000, 0x100000, CRC(9bb28fc8) SHA1(6368753c29607f2d212d68c5cca3f10aa069649b) )
ROM_END


ROM_START( mgzza ) // IGS PCB 0295-00 (IGS027A, M6295, IGS031, 8255, Battery)
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "f1_027a.bin", 0x00000, 0x4000, CRC(4b270edb) SHA1(3b4821f7cb785056809c121e6508348df123bfa1) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-100cn.u10", 0x000000, 0x80000, CRC(278964f7) SHA1(75e48e3124d038f16f93fe3c1f63dd1568f0c018) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "text.u9", 0x000000, 0x80000, CRC(10792638) SHA1(ae5d93659140252da332a3dd03cb188ddf79ad5a) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg.u17", 0x000000, 0x400000, CRC(1643fa78) SHA1(4c08b62d3dd7171a9ad3182634f66befa82cb581) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sp.u14", 0x00000, 0x80000, CRC(f037952e) SHA1(0fa83e164937c9e8245861da7fd11f225525918d) )
ROM_END

/*********************************************************************************

Long Teng Hu Yue, IGS 1999

PCB Layout
----------

IGS PCB-0227-03
|-----------------------------------------|
|                JAMMA                    |
|1                              M2401.U17 |
|8      22MHz   27C4096.U9                |
|W  SW1                                   |
|A  SW2                                   |
|Y                              LM2933    |
|                IGS031         PAL       |
|       61256                             |
|                                IGS027A  |
|     82C55                      (LABEL D6)
|1              27C4096.U10               |
|0                       35256            |
|W                                        |
|A  7805                                  |
|Y   TDA1020      T518B         BATTERY   |
|      VOL     K668    S2402.U14       SW3|
|-----------------------------------------|
Notes:
       K668 - Oki M6295 clone. Clock 1.000MHz [22/22]. Pin 7 HIGH.
      SW1/2 - 8-Position DIP Switch
        SW3 - Reset / NVRAM Clear
      61256 - EliteMT LP61256 32kBx8-bit SRAM (SOJ28)
      35256 - Sanyo LC35256 32kBx8-bit SRAM (SOP28)
      T518B - Reset IC

*********************************************************************************/

ROM_START( lthy ) // appears to be a different edition of lhzb3 (GFX and sound ROM match)
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "d6_igs027a", 0x00000, 0x4000, CRC(b29ee32b) SHA1(aa5f1f8ed8d61dd328c4c28a7bba700935526d26) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "27c4096.u10", 0x000000, 0x80000, CRC(bd04f2e9) SHA1(3d5a2101c7214a37f159e0d17f3e66a9b6ab94ff) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "27c4096.u9", 0x000000, 0x80000, CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2401.u17", 0x000000, 0x400000, CRC(81428f18) SHA1(9fb19c8a79cc3443642f4b044e04735df2cb45be) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s2402.u14", 0x000000, 0x100000, CRC(56083fe2) SHA1(62afd651809bf5e639bfda6e5579dbf4b903b664) )
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
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "gonefsh2_igs027a", 0x00000, 0x4000, NO_DUMP ) // unknown sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "gfii_v-904uso.u12", 0x000000, 0x80000, CRC(ef0f6735) SHA1(0add92599b0989f3e50dc64e32ce234b4bd87d33) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "gfii_text.u15", 0x000000, 0x80000, CRC(b48118fd) SHA1(e718d23ce5f7f41ab94df2d05cdd3adbf27eef89) )

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

IGS 025  - Custom programmed A8B1723(?) - stickered S1
IGS 0027 - Custom programmed ARM9

*/

ROM_START( chessc2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "c8_igs027a", 0x00000, 0x4000, NO_DUMP ) // stickered C8

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ccii_v-707uso.u12", 0x000000, 0x80000, CRC(5937b67b) SHA1(967b3adf6f5bf92d63ec460d595e473898a78372) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "ccii_text.u15", 0x000000, 0x80000, CRC(25fed033) SHA1(b321c4994f609906597c3f7d5cdfc2dca63cd340) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "ccii_cg.u17", 0x000000, 0x200000, CRC(47e45157) SHA1(4459799a4a6c30a2d0a3ad9ac54e92b62221e10b) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "ccii_sp.u13", 0x00000, 0x080000,  CRC(220a7b71) SHA1(7dab7baa97c20b83763cf46ef0a6e5e8c4d6a348) )
ROM_END


// Games with prg at u17
// text at u27
// cg at u28
// samples at u4 (or u5?)

ROM_START( sddz )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "sddz_igs027a", 0x00000, 0x4000, NO_DUMP ) // unknown sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ddz_218cn.u17", 0x000000, 0x80000, CRC(3cfe38d5) SHA1(9c7f82ecffbc22879583519d5f753bb35e973ee3) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "ddz_text.u27", 0x000000, 0x80000, CRC(520dc392) SHA1(0ab2620f20af8253806b6ff4e1d9d77a694da17c) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "ddz_ani.u28", 0x000000, 0x400000, CRC(72487508) SHA1(9f4bbc858960ddaae403e4a3330b2345f6fd6cb3))

	ROM_REGION( 0x200000, "samples", 0 ) // samples, but not OKI? possibly ICS?
	ROM_LOAD( "ddz_sp.u4", 0x00000, 0x200000, CRC(7ef65d95) SHA1(345c587cd449d6d06908e9687480be76b2cb2d28) )
ROM_END

ROM_START( lhzb4 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "lhzb4_igs027a", 0x00000, 0x4000, NO_DUMP ) // unknown sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb4_104.u17", 0x000000, 0x80000, CRC(6f349bbb) SHA1(54cf895889ef0f208637ba732ede696ca3603ee0) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "lhzb4_text.u27", 0x000000, 0x80000, CRC(8488b039) SHA1(59bc9eccba810fcac2a53866b2da1e71bfd8a6e7) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "a05501.u28", 0x000000, 0x400000, CRC(f78b3714) SHA1(c73d8e50b04126bc4f91783384713624ed133ee2) )

	ROM_REGION( 0x200000, "samples", 0 ) // samples, but not OKI? possibly ICS?
	ROM_LOAD( "w05502.u5", 0x00000, 0x200000, CRC(467f677e) SHA1(63927c0d606176c0e22db89ea3a9777ed702abbd) )
ROM_END

// Que Long Gao Shou (Master on Mahjong Dragon) (IGS, 1999) - PCB-0489-17-FM-1 (IGS027A, M6295, IGS031, 8255, Battery)
ROM_START( qlgs )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "c3_igs027a.bin", 0x00000, 0x4000, CRC(7b107594) SHA1(274d9927b396b610f99f4a3c760f9f4d9c21d29c) ) // has a 'DJ-2 U17' and a 'C3' sticker

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s-501cn.u17", 0x000000, 0x200000, CRC(c80b61c0) SHA1(4e9920beb85fd559620f3136ea52ab6532657b1f) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "text_u26.u26", 0x000000, 0x200000, CRC(4cd44ba8) SHA1(49f73233d466f196ee62bfca0c3e1042f38ee340) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg_u28.u28", 0x000000, 0x400000, CRC(b34e22a0) SHA1(60c7efb9a0112745c5f9934a04578f6ce5071976) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "sp_u5.u5", 0x00000, 0x200000, CRC(6049b892) SHA1(f87285a288bd3fd169080045f70ff15181661582) ) // 11xxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

// PCB-0489-15-FM-1 (IGS027A, M6295, IGS031, 8255, Battery) - same PCB as qlgs
ROM_START( mgcs3 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "m9_027a.bin", 0x00000, 0x4000, CRC(f40b3202) SHA1(6e2ba210afa886be9b43ed7027023d2724aaa538) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "m2401_v101cn.u17", 0x000000, 0x200000, CRC(d0d78fb6) SHA1(10a16f1ce0a89b5281822be26a2cdcc86702b188) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "text.u26", 0x000000, 0x200000, CRC(14881df6) SHA1(261e813498981a30a938a9f6205c8c8f79a57029) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg1.u28", 0x000000, 0x400000, CRC(59522cf8) SHA1(b33448d72769e78b097edd2e8055c9144491120d) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "sp.u5", 0x00000, 0x200000, CRC(eb27b166) SHA1(eb9641516245d9094861d6ba6e902eac62019968) )
ROM_END

ROM_START( extradrw ) // IGS PCB 0326-05-DV-1
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "e1_igs027a", 0x00000, 0x4000, NO_DUMP ) // has a 'E1' sticker

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg?
	ROM_LOAD( "u21", 0x00000, 0x80000, BAD_DUMP CRC(c1641b14) SHA1(bd2525a5b38d4d8a39e99e43ef62e1d2fd3c044d) ) // 1ST AND 2ND HALF IDENTICAL, label not readable, suspected bad dump (doesn't decrypt with usual methods)

	ROM_REGION( 0x280000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "u12",           0x000000, 0x200000, CRC(642247fb) SHA1(69c01c3551551120a3786522b28a80621a0d5082) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF, label not readable
	ROM_LOAD16_WORD_SWAP( "igs m3001.u4",  0x000000, 0x080000, CRC(d161f8f7) SHA1(4b495197895fd805979c5d5c5a4b7f07a68f4171) ) // label barely readable

	ROM_REGION( 0x100000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "u3", 0x000000, 0x80000, CRC(97227767) SHA1(c6a1916c0df1aceafbd488ecace5794390058c49) ) // FIXED BITS (xxxxxxx0xxxxxxxx), label not readable

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "igs s3002.u18", 0x00000, 0x200000, CRC(48601c32) SHA1(8ef3bad80931f4b1badf0598463e15508602f104) ) // BADADDR   --xxxxxxxxxxxxxxxxxxx
ROM_END


void igs_m027_state::pgm_create_dummy_internal_arm_region()
{
	u16 *temp16 = (u16 *)memregion("maincpu")->base();

	// fill with RX 14
	for (int i = 0; i < 0x4000 / 2; i += 2)
	{
		temp16[i] = 0xff1e;
		temp16[ i +1] = 0xe12f;

	}

	// jump straight to external area
	temp16[(0x0000) / 2] = 0xd088;
	temp16[(0x0002) / 2] = 0xe59f;
	temp16[(0x0004) / 2] = 0x0680;
	temp16[(0x0006) / 2] = 0xe3a0;
	temp16[(0x0008) / 2] = 0xff10;
	temp16[(0x000a) / 2] = 0xe12f;
	temp16[(0x0090) / 2] = 0x0400;
	temp16[(0x0092) / 2] = 0x1000;
}

void igs_m027_state::init_sdwx()
{
	sdwx_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_klxyj()
{
	klxyj_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_chessc2()
{
	chessc2_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_lhzb4()
{
	lhzb4_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_lhzb3()
{
	lhzb3_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_sddz()
{
	sddz_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_gonefsh2()
{
	gonefsh2_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_zhongguo()
{
	zhongguo_decrypt(machine());
	m_igs017_igs031->mgcs_decrypt_tiles(); // close, but wrong, see copyright date
	// sprites aren't encrypted
}

void igs_m027_state::init_slqz3()
{
	slqz3_decrypt(machine());
	//m_igs017_igs031->slqz3_decrypt_tiles(); // none of the existing functions are correct for this
	// sprite gfx not encrypted
}

void igs_m027_state::init_fruitpar()
{
	fruitpar_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}

void igs_m027_state::init_oceanpar()
{
	oceanpar_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_amazonia()
{
	amazonia_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_amazoni2()
{
	amazoni2_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt();
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_qlgs()
{
	qlgs_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}

void igs_m027_state::init_mgzz()
{
	mgzz_decrypt(machine());
}

void igs_m027_state::init_mgcs3()
{
	mgcs3_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt(); // wrong?
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}

void igs_m027_state::init_jking02()
{
	jking02_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	m_igs017_igs031->tarzan_decrypt_sprites(0x400000, 0x400000);
	// the sprite ROM at 0x400000 doesn't require decryption
}

void igs_m027_state::init_lthy()
{
	lthy_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt(); // wrong
	//pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_luckycrs()
{
	luckycrs_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_olympic5()
{
	olympic5_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027_state::init_lhdmg()
{
	lhdmg_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	//pgm_create_dummy_internal_arm_region();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000000c, 0x4000000f, read32smo_delegate(*this, FUNC(igs_m027_state::lhdmg_unk2_r)));
}

void igs_m027_state::init_lhdmgp()
{
	lhdmgp_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	//pgm_create_dummy_internal_arm_region();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000000c, 0x4000000f, read32smo_delegate(*this, FUNC(igs_m027_state::lhdmg_unk2_r)));
}


} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

// Complete dumps
GAME( 1999, slqz3,     0,        igs_mahjong, base,     igs_m027_state, init_slqz3,    ROT0, "IGS", "Mahjong Shuang Long Qiang Zhu 3 (China, VS107C)", MACHINE_IS_SKELETON )
GAME( 1999, qlgs,      0,        igs_mahjong, qlgs,     igs_m027_state, init_qlgs,     ROT0, "IGS", "Que Long Gao Shou", MACHINE_IS_SKELETON )
GAME( 1999, fruitpar,  0,        igs_mahjong, base,     igs_m027_state, init_fruitpar, ROT0, "IGS", "Fruit Paradise (V214)", MACHINE_IS_SKELETON )
GAME( 1999, lhdmg,     0,        igs_mahjong, base,     igs_m027_state, init_lhdmg,    ROT0, "IGS", "Long Hu Da Man Guan", MACHINE_IS_SKELETON )
GAME( 1999, lhdmgp,    lhdmg,    igs_mahjong, base,     igs_m027_state, init_lhdmgp,   ROT0, "IGS", "Long Hu Da Man Guan Plus", MACHINE_IS_SKELETON )
GAME( 1999, lthy,      0,        igs_mahjong, base,     igs_m027_state, init_lthy,     ROT0, "IGS", "Long Teng Hu Yue", MACHINE_IS_SKELETON )
GAME( 2000, zhongguo,  0,        igs_mahjong, base,     igs_m027_state, init_zhongguo, ROT0, "IGS", "Zhong Guo Chu Da D", MACHINE_IS_SKELETON )
GAME( 200?, jking02,   0,        igs_mahjong, jking02,  igs_m027_state, init_jking02,  ROT0, "IGS", "Jungle King 2002 (V209US)", MACHINE_IS_SKELETON )
GAME( 2003, mgzz,      0,        igs_mahjong, base,     igs_m027_state, init_mgzz,     ROT0, "IGS", "Man Guan Zhi Zun (V101CN)", MACHINE_IS_SKELETON )
GAME( 2000, mgzza,     0,        igs_mahjong, base,     igs_m027_state, init_mgzz,     ROT0, "IGS", "Man Guan Zhi Zun (V100CN)", MACHINE_IS_SKELETON )
GAME( 2007, mgcs3,     0,        igs_mahjong, base,     igs_m027_state, init_mgcs3,    ROT0, "IGS", "Man Guan Caishen 3 (V101CN)", MACHINE_IS_SKELETON )

// Incomplete dumps
GAME( 1999, amazonia,  0,        igs_mahjong, amazonia, igs_m027_state, init_amazonia, ROT0, "IGS", "Amazonia King (V104BR)", MACHINE_IS_SKELETON )
GAME( 1999, amazonkp,  amazonia, igs_mahjong, amazonia, igs_m027_state, init_amazonia, ROT0, "IGS", "Amazonia King Plus (V204BR)", MACHINE_IS_SKELETON )
GAME( 1999, oceanpar,  0,        igs_mahjong, base,     igs_m027_state, init_oceanpar, ROT0, "IGS", "Ocean Paradise (V105US)", MACHINE_IS_SKELETON ) // 1999 copyright in ROM
GAME( 1999, oceanpara, oceanpar, igs_mahjong, base,     igs_m027_state, init_oceanpar, ROT0, "IGS", "Ocean Paradise (V101US)", MACHINE_IS_SKELETON ) // 1999 copyright in ROM
GAME( 200?, luckycrs,  0,        igs_mahjong, base,     igs_m027_state, init_luckycrs, ROT0, "IGS", "Lucky Cross (V106SA)", MACHINE_IS_SKELETON )
GAME( 2005, olympic5,  0,        igs_mahjong, base,     igs_m027_state, init_olympic5, ROT0, "IGS", "Olympic 5 (V112US)", MACHINE_IS_SKELETON ) // IGS FOR V112US 2005 02 14
GAME( 2003, olympic5a, olympic5, igs_mahjong, base,     igs_m027_state, init_olympic5, ROT0, "IGS", "Olympic 5 (V107US)", MACHINE_IS_SKELETON ) // IGS FOR V107US 2003 10 2
GAME( 2003, amazoni2,  0,        igs_mahjong, base,     igs_m027_state, init_amazoni2, ROT0, "IGS", "Amazonia King II (V202BR)", MACHINE_IS_SKELETON )
GAME( 2002, sdwx,      0,        igs_mahjong, base,     igs_m027_state, init_sdwx,     ROT0, "IGS", "Sheng Dan Wu Xian", MACHINE_IS_SKELETON ) // aka Christmas 5 Line? (or Amazonia King II, shares roms at least?)
GAME( 200?, sddz,      0,        igs_mahjong, base,     igs_m027_state, init_sddz,     ROT0, "IGS", "Super Dou Di Zhu", MACHINE_IS_SKELETON )
GAME( 200?, lhzb3,     0,        igs_mahjong, base,     igs_m027_state, init_lhzb3,    ROT0, "IGS", "Long Hu Zhengba III", MACHINE_IS_SKELETON ) // 龙虎争霸Ⅲ
GAME( 2004, lhzb4,     0,        igs_mahjong, base,     igs_m027_state, init_lhzb4,    ROT0, "IGS", "Long Hu Zhengba 4", MACHINE_IS_SKELETON ) // 龙虎争霸4
GAME( 200?, klxyj,     0,        igs_mahjong, base,     igs_m027_state, init_klxyj,    ROT0, "IGS", "Kuai Le Xi You Ji", MACHINE_IS_SKELETON )
GAME( 200?, extradrw,  0,        igs_mahjong, base,     igs_m027_state, init_qlgs,     ROT0, "IGS", "Extra Draw", MACHINE_IS_SKELETON )
// these have an IGS025 protection device instead of the 8255
GAME( 200?, gonefsh2,  0,        igs_mahjong, base,     igs_m027_state, init_gonefsh2, ROT0, "IGS", "Gone Fishing 2", MACHINE_IS_SKELETON )
GAME( 2002, chessc2,   0,        igs_mahjong, base,     igs_m027_state, init_chessc2,  ROT0, "IGS", "Chess Challenge II", MACHINE_IS_SKELETON )
