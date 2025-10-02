// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Driver for Epoch Super Cassette Vision

TODO:
- verify video clock in European units.
- verify raw video parameters.

***************************************************************************/

#include "emu.h"
#include "cpu/upd177x/upd177x.h"
#include "cpu/upd7810/upd7810.h"
#include "bus/scv/slot.h"
#include "bus/scv/rom.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class scv_state : public driver_device
{
public:
	scv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this,"videoram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_upd1771c(*this, "upd1771c"),
		m_cart(*this, "cartslot"),
		m_pa(*this, "PA.%u", 0),
		m_pc0(*this, "PC0"),
		m_charrom(*this, "charrom")
	{ }

	void scv(machine_config &config);
	void scv_pal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vblank_update);
	TIMER_CALLBACK_MEMBER(pa_w_update);

private:
	void scv_palette(palette_device &palette) const;
	u32 screen_update_scv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_sprite_part(bitmap_ind16 &bitmap, u8 x, u8 y, u8 pat, u8 col);
	void draw_sprite(bitmap_ind16 &bitmap, u8 x, u8 y, u8 tile_idx, u8 col, bool left, bool right, bool top, bool bottom, u8 clip_y);
	void draw_sprites(bitmap_ind16 &bitmap);
	void draw_text(bitmap_ind16 &bitmap, u8 x, u8 y, u8 *char_data, u8 fg, uint8_t bg);
	void draw_semi_graph(bitmap_ind16 &bitmap, u8 x, u8 y, u8 data, u8 fg);
	void draw_block_graph(bitmap_ind16 &bitmap, u8 x, u8 y, u8 col);

	void scv_mem(address_map &map) ATTR_COLD;

	static const u8 s_spr_2col_lut0[16];
	static const u8 s_spr_2col_lut1[16];

	u8 m_porta;
	u8 m_portc;
	emu_timer *m_vb_timer;

	required_shared_ptr<u8> m_videoram;
	required_device<upd7801_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<upd1771c_cpu_device> m_upd1771c;
	required_device<scv_cart_slot_device> m_cart;
	required_ioport_array<8> m_pa;
	required_ioport m_pc0;
	required_memory_region m_charrom;
	emu_timer *m_pa_w_timer;
};


const u8 scv_state::s_spr_2col_lut0[16] = {0, 15, 12, 13, 10, 11,  8, 9, 6, 7,  4,  5, 2, 3,  1,  1};
const u8 scv_state::s_spr_2col_lut1[16] = {0,  1,  8, 11,  2,  3, 10, 9, 4, 5, 12, 13, 6, 7, 14, 15};


void scv_state::scv_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom();   // BIOS

	map(0x2000, 0x3403).ram().share(m_videoram);  // VRAM + 4 registers
	map(0x3600, 0x3600).lw8(NAME([this] (u8 data) {
		// With the current cores there are sync issues between writing and clearing PA in scv kungfurd when playing back adpcm.
		// During a reset an OUT PA gets executed just around the same time when the main cpu is writing to PA.
		// Since the OUT PA completes while the external write to PA is still in progress the external write to PA wins.
		m_pa_w_timer->adjust(m_upd1771c->cycles_to_attotime(8), data);
	}));

	// 8000 - ff7f - Cartridge
	// ff80 - ffff - CPU internal RAM
}


static INPUT_PORTS_START(scv)
	PORT_START("PA.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PA.1" )
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PA.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)

	PORT_START("PA.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("PA.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)

	PORT_START("PA.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("PA.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("PA.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cl") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("En") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("PC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_O)
INPUT_PORTS_END


void scv_state::scv_palette(palette_device &palette) const
{
	/*
	  SCV Epoch-1A chip RGB voltage readouts from paused BIOS color test:

	  (values in millivolts)

	        R   G   B
	  0    29  29 325
	  1    29  27  22
	  2    25  24 510
	  3   337  28 508
	  4    29 515  22
	  5   336 512 329
	  6    26 515 511
	  7    29 337  25
	  8   520  24  22
	  9   517 338  21
	  10  520  25 512
	  11  521 336 333
	  12  518 515  21
	  13  342 336  22
	  14  337 336 330
	  15  516 511 508

	  Only tree 'bins' of values are obviously captured
	   25 ish
	  330 ish
	  520 ish.

	  Quantizing/scaling/rounding between 0 and 255 we thus get:

	*/
	palette.set_pen_color( 0,   0,   0, 155);
	palette.set_pen_color( 1,   0,   0,   0);
	palette.set_pen_color( 2,   0,   0, 255);
	palette.set_pen_color( 3, 161,   0, 255);
	palette.set_pen_color( 4,   0, 255,   0);
	palette.set_pen_color( 5, 160, 255, 157);
	palette.set_pen_color( 6,   0, 255, 255);
	palette.set_pen_color( 7,   0, 161,   0);
	palette.set_pen_color( 8, 255,   0,   0);
	palette.set_pen_color( 9, 255, 161,   0);
	palette.set_pen_color(10, 255,   0, 255);
	palette.set_pen_color(11, 255, 160, 159);
	palette.set_pen_color(12, 255, 255,   0);
	palette.set_pen_color(13, 163, 160,   0);
	palette.set_pen_color(14, 161, 160, 157);
	palette.set_pen_color(15, 255, 255, 255);
}


TIMER_CALLBACK_MEMBER(scv_state::vblank_update)
{
	int vpos = m_screen->vpos();

	switch (vpos)
	{
	case 240:
		m_maincpu->set_input_line(UPD7810_INTF2, ASSERT_LINE);
		break;
	case 0:
		m_maincpu->set_input_line(UPD7810_INTF2, CLEAR_LINE);
		break;
	}

	m_vb_timer->adjust(m_screen->time_until_pos((vpos + 1) % 262, 0));
}


void scv_state::draw_sprite_part(bitmap_ind16 &bitmap, u8 x, u8 y, u8 pat, u8 col)
{
	if (x >= 4)
	{
		x -= 4;

		if (BIT(pat, 3))
			bitmap.pix(y + 2, x) = col;

		if (BIT(pat, 2) && x < 255)
			bitmap.pix(y + 2, x + 1) = col;

		if (BIT(pat, 1) && x < 254)
			bitmap.pix(y + 2, x + 2) = col;

		if (BIT(pat, 0) && x < 253)
			bitmap.pix(y + 2, x + 3) = col;
	}
}


void scv_state::draw_sprite(bitmap_ind16 &bitmap, u8 x, u8 y, u8 tile_idx, u8 col, bool left, bool right, bool top, bool bottom, u8 clip_y)
{
	y += clip_y * 2;
	for (int j = clip_y * 4; j < 32; j += 4, y += 2)
	{
		if ((top && (j < 16)) || (bottom && (j >= 16)))
		{
			if (left)
			{
				const u8 pat0 = m_videoram[tile_idx * 32 + j + 0];
				const u8 pat1 = m_videoram[tile_idx * 32 + j + 1];

				draw_sprite_part(bitmap, x     , y,     pat0 >> 4,   col);
				draw_sprite_part(bitmap, x +  4, y,     pat1 >> 4,   col);
				draw_sprite_part(bitmap, x     , y + 1, pat0 & 0x0f, col);
				draw_sprite_part(bitmap, x +  4, y + 1, pat1 & 0x0f, col);
			}
			if (right)
			{
				const u8 pat2 = m_videoram[tile_idx * 32 + j + 2];
				const u8 pat3 = m_videoram[tile_idx * 32 + j + 3];

				draw_sprite_part(bitmap, x +  8, y,     pat2 >> 4,   col);
				draw_sprite_part(bitmap, x + 12, y,     pat3 >> 4,   col);
				draw_sprite_part(bitmap, x +  8, y + 1, pat2 & 0x0f, col);
				draw_sprite_part(bitmap, x + 12, y + 1, pat3 & 0x0f, col);
			}
		}
	}
}


void scv_state::draw_sprites(bitmap_ind16 &bitmap)
{
	const u8 num_sprites = BIT(m_videoram[0x1400], 2) ? 64 : 128;

	for (int i = 0; i < num_sprites; i++)
	{
		u8 spr_y = m_videoram[0x1200 + i * 4] & 0xfe;
		u8 y_32 = BIT(m_videoram[0x1200 + i * 4], 0);       // Xx32 sprite
		u8 clip = m_videoram[0x1201 + i * 4] >> 4;
		u8 col = m_videoram[0x1201 + i * 4] & 0x0f;
		u8 spr_x = m_videoram[0x1202 + i * 4] & 0xfe;
		u8 x_32 = BIT(m_videoram[0x1202 + i * 4], 0);       // 32xX sprite
		u8 tile_idx = m_videoram[0x1203 + i * 4] & 0x7f;
		const bool half = BIT(m_videoram[0x1203 + i * 4], 7);
		bool left = true;
		bool right = true;
		bool top = true;
		bool bottom = true;

		if (!col || !spr_y)
			continue;

		if (half)
		{
			if (BIT(tile_idx, 6))
			{
				if (y_32)
				{
					spr_y -= 8;
					top = false;
					y_32 = 0;
				}
				else
					bottom = false;
			}
			if (x_32)
			{
				spr_x -= 8;
				left = false;
				x_32 = 0;
			}
			else
				right = false;
		}

		if (BIT(m_videoram[0x1400], 5) && BIT(i, 5))
		{
			// 2 color sprite handling
			draw_sprite(bitmap, spr_x, spr_y, tile_idx, col, left, right, top, bottom, clip);
			if (x_32 || y_32)
			{
				col = BIT(i, 4) ? s_spr_2col_lut1[col] : s_spr_2col_lut0[col];
				tile_idx ^= (8 * x_32 + y_32);

				draw_sprite(bitmap, spr_x, spr_y, tile_idx, col, left, right, top, bottom, clip);
			}
		}
		else
		{
			// regular sprite handling
			draw_sprite(bitmap, spr_x, spr_y, tile_idx, col, left, right, top, bottom, clip);
			if (x_32)
				draw_sprite(bitmap, spr_x + 16, spr_y, tile_idx | 8, col, 1, 1, top, bottom, clip);

			if (y_32)
			{
				clip = BIT(clip, 3) ? (clip & 0x07) : 0;
				draw_sprite(bitmap, spr_x, spr_y + 16, tile_idx | 1, col, left, right, 1, 1, clip);
				if (x_32)
					draw_sprite(bitmap, spr_x + 16, spr_y + 16, tile_idx | 9, col, 1, 1, 1, 1, clip);
			}
		}
	}
}


void scv_state::draw_text(bitmap_ind16 &bitmap, u8 x, u8 y, u8 *char_data, u8 fg, u8 bg)
{
	for (int i = 0; i < 8; i++)
	{
		const u8 d = char_data[i];

		for (int j = 0; j < 8; j++)
			bitmap.pix(y + i, x + j) = (BIT(d, 7 - j)) ? fg : bg;
	}

	for (int i = 8; i < 16; i++)
		for (int j = 0; j < 8; j++)
			bitmap.pix(y + i, x + j) = bg;
}


void scv_state::draw_semi_graph(bitmap_ind16 &bitmap, u8 x, u8 y, u8 data, u8 fg)
{
	if (!data)
		return;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			bitmap.pix(y + i, x + j) = fg;
}


void scv_state::draw_block_graph(bitmap_ind16 &bitmap, u8 x, u8 y, u8 col)
{
	if (!col)
		return;

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			bitmap.pix(y + i, x + j) = col;
}


u32 scv_state::screen_update_scv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 fg = m_videoram[0x1403] >> 4;
	const u8 bg = m_videoram[0x1403] & 0x0f;
	const u8 gr_fg = m_videoram[0x1401] >> 4;
	const u8 gr_bg = m_videoram[0x1401] & 0x0f;
	const int clip_x = (m_videoram[0x1402] & 0x0f) * 2;
	const int clip_y = m_videoram[0x1402] >> 4;

	// Clear the screen
	bitmap.fill(gr_bg, cliprect);

	// Draw background
	for (int y = 0; y < 16; y++)
	{
		const bool text_y = (y < clip_y) ? !BIT(m_videoram[0x1400], 7) : BIT(m_videoram[0x1400], 7);

		for (int x = 0; x < 32; x++)
		{
			const bool text_x = (x < clip_x) ? !BIT(m_videoram[0x1400], 6) : BIT(m_videoram[0x1400], 6);
			const u8 d = m_videoram[0x1000 + y * 32 + x];

			if (text_x && text_y)
			{
				// Text mode
				uint8_t *char_data = m_charrom->base() + (d & 0x7f) * 8;
				draw_text(bitmap, x * 8, y * 16, char_data, fg, bg);
			}
			else
			{
				switch (m_videoram[0x1400] & 0x03)
				{
				case 0x01:      // Semi graphics mode
					draw_semi_graph(bitmap, x * 8    , y * 16     , d & 0x80, gr_fg);
					draw_semi_graph(bitmap, x * 8 + 4, y * 16     , d & 0x40, gr_fg);
					draw_semi_graph(bitmap, x * 8    , y * 16 +  4, d & 0x20, gr_fg);
					draw_semi_graph(bitmap, x * 8 + 4, y * 16 +  4, d & 0x10, gr_fg);
					draw_semi_graph(bitmap, x * 8    , y * 16 +  8, d & 0x08, gr_fg);
					draw_semi_graph(bitmap, x * 8 + 4, y * 16 +  8, d & 0x04, gr_fg);
					draw_semi_graph(bitmap, x * 8    , y * 16 + 12, d & 0x02, gr_fg);
					draw_semi_graph(bitmap, x * 8 + 4, y * 16 + 12, d & 0x01, gr_fg);
					break;

				case 0x03:      // Block graphics mode
					draw_block_graph(bitmap, x * 8, y * 16    , d >> 4);
					draw_block_graph(bitmap, x * 8, y * 16 + 8, d & 0x0f);
					break;

				default:        // Otherwise draw nothing?
					break;
				}
			}
		}
	}

	if (BIT(m_videoram[0x1400], 4))
		draw_sprites(bitmap);

	return 0;
}


TIMER_CALLBACK_MEMBER(scv_state::pa_w_update)
{
	m_upd1771c->pa_w(param);
}


void scv_state::machine_start()
{
	m_vb_timer = timer_alloc(FUNC(scv_state::vblank_update), this);
	m_pa_w_timer = timer_alloc(FUNC(scv_state::pa_w_update), this);

	save_item(NAME(m_porta));
	save_item(NAME(m_portc));
}


void scv_state::machine_reset()
{
	m_vb_timer->adjust(m_screen->time_until_pos(0, 0));
}


// F4 Character Displayer
static const gfx_layout scv_charlayout =
{
	8, 8,               // 8 x 8 characters
	128,                // 128 characters
	1,                  // 1 bits per pixel
	{0},                // no bitplanes
	// x offsets
	{0, 1, 2, 3, 4, 5, 6, 7},
	// y offsets
	{0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                 // every char takes 8 bytes
};

static GFXDECODE_START(gfx_scv)
	GFXDECODE_ENTRY("charrom", 0x0000, scv_charlayout, 0, 8)
GFXDECODE_END


static void scv_cart(device_slot_interface &device)
{
	device.option_add_internal("rom8k",       SCV_ROM8K);
	device.option_add_internal("rom16k",      SCV_ROM16K);
	device.option_add_internal("rom32k",      SCV_ROM32K);
	device.option_add_internal("rom32k_ram",  SCV_ROM32K_RAM8K);
	device.option_add_internal("rom64k",      SCV_ROM64K);
	device.option_add_internal("rom128k",     SCV_ROM128K);
	device.option_add_internal("rom128k_ram", SCV_ROM128K_RAM4K);
}

void scv_state::scv(machine_config &config)
{
	UPD7801(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &scv_state::scv_mem);
	m_maincpu->pa_out_cb().set([this] (u8 data) { m_porta = data; });
	m_maincpu->pb_in_cb().set([this] () {
		u8 data = 0xff;

		for (int i = 0; i < 8; i++)
			if (!BIT(m_porta, i))
				data &= m_pa[i]->read();

		return data;
	});
	m_maincpu->pc_in_cb().set([this] () {
		return BIT(m_pc0->read(), 0);
	});
	m_maincpu->pc_out_cb().set([this] (u8 data) {
		m_portc = data;
		m_cart->write_bank(m_portc); // Only bits 5 & 6 are exposed to the cartridge?
		m_upd1771c->set_input_line(INPUT_LINE_RESET, BIT(m_portc, 3) ? CLEAR_LINE : ASSERT_LINE);
	});
	config.set_perfect_quantum(m_maincpu);

	// Video chip is EPOCH TV-1
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(14'318'181)/2, 456, 24, 24+192, 262, 23, 23+222);  // Clock verified. TODO: Verify rest of the parameters
	m_screen->set_screen_update(FUNC(scv_state::screen_update_scv));
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_scv);
	PALETTE(config, "palette", FUNC(scv_state::scv_palette), 16);

	// Sound is generated by UPD1771C clocked at XTAL(6'000'000)
	SPEAKER(config, "mono").front_center();
	UPD1771C(config, m_upd1771c, 6_MHz_XTAL);
	m_upd1771c->pb_out_cb().set([this] (u8 data) { m_maincpu->set_input_line(UPD7810_INTF1, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE); });
	m_upd1771c->add_route(ALL_OUTPUTS, "mono", 0.70);

	SCV_CART_SLOT(config, m_cart, scv_cart, nullptr);
	m_cart->set_address_space(m_maincpu, AS_PROGRAM);

	// Software lists
	SOFTWARE_LIST(config, "cart_list").set_original("scv");
}


void scv_state::scv_pal(machine_config &config)
{
	scv(config);

	// Video chip is EPOCH TV-1A
	m_screen->set_raw(13.4_MHz_XTAL/2, 456, 24, 24+192, 294, 23, 23+222);     // TODO: Verify clock and video parameters
}


// The same bios is used in both the NTSC and PAL versions of the console
ROM_START(scv)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7801g.s01", 0, 0x1000, CRC(7ac06182) SHA1(6e89d1227581c76441a53d605f9e324185f1da33))

	ROM_REGION(0x400, "charrom", 0)
	ROM_LOAD("epochtv.chr.s02", 0, 0x400, BAD_DUMP CRC(db521533) SHA1(40b4e44838c35191f115437a14f200f052e71509))

	ROM_REGION(0x400, "upd1771c", 0)
	ROM_LOAD16_WORD("upd1771c-017.s03", 0, 0x400, CRC(975bd68d) SHA1(e777217c622331677ac1d6520a741d48ad9133c0))
ROM_END


ROM_START(scv_pal)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7801g.s01", 0, 0x1000, CRC(7ac06182) SHA1(6e89d1227581c76441a53d605f9e324185f1da33))

	ROM_REGION(0x400, "charrom", 0)
	ROM_LOAD("epochtv.chr.s02", 0, 0x400, BAD_DUMP CRC(db521533) SHA1(40b4e44838c35191f115437a14f200f052e71509))

	ROM_REGION(0x400, "upd1771c", 0)
	ROM_LOAD16_WORD("upd1771c-017.s03", 0, 0x400, CRC(975bd68d) SHA1(e777217c622331677ac1d6520a741d48ad9133c0))
ROM_END

} // anonymous namespace


/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME                       FLAGS */
CONS(1984, scv,     0,      0,      scv,     scv,   scv_state, empty_init, "Epoch", "Super Cassette Vision",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(198?, scv_pal, scv,    0,      scv_pal, scv,   scv_state, empty_init, "Yeno",  "Super Cassette Vision (PAL)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
