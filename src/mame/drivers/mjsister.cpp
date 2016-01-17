// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

    Mahjong Sisters (c) 1986 Toa Plan

    Driver by Uki

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/ay8910.h"

#define MCLK 12000000


class mjsister_state : public driver_device
{
public:
	enum
	{
		TIMER_DAC
	};

	mjsister_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac") { }

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap0;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap1;
	int  m_flip_screen;
	int  m_video_enable;
	int  m_screen_redraw;
	int  m_vrambank;
	int  m_colorbank;

	/* misc */
	int  m_input_sel1;
	int  m_input_sel2;

	int  m_rombank0;
	int  m_rombank1;

	UINT32 m_dac_adr;
	UINT32 m_dac_bank;
	UINT32 m_dac_adr_s;
	UINT32 m_dac_adr_e;
	UINT32 m_dac_busy;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<dac_device> m_dac;

	/* memory */
	UINT8 m_videoram0[0x8000];
	UINT8 m_videoram1[0x8000];
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(dac_adr_s_w);
	DECLARE_WRITE8_MEMBER(dac_adr_e_w);
	DECLARE_WRITE8_MEMBER(banksel1_w);
	DECLARE_WRITE8_MEMBER(banksel2_w);
	DECLARE_WRITE8_MEMBER(input_sel1_w);
	DECLARE_WRITE8_MEMBER(input_sel2_w);
	DECLARE_READ8_MEMBER(keys_r);
	TIMER_CALLBACK_MEMBER(dac_callback);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void redraw();
	void plot0( int offset, UINT8 data );
	void plot1( int offset, UINT8 data );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void mjsister_state::video_start()
{
	m_tmpbitmap0 = std::make_unique<bitmap_ind16>(256, 256);
	m_tmpbitmap1 = std::make_unique<bitmap_ind16>(256, 256);

	save_item(NAME(m_videoram0));
	save_item(NAME(m_videoram1));
}

void mjsister_state::plot0( int offset, UINT8 data )
{
	int x, y, c1, c2;

	x = offset & 0x7f;
	y = offset / 0x80;

	c1 = (data & 0x0f)        + m_colorbank * 0x20;
	c2 = ((data & 0xf0) >> 4) + m_colorbank * 0x20;

	m_tmpbitmap0->pix16(y, x * 2 + 0) = c1;
	m_tmpbitmap0->pix16(y, x * 2 + 1) = c2;
}

void mjsister_state::plot1( int offset, UINT8 data )
{
	int x, y, c1, c2;

	x = offset & 0x7f;
	y = offset / 0x80;

	c1 = data & 0x0f;
	c2 = (data & 0xf0) >> 4;

	if (c1)
		c1 += m_colorbank * 0x20 + 0x10;
	if (c2)
		c2 += m_colorbank * 0x20 + 0x10;

	m_tmpbitmap1->pix16(y, x * 2 + 0) = c1;
	m_tmpbitmap1->pix16(y, x * 2 + 1) = c2;
}

WRITE8_MEMBER(mjsister_state::videoram_w)
{
	if (m_vrambank)
	{
		m_videoram1[offset] = data;
		plot1(offset, data);
	}
	else
	{
		m_videoram0[offset] = data;
		plot0(offset, data);
	}
}

UINT32 mjsister_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = m_flip_screen;
	int i, j;

	if (m_screen_redraw)
	{
		int offs;

		for (offs = 0; offs < 0x8000; offs++)
		{
			plot0(offs, m_videoram0[offs]);
			plot1(offs, m_videoram1[offs]);
		}
		m_screen_redraw = 0;
	}

	if (m_video_enable)
	{
		for (i = 0; i < 256; i++)
			for (j = 0; j < 4; j++)
				bitmap.pix16(i, 256 + j) = m_colorbank * 0x20;

		copybitmap(bitmap, *m_tmpbitmap0, flip, flip, 0, 0, cliprect);
		copybitmap_trans(bitmap, *m_tmpbitmap1, flip, flip, 2, 0, cliprect, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void mjsister_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_DAC:
		dac_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in mjsister_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(mjsister_state::dac_callback)
{
	UINT8 *DACROM = memregion("samples")->base();

	m_dac->write_unsigned8(DACROM[(m_dac_bank * 0x10000 + m_dac_adr++) & 0x1ffff]);

	if (((m_dac_adr & 0xff00 ) >> 8) !=  m_dac_adr_e)
		timer_set(attotime::from_hz(MCLK) * 1024, TIMER_DAC);
	else
		m_dac_busy = 0;
}

WRITE8_MEMBER(mjsister_state::dac_adr_s_w)
{
	m_dac_adr_s = data;
}

WRITE8_MEMBER(mjsister_state::dac_adr_e_w)
{
	m_dac_adr_e = data;
	m_dac_adr = m_dac_adr_s << 8;

	if (m_dac_busy == 0)
		synchronize(TIMER_DAC);

	m_dac_busy = 1;
}

WRITE8_MEMBER(mjsister_state::banksel1_w)
{
	int tmp = m_colorbank;

	switch (data)
	{
		case 0x0: m_rombank0 = 0 ; break;
		case 0x1: m_rombank0 = 1 ; break;

		case 0x2: m_flip_screen = 0 ; break;
		case 0x3: m_flip_screen = 1 ; break;

		case 0x4: m_colorbank &= 0xfe; break;
		case 0x5: m_colorbank |= 0x01; break;
		case 0x6: m_colorbank &= 0xfd; break;
		case 0x7: m_colorbank |= 0x02; break;
		case 0x8: m_colorbank &= 0xfb; break;
		case 0x9: m_colorbank |= 0x04; break;

		case 0xa: m_video_enable = 0 ; break;
		case 0xb: m_video_enable = 1 ; break;

		case 0xe: m_vrambank = 0 ; break;
		case 0xf: m_vrambank = 1 ; break;

		default:
			logerror("%04x p30_w:%02x\n", space.device().safe_pc(), data);
	}

	if (tmp != m_colorbank)
		m_screen_redraw = 1;

	membank("bank1")->set_entry(m_rombank0 * 2 + m_rombank1);
}

WRITE8_MEMBER(mjsister_state::banksel2_w)
{
	switch (data)
	{
		case 0xa: m_dac_bank = 0; break;
		case 0xb: m_dac_bank = 1; break;

		case 0xc: m_rombank1 = 0; break;
		case 0xd: m_rombank1 = 1; break;

		default:
			logerror("%04x p31_w:%02x\n", space.device().safe_pc(), data);
	}

	membank("bank1")->set_entry(m_rombank0 * 2 + m_rombank1);
}

WRITE8_MEMBER(mjsister_state::input_sel1_w)
{
	m_input_sel1 = data;
}

WRITE8_MEMBER(mjsister_state::input_sel2_w)
{
	m_input_sel2 = data;
}

READ8_MEMBER(mjsister_state::keys_r)
{
	int p, i, ret = 0;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	p = m_input_sel1 & 0x3f;
	//  p |= ((m_input_sel2 & 8) << 4) | ((m_input_sel2 & 0x20) << 1);

	for (i = 0; i < 6; i++)
	{
		if (BIT(p, i))
			ret |= ioport(keynames[i])->read();
	}

	return ret;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( mjsister_map, AS_PROGRAM, 8, mjsister_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1") AM_WRITE(videoram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjsister_io_map, AS_IO, 8, mjsister_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITENOP /* HD46505? */
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0x11, 0x11) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x12, 0x12) AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x20, 0x20) AM_READ(keys_r)
	AM_RANGE(0x21, 0x21) AM_READ_PORT("IN0")
	AM_RANGE(0x30, 0x30) AM_WRITE(banksel1_w)
	AM_RANGE(0x31, 0x31) AM_WRITE(banksel2_w)
	AM_RANGE(0x32, 0x32) AM_WRITE(input_sel1_w)
	AM_RANGE(0x33, 0x33) AM_WRITE(input_sel2_w)
	AM_RANGE(0x34, 0x34) AM_WRITE(dac_adr_s_w)
	AM_RANGE(0x35, 0x35) AM_WRITE(dac_adr_e_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mjsister )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* service mode */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* memory reset 1 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* analyzer */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* memory reset 2 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* pay out */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* hopper */

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_H )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_SCORE )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_RON )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_BIG )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_BET )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mjsister_state::redraw()
{
	/* we can skip saving tmpbitmaps because we can redraw them from vram */
	m_screen_redraw = 1;
}

void mjsister_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x8000);

	save_item(NAME(m_dac_busy));
	save_item(NAME(m_flip_screen));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_vrambank));
	save_item(NAME(m_colorbank));
	save_item(NAME(m_input_sel1));
	save_item(NAME(m_input_sel2));
	save_item(NAME(m_rombank0));
	save_item(NAME(m_rombank1));
	save_item(NAME(m_dac_adr));
	save_item(NAME(m_dac_bank));
	save_item(NAME(m_dac_adr_s));
	save_item(NAME(m_dac_adr_e));
	machine().save().register_postload(save_prepost_delegate(FUNC(mjsister_state::redraw), this));
}

void mjsister_state::machine_reset()
{
	m_dac_busy = 0;
	m_flip_screen = 0;
	m_video_enable = 0;
	m_screen_redraw = 0;
	m_vrambank = 0;
	m_colorbank = 0;
	m_input_sel1 = 0;
	m_input_sel2 = 0;
	m_rombank0 = 0;
	m_rombank1 = 0;
	m_dac_adr = 0;
	m_dac_bank = 0;
	m_dac_adr_s = 0;
	m_dac_adr_e = 0;
}


static MACHINE_CONFIG_START( mjsister, mjsister_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MCLK/2) /* 6.000 MHz */
	MCFG_CPU_PROGRAM_MAP(mjsister_map)
	MCFG_CPU_IO_MAP(mjsister_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(mjsister_state, irq0_line_hold, 2*60)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256+4, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255+4, 8, 247)
	MCFG_SCREEN_UPDATE_DRIVER(mjsister_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MCLK/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mjsister )
	ROM_REGION( 0x30000, "maincpu", 0 )   /* CPU */
	ROM_LOAD( "ms00.bin",  0x00000, 0x08000, CRC(9468c33b) SHA1(63aecdcaa8493d58549dfd1d217743210cf953bc) )
	ROM_LOAD( "ms01t.bin", 0x10000, 0x10000, CRC(a7b6e530) SHA1(fda9bea214968a8814d2c43226b3b32316581050) ) /* banked */
	ROM_LOAD( "ms02t.bin", 0x20000, 0x10000, CRC(7752b5ba) SHA1(84dcf27a62eb290ba07c85af155897ec72f320a8) ) /* banked */

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "ms03.bin", 0x00000,  0x10000, CRC(10a68e5e) SHA1(a0e2fa34c1c4f34642f65fbf17e9da9c2554a0c6) )
	ROM_LOAD( "ms04.bin", 0x10000,  0x10000, CRC(641b09c1) SHA1(15cde906175bcb5190d36cc91cbef003ef91e425) )

	ROM_REGION( 0x00400, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "ms05.bpr", 0x0000,  0x0100, CRC(dd231a5f) SHA1(be008593ac8ba8f5a1dd5b188dc7dc4c03016805) ) // R
	ROM_LOAD( "ms06.bpr", 0x0100,  0x0100, CRC(df8e8852) SHA1(842a891440aef55a560d24c96f249618b9f4b97f) ) // G
	ROM_LOAD( "ms07.bpr", 0x0200,  0x0100, CRC(6cb3a735) SHA1(468ae3d40552dc2ec24f5f2988850093d73948a6) ) // B
	ROM_LOAD( "ms08.bpr", 0x0300,  0x0100, CRC(da2b3b38) SHA1(4de99c17b227653bc1b904f1309f447f5a0ab516) ) // ?
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, mjsister, 0, mjsister, mjsister, driver_device, 0, ROT0, "Toaplan", "Mahjong Sisters (Japan)", MACHINE_SUPPORTS_SAVE )
