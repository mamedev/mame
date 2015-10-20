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
#include "alinvade.lh"

class alinvade_state : public driver_device
{
public:
	alinvade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram")
		, m_discrete(*this, "discrete")
	{ }

	DECLARE_READ8_MEMBER(irqmask_r);
	DECLARE_WRITE8_MEMBER(irqmask_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(sounden_w);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	UINT8 m_irqmask;
	UINT8 m_irqff;
	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
	required_device<discrete_device> m_discrete;
};

static const discrete_dac_r1_ladder alinvade_music_dac =
	{3, {0, RES_K(47), RES_K(12)}, 0, 0, 0, CAP_U(0.01)};

#define ALINVADE_MUSIC_CLK      (75000)

DISCRETE_SOUND_START(alinvade)
	DISCRETE_INPUT_DATA (NODE_01)

	DISCRETE_NOTE(NODE_20, 1, ALINVADE_MUSIC_CLK, NODE_01, 255, 5, DISC_CLK_IS_FREQ)

	// Convert count to 7492 output
	DISCRETE_TRANSFORM2(NODE_21, NODE_20, 2, "01>0+")

	DISCRETE_DAC_R1(NODE_22, NODE_21, DEFAULT_TTL_V_LOGIC_1, &alinvade_music_dac)

	DISCRETE_CRFILTER(NODE_80, NODE_22, RES_K(10), CAP_U(0.1))

	DISCRETE_OUTPUT(NODE_80, 2500)

DISCRETE_SOUND_END

WRITE8_MEMBER( alinvade_state::sound_w )
{
	m_discrete->write(space, NODE_01, (data^0x3f)<<2);
}

WRITE8_MEMBER( alinvade_state::sounden_w )
{
	machine().sound().system_enable(data == 4);
}

READ8_MEMBER(alinvade_state::irqmask_r)
{
	return 0; // TODO: might be anything
}


WRITE8_MEMBER(alinvade_state::irqmask_w)
{
	if((!(m_irqff & 1)) && (data & 1)) // f/f, active high? If the above actually returns 0xff this could be active low ...
		m_irqmask^= 1;

	m_irqff = data;
}

static ADDRESS_MAP_START( alinvade_map, AS_PROGRAM, 8, alinvade_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0400, 0x0bff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0dff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_WRITE(sound_w)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("COIN")
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("DSW")
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN0")
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("IN1")
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("IN2")
	AM_RANGE(0x8003, 0x8003) AM_READ_PORT("IN3")
	AM_RANGE(0x8004, 0x8004) AM_READ_PORT("IN4")
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP //??
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0xff0) AM_ROM AM_REGION("proms",0)
	AM_RANGE(0xe000, 0xe3ff) AM_ROM
	AM_RANGE(0xe400, 0xe400) AM_WRITE(sounden_w)
	AM_RANGE(0xe800, 0xe800) AM_READWRITE(irqmask_r,irqmask_w) //??
	AM_RANGE(0xec00, 0xffff) AM_ROM
ADDRESS_MAP_END


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
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR ( Unknown ) )   // read, but not tested afterwards?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
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

UINT32 alinvade_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		int i;

		UINT8 x = (offs << 3)&0x7f;
		int y = (offs >> 4)&0x7f;
		UINT8 data = m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x01) ? rgb_t::white : rgb_t::black;
			bitmap.pix32(y, x) = pen;

			data = data >> 1;
			x = x + 1;
		}
	}


	return 0;
}

INTERRUPT_GEN_MEMBER(alinvade_state::vblank_irq)
{
	if(m_irqmask & 1)
		m_maincpu->set_input_line(0,HOLD_LINE);
}

static MACHINE_CONFIG_START( alinvade, alinvade_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502,2000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(alinvade_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", alinvade_state,  vblank_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(128, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 128-1, 0, 128-1)
	MCFG_SCREEN_UPDATE_DRIVER(alinvade_state, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DISCRETE_ADD("discrete", 0, alinvade)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



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


GAMEL( 198?, alinvade,  0,    alinvade, alinvade, driver_device,  0, ROT90, "Forbes?", "Alien Invaders", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_alinvade )
