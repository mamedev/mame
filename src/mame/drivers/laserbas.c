/********************************************
 Laser Base / Future Flash driver

 IC marked as Z1 is probably protection device
 mapped in memory region f800-fbff
 (similar to the one used in Parallel Turn)

 Reads form this device depends on previous
 writes (adr, data), address and previous
 operation (read or write).
 Pinout is almost identical to 2716 - type EPROM,
 except separated /RD and /WR   signals and lacks
 of /CS

 Tomasz Slanina analog [at] op.pl

============================================

DASM notes:

0x100: check if test mode bit is active.
0x3ae8: ?
0x3aec: tests 0xfc00 work ram ONLY, resets if fails
0x3afe: fill 0xfc00-0xffff to zero
0x20dc: writes ROM 0x3146 to prot RAM 0xf800-0xfbff
0x20e9: reads from 0xfa47, A = (n & 0x8) | 0x80 then HL = 0x0200 | A
0x2cef: unknown, reads from 0x02** to 0x2d00, fancy ROM checksum?
...
0x0577

********************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/pit8253.h"

class laserbas_state : public driver_device
{
public:
	laserbas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_protram(*this, "protram"){ }

	/* misc */
	int      m_count;

	/* video-related */
	int      m_vrambank;
	UINT8    m_vram1[0x8000];
	UINT8    m_vram2[0x8000];
	required_shared_ptr<UINT8> m_protram;
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(read_unk);
	DECLARE_WRITE8_MEMBER(vrambank_w);
	DECLARE_READ8_MEMBER(protram_r);
	DECLARE_WRITE8_MEMBER(protram_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_laserbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


void laserbas_state::video_start()
{

	save_item(NAME(m_vram1));
	save_item(NAME(m_vram2));
}

UINT32 laserbas_state::screen_update_laserbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	for (y = 0; y < 256; y++)
		for(x = 0; x < 128; x++)
		{
			if (m_vram2[y * 128 + x] & 0xf)
				bitmap.pix16(y, x * 2) = (m_vram2[y * 128 + x] & 0xf);
			else
				bitmap.pix16(y, x * 2) = (m_vram1[y * 128 + x] & 0xf) + 16;

			if (m_vram2[y * 128 + x] >> 4)
				bitmap.pix16(y, x * 2 + 1) = (m_vram2[y * 128 + x] >> 4);
			else
				bitmap.pix16(y, x * 2 + 1) = (m_vram1[y * 128 + x] >> 4) + 16;
		}
	return 0;
}

READ8_MEMBER(laserbas_state::vram_r)
{

	if(!m_vrambank)
		return m_vram1[offset];
	else
		return m_vram2[offset];
}

WRITE8_MEMBER(laserbas_state::vram_w)
{

	if(!m_vrambank)
		m_vram1[offset] = data;
	else
		m_vram2[offset] = data;
}

#if 0
READ8_MEMBER(laserbas_state::read_unk)
{

	m_count ^= 0x80;
	return m_count | 0x7f;
}
#endif

WRITE8_MEMBER(laserbas_state::vrambank_w)
{

	/* either bit 2 or 3 controls flip screen */

	m_vrambank = data & 0x40;
}

READ8_MEMBER(laserbas_state::protram_r)
{

	return m_protram[offset];
}

WRITE8_MEMBER(laserbas_state::protram_w)
{

	m_protram[offset] = data;
}

static ADDRESS_MAP_START( laserbas_memory, AS_PROGRAM, 8, laserbas_state )
	//ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0xbfff) AM_READWRITE(vram_r, vram_w)
	AM_RANGE(0xc000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(protram_r, protram_w) AM_SHARE("protram") /* protection device */
	AM_RANGE(0xfc00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( laserbas_io, AS_IO, 8, laserbas_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(vrambank_w)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("IN1") // DSW + something else?
	AM_RANGE(0x21, 0x21) AM_READ_PORT("IN0")
	AM_RANGE(0x22, 0x22) AM_READ_PORT("IN2")
//  AM_RANGE(0x23, 0x23) AM_WRITE_LEGACY(test_w) bit 2 presumably is a mux for 0x20?
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE_LEGACY("pit0", pit8253_r, pit8253_w)
	AM_RANGE(0x44, 0x47) AM_DEVREADWRITE_LEGACY("pit1", pit8253_r, pit8253_w)
	AM_RANGE(0x80, 0x9f) AM_RAM_WRITE(paletteram_RRRGGGBB_byte_w) AM_SHARE("paletteram")
ADDRESS_MAP_END

static INPUT_PORTS_START( laserbas )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "0-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "0-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x10, "0-3" ) // another coin chute
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Test Mode" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1") // DSW
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void laserbas_state::machine_start()
{

	save_item(NAME(m_vrambank));
	save_item(NAME(m_count));
}

void laserbas_state::machine_reset()
{

	m_vrambank = 0;
	m_count = 0;
}

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


/* TODO: clocks aren't known */
static const struct pit8253_config laserbas_pit8253_intf_0 =
{
	{
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};

static const struct pit8253_config laserbas_pit8253_intf_1 =
{
	{
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};

static MACHINE_CONFIG_START( laserbas, laserbas_state )

	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(laserbas_memory)
	MCFG_CPU_IO_MAP(laserbas_io)
	MCFG_CPU_VBLANK_INT("screen",irq0_line_hold)

	MCFG_PIT8253_ADD("pit0", laserbas_pit8253_intf_0)
	MCFG_PIT8253_ADD("pit1", laserbas_pit8253_intf_1)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(laserbas_state, screen_update_laserbas)

	MCFG_MC6845_ADD("crtc", H46505, 3000000/4, mc6845_intf)	/* unknown clock, hand tuned to get ~60 fps */

	MCFG_PALETTE_LENGTH(32)
MACHINE_CONFIG_END

/*
Amstar LaserBase 1981 (Hoei)

XBC-101-00-1 - CPU board
  Z80A (D780C-1)
  2 NB5K8253
  2 2114 Rams
  8 Dipswitches

XBC-102-01-1 - RAM board?
  HD46505R
  32 MB8118 Rams

------------------------------------

Filename  Label Type   CSum Description
--------- ----- ------ ---- -----------------------------
MB8532.1    1   2532   9316
MB8532.2    2   2532   5662
MB8532.3    3   2532   7E9F
MB8532.4    4   2532   7CAD
MB8532.5    5   2532   C7D4 (Marked F.F.)
MB8532.6    6   2532   16BE
MB8532.7    7   2532   CF6C (Marked F.F.)
MB8716.8    8   2716   9601 (Marked F.F.)
TI2716.Z1  Z1   TI2716 D925
--------- ----- ------ ---- -----------------------------

I believe the F.F. markings on these chips show that
these roms have been changed to Future Flash.

It is unknown what the Z1 chip is, but the label screened
on the board under the socket says 2716.  All the identifying
numbers have been scratched off and has Z1 stamped on it.
It appears that each one was then numbered by hand in red
marker and stamped with white ink with Z1.

The Z1 chip was read from 3 different boards, it is Valid.
The chips were numbered 69, 82 & 624, all three read the same.
Turns out it was a TI2716.  The TI chip has A10 on a different
pin than a standard 2716.

*/

ROM_START( laserbas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb8532.1",   0x0000, 0x1000, CRC(ef85e0c5) SHA1(c26da919c206a23eb6e53ffe39acd5a5dfd47c18) )
	ROM_LOAD( "mb8532.2",   0x1000, 0x1000, CRC(0ba2236c) SHA1(416e4be957c395b05537d2e513e0c4561d8ca7c5) )
	ROM_LOAD( "mb8532.3",   0x2000, 0x1000, CRC(83998e0b) SHA1(ac435fb8dd67aec0737d6c750c527841b2b91a5b) )
	ROM_LOAD( "mb8532.4",   0x3000, 0x1000, CRC(00f9d909) SHA1(90b800cc5fcea53454584f8ad93eebbd193bdd21) )
	ROM_LOAD( "lb2532.5",   0xc000, 0x1000, CRC(6459073e) SHA1(78b8a23534826dd2d3b3c6c5d5708c8a78a4b6bf) )
	ROM_LOAD( "lb2532.6",   0xd000, 0x1000, CRC(a2dc1e7e) SHA1(78643a3aa852c73dab12e09a6cfc53141c936d12) )
	ROM_LOAD( "mb8532.7",   0xe000, 0x1000, CRC(9d2148d7) SHA1(24954d82a09d9fcfdc61e91b7c824daa5dd701c3) )
	ROM_LOAD( "mb8516.8",   0xf000, 0x0800, CRC(623f558f) SHA1(be6c6565df658555f21c43a8c2459cf399794a84) )
ROM_END

ROM_START( laserbasa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732.u1",       0x0000, 0x1000, CRC(f3ab00dc) SHA1(4730b13b55c93c71ed483463e180e71e506cfbd6) )
	ROM_LOAD( "2732.u2",       0x1000, 0x1000, CRC(0ba2236c) SHA1(416e4be957c395b05537d2e513e0c4561d8ca7c5) )
	ROM_LOAD( "mb8532.u3",     0x2000, 0x1000, CRC(c58a7745) SHA1(382e2235d89520860335c1c2760339e116c0466f) )
	ROM_LOAD( "mbm2732.u4",    0x3000, 0x1000, CRC(00f9d909) SHA1(90b800cc5fcea53454584f8ad93eebbd193bdd21) )
	ROM_LOAD( "2732.u5",       0xc000, 0x1000, CRC(6459073e) SHA1(78b8a23534826dd2d3b3c6c5d5708c8a78a4b6bf) )
	ROM_LOAD( "2732.u6",       0xd000, 0x1000, CRC(a2dc1e7e) SHA1(78643a3aa852c73dab12e09a6cfc53141c936d12) )
	ROM_LOAD( "2732.u7",       0xe000, 0x1000, CRC(9d2148d7) SHA1(24954d82a09d9fcfdc61e91b7c824daa5dd701c3) )
	ROM_LOAD( "mb8516.u8",     0xf000, 0x0800, CRC(623f558f) SHA1(be6c6565df658555f21c43a8c2459cf399794a84) )
ROM_END

/*
It was unclear what type of device FF.9 was. The silkscreen on the PCB said
2716,
but the device is a masked ROM with its identifying marks rubbed off.
I dumped it
as a 2716 (FF.9), a 2532 like the others (FF.9A) and a 2732 (FF.9B).
*/

ROM_START( futflash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ff.1",   	  0x0000, 0x1000, CRC(bcd6b998) SHA1(4a210c40ce6015e2b921558b7571b7f2a27e1815) )
	ROM_LOAD( "ff.2",         0x1000, 0x1000, CRC(1b1f6953) SHA1(8cd7b7e2236700ce63c60b4d2286099c8091bdbd) )
	ROM_LOAD( "ff.3",         0x2000, 0x1000, CRC(30008f04) SHA1(e03b2dbcb6d2615650cdd47ecf1d587906ce149b) )
	ROM_LOAD( "ff.4",         0x3000, 0x1000, CRC(e559aa12) SHA1(0fecfb60b0147e8060c640f684f69503478200ff) )
	ROM_LOAD( "ff.5",         0xc000, 0x1000, CRC(6459073e) SHA1(78b8a23534826dd2d3b3c6c5d5708c8a78a4b6bf) )
	ROM_LOAD( "ff.6",         0xd000, 0x1000, CRC(a8b17f49) SHA1(aea349bd19d001233bfb1805e586c950275010b4) )
	ROM_LOAD( "ff.7",         0xe000, 0x1000, CRC(9d2148d7) SHA1(24954d82a09d9fcfdc61e91b7c824daa5dd701c3) )
	ROM_LOAD( "ff.8",         0xf000, 0x0800, CRC(623f558f) SHA1(be6c6565df658555f21c43a8c2459cf399794a84) )
ROM_END

GAME( 1981, laserbas, 0,        laserbas, laserbas, driver_device, 0, ROT270, "Hoei (Amstar license)", "Laser Base (set 1)", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1981, laserbasa,laserbas, laserbas, laserbas, driver_device, 0, ROT270, "Hoei (Amstar license)", "Laser Base (set 2)", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1981, futflash, laserbas, laserbas, laserbas, driver_device, 0, ROT270, "Hoei",                  "Future Flash",       GAME_NO_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
