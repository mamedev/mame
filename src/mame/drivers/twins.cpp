// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Twins
Electronic Devices, 1994

PCB Layout
----------

ECOGAMES Twins
_________________________________________________________
| LABEL: VIDEOR-191193                                   |
| ____________________         _____________             |
| | U24 40PIN DEVICE |         | IMSG176P-66|        ____|
| |__________________|         |____________|        |
|                                                    |___
| ___________   74HC574  ___________       74HC574   ----|
| | D43256AC |           | D43256AC |                ----|
| |__________|  74LS245  |__________|      74LS245   ----|
|                                                    ----|
| 74LS245       74HC573  74LS245           74HC573   ----|
|  ____________   ____________  __________________   ----|
|  | 84256A-10L|  | 84256A-10L| | AY-3-8910A      |  ----|
|  |___________|  |___________| |_________________|  ----|
| _____________  _____________                       ----|
| | 1 27C4001  | | 2 27C4001  |                      ----|
| |____________| |____________| ___________________  ----|
|                               |U30 40PIN DEVICE |  ----|
| 74LS245      74LS245  74HC573 |_________________|  ----|
|                                                    ----|
| 74HC573      74HC573  74HC74                       ____|
|  __________________                                |
|  |NEC V30 9327N5   |  74HC04      24C2AB1          |___
|  |_________________|         XTAL8MHz                  |
|________________________________________________________|


ELECTRONIC DEVICES Twins

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


Twins (Electronic Devices license, set 2) is significantly changed hardware, uses a regular RAMDAC hookup for plaette etc.


To access Service Mode in Spider you must boot with P1 Left and P1 Right held down,
this requires the -joystick_contradictory switch on the commandline.

*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "sound/ay8910.h"
#include "machine/i2cmem.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class twins_state : public driver_device
{
public:
	twins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_paletteram(*this, "paletteram")
		, m_palette(*this, "palette")
		, m_i2cmem(*this, "i2cmem")
		, m_spritesinit(0)
		, m_videorambank(0)
	{ }

	void spider(machine_config &config);
	void twinsed1(machine_config &config);
	void twins(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<uint16_t> m_paletteram;
	required_device<palette_device> m_palette;
	optional_device<i2cmem_device> m_i2cmem;
	uint16_t m_paloff;
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

	uint16_t m_mainram[0x10000 / 2];
	uint16_t m_videoram[0x10000 / 2];
	uint16_t m_videoram2[0x10000 / 2];
	uint16_t m_videorambank;

	uint32_t screen_update_twins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void video_start() override;
	uint16_t* m_rom16;
	uint8_t* m_rom8;

	void ramdac_map(address_map &map);
	void spider_io(address_map &map);
	void twinsed1_io(address_map &map);
	void twins_map(address_map &map);
	void twins_io(address_map &map);
};


void twins_state::machine_start()
{
	m_rom16 = (uint16_t*)memregion("maincpu")->base();
	m_rom8 = memregion("maincpu")->base();
}

/* port 4 is eeprom */
READ16_MEMBER(twins_state::twins_port4_r)
{
// doesn't work??
//  printf("%08x: twins_port4_r %04x\n", m_maincpu->pc(), mem_mask);
//  return m_i2cmem->read_sda();// | 0xfffe;

	return 0x0001;
}

WRITE16_MEMBER(twins_state::twins_port4_w)
{
//  printf("%08x: twins_port4_w %04x %04x\n", m_maincpu->pc(), data, mem_mask);
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
		r = bitswap<8>(r,7,6,5,0,1,2,3,4);

		g = (dat>>5) & 0x1f;
		g = bitswap<8>(g,7,6,5,0,1,2,3,4);

		b = (dat>>10) & 0x1f;
		b = bitswap<8>(b,7,6,5,0,1,2,3,4);

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
	uint16_t* vram;
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
		uint16_t *src = m_rom16;
		return src[offset];
	}
}


WRITE16_MEMBER(twins_state::spider_blitter_w)
{
	// this is very strange, we use the offset (address bits) not data bits to set values..
	// I get the impression this might actually overlay the entire address range, including RAM and regular VRAM?
	uint16_t* vram;
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
			uint8_t *src = m_rom8;

		//  printf("spider_blitter_w %08x %04x %04x (previous data width %d address %08x)\n", offset * 2, data, mem_mask, m_spriteswidth, m_spritesaddr);
			offset &= 0x7fff;

			for (int i = 0; i < m_spriteswidth; i++)
			{
				uint8_t data;

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


void twins_state::twins_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(twins_state::spider_blitter_r), FUNC(twins_state::spider_blitter_w));
}

void twins_state::twinsed1_io(address_map &map)
{
	map(0x0000, 0x0003).w("aysnd", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x0002, 0x0002).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x0004, 0x0005).rw(FUNC(twins_state::twins_port4_r), FUNC(twins_state::twins_port4_w));
	map(0x0006, 0x0007).w(FUNC(twins_state::twins_pal_w)).share("paletteram");
	map(0x000e, 0x000f).w(FUNC(twins_state::porte_paloff0_w));
}

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


uint32_t twins_state::screen_update_twins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x,count;
	static const int xxx=320,yyy=204;

	bitmap.fill(m_palette->black_pen());

	count=0;
	uint8_t *videoram = (uint8_t*)m_videoram;
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

uint32_t twins_state::screen_update_spider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x,count;
	static const int xxx=320,yyy=204;

	bitmap.fill(m_palette->black_pen());

	count=0;
	uint8_t *videoram = (uint8_t*)m_videoram;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			bitmap.pix16(y, x) = videoram[BYTE_XOR_LE(count)];
			count++;
		}
	}

	count = 0;
	videoram = (uint8_t*)m_videoram2;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			uint8_t pixel = videoram[BYTE_XOR_LE(count)];
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


void twins_state::twinsed1(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &twins_state::twins_map);
	m_maincpu->set_addrmap(AS_IO, &twins_state::twinsed1_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8000000, 512, 0, 320, 312, 0, 200); // 15.625 kHz horizontal???
	screen.set_screen_update(FUNC(twins_state::screen_update_twins));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	I2C_24C02(config, m_i2cmem);

	PALETTE(config, m_palette).set_entries(0x100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 2000000));
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


/* The Ecogames set and the Electronic Devices second set has different palette hardware
   and a different port map than Electronic Devices first set */

void twins_state::twins_io(address_map &map)
{
	map(0x0000, 0x0000).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0002, 0x0002).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0004, 0x0004).rw("ramdac", FUNC(ramdac_device::pal_r), FUNC(ramdac_device::pal_w));
	map(0x0008, 0x0008).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x0010, 0x0010).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x0018, 0x0019).r(FUNC(twins_state::twins_port4_r)).w(FUNC(twins_state::twins_port4_w));
}


void twins_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


void twins_state::twins(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, XTAL(16'000'000)/2); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &twins_state::twins_map);
	m_maincpu->set_addrmap(AS_IO, &twins_state::twins_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8000000, 512, 0, 320, 312, 0, 200); // 15.625 kHz horizontal???
	screen.set_screen_update(FUNC(twins_state::screen_update_twins));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, m_palette).set_entries(256);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &twins_state::ramdac_map);
	ramdac.set_split_read(0);

	I2C_24C02(config, m_i2cmem);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(16'000'000)/8)); /* verified on pcb */
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

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

	uint16_t* vram;
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


void twins_state::spider_io(address_map &map)
{
	map(0x0000, 0x0003).w("aysnd", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x0002, 0x0002).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x0004, 0x0005).rw(FUNC(twins_state::twins_port4_r), FUNC(twins_state::twins_port4_w));
	map(0x0008, 0x0009).w(FUNC(twins_state::spider_pal_w)).share("paletteram");
	map(0x0010, 0x0011).w(FUNC(twins_state::spider_paloff0_w));

	map(0x0018, 0x0019).r(FUNC(twins_state::spider_port_18_r));
	map(0x001a, 0x001b).w(FUNC(twins_state::spider_port_1a_w));
	map(0x001c, 0x001d).w(FUNC(twins_state::spider_port_1c_w));
	map(0x001e, 0x001f).r(FUNC(twins_state::spider_port_1e_r));


}


void twins_state::spider(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &twins_state::twins_map);
	m_maincpu->set_addrmap(AS_IO, &twins_state::spider_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8000000, 512, 0, 320, 312, 0, 200); // 15.625 kHz horizontal???
	screen.set_screen_update(FUNC(twins_state::screen_update_spider));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, m_palette).set_entries(0x100);

	I2C_24C02(config, m_i2cmem);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 2000000));
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


/* ECOGAMES Twins */
ROM_START( twins )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.u8", 0x000000, 0x080000, CRC(1ec942b0) SHA1(627deb739c50f93c4cb61b8baf2a07213f1613b3) )
	ROM_LOAD16_BYTE( "1.u9", 0x000001, 0x080000, CRC(4417ff34) SHA1(be992128fe48556a0a7c018953702b4ce9076526) )

	/* Unused */
	ROM_REGION( 0x000100, "extra", 0 )
	ROM_LOAD("24c02.u15", 0x000000, 0x000100, CRC(5ba30b14) SHA1(461f701879b76f1784705e067a5b6b31bfda4606) )
ROM_END

/** Electronic Devices Twins */
ROM_START( twinsed1 )
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

ROM_START( twinsed2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lp.bin", 0x000000, 0x080000, CRC(4f07862e) SHA1(fbda1973f79c6938c7f026a4db706e78781c2df8) )
	ROM_LOAD16_BYTE( "hp.bin", 0x000001, 0x080000, CRC(aaf74b83) SHA1(09bd76b9fc5cb7ba6ffe1a2581ffd5633fe440b3) )
ROM_END

ROM_START( spider )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20.bin", 0x000001, 0x080000, CRC(25e15f11) SHA1(b728f35c817f60a294e38d66559da8977b94a1f5) )
	ROM_LOAD16_BYTE( "21.bin", 0x000000, 0x080000, CRC(ff224206) SHA1(d8d45850983542e811facc917d016841fc56a97f) )
ROM_END

GAME( 1993, twins,    0,     twins,    twins, twins_state, empty_init, ROT0, "Ecogames",                              "Twins",                                     MACHINE_SUPPORTS_SAVE )
GAME( 1994, twinsed1, twins, twinsed1, twins, twins_state, empty_init, ROT0, "Ecogames (Electronic Devices license)", "Twins (Electronic Devices license, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, twinsed2, twins, twins,    twins, twins_state, empty_init, ROT0, "Ecogames (Electronic Devices license)", "Twins (Electronic Devices license, set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, spider,   0,     spider,   twins, twins_state, empty_init, ROT0, "Buena Vision",                          "Spider",                                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
