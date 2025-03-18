// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    VIC Dual Game board

    Games supported:
        * Depth Charge
        * Sub Hunter
        * Safari
        * Frogs
        * Space Attack
        * Space Attack / Head On
        * Head On
        * Head On 2
        * Invinco / Head On 2
        * N-Sub
        * Samurai
        * Invinco
        * Invinco / Deep Scan
        * Tranquillizer Gun
        * Space Trek
        * Carnival
        * Borderline
        * Digger
        * Pulsar
        * Heiankyo Alien
        * Alpha Fighter / Head On

        * and a few clones and bootlegs

    Notes:
        * Head On and Space Attack had both color and monochrome
          versions. There is a game configuration option to
          switch between them.
        * There existed a vertical version of Head On as well.
        * According to the manuals, Borderline has the same sound
          board as Tranquillizer Gun.
        * The relationship between nostromo and startrks isn't clear:
          is one a bootleg of the other or are both bootlegs of an unknown
          original?

    Known issues/to-do's:
        * Analog sound missing in many games
        * Missing color PROM for "Alpha Fighter / Head On"
        * A few of the games have an extra 18K pull-up resistor on the
          blue color gun, Carnival, for example.
          Colors inaccurate?  Blue background?
        * Do other games have coinage hardware like nsub?
          brdrline and tranqgun have one too according to the manuals,
          but hardware details aren't known yet.
        * DIP switches need verifying in most of the games
        * DIP switch locations need to be added to some

****************************************************************************/

#include "emu.h"
#include "vicdual.h"
#include "vicdual-97271p.h"
#include "vicdual-97269pb.h"

#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"
#include "speaker.h"

#include "depthch.lh"



#define VICDUAL_MASTER_CLOCK                (XTAL(15'468'480))
#define VICDUAL_MAIN_CPU_CLOCK              (VICDUAL_MASTER_CLOCK/8)
#define VICDUAL_PIXEL_CLOCK                 (VICDUAL_MASTER_CLOCK/3)

#define VICDUAL_HTOTAL                      (0x148)
#define VICDUAL_HBEND                       (0x000)
#define VICDUAL_HBSTART                     (0x100)
#define VICDUAL_HSSTART                     (0x110)
#define VICDUAL_HSEND                       (0x130)
#define VICDUAL_VTOTAL                      (0x106)
#define VICDUAL_VBEND                       (0x000)
#define VICDUAL_VBSTART                     (0x0e0)
#define VICDUAL_VSSTART                     (0x0ec)
#define VICDUAL_VSEND                       (0x0f0)



/*
 *  Carnival sound routines
 *
 */

/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_CARNIVAL_PORT_1_RIFLE        0x01
#define OUT_CARNIVAL_PORT_1_CLANG        0x02
#define OUT_CARNIVAL_PORT_1_DUCK1        0x04
#define OUT_CARNIVAL_PORT_1_DUCK2        0x08
#define OUT_CARNIVAL_PORT_1_DUCK3        0x10
#define OUT_CARNIVAL_PORT_1_PIPEHIT      0x20
#define OUT_CARNIVAL_PORT_1_BONUS1       0x40
#define OUT_CARNIVAL_PORT_1_BONUS2       0x80

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_CARNIVAL_PORT_2_BEAR         0x04
#define OUT_CARNIVAL_PORT_2_RANKING      0x20


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const carnival_sample_names[] =
{
	"*carnival",
	"bear",
	"bonus1",
	"bonus2",
	"clang",
	"duck1",
	"duck2",
	"duck3",
	"pipehit",
	"ranking",
	"rifle",
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_CARNIVAL_BEAR = 0,
	SND_CARNIVAL_BONUS1,
	SND_CARNIVAL_BONUS2,
	SND_CARNIVAL_CLANG,
	SND_CARNIVAL_DUCK1,
	SND_CARNIVAL_DUCK2,
	SND_CARNIVAL_DUCK3,
	SND_CARNIVAL_PIPEHIT,
	SND_CARNIVAL_RANKING,
	SND_CARNIVAL_RIFLE
};


void carnival_state::carnival_audio_1_w(uint8_t data)
{
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port1State = data;

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_RIFLE )
	{
		PLAY( m_samples, SND_CARNIVAL_RIFLE, 0 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_CLANG )
	{
		PLAY( m_samples, SND_CARNIVAL_CLANG, 0 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_DUCK1 )
	{
		PLAY( m_samples, SND_CARNIVAL_DUCK1, 1 );
	}
	if ( bitsGoneHigh & OUT_CARNIVAL_PORT_1_DUCK1 )
	{
		STOP( m_samples, SND_CARNIVAL_DUCK1 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_DUCK2 )
	{
		PLAY( m_samples, SND_CARNIVAL_DUCK2, 1 );
	}
	if ( bitsGoneHigh & OUT_CARNIVAL_PORT_1_DUCK2 )
	{
		STOP( m_samples, SND_CARNIVAL_DUCK2 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_DUCK3 )
	{
		PLAY( m_samples, SND_CARNIVAL_DUCK3, 1 );
	}
	if ( bitsGoneHigh & OUT_CARNIVAL_PORT_1_DUCK3 )
	{
		STOP( m_samples, SND_CARNIVAL_DUCK3 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_PIPEHIT )
	{
		PLAY( m_samples, SND_CARNIVAL_PIPEHIT, 0 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_BONUS1 )
	{
		PLAY( m_samples, SND_CARNIVAL_BONUS1, 0 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_1_BONUS2 )
	{
		PLAY( m_samples, SND_CARNIVAL_BONUS2, 0 );
	}
}

void carnival_state::carnival_audio_2_w(uint8_t data)
{
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port2State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port2State = data;

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_2_BEAR )
	{
		PLAY( m_samples, SND_CARNIVAL_BEAR, 0 );
	}

	if ( bitsGoneLow & OUT_CARNIVAL_PORT_2_RANKING )
	{
		PLAY( m_samples, SND_CARNIVAL_RANKING, 0 );
	}

	// d4: music board MCU reset
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
}


/* Music board */

// common

void carnival_state::mboard_map(address_map &map)
{
	map(0x0000, 0x03ff).rom();
}

int carnival_state::carnival_music_port_t1_r()
{
	// T1: comms from audio port 2 d3
	return ~m_port2State >> 3 & 1;
}


// AY8912 music

void carnival_state::carnival_psg_latch()
{
	if (m_musicBus & 1)
	{
		// BDIR W, BC1 selects address or data
		if (m_musicBus & 2)
			m_psg->address_w(m_musicData);
		else
			m_psg->data_w(m_musicData);
	}
}

void carnival_state::carnivala_music_port_1_w(uint8_t data)
{
	// P1: AY8912 d0-d7
	m_musicData = data;
	carnival_psg_latch();
}

void carnival_state::carnivala_music_port_2_w(uint8_t data)
{
	// P2 d6: AY8912 BDIR(R/W)
	// P2 d7: AY8912 BC1
	m_musicBus = data >> 6 & 3;
	carnival_psg_latch();
}

void carnival_state::carnivala_audio(machine_config &config)
{
	/* music board */
	I8035(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &carnival_state::mboard_map);
	m_audiocpu->p1_out_cb().set(FUNC(carnival_state::carnivala_music_port_1_w));
	m_audiocpu->p2_out_cb().set(FUNC(carnival_state::carnivala_music_port_2_w));
	m_audiocpu->t1_in_cb().set(FUNC(carnival_state::carnival_music_port_t1_r));

	AY8912(config, m_psg, XTAL(3'579'545)/3).add_route(ALL_OUTPUTS, "mono", 0.25); // also seen with AY-3-8910, daughterboard has place for either chip

	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(10);
	m_samples->set_samples_names(carnival_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}


// PIT8253 music

void carnival_state::carnivalb_music_port_1_w(uint8_t data)
{
	// P1: PIT8253 d0-d7
	m_musicData = data;
}

void carnival_state::carnivalb_music_port_2_w(uint8_t data)
{
	// P2 d7: PIT8253 write strobe
	// P2 d5,d6: PIT8253 A0,A1
	if (~m_musicBus & data & 0x80)
		m_pit->write(data >> 5 & 3, m_musicData);

	m_musicBus = data;
}

void carnival_state::carnivalb_audio(machine_config &config)
{
	// already inherited carnivala_audio
	config.device_remove("psg");

	m_audiocpu->p1_out_cb().set(FUNC(carnival_state::carnivalb_music_port_1_w));
	m_audiocpu->p2_out_cb().set(FUNC(carnival_state::carnivalb_music_port_2_w));

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(XTAL(3'579'545)/3);
	m_pit->set_clk<1>(XTAL(3'579'545)/3);
	m_pit->set_clk<2>(XTAL(3'579'545)/3);

	m_pit->out_handler<0>().set(m_dac[0], FUNC(dac_bit_interface::write));
	m_pit->out_handler<1>().set(m_dac[1], FUNC(dac_bit_interface::write));
	m_pit->out_handler<2>().set(m_dac[2], FUNC(dac_bit_interface::write));

	for (int i = 0; i < 3; i++)
		DAC_1BIT(config, m_dac[i]).add_route(ALL_OUTPUTS, "mono", 0.15);
}



/*
 *  Deptch Charge sound routines
 *
 */

/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_DEPTHCH_PORT_1_LONGEXPL     0x01
#define OUT_DEPTHCH_PORT_1_SHRTEXPL     0x02
#define OUT_DEPTHCH_PORT_1_SPRAY        0x04
#define OUT_DEPTHCH_PORT_1_SONAR        0x08


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const depthch_sample_names[] =
{
	"*depthch",
	"longex",
	"shortex",
	"spray",
	"bonus",
	"sonar",
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_DEPTHCH_LONGEXPL = 0,
	SND_DEPTHCH_SHRTEXPL,
	SND_DEPTHCH_SPRAY,
	SND_DEPTHCH_BONUS,
	SND_DEPTHCH_SONAR
};


void vicdual_state::depthch_audio_w(uint8_t data)
{
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port1State = data;

	if ( bitsGoneHigh & OUT_DEPTHCH_PORT_1_LONGEXPL )
	{
		PLAY( m_samples, SND_DEPTHCH_LONGEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_DEPTHCH_PORT_1_SHRTEXPL )
	{
		PLAY( m_samples, SND_DEPTHCH_SHRTEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_DEPTHCH_PORT_1_SPRAY )
	{
		PLAY( m_samples, SND_DEPTHCH_SPRAY, 0 );
	}

	if ( bitsGoneHigh & OUT_DEPTHCH_PORT_1_SONAR )
	{
		PLAY( m_samples, SND_DEPTHCH_SONAR, 1 );
	}
	if ( bitsGoneLow & OUT_DEPTHCH_PORT_1_SONAR )
	{
		STOP( m_samples, SND_DEPTHCH_SONAR );

		// bonus sound on same line as sonar
		PLAY( m_samples, SND_DEPTHCH_BONUS, 0 );
	}
}


void vicdual_state::depthch_audio(machine_config &config)
{
	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(5);
	m_samples->set_samples_names(depthch_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}



/*
 *  Invinco sound routines
 *
 */

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_INVINCO_PORT_2_SAUCER       0x04
#define OUT_INVINCO_PORT_2_MOVE1        0x08
#define OUT_INVINCO_PORT_2_MOVE2        0x10
#define OUT_INVINCO_PORT_2_FIRE         0x20
#define OUT_INVINCO_PORT_2_INVHIT       0x40
#define OUT_INVINCO_PORT_2_SHIPHIT      0x80


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const invinco_sample_names[] =
{
	"*invinco",
	"saucer",
	"move1",
	"move2",
	"fire",
	"invhit",
	"shiphit",
	"move3",    /* currently not used */
	"move4",    /* currently not used */
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_INVINCO_SAUCER = 0,
	SND_INVINCO_MOVE1,
	SND_INVINCO_MOVE2,
	SND_INVINCO_FIRE,
	SND_INVINCO_INVHIT,
	SND_INVINCO_SHIPHIT,
	SND_INVINCO_MOVE3,
	SND_INVINCO_MOVE4
};


void vicdual_state::invinco_audio_w(uint8_t data)
{
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port2State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port2State = data;

	if ( bitsGoneLow & OUT_INVINCO_PORT_2_SAUCER )
	{
		PLAY( m_samples, SND_INVINCO_SAUCER, 0 );
	}

	if ( bitsGoneLow & OUT_INVINCO_PORT_2_MOVE1 )
	{
		PLAY( m_samples, SND_INVINCO_MOVE1, 0 );
	}

	if ( bitsGoneLow & OUT_INVINCO_PORT_2_MOVE2 )
	{
		PLAY( m_samples, SND_INVINCO_MOVE2, 0 );
	}

	if ( bitsGoneLow & OUT_INVINCO_PORT_2_FIRE )
	{
		PLAY( m_samples, SND_INVINCO_FIRE, 0 );
	}

	if ( bitsGoneLow & OUT_INVINCO_PORT_2_INVHIT )
	{
		PLAY( m_samples, SND_INVINCO_INVHIT, 0 );
	}

	if ( bitsGoneLow & OUT_INVINCO_PORT_2_SHIPHIT )
	{
		PLAY( m_samples, SND_INVINCO_SHIPHIT, 0 );
	}
}


void vicdual_state::invinco_audio(machine_config &config)
{
	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(8);
	m_samples->set_samples_names(invinco_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}



/*
 *  Pulsar sound routines
 *
 *  TODO: change heart rate based on bit 7 of Port 1
 *
 */

/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PULSAR_PORT_1_CLANG        0x01
#define OUT_PULSAR_PORT_1_KEY          0x02
#define OUT_PULSAR_PORT_1_ALIENHIT     0x04
#define OUT_PULSAR_PORT_1_PHIT         0x08
#define OUT_PULSAR_PORT_1_ASHOOT       0x10
#define OUT_PULSAR_PORT_1_PSHOOT       0x20
#define OUT_PULSAR_PORT_1_BONUS        0x40
#define OUT_PULSAR_PORT_1_HBEAT_RATE   0x80    /* currently not used */

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PULSAR_PORT_2_SIZZLE       0x01
#define OUT_PULSAR_PORT_2_GATE         0x02
#define OUT_PULSAR_PORT_2_BIRTH        0x04
#define OUT_PULSAR_PORT_2_HBEAT        0x08
#define OUT_PULSAR_PORT_2_MOVMAZE      0x10


#define PLAY(samp,id,loop)           samp->start( id, id, loop )
#define STOP(samp,id)                samp->stop( id )


/* sample file names */
static const char *const pulsar_sample_names[] =
{
	"*pulsar",
	"clang",
	"key",
	"alienhit",
	"phit",
	"ashoot",
	"pshoot",
	"bonus",
	"sizzle",
	"gate",
	"birth",
	"hbeat",
	"movmaze",
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_PULSAR_CLANG = 0,
	SND_PULSAR_KEY,
	SND_PULSAR_ALIENHIT,
	SND_PULSAR_PHIT,
	SND_PULSAR_ASHOOT,
	SND_PULSAR_PSHOOT,
	SND_PULSAR_BONUS,
	SND_PULSAR_SIZZLE,
	SND_PULSAR_GATE,
	SND_PULSAR_BIRTH,
	SND_PULSAR_HBEAT,
	SND_PULSAR_MOVMAZE
};


void vicdual_state::pulsar_audio_1_w(uint8_t data)
{
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port1State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port1State = data;

	if ( bitsGoneLow & OUT_PULSAR_PORT_1_CLANG )
	{
		PLAY( m_samples, SND_PULSAR_CLANG, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_1_KEY )
	{
		PLAY( m_samples, SND_PULSAR_KEY, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_1_ALIENHIT )
	{
		PLAY( m_samples, SND_PULSAR_ALIENHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_1_PHIT )
	{
		PLAY( m_samples, SND_PULSAR_PHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_1_ASHOOT )
	{
		PLAY( m_samples, SND_PULSAR_ASHOOT, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_1_PSHOOT )
	{
		PLAY( m_samples, SND_PULSAR_PSHOOT, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_1_BONUS )
	{
		PLAY( m_samples, SND_PULSAR_BONUS, 0 );
	}
}


void vicdual_state::pulsar_audio_2_w(uint8_t data)
{
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port2State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port2State = data;

	if ( bitsGoneLow & OUT_PULSAR_PORT_2_SIZZLE )
	{
		PLAY( m_samples, SND_PULSAR_SIZZLE, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_2_GATE )
	{
		m_samples->start(SND_PULSAR_CLANG, SND_PULSAR_GATE);
	}
	if ( bitsGoneHigh & OUT_PULSAR_PORT_2_GATE )
	{
		STOP( m_samples, SND_PULSAR_CLANG );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_2_BIRTH )
	{
		PLAY( m_samples, SND_PULSAR_BIRTH, 0 );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_2_HBEAT )
	{
		PLAY( m_samples, SND_PULSAR_HBEAT, 1 );
	}
	if ( bitsGoneHigh & OUT_PULSAR_PORT_2_HBEAT )
	{
		STOP( m_samples, SND_PULSAR_HBEAT );
	}

	if ( bitsGoneLow & OUT_PULSAR_PORT_2_MOVMAZE )
	{
		PLAY( m_samples, SND_PULSAR_MOVMAZE, 1 );
	}
	if ( bitsGoneHigh & OUT_PULSAR_PORT_2_MOVMAZE )
	{
		STOP( m_samples, SND_PULSAR_MOVMAZE );
	}
}


void vicdual_state::pulsar_audio(machine_config &config)
{
	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(12);
	m_samples->set_samples_names(pulsar_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}



/*************************************
 *
 *  Coin handling
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(vicdual_state::clear_coin_status)
{
	m_coin_status = 0;
}

void vicdual_state::assert_coin_status()
{
	m_coin_status = 1;
}

int vicdual_state::coin_status_r()
{
	return m_coin_status;
}


/* the main CPU is reset when a coin is inserted */
void vicdual_state::coin_in()
{
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	/* simulate the coin switch being closed for a while */
	m_coinstate_timer->adjust(attotime::from_msec(70));
}

INPUT_CHANGED_MEMBER(vicdual_state::coin_changed)
{
	if (newval)
	{
		/* increment the coin counter */
		machine().bookkeeping().coin_counter_w(0, 1);
		machine().bookkeeping().coin_counter_w(0, 0);

		coin_in();
	}
}

INPUT_CHANGED_MEMBER( headonsa_state::headonsa_coin_inserted )
{
	if (newval)
	{
		/* increment the coin counter */
		machine().bookkeeping().coin_counter_w(0, 1);
		machine().bookkeeping().coin_counter_w(0, 0);

		// hooked up to NMI instead of RESET line
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

		// checks for coin status in a much shorter time frame
		// check headon2s at 0x5f80 onward
		m_coin_status = 1;
		m_coinstate_timer->adjust(attotime::from_msec(10));
	}
}

#define PORT_COIN_DEFAULT                               \
	PORT_START("COIN")                                  \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vicdual_state::coin_changed), 0)



/*************************************
 *
 *  Timing
 *
 *  Various games use various timing
 *  sources -- the only way to tell for
 *  sure which is which is from the
 *  schematics.
 *
 *************************************/

int vicdual_state::get_vcounter()
{
	int vcounter = m_screen->vpos();

	/* the vertical synch counter gets incremented at the end of HSYNC,
	   compensate for this */
	if (m_screen->hpos() >= VICDUAL_HSEND)
		vcounter = (vcounter + 1) % VICDUAL_VTOTAL;

	return vcounter;
}


int vicdual_state::get_64v()
{
	return (get_vcounter() >> 6) & 0x01;
}


int vicdual_state::vblank_comp_r()
{
	return (get_vcounter() < VICDUAL_VBSTART);
}


int vicdual_state::cblank_comp_r()
{
	return (vblank_comp_r() && !m_screen->hblank());
}


int vicdual_state::timer_value_r()
{
	/* return the state of the timer (old code claims "4MHz square wave", but it was toggled once every 2msec, or 500Hz) */
	return machine().time().as_ticks(500) & 1;
}


/*************************************
 *
 *  Color vs. B&W configuration
 *
 *************************************/

#define COLOR_BW_PORT_TAG       "COLOR_BW"


int vicdual_state::is_cabinet_color()
{
	return (m_color_bw.read_safe(0) & 1) ? 0 : 1;
}


#define PORT_CABINET_COLOR_OR_BW                        \
	PORT_START(COLOR_BW_PORT_TAG)                       \
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Cabinet ) )     \
	PORT_CONFSETTING(    0x00, "Color" )                \
	PORT_CONFSETTING(    0x01, "Black and White" )



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/


void vicdual_state::videoram_w(offs_t offset, uint8_t data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_videoram[offset] = data;
}


void vicdual_state::characterram_w(offs_t offset, uint8_t data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_characterram[offset] = data;
}


/*************************************
 *
 *  Root driver structure
 *
 *************************************/

void vicdual_state::machine_start()
{
	m_coin_status = 0;
	m_palette_bank = 0;

	m_port1State = 0;
	m_port2State = 0;

	save_item(NAME(m_coin_status));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_port1State));
	save_item(NAME(m_port2State));
}

void carnival_state::machine_start()
{
	vicdual_state::machine_start();

	m_musicData = 0;
	m_musicBus = 0;

	save_item(NAME(m_musicData));
	save_item(NAME(m_musicBus));
}

void carnivalh_state::machine_start()
{
	carnival_state::machine_start();

	m_previousaddress = 0;
	m_previousvalue = 0;

	save_item(NAME(m_previousaddress));
	save_item(NAME(m_previousvalue));
}


void tranqgun_state::machine_start()
{
	vicdual_state::machine_start();

	m_tranqgun_prot_return = 0;

	save_item(NAME(m_tranqgun_prot_return));
}


void vicdual_state::vicdual_root(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, VICDUAL_MAIN_CPU_CLOCK);

	TIMER(config, m_coinstate_timer).configure_generic(FUNC(vicdual_state::clear_coin_status));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VICDUAL_PIXEL_CLOCK, VICDUAL_HTOTAL, VICDUAL_HBEND, VICDUAL_HBSTART, VICDUAL_VTOTAL, VICDUAL_VBEND, VICDUAL_VBSTART);
}



/*************************************
 *
 *  Depthcharge
 *
 *************************************/

uint8_t vicdual_state::depthch_io_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset & 0x01)  ret = m_in0->read();
	if (offset & 0x08)  ret = m_in1->read();

	return ret;
}


void vicdual_state::depthch_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x04)  depthch_audio_w(data);
}


void vicdual_state::depthch_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0x83ff).mirror(0x7000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0x8400, 0x87ff).mirror(0x7000).ram();
	map(0x8800, 0x8fff).mirror(0x7000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}


void vicdual_state::depthch_io_map(address_map &map)
{
	map.global_mask(0x0f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x0f).rw(FUNC(vicdual_state::depthch_io_r), FUNC(vicdual_state::depthch_io_w));
}


static INPUT_PORTS_START( depthch )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::get_64v))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT
INPUT_PORTS_END


void vicdual_state::depthch(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	I8080(config.replace(), m_maincpu, VICDUAL_MAIN_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::depthch_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::depthch_io_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_bw));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	depthch_audio(config);
}



/*************************************
 *
 *  Safari
 *
 *************************************/

uint8_t vicdual_state::safari_io_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset & 0x01)  ret = m_in0->read();
	if (offset & 0x08)  ret = m_in1->read();

	return ret;
}


void vicdual_state::safari_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02) { /* safari_audio_w(0, data) */ }
}


void vicdual_state::safari_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).noprw(); /* unused */
	map(0x8000, 0x8fff).mirror(0x3000).ram();
	map(0xc000, 0xc3ff).mirror(0x3000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0xc400, 0xc7ff).mirror(0x3000).ram();
	map(0xc800, 0xcfff).mirror(0x3000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}


void vicdual_state::safari_io_map(address_map &map)
{
	map.global_mask(0x0f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x0f).rw(FUNC(vicdual_state::safari_io_r), FUNC(vicdual_state::safari_io_w));
}


static INPUT_PORTS_START( safari )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Aim Up") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Aim Down") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::get_64v))
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT
INPUT_PORTS_END


void vicdual_state::safari(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::safari_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::safari_io_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_bw));
}



/*************************************
 *
 *  Frogs
 *
 *************************************/

uint8_t vicdual_state::frogs_io_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset & 0x01)  ret = m_in0->read();
	if (offset & 0x08)  ret = m_in1->read();

	return ret;
}


void vicdual_state::frogs_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)  m_vicdual_sound->write(data);
}


void vicdual_state::frogs_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0x83ff).mirror(0x7000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0x8400, 0x87ff).mirror(0x7000).ram();
	map(0x8800, 0x8fff).mirror(0x7000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}


void vicdual_state::frogs_io_map(address_map &map)
{
	map.global_mask(0x0f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x0f).rw(FUNC(vicdual_state::frogs_io_r), FUNC(vicdual_state::frogs_io_w));
}


static INPUT_PORTS_START( frogs )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY   /* The original joystick was a 3-way */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY   /* stick, of which Mame's 4-way does */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY   /* a fine simulation */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DOOR:1") // 1 switch located on the inside of the coin door
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	// There is no dipswitch for these game settings: on a physical level, they are applied
	// by grounding otherwise floating pins (ground wires are provided on pin 1/30)
	PORT_CONFNAME( 0x10, 0x10, "Wire pin 5/26: Allow Free Game" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x10, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x20, 0x20, "Wire pin 4/27: Game Time" )
	PORT_CONFSETTING(    0x00, "60" )
	PORT_CONFSETTING(    0x20, "90" )
	PORT_CONFNAME( 0x40, 0x40, "Wire pin 3/28: Coinage" )
	PORT_CONFSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_CONFSETTING(    0x40, DEF_STR( 1C_1C ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::get_64v))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT

//  PORT_START("IN2")
//  PORT_ADJUSTER( 25, "Boing Volume" )

//  PORT_START("IN3")
//  PORT_ADJUSTER( 25, "Buzzz Volume" )

//  PORT_START("IN4")
//  PORT_ADJUSTER( 25, "Croak Volume" )

//  PORT_START("IN5")
//  PORT_ADJUSTER( 25, "Hop Volume" )

//  PORT_START("IN6")
//  PORT_ADJUSTER( 50, "Splash Volume" )

	PORT_START("R93")
	PORT_ADJUSTER( 50, "Zip Volume" )
INPUT_PORTS_END


void vicdual_state::frogs(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::frogs_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::frogs_io_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_bw));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	FROGS_AUDIO(config, m_vicdual_sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  Head On
 *  Space Attack
 *
 *************************************/

uint8_t vicdual_state::headon_io_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset & 0x01)  ret = m_in0->read();
	if (offset & 0x08)  ret = m_in1->read();

	return ret;
}


uint8_t vicdual_state::sspaceat_io_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset & 0x01)  ret = m_in0->read();
	if (offset & 0x04)  ret = m_in1->read();
	if (offset & 0x08)  ret = m_in2->read();

	return ret;
}


void vicdual_state::headon_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)  headon_audio_w(data);
	if (offset & 0x04) { /* palette_bank_w(data)  */ }    /* not written to */
}


void vicdual_state::headon_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).rom();
	map(0x8000, 0xbfff).noprw(); /* unused */
	map(0xc000, 0xc3ff).mirror(0x3000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0xc400, 0xc7ff).mirror(0x3000).ram();
	map(0xc800, 0xcfff).mirror(0x3000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}


void vicdual_state::headon_io_map(address_map &map)
{
	map.global_mask(0x0f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x0f).rw(FUNC(vicdual_state::headon_io_r), FUNC(vicdual_state::headon_io_w));
}


void vicdual_state::sspaceat_io_map(address_map &map)
{
	map.global_mask(0x0f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x0f).rw(FUNC(vicdual_state::sspaceat_io_r), FUNC(vicdual_state::headon_io_w));
}


static INPUT_PORTS_START( headon )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::get_64v))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( headonmz )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::get_64v))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) // protection? (check on startup)
	PORT_BIT( 0x7a, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( headons )
	PORT_INCLUDE( headon )

	PORT_MODIFY(COLOR_BW_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* no color/bw option */
INPUT_PORTS_END

static INPUT_PORTS_START( startrks )
	PORT_INCLUDE( headons )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( hocrash )
	PORT_INCLUDE( headons )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
INPUT_PORTS_END

// e7f1: coin counter
static INPUT_PORTS_START( headonsa )
	PORT_INCLUDE( headons )

	// flipped activeness, added a start button while at it
	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(headonsa_state::headonsa_coin_inserted), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( supcrash )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) // both this and the following switch seem to select between 5 and 6 lives? but in combination with 0x02 of IN1 you can get 3 and 4 lives?
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) // see comment above
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Rom Test" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x78, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( carnivalh )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( sspaceat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "Bonus Life For Final UFO" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0e, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x0a, "5" )
	PORT_DIPSETTING(    0x06, "6" )
/* the following are duplicates
    PORT_DIPSETTING(    0x00, "4" )
    PORT_DIPSETTING(    0x04, "4" )
    PORT_DIPSETTING(    0x08, "4" )
    PORT_DIPSETTING(    0x02, "5" ) */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "Credits Display" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN_DEFAULT
INPUT_PORTS_END


void vicdual_state::headon(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::headon_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::headon_io_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_bw_or_color));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	headon_audio(config);
}

void vicdual_state::headons(machine_config &config)
{
	headon(config);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_bw));
}


void vicdual_state::sspaceat(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::headon_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::sspaceat_io_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_bw_or_color));
}



/*************************************
 *
 *  Head On 2
 *  Digger
 *
 *************************************/

uint8_t vicdual_state::headon2_io_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset & 0x01) data &= m_in0->read(); // schematics IN1
	if (offset & 0x02) data &= 0xff;          // schematics IN2 (not read)
	if (offset & 0x04) data &= m_in1->read(); // schematics IN4
	if (offset & 0x08) data &= m_in2->read();

	return data;
}

void vicdual_state::headon2_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01) assert_coin_status();
	if (offset & 0x02) headon_audio_w(data);
}


void vicdual_state::digger_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02) { /* digger_audio_1_w(0, data) */ }
	if (offset & 0x04)
	{
		palette_bank_w(data & 0x03);
		/* digger_audio_2_w(0, data & 0xfc); */
	}

	if (offset & 0x08) { /* schematics show this as going into a shifter circuit, but never written to */ }
	if (offset & 0x10) { /* schematics show this as going to an edge connector, but never written to */ }
	if (offset & 0x18)  logerror("********* Write to port %x\n", offset);
}


void vicdual_state::headon2_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).rom();
	/* map(0x8000, 0x80ff).mirror(0x3f00); */  /* schematics show this as battery backed RAM, but doesn't appear to be used */
	map(0xc000, 0xc3ff).mirror(0x3000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0xc400, 0xc7ff).mirror(0x3000).ram();
	map(0xc800, 0xcfff).mirror(0x3000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}


void vicdual_state::headon2_io_map(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x1f).rw(FUNC(vicdual_state::headon2_io_r), FUNC(vicdual_state::headon2_io_w));
}


void vicdual_state::digger_io_map(address_map &map)
{
	map.global_mask(0x1f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x1f).rw(FUNC(vicdual_state::headon2_io_r), FUNC(vicdual_state::digger_io_w));
}


static INPUT_PORTS_START( headon2 )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY

	PORT_START("IN1")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME(0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(   0x18, "4" )
	PORT_DIPSETTING(   0x10, "5" )
//  PORT_DIPSETTING(   0x08, "5" )
	PORT_DIPSETTING(   0x00, "6" )
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME(0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x02, DEF_STR( On ) )
	PORT_BIT(0x7c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( car2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/* controls are active_high around on this bootleg */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_4WAY

	PORT_START("IN1")
	// seems to ignore lives dip-switches (hardcoded to 3 lives)
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_BIT( 0x7c, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( headon2s )
	PORT_INCLUDE( car2 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(headonsa_state::headonsa_coin_inserted), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( digger )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
//  PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))          // it's like this according to the schematics, but gameplay speed is too fast;
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r)) // gameplay speed is correct now, there's likely an error in the schematics then...
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT
INPUT_PORTS_END


MACHINE_RESET_MEMBER( vicdual_state, headon2 )
{
	m_palette_bank = 3;
}


void vicdual_state::headon2(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::headon2_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::headon2_io_map);

	MCFG_MACHINE_RESET_OVERRIDE(vicdual_state, headon2)

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_color));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	headon_audio(config);
}

void vicdual_state::headon2bw(machine_config &config)
{
	headon2(config);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_bw));
}


void vicdual_state::digger(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::headon2_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::digger_io_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_color));
}



/*************************************
 *
 *  Invinco / Head On 2
 *  Invinco / Deap Scan
 *  Space Attack / Head On
 *  Tranquillizer Gun
 *  Space Trek
 *  Carnival
 *  Borderline
 *  Pulsar
 *  Heiankyo Alien
 *  Alpha Fighter / Head On
 *
 *************************************/

void vicdual_state::invho2_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  invho2_audio_w(data);
	if (offset & 0x02)  invinco_audio_w(data);
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}


void vicdual_state::invds_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  invinco_audio_w(data);
	if (offset & 0x02) { /* deepscan_audio_w(0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}

void vicdual_state::carhntds_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01) { /* invinco_audio_w(data); */ }
	if (offset & 0x02) { /* deepscan_audio_w(0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}


void vicdual_state::sspacaho_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  invho2_audio_w(data);
	if (offset & 0x02) { /* sspaceatt_audio_w(data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}


void tranqgun_state::tranqgun_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  m_vicdual_sound->write(data);
	if (offset & 0x02)  palette_bank_w(data);
	if (offset & 0x08)  assert_coin_status();
}


void vicdual_state::spacetrk_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01) { /* spacetrk_audio_w(data) */ }
	if (offset & 0x02) { /* spacetrk_audio_w(data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}


void carnival_state::carnival_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  carnival_audio_1_w(data);
	if (offset & 0x02)  carnival_audio_2_w(data);
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}


void vicdual_state::brdrline_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  m_vicdual_sound->write(data);
	if (offset & 0x02)  palette_bank_w(data);
	if (offset & 0x08)  assert_coin_status();
}


void vicdual_state::pulsar_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  pulsar_audio_1_w(data);
	if (offset & 0x02)  pulsar_audio_2_w(data);
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}


void vicdual_state::heiankyo_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01) { /* heiankyo_audio_1_w(0, data) */ }
	if (offset & 0x02) { /* heiankyo_audio_2_w(0, data) */ }
	if (offset & 0x08)  assert_coin_status();
}


void vicdual_state::alphaho_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01) { /* headon_audio_w(0, data) */ }
	if (offset & 0x02) { /* alphaf_audio_w(0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  palette_bank_w(data);
}


void vicdual_state::headonn_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01) invho2_audio_w(data);
	if (offset & 0x02)
	{
		// 7654----  unused?
		// ----32--  always 1
		// ------10  palette bank (bit 0 inverted)

		palette_bank_w((data & 3) ^ 1);
	}
	if (offset & 0x08) assert_coin_status();
}


void vicdual_state::vicdual_dualgame_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0x83ff).mirror(0x7000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0x8400, 0x87ff).mirror(0x7000).ram();
	map(0x8800, 0x8fff).mirror(0x7000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}

// see code at 0x3f04 (game over) and 0x3eba (passing level 3) in ROM
// both functions make the same 3 checks, assume we never want the 'fail' result
uint8_t tranqgun_state::tranqgun_prot_r(offs_t offset)
{
	logerror("%s: tranqgun_prot_r %04x\n", machine().describe_context(), offset);

	if (offset == 0x3800)
		return m_tranqgun_prot_return;

	return 0x00;
}

void tranqgun_state::tranqgun_prot_w(offs_t offset, uint8_t data)
{
	logerror("%s: tranqgun_prot_w %04x %02x\n", machine().describe_context(), offset, data);

	if (offset == 0x0000)
	{
		if (data == 0xd8)
			m_tranqgun_prot_return = 0x02;
		else if (data == 0x3a)
			m_tranqgun_prot_return = 0x01;
		else if (data == 0x6a)
			m_tranqgun_prot_return = 0x06;
	}
}

uint8_t tranqgun_state::brdrlinet_prot_r()
{
	logerror("%s: brdrlinet_prot_r\n", machine().describe_context());

	return m_tranqgun_prot_return;
}

void tranqgun_state::brdrlinet_prot_w(uint8_t data)
{
	logerror("%s: brdrlinet_prot_w %02x\n", machine().describe_context(), data);

	if (data == 0xd8)
		m_tranqgun_prot_return = 0x02;
	else if (data == 0x3a)
		m_tranqgun_prot_return = 0x01;
}

void tranqgun_state::tranqgun_dualgame_map(address_map &map)
{
	vicdual_dualgame_map(map);
	map(0x4000, 0x7fff).rw(FUNC(tranqgun_state::tranqgun_prot_r), FUNC(tranqgun_state::tranqgun_prot_w));
}

void tranqgun_state::brdrlinet_dualgame_map(address_map &map)
{
	vicdual_dualgame_map(map);
	map(0x7800, 0x7800).r(FUNC(tranqgun_state::brdrlinet_prot_r));
	map(0xe9a8, 0xe9a8).w(FUNC(tranqgun_state::brdrlinet_prot_w));
}

uint8_t carnivalh_state::carnivalh_prot_r(offs_t offset)
{
	// There appear to be 2 protection functions in the code
	// The first one is called if a duck gets to the bottom of the screen, or if Player 1 Game Over condition is reached and Player 2 needs to start
	// The second one is called when you complete a bonus round

	uint8_t retdat = 0;

	if (/*((m_previousaddress == 0xe3d4) || (m_previousaddress == 0xe76b))*/ true && (m_previousvalue == 0x24))
		retdat = 0x02;
	else if (/*((m_previousaddress == 0xe3d4) || (m_previousaddress == 0xe76d))*/ true && (m_previousvalue == 0x66))
		retdat = 0x07;
	else if (/*((m_previousaddress == 0xe3d4) || (m_previousaddress == 0xe76f))*/ true && (m_previousvalue == 0x48))
		retdat = 0x03;
	else
		popmessage("%s: carnivalh_prot_r %04x %04x %02x\n", machine().describe_context(), offset, m_previousaddress, m_previousvalue);

	return retdat;
}

void carnivalh_state::carnivalh_prot_w(offs_t offset, uint8_t data)
{
	// the protection writes appear to overlay regular RAM areas?
	// protection writes are to 0xe76b, 0xe76d, 0xe76f for one function and 0xe3d4 for the other (see code at 0x26AF & 0x2892)

	m_previousaddress = offset+0xe000;
	m_previousvalue = data;

	// fall through to usual accesses (write to the non-mirror address so we're not recursive)
	address_space& spc = m_maincpu->space(AS_PROGRAM);
	spc.write_byte(0x8000+offset, data);
}


void carnivalh_state::carnivalh_dualgame_map(address_map &map)
{
	vicdual_dualgame_map(map);
	map(0x4000, 0x7fff).r(FUNC(carnivalh_state::carnivalh_prot_r));
	map(0xe000, 0xefff).w(FUNC(carnivalh_state::carnivalh_prot_w));
}

void vicdual_state::carhntds_dualgame_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // also has part of a rom mapped at 0x4000
	map(0x8000, 0x83ff).mirror(0x7000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0x8400, 0x87ff).mirror(0x7000).ram();
	map(0x8800, 0x8fff).mirror(0x7000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}

void vicdual_state::invho2_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::invho2_io_w));
}


void vicdual_state::invds_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::invds_io_w));
}

void vicdual_state::carhntds_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::carhntds_io_w));
}

void vicdual_state::sspacaho_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::sspacaho_io_w));
}


void tranqgun_state::tranqgun_io_map(address_map &map)
{
	map.global_mask(0x0f);

	map(0x00, 0x00).mirror(0x0c).portr("IN0");
	map(0x01, 0x01).mirror(0x0c).portr("IN1");
	map(0x02, 0x02).mirror(0x0c).portr("IN2");
	map(0x03, 0x03).mirror(0x0c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x0f).w(FUNC(tranqgun_state::tranqgun_io_w));
}


void vicdual_state::spacetrk_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::spacetrk_io_w));
}


void carnival_state::carnival_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(carnival_state::carnival_io_w));
}


void vicdual_state::brdrline_io_map(address_map &map)
{
	map.global_mask(0x0f);

	map(0x00, 0x00).mirror(0x0c).portr("IN0");
	map(0x01, 0x01).mirror(0x0c).portr("IN1");
	map(0x02, 0x02).mirror(0x0c).portr("IN2");
	map(0x03, 0x03).mirror(0x0c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x0f).w(FUNC(vicdual_state::brdrline_io_w));
}


void vicdual_state::pulsar_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::pulsar_io_w));
}


void vicdual_state::heiankyo_io_map(address_map &map)
{
	map.global_mask(0x0f);

	map(0x00, 0x00).mirror(0x0c).portr("IN0");
	map(0x01, 0x01).mirror(0x0c).portr("IN1");
	map(0x02, 0x02).mirror(0x0c).portr("IN2");
	map(0x03, 0x03).mirror(0x0c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x0f).w(FUNC(vicdual_state::heiankyo_io_w));
}


void vicdual_state::alphaho_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::alphaho_io_w));
}


void vicdual_state::headonn_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::headonn_io_w));
}


/* several of the games' lives DIPs are spread across two input ports */
template <int Param>
int vicdual_state::fake_lives_r()
{
	// FIXME: there should be two template parameters, but that makes port macros confused
	// use the low byte for the bitmask
	uint8_t bit_mask = (Param) & 0xff;

	// and use d8 for the port
	int port = (Param) >> 8 & 1;
	return (m_fake_lives[port].read_safe(0) & bit_mask) ? 0 : 1;
}


static INPUT_PORTS_START( invho2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x101>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x101>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT

	PORT_START("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x01, "Head On 2 Lives" )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
//  PORT_DIPSETTING(    0x02, "3" ) // dupe
	PORT_DIPSETTING(    0x03, "4" )

	/* There's probably a bug in the Invinco game code:
	   it does support lives set to 5 or 6, but the game
	   reads IN3 bit 3 instead of bit 2.
	   Note that the manual only lists setting it to 3 or 4.
	*/
	PORT_START("FAKE_LIVES.1")
	PORT_DIPNAME( 0x03, 0x03, "Invinco Lives" )     PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
//  PORT_DIPSETTING(    0x01, "5" ) // results in 3, see above
//  PORT_DIPSETTING(    0x00, "6" ) // results in 4, see above

INPUT_PORTS_END


static INPUT_PORTS_START( carhntds )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_NAME("P1 Up / Fire Left") // it's UP on Car Hunt but Fire Left on Deep Scan, what was it on the control panel??
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x002>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x101>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x102>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT

	PORT_START("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x01, "Car Hunt Lives" )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("FAKE_LIVES.1")
	PORT_DIPNAME( 0x03, 0x03, "Deep Scan Lives" )     PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x03, "4" )
INPUT_PORTS_END

// dip locations needs to be verified
// TODO: is it actually one button? Schems shows 2 but button 1 doesn't do anything?
static INPUT_PORTS_START( wantsega )
	PORT_INCLUDE( carhntds )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )

	PORT_MODIFY("IN3")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, "20k" )
	PORT_DIPSETTING(    0x00, "30k" )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) // teleport

	PORT_MODIFY("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_MODIFY("FAKE_LIVES.1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( invds )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x002>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x101>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x101>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT

	// SW1 @ C1, 6-pos (where are 5 & 6?)
	PORT_START("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x03, "Invinco Lives" )     PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )

	PORT_START("FAKE_LIVES.1")
	PORT_DIPNAME( 0x03, 0x01, "Deep Scan Lives" )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x03, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( sspacaho )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_DIPNAME( 0x08, 0x00, "Head On Lives" )         PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x002>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, "Space Attack Bonus Life" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, "Space Attack Final UFO Bonus" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT

	PORT_START("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x03, "Space Attack Lives" )    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END


static INPUT_PORTS_START( tranqgun )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::vblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( spacetrk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2") // unknown, but used
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* must be high for bonus life to work */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* must be high for bonus life to work */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( spacetrkc )
	PORT_INCLUDE( spacetrk )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( carnival )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DOOR:1") // 1 switch located on the inside of the coin door
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( carnivalc )
	PORT_INCLUDE( carnival )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( brdrline )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 8-pos (is 6-8 unconnected?)
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x002>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::vblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x004>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::get_64v))  /* yes, this is different */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT

	PORT_START("FAKE_LIVES.0")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x07, "Infinite (Cheat)" )
INPUT_PORTS_END


static INPUT_PORTS_START( starrkr )
	PORT_INCLUDE( brdrline )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( pulsar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x002>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT

	PORT_START("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x03, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( heiankyo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1") // bonus life?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "2 Players Mode" )    PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, "Alternating" )
	PORT_DIPSETTING(    0x00, "Simultaneous" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2") // bonus life?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* has to be 0, protection? */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3") // bonus life?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* has to be 0, protection? */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( alphaho )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x001>))
	PORT_DIPNAME( 0x08, 0x00, "Head On Lives" )         PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::fake_lives_r<0x002>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Alpha Fighter Bonus Life" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "1500" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Alpha Fighter Final UFO Bonus" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT

	PORT_START("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x03, "Alpha Fighter Lives" )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END


static INPUT_PORTS_START( alphahob )
	PORT_INCLUDE( alphaho )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x08, 0x00, "Circuit Lives" )           PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x00, "Missile Bonus Life" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "10k" )
	PORT_DIPSETTING(    0x04, "15k" )

	PORT_MODIFY("IN3")
	PORT_DIPNAME( 0x04, 0x00, "Missile Final UFO Bonus" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("FAKE_LIVES.0")
	PORT_DIPNAME( 0x03, 0x03, "Missile Lives" )           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END


// there is a dip switch with 8 switches on the board, but only switch a is used
static INPUT_PORTS_START( headonn )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_4WAY PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME(0x08, 0x00, DEF_STR(Lives))          PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(   0x00, "3")
	PORT_DIPSETTING(   0x08, "4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_4WAY
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_4WAY
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_4WAY
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_4WAY PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_4WAY PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_COIN_DEFAULT
INPUT_PORTS_END


void vicdual_state::vicdual_dualgame_root(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::vicdual_dualgame_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_color));
}



void vicdual_state::invho2(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::invho2_io_map);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	invinco_audio(config);
	headon_audio(config);
}



void vicdual_state::invds(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::invds_io_map);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	invinco_audio(config);
}

void vicdual_state::carhntds(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::carhntds_dualgame_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::carhntds_io_map);
}


void vicdual_state::sspacaho(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::sspacaho_io_map);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	headon_audio(config);
}


void vicdual_state::spacetrk(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::spacetrk_io_map);
}


void carnival_state::carnival(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &carnival_state::carnival_io_map);

	config.set_perfect_quantum(m_maincpu);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	carnivala_audio(config);
}

void carnival_state::carnivalb(machine_config &config)
{
	carnival(config);

	/* audio hardware */
	carnivalb_audio(config);
}

void carnivalh_state::carnivalh(machine_config &config)
{
	carnival(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &carnivalh_state::carnivalh_dualgame_map);
	m_maincpu->set_addrmap(AS_IO, &carnivalh_state::headon_io_map);
}


void tranqgun_state::tranqgun(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &tranqgun_state::tranqgun_io_map);
	m_maincpu->set_addrmap(AS_PROGRAM, &tranqgun_state::tranqgun_dualgame_map);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BORDERLINE_AUDIO(config, m_vicdual_sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void tranqgun_state::brdrlinet(machine_config &config)
{
	tranqgun(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &tranqgun_state::brdrlinet_dualgame_map);
}

void vicdual_state::brdrline(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::brdrline_io_map);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BORDERLINE_AUDIO(config, m_vicdual_sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void vicdual_state::pulsar(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::pulsar_io_map);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	pulsar_audio(config);
}


void vicdual_state::heiankyo(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::heiankyo_io_map);
}


void vicdual_state::alphaho(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::alphaho_io_map);
}


void vicdual_state::headonn(machine_config &config)
{
	vicdual_dualgame_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::headonn_io_map);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	headon_audio(config);
}


/*************************************
 *
 *  Samurai
 *
 *************************************/

void vicdual_state::samurai_protection_w(uint8_t data)
{
	m_samurai_protection_data = data;
}


template <int Offset>
int vicdual_state::samurai_protection_r()
{
	uint32_t answer = 0;

	if (m_samurai_protection_data == 0xab)
		answer = 0x02;
	else if (m_samurai_protection_data == 0x1d)
		answer = 0x0c;

	return (answer >> Offset) & 0x01;
}


void vicdual_state::samurai_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x02) { palette_bank_w(data); /* samurai_audio_w(0, data >> 4) */ }
	if (offset & 0x08)  assert_coin_status();
}


/* dual game hardware */
void vicdual_state::samurai_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom().w(FUNC(vicdual_state::samurai_protection_w));
	map(0x8000, 0x83ff).mirror(0x7000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0x8400, 0x87ff).mirror(0x7000).ram();
	map(0x8800, 0x8fff).mirror(0x7000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}


void vicdual_state::samurai_io_map(address_map &map)
{
	map.global_mask(0x7f);

	map(0x00, 0x00).mirror(0x7c).portr("IN0");
	map(0x01, 0x01).mirror(0x7c).portr("IN1");
	map(0x02, 0x02).mirror(0x7c).portr("IN2");
	map(0x03, 0x03).mirror(0x7c).portr("IN3");

	/* no decoder, just logic gates, so in theory the
	   game can write to multiple locations at once */
	map(0x00, 0x7f).w(FUNC(vicdual_state::samurai_io_w));
}


static INPUT_PORTS_START( samurai )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x08, "Infinite Lives (Cheat)" )    PORT_DIPLOCATION("SW1:5") // SW1 @ C1, 6-pos (is #6 unconnected?)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::samurai_protection_r<1>))
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:2") // unknown, but used
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::samurai_protection_r<2>))
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::timer_value_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::samurai_protection_r<3>))
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN_DEFAULT
INPUT_PORTS_END


MACHINE_START_MEMBER(vicdual_state,samurai)
{
	m_samurai_protection_data = 0;
	save_item(NAME(m_samurai_protection_data));

	machine_start();
}

void vicdual_state::samurai(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::samurai_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::samurai_io_map);

	MCFG_MACHINE_START_OVERRIDE(vicdual_state,samurai)

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_color));
}



/*************************************
 *
 *  N-Sub
 *
 *************************************/

uint8_t nsub_state::nsub_io_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset & 0x01)  ret = m_in0->read();
	if (offset & 0x08)  ret = m_in1->read();

	return ret;
}


void nsub_state::nsub_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)
	{
		m_s97271p->port_w(data);
	}
	if (offset & 0x04)
	{
		palette_bank_w(data);
		m_s97269pb->palette_bank_w(data);
	}
}


void nsub_state::nsub_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0xbfff).noprw(); /* unused */
	map(0xc000, 0xc3ff).mirror(0x3000).ram().w(FUNC(nsub_state::videoram_w)).share("videoram");
	map(0xc400, 0xc7ff).mirror(0x3000).ram();
	map(0xc800, 0xcfff).mirror(0x3000).ram().w(FUNC(nsub_state::characterram_w)).share("characterram");
}


void nsub_state::nsub_io_map(address_map &map)
{
	map.global_mask(0x0f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x0f).rw(FUNC(nsub_state::nsub_io_r), FUNC(nsub_state::nsub_io_w));
}

void nsub_state::nsubc_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)
		m_s97271p->port_w(data);

	if (offset & 0x02)
	{
		palette_bank_w(data);
		m_s97269pb->palette_bank_w(data);
	}

	if (offset & 0x08)
		assert_coin_status();
}

void nsub_state::nsubc_prot_w(uint8_t data)
{
	switch (data)
	{
		case 0x24: m_prot_value = 0x02; break;
		case 0x66: m_prot_value = 0x07; break;
		default: logerror("%s: unknown protection value %02x\n", machine().describe_context(), data);
	}
}

void nsub_state::nsubc_map(address_map &map)
{
	nsub_map(map);
	map(0x7f00, 0x7f00).lr8(NAME([this] () -> uint8_t { return m_prot_value; }));
	map(0xebe8, 0xebe8).w(FUNC(nsub_state::nsubc_prot_w));
}

void nsub_state::nsubc_io_map(address_map &map)
{
	map.global_mask(0x0f);

	map(0x00, 0x00).mirror(0x0c).portr("IN0");
	map(0x01, 0x01).mirror(0x0c).portr("IN1");
	map(0x02, 0x02).mirror(0x0c).portr("IN2");
	map(0x03, 0x03).mirror(0x0c).portr("IN3");
	map(0x00, 0x0f).w(FUNC(nsub_state::nsubc_io_w));
}


// coinage is handled by extra hardware on a daughterboard, put before the coin-in pin on the main logic board
// IC board "COIN CALCULATOR" (97201-P): two 74191 counters, a 555 timer, coin meters, and lots of other TTL
TIMER_DEVICE_CALLBACK_MEMBER(nsub_state::nsub_coin_pulse)
{
	if (m_nsub_play_counter > 0)
	{
		m_nsub_play_counter--;
		coin_in();
	}
}

INPUT_CHANGED_MEMBER(nsub_state::nsub_coin_in)
{
	if (newval)
	{
		int which = (int)param;
		int coinage = m_coinage->read();

		switch (which)
		{
			// normal coin
			case 0: case 1:
				if (which && ~coinage & 0x40)
				{
					// x credits per coin
					m_nsub_play_counter += (coinage >> 3 & 7);
				}
				else
				{
					// x coins per credit
					if (--m_nsub_coin_counter == 0)
					{
						m_nsub_coin_counter = coinage & 7;
						m_nsub_play_counter++;
					}
				}

				// increment coin counter
				machine().bookkeeping().coin_counter_w(which, 1);
				machine().bookkeeping().coin_counter_w(which, 0);
				break;

			// service coin
			case 2:
				m_nsub_play_counter++;
				break;

			default:
				break;
		}
	}
}


static INPUT_PORTS_START( nsub )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(nsub_state::cblank_comp_r))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(nsub_state::coin_status_r))

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nsub_state::nsub_coin_in), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nsub_state::nsub_coin_in), 1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nsub_state::nsub_coin_in), 2)

	PORT_START("COINAGE") // "OPTION SW." on daughterboard
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 0C_1C ) ) // invalid
	PORT_DIPNAME( 0x78, 0x08, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW:4,5,6,7")
	PORT_DIPSETTING(    0x40, "Shared With Coin A" )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_0C ) ) // invalid
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_7C ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( nsubc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(nsub_state::cblank_comp_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_COIN_DEFAULT

	// TODO: hook this up. The main PCB also has a bank of 6 DIPs, but they don't seem used?
	PORT_START("COINAGE") // "OPTION SW." on daughterboard
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 0C_1C ) ) // invalid
	PORT_DIPNAME( 0x78, 0x08, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW:4,5,6,7")
	PORT_DIPSETTING(    0x40, "Shared With Coin A" )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_0C ) ) // invalid
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_7C ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW:8" )
INPUT_PORTS_END


void nsub_state::machine_start()
{
	m_nsub_play_counter = 0;
	save_item(NAME(m_nsub_coin_counter));
	save_item(NAME(m_nsub_play_counter));

	vicdual_state::machine_start();

	// playcounter 555 timer frequency is unknown
	// keep in mind that intervals need to be longer than the main coin_in timeout
	if (m_nsub_coinage_timer.found())
		m_nsub_coinage_timer->adjust(attotime::zero, 0, attotime::from_msec(150));
}

void nsub_state::machine_reset()
{
	if (m_coinage.found())
		m_nsub_coin_counter = m_coinage->read() & 7;

	vicdual_state::machine_reset();
}

MACHINE_START_MEMBER(nsub_state, nsubc)
{
	nsub_state::machine_start();

	save_item(NAME(m_prot_value));
}

void nsub_state::nsub(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, VICDUAL_MAIN_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nsub_state::nsub_map);
	m_maincpu->set_addrmap(AS_IO, &nsub_state::nsub_io_map);

	TIMER(config, m_coinstate_timer).configure_generic(FUNC(nsub_state::clear_coin_status));
	TIMER(config, m_nsub_coinage_timer).configure_generic(FUNC(nsub_state::nsub_coin_pulse));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VICDUAL_PIXEL_CLOCK, VICDUAL_HTOTAL, VICDUAL_HBEND, VICDUAL_HBSTART, VICDUAL_VTOTAL, VICDUAL_VBEND, VICDUAL_VBSTART);
	m_screen->set_screen_update(FUNC(nsub_state::screen_update_color));

	S97269PB(config, m_s97269pb, 0);

	/* audio hardware */
	S97271P(config, m_s97271p, 0);
}

void nsub_state::nsubc(machine_config &config)
{
	vicdual_root(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &nsub_state::nsubc_map);
	m_maincpu->set_addrmap(AS_IO, &nsub_state::nsubc_io_map);

	m_screen->set_screen_update(FUNC(nsub_state::screen_update_color));

	S97269PB(config, m_s97269pb, 0);

	MCFG_MACHINE_START_OVERRIDE(nsub_state, nsubc)

	/* audio hardware */
	S97271P(config, m_s97271p, 0);
}



/*************************************
 *
 *  Invinco
 *
 *************************************/

uint8_t vicdual_state::invinco_io_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset & 0x01)  ret = m_in0->read();
	if (offset & 0x02)  ret = m_in1->read();
	if (offset & 0x08)  ret = m_in2->read();

	return ret;
}


void vicdual_state::invinco_io_w(offs_t offset, uint8_t data)
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)  invinco_audio_w(data);
	if (offset & 0x04)  palette_bank_w(data);
}


void vicdual_state::invinco_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0xbfff).noprw(); /* unused */
	map(0xc000, 0xc3ff).mirror(0x3000).ram().w(FUNC(vicdual_state::videoram_w)).share("videoram");
	map(0xc400, 0xc7ff).mirror(0x3000).ram();
	map(0xc800, 0xcfff).mirror(0x3000).ram().w(FUNC(vicdual_state::characterram_w)).share("characterram");
}


void vicdual_state::invinco_io_map(address_map &map)
{
	map.global_mask(0x0f);

	/* no decoder, just logic gates, so in theory the
	   game can read/write from multiple locations at once */
	map(0x00, 0x0f).rw(FUNC(vicdual_state::invinco_io_r), FUNC(vicdual_state::invinco_io_w));
}


static INPUT_PORTS_START( invinco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::cblank_comp_r))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vicdual_state::coin_status_r))

	PORT_COIN_DEFAULT
INPUT_PORTS_END


void vicdual_state::invinco(machine_config &config)
{
	vicdual_root(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &vicdual_state::invinco_map);
	m_maincpu->set_addrmap(AS_IO, &vicdual_state::invinco_io_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(vicdual_state::screen_update_color));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	invinco_audio(config);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( depthch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "50a", 0x0000, 0x0400, CRC(56c5ffed) SHA1(f1e6cc322da93615d59850b3225a50f06fe58259) )
	ROM_LOAD( "51a", 0x0400, 0x0400, CRC(695eb81f) SHA1(f2491b8b9ce2dbb6d2606dcfaeb8658671f25400) )
	ROM_LOAD( "52",  0x0800, 0x0400, CRC(aed0ba1b) SHA1(cb7473e6b3c192953ae1832ab444545ddd85babb) )
	ROM_LOAD( "53",  0x0c00, 0x0400, CRC(2ccbd2d0) SHA1(76d8459bbad709666ce0c0be51f1d09e091983a2) )
	ROM_LOAD( "54a", 0x1000, 0x0400, CRC(1b7f6a43) SHA1(08d7864378b012a735eac4968f4dd86e36dc9d8d) )
	ROM_LOAD( "55a", 0x1400, 0x0400, CRC(9fc2eb41) SHA1(95a1684da346709908cd66bec06acfaeead596cf) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( depthcho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "316-0025.u63", 0x0000, 0x0400, CRC(bec75b9c) SHA1(8abe8b63be892e6abb7a886222b9eab40c5fcda0) )
	ROM_LOAD_NIB_HIGH( "316-0022.u51", 0x0000, 0x0400, CRC(977b7889) SHA1(dc1e874c2fd44709117474c5b210d67130ac361f) )
	ROM_LOAD_NIB_LOW ( "316-0030.u89", 0x0400, 0x0400, CRC(9e2bbb45) SHA1(be60d7330a160e15a8a822aa791aed3060b3b1db) )
	ROM_LOAD_NIB_HIGH( "316-0028.u77", 0x0400, 0x0400, CRC(597ae441) SHA1(8d3af5e64e838a57057d46f97a7b1c1037c1a0cf) )
	ROM_LOAD_NIB_LOW ( "316-0026.u64", 0x0800, 0x0400, CRC(61cc0802) SHA1(7173920f38188a1d1637cb2cb48e31cdd03a194e) )
	ROM_LOAD_NIB_HIGH( "316-0023.u52", 0x0800, 0x0400, CRC(9244b613) SHA1(6587035ec22d90194cdc3efaed3571a1ab975e1c) )
	ROM_LOAD_NIB_LOW ( "316-0031.u90", 0x0c00, 0x0400, CRC(861ffed1) SHA1(14e0a6a13726052000477c3586d99486167b8812) )
	ROM_LOAD_NIB_HIGH( "316-0029.u78", 0x0c00, 0x0400, CRC(53178634) SHA1(d8c4b70c3ab5144938ca0989300ad68e48391490) )
	ROM_LOAD_NIB_LOW ( "316-0027.u65", 0x1000, 0x0400, CRC(4eecfc70) SHA1(1a1f6cc5da6df91e9e9016def65184201c3d2672) )
	ROM_LOAD_NIB_HIGH( "316-0024.u53", 0x1000, 0x0400, CRC(a9f55883) SHA1(78bfbc76f84657d32eb1b8072186b403729ea614) )
	ROM_LOAD_NIB_LOW ( "316-0049.u91", 0x1400, 0x0400, CRC(dc7eff35) SHA1(1915e92c09cba5868bd2e73ad395e19ddf47a3de) )
	ROM_LOAD_NIB_HIGH( "316-0048.u79", 0x1400, 0x0400, CRC(6e700621) SHA1(2b8db1cbbaf7808d4bf446435bbbbfc4d7761db8) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0013.u27", 0x0000, 0x0020, CRC(690ef530) SHA1(6c0de3fa87a341cd378fefb8e06bf7918db9a074) )    /* control PROM */
	ROM_LOAD( "316-0014.u28", 0x0020, 0x0020, CRC(7b7a8492) SHA1(6ba8d891cc6eb0dd80051377b6b832e8894655e7) )    /* sequence PROM */
ROM_END

/* Fully working PCB set from my full-size Sub Hunter upright machine.

Essentally this pcb set is a Taito-made license of
'Depthcharge' by Gremlin.

PCB markings are:
- DP070001 / DPN00001 (main)  (paper label DP80)
- DP070002 / DPN00002 (sound) (paper label DP80)
Serial number 430229

12x program Proms were harris 7643
2x gfx proms were Harris 7603 */

ROM_START( subhunt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "dp04.u63",     0x0000, 0x0400, CRC(0ace1aef) SHA1(071256dd63e2e449093a65a4c9b006be5e17b786) )
	ROM_LOAD_NIB_HIGH( "dp01.u51",     0x0000, 0x0400, CRC(da9e835b) SHA1(505c969b479aeab11bb6a21ef06837280846d90a) )
	ROM_LOAD_NIB_LOW ( "dp10.u89",     0x0400, 0x0400, CRC(de752f20) SHA1(513a92554d14a09d6b80ba8017d161c7cda9ed8c) )
	ROM_LOAD_NIB_HIGH( "316-0028.u77", 0x0400, 0x0400, CRC(597ae441) SHA1(8d3af5e64e838a57057d46f97a7b1c1037c1a0cf) ) // dp07.u77
	ROM_LOAD_NIB_LOW ( "dp05.u64",     0x0800, 0x0400, CRC(1c0530cf) SHA1(b1f2b1038ee063533669341f1a71755eecc2e1a9) )
	ROM_LOAD_NIB_HIGH( "316-0023.u52", 0x0800, 0x0400, CRC(9244b613) SHA1(6587035ec22d90194cdc3efaed3571a1ab975e1c) ) // dp02.u52
	ROM_LOAD_NIB_LOW ( "dp11.u90",     0x0c00, 0x0400, CRC(0007044a) SHA1(c8d7c693e3059ff020563336fe712c234e94b8f9) )
	ROM_LOAD_NIB_HIGH( "dp08.u78",     0x0c00, 0x0400, CRC(4d4e3ec8) SHA1(a0d5392fe5795cc6bf7373f194186506283c947c) )
	ROM_LOAD_NIB_LOW ( "dp06.u65",     0x1000, 0x0400, CRC(63e1184b) SHA1(91934cb041365dabdc58a831312577fdb0dc923b) )
	ROM_LOAD_NIB_HIGH( "dp03.u53",     0x1000, 0x0400, CRC(d70dbfd8) SHA1(0183a6b1ffd87a9e28588a7a9aa18aeb003560f0) )
	ROM_LOAD_NIB_LOW ( "dp12.u91",     0x1400, 0x0400, CRC(170d7718) SHA1(4348e4e2dbb1edd9a4228fd3ccef58c50f1ae129) )
	ROM_LOAD_NIB_HIGH( "dp09.u79",     0x1400, 0x0400, CRC(97466803) SHA1(f04ba4a1a960836974a85832596fc3a92a711094) )

	ROM_REGION( 0x0040, "user1", 0 )
	ROM_LOAD( "316-0013.u27", 0x0000, 0x0020, CRC(690ef530) SHA1(6c0de3fa87a341cd378fefb8e06bf7918db9a074) )    /* dp13.u27 - control PROM */
	ROM_LOAD( "316-0014.u28", 0x0020, 0x0020, CRC(7b7a8492) SHA1(6ba8d891cc6eb0dd80051377b6b832e8894655e7) )    /* dp14.u28 - sequence PROM */
ROM_END


ROM_START( safari )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-0066.u48", 0x0000, 0x0400, CRC(2a26b098) SHA1(a16b04110fb142cec01c10460b14ec0c4e8d99af) )
	ROM_LOAD( "316-0065.u47", 0x0400, 0x0400, CRC(b776f7db) SHA1(7332d1b18e1b199d87367182f185abafd9ad0bb1) )
	ROM_LOAD( "316-0064.u46", 0x0800, 0x0400, CRC(19d8c196) SHA1(219dca308a4f917617cfe291580eb23fc2cb4687) )
	ROM_LOAD( "316-0063.u45", 0x0c00, 0x0400, CRC(028bad25) SHA1(94120f197c15705d9447d4615b82e31b61672f89) )
	ROM_LOAD( "316-0062.u44", 0x1000, 0x0400, CRC(504e0575) SHA1(069390941a0d79d623dce816fefef4d52b6e929f) )
	ROM_LOAD( "316-0061.u43", 0x1400, 0x0400, CRC(d4c528e0) SHA1(8b28b70f4cdb12189bb7526d70e4df849a4b9c42) )
	ROM_LOAD( "316-0060.u42", 0x1800, 0x0400, CRC(48c7b0cc) SHA1(26f757927212a01b2682ab520dd3b26a5524bdc3) )
	ROM_LOAD( "316-0059.u41", 0x1c00, 0x0400, CRC(3f7baaff) SHA1(5f935cb2212718226cca10f4bcb28a5fdde109c7) )
	ROM_LOAD( "316-0058.u40", 0x2000, 0x0400, CRC(0d5058f1) SHA1(00fd39a058e206b1bc5669438ab9670fa4db1921) )
	ROM_LOAD( "316-0057.u39", 0x2400, 0x0400, CRC(298e8c41) SHA1(b9b6bc84d2531c85e4529c910d6e97ea83650ce3) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( safaria ) // bootleg board, but possibly a legit alt revision
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hu1.22c",      0x0000, 0x0400, CRC(f27d5961) SHA1(9780e9659746c959206b8700598cfb3925ae7938) )
	ROM_LOAD( "hu2.20c",      0x0400, 0x0400, CRC(11a9cb59) SHA1(5132151a97146d973292b15284e4d58de9ee7cc6) )
	ROM_LOAD( "hu3.19c",      0x0800, 0x0400, CRC(4fe746cb) SHA1(b4b6eac78dd9a76c102994990c219ae832442cc1) )
	ROM_LOAD( "hu4.17c",      0x0c00, 0x0400, CRC(f0bad948) SHA1(5dda4513e96cfb0b6535184c611ac5832afbbfde) )
	ROM_LOAD( "hu5.16c",      0x1000, 0x0400, CRC(d994f98a) SHA1(a51db8b8975f1fa7ca3bae56d4d929b8d3d7bfb7) )
	ROM_LOAD( "hu6.15c",      0x1400, 0x0400, CRC(174b5964) SHA1(dfa88aaa572d4d46ffe3c8f247dbc370e624f5c4) )
	ROM_LOAD( "hu7.13c",      0x1800, 0x0400, CRC(3e94caa1) SHA1(520ae5924b970126c07368bba900b7603997c5cc) )
	ROM_LOAD( "hu8.12c",      0x1c00, 0x0400, CRC(a8a5dca0) SHA1(2424d9e3b4ed2c73b14ec0c26d63453fb4f7f6c2) )
	ROM_LOAD( "hu9.11c",      0x2000, 0x0400, CRC(0ace0939) SHA1(34704b836445628341fb6a77b1ebd47a76c5640d) )
	ROM_LOAD( "hu10.9c",      0x2400, 0x0400, CRC(9dae33ca) SHA1(91472e3b60ff055724ae574b182a450d2a00081c) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "32.21e", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )  /* control PROM */
	ROM_LOAD( "31.22e", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )  /* sequence PROM */
ROM_END


ROM_START( frogs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-119a.u48",     0x0000, 0x0400, CRC(b1d1fce4) SHA1(572015bede39b14526e93919b63b6d01ae38a09a) )
	ROM_LOAD( "316-118a.u47",     0x0400, 0x0400, CRC(12fdcc05) SHA1(06c6d17edec9fb03f46514c1f6c5d8c420ef4d05) )
	ROM_LOAD( "316-117a.u46",     0x0800, 0x0400, CRC(8a5be424) SHA1(ed8a09b4318929b83118f87e2da601028349f2bd) )
	ROM_LOAD( "316-116b.u45",     0x0c00, 0x0400, CRC(09b82619) SHA1(1063e268138b4ff6a8037d8d1a0816c34bbac690) )
	ROM_LOAD( "316-115a.u44",     0x1000, 0x0400, CRC(3d4e4fa8) SHA1(4655c4922328837af410cb298e0c296ae0099591) )
	ROM_LOAD( "316-114a.u43",     0x1400, 0x0400, CRC(04a21853) SHA1(1e84eb84d5770f54925055b748ab9ca2aa72c1cc) )
	ROM_LOAD( "316-113a.u42",     0x1800, 0x0400, CRC(02786692) SHA1(8a8937fd92beecf1119fe3f6b41a700725412aa1) )
	ROM_LOAD( "316-112a.u41",     0x1c00, 0x0400, CRC(0be2a058) SHA1(271f3b60cba422fff7e782fda198c3897c275b46) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

/*

N-Sub by SEGA 1979

97399-P-16 N-SUB UPRIGHT

Label says : U41(U20) ~ U48(U27)
        EPR   EPR
        268   275

and also : PR69

Despite what the label says, here is correct name and position from a real pcb !

Epr-268.u48
Epr-269.u47
Epr-270.u46
Epr-271.u45
Epr-272.u44
Epr-273.u43
Epr-274.u42
Epr-275.u41
Pr-69.u11

This game use a separate "daughter" board for gradient and starfield ref: 97269-P-B
with 3 proms on it : PR-02 type MMI 6336-1j, PR-56 type MB7054 and PR-57 type MB7054

*/

ROM_START( nsub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-268.u48", 0x0000, 0x0800, CRC(485b4704) SHA1(d3989cfe5f8d723bc1a6be185614d138666912d2) )
	ROM_LOAD( "epr-269.u47", 0x0800, 0x0800, CRC(32774ac9) SHA1(0a2c209f627a8d703c02e75c361c363272d1f435) )
	ROM_LOAD( "epr-270.u46", 0x1000, 0x0800, CRC(af7ca40a) SHA1(c0f5732079a51979758f3a159084b84be8b2ad3b) )
	ROM_LOAD( "epr-271.u45", 0x1800, 0x0800, CRC(3f9c180b) SHA1(7438454a348b36d1f5ea59f179f715b827244142) )
	ROM_LOAD( "epr-272.u44", 0x2000, 0x0800, CRC(d818aa51) SHA1(1d3ca550f597c4924b9a805fa955a4a8ff557769) )
	ROM_LOAD( "epr-273.u43", 0x2800, 0x0800, CRC(03a6f12a) SHA1(1eefd4607a718c291b29f1b0a6adf0367840b242) )
	ROM_LOAD( "epr-274.u42", 0x3000, 0x0800, CRC(d69eb098) SHA1(fd3e67d18b5891aa65aab5967d49810c5d88dcee) )
	ROM_LOAD( "epr-275.u41", 0x3800, 0x0800, CRC(1c7d90cc) SHA1(8483825d9811c925407328836ae10f98b011c3dd) )

	ROM_REGION( 0x0020, "proms", ROMREGION_INVERT )
	ROM_LOAD( "pr-69.u11", 0x0000, 0x0020, CRC(c94dd091) SHA1(f88cfb033ff83adb7375652be1fa32ba489d8418) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "pr-33.u82", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "pr-34.u83", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

// Heavily modified Carnival PCB with an extra video output card matching the one on the upright version.
// Had 2x PROMs missing but worked with ones dumped from the existing set.  PCB found in genuine N-Sub cocktail cabinet.
ROM_START( nsubc ) // S-97396-P main board + 97093-P-B-S sub board + 97269-P-B (gradient and starfield) + 97201-P PCB for coinage
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-581.u33", 0x0000, 0x0400, CRC(be5f0f44) SHA1(5e6802c6e15c9857f0a8150bb9b37aabfa0e0245) )
	ROM_LOAD( "epr-582.u32", 0x0400, 0x0400, CRC(97a5e094) SHA1(c5ee9af7488f03627e780a1cf23af5f296520d95) )
	ROM_LOAD( "epr-583.u31", 0x0800, 0x0400, CRC(bcf48a8d) SHA1(6fa96a449bc8440844a1b61c545ebed724c2d71d) )
	ROM_LOAD( "epr-584.u30", 0x0c00, 0x0400, CRC(a0197ffb) SHA1(6bb72105e1c7a3126ada1f1b9eaf026383c203be) )
	ROM_LOAD( "epr-585.u29", 0x1000, 0x0400, CRC(68f64111) SHA1(e7bec469316317560c99798b8b10e6a3cacf5be4) )
	ROM_LOAD( "epr-586.u28", 0x1400, 0x0400, CRC(d7e75bce) SHA1(99726c800d60154efc42795ee9dd195e9b055bd6) )
	ROM_LOAD( "epr-587.u27", 0x1800, 0x0400, CRC(a9f1c390) SHA1(31a197d27919a9db8e1b1a4089918f857a9e9ed6) )
	ROM_LOAD( "epr-588.u26", 0x1c00, 0x0400, CRC(77598bf2) SHA1(df46e6f4dbf165e46c8d85d037828e96d87ad654) )
	ROM_LOAD( "epr-589.u8",  0x2000, 0x0400, CRC(917c5204) SHA1(6633160e37d72f92f2135c9e56cd79e3d0cc6082) )
	ROM_LOAD( "epr-590.u7",  0x2400, 0x0400, CRC(3cc9b52e) SHA1(6f2d7ce39c39b804924bafc09df585bcce250091) )
	ROM_LOAD( "epr-591.u6",  0x2800, 0x0400, CRC(3e0ac65f) SHA1(3ae2f8c36b3d7042e5621125e6cdfcfbc0537dfe) )
	ROM_LOAD( "epr-592.u5",  0x2c00, 0x0400, CRC(7864676c) SHA1(012292ead2f4f9fb16edc56e4ebe99e0cb4cf2d4) )
	ROM_LOAD( "epr-593.u4",  0x3000, 0x0400, CRC(30233912) SHA1(a03a1b6271683a2e00298c30c2fea137403efab0) )
	ROM_LOAD( "epr-594.u3",  0x3400, 0x0400, CRC(e93807cc) SHA1(2a6ad5162e539ed7be5efab1858f1c8119b70737) )
	ROM_LOAD( "epr-595.u2",  0x3800, 0x0400, CRC(01f3f941) SHA1(33991ac68e73ef08d8dffcc5e9319f08dd2b2836) )
	ROM_LOAD( "epr-596.u1",  0x3c00, 0x0400, CRC(ac63b3b5) SHA1(16466899d17a81c77dcb040028e54bfaf9ba9e9d) )

	ROM_REGION( 0x0020, "proms", ROMREGION_INVERT )
	ROM_LOAD( "pr-69.u49", 0x0000, 0x0020, CRC(c94dd091) SHA1(f88cfb033ff83adb7375652be1fa32ba489d8418) )

	ROM_REGION( 0x0040, "user1", 0 )    // timing PROMs, missing on PCB but using the ones from nsub made the PCB work again
	ROM_LOAD( "pr-33.u14", 0x0000, 0x0020, BAD_DUMP CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    // control PROM
	ROM_LOAD( "pr-34.u15", 0x0020, 0x0020, BAD_DUMP CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    // sequence PROM

	ROM_REGION( 0x117, "plds", 0 ) // Protection related? on 97093-P-B-S stickered sub board.
	ROM_LOAD( "pay03.ic4", 0x000, 0x117, CRC(319ffc87) SHA1(07689c28eec19676fdec98f6d488c22754542322) ) // PAL10H8, but converted for GAL16V8
ROM_END

ROM_START( sspaceat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "155.u27",      0x0000, 0x0400, CRC(ba7bb86f) SHA1(030e6f69d3ae00456fc02d1dc0fb915a81689df4) )
	ROM_LOAD( "156.u26",      0x0400, 0x0400, CRC(0b3a491c) SHA1(19cef304bb91f745797f27adbb9d334876d4fb78) )
	ROM_LOAD( "157.u25",      0x0800, 0x0400, CRC(3d3fac3b) SHA1(b22c2517af7c7077032d1b83e4628173d168e3ca) )
	ROM_LOAD( "158.u24",      0x0c00, 0x0400, CRC(843b80f6) SHA1(b61466d3546f1e0759ec84e841664cbe4d2a0a4d) )
	ROM_LOAD( "159.u23",      0x1000, 0x0400, CRC(1eacf60d) SHA1(52d5bfad4357619a9bdc7435e66ed5accadc6401) )
	ROM_LOAD( "160.u22",      0x1400, 0x0400, CRC(e61d482f) SHA1(d41550af1cb244adddff5c151fe2c140591c58ff) )
	ROM_LOAD( "161.u21",      0x1800, 0x0400, CRC(eb5e0993) SHA1(745135f50e50a8516a529a3caff27ee2580227f1) )
	ROM_LOAD( "162.u20",      0x1c00, 0x0400, CRC(5f84d550) SHA1(4fa2c48f843ad49b55598b2757e0e4e1e117aacb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( sspaceat2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "81.u48",       0x0000, 0x0400, CRC(3e4b29f6) SHA1(ec99b7e156bad1f9f900fdebb289f0c9abf08647) )
	ROM_LOAD( "58.u47",       0x0400, 0x0400, CRC(176adb80) SHA1(9798d3b2d59fe4b7d26927b444746f135f0f0d8e) )
	ROM_LOAD( "59.u46",       0x0800, 0x0400, CRC(b2400d05) SHA1(12011fd91bbdfc94b02f9089be54d7cbb8dedece) )
	ROM_LOAD( "150.u45",      0x0c00, 0x0400, CRC(cf9bfa65) SHA1(3521bd2608705a83bd8d3daa0239708d2a8755e3) )
	ROM_LOAD( "151.u44",      0x1000, 0x0400, CRC(064530f1) SHA1(8278b271ae7d67e0b5433aefb150fd743ce6558a) )
	ROM_LOAD( "152.u43",      0x1400, 0x0400, CRC(c65c30fe) SHA1(849a0d46575ad0c72aceef28daa27911ec35181a) )
	ROM_LOAD( "153.u42",      0x1800, 0x0400, CRC(ea70c7f6) SHA1(656d113636224ec9c30982daa6a43877bc6ee58f) )
	ROM_LOAD( "156a.u41",     0x1c00, 0x0400, CRC(9029d2ce) SHA1(57d21650bfe9d76d874661443768213321acc56b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( sspaceat3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-115.u48",   0x0000, 0x0400, CRC(9bc36d80) SHA1(519b3f810b133ac82f066851626b73460956a807) )
	ROM_LOAD( "epr-116.u47",   0x0400, 0x0400, CRC(2c2750b3) SHA1(eab297678e6ee45d6f723d8ff7e6a29086ad4c78) )
	ROM_LOAD( "epr-117.u46",   0x0800, 0x0400, CRC(fa7c2cc0) SHA1(26e4f2c8599d16f1c7ec4bfb0a5a3dc709901045) )
	ROM_LOAD( "epr-118.u45",   0x0c00, 0x0400, CRC(273884ae) SHA1(9efae4acb9ba9bdef0fb58c2a16e0092c6c1a2ba) )
	ROM_LOAD( "epr-119.u44",   0x1000, 0x0400, CRC(1b53c6de) SHA1(7d8f3f5026e7d1a3b78a54c9c1acbb50a4f02c94) )
	ROM_LOAD( "epr-120.u43",   0x1400, 0x0400, CRC(60add585) SHA1(01d78d5cbad680b8ad7eb39f53eefad148d48ee2) )
	ROM_LOAD( "epr-121.u42",   0x1800, 0x0400, CRC(0979f72b) SHA1(244e80552b905df5484bb52100b2e46859fd2cf6) )
	ROM_LOAD( "epr-122.u41",   0x1c00, 0x0400, CRC(45cb3486) SHA1(0e9d5e8dd43643588989354847483283487b9a75) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( sspaceatc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "139.u27",      0x0000, 0x0400, CRC(9f2112fc) SHA1(89c129ef1a95c5934a7c775994aafc91911b0051) )
	ROM_LOAD( "140.u26",      0x0400, 0x0400, CRC(ddbeed35) SHA1(48b33d7b35457675b545ca42c8afd79b86ce6035) )
	ROM_LOAD( "141.u25",      0x0800, 0x0400, CRC(b159924d) SHA1(320bbe156493f30a573ff548398f8f469e261e21) )
	ROM_LOAD( "142.u24",      0x0c00, 0x0400, CRC(f2ebfce9) SHA1(4792bca4a331bc41fd850760e6260e933063398f) )
	ROM_LOAD( "143.u23",      0x1000, 0x0400, CRC(bff34a66) SHA1(8a7490a13b9526c45f3afee1eee59d2b0096105f) )
	ROM_LOAD( "144.u22",      0x1400, 0x0400, CRC(fa062d58) SHA1(76e15e1d29b0e22ab310381d3d9faddf8912b205) )
	ROM_LOAD( "145.u21",      0x1800, 0x0400, CRC(7e950614) SHA1(0fe4de728ca550f0ef904b2d7d84fb2d56648401) )
	ROM_LOAD( "146.u20",      0x1c00, 0x0400, CRC(8ba94fbc) SHA1(371702c2d489fa0f1959734ebd35af45006712fa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( headon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-163a.u27", 0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "316-164a.u26", 0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "316-165a.u25", 0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "316-166c.u24", 0x0c00, 0x0400, CRC(65d12951) SHA1(25fb0da2ea62a2b1ec214ce5c599a183e121b98a) )
	ROM_LOAD( "316-167c.u23", 0x1000, 0x0400, CRC(2280831e) SHA1(128e7f7444440f113b3395dcb333281e0e8bef93) )
	ROM_LOAD( "316-192a.u22", 0x1400, 0x0400, CRC(ed4666f2) SHA1(a12c22bfbb027eab3181627804b69129e89bd22c) )
	ROM_LOAD( "316-193a.u21", 0x1800, 0x0400, CRC(37a1df4c) SHA1(45e1670351f0ef92ef4d9100b0e60ae598df4275) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END


ROM_START( headon1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-163a.u27", 0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "316-164a.u26", 0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "316-165a.u25", 0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "316-166b.u24", 0x0c00, 0x0400, CRC(1c59008a) SHA1(430ecc3c2422d61af35eab77b96a480254572cc6) )
	ROM_LOAD( "316-167a.u23", 0x1000, 0x0400, CRC(069e839e) SHA1(e1ed68573c13c0c88a2bb7b2096860523de952c0) )
	ROM_LOAD( "316-192a.u22", 0x1400, 0x0400, CRC(ed4666f2) SHA1(a12c22bfbb027eab3181627804b69129e89bd22c) )
	ROM_LOAD( "316193a1.u21", 0x1800, 0x0400, CRC(d3782c1d) SHA1(340782374b7015a0aaf98aeb6503b759e199a58a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

/*
Head-On N (Nintendo)

EPROMS were all Intel 2708's.

Bi-polar proms were Signetics 82s123's

prom B6 = lines on screen when removed, address decoding?
prom G2 = white screen when removed, colour prom?
prom F2 = blank screen, audio screeches - no booting...

CPU clock pin 6 = 1933520 hz

PCBs:
THO CPU PI-500890
THO VIDEO PI-500891
*/

ROM_START( headonn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom.e4", 0x0000, 0x0400, CRC(a6cd13fc) SHA1(0fda6b06e7864feae8e4e94cf17f50c9c033aa39) )
	ROM_LOAD( "rom.f4", 0x0400, 0x0400, CRC(d1cd498f) SHA1(ded1a97c7f76af195b247135469b84c25d4096c4) )
	ROM_LOAD( "rom.g4", 0x0800, 0x0400, CRC(0fb02db2) SHA1(6e9ad34a205838339a76ba6eeaf02cf663c2b750) )
	ROM_LOAD( "rom.h4", 0x0c00, 0x0400, CRC(38db2d02) SHA1(a86c4208154f65a83f55c1fbfd041b56ebd11106) )
	ROM_LOAD( "rom.i4", 0x1000, 0x0400, CRC(a04d8522) SHA1(58d6c3168ac7227e78ff699449008dbf999ea794) )
	ROM_LOAD( "rom.j4", 0x1400, 0x0400, CRC(52bd2151) SHA1(4f6bc10dabadf65f638aed46b0f0a1ba00e83afd) )
	ROM_LOAD( "rom.k4", 0x1800, 0x0400, CRC(9488a8b3) SHA1(a7e8bc341c839eba3b84af78f19cefafe91d42e3) )
	ROM_LOAD( "rom.l4", 0x1c00, 0x0400, CRC(a37f0be0) SHA1(4548cf05256764588feeade29368f5abfbf75a7d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.g2", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "prom.b6", 0x0000, 0x0020, CRC(7e1cb76b) SHA1(3366ead65d49cab076ccdafbb13726c7e05f8b9a) )    /* control PROM */
	ROM_LOAD( "prom.f2", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( headonmz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.bin",      0x0000, 0x0400, CRC(1febc85a) SHA1(7cc422d6819d5a2507467bdf91f82e76b0d12643) ) // this ROM was loose from the rest, but should be correct..
	ROM_LOAD( "1.bin",      0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "2.bin",      0x0800, 0x0400, CRC(a5d0e0f5) SHA1(025a64a9bd95ceef93009676a0679008c186223b) )
	ROM_LOAD( "3.bin",      0x0c00, 0x0400, CRC(721f3b03) SHA1(69255d9fd3628c3ab46856aaad3d2203d487a983) )
	ROM_LOAD( "4.bin",      0x1000, 0x0400, CRC(82c73635) SHA1(210f6868a4b63340d01ad660b202338b9638e422) )
	ROM_LOAD( "5.bin",      0x1400, 0x0400, CRC(17c04c3a) SHA1(819692f37a25a7fe73cd62d781eaf5432f5d5b8a) )
	ROM_LOAD( "6.bin",      0x1800, 0x0400, CRC(88e43434) SHA1(b2550f98df3b4a6c2fd6c5621e289367587352f6) )

	// assuming to be the same
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )
	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "10303.3e", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "10302.2e", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

/*
Head On (Sidam) Notes

Board made by Sidam, but no Sidam copyright notice

---CPU:

1x Z80CPU (main)
1x oscillator 15.468MHz

---ROMs:

7x F2708
2x N82S123N

---Note:

1x 22x2 edge connector
1x trimmer (volume)
*/

ROM_START( headons )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.1a",         0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "1.3a",         0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "2.4a",         0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "3.6a",         0x0c00, 0x0400, CRC(461c2658) SHA1(561ef24a20fb2cc3c05d836c06026069400be085) )
	ROM_LOAD( "4.8a",         0x1000, 0x0400, CRC(79fc7f31) SHA1(835fbaa2bac8b955bc8fe5e932705c67e10308ac) )
	ROM_LOAD( "5.9a",         0x1400, 0x0400, CRC(ed4666f2) SHA1(a12c22bfbb027eab3181627804b69129e89bd22c) )
	ROM_LOAD( "6.11a",        0x1800, 0x0400, CRC(7a709d68) SHA1(c1f0178c7a8cb39948e52e91a841401cfd932271) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

/* this one is the same PCB but does show the Sidam copyright */
ROM_START( headonsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10305.0.9a",      0x0000, 0x0400, CRC(9a37407b) SHA1(3cd3dbd13c76d01b7541307de92f69d6779046f5) )
	ROM_LOAD( "10305.1.8a",      0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "10305.2.7a",      0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "10305.3.6a",      0x0c00, 0x0400, CRC(ae33fcc4) SHA1(7e0a27f1f502c5293f294875b49186e800a2c749) )
	ROM_LOAD( "10305.4.5a",      0x1000, 0x0400, CRC(e87f6fd8) SHA1(7fc1ade66c6783861ab310790f023b02a8db7e08) )
	ROM_LOAD( "10305.5.4a",      0x1400, 0x0400, CRC(387e2eba) SHA1(9feca874e795710884d17ca5122280c30c6b6af0) )
	ROM_LOAD( "10305.6b.3a",     0x1800, 0x0400, CRC(18749071) SHA1(6badb5cf6f6017d884492e9ef16195f1112d23b5) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "10303.3e", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "10302.2e", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END


ROM_START( headon2s )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10304.0.9a",      0x0000, 0x0400, CRC(256a1fc8) SHA1(2ff621d7160e1420fd2cd9ad62d134e22b1650b3) )
	ROM_LOAD( "10304.1.8a",      0x0400, 0x0400, CRC(61c47b15) SHA1(47619bd51fcaf47dd72e940c474f310c9287f2f4) )
	ROM_LOAD( "10304.2.7a",      0x0800, 0x0400, CRC(a6c268d4) SHA1(3b6c27f700ea4474f5354bbdcce82883c2e7e6e9) )
	ROM_LOAD( "10304.3.6a",      0x0c00, 0x0400, CRC(17a09f24) SHA1(0cb40ec185f2ee3a26e943d84e8e2834d5f9d3ed) )
	ROM_LOAD( "10304.4.5a",      0x1000, 0x0400, CRC(9af8a2e0) SHA1(92f45bc593fabf7a30615820b4b91677071bc67e) )
	ROM_LOAD( "10304.5.4a",      0x1400, 0x0400, CRC(6975286c) SHA1(bcb5af18a991b9898fe28e69575c89c7b02d762d) )
	ROM_LOAD( "10304.6.3a",      0x1800, 0x0400, CRC(06fbcdce) SHA1(821b501dbf59c45d5e03afa3c786fca727da9cd6) )
	ROM_LOAD( "10304.7b.2a",     0x1c00, 0x0400, CRC(3588fc8f) SHA1(4529b79a1b654591ee2e879922a5377edc1faee5) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "10303.3e", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "10302.2e", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END


/*
Super Crash notes

---CPU:

1x MK3880N-Z80CPU
1x oscillator 14MHz

---ROMs:

3x TMS2716JL
1x FA2708
2x MMI6331

---Note:

1x 22x2 edge connector
1x 4 switches dip
1x trimmer (volume)
*/

ROM_START( supcrash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-2-scrash.bin", 0x0000, 0x0800, CRC(789a8b73) SHA1(ce0b844729fc4d46ddc82635c8d5a49aa88a3797) )
	ROM_LOAD( "3-4-scrash.bin", 0x0800, 0x0800, CRC(7a310527) SHA1(384c7ddc8da4282b705ad387ae3946a30f0fd05b) )
	ROM_LOAD( "5-6-scrash.bin", 0x1000, 0x0800, CRC(62d33c09) SHA1(ade49f417380f64212491f6be16de39c0c00a364) )
	ROM_LOAD( "7-8-scrash.bin", 0x1800, 0x0400, CRC(0f8ea335) SHA1(cf2d6cd54dbf689bc0f23aa908bffb0766e8bbd3) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87",   0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )  /* control PROM */
	ROM_LOAD( "316-0042.u88",   0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )  /* sequence PROM */
ROM_END

/*
This PCB is a bootleg of Sidam's Head On bootleg manufactured in Torino (Italy) by Fraber.
*/

ROM_START( hocrash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-0s.0s",      0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "2-0r.0r",      0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "3-0p.0p",      0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "4-0m.0m",      0x0c00, 0x0400, CRC(fd67208d) SHA1(539b0db174aef66ac7d8137e4eca4e3237bc7a82) )
	ROM_LOAD( "5-0l.0l",      0x1000, 0x0400, CRC(069e839e) SHA1(e1ed68573c13c0c88a2bb7b2096860523de952c0) )
	ROM_LOAD( "6-0k.0k",      0x1400, 0x0400, CRC(11960190) SHA1(f3908fece95b7e5468ae4bba5a9f2d2482ed6656) )
	ROM_LOAD( "7-0j.0j",      0x1800, 0x0400, CRC(d3782c1d) SHA1(340782374b7015a0aaf98aeb6503b759e199a58a) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

// Head On bootleg by Niemer. Main xtal is 15.468.40 MHz
ROM_START( bumba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.a1",  0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "1.a3",  0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "2.a5",  0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "3.a6",  0x0c00, 0x0400, CRC(461c2658) SHA1(561ef24a20fb2cc3c05d836c06026069400be085) )
	ROM_LOAD( "4.a8",  0x1000, 0x0400, CRC(c7c13dd1) SHA1(ee27b72f16c8dcdbe397e041e7728ea6176c21ce) )
	ROM_LOAD( "5.a9",  0x1400, 0x0400, CRC(ed4666f2) SHA1(a12c22bfbb027eab3181627804b69129e89bd22c) )
	ROM_LOAD( "6.a11", 0x1800, 0x0400, CRC(95fb39df) SHA1(a008227041bf2e884bdc88f6c2f5cd23cac8f4d4) )

	// No PROMs on this PCB
ROM_END

ROM_START( headon2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u27.bin",      0x0000, 0x0400, CRC(fa47d2fb) SHA1(b3208f5bce228c453bdafbc9c1f2c8e1bd920d32) )
	ROM_LOAD( "u26.bin",      0x0400, 0x0400, CRC(61c47b15) SHA1(47619bd51fcaf47dd72e940c474f310c9287f2f4) )
	ROM_LOAD( "u25.bin",      0x0800, 0x0400, CRC(bb16db92) SHA1(f57dfbe52b0e545c7c889ac846dc7281d28f2698) )
	ROM_LOAD( "u24.bin",      0x0c00, 0x0400, CRC(17a09f24) SHA1(0cb40ec185f2ee3a26e943d84e8e2834d5f9d3ed) )
	ROM_LOAD( "u23.bin",      0x1000, 0x0400, CRC(0024895e) SHA1(60f81c383f1541555c26f7cf111a12a34f7f4f3e) )
	ROM_LOAD( "u22.bin",      0x1400, 0x0400, CRC(f798304d) SHA1(55526c6daead9b74a88e8bc0311155aa41a93210) )
	ROM_LOAD( "u21.bin",      0x1800, 0x0400, CRC(4c19dd40) SHA1(0bdfed47594c7aa5ff655b507350fc6a912b6855) )
	ROM_LOAD( "u20.bin",      0x1c00, 0x0400, CRC(25887ff2) SHA1(67cfb5ac93902b4c603f02c876c021ff453e5f0e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u65", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END


/*
    Car 2 (Headon 2)

    1x Z80
    1x TDA2002 (sound)
    2x NE556A (sound)
    1x oscillator 15468.48
*/

ROM_START( car2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "car2.0",      0x0000, 0x0400, CRC(37e031f9) SHA1(b8aee0db50507410e5be70d3a8574b7e9dd4a959) )
	ROM_LOAD( "car2.1",      0x0400, 0x0400, CRC(61c47b15) SHA1(47619bd51fcaf47dd72e940c474f310c9287f2f4) )
	ROM_LOAD( "car2.2",      0x0800, 0x0400, CRC(a6c268d4) SHA1(3b6c27f700ea4474f5354bbdcce82883c2e7e6e9) )
	ROM_LOAD( "car2.3",      0x0c00, 0x0400, CRC(17a09f24) SHA1(0cb40ec185f2ee3a26e943d84e8e2834d5f9d3ed) )
	ROM_LOAD( "car2.4",      0x1000, 0x0400, CRC(9af8a2e0) SHA1(92f45bc593fabf7a30615820b4b91677071bc67e) )
	ROM_LOAD( "car2.5",      0x1400, 0x0400, CRC(6975286c) SHA1(bcb5af18a991b9898fe28e69575c89c7b02d762d) )
	ROM_LOAD( "car2.6",      0x1800, 0x0400, CRC(4c19dd40) SHA1(0bdfed47594c7aa5ff655b507350fc6a912b6855) )
	ROM_LOAD( "car2.7",      0x1c00, 0x0400, CRC(41a93920) SHA1(e63df556f998b5e5d99d69a9fd200aaf0403f3f7) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u65", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( invho2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "271b.u33",     0x0000, 0x0400, CRC(44356a73) SHA1(6ff1050d84b6b7a006762c35e0b3d2befb0f90d6) )
	ROM_LOAD( "272b.u32",     0x0400, 0x0400, CRC(bd251265) SHA1(134f081c62173fab80b46918e4a073cf5f72df77) )
	ROM_LOAD( "273b.u31",     0x0800, 0x0400, CRC(2fc80cd9) SHA1(2790fb45233c8d6e74a56fcdfde1c468926d44d2) )
	ROM_LOAD( "274b.u30",     0x0c00, 0x0400, CRC(4fac4210) SHA1(9bb2fe888edab7e52a180d7f4d7fdab17392a736) )
	ROM_LOAD( "275b.u29",     0x1000, 0x0400, CRC(85af508e) SHA1(4b71c9583fe3b16a24f05d685d1edbd53ff81f81) )
	ROM_LOAD( "276b.u28",     0x1400, 0x0400, CRC(e305843a) SHA1(bb5113d3e0a4ca81e055da9c03755d0e6270d927) )
	ROM_LOAD( "277b.u27",     0x1800, 0x0400, CRC(b6b4221e) SHA1(8cfeff5ca7d29d973409df7f422428411462eab6) )
	ROM_LOAD( "278b.u26",     0x1c00, 0x0400, CRC(74d42250) SHA1(023227d314ec91c9b508b7fd60f163414165c25b) )
	ROM_LOAD( "279b.u8",      0x2000, 0x0400, CRC(8d30a3e0) SHA1(ac8cfca1b334d95e209bcfceeeeca31c03faecc8) )
	ROM_LOAD( "280b.u7",      0x2400, 0x0400, CRC(b5ee60ec) SHA1(dbc682b5770755fed8c04ef0f0311b2850228236) )
	ROM_LOAD( "281b.u6",      0x2800, 0x0400, CRC(21a6d4f2) SHA1(e8c8b263ffe53d5af50a561d038bfa96136767ad) )
	ROM_LOAD( "282b.u5",      0x2c00, 0x0400, CRC(07d54f8a) SHA1(f99af42ec24938cdd19c0ff7ac2b9e9882dc3655) )
	ROM_LOAD( "283b.u4",      0x3000, 0x0400, CRC(bdbe7ec1) SHA1(45e08c533acc538d88a0580535325b9ff1a60f2f) )
	ROM_LOAD( "284b.u3",      0x3400, 0x0400, CRC(ae9e9f16) SHA1(7f708ce49f34582d53abad0d811265dd28af899f) )
	ROM_LOAD( "285b.u2",      0x3800, 0x0400, CRC(8dc3ec34) SHA1(1f70027ad3b781244b672ee77225e32589c61e46) )
	ROM_LOAD( "286b.u1",      0x3c00, 0x0400, CRC(4bab9ba2) SHA1(a74881acf222466deb9c9e35dff532af6b10a7fc) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0287.u49", 0x0000, 0x0020, CRC(d4374b01) SHA1(85ea0915f23571358e2e0c2b66b968e7b93f4bd6) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( invho2a ) // found on a Gremlin 'EXTENDED ROM VIDEO LOGIC ASSY NO 800-003' PCB, with a 'S-96674-P-BK' sticker and 'rev. D' handwritten
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "326.u33", 0x0000, 0x0400, CRC(35bc6216) SHA1(92a98694807009c41335c2b039a06205102daf44) )
	ROM_LOAD( "327.u32", 0x0400, 0x0400, CRC(9484a3db) SHA1(73908de15197bfcdbadfd6cadaa8b01863b60e18) )
	ROM_LOAD( "328.u31", 0x0800, 0x0400, CRC(000cf180) SHA1(685c4bec01a34223cfb68917efdd8873d8d5bac6) )
	ROM_LOAD( "329.u30", 0x0c00, 0x0400, CRC(12ce15c4) SHA1(56aed8127e02f0ee7ee7a2b617624c8fb968fe7f) )
	ROM_LOAD( "330.u29", 0x1000, 0x0400, CRC(2f02f721) SHA1(9124eb2eed0cbab02a3f3b5c811da0ef4b1d1a48) )
	ROM_LOAD( "331.u28", 0x1400, 0x0400, CRC(38f4ab51) SHA1(588c9adbe2cb3c1c287b1f090383ba544b9109fa) )
	ROM_LOAD( "332.u27", 0x1800, 0x0400, CRC(94b1cacb) SHA1(005e40957926624d8d60cdd96f27c2cfbcd07d6e) )
	ROM_LOAD( "333.u26", 0x1c00, 0x0400, CRC(455a8f42) SHA1(4609839ff502c128e5692fa6c1ce31e6d32b79eb) )
	ROM_LOAD( "334.u8",  0x2000, 0x0400, CRC(9c578f37) SHA1(88b97295d0375ada227ade8b9ec5f70086410927) )
	ROM_LOAD( "335.u7",  0x2400, 0x0400, CRC(56df9e11) SHA1(6e02f0de8621244282504526c99a0e2f5aedf5ed) )
	ROM_LOAD( "336.u6",  0x2800, 0x0400, CRC(219b2513) SHA1(c5f92c266d6cd82d36961aefbbf8b4d522824e82) )
	ROM_LOAD( "337.u5",  0x2c00, 0x0400, CRC(d6410227) SHA1(bcdace3d0759df5e6f7dd10a9541db551b129399) )
	ROM_LOAD( "338.u4",  0x3000, 0x0400, CRC(e4550755) SHA1(f5874cb0d0a8e950de555762cc5bb6697e04888f) )
	ROM_LOAD( "339.u3",  0x3400, 0x0400, CRC(426dd538) SHA1(cf591b380895478d54018cfbe5fb41c24146c2ea) )
	ROM_LOAD( "340.u2",  0x3800, 0x0400, CRC(aec47500) SHA1(4500a63967b2b2c52d41cbde9e5e6fe3d39322c2) )
	ROM_LOAD( "341.u1",  0x3c00, 0x0400, CRC(0b7a0607) SHA1(bdb58da071d9b7d2796b3f244c6a95ce1c9f2833) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-16.u49", 0x0000, 0x0020, CRC(733eac6f) SHA1(3cf488155992977875a76f8aa0ce73c1faf8a1a4) ) // TODO: multiple dump attempts give consistent results, but with this Head On 2 is completely black, while Invinco shows unexpected colors

	ROM_REGION( 0x0040, "user1", 0 )    // timing PROMs
	ROM_LOAD( "pr-34.u14", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) ) // == 316-0043 - control PROM
	ROM_LOAD( "pr-33.u15", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) ) // == 316-0042 - sequence PROM
ROM_END

ROM_START( sspacaho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-0001.bin", 0x0000, 0x0800, CRC(ba62f57a) SHA1(7cfc079c6afe317b6c389c06802fdf1f83858510) )
	ROM_LOAD( "epr-0002.bin", 0x0800, 0x0800, CRC(94b3c59c) SHA1(e6ee1c25fb45d03d514421c231d794f9da05f47f) )
	ROM_LOAD( "epr-0003.bin", 0x1000, 0x0800, CRC(df13aef2) SHA1(61d210eeb59fe132e14fdd7eb6a39ebc55168097) )
	ROM_LOAD( "epr-0004.bin", 0x1800, 0x0800, CRC(8431e15e) SHA1(b028e718ee90f37c848e5f83494be61cb90338e2) )
	ROM_LOAD( "epr-0005.bin", 0x2000, 0x0800, CRC(eec2b6e7) SHA1(4ed830755b4d1da6111afc6c16c7c633521ccb9c) )
	ROM_LOAD( "epr-0006.bin", 0x2800, 0x0800, CRC(780e47ed) SHA1(533879b8abf0e69644fd8b784dbe9bf10cde6d9f) )
	ROM_LOAD( "epr-0007.bin", 0x3000, 0x0800, CRC(8189a2fa) SHA1(3c13a394f48adb8f7c6b8203bd0749921461ea06) )
	ROM_LOAD( "epr-0008.bin", 0x3800, 0x0800, CRC(34a64a80) SHA1(a588fae0ecaa80677887d8c95ef8896a4bdd77ee) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

// Found on a Gremlin 'EXTENDED ROM VIDEO LOGIC ASSY NO 800-003' PCB with a '834-0118' sticker and a Sega 96718-P-A riser board.
// Everything on the board is dated 1979, but possibly the PCB was converted from something else, given the epr numbers are way bigger than the Japanese version ones
ROM_START( samurai )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-1217.u33",   0x0000, 0x0400, CRC(a1a9cb03) SHA1(1875a86ad5938295dd5db6bb045be46eba8638ba) )
	ROM_LOAD( "epr-1218.u32",   0x0400, 0x0400, CRC(4b45d07d) SHA1(ff8cb6bae5ed239a0245265ca36305e4fadc7695) )
	ROM_LOAD( "epr-1219.u31",   0x0800, 0x0400, CRC(9fd4b195) SHA1(001e3952927dac36993b20f6deae343fdb95e2cd) )
	ROM_LOAD( "epr-1220.u30",   0x0c00, 0x0400, CRC(90370e13) SHA1(6a7419a52299a78aff16689775ce0533ac8aa443) )
	ROM_LOAD( "epr-1221.u29",   0x1000, 0x0400, CRC(dcc47158) SHA1(801a16aa100fee3f13b4acd426612dde9c906f5d) )
	ROM_LOAD( "epr-1222.u28",   0x1400, 0x0400, CRC(d2fab27a) SHA1(d289f0451859d87665f8047aabb43494a22fae28) )
	ROM_LOAD( "epr-1223.u27",   0x1800, 0x0400, CRC(f7e2ad95) SHA1(078055961fc09ee91734e86b30a7b48a03a67f4a) )
	ROM_LOAD( "epr-1224.u26",   0x1c00, 0x0400, CRC(d46e306b) SHA1(51532a054fc334a04d3e5cad3b41aa3508318bc5) )
	ROM_LOAD( "epr-1225.u8",    0x2000, 0x0400, CRC(3dd5c41f) SHA1(de400d30b732edd94ad5388e82cdae27d92f0e7f) )
	ROM_LOAD( "epr-1226.u7",    0x2400, 0x0400, CRC(7c3561b1) SHA1(9f14924dbd753f9389b0516eb5af6241e897e9a1) )
	ROM_LOAD( "epr-1227.u6",    0x2800, 0x0400, CRC(e72c71a4) SHA1(52b6ea96897f2b5b1ee5aab7c1737e35d986b75e) )
	ROM_LOAD( "epr-1228.u5",    0x2c00, 0x0400, CRC(d76f4a56) SHA1(77746f6e73c48fa5ca27a097e9b13a36363f3aa8) )
	ROM_LOAD( "epr-1229.u4",    0x3000, 0x0400, CRC(e0d40395) SHA1(343addf9148c26c3f3e7fbf3005ad6c9b33e3899) )
	ROM_LOAD( "epr-1230.u3",    0x3400, 0x0400, CRC(55e9a5c4) SHA1(022430a842ec2a38a64d9ede53dc1e1ac0c66efd) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr55.clr",     0x0000, 0x0020, CRC(975f5fb0) SHA1(d5917d68ad5549fe5cc997521c3b0a5a279d2231) )
ROM_END

ROM_START( samuraij )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-289.u33",   0x0000, 0x0400, CRC(a1a9cb03) SHA1(1875a86ad5938295dd5db6bb045be46eba8638ba) )
	ROM_LOAD( "epr-290.u32",   0x0400, 0x0400, CRC(49fede51) SHA1(58ab1779d555281ec436ae90dcdf4ada42625892) )
	ROM_LOAD( "epr-291.u31",   0x0800, 0x0400, CRC(6503dd72) SHA1(e0a3f42418a13f38314ec6e7951cd45c686fecbc) )
	ROM_LOAD( "epr-292.u30",   0x0c00, 0x0400, CRC(179c224f) SHA1(0a202718a9c2f5f4f1553d1dccd99bebb511363f) )
	ROM_LOAD( "epr-366.u29",   0x1000, 0x0400, CRC(3df2abec) SHA1(f52182c93026de0f5e7a3c36fe4a35e386c95d0c) )
	ROM_LOAD( "epr-355.u28",   0x1400, 0x0400, CRC(b24517a4) SHA1(51d613ac21e33c70705f1731b905af72dc561dbf) )
	ROM_LOAD( "epr-367.u27",   0x1800, 0x0400, CRC(992a6e5a) SHA1(45ae4bf297d7ce52d879d3af5e26960c0dd5034c) )
	ROM_LOAD( "epr-368.u26",   0x1c00, 0x0400, CRC(403c72ce) SHA1(eaa7a56a28b393db97fd6f1a62b7592cd47060f0) )
	ROM_LOAD( "epr-369.u8",    0x2000, 0x0400, CRC(3cfd115b) SHA1(58d1d34605b75f0078eb7e61eca8d5897a8b8294) )
	ROM_LOAD( "epr-370.u7",    0x2400, 0x0400, CRC(2c30db12) SHA1(9dcf8aa5cfdda5b350fc6b70524a15f970d98d91) )
	ROM_LOAD( "epr-299.u6",    0x2800, 0x0400, CRC(87c71139) SHA1(10c273c2f6a58bba8b5891dfd851e18898b45fd1) )
	ROM_LOAD( "epr-371.u5",    0x2c00, 0x0400, CRC(761f56cf) SHA1(0d63e8b0ac7bfe9f3cd08cea68eb941b1a5a536c) )
	ROM_LOAD( "epr-301.u4",    0x3000, 0x0400, CRC(23de1ff7) SHA1(51bf437a62ee770918524c8d0e8b0e007a800021) )
	ROM_LOAD( "epr-372.u3",    0x3400, 0x0400, CRC(292cfd89) SHA1(a8131f03e8e9f5009508813445bbea559bc27726) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr55.clr",     0x0000, 0x0020, CRC(975f5fb0) SHA1(d5917d68ad5549fe5cc997521c3b0a5a279d2231) )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( invinco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "310a.u27",     0x0000, 0x0400, CRC(e3931365) SHA1(e34083004515ad45ddbf9ab89c34473b6c5d46fb) )
	ROM_LOAD( "311a.u26",     0x0400, 0x0400, CRC(de1a6c4a) SHA1(ca7ab7b4c77319f7923d56ad8b60d16211af19bc) )
	ROM_LOAD( "312a.u25",     0x0800, 0x0400, CRC(e3c08f39) SHA1(13c177980722559ec885565d3f889b830322f25e) )
	ROM_LOAD( "313a.u24",     0x0c00, 0x0400, CRC(b680b306) SHA1(b03a5621bdf6a0bdc79af9361a2c0cc339b50b4b) )
	ROM_LOAD( "314a.u23",     0x1000, 0x0400, CRC(790f07d9) SHA1(222e1c392c92fc547a67d96410f9a4277f3bb6cb) )
	ROM_LOAD( "315a.u22",     0x1400, 0x0400, CRC(0d13bed2) SHA1(c23d937acad2171aae8f76671843a841f62d147f) )
	ROM_LOAD( "316a.u21",     0x1800, 0x0400, CRC(88d7eab8) SHA1(272b099414b68a428f9538f0219f92569525b87c) )
	ROM_LOAD( "317a.u20",     0x1c00, 0x0400, CRC(75389463) SHA1(277ba127f322caf2486735b82bf7b96bb9e34a56) )
	ROM_LOAD( "318a.uxx",     0x2000, 0x0400, CRC(0780721d) SHA1(580aefb382702babdf8248e36330c4b22e8579c8) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0246.u44", 0x0000, 0x0020, CRC(fe4406cb) SHA1(92e2459420a7f7412f02cfaf68604fc233b0a245) )    /* color PROM */
ROM_END

ROM_START( invcarht ) // found on a Gremlin 'DUAL GAME VIC ASSY NO 800-0058' PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "411.u33", 0x0000, 0x0400, CRC(efefba5f) SHA1(9b17a56ffaf178e734f9ca7927174556d469f629) )
	ROM_CONTINUE(0x4000, 0x400)
	ROM_LOAD( "412.u32", 0x0400, 0x0400, CRC(8fe401e2) SHA1(5e9ed75ab7ad91ca095e5d92a697933ff7c93eca) )
	ROM_LOAD( "413.u31", 0x0800, 0x0400, CRC(61ba1046) SHA1(0b4d83e40706df428d0bde1d739e44fd8eac6179) )
	ROM_LOAD( "414.u30", 0x0c00, 0x0400, CRC(4a521dbb) SHA1(1c105810900b4e036d4ab665e43451da4acdeed7) )
	ROM_LOAD( "415.u29", 0x1000, 0x0400, CRC(ce12b71c) SHA1(26c34a034ab4ba4e0ba83b4ea0535d8e112c5e16) )
	ROM_LOAD( "416.u28", 0x1400, 0x0400, CRC(6899d59c) SHA1(5234ddd6d38ad3d8fe9d2468e05049d4b242ce0f) )
	ROM_LOAD( "417.u27", 0x1800, 0x0400, CRC(26cef14e) SHA1(b819bebcd64464b5898b6787d312b6f1eb87b08b) )
	ROM_LOAD( "418.u26", 0x1c00, 0x0400, CRC(02b1f507) SHA1(649171358a0653fcafc8e341367be61f1495d0a1) )
	ROM_LOAD( "419.u8",  0x2000, 0x0400, CRC(42385c4d) SHA1(c85e9435d3f1bcd7432757a897e236f713db6e0b) )
	ROM_LOAD( "420.u7",  0x2400, 0x0400, CRC(ee83d873) SHA1(e3c649165b5496d1d3788cbd8d44052bbc2c0ec2) )
	ROM_LOAD( "421.u6",  0x2800, 0x0400, CRC(2faa2e76) SHA1(63929a8be150be1be88e54c18c7b79d4d24fbf5f) )
	ROM_LOAD( "422.u5",  0x2c00, 0x0400, CRC(f8e5dc61) SHA1(c51d3195b93f921ba39e9bf566aab5d4ca5dfc97) )
	ROM_LOAD( "423.u4",  0x3000, 0x0400, CRC(d783eb72) SHA1(8086d183f4486005e8b2db54b2d219a606c62766) )
	ROM_LOAD( "424.u3",  0x3400, 0x0400, CRC(8fd4f3d4) SHA1(1c6efa2ff1863ca08c170232cdcc77db3836442a) )
	ROM_LOAD( "425.u2",  0x3800, 0x0400, CRC(b0552cd4) SHA1(7f2b930ad97731124f3b7128bba7dfed7d50dd9d) )
	ROM_LOAD( "426.u1",  0x3c00, 0x0400, CRC(b611061b) SHA1(70b400aece52ca73f0b88d6d20576aee6a549beb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0389.u49", 0x0000, 0x0020, CRC(95cfe0d2) SHA1(baee94207d361e35aa6cb2dc7c003444f039e5b4) )    // color PROM

	ROM_REGION( 0x0020, "user1", 0 )    // timing PROM
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    // control PROM
ROM_END

ROM_START( invds )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "367.u33",      0x0000, 0x0400, CRC(e6a33eae) SHA1(16de70e8fcd093964a448a86bc89b1c607152ead) )
	ROM_LOAD( "368.u32",      0x0400, 0x0400, CRC(421554a8) SHA1(efb6759998e36322258c172aa7e8ba6416b3235f) )
	ROM_LOAD( "369.u31",      0x0800, 0x0400, CRC(531e917a) SHA1(f1d48e18f51de36d01bafab5bfa68d16a8c0192b) )
	ROM_LOAD( "370.u30",      0x0c00, 0x0400, CRC(2ad68f8c) SHA1(0ceee2d04e239856ecf18f93b5ee75d72e031f92) )
	ROM_LOAD( "371.u29",      0x1000, 0x0400, CRC(1b98dc5c) SHA1(ac8a1226405cab9d6049304163d658c5ae611c1a) )
	ROM_LOAD( "372.u28",      0x1400, 0x0400, CRC(3a72190a) SHA1(f2852b3ca5fce274ed5452d28d06a3a27fe8a94f) )
	ROM_LOAD( "373.u27",      0x1800, 0x0400, CRC(3d361520) SHA1(bf8441cd5050f643535229b4761310f7dc8a2997) )
	ROM_LOAD( "374.u26",      0x1c00, 0x0400, CRC(e606e7d9) SHA1(66efa2beda9fcc2f3cd8fc77c99c196de30d1a30) )
	ROM_LOAD( "375.u8",       0x2000, 0x0400, CRC(adbe8d32) SHA1(5c883f0053c4f8124b81c49847b00301edb9f654) )
	ROM_LOAD( "376.u7",       0x2400, 0x0400, CRC(79409a46) SHA1(12772cf3e06d5127adb16396c4589cd9a06cbce5) )
	ROM_LOAD( "377.u6",       0x2800, 0x0400, CRC(3f021a71) SHA1(ba4a3833c6f7d28c3033de77882ff7ee0b96b190) )
	ROM_LOAD( "378.u5",       0x2c00, 0x0400, CRC(49a542b0) SHA1(5aa90df413ac0498cadaff9ca805896066e0f8a8) )
	ROM_LOAD( "379.u4",       0x3000, 0x0400, CRC(ee140e49) SHA1(5c29e34afdf91b68554e678699179802616c1f94) )
	ROM_LOAD( "380.u3",       0x3400, 0x0400, CRC(688ba831) SHA1(24f977c99714a08bf14c7753d42e2744aa86c396) )
	ROM_LOAD( "381.u2",       0x3800, 0x0400, CRC(798ba0c7) SHA1(7867d107f95077c539263619155c7f3ce4ef0968) )
	ROM_LOAD( "382.u1",       0x3c00, 0x0400, CRC(8d195c24) SHA1(5c314947ba13112b4154d3be069892cca4f5da42) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0246.u44", 0x0000, 0x0020, CRC(fe4406cb) SHA1(92e2459420a7f7412f02cfaf68604fc233b0a245) )    /* color PROM */

	ROM_REGION( 0x0020, "user1", 0 )    /* misc PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( carhntds )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr617.u33",      0x0000, 0x0400, CRC(0bbfdb4e) SHA1(383599276923264602a0b5efeac9697d9c15a20f) )
	ROM_CONTINUE(0x4000, 0x400)
	ROM_LOAD( "epr618.u32",      0x0400, 0x0400, CRC(5a080b1d) SHA1(3c3d6b8b16d9a8976ee63435e949b67f025e38d8) )
	ROM_LOAD( "epr619.u31",      0x0800, 0x0400, CRC(c6f2f399) SHA1(4bb34816042ec352b0bb71d2d654ebb6fb5083e9) )
	ROM_LOAD( "epr620.u30",      0x0c00, 0x0400, CRC(d9deb88f) SHA1(a863f29cfed782c8871a2268aa28a67ccced1b45) )
	ROM_LOAD( "epr621.u29",      0x1000, 0x0400, CRC(43e5de5c) SHA1(3adff5042be73f4693cee7977c88f425e930532d) )
	ROM_LOAD( "epr622.u28",      0x1400, 0x0400, CRC(c881a3bc) SHA1(fbff24c4075103fd686cfb376b08cf0677522222) )
	ROM_LOAD( "epr623.u27",      0x1800, 0x0400, CRC(297e7f42) SHA1(9e1042fe96f3bf55228b759b905467eff814ad84) )
	ROM_LOAD( "epr624.u26",      0x1c00, 0x0400, CRC(dc943125) SHA1(39f18e29cc3d03ee1b98a969f739e22dfd76b6f7) )
	ROM_LOAD( "epr625.u8",       0x2000, 0x0400, CRC(c86a0842) SHA1(5c59fe64985936b847a4c033a805153f627b1883) )
	ROM_LOAD( "epr626.u7",       0x2400, 0x0400, CRC(9a48c939) SHA1(2c898810f6fd97d4a6edb97236aaa29d08f5598a) )
	ROM_LOAD( "epr627.u6",       0x2800, 0x0400, CRC(b4b147e2) SHA1(8880f80708711b253f5da352679820500680ebab) )
	ROM_LOAD( "epr628.u5",       0x2c00, 0x0400, CRC(aecf3c26) SHA1(2f4419c6ccf03042cf32a415160e41d52fd0ef9c) )
	ROM_LOAD( "epr629.u4",       0x3000, 0x0400, CRC(c5be665b) SHA1(8bb145c140afa6166f08881b7e820335c3f83c08) )
	ROM_LOAD( "epr630.u3",       0x3400, 0x0400, CRC(4312388b) SHA1(d0b53d505276754651d2aeda4b17f2d600f65166) )
	ROM_LOAD( "epr631.u2",       0x3800, 0x0400, CRC(6766c7e5) SHA1(05b0ac31894c3c80d3940206aac2ef21a96ff849) )
	ROM_LOAD( "epr632.u1",       0x3c00, 0x0400, CRC(ae68b7d5) SHA1(de449b62ba39331a4ecf3dfe81511b21b7c881d5) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0390.u49", 0x0000, 0x0020, CRC(a0811288) SHA1(a6e78c26f7eeb70125eee715eb6a3e3c82ed7fc8) )    /* color PROM */
ROM_END

ROM_START( tranqgun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u33.bin",      0x0000, 0x0400, CRC(6d50e902) SHA1(1d14c0b28cb3650bb57b9ef61265fe94c453d648) )
	ROM_LOAD( "u32.bin",      0x0400, 0x0400, CRC(f0ba0e60) SHA1(fcdd4355ccc1893a8a0450403f466bee916793dc) )
	ROM_LOAD( "u31.bin",      0x0800, 0x0400, CRC(9fe440d3) SHA1(a0be6caf3ea821c84be83351a0b80c240485218f) )
	ROM_LOAD( "u30.bin",      0x0c00, 0x0400, CRC(1041608e) SHA1(9f9667a4ad98f43c606b534157488fb7dd7b18ba) )
	ROM_LOAD( "u29.bin",      0x1000, 0x0400, CRC(fb5de95f) SHA1(7ff57c2e7b074e74936fde808f455a118dc495c2) )
	ROM_LOAD( "u28.bin",      0x1400, 0x0400, CRC(03fd8727) SHA1(d334c50adcaea02f9b55b4ec1b4c7dda3a0298ca) )
	ROM_LOAD( "u27.bin",      0x1800, 0x0400, CRC(3d93239b) SHA1(211748992f5735cf4b8a69f035aa89b40c814d75) )
	ROM_LOAD( "u26.bin",      0x1c00, 0x0400, CRC(20f64a7f) SHA1(e61e7dd0f418c17954b72d443a1de206ef8cf4d0) )
	ROM_LOAD( "u8.bin",       0x2000, 0x0400, CRC(5121c695) SHA1(6b8ae30f2f17ff886e6c61783613a60b5c3d9b82) )
	ROM_LOAD( "u7.bin",       0x2400, 0x0400, CRC(b13d21f7) SHA1(f624e96c1dada3222b6b5a83f0bdf52b821077d6) )
	ROM_LOAD( "u6.bin",       0x2800, 0x0400, CRC(603cee59) SHA1(7c1f55cfe36cf52687f1c60d9277b46bd12054d4) )
	ROM_LOAD( "u5.bin",       0x2c00, 0x0400, CRC(7f25475f) SHA1(f519d59872d9429300040cb0ce940e8103087763) )
	ROM_LOAD( "u4.bin",       0x3000, 0x0400, CRC(57dc3123) SHA1(db57cb77d840bc97d0fa9de28cc4eba90b0319d0) )
	ROM_LOAD( "u3.bin",       0x3400, 0x0400, CRC(7aa7829b) SHA1(ef9835d83d4468a28aa2a837b3c23760d9e5480e) )
	ROM_LOAD( "u2.bin",       0x3800, 0x0400, CRC(a9b10df5) SHA1(f74e83c5f51ecb53e3190c1ae83fa59b60806f62) )
	ROM_LOAD( "u1.bin",       0x3c00, 0x0400, CRC(431a7449) SHA1(67a142c084078ebebe9ed91b577fffe1ab0f39a2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u49.bin",      0x0000, 0x0020, CRC(6481445b) SHA1(2136408f25a95ed73882deaa5a174d4a1a7ba438) )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( spacetrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u33.bin",      0x0000, 0x0400, CRC(9033fe50) SHA1(0a9b86af03956575403d8b494963f55887fc4dc3) )
	ROM_LOAD( "u32.bin",      0x0400, 0x0400, CRC(08f61f0d) SHA1(f206b18959e2cb6d4f6962415695eca0412da739) )
	ROM_LOAD( "u31.bin",      0x0800, 0x0400, CRC(1088a8c4) SHA1(ca40098137f0d90548d62a4daead3c6bc488fde0) )
	ROM_LOAD( "u30.bin",      0x0c00, 0x0400, CRC(55560cc8) SHA1(751585e261088cc740aa293a1e71f13b928c37f9) )
	ROM_LOAD( "u29.bin",      0x1000, 0x0400, CRC(71713958) SHA1(85399d3cbe5ed265b59b4441c17247b09c4c34d0) )
	ROM_LOAD( "u28.bin",      0x1400, 0x0400, CRC(7bcf5ca3) SHA1(05a6d31c0229e840081cad7fb17bdebc0fd6484f) )
	ROM_LOAD( "u27.bin",      0x1800, 0x0400, CRC(ad7a2065) SHA1(04a98b4578c7770f69525c614c402ccf2f3c99ed) )
	ROM_LOAD( "u26.bin",      0x1c00, 0x0400, CRC(6060fe77) SHA1(d7606daabdb0212daef6724b0ba6abab13731080) )
	ROM_LOAD( "u8.bin",       0x2000, 0x0400, CRC(75a90624) SHA1(61f10dd66ba1daa1b37c407d22d2ea3da2c8fec8) )
	ROM_LOAD( "u7.bin",       0x2400, 0x0400, CRC(7b31a2ab) SHA1(58ebf8ccb2e807aacc24c0017cb8c094124c1776) )
	ROM_LOAD( "u6.bin",       0x2800, 0x0400, CRC(94135b33) SHA1(deb836034ab1de016a26d45e7176f292aa34038c) )
	ROM_LOAD( "u5.bin",       0x2c00, 0x0400, CRC(cfbf2538) SHA1(d200ae1a866a6e4587226e449a3d09d330a79cf4) )
	ROM_LOAD( "u4.bin",       0x3000, 0x0400, CRC(b4b95129) SHA1(04c4ced1b8d0bf2cbb2533d9870574395a5f0793) )
	ROM_LOAD( "u3.bin",       0x3400, 0x0400, CRC(03ca1d70) SHA1(66ed7664ee1ddbadd6993d44c3684e6cb47912b3) )
	ROM_LOAD( "u2.bin",       0x3800, 0x0400, CRC(a968584b) SHA1(4ea4b83e116627daec4d226591924f84740e7786) )
	ROM_LOAD( "u1.bin",       0x3c00, 0x0400, CRC(e6e300e8) SHA1(4e198915c4f2b904dcc9ad81e5106bd711546e4e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u49.bin",      0x0000, 0x0020, CRC(aabae4cd) SHA1(6748d20318aed1c9949a3373166ebdca13eae965) )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( spacetrkc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u33c.bin",     0x0000, 0x0400, CRC(b056b928) SHA1(1bbf5c30b226c5ca3c09fcff36a1b21132a524b6) )
	ROM_LOAD( "u32c.bin",     0x0400, 0x0400, CRC(dffb11d9) SHA1(5c95b7e493ac9e8714d91d19b6f01967559ce55c) )
	ROM_LOAD( "u31c.bin",     0x0800, 0x0400, CRC(9b25d46f) SHA1(29362fe4587f6425cc55c6f5592f853caf8bc1e2) )
	ROM_LOAD( "u30c.bin",     0x0c00, 0x0400, CRC(3a612bfe) SHA1(914a9a1192bfb255ee8b72671d33b5ae17b3a1b7) )
	ROM_LOAD( "u29c.bin",     0x1000, 0x0400, CRC(d8bb6e0c) SHA1(6bf57dde384a447350deb7125e287af147d51878) )
	ROM_LOAD( "u28c.bin",     0x1400, 0x0400, CRC(0e367740) SHA1(5140244d4cd64727ddfe052b7b4880e9599998f9) )
	ROM_LOAD( "u27c.bin",     0x1800, 0x0400, CRC(d59fec86) SHA1(8f0163aa61fb35a9977e24c36f7c01c3fb0bc156) )
	ROM_LOAD( "u26c.bin",     0x1c00, 0x0400, CRC(9deefa0f) SHA1(d109743e737931227bb504c47ba3cc959ae24756) )
	ROM_LOAD( "u8c.bin",      0x2000, 0x0400, CRC(613116c5) SHA1(23ad643a3df0d3823b776422454cba2e590a49c6) )
	ROM_LOAD( "u7c.bin",      0x2400, 0x0400, CRC(3bdf2464) SHA1(a19a863da8fb33650483ba5b4ea133f1c6393620) )
	ROM_LOAD( "u6c.bin",      0x2800, 0x0400, CRC(039d73fa) SHA1(08498e76b4f29f22078dfbe7ec52f47528f6db42) )
	ROM_LOAD( "u5c.bin",      0x2c00, 0x0400, CRC(1638344f) SHA1(0f8488763f3f3776ab09eb5cf1d4db8ebd384345) )
	ROM_LOAD( "u4c.bin",      0x3000, 0x0400, CRC(e34443cd) SHA1(8f2302f48aaceae70d8e576f318f44e96f92d4f3) )
	ROM_LOAD( "u3c.bin",      0x3400, 0x0400, CRC(6f16cbd7) SHA1(ef32558731d374a1417bb58ba24c1aa79d71ecd8) )
	ROM_LOAD( "u2c.bin",      0x3800, 0x0400, CRC(94da3cdc) SHA1(082f96c6fad0ccb314e80efd3a2d7e156a55c48e) )
	ROM_LOAD( "u1c.bin",      0x3c00, 0x0400, CRC(2a228bf4) SHA1(7c39700c7a63ca3202134acefa8e6f04f7145dfd) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u49.bin",      0x0000, 0x0020, CRC(aabae4cd) SHA1(6748d20318aed1c9949a3373166ebdca13eae965) )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( carnival )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-651.u33",   0x0000, 0x0400, CRC(9f2736e6) SHA1(c3fb9197b5e83dc7d5335de2268e0acb30cf8328) )
	ROM_LOAD( "epr-652.u32",   0x0400, 0x0400, CRC(a1f58beb) SHA1(e027beca7bf3ef5ef67e2195f909332fd194b5dc) )
	ROM_LOAD( "epr-653.u31",   0x0800, 0x0400, CRC(67b17922) SHA1(46cdfd0371dec61a5440c2111660729c0f0ecdb8) )
	ROM_LOAD( "epr-654.u30",   0x0c00, 0x0400, CRC(befb09a5) SHA1(da44b6a869b5eb0705e01fee4478696f6bef9de8) )
	ROM_LOAD( "epr-655.u29",   0x1000, 0x0400, CRC(623fcdad) SHA1(35890964f5cf799c141002916641089ccec0fcc9) )
	ROM_LOAD( "epr-656.u28",   0x1400, 0x0400, CRC(53040332) SHA1(ff7a06d93cb890abf0616770774668396d128ba3) )
	ROM_LOAD( "epr-657.u27",   0x1800, 0x0400, CRC(f2537467) SHA1(262b859098f4f7e5e9bf2f83bda833044824226e) )
	ROM_LOAD( "epr-658.u26",   0x1c00, 0x0400, CRC(fcc3854e) SHA1(7adbd6ca6f636dec75fa6eccdf3381686e074bc6) )
	ROM_LOAD( "epr-659.u8",    0x2000, 0x0400, CRC(28be8d69) SHA1(2d9ac9a53f00fe2282e4317585e6bddadb676c0f) )
	ROM_LOAD( "epr-660.u7",    0x2400, 0x0400, CRC(3873ccdb) SHA1(56be81fdee8947758ba966915c0739e5560a7f94) )
	ROM_LOAD( "epr-661.u6",    0x2800, 0x0400, CRC(d9a96dff) SHA1(0366acf3418901bfeeda59d4cd51fe8ceaad4577) )
	ROM_LOAD( "epr-662.u5",    0x2c00, 0x0400, CRC(d893ca72) SHA1(564176ab7f3757d51db8eef9fbc4228fa2ce328f) )
	ROM_LOAD( "epr-663.u4",    0x3000, 0x0400, CRC(df8c63c5) SHA1(e8d0632b5cb5bd7f698485531f3edeb13efdc685) )
	ROM_LOAD( "epr-664.u3",    0x3400, 0x0400, CRC(689a73e8) SHA1(b4134e8d892df7ba3352e4d3f581923decae6e54) )
	ROM_LOAD( "epr-665.u2",    0x3800, 0x0400, CRC(28e7b2b6) SHA1(57eb5dd0f11da8ff8001e76036264246d6bc27d2) )
	ROM_LOAD( "epr-666.u1",    0x3c00, 0x0400, CRC(4eec7fae) SHA1(cdc858165136c55b01511805c9d4dc6bc598fe1f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0633.u49",  0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) )

	ROM_REGION( 0x0400, "audiocpu", 0 ) /* sound ROM */
	ROM_LOAD( "epr-412.u5",    0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u14",  0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( carnivalb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-651.u33",   0x0000, 0x0400, CRC(9f2736e6) SHA1(c3fb9197b5e83dc7d5335de2268e0acb30cf8328) )
	ROM_LOAD( "epr-652.u32",   0x0400, 0x0400, CRC(a1f58beb) SHA1(e027beca7bf3ef5ef67e2195f909332fd194b5dc) )
	ROM_LOAD( "epr-653.u31",   0x0800, 0x0400, CRC(67b17922) SHA1(46cdfd0371dec61a5440c2111660729c0f0ecdb8) )
	ROM_LOAD( "epr-654.u30",   0x0c00, 0x0400, CRC(befb09a5) SHA1(da44b6a869b5eb0705e01fee4478696f6bef9de8) )
	ROM_LOAD( "epr-655.u29",   0x1000, 0x0400, CRC(623fcdad) SHA1(35890964f5cf799c141002916641089ccec0fcc9) )
	ROM_LOAD( "epr-656.u28",   0x1400, 0x0400, CRC(53040332) SHA1(ff7a06d93cb890abf0616770774668396d128ba3) )
	ROM_LOAD( "epr-657.u27",   0x1800, 0x0400, CRC(f2537467) SHA1(262b859098f4f7e5e9bf2f83bda833044824226e) )
	ROM_LOAD( "epr-658.u26",   0x1c00, 0x0400, CRC(fcc3854e) SHA1(7adbd6ca6f636dec75fa6eccdf3381686e074bc6) )
	ROM_LOAD( "epr-659.u8",    0x2000, 0x0400, CRC(28be8d69) SHA1(2d9ac9a53f00fe2282e4317585e6bddadb676c0f) )
	ROM_LOAD( "epr-660.u7",    0x2400, 0x0400, CRC(3873ccdb) SHA1(56be81fdee8947758ba966915c0739e5560a7f94) )
	ROM_LOAD( "epr-661.u6",    0x2800, 0x0400, CRC(d9a96dff) SHA1(0366acf3418901bfeeda59d4cd51fe8ceaad4577) )
	ROM_LOAD( "epr-662.u5",    0x2c00, 0x0400, CRC(d893ca72) SHA1(564176ab7f3757d51db8eef9fbc4228fa2ce328f) )
	ROM_LOAD( "epr-663.u4",    0x3000, 0x0400, CRC(df8c63c5) SHA1(e8d0632b5cb5bd7f698485531f3edeb13efdc685) )
	ROM_LOAD( "epr-664.u3",    0x3400, 0x0400, CRC(689a73e8) SHA1(b4134e8d892df7ba3352e4d3f581923decae6e54) )
	ROM_LOAD( "epr-665.u2",    0x3800, 0x0400, CRC(28e7b2b6) SHA1(57eb5dd0f11da8ff8001e76036264246d6bc27d2) )
	ROM_LOAD( "epr-666.u1",    0x3c00, 0x0400, CRC(4eec7fae) SHA1(cdc858165136c55b01511805c9d4dc6bc598fe1f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0633.u49",  0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) )

	ROM_REGION( 0x0400, "audiocpu", 0 ) /* sound ROM */
	ROM_LOAD( "carnival_b.u5", 0x0000, 0x0400, CRC(422abb43) SHA1(edb6f74ccf9382624353e927ac225e1455bb1144) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u14",  0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( carnivalc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-501.u33", 0x0000, 0x0400, CRC(688503d2) SHA1(a1fe03c23276d458ba74f7473524918eb9b7c7e5) )
	ROM_LOAD( "epr-652.u32", 0x0400, 0x0400, CRC(a1f58beb) SHA1(e027beca7bf3ef5ef67e2195f909332fd194b5dc) )
	ROM_LOAD( "epr-653.u31", 0x0800, 0x0400, CRC(67b17922) SHA1(46cdfd0371dec61a5440c2111660729c0f0ecdb8) )
	ROM_LOAD( "epr-654.u30", 0x0c00, 0x0400, CRC(befb09a5) SHA1(da44b6a869b5eb0705e01fee4478696f6bef9de8) )
	ROM_LOAD( "epr-655.u29", 0x1000, 0x0400, CRC(623fcdad) SHA1(35890964f5cf799c141002916641089ccec0fcc9) )
	ROM_LOAD( "epr-506.u28", 0x1400, 0x0400, CRC(ba916e97) SHA1(87e1ac8bd21bafb815b702048c6be24f410a8998) )
	ROM_LOAD( "epr-507.u27", 0x1800, 0x0400, CRC(d0bda4a5) SHA1(faf3f3c2a8f962c6c2901e5a4a31b452b506ee22) )
	ROM_LOAD( "epr-508.u26", 0x1c00, 0x0400, CRC(f0258cad) SHA1(065c90835dadeda7085422295cde09491c94b6e0) )
	ROM_LOAD( "epr-509.u8",  0x2000, 0x0400, CRC(dcc8a530) SHA1(2d1cac3b40f5afab5423d45ecc3f5565266f9c03) )
	ROM_LOAD( "epr-510.u7",  0x2400, 0x0400, CRC(92c2ba51) SHA1(c0c0c836d2aa6943bd808acc12161adf23cd0d21) )
	ROM_LOAD( "epr-511.u6",  0x2800, 0x0400, CRC(3af899a0) SHA1(540cbdc6a912cb325bbea33935bfa06e13f0021a) )
	ROM_LOAD( "epr-512.u5",  0x2c00, 0x0400, CRC(09f7b3e6) SHA1(18a58680500148e7a031e91230d73b2ce4dc712d) )
	ROM_LOAD( "epr-513.u4",  0x3000, 0x0400, CRC(8f41974c) SHA1(65e9c2378ad99f804590de36ffba4c60bfa1bfe3) )
	ROM_LOAD( "epr-514.u3",  0x3400, 0x0400, CRC(2788d140) SHA1(7341c44fb1f7eb8c4d25d3e1e75bcf3bfdb9a89c) )
	ROM_LOAD( "epr-515.u2",  0x3800, 0x0400, CRC(10decaa9) SHA1(4c980164dde275e1488ae74a7ad61e6acc75e608) )
	ROM_LOAD( "epr-516.u1",  0x3c00, 0x0400, CRC(7c32b352) SHA1(8cb472a7f71a301417c6a8e4a26a9bdcd43b6062) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0633.u49", 0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) )

	ROM_REGION( 0x0400, "audiocpu", 0 ) /* sound ROM */
	ROM_LOAD( "epr-412.u5",   0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( carnivalca ) // S-97095-P + 97093-P-B
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-479.u33", 0x0000, 0x0400, CRC(620bff59) SHA1(87d3b4991cacb0a8cd7eeef652eb3f1c1eaadb17) )
	ROM_LOAD( "epr-480.u32", 0x0400, 0x0400, CRC(90768717) SHA1(a34e6c28d486614b0a084a088d75feb8ecb6ef15) )
	ROM_LOAD( "epr-481.u31", 0x0800, 0x0400, CRC(5c903af9) SHA1(5f11c6c800cfb012ba592b890d9849cc03192aaa) )
	ROM_LOAD( "epr-482.u30", 0x0c00, 0x0400, CRC(fb2398a1) SHA1(abab2cd43054b719b0bee0f7452908df062e34bc) )
	ROM_LOAD( "epr-483.u29", 0x1000, 0x0400, CRC(306f50d2) SHA1(93faa3e2a8e49f3afb591a2fe7bb401278a354e2) )
	ROM_LOAD( "epr-484.u28", 0x1400, 0x0400, CRC(e80a6ebc) SHA1(5ef94a2ee7040b56951ef68ee4198f64af1449b0) )
	ROM_LOAD( "epr-485.u27", 0x1800, 0x0400, CRC(993e2885) SHA1(82a04258d5c25bddb6556a73f0e85a9af9d510d2) )
	ROM_LOAD( "epr-486.u26", 0x1c00, 0x0400, CRC(e6b7eb05) SHA1(1b998cdf39d38437b5d32958dbde80b29c1b2736) )
	ROM_LOAD( "epr-487.u8",  0x2000, 0x0400, CRC(41abf6f8) SHA1(1a02c268368ea761113c831d4a6dfc0a1853ebc7) )
	ROM_LOAD( "epr-488.u7",  0x2400, 0x0400, CRC(b0288f2a) SHA1(c02311f4f640a17b68cc10ca01ab3e919a9355b0) )
	ROM_LOAD( "epr-489.u6",  0x2800, 0x0400, CRC(c9c529ab) SHA1(98bb99afc1e23dff13ab73c051beec44750eb1b3) )
	ROM_LOAD( "epr-490.u5",  0x2c00, 0x0400, CRC(9d4ffe25) SHA1(b0dec556f16d505190b8703c367ecb4d07147156) )
	ROM_LOAD( "epr-491.u4",  0x3000, 0x0400, CRC(5d086aec) SHA1(d7b9125b0f9dd71d5ffcac321ec2c0f3f7953031) )
	ROM_LOAD( "epr-492.u3",  0x3400, 0x0400, CRC(0e20e4a5) SHA1(82c8ad21a72c8dff81eb4bb97e64faff33a8b46f) )
	ROM_LOAD( "epr-493.u2",  0x3800, 0x0400, CRC(b4a7c5c9) SHA1(832597a2b586bffebaf2d67be8dff316d8120974) )
	ROM_LOAD( "epr-494.u1",  0x3c00, 0x0400, NO_DUMP ) // missing on PCB, but listed on the ROM sheet it came with

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-62.u49",      0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) )

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "epr-412.u5",   0x0000, 0x0400, BAD_DUMP CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) ) // not dumped for this set (sound PCB wasn't available), so taken from carnivalc

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "bin.u14", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    // control PROM, possibly pr-34 but unreadable
ROM_END

ROM_START( carnivalh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-155.u48",   0x0000, 0x0800, CRC(0a5f1f65) SHA1(7830157ad378f92a8069debf78d50b9deafb4d40) )
	ROM_LOAD( "epr-156.u47",   0x0800, 0x0800, CRC(422221ff) SHA1(57180236dd1cf5838131b35256c3d46cb8015804) )
	ROM_LOAD( "epr-157.u46",   0x1000, 0x0800, CRC(1551dffb) SHA1(f91563a14c5febee64b45579acf49518cd54bec3) )
	ROM_LOAD( "epr-158.u45",   0x1800, 0x0800, CRC(9238b5c0) SHA1(673f4fedd2e7beef6bc5e6371b2ed7722f3c141c) )
	ROM_LOAD( "epr-159.u44",   0x2000, 0x0800, CRC(5c2b9a33) SHA1(2cc57ceebd2938e589c07892b6587b8571678fcc) )
	ROM_LOAD( "epr-160.u43",   0x2800, 0x0800, CRC(dd70471f) SHA1(2d987e946873955998c428930592ea7734970b16) )
	ROM_LOAD( "epr-161.u42",   0x3000, 0x0800, CRC(42714a0d) SHA1(96f8904912d85b2e7037d129ff2443e90693fe22) )
	ROM_LOAD( "epr-162.u41",   0x3800, 0x0800, CRC(56e1c120) SHA1(24816b6a9bc238571ab8ea79bb876cf249ed4d60) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-62.u44",      0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) ) /* Same as 316-0633 */

	ROM_REGION( 0x0400, "audiocpu", 0 ) /* sound ROM */
	ROM_LOAD( "epr-412.u5",     0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs (type n82s123) */
	ROM_LOAD( "316-043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) ) /* control PROM */
	ROM_LOAD( "316-042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) ) /* sequence PROM */
ROM_END

ROM_START( carnivalha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-155.u48",   0x0000, 0x0800, CRC(0a5f1f65) SHA1(7830157ad378f92a8069debf78d50b9deafb4d40) )
	ROM_LOAD( "epr-156.u47",   0x0800, 0x0800, CRC(422221ff) SHA1(57180236dd1cf5838131b35256c3d46cb8015804) )
	ROM_LOAD( "epr-157.u46",   0x1000, 0x0800, CRC(1551dffb) SHA1(f91563a14c5febee64b45579acf49518cd54bec3) )
	ROM_LOAD( "epr-158.u45",   0x1800, 0x0800, CRC(9238b5c0) SHA1(673f4fedd2e7beef6bc5e6371b2ed7722f3c141c) )
	ROM_LOAD( "epr-159.u44",   0x2000, 0x0800, CRC(5c2b9a33) SHA1(2cc57ceebd2938e589c07892b6587b8571678fcc) )
	ROM_LOAD( "epr-160.u43",   0x2800, 0x0800, CRC(dd70471f) SHA1(2d987e946873955998c428930592ea7734970b16) )
	ROM_LOAD( "epr-161x.u42",  0x3000, 0x0800, CRC(8133ba08) SHA1(fc314e847810140d99ad071667c10a1a57c9f892) ) // rom marked with 161X, probably modified in some way
	ROM_LOAD( "epr-162.u41",   0x3800, 0x0800, CRC(56e1c120) SHA1(24816b6a9bc238571ab8ea79bb876cf249ed4d60) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-62.u44",      0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) ) /* Same as 316-0633 */

	ROM_REGION( 0x0400, "audiocpu", 0 ) /* sound ROM */
	ROM_LOAD( "epr-412.u5",     0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs (type n82s123) */
	ROM_LOAD( "316-043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) ) /* control PROM */
	ROM_LOAD( "316-042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) ) /* sequence PROM */
ROM_END

ROM_START( carnivalmm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "16mm.u33",  0x0000, 0x0400, CRC(0230bd0a) SHA1(1f5ae709e4cc86b8e682401b2d82b31eb45b41d5) )
	ROM_LOAD( "15mm.u32",  0x0400, 0x0400, CRC(a1f58beb) SHA1(e027beca7bf3ef5ef67e2195f909332fd194b5dc) )
	ROM_LOAD( "14mm.u31",  0x0800, 0x0400, CRC(67b17922) SHA1(46cdfd0371dec61a5440c2111660729c0f0ecdb8) )
	ROM_LOAD( "13mm.u30",  0x0c00, 0x0400, CRC(befb09a5) SHA1(da44b6a869b5eb0705e01fee4478696f6bef9de8) )
	ROM_LOAD( "12mm.u29",  0x1000, 0x0400, CRC(b5230913) SHA1(0db699e40c35da113f9301cedcd958d765699aac) )
	ROM_LOAD( "11mm.u28",  0x1400, 0x0400, CRC(53040332) SHA1(ff7a06d93cb890abf0616770774668396d128ba3) )
	ROM_LOAD( "10mm.u27",  0x1800, 0x0400, CRC(a3b9c2db) SHA1(2893fae8c2aeab2a68297bf1f2c3022ff98dca95) )
	ROM_LOAD( "9mm.u26",   0x1c00, 0x0400, CRC(fcc3854e) SHA1(7adbd6ca6f636dec75fa6eccdf3381686e074bc6) )
	ROM_LOAD( "8mm.u8",    0x2000, 0x0400, CRC(28be8d69) SHA1(2d9ac9a53f00fe2282e4317585e6bddadb676c0f) )
	ROM_LOAD( "7mm.u7",    0x2400, 0x0400, CRC(3873ccdb) SHA1(56be81fdee8947758ba966915c0739e5560a7f94) )
	ROM_LOAD( "6mm.u6",    0x2800, 0x0400, CRC(d9a96dff) SHA1(0366acf3418901bfeeda59d4cd51fe8ceaad4577) )
	ROM_LOAD( "5mm.u5",    0x2c00, 0x0400, CRC(d893ca72) SHA1(564176ab7f3757d51db8eef9fbc4228fa2ce328f) )
	ROM_LOAD( "4mm.u4",    0x3000, 0x0400, CRC(df8c63c5) SHA1(e8d0632b5cb5bd7f698485531f3edeb13efdc685) )
	ROM_LOAD( "3mm.u3",    0x3400, 0x0400, CRC(689a73e8) SHA1(b4134e8d892df7ba3352e4d3f581923decae6e54) )
	ROM_LOAD( "2mm.u2",    0x3800, 0x0400, CRC(b94ef7ab) SHA1(8079e0f97688817d2f6ed5810aa1501dc09585d6) )
	ROM_LOAD( "1mm.u1",    0x3c00, 0x0400, CRC(6e10c057) SHA1(743a28bb6f4f395fd3db36d5e40acc3475f55f5d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.u4",       0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) )

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "sound.u25",    0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "mmi6331.u14",  0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )
ROM_END

ROM_START( verbena )
	ROM_REGION( 0x10000, "maincpu", 0 ) // all 2708s
	ROM_LOAD( "16v.u33",   0x0000, 0x0400, CRC(a47a1c54) SHA1(1c8944c24b6eb1d35b778dc7448466f1c5be484e) ) // handwritten label
	ROM_LOAD( "15mm.u32",  0x0400, 0x0400, CRC(a1f58beb) SHA1(e027beca7bf3ef5ef67e2195f909332fd194b5dc) )
	ROM_LOAD( "14mm.u31",  0x0800, 0x0400, CRC(67b17922) SHA1(46cdfd0371dec61a5440c2111660729c0f0ecdb8) )
	ROM_LOAD( "13mm.u30",  0x0c00, 0x0400, CRC(befb09a5) SHA1(da44b6a869b5eb0705e01fee4478696f6bef9de8) )
	ROM_LOAD( "12mm.u29",  0x1000, 0x0400, CRC(b5230913) SHA1(0db699e40c35da113f9301cedcd958d765699aac) )
	ROM_LOAD( "11mm.u28",  0x1400, 0x0400, CRC(53040332) SHA1(ff7a06d93cb890abf0616770774668396d128ba3) )
	ROM_LOAD( "10mm.u27",  0x1800, 0x0400, CRC(a3b9c2db) SHA1(2893fae8c2aeab2a68297bf1f2c3022ff98dca95) )
	ROM_LOAD( "9mm.u26",   0x1c00, 0x0400, CRC(fcc3854e) SHA1(7adbd6ca6f636dec75fa6eccdf3381686e074bc6) )
	ROM_LOAD( "8mm.u8",    0x2000, 0x0400, CRC(28be8d69) SHA1(2d9ac9a53f00fe2282e4317585e6bddadb676c0f) )
	ROM_LOAD( "7mm.u7",    0x2400, 0x0400, CRC(3873ccdb) SHA1(56be81fdee8947758ba966915c0739e5560a7f94) )
	ROM_LOAD( "6mm.u6",    0x2800, 0x0400, CRC(d9a96dff) SHA1(0366acf3418901bfeeda59d4cd51fe8ceaad4577) )
	ROM_LOAD( "5mm.u5",    0x2c00, 0x0400, CRC(d893ca72) SHA1(564176ab7f3757d51db8eef9fbc4228fa2ce328f) )
	ROM_LOAD( "4mm.u4",    0x3000, 0x0400, CRC(df8c63c5) SHA1(e8d0632b5cb5bd7f698485531f3edeb13efdc685) )
	ROM_LOAD( "3mm.u3",    0x3400, 0x0400, CRC(689a73e8) SHA1(b4134e8d892df7ba3352e4d3f581923decae6e54) )
	ROM_LOAD( "2mm.u2",    0x3800, 0x0400, CRC(b94ef7ab) SHA1(8079e0f97688817d2f6ed5810aa1501dc09585d6) )
	ROM_LOAD( "1v.u1",     0x3c00, 0x0400, CRC(6e10c057) SHA1(743a28bb6f4f395fd3db36d5e40acc3475f55f5d) ) // handwritten label

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.u4",       0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) ) // == 316-0633 / pr-62.u44

	ROM_REGION( 0x0400, "audiocpu", 0 ) /* sound ROM */
	ROM_LOAD( "sound.u25",    0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) ) // == epr-412.u5 - 2708

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "mmi6331.u14",  0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( brdrline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1.bin",       0x0000, 0x0400, CRC(df182769) SHA1(2b1b70c6282b32e0a4ed80ab4e6b20f90630e910) )
	ROM_LOAD( "b2.bin",       0x0400, 0x0400, CRC(e1d1c4ce) SHA1(86320c836577549af6fe6c311f8475a51de52627) )
	ROM_LOAD( "b3.bin",       0x0800, 0x0400, CRC(4ec4afa2) SHA1(dd5b97f1a37cd655064b773e4a755b87de4c6a3f) )
	ROM_LOAD( "b4.bin",       0x0c00, 0x0400, CRC(88de95f6) SHA1(fe3388346785ad15dead89913e4ff36120a83599) )
	ROM_LOAD( "b5.bin",       0x1000, 0x0400, CRC(2e4e13b9) SHA1(bdf31c11733127b8b77fa72933d3b9dc6834d5d8) )
	ROM_LOAD( "b6.bin",       0x1400, 0x0400, CRC(c181e87a) SHA1(426e1ce15477039e4a19b536500f387518026efc) )
	ROM_LOAD( "b7.bin",       0x1800, 0x0400, CRC(21180015) SHA1(b23f876db1a9a986f1087ead07a01e836d5ee842) )
	ROM_LOAD( "b8.bin",       0x1c00, 0x0400, CRC(56a7fee0) SHA1(495efa91773fd3cf36da4e538893db08e64e5bab) )
	ROM_LOAD( "b9.bin",       0x2000, 0x0400, CRC(bb532e63) SHA1(da511e0be58b13781897e6efb5a59a3558016b12) )
	ROM_LOAD( "b10.bin",      0x2400, 0x0400, CRC(64793709) SHA1(fabfb783f1d93a3d9454fc345a64498e4b5b9138) )
	ROM_LOAD( "b11.bin",      0x2800, 0x0400, CRC(2ae2f928) SHA1(afd99c800801d38ee59008344bd9a3901f72ff50) )
	ROM_LOAD( "b12.bin",      0x2c00, 0x0400, CRC(e14cfaf5) SHA1(d159e93f703aae3c04da08102ff718d5a4ca7a91) )
	ROM_LOAD( "b13.bin",      0x3000, 0x0400, CRC(605e0d27) SHA1(771de6d31ee7896a2441f1df4565027793d99989) )
	ROM_LOAD( "b14.bin",      0x3400, 0x0400, CRC(93f5714f) SHA1(50a043be1e1cf8b1aeb846571a12fe70cbb3477e) )
	ROM_LOAD( "b15.bin",      0x3800, 0x0400, CRC(2f8a9b1c) SHA1(853d5ca017b133c1f13f703cceb20f04199d4752) )
	ROM_LOAD( "b16.bin",      0x3c00, 0x0400, CRC(cc138bed) SHA1(7d3eebdeaff19783d5ef20a7ececec00773434fc) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "borderc.49",   0x0000, 0x0020, CRC(bc6be94e) SHA1(34e113ec25e19212b74907d35be5cb8714a8249c) )

	ROM_REGION( 0x0100, "user1", 0 )    /* misc PROM */
	ROM_LOAD( "border.32",   0x0000, 0x0020, CRC(c128d0ba) SHA1(0ce9febbb7e2f5388ed999a479e3d385dba0b342) )
	ROM_LOAD( "bordera.15",  0x0000, 0x0020, CRC(6449e678) SHA1(421c45c8fba3c2bc2a7ebbea2c837c8fa1a5a2f3) ) // sequence
	ROM_LOAD( "borderb.14",  0x0000, 0x0020, CRC(55dcdef1) SHA1(6fbd041edc258b7e1b99bbe9526612cfb1b541f8) ) // control
	/* following 2 from sound board */
	ROM_LOAD( "prom93427.1", 0x0000, 0x0100, CRC(64b98dc7) SHA1(f0bb7d0b4b56cc2936ce4cbec165394f3026ed6d) )
	ROM_LOAD( "prom93427.2", 0x0000, 0x0100, CRC(bda82367) SHA1(1c96453c2ae372892c39b5657cf2b252a90a10a9) )

	ROM_REGION( 0x0400, "user2", 0 ) /* sound ROM */
	ROM_LOAD( "au.bin",       0x0000, 0x0400, CRC(a23e1d9f) SHA1(ce209571f6341aa6f036a015e666673098bc98ea) )
ROM_END


ROM_START( brdrlinet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1171a.u33",      0x0000, 0x0400, CRC(38dd9880) SHA1(1a879ce990129fd34e7265010872ac998d16accf) )
	ROM_LOAD( "1172a.u32",      0x0400, 0x0400, CRC(1a3adff0) SHA1(3fab79688b739d6a5979638115629d0a61f8878b) )
	ROM_LOAD( "1173a.u31",      0x0800, 0x0400, CRC(e668734d) SHA1(b01b06f4a107f14001c70e63e072c575cd97c89b) )
	ROM_LOAD( "1174a.u30",      0x0c00, 0x0400, CRC(d1ca5d52) SHA1(2ed9368741a409f7483020c1f7a44edb0a190a46) )
	ROM_LOAD( "1175a.u29",      0x1000, 0x0400, CRC(116517b8) SHA1(a7ded7cb53735e6cf4994ff70db35dead04828e6) )
	ROM_LOAD( "1176a.u28",      0x1400, 0x0400, CRC(2b2c4ba8) SHA1(fe9ccb94b9d5d7fb9ec6170a7b71f859b22981d3) )
	ROM_LOAD( "1177a.u27",      0x1800, 0x0400, CRC(d8cbcc1e) SHA1(632d2ba84d4276155960b176bf7ac514b214e481) )
	ROM_LOAD( "1178a.u26",      0x1c00, 0x0400, CRC(05b1e3ea) SHA1(38657eb1df334dd66e66d7a1ec5cbe663982e12b) )
	ROM_LOAD( "1179a.u8",       0x2000, 0x0400, CRC(c2dc3181) SHA1(8bd639a8c1d86d6432b9115195b5f394898c936a) )
	ROM_LOAD( "1180a.u7",       0x2400, 0x0400, CRC(c00543a7) SHA1(21df4f31c8070c76f592b8c231fc7b3b192fc20f) )
	ROM_LOAD( "1181a.u6",       0x2800, 0x0400, CRC(aba9ca30) SHA1(2d9d7a13adb21c71f0094c3c0518f995423fabe5) )
	ROM_LOAD( "1182a.u5",       0x2c00, 0x0400, CRC(fe7cfc31) SHA1(93789ffbf9bd3a825dacb551a745ed1154de1773) )
	ROM_LOAD( "1183a.u4",       0x3000, 0x0400, CRC(4e0684cd) SHA1(b1442a5f7b509f2a1775359174ecdf5e90df8150) )
	ROM_LOAD( "1184a.u3",       0x3400, 0x0400, CRC(0f38ca4c) SHA1(1e5e19eae33c258f25495562f542aed38bba34e7) )
	ROM_LOAD( "1185a.u2",       0x3800, 0x0400, CRC(1dff2ab0) SHA1(aab3482b15d1401897feabd4085b16acd5413c8f) )
	ROM_LOAD( "1186a.u1",       0x3c00, 0x0400, CRC(5828ca5a) SHA1(cffa6dc6baa8a53bd165c40d1fc96c590c6e485c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u49.bin",      0x0000, 0x0020, CRC(0a2156b3) SHA1(504abe8e253ff9b12ac6ffacd92722f8ee8a30ae) )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs */
	ROM_LOAD( "pr-52.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )
ROM_END

/*
Star Raker

Notes from dumper:
There is a mainboard and a small board. The main board is a normal VIC board and is from a working cab we own.
From an op we got a box with a similar board plus a small board which I assumed belongs to it,
but I have no idea what its purpose is. Its not the soundboard and its not included in our working Star Raker cab.
So maybe it belongs to a different game, but I had it dumped anyway.
*/

ROM_START( starrkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-767.u33", 0x0000, 0x0400, CRC(2cfe979c) SHA1(a64f7035788ed428bc5f3ca32e8b1208c378dea9) )
	ROM_LOAD( "epr-768.u32", 0x0400, 0x0400, CRC(cf85f158) SHA1(f091fdc30b10a202318a0c39af450f765b905528) )
	ROM_LOAD( "epr-769.u31", 0x0800, 0x0400, CRC(22ac6362) SHA1(de60ea8480ada2b029956d65f9edbf680efae846) )
	ROM_LOAD( "epr-770.u30", 0x0c00, 0x0400, CRC(d8d2fc6a) SHA1(950f60f3ef11345a08face4d421a678462be917c) )
	ROM_LOAD( "epr-771.u29", 0x1000, 0x0400, CRC(9a88d577) SHA1(aac34dffda10e513e8c656983de42387167b7e92) )
	ROM_LOAD( "epr-772.u28", 0x1400, 0x0400, CRC(bab1574f) SHA1(cf569545383e7b5080b2a363cbc5f90fc3a5f84e) )
	ROM_LOAD( "epr-773.u27", 0x1800, 0x0400, CRC(c2406abd) SHA1(be1d31686a5192c4e72b549ee44160a65ac1a3b1) )
	ROM_LOAD( "epr-774.u26", 0x1c00, 0x0400, CRC(77686d3b) SHA1(16b8f8ea1bfdbe684113c96128c6324ce63ef4f3) )
	ROM_LOAD( "epr-775.u8",  0x2000, 0x0400, CRC(1d00b276) SHA1(d4f850288c0757c937583b1cd80ca540c29fb84d) )
	ROM_LOAD( "epr-776.u7",  0x2400, 0x0400, CRC(7215a72b) SHA1(bbf1f7e291f26cd8379679bab2fd1890b1c7524a) )
	ROM_LOAD( "epr-777.u6",  0x2800, 0x0400, CRC(59176c4c) SHA1(22b9cf5661eb0660039d6d36fb57b0a187b202da) )
	ROM_LOAD( "epr-778.u5",  0x2c00, 0x0400, CRC(b4586631) SHA1(82dbbcf991c4ffda70518401d5cc700da14afb9c) )
	ROM_LOAD( "epr-779.u4",  0x3000, 0x0400, CRC(1f9a736d) SHA1(629bf6da13278cb521d291d0551effba1f415027) )
	ROM_LOAD( "epr-780.u3",  0x3400, 0x0400, CRC(01d89786) SHA1(ccc5644ff6c7f577db4ac4b66a5cd6eb1ff66c1e) )
	ROM_LOAD( "epr-781.u2",  0x3800, 0x0400, CRC(7d1238a2) SHA1(b9195608b0255ddd4c4a03d863f7749a4ee9f706) )
	ROM_LOAD( "epr-782.u1",  0x3c00, 0x0400, CRC(121ce164) SHA1(9d046ad189a3c009547eb775028143e2fffe243d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-23.u49",   0x0000, 0x0020, CRC(0a2156b3) SHA1(504abe8e253ff9b12ac6ffacd92722f8ee8a30ae) )

	ROM_REGION( 0x0800, "user1", 0 )    /* misc PROM */
	ROM_LOAD( "pr-33.u15",  0x0000, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )
	ROM_LOAD( "pr-34.u14",  0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )
	/* following from Small PCB (#97270-P) */
	ROM_LOAD( "pr-58.5",    0x0000, 0x0800, CRC(526ed9d8) SHA1(173a05b7e01147e415b92ec66661f2544dce0ffd) )
	ROM_LOAD( "pr-60.6",    0x0000, 0x0800, CRC(59e6067f) SHA1(e6bd08e23ba6c140fca2b280e7d39ac1092d3926) )
	ROM_LOAD( "pr-59.12",   0x0000, 0x0800, CRC(a2e8090a) SHA1(61d8133d4469243a6a1bbeb851e51c73b22bc3e1) )
	ROM_LOAD( "pr-61.13",   0x0000, 0x0800, CRC(fc663474) SHA1(9fd28575606e19e84abbc63fb66b85b7edc5bc99) )
	ROM_LOAD( "pr-65.17",   0x0000, 0x0800, CRC(a12430b2) SHA1(2df1fc2a0e5afcb41e4b0b1cbe7927d7ae8b5146) )
	ROM_LOAD( "pr-63.18",   0x0000, 0x0800, CRC(b3297499) SHA1(67a9e22c80627fe1924112774201add339f40c62) )
	ROM_LOAD( "pr-64.25",   0x0000, 0x0800, CRC(7342cf53) SHA1(761aa5a38a28c044cbbbe66e3a8a2f47c493d56d) )
	ROM_LOAD( "pr-62.26",   0x0000, 0x0800, CRC(d352c545) SHA1(6da4f7a7974e2f471b081d230a47767315b2f1a7) )
	ROM_LOAD( "pr-66.28",   0x0000, 0x0800, CRC(895c5733) SHA1(881a274cdcf23292ea658dcab793303cfb445e51) )

	ROM_REGION( 0x0400, "user2", 0 ) /* sound ROM */
	ROM_LOAD( "epr-613.1",   0x0000, 0x0400, CRC(ff4be0c7) SHA1(7311c34aa88f6ba905a01e7a9f2ed99a0353a06b) )
ROM_END

/*
Notes on Sidam set

This set isn't really very interesting, it just has the code to draw the status bar text patched out.
Only one ROM is changed.  Supported because Sidam were a well known Italian bootlegger so this has
some historical importance.  'Sidam' isn't displayed anywhere ingame, only on the PCB?.

---CPU

Lower board (label "BLC200681"):
1x MK3880-Z80CPU (main)
1x oscillator 10.000MHz

---ROMs:

Lower board (label "BLC200681"):
16x F2708
3x PROM 5610

Upper board (label "BLC300681 MADE IN ITALY"):
1x F2708
1x PROM 82S123
2x PROM 93427

---Note:

Lower board (label "BLC200681"):
1x 22x2 edge connector
1x 8 switches dip

Upper board (label "BLC300681 MADE IN ITALY"):
7x trimmer
*/

ROM_START( brdrlins )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.33",       0x0000, 0x0400, CRC(df182769) SHA1(2b1b70c6282b32e0a4ed80ab4e6b20f90630e910) )
	ROM_LOAD( "2.32",       0x0400, 0x0400, CRC(98b26e2a) SHA1(ae1c3726827571ee1c2716e1044d6aae5608c0af) ) // 3 bytes changed in this rom
	ROM_LOAD( "3.31",       0x0800, 0x0400, CRC(4ec4afa2) SHA1(dd5b97f1a37cd655064b773e4a755b87de4c6a3f) )
	ROM_LOAD( "4.30",       0x0c00, 0x0400, CRC(88de95f6) SHA1(fe3388346785ad15dead89913e4ff36120a83599) )
	ROM_LOAD( "5.29",       0x1000, 0x0400, CRC(2e4e13b9) SHA1(bdf31c11733127b8b77fa72933d3b9dc6834d5d8) )
	ROM_LOAD( "6.28",       0x1400, 0x0400, CRC(c181e87a) SHA1(426e1ce15477039e4a19b536500f387518026efc) )
	ROM_LOAD( "7.27",       0x1800, 0x0400, CRC(21180015) SHA1(b23f876db1a9a986f1087ead07a01e836d5ee842) )
	ROM_LOAD( "8.26",       0x1c00, 0x0400, CRC(56a7fee0) SHA1(495efa91773fd3cf36da4e538893db08e64e5bab) )
	ROM_LOAD( "9.8",        0x2000, 0x0400, CRC(bb532e63) SHA1(da511e0be58b13781897e6efb5a59a3558016b12) )
	ROM_LOAD( "10.7",       0x2400, 0x0400, CRC(64793709) SHA1(fabfb783f1d93a3d9454fc345a64498e4b5b9138) )
	ROM_LOAD( "11.6",       0x2800, 0x0400, CRC(2ae2f928) SHA1(afd99c800801d38ee59008344bd9a3901f72ff50) )
	ROM_LOAD( "12.5",       0x2c00, 0x0400, CRC(e14cfaf5) SHA1(d159e93f703aae3c04da08102ff718d5a4ca7a91) )
	ROM_LOAD( "13.4",       0x3000, 0x0400, CRC(605e0d27) SHA1(771de6d31ee7896a2441f1df4565027793d99989) )
	ROM_LOAD( "14.3",       0x3400, 0x0400, CRC(93f5714f) SHA1(50a043be1e1cf8b1aeb846571a12fe70cbb3477e) )
	ROM_LOAD( "15.2",       0x3800, 0x0400, CRC(2f8a9b1c) SHA1(853d5ca017b133c1f13f703cceb20f04199d4752) )
	ROM_LOAD( "16.1",       0x3c00, 0x0400, CRC(cc138bed) SHA1(7d3eebdeaff19783d5ef20a7ececec00773434fc) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "5610.49",    0x0000, 0x0020, CRC(bc6be94e) SHA1(34e113ec25e19212b74907d35be5cb8714a8249c) )

	ROM_REGION( 0x0100, "user1", 0 )    /* misc PROM */
	ROM_LOAD( "82s123.bin", 0x0000, 0x0020, CRC(c128d0ba) SHA1(0ce9febbb7e2f5388ed999a479e3d385dba0b342) )
	ROM_LOAD( "5610.15",    0x0000, 0x0020, CRC(6449e678) SHA1(421c45c8fba3c2bc2a7ebbea2c837c8fa1a5a2f3) )
	ROM_LOAD( "5610.14",    0x0000, 0x0020, CRC(55dcdef1) SHA1(6fbd041edc258b7e1b99bbe9526612cfb1b541f8) )
	/* following 2 from sound board */
	ROM_LOAD( "93427.1",    0x0000, 0x0100, CRC(64b98dc7) SHA1(f0bb7d0b4b56cc2936ce4cbec165394f3026ed6d) )
	ROM_LOAD( "93427.2",    0x0000, 0x0100, CRC(bda82367) SHA1(1c96453c2ae372892c39b5657cf2b252a90a10a9) )

	ROM_REGION( 0x0400, "user2", 0 ) /* sound ROM */
	ROM_LOAD( "au.bin",     0x0000, 0x0400, CRC(a23e1d9f) SHA1(ce209571f6341aa6f036a015e666673098bc98ea) )
ROM_END

ROM_START( brdrlinb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "border1.33",   0x0000, 0x0800, CRC(48387706) SHA1(b4db2f05e722812370b0b24cd15061d6fc578560) ) // karateco
	ROM_LOAD( "border2.30",   0x0800, 0x0800, CRC(1d669b60) SHA1(47ef5141591e177419fa352968be26ecc6fafd89) )
	ROM_LOAD( "border3.29",   0x1000, 0x0800, CRC(6e4d6fb3) SHA1(4747359c4f09e93c563b93ee1189743894332b47) )
	ROM_LOAD( "border4.27",   0x1800, 0x0800, CRC(718446d8) SHA1(cc15cb37ebb51970fcdebf74ab4308a25b40af2a) )
	ROM_LOAD( "border5.08",   0x2000, 0x0800, CRC(a0584337) SHA1(1fd9c60dc42a178c88d4e4e4b4d5de495ea906c6) )
	ROM_LOAD( "border6.06",   0x2800, 0x0800, CRC(cb30fb98) SHA1(96ea83111e8ccf409fe40f19ce200a50ea9ea288) )
	ROM_LOAD( "border7.04",   0x3000, 0x0800, CRC(200c5321) SHA1(f9faa6125dd2adc69c4ba9d962c0998f815ccd1c) )
	ROM_LOAD( "border8.02",   0x3800, 0x0800, CRC(735e140d) SHA1(1c0b6cf2d8c88601084dfcb8d7490b85ef1a86d5) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "borderc.49",   0x0000, 0x0020, CRC(bc6be94e) SHA1(34e113ec25e19212b74907d35be5cb8714a8249c) )

	ROM_REGION( 0x0020, "user1", 0 )    /* misc PROM */
	ROM_LOAD( "border.32",   0x0000, 0x0020, CRC(c128d0ba) SHA1(0ce9febbb7e2f5388ed999a479e3d385dba0b342) )
	ROM_LOAD( "bordera.15",  0x0000, 0x0020, CRC(6449e678) SHA1(421c45c8fba3c2bc2a7ebbea2c837c8fa1a5a2f3) )
	ROM_LOAD( "borderb.14",  0x0000, 0x0020, CRC(55dcdef1) SHA1(6fbd041edc258b7e1b99bbe9526612cfb1b541f8) )

	ROM_REGION( 0x0400, "user2", 0 ) /* sound ROM */
	ROM_LOAD( "bords.bin",    0x0000, 0x0400, CRC(a23e1d9f) SHA1(ce209571f6341aa6f036a015e666673098bc98ea) )
ROM_END

ROM_START( startrks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.1a",         0x0000, 0x0400, CRC(2ba4202a) SHA1(1e46e36c37c9f1cecb505a1fe393c24d0e271168) )
	ROM_LOAD( "1.3a",         0x0400, 0x0400, CRC(cf6081b8) SHA1(2294cf6849a8191928f268ba1dbef21659a1475f) )
	ROM_LOAD( "2.4a",         0x0800, 0x0400, CRC(fd983c0c) SHA1(a4aea8707878dcac8b4e3d9acad94f60eee956c6) )
	ROM_LOAD( "3.6a",         0x0c00, 0x0400, CRC(607991c7) SHA1(abfbfdbcedc7192a0cf84f2d28c600fe6f7c7a84) )
	ROM_LOAD( "4.8a",         0x1000, 0x0400, CRC(043bf767) SHA1(76dd79a4cdd23f75a4e7b5642db05e94e0d2b75e) )
	ROM_LOAD( "5.9a",         0x1400, 0x0400, CRC(2aa21da3) SHA1(85bb7b74b3df678bf32370ef77ee620c5d7249ab) )
	ROM_LOAD( "6.11a",        0x1800, 0x0400, CRC(a5315dc8) SHA1(ba6d27d03c9f0100fe89ebbf4b58d166bf259fa8) )

	ROM_REGION( 0x0040, "user1", 0 )    /* timing PROMs */
	ROM_LOAD( "82s123.15c", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "82s123.14c", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( nostromo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ns_1.bin", 0x0000, 0x0400, CRC(2ba4202a) SHA1(1e46e36c37c9f1cecb505a1fe393c24d0e271168) )
	ROM_LOAD( "ns_2.bin", 0x0400, 0x0400, CRC(cf6081b8) SHA1(2294cf6849a8191928f268ba1dbef21659a1475f) )
	ROM_LOAD( "ns_3.bin", 0x0800, 0x0400, CRC(fd983c0c) SHA1(a4aea8707878dcac8b4e3d9acad94f60eee956c6) )
	ROM_LOAD( "ns_4.bin", 0x0c00, 0x0400, CRC(79e1dc92) SHA1(53fcf9b7b7e894122419b43adccb1aaf7dab8bf9) )
	ROM_LOAD( "ns_5.bin", 0x1000, 0x0400, CRC(4deb2ce3) SHA1(2935a4ab4be0190e8efa560e30fa7349d83d3e69) )
	ROM_LOAD( "ns_6.bin", 0x1400, 0x0400, CRC(e4ddf052) SHA1(8734c87742a1d0b68e679e1650806d0738c432c6) )
	ROM_LOAD( "ns_7.bin", 0x1800, 0x0400, CRC(a5315dc8) SHA1(ba6d27d03c9f0100fe89ebbf4b58d166bf259fa8) )

	ROM_REGION( 0x0040, "user1", 0 )    // timing PROMs, not dumped for this set but probably same as startrks given how close they are
	ROM_LOAD( "82s123.15c", 0x0000, 0x0020, BAD_DUMP CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    // control PROM
	ROM_LOAD( "82s123.14c", 0x0020, 0x0020, BAD_DUMP CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    // sequence PROM
ROM_END

ROM_START( digger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "684.u27",      0x0000, 0x0400, CRC(bba0d7c2) SHA1(1e55dd95b07b562dcc1e52ecf9460d302b14ee60) )
	ROM_LOAD( "685.u26",      0x0400, 0x0400, CRC(85210d8b) SHA1(8260ca809a3a20a52b146d357253aa958d08887e) )
	ROM_LOAD( "686.u25",      0x0800, 0x0400, CRC(2d87238c) SHA1(de8dbe56c4c71b5d75e77c39cfbd771b91c0db0f) )
	ROM_LOAD( "687.u24",      0x0c00, 0x0400, CRC(0dd0604e) SHA1(d357e195a6e61615b0e8cfb027628c2ce5f2b1c5) )
	ROM_LOAD( "688.u23",      0x1000, 0x0400, CRC(2f649667) SHA1(f4c08c7c8f1e9cee84bf844810f006d27bd35025) )
	ROM_LOAD( "689.u22",      0x1400, 0x0400, CRC(89fd63d9) SHA1(413f5df7eedb67848119c675e4edd1ce211ded24) )
	ROM_LOAD( "690.u21",      0x1800, 0x0400, CRC(a86622a6) SHA1(44c0712cc1e11255dc0e27f4754984885c9fa8ad) )
	ROM_LOAD( "691.u20",      0x1c00, 0x0400, CRC(8aca72d8) SHA1(59306dbc350dc64a17ad2f9909ef00e9860c56e7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-507",      0x0000, 0x0020, CRC(fdb22e8f) SHA1(b09241b532cfe7679e837e9f3e5956cfc588a0be) )

	ROM_REGION( 0x0020, "user1", 0 )    /* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( pulsar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "790.u33",      0x0000, 0x0400, CRC(5e3816da) SHA1(83f019fa3598e383310b4c21441e4f8ef0c9d4fb) )
	ROM_LOAD( "791.u32",      0x0400, 0x0400, CRC(ce0aee83) SHA1(f3755592a9aaa2d493d017c8da19354fd5598860) )
	ROM_LOAD( "792.u31",      0x0800, 0x0400, CRC(72d78cf1) SHA1(d292d80826081073d279d163a107926fa80f02d0) )
	ROM_LOAD( "793.u30",      0x0c00, 0x0400, CRC(42155dd4) SHA1(d3032b0509eb3aa9c49b92ae69971ec019310b16) )
	ROM_LOAD( "794.u29",      0x1000, 0x0400, CRC(11c7213a) SHA1(417a608913e84c456f3181e94e496b05599dfb14) )
	ROM_LOAD( "795.u28",      0x1400, 0x0400, CRC(d2f02e29) SHA1(2b4333ecb89abd80634686b9ef667213423bc4a8) )
	ROM_LOAD( "796.u27",      0x1800, 0x0400, CRC(67737a2e) SHA1(6e8616bb0fba603fec9d97a1852986390db67b0a) )
	ROM_LOAD( "797.u26",      0x1c00, 0x0400, CRC(ec250b24) SHA1(b8bbdc888008d072672024fced55fcf3da668532) )
	ROM_LOAD( "798.u8",       0x2000, 0x0400, CRC(1d34912d) SHA1(32b10bc9617fa14b9e98bd572c86bba495f64d18) )
	ROM_LOAD( "799.u7",       0x2400, 0x0400, CRC(f5695e4c) SHA1(77251f917802d01d76a9a6cf8bb6c12919803388) )
	ROM_LOAD( "800.u6",       0x2800, 0x0400, CRC(bf91ad92) SHA1(ebadcb9a6d2ee5e4b432f132095fac8d72e05a34) )
	ROM_LOAD( "801.u5",       0x2c00, 0x0400, CRC(1e9721dc) SHA1(ddb94275c8fd26f7323b89d234b34883737f8738) )
	ROM_LOAD( "802.u4",       0x3000, 0x0400, CRC(d32d2192) SHA1(334c10dd418fe320fc9a689c798e90787aaae7bf) )
	ROM_LOAD( "803.u3",       0x3400, 0x0400, CRC(3ede44d5) SHA1(3967fc6dc25b5872ae1ad389a31257b39879e13d) )
	ROM_LOAD( "804.u2",       0x3800, 0x0400, CRC(62847b01) SHA1(79763623df6eb7d189a0b0583ecc12c88602a44d) )
	ROM_LOAD( "805.u1",       0x3c00, 0x0400, CRC(ab418e86) SHA1(ad0dfd982b3bd3943aaf670d48485218f1cc4998) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0789.u49", 0x0000, 0x0020, CRC(7fc1861f) SHA1(e005a8afd6b9e6b7d4ddf362c204e472b80582c6) )

	ROM_REGION( 0x0020, "user1", 0 )    /* misc PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )    /* control PROM */
ROM_END

ROM_START( heiankyo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ha16.u33",     0x0000, 0x0400, CRC(1eec8b36) SHA1(55644cfeb7a9d64e52f11611c91c6186038772a3) )
	ROM_LOAD( "ha15.u32",     0x0400, 0x0400, CRC(c1b9a1a5) SHA1(068ad2da4852a50c948c4f9b3e1b1aa5c5bf5ca5) )
	ROM_LOAD( "ha14.u31",     0x0800, 0x0400, CRC(5b7b582e) SHA1(078b8b7d1836cc31cee244a58fb6a6a50135f833) )
	ROM_LOAD( "ha13.u30",     0x0c00, 0x0400, CRC(4aa67e01) SHA1(5539a028cb1935bb4d6ab747c92792f5462add1f) )
	ROM_LOAD( "ha12.u29",     0x1000, 0x0400, CRC(75889ca6) SHA1(552bcf976f31d7b634b79175c0470978b6b82433) )
	ROM_LOAD( "ha11.u28",     0x1400, 0x0400, CRC(d469226a) SHA1(dfca01d956e12162fab261a017c727a756b67206) )
	ROM_LOAD( "ha10.u27",     0x1800, 0x0400, CRC(4e203074) SHA1(1a80c396ceb9a2b1c737e1af791dbab2bee10ce5) )
	ROM_LOAD( "ha9.u26",      0x1c00, 0x0400, CRC(9c3a3dd2) SHA1(afcd85ec0174bdcab31135b4e271cec1eb75fd02) )
	ROM_LOAD( "ha8.u8",       0x2000, 0x0400, CRC(6cc64878) SHA1(4d03ff925d80835c27512b3bd04ea57f91b4491f) )
	ROM_LOAD( "ha7.u7",       0x2400, 0x0400, CRC(6d2f9527) SHA1(4e51c5404d0302547c1ae85b27ffe4de11d68224) )
	ROM_LOAD( "ha6.u6",       0x2800, 0x0400, CRC(e467c353) SHA1(a76b4f6d9702f760f287b5285f76ea4206c6934a) )
	ROM_LOAD( "ha3.u3",       0x2c00, 0x0400, CRC(6a55eda8) SHA1(f526ebf18a33271b798e53cfcadb27e4c3a03466) )
	ROM_FILL(                 0x3000, 0x0400, 0x00 )
	ROM_FILL(                 0x3400, 0x0400, 0x00 )
	ROM_LOAD( "ha2.u2",       0x3800, 0x0400, CRC(056b3b8b) SHA1(3cce6c928599604ffdcdb767caa7b32d8ec1e03d) )
	ROM_LOAD( "ha1.u1",       0x3c00, 0x0400, CRC(b8da2b5e) SHA1(70d97b89cb3162bd479203c53148319179a9873f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-138.u49",  0x0010, 0x0010, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )
	ROM_CONTINUE(             0x0000, 0x0010 )

	ROM_REGION( 0x0040, "user1", 0 )    /* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )    /* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )    /* sequence PROM */
ROM_END

ROM_START( alphaho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c0.bin",       0x0000, 0x0400, CRC(db774c23) SHA1(c5042872110ae8d0c5c7629892a16b87e8f19d96) )
	ROM_LOAD( "c1.bin",       0x0400, 0x0400, CRC(b63f4695) SHA1(95b3ca96ca48f2c525eaf2b49956248e46686688) )
	ROM_LOAD( "c2.bin",       0x0800, 0x0400, CRC(4ebf0ba4) SHA1(b6651f7424fd5e62422b45ccce118db64dab50bf) )
	ROM_LOAD( "c3.bin",       0x0c00, 0x0400, CRC(126f17ec) SHA1(a78b9f62d76305903ad56ebf995ba0745774e6f2) )
	ROM_LOAD( "c4.bin",       0x1000, 0x0400, CRC(52798c61) SHA1(c01c11b99f56ca7f5d8824a3b9f795b2075d21a0) )
	ROM_LOAD( "c5.bin",       0x1400, 0x0400, CRC(4827cb36) SHA1(a5f56aa73147503fb6c5a3f38489754d8d27f257) )
	ROM_LOAD( "c6.bin",       0x1800, 0x0400, CRC(8b2ff47e) SHA1(43c84fe40180c42ade3c77790aeadeeb28e5c5cd) )
	ROM_LOAD( "c7.bin",       0x1c00, 0x0400, CRC(44921df4) SHA1(3ece06f20330aebe2770bf74142cd5e7bc5297ec) )
	ROM_LOAD( "c8.bin",       0x2000, 0x0400, CRC(9fb12fca) SHA1(a5b49ddd86be44b9220cc4ceb84b32c55e5d432b) )
	ROM_LOAD( "c9.bin",       0x2400, 0x0400, CRC(e5f622f7) SHA1(57858b6abbf34fc4ab2b19a469cbd945a0e14a0e) )
	ROM_LOAD( "ca.bin",       0x2800, 0x0400, CRC(82b28e77) SHA1(c4f425daa02acbc19d4ecee8bd38d8bb338e085f) )
	ROM_LOAD( "cb.bin",       0x2c00, 0x0400, CRC(94fba0ad) SHA1(c623368c710ddf8ff05e09c963eeb863c9951df3) )
	ROM_LOAD( "cc.bin",       0x3000, 0x0400, CRC(de338b6d) SHA1(a465aaac041e4aa017afb9821dd049ac4950ba92) )
	ROM_LOAD( "cd.bin",       0x3400, 0x0400, CRC(be76baac) SHA1(4b6a46c9484cfc90fa405f8568df44cfc96b1d7a) )
	ROM_LOAD( "ce.bin",       0x3800, 0x0400, CRC(3c409d57) SHA1(fda00b4eaaa187f489a8957952005e08b2b7ba40) )
	ROM_LOAD( "cf.bin",       0x3c00, 0x0400, CRC(d03c5a09) SHA1(eebc1d2302bd2bae28c25187bb099ae618c5cd05) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "alphaho.col", 0x0000, 0x0020, BAD_DUMP CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) ) // wasn't dumped for this set, use the one from sspacaho (the game Data East cloned and modified) for now
ROM_END

ROM_START( alphahob ) // PCB marked NNS
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "03.bin", 0x0000, 0x0400, CRC(87de1bb0) SHA1(63626ac2b1f1419fd93dca33417834922ffc8634) )
	ROM_LOAD( "13.bin", 0x0400, 0x0400, CRC(bd6dbd8e) SHA1(a001392b32be8218d5aebb1c02fc88a41ce995b0) )
	ROM_LOAD( "16.bin", 0x0800, 0x0400, CRC(4c043d8b) SHA1(72f7db24fc60dae9879d8daf01ef2bb14e9597aa) )
	ROM_LOAD( "04.bin", 0x0c00, 0x0400, CRC(10005956) SHA1(2e742b1764520c3de5b44018f94dc6476c9a4e84) )
	ROM_LOAD( "05.bin", 0x1000, 0x0400, CRC(bbdaf42a) SHA1(57e5e72613c0852bc91e12a5713d93c222ed5b32) )
	ROM_LOAD( "06.bin", 0x1400, 0x0400, CRC(07d4c69a) SHA1(7bb7dad200526a642debe3406ef9c0a74f6fc374) )
	ROM_LOAD( "07.bin", 0x1800, 0x0400, CRC(8f766b30) SHA1(91e4c8ccb479dcd89f1c76f619bfac832ca7910e) )
	ROM_LOAD( "08.bin", 0x1c00, 0x0400, CRC(88032202) SHA1(64e087c0a9a97f19c1c5ba5ca1d249d38efab391) )
	ROM_LOAD( "01.bin", 0x2000, 0x0400, CRC(eb8278b3) SHA1(6349c64f718fc23f78788a612378681d9e6ef69b) )
	ROM_LOAD( "09.bin", 0x2400, 0x0400, CRC(e5f622f7) SHA1(57858b6abbf34fc4ab2b19a469cbd945a0e14a0e) )
	ROM_LOAD( "11.bin", 0x2800, 0x0400, CRC(5673be86) SHA1(d8f6bcc0231163c40c217e73dff9dd410fcd488c) )
	ROM_LOAD( "12.bin", 0x2c00, 0x0400, CRC(939903c6) SHA1(2a570b9880bb8594501fa9b3d1320e5ad2abc86a) )
	ROM_LOAD( "02.bin", 0x3000, 0x0400, CRC(404f120c) SHA1(7359c819f8a355783c00a972cb65a10aeff6bde4) )
	ROM_LOAD( "14.bin", 0x3400, 0x0400, CRC(be76baac) SHA1(4b6a46c9484cfc90fa405f8568df44cfc96b1d7a) )
	ROM_LOAD( "15.bin", 0x3800, 0x0400, CRC(ced0a642) SHA1(7f90803af926861261c9467fda293093409aff83) )
	ROM_LOAD( "10.bin", 0x3c00, 0x0400, CRC(ba978bca) SHA1(69ee157d1aee8cf323d15630b99cff2747ed0189) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.u49", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "timing_proms", 0 )
	ROM_LOAD( "prom.u14", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) ) // control PROM
	ROM_LOAD( "prom.u15", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) ) // sequence PROM
ROM_END

// Wanted uses 2 sound boards. One from Head On and one from Space Attack. The sound board has the name printed on the bottom
// together with the Sega Gremlin logo and other numbers.

ROM_START( wantsega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4-19_1_9afc.u33",       0x0000, 0x0800, CRC(fc2ccf14) SHA1(345ed91ce74615a89b4ea0a5dc38489b36a4cc6e) )
	ROM_LOAD( "4-19_2_d170.u32",       0x0800, 0x0800, CRC(c1c59576) SHA1(5174c769390438d05682a4999439b2f5659cbda8) )
	ROM_LOAD( "4-19_3_f291.u31",       0x1000, 0x0800, CRC(862b7303) SHA1(8e89311ca23ca89d258744de393ecd88fc8d3bfb) )
	ROM_LOAD( "4-19_4_a49b.u30",       0x1800, 0x0800, CRC(0dfab42b) SHA1(723bda17dfaeec90545a0e8ef9ac83e5d92eec1e) )
	ROM_LOAD( "4-19_5_7b35.u29",       0x2000, 0x0800, CRC(530b9a43) SHA1(af07c2dbfe45c10ef3bd06efe56f780b6afbc9fc) )
	ROM_LOAD( "4-19_6_bc98.u28",       0x2800, 0x0800, CRC(d038b76e) SHA1(b72031b1a6010d70a1db66fc825cb9046fb98330) )
	ROM_LOAD( "4-19_7_f852.u27",       0x3000, 0x0800, CRC(e1bf29bd) SHA1(541e9e83f05e764f9bb6026ba21e15627a9f3cb7) )
	ROM_LOAD( "4-19_8_35df.u26",       0x3800, 0x0800, CRC(1b8a52cf) SHA1(59ecff36406191a5b92892147f6d84d9a501b697) )
	ROM_LOAD( "4-19_9_ffa9.u8",        0x4000, 0x0800, CRC(bd84a06a) SHA1(c60f6c915f01f8f474bcb2f4ebb65be06c51b7f8) )
	ROM_LOAD( "4-19_10_b2be.u7",       0x4800, 0x0800, CRC(bd321dd9) SHA1(25e18f4a35ece041a20e5dc1cd6bc529d3ba2719) )
	ROM_LOAD( "4-19_11_d8fb.u6",       0x5000, 0x0800, CRC(2cb222ca) SHA1(74717c194cdf07dd5b6d99552881a64ab7ef4886) )
	ROM_LOAD( "4-19_12_0bc6.u5",       0x5800, 0x0800, CRC(390e3c32) SHA1(1fd219bf8c09472c337b2f33052092eb8c970f5d) )
	ROM_LOAD( "4-19_13_e95d.u4",       0x6000, 0x0800, CRC(3779e91c) SHA1(0b338ba1558d4b7b1faa7fb1354b39764b1a43cb) )
	ROM_LOAD( "4-19_14_dfca.u3",       0x6800, 0x0800, CRC(7e70b2f7) SHA1(d3986716e8447f258186fafd1a95ce9c52f61033) )
	ROM_LOAD( "4-19_15_7032.u2",       0x7000, 0x0800, CRC(46b71a75) SHA1(a512ebee6020fd2ccbff9342a956131911177b08) )
	ROM_LOAD( "4-19_16_0000.u1",       0x7800, 0x0800, CRC(f1e8ba9e) SHA1(605db3fdbaff4ba13729371ad0c4fbab3889378e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u49",  0x0000, 0x0020, CRC(3470a4b6) SHA1(8ccca436bf24246729ac62d196087704d4d3310c) )

	ROM_REGION( 0x0040, "timing_proms", 0 )
	ROM_LOAD( "pr3_82s123.u14",   0x0000, 0x0020, CRC(c6452647) SHA1(9a9e21bc578523106a6bf7270b37c453c7098f35) )
	ROM_LOAD( "pr33_82s123.u15",  0x0020, 0x0020, CRC(98579402) SHA1(1a5fa9b31aba83408f255e64799fd6bdd51d647e) )
ROM_END

/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR  NAME        PARENT    MACHINE    INPUT      CLASS            INIT        ORIENT. COMPANY,                    FULLNAME,                                                FLAGS
GAMEL(1977, depthch,    0,        depthch,   depthch,   vicdual_state,   empty_init, ROT0,   "Gremlin",                 "Depthcharge",                                            MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_depthch )
GAMEL(1977, depthcho,   depthch,  depthch,   depthch,   vicdual_state,   empty_init, ROT0,   "Gremlin",                 "Depthcharge (older)",                                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_depthch )
GAMEL(1977, subhunt,    depthch,  depthch,   depthch,   vicdual_state,   empty_init, ROT0,   "Gremlin (Taito license)", "Sub Hunter (Gremlin / Taito)",                           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_depthch )
GAME( 1977, safari,     0,        safari,    safari,    vicdual_state,   empty_init, ROT0,   "Gremlin",                 "Safari (set 1)",                                         MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1977, safaria,    safari,   safari,    safari,    vicdual_state,   empty_init, ROT0,   "Gremlin",                 "Safari (set 2, bootleg?)",                               MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // on a bootleg board, but seems a different code revision too
GAME( 1978, frogs,      0,        frogs,     frogs,     vicdual_state,   empty_init, ROT0,   "Gremlin",                 "Frogs",                                                  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, sspaceat,   0,        sspaceat,  sspaceat,  vicdual_state,   empty_init, ROT270, "Sega",                    "Space Attack (upright set 1)",                           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, sspaceat2,  sspaceat, sspaceat,  sspaceat,  vicdual_state,   empty_init, ROT270, "Sega",                    "Space Attack (upright set 2)",                           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, sspaceat3,  sspaceat, sspaceat,  sspaceat,  vicdual_state,   empty_init, ROT270, "Sega",                    "Space Attack (upright set 3)",                           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, sspaceatc,  sspaceat, sspaceat,  sspaceat,  vicdual_state,   empty_init, ROT270, "Sega",                    "Space Attack (cocktail)",                                MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, sspacaho,   0,        sspacaho,  sspacaho,  vicdual_state,   empty_init, ROT270, "Sega",                    "Space Attack / Head On",                                 MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headon,     0,        headon,    headon,    vicdual_state,   empty_init, ROT0,   "Gremlin",                 "Head On (2 players)",                                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headon1,    headon,   headon,    headon,    vicdual_state,   empty_init, ROT0,   "Gremlin",                 "Head On (1 player)",                                     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headonn,    headon,   headonn,   headonn,   vicdual_state,   empty_init, ROT270, "Nintendo",                "Head On N",                                              MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headons,    headon,   headons,   headons,   vicdual_state,   empty_init, ROT0,   "bootleg (Sidam)",         "Head On (Sidam bootleg, set 1)",                         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headonsa,   headon,   headons,   headonsa,  headonsa_state,  empty_init, ROT0,   "bootleg (Sidam)",         "Head On (Sidam bootleg, set 2)",                         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headonmz,   headon,   headon,    headonmz,  vicdual_state,   empty_init, ROT0,   "bootleg",                 "Head On (bootleg, alt maze)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1979, supcrash,   headon,   headons,   supcrash,  vicdual_state,   empty_init, ROT0,   "bootleg (VGG)",           "Super Crash (bootleg of Head On)",                       MACHINE_NO_SOUND  | MACHINE_SUPPORTS_SAVE )
GAME( 1979, hocrash,    headon,   headons,   hocrash,   vicdual_state,   empty_init, ROT0,   "bootleg (Fraber)",        "Crash (bootleg of Head On)",                             MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, bumba,      headon,   headons,   headons,   vicdual_state,   empty_init, ROT0,   "bootleg (Niemer)",        "Bumba (bootleg of Head On)",                             MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headon2,    0,        headon2,   headon2,   vicdual_state,   empty_init, ROT0,   "Sega",                    "Head On 2",                                              MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, headon2s,   headon2,  headon2bw, headon2s,  headonsa_state,  empty_init, ROT0,   "bootleg (Sidam)",         "Head On 2 (Sidam bootleg)",                              MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, car2,       headon2,  headon2bw, car2,      vicdual_state,   empty_init, ROT0,   "bootleg (RZ Bologna)",    "Car 2 (bootleg of Head On 2)",                           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // title still says 'HeadOn 2'
GAME( 1979, invho2,     0,        invho2,    invho2,    vicdual_state,   empty_init, ROT270, "Sega",                    "Invinco / Head On 2 (set 1)",                            MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, invho2a,    invho2,   invho2,    invho2,    vicdual_state,   empty_init, ROT270, "Sega",                    "Invinco / Head On 2 (set 2)",                            MACHINE_NOT_WORKING | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // wrong colors make Head On 2 unplayable (all black)
GAME( 1980, nsub,       0,        nsub,      nsub,      nsub_state,      empty_init, ROT270, "Sega",                    "N-Sub (upright)",                                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, nsubc,      nsub,     nsubc,     nsubc,     nsub_state,      empty_init, ROT270, "Sega",                    "N-Sub (cocktail)",                                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, samurai,    0,        samurai,   samurai,   vicdual_state,   empty_init, ROT270, "Sega",                    "Samurai (World)",                                        MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, samuraij,   samurai,  samurai,   samurai,   vicdual_state,   empty_init, ROT270, "Sega",                    "Samurai (Japan)",                                        MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, invinco,    0,        invinco,   invinco,   vicdual_state,   empty_init, ROT270, "Sega",                    "Invinco",                                                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, invcarht,   0,        carhntds,  carhntds,  vicdual_state,   empty_init, ROT270, "Sega",                    "Invinco / Car Hunt (Germany)",                           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, invds,      0,        invds,     invds,     vicdual_state,   empty_init, ROT270, "Sega",                    "Invinco / Deep Scan",                                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, carhntds,   0,        carhntds,  carhntds,  vicdual_state,   empty_init, ROT270, "Sega",                    "Car Hunt / Deep Scan (France)",                          MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, tranqgun,   0,        tranqgun,  tranqgun,  tranqgun_state,  empty_init, ROT270, "Sega",                    "Tranquillizer Gun",                                      MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacetrk,   0,        spacetrk,  spacetrk,  vicdual_state,   empty_init, ROT270, "Sega",                    "Space Trek (upright)",                                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacetrkc,  spacetrk, spacetrk,  spacetrkc, vicdual_state,   empty_init, ROT270, "Sega",                    "Space Trek (cocktail)",                                  MACHINE_IMPERFECT_GRAPHICS |MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, carnival,   0,        carnival,  carnival,  carnival_state,  empty_init, ROT270, "Sega",                    "Carnival (upright, AY8912 music)",                       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, carnivalb,  carnival, carnivalb, carnival,  carnival_state,  empty_init, ROT270, "Sega",                    "Carnival (upright, PIT8253 music)",                      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, carnivalc,  carnival, carnival,  carnivalc, carnival_state,  empty_init, ROT270, "Sega",                    "Carnival (cocktail)",                                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, carnivalca, carnival, carnival,  carnivalc, carnival_state,  empty_init, ROT270, "Sega",                    "Carnival (cocktail, earlier)",                           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // one missing ROM
GAME( 1980, carnivalh,  carnival, carnivalh, carnivalh, carnivalh_state, empty_init, ROT270, "Sega",                    "Carnival (Head On hardware, set 1)",                     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, carnivalha, carnival, carnivalh, carnivalh, carnivalh_state, empty_init, ROT270, "Sega",                    "Carnival (Head On hardware, set 2)",                     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, carnivalmm, carnival, carnival,  carnival,  carnival_state,  empty_init, ROT270, "bootleg (ManilaMatic)",   "Carnival (ManilaMatic bootleg)",                         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, verbena,    carnival, carnival,  carnival,  carnival_state,  empty_init, ROT270, "bootleg (Cocamatic)",     "Verbena (bootleg of Carnival)",                          MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, brdrline,   0,        brdrline,  brdrline,  vicdual_state,   empty_init, ROT270, "Sega",                    "Borderline",                                             MACHINE_SUPPORTS_SAVE )
GAME( 1981, starrkr,    brdrline, brdrline,  starrkr,   vicdual_state,   empty_init, ROT270, "Sega",                    "Star Raker",                                             MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, brdrlins,   brdrline, brdrline,  brdrline,  vicdual_state,   empty_init, ROT270, "bootleg (Sidam)",         "Borderline (Sidam bootleg)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1981, brdrlinb,   brdrline, brdrline,  brdrline,  vicdual_state,   empty_init, ROT270, "bootleg (Karateco)",      "Borderline (Karateco bootleg)",                          MACHINE_SUPPORTS_SAVE )
GAME( 1981, brdrlinet,  brdrline, brdrlinet, brdrline,  tranqgun_state,  empty_init, ROT270, "Sega",                    "Borderline (Tranquillizer Gun conversion)",              MACHINE_SUPPORTS_SAVE ) // official factory conversion
GAME( 198?, nostromo,   0,        headons,   startrks,  vicdual_state,   empty_init, ROT0,   "bootleg",                 "Nostromo",                                               MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // sound board probably differs
GAME( 198?, startrks,   nostromo, headons,   startrks,  vicdual_state,   empty_init, ROT0,   "bootleg (Sidam)",         "Star Trek (Head On hardware)",                           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, digger,     0,        digger,    digger,    vicdual_state,   empty_init, ROT270, "Sega",                    "Digger",                                                 MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pulsar,     0,        pulsar,    pulsar,    vicdual_state,   empty_init, ROT270, "Sega",                    "Pulsar",                                                 MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, heiankyo,   0,        heiankyo,  heiankyo,  vicdual_state,   empty_init, ROT270, "Denki Onkyo",             "Heiankyo Alien",                                         MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 19??, alphaho,    0,        alphaho,   alphaho,   vicdual_state,   empty_init, ROT270, "Data East Corporation",   "Alpha Fighter / Head On",                                MACHINE_WRONG_COLORS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 19??, alphahob,   alphaho,  alphaho,   alphahob,  vicdual_state,   empty_init, ROT270, "bootleg",                 "Missile / Circuit (bootleg of Alpha Fighter / Head On)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, wantsega,   0,        carhntds,  wantsega,  vicdual_state,   empty_init, ROT270, "Sega",                    "Wanted (Sega)",                                          MACHINE_NO_SOUND | MACHINE_IMPERFECT_CONTROLS | MACHINE_SUPPORTS_SAVE )
