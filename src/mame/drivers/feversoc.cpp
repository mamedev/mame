// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Nicola Salmoria
/*******************************************************************************************

Fever Soccer (c) 2004 Seibu Kaihatsu

A down-grade of the Seibu SPI Hardware with SH-2 as main cpu.

driver by Angelo Salese & Nicola Salmoria

TODO:
- Add eeprom emulation;
- Real Time Clock emulation (uses a JRC 6355E / NJU6355E)

============================================================================

Fever Soccer (JAMMA based Gambling Game)

Seibu Kaihatsu Inc.

PCB (c) 2004 SYS_SH2B + SYS_SH2B Rom Board

Very simple PCB contains:

     CPU: Hitachi SH-2 (HD6417604F28)
   Audio: OKI 6295 (rebadged as AD-65)
GFX CHIP: RISE11 (custom graphics chip with programmable decryption)
  EEPROM: ST93C56A
     OSC: 28.63636MHz
     DSW: Single 4 switch

Other: Battery (CR2032)
       Sigma XLINX 9572
       JRC 6355E Serial Real Time Clock (connected to a 32.768KHz OSC)

RAM: BSI BS62LV1024SC-70 (x2)
     EtronTech EM51256C-15J (x4)


PRG0.U0139 ST M27C1001 (PCB can handle up to 27C080)
PRG1.U0140 ST M27C1001

PCM.U0743 ST M27C4001

On the SYS_SH2B ROM BOARD:
OBJ1.U011 ST M27C160
OBJ2.U012 ST M27C160
OBJ3.U013 ST M27C160

Not used / unpopulated:

U0145 LH28F800SU (Alt program ROM near SH-2)

U0744 LH28F800SU (Alt PCM ROM near AD-65)

U0561 LH28F800SU OBJ1-1 (near ROM BOARD connector & RISE11 chip)
U0562 LH28F800SU OBJ2-1
U0563 LH28F800SU OBJ3-1
U0564 LH28F800SU OBJ4-1

*******************************************************************************************/

#include "emu.h"
#include "cpu/sh2/sh2.h"
#include "machine/seibuspi.h"
#include "sound/okim6295.h"


class feversoc_state : public driver_device
{
public:
	feversoc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT16 m_x;
	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_spriteram;
	DECLARE_READ32_MEMBER(in0_r);
	DECLARE_WRITE32_MEMBER(output_w);
	DECLARE_DRIVER_INIT(feversoc);
	virtual void video_start() override;
	UINT32 screen_update_feversoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(feversoc_irq);
	required_device<sh2_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


#define MASTER_CLOCK XTAL_28_63636MHz

void feversoc_state::video_start()
{
}

UINT32 feversoc_state::screen_update_feversoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT32 *spriteram32 = m_spriteram;
	int offs,spr_offs,colour,sx,sy,h,w,dx,dy;

	bitmap.fill(m_palette->pen(0), cliprect); //black pen

	for(offs=(0x2000/4)-2;offs>-1;offs-=2)
	{
		spr_offs = (spriteram32[offs+0] & 0x3fff);
		if(spr_offs == 0)
			continue;
		sy = (spriteram32[offs+1] & 0x01ff);
		sx = (spriteram32[offs+1] & 0x01ff0000)>>16;
		colour = (spriteram32[offs+0] & 0x003f0000)>>16;
		w = ((spriteram32[offs+0] & 0x07000000)>>24)+1;
		h = ((spriteram32[offs+0] & 0x70000000)>>28)+1;

		if( sy & 0x100)
			sy-=0x200;

		for(dx=0;dx<w;dx++)
			for(dy=0;dy<h;dy++)
				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,spr_offs++,colour,0,0,(sx+dx*16),(sy+dy*16),0x3f);
	}

	return 0;
}



READ32_MEMBER(feversoc_state::in0_r)
{
	UINT32 io0 = (ioport("IN1")->read()&0xffff) << 16;
	UINT32 io1 = (ioport("IN0")->read()&0xffff) << 0;
	m_x^=0x40; //vblank? eeprom read bit?
	return io0 | io1 | m_x;
}

WRITE32_MEMBER(feversoc_state::output_w)
{
	if(ACCESSING_BITS_16_31)
	{
		/* probably eeprom stuff too */
		machine().bookkeeping().coin_lockout_w(0,~data>>16 & 0x40);
		machine().bookkeeping().coin_lockout_w(1,~data>>16 & 0x40);
		machine().bookkeeping().coin_counter_w(0,data>>16 & 1);
		//data>>16 & 2 coin out
		machine().bookkeeping().coin_counter_w(1,data>>16 & 4);
		//data>>16 & 8 coin hopper
		m_oki->set_bank_base(0x40000 * (((data>>16) & 0x20)>>5));
	}
	if(ACCESSING_BITS_0_15)
	{
		/* -xxx xxxx lamps*/
		machine().bookkeeping().coin_counter_w(2,data & 0x2000); //key in
		//data & 0x4000 key out
	}
}

static ADDRESS_MAP_START( feversoc_map, AS_PROGRAM, 32, feversoc_state )
	AM_RANGE(0x00000000, 0x0003ffff) AM_ROM
	AM_RANGE(0x02000000, 0x0203dfff) AM_RAM AM_SHARE("workram") //work ram
	AM_RANGE(0x0203e000, 0x0203ffff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x06000000, 0x06000003) AM_WRITE(output_w)
	AM_RANGE(0x06000004, 0x06000007) AM_WRITENOP //???
	AM_RANGE(0x06000008, 0x0600000b) AM_READ(in0_r)
	AM_RANGE(0x0600000c, 0x0600000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff0000)
//  AM_RANGE(0x06010000, 0x06017fff) AM_RAM //contains RISE11 keys and other related stuff.
//  AM_RANGE(0x06010060, 0x06010063) //bit 0 almost certainly irq ack
	AM_RANGE(0x06018000, 0x06019fff) AM_RAM_DEVWRITE("palette",  palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

static const gfx_layout spi_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3)+0,RGN_FRAC(0,3)+8,RGN_FRAC(1,3)+0,RGN_FRAC(1,3)+8,RGN_FRAC(2,3)+0,RGN_FRAC(2,3)+8 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};


static GFXDECODE_START( feversoc )
	GFXDECODE_ENTRY( "gfx1", 0, spi_spritelayout,   0, 0x40 )
GFXDECODE_END

static INPUT_PORTS_START( feversoc )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) //hopper i/o
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // maybe eeprom in / vblank
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("Slottle") PORT_CODE(KEYCODE_Z)
	PORT_DIPNAME( 0x0100, 0x0100, "DIP 1-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIP 1-2"  )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIP 1-3"  )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIP 1-4"  )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIP 1-5"  )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIP 1-6"  )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIP 1-7"  )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIP 1-8" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) // ?
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) // ?
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_N) // ?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(feversoc_state::feversoc_irq)
{
	m_maincpu->set_input_line(8, HOLD_LINE );
}

static MACHINE_CONFIG_START( feversoc, feversoc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",SH2,MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(feversoc_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", feversoc_state, feversoc_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1) //dynamic resolution?
	MCFG_SCREEN_UPDATE_DRIVER(feversoc_state, screen_update_feversoc)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", feversoc)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", MASTER_CLOCK/16, OKIM6295_PIN7_LOW) //pin 7 & frequency not verified (clock should be 28,6363 / n)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.6)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( feversoc )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prog0.u0139",   0x00001, 0x20000, CRC(fa699503) SHA1(96a834d4f7d5b764aa51db745afc2cd9a7c9783d) )
	ROM_LOAD16_BYTE( "prog1.u0140",   0x00000, 0x20000, CRC(fd4d7943) SHA1(d7d782f878656bc79d70589f9df2cbcfff0adb5e) )

	ROM_REGION( 0x600000, "gfx1", 0)    /* text */
	ROM_LOAD("obj1.u011", 0x000000, 0x200000, CRC(d8c8dde7) SHA1(3ef815fb1e21a0bd907ee835bc7a32d80f6a9d28) )
	ROM_LOAD("obj2.u012", 0x200000, 0x200000, CRC(8e93bfda) SHA1(3b4740cefb164efc320fb69f58e8800d2646fea6) )
	ROM_LOAD("obj3.u013", 0x400000, 0x200000, CRC(8c8c6e8b) SHA1(bed4990d6eebb7aefa200ad2bed9b7e71e6bd064) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pcm.u0743", 0x00000, 0x80000, CRC(20b0c0e3) SHA1(dcf2f620a8fe695688057dbaf5c431a32a832440) )
ROM_END

DRIVER_INIT_MEMBER(feversoc_state,feversoc)
{
	UINT32 *rom = (UINT32 *)memregion("maincpu")->base();

	seibuspi_rise11_sprite_decrypt_feversoc(memregion("gfx1")->base(), 0x200000);

	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0003ffff, 1, rom);
	m_maincpu->sh2drc_add_fastram(0x02000000, 0x0203dfff, 0, &m_mainram[0]);
	m_maincpu->sh2drc_add_fastram(0x0203e000, 0x0203ffff, 0, &m_spriteram[0]);
}

GAME( 2004, feversoc,  0,       feversoc,  feversoc, feversoc_state,  feversoc, ROT0, "Seibu Kaihatsu", "Fever Soccer", 0 )
