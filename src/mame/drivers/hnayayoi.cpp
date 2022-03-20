// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/****************************************************************************

Some Dynax games using the first version of their blitter

driver by Nicola Salmoria, blitter support based on work by Luca Elia

CPU:    Z80-A
Sound:  YM2203C
        M5205
OSC:    20.0000MHz
Video:  HD46505SP

---------------------------------------
Year + Game                 Board
---------------------------------------
87 Hana Yayoi               D0208298L1
87 Hana Fubuki              D0602048
87 Untouchable              D0806298
---------------------------------------

Notes:
- In service mode, press "analyzer" (0) and "test" (F2) to see a gfx test

- hnfubuki doesn't have a service mode dip, press "analyzer" instead

- untoucha doesn't have it either; press "test" during boot for one kind
  of service menu, "analyzer" at any other time for another menu (including
  dip switch settings)
  Note: screen asking to press test during boot does not show if machine contains credits.

TODO:
- dips/inputs for some games
- untoucha: player high scores are lost when resetting
- hnayayoi: with the correct clocks the game seems to run a voice test at start up that didn't
  happen with the Z80 wrongly clocked at 5 MHz. Halving the 'nmiclock' restores previous behaviour.
  Verify on PCB what's the correct behaviour.

15-July 2016 - DIPs added to untoucha [theguru]

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_IRQ     (1U <<  1)
#define LOG_BLITTER (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_IRQ | LOG_BLITTER)

#include "logmacro.h"

#define LOGIRQ(...)     LOGMASKED(LOG_IRQ,     __VA_ARGS__)
#define LOGBLITTER(...) LOGMASKED(LOG_BLITTER, __VA_ARGS__)


namespace {

class hnayayoi_state : public driver_device
{
public:
	hnayayoi_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette"),
		m_blitrom(*this, "blitter"),
		m_key{ { *this, "P1_KEY%u", 0U }, { *this, "P2_KEY%u", 0U } }
	{ }

	void hnayayoi(machine_config &config);
	void hnfubuki(machine_config &config);
	void untoucha(machine_config &config);

	void init_hnfubuki();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// video-related
	std::unique_ptr<uint8_t[]> m_pixmap[8];
	uint16_t m_palbank;
	uint8_t m_blit_layer;
	uint16_t m_blit_dest;
	uint32_t m_blit_src;

	// misc
	uint8_t m_keyb;
	bool m_nmi_enable;

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	required_device<msm5205_device> m_msm;
	required_device<palette_device> m_palette;

	required_region_ptr<uint8_t> m_blitrom;

	required_ioport_array<5> m_key[2];

	template <uint8_t Which> uint8_t keyboard_r();
	void keyboard_w(uint8_t data);
	void dynax_blitter_rev1_param_w(offs_t offset, uint8_t data);
	void dynax_blitter_rev1_start_w(uint8_t data);
	void dynax_blitter_rev1_clear_w(uint8_t data);
	void palbank_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_clock_w);
	DECLARE_VIDEO_START(untoucha);
	MC6845_UPDATE_ROW(hnayayoi_update_row);
	MC6845_UPDATE_ROW(untoucha_update_row);
	void common_vh_start(int num_pixmaps);
	void copy_pixel(int x, int y, int pen);
	void draw_layer_interleaved(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t row, uint16_t y, uint8_t x_count, int left_pixmap, int right_pixmap, int palbase, bool transp);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);

	void hnayayoi_map(address_map &map);
	void hnayayoi_io_map(address_map &map);
	void hnfubuki_map(address_map &map);
	void untoucha_map(address_map &map);
	void untoucha_io_map(address_map &map);
};


// video

/***************************************************************************

First version of the Dynax blitter.

Can handle up to 8 256x256 bitmaps; in the games supported, every pair of
bitmaps is interleaved horizontally to form 4 higher res 512x256 layer.

The blitter reads compressed data from ROM and copies it to the bitmap RAM.

***************************************************************************/

void hnayayoi_state::common_vh_start(int num_pixmaps)
{
	for (int i = 0; i < num_pixmaps; i++)
	{
		m_pixmap[i] = make_unique_clear<uint8_t[]>(256 * 256);
		save_pointer(NAME(m_pixmap[i]), 256 * 256, i);
	}
}

void hnayayoi_state::video_start()
{
	common_vh_start(4);  // 4 bitmaps -> 2 layers
}

VIDEO_START_MEMBER(hnayayoi_state, untoucha)
{
	common_vh_start(8);  // 8 bitmaps -> 4 layers
}



/***************************************************************************

Blitter support

three parameters:
blit_layer: mask of the bitmaps to write to (can write to multiple bitmaps
            at the same time)
blit_dest:  position in the destination bitmap where to start blitting
blit_src:   address of source data in the gfx ROM

additional parameters specify the palette base, but this is handled while rendering
the screen, not during blitting (games change the palette base without redrawing
the screen).

It is not known whether the palette base control registers are part of the blitter
hardware or latched somewhere else. Since they are mapped in memory immediately
before the bitter parameters, they probably are part of the blitter, but I'm
handling them separately anyway.


The format of the blitter data stored in ROM is very simple:

7654 ----   Pen to draw with
---- 3210   Command

Commands:

0       Stop
1-b     Draw 1-b pixels along X.
c       Followed by 1 byte (N): draw N pixels along X.
d       Followed by 2 bytes (X,N): move on the line to pixel (start+X), draw N pixels
        along X.
e       Followed by 1 byte (N): set blit_layer = N. Used to draw interleaved graphics
        with a single blitter run.
f       Move to next line.

At the end of the blit, blit_src is left pointing to the next data in the gfx ROM.
This is used to draw interleaved graphics with two blitter runs without having to set
up blit_src for the second call.

***************************************************************************/

void hnayayoi_state::dynax_blitter_rev1_param_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: m_blit_dest = (m_blit_dest & 0xff00) | (data << 0); break;
		case 1: m_blit_dest = (m_blit_dest & 0x00ff) | (data << 8); break;
		case 2: m_blit_layer = data; break;
		case 3: m_blit_src = (m_blit_src & 0xffff00) | (data << 0); break;
		case 4: m_blit_src = (m_blit_src & 0xff00ff) | (data << 8); break;
		case 5: m_blit_src = (m_blit_src & 0x00ffff) | (data <<16); break;
	}
}

void hnayayoi_state::copy_pixel(int x, int y, int pen)
{
	if (x >= 0 && x <= 255 && y >= 0 && y <= 255)
	{
		for (int i = 0; i < 8; i++)
		{
			if ((~m_blit_layer & (1 << i)) && (m_pixmap[i]))
				m_pixmap[i][256 * y + x] = pen;
		}
	}
}

void hnayayoi_state::dynax_blitter_rev1_start_w(uint8_t data)
{
	int romlen = m_blitrom.bytes();

	int sx = m_blit_dest & 0xff;
	int sy = m_blit_dest >> 8;
	int x = sx;
	int y = sy;
	while (m_blit_src < romlen)
	{
		int cmd = m_blitrom[m_blit_src] & 0x0f;
		int pen = m_blitrom[m_blit_src] >> 4;

		m_blit_src++;

		switch (cmd)
		{
			case 0xf:
				y++;
				x = sx;
				break;

			case 0xe:
				if (m_blit_src >= romlen)
				{
					LOGBLITTER("GFXROM OVER %06x", m_blit_src);
					return;
				}
				x = sx;
				m_blit_layer = m_blitrom[m_blit_src++];
				break;

			case 0xd:
				if (m_blit_src >= romlen)
				{
					LOGBLITTER("GFXROM OVER %06x", m_blit_src);
					return;
				}
				x = sx + m_blitrom[m_blit_src++];
				[[fallthrough]];
			case 0xc:
				if (m_blit_src >= romlen)
				{
					LOGBLITTER("GFXROM OVER %06x", m_blit_src);
					return;
				}
				cmd = m_blitrom[m_blit_src++];
				[[fallthrough]];
			case 0xb:
			case 0xa:
			case 0x9:
			case 0x8:
			case 0x7:
			case 0x6:
			case 0x5:
			case 0x4:
			case 0x3:
			case 0x2:
			case 0x1:
				while (cmd--)
					copy_pixel(x++, y, pen);
				break;

			case 0x0:
				return;
		}
	}

	LOGBLITTER("GFXROM OVER %06x", m_blit_src);
}

void hnayayoi_state::dynax_blitter_rev1_clear_w(uint8_t data)
{
	int pen = data >> 4;

	for (int i = 0; i < 8; i++)
	{
		if ((~m_blit_layer & (1 << i)) && (m_pixmap[i]))
			std::fill(&m_pixmap[i][m_blit_dest], &m_pixmap[i][0x10000], pen);
	}
}


void hnayayoi_state::palbank_w(offs_t offset, uint8_t data)
{
	offset *= 8;
	m_palbank = (m_palbank & (0xff00 >> offset)) | (data << offset);
}


void hnayayoi_state::draw_layer_interleaved(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t row, uint16_t y, uint8_t x_count, int left_pixmap, int right_pixmap, int palbase, bool transp)
{
	uint8_t *src1 = &m_pixmap[left_pixmap][(row & 255) * 256];
	uint8_t *src2 = &m_pixmap[right_pixmap][(row & 255) * 256];
	uint32_t *dst = &bitmap.pix(y);

	const pen_t *pal = &m_palette->pens()[palbase * 16];

	if (transp)
	{
		for (int countx = x_count * 2 - 1; countx >= 0; countx--, dst += 2)
		{
			int pen = *(src1++);
			if (pen) *dst = pal[pen];
			pen = *(src2++);
			if (pen) *(dst + 1) = pal[pen];
		}
	}
	else
	{
		for (int countx = x_count * 2 - 1; countx >= 0; countx--, dst += 2)
		{
			*dst = pal[*(src1++)];
			*(dst + 1) = pal[*(src2++)];
		}
	}
}


MC6845_UPDATE_ROW(hnayayoi_state::hnayayoi_update_row)
{
	int col0 = (m_palbank >>  0) & 0x0f;
	int col1 = (m_palbank >>  4) & 0x0f;

	draw_layer_interleaved(bitmap, cliprect, y, y, x_count, 3, 2, col1, false);
	draw_layer_interleaved(bitmap, cliprect, y, y, x_count, 1, 0, col0, true);
}


MC6845_UPDATE_ROW(hnayayoi_state::untoucha_update_row)
{
	int col0 = (m_palbank >>  0) & 0x0f;
	int col1 = (m_palbank >>  4) & 0x0f;
	int col2 = (m_palbank >>  8) & 0x0f;
	int col3 = (m_palbank >> 12) & 0x0f;

	draw_layer_interleaved(bitmap, cliprect, y + 16, y, x_count, 7, 6, col3, false);
	draw_layer_interleaved(bitmap, cliprect, y + 16, y, x_count, 5, 4, col2, true);
	draw_layer_interleaved(bitmap, cliprect, y + 16, y, x_count, 3, 2, col1, true);
	draw_layer_interleaved(bitmap, cliprect, y + 16, y, x_count, 1, 0, col0, true);
}


// machine

template <uint8_t Which>
uint8_t hnayayoi_state::keyboard_r()
{
	int res = 0x3f;

	for (int i = 0; i < 5; i++)
	{
		if (~m_keyb & (1 << i))
			res &= m_key[Which][i]->read();
	}

	return res;
}

void hnayayoi_state::keyboard_w(uint8_t data)
{
	m_keyb = data;
}


WRITE_LINE_MEMBER(hnayayoi_state::coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


WRITE_LINE_MEMBER(hnayayoi_state::nmi_enable_w)
{
	m_nmi_enable = state;
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


WRITE_LINE_MEMBER(hnayayoi_state::nmi_clock_w)
{
	if (m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}


void hnayayoi_state::hnayayoi_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom();
}

void hnayayoi_state::hnayayoi_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ymsnd", FUNC(ym2203_device::write));
	map(0x02, 0x03).r("ymsnd", FUNC(ym2203_device::read));
	map(0x04, 0x04).portr("DSW3");
	map(0x06, 0x06).w(m_msm, FUNC(msm5205_device::data_w));
	map(0x08, 0x08).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x09, 0x09).w("crtc", FUNC(hd6845s_device::register_w));
	map(0x0a, 0x0a).w(FUNC(hnayayoi_state::dynax_blitter_rev1_start_w));
	map(0x0c, 0x0c).w(FUNC(hnayayoi_state::dynax_blitter_rev1_clear_w));
	map(0x20, 0x27).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x40, 0x40).w(FUNC(hnayayoi_state::keyboard_w));
	map(0x41, 0x41).r(FUNC(hnayayoi_state::keyboard_r<0>));
	map(0x42, 0x42).r(FUNC(hnayayoi_state::keyboard_r<1>));
	map(0x43, 0x43).portr("COIN");
	map(0x60, 0x61).w(FUNC(hnayayoi_state::palbank_w));
	map(0x62, 0x67).w(FUNC(hnayayoi_state::dynax_blitter_rev1_param_w));
}

void hnayayoi_state::hnfubuki_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram().share("nvram");
	map(0x8000, 0xfeff).rom();
	map(0xff00, 0xff01).w("ymsnd", FUNC(ym2203_device::write));
	map(0xff02, 0xff03).r("ymsnd", FUNC(ym2203_device::read));
	map(0xff04, 0xff04).portr("DSW3");
	map(0xff06, 0xff06).w(m_msm, FUNC(msm5205_device::data_w));
	map(0xff08, 0xff08).w("crtc", FUNC(hd6845s_device::address_w));
	map(0xff09, 0xff09).w("crtc", FUNC(hd6845s_device::register_w));
	map(0xff0a, 0xff0a).w(FUNC(hnayayoi_state::dynax_blitter_rev1_start_w));
	map(0xff0c, 0xff0c).w(FUNC(hnayayoi_state::dynax_blitter_rev1_clear_w));
	map(0xff20, 0xff27).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0xff40, 0xff40).w(FUNC(hnayayoi_state::keyboard_w));
	map(0xff41, 0xff41).r(FUNC(hnayayoi_state::keyboard_r<0>));
	map(0xff42, 0xff42).r(FUNC(hnayayoi_state::keyboard_r<1>));
	map(0xff43, 0xff43).portr("COIN");
	map(0xff60, 0xff61).w(FUNC(hnayayoi_state::palbank_w));
	map(0xff62, 0xff67).w(FUNC(hnayayoi_state::dynax_blitter_rev1_param_w));
}

void hnayayoi_state::untoucha_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom();
}

void hnayayoi_state::untoucha_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w("ymsnd", FUNC(ym2203_device::address_w));
	map(0x11, 0x11).r("ymsnd", FUNC(ym2203_device::status_r));
	map(0x12, 0x12).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x13, 0x13).w(m_msm, FUNC(msm5205_device::data_w));
	map(0x14, 0x14).portr("COIN");
	map(0x15, 0x15).r(FUNC(hnayayoi_state::keyboard_r<1>));
	map(0x16, 0x16).r(FUNC(hnayayoi_state::keyboard_r<0>));  // bit 7 = blitter busy flag
	map(0x17, 0x17).w(FUNC(hnayayoi_state::keyboard_w));
	map(0x18, 0x19).w(FUNC(hnayayoi_state::palbank_w));
	map(0x1a, 0x1f).w(FUNC(hnayayoi_state::dynax_blitter_rev1_param_w));
	map(0x20, 0x20).w(FUNC(hnayayoi_state::dynax_blitter_rev1_clear_w));
	map(0x28, 0x28).w(FUNC(hnayayoi_state::dynax_blitter_rev1_start_w));
	map(0x30, 0x37).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x50, 0x50).w("ymsnd", FUNC(ym2203_device::data_w));
	map(0x51, 0x51).r("ymsnd", FUNC(ym2203_device::data_r));
	map(0x52, 0x52).w("crtc", FUNC(hd6845s_device::register_w));
}

static INPUT_PORTS_START( hf_keyboard )
	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( hnayayoi ) // test mode shows and test 3 dip banks, but PCB seems to have only one installed (DSW3 below)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 1:1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW 2:1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )   // blitter busy flag, manual has DSW 8 blank
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Service_Mode ) )  PORT_DIPLOCATION( "SW 3:7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Disable Speech" )         PORT_DIPLOCATION( "SW 3:6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )       PORT_DIPLOCATION( "SW 3:5" ) // manual has it blank
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )    PORT_DIPLOCATION( "SW 3:4,3" )
	PORT_DIPSETTING(    0x30, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) )        PORT_DIPLOCATION( "SW 3:2,1" ) // coin B is always 10*coin A
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) // there is also a dip switch
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )  PORT_NAME("Analizer")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "Non Use" in service mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  // "Note" ("Paper Money") = 10 Credits
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( hf_keyboard )
INPUT_PORTS_END

static INPUT_PORTS_START( hnfubuki )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )   // blitter busy flag
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )  PORT_NAME("Analizer")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  // "Note" ("Paper Money") = 10 Credits
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( hf_keyboard )
INPUT_PORTS_END


static INPUT_PORTS_START( untoucha )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Difficulty" )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, "Normal" )
	PORT_DIPSETTING(    0x00, "Difficult" )
	PORT_DIPNAME( 0x06, 0x06, "Stage-Up Difficulty" )      PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )      PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x60, "Score Limit" )              PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x60, "200, 1000, 10000, 70000" )
	PORT_DIPSETTING(    0x40, "250, 2000, 10000, 70000" )
	PORT_DIPSETTING(    0x20, "500, 5000, 10000, 70000" )
	PORT_DIPSETTING(    0x00, "1000, 5000, 10000, 70000" )
	PORT_DIPNAME( 0x80, 0x80, "Stages" )                   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x00, "3" )          // nudity only seems to show when Stages = 3?

	PORT_START("DSW2")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )     PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Speech" )                   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown (Aumit?)" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )             // DIPSW sheet says 'AUMIT CUT WHEN ON'
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )              // Maybe it is Audit, but where/what?
	PORT_DIPNAME( 0x10, 0x10, "Auto Hold" )                PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, "Coin 2 (Credits)" )         PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, "1 Coin/5 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/8 Credits" )
	PORT_DIPNAME( 0x07, 0x07, "Coin 1 (Score)" )           PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0x01, "1 Coin/75 Score" )
	PORT_DIPSETTING(    0x02, "1 Coin/50 Score" )
	PORT_DIPSETTING(    0x03, "1 Coin/20 Score" )
	PORT_DIPSETTING(    0x04, "1 Coin/15 Score" )
	PORT_DIPSETTING(    0x07, "1 Coin/25 Score" )
	PORT_DIPSETTING(    0x05, "1 Coin/10 Score" )
	PORT_DIPSETTING(    0x06, "1 Coin/5 Score" )
	PORT_DIPSETTING(    0x00, "1 Coin/100 Score" )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )  PORT_NAME("Analizer")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  // "Note" ("Paper Money") = 5 or 8 Credits
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("P1_KEY0")  // P1 keyboard
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY0")  // P2 keyboard
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Hold 5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Bet")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Take Score")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Hold 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Hold 4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(2) PORT_NAME("P2 Deal")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(2) PORT_NAME("P2 Double Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



WRITE_LINE_MEMBER(hnayayoi_state::irqhandler)
{
	LOGIRQ("irq");
//  m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void hnayayoi_state::machine_start()
{
	save_item(NAME(m_palbank));
	save_item(NAME(m_blit_layer));
	save_item(NAME(m_blit_dest));
	save_item(NAME(m_blit_src));
	save_item(NAME(m_keyb));
	save_item(NAME(m_nmi_enable));
}

void hnayayoi_state::machine_reset()
{
	m_palbank = 0;
	m_blit_layer = 0;
	m_blit_dest = 0;
	m_blit_src = 0;
	m_keyb = 0;
}


void hnayayoi_state::hnayayoi(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 20_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &hnayayoi_state::hnayayoi_map);
	m_maincpu->set_addrmap(AS_IO, &hnayayoi_state::hnayayoi_io_map);

	CLOCK(config, "nmiclock", 8000).signal_handler().set(FUNC(hnayayoi_state::nmi_clock_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(hnayayoi_state::coin_counter_w));
	m_mainlatch->q_out_cb<2>().set(m_msm, FUNC(msm5205_device::reset_w)).invert();
	m_mainlatch->q_out_cb<3>().set(m_msm, FUNC(msm5205_device::vclk_w));
	m_mainlatch->q_out_cb<4>().set(FUNC(hnayayoi_state::nmi_enable_w)).invert();

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(20_MHz_XTAL / 2, 632, 0, 512, 263, 0, 243);
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	hd6845s_device &crtc(HD6845S(config, "crtc", 20_MHz_XTAL / 8));
	crtc.set_screen("screen");
	crtc.set_char_width(4);
	crtc.set_show_border_area(false);
	crtc.out_vsync_callback().set_inputline(m_maincpu, 0);
	crtc.set_update_row_callback(FUNC(hnayayoi_state::hnayayoi_update_row));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 20_MHz_XTAL / 16));
	ymsnd.irq_handler().set(FUNC(hnayayoi_state::irqhandler));
	ymsnd.port_a_read_callback().set_ioport("DSW1");
	ymsnd.port_b_read_callback().set_ioport("DSW2");
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.80);

	MSM5205(config, m_msm, 384000);
	m_msm->set_prescaler_selector(msm5205_device::SEX_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void hnayayoi_state::hnfubuki(machine_config &config)
{
	hnayayoi(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &hnayayoi_state::hnfubuki_map);
	m_maincpu->set_addrmap(AS_IO, address_map_constructor());

	// D5
	m_mainlatch->q_out_cb<4>().set(FUNC(hnayayoi_state::nmi_enable_w));
}

void hnayayoi_state::untoucha(machine_config &config)
{
	hnayayoi(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &hnayayoi_state::untoucha_map);
	m_maincpu->set_addrmap(AS_IO, &hnayayoi_state::untoucha_io_map);

	m_mainlatch->q_out_cb<1>().set(m_msm, FUNC(msm5205_device::vclk_w));
	m_mainlatch->q_out_cb<2>().set(FUNC(hnayayoi_state::nmi_enable_w));
	m_mainlatch->q_out_cb<3>().set(m_msm, FUNC(msm5205_device::reset_w)).invert();
	m_mainlatch->q_out_cb<4>().set_nop(); // ?

	subdevice<hd6845s_device>("crtc")->set_update_row_callback(FUNC(hnayayoi_state::untoucha_update_row));

	MCFG_VIDEO_START_OVERRIDE(hnayayoi_state,untoucha)
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( hnayayoi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "021.4a",     0x00000, 0x08000, CRC(d9734da4) SHA1(a2c8f5113c8136bea990c282d60f67b2793f9a2c) )
	ROM_LOAD( "022.3a",     0x08000, 0x08000, CRC(e6be5af4) SHA1(cdc56705ba0d191930f892618512cb687975ecbb) )

	ROM_REGION( 0x38000, "blitter", 0 )
	ROM_LOAD( "023.8f",     0x00000, 0x08000, CRC(81ae7317) SHA1(9e37dad046420138b4655d0692fe4bac3a8e09de) )
	ROM_LOAD( "024.9f",     0x08000, 0x08000, CRC(413ab77a) SHA1(4b44d2a76c37f25f126e3759ab61fadba02e2b55) )
	ROM_LOAD( "025.10f",    0x10000, 0x08000, CRC(56d16426) SHA1(38162f2a240ce6828232d4280120acc576f71200) )
	ROM_LOAD( "026.12f",    0x18000, 0x08000, CRC(a99779d9) SHA1(1c4586c5070ec9646440cafefa8cf8a550ad17bd) )
	ROM_LOAD( "027.8d",     0x20000, 0x08000, CRC(209c149a) SHA1(4b7232f1b8b7f88fd8e53b719baff85afce72541) )
	ROM_LOAD( "028.9d",     0x28000, 0x08000, CRC(6981b043) SHA1(9d8f54945bce4ef149290ff34ef6e4532f4e6e8f) )
	ROM_LOAD( "029.10d",    0x30000, 0x08000, CRC(a266f1eb) SHA1(525d8c350cdad2461cb814e4eaa785e84b6fc529) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "r.16b",      0x0000, 0x0100, CRC(b6e9ac04) SHA1(f3ec1c285d40e6189ad8722fab202d2b877f7f75) )
	ROM_LOAD( "g.17b",      0x0100, 0x0100, CRC(a595f310) SHA1(6bd16897d3abcceae76a613a1296992c2e9e1a37) )
	ROM_LOAD( "b.17c",      0x0200, 0x0100, CRC(e33bd9ea) SHA1(47395511d7bd44a27fa55d43094c67c5551cdfbf) )
ROM_END

ROM_START( hnfubuki )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s1.s2c",     0x00000, 0x10000, CRC(afe3179c) SHA1(fdfba1e7073318f9782d628f3c7dd0d9c84cbeea) )

	ROM_REGION( 0x40000, "blitter", 0 )
	ROM_LOAD( "062.8f",     0x00000, 0x10000, CRC(0d96a540) SHA1(1cadf19d8fd48962acb0e45a50431fabd6f13672) )
	ROM_LOAD( "063.9f",     0x10000, 0x10000, CRC(14250093) SHA1(8459024ebe5f8c3fa146e3303a155c2cf5c487b3) )
	ROM_LOAD( "064.10f",    0x20000, 0x10000, CRC(41546fb9) SHA1(3c6028c19aa65dcb7ccfc01c223c2cba36cc9bb4) )
	ROM_LOAD( "0652.12f",   0x30000, 0x10000, CRC(e7b54ea3) SHA1(157306db5f3e3cc652fcdb7b42d33f0b93eda07b) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "r-16b",      0x0000, 0x0100, CRC(e6fd8f5d) SHA1(fb9dedd46c4aaa707d4f7d2614227435eea0bca3) )
	ROM_LOAD( "g-17b",      0x0100, 0x0100, CRC(3f425f67) SHA1(3b0c6585d74871638681749f7162b0780318645c) )
	ROM_LOAD( "b-17c",      0x0200, 0x0100, CRC(d1f912e5) SHA1(0e0b61498ef5e4685754c0786b7288156b8a56ce) )
ROM_END

ROM_START( untoucha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4.10b",     0x00000, 0x10000, CRC(4df04e41) SHA1(4d5232c2f383640394d85417aa973f92c78184c9) )

	ROM_REGION( 0x90000, "blitter", 0 )
	ROM_LOAD( "081.10f",    0x00000, 0x10000, CRC(36ba990d) SHA1(10b2865f1d19c01cc898029a23489f47ade2ce86) )
	ROM_LOAD( "082.12f",    0x10000, 0x10000, CRC(2beb6277) SHA1(ea57970051c674800a9bedd581d734bd9beaa894) )
	ROM_LOAD( "083.13f",    0x20000, 0x10000, CRC(c3fed8ff) SHA1(405a6563ff7420686063e04fb99dfe6f0f7378dc) )
	ROM_LOAD( "084.14f",    0x30000, 0x10000, CRC(10de3aae) SHA1(c006f58f03f261ed104da870e944e3e3bed3ecfe) )
	ROM_LOAD( "085.16f",    0x40000, 0x10000, CRC(527e5879) SHA1(3a126e64890157f346320923427889ab24a39fcc) )
	ROM_LOAD( "086.10h",    0x50000, 0x10000, CRC(be3f0a2e) SHA1(46b52e05c340d32de49769ebf69c9f4f8c1baf1e) )
	ROM_LOAD( "087.12h",    0x60000, 0x10000, CRC(35e072b7) SHA1(e15d56fbc7ec71b21a22a12121ab6d2bfdfc530d) )
	ROM_LOAD( "088.13h",    0x70000, 0x10000, CRC(742cf3c0) SHA1(542fe62feea752e57895c20932865d53953e4945) )
	ROM_LOAD( "089.14h",    0x80000, 0x10000, CRC(ff497db1) SHA1(c786d1b6d07c889e6fc1f812d7dac50674eae356) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "08r.9f",     0x0000, 0x0100, CRC(308e65b4) SHA1(73ec6df141502270148d898376f0ed3211436f04) )
	ROM_LOAD( "08g.8f",     0x0100, 0x0100, CRC(349c3de3) SHA1(ffbbc3a049a749cea5867f575ac77cfd8f5299b1) )
	ROM_LOAD( "08b.7f",     0x0200, 0x0100, CRC(2007435a) SHA1(2fae6f349b36f700e65ae9b980adce691bdc6dde) )
ROM_END


void hnayayoi_state::init_hnfubuki()
{
	uint8_t *rom = memregion("blitter")->base();
	int len = memregion("blitter")->bytes();

	// interestingly, the blitter data has a slight encryption
	// swap address bits 4 and 5
	for (int i = 0; i < len; i += 0x40)
	{
		for (int j = 0; j < 0x10; j++)
		{
			uint8_t t = rom[i + j + 0x10];
			rom[i + j + 0x10] = rom[i + j + 0x20];
			rom[i + j + 0x20] = t;
		}
	}

	// swap data bits 0 and 1
	for (int i = 0; i < len; i++)
	{
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 0, 1);
	}
}

} // anonymous namespace


GAME( 1987, hnayayoi, 0,        hnayayoi, hnayayoi, hnayayoi_state, empty_init,    ROT0, "Dyna Electronics", "Hana Yayoi (Japan)",        MACHINE_SUPPORTS_SAVE )
GAME( 1987, hnfubuki, hnayayoi, hnfubuki, hnfubuki, hnayayoi_state, init_hnfubuki, ROT0, "Dynax",            "Hana Fubuki [BET] (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, untoucha, 0,        untoucha, untoucha, hnayayoi_state, empty_init,    ROT0, "Dynax",            "Untouchable (Ver. 2.10)",   MACHINE_SUPPORTS_SAVE )
