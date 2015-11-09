// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Twins
Electronic Devices, 1994

PCB Layout
----------

This is a very tiny PCB,
only about 6 inches square.

|-----------------------|
|     6116   16MHz 62256|
|TEST 6116 24C02        |
|             PAL  62256|
|J   62256 |--------|   |
|A         |TPC1020 | 2 |
|M   62256 |AFN-084C|   |
|M         |        | 1 |
|A         |--------|   |
| AY3-8910              |
|                 D70116|
|-----------------------|
Notes:
    V30 clock      : 8.000MHz (16/2)
    AY3-8910 clock : 2.000MHz (16/8)
    VSync          : 50Hz



seems a similar board to Hot Blocks

same TPC1020 AFN-084C chip
same 24c02 eeprom
V30 instead of I8088
AY3-8910 instead of YM2149 (compatible)

video is not banked in this case instead palette data is sent to the ports
strange palette format.

todo:
hook up eeprom (doesn't seem to work when hooked up??)
Twins set 1 takes a long time to boot (eeprom?)
Improve blitter / clear logic for Spider.

Electronic Devices was printed on rom labels
1994 date string is in ROM

Spider PCB appears almost identical but uses additional 'blitter' features.
It is possible the Twins PCB has them too and doesn't use them.


Twins (set 2) is significantly changed hardware, uses a regular RAMDAC hookup for plaette etc.

To access Service Mode in Spider you must boot with P1 Left and P1 Right held down,
this requires the -joystick_contradictory switch on the commandline.


*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "sound/ay8910.h"
#include "machine/i2cmem.h"
#include "video/ramdac.h"

class twins_state : public driver_device
{
public:
	twins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_paletteram(*this, "paletteram"),
		m_palette(*this, "palette"),
		m_i2cmem(*this, "i2cmem"),
		m_spritesinit(0),
		m_videorambank(0)
		{ }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT16> m_paletteram;
	required_device<palette_device> m_palette;
	optional_device<i2cmem_device> m_i2cmem;
	UINT16 m_paloff;
	DECLARE_READ16_MEMBER(twins_port4_r);
	DECLARE_WRITE16_MEMBER(twins_port4_w);
	DECLARE_WRITE16_MEMBER(twins_pal_w);
	DECLARE_WRITE16_MEMBER(spider_pal_w);
	DECLARE_WRITE16_MEMBER(porte_paloff0_w);
	DECLARE_WRITE16_MEMBER(spider_paloff0_w);
	DECLARE_WRITE16_MEMBER(spider_blitter_w);
	DECLARE_READ16_MEMBER(spider_blitter_r);

	DECLARE_READ16_MEMBER(spider_port_18_r);
	DECLARE_READ16_MEMBER(spider_port_1e_r);
	DECLARE_WRITE16_MEMBER(spider_port_1a_w);
	DECLARE_WRITE16_MEMBER(spider_port_1c_w);
	int m_spritesinit;
	int m_spriteswidth;
	int m_spritesaddr;

	UINT16 m_mainram[0x10000 / 2];
	UINT16 m_videoram[0x10000 / 2];
	UINT16 m_videoram2[0x10000 / 2];
	UINT16 m_videorambank;

	UINT32 screen_update_twins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start();
	virtual void video_start();
	UINT16* m_rom16;
	UINT8* m_rom8;

};


void twins_state::machine_start()
{
	m_rom16 = (UINT16*)memregion("maincpu")->base();
	m_rom8 = memregion("maincpu")->base();
}

/* port 4 is eeprom */
READ16_MEMBER(twins_state::twins_port4_r)
{
// doesn't work??
//  printf("%08x: twins_port4_r %04x\n", space.device().safe_pc(), mem_mask);
//  return m_i2cmem->read_sda();// | 0xfffe;

	return 0x0001;
}

WRITE16_MEMBER(twins_state::twins_port4_w)
{
//  printf("%08x: twins_port4_w %04x %04x\n", space.device().safe_pc(), data, mem_mask);
	int i2c_clk = BIT(data, 1);
	int i2c_mem = BIT(data, 0);
	m_i2cmem->write_scl(i2c_clk);
	m_i2cmem->write_sda(i2c_mem);
}

WRITE16_MEMBER(twins_state::twins_pal_w)
{
	COMBINE_DATA(&m_paletteram[m_paloff]);

	{
		int dat,r,g,b;
		dat = m_paletteram[m_paloff];

		r = dat & 0x1f;
		r = BITSWAP8(r,7,6,5,0,1,2,3,4);

		g = (dat>>5) & 0x1f;
		g = BITSWAP8(g,7,6,5,0,1,2,3,4);

		b = (dat>>10) & 0x1f;
		b = BITSWAP8(b,7,6,5,0,1,2,3,4);

		m_palette->set_pen_color(m_paloff, pal5bit(r),pal5bit(g),pal5bit(b));

	}

	m_paloff = (m_paloff + 1) & 0xff;
}

/* ??? weird ..*/
WRITE16_MEMBER(twins_state::porte_paloff0_w)
{
//  printf("porte_paloff0_w %04x\n", data);
	m_paloff = 0;
}

READ16_MEMBER(twins_state::spider_blitter_r)
{
	UINT16* vram;
	if (m_videorambank & 1)
		vram = m_videoram2;
	else
		vram = m_videoram;

	if (offset < 0x10000 / 2)
	{
		return m_mainram[offset&0x7fff];
	}
	else if (offset < 0x20000 / 2)
	{
		return vram[offset&0x7fff];
	}
	else
	{
		UINT16 *src = m_rom16;
		return src[offset];
	}
}


WRITE16_MEMBER(twins_state::spider_blitter_w)
{
	// this is very strange, we use the offset (address bits) not data bits to set values..
	// I get the impression this might actually overlay the entire address range, including RAM and regular VRAM?
	UINT16* vram;
	if (m_videorambank & 1)
		vram = m_videoram2;
	else
		vram = m_videoram;

	if (m_spritesinit == 1)
	{
	//  printf("spider_blitter_w %08x %04x %04x (init?) (base?)\n", offset * 2, data, mem_mask);

		m_spritesinit = 2;
		m_spritesaddr = offset;
	}
	else if (m_spritesinit == 2)
	{
	//  printf("spider_blitter_w %08x %04x %04x (init2) (width?)\n", offset * 2, data, mem_mask);
		m_spriteswidth = offset & 0xff;
		if (m_spriteswidth == 0)
			m_spriteswidth = 80;

		m_spritesinit = 0;

	}
	else
	{
		if (offset < 0x10000 / 2)
		{
			COMBINE_DATA(&m_mainram[offset&0x7fff]);
		}
		else if (offset < 0x20000 / 2)
		{
			COMBINE_DATA(&vram[offset&0x7fff]);
		}
		else if (offset < 0x30000 / 2)
		{
			UINT8 *src = m_rom8;

		//  printf("spider_blitter_w %08x %04x %04x (previous data width %d address %08x)\n", offset * 2, data, mem_mask, m_spriteswidth, m_spritesaddr);
			offset &= 0x7fff;

			for (int i = 0; i < m_spriteswidth; i++)
			{
				UINT8 data;

				data = (src[(m_spritesaddr * 2) + 1]);

				if (data)
					vram[offset] = (vram[offset] & 0x00ff) | data << 8;


				data = src[(m_spritesaddr*2)];

				if (data)
					vram[offset] = (vram[offset] & 0xff00) | data;


				m_spritesaddr ++;
				offset++;

				offset &= 0x7fff;
			}
		}
		else
		{
			printf("spider_blitter_w unhandled RAM access %08x %04x %04x", offset * 2, data, mem_mask);
		}
	}
}


static ADDRESS_MAP_START( twins_map, AS_PROGRAM, 16, twins_state )
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(spider_blitter_r, spider_blitter_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( twins_io, AS_IO, 16, twins_state )
	AM_RANGE(0x0000, 0x0003) AM_DEVWRITE8("aysnd", ay8910_device, address_data_w, 0x00ff)
	AM_RANGE(0x0002, 0x0003) AM_DEVREAD8("aysnd", ay8910_device, data_r, 0x00ff)
	AM_RANGE(0x0004, 0x0005) AM_READWRITE(twins_port4_r, twins_port4_w)
	AM_RANGE(0x0006, 0x0007) AM_WRITE(twins_pal_w) AM_SHARE("paletteram")
	AM_RANGE(0x000e, 0x000f) AM_WRITE(porte_paloff0_w)
ADDRESS_MAP_END

void twins_state::video_start()
{
	m_paloff = 0;

	save_item(NAME(m_paloff));
	save_item(NAME(m_spritesinit));
	save_item(NAME(m_spriteswidth));
	save_item(NAME(m_spritesaddr));
	save_item(NAME(m_mainram));
	save_item(NAME(m_videoram));
	save_item(NAME(m_videoram2));
	save_item(NAME(m_videorambank));
}




UINT32 twins_state::screen_update_twins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x,count;
	static const int xxx=320,yyy=204;

	bitmap.fill(m_palette->black_pen());

	count=0;
	UINT8 *videoram = (UINT8*)m_videoram;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			bitmap.pix16(y, x) = videoram[BYTE_XOR_LE(count)];
			count++;
		}
	}
	return 0;
}

UINT32 twins_state::screen_update_spider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x,count;
	static const int xxx=320,yyy=204;

	bitmap.fill(m_palette->black_pen());

	count=0;
	UINT8 *videoram = (UINT8*)m_videoram;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			bitmap.pix16(y, x) = videoram[BYTE_XOR_LE(count)];
			count++;
		}
	}

	count = 0;
	videoram = (UINT8*)m_videoram2;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			UINT8 pixel = videoram[BYTE_XOR_LE(count)];
			if (pixel) bitmap.pix16(y, x) = pixel;
			count++;
		}
	}

	return 0;
}

static INPUT_PORTS_START(twins)
	PORT_START("P1")    /* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("P2")    /* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static MACHINE_CONFIG_START( twins, twins_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 8000000)
	MCFG_CPU_PROGRAM_MAP(twins_map)
	MCFG_CPU_IO_MAP(twins_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", twins_state,  nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(twins_state, screen_update_twins)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_24C02_ADD("i2cmem")

	MCFG_PALETTE_ADD("palette", 0x100)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("P1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("P2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* The second set has different palette hardware and a different port map */



static ADDRESS_MAP_START( twinsa_io, AS_IO, 16, twins_state )
	AM_RANGE(0x0000, 0x0001) AM_DEVWRITE8("ramdac",ramdac_device,index_w,0x00ff)
	AM_RANGE(0x0002, 0x0003) AM_DEVWRITE8("ramdac",ramdac_device,mask_w,0x00ff)
	AM_RANGE(0x0004, 0x0005) AM_DEVREADWRITE8("ramdac",ramdac_device,pal_r,pal_w,0x00ff)
	AM_RANGE(0x0008, 0x0009) AM_DEVWRITE8("aysnd", ay8910_device, address_w, 0x00ff)
	AM_RANGE(0x0010, 0x0011) AM_DEVREADWRITE8("aysnd", ay8910_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x0018, 0x0019) AM_READ(twins_port4_r) AM_WRITE(twins_port4_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ramdac_map, AS_0, 8, twins_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb666_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_START( twinsa, twins_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, XTAL_16MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(twins_map)
	MCFG_CPU_IO_MAP(twinsa_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", twins_state,  nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(twins_state, screen_update_twins)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")
	MCFG_RAMDAC_SPLIT_READ(0)

	MCFG_24C02_ADD("i2cmem")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_16MHz/8) /* verified on pcb */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("P1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("P2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

WRITE16_MEMBER(twins_state::spider_pal_w)
{
	// ths first write doesn't appear to be a palette value
	if (m_paloff!=0)
	{
		COMBINE_DATA(&m_paletteram[m_paloff-1]);
		int dat,r,g,b;
		dat = m_paletteram[m_paloff-1];

		r = dat & 0x1f;
		g = (dat>>5) & 0x1f;
		b = (dat>>10) & 0x1f;
		m_palette->set_pen_color(m_paloff-1, pal5bit(r),pal5bit(g),pal5bit(b));
	}
	else
	{
	//  printf("first palette write %04x\n", data);
	}

	m_paloff++;

	if (m_paloff == 0x101)
		m_paloff = 0;
}


WRITE16_MEMBER(twins_state::spider_paloff0_w)
{
	// this seems to be video ram banking
	COMBINE_DATA(&m_videorambank);
}

WRITE16_MEMBER(twins_state::spider_port_1a_w)
{
	// writes 1
}


WRITE16_MEMBER(twins_state::spider_port_1c_w)
{
	// done before the 'sprite' read / writes
	// might clear a buffer?

	// game is only animating sprites at 30fps, maybe there's some double buffering too?

//  data written is always 00, only seems to want the upper layer to be cleared
//  otherwise you get garbage sprites between rounds and the bg incorrectly wiped

	UINT16* vram;
//  if (m_videorambank & 1)
		vram = m_videoram2;
//  else
//      vram = m_videoram;

	for (int i = 0; i < 0x8000; i++)
	{
		vram[i] = 0x0000;
	}

}


READ16_MEMBER(twins_state::spider_port_18_r)
{
	// read before each blitter command
	// seems to put the bus in a state where the next 2 bus access offsets (anywhere) are the blitter params
	m_spritesinit = 1;

	return 0xff;
}

READ16_MEMBER(twins_state::spider_port_1e_r)
{
	// done before each sprite pixel 'write'
	// the data read is the data written, but only reads one pixel??
	return 0xff;
}


static ADDRESS_MAP_START( spider_io, AS_IO, 16, twins_state )
	AM_RANGE(0x0000, 0x0003) AM_DEVWRITE8("aysnd", ay8910_device, address_data_w, 0x00ff)
	AM_RANGE(0x0002, 0x0003) AM_DEVREAD8("aysnd", ay8910_device, data_r, 0x00ff)
	AM_RANGE(0x0004, 0x0005) AM_READWRITE(twins_port4_r, twins_port4_w)
	AM_RANGE(0x0008, 0x0009) AM_WRITE(spider_pal_w) AM_SHARE("paletteram")
	AM_RANGE(0x0010, 0x0011) AM_WRITE(spider_paloff0_w)

	AM_RANGE(0x0018, 0x0019) AM_READ(spider_port_18_r)
	AM_RANGE(0x001a, 0x001b) AM_WRITE(spider_port_1a_w)
	AM_RANGE(0x001c, 0x001d) AM_WRITE(spider_port_1c_w)
	AM_RANGE(0x001e, 0x001f) AM_READ(spider_port_1e_r)


ADDRESS_MAP_END





static MACHINE_CONFIG_START( spider, twins_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 8000000)
	MCFG_CPU_PROGRAM_MAP(twins_map)
	MCFG_CPU_IO_MAP(spider_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", twins_state,  nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(twins_state, screen_update_spider)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_24C02_ADD("i2cmem")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("P1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("P2"))

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( twins )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.bin", 0x000000, 0x080000, CRC(d5ef7b0d) SHA1(7261dca5bb0aef755b4f2b85a159b356e7ac8219) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x080000, CRC(8a5392f4) SHA1(e6a2ecdb775138a87d27aa4ad267bdec33c26baa) )
ROM_END

/*
Shang Hay Twins
Electronic Devices

1x nec9328n8-v30-d70116c-8 (main)
2x ay-3-8910a (sound)
1x blank (z80?)
1x oscillator 8.000

2x M27c4001

1x jamma edge connector
1x trimmer (volume)

hmm, we're only emulating 1x ay-3-8910, is the other at port 0 on this?

*/

ROM_START( twinsa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lp.bin", 0x000000, 0x080000, CRC(4f07862e) SHA1(fbda1973f79c6938c7f026a4db706e78781c2df8) )
	ROM_LOAD16_BYTE( "hp.bin", 0x000001, 0x080000, CRC(aaf74b83) SHA1(09bd76b9fc5cb7ba6ffe1a2581ffd5633fe440b3) )
ROM_END

ROM_START( spider )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20.bin", 0x000001, 0x080000, CRC(25e15f11) SHA1(b728f35c817f60a294e38d66559da8977b94a1f5) )
	ROM_LOAD16_BYTE( "21.bin", 0x000000, 0x080000, CRC(ff224206) SHA1(d8d45850983542e811facc917d016841fc56a97f) )
ROM_END

GAME( 1994, twins,  0,     twins,  twins, driver_device, 0, ROT0, "Electronic Devices", "Twins (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, twinsa, twins, twinsa, twins, driver_device, 0, ROT0, "Electronic Devices", "Twins (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, spider,  0,     spider,  twins, driver_device, 0, ROT0, "Buena Vision", "Spider", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
