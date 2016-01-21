// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Tomasz Slanina
/************************************************************************************

 Super Crowns Golf (c) 1989 Nasco Japan

 driver by Tomasz Slanina & Angelo Salese

 TODO:
 - remove the patch and understand what needs to be modified for the gfxs, game
   doesn't crash anymore (note: I suspect it's actually a ppi port C bug);
 - Some weird framebuffer vertical gaps with some object, namely the green and the
   trees (zooming?)
 - not sure if the analog inputs are handled correctly;
 - Fix the framebuffer display in cocktail mode;
 - Albatross: bad graphics, caused by missing rom(s).

 Notes:
 - The game uses special control panel with 1 golf club shaped device to select shot
   strength (0,1,2,3), and 6 buttons (direction L&R, select angle of club head, club
   select, shot, and power to use items). -YO

************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "machine/i8255.h"

class suprgolf_state : public driver_device
{
public:
	suprgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;

	tilemap_t *m_tilemap;
	std::unique_ptr<UINT8[]> m_paletteram;
	std::unique_ptr<UINT8[]> m_bg_vram;
	std::unique_ptr<UINT16[]> m_bg_fb;
	std::unique_ptr<UINT16[]> m_fg_fb;
	UINT8 m_rom_bank;
	UINT8 m_bg_bank;
	UINT8 m_vreg_bank;
	UINT8 m_msm5205next;
	UINT8 m_msm_nmi_mask;
	UINT8 m_vreg_pen;
	UINT8 m_palette_switch;
	UINT8 m_bg_vreg_test;
	UINT8 m_toggle;

	DECLARE_READ8_MEMBER(videoram_r);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_READ8_MEMBER(bg_vram_r);
	DECLARE_WRITE8_MEMBER(bg_vram_w);
	DECLARE_WRITE8_MEMBER(pen_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(rom2_bank_select_w);
	DECLARE_READ8_MEMBER(vregs_r);
	DECLARE_WRITE8_MEMBER(vregs_w);
	DECLARE_READ8_MEMBER(rom_bank_select_r);
	DECLARE_WRITE8_MEMBER(rom_bank_select_w);
	DECLARE_READ8_MEMBER(pedal_extra_bits_r);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_WRITE8_MEMBER(writeA);
	DECLARE_WRITE8_MEMBER(writeB);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	TILE_GET_INFO_MEMBER(get_tile_info);

	DECLARE_DRIVER_INIT(suprgolf);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

TILE_GET_INFO_MEMBER(suprgolf_state::get_tile_info)
{
	int code = m_videoram[tile_index*2]+256*(m_videoram[tile_index*2+1]);
	int color = m_videoram[tile_index*2+0x800] & 0x7f;

	SET_TILE_INFO_MEMBER(0,
		code,
		color,
		0);
}

void suprgolf_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(suprgolf_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32 );
	m_paletteram = std::make_unique<UINT8[]>(0x1000);
	m_bg_vram = std::make_unique<UINT8[]>(0x2000*0x20);
	m_bg_fb = std::make_unique<UINT16[]>(0x2000*0x20);
	m_fg_fb = std::make_unique<UINT16[]>(0x2000*0x20);

	m_tilemap->set_transparent_pen(15);

	save_item(NAME(m_bg_bank));
	save_item(NAME(m_vreg_bank));
	save_item(NAME(m_vreg_pen));
	save_item(NAME(m_palette_switch));
	save_item(NAME(m_bg_vreg_test));
	save_pointer(NAME(m_paletteram.get()), 0x1000);
	save_pointer(NAME(m_bg_vram.get()), 0x2000*0x20);
	save_pointer(NAME(m_bg_fb.get()), 0x2000*0x20);
	save_pointer(NAME(m_fg_fb.get()), 0x2000*0x20);
}

UINT32 suprgolf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count,color;
	bitmap.fill(m_palette->black_pen(), cliprect);

	{
		count = 0;

		for(y=0;y<256;y++)
		{
			for(x=0;x<512;x++)
			{
				color = m_bg_fb[count];

				if(x <= cliprect.max_x && y <= cliprect.max_y)
					bitmap.pix16(y, x) = m_palette->pen((color & 0x7ff));

				count++;
			}
		}
	}

	{
		count = 0;

		for(y=0;y<256;y++)
		{
			for(x=0;x<512;x++)
			{
				color = m_fg_fb[count];

				if(((m_fg_fb[count] & 0x0f) != 0x0f) && (x <= cliprect.max_x && y <= cliprect.max_y))
					bitmap.pix16(y, x) = m_palette->pen((color & 0x7ff));

				count++;
			}
		}
	}

	m_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}

READ8_MEMBER(suprgolf_state::videoram_r)
{
	if (m_palette_switch)
		return m_paletteram[offset];
	else
		return m_videoram[offset];
}

WRITE8_MEMBER(suprgolf_state::videoram_w)
{
	if(m_palette_switch)
	{
		int r,g,b,datax;
		m_paletteram[offset] = data;
		offset>>=1;
		datax = m_paletteram[offset*2] + 256*m_paletteram[offset*2 + 1];

		b = (datax & 0x8000) ? 0 : ((datax)&0x001f)>>0;
		g = (datax & 0x8000) ? 0 : ((datax)&0x03e0)>>5;
		r = (datax & 0x8000) ? 0 : ((datax)&0x7c00)>>10;

		m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
	}
	else
	{
		m_videoram[offset] = data;
		m_tilemap->mark_tile_dirty((offset & 0x7fe) >> 1);
	}
}

READ8_MEMBER(suprgolf_state::vregs_r)
{
	return m_vreg_bank;
}

WRITE8_MEMBER(suprgolf_state::vregs_w)
{
	//printf("%02x\n",data);

	//bits 0,1,2 and probably 3 controls the background vram banking
	m_vreg_bank = data;
	m_palette_switch = (data & 0x80);
	m_bg_bank = (data & 0x1f);

	m_bg_vreg_test = data & 0x20;

	//if(data & 0x60)
	//  printf("Video regs with data %02x activated\n",data);
}

READ8_MEMBER(suprgolf_state::bg_vram_r)
{
	return m_bg_vram[offset+m_bg_bank*0x2000];
}

WRITE8_MEMBER(suprgolf_state::bg_vram_w)
{
	UINT8 hi_nibble,lo_nibble;
	UINT8 hi_dirty_dot,lo_dirty_dot; // helpers

	hi_nibble = data & 0xf0;
	lo_nibble = data & 0x0f;
	hi_dirty_dot = 1;
	lo_dirty_dot = 1;

	if(hi_nibble == 0xf0)
	{
		hi_nibble = m_bg_vram[offset+m_bg_bank*0x2000] & 0xf0;
		if(!(m_vreg_pen & 0x80) && (!(m_bg_bank & 0x10)))
			hi_dirty_dot = 0;
	}

	if(lo_nibble == 0x0f)
	{
		lo_nibble = m_bg_vram[offset+m_bg_bank*0x2000] & 0x0f;
		if(!(m_vreg_pen & 0x80) && (!(m_bg_bank & 0x10)))
			lo_dirty_dot = 0;
	}

	if(m_vreg_pen & 0x80 || m_bg_bank & 0x10)
		m_bg_vram[offset+m_bg_bank*0x2000] = data;
	else
		m_bg_vram[offset+m_bg_bank*0x2000] = hi_nibble|lo_nibble;

	if(m_bg_bank & 0x10)
	{
		if(hi_dirty_dot)
			m_fg_fb[(offset+(m_bg_bank & 0x0f)*0x2000)*2+1] = (m_vreg_pen & 0x7f)<<4 | ((m_bg_vram[offset+m_bg_bank*0x2000] & 0xf0)>>4);
		if(lo_dirty_dot)
			m_fg_fb[(offset+(m_bg_bank & 0x0f)*0x2000)*2+0] = (m_vreg_pen & 0x7f)<<4 | ((m_bg_vram[offset+m_bg_bank*0x2000] & 0x0f)>>0);
	}
	else
	{
		if(hi_dirty_dot)
			m_bg_fb[(offset+(m_bg_bank & 0x0f)*0x2000)*2+1] = (m_vreg_pen & 0x7f)<<4 | ((m_bg_vram[offset+m_bg_bank*0x2000] & 0xf0)>>4);
		if(lo_dirty_dot)
			m_bg_fb[(offset+(m_bg_bank & 0x0f)*0x2000)*2+0] = (m_vreg_pen & 0x7f)<<4 | ((m_bg_vram[offset+m_bg_bank*0x2000] & 0x0f)>>0);
	}
}

void suprgolf_state::machine_start()
{
	membank("bank1")->configure_entries(0, 16, memregion("user2")->base(), 0x4000);
	membank("bank2")->configure_entries(0, 64, memregion("user1")->base(), 0x4000);

	save_item(NAME(m_rom_bank));
	save_item(NAME(m_msm5205next));
	save_item(NAME(m_msm_nmi_mask));
	save_item(NAME(m_toggle));
}

WRITE8_MEMBER(suprgolf_state::pen_w)
{
	m_vreg_pen = data;
}

WRITE8_MEMBER(suprgolf_state::adpcm_data_w)
{
	m_msm5205next = data;
}

READ8_MEMBER(suprgolf_state::rom_bank_select_r)
{
	return m_rom_bank;
}

WRITE8_MEMBER(suprgolf_state::rom_bank_select_w)
{
	m_rom_bank = data;

	//popmessage("%08x %02x",((data & 0x3f) * 0x4000),data);
	//osd_printf_debug("ROM_BANK 0x8000 - %X @%X\n",data,space.device().safe_pcbase());
	membank("bank2")->set_entry(data & 0x3f);

	m_msm_nmi_mask = data & 0x40;
	flip_screen_set(data & 0x80);
}

WRITE8_MEMBER(suprgolf_state::rom2_bank_select_w)
{
	//osd_printf_debug("ROM_BANK 0x4000 - %X @%X\n",data,space.device().safe_pcbase());
	membank("bank1")->set_entry(data & 0x0f);

	if(data & 0xf0)
		printf("Rom bank select 2 with data %02x activated\n",data);
}

READ8_MEMBER(suprgolf_state::pedal_extra_bits_r)
{
	UINT8 p1_sht_sw,p2_sht_sw;

	p1_sht_sw = (ioport("P1_RELEASE")->read() & 0x80)>>7;
	p2_sht_sw = (ioport("P2_RELEASE")->read() & 0x80)>>6;

	return p1_sht_sw | p2_sht_sw;
}

READ8_MEMBER(suprgolf_state::p1_r)
{
	return (ioport("P1")->read() & 0xf0) | ((ioport("P1_ANALOG")->read() & 0xf));
}

READ8_MEMBER(suprgolf_state::p2_r)
{
	return (ioport("P2")->read() & 0xf0) | ((ioport("P2_ANALOG")->read() & 0xf));
}

static ADDRESS_MAP_START( suprgolf_map, AS_PROGRAM, 8, suprgolf_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x4000) AM_WRITE(rom2_bank_select_w )
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(bg_vram_r, bg_vram_w ) // banked background vram
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(videoram_r, videoram_w ) AM_SHARE("videoram") //foreground vram + paletteram
	AM_RANGE(0xf000, 0xf000) AM_WRITE(pen_w )
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, suprgolf_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x08, 0x09) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(adpcm_data_w)
	ADDRESS_MAP_END

static INPUT_PORTS_START( suprgolf )
	PORT_START("P1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) /* low port of P1 Pedal */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)      /* D.L */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)       /* D.R */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)         /* CNT - shot switch */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)         /* SEL */

	PORT_START("P1_ANALOG")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_PLAYER(1)

	/* simulate spring throttle with the following button */
	PORT_START("P1_RELEASE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) /* release power? */

	PORT_START("P2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) /* low port of P2 Pedal */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)      /* D.L */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)       /* D.R */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)         /* CNT - shot switch */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)         /* SEL */

	PORT_START("P2_ANALOG")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_PLAYER(2)

	/* simulate spring throttle with the following button */
	PORT_START("P2_RELEASE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) /* release power? */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )                         /* 1P */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)         /* POW */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )                         /* 1P */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)         /* POW */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "TST" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	/* According to the manual, 4 and 5 are for Indoor Practice tries, but doesn't suit well...different version? */
	PORT_DIPNAME( 0x08, 0x08, "Indoor Practice" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Number of Balls" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	/* According to the manual, Allow Continue should be dip-sw 2:5 */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPNAME( 0x06, 0x00, "Percentage of wind over 10m/s" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, "30%" )
	PORT_DIPSETTING(    0x04, "40%" )
	PORT_DIPSETTING(    0x02, "50%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:8" )
INPUT_PORTS_END

WRITE8_MEMBER(suprgolf_state::writeA)
{
	osd_printf_debug("ymwA\n");
}

WRITE8_MEMBER(suprgolf_state::writeB)
{
	osd_printf_debug("ymwA\n");
}

WRITE_LINE_MEMBER(suprgolf_state::adpcm_int)
{
	m_msm->reset_w(0);
	m_toggle ^= 1;
	if(m_toggle)
	{
		m_msm->data_w((m_msm5205next & 0xf0) >> 4);
		if(m_msm_nmi_mask) { m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE); }
	}
	else
	{
		m_msm->data_w((m_msm5205next & 0x0f) >> 0);
	}
}

static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

static GFXDECODE_START( suprgolf )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout,   0, 0x80 )
GFXDECODE_END

void suprgolf_state::machine_reset()
{
	m_msm_nmi_mask = 0;
}

#define MASTER_CLOCK XTAL_12MHz

static MACHINE_CONFIG_START( suprgolf, suprgolf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MASTER_CLOCK/2) /* guess */
	MCFG_CPU_PROGRAM_MAP(suprgolf_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", suprgolf_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(suprgolf_state, p1_r))
	MCFG_I8255_IN_PORTB_CB(READ8(suprgolf_state, p2_r))
	MCFG_I8255_IN_PORTC_CB(READ8(suprgolf_state, pedal_extra_bits_r))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("SYSTEM"))
	MCFG_I8255_IN_PORTB_CB(READ8(suprgolf_state, rom_bank_select_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(suprgolf_state, rom_bank_select_w))
	MCFG_I8255_IN_PORTC_CB(READ8(suprgolf_state, vregs_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(suprgolf_state, vregs_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 191)
	MCFG_SCREEN_UPDATE_DRIVER(suprgolf_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suprgolf)
	MCFG_PALETTE_ADD("palette", 0x800)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK/4) /* guess */
	//MCFG_YM2203_IRQ_HANDLER(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW0"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(suprgolf_state, writeA))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(suprgolf_state, writeB))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz) /* guess */
	MCFG_MSM5205_VCLK_CB(WRITELINE(suprgolf_state, adpcm_int))      /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)  /* 4KHz 4-bit */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/*
----------------------
CG24     6K        CONN BD
CG1      6J         "
CG2      6G         "
CG3      6F         "
CG4      6D         "
CG5      6C         "
CG6      6A         "
CG7      5J         "
CG8      5G         "
CG9      5F         "
CG10     5D         "
CG11     5A         "
CG12     6K         "
CG13     6J         "
CG14     5K         "
CG15     5J         "
CG16     5G         "
CG17     5F         "

CG18     3K        DAUGHTER BOARD
CG20     7K         "
CG21     7J         "
CG22     7G         "
CG23     7F         "
*/

ROM_START( suprgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cg24.6k",0x000000, 0x08000, CRC(de548044) SHA1(f96b4cfcfca4dffabfaf205eb903cbc70972626b) )

	ROM_REGION( 0x100000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "cg1.6j", 0x000000, 0x10000, CRC(ee545c71) SHA1(8ee459a85e52257d3f9a2aa7263b641aad87bafd) )
	ROM_LOAD( "cg2.6g", 0x010000, 0x10000, CRC(a2ed2159) SHA1(5e13b6c4eaba8146a4c6c2ff24197f3ffca29b92) )
	ROM_LOAD( "cg3.6f", 0x020000, 0x10000, CRC(4543334d) SHA1(7ee268ed6d02c78db8c222418313593df37cde4b) )
	ROM_LOAD( "cg4.6d", 0x030000, 0x10000, CRC(85ace664) SHA1(5267406c98e2d124a4985816f8e2e32e74e09614) )
	ROM_LOAD( "cg5.6c", 0x040000, 0x10000, CRC(609d5b37) SHA1(60640a9bd0883bf4dc999077d89ef793e827ac23) )
	ROM_LOAD( "cg6.6a", 0x050000, 0x10000, CRC(5e4a8ddb) SHA1(0c71c7eba9fe79187c4214eb639a481305070dcc) )
	ROM_LOAD( "cg7.5j", 0x060000, 0x10000, CRC(90ac6734) SHA1(2656397fca6dceabf8e35c093c0ba25e08d2ad1e) )
	ROM_LOAD( "cg8.5g", 0x070000, 0x10000, CRC(2e9edece) SHA1(a0961bb23f312ed137134746d2d3d438fe098085) )
	ROM_LOAD( "cg9.5f", 0x080000, 0x10000, CRC(139d71f1) SHA1(756ed068e1e2b76a9d1df95b432976e632edfa77) )
	ROM_LOAD( "cg10.5d",0x090000, 0x10000, CRC(c069e75e) SHA1(77f1b7571e677aef601b8b1c481b352ca6e485d6) )
	/* no 5c? */
	ROM_LOAD( "cg11.5a",0x0b0000, 0x10000, CRC(cfec1a0f) SHA1(c09ece059cb3c456b66c016c6fab3139d3f61c6a) )

	ROM_REGION( 0x100000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD( "cg20.7k",0x000000, 0x10000, CRC(1e3fa2fd) SHA1(4771b90e40ebfbae4a98ff7ce6db50f635232597) )
	ROM_LOAD( "cg21.7j",0x010000, 0x10000, CRC(0323a2cd) SHA1(d7d4b35ad451acb2fa3d117bb0ae2f8fbd883f17) )
	ROM_LOAD( "cg22.7g",0x020000, 0x10000, CRC(83bcbefd) SHA1(77f29cfd1583d2506e95b8513cb9f87569c31821) )
	ROM_LOAD( "cg23.7f",0x030000, 0x10000, CRC(50191b4d) SHA1(8f74cba2a2b5fd2a03eaf13a6d6b39af8833a4ab) )

	ROM_REGION( 0x70000, "gfx1", 0 )
	ROM_LOAD( "cg12.6k",0x00000, 0x10000, CRC(5707b3d5) SHA1(9102a40fefb6426f2cd9d92d66fdc77e078e3f4c) )
	ROM_LOAD( "cg13.6j",0x10000, 0x10000, CRC(02ff0187) SHA1(aeeb3b2d15c3c8ff4695ecf6cfc0c385295ecce6) )
	ROM_LOAD( "cg14.5k",0x20000, 0x10000, CRC(ca12e01d) SHA1(9c627fb527c8966e16dc6bdb99ec0b9728b5c5f9) )
	ROM_LOAD( "cg15.5j",0x30000, 0x10000, CRC(0fb88270) SHA1(d85a7f1bc5b3c4b13bbd887cea4c055541cbb737) )
	ROM_LOAD( "cg16.5g",0x40000, 0x10000, CRC(0498aa2e) SHA1(988965c3a584dac17ad8c7e504fa1f1e49775611) )
	ROM_LOAD( "cg17.5f",0x50000, 0x10000, CRC(d27f87b5) SHA1(5b2927e89615589540e3853593aeff517584b6a0) )
	ROM_LOAD( "cg18.3k",0x60000, 0x10000, CRC(36edd88e) SHA1(374c95721198a88831d6f7e0b71d05e2f8465271) )
ROM_END

ROM_START( albatross )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.6k",         0x000000, 0x008000, CRC(6f934951) SHA1(b7217a4e509e452f15f414ce7e23c724ecac6184) )

	ROM_REGION( 0x100000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "at1.6h",       0x000000, 0x010000, CRC(ee545c71) SHA1(8ee459a85e52257d3f9a2aa7263b641aad87bafd) )
	ROM_LOAD( "at2.6g",       0x010000, 0x010000, CRC(a2ed2159) SHA1(5e13b6c4eaba8146a4c6c2ff24197f3ffca29b92) )
	ROM_LOAD( "at3.6e",       0x020000, 0x010000, CRC(4543334d) SHA1(7ee268ed6d02c78db8c222418313593df37cde4b) )
	ROM_LOAD( "at4.6d",       0x030000, 0x010000, CRC(85ace664) SHA1(5267406c98e2d124a4985816f8e2e32e74e09614) )
	ROM_LOAD( "at5.6c",       0x040000, 0x010000, CRC(609d5b37) SHA1(60640a9bd0883bf4dc999077d89ef793e827ac23) )
	ROM_LOAD( "at6.6a",       0x050000, 0x010000, CRC(5e4a8ddb) SHA1(0c71c7eba9fe79187c4214eb639a481305070dcc) )
	ROM_LOAD( "at7.4h",       0x060000, 0x010000, CRC(90ac6734) SHA1(2656397fca6dceabf8e35c093c0ba25e08d2ad1e) )
	ROM_LOAD( "at8.4g",       0x070000, 0x010000, CRC(2e9edece) SHA1(a0961bb23f312ed137134746d2d3d438fe098085) )
	ROM_LOAD( "kage.4e",      0x080000, 0x010000, CRC(139d71f1) SHA1(756ed068e1e2b76a9d1df95b432976e632edfa77) )
	ROM_LOAD( "at10.4d",      0x090000, 0x010000, CRC(c4d5617c) SHA1(5f2d66f827d8d7437fde84ffa17db105a5352f06) )
	/* 4c is connected below */
	ROM_LOAD( "map.4a",       0x0b0000, 0x010000, CRC(cfec1a0f) SHA1(c09ece059cb3c456b66c016c6fab3139d3f61c6a) )

	ROM_REGION( 0x100000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD( "cg20.7k",0x000000, 0x10000, BAD_DUMP CRC(1e3fa2fd) SHA1(4771b90e40ebfbae4a98ff7ce6db50f635232597) ) // - empty sockets on PCB :/ (temps from Super Crowns Golf)
	ROM_LOAD( "cg21.7j",0x010000, 0x10000, BAD_DUMP CRC(0323a2cd) SHA1(d7d4b35ad451acb2fa3d117bb0ae2f8fbd883f17) ) // /
	ROM_LOAD( "2.4c",   0x020000, 0x20000, CRC(08d4363b) SHA1(60c5543c35f44af2f4a8f7ca4bc10633f5fa67fb) )

	ROM_REGION( 0x70000, "gfx1", 0 )
	ROM_LOAD( "chr1.3h",      0x000000, 0x020000, CRC(e62d2bb4) SHA1(f931699114a99b7eb25f8bb841d85de0d6a106a5) )
	ROM_LOAD( "chr2.3g",      0x020000, 0x020000, CRC(808c15e6) SHA1(d7d1ac7456f492dfcc1c1b501f8dde86e405fd7b) )
	ROM_LOAD( "chr3.3e",      0x040000, 0x020000, CRC(9a60193d) SHA1(d22c958b5bd82626fcfc94f7ad16d8cd4bacdda2) )
	ROM_LOAD( "chr4.3d",      0x060000, 0x010000, CRC(0fb88270) SHA1(d85a7f1bc5b3c4b13bbd887cea4c055541cbb737) )
ROM_END




DRIVER_INIT_MEMBER(suprgolf_state,suprgolf)
{
	UINT8 *ROM = memregion("user2")->base();

	ROM[0x74f4-0x4000] = 0x00;
	ROM[0x74f5-0x4000] = 0x00;
	ROM[0x6d72+(0x4000*3)-0x4000] = 0x20; //patch ROM check
}

GAME( 1989, suprgolf,  0,         suprgolf,  suprgolf, suprgolf_state,  suprgolf, ROT0, "Nasco", "Super Crowns Golf (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, albatross, suprgolf,  suprgolf,  suprgolf, driver_device,  0,        ROT0, "Nasco", "Albatross (US Prototype?)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL| MACHINE_SUPPORTS_SAVE )
