// license:BSD-3-Clause
// copyright-holders:David Haywood
/********************************************************************

 PCB is marked 'Ver 1.8 B/D-' on 2 of the edges

 Custom chip marked
 ORIENTAL SOFT
  SPR800F1
    0011E

*********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"

class mjsenpu_state : public driver_device
{
public:
	mjsenpu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_oki(*this, "oki"),
			m_mainram(*this, "mainram"),
			m_vram(*this, "vram"),
			m_palette(*this, "palette")
	{
	}

	/* devices */
	required_device<e132xt_device> m_maincpu;
	required_device<okim6295_device> m_oki;

	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_vram;
	UINT8 m_pal[0x200];


	DECLARE_READ8_MEMBER(palette_low_r);
	DECLARE_READ8_MEMBER(palette_high_r);
	DECLARE_WRITE8_MEMBER(palette_low_w);
	DECLARE_WRITE8_MEMBER(palette_high_w);
	void set_palette(int offset);
	
	DECLARE_WRITE8_MEMBER(oki_bank_w);
	DECLARE_WRITE8_MEMBER(mux_w);

	DECLARE_READ32_MEMBER(mjsenpu_speedup_r);

	DECLARE_DRIVER_INIT(mjsenpu);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_mjsenpu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<palette_device> m_palette;
};


READ8_MEMBER(mjsenpu_state::palette_low_r)
{
	return m_pal[(offset * 2) + 0];
}


READ8_MEMBER(mjsenpu_state::palette_high_r)
{
	return m_pal[(offset * 2) + 1];
}

void mjsenpu_state::set_palette(int offset)
{
	UINT16 paldata = (m_pal[(offset * 2) + 0] << 8) | (m_pal[(offset * 2) + 1]);
	m_palette->set_pen_color(offset, pal5bit(paldata >> 0), pal5bit(paldata >> 5), pal6bit(paldata >> 10));
}

WRITE8_MEMBER(mjsenpu_state::palette_low_w)
{
	m_pal[(offset * 2)+0] = data;
	set_palette(offset);
}

WRITE8_MEMBER(mjsenpu_state::palette_high_w)
{
	m_pal[(offset * 2)+1] = data;
	set_palette(offset);
}

WRITE8_MEMBER(mjsenpu_state::oki_bank_w)
{
	// or some input muxing?
	if (data!=0x80 && data !=0x81)
		printf("oki bank_w %02x\n", data);
}

WRITE8_MEMBER(mjsenpu_state::mux_w)
{
//	printf("mux_w %02x\n", data);
}

static ADDRESS_MAP_START( mjsenpu_32bit_map, AS_PROGRAM, 32, mjsenpu_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x40000000, 0x401fffff) AM_ROM AM_REGION("user2",0) // main game rom

	AM_RANGE(0x80000000, 0x8001ffff) AM_RAM AM_SHARE("vram")

	AM_RANGE(0xffc00000, 0xffc000ff) AM_READWRITE8(palette_low_r, palette_low_w, 0xffffffff)
	AM_RANGE(0xffd00000, 0xffd000ff) AM_READWRITE8(palette_high_r, palette_high_w, 0xffffffff)

	AM_RANGE(0xffe00000, 0xffe0ffff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("user1",0) // boot rom
ADDRESS_MAP_END


static ADDRESS_MAP_START( mjsenpu_io, AS_IO, 32, mjsenpu_state )
	AM_RANGE(0x4000, 0x4003)  AM_READ_PORT("IN0")
	AM_RANGE(0x4010, 0x4013)  AM_READ_PORT("IN1")

	AM_RANGE(0x4020, 0x4023)  AM_WRITE8( oki_bank_w, 0x000000ff)

	AM_RANGE(0x4030, 0x4033)  AM_READ_PORT("DSW1")
	AM_RANGE(0x4040, 0x4043)  AM_READ_PORT("DSW2")
	AM_RANGE(0x4050, 0x4053)  AM_READ_PORT("DSW3")

	AM_RANGE(0x4060, 0x4063)  AM_WRITE8( mux_w, 0x000000ff)

	AM_RANGE(0x4070, 0x4073)  AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x000000ff)
ADDRESS_MAP_END

static INPUT_PORTS_START( mjsenpu )

	PORT_START("IN0")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN0" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_BET    )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 ) // or maybe service?
	PORT_DIPNAME( 0x00000002, 0x00000002, "IN1" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	// the input test mode seems broken in MAME? the first switch controls if the dipswitch shows at all, the 2nd switch displays 0/1 for the first position etc.
	// the actual test mode section which shows the effect of the dips works as expected tho.
	// maybe a game bug? maybe a core bug?
	PORT_START("DSW1")
	PORT_DIPNAME( 0x00000003, 0x00000003, "Ratio 1" )
	PORT_DIPSETTING(          0x00000000, "0" )
	PORT_DIPSETTING(          0x00000001, "1" )
	PORT_DIPSETTING(          0x00000002, "2" )
	PORT_DIPSETTING(          0x00000003, "3" )
	PORT_DIPNAME( 0x0000000c, 0x0000000c, "Value 1" )
	PORT_DIPSETTING(          0x00000000, "0" )
	PORT_DIPSETTING(          0x00000004, "1" )
	PORT_DIPSETTING(          0x00000008, "2" )
	PORT_DIPSETTING(          0x0000000c, "3" )
	PORT_DIPNAME( 0x00000030, 0x00000030, "Ratio 2" )
	PORT_DIPSETTING(          0x00000000, "0" )
	PORT_DIPSETTING(          0x00000010, "1" )
	PORT_DIPSETTING(          0x00000020, "2" )
	PORT_DIPSETTING(          0x00000030, "3" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, "Percentage 1" )
	PORT_DIPSETTING(          0x00000000, "0" )
	PORT_DIPSETTING(          0x00000040, "1" )
	PORT_DIPSETTING(          0x00000080, "2" )
	PORT_DIPSETTING(          0x000000c0, "3" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x00000003, 0x00000003, "Value 2" )
	PORT_DIPSETTING(          0x00000000, "0" )
	PORT_DIPSETTING(          0x00000001, "1" )
	PORT_DIPSETTING(          0x00000002, "2" )
	PORT_DIPSETTING(          0x00000003, "3" )
	PORT_DIPNAME( 0x00000004, 0x00000004, "Value 3" )
	PORT_DIPSETTING(          0x00000004, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPNAME( 0x00000008, 0x00000008, "Symbol 1" )
	PORT_DIPSETTING(          0x00000008, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000000e0, 0x000000e0, "Percentage 2" )
	PORT_DIPSETTING(          0x00000000, "0" )
	PORT_DIPSETTING(          0x00000020, "1" )
	PORT_DIPSETTING(          0x00000040, "2" )
	PORT_DIPSETTING(          0x00000060, "3" )
	PORT_DIPSETTING(          0x00000080, "4" )
	PORT_DIPSETTING(          0x000000a0, "5" )
	PORT_DIPSETTING(          0x000000c0, "6" )
	PORT_DIPSETTING(          0x000000e0, "7" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Value 4" )
	PORT_DIPSETTING(          0x00000001, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Symbol 2" )
	PORT_DIPSETTING(          0x00000002, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPNAME( 0x00000004, 0x00000004, "Symbol 3" )
	PORT_DIPSETTING(          0x00000004, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPNAME( 0x00000008, 0x00000008, "Symbol 4" )
	PORT_DIPSETTING(          0x00000008, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPNAME( 0x00000010, 0x00000010, "Symbol 5" )
	PORT_DIPSETTING(          0x00000010, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPNAME( 0x00000060, 0x00000060, "Percentage 3" )
	PORT_DIPSETTING(          0x00000000, "0" )
	PORT_DIPSETTING(          0x00000020, "1" )
	PORT_DIPSETTING(          0x00000040, "2" )
	PORT_DIPSETTING(          0x00000060, "3" )
	PORT_DIPNAME( 0x00000080, 0x00000080, "Symbol 6" )
	PORT_DIPSETTING(          0x00000080, "0" )
	PORT_DIPSETTING(          0x00000000, "1" )
INPUT_PORTS_END


void mjsenpu_state::video_start()
{
}



UINT32 mjsenpu_state::screen_update_mjsenpu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	int color;

	count = 0;
	for (y=0;y < 256;y++)
	{
		for (x=0;x < 512/4;x++)
		{
			color = m_vram[count] & 0x000000ff;
			bitmap.pix16(y, x*4 + 2) = color;

			color = (m_vram[count] & 0x0000ff00) >> 8;
			bitmap.pix16(y, x*4 + 3) = color;

			color = (m_vram[count] & 0x00ff0000) >> 16;
			bitmap.pix16(y, x*4 + 0) = color;

			color = (m_vram[count] & 0xff000000) >> 24;
			bitmap.pix16(y, x*4 + 1) = color;

			count++;
		}
	}
	return 0;
}


void mjsenpu_state::machine_start()
{
	save_item(NAME(m_pal));
}

void mjsenpu_state::machine_reset()
{
}

/* 
following clocks are on the PCB

22.1184
27.000
1.0000000

*/

static MACHINE_CONFIG_START( mjsenpu, mjsenpu_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", E132XT, 27000000*2) /* ?? Mhz */
	MCFG_CPU_PROGRAM_MAP(mjsenpu_32bit_map)
	MCFG_CPU_IO_MAP(mjsenpu_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mjsenpu_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(mjsenpu_state, screen_update_mjsenpu)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) /* 1 Mhz? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


ROM_START( mjsenpu )
	ROM_REGION32_BE( 0x80000, "user1", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "U1", 0x000000, 0x080000, CRC(ebfb1079) SHA1(9d676c635d5ee464df5730518399e141ebc515ed) )

	ROM_REGION32_BE( 0x200000, "user2", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "U13", 0x000000, 0x200000, CRC(a803c5a5) SHA1(61c7386a1bb6224b788de01293697d0e896839a8) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "SU2", 0x000000, 0x080000, CRC(848045d5) SHA1(4d32e1a5bd0937069dd8d50dfd8b63d4a45e40e6) )
ROM_END



READ32_MEMBER(mjsenpu_state::mjsenpu_speedup_r)
{
	int pc = m_maincpu->pc();
	
	if (pc == 0xadb8)
	{
		space.device().execute().spin_until_interrupt();
	}
	else
	{
	//	printf("%08x\n", pc);
	}

	return m_mainram[0x23468/4];
}




DRIVER_INIT_MEMBER(mjsenpu_state,mjsenpu)
{
/*
0000ADAE: LDHU.D L42, L38, $0
0000ADB2: LDW.A 0, L39, $23468
0000ADB8: ADDI L38, $1
0000ADBA: STHU.D L42, L38, $0
0000ADBE: CMPI L39, $0
0000ADC0: BNE $adae

   (loops for 744256 instructions)
*/
	// not especially effective, might be wrong.
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x23468, 0x2346b, read32_delegate(FUNC(mjsenpu_state::mjsenpu_speedup_r), this));
}




GAME( 2002, mjsenpu, 0, mjsenpu, mjsenpu, mjsenpu_state, mjsenpu, ROT0, "Oriental Soft", "Mahjong Senpu", MACHINE_NOT_WORKING )
