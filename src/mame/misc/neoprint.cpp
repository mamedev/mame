// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*******************************************************************************************

    Neo Print (c) 1996 SNK

    preliminary driver by David Haywood & Angelo Salese

    npcartv1 bp 1260 pc += 2
    98best44 bp 1312 pc += 2

    TODO:
    - implement remaining video features;
    - sound interface, needs full Neo-Geo conversion;
    - inputs are bare bones and needs extra work;
    - printer/camera devices;
    - lamps;
    - upd4990a returns 4 years less than expected?
    - nprsp: paletteram has 0x40000 palette entries, kludged to work.

*******************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/upd1990a.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "utf8.h"


namespace {

class neoprint_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::CAMERA | feature::PRINTER; }

	neoprint_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_npvidram(*this, "npvidram"),
		m_npvidregs(*this, "npvidregs"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd4990a(*this, "upd4990a"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_generic_paletteram_16(*this, "paletteram")
	{ }

	void neoprint(machine_config &config);
	void nprsp(machine_config &config);

	void init_98best44();
	void init_npmillen();
	void init_npcartv1();
	void init_npotogib();
	void init_nprsp();
	void init_npscv1();
	void init_npskv();
	void init_npsprgv4();
	void init_unkneo();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t calendar_r();
	void calendar_w(uint8_t data);
	uint8_t unk_r();
	uint8_t audio_result_r();
	void audio_cpu_clear_nmi_w(uint8_t data);
	void audio_command_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t audio_command_r();
	void audio_result_w(uint8_t data);
	void nprsp_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nprsp_bank_w(uint8_t data);
	uint16_t rom_window_r(offs_t offset);
	DECLARE_MACHINE_RESET(nprsp);
	uint32_t screen_update_neoprint(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_nprsp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audio_io_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
	void neoprint_map(address_map &map) ATTR_COLD;
	void nprsp_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint16_t> m_npvidram;
	required_shared_ptr<uint16_t> m_npvidregs;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<upd4990a_device> m_upd4990a;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_shared_ptr<uint16_t> m_generic_paletteram_16;

	uint8_t m_audio_result = 0;
	uint8_t m_bank_val = 0;
	uint8_t m_vblank = 0;
	void draw_layer(bitmap_ind16 &bitmap,const rectangle &cliprect,int layer,int data_shift);
	void audio_cpu_assert_nmi();
};


void neoprint_state::video_start()
{
}

/*
video registers:
xxxx xxxx xxxx xxxx [0] scroll X, signed
xxxx xxxx xxxx xxxx [2] scroll Y, signed
---- ---x ---- ---- [6] enabled on layer 2 only, priority?
---- ---- -x-- ---- [6] layer enable?
---- ---- --?? ??xx [6] map register
*/

void neoprint_state::draw_layer(bitmap_ind16 &bitmap,const rectangle &cliprect,int layer,int data_shift)
{
	int i, y, x;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int16_t scrollx, scrolly;

	i = (m_npvidregs[((layer*8)+0x06)/2] & 7) * 0x1000/4;
	scrollx = ((m_npvidregs[((layer*8)+0x00)/2] - (0xd8 + layer*4)) & 0x03ff);
	scrolly = ((m_npvidregs[((layer*8)+0x02)/2] - 0xffeb) & 0x03ff);

	scrollx/=2;
	scrolly/=2;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			uint16_t dat = m_npvidram[i*2] >> data_shift; // a video register?
			uint16_t color;
			if(m_npvidram[i*2+1] & 0x0020) // TODO: 8bpp switch?
				color = ((m_npvidram[i*2+1] & 0x8000) << 1) | 0x200 | ((m_npvidram[i*2+1] & 0xff00) >> 7);
			else
				color = ((m_npvidram[i*2+1] & 0xff00) >> 8) | ((m_npvidram[i*2+1] & 0x0010) << 4);
			uint8_t fx = (m_npvidram[i*2+1] & 0x0040);
			uint8_t fy = (m_npvidram[i*2+1] & 0x0080);

			gfx->transpen(bitmap,cliprect,dat,color,fx,fy,x*16+scrollx,y*16-scrolly,0);
			gfx->transpen(bitmap,cliprect,dat,color,fx,fy,x*16+scrollx-512,y*16-scrolly,0);
			gfx->transpen(bitmap,cliprect,dat,color,fx,fy,x*16+scrollx,y*16-scrolly-512,0);
			gfx->transpen(bitmap,cliprect,dat,color,fx,fy,x*16+scrollx-512,y*16-scrolly-512,0);

			i++;
			//i&=0x3ff;
		}
	}
}

uint32_t neoprint_state::screen_update_neoprint(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_layer(bitmap,cliprect,1,2);
	draw_layer(bitmap,cliprect,0,2);

	return 0;
}

uint32_t neoprint_state::screen_update_nprsp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_layer(bitmap,cliprect,1,0);
	draw_layer(bitmap,cliprect,2,0);
	draw_layer(bitmap,cliprect,0,0);

	return 0;
}


uint8_t neoprint_state::calendar_r()
{
	return (m_upd4990a->data_out_r() << 7) | (m_upd4990a->tp_r() << 6);
}

void neoprint_state::calendar_w(uint8_t data)
{
	m_upd4990a->data_in_w(data >> 0 & 1);
	m_upd4990a->clk_w(data >> 1 & 1);
	m_upd4990a->stb_w(data >> 2 & 1);
}

uint8_t neoprint_state::unk_r()
{
	/* ---x ---- tested in irq routine, odd/even field number? */
	/* ---- xx-- one of these two must be high */
	/* ---- --xx checked right before entering into attract mode, presumably printer/camera related */

	m_vblank = (m_screen->frame_number() & 0x1) ? 0x10 : 0x00;

	//if(m_maincpu->pc() != 0x1504 && m_maincpu->pc() != 0x5f86 && m_maincpu->pc() != 0x5f90)
	//  printf("%08x\n",m_maincpu->pc());

	return m_vblank| 4 | 3;
}

uint8_t neoprint_state::audio_result_r()
{
	return m_audio_result;
}

void neoprint_state::audio_cpu_assert_nmi()
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void neoprint_state::audio_cpu_clear_nmi_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void neoprint_state::audio_command_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_soundlatch->write(data);

	audio_cpu_assert_nmi();

	/* boost the interleave to let the audio CPU read the command */
	machine().scheduler().perfect_quantum(attotime::from_usec(50));

	//if (LOG_CPU_COMM) logerror("MAIN CPU PC %06x: audio_command_w %04x - %04x\n", m_maincpu->pc(), data, mem_mask);
}


uint8_t neoprint_state::audio_command_r()
{
	uint8_t ret = m_soundlatch->read();

	//if (LOG_CPU_COMM) logerror(" AUD CPU PC   %04x: audio_command_r %02x\n", m_audiocpu->pc(), ret);

	/* this is a guess */
	audio_cpu_clear_nmi_w(0);

	return ret;
}



void neoprint_state::audio_result_w(uint8_t data)
{
	//if (LOG_CPU_COMM && (m_audio_result != data)) logerror(" AUD CPU PC   %04x: audio_result_w %02x\n", m_audiocpu->pc(), data);

	m_audio_result = data;
}

void neoprint_state::neoprint_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
/*  map(0x100000, 0x17ffff) multi-cart or banking, some writes points here if anything lies there too */
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x30ffff).ram().share("nvram");
	map(0x400000, 0x43ffff).ram().share("npvidram");
	map(0x500000, 0x51ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x600000, 0x600000).rw(FUNC(neoprint_state::audio_result_r), FUNC(neoprint_state::audio_command_w));
	map(0x600002, 0x600002).rw(FUNC(neoprint_state::calendar_r), FUNC(neoprint_state::calendar_w));
	map(0x600004, 0x600005).portr("SYSTEM").nopw();
	map(0x600006, 0x600007).portr("IN").nopw();
	map(0x600008, 0x600009).portr("DSW1");
	map(0x60000a, 0x60000a).r(FUNC(neoprint_state::unk_r));
	map(0x60000c, 0x60000d).portr("DSW2");
	map(0x60000e, 0x60000f).nopw();

	map(0x700000, 0x70001b).ram().share("npvidregs");

	map(0x70001e, 0x70001f).nopw(); //watchdog
}

void neoprint_state::nprsp_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint8_t r,g,b,i;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);

	g = (m_generic_paletteram_16[offset & ~1] & 0xf800) >> 8;
	r = (m_generic_paletteram_16[offset & ~1] & 0x00f8) >> 0;
	i = (m_generic_paletteram_16[offset | 1] & 0x1c00) >> 10;
	b = (m_generic_paletteram_16[offset | 1] & 0x00f8) >> 0;
	r |= i;
	g |= i;
	b |= i;

	/* hack: bypass MAME 65536 palette entries limit */
	if(offset & 0x10000)
		return;

	{
		uint32_t pal_entry;

		pal_entry = ((offset & 0xfffe) >> 1) + ((offset & 0x20000) ? 0x8000 : 0);

		m_palette->set_pen_color(pal_entry, rgb_t(r,g,b));
	}
}

void neoprint_state::nprsp_bank_w(uint8_t data)
{
	/* this register seems flip-flop based ... */

	if((data & 0xf0) == 0x20)
	{
		if((data & 0xf) == 0x1)
			m_bank_val = 1;
		if((data & 0xf) == 0x2)
			m_bank_val = 0;
	}
}

uint16_t neoprint_state::rom_window_r(offs_t offset)
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	return rom[offset | 0x80000/2 | m_bank_val*0x40000/2];
}

void neoprint_state::nprsp_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x0fffff).r(FUNC(neoprint_state::rom_window_r));
	map(0x200000, 0x200000).rw(FUNC(neoprint_state::audio_result_r), FUNC(neoprint_state::audio_command_w));
	map(0x200002, 0x200002).rw(FUNC(neoprint_state::calendar_r), FUNC(neoprint_state::calendar_w));
	map(0x200004, 0x200005).portr("SYSTEM").nopw();
	map(0x200006, 0x200007).portr("IN").nopw();
	map(0x200008, 0x200009).portr("DSW1");
	map(0x200008, 0x200008).w(FUNC(neoprint_state::nprsp_bank_w));
	map(0x20000a, 0x20000a).r(FUNC(neoprint_state::unk_r));
	map(0x20000c, 0x20000d).portr("DSW2");
	map(0x20000e, 0x20000f).nopw();

	map(0x240000, 0x24001b).ram().share("npvidregs");
	map(0x24001e, 0x24001f).nopw(); //watchdog

	map(0x300000, 0x33ffff).ram().share("nvram");
	map(0x380000, 0x38ffff).ram();
	map(0x400000, 0x43ffff).ram().share("npvidram");
	map(0x500000, 0x57ffff).ram().w(FUNC(neoprint_state::nprsp_palette_w)).share("paletteram");
}

/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

void neoprint_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();//.bankr(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK);
//  map(0x8000, 0xbfff).bankr(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 3);
//  map(0xc000, 0xdfff).bankr(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 2);
//  map(0xe000, 0xefff).bankr(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 1);
//  map(0xf000, 0xf7ff).bankr(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 0);
	map(0xf800, 0xffff).ram();
}



/*************************************
 *
 *  Audio CPU port handlers
 *
 *************************************/

void neoprint_state::audio_io_map(address_map &map)
{
	/*map(0x00, 0x00).mirror(0xff00).rw(FUNC(neoprint_state::audio_command_r), FUNC(neoprint_state::audio_cpu_clear_nmi_w));*/  /* may not and NMI clear */
	map(0x00, 0x00).mirror(0xff00).r(FUNC(neoprint_state::audio_command_r)).nopw();
	map(0x04, 0x07).mirror(0xff00).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
//  map(0x08, 0x08).mirror(0xff00); /* write - NMI enable / acknowledge? (the data written doesn't matter) */
//  map(0x08, 0x08).select(0xfff0).r(FUNC(neoprint_state::audio_cpu_bank_select_f000_f7ff_r));
//  map(0x09, 0x09).select(0xfff0).r(FUNC(neoprint_state::audio_cpu_bank_select_e000_efff_r));
//  map(0x0a, 0x0a).select(0xfff0).r(FUNC(neoprint_state::audio_cpu_bank_select_c000_dfff_r));
//  map(0x0b, 0x0b).select(0xfff0).r(FUNC(neoprint_state::audio_cpu_bank_select_8000_bfff_r));
	map(0x0c, 0x0c).mirror(0xff00).w(FUNC(neoprint_state::audio_result_w));
//  map(0x18, 0x18).mirror(0xff00); /* write - NMI disable? (the data written doesn't matter) */
}

static INPUT_PORTS_START( neoprint )
	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
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

	PORT_START("IN")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "IN0" )
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
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME( UTF8_RIGHT " Button") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME( UTF8_LEFT " Button") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Green Button") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Red Button") PORT_CODE(KEYCODE_Z)
	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW1" )
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
	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW2" )
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
INPUT_PORTS_END


static const gfx_layout neoprint_layout =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ 48, 16, 40, 8, 32, 0  },
	{ 0,1,2,3,4,5,6,7, 1024+0,1024+1,1024+2,1024+3,1024+4,1024+5,1024+6,1024+7 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	32*64,
};

static GFXDECODE_START( gfx_neoprint )
	GFXDECODE_ENTRY( "gfx1", 0, neoprint_layout,   0x0, 0x1000 )
GFXDECODE_END

void neoprint_state::machine_start()
{
	// enable rtc and serial mode
	m_upd4990a->cs_w(1);
	m_upd4990a->oe_w(1);
	m_upd4990a->c0_w(1);
	m_upd4990a->c1_w(1);
	m_upd4990a->c2_w(1);
}

void neoprint_state::neoprint(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &neoprint_state::neoprint_map);
	m_maincpu->set_periodic_int(FUNC(neoprint_state::irq3_line_hold), attotime::from_hz(45)); /* camera / printer irq, unknown timing */
	m_maincpu->set_vblank_int("screen", FUNC(neoprint_state::irq2_line_hold)); // lv1,2,3 valid?

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &neoprint_state::audio_map);
	m_audiocpu->set_addrmap(AS_IO, &neoprint_state::audio_io_map);

	UPD4990A(config, m_upd4990a);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_neoprint);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 48*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(neoprint_state::screen_update_neoprint));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 24000000 / 3));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(0, "rspeaker", 0.60);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

MACHINE_RESET_MEMBER(neoprint_state,nprsp)
{
	m_bank_val = 0;
}

void neoprint_state::nprsp(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &neoprint_state::nprsp_map);
	m_maincpu->set_periodic_int(FUNC(neoprint_state::irq3_line_hold), attotime::from_hz(45)); /* camera / printer irq, unknown timing */
	m_maincpu->set_vblank_int("screen", FUNC(neoprint_state::irq2_line_hold)); // lv1,2,3 valid?

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &neoprint_state::audio_map);
	m_audiocpu->set_addrmap(AS_IO, &neoprint_state::audio_io_map);

	UPD4990A(config, m_upd4990a);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_neoprint);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 48*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(neoprint_state::screen_update_nprsp));
	m_screen->set_palette(m_palette);

	MCFG_MACHINE_RESET_OVERRIDE(neoprint_state,nprsp)

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 24000000 / 3));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(0, "rspeaker", 0.60);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}


// uses NEO-MVS PROGBK1 (Same as NeoGeo MVS cart)
// and PSTM-ROMC (unique to NeoPrint) (has ZMC chip)
ROM_START( neoprint ) // NP 1.21 19961210 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ep1.bin", 0x000000, 0x80000, CRC(271da3ee) SHA1(50132d2ac5524e880ec0c2ba3617bf516fd36e7d) )
//  ROM_RELOAD(                      0x100000, 0x80000 ) /* checks the same string from above to be present there? Why? */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "m1- tc571000.bin", 0x00000, 0x20000, CRC(d720f53e) SHA1(7a20200065d3da43fcf7d5922d1808dc896b0da8) ) // possible bad dump? (although looks ok, just mostly empty?)

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "v1.bin", 0x00000, 0x200000, CRC(c1984fa9) SHA1(9702d253ad75e0b0f0182d8da449328648ff1a2f) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "c1.bin", 0x000000, 0x80000, CRC(ec6770e1) SHA1(2f4629dbadcdbd90117fe5fad1aa5601808864c9) )
	ROM_LOAD32_BYTE( "c2.bin", 0x000001, 0x80000, CRC(8855bae0) SHA1(cb717abb3f8a121c0e5bb7c6ef76ff30bbc66895) )
	ROM_LOAD32_BYTE( "c3.bin", 0x200000, 0x80000, CRC(c54be0b2) SHA1(ab64c6115c9033babcaeb0910609ef0f41a5f3f2) )
	ROM_LOAD32_BYTE( "c4.bin", 0x200001, 0x80000, CRC(c7e5e6ce) SHA1(f1e37732446ae6146b3cb51a9714c5edd539d7e4) )
ROM_END

ROM_START( npcartv1 ) // NP 1.11 19961018 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ep1.bin", 0x000000, 0x80000, CRC(18606198) SHA1(d968e09131c22769e22c7310aca1f02e739f38f1) )
//  ROM_RELOAD(                      0x100000, 0x80000 ) /* checks the same string from above to be present there? Why? */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "m1.bin", 0x00000, 0x20000, CRC(b2d38e12) SHA1(ab96c5d3d22eb71ed6e0a03f3ff5d4b23e72fad8) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "v1.bin", 0x00000, 0x80000, CRC(2d6608f9) SHA1(7dbde1c305ab3438b7fe7417816427c682371bd4) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "c1.bin", 0x00000, 0x80000, CRC(b89f1fb6) SHA1(e711f91a7872b2e0edc3f42a726d969096d684f2) )
	ROM_LOAD32_BYTE( "c2.bin", 0x00001, 0x80000, CRC(7ce39dc2) SHA1(c5be90657350258b670b55dd9c77f7899133ced3) )
ROM_END

ROM_START( npsprgv4 ) // NP 1.30 19970228 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p004-ep1 neo-mvs progbk1.ep1", 0x000000, 0x80000, CRC(4a322439) SHA1(4478f0e20d2c892a2c8e67ccc1173fd5edaa42e3) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "p004-m1 neo-pstm cha136.m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) ) // same as 98best44

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "p004-v1 neo-mvs progbk1.v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) ) // same as 98best44

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p004-c1 neo-pstm cha136.c1", 0x000000, 0x200000, CRC(3654d45c) SHA1(f2f770d090e45008da682775847fdb3433163eae) )
	ROM_LOAD32_BYTE( "p004-c2 neo-pstm cha136.c2", 0x000001, 0x200000, CRC(de75f729) SHA1(b8880c8b358aa4b59d79493458d27a0a13d80094) )
ROM_END

	/* logo: Neo Print
	small text: Cassette supporting Neo Print and Neo Print Multi
	(cassette=cartridge)
	title: '98 NeoPri Best 44 version */

ROM_START( 98best44 ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p060-ep1", 0x000000, 0x080000, CRC(d42e505d) SHA1(0ad6b0288f36c339832730a03e53cbc07dab4f82))
//  ROM_RELOAD(                      0x100000, 0x80000 ) /* checks the same string from above to be present there? Why? */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "pt004-m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "pt004-v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "pt060-c1", 0x000000, 0x100000, CRC(22a23090) SHA1(0e219fcfea6ca2ddf4b7b4197aac8bc55a29d5cf) )
	ROM_LOAD32_BYTE( "pt060-c2", 0x000001, 0x100000, CRC(66a8e56a) SHA1(adfd1e52d52806a785f1e9b1ae2ac969b6ed60af) )
ROM_END

ROM_START( npsprg98 ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p042-p1 neo-mvs progbk1.p1", 0x000000, 0x100000, CRC(c0621456) SHA1(eb615a11f909a680aed2d99c641b3c47be4fc56e) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "pt004-m1 neo-pstm cha136.m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) ) // same as 98best44

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "pt004-v1 neo-mvs progbk1.v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) ) // same as 98best44

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p042-c1 neo-pstm cha136.c1", 0x000000, 0x100000, CRC(bc822f3f) SHA1(a24e0a556d2f15a61fd80a049b7b31d61cb0596e) )
	ROM_LOAD32_BYTE( "p042-c2 neo-pstm cha136.c2", 0x000001, 0x100000, CRC(95a4a0a9) SHA1(2e07006af6c84c98a5b5ab3191e3278766e91faa) )
ROM_END

ROM_START( npskv ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p012-p1 neo-mvs progbk1.p1", 0x000000, 0x100000, CRC(de8996f6) SHA1(8fb2bc78206ec543148740f94c19bcdb50ad3271) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "p004-m1 neo-pstm cha136.m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) ) // same as 98best44

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "p04-v1 neo-mvs progbk1.v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) ) // same as 98best44

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p012-c1 neo-pstm cha136.c1", 0x000000, 0x100000, CRC(315f27c0) SHA1(889c82073ac0fb94e5bd7b6ff11f19261d79f011) )
	ROM_LOAD32_BYTE( "p012-c2 neo-pstm cha136.c2", 0x000001, 0x100000, CRC(0711f184) SHA1(4ab860c5e200fec70374ab552c97b59a35ca73c3) )
ROM_END

ROM_START( npusagif ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p061-ep1 neo-mvs progbk1.ep1", 0x000000, 0x080000, CRC(ec6d7fda) SHA1(f219f8a9763f92ef952236ea3e01fe9b684823df) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "pt004-m1 neo-pstm cha136.m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) ) // same as 98best44

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "pt004-v1 neo-mvs progbk1.v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) ) // same as 98best44

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p061-c1 neo-pstm cha136.c1", 0x000000, 0x100000, CRC(9686c78d) SHA1(a72a191540bbe3121e3be9e05e683ba9f2714aba) )
	ROM_LOAD32_BYTE( "p061-c2 neo-pstm cha136.c2", 0x000001, 0x100000, CRC(f354b86b) SHA1(1058465af35fef6923f7fbe2cccf4c01509528d6) )
ROM_END

ROM_START( npotogib ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p0025-ep1 neo-mvs progbk1.ep1", 0x000000, 0x080000, CRC(eaefe748) SHA1(9facab7e70901a9030d40b823473e46cfa5389ad) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "pt004-m1 neo-pstm cha136.m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) ) // same as 98best44

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "pt004-v1 neo-mvs progbk1.v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) ) // same as 98best44

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p0025-c1 pstm-romc.c1", 0x000000, 0x80000, CRC(585451c5) SHA1(288d1f725c8747d260f46ed7ef8f380c991e8b33) )
	ROM_LOAD32_BYTE( "p0025-c2 pstm-romc.c2", 0x000001, 0x80000, CRC(74468a3d) SHA1(55b85c02d033636e8f7feda79bb3c0b29408f361) )
	ROM_LOAD32_BYTE( "p0025-c3 pstm-romc.c3", 0x200001, 0x80000, CRC(ef1854e7) SHA1(e254bcf03845b61d9f40efeaba7fba133aaad79d) )
	ROM_LOAD32_BYTE( "p0025-c4 pstm-romc.c4", 0x200002, 0x80000, CRC(a2261905) SHA1(cb05a11ed4c302448c3e6779cb5428dd0c907e18) )
ROM_END

ROM_START( npfpit ) // NP 1.30 19990225 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "po97-ep1 neo-mvs progbk1.ep1", 0x000000, 0x080000, CRC(d2940f25) SHA1(d65e719d9df993e1433e580797bf0580d564c9a2) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "pt004-m1 neo-pstm cha136.m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) ) // same as 98best44

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "pt004-v1 neo-mvs progbk1.v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) ) // same as 98best44

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p097-c1 pstm-romc.c1", 0x000000, 0x80000, CRC(60163d90) SHA1(3707be8064c083814a0c4a31bccc7150aa250891) )
	ROM_LOAD32_BYTE( "p097-c2 pstm-romc.c2", 0x000001, 0x80000, CRC(29f59ff7) SHA1(acb427383bb2f7fd675d7f5b97fea7f092bdc8d5) )
	ROM_LOAD32_BYTE( "p097-c3 pstm-romc.c3", 0x200001, 0x80000, CRC(8ceddebf) SHA1(2d11721f3b9724358cc528e8028b091972506b91) )
	ROM_LOAD32_BYTE( "p097-c4 pstm-romc.c4", 0x200002, 0x80000, CRC(88d0fb2e) SHA1(78bd4262e009ede7c179b783ab2b9bd1662a6f56) )
ROM_END

ROM_START( nprsp ) // STAFYAMA19980925 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "s038a-ep1.bin", 0x000000, 0x080000, CRC(529fb4fa) SHA1(f31ba8998bb01458f43df1934222995f22d590a1) ) // program ROM
	ROM_LOAD16_WORD_SWAP( "s046-ep2.bin",  0x080000, 0x080000, CRC(846ae929) SHA1(e5544cde32794865e17d7dffd4e603ad5418d91e) ) // data ROM

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "s001-m1.bin", 0x00000, 0x20000, CRC(ea8111c1) SHA1(69e6bb7ad9a8d61db4513a762c0ce9e9da2a1785) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "s001-v1.bin", 0x000000, 0x100000, CRC(13d63625) SHA1(4a9e3b1192a4a7e405becfd5d2a95ffc14ae6e79)  )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "s046-c1.bin", 0x000000, 0x200000, CRC(06fffce0) SHA1(0d9bb9d3107b1efb66ee82341c3f80ec093d5987) )
	ROM_LOAD32_BYTE( "s046-c2.bin", 0x000001, 0x200000, CRC(7cc353e7) SHA1(5c4fa9fdf90bd0d03608becaa174d68735f28bbb) )
	ROM_LOAD32_BYTE( "s046-c3.bin", 0x000002, 0x200000, CRC(f68f0f6f) SHA1(2fc105953a17259353f74376661c442658f9a644) )
	// 8bpp might be possible with another ROM?
ROM_END

ROM_START( npssr2 ) // STAFYAMA19980925 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "s038a-ep1 nps-prg1.ep1", 0x000000, 0x080000, CRC(529fb4fa) SHA1(f31ba8998bb01458f43df1934222995f22d590a1) ) // program ROM, same as nprsp
	ROM_LOAD16_WORD_SWAP( "s072-ep2 nps-prg1.ep2",  0x080000, 0x080000, CRC(5514e29f) SHA1(fd508b6b4b2ed587b5dfd4a186865c72181612e6) ) // data ROM

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "s001-m1 neo-pstm cha64c.m1", 0x00000, 0x20000, CRC(ea8111c1) SHA1(69e6bb7ad9a8d61db4513a762c0ce9e9da2a1785) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "s001-v1 nps-prg1.v1", 0x000000, 0x100000, CRC(13d63625) SHA1(4a9e3b1192a4a7e405becfd5d2a95ffc14ae6e79)  )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "s072-c1 neo-pstm cha64c.c1", 0x000000, 0x200000, CRC(c5f72b00) SHA1(b81a9d6025e79f98bdfd55d630ede181ef821132) )
	ROM_LOAD32_BYTE( "s072-c2 neo-pstm cha64c.c2", 0x000001, 0x200000, CRC(3153db5a) SHA1(2166f452131f64e3806d177b5c5a6ddcd312adfd) )
	ROM_LOAD32_BYTE( "s072-c3 neo-pstm cha64c.c3", 0x000002, 0x200000, CRC(57e8888f) SHA1(59acb22b2744bb798dd4779d9413d01bee79b43e) )

	ROM_REGION( 0x100, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "br9020 nps-prg1.u5", 0x000, 0x100, NO_DUMP ) // dump provided was 0xff filled
ROM_END

ROM_START( npmillen ) // NP 1.30 19990225 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p093-ep1.bin", 0x000000, 0x080000, CRC(47783f56) SHA1(1845e90b05a58010054c4158ef08e167e61ea370) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "p016-m1.bin", 0x00000, 0x20000, CRC(f40cf036) SHA1(63041318d8bec144a4688cc5f45107f8331809bf) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "p016-v1.bin", 0x000000, 0x200000, CRC(400ca9ce) SHA1(f8636a4600200ef9000a25e80cf20f252703ad37) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p093-c1.bin", 0x000000, 0x80000, CRC(bbb8266b) SHA1(3cbe8c9a6a82d9927910833b5874fe0a4a8c8384) )
	ROM_LOAD32_BYTE( "p093-c2.bin", 0x000001, 0x80000, CRC(a82e79f4) SHA1(cc3a0171d488167212c2baaeda7c6cf13bb19611) )
	ROM_LOAD32_BYTE( "p093-c3.bin", 0x200001, 0x80000, CRC(11554065) SHA1(4a75dbcc04b5f6bf82cf22093ed3d29ae8ee4c5d) )
	ROM_LOAD32_BYTE( "p093-c4.bin", 0x200002, 0x80000, CRC(01d9c22a) SHA1(bf3f40de7f70cb5bb0fe487021ab110192c3b247) )
ROM_END

ROM_START( npscv1 ) // NP 1.10 19961015 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "np-jp1 neo-mvs progbk1.ep1", 0x000000, 0x080000, CRC(c4648dfa) SHA1(ca7770f363027e3fe2f47d77085464486c024d2a) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "np-m1 pstm-romc.m1", 0x00000, 0x20000, CRC(adbcad85) SHA1(76ebecea081a47b9fb133ef7793b48b51ef2f5c5) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "np-v1 neo-mvs progbk1.v1", 0x000000, 0x80000, CRC(99d414e8) SHA1(5aecb09c7f18fca18f61e67047dfca06744928ed) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "np-jc1 pstm-romc.c1", 0x000000, 0x080000, CRC(a729d3d4) SHA1(b0a8dfd2fc5c6707e5dd2a6d702a5d43a927c716) )
	ROM_LOAD32_BYTE( "np-jc2 pstm-romc.c2", 0x000001, 0x080000, CRC(c9687cd8) SHA1(809d9efd7ea76de0884ad76effb71217e9068f89) )
ROM_END

ROM_START( npcramen ) // ? string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "l009-ep1 neo-mvs progbk1.ep1", 0x000000, 0x080000, BAD_DUMP CRC(ff470ded) SHA1(d33dd90f9ac1cc7f2dcadb6a855d9cd5f3260d00) ) // 111111111xxxxxxxxxx = 0xFF, reads were consistent, but..

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* Z80 program */
	// empty (removed or never populated)?

	ROM_REGION( 0x80000, "ymsnd:adpcma", ROMREGION_ERASEFF ) /* Samples */
	// empty (removed or never populated)?

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "l009-c1 pstm-romc.c1", 0x000000, 0x080000, CRC(d6b44550) SHA1(d402ffe372646b93344f7e36d29e4fe913f2479a) )
	ROM_LOAD32_BYTE( "l009-c2 pstm-romc.c2", 0x000001, 0x080000, CRC(d63dea34) SHA1(cf2dbf982ed955fe5a4c737d1752cdb66ab5f84a) )
ROM_END

ROM_START( npft ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ft_ep1.ep1", 0x000000, 0x080000, CRC(870d6e77) SHA1(651958d5254763f308e53ab46c4c70694e57acd7) ) // hand-written label

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "p004-m1 neo-pstm cha136.m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) ) // same as 98best44

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "p004-v1 neo-mvs progbk1.v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) ) // same as 98best44

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 ) // hand-written labels
	ROM_LOAD32_BYTE( "ft_c1.c1", 0x000000, 0x080000, CRC(2977a558) SHA1(fc4777899c2dcb01aa31a974d3b7c83cdb38e650) )
	ROM_LOAD32_BYTE( "ft_c2.c2", 0x000001, 0x080000, CRC(15468148) SHA1(9a96e75894cc2bb51c3c3dc83b31f13e68c2bd85) )
	ROM_LOAD32_BYTE( "ft_c3.c3", 0x200001, 0x080000, CRC(dcc527cf) SHA1(e49ade0f422439e99a7dbe97d1f9df833037301d) )
	ROM_LOAD32_BYTE( "ft_c4.c4", 0x200001, 0x080000, CRC(c91dea9d) SHA1(a99e18c4f737e3e5964371cf5e05fecbc6e7a6fe) )
ROM_END

ROM_START( nppopeye ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p027-ep1", 0x000000, 0x080000, CRC(f928ad2e) SHA1(a958b2d357af6daf2bde6d5b8874963c9c4130c3))

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 program */
	ROM_LOAD( "pt004-m1", 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "pt004-v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p027-1-c1", 0x000000, 0x80000, CRC(08663e2f) SHA1(019b37125a639e205acdc34f486d7ba8318d92d5) )
	ROM_LOAD32_BYTE( "p027-1-c2", 0x000001, 0x80000, CRC(67cec95c) SHA1(5ff33b4adaa21604f20363fad1f3930f5230a52e) )
	ROM_LOAD32_BYTE( "p027-1-c3", 0x200000, 0x80000, CRC(c299399d) SHA1(1c2ed93131cd5771ed219ec11870ada9945b3fdc) )
	ROM_LOAD32_BYTE( "p027-1-c4", 0x200001, 0x80000, CRC(a406a483) SHA1(c02c682dd964bc006baecde193bfb8d95c4795a8) )
ROM_END

ROM_START( npeurver ) // NP 1.30 19970430 string
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p016-1-ep1", 0x000000, 0x080000, CRC(941af83b) SHA1(c385164f2671e183fbcec543d738463c03f1829a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "p016-m1.bin", 0x00000, 0x20000, CRC(f40cf036) SHA1(63041318d8bec144a4688cc5f45107f8331809bf) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "p016-v1.bin", 0x000000, 0x200000, CRC(400ca9ce) SHA1(f8636a4600200ef9000a25e80cf20f252703ad37) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "p016-c1", 0x000000, 0x80000, CRC(10b96226) SHA1(b791b3dd43a363255033648c1be160d42a024cbe) )
	ROM_LOAD32_BYTE( "p016-c2", 0x000001, 0x80000, CRC(88df9dce) SHA1(10a8866672c1d05efb8a1f7516d7fba340f3fb0b) )
	ROM_LOAD32_BYTE( "p016-c3", 0x200001, 0x80000, CRC(c69d82f6) SHA1(4c4267a851e438c4e50af1f2bbe80220d6f1d920) )
	ROM_LOAD32_BYTE( "p016-c4", 0x200002, 0x80000, CRC(faa3d47e) SHA1(c57324e339e4c6e60000309597889d2e17f0d3bd) )
ROM_END

/* FIXME: get rid of these two, probably something to do with irq3 and camera / printer devices */
void neoprint_state::init_npcartv1()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();

	ROM[0x1260/2] = 0x4e71;

	ROM[0x43c8/2] = 0x4e71; //ROM checksum
}


void neoprint_state::init_98best44()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();

	ROM[0x1312/2] = 0x4e71;
}

void neoprint_state::init_npmillen()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();

	ROM[0x1312/2] = 0x4e71;

	ROM[0x42a8/2] = 0x4e71; //ROM checksum
}

void neoprint_state::init_npsprgv4()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();

	ROM[0x12e8/2] = 0x4e71;

	ROM[0x3ba0/2] = 0x4e71; //ROM checksum
}

void neoprint_state::init_npskv()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();

	ROM[0x130a/2] = 0x4e71;

	ROM[0x3bf4/2] = 0x4e71; //ROM checksum
}

void neoprint_state::init_nprsp()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();

	ROM[0x13a4/2] = 0x4e71;
	ROM[0x13bc/2] = 0x4e71;
	ROM[0x140c/2] = 0x4e71;

	ROM[0x4832/2] = 0x4e71; //ROM checksum
	ROM[0x4834/2] = 0x4e71;
}

void neoprint_state::init_unkneo()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();
	ROM[0x12c2/2] = 0x4e71;
}

void neoprint_state::init_npscv1()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();
	ROM[0x1242/2] = 0x4e71;

	ROM[0x4390/2] = 0x4e71; //ROM checksum
}

void neoprint_state::init_npotogib()
{
	uint16_t *ROM = (uint16_t *)memregion( "maincpu" )->base();
	ROM[0x1312/2] = 0x4e71;

	ROM[0x3f4e/2] = 0x4e71; //ROM checksum
}

} // anonymous namespace


GAME( 1996, neoprint,    0,        neoprint,    neoprint, neoprint_state, init_unkneo,   ROT0, "SNK", "Neo Print (Japan) (T2d)",                                       MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1996, npcartv1,    0,        neoprint,    neoprint, neoprint_state, init_npcartv1, ROT0, "SNK", "Neo Print V1 (World) (E1a)",                                    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1996, npscv1,      0,        neoprint,    neoprint, neoprint_state, init_npscv1,   ROT0, "SNK", "Neo Print - Senyou Cassette Ver. 1 (Japan)",                    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1996, npcramen,    0,        neoprint,    neoprint, neoprint_state, empty_init,    ROT0, "SNK", "Neo Print - Chicken Ramen (Japan)",                             MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, npsprgv4,    0,        neoprint,    neoprint, neoprint_state, init_npsprgv4, ROT0, "SNK", "Neo Print - Spring Ver. 4 (Japan) (T4f 1.00)",                  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, npskv,       0,        neoprint,    neoprint, neoprint_state, init_npskv,    ROT0, "SNK", "Neo Print - Suizokukan Version (Japan) (T4i 2.00)",             MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, npotogib,    0,        neoprint,    neoprint, neoprint_state, init_npotogib, ROT0, "SNK", "Neo Print - Otogibanashi (Japan) (T4i 3.00)",                   MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, nppopeye,    0,        neoprint,    neoprint, neoprint_state, init_98best44, ROT0, "SNK", "Neo Print - Popeye (Japan) (T4i 3.04)",                         MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, npeurver,    0,        neoprint,    neoprint, neoprint_state, init_npskv,    ROT0, "SNK", "Neo Print - European Version (World) (T4i 2.00)",               MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1997, npusagif,    0,        neoprint,    neoprint, neoprint_state, init_98best44, ROT0, "SNK", "Neo Print - Usagi Frame (Japan) (T4i 3.07)",                    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1998, 98best44,    0,        neoprint,    neoprint, neoprint_state, init_98best44, ROT0, "SNK", "Neo Print - '98 NeoPri Best 44 (Japan) (T4i 3.07)",             MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1998, npsprg98,    0,        neoprint,    neoprint, neoprint_state, init_npmillen, ROT0, "SNK", "Neo Print - Spring '98 (T4i 3.07)",                             MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, npmillen,    0,        neoprint,    neoprint, neoprint_state, init_npmillen, ROT0, "SNK", "Neo Print - Millennium Multi Shot Edition (World) (T4i 3.07)",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, npfpit,      0,        neoprint,    neoprint, neoprint_state, init_npmillen, ROT0, "SNK", "Neo Print - Fuyu Pri Iitoko-dori (Japan) (T4i 3.07)",           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, npft,        0,        neoprint,    neoprint, neoprint_state, init_npmillen, ROT0, "SNK", "Neo Print - Fairy Tales (World) (T4i 3.07)",                    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1996, nprsp,       0,        nprsp,       neoprint, neoprint_state, init_nprsp,    ROT0, "SNK", "NeopriSP Retro Collection (Japan)",                             MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1996, npssr2,      0,        nprsp,       neoprint, neoprint_state, init_nprsp,    ROT0, "SNK", "Neo Print Special: Sekai Ryokou 2 (Japan)",                     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
