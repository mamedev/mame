// license:BSD-3-Clause
// copyright-holders:Hau,Derrick Renaud
/*

-Galaxy Force
-Run Away
--------------------------
Dai San Wakusei Meteor
(c)1979 Sun Electronics

SIV-01-B
TVG_13.6     [8e8b40b1]
TVG_14.7     [d48cbabe]
TVG_15.8     [cf44bd60]
TVG_16.9     [ae723f56]
--------------------------
-Warp 1


Dumped by Chack'n
01/04/2009

Written by Hau
02/18/2009
12/14/2010

Discrete by Andy
11/11/2009


Driver Notes:

- Two player games are automatically displayed in cocktail mode.
  Is this by design (a cocktail only romset)?

- Discrete audio needs adding to replace hardcoded samples

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "machine/rescap.h"
#include "sound/sn76477.h"

#define USE_SAMPLES     (1)


class dai3wksi_state : public driver_device
{
public:
	dai3wksi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_ic77(*this, "ic77"),
		m_ic78(*this, "ic78"),
		m_ic79(*this, "ic79"),
		m_ic80(*this, "ic80"),
		m_ic81(*this, "ic81"),
		m_palette(*this, "palette"),
		m_dai3wksi_videoram(*this, "videoram"),
		m_in2(*this, "IN2") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	optional_device<sn76477_device> m_ic77;
	optional_device<sn76477_device> m_ic78;
	optional_device<sn76477_device> m_ic79;
	optional_device<sn76477_device> m_ic80;
	optional_device<sn76477_device> m_ic81;
	required_device<palette_device> m_palette;

	/* video */
	required_shared_ptr<UINT8> m_dai3wksi_videoram;
	int         m_dai3wksi_flipscreen;
	int         m_dai3wksi_redscreen;
	int         m_dai3wksi_redterop;
	UINT32 screen_update_dai3wksi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	/* sound */
	UINT8       m_port_last1;
	UINT8       m_port_last2;
	int         m_enabled_sound;
	int         m_sound3_counter;
	DECLARE_WRITE8_MEMBER(dai3wksi_audio_1_w);
	DECLARE_WRITE8_MEMBER(dai3wksi_audio_2_w);
	DECLARE_WRITE8_MEMBER(dai3wksi_audio_3_w);

	/* i/o ports */
	required_ioport m_in2;

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


/*************************************
 *
 *  Video system
 *
 *************************************/

static const UINT8 vr_prom1[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};

static const UINT8 vr_prom2[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};

UINT32 dai3wksi_state::screen_update_dai3wksi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_dai3wksi_videoram.bytes(); offs++)
	{
		UINT8 x = offs << 2;
		UINT8 y = offs >> 6;
		UINT8 data = m_dai3wksi_videoram[offs];
		UINT8 color;
		int value = (x >> 2) + ((y >> 5) << 6) + 64 * 8 * (m_dai3wksi_redterop ? 1 : 0);

		if (m_dai3wksi_redscreen)
		{
			color = 0x02;
		}
		else
		{
			if (m_in2->read() & 0x03)
				color = vr_prom2[value];
			else
				color = vr_prom1[value];
		}

		for (int i = 0; i <= 3; i++)
		{
			rgb_t pen = (data & (1 << i)) ? m_palette->pen_color(color) : rgb_t::black;

			if (m_dai3wksi_flipscreen)
				bitmap.pix32(255-y, 255-x) = pen;
			else
				bitmap.pix32(y, x) = pen;

			x++;
		}
	}

	return 0;
}


/*************************************
 *
 *  Audio system
 *
 *************************************/

#define SAMPLE_SOUND1       0
#define SAMPLE_SOUND2       1
#define SAMPLE_SOUND3_1     2
#define SAMPLE_SOUND3_2     3
#define SAMPLE_SOUND4       4
#define SAMPLE_SOUND5       5
#define SAMPLE_SOUND6_1     6
#define SAMPLE_SOUND6_2     7

#define CHANNEL_SOUND1      0
#define CHANNEL_SOUND2      1
#define CHANNEL_SOUND3      2
#define CHANNEL_SOUND4      3
#define CHANNEL_SOUND5      4
#define CHANNEL_SOUND6      5


#if (USE_SAMPLES)
WRITE8_MEMBER(dai3wksi_state::dai3wksi_audio_1_w)
{
	UINT8 rising_bits = data & ~m_port_last1;

	m_enabled_sound = data & 0x80;

	if ((rising_bits & 0x20) && m_enabled_sound)
	{
		if (data & 0x04)
			m_samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5);
		else
			m_samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5, true);
	}
	if (!(data & 0x20) && (m_port_last1 & 0x20))
		m_samples->stop(CHANNEL_SOUND5);

	m_port_last1 = data;
}

WRITE8_MEMBER(dai3wksi_state::dai3wksi_audio_2_w)
{
	UINT8 rising_bits = data & ~m_port_last2;

	m_dai3wksi_flipscreen = data & 0x10;
	m_dai3wksi_redscreen  = ~data & 0x20;
	m_dai3wksi_redterop   = data & 0x40;

	if (m_enabled_sound)
	{
		if (rising_bits & 0x01) m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND1);
		if (rising_bits & 0x02) m_samples->start(CHANNEL_SOUND2, SAMPLE_SOUND2);
		if (rising_bits & 0x08) m_samples->start(CHANNEL_SOUND4, SAMPLE_SOUND4);
		if (rising_bits & 0x04)
		{
			if (!m_sound3_counter)
				m_samples->start(CHANNEL_SOUND3, SAMPLE_SOUND3_1);
			else
				m_samples->start(CHANNEL_SOUND3, SAMPLE_SOUND3_2);

			m_sound3_counter ^= 1;
		}
	}

	m_port_last2 = data;
}

WRITE8_MEMBER(dai3wksi_state::dai3wksi_audio_3_w)
{
	if (m_enabled_sound)
	{
		if (data & 0x40)
			m_samples->start(CHANNEL_SOUND6, SAMPLE_SOUND6_1);
		else if (data & 0x80)
			m_samples->start(CHANNEL_SOUND6, SAMPLE_SOUND6_2);
	}
}


static const char *const dai3wksi_sample_names[] =
{
	"*dai3wksi",
	"1",
	"2",
	"3",
	"3-2",
	"4",
	"5",
	"6",
	"6-2",
	nullptr
};


#else

WRITE8_MEMBER(dai3wksi_state::dai3wksi_audio_1_w)
{
	machine().sound().system_enable(data & 0x80);

	m_ic79->enable_w((~data >> 5) & 0x01);        /* invader movement enable */
	m_ic79->envelope_1_w((~data >> 2) & 0x01);    /* invader movement envelope control*/
}

WRITE8_MEMBER(dai3wksi_state::dai3wksi_audio_2_w)
{
	m_dai3wksi_flipscreen =  data & 0x10;
	m_dai3wksi_redscreen  = ~data & 0x20;
	m_dai3wksi_redterop   =  data & 0x40;

	m_ic77->enable_w((~data >> 0) & 0x01);    /* ship movement */
	m_ic78->enable_w((~data >> 1) & 0x01);    /* danger text */
	/* ic76 - invader hit  (~data >> 2) & 0x01 */
	m_ic80->enable_w((~data >> 3) & 0x01);    /* planet explosion */
}

WRITE8_MEMBER(dai3wksi_state::dai3wksi_audio_3_w)
{
	m_ic81->enable_w((~data >> 2) & 0x01);    /* player shoot enable */
	m_ic81->vco_w((~data >> 3) & 0x01);       /* player shoot vco control */
}

#endif


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, dai3wksi_state )
	AM_RANGE(0x0000, 0x1bff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0x24ff) AM_MIRROR(0x100) AM_READ_PORT("IN0")
	AM_RANGE(0x2800, 0x28ff) AM_MIRROR(0x100) AM_READ_PORT("IN1")
	AM_RANGE(0x3000, 0x3000) AM_WRITE(dai3wksi_audio_1_w)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(dai3wksi_audio_2_w)
	AM_RANGE(0x3800, 0x3800) AM_WRITE(dai3wksi_audio_3_w)
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( dai3wksi )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x10, 0x00, "DIPSW #7" )                      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW #8" )                      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW #1" )                      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW #2" )                      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void dai3wksi_state::machine_start()
{
	/* Set up save state */
	save_item(NAME(m_dai3wksi_flipscreen));
	save_item(NAME(m_dai3wksi_redscreen));
	save_item(NAME(m_dai3wksi_redterop));
	save_item(NAME(m_port_last1));
	save_item(NAME(m_port_last2));
	save_item(NAME(m_enabled_sound));
	save_item(NAME(m_sound3_counter));
}

void dai3wksi_state::machine_reset()
{
	m_port_last1 = 0;
	m_port_last2 = 0;
	m_enabled_sound = 0;
	m_sound3_counter = 0;
}


static MACHINE_CONFIG_START( dai3wksi, dai3wksi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10MHz/4)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dai3wksi_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(4, 251, 8, 247)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(dai3wksi_state, screen_update_dai3wksi)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")

#if (USE_SAMPLES)
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(dai3wksi_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
#else
	// Invader Hit
	MCFG_SOUND_ADD("ic76", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                   // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(RES_K(4.7))                   // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(0.1), RES_K(4.7))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.022), RES_K(33))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(0, 0)                        // slf caps + res: N/C
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                    // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(0)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(0, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(0)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	// Ship Movement
	MCFG_SOUND_ADD("ic77", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                   // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(RES_K(4.7))                   // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(0.1), RES_K(4.7))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, 0, 0)                     // VCO volt + cap + res: N/C
	MCFG_SN76477_PITCH_VOLTAGE(0)                        // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(0.0022), RES_K(200))   // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(10), RES_K(4.7))   // oneshot caps + res
	MCFG_SN76477_VCO_MODE(5)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(5, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(5, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	// Danger
	MCFG_SOUND_ADD("ic78", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), 0, 0)           // noise + filter
	MCFG_SN76477_DECAY_RES(RES_K(200))                   // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(0.1), RES_K(4.7))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.47), RES_K(75))   // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_N(1), RES_K(47))         // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(10), RES_K(22))    // oneshot caps + res
	MCFG_SN76477_VCO_MODE(5)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(5, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	// Invader Marching Noise
	MCFG_SOUND_ADD("ic79", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                   // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(RES_K(56))                    // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(0.1), RES_K(4.7))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.01), RES_K(100))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_N(1), RES_K(150))        // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(10), RES_K(22))    // oneshot caps + res
	MCFG_SN76477_VCO_MODE(5)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(5, 5)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	// Big Planet Explosion
	MCFG_SOUND_ADD("ic80", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_K(330), CAP_P(470)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_M(2))                     // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(1.0), RES_K(4.7))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, 0, 0)                     // VCO volt + cap + res: N/C
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(0, 0)                        // slf caps + res: N/C
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(10), RES_K(55))    // oneshot caps + res
	MCFG_SN76477_VCO_MODE(5)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 5, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(5, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	// Plane Shoot noise
	MCFG_SOUND_ADD("ic81", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                    // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(RES_K(200))                    // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(10), RES_K(4.7))     // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                      // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                  // feedback_res
	MCFG_SN76477_VCO_PARAMS(2.5, CAP_U(0.01), RES_K(100)) // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                       // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_N(0.47), RES_K(100))      // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(10), RES_K(6.8))    // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                              // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 5, 5)                    // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(5, 0)                    // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                                // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)
#endif
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( dai3wksi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg_13.6",  0x0000, 0x0800, CRC(8e8b40b1) SHA1(25b9223486dd348ea302e8e8f1d47c804a88b142) )
	ROM_LOAD( "tvg_14.7",  0x0800, 0x0800, CRC(d48cbabe) SHA1(64b571cd778fc7d67a5fa998a0defd36c04f111f) )
	ROM_LOAD( "tvg_15.8",  0x1000, 0x0800, CRC(cf44bd60) SHA1(61e0b3f9c4a1f9da1de57fb8276d4fc9e43b8f24) )
	ROM_LOAD( "tvg_16.9",  0x1800, 0x0400, CRC(ae723f56) SHA1(c25c27d6144533b2b2a888bfa8dbf48ed8d8b09a) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, dai3wksi, 0, dai3wksi, dai3wksi, driver_device, 0, ROT270, "Sun Electronics", "Dai San Wakusei Meteor (Japan)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
