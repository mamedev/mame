// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/**********************************************************************************************************

PACHI FEVER / (c) 1983 SANKI DENSHI KOGYO

===========================================================================================================

Many thanks to Olivier Galibert and Wilbert Pol for identify the CPU

Driver by Tomasz Slanina

- CPU has scratched surface and custom marks "GEN480830   DBS 30102" plus TI logo.
  There's "GENZUK 01" print on the PCB, near the chip.
  It's  TMS9995 or derivative ( decrementer + lvl3 interrupt, internal ram)
  XTAL:12.000MHZ
- VDP has also custom label ( "RY 050012    DDU 30600" ) plus TI logo
  Seems to be TMS9928A
  XTAL:10.738MHZ
- 2xY2404 ( SN76489A compatible? ) for music and sfx
- MSM5205 - sample player (see below)

- TODO:
  - what's the correct game title - Pachifever ? Fever 777 ?
  - unknown writes ($ffxx range)
  - coins : try to understand why they don't always register (use PORT_IMPULSE ?)
  - controls : unused bits (is the BUTTON1 used _only_ for entering initials?)
  - controls : make PLUNGER (or whatever it is in reality) implementation more clear
               and find a better way to handle cocktail mode;
               here's some code used to read plunger pos(angle?):

0284: 04CC             clr  R12             ; CRU address
0286: 0208 FF00        li   R8,>ff00        ; R8=ff00 - initial data ($ff)
028A: D688             movb R8,*R10         ; ff-> ff40  - write to output
028C: 3449             stcr R9,1            ; CRU read (one bit)
028E: 1603             jne  >0296           ; not zer0 - end
0290: 0228 FC00        ai   R8,>fc00        ; R8=R8-4
0294: 18FA             joc  >028a           ; loop

0296: D6A0 020F        movb @>020f,*R10     ; 00 ->ff40 - end of controls read
029A: 045B             b    *R11            ; b $1ca - process data in R8


Stephh's notes (based on the game TMS9995 code and some tests) :

  - "Difficulty" Dip Switch is a guess, but I can't see what else it can be. The values (-8 to +8) are
    correct, but it's impossible for me to tell what are the easiest and the hardest settings.
  - In "attract mode", the game plays the 4th level. This "attract mode" will end when the computer's
    balls reaches the limit or when the time is over (same rules as for the players - see gameplay below).
  - Gameplay :
      * On levels 1 to 3, your number of balls must reach the limit based on DSW2 bits 6 and 7
        to complete the level; once this is done, you go to next level or bonus stage (after level 3).
        Time is reset to value based on DSW2 bits 6 and 7 only at the beginning of level 1,
        while balls are reset on each level to value based on DSW1 bits 4 and 5.
      * On the bonus stage, you only have 3 balls (which are decremented from your number of balls)
        to get maximum bonus time, bonus balls, and points. During this stage, time is not decremented.
      * On level 4, your number of balls must reach the limit based on DSW2 bits 4 and 5 (making the game
        sometimes harder) to complete the level; once this is done, the game is over for current player.
        The main difference with levels 1 to 3 is that balls are not reset at the beginning of the level.
      * Once time or game is over, the game switches player 2 (if available). When all players have ended
        their game, they can decide to continue : a message is displayed but you don't see any timer.
        When you continue, only the balls aren't reset, while score, time and level (GASP !) are.
        If you want to continue in a 2 players game, BOTH players will have to continue, which means that
        you must have at least 2 credits ("REPLAY") and that you can't continue player 2 without player 1.
      * If you manage to get a score, use BUTTON1 to cycle through available symbols (letters A-Z and '.'),
        and pull the plunger to at least 63% (the code expects a value >= 0xa0) to go to next initial.
        Be aware that again there is a timer to do so, but that again the timer is not displayed.
  - Useful addresses :
      * 0xe001.b : level (0x00-0x04 : 0x01 = level 1 - 0x02 = level 2 - 0x00 = level 3 - 0x3 = bonus - 0x04 = level 4)
      * 0xe00f.b : player (0x00 = P1 - 0x01 = P2)
      * 0xe016.w : P1 balls (MSB first)
      * 0xe018.d : P1 score (MSB first)
      * 0xe01c.w : P1 time  (MSB first)
      * 0xe01e.w : P2 balls (MSB first)
      * 0xe024.w : P2 time  (MSB first)
      * 0xe020.d : P1 score (MSB first)
      * 0xe028.w : limit (DSW2 bits 6 and 7) for levels 1 to 3 (MSB first)
      * 0xe02a.w : limit (DSW2 bits 4 and 5) for level  4      (MSB first)

***********************************************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "speaker.h"


namespace {

#define USE_MSM 0
#define NUM_PLUNGER_REPEATS    50

class pachifev_state : public driver_device
{
public:
	pachifev_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	void pachifev(machine_config &config);

private:
	/* controls related */
	int m_power = 0;
	int m_max_power = 0;
	int m_input_power = 0;
	int m_previous_power = 0;
	int m_cnt = 0;

#if USE_MSM
	uint32_t m_adpcm_pos = 0;
	uint8_t m_adpcm_idle = 0;
	uint8_t m_trigger = 0;
	uint8_t m_adpcm_data = 0;
#endif
	void controls_w(uint8_t data);
	uint8_t controls_r();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void vblank_w(int state);
	required_device<tms9995_device> m_maincpu;
	void pachifev_cru(address_map &map) ATTR_COLD;
	void pachifev_map(address_map &map) ATTR_COLD;
};

void pachifev_state::controls_w(uint8_t data)
{
	if(!data)
	{
		/*end of input read*/
		m_power=0;
		m_max_power=m_input_power;
		if(--m_cnt <= 0) /* why to do it N times? no idea.. someone should fix it */
		{
			m_cnt=0;
			m_input_power=0;
		}
	}
}

uint8_t pachifev_state::controls_r()
{
	int output_bit=(m_power < m_max_power)?0:1;
	++m_power;
	return output_bit;
}

void pachifev_state::pachifev_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();

	map(0xe000, 0xe7ff).ram();
	map(0xf000, 0xf0fb).noprw();  /* internal ram */
	map(0xff00, 0xff00).portr("IN0");
	map(0xff02, 0xff02).portr("IN1");
	map(0xff04, 0xff04).portr("DSW1");
	map(0xff06, 0xff06).portr("DSW2");
	map(0xff08, 0xff08).portr("DSW3");
	map(0xff10, 0xff10).rw("tms9928a", FUNC(tms9928a_device::vram_read), FUNC(tms9928a_device::vram_write));
	map(0xff12, 0xff12).rw("tms9928a", FUNC(tms9928a_device::register_read), FUNC(tms9928a_device::register_write));
	map(0xff20, 0xff20).w("y2404_1", FUNC(y2404_device::write));
	map(0xff30, 0xff30).w("y2404_2", FUNC(y2404_device::write));
	map(0xff40, 0xff40).w(FUNC(pachifev_state::controls_w));
	map(0xff50, 0xff50).nopw(); /* unknown */
	map(0xfffa, 0xfffb).noprw(); /* decrementer */
	map(0xfffc, 0xffff).noprw(); /* nmi */
}

void pachifev_state::pachifev_cru(address_map &map)
{
	map(0x0000, 0x0001).r(FUNC(pachifev_state::controls_r));
}


/* verified from TMS9995 code */
static INPUT_PORTS_START( pachifev )
	/* 0xff00, cpl'ed -> 0xf0a0 (internal RAM) */
	PORT_START("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )                 /* select initial for player 1 and player 2 (upright) */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x4d, IP_ACTIVE_LOW, IPT_UNUSED )

	/* 0xff02, cpl'ed -> 0xf0a1 (internal RAM) */
	PORT_START("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL   /* select initial for player 2 (cocktail) */
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	/* 0xff04, cpl'ed -> 0xf0a2 (internal RAM) */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Difficulty ?" )              /* table at 0x5000 - see notes */
	PORT_DIPSETTING(    0x07, "-8" )
	PORT_DIPSETTING(    0x06, "-6" )
	PORT_DIPSETTING(    0x05, "-4" )
	PORT_DIPSETTING(    0x04, "-2" )
	PORT_DIPSETTING(    0x03, "+2" )
	PORT_DIPSETTING(    0x02, "+4" )
	PORT_DIPSETTING(    0x01, "+6" )
	PORT_DIPSETTING(    0x00, "+8" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x00, "Balls" )                     /* table at 0x5020 */
	PORT_DIPSETTING(    0x30, "25" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x10, "100" )
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )

	/* 0xff06, cpl'ed -> 0xf0a3 (internal RAM) */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Time" )                      /* table at 0x5028 */
	PORT_DIPSETTING(    0x00, "180" )
	PORT_DIPSETTING(    0x01, "120" )
	PORT_DIPSETTING(    0x02, "150" )
	PORT_DIPSETTING(    0x03, "90" )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPUNUSED( 0x08, 0x08 )
	PORT_DIPNAME( 0x30, 0x30, "Limit (Level 4)" )           /* table at 0x5038 - stored at 0xe02a.w */
	PORT_DIPSETTING(    0x30, "500" )
	PORT_DIPSETTING(    0x20, "1000" )
	PORT_DIPSETTING(    0x10, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Limit (Levels 1 to 3)" )     /* table at 0x5030 - stored at 0xe028.w */
	PORT_DIPSETTING(    0xc0, "300" )
	PORT_DIPSETTING(    0x80, "500" )
	PORT_DIPSETTING(    0x40, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )

	/* 0xff08, cpl'ed -> 0xf0a4 (internal RAM) */
	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, 0x02 )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPNAME( 0x18, 0x18, "Bonus Time" )                /* table at 0x3500 */
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "15" )
	PORT_DIPNAME( 0xe0, 0xe0, "Bonus Ball" )                /* table at 0x3508 */
	PORT_DIPSETTING(    0xe0, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPSETTING(    0xa0, "5" )
	PORT_DIPSETTING(    0x80, "8" )
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPSETTING(    0x40, "11" )
	PORT_DIPSETTING(    0x20, "13" )
	PORT_DIPSETTING(    0x00, "15" )

	PORT_START("PLUNGER_P1")
	PORT_BIT( 0x3f, 0x00, IPT_POSITIONAL ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(30) PORT_KEYDELTA(4) PORT_CENTERDELTA(0xff)

	PORT_START("PLUNGER_P2")
	PORT_BIT( 0x3f, 0x00, IPT_POSITIONAL ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(30) PORT_KEYDELTA(4) PORT_CENTERDELTA(0xff) PORT_COCKTAIL
INPUT_PORTS_END


#if USE_MSM


void pachifev_state::pf_adpcm_int(int state)
{
	if (m_adpcm_pos >= 0x4000 || m_adpcm_idle)
	{
		m_adpcm_idle = 1;
		m_msm->reset_w(1);
		m_trigger = 0;
	}
	else
	{
		uint8_t *ROM = memregion("adpcm")->base();

		m_adpcm_data = ((m_trigger ? (ROM[m_adpcm_pos] & 0x0f) : (ROM[m_adpcm_pos] & 0xf0)>>4) );
		m_msm->data_w(m_adpcm_data & 0xf);
		m_trigger^=1;
		if(m_trigger == 0)
		{
			m_adpcm_pos++;
			if((ROM[m_adpcm_pos] & 0xff) == 0xff)
				m_adpcm_idle = 1;
		}
	}
}

#endif

void pachifev_state::machine_reset()
{
	// Pulling down the line on RESET configures the CPU to insert one wait
	// state on external memory accesses
	m_maincpu->ready_line(CLEAR_LINE);
	m_maincpu->reset_line(ASSERT_LINE);

	m_power=0;
	m_max_power=0;
	m_input_power=0;
	m_previous_power=0;
	m_cnt=0;

#if USE_MSM
	m_adpcm_pos = 0;
#endif
}


void pachifev_state::vblank_w(int state)
{
	if (state)
	{
		static const char *const inname[2] = { "PLUNGER_P1", "PLUNGER_P2" };

		/* I wish I had found a better way to handle cocktail inputs, but I can't find a way to access internal RAM */
		/* (bit 5 of 0xf0aa : 0 = player 1 and 1 = player 2 - bit 6 of 0xf0aa : 0 = upright and 1 = cocktail). */
		/* All I found is that in main RAM, 0xe00f.b determines the player : 0x00 = player 1 and 0x01 = player 2. */
		address_space &ramspace = m_maincpu->space(AS_PROGRAM);
		uint8_t player = 0;

		if ((ramspace.read_byte(0xe00f) == 0x01) && ((ioport("DSW1")->read() & 0x08) == 0x00))
			player = 1;

		int current_power=ioport(inname[player])->read() & 0x3f;
		if(current_power != m_previous_power)
		{
			popmessage    ("%d%%", (current_power * 100) / 0x3f);
		}

		if( (!current_power) && (m_previous_power) )
		{
			m_input_power=m_previous_power;
			m_cnt=NUM_PLUNGER_REPEATS;
		}

		m_previous_power=current_power;
	}

}

void pachifev_state::machine_start()
{
	save_item(NAME(m_power));
	save_item(NAME(m_max_power));
	save_item(NAME(m_input_power));
	save_item(NAME(m_previous_power));
	save_item(NAME(m_cnt));
}

void pachifev_state::pachifev(machine_config &config)
{
	// CPU TMS9995, standard variant; no line connections
	TMS9995(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pachifev_state::pachifev_map);
	m_maincpu->set_addrmap(AS_IO, &pachifev_state::pachifev_cru);

	/* video hardware */
	tms9928a_device &vdp(TMS9928A(config, "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.screen_vblank().set(FUNC(pachifev_state::vblank_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
#if USE_MSM
	msm5205_device &adpcm(MSM5205(config, "adpcm", XTAL(384'000))); /* guess */
	adpcm.vck_legacy_callback().set(FUNC(pachifev_state::pf_adpcm_int));    /* interrupt function */
	adpcm.set_prescaler_selector(msm5205_device::S48_4B);    /* 8kHz */
	adpcm.add_route(ALL_OUTPUTS, "mono", 1.00);
#endif
	Y2404(config, "y2404_1", XTAL(10'738'635)/3).add_route(ALL_OUTPUTS, "mono", 0.30); /* guess */
	Y2404(config, "y2404_2", XTAL(10'738'635)/3).add_route(ALL_OUTPUTS, "mono", 0.30); /* guess */
}

ROM_START( pachifev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic42.00",   0x00000, 0x2000, CRC(9653546e) SHA1(0836d01118241d38bbf61732275afe3ae47d0622) )
	ROM_LOAD( "ic43.01",   0x02000, 0x2000, CRC(5572dce5) SHA1(fad45b33e095ac6e3ed3d7cdc3d8678c153a1b38) )
	ROM_LOAD( "ic44.02",   0x04000, 0x2000, CRC(98b3841f) SHA1(0563139877bf01e1673767ee1798bbcf68adadea) )
	ROM_LOAD( "ic45.03",   0x06000, 0x2000, CRC(6b76e6fa) SHA1(5be10ab0b76e2061fc7e9c77649572955bee7661) )
	ROM_LOAD( "ic46.04",   0x08000, 0x2000, CRC(1c8c66d7) SHA1(3b9b05f35b20d798651c7d5fdb35e6af956615a1) )
	/* empty ROM socket  - no data for 0xa000 - 0xbfff */
	ROM_LOAD( "ic48.50",   0x0c000, 0x2000, CRC(4ff52b70) SHA1(a459b52712250d2ecdbe6aeb8c400806867e9857) )

	ROM_REGION( 0x4000, "adpcm", 0 )
	ROM_LOAD( "ic66.10",   0x0000, 0x2000, CRC(217c573e) SHA1(6fb90865d1d81f5ea00fa7916d0ccb6756ef5ce5) )
ROM_END

} // anonymous namespace


GAME( 1983, pachifev,  0,       pachifev,  pachifev, pachifev_state, empty_init, ROT270, "Sanki Denshi Kogyo", "Pachifever", MACHINE_IMPERFECT_SOUND )
