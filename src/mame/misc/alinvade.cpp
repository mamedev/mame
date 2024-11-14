// license:BSD-3-Clause
// copyright-holders:David Haywood, Mariusz Wojcieszek
/*

 tiny bartop b&w Space Invaders type game with colour overlay

 Driver by David Haywood and Mariusz Wojcieszek

 TODO:
 - 16 bytes are protected in the c*** range. I'm guessing they used a PROM to protect a
   simple sub-routine because:
   * It attempts to jsr from RAM to that area with a 0x10 byte offset (i.e. ROM copies a code snippet to RAM; when it executes
     it code executes jsr 0xc400 then self-modifies it to 0xc410, rinse and repeat ... up to 0xc7f0 and rolls back);
   * After that the program has an amusing left-over located at 0xe000-0xe00f (yup, NOPs + a RTS), with the
     exact same number of times as above;
   It's unknown at current stage what it really protects tho, game seems working for all I can see ... -AS

 - Sound is entirely guesswork.

 */

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/discrete.h"
#include "screen.h"
#include "speaker.h"

#include "alinvade.lh"


namespace {

class alinvade_state : public driver_device
{
public:
	alinvade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram")
		, m_discrete(*this, "discrete")
	{ }

	void alinvade(machine_config &config);

private:
	uint8_t irqmask_r();
	void irqmask_w(uint8_t data);
	void sound_w(uint8_t data);
	void sounden_w(uint8_t data);
	void vblank_irq(int state);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void alinvade_map(address_map &map) ATTR_COLD;

	uint8_t m_irqmask = 0;
	uint8_t m_irqff = 0;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<discrete_device> m_discrete;
};

static const discrete_dac_r1_ladder alinvade_music_dac =
	{3, {0, RES_K(47), RES_K(12)}, 0, 0, 0, CAP_U(0.01)};

#define ALINVADE_MUSIC_CLK      (75000)

DISCRETE_SOUND_START(alinvade_discrete)
	DISCRETE_INPUT_DATA (NODE_01)

	DISCRETE_NOTE(NODE_20, 1, ALINVADE_MUSIC_CLK, NODE_01, 255, 5, DISC_CLK_IS_FREQ)

	// Convert count to 7492 output
	DISCRETE_TRANSFORM2(NODE_21, NODE_20, 2, "01>0+")

	DISCRETE_DAC_R1(NODE_22, NODE_21, DEFAULT_TTL_V_LOGIC_1, &alinvade_music_dac)

	DISCRETE_CRFILTER(NODE_80, NODE_22, RES_K(10), CAP_U(0.1))

	DISCRETE_OUTPUT(NODE_80, 2500)

DISCRETE_SOUND_END

void  alinvade_state::sound_w(uint8_t data)
{
	m_discrete->write(NODE_01, (data^0x3f)<<2);
}

void alinvade_state::sounden_w(uint8_t data)
{
	machine().sound().system_mute(data != 4);
}

uint8_t alinvade_state::irqmask_r()
{
	return 0; // TODO: might be anything
}


void alinvade_state::irqmask_w(uint8_t data)
{
	if((!(m_irqff & 1)) && (data & 1)) // f/f, active high? If the above actually returns 0xff this could be active low ...
		m_irqmask^= 1;

	m_irqff = data;
}

void alinvade_state::alinvade_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x0400, 0x0bff).ram().share("videoram");
	map(0x0c00, 0x0dff).ram();
	map(0x2000, 0x2000).w(FUNC(alinvade_state::sound_w));
	map(0x4000, 0x4000).portr("COIN");
	map(0x6000, 0x6000).portr("DSW");
	map(0x8000, 0x8000).portr("IN0");
	map(0x8001, 0x8001).portr("IN1");
	map(0x8002, 0x8002).portr("IN2");
	map(0x8003, 0x8003).portr("IN3");
	map(0x8004, 0x8004).portr("IN4");
	map(0xa000, 0xa000).nopw(); //??
	map(0xc000, 0xc00f).mirror(0xff0).rom().region("proms", 0);
	map(0xe000, 0xe3ff).rom();
	map(0xe400, 0xe400).w(FUNC(alinvade_state::sounden_w));
	map(0xe800, 0xe800).rw(FUNC(alinvade_state::irqmask_r), FUNC(alinvade_state::irqmask_w)); //??
	map(0xec00, 0xffff).rom();
}


static INPUT_PORTS_START( alinvade )
	PORT_START("COIN")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0xef, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT(0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT(0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT(0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT(0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR ( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "10k" )
	PORT_DIPSETTING(    0x00, "13k" )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void alinvade_state::machine_start()
{
	save_item(NAME(m_irqff));
	save_item(NAME(m_irqmask));
}

void alinvade_state::machine_reset()
{
	m_irqmask = 1;
}

uint32_t alinvade_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t x = (offs << 3)&0x7f;
		int y = (offs >> 4)&0x7f;
		uint8_t data = m_videoram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x01) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x) = pen;

			data >>= 1;
			x++;
		}
	}


	return 0;
}

void alinvade_state::vblank_irq(int state)
{
	if (state && BIT(m_irqmask, 0))
		m_maincpu->set_input_line(0,HOLD_LINE);
}

void alinvade_state::alinvade(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2000000);         /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &alinvade_state::alinvade_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128, 128);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(alinvade_state::screen_update));
	screen.screen_vblank().set(FUNC(alinvade_state::vblank_irq));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, alinvade_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( alinvade )
	ROM_REGION( 0x10000, "maincpu", 0 ) // todo, check mapping
	ROM_LOAD( "alien28.708", 0xe000, 0x0400, CRC(de376295) SHA1(e8eddbb1be1f8661c6b5b39c0d78a65bded65db2) )
	ROM_LOAD( "alien29.708", 0xec00, 0x0400, CRC(20212977) SHA1(9d24a6b403d968267079fa6241545bd5a01afebb) )
	ROM_LOAD( "alien30.708", 0xf000, 0x0400, CRC(734b691c) SHA1(9e562159061eecf4b1dee4ea0ee4752c901a54aa) )
	ROM_LOAD( "alien31.708", 0xf400, 0x0400, CRC(5a70535c) SHA1(2827e7d4bffca78bd035da04481e1e972ee2da39) )
	ROM_LOAD( "alien32.708", 0xf800, 0x0400, CRC(332dd234) SHA1(9974668344a2a351868a9e7757d1c3a497dc5621) )
	ROM_LOAD( "alien33.708", 0xfc00, 0x0400, CRC(e0d57fc7) SHA1(7b8ddcb4a86811592d2d0bbc61b2f19e5caa9ccc) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom", 0, 0x20, NO_DUMP )
	ROM_FILL( 0x00, 0x0f, 0xea )
	ROM_FILL( 0x0f, 0x01, 0x60 )    // rts for whole area, interrupt code jumps to various addresses here, check note on top.
ROM_END

} // anonymous namespace


GAMEL( 198?, alinvade, 0, alinvade, alinvade, alinvade_state, empty_init, ROT90, "Forbes?", "Alien Invaders", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_alinvade )
