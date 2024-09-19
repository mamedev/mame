// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Tomasz Slanina
/**************************************************************************************************

 Super Crowns Golf (c) 1989 Nasco Japan

 driver by Tomasz Slanina & Angelo Salese

 TODO:
 - framebuffer handling is exotic and definitely not right, it is unknown how the HW really
   arranges for RMW drawing, and how the pens really mix/if they are readable:
 \- Polygons won't fill properly without a ROM patch (which actually sets up BG bank & 0x08,
    readback correction?). Note that wrong handling of the area will make the game to crash.
 \- Uneven vertical gaps with some object, namely the green and the trees (RMW extension? Wrong
    readback?)
 \- Map display arrow direction (transparency) and ball plot tracing;
 \- Mixing, map display may as well go above tilemap but that normally breaks club / ball
 \- Color of the ball (green) during attract mode how to play screen;
 \- Display in cocktail mode;
 - not sure if the analog inputs are handled correctly;
 - Confirm ADPCM hookup.
 - Albatross: controls.


 Notes:
 - The game uses special control panel with 1 golf club shaped device to select shot
   strength (0,1,2,3), and 6 buttons (direction L&R, select angle of club head, club
   select, shot, and power to use items). -YO

**************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class suprgolf_state : public driver_device
{
public:
	suprgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_msm(*this, "msm")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_paletteram(*this, "paletteram")
		, m_vram_palette_view(*this, "vram_palette_view")
	{ }

	void suprgolf(machine_config &config);

	void init_suprgolf();
	void init_suprgolfj();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_paletteram;

	memory_view m_vram_palette_view;

	tilemap_t *m_tilemap = nullptr;
	std::unique_ptr<uint8_t[]> m_bg_vram;
	std::unique_ptr<uint16_t[]> m_bg_fb;
	std::unique_ptr<uint16_t[]> m_fg_fb;
	uint8_t m_bg_bank = 0;
	uint8_t m_vreg_pen = 0;

	uint8_t m_adpcm_data = 0;
	bool m_adpcm_nmi_enable = 0;
	uint8_t m_adpcm_available_data = 0;

	void paletteram_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t bg_vram_r(offs_t offset);
	void bg_vram_w(offs_t offset, uint8_t data);
	void pen_w(uint8_t data);
	void adpcm_data_w(uint8_t data);
	void rom2_bank_select_w(uint8_t data);
	void vregs_w(uint8_t data);
	void rom_bank_select_w(uint8_t data);
	uint8_t pedal_extra_bits_r();
	uint8_t p1_r();
	uint8_t p2_r();
	void adpcm_int(int state);

	TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;
	void suprgolf_map(address_map &map) ATTR_COLD;
};

TILE_GET_INFO_MEMBER(suprgolf_state::get_tile_info)
{
	int code = m_videoram[tile_index*2]+256*(m_videoram[tile_index*2+1]);
	int color = m_videoram[tile_index*2+0x800] & 0x7f;

	tileinfo.set(0,
		code,
		color,
		0);
}

void suprgolf_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(suprgolf_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32 );
	m_bg_vram = std::make_unique<uint8_t[]>(0x2000*0x20);
	m_bg_fb = std::make_unique<uint16_t[]>(0x2000*0x20);
	m_fg_fb = std::make_unique<uint16_t[]>(0x2000*0x20);

	m_tilemap->set_transparent_pen(0xf);

	save_item(NAME(m_bg_bank));
	save_item(NAME(m_vreg_pen));
	save_pointer(NAME(m_bg_vram), 0x2000*0x20);
	save_pointer(NAME(m_bg_fb), 0x2000*0x20);
	save_pointer(NAME(m_fg_fb), 0x2000*0x20);
}

uint32_t suprgolf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	int count = 0;

	for(int y=0;y<256;y++)
	{
		for(int x=0;x<512;x++)
		{
			int const color = m_bg_fb[count];

			if(x <= cliprect.max_x && y <= cliprect.max_y)
				bitmap.pix(y, x) = m_palette->pen((color & 0x7ff));

			count++;
		}
	}

	count = 0;

	for(int y=0;y<256;y++)
	{
		for(int x=0;x<512;x++)
		{
			int const color = m_fg_fb[count];

			if(((m_fg_fb[count] & 0x0f) != 0x0f) && (x <= cliprect.max_x && y <= cliprect.max_y))
				bitmap.pix(y, x) = m_palette->pen((color & 0x7ff));

			count++;
		}
	}

	m_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}

void suprgolf_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty((offset & 0x7fe) >> 1);
}

void suprgolf_state::paletteram_w(offs_t offset, uint8_t data)
{
	u16 datax;
	u16 r, g, b;

	m_paletteram[offset] = data;
	offset >>= 1;
	datax = m_paletteram[offset * 2] + (m_paletteram[offset*2 + 1] << 8);

	b = (datax & 0x8000) ? 0 : ((datax)&0x001f)>>0;
	g = (datax & 0x8000) ? 0 : ((datax)&0x03e0)>>5;
	r = (datax & 0x8000) ? 0 : ((datax)&0x7c00)>>10;

	m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

void suprgolf_state::vregs_w(uint8_t data)
{
	//printf("%02x\n",data);

	m_vram_palette_view.select(BIT(data, 7));
	//bits 0,1,2 and probably 3 controls the background vram banking
	m_bg_bank = (data & 0x1f);

	//if(data & 0x60)
	//  printf("Video regs with data %02x activated\n",data);
}

uint8_t suprgolf_state::bg_vram_r(offs_t offset)
{
	return m_bg_vram[offset+m_bg_bank*0x2000];
}

void suprgolf_state::bg_vram_w(offs_t offset, uint8_t data)
{
	uint8_t hi_nibble,lo_nibble;
	uint8_t hi_dirty_dot,lo_dirty_dot; // helpers

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

void suprgolf_state::pen_w(uint8_t data)
{
	m_vreg_pen = data;
}

void suprgolf_state::adpcm_data_w(uint8_t data)
{
	m_adpcm_data = data;
	m_adpcm_available_data = 2;
}

void suprgolf_state::rom_bank_select_w(uint8_t data)
{
	membank("bank2")->set_entry(data & 0x3f);

	m_adpcm_nmi_enable = bool(BIT(data, 6));
	flip_screen_set(BIT(data, 7));
	// assuming a reset is triggered if NMI is off
	m_msm->reset_w(!m_adpcm_nmi_enable);
}

void suprgolf_state::rom2_bank_select_w(uint8_t data)
{
	membank("bank1")->set_entry(data & 0x0f);
}

uint8_t suprgolf_state::pedal_extra_bits_r()
{
	uint8_t p1_sht_sw,p2_sht_sw;

	p1_sht_sw = (ioport("P1_RELEASE")->read() & 0x80)>>7;
	p2_sht_sw = (ioport("P2_RELEASE")->read() & 0x80)>>6;

	return p1_sht_sw | p2_sht_sw;
}

uint8_t suprgolf_state::p1_r()
{
	return (ioport("P1")->read() & 0xf0) | ((ioport("P1_ANALOG")->read() & 0xf));
}

uint8_t suprgolf_state::p2_r()
{
	return (ioport("P2")->read() & 0xf0) | ((ioport("P2_ANALOG")->read() & 0xf));
}

void suprgolf_state::suprgolf_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("bank1");
	map(0x4000, 0x4000).w(FUNC(suprgolf_state::rom2_bank_select_w));
	map(0x8000, 0xbfff).bankr("bank2");
	map(0xc000, 0xdfff).rw(FUNC(suprgolf_state::bg_vram_r), FUNC(suprgolf_state::bg_vram_w)); // banked background vram
	map(0xe000, 0xefff).view(m_vram_palette_view);
	m_vram_palette_view[0](0xe000, 0xefff).ram().w(FUNC(suprgolf_state::videoram_w)).share("videoram");
	m_vram_palette_view[1](0xe000, 0xefff).ram().w(FUNC(suprgolf_state::paletteram_w)).share("paletteram");
	map(0xf000, 0xf000).w(FUNC(suprgolf_state::pen_w));
	map(0xf800, 0xffff).ram();
}

void suprgolf_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x09).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x0c, 0x0c).w(FUNC(suprgolf_state::adpcm_data_w));
}

static INPUT_PORTS_START( suprgolf )
	PORT_START("P1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_CUSTOM ) /* low port of P1 Pedal */
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
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_CUSTOM ) /* low port of P2 Pedal */
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

// guesswork, assume played back from VCLK control
// 0 -> 1 clocks NMI, 1 -> 0 pushes data to the ADPCM if available from the bus.
void suprgolf_state::adpcm_int(int state)
{
	if(!state && m_adpcm_available_data)
	{
		m_msm->data_w(m_adpcm_data >> 4);
		m_adpcm_data <<= 4;
		m_adpcm_available_data--;
		return;
	}

	if(state && m_adpcm_available_data == 0 && m_adpcm_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

static GFXDECODE_START( gfx_suprgolf )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb,   0, 0x80 )
GFXDECODE_END

void suprgolf_state::machine_start()
{
	membank("bank1")->configure_entries(0, 16, memregion("user2")->base(), 0x4000);
	membank("bank2")->configure_entries(0, 64, memregion("user1")->base(), 0x4000);

	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_adpcm_nmi_enable));
	save_item(NAME(m_adpcm_available_data));
}

void suprgolf_state::machine_reset()
{
	m_adpcm_nmi_enable = 0;
	m_adpcm_available_data = 0;
	m_vram_palette_view.select(0);
}

#define MASTER_CLOCK XTAL(12'000'000)

void suprgolf_state::suprgolf(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/2); /* guess */
	m_maincpu->set_addrmap(AS_PROGRAM, &suprgolf_state::suprgolf_map);
	m_maincpu->set_addrmap(AS_IO, &suprgolf_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(suprgolf_state::irq0_line_hold));

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set(FUNC(suprgolf_state::p1_r));
	ppi0.in_pb_callback().set(FUNC(suprgolf_state::p2_r));
	ppi0.in_pc_callback().set(FUNC(suprgolf_state::pedal_extra_bits_r));

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("SYSTEM");
	ppi1.out_pb_callback().set(FUNC(suprgolf_state::rom_bank_select_w));
	ppi1.out_pc_callback().set(FUNC(suprgolf_state::vregs_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 0, 191);
	screen.set_screen_update(FUNC(suprgolf_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suprgolf);
	PALETTE(config, m_palette).set_entries(0x800);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", MASTER_CLOCK/4)); /* guess */
	//ymsnd.irq_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	ymsnd.port_a_read_callback().set_ioport("DSW0");
	ymsnd.port_b_read_callback().set_ioport("DSW1");
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_msm, XTAL(384'000)); /* guess */
	m_msm->vck_callback().set(FUNC(suprgolf_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( suprgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )  // on the YUVO-702A main board
	ROM_LOAD( "cg34.k6",0x000000, 0x08000, CRC(4f5ffbce) SHA1(8ca0d41a359a927340058be6d18e01b398f1b077) )

	ROM_REGION( 0x100000, "user1", ROMREGION_ERASEFF ) // on the YUVO-702A main board
	ROM_LOAD( "cg1.hj6", 0x000000, 0x10000, CRC(ee545c71) SHA1(8ee459a85e52257d3f9a2aa7263b641aad87bafd) )
	ROM_LOAD( "cg2.g6",  0x010000, 0x10000, CRC(a2ed2159) SHA1(5e13b6c4eaba8146a4c6c2ff24197f3ffca29b92) )
	ROM_LOAD( "cg3.ef6", 0x020000, 0x10000, CRC(4543334d) SHA1(7ee268ed6d02c78db8c222418313593df37cde4b) )
	ROM_LOAD( "cg4.d6",  0x030000, 0x10000, CRC(85ace664) SHA1(5267406c98e2d124a4985816f8e2e32e74e09614) )
	ROM_LOAD( "cg5.c6",  0x040000, 0x10000, CRC(609d5b37) SHA1(60640a9bd0883bf4dc999077d89ef793e827ac23) )
	ROM_LOAD( "cg6.a6",  0x050000, 0x10000, CRC(5e4a8ddb) SHA1(0c71c7eba9fe79187c4214eb639a481305070dcc) )
	ROM_LOAD( "cg7.hj4", 0x060000, 0x10000, CRC(90ac6734) SHA1(2656397fca6dceabf8e35c093c0ba25e08d2ad1e) )
	ROM_LOAD( "cg8.g4",  0x070000, 0x10000, CRC(2e9edece) SHA1(a0961bb23f312ed137134746d2d3d438fe098085) )
	ROM_LOAD( "cg9.ef4", 0x080000, 0x10000, CRC(139d71f1) SHA1(756ed068e1e2b76a9d1df95b432976e632edfa77) )
	ROM_LOAD( "cg10.d4", 0x090000, 0x10000, CRC(c069e75e) SHA1(77f1b7571e677aef601b8b1c481b352ca6e485d6) )
	/* B4 not populated */
	ROM_LOAD( "cg11.a4", 0x0b0000, 0x10000, CRC(cfec1a0f) SHA1(c09ece059cb3c456b66c016c6fab3139d3f61c6a) )

	ROM_REGION( 0x100000, "user2", ROMREGION_ERASEFF ) // on the OG7-0203 daughter board
	ROM_LOAD( "cg30.ic14", 0x000000, 0x10000, CRC(6b7ffee9) SHA1(7b7f0f9801ab604ea4280c6d75dfcfdb4123520c) )
	ROM_LOAD( "cg31.ic13", 0x010000, 0x10000, CRC(c5ba8e39) SHA1(aff8d5fd532f1e1d90c21bc42a349e3e83c67064) )
	ROM_LOAD( "cg32.ic12", 0x020000, 0x10000, CRC(a2265aa0) SHA1(841e1794bc1b1fc60cdfd4003d1d87bb7ebf503e) )
	ROM_LOAD( "cg33.ic11", 0x030000, 0x10000, CRC(90f1f09d) SHA1(e6c4b6c088a40f97281a1ceb71ed0e73a07ff040) )

	ROM_REGION( 0x70000, "gfx1", 0 ) // on the OG7-0203 daughter board
	ROM_LOAD( "cg12.ic10", 0x00000, 0x10000, CRC(5707b3d5) SHA1(9102a40fefb6426f2cd9d92d66fdc77e078e3f4c) )
	ROM_LOAD( "cg13.ic9",  0x10000, 0x10000, CRC(02ff0187) SHA1(aeeb3b2d15c3c8ff4695ecf6cfc0c385295ecce6) )
	ROM_LOAD( "cg14.ic6",  0x20000, 0x10000, CRC(ca12e01d) SHA1(9c627fb527c8966e16dc6bdb99ec0b9728b5c5f9) )
	ROM_LOAD( "cg15.ic5",  0x30000, 0x10000, CRC(0fb88270) SHA1(d85a7f1bc5b3c4b13bbd887cea4c055541cbb737) )
	ROM_LOAD( "cg16.ic4",  0x40000, 0x10000, CRC(0498aa2e) SHA1(988965c3a584dac17ad8c7e504fa1f1e49775611) )
	ROM_LOAD( "cg17.ic3",  0x50000, 0x10000, CRC(d27f87b5) SHA1(5b2927e89615589540e3853593aeff517584b6a0) )
	ROM_LOAD( "cg18.ic2",  0x60000, 0x10000, CRC(36edd88e) SHA1(374c95721198a88831d6f7e0b71d05e2f8465271) )
ROM_END

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

ROM_START( suprgolfj )
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
	ROM_LOAD( "cg30.ic14", 0x000000, 0x10000, BAD_DUMP CRC(6b7ffee9) SHA1(7b7f0f9801ab604ea4280c6d75dfcfdb4123520c) ) // - empty sockets on PCB :/ (temps from Super Crowns Golf World version)
	ROM_LOAD( "cg31.ic13", 0x010000, 0x10000, BAD_DUMP CRC(c5ba8e39) SHA1(aff8d5fd532f1e1d90c21bc42a349e3e83c67064) ) //
	ROM_LOAD( "2.4c",      0x020000, 0x20000, CRC(08d4363b) SHA1(60c5543c35f44af2f4a8f7ca4bc10633f5fa67fb) ) // matches cg32 + cg33 of the World version

	ROM_REGION( 0x70000, "gfx1", 0 )
	ROM_LOAD( "chr1.3h",      0x000000, 0x010000, CRC(e62d2bb4) SHA1(f931699114a99b7eb25f8bb841d85de0d6a106a5) )
	ROM_CONTINUE(             0x040000, 0x010000 )
	ROM_LOAD( "chr2.3g",      0x010000, 0x010000, CRC(808c15e6) SHA1(d7d1ac7456f492dfcc1c1b501f8dde86e405fd7b) )
	ROM_CONTINUE(             0x050000, 0x010000 )
	ROM_LOAD( "chr3.3e",      0x020000, 0x010000, CRC(9a60193d) SHA1(d22c958b5bd82626fcfc94f7ad16d8cd4bacdda2) )
	ROM_CONTINUE(             0x060000, 0x010000 )
	ROM_LOAD( "chr4.3d",      0x030000, 0x010000, CRC(0fb88270) SHA1(d85a7f1bc5b3c4b13bbd887cea4c055541cbb737) )
ROM_END


void suprgolf_state::init_suprgolf()
{
	uint8_t *ROM = memregion("user2")->base();

	ROM[0x74f4-0x4000] = 0x00;
	ROM[0x74f5-0x4000] = 0x00;
	ROM[0x74fa+(0x4000*3)-0x4000] = 0x20; // patch ROM check
}

void suprgolf_state::init_suprgolfj()
{
	uint8_t *ROM = memregion("user2")->base();

	ROM[0x74f4-0x4000] = 0x00;
	ROM[0x74f5-0x4000] = 0x00;
	ROM[0x6d72+(0x4000*3)-0x4000] = 0x20; // patch ROM check
}

} // Anonymous namespace


GAME( 1989, suprgolf,  0,         suprgolf,  suprgolf, suprgolf_state, init_suprgolf,  ROT0, "Nasco", "Super Crowns Golf (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, suprgolfj, suprgolf,  suprgolf,  suprgolf, suprgolf_state, init_suprgolfj, ROT0, "Nasco", "Super Crowns Golf (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, albatross, suprgolf,  suprgolf,  suprgolf, suprgolf_state, init_suprgolf,  ROT0, "Nasco (Technos license)", "Albatross (US prototype?)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL| MACHINE_SUPPORTS_SAVE )
