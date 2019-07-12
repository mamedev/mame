// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, hap
/*
 *  Carnival audio routines
 */

#include "emu.h"
#include "includes/vicdual.h"


/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_RIFLE        0x01
#define OUT_PORT_1_CLANG        0x02
#define OUT_PORT_1_DUCK1        0x04
#define OUT_PORT_1_DUCK2        0x08
#define OUT_PORT_1_DUCK3        0x10
#define OUT_PORT_1_PIPEHIT      0x20
#define OUT_PORT_1_BONUS1       0x40
#define OUT_PORT_1_BONUS2       0x80

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PORT_2_BEAR         0x04
#define OUT_PORT_2_RANKING      0x20


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
	SND_BEAR = 0,
	SND_BONUS1,
	SND_BONUS2,
	SND_CLANG,
	SND_DUCK1,
	SND_DUCK2,
	SND_DUCK3,
	SND_PIPEHIT,
	SND_RANKING,
	SND_RIFLE
};


WRITE8_MEMBER( carnival_state::carnival_audio_1_w )
{
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port1State = data;

	if ( bitsGoneLow & OUT_PORT_1_RIFLE )
	{
		PLAY( m_samples, SND_RIFLE, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_CLANG )
	{
		PLAY( m_samples, SND_CLANG, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_DUCK1 )
	{
		PLAY( m_samples, SND_DUCK1, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_1_DUCK1 )
	{
		STOP( m_samples, SND_DUCK1 );
	}

	if ( bitsGoneLow & OUT_PORT_1_DUCK2 )
	{
		PLAY( m_samples, SND_DUCK2, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_1_DUCK2 )
	{
		STOP( m_samples, SND_DUCK2 );
	}

	if ( bitsGoneLow & OUT_PORT_1_DUCK3 )
	{
		PLAY( m_samples, SND_DUCK3, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_1_DUCK3 )
	{
		STOP( m_samples, SND_DUCK3 );
	}

	if ( bitsGoneLow & OUT_PORT_1_PIPEHIT )
	{
		PLAY( m_samples, SND_PIPEHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_BONUS1 )
	{
		PLAY( m_samples, SND_BONUS1, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_BONUS2 )
	{
		PLAY( m_samples, SND_BONUS2, 0 );
	}
}

WRITE8_MEMBER( carnival_state::carnival_audio_2_w )
{
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port2State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port2State = data;

	if ( bitsGoneLow & OUT_PORT_2_BEAR )
	{
		PLAY( m_samples, SND_BEAR, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_RANKING )
	{
		PLAY( m_samples, SND_RANKING, 0 );
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

READ_LINE_MEMBER( carnival_state::carnival_music_port_t1_r )
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

WRITE8_MEMBER( carnival_state::carnivala_music_port_1_w )
{
	// P1: AY8912 d0-d7
	m_musicData = data;
	carnival_psg_latch();
}

WRITE8_MEMBER( carnival_state::carnivala_music_port_2_w )
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

WRITE8_MEMBER( carnival_state::carnivalb_music_port_1_w )
{
	// P1: PIT8253 d0-d7
	m_musicData = data;
}

WRITE8_MEMBER( carnival_state::carnivalb_music_port_2_w )
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
	{
		DAC_1BIT(config, m_dac[i]).add_route(ALL_OUTPUTS, "mono", 0.15);
		VOLTAGE_REGULATOR(config, m_vref[i]).add_route(0, m_dac[i], 1.0, DAC_VREF_POS_INPUT);
	}
}
