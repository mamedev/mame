// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation (Video)

**********************************************************************/

#include "emu.h"
#include "spg2xx_video.h"

DEFINE_DEVICE_TYPE(SPG24X_VIDEO, spg24x_video_device, "spg24x_video", "SPG240-series System-on-a-Chip (Video)")

#define LOG_IRQS            (1U << 4)
#define LOG_VLINES          (1U << 5)
#define LOG_DMA             (1U << 9)
#define LOG_PPU_READS       (1U << 22)
#define LOG_PPU_WRITES      (1U << 23)
#define LOG_UNKNOWN_PPU     (1U << 24)
#define LOG_PPU             (LOG_PPU_READS | LOG_PPU_WRITES | LOG_UNKNOWN_PPU)
#define LOG_ALL             (LOG_IRQS | LOG_PPU | LOG_VLINES | LOG_DMA )

#define VERBOSE             (0)
#include "logmacro.h"

#define VIDEO_IRQ_ENABLE    m_video_regs[0x62]
#define VIDEO_IRQ_STATUS    m_video_regs[0x63]

spg2xx_video_device::spg2xx_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_guny_in(*this),
	m_gunx_in(*this),
	m_sprlimit_read_cb(*this),
	m_video_irq_cb(*this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_scrollram(*this, "scrollram"),
	m_hcompram(*this, "hcompram"),
	m_paletteram(*this, "paletteram"),
	m_spriteram(*this, "spriteram"),
	m_renderer(*this, "renderer")
{
}

spg24x_video_device::spg24x_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg2xx_video_device(mconfig, SPG24X_VIDEO, tag, owner, clock)
{
}

void spg2xx_video_device::device_start()
{
	m_guny_in.resolve_safe(0);
	m_gunx_in.resolve_safe(0);

	m_screenpos_timer = timer_alloc(FUNC(spg2xx_video_device::screenpos_hit), this);
	m_screenpos_timer->adjust(attotime::never);

	save_item(NAME(m_video_regs));

	m_sprlimit_read_cb.resolve_safe(0);
	m_video_irq_cb.resolve();
}

void spg2xx_video_device::device_reset()
{
	memset(m_video_regs, 0, 0x100 * sizeof(uint16_t));

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;
	//m_video_regs[0x3c] = 0x0020;
	//m_video_regs[0x42] = 0x0001;
}

/*************************
*     Video Hardware     *
*************************/

uint32_t spg2xx_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	if (0)
	{
		uint16_t attr1 = m_video_regs[0x12];
		uint16_t ctrl1 = m_video_regs[0x13];
		uint16_t attr2 = m_video_regs[0x18];
		uint16_t ctrl2 = m_video_regs[0x19];

		// attr   --zz pppp ssss FFbb
		// ctrl  ---- ---b hZzR ewrb

		popmessage("Pg1 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n"
			"Pg2 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n"
			"Pg1 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n"
			"Pg2 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",
			attr1, (attr1 & 0xc000) >> 14, (attr1 >> 12) & 3, (attr1 >> 8) & 15, 8 << ((attr1 >> 6) & 3), 8 << ((attr1 >> 4) & 3), BIT(attr1, 3), BIT(attr1, 2), 2 * ((attr1 & 3) + 1),
			attr2, (attr2 & 0xc000) >> 14, (attr2 >> 12) & 3, (attr2 >> 8) & 15, 8 << ((attr2 >> 6) & 3), 8 << ((attr2 >> 4) & 3), BIT(attr2, 3), BIT(attr2, 2), 2 * ((attr2 & 3) + 1),
			ctrl1, (ctrl1 & 0xfe00), BIT(ctrl1, 8), BIT(ctrl1, 7), BIT(ctrl1, 6), BIT(ctrl1, 5), BIT(ctrl1, 4), BIT(ctrl1, 3), BIT(ctrl1, 2), BIT(ctrl1, 1), BIT(ctrl1, 0),
			ctrl2, (ctrl2 & 0xfe00), BIT(ctrl2, 8), BIT(ctrl2, 7), BIT(ctrl2, 6), BIT(ctrl2, 5), BIT(ctrl2, 4), BIT(ctrl2, 3), BIT(ctrl2, 2), BIT(ctrl2, 1), BIT(ctrl2, 0));

	}


	const uint32_t page1_addr = 0x40 * m_video_regs[0x20];
	const uint32_t page2_addr = 0x40 * m_video_regs[0x21];
	const uint32_t sprite_addr = 0x40 * m_video_regs[0x22];

	uint16_t *page1_scroll = m_video_regs + 0x10;
	uint16_t *page2_scroll = m_video_regs + 0x16;
	uint16_t *page1_regs = m_video_regs + 0x12;
	uint16_t *page2_regs = m_video_regs + 0x18;

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		m_renderer->new_line(cliprect);

		for (int i = 0; i < 4; i++)
		{
			m_renderer->draw_page(false, false, false, 0, cliprect, scanline, i, page1_addr, page1_scroll, page1_regs, mem, m_paletteram, m_scrollram, 0);
			m_renderer->draw_page(false, false, false, 0, cliprect, scanline, i, page2_addr, page2_scroll, page2_regs, mem, m_paletteram, m_scrollram, 1);
			m_renderer->draw_sprites(false, 0, false, 0, false, cliprect, scanline, i, sprite_addr, mem, m_paletteram, m_spriteram, m_sprlimit_read_cb());
		}

		m_renderer->apply_saturation_and_fade(bitmap, cliprect, scanline);
	}

	return 0;
}


void spg2xx_video_device::do_sprite_dma(uint32_t len)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	uint32_t src = m_video_regs[0x70] & 0x3fff;
	uint32_t dst = m_video_regs[0x71];

	for (uint32_t j = 0; j < len; j++)
	{
		m_spriteram[(dst + j) & 0x3ff] = mem.read_word(src + j);
	}

	m_video_regs[0x72] = 0;
	if (VIDEO_IRQ_ENABLE & 4)
	{
		const uint16_t old = VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS |= 4;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
	}
}

uint16_t spg2xx_video_device::video_r(offs_t offset)
{
	switch (offset)
	{
	case 0x10: // Page 1 X scroll
		LOGMASKED(LOG_PPU_READS, "video_r: Page 1 X Scroll\n");
		return m_video_regs[offset];

	case 0x11: // Page 1 Y scroll
		LOGMASKED(LOG_PPU_READS, "video_r: Page 1 Y Scroll\n");
		return m_video_regs[offset];

	case 0x1c: // vertical compression, amount, 0x20 = no scale? (not on spg288?)
		LOGMASKED(LOG_PPU_READS, "video_r: Ycmp_Value\n");
		return m_renderer->get_video_reg_1c();

	case 0x1d: // (not on spg288?)
		LOGMASKED(LOG_PPU_READS, "video_r: Ycmp_Y_Offset\n");
		return m_renderer->get_video_reg_1d();

	case 0x1e: // (not on spg288?)
		LOGMASKED(LOG_PPU_READS, "video_r: Ycmp_Step\n");
		return m_renderer->get_video_reg_1e();

	case 0x22: // Sprite Segment Address
		LOGMASKED(LOG_PPU_READS, "video_r: Sprite Segment Address\n");
		return m_video_regs[offset];

	case 0x2a: // Blend Level Control
		LOGMASKED(LOG_PPU_READS, "video_r: Blend Level Control\n");
		return m_renderer->get_video_reg_2a();

	case 0x30: // Fade Effect Control
		LOGMASKED(LOG_PPU_READS, "video_r: Fade Effect Control\n");
		return m_renderer->get_video_reg_30();
		break;

	case 0x38: // Current Line
		LOGMASKED(LOG_VLINES, "video_r: Current Line: %04x\n", m_screen->vpos());
		return m_screen->vpos();

	case 0x3c: // TV Control 1
		LOGMASKED(LOG_PPU_READS, "video_r: TV Control 1\n");
		return m_renderer->get_video_reg_3c();

	case 0x3e: // Light Pen Y Position
		LOGMASKED(LOG_PPU_READS, "video_r: Light Pen Y / Lightgun Y\n");
		return m_guny_in();

	case 0x3f: // Light Pen X Position
		LOGMASKED(LOG_PPU_READS, "video_r: Light Pen X / Lightgun X\n");
		return m_gunx_in();

	case 0x42: // Sprite Control
		LOGMASKED(LOG_PPU_READS, "video_w: Sprite Control\n");
		return m_renderer->get_video_reg_42();

	case 0x62: // Video IRQ Enable
		LOGMASKED(LOG_IRQS, "video_r: Video IRQ Enable: %04x\n", VIDEO_IRQ_ENABLE);
		return VIDEO_IRQ_ENABLE;

	case 0x63: // Video IRQ Status
		LOGMASKED(LOG_IRQS, "video_r: Video IRQ Status: %04x\n", VIDEO_IRQ_STATUS);
		return VIDEO_IRQ_STATUS;

	default:
		LOGMASKED(LOG_UNKNOWN_PPU, "video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
		break;
	}
	return m_video_regs[offset];
}

void spg2xx_video_device::video_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0x10: // Page 1 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x11: // Page 1 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Y Scroll = %04x\n", data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x12: // Page 1 Attributes
	{
		uint16_t attr1 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w: Pg1 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n",
			attr1, (attr1 & 0xc000) >> 14, (attr1 >> 12) & 3, (attr1 >> 8) & 15, 8 << ((attr1 >> 6) & 3), 8 << ((attr1 >> 4) & 3), BIT(attr1, 3), BIT(attr1, 2), 2 * ((attr1 & 3) + 1));
		m_video_regs[offset] = data;
		break;
	}

	case 0x13: // Page 1 Control
	{
		uint16_t ctrl1 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w Pg1 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",
			ctrl1, (ctrl1 & 0xfe00), BIT(ctrl1, 8), BIT(ctrl1, 7), BIT(ctrl1, 6), BIT(ctrl1, 5), BIT(ctrl1, 4), BIT(ctrl1, 3), BIT(ctrl1, 2), BIT(ctrl1, 1), BIT(ctrl1, 0));
		m_video_regs[offset] = data;
		break;
	}

	case 0x14: // Page 1 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Tile Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x15: // Page 1 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Attribute Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x16: // Page 2 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x17: // Page 2 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x18: // Page 2 Attributes
	{
		uint16_t attr2 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w: Pg2 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n",
			attr2, (attr2 & 0xc000) >> 14, (attr2 >> 12) & 3, (attr2 >> 8) & 15, 8 << ((attr2 >> 6) & 3), 8 << ((attr2 >> 4) & 3), BIT(attr2, 3), BIT(attr2, 2), 2 * ((attr2 & 3) + 1));
		m_video_regs[offset] = data;
		break;
	}

	case 0x19: // Page 2 Control
	{
		uint16_t ctrl2 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w: Pg2 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",
			ctrl2, (ctrl2 & 0xfe00), BIT(ctrl2, 8), BIT(ctrl2, 7), BIT(ctrl2, 6), BIT(ctrl2, 5), BIT(ctrl2, 4), BIT(ctrl2, 3), BIT(ctrl2, 2), BIT(ctrl2, 1), BIT(ctrl2, 0));
		m_video_regs[offset] = data;
		break;
	}

	case 0x1a: // Page 2 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Tile Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x1b: // Page 2 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Attribute Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x1c: // vertical compression, amount, 0x20 = no scale? (not on spg288?)
		LOGMASKED(LOG_PPU_WRITES, "video_w: Ycmp_Value = %04x\n", data);
		m_video_regs[offset] = data;
		m_renderer->set_video_reg_1c(data);
		break;

	case 0x1d: // (not on spg288?)
		LOGMASKED(LOG_PPU_WRITES, "video_w: Ycmp_Y_Offset = %04x\n", data);
		m_video_regs[offset] = data;
		m_renderer->set_video_reg_1d(data);
		break;

	case 0x1e: // (not on spg288?)
		LOGMASKED(LOG_PPU_WRITES, "video_w: Ycmp_Step = %04x\n", data);
		m_video_regs[offset] = data;
		m_renderer->set_video_reg_1e(data);
		break;

	case 0x20: // Page 1 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x21: // Page 2 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x22: // Sprite Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Sprite Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x2a: // Blend Level Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Blend Level Control = %04x\n", data & 0x0003);
		m_video_regs[offset] = data & 0x0003;
		m_renderer->set_video_reg_2a(data);
		break;

	case 0x30: // Fade Effect Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Fade Effect Control = %04x\n", data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		m_renderer->set_video_reg_30(data);
		break;

	case 0x36: // IRQ pos V
	case 0x37: // IRQ pos H
		m_video_regs[offset] = data & 0x01ff;
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Position: %04x,%04x (%04x)\n", m_video_regs[0x37], m_video_regs[0x36], 0x2800 | offset);
		if (m_video_regs[0x37] < 160 && m_video_regs[0x36] < 240)
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
		else
			m_screenpos_timer->adjust(attotime::never);
		break;

	case 0x39: // Latch 1st Line Pen Pulse
		LOGMASKED(LOG_PPU_WRITES, "video_w: Latch 1st Line Pen Pulse = %04x\n", data & 0x0001);
		m_video_regs[offset] = data & 0x0001;
		break;

	case 0x3c: // TV Control 1
		LOGMASKED(LOG_PPU_WRITES, "video_w: TV Control 1 = %04x (Hue:%02x, Saturation:%02x)\n", data, data >> 8, data & 0x00ff);
		m_video_regs[offset] = data;
		m_renderer->set_video_reg_3c(data);
		break;

	case 0x3d: // TV Control 2
	{
		static const char* const s_lpf_mode[4] = { "LPF1", "LPF2", "All", "Edge" };
		LOGMASKED(LOG_PPU_WRITES, "video_w: TV Control 2 = %04x (LPFMode:%s, Enable:%d, Interlace:%d)\n", data & 0x000f
			, s_lpf_mode[(data >> 2) & 3], BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data & 0x000f;
		break;
	}

	case 0x3e: // Light Pen Y Position
		LOGMASKED(LOG_PPU_WRITES, "video_w: Light Pen Y / Lightgun Y (read only) = %04x\n", data & 0x01ff);
		break;

	case 0x3f: // Light Pen X Position
		LOGMASKED(LOG_PPU_WRITES, "video_w: Light Pen X / Lightgun X (read only) = %04x\n", data & 0x01ff);
		break;

	case 0x42: // Sprite Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Sprite Control = %04x (TopLeft:%d, Enable:%d)\n", data, BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data;
		m_renderer->set_video_reg_42(data);
		break;

	case 0x62: // Video IRQ Enable
	{
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Enable = %04x (DMA:%d, Timing:%d, Blanking:%d)\n", data, BIT(data, 2), BIT(data, 1), BIT(data, 0));
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_ENABLE = data & 0x0007;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x63: // Video IRQ Acknowledge
	{
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Acknowledge = %04x\n", data);
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS &= ~data;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x70: // Sprite DMA Source
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Source = %04x\n", data & 0x3fff);
		m_video_regs[offset] = data & 0x3fff;
		break;

	case 0x71: // Sprite DMA Dest
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Dest = %04x\n", data & 0x03ff);
		m_video_regs[offset] = data & 0x03ff;
		break;

	case 0x72: // Sprite DMA Length
	{
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Length = %04x\n", data & 0x03ff);
		uint16_t length = data & 0x3ff;
		do_sprite_dma(length ? length : 0x400);
		break;
	}

	default:
		LOGMASKED(LOG_UNKNOWN_PPU, "video_w: Unknown register %04x = %04x\n", 0x2800 + offset, data);
		m_video_regs[offset] = data;
		break;
	}
}

WRITE_LINE_MEMBER(spg2xx_video_device::vblank)
{
	if (!state)
	{
		VIDEO_IRQ_STATUS &= ~1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
		return;
	}

	if (VIDEO_IRQ_ENABLE & 1)
	{
		VIDEO_IRQ_STATUS |= 1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
	}
}

void spg2xx_video_device::check_video_irq()
{
	LOGMASKED(LOG_IRQS, "%ssserting IRQ0 (%04x, %04x)\n", (VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? "A" : "Dea", VIDEO_IRQ_STATUS, VIDEO_IRQ_ENABLE);
	m_video_irq_cb((VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(spg2xx_video_device::screenpos_hit)
{
	if (VIDEO_IRQ_ENABLE & 2)
	{
		VIDEO_IRQ_STATUS |= 2;
		check_video_irq();
	}
	m_screen->update_partial(m_screen->vpos());

	// fire again, jak_dbz pinball needs this
	m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
}


void spg2xx_video_device::device_add_mconfig(machine_config &config)
{
	SPG_RENDERER(config, m_renderer, 0);
}
