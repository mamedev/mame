// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/* Rotary Fighter, 01/1979, Kasco (Kansai Seiki Seisakusho Co.)
 board KIV-101 CPU: xtal(??mhz), i8085A, 40 pin IC(i8255?), 6*ROM, 1*RAM, DIP(8 switches), ..
 board KIV-101 CRT: 2*RAM, lots of 74xx TTL

driver by Barry Rodewald
 based on Initial work by David Haywood

 todo:

 sound
 verify game speed if possible (related to # of interrupts)

*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/samples.h"
#include "sound/sn76477.h"

class rotaryf_state : public driver_device
{
public:
	rotaryf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_samples(*this, "samples"),
		m_sn(*this, "snsnd"),
		m_videoram(*this, "videoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<sn76477_device> m_sn;

	required_shared_ptr<UINT8> m_videoram;

	DECLARE_READ8_MEMBER(port29_r);
	DECLARE_WRITE8_MEMBER(port28_w);
	DECLARE_WRITE8_MEMBER(port30_w);

	bool m_flipscreen;
	UINT8 m_last;

	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(rotaryf_interrupt);
};


static const char *const rotaryf_sample_names[] =
{
	"*invaders",
	"1",        /* shot/missle */
	"2",        /* base hit/explosion */
	"3",        /* invader hit */
	"4",        /* fleet move 1 */
	"5",        /* fleet move 2 */
	"6",        /* fleet move 3 */
	"7",        /* fleet move 4 */
	"8",        /* UFO/saucer hit */
	"9",        /* bonus base */
	0
};


void rotaryf_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_last));
}

READ8_MEMBER( rotaryf_state::port29_r )
{
	UINT8 data = ioport("INPUTS")->read();

	if (m_flipscreen) return data;

	return (data & 0xCD) | ((data & 0x01) << 1) | ((data & 0x0c) << 2);
}

WRITE8_MEMBER( rotaryf_state::port28_w )
{
	UINT8 rising_bits = data & ~m_last;

	if (BIT(rising_bits, 0)) m_samples->start (3, 7);   /* Hit Saucer */
	if (BIT(rising_bits, 2)) m_samples->start (5, 8);   /* Bonus */
	if (BIT(rising_bits, 5)) m_samples->start (1, 1);   /* Death */
	if (BIT(rising_bits, 6)) m_samples->start (2, 2);   /* Hit */
	if (BIT(rising_bits, 7)) m_samples->start (0, 0);   /* Shoot */

	m_sn->enable_w((data & 3) ? 1 : 0);     /* Saucer Sound */

	if (BIT(rising_bits, 4))
	{
		if (BIT(rising_bits, 3))
			m_samples->start (4, 3);        /* Fleet 1 */
		else
			m_samples->start (4, 6);        /* Fleet 2 */
	}

	m_last = data;
}

WRITE8_MEMBER( rotaryf_state::port30_w )
{
	/* bit 0 = player 2 is playing */

	m_flipscreen = BIT(data, 0) & ioport("COCKTAIL")->read();
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(rotaryf_state::rotaryf_interrupt)
{
	int scanline = param;

	if (scanline == 256)
		m_maincpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
	else if((scanline % 64) == 0)
	{
		m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
		m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
	}
}



/*************************************
 *
 *  Video system
 *
 *************************************/

UINT32 rotaryf_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;
	pen_t pens[2];
	pens[0] = rgb_t::black;
	pens[1] = rgb_t::white;
	UINT8 i,x,y,data;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		x = offs << 3;
		y = offs >> 5;
		data = m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			if (m_flipscreen)
				bitmap.pix32(255-y, 247-(x|i)) = pens[data & 1];
			else
				bitmap.pix32(y, x|i) = pens[data & 1];

			data >>= 1;
		}
	}

	return 0;
}


static ADDRESS_MAP_START( rotaryf_map, AS_PROGRAM, 8, rotaryf_state )
	AM_RANGE(0x0000, 0x17ff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0x7000, 0x73ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x8000, 0x9fff) AM_MIRROR(0x4000) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xa1ff) AM_RAM /* writes 00, 18, 27, 3C, 7E, FE to A019, A039, A059... A179 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( rotaryf_io_map, AS_IO, 8, rotaryf_state )
	AM_RANGE(0x21, 0x21) AM_READ_PORT("COIN")
	AM_RANGE(0x26, 0x26) AM_READ_PORT("DSW")
	AM_RANGE(0x29, 0x29) AM_READ(port29_r)

	AM_RANGE(0x02, 0x02) AM_WRITENOP
	AM_RANGE(0x04, 0x04) AM_WRITENOP
	AM_RANGE(0x07, 0x07) AM_WRITENOP
	AM_RANGE(0x20, 0x20) AM_WRITENOP
	AM_RANGE(0x21, 0x21) AM_WRITENOP
	AM_RANGE(0x28, 0x28) AM_WRITE(port28_w)
	AM_RANGE(0x2a, 0x2a) AM_WRITENOP
	AM_RANGE(0x2b, 0x2b) AM_WRITENOP
	AM_RANGE(0x30, 0x30) AM_WRITE(port30_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( rotaryf )
	PORT_START("COIN")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x81, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x81, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_START("COCKTAIL")      /* Dummy port for cocktail mode */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static MACHINE_CONFIG_START( rotaryf, rotaryf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A,4000000) /* ?? MHz */
	MCFG_CPU_PROGRAM_MAP(rotaryf_map)
	MCFG_CPU_IO_MAP(rotaryf_io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", rotaryf_state, rotaryf_interrupt, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(32*8, 262)     /* vert size is a guess, taken from mw8080bw */
	MCFG_SCREEN_VISIBLE_AREA(1*8, 30*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(rotaryf_state, screen_update)
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                 // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                          // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))          // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                    // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))               // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2)) // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                    // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))    // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                  // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                           // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                 // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                 // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                             // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(rotaryf_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( rotaryf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "krf-1.bin", 0x0000, 0x0400, CRC(f7b2d3e6) SHA1(be7afc1a14be60cb895fc4180167353c7156fc4c) )
	ROM_LOAD( "krf-2.bin", 0x0400, 0x0400, CRC(be9f047a) SHA1(e5dd2b5b4fda7f178e7f1137592ba49fbc9cc82e) )
	ROM_LOAD( "krf-3.bin", 0x0800, 0x0400, CRC(c7629eb6) SHA1(03aae964783ce4b1de77737e83fd2094483fbda4) )
	ROM_LOAD( "krf-4.bin", 0x0c00, 0x0400, CRC(b4703093) SHA1(9239d6da818049bc98a631c3bf5b962b5df5b2ea) )
	ROM_LOAD( "krf-5.bin", 0x1000, 0x0400, CRC(ae233f07) SHA1(a7bbd2ee4477ee041d170e2fc4e94c99c3b564fc) )
	ROM_LOAD( "krf-6.bin", 0x1400, 0x0400, CRC(e28b3713) SHA1(428f73891125f80c722357f1029b18fa9416bcfd) )
ROM_END


GAME( 1979, rotaryf, 0, rotaryf, rotaryf, driver_device, 0, ROT270, "Kasco", "Rotary Fighter", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
