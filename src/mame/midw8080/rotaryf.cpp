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
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/samples.h"
#include "sound/sn76477.h"

#include "screen.h"
#include "speaker.h"


namespace {

class rotaryf_state : public driver_device
{
public:
	rotaryf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_samples(*this, "samples"),
		m_sn(*this, "snsnd"),
		m_videoram(*this, "videoram")
	{
	}

	void rotaryf(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<sn76477_device> m_sn;

	required_shared_ptr<uint8_t> m_videoram;

	uint8_t portb_r();
	void porta_w(uint8_t data);
	void portc_w(uint8_t data);
	void port30_w(uint8_t data);

	bool m_flipscreen = 0;
	uint8_t m_last = 0U;

	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(rotaryf_interrupt);
	void rotaryf_io_map(address_map &map) ATTR_COLD;
	void rotaryf_map(address_map &map) ATTR_COLD;
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
	nullptr
};


void rotaryf_state::machine_start()
{
	m_last = 0xff;

	save_item(NAME(m_flipscreen));
	save_item(NAME(m_last));
}

uint8_t rotaryf_state::portb_r()
{
	uint8_t data = ioport("INPUTS")->read();

	if (m_flipscreen) return data;

	return (data & 0xCD) | ((data & 0x01) << 1) | ((data & 0x0c) << 2);
}

void rotaryf_state::porta_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_last;

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

void rotaryf_state::portc_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1));

	// bit 5 set when game starts, but isn't coin lockout?
}

void rotaryf_state::port30_w(uint8_t data)
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

uint32_t rotaryf_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const pens[2] = { rgb_t::black(), rgb_t::white() };

	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t x = offs << 3;
		uint8_t y = offs >> 5;
		uint8_t data = m_videoram[offs];

		for (uint8_t i = 0; i < 8; i++)
		{
			if (m_flipscreen)
				bitmap.pix(255-y, 247-(x|i)) = pens[data & 1];
			else
				bitmap.pix(y, x|i) = pens[data & 1];

			data >>= 1;
		}
	}

	return 0;
}


void rotaryf_state::rotaryf_map(address_map &map)
{
	map(0x0000, 0x17ff).mirror(0x4000).rom();
	map(0x7000, 0x73ff).mirror(0x0c00).ram();
	map(0x8000, 0x9fff).mirror(0x4000).ram().share("videoram");
	map(0xa000, 0xa1ff).ram(); /* writes 00, 18, 27, 3C, 7E, FE to A019, A039, A059... A179 */
}

void rotaryf_state::rotaryf_io_map(address_map &map)
{
	map(0x02, 0x02).nopw();
	map(0x04, 0x04).nopw();
	map(0x07, 0x07).nopw();
	map(0x20, 0x20).nopw();
	map(0x21, 0x21).portr("COIN").nopw();
	map(0x26, 0x26).portr("DSW");
	map(0x28, 0x2b).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x30).w(FUNC(rotaryf_state::port30_w));
}


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


void rotaryf_state::rotaryf(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 4000000); /* ?? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &rotaryf_state::rotaryf_map);
	m_maincpu->set_addrmap(AS_IO, &rotaryf_state::rotaryf_io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(rotaryf_state::rotaryf_interrupt), "screen", 0, 1);

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(rotaryf_state::porta_w));
	ppi.in_pb_callback().set(FUNC(rotaryf_state::portb_r));
	ppi.out_pc_callback().set(FUNC(rotaryf_state::portc_w));
	ppi.tri_pc_callback().set_constant(0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(32*8, 262);     /* vert size is a guess, taken from mw8080bw */
	screen.set_visarea(1*8, 30*8-1, 0*8, 32*8-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(rotaryf_state::screen_update));

	SPEAKER(config, "mono").front_center();

	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, RES_K(100));
	m_sn->set_amp_res(RES_K(56));
	m_sn->set_feedback_res(RES_K(10));
	m_sn->set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(120));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.5);

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(rotaryf_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( rotaryf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "krf-1.bin", 0x0000, 0x0400, CRC(f7b2d3e6) SHA1(be7afc1a14be60cb895fc4180167353c7156fc4c) )
	ROM_LOAD( "krf-2.bin", 0x0400, 0x0400, CRC(be9f047a) SHA1(e5dd2b5b4fda7f178e7f1137592ba49fbc9cc82e) )
	ROM_LOAD( "krf-3.bin", 0x0800, 0x0400, CRC(c7629eb6) SHA1(03aae964783ce4b1de77737e83fd2094483fbda4) )
	ROM_LOAD( "krf-4.bin", 0x0c00, 0x0400, CRC(b4703093) SHA1(9239d6da818049bc98a631c3bf5b962b5df5b2ea) )
	ROM_LOAD( "krf-5.bin", 0x1000, 0x0400, CRC(ae233f07) SHA1(a7bbd2ee4477ee041d170e2fc4e94c99c3b564fc) )
	ROM_LOAD( "krf-6.bin", 0x1400, 0x0400, CRC(e28b3713) SHA1(428f73891125f80c722357f1029b18fa9416bcfd) )
ROM_END

} // anonymous namespace


GAME( 1979, rotaryf, 0, rotaryf, rotaryf, rotaryf_state, empty_init, ROT270, "Kasco", "Rotary Fighter", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
