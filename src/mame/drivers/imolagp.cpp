// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

  IMOLA GP by RB Bologna (Alberici?)

TODO:
- document remaining dips
- need better mappings for shifter, currently 3 buttons
- cpu/audio clocks, the XTAL on pcb is unlabeled
- correct video timing, see sprites disappear partially
- vreg[0xf] autosteering, the car should probably only auto-steer when
  it gets on the grass(road edges) at high speed. How does the hardware
  know the sprite position then?
- verify colors

========================================
www.andys-arcade.com

Dumped by Andy Welburn on a windy and rainy day 07/07/04

Possibly has clk/dir type steering.

Shows RB BO ITALY on the title screen and is a top-down driving game,
a lot like monaco GP, it even has stages where you have headlights.
Board colour, screening, track patterns, and most importantly
component type and colour of sockets indicates to me a pcb made in
the same factory as 'Sidam' and some 'Olympia' games. There is no
manufacturer name, no game name, all i see is : AA20/80 etched
on the underside of the pcb.

I have had this pcb for a number of years, i always thought it was
some sort of pinball logic pcb so didn't treat it too well. When it
came to clearing out my boxes of junk i took another look at it, and
it was the bank of 4116 rams that made me take a closer look.

I hooked it up and saw some video on my scope, then it died.
The +12v had shorted.. Suspecting the godamn tantalum capacitors
(often short out for no reason) i found a shorted one, removed
it and away we went. It had separate H + V sync, so i loaded
a 74ls08 into a spare ic space and AND'ed the two signals to get
composite, voila, i got a stable picture. The colours aren't right,
and maybe the video isn't quite right either, but it worked enough
for me to realise i had never seen a game like it, so i dumped it.

I couldn't get any sound out of it, could be broken, or not
hooked up right, i would suspect the latter is the case.


Hardware :
==========
2x  z80's
1x  AY-3-8910
1x  8255
2   pairs of 2114 RAM (512 bytes each)
16x 4116 RAM (2k each)
4x  2716 (ROM)
12x 2708 (ROM)


ROMS layout:
------------
(add .bin to the end to get the filenames)
YC,YD,YA and YB are all 2716 eproms, everything else is 2708's.

(tabulation used here, see photo for clarity)

YC  YD      XX
YA  YB
            XC
        XI  XM
        XY  XD
        XP  XS
        XA  XE
        XR  XO

Andy Welburn
www.andys-arcade.com

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"

#include "imolagp.lh"


class imolagp_state : public driver_device
{
public:
	imolagp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slavecpu(*this, "slave"),
		m_steer_pot_timer(*this, "pot"),
		m_steer_inp(*this, "STEER")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slavecpu;
	required_device<timer_device> m_steer_pot_timer;
	required_ioport m_steer_inp;

	UINT8 m_videoram[2][0x4000]; // 2 layers of 16KB
	UINT8 m_comms_latch[2];
	UINT8 m_vcontrol;
	UINT8 m_vreg[0x10];
	UINT8 m_scroll;
	UINT8 m_steerlatch;
	UINT8 m_draw_mode;

	DECLARE_WRITE8_MEMBER(transmit_data_w);
	DECLARE_READ8_MEMBER(trigger_slave_nmi_r);
	DECLARE_READ8_MEMBER(receive_data_r);
	DECLARE_WRITE8_MEMBER(imola_led_board_w);
	DECLARE_READ8_MEMBER(vreg_data_r);
	DECLARE_WRITE8_MEMBER(screenram_w);
	DECLARE_READ8_MEMBER(imola_draw_mode_r);
	DECLARE_WRITE8_MEMBER(vreg_control_w);
	DECLARE_WRITE8_MEMBER(vreg_data_w);
	DECLARE_CUSTOM_INPUT_MEMBER(imolagp_steerlatch_r);
	INTERRUPT_GEN_MEMBER(slave_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(imolagp_pot_callback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(imolagp);
	UINT32 screen_update_imolagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(imolagp_state, imolagp)
{
	// palette seems like 3bpp + intensity
	// this still needs to be verified
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i*4+0, 0, 0, 0);
		palette.set_pen_color(i*4+1, pal1bit(i >> 2)/2, pal1bit(i >> 1)/2, pal1bit(i >> 0)/2);
		palette.set_pen_color(i*4+2, 0, 0, 0);
		palette.set_pen_color(i*4+3, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}
}

void imolagp_state::video_start()
{
	memset(m_videoram, 0, sizeof(m_videoram));
	save_item(NAME(m_videoram));
}


UINT32 imolagp_state::screen_update_imolagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw solid background layer first, then sprites on top
	for (int layer = 0; layer < 2; layer++)
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			const UINT8 *source = &m_videoram[layer][(y & 0xff) * 0x40];
			UINT16 *dest = &bitmap.pix16(y & 0xff);
			for (int i = 0; i < 0x40; i++)
			{
				UINT8 data = source[i];
				if (data || layer == 0)
				{
					// one color per each 4 pixels
					UINT8 color = (data & 0xf0) >> 3;
					UINT8 x = (i << 2) - (m_scroll ^ 3);
					for (int x2 = 0; x2 < 4; x2++)
					{
						UINT8 offset = x + x2;
						if (offset >= cliprect.min_x && offset <= cliprect.max_x)
							dest[offset] = color | (data >> x2 & 1);
					}
				}
			}
		}
	}

	return 0;
}



/***************************************************************************

  Interrupts

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(imolagp_state::imolagp_pot_callback)
{
	int steer = m_steer_inp->read();
	if (steer & 0x7f)
	{
		if (~steer & 0x80)
		{
			// shift register when steering left
			steer = -steer;
			m_steerlatch = (m_steerlatch << 1) | (~m_steerlatch >> 1 & 1);
		}

		// steering speed is determined by timer period
		// these values(in usec) may need tweaking:
		const int base = 6500;
		const int range = 100000;
		m_steer_pot_timer->adjust(attotime::from_usec(base + range * (1.0 / (double)(steer & 0x7f))));
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	else
		m_steer_pot_timer->adjust(attotime::from_msec(20));
}

INTERRUPT_GEN_MEMBER(imolagp_state::slave_vblank_irq)
{
	m_scroll = m_vreg[0xe]; // latch scroll
	device.execute().set_input_line(0, HOLD_LINE);
}



/***************************************************************************

  I/O and Memory Maps

***************************************************************************/

/* The master CPU transmits data to the slave CPU one word at a time using a rapid sequence of triggered NMIs.
 * The slave CPU pauses as it enters its vblank irq, awaiting this burst of data.
 * Handling the NMI takes more time than triggering the NMI, implying that the slave CPU either runs at
 * a higher clock, or has a way to force the main CPU to wait.
 */
WRITE8_MEMBER(imolagp_state::transmit_data_w)
{
	m_comms_latch[offset] = data;
}

READ8_MEMBER(imolagp_state::receive_data_r)
{
	return m_comms_latch[offset];
}

READ8_MEMBER(imolagp_state::trigger_slave_nmi_r)
{
	m_slavecpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	return 0;
}


WRITE8_MEMBER(imolagp_state::imola_led_board_w)
{
	// not sure what chip is used here, this is copied from turbo.c
	static const UINT8 ls48_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	output().set_digit_value(offset, ls48_map[data & 0x0f]);
/*
    score:         0,  1,  2,  3
    time:          4,  5
    result:       10, 11
    credits:      12, 13
    highscore 1:  32, 33, 34, 35
    highscore 2:  36, 37, 24, 25
    highscore 3:  26, 27, 28, 29
    highscore 4:  16, 17, 18, 19
    highscore 5:  20, 21,  8,  9
*/
}


WRITE8_MEMBER(imolagp_state::vreg_control_w)
{
	m_vcontrol = data & 0xf;
}

READ8_MEMBER(imolagp_state::vreg_data_r)
{
	// auto-steer related
	return 0;
	//return 0xf7; // -> go left?
	//return 0x17; // it checks for this too
}

WRITE8_MEMBER(imolagp_state::vreg_data_w)
{
	// $07: always $ff?
	// $0e: x scroll
	// $0f: auto-steer related
	m_vreg[m_vcontrol] = data;
}


WRITE8_MEMBER(imolagp_state::screenram_w)
{
	// when in tunnel: $81/$82 -> sprite ram?
	if (m_draw_mode & 0x80)
		m_videoram[1][offset] = data;

	// sprites: $05
	else if (m_draw_mode & 0x01)
		m_videoram[1][offset] = data;

	// background: $06
	else
		m_videoram[0][offset] = data;
}

READ8_MEMBER(imolagp_state::imola_draw_mode_r)
{
	// the game reads a port before and after writing to screen ram
	m_draw_mode = offset;
	return 0;
}

static ADDRESS_MAP_START( imolagp_master_map, AS_PROGRAM, 8, imolagp_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(vreg_control_w)
	AM_RANGE(0x37f0, 0x37f0) AM_DEVWRITE("aysnd", ay8910_device, address_w)
//  AM_RANGE(0x37f7, 0x37f7) AM_NOP
	AM_RANGE(0x3800, 0x3800) AM_READWRITE(vreg_data_r, vreg_data_w)
	AM_RANGE(0x3810, 0x3810) AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("DSWA")
	AM_RANGE(0x47ff, 0x4800) AM_WRITE(transmit_data_w)
	AM_RANGE(0x5000, 0x50ff) AM_WRITE(imola_led_board_w)
	AM_RANGE(0x5800, 0x5800) AM_READ_PORT("DSWA") // assume mirror
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("DSWB")
ADDRESS_MAP_END

static ADDRESS_MAP_START( imolagp_master_io, AS_IO, 8, imolagp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(trigger_slave_nmi_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( imolagp_slave_map, AS_PROGRAM, 8, imolagp_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x9fff, 0xa000) AM_READ(receive_data_r)
	AM_RANGE(0xc000, 0xffff) AM_WRITE(screenram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( imolagp_slave_io, AS_IO, 8, imolagp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0xff) AM_READ(imola_draw_mode_r)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

CUSTOM_INPUT_MEMBER(imolagp_state::imolagp_steerlatch_r)
{
	return m_steerlatch & 0xf;
}

static INPUT_PORTS_START( imolagp )
	PORT_START("DSWA") /* 0x4000 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, "TEST A" )
	PORT_DIPSETTING(    0x40, "TEST C" )
	PORT_DIPSETTING(    0x60, "TEST D" )
	PORT_DIPSETTING(    0x80, "Memory" )
	PORT_DIPSETTING(    0xa0, "Color Test" )
	PORT_DIPSETTING(    0xc0, "Grid Test" )
	PORT_DIPSETTING(    0xe0, DEF_STR( Unused ) )

	PORT_START("DSWB") /* 0x6000 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, imolagp_state, imolagp_steerlatch_r, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x01, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(24)
INPUT_PORTS_END


static INPUT_PORTS_START( imolagpo )
	PORT_INCLUDE( imolagp )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, imolagp_state, imolagp_steerlatch_r, NULL)
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void imolagp_state::machine_start()
{
	save_item(NAME(m_vcontrol));
	save_item(NAME(m_vreg));
	save_item(NAME(m_scroll));
	save_item(NAME(m_steerlatch));
	save_item(NAME(m_draw_mode));
	save_item(NAME(m_comms_latch));
}

void imolagp_state::machine_reset()
{
	// reset steering wheel
	m_steerlatch = 0;
	m_steer_pot_timer->adjust(attotime::from_msec(20));
}


static MACHINE_CONFIG_START( imolagp, imolagp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3000000) // ? (assume slower than slave)
	MCFG_CPU_PROGRAM_MAP(imolagp_master_map)
	MCFG_CPU_IO_MAP(imolagp_master_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", imolagp_state, irq0_line_hold)
	MCFG_TIMER_DRIVER_ADD("pot", imolagp_state, imolagp_pot_callback) // maincpu nmi

	MCFG_CPU_ADD("slave", Z80, 4000000) // ?
	MCFG_CPU_PROGRAM_MAP(imolagp_slave_map)
	MCFG_CPU_IO_MAP(imolagp_slave_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", imolagp_state, slave_vblank_irq)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	// mode $91 - ports A & C-lower as input, ports B & C-upper as output
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(LOGGER("PPI8255 - unmapped read port B", 0))
	MCFG_I8255_OUT_PORTB_CB(LOGGER("PPI8255 - unmapped write port B", 0))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN1"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256,256)
	MCFG_SCREEN_VISIBLE_AREA(0+48,255,0+16,255)
	MCFG_SCREEN_UPDATE_DRIVER(imolagp_state, screen_update_imolagp)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x20)
	MCFG_PALETTE_INIT_OWNER(imolagp_state, imolagp)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 2000000) // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


ROM_START( imolagp ) // same hardware, but larger video roms
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "03.bin",   0x0000, 0x0800, CRC(a8be1c83) SHA1(e3147515f9f44e435192db838ffbb5c592f6e8d7) )
	ROM_LOAD( "01.bin",   0x0800, 0x0800, CRC(aa6fc1ea) SHA1(3f9c559aaba7b00ffd0210c6977dd4f966451a4b) )
	ROM_LOAD( "04.bin",   0x1000, 0x0800, CRC(c45c459c) SHA1(e51bbfe79bcd66d80b9179067611ea2029c9fd7a) )
	ROM_LOAD( "02.bin",   0x1800, 0x0800, CRC(a80e193b) SHA1(b31bf30dfe1bc498a4324719e4a6656fb94b8d96) )

	ROM_REGION( 0x10000, "slave", 0 ) /* Z80 code */
	ROM_LOAD( "12.bin",   0x0000, 0x0800, CRC(f9658100) SHA1(00fe32ef6b7cd909e8b69f0f8431c78591318aff) )
	ROM_LOAD( "08.bin",   0x0800, 0x0800, CRC(3a23a90e) SHA1(3a9ce5717147f2cf8c58432dd5ddcf70c2a041aa) )
	ROM_LOAD( "11.bin",   0x1000, 0x0800, CRC(5252e22e) SHA1(bb032af93f5a027235b35467c8a2c2c6fe6d1461) )
	ROM_LOAD( "07.bin",   0x1800, 0x0800, CRC(ce3459ff) SHA1(e336f9411cff71d85cdcc30af7405eca02c8c8f8) )
	ROM_LOAD( "10.bin",   0x2000, 0x0800, CRC(a043ded9) SHA1(0e3b53897da98ef3953622f8bd7dc379916ac0c0) )
	ROM_LOAD( "06.bin",   0x2800, 0x0800, CRC(3ff5997d) SHA1(95ac9dbda782b94b9b2dc4c9baea86113968077f) )
	ROM_LOAD( "09.bin",   0x3000, 0x0800, CRC(b5eb210e) SHA1(81340b1797f2401fbf0485091bf3b309c153476a) )
	ROM_LOAD( "05.bin",   0x3800, 0x0800, CRC(f59e426e) SHA1(ec9bdeed74e2450acb7f00abd13cb5ceb3205016) )
ROM_END

ROM_START( imolagpo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "yd.bin",   0x0000, 0x0800, CRC(5eb61bb7) SHA1(b897ecc7fa9aa1ae4e095d22d16a901b9d439a8e) )
	ROM_LOAD( "yc.bin",   0x0800, 0x0800, CRC(f7468a3b) SHA1(af1664e30b732b3d5321e76659961af3ebeb1237) )
	ROM_LOAD( "yb.bin",   0x1000, 0x0800, CRC(9f21506e) SHA1(6b46ff4815b8a02b190ec13e067f9a6687980774) )
	ROM_LOAD( "ya.bin",   0x1800, 0x0800, CRC(23fbcf14) SHA1(e8b5a9b01f715356c14aa41dbc9ca26732d3a4e4) )

	ROM_REGION( 0x10000, "slave", ROMREGION_ERASEFF ) /* Z80 code */
	ROM_LOAD( "xx.bin",   0x0000, 0x0400, CRC(059d6294) SHA1(38f075753e7a9fcabb857e5587e8a5966052cbcd) )
	// empty socket, please don't put NO_DUMP unless 100% sure
	ROM_LOAD( "xm.bin",   0x0800, 0x0400, CRC(64ebb7de) SHA1(fc5477bbedf44e93a578a71d2ff376f6f0b51a71) ) // ? gfx: B
	// empty socket, "
	ROM_LOAD( "xc.bin",   0x1000, 0x0400, CRC(397fd1f3) SHA1(e6b927933847ddcdbbcbeb5e5f37fea063356b24) )
	// empty socket, "
	// empty socket, "
	ROM_LOAD( "xi.bin",   0x1c00, 0x0400, CRC(ef54efa2) SHA1(c8464f11ccfd9eaf9aefb2cd3ac2b9e8bc2d11b6) ) // contains bitmap for "R.B."
	ROM_LOAD( "xy.bin",   0x2000, 0x0400, CRC(fea8e31e) SHA1(f85eac74d32ebd28170b466b136faf21a8ab220f) )
	ROM_LOAD( "xd.bin",   0x2400, 0x0400, CRC(0c601fc9) SHA1(e655f292b502a14068f5c35428001f8ceedf3637) )
	ROM_LOAD( "xs.bin",   0x2800, 0x0400, CRC(5d15ac52) SHA1(b4f97854018f72e4086c7d830d1b312aea1420a7) )
	ROM_LOAD( "xa.bin",   0x2c00, 0x0400, CRC(a95f5461) SHA1(2645fb93bc4ad5354eef5a385fa94021fb7291dc) ) // ? car - good?
	ROM_LOAD( "xp.bin",   0x3000, 0x0400, CRC(4b6d63ef) SHA1(16f9e31e588b989f5259ab59c0a3a2c7787f3a16) ) // ? gfx: AEIOSXTDNMVGYRPL
	ROM_LOAD( "xo.bin",   0x3400, 0x0400, CRC(c1d7f67c) SHA1(2ddfe9e59e323cd041fd760531b9e15ccd050058) ) // ? gfx: C
	ROM_LOAD( "xr.bin",   0x3800, 0x0400, CRC(8a8667aa) SHA1(53f34b6c5327d4398de644d7f318d460da56c2de) ) // ? gfx: sign+explosion
	ROM_LOAD( "xe.bin",   0x3c00, 0x0400, CRC(e0e81120) SHA1(14a77dfd069be342df4dbb1b747443c6d121d3fe) ) // ? car+misc
ROM_END


/*    YEAR,  NAME,     PARENT,  MACHINE, INPUT,    INIT,              MONITOR, COMPANY, FULLNAME, FLAGS */
GAMEL(1983?, imolagp,  0,       imolagp, imolagp,  driver_device, 0,  ROT90,   "RB Bologna", "Imola Grand Prix (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE, layout_imolagp ) // made by Alberici? year not shown, PCB labels suggests it's from 1983
GAMEL(1983?, imolagpo, imolagp, imolagp, imolagpo, driver_device, 0,  ROT90,   "RB Bologna", "Imola Grand Prix (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE, layout_imolagp ) // "
