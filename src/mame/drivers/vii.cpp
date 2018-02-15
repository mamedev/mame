// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Robbbert
/******************************************************************************


    Sport Vii / The Batman
    ----------------------

    driver by Ryan Holtz
    Based largely off of Unununium, by Segher


*******************************************************************************

    Short Description:

        Systems run on the SPG243 SoC

    Status:

        Mostly working

    To-Do:

        Audio (SPG243)
        Motion controls (Vii)

    Known u'nSP-Based Systems:

	 	 D - SPG240 - Radica Skateboarder (Sunplus QL8041C die)
        ND - SPG243 - Some form of Leapfrog "edutainment" system
        ND - SPG243 - Star Wars: Clone Wars
        ND - SPG243 - Toy Story
        ND - SPG243 - Animal Art Studio
        ND - SPG243 - Finding Nemo
         D - SPG243 - The Batman
         D - SPG243 - Wall-E
         D - SPG243 - Chintendo / KenSingTon / Siatronics / Jungle Soft Vii
 Partial D - SPG200 - VTech V.Smile
        ND - unknown - Zone 40
         D - SPG243 - Zone 60
         D - SPG243 - Wireless 60
        ND - unknown - Wireless Air 60
        ND - Likely many more


Similar Systems: ( from http://en.wkikpedia.org/wiki/V.Smile )
- V.Smile by VTech, a system designed for children under the age of 10
- V.Smile Pocket (2 versions)
- V.Smile Cyber Pocket
- V.Smile PC Pal
- V-Motion Active Learning System
- Leapster
- V.Smile Baby Infant Development System
- V.Flash

Detailed list of bugs:
- When loading a cart from file manager, sometimes it will crash
- On 'vii_vc1' & 'vii_vc2' cart, the left-right keys are transposed with the up-down keys
- The game 'Jewel Master' on both above carts displays a priority error at top of screen
- In the default bios (no cart loaded):
-- In the menu, when 'Come On!' is selected, a graphics error appears
-- Catch Fish, black screen
-- Come On! freezes at the high score screen, controls seem haywire
-- Bird Knight, no controls
-- Lucky Dice, the dice never stop spinning
-- Fever Move, after pressing A it freezes
-- Alacrity Golf, black screen
-- Smart Dart, black screen
-- Happy Tennis, controls are haywire
-- Bowling, freezes at the high score screen
-- The "EEPROM TEST" option in the diagnostic menu (accessible by holding 1+2 or A+B during startup) freezes when selected
-- The "MOTOR" option in the diagnostic menu does nothing when selected
-- The input for the gyroscopic sensor tests in the "KEYBOARD + G-SENSOR" sub-menu goes haywire
- Zone 60 / Wireless 60:
-- Auto Racing / Auto X, Dragon, Yummy, some other games: some sprites are inverted or opaque where they should be transparent
-- Basketball: emulator crashes when starting the game due to an unimplemented instruction


*******************************************************************************/

#include "emu.h"

#include "cpu/unsp/unsp.h"
#include "machine/i2cmem.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist.h"


#define PAGE_ENABLE_MASK        0x0008

#define PAGE_DEPTH_FLAG_MASK    0x3000
#define PAGE_DEPTH_FLAG_SHIFT   12
#define PAGE_TILE_HEIGHT_MASK   0x00c0
#define PAGE_TILE_HEIGHT_SHIFT  6
#define PAGE_TILE_WIDTH_MASK    0x0030
#define PAGE_TILE_WIDTH_SHIFT   4
#define TILE_X_FLIP             0x0004
#define TILE_Y_FLIP             0x0008

class spg2xx_game_state : public driver_device
{
public:
	spg2xx_game_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_p_ram(*this, "p_ram"),
		m_p_rowscroll(*this, "p_rowscroll"),
		m_p_palette(*this, "p_palette"),
		m_p_spriteram(*this, "p_spriteram"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_p3(*this, "P3"),
		m_bank(*this, "cart")
	{ }

	DECLARE_READ16_MEMBER(video_r);
	DECLARE_WRITE16_MEMBER(video_w);
	DECLARE_READ16_MEMBER(audio_r);
	DECLARE_WRITE16_MEMBER(audio_w);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_READ16_MEMBER(rom_r);

	DECLARE_DRIVER_INIT(walle);
	DECLARE_DRIVER_INIT(batman);
	DECLARE_DRIVER_INIT(wirels60);
	DECLARE_DRIVER_INIT(rad_skat);

	uint32_t screen_update_vii(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vii_vblank);

	TIMER_CALLBACK_MEMBER(tmb1_tick);
	TIMER_CALLBACK_MEMBER(tmb2_tick);

	void spg2xx_base(machine_config &config);
	void spg2xx_basep(machine_config &config);
	void batman(machine_config &config);
	void vii_mem(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void switch_bank(uint32_t bank);
	uint32_t m_centered_coordinates; // this must be a vreg?
	void test_centered(uint8_t *ROM);

	typedef delegate<uint16_t(uint16_t, int)> vii_io_rw_delegate;
	vii_io_rw_delegate   m_vii_io_rw;
private:

	uint16_t do_spg240_rad_skat_io(uint16_t what, int index);
	uint16_t do_spg243_batman_io(uint16_t what, int index);
	uint16_t do_spg243_wireless60_io(uint16_t what, int index);

	uint32_t m_current_bank;

	uint16_t m_video_regs[0x100];

	struct
	{
		uint8_t r, g, b;
	}
	m_screenram[320 * 240];

	uint16_t m_io_regs[0x200];
	uint16_t m_uart_rx_count;
	uint8_t m_controller_input[8];
	uint8_t m_w60_controller_input;

	emu_timer *m_tmb1;
	emu_timer *m_tmb2;
	void do_dma(uint32_t len);
	void do_gpio(uint32_t offset);
	void do_i2c();
	void spg_do_dma(uint32_t len);

	void blit(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile);
	void blit_page(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t bitmap_addr, uint16_t *regs);
	void blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t base_addr);
	void blit_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth);
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3, 4);
	inline uint8_t expand_rgb5_to_rgb8(uint8_t val);
	inline uint8_t mix_channel(uint8_t a, uint8_t b);
	void mix_pixel(uint32_t offset, uint16_t rgb);
	void set_pixel(uint32_t offset, uint16_t rgb);

	// devices

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_p_ram;
	required_shared_ptr<uint16_t> m_p_rowscroll;
	required_shared_ptr<uint16_t> m_p_palette;
	required_shared_ptr<uint16_t> m_p_spriteram;
	required_ioport m_io_p1;
	optional_ioport m_io_p2;
	optional_ioport m_io_p3;
protected:
	required_memory_bank m_bank;
};


class spg2xx_cart_state : public spg2xx_game_state
{
public:
	spg2xx_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: spg2xx_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	DECLARE_DRIVER_INIT(vii);
	DECLARE_DRIVER_INIT(vsmile);

	uint16_t do_spg243_vsmile_io(uint16_t what, int index);
	uint16_t do_spg243_vii_io(uint16_t what, int index);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(vii_cart);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(vsmile_cart);

	void vii(machine_config &config);
	void vsmile(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	optional_device<generic_slot_device> m_cart;
	memory_region *m_cart_rom;
};

#define VII_CTLR_IRQ_ENABLE m_io_regs[0x21]
#define VII_VIDEO_IRQ_ENABLE    m_video_regs[0x62]
#define VII_VIDEO_IRQ_STATUS    m_video_regs[0x63]

#define VERBOSE_LEVEL   (3)

#define ENABLE_VERBOSE_LOG (1)

inline void spg2xx_game_state::verboselog(int n_level, const char *s_fmt, ...)
{
#if ENABLE_VERBOSE_LOG
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
	}
#endif
}

/*************************
*     Video Hardware     *
*************************/

void spg2xx_game_state::video_start()
{
}

inline uint8_t spg2xx_game_state::expand_rgb5_to_rgb8(uint8_t val)
{
	uint8_t temp = val & 0x1f;
	return (temp << 3) | (temp >> 2);
}

// Perform a lerp between a and b
inline uint8_t spg2xx_game_state::mix_channel(uint8_t a, uint8_t b)
{
	uint8_t alpha = m_video_regs[0x1c] & 0x00ff;
	return ((64 - alpha) * a + alpha * b) / 64;
}

void spg2xx_game_state::mix_pixel(uint32_t offset, uint16_t rgb)
{
	m_screenram[offset].r = mix_channel(m_screenram[offset].r, expand_rgb5_to_rgb8(rgb >> 10));
	m_screenram[offset].g = mix_channel(m_screenram[offset].g, expand_rgb5_to_rgb8(rgb >> 5));
	m_screenram[offset].b = mix_channel(m_screenram[offset].b, expand_rgb5_to_rgb8(rgb));
}

void spg2xx_game_state::set_pixel(uint32_t offset, uint16_t rgb)
{
	m_screenram[offset].r = expand_rgb5_to_rgb8(rgb >> 10);
	m_screenram[offset].g = expand_rgb5_to_rgb8(rgb >> 5);
	m_screenram[offset].b = expand_rgb5_to_rgb8(rgb);
}

void spg2xx_game_state::blit(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	uint32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	uint32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t yflipmask = attr & TILE_Y_FLIP ? h - 1 : 0;
	uint32_t xflipmask = attr & TILE_X_FLIP ? w - 1 : 0;

	uint32_t nc = ((attr & 0x0003) + 1) << 1;

	uint32_t palette_offset = (attr & 0x0f00) >> 4;
	palette_offset >>= nc;
	palette_offset <<= nc;

	uint32_t m = bitmap_addr + nc * w*h / 16 * tile;
	uint32_t bits = 0;
	uint32_t nbits = 0;

	uint32_t x, y;

	for (y = 0; y < h; y++)
	{
		uint32_t yy = (yoff + (y ^ yflipmask)) & 0x1ff;

		for (x = 0; x < w; x++)
		{
			uint32_t xx = (xoff + (x ^ xflipmask)) & 0x1ff;
			uint32_t pal;

			bits <<= nc;
			if (nbits < nc)
			{
				uint16_t b = space.read_word(m++ & 0x3fffff);
				b = (b << 8) | (b >> 8);
				bits |= b << (nc - nbits);
				nbits += 16;
			}
			nbits -= nc;

			pal = palette_offset | (bits >> 16);
			bits &= 0xffff;

			if ((ctrl & 0x0010) && yy < 240)
			{
				xx = (xx - (int16_t)m_p_rowscroll[yy]) & 0x01ff;
			}

			if (xx < 320 && yy < 240)
			{
				uint16_t rgb = m_p_palette[pal];
				if (!(rgb & 0x8000))
				{
					if (attr & 0x4000)
					{
						mix_pixel(xx + 320 * yy, rgb);
					}
					else
					{
						set_pixel(xx + 320 * yy, rgb);
					}
				}
			}
		}
	}
}

void spg2xx_game_state::blit_page(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t bitmap_addr, uint16_t *regs)
{
	uint32_t x0, y0;
	uint32_t xscroll = regs[0];
	uint32_t yscroll = regs[1];
	uint32_t attr = regs[2];
	uint32_t ctrl = regs[3];
	uint32_t tilemap = regs[4];
	uint32_t palette_map = regs[5];
	uint32_t h, w, hn, wn;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (!(ctrl & PAGE_ENABLE_MASK))
	{
		return;
	}

	if (((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

	h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	hn = 256 / h;
	wn = 512 / w;

	for (y0 = 0; y0 < hn; y0++)
	{
		for (x0 = 0; x0 < wn; x0++)
		{
			uint16_t tile = space.read_word(tilemap + x0 + wn * y0);
			uint16_t palette = 0;
			uint32_t xx, yy;

			if (!tile)
			{
				continue;
			}

			palette = space.read_word(palette_map + (x0 + wn * y0) / 2);
			if (x0 & 1)
			{
				palette >>= 8;
			}

			uint32_t tileattr = attr;
			uint32_t tilectrl = ctrl;
			if ((ctrl & 2) == 0)
			{   // -(1) bld(1) flip(2) pal(4)
				tileattr &= ~0x000c;
				tileattr |= (palette >> 2) & 0x000c;    // flip

				tileattr &= ~0x0f00;
				tileattr |= (palette << 8) & 0x0f00;    // palette

				tileattr &= ~0x0100;
				tileattr |= (palette << 2) & 0x0100;    // blend
			}

			yy = ((h*y0 - yscroll + 0x10) & 0xff) - 0x10;
			xx = (w*x0 - xscroll) & 0x1ff;

			blit(bitmap, cliprect, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
		}
	}
}

void spg2xx_game_state::blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t base_addr)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t tile, attr;
	int16_t x, y;
	uint32_t h, w;
	uint32_t bitmap_addr = 0x40 * m_video_regs[0x22];

	tile = space.read_word(base_addr + 0);
	x = space.read_word(base_addr + 1);
	y = space.read_word(base_addr + 2);
	attr = space.read_word(base_addr + 3);

	if (!tile)
	{
		return;
	}

	if (((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

	if (m_centered_coordinates)
	{
		x = 160 + x;
		y = 120 - y;

		h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
		w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

		x -= (w / 2);
		y -= (h / 2) - 8;
	}

	x &= 0x01ff;
	y &= 0x01ff;

	blit(bitmap, cliprect, x, y, attr, 0, bitmap_addr, tile);
}

void spg2xx_game_state::blit_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth)
{
	uint32_t n;

	if (!(m_video_regs[0x42] & 1))
	{
		return;
	}

	for (n = 0; n < 256; n++)
	{
		//if(space.read_word(0x2c00 + 4*n)
		{
			blit_sprite(bitmap, cliprect, depth, 0x2c00 + 4 * n);
		}
	}
}

uint32_t spg2xx_game_state::screen_update_vii(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, x, y;

	bitmap.fill(0, cliprect);

	memset(m_screenram, 0, sizeof(m_screenram));

	for (i = 0; i < 4; i++)
	{
		blit_page(bitmap, cliprect, i, 0x40 * m_video_regs[0x20], m_video_regs + 0x10);
		blit_page(bitmap, cliprect, i, 0x40 * m_video_regs[0x21], m_video_regs + 0x16);
		blit_sprites(bitmap, cliprect, i);
	}

	for (y = 0; y < 240; y++)
	{
		for (x = 0; x < 320; x++)
		{
			bitmap.pix32(y, x) = (m_screenram[x + 320 * y].r << 16) | (m_screenram[x + 320 * y].g << 8) | m_screenram[x + 320 * y].b;
		}
	}

	return 0;
}

/*************************
*    Machine Hardware    *
*************************/

void spg2xx_game_state::do_dma(uint32_t len)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	uint32_t src = m_video_regs[0x70];
	uint32_t dst = m_video_regs[0x71] + 0x2c00;
	uint32_t j;

	for (j = 0; j < len; j++)
	{
		mem.write_word(dst + j, mem.read_word(src + j));
	}

	m_video_regs[0x72] = 0;
	m_video_regs[0x63] |= 4;
}

READ16_MEMBER(spg2xx_game_state::video_r)
{
	switch (offset)
	{
	case 0x62: // Video IRQ Enable
		verboselog(0, "video_r: Video IRQ Enable: %04x\n", VII_VIDEO_IRQ_ENABLE);
		return VII_VIDEO_IRQ_ENABLE;

	case 0x63: // Video IRQ Status
		verboselog(0, "video_r: Video IRQ Status: %04x\n", VII_VIDEO_IRQ_STATUS);
		return VII_VIDEO_IRQ_STATUS;

	default:
		verboselog(0, "video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
		break;
	}
	return m_video_regs[offset];
}

WRITE16_MEMBER(spg2xx_game_state::video_w)
{
	switch (offset)
	{
	case 0x10: case 0x16:   // page 1,2 X scroll
		data &= 0x01ff;
		COMBINE_DATA(&m_video_regs[offset]);
		break;

	case 0x11: case 0x17:   // page 1,2 Y scroll
		data &= 0x00ff;
		COMBINE_DATA(&m_video_regs[offset]);
		break;
	case 0x36:      // IRQ pos V
	case 0x37:      // IRQ pos H
		data &= 0x01ff;
		COMBINE_DATA(&m_video_regs[offset]);
		break;
	case 0x62: // Video IRQ Enable
		verboselog(0, "video_w: Video IRQ Enable = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&VII_VIDEO_IRQ_ENABLE);
		break;

	case 0x63: // Video IRQ Acknowledge
		verboselog(0, "video_w: Video IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
		VII_VIDEO_IRQ_STATUS &= ~data;
		if (!VII_VIDEO_IRQ_STATUS)
		{
			m_maincpu->set_input_line(UNSP_IRQ0_LINE, CLEAR_LINE);
		}
		break;

	case 0x70: // Video DMA Source
		verboselog(0, "video_w: Video DMA Source = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;

	case 0x71: // Video DMA Dest
		verboselog(0, "video_w: Video DMA Dest = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;

	case 0x72: // Video DMA Length
		verboselog(0, "video_w: Video DMA Length = %04x (%04x)\n", data, mem_mask);
		do_dma(data);
		break;

	default:
		verboselog(0, "video_w: Unknown register %04x = %04x (%04x)\n", 0x2800 + offset, data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;
	}
}

READ16_MEMBER(spg2xx_game_state::audio_r)
{
	switch (offset)
	{
	default:
		verboselog(4, "audio_r: Unknown register %04x\n", 0x3000 + offset);
		break;
	}
	return 0;
}

WRITE16_MEMBER(spg2xx_game_state::audio_w)
{
	switch (offset)
	{
	default:
		verboselog(4, "audio_w: Unknown register %04x = %04x (%04x)\n", 0x3000 + offset, data, mem_mask);
		break;
	}
}

void spg2xx_game_state::switch_bank(uint32_t bank)
{
	if (bank != m_current_bank)
	{
		m_current_bank = bank;
		m_bank->set_entry(bank);
	}
}

uint16_t spg2xx_cart_state::do_spg243_vii_io(uint16_t what, int index)
{
	if (index == 1)
	{
		uint32_t bank = ((what & 0x80) >> 7) | ((what & 0x20) >> 4);
		switch_bank(bank);
	}
	return what;
}

uint16_t spg2xx_cart_state::do_spg243_vsmile_io(uint16_t what, int index)
{
	// TODO: find out how vsmile accesses these GPIO regs!
	return what;
}

uint16_t spg2xx_game_state::do_spg243_batman_io(uint16_t what, int index)
{
	if (index == 0)
	{
		uint16_t temp = m_io_p1->read();
		what |= (temp & 0x0001) ? 0x8000 : 0;
		what |= (temp & 0x0002) ? 0x4000 : 0;
		what |= (temp & 0x0004) ? 0x2000 : 0;
		what |= (temp & 0x0008) ? 0x1000 : 0;
		what |= (temp & 0x0010) ? 0x0800 : 0;
		what |= (temp & 0x0020) ? 0x0400 : 0;
		what |= (temp & 0x0040) ? 0x0200 : 0;
		what |= (temp & 0x0080) ? 0x0100 : 0;
	}

	if (index == 2)
	{
		// TODO: what is here?
	}

	return what;
}

uint16_t spg2xx_game_state::do_spg240_rad_skat_io(uint16_t what, int index)
{
	// have not checked for outputs yet.

	if (index == 0)
	{
		what = m_io_p1->read();
	}
	else if (index == 1)
	{
		what = m_io_p2->read();
	}
	else if (index == 2)
	{
		what = m_io_p3->read();
	}
	return what;
}

uint16_t spg2xx_game_state::do_spg243_wireless60_io(uint16_t what, int index)
{
	if (index == 0)
	{
		switch (what & 0x300)
		{
		case 0x300:
			m_w60_controller_input = -1;
			break;

		case 0x200:
			m_w60_controller_input++;
			break;

		default:
			uint16_t temp1 = m_io_p1->read();
			uint16_t temp2 = m_io_p2->read();
			uint16_t temp3 = 1 << m_w60_controller_input;
			if (temp1 & temp3) what ^= 0x400;
			if (temp2 & temp3) what ^= 0x800;
			break;
		}
	}

	if (index == 1)
	{
		uint32_t bank = (what & 7);
		switch_bank(bank);
	}

	return what;
}


void spg2xx_game_state::do_gpio(uint32_t offset)
{
	uint32_t index = (offset - 1) / 5;
	uint16_t buffer = m_io_regs[5 * index + 2];
	uint16_t dir = m_io_regs[5 * index + 3];
	uint16_t attr = m_io_regs[5 * index + 4];
	uint16_t special = m_io_regs[5 * index + 5];

	uint16_t push = dir;
	uint16_t pull = (~dir) & (~attr);
	uint16_t what = (buffer & (push | pull));
	what ^= (dir & ~attr);
	what &= ~special;

	if (!m_vii_io_rw.isnull())
		what = m_vii_io_rw(what, index);

	m_io_regs[5 * index + 1] = what;
}

void spg2xx_game_state::do_i2c()
{
}

void spg2xx_game_state::spg_do_dma(uint32_t len)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	uint32_t src = ((m_io_regs[0x101] & 0x3f) << 16) | m_io_regs[0x100];
	uint32_t dst = m_io_regs[0x103] & 0x3fff;
	uint32_t j;

	for (j = 0; j < len; j++)
		mem.write_word(dst + j, mem.read_word(src + j));

	m_io_regs[0x102] = 0;
}

READ16_MEMBER(spg2xx_game_state::io_r)
{
	logerror("io_r %04x\n", offset);

	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[] = { 'A', 'B', 'C' };

	uint16_t val = m_io_regs[offset];

	switch (offset)
	{
	case 0x01: case 0x06: case 0x0b: // GPIO Data Port A/B/C
		do_gpio(offset);
		verboselog(3, "io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
		val = m_io_regs[offset];
		break;

	case 0x02: case 0x03: case 0x04: case 0x05:
	case 0x07: case 0x08: case 0x09: case 0x0a:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Other GPIO regs
		verboselog(3, "io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
		break;

	case 0x1c: // Random
		val = machine().rand() & 0x00ff;
		verboselog(3, "io_r: Random = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x21: // IRQ Control
		verboselog(3, "io_r: Controller IRQ Control = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x22: // IRQ Status
		verboselog(3, "io_r: Controller IRQ Status = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x2b:
		return 0x0000;

	case 0x2c: case 0x2d: // Timers?
		val = machine().rand() & 0x0000ffff;
		verboselog(3, "io_r: Unknown Timer %d Register = %04x (%04x)\n", offset - 0x2c, val, mem_mask);
		break;

	case 0x2f: // Data Segment
		val = m_maincpu->state_int(UNSP_SR) >> 10;
		verboselog(3, "io_r: Data Segment = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x31: // Unknown, UART Status?
		verboselog(3, "io_r: Unknown (UART Status?) = %04x (%04x)\n", 3, mem_mask);
		val = 3;
		break;

	case 0x36: // UART RX Data
		val = m_controller_input[m_uart_rx_count];
		m_uart_rx_count = (m_uart_rx_count + 1) % 8;
		verboselog(3, "io_r: UART RX Data = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x59: // I2C Status
		verboselog(3, "io_r: I2C Status = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x5e: // I2C Data In
		verboselog(3, "io_r: I2C Data In = %04x (%04x)\n", val, mem_mask);
		break;

	default:
		verboselog(3, "io_r: Unknown register %04x\n", 0x3d00 + offset);
		break;
	}

	return val;
}

WRITE16_MEMBER(spg2xx_game_state::io_w)
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[3] = { 'A', 'B', 'C' };

	uint16_t temp = 0;

	switch (offset)
	{
	case 0x00: // GPIO special function select
		verboselog(3, "io_w: GPIO Function Select = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x01: case 0x06: case 0x0b: // GPIO data, port A/B/C
		offset++;
		// Intentional fallthrough

	case 0x02: case 0x03: case 0x04: case 0x05: // Port A
	case 0x07: case 0x08: case 0x09: case 0x0a: // Port B
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Port C
		verboselog(3, "io_w: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		do_gpio(offset);
		break;

	case 0x10:      // timebase control
		if ((m_io_regs[offset] & 0x0003) != (data & 0x0003)) {
			uint16_t hz = 8 << (data & 0x0003);
			verboselog(3, "*** TMB1 FREQ set to %dHz\n", hz);
			m_tmb1->adjust(attotime::zero, 0, attotime::from_hz(hz));
		}
		if ((m_io_regs[offset] & 0x000c) != (data & 0x000c)) {
			uint16_t hz = 128 << ((data & 0x000c) >> 2);
			verboselog(3, "*** TMB2 FREQ set to %dHz\n", hz);
			m_tmb2->adjust(attotime::zero, 0, attotime::from_hz(hz));
		}
		COMBINE_DATA(&m_io_regs[offset]);
		break;
	case 0x21: // IRQ Enable
		verboselog(3, "io_w: Controller IRQ Control = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&VII_CTLR_IRQ_ENABLE);
		if (!VII_CTLR_IRQ_ENABLE)
		{
			m_maincpu->set_input_line(UNSP_IRQ3_LINE, CLEAR_LINE);
		}
		break;

	case 0x22: // IRQ Acknowledge
		verboselog(3, "io_w: Controller IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
		m_io_regs[0x22] &= ~data;
		if (!m_io_regs[0x22])
		{
			m_maincpu->set_input_line(UNSP_IRQ3_LINE, CLEAR_LINE);
		}
		break;

	case 0x2f: // Data Segment
		temp = m_maincpu->state_int(UNSP_SR);
		m_maincpu->set_state_int(UNSP_SR, (temp & 0x03ff) | ((data & 0x3f) << 10));
		verboselog(3, "io_w: Data Segment = %04x (%04x)\n", data, mem_mask);
		break;

	case 0x31: // Unknown UART
		verboselog(3, "io_w: Unknown UART = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x32: // UART Reset
		verboselog(3, "io_w: UART Reset\n");
		break;

	case 0x33: // UART Baud Rate
		verboselog(3, "io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (m_io_regs[0x34] << 8) - data));
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x35: // UART TX Data
		verboselog(3, "io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (data << 8) - m_io_regs[0x33]));
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5a: // I2C Access Mode
		verboselog(3, "io_w: I2C Access Mode = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5b: // I2C Device Address
		verboselog(3, "io_w: I2C Device Address = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5c: // I2C Sub-Address
		verboselog(3, "io_w: I2C Sub-Address = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5d: // I2C Data Out
		verboselog(3, "io_w: I2C Data Out = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5e: // I2C Data In
		verboselog(3, "io_w: I2C Data In = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5f: // I2C Controller Mode
		verboselog(3, "io_w: I2C Controller Mode = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x58: // I2C Command
		verboselog(3, "io_w: I2C Command = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		do_i2c();
		break;

	case 0x59: // I2C Status / IRQ Acknowledge(?)
		verboselog(3, "io_w: I2C Status / Ack = %04x (%04x)\n", data, mem_mask);
		m_io_regs[offset] &= ~data;
		break;

	case 0x100: // DMA Source (L)
	case 0x101: // DMA Source (H)
	case 0x103: // DMA Destination
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x102: // DMA Length
		spg_do_dma(data);
		break;

	default:
		verboselog(3, "io_w: Unknown register %04x = %04x (%04x)\n", 0x3d00 + offset, data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;
	}
}

/*
WRITE16_MEMBER( spg2xx_game_state::rowscroll_w )
{
    switch(offset)
    {
        default:
            verboselog(0, "rowscroll_w: %04x = %04x (%04x)\n", 0x2900 + offset, data, mem_mask);
            break;
    }
}

WRITE16_MEMBER( spg2xx_game_state::spriteram_w )
{
    switch(offset)
    {
        default:
            verboselog(0, "spriteram_w: %04x = %04x (%04x)\n", 0x2c00 + offset, data, mem_mask);
            break;
    }
}
*/

ADDRESS_MAP_START(spg2xx_game_state::vii_mem)
	AM_RANGE( 0x000000, 0x3fffff ) AM_ROMBANK("cart")

	AM_RANGE( 0x000000, 0x0027ff ) AM_RAM AM_SHARE("p_ram")
	AM_RANGE( 0x002800, 0x0028ff ) AM_READWRITE(video_r, video_w)
	AM_RANGE( 0x002900, 0x002aff ) AM_RAM AM_SHARE("p_rowscroll")
	AM_RANGE( 0x002b00, 0x002bff ) AM_RAM AM_SHARE("p_palette")
	AM_RANGE( 0x002c00, 0x002fff ) AM_RAM AM_SHARE("p_spriteram")
	AM_RANGE( 0x003000, 0x0037ff ) AM_READWRITE(audio_r, audio_w)
	AM_RANGE( 0x003d00, 0x003eff ) AM_READWRITE(io_r,    io_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( vii )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("Button A")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Button B")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Button C")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Button D")
INPUT_PORTS_END

static INPUT_PORTS_START( batman )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")
INPUT_PORTS_END

static INPUT_PORTS_START( vsmile )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")
INPUT_PORTS_END

static INPUT_PORTS_START( walle )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
INPUT_PORTS_END

static INPUT_PORTS_START( wirels60 )
	PORT_START("P1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Start")

	PORT_START("P2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_NAME("Joypad Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_NAME("Joypad Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_NAME("Joypad Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_NAME("Joypad Right")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2) PORT_NAME("A Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(2) PORT_NAME("B Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(2) PORT_NAME("Menu")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(2) PORT_NAME("Start")
INPUT_PORTS_END

static INPUT_PORTS_START( rad_skat )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Full Left")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Full Right")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Slight Left") // you have to use this for the menus (eg trick lists)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Slight Right")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Front")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Back")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	// there only seem to be 3 buttons on the pad part, so presumably all the above are the skateboard, and below are the pad?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("M Button")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("X Button")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("O Button")
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED ) // read but unused?


	PORT_START("P3") // PAL/NTSC flag
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_SPECIAL ) 
INPUT_PORTS_END

static INPUT_PORTS_START( rad_skatp )
	PORT_INCLUDE(rad_skat)
	
	PORT_MODIFY("P3") // PAL/NTSC flag
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) 
INPUT_PORTS_END



void spg2xx_game_state::test_centered(uint8_t *ROM)
{
	if (ROM[0x3cd808] == 0x99 &&
		ROM[0x3cd809] == 0x99 &&
		ROM[0x3cd80a] == 0x83 &&
		ROM[0x3cd80b] == 0x5e &&
		ROM[0x3cd80c] == 0x52 &&
		ROM[0x3cd80d] == 0x6b &&
		ROM[0x3cd80e] == 0x78 &&
		ROM[0x3cd80f] == 0x7f)
	{
		m_centered_coordinates = 0;
	}
}



TIMER_CALLBACK_MEMBER(spg2xx_game_state::tmb1_tick)
{
	m_io_regs[0x22] |= 1;
}

TIMER_CALLBACK_MEMBER(spg2xx_game_state::tmb2_tick)
{
	m_io_regs[0x22] |= 2;
}

void spg2xx_cart_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, ceilf((float)m_cart_rom->bytes() / 0x800000), m_cart_rom->base(), 0x800000);
		m_bank->set_entry(0);
	}
}

void spg2xx_game_state::machine_start()
{
	memset(m_video_regs, 0, 0x100 * sizeof(uint16_t));
	memset(m_io_regs, 0, 0x100 * sizeof(uint16_t));
	m_current_bank = 0;

	m_controller_input[0] = 0;
	m_controller_input[4] = 0;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;
	m_w60_controller_input = -1;

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;

	m_tmb1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_game_state::tmb1_tick), this));
	m_tmb2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg2xx_game_state::tmb2_tick), this));
	m_tmb1->reset();
	m_tmb2->reset();

	m_bank->configure_entries(0, ceilf((float)memregion("maincpu")->bytes() / 0x800000), memregion("maincpu")->base(), 0x800000);
	m_bank->set_entry(0);
}

void spg2xx_game_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(spg2xx_game_state::vii_vblank)
{
	uint32_t x = machine().rand() & 0x3ff;
	uint32_t y = machine().rand() & 0x3ff;
	uint32_t z = machine().rand() & 0x3ff;

	m_controller_input[0] = m_io_p1->read();
	m_controller_input[1] = (uint8_t)x;
	m_controller_input[2] = (uint8_t)y;
	m_controller_input[3] = (uint8_t)z;
	m_controller_input[4] = 0;
	x >>= 8;
	y >>= 8;
	z >>= 8;
	m_controller_input[5] = (z << 4) | (y << 2) | x;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;

	m_uart_rx_count = 0;

	VII_VIDEO_IRQ_STATUS = VII_VIDEO_IRQ_ENABLE & 1;
	if (VII_VIDEO_IRQ_STATUS)
	{
		verboselog(0, "Video IRQ\n");
		m_maincpu->set_input_line(UNSP_IRQ0_LINE, ASSERT_LINE);
	}

	//  {
	//      verboselog(0, "audio 1 IRQ\n");
	//      m_maincpu->set_input_line(UNSP_IRQ1_LINE, ASSERT_LINE);
	//  }
	if (m_io_regs[0x22] & m_io_regs[0x21] & 0x0c00)
	{
		verboselog(0, "timerA, timer B IRQ\n");
		m_maincpu->set_input_line(UNSP_IRQ2_LINE, ASSERT_LINE);
	}

	//if(m_io_regs[0x22] & m_io_regs[0x21] & 0x2100)
	// For now trigger always if any enabled
	if (VII_CTLR_IRQ_ENABLE)
	{
		verboselog(0, "UART, ADC IRQ\n");
		m_maincpu->set_input_line(UNSP_IRQ3_LINE, ASSERT_LINE);
	}
	//  {
	//      verboselog(0, "audio 4 IRQ\n");
	//      m_maincpu->set_input_line(UNSP_IRQ4_LINE, ASSERT_LINE);
	//  }

	if (m_io_regs[0x22] & m_io_regs[0x21] & 0x1200)
	{
		verboselog(0, "External IRQ\n");
		m_maincpu->set_input_line(UNSP_IRQ5_LINE, ASSERT_LINE);
	}
	if (m_io_regs[0x22] & m_io_regs[0x21] & 0x0070)
	{
		verboselog(0, "1024Hz, 2048HZ, 4096HZ IRQ\n");
		m_maincpu->set_input_line(UNSP_IRQ6_LINE, ASSERT_LINE);
	}
	if (m_io_regs[0x22] & m_io_regs[0x21] & 0x008b)
	{
		verboselog(0, "TMB1, TMB2, 4Hz, key change IRQ\n");
		m_maincpu->set_input_line(UNSP_IRQ7_LINE, ASSERT_LINE);
	}
}



DEVICE_IMAGE_LOAD_MEMBER(spg2xx_cart_state, vii_cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size < 0x800000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	test_centered(m_cart->get_rom_base());

	return image_init_result::PASS;
}

DEVICE_IMAGE_LOAD_MEMBER(spg2xx_cart_state, vsmile_cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

MACHINE_CONFIG_START(spg2xx_game_state::spg2xx_base)
	MCFG_CPU_ADD( "maincpu", UNSP, XTAL(27'000'000))
	MCFG_CPU_PROGRAM_MAP( vii_mem )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spg2xx_game_state,  vii_vblank)

	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(spg2xx_game_state, screen_update_vii)
	MCFG_PALETTE_ADD("palette", 32768)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(spg2xx_game_state::spg2xx_basep)
	spg2xx_base(config);

	MCFG_SCREEN_MODIFY( "screen" )
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(spg2xx_cart_state::vii)
	spg2xx_base(config);
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "vii_cart")
	MCFG_GENERIC_WIDTH(GENERIC_ROM16_WIDTH)
	MCFG_GENERIC_LOAD(spg2xx_cart_state, vii_cart)

	MCFG_SOFTWARE_LIST_ADD("vii_cart","vii")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(spg2xx_cart_state::vsmile)
	spg2xx_base(config);
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "vsmile_cart")
	MCFG_GENERIC_WIDTH(GENERIC_ROM16_WIDTH)
	MCFG_GENERIC_LOAD(spg2xx_cart_state, vsmile_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list","vsmile_cart")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(spg2xx_game_state::batman)
	spg2xx_base(config);
	MCFG_I2CMEM_ADD("i2cmem")
	MCFG_I2CMEM_DATA_SIZE(0x200)
MACHINE_CONFIG_END


DRIVER_INIT_MEMBER(spg2xx_cart_state, vii)
{
	m_vii_io_rw = vii_io_rw_delegate(&spg2xx_cart_state::do_spg243_vii_io, this);
	m_centered_coordinates = 1;
}

DRIVER_INIT_MEMBER(spg2xx_cart_state, vsmile)
{
	m_vii_io_rw = vii_io_rw_delegate(&spg2xx_cart_state::do_spg243_vsmile_io, this);
	m_centered_coordinates = 1;
}

DRIVER_INIT_MEMBER(spg2xx_game_state, batman)
{
	m_vii_io_rw = vii_io_rw_delegate(&spg2xx_game_state::do_spg243_batman_io, this);
	m_centered_coordinates = 1;
}

DRIVER_INIT_MEMBER(spg2xx_game_state, rad_skat)
{
	m_vii_io_rw = vii_io_rw_delegate(&spg2xx_game_state::do_spg240_rad_skat_io, this);
	m_centered_coordinates = 1;
}

DRIVER_INIT_MEMBER(spg2xx_game_state, walle)
{
	m_vii_io_rw = vii_io_rw_delegate(&spg2xx_game_state::do_spg243_batman_io, this);
	m_centered_coordinates = 0;
}

DRIVER_INIT_MEMBER(spg2xx_game_state, wirels60)
{
	m_vii_io_rw = vii_io_rw_delegate(&spg2xx_game_state::do_spg243_wireless60_io, this);
	m_centered_coordinates = 1;
}

ROM_START( vii )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "vii.bin", 0x0000, 0x2000000, CRC(04627639) SHA1(f883a92d31b53c9a5b0cdb112d07cd793c95fc43))
ROM_END

ROM_START( batmantv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "batman.bin", 0x000000, 0x400000, CRC(46f848e5) SHA1(5875d57bb3fe0cac5d20e626e4f82a0e5f9bb94c) )
ROM_END

ROM_START( vsmile )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD( "vsmilebios.bin", 0x000000, 0x200000, CRC(11f1b416) SHA1(11f77c4973d29c962567390e41879c86a759c93b) )
ROM_END

ROM_START( vsmileg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "bios german.bin", 0x000000, 0x200000, CRC(205c5296) SHA1(7fbcf761b5885c8b1524607aabaf364b4559c8cc) )
ROM_END

ROM_START( vsmilef )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "sysrom_france", 0x000000, 0x200000, CRC(0cd0bdf5) SHA1(5c8d1eada1b6b545555b8d2b09325d7127681af8) )
ROM_END

ROM_START( vsmileb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD( "vbabybios.bin", 0x000000, 0x800000, CRC(ddc7f845) SHA1(2c17d0f54200070176d03d44a40c7923636e596a) )
ROM_END

ROM_START( walle )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, BAD_DUMP CRC(bd554cba) SHA1(6cd06a036ab12e7b0e1fd8003db873b0bb783868) )
	// Alternate dump, we need to decide which one is correct.
	//ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, CRC(6bc90b16) SHA1(184d72de059057aae7800da510fcf05ed1da9ec9))
ROM_END

ROM_START( zone40 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "zone40.bin", 0x0000, 0x4000000, CRC(4ba1444f) SHA1(de83046ab93421486668a247972ad6d3cda19440) )
ROM_END

ROM_START( zone60 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "zone60.bin", 0x0000, 0x4000000, CRC(4cb637d1) SHA1(1f97cbdb4299ac0fbafc2a3aa592066cb0727066))
ROM_END

ROM_START( wirels60 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "wirels60.bin", 0x0000, 0x4000000, CRC(b4df8b28) SHA1(00e3da542e4bc14baf4724ad436f66d4c0f65c84))
ROM_END

ROM_START( rad_skat )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "skateboarder.bin", 0x000000, 0x400000, CRC(08b9ab91) SHA1(6665edc4740804956136c68065890925a144626b) )
ROM_END

ROM_START( rad_skatp ) // rom was dumped from the NTSC version, but region comes from an io port, so ROM is probably the same
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "skateboarder.bin", 0x000000, 0x400000, CRC(08b9ab91) SHA1(6665edc4740804956136c68065890925a144626b) )
ROM_END

/*
Wireless Air 60
(info provided with dump)

System: Wireless Air 60
ROM: Toshiba TC58NVG0S3ETA00
RAM: ESMT M12L128168A

This is a RAW NAND FLASH DUMP

Interesting Strings:

GPnandnand; (GP is General Plus, which is Sunplus by another name)
GLB_GP-F_5B_USBD_1.0.0
SP_ToneMaker
GLB_GP-FS1_0405L_SPU_1.0.2.3
SPF2ALP

"GPnandnand" as a required signature appears to be referenced right here, in page 19 of a GeneralPlus document;
http://www.lcis.com.tw/paper_store/paper_store/GPL162004A-507A_162005A-707AV10_code_reference-20147131205102.pdf

*/

ROM_START( wlsair60 )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASEFF )     
	ROM_LOAD16_WORD_SWAP( "wlsair60.nand", 0x0000, 0x8400000, CRC(eec23b97) SHA1(1bb88290cf54579a5bb51c08a02d793cd4d79f7a) )
ROM_END

//    YEAR  NAME      PARENT    COMPAT    MACHINE      INPUT     STATE              INIT      COMPANY                                              FULLNAME             FLAGS

// VTech systems
CONS( 2005, vsmile,   0,        0,        vsmile,      vsmile,   spg2xx_cart_state, vsmile,   "VTech",                                             "V.Smile (US)", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
CONS( 2005, vsmileg,  vsmile,   0,        vsmile,      vsmile,   spg2xx_cart_state, vsmile,   "VTech",                                             "V.Smile (Germany)", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
CONS( 2005, vsmilef,  vsmile,   0,        vsmile,      vsmile,   spg2xx_cart_state, vsmile,   "VTech",                                             "V.Smile (France)",  MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
CONS( 2005, vsmileb,  0,        0,        vsmile,      vsmile,   spg2xx_cart_state, vsmile,   "VTech",                                             "V.Smile Baby (US)", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )

// Jungle Soft TV games
CONS( 2007, vii,      0,        0,        vii,         vii,      spg2xx_cart_state, vii,      "Jungle Soft / KenSingTon / Chintendo / Siatronics", "Vii",               MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // some games run, others crash

CONS( 2010, zone60,   0,        0,        spg2xx_base, wirels60, spg2xx_game_state, wirels60, "Jungle Soft / Ultimate Products (HK) Ltd",          "Zone 60",           MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2010, wirels60, 0,        0,        spg2xx_base, wirels60, spg2xx_game_state, wirels60, "Jungle Soft / Kids Station Toys Inc",               "Wireless 60",       MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// JAKKS Pacific Inc TV games
CONS( 2004, batmantv, 0,        0,        batman,      batman,   spg2xx_game_state, batman,   "JAKKS Pacific Inc / HotGen Ltd",                    "The Batman",        MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2008, walle,    0,        0,        batman,      walle,    spg2xx_game_state, walle,    "JAKKS Pacific Inc",                                 "Wall-E",            MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Radica TV games
CONS( 2006, rad_skat,  0,       0,        spg2xx_base, rad_skat, spg2xx_game_state, rad_skat, "Radica",                                            "Play TV Skateboarder (NTSC)",       MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2006, rad_skatp, rad_skat,0,        spg2xx_basep,rad_skatp,spg2xx_game_state, rad_skat, "Radica",                                            "Connectv Skateboarder (PAL)",       MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// might not fit here.  First 0x8000 bytes are blank (not too uncommon for these) then rest of rom looks like it's probably encrypted at least
CONS( 200?, zone40,    0,        0,        spg2xx_base, wirels60, spg2xx_game_state, wirels60, "Jungle Soft",                                      "Zone 40", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
// might not fit here, NAND dump, has internal bootstrap at least, see above.
CONS( 200?, wlsair60,  0,        0,        spg2xx_base, wirels60, spg2xx_game_state, wirels60, "Jungle Soft",                                      "Wireless Air 60", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

