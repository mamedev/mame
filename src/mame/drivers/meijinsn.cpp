// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*
 Meijinsen (snk/alpha)
 ---------------------

 driver by Tomasz Slanina

It's something between typical alpha 68k hardware
(alpha mcu, sound hw (same as in jongbou)) and
old alpha shougi hardware (framebuffer).

There's probably no upright cabinet, only
cocktail table (controls in 2p mode are inverted).

Buttons:
 1st = 'decision'
 2nd = 'promotion'

 Service switch  = memory clear

--------------------------------------------------



                        p8      p7
   16mhz                p6      p5
                5816    p4      p3
                5816    p2      p1
                 ?
                          68000-8

        4416 4416 4416 4416
             clr             8910
     z80 p9 p10 2016



5816 = Sony CXK5816-10L (Ram for video)
2016 = Toshiba TMM2016AP-10 (SRAM for sound)
4416 = TI TMS4416-15NL (DRAM for MC68000)
clr  = TI TBP18S030N (32*8 Bipolar PROM)
Z80  = Sharp LH0080A Z80A-CPU-D
8910 = GI AY-3-8910A (Sound chip)
?    = Chip with Surface Scratched ....

"0" "1" MC68000 Program ROMs:
p1  p2
p3  p4
p5  p6
p7  p8

P9  = Z80 Program
P10 = AY-3-8910A Sounds

Text inside P9:

ALPHA DENSHI CO.,LTD  JUNE / 24 / 1986  FOR
* SHOUGI * GAME USED  SOUND BOARD CONTROL
SOFT  PSG & VOICE  BY M.C & S.H

*/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "video/resnet.h"
#include "sound/ay8910.h"

class meijinsn_state : public driver_device
{
public:
	meijinsn_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram"),
		m_shared_ram(*this, "shared_ram"){ }

	required_device<cpu_device> m_maincpu;
	/* memory pointers */
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_shared_ram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	UINT8    m_bg_bank;

	/* misc */
	UINT8 m_deposits1;
	UINT8 m_deposits2;
	UINT8 m_credits;
	UINT8 m_coinvalue;
	int m_mcu_latch;

	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_READ16_MEMBER(alpha_mcu_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(meijinsn);
	UINT32 screen_update_meijinsn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(meijinsn_interrupt);
};



WRITE16_MEMBER(meijinsn_state::sound_w)
{
	if (ACCESSING_BITS_0_7)
		soundlatch_byte_w(space, 0, data & 0xff);
}

READ16_MEMBER(meijinsn_state::alpha_mcu_r)
{
	static const UINT8 coinage1[2][2] = {{1,1}, {1,2}};
	static const UINT8 coinage2[2][2] = {{1,5}, {2,1}};

	int source = m_shared_ram[offset];

	switch (offset)
	{
		case 0: /* Dipswitch 2 */
			m_shared_ram[0] = (source & 0xff00) | ioport("DSW")->read();
			return 0;

		case 0x22: /* Coin value */
			m_shared_ram[0x22] = (source & 0xff00) | (m_credits & 0x00ff);
			return 0;

		case 0x29: /* Query microcontroller for coin insert */

			m_credits = 0;

			if ((ioport("COINS")->read() & 0x3) == 3)
				m_mcu_latch = 0;

			if ((ioport("COINS")->read() & 0x1) == 0 && !m_mcu_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | 0x22;  // coinA
				m_shared_ram[0x22] = (source & 0xff00) | 0x00;
				m_mcu_latch = 1;

				m_coinvalue = (~ioport("DSW")->read()>>3) & 1;

				m_deposits1++;
				if (m_deposits1 == coinage1[m_coinvalue][0])
				{
					m_credits = coinage1[m_coinvalue][1];
					m_deposits1 = 0;
				}
				else
					m_credits = 0;
			}
			else if ((ioport("COINS")->read() & 0x2) == 0 && !m_mcu_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | 0x22;  // coinA
				m_shared_ram[0x22] = (source & 0xff00) | 0x00;
				m_mcu_latch = 1;

				m_coinvalue = (~ioport("DSW")->read() >> 3) & 1;

				m_deposits2++;
				if (m_deposits2 == coinage2[m_coinvalue][0])
				{
					m_credits = coinage2[m_coinvalue][1];
					m_deposits2 = 0;
				}
				else
					m_credits = 0;
			}
			else
			{
				m_shared_ram[0x29] = (source & 0xff00) | 0x22;
			}
			return 0;
	}
	return 0;
}



static ADDRESS_MAP_START( meijinsn_map, AS_PROGRAM, 16, meijinsn_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080e00, 0x080fff) AM_READ(alpha_mcu_r) AM_WRITENOP
	AM_RANGE(0x100000, 0x107fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x180000, 0x180dff) AM_RAM
	AM_RANGE(0x180e00, 0x180fff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0x181000, 0x181fff) AM_RAM
	AM_RANGE(0x1c0000, 0x1c0001) AM_READ_PORT("P2")
	AM_RANGE(0x1a0000, 0x1a0001) AM_READ_PORT("P1") AM_WRITE(sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meijinsn_sound_map, AS_PROGRAM, 8, meijinsn_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( meijinsn_sound_io_map, AS_IO, 8, meijinsn_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(soundlatch_clear_byte_w)
	AM_RANGE(0x06, 0x06) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( meijinsn )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x7cc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SERVICE )

	PORT_START("P2")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, "Game time (actual game)" )
	PORT_DIPSETTING(    0x07, "1:00" )
	PORT_DIPSETTING(    0x06, "2:00" )
	PORT_DIPSETTING(    0x05, "3:00" )
	PORT_DIPSETTING(    0x04, "4:00" )
	PORT_DIPSETTING(    0x03, "5:00" )
	PORT_DIPSETTING(    0x02, "10:00" )
	PORT_DIPSETTING(    0x01, "20:00" )
	PORT_DIPSETTING(    0x00, "0:30" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, "A 1C/1C B 1C/5C" )
	PORT_DIPSETTING(    0x00, "A 1C/2C B 2C/1C" )
	PORT_DIPNAME( 0x10, 0x00, "2 Player" )
	PORT_DIPSETTING(    0x00, "1C" )
	PORT_DIPSETTING(    0x10, "2C" )
	PORT_DIPNAME( 0x20, 0x00, "Game time (tsumeshougi)" )
	PORT_DIPSETTING(    0x20, "1:00" )
	PORT_DIPSETTING(    0x00, "2:00" )

	PORT_START("COINS")  /* Coin input to microcontroller */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

void meijinsn_state::video_start()
{
}

PALETTE_INIT_MEMBER(meijinsn_state, meijinsn)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	static const int resistances_b[2]  = { 470, 220 };
	static const int resistances_rg[3] = { 1000, 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];


	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  0,  1000+1000,
			3,  resistances_rg, weights_g,  0,  1000+1000,
			2,  resistances_b,  weights_b,  0,  1000+1000);

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = combine_3_weights(weights_r, bit0, bit1, bit2);

		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = combine_3_weights(weights_g, bit0, bit1, bit2);

		/* blue component */
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		b = combine_2_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


UINT32 meijinsn_state::screen_update_meijinsn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = 0; offs < 0x4000; offs++)
	{
		int sx, sy, x, data1, data2, color, data;

		sx = offs >> 8;
		sy = offs & 0xff;

		data1 = m_videoram[offs] >> 8;
		data2 = m_videoram[offs] & 0xff;

		for (x = 0; x < 4; x++)
		{
			color= BIT(data1, x) | (BIT(data1, x + 4) << 1);
			data = BIT(data2, x) | (BIT(data2, x + 4) << 1);
			bitmap.pix16(sy, (sx * 4 + (3 - x))) = color * 4 + data;
		}
	}
	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(meijinsn_state::meijinsn_interrupt)
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

void meijinsn_state::machine_start()
{
	save_item(NAME(m_deposits1));
	save_item(NAME(m_deposits2));
	save_item(NAME(m_credits));
}

void meijinsn_state::machine_reset()
{
	m_deposits1 = 0;
	m_deposits2 = 0;
	m_credits   = 0;
}


static MACHINE_CONFIG_START( meijinsn, meijinsn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 9000000 )
	MCFG_CPU_PROGRAM_MAP(meijinsn_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", meijinsn_state, meijinsn_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(meijinsn_sound_map)
	MCFG_CPU_IO_MAP(meijinsn_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(meijinsn_state, irq0_line_hold,  160*60)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12, 243, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(meijinsn_state, screen_update_meijinsn)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(meijinsn_state, meijinsn)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

MACHINE_CONFIG_END


ROM_START( meijinsn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p1", 0x00000, 0x04000, CRC(8c9697a3) SHA1(19258e20a6aaadd6ba3469079fef85bc6dba548c) )
	ROM_CONTINUE   (       0x20000,  0x4000 )
	ROM_LOAD16_BYTE( "p2", 0x00001, 0x04000, CRC(f7da3535) SHA1(fdbacd075d45abda782966b16b3ea1ad68d60f91) )
	ROM_CONTINUE   (       0x20001,  0x4000 )
	ROM_LOAD16_BYTE( "p3", 0x08000, 0x04000, CRC(0af0b266) SHA1(d68ed31bc932bc5e9c554b2c0df06a751dc8eb96) )
	ROM_CONTINUE   (       0x28000,  0x4000 )
	ROM_LOAD16_BYTE( "p4", 0x08001, 0x04000, CRC(aab159c5) SHA1(0c9cad8f9893f4080b498433068e9324c7f2e13c) )
	ROM_CONTINUE   (       0x28001,  0x4000 )
	ROM_LOAD16_BYTE( "p5", 0x10000, 0x04000, CRC(0ed10a47) SHA1(9e89ec69f1f4e1ffa712f2e0c590d067c8c63026) )
	ROM_CONTINUE   (       0x30000,  0x4000 )
	ROM_LOAD16_BYTE( "p6", 0x10001, 0x04000, CRC(60b58755) SHA1(1786fc1b4c6d1793fb8e9311356fa4119611cfae) )
	ROM_CONTINUE   (       0x30001,  0x4000 )
	ROM_LOAD16_BYTE( "p7", 0x18000, 0x04000, CRC(604c76f1) SHA1(37fdf904f5e4d69dc8cb711cf3dece8f3075254a) )
	ROM_CONTINUE   (       0x38000,  0x4000 )
	ROM_LOAD16_BYTE( "p8", 0x18001, 0x04000, CRC(e3eaef19) SHA1(b290922f252a790443109e5023c3c35b133275cc) )
	ROM_CONTINUE   (       0x38001,  0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "p9", 0x00000, 0x04000, CRC(aedfefdf) SHA1(f9d35737a0e942fe7d483f87c52efa92a1bbb3e5) )
	ROM_LOAD( "p10",0x04000, 0x04000, CRC(93b4d764) SHA1(4fedd3fd1f3ef6c5f60ca86219f877df68d3027d) )

	ROM_REGION( 0x20, "proms", 0 ) /* Colour PROM */
	ROM_LOAD( "clr", 0x00, 0x20, CRC(7b95b5a7) SHA1(c15be28bcd6f5ffdde659f2d352ae409f04b2557) )
ROM_END

GAME( 1986, meijinsn, 0, meijinsn, meijinsn, driver_device, 0, ROT0, "SNK", "Meijinsen", MACHINE_SUPPORTS_SAVE )
