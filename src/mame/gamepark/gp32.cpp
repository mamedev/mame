// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/**************************************************************************
 *
 * gp32.cpp - Game Park GP32
 * Driver by Tim Schuerewegen
 *
 * CPU: Samsung S3C2400X01 SoC
 * S3C2400X01 consists of:
 *    ARM920T CPU core + MMU
 *    LCD controller
 *    DMA controller
 *    Interrupt controller
 *    USB controller
 *    and more.
 *
 * TODO:
 * - device-ify s3c240x;
 * - console screen is horizontal, but here screen is setted up with
 *   Height < Width and ROT270, in a double negation fashion. Simplify and
 *   eventually update video fns;
 * - Normalize palette to actual TFT color space;
 * - Several games have dubious sound clipping and mixing;
 * - RF and internet comms & netplay (rallypop has both);
 * - Games from SW list doesn't reload after save, is it even supported?
 * - Add slot for USB PC-Link application, add a host machine connection
 *   somehow;
 * - Verify MP3 support, which in turn needs checking out how the filesystem
 *   works here (and eventually a tool for direct injecting);
 * - Verify gp32linux distro;
 *
 **************************************************************************/

#include "emu.h"
#include "gp32.h"

#include "cpu/arm7/arm7.h"

#include "softlist_dev.h"
#include "speaker.h"

#define LOG_STARTSTOP (1U << 1)
#define LOG_TIMER     (1U << 2)
#define LOG_VRAM      (1U << 3)
#define LOG_MISC      (1U << 4)
#define LOG_TRACE     (1U << 5)

#define VERBOSE LOG_GENERAL

#include "logmacro.h"

#define CLOCK_MULTIPLIER 1

#define MPLLCON  1
#define UPLLCON  2

#define BITS(x,m,n) (((x)>>(n))&((1<<((m)-(n)+1))-1))


// LCD CONTROLLER


#define BPPMODE_TFT_01  0x08
#define BPPMODE_TFT_02  0x09
#define BPPMODE_TFT_04  0x0A
#define BPPMODE_TFT_08  0x0B
#define BPPMODE_TFT_16  0x0C

inline rgb_t gp32_state::s3c240x_get_color_5551( uint16_t data)
{
	uint8_t r, g, b, i;
	r = BITS( data, 15, 11) << 3;
	g = BITS( data, 10, 6) << 3;
	b = BITS( data, 5, 1) << 3;
	i = BIT( data, 1) << 2;
	return rgb_t( r | i, g | i, b | i);
}

void gp32_state::s3c240x_lcd_dma_reload()
{
	m_s3c240x_lcd.vramaddr_cur = m_s3c240x_lcd_regs[5] << 1;
	m_s3c240x_lcd.vramaddr_max = ((m_s3c240x_lcd_regs[5] & 0xFFE00000) | m_s3c240x_lcd_regs[6]) << 1;
	m_s3c240x_lcd.offsize = BITS( m_s3c240x_lcd_regs[7], 21, 11);
	m_s3c240x_lcd.pagewidth_cur = 0;
	m_s3c240x_lcd.pagewidth_max = BITS( m_s3c240x_lcd_regs[7], 10, 0);
	LOGMASKED(LOG_VRAM, "LCD - vramaddr %08X %08X offsize %08X pagewidth %08X\n", m_s3c240x_lcd.vramaddr_cur, m_s3c240x_lcd.vramaddr_max, m_s3c240x_lcd.offsize, m_s3c240x_lcd.pagewidth_max);
}

void gp32_state::s3c240x_lcd_dma_init()
{
	s3c240x_lcd_dma_reload();
	m_s3c240x_lcd.bppmode = BITS( m_s3c240x_lcd_regs[0], 4, 1);
	m_s3c240x_lcd.bswp = BIT( m_s3c240x_lcd_regs[4], 1);
	m_s3c240x_lcd.hwswp = BIT( m_s3c240x_lcd_regs[4], 0);
	m_s3c240x_lcd.lineval = BITS( m_s3c240x_lcd_regs[1], 23, 14);
	m_s3c240x_lcd.hozval = BITS( m_s3c240x_lcd_regs[2], 18, 8);
}

uint32_t gp32_state::s3c240x_lcd_dma_read( )
{
	uint8_t *vram, data[4];
	int i;
	for (i = 0; i < 2; i++)
	{
		vram = (uint8_t *)m_s3c240x_ram.target() + m_s3c240x_lcd.vramaddr_cur - 0x0C000000;
		data[i*2+0] = vram[0];
		data[i*2+1] = vram[1];
		m_s3c240x_lcd.vramaddr_cur += 2;
		m_s3c240x_lcd.pagewidth_cur++;
		if (m_s3c240x_lcd.pagewidth_cur >= m_s3c240x_lcd.pagewidth_max)
		{
			m_s3c240x_lcd.vramaddr_cur += m_s3c240x_lcd.offsize << 1;
			m_s3c240x_lcd.pagewidth_cur = 0;
		}
	}
	if (m_s3c240x_lcd.hwswp == 0)
	{
		if (m_s3c240x_lcd.bswp == 0)
		{
			return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0] << 0);
		}
		else
		{
			return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
		}
	}
	else
	{
		if (m_s3c240x_lcd.bswp == 0)
		{
			return (data[1] << 24) | (data[0] << 16) | (data[3] << 8) | (data[2] << 0);
		}
		else
		{
			return (data[2] << 24) | (data[3] << 16) | (data[0] << 8) | (data[1] << 0);
		}
	}
}

void gp32_state::s3c240x_lcd_render_01( )
{
	bitmap_rgb32 &bitmap = m_bitmap;
	uint32_t *scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c240x_lcd_dma_read();
		for (int j = 0; j < 32; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 31) & 0x01);
			data <<= 1;
			m_s3c240x_lcd.hpos++;
			if (m_s3c240x_lcd.hpos >= (m_s3c240x_lcd.pagewidth_max << 4))
			{
				m_s3c240x_lcd.vpos = (m_s3c240x_lcd.vpos + 1) % (m_s3c240x_lcd.lineval + 1);
				m_s3c240x_lcd.hpos = 0;
				scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
			}
		}
	}
}

void gp32_state::s3c240x_lcd_render_02( )
{
	bitmap_rgb32 &bitmap = m_bitmap;
	uint32_t *scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c240x_lcd_dma_read();
		for (int j = 0; j < 16; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 30) & 0x03);
			data <<= 2;
			m_s3c240x_lcd.hpos++;
			if (m_s3c240x_lcd.hpos >= (m_s3c240x_lcd.pagewidth_max << 3))
			{
				m_s3c240x_lcd.vpos = (m_s3c240x_lcd.vpos + 1) % (m_s3c240x_lcd.lineval + 1);
				m_s3c240x_lcd.hpos = 0;
				scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
			}
		}
	}
}

void gp32_state::s3c240x_lcd_render_04( )
{
	bitmap_rgb32 &bitmap = m_bitmap;
	uint32_t *scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c240x_lcd_dma_read( );
		for (int j = 0; j < 8; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 28) & 0x0F);
			data <<= 4;
			m_s3c240x_lcd.hpos++;
			if (m_s3c240x_lcd.hpos >= (m_s3c240x_lcd.pagewidth_max << 2))
			{
				m_s3c240x_lcd.vpos = (m_s3c240x_lcd.vpos + 1) % (m_s3c240x_lcd.lineval + 1);
				m_s3c240x_lcd.hpos = 0;
				scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
			}
		}
	}
}

void gp32_state::s3c240x_lcd_render_08( )
{
	bitmap_rgb32 &bitmap = m_bitmap;
	uint32_t *scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c240x_lcd_dma_read();
		for (int j = 0; j < 4; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 24) & 0xFF);
			data <<= 8;
			m_s3c240x_lcd.hpos++;
			if (m_s3c240x_lcd.hpos >= (m_s3c240x_lcd.pagewidth_max << 1))
			{
				m_s3c240x_lcd.vpos = (m_s3c240x_lcd.vpos + 1) % (m_s3c240x_lcd.lineval + 1);
				m_s3c240x_lcd.hpos = 0;
				scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
			}
		}
	}
}

void gp32_state::s3c240x_lcd_render_16( )
{
	bitmap_rgb32 &bitmap = m_bitmap;
	uint32_t *scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c240x_lcd_dma_read();
		for (int j = 0; j < 2; j++)
		{
			*scanline++ = s3c240x_get_color_5551( (data >> 16) & 0xFFFF);
			data <<= 16;
			m_s3c240x_lcd.hpos++;
			if (m_s3c240x_lcd.hpos >= (m_s3c240x_lcd.pagewidth_max << 0))
			{
				m_s3c240x_lcd.vpos = (m_s3c240x_lcd.vpos + 1) % (m_s3c240x_lcd.lineval + 1);
				m_s3c240x_lcd.hpos = 0;
				scanline = &bitmap.pix(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(gp32_state::s3c240x_lcd_timer_exp)
{
	LOGMASKED(LOG_TIMER, "LCD timer callback\n");
	m_s3c240x_lcd.vpos = m_screen->vpos();
	m_s3c240x_lcd.hpos = m_screen->hpos();
	LOGMASKED(LOG_VRAM, "LCD - vpos %d hpos %d\n", m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos);
	if (m_s3c240x_lcd.vramaddr_cur >= m_s3c240x_lcd.vramaddr_max)
	{
		s3c240x_lcd_dma_reload();
	}
	LOGMASKED(LOG_VRAM, "LCD - vramaddr %08X\n", m_s3c240x_lcd.vramaddr_cur);
	while (m_s3c240x_lcd.vramaddr_cur < m_s3c240x_lcd.vramaddr_max)
	{
		switch (m_s3c240x_lcd.bppmode)
		{
			case BPPMODE_TFT_01 : s3c240x_lcd_render_01(); break;
			case BPPMODE_TFT_02 : s3c240x_lcd_render_02(); break;
			case BPPMODE_TFT_04 : s3c240x_lcd_render_04(); break;
			case BPPMODE_TFT_08 : s3c240x_lcd_render_08(); break;
			case BPPMODE_TFT_16 : s3c240x_lcd_render_16(); break;
			default : LOG("s3c240x_lcd_timer_exp: bppmode %d not supported\n", m_s3c240x_lcd.bppmode); break;
		}
		if ((m_s3c240x_lcd.vpos == 0) && (m_s3c240x_lcd.hpos == 0)) break;
	}
	m_s3c240x_lcd_timer->adjust(m_screen->time_until_pos(m_s3c240x_lcd.vpos, m_s3c240x_lcd.hpos));
}

void gp32_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
}

uint32_t gp32_state::screen_update_gp32(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	s3c240x_lcd_dma_init();
	return 0;
}

uint32_t gp32_state::s3c240x_lcd_r(offs_t offset)
{
	uint32_t data = m_s3c240x_lcd_regs[offset];
	switch (offset)
	{
		// LCDCON1
		case 0x00 / 4 :
		{
			// make sure line counter is going
			uint32_t lineval = BITS( m_s3c240x_lcd_regs[1], 23, 14);
			data = (data & ~0xFFFC0000) | ((lineval - m_screen->vpos()) << 18);
		}
		break;
	}
	LOGMASKED(LOG_TRACE, "(LCD) %08X -> %08X %s\n", 0x14A00000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_lcd_configure()
{
	uint32_t vspw, vbpd, lineval, vfpd, hspw, hbpd, hfpd, hozval, clkval, hclk;
	double framerate, vclk;
	rectangle visarea;
	vspw = BITS( m_s3c240x_lcd_regs[1], 5, 0);
	vbpd = BITS( m_s3c240x_lcd_regs[1], 31, 24);
	lineval = BITS( m_s3c240x_lcd_regs[1], 23, 14);
	vfpd = BITS( m_s3c240x_lcd_regs[1], 13, 6);
	hspw = BITS( m_s3c240x_lcd_regs[3], 7, 0);
	hbpd = BITS( m_s3c240x_lcd_regs[2], 25, 19);
	hfpd = BITS( m_s3c240x_lcd_regs[2], 7, 0);
	hozval = BITS( m_s3c240x_lcd_regs[2], 18, 8);
	clkval = BITS( m_s3c240x_lcd_regs[0], 17, 8);
	hclk = s3c240x_get_hclk(MPLLCON);
	LOGMASKED(LOG_VRAM, "LCD - vspw %d vbpd %d lineval %d vfpd %d hspw %d hbpd %d hfpd %d hozval %d clkval %d hclk %d\n", vspw, vbpd, lineval, vfpd, hspw, hbpd, hfpd, hozval, clkval, hclk);
	vclk = (double)(hclk / ((clkval + 1) * 2));
	LOGMASKED(LOG_VRAM, "LCD - vclk %f\n", vclk);
	framerate = vclk / (((vspw + 1) + (vbpd + 1) + (lineval + 1) + (vfpd + 1)) * ((hspw + 1) + (hbpd + 1) + (hfpd + 1) + (hozval + 1)));
	LOGMASKED(LOG_VRAM, "LCD - framerate %f\n", framerate);
	visarea.set(0, hozval, 0, lineval);
	LOGMASKED(LOG_VRAM, "LCD - visarea min_x %d min_y %d max_x %d max_y %d\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	m_screen->configure(hozval + 1, lineval + 1, visarea, HZ_TO_ATTOSECONDS( framerate));
}

void gp32_state::s3c240x_lcd_start()
{
	LOGMASKED(LOG_STARTSTOP, "LCD start\n");
	s3c240x_lcd_configure();
	s3c240x_lcd_dma_init();
	m_s3c240x_lcd_timer->adjust(m_screen->time_until_pos(0, 0));
}

void gp32_state::s3c240x_lcd_stop()
{
	LOGMASKED(LOG_STARTSTOP, "LCD stop\n");
	m_s3c240x_lcd_timer->adjust( attotime::never);
}

void gp32_state::s3c240x_lcd_recalc()
{
	if (m_s3c240x_lcd_regs[0] & 1)
	{
		s3c240x_lcd_start();
	}
	else
	{
		s3c240x_lcd_stop();
	}
}

void gp32_state::s3c240x_lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = m_s3c240x_lcd_regs[offset];
	LOGMASKED(LOG_TRACE, "(LCD) %08X <- %08X %s\n", 0x14A00000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_lcd_regs[offset]);
	switch (offset)
	{
		// LCDCON1
		case 0x00 / 4 :
		{
			if ((old_value & 1) != (data & 1))
			{
				s3c240x_lcd_recalc();
			}
		}
		break;
	}
}

// LCD PALETTE


uint32_t gp32_state::s3c240x_lcd_palette_r(offs_t offset)
{
	uint32_t data = m_s3c240x_lcd_palette[offset];
	LOGMASKED(LOG_TRACE, "(LCD) %08X -> %08X %s\n", 0x14A00400 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_lcd_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(LCD) %08X <- %08X %s\n", 0x14A00400 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_lcd_palette[offset]);
	if (mem_mask != 0xffffffff)
	{
		LOG("s3c240x_lcd_palette_w: unknown mask %08x\n", mem_mask);
	}
	m_palette->set_pen_color( offset, s3c240x_get_color_5551( data & 0xFFFF));
}

// CLOCK & POWER MANAGEMENT


uint32_t gp32_state::s3c240x_get_fclk(int reg)
{
	uint32_t data, mdiv, pdiv, sdiv;
	data = m_s3c240x_clkpow_regs[reg]; // MPLLCON or UPLLCON
	mdiv = BITS( data, 19, 12);
	pdiv = BITS( data, 9, 4);
	sdiv = BITS( data, 1, 0);
	return (uint32_t)((double)((mdiv + 8) * 12000000) / (double)((pdiv + 2) * (1 << sdiv)));
}

uint32_t gp32_state::s3c240x_get_hclk(int reg)
{
	switch (m_s3c240x_clkpow_regs[5] & 0x3) // CLKDIVN
	{
		case 0 : return s3c240x_get_fclk(reg) / 1;
		case 1 : return s3c240x_get_fclk(reg) / 1;
		case 2 : return s3c240x_get_fclk(reg) / 2;
		case 3 : return s3c240x_get_fclk(reg) / 2;
	}
	return 0;
}

uint32_t gp32_state::s3c240x_get_pclk(int reg)
{
	switch (m_s3c240x_clkpow_regs[5] & 0x3) // CLKDIVN
	{
		case 0 : return s3c240x_get_fclk(reg) / 1;
		case 1 : return s3c240x_get_fclk(reg) / 2;
		case 2 : return s3c240x_get_fclk(reg) / 2;
		case 3 : return s3c240x_get_fclk(reg) / 4;
	}
	return 0;
}

uint32_t gp32_state::s3c240x_clkpow_r(offs_t offset)
{
	uint32_t data = m_s3c240x_clkpow_regs[offset];
	LOGMASKED(LOG_TRACE, "(CLKPOW) %08X -> %08X %s\n", 0x14800000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_clkpow_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(CLKPOW) %08X <- %08X %s\n", 0x14800000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_clkpow_regs[offset]);
	switch (offset)
	{
		// MPLLCON
		case 0x04 / 4 :
		{
			m_maincpu->set_unscaled_clock(s3c240x_get_fclk(MPLLCON) * CLOCK_MULTIPLIER);
		}
		break;
	}
}

// INTERRUPT CONTROLLER


void gp32_state::s3c240x_check_pending_irq()
{
	if (m_s3c240x_irq_regs[0] != 0)
	{
		uint32_t int_type = 0, temp;
		temp = m_s3c240x_irq_regs[0];
		while (!(temp & 1))
		{
			int_type++;
			temp = temp >> 1;
		}
		m_s3c240x_irq_regs[4] |= (1 << int_type); // INTPND
		m_s3c240x_irq_regs[5] = int_type; // INTOFFSET
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);
	}
}

void gp32_state::s3c240x_request_irq(uint32_t int_type)
{
	LOGMASKED(LOG_MISC, "request irq %d\n", int_type);
	if (m_s3c240x_irq_regs[0] == 0)
	{
		m_s3c240x_irq_regs[0] |= (1 << int_type); // SRCPND
		m_s3c240x_irq_regs[4] |= (1 << int_type); // INTPND
		m_s3c240x_irq_regs[5] = int_type; // INTOFFSET
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
	}
	else
	{
		m_s3c240x_irq_regs[0] |= (1 << int_type); // SRCPND
		s3c240x_check_pending_irq();
	}
}


uint32_t gp32_state::s3c240x_irq_r(offs_t offset)
{
	uint32_t data = m_s3c240x_irq_regs[offset];
	LOGMASKED(LOG_TRACE, "(IRQ) %08X -> %08X %s\n", 0x14400000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_irq_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = m_s3c240x_irq_regs[offset];
	LOGMASKED(LOG_TRACE, "(IRQ) %08X <- %08X %s\n", 0x14400000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_irq_regs[offset]);
	switch (offset)
	{
		// SRCPND
		case 0x00 / 4 :
		{
			m_s3c240x_irq_regs[0] = (old_value & ~data); // clear only the bit positions of SRCPND corresponding to those set to one in the data
			s3c240x_check_pending_irq();
		}
		break;
		// INTPND
		case 0x10 / 4 :
		{
			m_s3c240x_irq_regs[4] = (old_value & ~data); // clear only the bit positions of INTPND corresponding to those set to one in the data
		}
		break;
	}
}

// PWM TIMER

#if 0
static const char *const timer_reg_names[] =
{
	"Timer config 0",
	"Timer config 1",
	"Timer control",
	"Timer count buffer 0",
	"Timer compare buffer 0",
	"Timer count observation 0",
	"Timer count buffer 1",
	"Timer compare buffer 1",
	"Timer count observation 1",
	"Timer count buffer 2",
	"Timer compare buffer 2",
	"Timer count observation 2",
	"Timer count buffer 3",
	"Timer compare buffer 3",
	"Timer count observation 3",
	"Timer count buffer 4",
	"Timer compare buffer 4",
	"Timer count observation 4",
};
#endif


uint32_t gp32_state::s3c240x_pwm_r(offs_t offset)
{
	uint32_t data = m_s3c240x_pwm_regs[offset];
	LOGMASKED(LOG_TRACE, "(PWM) %08X -> %08X %s\n", 0x15100000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_pwm_start(int timer)
{
	static const int mux_table[] = { 2, 4, 8, 16 };
	static const int prescaler_shift[] = { 0, 0, 8, 8, 8 };
	static const int mux_shift[] = { 0, 4, 8, 12, 16 };
	static const int tcon_shift[] = { 0, 8, 12, 16, 20 };
	const uint32_t *regs = &m_s3c240x_pwm_regs[3+timer*3];
	uint32_t prescaler, mux, cnt, cmp, auto_reload;
	double freq, hz;
	LOGMASKED(LOG_STARTSTOP, "PWM %d start\n", timer);
	prescaler = (m_s3c240x_pwm_regs[0] >> prescaler_shift[timer]) & 0xFF;
	mux = (m_s3c240x_pwm_regs[1] >> mux_shift[timer]) & 0x0F;
	freq = s3c240x_get_pclk(MPLLCON) / (prescaler + 1) / mux_table[mux];
	cnt = BITS( regs[0], 15, 0);
	if (timer != 4)
	{
		cmp = BITS( regs[1], 15, 0);
		auto_reload = BIT( m_s3c240x_pwm_regs[2], tcon_shift[timer] + 3);
	}
	else
	{
		cmp = 0;
		auto_reload = BIT( m_s3c240x_pwm_regs[2], tcon_shift[timer] + 2);
	}
	hz = freq / (cnt - cmp + 1);
	LOGMASKED(LOG_MISC, "PWM %d - FCLK=%d HCLK=%d PCLK=%d prescaler=%d div=%d freq=%f cnt=%d cmp=%d auto_reload=%d hz=%f\n", timer, s3c240x_get_fclk(MPLLCON), s3c240x_get_hclk(MPLLCON), s3c240x_get_pclk(MPLLCON), prescaler, mux_table[mux], freq, cnt, cmp, auto_reload, hz);
	if (auto_reload)
	{
		m_s3c240x_pwm_timer[timer]->adjust( attotime::from_hz( hz), timer, attotime::from_hz( hz));
	}
	else
	{
		m_s3c240x_pwm_timer[timer]->adjust( attotime::from_hz( hz), timer);
	}
}

void gp32_state::s3c240x_pwm_stop(int timer)
{
	LOGMASKED(LOG_STARTSTOP, "PWM %d stop\n", timer);
	m_s3c240x_pwm_timer[timer]->adjust( attotime::never);
}

void gp32_state::s3c240x_pwm_recalc(int timer)
{
	static const int tcon_shift[] = { 0, 8, 12, 16, 20 };
	if (m_s3c240x_pwm_regs[2] & (1 << tcon_shift[timer]))
	{
		s3c240x_pwm_start(timer);
	}
	else
	{
		s3c240x_pwm_stop(timer);
	}
}

void gp32_state::s3c240x_pwm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = m_s3c240x_pwm_regs[offset];
	LOGMASKED(LOG_TRACE, "(PWM) %08X <- %08X %s\n", 0x15100000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_pwm_regs[offset]);
	switch (offset)
	{
		// TCON
		case 0x08 / 4 :
		{
			if ((data & 1) != (old_value & 1))
			{
				s3c240x_pwm_recalc(0);
			}
			if ((data & 0x100) != (old_value & 0x100))
			{
				s3c240x_pwm_recalc(1);
			}
			if ((data & 0x1000) != (old_value & 0x1000))
			{
				s3c240x_pwm_recalc(2);
			}
			if ((data & 0x10000) != (old_value & 0x10000))
			{
				s3c240x_pwm_recalc(3);
			}
			if ((data & 0x100000) != (old_value & 0x100000))
			{
				s3c240x_pwm_recalc(4);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(gp32_state::s3c240x_pwm_timer_exp)
{
	int ch = param;
	static const int ch_int[] = { INT_TIMER0, INT_TIMER1, INT_TIMER2, INT_TIMER3, INT_TIMER4 };
	LOGMASKED(LOG_TIMER, "PWM %d timer callback\n", ch);
	if (BITS( m_s3c240x_pwm_regs[1], 23, 20) == (ch + 1))
	{
		s3c240x_dma_request_pwm();
	}
	else
	{
		s3c240x_request_irq(ch_int[ch]);
	}
}

// DMA


void gp32_state::s3c240x_dma_reload(int dma)
{
	uint32_t *regs = &m_s3c240x_dma_regs[dma<<3];
	regs[3] = (regs[3] & ~0x000FFFFF) | BITS( regs[2], 19, 0);
	regs[4] = (regs[4] & ~0x1FFFFFFF) | BITS( regs[0], 28, 0);
	regs[5] = (regs[5] & ~0x1FFFFFFF) | BITS( regs[1], 28, 0);
}

void gp32_state::s3c240x_dma_trigger(int dma)
{
	uint32_t *regs = &m_s3c240x_dma_regs[dma<<3];
	uint32_t curr_tc, curr_src, curr_dst;
	address_space &space = m_maincpu->space( AS_PROGRAM);
	int dsz, inc_src, inc_dst, servmode;
	static const uint32_t ch_int[] = { INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3 };
	LOGMASKED(LOG_MISC, "DMA %d trigger\n", dma);
	curr_tc = BITS( regs[3], 19, 0);
	curr_src = BITS( regs[4], 28, 0);
	curr_dst = BITS( regs[5], 28, 0);
	dsz = BITS( regs[2], 21, 20);
	servmode = BIT( regs[2], 26);
	inc_src = BIT( regs[0], 29);
	inc_dst = BIT( regs[1], 29);
	LOGMASKED(LOG_MISC, "DMA %d - curr_src %08X curr_dst %08X curr_tc %d dsz %d\n", dma, curr_src, curr_dst, curr_tc, dsz);
	while (curr_tc > 0)
	{
		curr_tc--;
		switch (dsz)
		{
			case 0 : space.write_byte( curr_dst, space.read_byte( curr_src)); break;
			case 1 : space.write_word( curr_dst, space.read_word( curr_src)); break;
			case 2 : space.write_dword( curr_dst, space.read_dword( curr_src)); break;
		}
		if (inc_src == 0) curr_src += (1 << dsz);
		if (inc_dst == 0) curr_dst += (1 << dsz);
		if (servmode == 0) break;
	}
	// update curr_src
	regs[4] = (regs[4] & ~0x1FFFFFFF) | curr_src;
	// update curr_dst
	regs[5] = (regs[5] & ~0x1FFFFFFF) | curr_dst;
	// update curr_tc
	regs[3] = (regs[3] & ~0x000FFFFF) | curr_tc;
	// ...
	if (curr_tc == 0)
	{
		int _int, reload;
		reload = BIT( regs[2], 22);
		if (!reload)
		{
			s3c240x_dma_reload(dma);
		}
		else
		{
			regs[6] &= ~(1 << 1); // clear on/off
		}
		_int = BIT( regs[2], 28);
		if (_int)
		{
			s3c240x_request_irq(ch_int[dma]);
		}
	}
}

void gp32_state::s3c240x_dma_request_iis()
{
	uint32_t *regs = &m_s3c240x_dma_regs[2<<3];
	LOGMASKED(LOG_MISC, "s3c240x_dma_request_iis\n");
	if ((BIT( regs[6], 1) != 0) && (BIT( regs[2], 23) != 0) && (BITS( regs[2], 25, 24) == 0))
	{
		s3c240x_dma_trigger(2);
	}
}

void gp32_state::s3c240x_dma_request_pwm()
{
	int i;
	LOGMASKED(LOG_MISC, "s3c240x_dma_request_pwm\n");
	for (i = 0; i < 4; i++)
	{
		if (i != 1)
		{
			uint32_t *regs = &m_s3c240x_dma_regs[i<<3];
			if ((BIT( regs[6], 1) != 0) && (BIT( regs[2], 23) != 0) && (BITS( regs[2], 25, 24) == 3))
			{
				s3c240x_dma_trigger(i);
			}
		}
	}
}

void gp32_state::s3c240x_dma_start(int dma)
{
	uint32_t addr_src, addr_dst, tc;
	uint32_t *regs = &m_s3c240x_dma_regs[dma<<3];
	uint32_t dsz, tsz, reload;
	int inc_src, inc_dst, _int, servmode, swhwsel, hwsrcsel;
	LOGMASKED(LOG_STARTSTOP, "DMA %d start\n", dma);
	addr_src = BITS( regs[0], 28, 0);
	addr_dst = BITS( regs[1], 28, 0);
	tc = BITS( regs[2], 19, 0);
	inc_src = BIT( regs[0], 29);
	inc_dst = BIT( regs[1], 29);
	tsz = BIT( regs[2], 27);
	_int = BIT( regs[2], 28);
	servmode = BIT( regs[2], 26);
	hwsrcsel = BITS( regs[2], 25, 24);
	swhwsel = BIT( regs[2], 23);
	reload = BIT( regs[2], 22);
	dsz = BITS( regs[2], 21, 20);
	LOGMASKED(LOG_MISC, "DMA %d - addr_src %08X inc_src %d addr_dst %08X inc_dst %d int %d tsz %d servmode %d hwsrcsel %d swhwsel %d reload %d dsz %d tc %d\n", dma, addr_src, inc_src, addr_dst, inc_dst, _int, tsz, servmode, hwsrcsel, swhwsel, reload, dsz, tc);
	LOGMASKED(LOG_MISC, "DMA %d - copy %08X bytes from %08X (%s) to %08X (%s)\n", dma, tc << dsz, addr_src, inc_src ? "fix" : "inc", addr_dst, inc_dst ? "fix" : "inc");
	s3c240x_dma_reload(dma);
	if (swhwsel == 0)
	{
		s3c240x_dma_trigger(dma);
	}
}

void gp32_state::s3c240x_dma_stop(int dma)
{
	LOGMASKED(LOG_STARTSTOP, "DMA %d stop\n", dma);
}

void gp32_state::s3c240x_dma_recalc(int dma)
{
	if (m_s3c240x_dma_regs[(dma<<3)+6] & 2)
	{
		s3c240x_dma_start(dma);
	}
	else
	{
		s3c240x_dma_stop(dma);
	}
}

uint32_t gp32_state::s3c240x_dma_r(offs_t offset)
{
	uint32_t data = m_s3c240x_dma_regs[offset];
	LOGMASKED(LOG_TRACE, "(DMA) %08X -> %08X %s\n", 0x14600000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = m_s3c240x_dma_regs[offset];
	LOGMASKED(LOG_TRACE, "(DMA) %08X <- %08X %s\n", 0x14600000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_dma_regs[offset]);
	switch (offset)
	{
		// DCON0
		case 0x08 / 4 :
		{
			if (((data >> 22) & 1) != 0) // reload
			{
				m_s3c240x_dma_regs[0x18/4] &= ~(1 << 1); // clear on/off
			}
		}
		break;
		// DMASKTRIG0
		case 0x18 / 4 :
		{
			if ((old_value & 2) != (data & 2)) s3c240x_dma_recalc(0);
		}
		break;
		// DCON1
		case 0x28 / 4 :
		{
			if (((data >> 22) & 1) != 0) // reload
			{
				m_s3c240x_dma_regs[0x38/4] &= ~(1 << 1); // clear on/off
			}
		}
		break;
		// DMASKTRIG1
		case 0x38 / 4 :
		{
			if ((old_value & 2) != (data & 2)) s3c240x_dma_recalc(1);
		}
		break;
		// DCON2
		case 0x48 / 4 :
		{
			if (((data >> 22) & 1) != 0) // reload
			{
				m_s3c240x_dma_regs[0x58/4] &= ~(1 << 1); // clear on/off
			}
		}
		break;
		// DMASKTRIG2
		case 0x58 / 4 :
		{
			if ((old_value & 2) != (data & 2)) s3c240x_dma_recalc(2);
		}
		break;
		// DCON3
		case 0x68 / 4 :
		{
			if (((data >> 22) & 1) != 0) // reload
			{
				m_s3c240x_dma_regs[0x78/4] &= ~(1 << 1); // clear on/off
			}
		}
		break;
		// DMASKTRIG3
		case 0x78 / 4 :
		{
			if ((old_value & 2) != (data & 2)) s3c240x_dma_recalc(3);
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER(gp32_state::s3c240x_dma_timer_exp)
{
	int ch = param;
	LOGMASKED(LOG_TIMER, "DMA %d timer callback\n", ch);
}

// SMARTMEDIA

void gp32_state::smc_reset()
{
	LOGMASKED(LOG_MISC, "smc_reset\n");
	m_smc.add_latch = 0;
	m_smc.chip = 0;
	m_smc.cmd_latch = 0;
	m_smc.do_read = 0;
	m_smc.do_write = 0;
	m_smc.read = 0;
	m_smc.wp = 0;
	m_smc.busy = 0;
}

void gp32_state::smc_init()
{
	LOGMASKED(LOG_MISC, "smc_init\n");
	smc_reset();
}

uint8_t gp32_state::smc_read()
{
	uint8_t data;
	data = m_smartmedia->data_r();
	LOGMASKED(LOG_MISC, "smc_read %08X\n", data);
	return data;
}

void gp32_state::smc_write(uint8_t data)
{
	LOGMASKED(LOG_MISC, "smc_write %08X\n", data);
	if ((m_smc.chip) && (!m_smc.read))
	{
		if (m_smc.cmd_latch)
		{
			LOGMASKED(LOG_MISC, "smartmedia_command_w %08X\n", data);
			m_smartmedia->command_w(data);
		}
		else if (m_smc.add_latch)
		{
			LOGMASKED(LOG_MISC, "smartmedia_address_w %08X\n", data);
			m_smartmedia->address_w(data);
		}
		else
		{
			LOGMASKED(LOG_MISC, "smartmedia_data_w %08X\n", data);
			m_smartmedia->data_w(data);
		}
	}
}

void gp32_state::smc_update()
{
	if (!m_smc.chip)
	{
		smc_reset();
	}
	else
	{
		if ((m_smc.do_write) && (!m_smc.read))
		{
			smc_write(m_smc.datatx);
		}
		else if ((!m_smc.do_write) && (m_smc.do_read) && (m_smc.read) && (!m_smc.cmd_latch) && (!m_smc.add_latch))
		{
			m_smc.datarx = smc_read();
		}
	}
}

// I2S

#define I2S_L3C ( 1 )
#define I2S_L3M ( 2 )
#define I2S_L3D ( 3 )

void gp32_state::i2s_reset()
{
	LOGMASKED(LOG_MISC, "i2s_reset\n");
	m_i2s.l3d = 0;
	m_i2s.l3m = 0;
	m_i2s.l3c = 0;
}

void gp32_state::i2s_init()
{
	LOGMASKED(LOG_MISC, "i2s_init\n");
	i2s_reset();
}

void gp32_state::i2s_write(int line, int data)
{
	switch (line)
	{
		case I2S_L3C :
		{
			if (data != m_i2s.l3c)
			{
				LOGMASKED(LOG_MISC, "I2S L3C %d\n", data);
				m_i2s.l3c = data;
			}
		}
		break;
		case I2S_L3M :
		{
			if (data != m_i2s.l3m)
			{
				LOGMASKED(LOG_MISC, "I2S L3M %d\n", data);
				m_i2s.l3m = data;
			}
		}
		break;
		case I2S_L3D :
		{
			if (data != m_i2s.l3d)
			{
				LOGMASKED(LOG_MISC, "I2S L3D %d\n", data);
				m_i2s.l3d = data;
			}
		}
		break;
	}
}

// I/O PORT


uint32_t gp32_state::s3c240x_gpio_r(offs_t offset)
{
	uint32_t data = m_s3c240x_gpio[offset];
	switch (offset)
	{
		// PBCON
		case 0x08 / 4 :
		{
			// smartmedia
			data = (data & ~0x00000001);
			if (!m_smc.read) data = data | 0x00000001;
		}
		break;
		// PBDAT
		case 0x0C / 4 :
		{
			// smartmedia
			data = (data & ~0x000000FF) | (m_smc.datarx & 0xFF);
			// buttons
			data = (data & ~0x0000FF00) | (m_io_in0->read() & 0x0000FF00);
		}
		break;
		// PDDAT
		case 0x24 / 4 :
		{
			// smartmedia
			data = (data & ~0x000003C0);
			if (!m_smc.busy) data = data | 0x00000200;
			if (!m_smc.do_read) data = data | 0x00000100;
			if (!m_smc.chip) data = data | 0x00000080;
			if (!m_smartmedia->is_protected()) data = data | 0x00000040;
		}
		break;
		// PEDAT
		case 0x30 / 4 :
		{
			// smartmedia
			data = (data & ~0x0000003C);
			if (m_smc.cmd_latch) data = data | 0x00000020;
			if (m_smc.add_latch) data = data | 0x00000010;
			if (!m_smc.do_write) data = data | 0x00000008;
			if (!m_smartmedia->is_present()) data = data | 0x00000004;
			// buttons
			data = (data & ~0x000000C0) | (m_io_in1->read() & 0x000000C0);
		}
		break;
	}
	LOGMASKED(LOG_TRACE, "(GPIO) %08X -> %08X %s\n", 0x15600000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_s3c240x_gpio[offset]);
	LOGMASKED(LOG_TRACE, "(GPIO) %08X <- %08X %s\n", 0x15600000 + (offset << 2), data, machine().describe_context());
	switch (offset)
	{
		// PBCON
		case 0x08 / 4 :
		{
			// smartmedia
			m_smc.read = ((data & 0x00000001) == 0);
			smc_update();
		}
		break;
		// PBDAT
		case 0x0C / 4 :
		{
			// smartmedia
			m_smc.datatx = data & 0xFF;
		}
		break;
		// PDDAT
		case 0x24 / 4 :
		{
			// smartmedia
			m_smc.do_read = ((data & 0x00000100) == 0);
			m_smc.chip = ((data & 0x00000080) == 0);
			m_smc.wp = ((data & 0x00000040) == 0);
			smc_update();
		}
		break;
		// PEDAT
		case 0x30 / 4 :
		{
			// smartmedia
			m_smc.cmd_latch = ((data & 0x00000020) != 0);
			m_smc.add_latch = ((data & 0x00000010) != 0);
			m_smc.do_write = ((data & 0x00000008) == 0);
			smc_update();
			// sound
			i2s_write(I2S_L3D, (data & 0x00000800) ? 1 : 0);
			i2s_write(I2S_L3M, (data & 0x00000400) ? 1 : 0);
			i2s_write(I2S_L3C, (data & 0x00000200) ? 1 : 0);
		}
		break;
#if 0
		// PGDAT
		case 0x48 / 4 :
		{
			int i2ssdo;
			i2ssdo = BIT( data, 3);
		}
		break;
#endif
	}
}

// MEMORY CONTROLLER


uint32_t gp32_state::s3c240x_memcon_r(offs_t offset)
{
	uint32_t data = m_s3c240x_memcon_regs[offset];
	LOGMASKED(LOG_TRACE, "(MEMCON) %08X -> %08X %s\n", 0x14000000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_memcon_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(MEMCON) %08X <- %08X %s\n", 0x14000000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_memcon_regs[offset]);
}

// USB HOST CONTROLLER


uint32_t gp32_state::s3c240x_usb_host_r(offs_t offset)
{
	uint32_t data = m_s3c240x_usb_host_regs[offset];
	LOGMASKED(LOG_TRACE, "(USB H) %08X -> %08X %s\n", 0x14200000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_usb_host_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(USB H) %08X <- %08X %s\n", 0x14200000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_usb_host_regs[offset]);
}

// UART 0


uint32_t gp32_state::s3c240x_uart_0_r(offs_t offset)
{
	uint32_t data = m_s3c240x_uart_0_regs[offset];
	switch (offset)
	{
		// UTRSTAT0
		case 0x10 / 4 :
		{
			data = (data & ~0x00000006) | 0x00000004 | 0x00000002; // [bit 2] Transmitter empty / [bit 1] Transmit buffer empty
		}
		break;
	}
	LOGMASKED(LOG_TRACE, "(UART 0) %08X -> %08X %s\n", 0x15000000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_uart_0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(UART 0) %08X <- %08X %s\n", 0x15000000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_uart_0_regs[offset]);
}

// UART 1


uint32_t gp32_state::s3c240x_uart_1_r(offs_t offset)
{
	uint32_t data = m_s3c240x_uart_1_regs[offset];
	switch (offset)
	{
		// UTRSTAT1
		case 0x10 / 4 :
		{
			data = (data & ~0x00000006) | 0x00000004 | 0x00000002; // [bit 2] Transmitter empty / [bit 1] Transmit buffer empty
		}
		break;
	}
	LOGMASKED(LOG_TRACE, "(UART 1) %08X -> %08X %s\n", 0x15004000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_uart_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(UART 1) %08X <- %08X %s\n", 0x15004000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_uart_1_regs[offset]);
}

// USB DEVICE


uint32_t gp32_state::s3c240x_usb_device_r(offs_t offset)
{
	uint32_t data = m_s3c240x_usb_device_regs[offset];
	LOGMASKED(LOG_TRACE, "(USB D) %08X -> %08X %s\n", 0x15200140 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_usb_device_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(USB D) %08X <- %08X %s\n", 0x15200140 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_usb_device_regs[offset]);
}

// WATCHDOG TIMER


uint32_t gp32_state::s3c240x_watchdog_r(offs_t offset)
{
	uint32_t data = m_s3c240x_watchdog_regs[offset];
	LOGMASKED(LOG_TRACE, "(WDOG) %08X -> %08X %s\n", 0x15300000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_watchdog_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(WDOG) %08X <- %08X %s\n", 0x15300000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_watchdog_regs[offset]);
}

// EEPROM

uint8_t gp32_state::eeprom_read(uint16_t address)
{
	uint8_t data;
	data = m_eeprom_data[address];
	LOGMASKED(LOG_MISC, "EEPROM %04X -> %02X\n", address, data);
	return data;
}

void gp32_state::eeprom_write(uint16_t address, uint8_t data)
{
	LOGMASKED(LOG_MISC, "EEPROM %04X <- %02X\n", address, data);
	m_eeprom_data[address] = data;
}

// IIC

#if 0
uint8_t gp32_state::i2cmem_read_byte( int last)
{
	uint8_t data = 0;
	int i;
	i2cmem_write( machine, 0, I2CMEM_SDA, 1);
	for (i = 0; i < 8; i++)
	{
		i2cmem_write( machine, 0, I2CMEM_SCL, 1);
		data = (data << 1) + (i2cmem_read( machine, 0, I2CMEM_SDA) ? 1 : 0);
		i2cmem_write( machine, 0, I2CMEM_SCL, 0);
	}
	i2cmem_write( machine, 0, I2CMEM_SDA, last);
	i2cmem_write( machine, 0, I2CMEM_SCL, 1);
	i2cmem_write( machine, 0, I2CMEM_SCL, 0);
	return data;
}
#endif

#if 0
void gp32_state::i2cmem_write_byte( uint8_t data)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		i2cmem_write( machine, 0, I2CMEM_SDA, (data & 0x80) ? 1 : 0);
		data = data << 1;
		i2cmem_write( machine, 0, I2CMEM_SCL, 1);
		i2cmem_write( machine, 0, I2CMEM_SCL, 0);
	}
	i2cmem_write( machine, 0, I2CMEM_SDA, 1); // ack bit
	i2cmem_write( machine, 0, I2CMEM_SCL, 1);
	i2cmem_write( machine, 0, I2CMEM_SCL, 0);
}
#endif

#if 0
void gp32_state::i2cmem_start( )
{
	i2cmem_write( machine, 0, I2CMEM_SDA, 1);
	i2cmem_write( machine, 0, I2CMEM_SCL, 1);
	i2cmem_write( machine, 0, I2CMEM_SDA, 0);
	i2cmem_write( machine, 0, I2CMEM_SCL, 0);
}
#endif

#if 0
void gp32_state::i2cmem_stop( )
{
	i2cmem_write( machine, 0, I2CMEM_SDA, 0);
	i2cmem_write( machine, 0, I2CMEM_SCL, 1);
	i2cmem_write( machine, 0, I2CMEM_SDA, 1);
	i2cmem_write( machine, 0, I2CMEM_SCL, 0);
}
#endif

void gp32_state::iic_start()
{
	LOGMASKED(LOG_STARTSTOP, "IIC start\n");
	m_s3c240x_iic.data_index = 0;
	m_s3c240x_iic_timer->adjust( attotime::from_msec( 1));
}

void gp32_state::iic_stop()
{
	LOGMASKED(LOG_STARTSTOP, "IIC stop\n");
	m_s3c240x_iic_timer->adjust( attotime::never);
}

void gp32_state::iic_resume()
{
	LOGMASKED(LOG_STARTSTOP, "IIC resume\n");
	m_s3c240x_iic_timer->adjust( attotime::from_msec( 1));
}

uint32_t gp32_state::s3c240x_iic_r(offs_t offset)
{
	uint32_t data = m_s3c240x_iic_regs[offset];
	switch (offset)
	{
		// IICSTAT
		case 0x04 / 4 :
		{
			data = data & ~0x0000000F;
		}
		break;
	}
	LOGMASKED(LOG_TRACE, "(IIC) %08X -> %08X %s\n", 0x15400000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_iic_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(IIC) %08X <- %08X %s\n", 0x15400000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_iic_regs[offset]);
	switch (offset)
	{
		// ADDR_IICCON
		case 0x00 / 4 :
		{
			int interrupt_pending_flag;
#if 0
			static const int div_table[] = { 16, 512 };
			int enable_interrupt, transmit_clock_value, tx_clock_source_selection
			double clock;
			transmit_clock_value = (data >> 0) & 0xF;
			tx_clock_source_selection = (data >> 6) & 1;
			enable_interrupt = (data >> 5) & 1;
			clock = (double)(s3c240x_get_pclk(MPLLCON) / div_table[tx_clock_source_selection] / (transmit_clock_value + 1));
#endif
			interrupt_pending_flag = BIT( data, 4);
			if (interrupt_pending_flag == 0)
			{
				int start_stop_condition;
				start_stop_condition = BIT( m_s3c240x_iic_regs[1], 5);
				if (start_stop_condition != 0)
				{
					iic_resume();
				}
			}
		}
		break;
		// IICSTAT
		case 0x04 / 4 :
		{
			int start_stop_condition;
			start_stop_condition = BIT( data, 5);
			if (start_stop_condition != 0)
			{
				iic_start();
			}
			else
			{
				iic_stop();
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER(gp32_state::s3c240x_iic_timer_exp)
{
	int enable_interrupt, mode_selection;
	LOGMASKED(LOG_TIMER, "IIC timer callback\n");
	mode_selection = BITS( m_s3c240x_iic_regs[1], 7, 6);
	switch (mode_selection)
	{
		// master receive mode
		case 2 :
		{
			if (m_s3c240x_iic.data_index == 0)
			{
				uint8_t data_shift = m_s3c240x_iic_regs[3] & 0xFF;
				LOGMASKED(LOG_MISC, "IIC write %02X\n", data_shift);
			}
			else
			{
				uint8_t data_shift = eeprom_read(m_s3c240x_iic.address);
				LOGMASKED(LOG_MISC, "IIC read %02X\n", data_shift);
				m_s3c240x_iic_regs[3] = (m_s3c240x_iic_regs[3] & ~0xFF) | data_shift;
			}
			m_s3c240x_iic.data_index++;
		}
		break;
		// master transmit mode
		case 3 :
		{
			uint8_t data_shift = m_s3c240x_iic_regs[3] & 0xFF;
			LOGMASKED(LOG_MISC, "IIC write %02X\n", data_shift);
			m_s3c240x_iic.data[m_s3c240x_iic.data_index++] = data_shift;
			if (m_s3c240x_iic.data_index == 3)
			{
				m_s3c240x_iic.address = (m_s3c240x_iic.data[1] << 8) | m_s3c240x_iic.data[2];
			}
			if ((m_s3c240x_iic.data_index == 4) && (m_s3c240x_iic.data[0] == 0xA0))
			{
				eeprom_write(m_s3c240x_iic.address, data_shift);
			}
		}
		break;
	}
	enable_interrupt = BIT( m_s3c240x_iic_regs[0], 5);
	if (enable_interrupt)
	{
		s3c240x_request_irq(INT_IIC);
	}
}

// IIS

void gp32_state::s3c240x_iis_start()
{
	static const uint32_t codeclk_table[] = { 256, 384 };
	double freq;
	int prescaler_enable, prescaler_control_a, prescaler_control_b, codeclk;
	LOGMASKED(LOG_STARTSTOP, "IIS start\n");
	prescaler_enable = BIT( m_s3c240x_iis_regs[0], 1);
	prescaler_control_a = BITS( m_s3c240x_iis_regs[2], 9, 5);
	prescaler_control_b = BITS( m_s3c240x_iis_regs[2], 4, 0);
	codeclk = BIT( m_s3c240x_iis_regs[1], 2);
	freq = (double)(s3c240x_get_pclk(MPLLCON) / (prescaler_control_a + 1) / codeclk_table[codeclk]) * 2; // why do I have to multiply by two?
	LOGMASKED(LOG_MISC, "IIS - pclk %d psc_enable %d psc_a %d psc_b %d codeclk %d freq %f\n", s3c240x_get_pclk(MPLLCON), prescaler_enable, prescaler_control_a, prescaler_control_b, codeclk_table[codeclk], freq);
	m_s3c240x_iis_timer->adjust( attotime::from_hz( freq), 0, attotime::from_hz( freq));
}

void gp32_state::s3c240x_iis_stop()
{
	LOGMASKED(LOG_STARTSTOP, "IIS stop\n");
	m_s3c240x_iis_timer->adjust( attotime::never);
}

void gp32_state::s3c240x_iis_recalc()
{
	if (m_s3c240x_iis_regs[0] & 1)
	{
		s3c240x_iis_start();
	}
	else
	{
		s3c240x_iis_stop();
	}
}

uint32_t gp32_state::s3c240x_iis_r(offs_t offset)
{
	uint32_t data = m_s3c240x_iis_regs[offset];
#if 0
	switch (offset)
	{
		// IISCON
		case 0x00 / 4 :
		{
			data = data & ~1; // for mp3 player
		}
		break;
	}
#endif
	LOGMASKED(LOG_TRACE, "(IIS) %08X -> %08X %s\n", 0x15508000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_iis_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = m_s3c240x_iis_regs[offset];
	LOGMASKED(LOG_TRACE, "(IIS) %08X <- %08X %s\n", 0x15508000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_iis_regs[offset]);
	switch (offset)
	{
		// IISCON
		case 0x00 / 4 :
		{
			if ((old_value & 1) != (data & 1)) s3c240x_iis_recalc();
		}
		break;
		// IISFIF
		case 0x10 / 4 :
		{
			if (ACCESSING_BITS_16_31)
			{
				m_s3c240x_iis.fifo[m_s3c240x_iis.fifo_index++] = BITS( data, 31, 16);
			}
			if (ACCESSING_BITS_0_15)
			{
				m_s3c240x_iis.fifo[m_s3c240x_iis.fifo_index++] = BITS( data, 15, 0);
			}
			if (m_s3c240x_iis.fifo_index == 2)
			{
				m_s3c240x_iis.fifo_index = 0;
				m_ldac->write(m_s3c240x_iis.fifo[0]);
				m_rdac->write(m_s3c240x_iis.fifo[1]);
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER(gp32_state::s3c240x_iis_timer_exp)
{
	LOGMASKED(LOG_TIMER, "IIS timer callback\n");
	s3c240x_dma_request_iis();
}

// RTC


uint32_t gp32_state::s3c240x_rtc_r(offs_t offset)
{
	uint32_t data = m_s3c240x_rtc_regs[offset];
	LOGMASKED(LOG_TRACE, "(RTC) %08X -> %08X %s\n", 0x15700040 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(RTC) %08X <- %08X %s\n", 0x15700040 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_rtc_regs[offset]);
}

// A/D CONVERTER


uint32_t gp32_state::s3c240x_adc_r(offs_t offset)
{
	uint32_t data = m_s3c240x_adc_regs[offset];
	LOGMASKED(LOG_TRACE, "(ADC) %08X -> %08X %s\n", 0x15800000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_adc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(ADC) %08X <- %08X %s\n", 0x15800000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_adc_regs[offset]);
}

// SPI


uint32_t gp32_state::s3c240x_spi_r(offs_t offset)
{
	uint32_t data = m_s3c240x_spi_regs[offset];
	LOGMASKED(LOG_TRACE, "(SPI) %08X -> %08X %s\n", 0x15900000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_spi_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(SPI) %08X <- %08X %s\n", 0x15900000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_spi_regs[offset]);
}

// MMC INTERFACE


uint32_t gp32_state::s3c240x_mmc_r(offs_t offset)
{
	uint32_t data = m_s3c240x_mmc_regs[offset];
	LOGMASKED(LOG_TRACE, "(MMC) %08X -> %08X %s\n", 0x15A00000 + (offset << 2), data, machine().describe_context());
	return data;
}

void gp32_state::s3c240x_mmc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACE, "(MMC) %08X <- %08X %s\n", 0x15A00000 + (offset << 2), data, machine().describe_context());
	COMBINE_DATA(&m_s3c240x_mmc_regs[offset]);
}

// ...

void gp32_state::s3c240x_machine_start()
{
	m_s3c240x_pwm_timer[0] = timer_alloc(FUNC(gp32_state::s3c240x_pwm_timer_exp), this);
	m_s3c240x_pwm_timer[1] = timer_alloc(FUNC(gp32_state::s3c240x_pwm_timer_exp), this);
	m_s3c240x_pwm_timer[2] = timer_alloc(FUNC(gp32_state::s3c240x_pwm_timer_exp), this);
	m_s3c240x_pwm_timer[3] = timer_alloc(FUNC(gp32_state::s3c240x_pwm_timer_exp), this);
	m_s3c240x_pwm_timer[4] = timer_alloc(FUNC(gp32_state::s3c240x_pwm_timer_exp), this);
	m_s3c240x_dma_timer[0] = timer_alloc(FUNC(gp32_state::s3c240x_dma_timer_exp), this);
	m_s3c240x_dma_timer[1] = timer_alloc(FUNC(gp32_state::s3c240x_dma_timer_exp), this);
	m_s3c240x_dma_timer[2] = timer_alloc(FUNC(gp32_state::s3c240x_dma_timer_exp), this);
	m_s3c240x_dma_timer[3] = timer_alloc(FUNC(gp32_state::s3c240x_dma_timer_exp), this);
	m_s3c240x_iic_timer = timer_alloc(FUNC(gp32_state::s3c240x_iic_timer_exp), this);
	m_s3c240x_iis_timer = timer_alloc(FUNC(gp32_state::s3c240x_iis_timer_exp), this);
	m_s3c240x_lcd_timer = timer_alloc(FUNC(gp32_state::s3c240x_lcd_timer_exp), this);
	m_eeprom_data = std::make_unique<uint8_t[]>(0x2000); // a dump of the EEPROM (S524AB0X91) resulted to be 0x1000
	m_nvram->set_base(m_eeprom_data.get(), 0x2000);
	smc_init();
	i2s_init();
}

void gp32_state::s3c240x_machine_reset()
{
	smc_reset();
	i2s_reset();
	m_s3c240x_iis.fifo_index = 0;
	m_s3c240x_iic.data_index = 0;
}

void gp32_state::gp32_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x0c000000, 0x0c7fffff).ram().share("s3c240x_ram");
	map(0x14000000, 0x1400003b).rw(FUNC(gp32_state::s3c240x_memcon_r), FUNC(gp32_state::s3c240x_memcon_w));
	map(0x14200000, 0x1420005b).rw(FUNC(gp32_state::s3c240x_usb_host_r), FUNC(gp32_state::s3c240x_usb_host_w));
	map(0x14400000, 0x14400017).rw(FUNC(gp32_state::s3c240x_irq_r), FUNC(gp32_state::s3c240x_irq_w));
	map(0x14600000, 0x1460007b).rw(FUNC(gp32_state::s3c240x_dma_r), FUNC(gp32_state::s3c240x_dma_w));
	map(0x14800000, 0x14800017).rw(FUNC(gp32_state::s3c240x_clkpow_r), FUNC(gp32_state::s3c240x_clkpow_w));
	map(0x14a00000, 0x14a003ff).rw(FUNC(gp32_state::s3c240x_lcd_r), FUNC(gp32_state::s3c240x_lcd_w));
	map(0x14a00400, 0x14a007ff).rw(FUNC(gp32_state::s3c240x_lcd_palette_r), FUNC(gp32_state::s3c240x_lcd_palette_w));
	map(0x15000000, 0x1500002b).rw(FUNC(gp32_state::s3c240x_uart_0_r), FUNC(gp32_state::s3c240x_uart_0_w));
	map(0x15004000, 0x1500402b).rw(FUNC(gp32_state::s3c240x_uart_1_r), FUNC(gp32_state::s3c240x_uart_1_w));
	map(0x15100000, 0x15100043).rw(FUNC(gp32_state::s3c240x_pwm_r), FUNC(gp32_state::s3c240x_pwm_w));
	map(0x15200140, 0x152001fb).rw(FUNC(gp32_state::s3c240x_usb_device_r), FUNC(gp32_state::s3c240x_usb_device_w));
	map(0x15300000, 0x1530000b).rw(FUNC(gp32_state::s3c240x_watchdog_r), FUNC(gp32_state::s3c240x_watchdog_w));
	map(0x15400000, 0x1540000f).rw(FUNC(gp32_state::s3c240x_iic_r), FUNC(gp32_state::s3c240x_iic_w));
	map(0x15508000, 0x15508013).rw(FUNC(gp32_state::s3c240x_iis_r), FUNC(gp32_state::s3c240x_iis_w));
	map(0x15600000, 0x1560005b).rw(FUNC(gp32_state::s3c240x_gpio_r), FUNC(gp32_state::s3c240x_gpio_w));
	map(0x15700040, 0x1570008b).rw(FUNC(gp32_state::s3c240x_rtc_r), FUNC(gp32_state::s3c240x_rtc_w));
	map(0x15800000, 0x15800007).rw(FUNC(gp32_state::s3c240x_adc_r), FUNC(gp32_state::s3c240x_adc_w));
	map(0x15900000, 0x15900017).rw(FUNC(gp32_state::s3c240x_spi_r), FUNC(gp32_state::s3c240x_spi_w));
	map(0x15a00000, 0x15a0003f).rw(FUNC(gp32_state::s3c240x_mmc_r), FUNC(gp32_state::s3c240x_mmc_w));
}

static INPUT_PORTS_START( gp32 )
	PORT_START("IN0")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("R") PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("L") PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B") PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A") PORT_PLAYER(1)
	PORT_START("IN1")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SELECT ) PORT_NAME("SELECT") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("START") PORT_PLAYER(1)
INPUT_PORTS_END

void gp32_state::machine_start()
{
	s3c240x_machine_start();
}

void gp32_state::machine_reset()
{
	s3c240x_machine_reset();
}

void gp32_state::gp32(machine_config &config)
{
	ARM9(config, m_maincpu, 40000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gp32_state::gp32_map);

	PALETTE(config, m_palette).set_entries(32768);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	// TODO: bad setup that theoretically should fail a validation check plus console doesn't have vertical screen anyway
	// TODO: retrieve actual defaults from BIOS
	m_screen->set_size(240, 320);
	m_screen->set_visarea(0, 239, 0, 319);
	m_screen->set_screen_update(FUNC(gp32_state::screen_update_gp32));

	SPEAKER(config, "speaker", 2).front();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 0); // unknown DAC
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1); // unknown DAC

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SMARTMEDIA(config, m_smartmedia, 0);

	SOFTWARE_LIST(config, "memc_list").set_original("gp32");
}

ROM_START( gp32 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "157e", "Firmware 1.5.7 (English)" )
	ROMX_LOAD( "gp32157e.bin", 0x000000, 0x080000, CRC(b1e35643) SHA1(1566bc2a27980602e9eb501cf8b2d62939bfd1e5), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "100k", "Firmware 1.0.0 (Korean)" )
	ROMX_LOAD( "gp32100k.bin", 0x000000, 0x080000, CRC(d9925ac9) SHA1(3604d0d7210ed72eddd3e3e0c108f1102508423c), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "156k", "Firmware 1.5.6 (Korean)" )
	ROMX_LOAD( "gp32156k.bin", 0x000000, 0x080000, CRC(667fb1c8) SHA1(d179ab8e96411272b6a1d683e59da752067f9da8), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "166m", "Firmware 1.6.6 (European)" )
	ROMX_LOAD( "gp32166m.bin", 0x000000, 0x080000, CRC(4548a840) SHA1(1ad0cab0af28fb45c182e5e8c87ead2aaa4fffe1), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "mfv2", "Mr. Spiv Multi Firmware V2" )
	ROMX_LOAD( "gp32mfv2.bin", 0x000000, 0x080000, CRC(7ddaaaeb) SHA1(5a85278f721beb3b00125db5c912d1dc552c5897), ROM_BIOS(4) )

	ROM_REGION( 0x4000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "x2c32.jed", 0, 0x3bbb, CRC(eeec10d8) SHA1(34c4b1b865511517a5de1fa352228d95cda387c5) ) // JEDEC format for the time being. X2C32: 32 Macrocell CoolRunner-II CPLD
ROM_END

CONS(2001, gp32, 0, 0, gp32, gp32, gp32_state, empty_init, "Game Park Holdings", "GP32", ROT270 | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS )
