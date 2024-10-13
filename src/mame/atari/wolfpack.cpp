// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch

/***************************************************************************

    Atari Wolf Pack (prototype) driver

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/watchdog.h"
#include "sound/s14001a.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class wolfpack_state : public driver_device
{
public:
	wolfpack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_maincpu(*this, "maincpu"),
		m_s14001a(*this, "speech"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_dial(*this, "DIAL"),
		m_led(*this, "led0")
	{ }

	void wolfpack(machine_config &config);

	template <int Bit> int dial_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices, pointers
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_device<cpu_device> m_maincpu;
	required_device<s14001a_device> m_s14001a;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport m_dial;
	output_finder<> m_led;

	bool m_collision = false;
	unsigned m_current_index = 0;
	uint8_t m_video_invert = 0;
	uint8_t m_ship_reflect = 0;
	uint8_t m_pt_pos_select = 0;
	uint8_t m_pt_horz = 0;
	uint8_t m_pt_pic = 0;
	uint8_t m_ship_h = 0;
	uint8_t m_torpedo_pic = 0;
	uint8_t m_ship_size = 0;
	uint8_t m_ship_h_precess = 0;
	uint8_t m_ship_pic = 0;
	uint8_t m_torpedo_h = 0;
	uint8_t m_torpedo_v = 0;
	std::unique_ptr<uint8_t[]> m_lfsr;
	bitmap_ind16 m_helper;
	emu_timer *m_periodic_timer = nullptr;

	void main_map(address_map &map) ATTR_COLD;

	void wolfpack_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_CALLBACK_MEMBER(periodic_callback);
	void draw_ship(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_torpedo(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_pt(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_water(palette_device &palette, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t misc_r();
	void high_explo_w(uint8_t data);
	void sonar_ping_w(uint8_t data);
	void sirlat_w(uint8_t data);
	void pt_sound_w(uint8_t data);
	void launch_torpedo_w(uint8_t data);
	void low_explo_w(uint8_t data);
	void screw_cont_w(uint8_t data);
	void lamp_flash_w(uint8_t data);
	void warning_light_w(uint8_t data);
	void audamp_w(uint8_t data);
	void attract_w(uint8_t data);
	void credit_w(uint8_t data);
	void coldetres_w(uint8_t data);
	void ship_size_w(uint8_t data);
	void video_invert_w(uint8_t data);
	void ship_reflect_w(uint8_t data);
	void pt_pos_select_w(uint8_t data);
	void pt_horz_w(uint8_t data);
	void pt_pic_w(uint8_t data);
	void ship_h_w(uint8_t data);
	void torpedo_pic_w(uint8_t data);
	void ship_h_precess_w(uint8_t data);
	void ship_pic_w(uint8_t data);
	void torpedo_h_w(uint8_t data);
	void torpedo_v_w(uint8_t data);
	void word_w(uint8_t data);
	void start_speech_w(uint8_t data);
};


void wolfpack_state::wolfpack_palette(palette_device &palette) const
{
	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_indirect_color(1, rgb_t(0xc1, 0xc1, 0xc1));
	palette.set_indirect_color(2, rgb_t(0x81, 0x81, 0x81));
	palette.set_indirect_color(3, rgb_t(0x48, 0x48, 0x48));

	for (int i = 0; i < 4; i++)
	{
		rgb_t const color = palette.indirect_color(i);
		palette.set_indirect_color(
				4 + i,
				rgb_t(
					(color.r() < 0xb8) ? (color.r() + 0x48) : 0xff,
					(color.g() < 0xb8) ? (color.g() + 0x48) : 0xff,
					(color.b() < 0xb8) ? (color.b() + 0x48) : 0xff));
	}

	palette.set_pen_indirect(0x00, 0);
	palette.set_pen_indirect(0x01, 1);
	palette.set_pen_indirect(0x02, 1);
	palette.set_pen_indirect(0x03, 0);
	palette.set_pen_indirect(0x04, 0);
	palette.set_pen_indirect(0x05, 2);
	palette.set_pen_indirect(0x06, 0);
	palette.set_pen_indirect(0x07, 3);
	palette.set_pen_indirect(0x08, 4);
	palette.set_pen_indirect(0x09, 5);
	palette.set_pen_indirect(0x0a, 6);
	palette.set_pen_indirect(0x0b, 7);
}


void wolfpack_state::ship_size_w(uint8_t data)
{
	m_ship_size = data;
}
void wolfpack_state::video_invert_w(uint8_t data)
{
	m_video_invert = data & 1;
}
void wolfpack_state::ship_reflect_w(uint8_t data)
{
	m_ship_reflect = data & 1;
}
void wolfpack_state::pt_pos_select_w(uint8_t data)
{
	m_pt_pos_select = data & 1;
}
void wolfpack_state::pt_horz_w(uint8_t data)
{
	m_pt_horz = data;
}
void wolfpack_state::pt_pic_w(uint8_t data)
{
	m_pt_pic = data & 0x3f;
}
void wolfpack_state::ship_h_w(uint8_t data)
{
	m_ship_h = data;
}
void wolfpack_state::torpedo_pic_w(uint8_t data)
{
	m_torpedo_pic = data;
}
void wolfpack_state::ship_h_precess_w(uint8_t data)
{
	m_ship_h_precess = data & 0x3f;
}
void wolfpack_state::ship_pic_w(uint8_t data)
{
	m_ship_pic = data & 0x0f;
}
void wolfpack_state::torpedo_h_w(uint8_t data)
{
	m_torpedo_h = data;
}
void wolfpack_state::torpedo_v_w(uint8_t data)
{
	m_torpedo_v = data;
}


void wolfpack_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_lfsr = std::make_unique<uint8_t []>(0x8000);
	for (uint16_t i = 0, val = 0; i < 0x8000; i++)
	{
		uint16_t const bit = ~(val ^ (val >> 14)) & 1;
		val = (val << 1) | bit;
		m_lfsr[i] = (val & 0x0c00) == 0x0c00;
	}

	m_current_index = 0x80;

	save_item(NAME(m_collision));
	save_item(NAME(m_current_index));
	save_item(NAME(m_video_invert));
	save_item(NAME(m_ship_reflect));
	save_item(NAME(m_pt_pos_select));
	save_item(NAME(m_pt_horz));
	save_item(NAME(m_pt_pic));
	save_item(NAME(m_ship_h));
	save_item(NAME(m_torpedo_pic));
	save_item(NAME(m_ship_size));
	save_item(NAME(m_ship_h_precess));
	save_item(NAME(m_ship_pic));
	save_item(NAME(m_torpedo_h));
	save_item(NAME(m_torpedo_v));
}


void wolfpack_state::draw_ship(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const uint32_t scaler[] =
	{
		0x00000, 0x00500, 0x00a00, 0x01000,
		0x01000, 0x01200, 0x01500, 0x01800,
		0x01800, 0x01d00, 0x02200, 0x02800,
		0x02800, 0x02800, 0x02800, 0x02800,
		0x02800, 0x03000, 0x03800, 0x04000,
		0x04000, 0x04500, 0x04a00, 0x05000,
		0x05000, 0x05500, 0x05a00, 0x06000,
		0x06000, 0x06a00, 0x07500, 0x08000,
		0x08000, 0x08a00, 0x09500, 0x0a000,
		0x0a000, 0x0b000, 0x0c000, 0x0d000,
		0x0d000, 0x0e000, 0x0f000, 0x10000,
		0x10000, 0x11a00, 0x13500, 0x15000,
		0x15000, 0x17500, 0x19a00, 0x1c000,
		0x1c000, 0x1ea00, 0x21500, 0x24000,
		0x24000, 0x26a00, 0x29500, 0x2c000,
		0x2c000, 0x2fa00, 0x33500, 0x37000
	};

	int const chop = (scaler[m_ship_size >> 2] * m_ship_h_precess) >> 16;


		m_gfxdecode->gfx(1)->zoom_transpen(bitmap, cliprect,
		m_ship_pic,
		0,
		m_ship_reflect, 0,
		2 * (m_ship_h - chop),
		128,
		2 * scaler[m_ship_size >> 2], scaler[m_ship_size >> 2], 0);
}


void wolfpack_state::draw_torpedo(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
		m_torpedo_pic,
		0,
		0, 0,
		2 * (244 - m_torpedo_h),
		224 - m_torpedo_v, 0);

	for (int y = 16; y < 224 - m_torpedo_v; y++)
	{
		if (y % 16 == 1)
			count = (count - 1) & 7;

		int const x1 = 248 - m_torpedo_h - count;
		int const x2 = 248 - m_torpedo_h + count;

		for (int x = 2 * x1; x < 2 * x2; x++)
			if (m_lfsr[(m_current_index + 0x300 * y + x) % 0x8000])
				bitmap.pix(y, x) = 1;
	}
}


void wolfpack_state::draw_pt(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle rect(cliprect);
	rect.setx(
			(m_pt_pic & 0x20) ? rect.left() : 256,
			(m_pt_pic & 0x10) ? rect.right() : 255);

	m_gfxdecode->gfx(2)->transpen(bitmap, rect,
		m_pt_pic,
		0,
		0, 0,
		2 * m_pt_horz,
		m_pt_pos_select ? 0x70 : 0xa0, 0);


	m_gfxdecode->gfx(2)->transpen(bitmap, rect,
		m_pt_pic,
		0,
		0, 0,
		2 * m_pt_horz - 512,
		m_pt_pos_select ? 0x70 : 0xa0, 0);
}


void wolfpack_state::draw_water(palette_device &palette, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= (std::min)(cliprect.bottom(), 127); y++)
	{
		uint16_t *const p = &bitmap.pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
			p[x] = palette.pen_indirect(p[x]) | 0x08;
	}
}


uint32_t wolfpack_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t color = 0x48;
	if (m_ship_size & 0x10) color += 0x13;
	if (m_ship_size & 0x20) color += 0x22;
	if (m_ship_size & 0x40) color += 0x3a;
	if (m_ship_size & 0x80) color += 0x48;

	m_palette->set_indirect_color(3, rgb_t(color,color,color));
	m_palette->set_indirect_color(7, rgb_t(color < 0xb8 ? color + 0x48 : 0xff,
																			color < 0xb8 ? color + 0x48 : 0xff,
																			color < 0xb8 ? color + 0x48 : 0xff));

	bitmap.fill(m_video_invert, cliprect);

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 32; j++)
		{
			int const code = m_alpha_num_ram[32 * i + j];


				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
				code,
				m_video_invert,
				0, 0,
				16 * j,
				192 + 8 * i);
		}

	draw_pt(bitmap, cliprect);
	draw_ship(bitmap, cliprect);
	draw_torpedo(bitmap, cliprect);
	draw_water(*m_palette, bitmap, cliprect);
	return 0;
}


void wolfpack_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_helper.fill(0);

		draw_ship(m_helper, m_helper.cliprect());

		for (int y = 128; y < 224 - m_torpedo_v; y++)
		{
			int x1 = 248 - m_torpedo_h - 1;
			int x2 = 248 - m_torpedo_h + 1;

			for (int x = 2 * x1; x < 2 * x2; x++)
			{
				if (x < 0 || x >= m_helper.width())
					continue;
				if (y < 0 || y >= m_helper.height())
					continue;

				if (m_helper.pix(y, x))
					m_collision = 1;
			}
		}

		m_current_index += 0x300 * 262;
	}
}


TIMER_CALLBACK_MEMBER(wolfpack_state::periodic_callback)
{
	int scanline = param;

	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	scanline += 64;

	if (scanline >= 262)
		scanline = 0;

	m_periodic_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

void wolfpack_state::machine_start()
{
	m_led.resolve();
	m_periodic_timer = timer_alloc(FUNC(wolfpack_state::periodic_callback), this);
}

void wolfpack_state::machine_reset()
{
	m_periodic_timer->adjust(m_screen->time_until_pos(0));
}


template <int Bit>
int wolfpack_state::dial_r()
{
	return ((m_dial->read() + Bit) / 2) & 0x01;
}


uint8_t wolfpack_state::misc_r()
{
	uint8_t val = 0;

	// BIT0 => SPEECH BUSY
	// BIT1 => COMP SIREN
	// BIT2 => SPARE
	// BIT3 => SPARE
	// BIT4 => COL DETECT
	// BIT5 => UNUSED
	// BIT6 => UNUSED
	// BIT7 => VBLANK

	if (!m_s14001a->busy_r())
		val |= 0x01;

	if (!m_collision)
		val |= 0x10;

	if (m_screen->vpos() >= 240)
		val |= 0x80;

	return val;
}


void wolfpack_state::high_explo_w(uint8_t data){}
void wolfpack_state::sonar_ping_w(uint8_t data){}
void wolfpack_state::sirlat_w(uint8_t data){}
void wolfpack_state::pt_sound_w(uint8_t data){}
void wolfpack_state::launch_torpedo_w(uint8_t data){}
void wolfpack_state::low_explo_w(uint8_t data){}
void wolfpack_state::screw_cont_w(uint8_t data){}
void wolfpack_state::lamp_flash_w(uint8_t data){}
void wolfpack_state::warning_light_w(uint8_t data){}
void wolfpack_state::audamp_w(uint8_t data){}

void wolfpack_state::word_w(uint8_t data)
{
	// latch word from bus into temp register, and place on s14001a input bus
	// there is no real need for a temp register at all, since the bus 'register' acts as one
	m_s14001a->data_w(data & 0x1f); // SA0 (IN5) is pulled low according to the schematic, so it's 0x1f and not 0x3f as one would expect
}

void wolfpack_state::start_speech_w(uint8_t data)
{
	m_s14001a->start_w(data & 1);
}


void wolfpack_state::attract_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_global_w(!(data & 1));
}


void wolfpack_state::credit_w(uint8_t data)
{
	m_led = BIT(~data, 0);
}


void wolfpack_state::coldetres_w(uint8_t data)
{
	m_collision = 0;
}


void wolfpack_state::main_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0x100);
	map(0x1000, 0x1000).portr("INPUTS");
	map(0x1000, 0x10ff).writeonly().share(m_alpha_num_ram);
	map(0x2000, 0x2000).r(FUNC(wolfpack_state::misc_r));
	map(0x2000, 0x2000).w(FUNC(wolfpack_state::high_explo_w));
	map(0x2001, 0x2001).w(FUNC(wolfpack_state::sonar_ping_w));
	map(0x2002, 0x2002).w(FUNC(wolfpack_state::sirlat_w));
	map(0x2003, 0x2003).w(FUNC(wolfpack_state::pt_sound_w));
	map(0x2004, 0x2004).w(FUNC(wolfpack_state::start_speech_w));
	map(0x2005, 0x2005).w(FUNC(wolfpack_state::launch_torpedo_w));
	map(0x2006, 0x2006).w(FUNC(wolfpack_state::low_explo_w));
	map(0x2007, 0x2007).w(FUNC(wolfpack_state::screw_cont_w));
	map(0x2008, 0x2008).w(FUNC(wolfpack_state::video_invert_w));
	map(0x2009, 0x2009).w(FUNC(wolfpack_state::ship_reflect_w));
	map(0x200a, 0x200a).w(FUNC(wolfpack_state::lamp_flash_w));
	map(0x200c, 0x200c).w(FUNC(wolfpack_state::credit_w));
	map(0x200d, 0x200d).w(FUNC(wolfpack_state::attract_w));
	map(0x200e, 0x200e).w(FUNC(wolfpack_state::pt_pos_select_w));
	map(0x200f, 0x200f).w(FUNC(wolfpack_state::warning_light_w));
	map(0x3000, 0x3000).portr("DSW");
	map(0x3000, 0x3000).w(FUNC(wolfpack_state::audamp_w));
	map(0x3001, 0x3001).w(FUNC(wolfpack_state::pt_horz_w));
	map(0x3003, 0x3003).w(FUNC(wolfpack_state::pt_pic_w));
	map(0x3004, 0x3004).w(FUNC(wolfpack_state::word_w));
	map(0x3007, 0x3007).w(FUNC(wolfpack_state::coldetres_w));
	map(0x4000, 0x4000).w(FUNC(wolfpack_state::ship_h_w));
	map(0x4001, 0x4001).w(FUNC(wolfpack_state::torpedo_pic_w));
	map(0x4002, 0x4002).w(FUNC(wolfpack_state::ship_size_w));
	map(0x4003, 0x4003).w(FUNC(wolfpack_state::ship_h_precess_w));
	map(0x4004, 0x4004).w(FUNC(wolfpack_state::ship_pic_w));
	map(0x4005, 0x4005).w(FUNC(wolfpack_state::torpedo_h_w));
	map(0x4006, 0x4006).w(FUNC(wolfpack_state::torpedo_v_w));
	map(0x5000, 0x5fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x7000, 0x7fff).rom();
	map(0x9000, 0x9000).nopr(); // debugger ROM location?
	map(0xf000, 0xffff).rom();
}


static INPUT_PORTS_START( wolfpack )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(wolfpack_state, dial_r<0>)    // dial connects here
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(wolfpack_state, dial_r<1>)    // dial connects here
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_SERVICE( 0x10, IP_ACTIVE_HIGH )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x08, "Play Time" )
	PORT_DIPSETTING(    0x00, "65 seconds" )
	PORT_DIPSETTING(    0x08, "97 seconds" )
	PORT_DIPSETTING(    0x10, "130 seconds" )
	PORT_DIPSETTING(    0x18, "160 seconds" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) // demo sound?
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, "Score for Extended Play" )
	PORT_DIPSETTING(    0x00, "8000" )
	PORT_DIPSETTING(    0x40, "12000" )
	PORT_DIPSETTING(    0x80, "16000" )
	PORT_DIPSETTING(    0xc0, "20000" )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(5)
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	16, 8,
	64,
	1,
	{ 0 },
	{
		0x7, 0x7, 0x6, 0x6, 0x5, 0x5, 0x4, 0x4,
		0xf, 0xf, 0xe, 0xe, 0xd, 0xd, 0xc, 0xc
	},
	{
		0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00
	},
	0x80
};

static const uint32_t ship_layout_xoffset[64] =
{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3f,
		0x44, 0x45, 0x46, 0x47, 0x4c, 0x4d, 0x4e, 0x4f,
		0x54, 0x55, 0x56, 0x57, 0x5c, 0x5d, 0x5e, 0x5f,
		0x64, 0x65, 0x66, 0x67, 0x6c, 0x6d, 0x6e, 0x6f,
		0x74, 0x75, 0x76, 0x77, 0x7c, 0x7d, 0x7e, 0x7f
};

static const gfx_layout ship_layout =
{
	64, 16,
	16,
	1,
	{ 0 },
	EXTENDED_XOFFS,
	{
			0x780, 0x700, 0x680, 0x600, 0x580, 0x500, 0x480, 0x400,
			0x380, 0x300, 0x280, 0x200, 0x180, 0x100, 0x080, 0x000
	},
	0x800,
	ship_layout_xoffset,
	nullptr
};

static const uint32_t pt_layout_xoffset[64] =
	{
		0x3f, 0x3f, 0x3e, 0x3e, 0x3d, 0x3d, 0x3c, 0x3c,
		0x37, 0x37, 0x36, 0x36, 0x35, 0x35, 0x34, 0x34,
		0x2f, 0x2f, 0x2e, 0x2e, 0x2d, 0x2d, 0x2c, 0x2c,
		0x27, 0x27, 0x26, 0x26, 0x25, 0x25, 0x24, 0x24,
		0x1f, 0x1f, 0x1e, 0x1e, 0x1d, 0x1d, 0x1c, 0x1c,
		0x17, 0x17, 0x16, 0x16, 0x15, 0x15, 0x14, 0x14,
		0x0f, 0x0f, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c,
		0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04
	};

static const gfx_layout pt_layout =
{
	64, 8,
	16,
	1,
	{ 0 },
	EXTENDED_XOFFS,
	{ 0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0 },
	0x200,
	pt_layout_xoffset,
	nullptr
};


static const gfx_layout torpedo_layout =
{
	16, 32,
	16,
	1,
	{ 0 },
	{
		0x4, 0x4, 0x5, 0x5, 0x6, 0x6, 0x7, 0x7,
		0xc, 0xc, 0xd, 0xd, 0xe, 0xe, 0xf, 0xf
	},
	{
		0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
		0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0,
		0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170,
		0x180, 0x190, 0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0, 0x1f0
	},
	0x0200
};


static GFXDECODE_START( gfx_wolfpack )
	GFXDECODE_ENTRY( "tile",    0, tile_layout,    0, 2 )
	GFXDECODE_ENTRY( "ship",    0, ship_layout,    6, 1 )
	GFXDECODE_ENTRY( "pt",      0, pt_layout,      0, 1 )
	GFXDECODE_ENTRY( "torpedo", 0, torpedo_layout, 4, 1 )
GFXDECODE_END


void wolfpack_state::wolfpack(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12'096'000 / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &wolfpack_state::main_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(512, 262);
	m_screen->set_visarea(0, 511, 16, 239);
	m_screen->set_screen_update(FUNC(wolfpack_state::screen_update));
	m_screen->screen_vblank().set(FUNC(wolfpack_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wolfpack);
	PALETTE(config, m_palette, FUNC(wolfpack_state::wolfpack_palette), 12, 8);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	S14001A(config, m_s14001a, 20'000); // RC Clock (C=100pf, R=470K-670K ohms, adjustable) ranging from 14925.37313hz to 21276.59574hz, likely factory set to 20000hz since anything below 19500 is too slow
	m_s14001a->add_route(ALL_OUTPUTS, "mono", 1.00);
}


ROM_START( wolfpack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "30285.e3", 0x7000, 0x0800, CRC(b4d30b33) SHA1(46645c227828632b57244bdccad455e1831b5273) )
	ROM_RELOAD       (             0xf000, 0x0800 )
	ROM_LOAD_NIB_HIGH( "30287.g3", 0x7000, 0x0800, CRC(c6300dc9) SHA1(6a0ec0bfa6ad4c870aa6f21bfde094da6975b58b) )
	ROM_RELOAD       (             0xf000, 0x0800 )
	ROM_LOAD_NIB_LOW ( "30286.f3", 0x7800, 0x0800, CRC(17dce9e8) SHA1(9c7bac1aa676548dc7908f1518efd58c72645ab7) )
	ROM_RELOAD       (             0xf800, 0x0800 )
	ROM_LOAD_NIB_HIGH( "30288.h3", 0x7800, 0x0800, CRC(b80ab7b6) SHA1(f2ede98ac5337064499ae2262a8a81f83505bd66) )
	ROM_RELOAD       (             0xf800, 0x0800 )

	ROM_REGION( 0x0400, "tile", 0 )
	ROM_LOAD( "30291.c1", 0x0000, 0x0400, CRC(7e3d22cf) SHA1(92e6bbe049dc8fcd674f2ff96cde3786f714508d) )

	ROM_REGION( 0x1000, "ship", 0 )
	ROM_LOAD( "30289.j6", 0x0000, 0x0800, CRC(f63e5629) SHA1(d64f19fc62060d395df5bb8663a7694a23b0aa2e) )
	ROM_LOAD( "30290.k6", 0x0800, 0x0800, CRC(70d5430e) SHA1(d512fc3bb0cf0816a1c987f7188c4b331303347f) )

	ROM_REGION( 0x0400, "pt", 0 )
	ROM_LOAD( "30294.p4", 0x0000, 0x0400, CRC(ea93f4b9) SHA1(48b4e0136f5349eb53fea7127a969d87457d70f9) )

	ROM_REGION( 0x0400, "torpedo", 0 )
	ROM_LOAD( "30293.m6", 0x0000, 0x0400, CRC(11900d47) SHA1(2dcb3c3488a5e9ed7f1751649f8dc25696f0f57a) )

	ROM_REGION( 0x0800, "speech", 0 )
	ROM_LOAD_NIB_LOW ( "30863.r1", 0x0000, 0x0800, CRC(3f779f13) SHA1(8ed8a1bf680e8277066416f467388e3875e8cbbd) )
	ROM_LOAD_NIB_HIGH( "30864.r3", 0x0000, 0x0800, CRC(c4a58d1d) SHA1(a2ba9354b99c739bbfa94458d671c109be163ca0) )
ROM_END

} // anonymous namespace


GAME( 1978, wolfpack, 0, wolfpack, wolfpack, wolfpack_state, empty_init, ORIENTATION_FLIP_Y, "Atari", "Wolf Pack (prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
