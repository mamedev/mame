/* Table Tennis Champions ?

 ___________________________________________________
|        __     _________ __________   __________  |
|__      |_|   |UM62256D| |UM62256D|   |UM62256D|  |
  |    LN358N  |________| |________|   |________|  |
 _|            _________ _________     __________  |
|_  _1_______  |TI     | |TI     |     |UM62256D|  |
|_  |27C020 |  |52C1HXW| |52ALROW|     |________|  |
|_  |_______|  |       | |       |     __________  |
|__            |       | |       |     |UM62256D|  |
 _| __         |_______| |_______|     |________|  |
|_  |_| 74HC74                                     |
|_  OKI        74LS244   _3_______  _5_______      |
|_  M6295      74LS244   |27C040 |  |27C040 |      |
|_    57C55LK            |_______|  |_______|      |
|_             ________  _2_______  _4_______      |
|_    57C55LK  |GM76C28| |27C040 |  |27C040 |      |
|_             |_______| |_______|  |_______|      |
|_    74LS244  ________                            |
|_             |GM76C28| PAL16L8B PAL16L8B 74LS245 |
|_             |_______|                           |
|_                       74LS244  74LS373N 74LS373 |
|_    74LS244  PIC16C84                            |
|_                       74HC273N 74LS373N 74LS245 |
|_             74HC74B1              _____________ |
  |                            OSC   |NEC V30    | |
 _|   74LS244  74LS14N 74HC74  16MHz |D70116C-10 | |
|__________________________________________________|

The PCB is Spanish and manufacured by Gamart.


Table tennis Championships by Gamart 1995

This game come from Gamart,an obscure spanish software house.
Hardware info:
main cpu: V30
sound chip: oki6295
custom chip: tpc1020bfn x2
osc: 16 mhz
Rom files definition:
ttennis2/3 main program
ttennis1 adpcm data
ttennis4/5 graphics
*there is a pic16c84 that i cannot dump because my programmer doesn't support it.

Dumped by tirino73 >isolani (at) interfree.it<




- works in a very similar way to 'Spider' (twins.c) 
  including the blitter (seems to be doubled up hardware tho, twice as many layers?)
- need to work out how it selects between upper/lower
  program roms as blitter source
- PIC is probably for sound
- more than one layer
- layer clearing

*/

#include "emu.h"
#include "cpu/nec/nec.h"


class ttchamp_state : public driver_device
{
public:
	ttchamp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")  { }

	UINT16* m_peno_mainram;

	UINT16 m_paloff;
	DECLARE_WRITE16_MEMBER(paloff_w);
	DECLARE_WRITE16_MEMBER(pcup_prgbank_w);
	DECLARE_WRITE16_MEMBER(paldat_w);

	DECLARE_WRITE16_MEMBER(port10_w);
	UINT16 m_port10;

	DECLARE_DRIVER_INIT(ttchamp);

	DECLARE_READ16_MEMBER(ttchamp_blit_start_r);

	DECLARE_READ16_MEMBER(ttchamp_mem_r);
	DECLARE_WRITE16_MEMBER(ttchamp_mem_w);

	UINT16 m_videoram0[0x10000 / 2];
	UINT16 m_videoram1[0x10000 / 2];
	UINT16 m_videoram2[0x10000 / 2];



	UINT16 m_mainram[0x10000 / 2];

	int m_spritesinit;
	int m_spriteswidth;
	int m_spritesaddr;

	virtual void machine_start();
	UINT16* m_rom16;
	UINT8* m_rom8;

	virtual void video_start();
	UINT32 screen_update_ttchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ttchamp_irq);
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};

void ttchamp_state::machine_start()
{
	m_rom16 = (UINT16*)memregion("maincpu")->base();
	m_rom8 = memregion("maincpu")->base();
}

void ttchamp_state::video_start()
{

}

UINT32 ttchamp_state::screen_update_ttchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	logerror("update\n");
	int y,x,count;

	static const int xxx=320,yyy=204;

	bitmap.fill(m_palette->black_pen());

	count=0;
	UINT8 *videoram = (UINT8*)m_videoram0;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			/*if(hotblock_port0&0x40)*/bitmap.pix16(y, x) = videoram[BYTE_XOR_LE(count)]+0x300;
			count++;
		}
	}

	count=0;
	videoram = (UINT8*)m_videoram1;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			UINT8 pix = videoram[BYTE_XOR_LE(count)];
			if (pix) bitmap.pix16(y, x) = pix+0x200;
			count++;
		}
	}

	count=0;
	videoram = (UINT8*)m_videoram2;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			UINT8 pix = videoram[BYTE_XOR_LE(count)];
			if (pix) bitmap.pix16(y, x) = pix+0x000;
			count++;
		}
	}

	for (int i = 0; i < 0x8000; i++)
	{
		// how are layers cleared?
		// I think it actually does more blit operations with
		// different bits of m_port10 set to redraw the backgrounds using the video ram data as a source rather than ROM - notice the garbage you see behind 'sprites' right now
		// this method also removes the text layer, which we don't want
		m_videoram1[i] = 0x0000;
		m_videoram2[i] = 0x0000;
	}

	return 0;
}


WRITE16_MEMBER(ttchamp_state::paloff_w)
{
	COMBINE_DATA(&m_paloff);
}


WRITE16_MEMBER(ttchamp_state::paldat_w)
{
	m_palette->set_pen_color(m_paloff & 0x7fff,pal5bit(data>>0),pal5bit(data>>5),pal5bit(data>>10));
}



READ16_MEMBER(ttchamp_state::ttchamp_mem_r)
{
	// bits 0xf0 are used too, so this is likely wrong.

	UINT16* vram;
	if ((m_port10&0xf) == 0x00)
		vram = m_videoram0;
	else if ((m_port10&0xf)  == 0x01)
		vram = m_videoram1;
	else if ((m_port10&0xf)  == 0x03)
		vram = m_videoram2;
	else
	{
		printf("unhandled video bank %02x\n", m_port10);
		vram = m_videoram2;
	}

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
		UINT16 *src = m_rom16 + (0x100000/2); // can the CPU ever see the lower bank?
		return src[offset];
	}
}

WRITE16_MEMBER(ttchamp_state::ttchamp_mem_w)
{
	// this is very strange, we use the offset (address bits) not data bits to set values..
	// I get the impression this might actually overlay the entire address range, including RAM and regular VRAM?

	// bits 0xf0 are used too, so this is likely wrong.

	UINT16* vram;
	if ((m_port10&0xf)  == 0x00)
		vram = m_videoram0;
	else if ((m_port10&0xf)  == 0x01)
		vram = m_videoram1;
	else if ((m_port10&0xf)  == 0x03)
		vram = m_videoram2;
	else
	{
		printf("unhandled video bank %02x\n", m_port10);
		vram = m_videoram2;
	}


	if (m_spritesinit == 1)
	{
	//	printf("spider_blitter_w %08x %04x %04x (init?) (base?)\n", offset * 2, data, mem_mask);

		m_spritesinit = 2;
		m_spritesaddr = offset;
	}
	else if (m_spritesinit == 2)
	{
	//	printf("spider_blitter_w %08x %04x %04x (init2) (width?)\n", offset * 2, data, mem_mask);
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
		else if (offset < 0x40000 / 2)
		{
			// 0x30000-0x3ffff and 0x40000-0x4ffff seem to be used here?
			offset &= 0x7fff;

			UINT8 *src = m_rom8;

			if (m_port10 & 2) // NO, wrong for the portraits
				src += 0x100000;

		//	printf("spider_blitter_w %08x %04x %04x (previous data width %d address %08x)\n", offset * 2, data, mem_mask, m_spriteswidth, m_spritesaddr);
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
			logerror("spider_blitter_w unhandled RAM access %08x %04x %04x\n", offset * 2, data, mem_mask);
		}
	}
}

READ16_MEMBER(ttchamp_state::ttchamp_blit_start_r)
{
	m_spritesinit = 1;
	return 0xff;
}

static ADDRESS_MAP_START( ttchamp_map, AS_PROGRAM, 16, ttchamp_state )
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(ttchamp_mem_r, ttchamp_mem_w)
ADDRESS_MAP_END

WRITE16_MEMBER(ttchamp_state::port10_w)
{
	COMBINE_DATA(&m_port10);
}


static ADDRESS_MAP_START( ttchamp_io, AS_IO, 16, ttchamp_state )
//	AM_RANGE(0x0000, 0x0001) AM_WRITENOP

	AM_RANGE(0x0002, 0x0003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0004, 0x0005) AM_READ_PORT("P1_P2")


	AM_RANGE(0x0018, 0x0019) AM_READ(ttchamp_blit_start_r)
	AM_RANGE(0x001e, 0x001f) AM_READNOP // read before each line is blit

	AM_RANGE(0x0008, 0x0009) AM_WRITE(paldat_w)
	AM_RANGE(0x000a, 0x000b) AM_WRITE(paloff_w)

	AM_RANGE(0x0010, 0x0011) AM_WRITE(port10_w)

//	AM_RANGE(0x0020, 0x0021) AM_WRITENOP

//	AM_RANGE(0x0034, 0x0035) AM_READ(peno_rand) AM_WRITENOP
ADDRESS_MAP_END


static INPUT_PORTS_START(ttchamp)
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "0x000003" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(ttchamp_state::ttchamp_irq)/* right? */
{
	device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( ttchamp, ttchamp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 8000000)
	MCFG_CPU_PROGRAM_MAP(ttchamp_map)
	MCFG_CPU_IO_MAP(ttchamp_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ttchamp_state,  ttchamp_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(1024,1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(ttchamp_state, screen_update_ttchamp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x8000)

MACHINE_CONFIG_END

ROM_START( ttchamp )
	ROM_REGION16_LE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.bin", 0x000000, 0x080000,  CRC(6a6c6d75) SHA1(3742b82462176d77732a69e142db9e6f61f25dc5) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x080000,  CRC(6062c0b2) SHA1(c5f0ac58c847ce2588c805f40180f2586a6477b7) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x080000,  CRC(4388dead) SHA1(1965e4b84452b244e32c8d218aace8d287c67ec2) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x080000,  CRC(fdbf9b28) SHA1(2d260555586097c8a396f65111f55ace801c7a5d) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* not verified if this is correct yet, seems very empty, maybe protected */
	ROM_LOAD( "pic16c84.rom", 0x000000, 0x4280,  CRC(900f2ef8) SHA1(08f206fe52f413437436e4b0d2b4ec310767446c) )

	ROM_REGION( 0x40000, "samples", 0 )
	ROM_LOAD( "27c020.1", 0x000000, 0x040000,  CRC(e2c4fe95) SHA1(da349035cc348db220a1e12b4c2a6021e2168425) )
ROM_END

ROM_START( ttchampa )
	ROM_REGION16_LE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ttennis2.bin", 0x000000, 0x080000,  CRC(b060e72c) SHA1(376e71bb4b1687fec4b719cbc5a7b25b64d159ac) )
	ROM_LOAD16_BYTE( "ttennis3.bin", 0x000001, 0x080000,  CRC(33e085a8) SHA1(ea6af05690b4b0803c303a3c858df10e4d907fb1) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x080000,  CRC(4388dead) SHA1(1965e4b84452b244e32c8d218aace8d287c67ec2) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x080000,  CRC(fdbf9b28) SHA1(2d260555586097c8a396f65111f55ace801c7a5d) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* not verified if this is correct yet, seems very empty, maybe protected */
	ROM_LOAD( "pic16c84.rom", 0x000000, 0x4280,  CRC(900f2ef8) SHA1(08f206fe52f413437436e4b0d2b4ec310767446c) )

	ROM_REGION( 0x40000, "samples", 0 )
	ROM_LOAD( "27c020.1", 0x000000, 0x040000,  CRC(e2c4fe95) SHA1(da349035cc348db220a1e12b4c2a6021e2168425) )
ROM_END

DRIVER_INIT_MEMBER(ttchamp_state,ttchamp)
{
//	UINT8 *ROM1 = memregion("user1")->base();
//	membank("bank1")->set_base(&ROM1[0x100000]);
//	membank("bank2")->set_base(&ROM1[0x180000]);
}

GAME( 1995, ttchamp, 0,        ttchamp, ttchamp, ttchamp_state, ttchamp, ROT0,  "Gamart",                               "Table Tennis Champions", GAME_NOT_WORKING|GAME_NO_SOUND ) // this has various advertising boards, including 'Electronic Devices' and 'Deniam'
GAME( 1995, ttchampa,ttchamp,  ttchamp, ttchamp, ttchamp_state, ttchamp, ROT0,  "Gamart (Palencia Elektronik license)", "Table Tennis Champions (Palencia Elektronik license)", GAME_NOT_WORKING|GAME_NO_SOUND ) // this only has Palencia Elektronik advertising boards
