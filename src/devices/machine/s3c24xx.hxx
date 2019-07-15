// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C2400 / S3C2410 / S3C2440

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "coreutil.h"

/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

//#define UART_PRINTF

#define CLOCK_MULTIPLIER 1

#if defined(DEVICE_S3C2400)

#define S3C24XX_TPAL_GET_TPALEN(x)  BIT(x,16)
#define S3C24XX_TPAL_GET_TPALVAL(x) BITS(x,15,0)

#else

#define S3C24XX_TPAL_GET_TPALEN(x)  BIT(x,24)
#define S3C24XX_TPAL_GET_TPALVAL(x) BITS(x,23,0)

#endif

#define S3C24XX_DCON_GET_TC(x)      BITS(x,19,0)
#define S3C24XX_DCON_GET_DSZ(x)     BITS(x,21,20)
#define S3C24XX_DCON_GET_RELOAD(x)  BIT(x,22)
#define S3C24XX_DCON_GET_SWHWSEL(x) BIT(x,23)

#define S3C24XX_DSTAT_GET_CURR_TC(x)   BITS(x,19,0)
#define S3C24XX_DSTAT_SET_CURR_TC(x,m) (CLR_BITS(x,19,0) | m)

#define S3C24XX_DMASKTRIG_GET_ON_OFF(x) BIT(x,1)

#if defined(DEVICE_S3C2400)

#define S3C24XX_DCON_GET_HWSRCSEL(x) BITS(x,25,24)
#define S3C24XX_DCON_GET_SERVMODE(x) BIT(x,26)
#define S3C24XX_DCON_GET_TSZ(x)      BIT(x,27)
#define S3C24XX_DCON_GET_INT(x)      BIT(x,28)

#define S3C24XX_DISRC_GET_SADDR(x) BITS(x,28,0)

#define S3C24XX_DIDST_GET_DADDR(x) BITS(x,28,0)

#define S3C24XX_DCSRC_GET_CURR_SRC(x)   BITS(x,28,0)
#define S3C24XX_DCSRC_SET_CURR_SRC(x,m) (CLR_BITS(x,28,0) | m)

#define S3C24XX_DCDST_GET_CURR_DST(x)   BITS(x,28,0)
#define S3C24XX_DCDST_SET_CURR_DST(x,m) (CLR_BITS(x,28,0) | m)

#else

#define S3C24XX_DCON_GET_HWSRCSEL(x) BITS(x,26,24)
#define S3C24XX_DCON_GET_SERVMODE(x) BIT(x,27)
#define S3C24XX_DCON_GET_TSZ(x)      BIT(x,28)
#define S3C24XX_DCON_GET_INT(x)      BIT(x,29)

#define S3C24XX_DISRC_GET_SADDR(x) BITS(x,30,0)

#define S3C24XX_DIDST_GET_DADDR(x) BITS(x,30,0)

#define S3C24XX_DCSRC_GET_CURR_SRC(x)   BITS(x,30,0)
#define S3C24XX_DCSRC_SET_CURR_SRC(x,m) (CLR_BITS(x,30,0) | m)

#define S3C24XX_DCDST_GET_CURR_DST(x)   BITS(x,30,0)
#define S3C24XX_DCDST_SET_CURR_DST(x,m) (CLR_BITS(x,30,0) | m)

#endif

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* ... */

void S3C24_CLASS_NAME::s3c24xx_reset()
{
	verboselog( *this, 1, "reset\n");
	m_cpu->reset();
	this->reset();
}

int S3C24_CLASS_NAME::iface_core_pin_r(int pin)
{
	if (!m_pin_r_cb.isnull())
	{
		return (m_pin_r_cb)(pin);
	}
	else
	{
		return 0;
	}
}

/* LCD Controller */

void S3C24_CLASS_NAME::s3c24xx_lcd_reset()
{
	memset( &m_lcd.regs, 0, sizeof( m_lcd.regs));
#if defined(DEVICE_S3C2410)
	m_lcd.regs.lcdintmsk = 3;
	m_lcd.regs.lpcsel = 4;
#elif defined(DEVICE_S3C2440)
	m_lcd.regs.lcdintmsk = 3;
	m_lcd.regs.tconsel = 0x0F84;
#endif
	m_lcd.vramaddr_cur = m_lcd.vramaddr_max = 0;
	m_lcd.offsize = 0;
	m_lcd.pagewidth_cur = m_lcd.pagewidth_max = 0;
	m_lcd.bppmode = 0;
	m_lcd.bswp = m_lcd.hwswp = 0;
	m_lcd.vpos = m_lcd.hpos = 0;
	m_lcd.framerate = 0;
	m_lcd.tpal = 0;
	m_lcd.hpos_min = m_lcd.hpos_max = m_lcd.vpos_min = m_lcd.vpos_max = 0;
	m_lcd.dma_data = m_lcd.dma_bits = 0;
	m_lcd.timer->adjust(attotime::never);
}

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_tft_16(uint16_t data)
{
	if ((m_lcd.regs.lcdcon5 & (1 << 11)) == 0)
	{
		uint8_t r, g, b, i;
		r = (BITS( data, 15, 11) << 3);
		g = (BITS( data, 10, 6) << 3);
		b = (BITS( data, 5, 1) << 3);
		i = BIT( data, 1) << 2;
		return rgb_t( r | i, g | i, b | i);
	}
	else
	{
		uint8_t r, g, b;
		r = BITS( data, 15, 11) << 3;
		g = BITS( data, 10, 5) << 2;
		b = BITS( data, 4, 0) << 3;
		return rgb_t( r, g, b);
	}
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_tft_24(uint32_t data)
{
	uint8_t r, g, b;
	r = BITS( data, 23, 16);
	g = BITS( data, 15, 8);
	b = BITS( data, 7, 0);
	return rgb_t( r, g, b);
}

#endif

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_stn_12(uint16_t data)
{
	uint8_t r, g, b;
	r = BITS( data, 11, 8) << 4;
	g = BITS( data, 7, 4) << 4;
	b = BITS( data, 3, 0) << 4;
	return rgb_t( r, g, b);
}

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_stn_08( uint8_t data)
{
	uint8_t r, g, b;
	r = ((m_lcd.regs.redlut   >> (BITS( data, 7, 5) << 2)) & 0xF) << 4;
	g = ((m_lcd.regs.greenlut >> (BITS( data, 4, 2) << 2)) & 0xF) << 4;
	b = ((m_lcd.regs.bluelut  >> (BITS( data, 1, 0) << 2)) & 0xF) << 4;
	return rgb_t( r, g, b);
}

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_stn_01(uint8_t data)
{
	if ((data & 1) == 0)
	{
		return rgb_t::black();
	}
	else
	{
		return rgb_t::white();
	}
}

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_stn_02(uint8_t data)
{
	uint8_t r, g, b;
	r = g = b = ((m_lcd.regs.bluelut >> (BITS( data, 1, 0) << 2)) & 0xF) << 4;
	return rgb_t( r, g, b);
}

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_stn_04(uint8_t data)
{
	uint8_t r, g, b;
	r = g = b = BITS( data, 3, 0) << 4;
	return rgb_t( r, g, b);
}

rgb_t S3C24_CLASS_NAME::s3c24xx_get_color_tpal()
{
#if defined(DEVICE_S3C2400)
	return s3c24xx_get_color_tft_16(S3C24XX_TPAL_GET_TPALVAL( m_lcd.tpal));
#else
	return s3c24xx_get_color_tft_24(S3C24XX_TPAL_GET_TPALVAL( m_lcd.tpal));
#endif
}

void S3C24_CLASS_NAME::s3c24xx_lcd_dma_reload()
{
	m_lcd.vramaddr_cur = m_lcd.regs.lcdsaddr1 << 1;
	m_lcd.vramaddr_max = ((m_lcd.regs.lcdsaddr1 & 0xFFE00000) | m_lcd.regs.lcdsaddr2) << 1;
	m_lcd.offsize = BITS( m_lcd.regs.lcdsaddr3, 21, 11);
	m_lcd.pagewidth_cur = 0;
	m_lcd.pagewidth_max = BITS( m_lcd.regs.lcdsaddr3, 10, 0);
	if (m_lcd.pagewidth_max == 0)
	{
		if (m_lcd.bppmode == S3C24XX_BPPMODE_STN_12_P)
		{
			m_lcd.pagewidth_max = (m_lcd.hpos_max - m_lcd.hpos_min + 1) / 16 * 12;
		}
	}
	verboselog( *this, 3, "LCD - vramaddr %08X %08X offsize %08X pagewidth %08X\n", m_lcd.vramaddr_cur, m_lcd.vramaddr_max, m_lcd.offsize, m_lcd.pagewidth_max);
	m_lcd.dma_data = 0;
	m_lcd.dma_bits = 0;
}

void S3C24_CLASS_NAME::s3c24xx_lcd_dma_init()
{
	m_lcd.bppmode = BITS( m_lcd.regs.lcdcon1, 4, 1);
	s3c24xx_lcd_dma_reload();
	m_lcd.bswp = BIT( m_lcd.regs.lcdcon5, 1);
	m_lcd.hwswp = BIT( m_lcd.regs.lcdcon5, 0);
	m_lcd.tpal = m_lcd.regs.tpal;
	verboselog( *this, 3, "LCD - bppmode %d hwswp %d bswp %d\n", m_lcd.bppmode, m_lcd.hwswp, m_lcd.bswp);
	m_lcd.dma_data = 0;
	m_lcd.dma_bits = 0;
}

#if 0
uint32_t S3C24_CLASS_NAME::s3c24xx_lcd_dma_read()
{
	address_space& space = m_cpu->memory().space( AS_PROGRAM);
	uint8_t *vram, data[4];
	vram = (uint8_t *)space.get_read_ptr( m_lcd.vramaddr_cur);
	for (int i = 0; i < 2; i++)
	{
		data[i*2+0] = *vram++;
		data[i*2+1] = *vram++;
		m_lcd.vramaddr_cur += 2;
		m_lcd.pagewidth_cur++;
		if (m_lcd.pagewidth_cur >= m_lcd.pagewidth_max)
		{
			m_lcd.vramaddr_cur += m_lcd.offsize << 1;
			m_lcd.pagewidth_cur = 0;
			vram = (uint8_t *)space.get_read_ptr( m_lcd.vramaddr_cur);
		}
	}
	if (m_lcd.hwswp == 0)
	{
		if (m_lcd.bswp == 0)
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
		if (m_lcd.bswp == 0)
		{
			return (data[1] << 24) | (data[0] << 16) | (data[3] << 8) | (data[2] << 0);
		}
		else
		{
			return (data[2] << 24) | (data[3] << 16) | (data[0] << 8) | (data[1] << 0);
		}
	}
}
#endif

uint32_t S3C24_CLASS_NAME::s3c24xx_lcd_dma_read()
{
	address_space& space = m_cpu->memory().space( AS_PROGRAM);
	uint8_t *vram, data[4];
	vram = (uint8_t *)space.get_read_ptr( m_lcd.vramaddr_cur);
	for (int i = 0; i < 2; i++)
	{
		if (m_lcd.hwswp == 0)
		{
			if (m_lcd.bswp == 0)
			{
				if ((m_lcd.vramaddr_cur & 2) == 0)
				{
					data[i*2+0] = *(vram + 3);
					data[i*2+1] = *(vram + 2);
				}
				else
				{
					data[i*2+0] = *(vram - 1);
					data[i*2+1] = *(vram - 2);
				}
			}
			else
			{
				data[i*2+0] = *(vram + 0);
				data[i*2+1] = *(vram + 1);
			}
		}
		else
		{
			if (m_lcd.bswp == 0)
			{
				data[i*2+0] = *(vram + 1);
				data[i*2+1] = *(vram + 0);
			}
			else
			{
				if ((m_lcd.vramaddr_cur & 2) == 0)
				{
					data[i*2+0] = *(vram + 2);
					data[i*2+1] = *(vram + 3);
				}
				else
				{
					data[i*2+0] = *(vram - 2);
					data[i*2+1] = *(vram - 1);
				}
			}
		}
		m_lcd.vramaddr_cur += 2;
		m_lcd.pagewidth_cur++;
		if (m_lcd.pagewidth_cur >= m_lcd.pagewidth_max)
		{
			m_lcd.vramaddr_cur += m_lcd.offsize << 1;
			m_lcd.pagewidth_cur = 0;
			vram = (uint8_t *)space.get_read_ptr( m_lcd.vramaddr_cur);
		}
		else
		{
			vram += 2;
		}
	}
	if (m_flags & S3C24XX_INTERFACE_LCD_REVERSE)
	{
		return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0] << 0);
	}
	else
	{
		return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
	}
}

uint32_t S3C24_CLASS_NAME::s3c24xx_lcd_dma_read_bits(int count)
{
	uint32_t data;
	if (count <= m_lcd.dma_bits)
	{
		m_lcd.dma_bits -= count;
		data = BITS( m_lcd.dma_data, 31, 32 - count);
		m_lcd.dma_data = m_lcd.dma_data << count;
	}
	else
	{
		if (m_lcd.dma_bits == 0)
		{
			if (count == 32)
			{
				data = s3c24xx_lcd_dma_read();
			}
			else
			{
				uint32_t temp = s3c24xx_lcd_dma_read();
				data = BITS( temp, 31, 32 - count);
				m_lcd.dma_data = temp << count;
				m_lcd.dma_bits = 32 - count;
			}
		}
		else
		{
			uint32_t temp = s3c24xx_lcd_dma_read();
			data = (m_lcd.dma_data >> (32 - count)) | BITS( temp, 31, 32 - (count - m_lcd.dma_bits));
			m_lcd.dma_data = temp << (count - m_lcd.dma_bits);
			m_lcd.dma_bits = 32 - (count - m_lcd.dma_bits);
		}
	}
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_tpal()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t color = s3c24xx_get_color_tpal();
	for (int y = m_lcd.vpos_min; y <= m_lcd.vpos_max; y++)
	{
		uint32_t *scanline = &bitmap.pix32(y, m_lcd.hpos_min);
		for (int x = m_lcd.hpos_min; x <= m_lcd.hpos_max; x++)
		{
			*scanline++ = color;
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_stn_01()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 32; j++)
		{
			if (m_flags & S3C24XX_INTERFACE_LCD_REVERSE)
			{
				*scanline++ = s3c24xx_get_color_stn_01( data & 0x01);
				data = data >> 1;
			}
			else
			{
				*scanline++ = s3c24xx_get_color_stn_01((data >> 31) & 0x01);
				data = data << 1;
			}
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 4))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_stn_02()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 16; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_02((data >> 30) & 0x03);
			data = data << 2;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 3))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_stn_04()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 8; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_04((data >> 28) & 0x0F);
			data = data << 4;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 2))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_stn_08()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 4; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_08((data >> 24) & 0xFF);
			data = data << 8;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 1))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_stn_12_p()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 16; i++)
	{
		*scanline++ = s3c24xx_get_color_stn_12(s3c24xx_lcd_dma_read_bits(12));
		m_lcd.hpos++;
		if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max * 16 / 12))
		{
			m_lcd.vpos++;
			if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
			m_lcd.hpos = m_lcd.hpos_min;
			scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_stn_12_u() // not tested
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 2; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_12((data >> 16) & 0x0FFF);
			data = data << 16;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 0))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_tft_01()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 32; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 31) & 0x01);
			data = data << 1;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 4))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_tft_02()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 16; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 30) & 0x03);
			data = data << 2;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 3))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_tft_04()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 8; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 28) & 0x0F);
			data = data << 4;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 2))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_tft_08()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 4; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 24) & 0xFF);
			data = data << 8;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 1))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_render_tft_16()
{
	bitmap_rgb32 &bitmap = *m_lcd.bitmap[0];
	uint32_t *scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		uint32_t data = s3c24xx_lcd_dma_read();
		for (int j = 0; j < 2; j++)
		{
			*scanline++ = s3c24xx_get_color_tft_16((data >> 16) & 0xFFFF);
			data = data << 16;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 0))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max) m_lcd.vpos = m_lcd.vpos_min;
				m_lcd.hpos = m_lcd.hpos_min;
				scanline = &bitmap.pix32(m_lcd.vpos, m_lcd.hpos);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_lcd_timer_exp )
{
	verboselog( *this, 2, "LCD timer callback\n");
	m_lcd.vpos = m_screen->vpos();
	m_lcd.hpos = m_screen->hpos();
	verboselog( *this, 3, "LCD - vpos %d hpos %d\n", m_lcd.vpos, m_lcd.hpos);
	uint32_t tpalen = S3C24XX_TPAL_GET_TPALEN( m_lcd.tpal);
	if (tpalen == 0)
	{
		if (m_lcd.vramaddr_cur >= m_lcd.vramaddr_max)
		{
			s3c24xx_lcd_dma_reload();
		}
		verboselog( *this, 3, "LCD - vramaddr %08X\n", m_lcd.vramaddr_cur);
		while (m_lcd.vramaddr_cur < m_lcd.vramaddr_max)
		{
			switch (m_lcd.bppmode)
			{
			case S3C24XX_BPPMODE_STN_01:    s3c24xx_lcd_render_stn_01();   break;
			case S3C24XX_BPPMODE_STN_02:    s3c24xx_lcd_render_stn_02();   break;
			case S3C24XX_BPPMODE_STN_04:    s3c24xx_lcd_render_stn_04();   break;
			case S3C24XX_BPPMODE_STN_08:    s3c24xx_lcd_render_stn_08();   break;
			case S3C24XX_BPPMODE_STN_12_P:  s3c24xx_lcd_render_stn_12_p(); break;
			case S3C24XX_BPPMODE_STN_12_U:  s3c24xx_lcd_render_stn_12_u(); break;
			case S3C24XX_BPPMODE_TFT_01:    s3c24xx_lcd_render_tft_01();   break;
			case S3C24XX_BPPMODE_TFT_02:    s3c24xx_lcd_render_tft_02();   break;
			case S3C24XX_BPPMODE_TFT_04:    s3c24xx_lcd_render_tft_04();   break;
			case S3C24XX_BPPMODE_TFT_08:    s3c24xx_lcd_render_tft_08();   break;
			case S3C24XX_BPPMODE_TFT_16:    s3c24xx_lcd_render_tft_16();   break;
			default:  verboselog( *this, 0, "s3c24xx_lcd_timer_exp: bppmode %d not supported\n", m_lcd.bppmode); break;
			}
			if ((m_lcd.vpos == m_lcd.vpos_min) && (m_lcd.hpos == m_lcd.hpos_min)) break;
		}
	}
	else
	{
		s3c24xx_lcd_render_tpal();
	}
	m_lcd.timer->adjust(m_screen->time_until_pos(m_lcd.vpos, m_lcd.hpos));
}

void S3C24_CLASS_NAME::s3c24xx_video_start()
{
	m_lcd.bitmap[0] = std::make_unique<bitmap_rgb32>(m_screen->width(), m_screen->height());
	m_lcd.bitmap[1] = std::make_unique<bitmap_rgb32>(m_screen->width(), m_screen->height());
}

void S3C24_CLASS_NAME::bitmap_blend( bitmap_rgb32 &bitmap_dst, bitmap_rgb32 &bitmap_src_1, bitmap_rgb32 &bitmap_src_2)
{
	for (int y = 0; y < bitmap_dst.height(); y++)
	{
		uint32_t *line0 = &bitmap_src_1.pix32(y);
		uint32_t *line1 = &bitmap_src_2.pix32(y);
		uint32_t *line2 = &bitmap_dst.pix32(y);
		for (int x = 0; x < bitmap_dst.width(); x++)
		{
			uint32_t color0 = line0[x];
			uint32_t color1 = line1[x];
			uint16_t r0 = (color0 >> 16) & 0x000000ff;
			uint16_t g0 = (color0 >>  8) & 0x000000ff;
			uint16_t b0 = (color0 >>  0) & 0x000000ff;
			uint16_t r1 = (color1 >> 16) & 0x000000ff;
			uint16_t g1 = (color1 >>  8) & 0x000000ff;
			uint16_t b1 = (color1 >>  0) & 0x000000ff;
			uint8_t r = (uint8_t)((r0 + r1) >> 1);
			uint8_t g = (uint8_t)((g0 + g1) >> 1);
			uint8_t b = (uint8_t)((b0 + b1) >> 1);
			line2[x] = (r << 16) | (g << 8) | b;
		}
	}
}

uint32_t S3C24_CLASS_NAME::s3c24xx_video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_lcd.regs.lcdcon1 & (1 << 0))
	{
		if (m_lcd.framerate >= 1195)
		{
			bitmap_blend( bitmap, *m_lcd.bitmap[0], *m_lcd.bitmap[1]);
			copybitmap( *m_lcd.bitmap[1], *m_lcd.bitmap[0], 0, 0, 0, 0, cliprect);
		}
		else
		{
			copybitmap( bitmap, *m_lcd.bitmap[0], 0, 0, 0, 0, cliprect);
		}
		s3c24xx_lcd_dma_init();
	}
	return 0;
}


READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_lcd_r )
{
	uint32_t data = ((uint32_t*)&m_lcd.regs)[offset];
	switch (offset)
	{
	case S3C24XX_LCDCON1:
		{
			// make sure line counter is going
			uint32_t vpos = m_screen->vpos();
			if (vpos < m_lcd.vpos_min) vpos = m_lcd.vpos_min;
			if (vpos > m_lcd.vpos_max) vpos = m_lcd.vpos_max;
			data = (data & ~0xFFFC0000) | ((m_lcd.vpos_max - vpos) << 18);
		}
		break;
	case S3C24XX_LCDCON5:
		{
			uint32_t vpos = m_screen->vpos();
			data = data & ~0x00018000;
			if (vpos < m_lcd.vpos_min) data = data | 0x00000000;
			if (vpos > m_lcd.vpos_max) data = data | 0x00018000;
			// todo: 00 = VSYNC, 01 = BACK Porch, 10 = ACTIVE, 11 = FRONT Porch
		}
		break;
	}
	verboselog( *this, 9, "(LCD) %08X -> %08X\n", S3C24XX_BASE_LCD + (offset << 2), data);
	return data;
}

int S3C24_CLASS_NAME::s3c24xx_lcd_configure_tft()
{
	verboselog( *this, 5, "s3c24xx_lcd_configure_tft\n");
	uint32_t vspw = BITS( m_lcd.regs.lcdcon2, 5, 0);
	uint32_t vbpd = BITS( m_lcd.regs.lcdcon2, 31, 24);
	uint32_t lineval = BITS( m_lcd.regs.lcdcon2, 23, 14);
	uint32_t vfpd = BITS( m_lcd.regs.lcdcon2, 13, 6);
	uint32_t hspw = BITS( m_lcd.regs.lcdcon4, 7, 0);
	uint32_t hbpd = BITS( m_lcd.regs.lcdcon3, 25, 19);
	uint32_t hfpd = BITS( m_lcd.regs.lcdcon3, 7, 0);
	uint32_t hozval = BITS( m_lcd.regs.lcdcon3, 18, 8);
	uint32_t clkval = BITS( m_lcd.regs.lcdcon1, 17, 8);
	uint32_t hclk = s3c24xx_get_hclk();
	verboselog( *this, 3, "LCD - vspw %d vbpd %d lineval %d vfpd %d hspw %d hbpd %d hfpd %d hozval %d clkval %d hclk %d\n", vspw, vbpd, lineval, vfpd, hspw, hbpd, hfpd, hozval, clkval, hclk);

	double vclk = (double)(hclk / ((clkval + 1) * 2));
	verboselog( *this, 3, "LCD - vclk %f\n", vclk);

	double framerate = vclk / (((vspw + 1) + (vbpd + 1) + (lineval + 1) + (vfpd + 1)) * ((hspw + 1) + (hbpd + 1) + (hozval + 1) + (hfpd + 1)));
	verboselog( *this, 3, "LCD - framerate %f\n", framerate);
	m_lcd.framerate = framerate;

	uint32_t width = (hspw + 1) + (hbpd + 1) + (hozval + 1) + (hfpd + 1);
	uint32_t height = (vspw + 1) + (vbpd + 1) + (lineval + 1) + (vfpd + 1);

	rectangle visarea;
	visarea.min_x = (hspw + 1) + (hbpd + 1);
	visarea.min_y = (vspw + 1) + (vbpd + 1);
	visarea.max_x = visarea.min_x + (hozval + 1) - 1;
	visarea.max_y = visarea.min_y + (lineval + 1) - 1;
	verboselog(*this, 3, "LCD - visarea min_x %d min_y %d max_x %d max_y %d\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	verboselog(*this, 3, "video_screen_configure %d %d %f\n", width, height, m_lcd.framerate);

	m_lcd.hpos_min = (hspw + 1) + (hbpd + 1);
	m_lcd.hpos_max = m_lcd.hpos_min + (hozval + 1) - 1;
	m_lcd.vpos_min = (vspw + 1) + (vbpd + 1);
	m_lcd.vpos_max = m_lcd.vpos_min + (lineval + 1) - 1;
	m_screen->configure(width, height, visarea, HZ_TO_ATTOSECONDS(m_lcd.framerate));
	return true;
}

int S3C24_CLASS_NAME::s3c24xx_lcd_configure_stn()
{
	verboselog( *this, 5, "s3c24xx_lcd_configure_stn\n");

	uint32_t pnrmode = BITS( m_lcd.regs.lcdcon1, 6, 5);
	uint32_t bppmode = BITS( m_lcd.regs.lcdcon1, 4, 1);
	uint32_t clkval = BITS( m_lcd.regs.lcdcon1, 17, 8);
	uint32_t lineval = BITS( m_lcd.regs.lcdcon2, 23, 14);
	uint32_t wdly = BITS( m_lcd.regs.lcdcon3, 20, 19);
	uint32_t hozval = BITS( m_lcd.regs.lcdcon3, 18, 8);
	uint32_t lineblank = BITS( m_lcd.regs.lcdcon3, 7, 0);
	uint32_t wlh = BITS( m_lcd.regs.lcdcon4, 1, 0);
	uint32_t hclk = s3c24xx_get_hclk();
	verboselog( *this, 3, "LCD - pnrmode %d bppmode %d clkval %d lineval %d wdly %d hozval %d lineblank %d wlh %d hclk %d\n", pnrmode, bppmode, clkval, lineval, wdly, hozval, lineblank, wlh, hclk);
	if (clkval == 0)
	{
		return false;
	}

	double vclk = (double)(hclk / ((clkval + 0) * 2));
	verboselog( *this, 3, "LCD - vclk %f\n", vclk);
	double framerate = 1 / (((1 / vclk) * (hozval + 1) + (1 / hclk) * ((1 << (4 + wlh)) + (1 << (4 + wdly)) + (lineblank * 8))) * (lineval + 1));
	verboselog( *this, 3, "LCD - framerate %f\n", framerate);
	m_lcd.framerate = framerate;

	uint32_t width = 0;
	switch (pnrmode)
	{
		case S3C24XX_PNRMODE_STN_04_SS: width = ((hozval + 1) * 4); break;
		case S3C24XX_PNRMODE_STN_04_DS: width = ((hozval + 1) * 4); break;
		case S3C24XX_PNRMODE_STN_08_SS: width = ((hozval + 1) * 8 / 3); break;
		default: break;
	}

	uint32_t height = lineval + 1;

	rectangle visarea;
	visarea.set(0, width - 1, 0, height - 1);
	verboselog( *this, 3, "LCD - visarea min_x %d min_y %d max_x %d max_y %d\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	verboselog( *this, 3, "video_screen_configure %d %d %f\n", width, height, m_lcd.framerate);

	m_lcd.hpos_min = 0;
	m_lcd.hpos_max = width - 1;
	m_lcd.vpos_min = 0;
	m_lcd.vpos_max = height - 1;
	m_screen->configure( width, height, visarea, HZ_TO_ATTOSECONDS( m_lcd.framerate));
	return true;
}

int S3C24_CLASS_NAME::s3c24xx_lcd_configure()
{
	verboselog( *this, 5, "s3c24xx_lcd_configure\n");
	uint32_t bppmode = BITS(m_lcd.regs.lcdcon1, 4, 1);
	if ((bppmode & (1 << 3)) == 0)
	{
		return s3c24xx_lcd_configure_stn();
	}
	else
	{
		return s3c24xx_lcd_configure_tft();
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_start()
{
	verboselog(*this, 1, "LCD start\n");
	if (s3c24xx_lcd_configure())
	{
		s3c24xx_lcd_dma_init();
		m_lcd.timer->adjust(m_screen->time_until_pos(m_lcd.vpos_min, m_lcd.hpos_min));
	}
}

void S3C24_CLASS_NAME::s3c24xx_lcd_stop()
{
	verboselog( *this, 1, "LCD stop\n");
	m_lcd.timer->adjust(attotime::never);
}

void S3C24_CLASS_NAME::s3c24xx_lcd_recalc()
{
	if (m_lcd.regs.lcdcon1 & (1 << 0))
	{
		s3c24xx_lcd_start();
	}
	else
	{
		s3c24xx_lcd_stop();
	}
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_lcd_w )
{
	uint32_t old_value = ((uint32_t*)&m_lcd.regs)[offset];
	verboselog( *this, 9, "(LCD) %08X <- %08X\n", S3C24XX_BASE_LCD + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_lcd.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_LCDCON1 :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				s3c24xx_lcd_recalc();
			}
		}
		break;
	}
}

/* LCD Palette */

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_lcd_palette_r )
{
	uint32_t data = m_lcdpal.regs.data[offset];
	verboselog( *this, 9, "(LCD) %08X -> %08X\n", S3C24XX_BASE_LCDPAL + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_lcd_palette_w )
{
	verboselog( *this, 9, "(LCD) %08X <- %08X\n", S3C24XX_BASE_LCDPAL + (offset << 2), data);
	COMBINE_DATA(&m_lcdpal.regs.data[offset]);
	if (mem_mask != 0xffffffff)
	{
		verboselog( *this, 0, "s3c24xx_lcd_palette_w: unknown mask %08x\n", mem_mask);
	}
	m_palette->set_pen_color( offset, s3c24xx_get_color_tft_16(data & 0xFFFF));
}

/* Clock & Power Management */

void S3C24_CLASS_NAME::s3c24xx_clkpow_reset()
{
	memset( &m_clkpow.regs, 0, sizeof(m_clkpow.regs));
#if defined(DEVICE_S3C2400)
	m_clkpow.regs.locktime = 0x00FFFFFF;
	m_clkpow.regs.mpllcon  = 0x0005C080;
	m_clkpow.regs.upllcon  = 0x00028080;
	m_clkpow.regs.clkcon   = 0x0000FFF8;
#elif defined(DEVICE_S3C2410)
	m_clkpow.regs.locktime = 0x00FFFFFF;
	m_clkpow.regs.mpllcon  = 0x0005C080;
	m_clkpow.regs.upllcon  = 0x00028080;
	m_clkpow.regs.clkcon   = 0x0007FFF0;
#elif defined(DEVICE_S3C2440)
	m_clkpow.regs.locktime = 0xFFFFFFFF;
	m_clkpow.regs.mpllcon  = 0x00096030;
	m_clkpow.regs.upllcon  = 0x0004D030;
	m_clkpow.regs.clkcon   = 0x00FFFFF0;
#endif
	m_clkpow.regs.clkslow = 4;
}

uint32_t S3C24_CLASS_NAME::s3c24xx_get_fclk()
{
	uint32_t mpllcon, clkslow, mdiv, pdiv, sdiv, fclk;
	double temp1, temp2;
	mpllcon = m_clkpow.regs.mpllcon;
	mdiv = BITS( mpllcon, 19, 12);
	pdiv = BITS( mpllcon, 9, 4);
	sdiv = BITS( mpllcon, 1, 0);
#if defined(DEVICE_S3C2400) || defined(DEVICE_S3C2410)
	temp1 = 1 * (mdiv + 8) * (double)clock();
#else
	temp1 = 2 * (mdiv + 8) * (double)clock();
#endif
	temp2 = (double)((pdiv + 2) * (1 << sdiv));
	fclk = (uint32_t)(temp1 / temp2);
	clkslow = m_clkpow.regs.clkslow;
	if (BIT( clkslow, 4) == 1)
	{
		uint32_t slow_val = BITS( clkslow, 2, 0);
		if (slow_val > 0)
		{
			fclk = fclk / (2 * slow_val);
		}
	}
	return fclk;
}

uint32_t S3C24_CLASS_NAME::s3c24xx_get_hclk()
{
#if defined(DEVICE_S3C2400) || defined(DEVICE_S3C2410)
	return s3c24xx_get_fclk() / (BIT( m_clkpow.regs.clkdivn, 1) + 1);
#else
	switch (BITS( m_clkpow.regs.clkdivn, 2, 1))
	{
	case 0: return s3c24xx_get_fclk() / 1;
	case 1: return s3c24xx_get_fclk() / 2;
	case 2: return s3c24xx_get_fclk() / (4 * (BIT( m_clkpow.regs.camdivn, 9) + 1));
	case 3: return s3c24xx_get_fclk() / (3 * (BIT( m_clkpow.regs.camdivn, 8) + 1));
	}
	return 0;
#endif
}

uint32_t S3C24_CLASS_NAME::s3c24xx_get_pclk()
{
	return s3c24xx_get_hclk() / (1 << BIT( m_clkpow.regs.clkdivn, 0));
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_clkpow_r )
{
	uint32_t data = ((uint32_t*)&m_clkpow.regs)[offset];
	verboselog( *this, 9, "(CLKPOW) %08X -> %08X\n", S3C24XX_BASE_CLKPOW + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_clkpow_w )
{
	verboselog( *this, 9, "(CLKPOW) %08X <- %08X\n", S3C24XX_BASE_CLKPOW + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_clkpow.regs)[offset]);
	switch (offset)
	{
	case S3C24XX_MPLLCON :
		verboselog( *this, 5, "CLKPOW - fclk %d hclk %d pclk %d\n", s3c24xx_get_fclk(), s3c24xx_get_hclk(), s3c24xx_get_pclk());
		m_cpu->set_unscaled_clock(s3c24xx_get_fclk() * CLOCK_MULTIPLIER);
		break;
	case S3C24XX_CLKSLOW :
		verboselog( *this, 5, "CLKPOW - fclk %d hclk %d pclk %d\n", s3c24xx_get_fclk(), s3c24xx_get_hclk(), s3c24xx_get_pclk());
		m_cpu->set_unscaled_clock(s3c24xx_get_fclk() * CLOCK_MULTIPLIER);
		break;
	}
}

/* Interrupt Controller */

void S3C24_CLASS_NAME::s3c24xx_irq_reset()
{
	memset(&m_irq.regs, 0, sizeof(m_irq.regs));
	m_irq.line_irq = m_irq.line_fiq = CLEAR_LINE;
	m_irq.regs.intmsk = 0xFFFFFFFF;
	m_irq.regs.priority = 0x7F;
#if defined(DEVICE_S3C2410)
	m_irq.regs.intsubmsk = 0x07FF;
#elif defined(DEVICE_S3C2440)
	m_irq.regs.intsubmsk = 0xFFFF;
#endif
}

void S3C24_CLASS_NAME::s3c24xx_check_pending_irq()
{
	uint32_t temp;
	// normal irq

	if ((m_irq.regs.intpnd == 0) && (m_irq.regs.intoffset == 0)) // without this "touryuu" crashes
	{
		temp = (m_irq.regs.srcpnd & ~m_irq.regs.intmsk) & ~m_irq.regs.intmod;
		if (temp != 0)
		{
			uint32_t int_type = 0;
			verboselog( *this, 5, "srcpnd %08X intmsk %08X intmod %08X\n", m_irq.regs.srcpnd, m_irq.regs.intmsk, m_irq.regs.intmod);
			while ((temp & 1) == 0)
			{
				int_type++;
				temp = temp >> 1;
			}
			verboselog( *this, 5, "intpnd set bit %d\n", int_type);
			m_irq.regs.intpnd |= (1 << int_type);
			m_irq.regs.intoffset = int_type;
			if (m_irq.line_irq != ASSERT_LINE)
			{
				verboselog( *this, 5, "ARM7_IRQ_LINE -> ASSERT_LINE\n");
				m_cpu->execute().set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
				m_irq.line_irq = ASSERT_LINE;
			}
		}
		else
		{
			if (m_irq.line_irq != CLEAR_LINE)
			{
				verboselog( *this, 5, "srcpnd %08X intmsk %08X intmod %08X\n", m_irq.regs.srcpnd, m_irq.regs.intmsk, m_irq.regs.intmod);
				verboselog( *this, 5, "ARM7_IRQ_LINE -> CLEAR_LINE\n");
				m_cpu->execute().set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);
				m_irq.line_irq = CLEAR_LINE;
			}
		}
	}

	// fast irq
	temp = (m_irq.regs.srcpnd & ~m_irq.regs.intmsk) & m_irq.regs.intmod;
	if (temp != 0)
	{
		uint32_t int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		if (m_irq.line_fiq != ASSERT_LINE)
		{
			verboselog( *this, 5, "ARM7_FIRQ_LINE -> ASSERT_LINE\n");
			m_cpu->execute().set_input_line(ARM7_FIRQ_LINE, ASSERT_LINE);
			m_irq.line_fiq = ASSERT_LINE;
		}
	}
	else
	{
		if (m_irq.line_fiq != CLEAR_LINE)
		{
			verboselog( *this, 5, "ARM7_FIRQ_LINE -> CLEAR_LINE\n");
			m_cpu->execute().set_input_line(ARM7_FIRQ_LINE, CLEAR_LINE);
			m_irq.line_fiq = CLEAR_LINE;
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_request_irq(uint32_t int_type)
{
	verboselog( *this, 5, "request irq %d\n", int_type);
	m_irq.regs.srcpnd |= (1 << int_type);
	s3c24xx_check_pending_irq();
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

void S3C24_CLASS_NAME::s3c24xx_check_pending_subirq()
{
	uint32_t temp = m_irq.regs.subsrcpnd & ~m_irq.regs.intsubmsk;
	if (temp != 0)
	{
		uint32_t int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		s3c24xx_request_irq( MAP_SUBINT_TO_INT[int_type]);
	}
}

ATTR_UNUSED void S3C24_CLASS_NAME::s3c24xx_request_subirq( uint32_t int_type)
{
	verboselog( *this, 5, "request subirq %d\n", int_type);
	m_irq.regs.subsrcpnd |= (1 << int_type);
	s3c24xx_check_pending_subirq();
}

void S3C24_CLASS_NAME::s3c24xx_check_pending_eint()
{
	uint32_t temp = m_gpio.regs.eintpend & ~m_gpio.regs.eintmask;
	if (temp != 0)
	{
		uint32_t int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		if (int_type < 8)
		{
			s3c24xx_request_irq(S3C24XX_INT_EINT4_7);
		}
		else
		{
			s3c24xx_request_irq(S3C24XX_INT_EINT8_23);
		}
	}
}

ATTR_UNUSED void S3C24_CLASS_NAME::s3c24xx_request_eint(uint32_t number)
{
	verboselog( *this, 5, "request external interrupt %d\n", number);
	if (number < 4)
	{
		s3c24xx_request_irq( S3C24XX_INT_EINT0 + number);
	}
	else
	{
		m_gpio.regs.eintpend |= (1 << number);
		s3c24xx_check_pending_eint();
	}
}

#endif

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_irq_r )
{
	uint32_t data = ((uint32_t*)&m_irq.regs)[offset];
	verboselog( *this, 9, "(IRQ) %08X -> %08X\n", S3C24XX_BASE_INT + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_irq_w )
{
	uint32_t old_value = ((uint32_t*)&m_irq.regs)[offset];
	verboselog( *this, 9, "(IRQ) %08X <- %08X\n", S3C24XX_BASE_INT + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_irq.regs)[offset]);
	switch (offset)
	{
	case S3C24XX_SRCPND:
		m_irq.regs.srcpnd = (old_value & ~data); // clear only the bit positions of SRCPND corresponding to those set to one in the data
		m_irq.regs.intoffset = 0; // "This bit can be cleared automatically by clearing SRCPND and INTPND."
		s3c24xx_check_pending_irq();
		break;
	case S3C24XX_INTMSK:
		s3c24xx_check_pending_irq();
		break;
	case S3C24XX_INTPND:
		m_irq.regs.intpnd = (old_value & ~data); // clear only the bit positions of INTPND corresponding to those set to one in the data
		m_irq.regs.intoffset = 0; // "This bit can be cleared automatically by clearing SRCPND and INTPND."
		s3c24xx_check_pending_irq();
		break;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	case S3C24XX_SUBSRCPND :
		m_irq.regs.subsrcpnd = (old_value & ~data); // clear only the bit positions of SRCPND corresponding to those set to one in the data
		s3c24xx_check_pending_subirq();
		break;
	case S3C24XX_INTSUBMSK:
		s3c24xx_check_pending_subirq();
		break;
#endif
	}
}

/* PWM Timer */

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_pwm_r )
{
	uint32_t data = ((uint32_t*)&m_pwm.regs)[offset];
	switch (offset)
	{
	case pwm_t::TCNTO0:
		data = (data & ~0x0000FFFF) | m_pwm.calc_observation(0);
		break;
	case pwm_t::TCNTO1:
		data = (data & ~0x0000FFFF) | m_pwm.calc_observation(1);
		break;
	case pwm_t::TCNTO2:
		data = (data & ~0x0000FFFF) | m_pwm.calc_observation(2);
		break;
	case pwm_t::TCNTO3:
		data = (data & ~0x0000FFFF) | m_pwm.calc_observation(3);
		break;
	case pwm_t::TCNTO4:
		data = (data & ~0x0000FFFF) | m_pwm.calc_observation(4);
		break;
	}
	verboselog( *this, 9, "(PWM) %08X -> %08X\n", S3C24XX_BASE_PWM + (offset << 2), data);
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_pwm_start(int timer)
{
	static constexpr int mux_table[] = { 2, 4, 8, 16 };
	static constexpr int prescaler_shift[] = { 0, 0, 8, 8, 8 };
	static constexpr int mux_shift[] = { 0, 4, 8, 12, 16 };
	uint32_t pclk, prescaler, mux, cnt, cmp, auto_reload;
	double freq, hz;
	verboselog( *this, 1, "PWM %d start\n", timer);
	pclk = s3c24xx_get_pclk();
	prescaler = (m_pwm.regs.tcfg0 >> prescaler_shift[timer]) & 0xFF;
	mux = (m_pwm.regs.tcfg1 >> mux_shift[timer]) & 0x0F;
	if (mux < 4)
	{
		freq = (double)pclk / (prescaler + 1) / mux_table[mux];
	}
	else
	{
		// todo
		freq = (double)pclk / (prescaler + 1) / 1;
	}
	switch (timer)
	{
	case 0:
		cnt = BITS(m_pwm.regs.tcntb0, 15, 0);
		cmp = BITS(m_pwm.regs.tcmpb0, 15, 0);
		auto_reload = BIT(m_pwm.regs.tcon, 3);
		break;
	case 1:
		cnt = BITS(m_pwm.regs.tcntb1, 15, 0);
		cmp = BITS(m_pwm.regs.tcmpb1, 15, 0);
		auto_reload = BIT(m_pwm.regs.tcon, 11);
		break;
	case 2:
		cnt = BITS(m_pwm.regs.tcntb2, 15, 0);
		cmp = BITS(m_pwm.regs.tcmpb2, 15, 0);
		auto_reload = BIT(m_pwm.regs.tcon, 15);
		break;
	case 3:
		cnt = BITS(m_pwm.regs.tcntb3, 15, 0);
		cmp = BITS(m_pwm.regs.tcmpb3, 15, 0);
		auto_reload = BIT(m_pwm.regs.tcon, 19);
		break;
	case 4:
		cnt = BITS(m_pwm.regs.tcntb4, 15, 0);
		cmp = 0;
		auto_reload = BIT(m_pwm.regs.tcon, 22);
		break;
	default:
		cnt = cmp = auto_reload = 0;
		break;
	}
//  hz = freq / (cnt - cmp + 1);
	if (cnt < 2)
	{
		hz = freq;
	}
	else
	{
		hz = freq / cnt;
	}
	verboselog(*this, 5, "PWM %d - pclk=%d prescaler=%d div=%d freq=%f cnt=%d cmp=%d auto_reload=%d hz=%f\n", timer, pclk, prescaler, mux_table[mux], freq, cnt, cmp, auto_reload, hz);
	m_pwm.cnt[timer] = cnt;
	m_pwm.cmp[timer] = cmp;
	m_pwm.freq[timer] = freq;
	if (auto_reload)
	{
		m_pwm.timer[timer]->adjust(attotime::from_hz( hz), timer, attotime::from_hz(hz));
	}
	else
	{
		m_pwm.timer[timer]->adjust(attotime::from_hz(hz), timer);
	}
}

void S3C24_CLASS_NAME::s3c24xx_pwm_stop(int timer)
{
	verboselog(*this, 1, "PWM %d stop\n", timer);
	m_pwm.timer[timer]->adjust(attotime::never);
}

void S3C24_CLASS_NAME::s3c24xx_pwm_recalc(int timer)
{
	static constexpr int tcon_shift[] = { 0, 8, 12, 16, 20 };
	if (m_pwm.regs.tcon & (1 << tcon_shift[timer]))
	{
		s3c24xx_pwm_start(timer);
	}
	else
	{
		s3c24xx_pwm_stop(timer);
	}
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_pwm_w )
{
	uint32_t const old_value = ((uint32_t*)&m_pwm.regs)[offset];
	verboselog( *this, 9, "(PWM) %08X <- %08X\n", S3C24XX_BASE_PWM + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_pwm.regs)[offset]);
	switch (offset)
	{
	case pwm_t::TCON:
		if (BIT(data ^ old_value, 0))
		{
			s3c24xx_pwm_recalc(0);
		}
		if (BIT(data ^ old_value, 8))
		{
			s3c24xx_pwm_recalc(1);
		}
		if (BIT(data ^ old_value, 12))
		{
			s3c24xx_pwm_recalc(2);
		}
		if (BIT(data ^ old_value, 16))
		{
			s3c24xx_pwm_recalc(3);
		}
		if (BIT(data ^ old_value, 20))
		{
			s3c24xx_pwm_recalc(4);
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_pwm_timer_exp )
{
	int ch = param;
	static constexpr int ch_int[] = { S3C24XX_INT_TIMER0, S3C24XX_INT_TIMER1, S3C24XX_INT_TIMER2, S3C24XX_INT_TIMER3, S3C24XX_INT_TIMER4 };
	verboselog(*this, 2, "PWM %d timer callback\n", ch);
	if (BITS(m_pwm.regs.tcfg1, 23, 20) == (ch + 1))
	{
		s3c24xx_dma_request_pwm();
	}
	else
	{
		s3c24xx_request_irq(ch_int[ch]);
	}
}

/* DMA */

void S3C24_CLASS_NAME::s3c24xx_dma_reset()
{
	for (dma_t &dma : m_dma)
	{
		memset(&dma.regs, 0, sizeof(dma.regs));
		dma.timer->adjust(attotime::never);
	}
}

void S3C24_CLASS_NAME::s3c24xx_dma_reload(int ch)
{
	dma_regs_t *regs = &m_dma[ch].regs;
	regs->dstat = S3C24XX_DSTAT_SET_CURR_TC(regs->dstat, S3C24XX_DCON_GET_TC(regs->dcon));
	regs->dcsrc = S3C24XX_DCSRC_SET_CURR_SRC(regs->dcsrc, S3C24XX_DISRC_GET_SADDR(regs->disrc));
	regs->dcdst = S3C24XX_DCDST_SET_CURR_DST(regs->dcdst, S3C24XX_DIDST_GET_DADDR(regs->didst));
}

void S3C24_CLASS_NAME::s3c24xx_dma_trigger(int ch)
{
	dma_regs_t *regs = &m_dma[ch].regs;
	uint32_t curr_tc, curr_src, curr_dst;
	address_space &space = m_cpu->memory().space(AS_PROGRAM);
	int dsz, inc_src, inc_dst, servmode, tsz;
	static constexpr uint32_t ch_int[] = { S3C24XX_INT_DMA0, S3C24XX_INT_DMA1, S3C24XX_INT_DMA2, S3C24XX_INT_DMA3 };
	verboselog(*this, 5, "DMA %d trigger\n", ch);
	curr_tc = S3C24XX_DSTAT_GET_CURR_TC(regs->dstat);
	dsz = S3C24XX_DCON_GET_DSZ(regs->dcon);
	curr_src = S3C24XX_DCSRC_GET_CURR_SRC(regs->dcsrc);
	curr_dst = S3C24XX_DCDST_GET_CURR_DST(regs->dcdst);
	servmode = S3C24XX_DCON_GET_SERVMODE(regs->dcon);
	tsz = S3C24XX_DCON_GET_TSZ(regs->dcon);
#if defined(DEVICE_S3C2400)
	inc_src = BIT(regs->disrc, 29);
	inc_dst = BIT(regs->didst, 29);
#else
	inc_src = BIT( regs->disrcc, 0);
	inc_dst = BIT(regs->didstc, 0);
#endif
	verboselog(*this, 5, "DMA %d - curr_src %08X curr_dst %08X curr_tc %d dsz %d\n", ch, curr_src, curr_dst, curr_tc, dsz);
	while (curr_tc > 0)
	{
		curr_tc--;
		for (int i = 0; i < 1 << (tsz << 1); i++)
		{
			switch (dsz)
			{
			case 0: space.write_byte(curr_dst, space.read_byte( curr_src)); break;
			case 1: space.write_word(curr_dst, space.read_word( curr_src)); break;
			case 2: space.write_dword(curr_dst, space.read_dword( curr_src)); break;
			}
			if (inc_src == 0) curr_src += (1 << dsz);
			if (inc_dst == 0) curr_dst += (1 << dsz);
		}
		if (servmode == 0) break;
	}
	regs->dcsrc = S3C24XX_DCSRC_SET_CURR_SRC(regs->dcsrc, curr_src);
	regs->dcdst = S3C24XX_DCDST_SET_CURR_DST(regs->dcdst, curr_dst);
	regs->dstat = S3C24XX_DSTAT_SET_CURR_TC(regs->dstat, curr_tc);
	if (curr_tc == 0)
	{
		if (S3C24XX_DCON_GET_RELOAD(regs->dcon) == 0)
		{
			s3c24xx_dma_reload(ch);
		}
		else
		{
			regs->dmasktrig &= ~(1 << 1); // clear on/off
		}
		if (S3C24XX_DCON_GET_INT(regs->dcon) != 0)
		{
			s3c24xx_request_irq(ch_int[ch]);
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_dma_request_iis()
{
	dma_regs_t *regs = &m_dma[2].regs;
	verboselog( *this, 5, "s3c24xx_dma_request_iis\n");
	if ((S3C24XX_DMASKTRIG_GET_ON_OFF(regs->dmasktrig) != 0) && (S3C24XX_DCON_GET_SWHWSEL(regs->dcon) != 0) && (S3C24XX_DCON_GET_HWSRCSEL(regs->dcon) == 0))
		s3c24xx_dma_trigger(2);
}

void S3C24_CLASS_NAME::s3c24xx_dma_request_pwm()
{
	verboselog( *this, 5, "s3c24xx_dma_request_pwm\n");
	for (int i = 0; i < 4; i++)
	{
		if (i != 1)
		{
			dma_regs_t *regs = &m_dma[i].regs;
			if ((S3C24XX_DMASKTRIG_GET_ON_OFF(regs->dmasktrig) != 0) && (S3C24XX_DCON_GET_SWHWSEL(regs->dcon) != 0) && (S3C24XX_DCON_GET_HWSRCSEL(regs->dcon) == 3))
			{
				s3c24xx_dma_trigger(i);
			}
		}
	}
}

void S3C24_CLASS_NAME::s3c24xx_dma_start(int ch)
{
	uint32_t addr_src, addr_dst, tc;
	dma_regs_t *regs = &m_dma[ch].regs;
	uint32_t dsz, tsz, reload;
	int inc_src, inc_dst, _int, servmode, swhwsel, hwsrcsel;
	verboselog(*this, 1, "DMA %d start\n", ch);
	addr_src = S3C24XX_DISRC_GET_SADDR(regs->disrc);
	addr_dst = S3C24XX_DIDST_GET_DADDR(regs->didst);
	tc = S3C24XX_DCON_GET_TC(regs->dcon);
	_int = S3C24XX_DCON_GET_INT(regs->dcon);
	servmode = S3C24XX_DCON_GET_SERVMODE(regs->dcon);
	hwsrcsel = S3C24XX_DCON_GET_HWSRCSEL(regs->dcon);
	swhwsel = S3C24XX_DCON_GET_SWHWSEL(regs->dcon);
	reload = S3C24XX_DCON_GET_RELOAD(regs->dcon);
	dsz = S3C24XX_DCON_GET_DSZ(regs->dcon);
	tsz = S3C24XX_DCON_GET_TSZ(regs->dcon);
#if defined(DEVICE_S3C2400)
	inc_src = BIT(regs->disrc, 29);
	inc_dst = BIT(regs->didst, 29);
#else
	inc_src = BIT(regs->disrcc, 0);
	inc_dst = BIT(regs->didstc, 0);
#endif
	verboselog(*this, 5, "DMA %d - addr_src %08X inc_src %d addr_dst %08X inc_dst %d int %d tsz %d servmode %d hwsrcsel %d swhwsel %d reload %d dsz %d tc %d\n", ch, addr_src, inc_src, addr_dst, inc_dst, _int, tsz, servmode, hwsrcsel, swhwsel, reload, dsz, tc);
	verboselog(*this, 5, "DMA %d - copy %08X bytes from %08X (%s) to %08X (%s)\n", ch, (tc << dsz) << (tsz << 1), addr_src, inc_src ? "fix" : "inc", addr_dst, inc_dst ? "fix" : "inc");
	s3c24xx_dma_reload(ch);
	if (swhwsel == 0)
		s3c24xx_dma_trigger(ch);
}

void S3C24_CLASS_NAME::s3c24xx_dma_stop(int ch)
{
	verboselog(*this, 1, "DMA %d stop\n", ch);
}

void S3C24_CLASS_NAME::s3c24xx_dma_recalc(int ch)
{
	if ((m_dma[ch].regs.dmasktrig & (1 << 1)) != 0)
		s3c24xx_dma_start(ch);
	else
		s3c24xx_dma_stop(ch);
}

uint32_t S3C24_CLASS_NAME::s3c24xx_dma_r(uint32_t ch, uint32_t offset)
{
	return ((uint32_t*)&m_dma[ch].regs)[offset];
}

void S3C24_CLASS_NAME::s3c24xx_dma_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_dma[ch].regs)[offset];
	COMBINE_DATA(&((uint32_t*)&m_dma[ch].regs)[offset]);
	switch (offset)
	{
	case S3C24XX_DCON :
#if 0 // is this code necessary ???
		if (BIT(data, 22)) // reload
		{
			dma_regs_t *regs = &m_dma[ch].regs;
			regs->dmasktrig &= ~(1 << 1); // clear on/off
		}
#endif
		break;
	case S3C24XX_DMASKTRIG :
		if (BIT(data ^ old_value, 1))
			s3c24xx_dma_recalc(ch);
		break;
	}
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_0_r )
{
	uint32_t data = s3c24xx_dma_r( 0, offset);
	verboselog(*this, 9, "(DMA 0) %08X -> %08X\n", S3C24XX_BASE_DMA_0 + (offset << 2), data);
	return data;
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_1_r )
{
	uint32_t data = s3c24xx_dma_r(1, offset);
	verboselog(*this, 9, "(DMA 1) %08X -> %08X\n", S3C24XX_BASE_DMA_1 + (offset << 2), data);
	return data;
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_2_r )
{
	uint32_t data = s3c24xx_dma_r( 2, offset);
	verboselog(*this, 9, "(DMA 2) %08X -> %08X\n", S3C24XX_BASE_DMA_2 + (offset << 2), data);
	return data;
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_3_r )
{
	uint32_t data = s3c24xx_dma_r( 3, offset);
	verboselog(*this, 9, "(DMA 3) %08X -> %08X\n", S3C24XX_BASE_DMA_3 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_0_w )
{
	verboselog(*this, 9, "(DMA 0) %08X <- %08X\n", S3C24XX_BASE_DMA_0 + (offset << 2), data);
	s3c24xx_dma_w(0, offset, data, mem_mask);
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_1_w )
{
	verboselog(*this, 9, "(DMA 1) %08X <- %08X\n", S3C24XX_BASE_DMA_1 + (offset << 2), data);
	s3c24xx_dma_w(1, offset, data, mem_mask);
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_2_w )
{
	verboselog(*this, 9, "(DMA 2) %08X <- %08X\n", S3C24XX_BASE_DMA_2 + (offset << 2), data);
	s3c24xx_dma_w(2, offset, data, mem_mask);
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_3_w )
{
	verboselog(*this, 9, "(DMA 3) %08X <- %08X\n", S3C24XX_BASE_DMA_3 + (offset << 2), data);
	s3c24xx_dma_w(3, offset, data, mem_mask);
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_dma_timer_exp )
{
	int ch = param;
	verboselog(*this, 2, "DMA %d timer callback\n", ch);
}

/* I/O Port */

void S3C24_CLASS_NAME::s3c24xx_gpio_reset()
{
	memset(&m_gpio.regs, 0, sizeof(m_gpio.regs));
#if defined(DEVICE_S3C2400)
	m_gpio.regs.gpacon = 0x0003FFFF;
	m_gpio.regs.gpbcon = 0xAAAAAAAA;
	m_gpio.regs.gpdup = 0x0620;
	m_gpio.regs.gpeup = 0x0003;
#elif defined(DEVICE_S3C2410)
	m_gpio.regs.gpacon = 0x007FFFFF;
	m_gpio.regs.gpgup = 0xF800;
	m_gpio.regs.misccr = 0x00010330;
	m_gpio.regs.eintmask = 0x00FFFFF0;
	m_gpio.regs.gstatus1 = 0x32410002;
#elif defined(DEVICE_S3C2440)
	m_gpio.regs.gpacon = 0x00FFFFFF;
	m_gpio.regs.gpgup = 0xFC00;
	m_gpio.regs.misccr = 0x00010020;
	m_gpio.regs.eintmask = 0x000FFFFF;
	m_gpio.regs.gstatus1 = 0x32440001;
#endif
	m_gpio.regs.gpdup = 0xF000;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	m_gpio.regs.gstatus2 = 1 << 0; // Boot is caused by power on reset
#endif
}

uint32_t S3C24_CLASS_NAME::iface_gpio_port_r(int port, uint32_t mask)
{
	if (!m_port_r_cb.isnull())
	{
		// TO CHECK : masking is not done in any of handlers
		// devcb do it automatically so guess is masks are not proper right now
		// without masking works fine
		return (m_port_r_cb)( port ); //, mask);
	}
	else
	{
		return 0;
	}
}

void S3C24_CLASS_NAME::iface_gpio_port_w(int port, uint32_t mask, uint32_t data)
{
	if (!m_port_w_cb.isnull())
	{
		(m_port_w_cb)( port, data, mask );
	}
}

uint16_t S3C24_CLASS_NAME::s3c24xx_gpio_get_mask( uint32_t con, int val)
{
	uint16_t mask = 0;
	for (int i = 0; i < 16; i++)
	{
		if (((con >> (i << 1)) & 3) == val)
			mask = mask | (1 << i);
	}
	return mask;
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_gpio_r )
{
	uint32_t data = ((uint32_t*)&m_gpio.regs)[offset];
	switch (offset)
	{
	case S3C24XX_GPADAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_A, 0) & S3C24XX_GPADAT_MASK;
		break;
	case S3C24XX_GPBDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_B, s3c24xx_gpio_get_mask(m_gpio.regs.gpbcon, 0) & S3C24XX_GPBDAT_MASK) & S3C24XX_GPBDAT_MASK;
		break;
	case S3C24XX_GPCDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_C, s3c24xx_gpio_get_mask(m_gpio.regs.gpccon, 0) & S3C24XX_GPCDAT_MASK) & S3C24XX_GPCDAT_MASK;
		break;
	case S3C24XX_GPDDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_D, s3c24xx_gpio_get_mask(m_gpio.regs.gpdcon, 0) & S3C24XX_GPDDAT_MASK) & S3C24XX_GPDDAT_MASK;
		break;
	case S3C24XX_GPEDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_E, s3c24xx_gpio_get_mask(m_gpio.regs.gpecon, 0) & S3C24XX_GPEDAT_MASK) & S3C24XX_GPEDAT_MASK;
		break;
	case S3C24XX_GPFDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_F, s3c24xx_gpio_get_mask(m_gpio.regs.gpfcon, 0) & S3C24XX_GPFDAT_MASK) & S3C24XX_GPFDAT_MASK;
		break;
	case S3C24XX_GPGDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_G, s3c24xx_gpio_get_mask(m_gpio.regs.gpgcon, 0) & S3C24XX_GPGDAT_MASK) & S3C24XX_GPGDAT_MASK;
		break;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	case S3C24XX_GPHDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_H, s3c24xx_gpio_get_mask(m_gpio.regs.gphcon, 0) & S3C24XX_GPHDAT_MASK) & S3C24XX_GPHDAT_MASK;
		break;
#endif
#if defined(DEVICE_S3C2440)
	case S3C24XX_GPJDAT :
		data = iface_gpio_port_r( S3C24XX_GPIO_PORT_J, s3c24xx_gpio_get_mask(m_gpio.regs.gpjcon, 0) & S3C24XX_GPJDAT_MASK) & S3C24XX_GPJDAT_MASK;
		break;
#endif
	}
	verboselog(*this, 9, "(GPIO) %08X -> %08X\n", S3C24XX_BASE_GPIO + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_gpio_w )
{
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	uint32_t old_value = ((uint32_t*)&m_gpio.regs)[offset];
#endif
	verboselog(*this, 9, "(GPIO) %08X <- %08X\n", S3C24XX_BASE_GPIO + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_gpio.regs)[offset]);
	switch (offset)
	{
	case S3C24XX_GPADAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_A, m_gpio.regs.gpacon ^ 0xFFFFFFFF, data & S3C24XX_GPADAT_MASK);
		break;
	case S3C24XX_GPBDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_B, s3c24xx_gpio_get_mask(m_gpio.regs.gpbcon, 1) & S3C24XX_GPBDAT_MASK, data & S3C24XX_GPBDAT_MASK);
		break;
	case S3C24XX_GPCDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_C, s3c24xx_gpio_get_mask(m_gpio.regs.gpccon, 1) & S3C24XX_GPCDAT_MASK, data & S3C24XX_GPCDAT_MASK);
		break;
	case S3C24XX_GPDDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_D, s3c24xx_gpio_get_mask(m_gpio.regs.gpdcon, 1) & S3C24XX_GPDDAT_MASK, data & S3C24XX_GPDDAT_MASK);
		break;
	case S3C24XX_GPEDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_E, s3c24xx_gpio_get_mask(m_gpio.regs.gpecon, 1) & S3C24XX_GPEDAT_MASK, data & S3C24XX_GPEDAT_MASK);
		break;
	case S3C24XX_GPFDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_F, s3c24xx_gpio_get_mask(m_gpio.regs.gpfcon, 1) & S3C24XX_GPFDAT_MASK, data & S3C24XX_GPFDAT_MASK);
		break;
	case S3C24XX_GPGDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_G, s3c24xx_gpio_get_mask(m_gpio.regs.gpgcon, 1) & S3C24XX_GPGDAT_MASK, data & S3C24XX_GPGDAT_MASK);
		break;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	case S3C24XX_GPHDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_H, s3c24xx_gpio_get_mask(m_gpio.regs.gphcon, 1) & S3C24XX_GPHDAT_MASK, data & S3C24XX_GPHDAT_MASK);
		break;
	case S3C24XX_EINTPEND:
		m_gpio.regs.eintpend = (old_value & ~data);
		s3c24xx_check_pending_eint();
		break;
	case S3C24XX_EINTMASK:
		s3c24xx_check_pending_eint();
		break;
	case S3C24XX_GSTATUS2:
		m_gpio.regs.gstatus2 = (old_value & ~data) & 7; // "The setting is cleared by writing '1' to this bit"
		break;
#endif
#if defined(DEVICE_S3C2440)
	case S3C24XX_GPJDAT:
		iface_gpio_port_w(S3C24XX_GPIO_PORT_J, s3c24xx_gpio_get_mask(m_gpio.regs.gpjcon, 1) & S3C24XX_GPJDAT_MASK, data & S3C24XX_GPJDAT_MASK);
		break;
#endif
	}
}

/* Memory Controller */

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_memcon_r )
{
	assert(offset < ARRAY_LENGTH(m_memcon.regs.data));
	uint32_t data = m_memcon.regs.data[offset];
	verboselog(*this, 9, "(MEMCON) %08X -> %08X\n", S3C24XX_BASE_MEMCON + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_memcon_w )
{
	verboselog(*this, 9, "(MEMCON) %08X <- %08X\n", S3C24XX_BASE_MEMCON + (offset << 2), data);
	COMBINE_DATA(&m_memcon.regs.data[offset]);
}

/* USB Host Controller */

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_usb_host_r )
{
	uint32_t data = m_usbhost.regs.data[offset];
	switch (offset)
	{
		// HcCommandStatus
		case 0x08 / 4 :
		{
			data = data & ~(1 << 0); // [bit 0] HostControllerReset
		}
		break;
		// HcPeriodStart
		case 0x40 / 4:
		{
			// "After a hardware reset, this field is cleared. This is then set by"
			// "HCD during the HC initialization. The value is calculated"
			// "roughly as 10% off from HcFmInterval.. A typical value will be 3E67h."
			data = (data & ~0x00003FFF) | 0x3E67;
		}
		break;
		// HcRhDescriptorA
		case 0x48 / 4:
		{
			data = (data & ~0xFF) | 2; // number of ports
		}
		break;
		// HcRhStatus
		case 0x50 / 4:
		{
			data = data & ~(1 << 16); // "The Root Hub does not support the local power status feature; thus, this bit is always read as ?0?."
		}
		break;
	}
	verboselog(*this, 9, "(USB H) %08X -> %08X\n", S3C24XX_BASE_USBHOST + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_usb_host_w )
{
	verboselog(*this, 9, "(USB H) %08X <- %08X\n", S3C24XX_BASE_USBHOST + (offset << 2), data);
	COMBINE_DATA(&m_usbhost.regs.data[offset]);
}

/* UART */

uint32_t S3C24_CLASS_NAME::s3c24xx_uart_r(uint32_t ch, uint32_t offset)
{
	uint32_t data = ((uint32_t*)&m_uart[ch].regs)[offset];
	switch (offset)
	{
	case uart_t::UTRSTAT:
		{
			data = (data & ~0x00000006) | 0x00000004 | 0x00000002; // [bit 2] Transmitter empty / [bit 1] Transmit buffer empty
		}
		break;
	case uart_t::URXH:
		{
			uint8_t rxdata = data & 0xFF;
			verboselog(*this, 5, "UART %d read %02X (%c)\n", ch, rxdata, ((rxdata >= 32) && (rxdata < 128)) ? (char)rxdata : '?');
			m_uart[ch].regs.utrstat &= ~1; // [bit 0] Receive buffer data ready
		}
		break;
	}
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_uart_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&((uint32_t*)&m_uart[ch].regs)[offset]);
	switch (offset)
	{
	case uart_t::UFCON :
		{
			m_uart[ch].regs.ufcon &= ~((1 << 2) | (1 << 1)); // bits 1 and 2 are auto-cleared after resetting FIFO
		}
		break;
	case uart_t::UTXH :
		{
			uint8_t txdata = data & 0xFF;
			verboselog( *this, 5, "UART %d write %02X (%c)\n", ch, txdata, ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#ifdef UART_PRINTF
			printf("%c", ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#endif
		}
		break;
	}
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_uart_0_r )
{
	uint32_t data = s3c24xx_uart_r(0, offset);
//  verboselog( *this, 9, "(UART 0) %08X -> %08X\n", S3C24XX_BASE_UART_0 + (offset << 2), data);
	return data;
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_uart_1_r )
{
	uint32_t data = s3c24xx_uart_r(1, offset);
//  verboselog( *this, 9, "(UART 1) %08X -> %08X\n", S3C24XX_BASE_UART_1 + (offset << 2), data);
	return data;
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_uart_2_r )
{
	uint32_t data = s3c24xx_uart_r(2, offset);
//  verboselog( *this, 9, "(UART 2) %08X -> %08X\n", S3C24XX_BASE_UART_2 + (offset << 2), data);
	return data;
}

#endif

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_uart_0_w )
{
//  verboselog( *this, 9, "(UART 0) %08X <- %08X\n", S3C24XX_BASE_UART_0 + (offset << 2), data);
	s3c24xx_uart_w(0, offset, data, mem_mask);
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_uart_1_w )
{
//  verboselog( *this, 9, "(UART 1) %08X <- %08X\n", S3C24XX_BASE_UART_1 + (offset << 2), data);
	s3c24xx_uart_w(1, offset, data, mem_mask);
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_uart_2_w )
{
//  verboselog( *this, 9, "(UART 2) %08X <- %08X\n", S3C24XX_BASE_UART_2 + (offset << 2), data);
	s3c24xx_uart_w(2, offset, data, mem_mask);
}

#endif

void S3C24_CLASS_NAME::s3c24xx_uart_fifo_w(int uart, uint8_t data)
{
//  printf( "s3c24xx_uart_fifo_w (%c)\n", data);
	m_uart[uart].regs.urxh = data;
	m_uart[uart].regs.utrstat |= 1; // [bit 0] Receive buffer data ready
}

/* USB Device */

void S3C24_CLASS_NAME::s3c24xx_usb_device_reset()
{
	memset(&m_usbdev.regs, 0, sizeof(m_usbdev.regs));
#if defined(DEVICE_S3C2400)
	m_usbdev.regs.data[0x0C/4] = 0x033F;
	m_usbdev.regs.data[0x14/4] = 0x000A;
	m_usbdev.regs.data[0x24/4] = 0x0001;
	m_usbdev.regs.data[0x44/4] = 0x0001;
	m_usbdev.regs.data[0x54/4] = 0x0001;
	m_usbdev.regs.data[0x64/4] = 0x0001;
	m_usbdev.regs.data[0x74/4] = 0x0001;
	m_usbdev.regs.data[0xB8/4] = 0x00FF;
#elif defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	m_usbdev.regs.data[0x1C/4] = 0xFF;
	m_usbdev.regs.data[0x2C/4] = 0x04;
	m_usbdev.regs.data[0x40/4] = 0x01;
	m_usbdev.regs.data[0x48/4] = 0x20;
#endif
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_usb_device_r )
{
	uint32_t data = m_usbdev.regs.data[offset];
	verboselog(*this, 9, "(USB D) %08X -> %08X\n", S3C24XX_BASE_USBDEV + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_usb_device_w )
{
	verboselog(*this, 9, "(USB D) %08X <- %08X\n", S3C24XX_BASE_USBDEV + (offset << 2), data);
	COMBINE_DATA(&m_usbdev.regs.data[offset]);
}

/* Watchdog Timer */

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_wdt_r )
{
	uint32_t data = ((uint32_t*)&m_wdt.regs)[offset];
	switch (offset)
	{
	case wdt_t::WTCNT:
		// is wdt active?
		if (BIT(m_wdt.regs.wtcon, 5))
		{
#if defined(DEVICE_S3C2410)
			data = m_wdt.calc_current_count();
#else
			data = 0;
#endif
		}
		break;
	}
	verboselog(*this, 9, "(WDT) %08X -> %08X\n", S3C24XX_BASE_WDT + (offset << 2), data);
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_wdt_start()
{
	uint32_t pclk, prescaler, clock;
	double freq, hz;
	verboselog(*this, 1, "WDT start\n");
	pclk = s3c24xx_get_pclk();
	prescaler = BITS(m_wdt.regs.wtcon, 15, 8);
	clock = 16 << BITS(m_wdt.regs.wtcon, 4, 3);
	freq = (double)pclk / (prescaler + 1) / clock;
	hz = freq / m_wdt.regs.wtcnt;
	verboselog(*this, 5, "WDT pclk %d prescaler %d clock %d freq %f hz %f\n", pclk, prescaler, clock, freq, hz);
	m_wdt.timer->adjust( attotime::from_hz( hz), 0, attotime::from_hz( hz));
#if defined(DEVICE_S3C2410)
	m_wdt.freq = freq;
	m_wdt.cnt = m_wdt.regs.wtcnt;
#endif
}

void S3C24_CLASS_NAME::s3c24xx_wdt_stop()
{
	verboselog( *this, 1, "WDT stop\n");
#if defined(DEVICE_S3C2410)
	m_wdt.regs.wtcnt = m_wdt.calc_current_count();
#else
	m_wdt.regs.wtcnt = 0;
#endif
	m_wdt.timer->adjust(attotime::never);
}

void S3C24_CLASS_NAME::s3c24xx_wdt_recalc()
{
	if (BIT(m_wdt.regs.wtcon, 5))
		s3c24xx_wdt_start();
	else
		s3c24xx_wdt_stop();
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_wdt_w )
{
	uint32_t old_value = ((uint32_t*)&m_wdt.regs)[offset];
	verboselog( *this, 9, "(WDT) %08X <- %08X\n", S3C24XX_BASE_WDT + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_wdt.regs)[offset]);
	switch (offset)
	{
	case wdt_t::WTCON :
		if (BIT(data ^ old_value, 5))
			s3c24xx_wdt_recalc();
		break;
	}
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_wdt_timer_exp )
{
	verboselog( *this, 2, "WDT timer callback\n");
	if (BIT(m_wdt.regs.wtcon, 2))
	{
#if defined(DEVICE_S3C2400) || defined(DEVICE_S3C2410)
		s3c24xx_request_irq(S3C24XX_INT_WDT);
#else
		s3c24xx_request_subirq(S3C24XX_SUBINT_WDT);
#endif
	}
	if (BIT(m_wdt.regs.wtcon, 0))
	{
		s3c24xx_reset();
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
		m_gpio.regs.gstatus2 = 1 << 2; // Watchdog reset
#endif
	}
}

/* IIC */

void S3C24_CLASS_NAME::s3c24xx_iic_reset()
{
	memset(&m_iic.regs, 0, sizeof(m_iic.regs));
	m_iic.count = 0;
	m_iic.timer->adjust(attotime::never);
}

void S3C24_CLASS_NAME::iface_i2c_scl_w( int state)
{
	if (!m_scl_w_cb.isnull())
		m_scl_w_cb(state);
}

void S3C24_CLASS_NAME::iface_i2c_sda_w(int state)
{
	if (!m_sda_w_cb.isnull())
		m_sda_w_cb(state);
}

int S3C24_CLASS_NAME::iface_i2c_sda_r()
{
	if (!m_sda_r_cb.isnull())
		return m_sda_r_cb();
	else
		return 1;
}

void S3C24_CLASS_NAME::i2c_send_start()
{
	// FIXME: this needs to sense busy condition and use realistic timing
	verboselog(*this, 5, "i2c_send_start\n");
	iface_i2c_sda_w(1);
	iface_i2c_scl_w(1);
	iface_i2c_sda_w(0);
	iface_i2c_scl_w(0);
}

void S3C24_CLASS_NAME::i2c_send_stop()
{
	// FIXME: this needs realistic timing
	verboselog(*this, 5, "i2c_send_stop\n");
	iface_i2c_sda_w(0);
	iface_i2c_scl_w(1);
	iface_i2c_sda_w(1);
	iface_i2c_scl_w(0);
}

uint8_t S3C24_CLASS_NAME::i2c_receive_byte(int ack)
{
	uint8_t data = 0;
	verboselog(*this, 5, "i2c_receive_byte ...\n");
	iface_i2c_sda_w(1);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_scl_w( 1);
		data = (data << 1) + (iface_i2c_sda_r() ? 1 : 0);
		iface_i2c_scl_w( 0);
	}
	verboselog(*this, 5, "recv data %02X\n", data);
	verboselog(*this, 5, "send ack %d\n", ack);
	iface_i2c_sda_w(ack ? 0 : 1);
	iface_i2c_scl_w(1);
	iface_i2c_scl_w(0);
	return data;
}

int S3C24_CLASS_NAME::i2c_send_byte(uint8_t data)
{
	int ack;
	verboselog(*this, 5, "i2c_send_byte ...\n");
	verboselog(*this, 5, "send data %02X\n", data);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_sda_w((data & 0x80) ? 1 : 0);
		data = data << 1;
		iface_i2c_scl_w(1);
		iface_i2c_scl_w(0);
	}
	iface_i2c_sda_w(1); // ack bit
	iface_i2c_scl_w(1);
	ack = iface_i2c_sda_r();
	verboselog(*this, 5, "recv ack %d\n", ack);
	iface_i2c_scl_w(0);
	return ack;
}

void S3C24_CLASS_NAME::iic_start()
{
	int mode_selection;
	verboselog(*this, 1, "IIC start\n");
	i2c_send_start();
	mode_selection = BITS( m_iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2: i2c_send_byte( m_iic.regs.iicds | 0x01); break;
		case 3: i2c_send_byte( m_iic.regs.iicds & 0xFE); break;
	}
	m_iic.timer->adjust( attotime::from_usec( 1));
}

void S3C24_CLASS_NAME::iic_stop()
{
	verboselog(*this, 1, "IIC stop\n");
	i2c_send_stop();
	m_iic.timer->adjust(attotime::never);
}

void S3C24_CLASS_NAME::iic_resume()
{
	int mode_selection;
	verboselog(*this, 1, "IIC resume\n");
	mode_selection = BITS(m_iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2: m_iic.regs.iicds = i2c_receive_byte(BIT( m_iic.regs.iiccon, 7)); break;
		case 3: i2c_send_byte( m_iic.regs.iicds & 0xFF); break;
	}
	m_iic.timer->adjust(attotime::from_usec(1));
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_iic_r )
{
	uint32_t data = ((uint32_t*)&m_iic.regs)[offset];
	switch (offset)
	{
	case S3C24XX_IICSTAT:
		data = data & ~0x0000000F;
		break;
	}
	verboselog(*this, 9, "(IIC) %08X -> %08X\n", S3C24XX_BASE_IIC + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_iic_w )
{
	uint32_t old_value = ((uint32_t*)&m_iic.regs)[offset];
	verboselog(*this, 9, "(IIC) %08X <- %08X\n", S3C24XX_BASE_IIC + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_iic.regs)[offset]);
	switch (offset)
	{
	case S3C24XX_IICCON:
		{
			int interrupt_pending_flag;
#if 0
			static constexpr int div_table[] = { 16, 512 };
			int enable_interrupt, transmit_clock_value, tx_clock_source_selection
			double clock;
			transmit_clock_value = (data >> 0) & 0xF;
			tx_clock_source_selection = (data >> 6) & 1;
			enable_interrupt = (data >> 5) & 1;
			clock = (double)s3c24xx_get_pclk() / div_table[tx_clock_source_selection] / (transmit_clock_value + 1);
#endif
			interrupt_pending_flag = BIT(old_value, 4);
			if (interrupt_pending_flag != 0)
			{
				interrupt_pending_flag = BIT(data, 4);
				if (interrupt_pending_flag == 0)
				{
					int start_stop_condition;
					start_stop_condition = BIT(m_iic.regs.iicstat, 5);
					if (start_stop_condition != 0)
					{
						if (m_iic.count == 0)
						{
							iic_start();

						}
						else
						{
							iic_resume();
						}
					}
					else
					{
						iic_stop();
					}
				}
			}
		}
		break;
	case S3C24XX_IICSTAT:
		{
			int interrupt_pending_flag;
			m_iic.count = 0;
			interrupt_pending_flag = BIT( m_iic.regs.iiccon, 4);
			if (interrupt_pending_flag == 0)
			{
				int start_stop_condition;
				start_stop_condition = BIT( data, 5);
				if (start_stop_condition != 0)
				{
					if (m_iic.count == 0)
					{
						iic_start();

					}
					else
					{
						iic_resume();
					}
				}
				else
				{
					iic_stop();
				}
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_iic_timer_exp )
{
	int enable_interrupt;
	verboselog(*this, 2, "IIC timer callback\n");
	m_iic.count++;
	enable_interrupt = BIT(m_iic.regs.iiccon, 5);
	if (enable_interrupt)
	{
		m_iic.regs.iiccon |= (1 << 4); // [bit 4] interrupt is pending
		s3c24xx_request_irq(S3C24XX_INT_IIC);
	}
}

/* IIS */

void S3C24_CLASS_NAME::iface_i2s_data_w(int ch, uint16_t data)
{
	if (!m_data_w_cb.isnull())
		(m_data_w_cb)(ch, data, 0);
}

void S3C24_CLASS_NAME::s3c24xx_iis_start()
{
	const uint32_t codeclk_table[] = { 256, 384};
	double freq;
	int pclk, prescaler_enable, prescaler_control_a, prescaler_control_b, codeclk;
	verboselog( *this, 1, "IIS start\n");
	prescaler_enable = BIT( m_iis.regs.iiscon, 1);
	prescaler_control_a = BITS( m_iis.regs.iispsr, 9, 5);
	prescaler_control_b = BITS( m_iis.regs.iispsr, 4, 0);
	codeclk = BIT( m_iis.regs.iismod, 2);
	pclk = s3c24xx_get_pclk();
	freq = ((double)pclk / (prescaler_control_a + 1) / codeclk_table[codeclk]) * 2; // why do I have to multiply by two?
	verboselog( *this, 5, "IIS - pclk %d psc_enable %d psc_a %d psc_b %d codeclk %d freq %f\n", pclk, prescaler_enable, prescaler_control_a, prescaler_control_b, codeclk_table[codeclk], freq);
	m_iis.timer->adjust( attotime::from_hz( freq), 0, attotime::from_hz( freq));
}

void S3C24_CLASS_NAME::s3c24xx_iis_stop()
{
	verboselog( *this, 1, "IIS stop\n");
	m_iis.timer->adjust( attotime::never);
}

void S3C24_CLASS_NAME::s3c24xx_iis_recalc()
{
	if (BIT(m_iis.regs.iiscon, 0))
		s3c24xx_iis_start();
	else
		s3c24xx_iis_stop();
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_iis_r )
{
	uint32_t data = ((uint32_t*)&m_iis.regs)[offset];
#if 0
	switch (offset)
	{
	case iis_t::IISCON :
		data = data & ~1; // hack for mp3 player
		break;
	}
#endif
	verboselog(*this, 9, "(IIS) %08X -> %08X\n", S3C24XX_BASE_IIS + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_iis_w )
{
	uint32_t old_value = ((uint32_t*)&m_iis.regs)[offset];
	verboselog(*this, 9, "(IIS) %08X <- %08X\n", S3C24XX_BASE_IIS + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_iis.regs)[offset]);
	switch (offset)
	{
	case iis_t::IISCON :
		if (BIT(data ^ old_value, 0))
			s3c24xx_iis_recalc();
		break;
	case iis_t::IISFIFO :
		if (ACCESSING_BITS_16_31)
		{
			m_iis.fifo[m_iis.fifo_index++] = BITS(data, 31, 16);
		}
		if (ACCESSING_BITS_0_15)
		{
			m_iis.fifo[m_iis.fifo_index++] = BITS(data, 15, 0);
		}
		if (m_iis.fifo_index == 2)
		{
			m_iis.fifo_index = 0;
			iface_i2s_data_w(0, m_iis.fifo[0]);
			iface_i2s_data_w(1, m_iis.fifo[1]);
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_iis_timer_exp )
{
	verboselog(*this, 2, "IIS timer callback\n");
	s3c24xx_dma_request_iis();
}

/* RTC */

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_rtc_r )
{
	uint32_t data = ((uint32_t*)&m_rtc.regs)[offset];
	verboselog( *this, 9, "(RTC) %08X -> %08X\n", S3C24XX_BASE_RTC + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_rtc_w )
{
	verboselog( *this, 9, "(RTC) %08X <- %08X\n", S3C24XX_BASE_RTC + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_rtc.regs)[offset]);
	switch (offset)
	{
	case rtc_t::TICNT :
		m_rtc.recalc();
		break;
	}
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_rtc_timer_tick_count_exp )
{
	verboselog(*this, 2, "RTC timer callback (tick count)\n");
	s3c24xx_request_irq(S3C24XX_INT_TICK);
}

void S3C24_CLASS_NAME::s3c24xx_rtc_update()
{
	m_rtc.update();
	verboselog( *this, 5, "RTC - %04d/%02d/%02d %02d:%02d:%02d\n", bcd_2_dec( m_rtc.regs.bcdyear) + 2000, bcd_2_dec( m_rtc.regs.bcdmon), bcd_2_dec( m_rtc.regs.bcdday), bcd_2_dec( m_rtc.regs.bcdhour), bcd_2_dec( m_rtc.regs.bcdmin), bcd_2_dec( m_rtc.regs.bcdsec));
}

void S3C24_CLASS_NAME::s3c24xx_rtc_check_alarm()
{
	if (m_rtc.check_alarm())
		s3c24xx_request_irq(S3C24XX_INT_RTC);
}

TIMER_CALLBACK_MEMBER( S3C24_CLASS_NAME::s3c24xx_rtc_timer_update_exp )
{
	verboselog(*this, 2, "RTC timer callback (update)\n");
	s3c24xx_rtc_update();
	s3c24xx_rtc_check_alarm();
}

/* A/D Converter */

void S3C24_CLASS_NAME::s3c24xx_adc_reset()
{
	memset(&m_adc.regs, 0, sizeof(m_adc.regs));
	m_adc.regs.adccon = 0x3FC4;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	m_adc.regs.adctsc = 0x58;
	m_adc.regs.adcdly = 0xFF;
#endif
}

uint32_t S3C24_CLASS_NAME::iface_adc_data_r(int ch)
{
	if (!m_data_r_cb.isnull())
	{
		int offs = ch;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
		if (BIT(m_adc.regs.adctsc, 2) != 0)
		{
			offs += 2;
		}
#endif
		return m_data_r_cb(offs, 0);
	}
	else
	{
		return 0;
	}
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_adc_r )
{
	uint32_t data = ((uint32_t*)&m_adc.regs)[offset];
	switch (offset)
	{
#if defined(DEVICE_S3C2400)
	case S3C24XX_ADCDAT:
		data = (data & ~0x3FF) | (iface_adc_data_r(0) & 0x3FF);
		break;
#else
	case S3C24XX_ADCDAT0:
		data = (data & ~0x3FF) | (iface_adc_data_r(0) & 0x3FF);
		break;
	case S3C24XX_ADCDAT1:
		data = (data & ~0x3FF) | (iface_adc_data_r(1) & 0x3FF);
		break;
#endif
	}
	verboselog(*this, 9, "(ADC) %08X -> %08X\n", S3C24XX_BASE_ADC + (offset << 2), data);
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_adc_start()
{
	verboselog(*this, 1, "ADC start\n");
	m_adc.regs.adccon &= ~(1 << 0); // A/D conversion is completed
	m_adc.regs.adccon |= (1 << 15); // End of A/D conversion
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	s3c24xx_request_subirq(S3C24XX_SUBINT_ADC);
#endif
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_adc_w )
{
	uint32_t old_value = ((uint32_t*)&m_adc.regs)[offset];
	verboselog(*this, 9, "(ADC) %08X <- %08X\n", S3C24XX_BASE_ADC + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_adc.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_ADCCON :
		{
			if (((old_value & (1 << 0)) == 0) && ((data & (1 << 0)) != 0))
			{
				s3c24xx_adc_start();
			}
		}
		break;
	}
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

void S3C24_CLASS_NAME::s3c24xx_touch_screen(int state)
{
	m_adc.regs.adcdat0 = ((state ? 0 : 1) << 15);
	m_adc.regs.adcdat1 = ((state ? 0 : 1) << 15);
	s3c24xx_request_subirq(S3C24XX_SUBINT_TC);
}

#endif

/* SPI */

void S3C24_CLASS_NAME::s3c24xx_spi_reset()
{
	for (spi_t &spi : m_spi)
	{
		memset(&spi.regs, 0, sizeof(spi.regs));
		spi.regs.spsta = 1;
#if defined(DEVICE_S3C2400) || defined(DEVICE_S3C2410)
		spi.regs.sppin = 2;
#endif
	}
}

uint32_t S3C24_CLASS_NAME::s3c24xx_spi_r(uint32_t ch, uint32_t offset)
{
	uint32_t data = ((uint32_t*)&m_spi[ch].regs)[offset];
	switch (offset)
	{
	case spi_t::SPSTA :
		data = data | (1 << 0); // [bit 0] Transfer Ready Flag
		break;
	}
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_spi_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&((uint32_t*)&m_spi[ch].regs)[offset]);
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_spi_0_r )
{
	uint32_t data = s3c24xx_spi_r(0, offset);
	verboselog(*this, 9, "(SPI 0) %08X -> %08X\n", S3C24XX_BASE_SPI_0 + (offset << 2), data);
	return data;
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_spi_1_r )
{
	uint32_t data = s3c24xx_spi_r(1, offset);
	verboselog(*this, 9, "(SPI 1) %08X -> %08X\n", S3C24XX_BASE_SPI_1 + (offset << 2), data);
	return data;
}

#endif

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_spi_0_w )
{
	verboselog( *this, 9, "(SPI 0) %08X <- %08X\n", S3C24XX_BASE_SPI_0 + (offset << 2), data);
	s3c24xx_spi_w( 0, offset, data, mem_mask);
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_spi_1_w )
{
	verboselog( *this, 9, "(SPI 1) %08X <- %08X\n", S3C24XX_BASE_SPI_1 + (offset << 2), data);
	s3c24xx_spi_w( 1, offset, data, mem_mask);
}

#endif

/* MMC Interface */

#if defined(DEVICE_S3C2400)

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_mmc_r )
{
	uint32_t data = m_mmc.regs.data[offset];
	verboselog(*this, 9, "(MMC) %08X -> %08X\n", S3C24XX_BASE_MMC + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_mmc_w )
{
	verboselog(*this, 9, "(MMC) %08X <- %08X\n", S3C24XX_BASE_MMC + (offset << 2), data);
	COMBINE_DATA(&m_mmc.regs.data[offset]);
}

#endif

/* SD Interface */

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

void S3C24_CLASS_NAME::s3c24xx_sdi_reset()
{
	memset(&m_sdi.regs, 0, sizeof(m_sdi.regs));
#if defined(DEVICE_S3C2410)
	m_sdi.regs.data[0x24/4] = 0x2000;
#elif defined(DEVICE_S3C2440)
	m_sdi.regs.data[0x04/4] = 1;
	m_sdi.regs.data[0x24/4] = 0x10000;
#endif
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_sdi_r )
{
	uint32_t data = m_sdi.regs.data[offset];
	verboselog( *this, 9, "(SDI) %08X -> %08X\n", S3C24XX_BASE_SDI + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_sdi_w )
{
	verboselog( *this, 9, "(SDI) %08X <- %08X\n", S3C24XX_BASE_SDI + (offset << 2), data);
	COMBINE_DATA(&m_sdi.regs.data[offset]);
}

#endif

/* NAND Flash */

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

void S3C24_CLASS_NAME::s3c24xx_nand_reset()
{
	memset(&m_nand.regs, 0, sizeof(m_nand.regs));
#if defined(DEVICE_S3C2440)
	m_nand.regs.nfconf = 0x1000;
	m_nand.regs.nfcont = 0x0384;
#endif
}

void S3C24_CLASS_NAME::iface_nand_command_w(uint8_t data)
{
	if (!m_command_w_cb.isnull())
	{
		m_command_w_cb(0, data, 0xff);
	}
}

void S3C24_CLASS_NAME::iface_nand_address_w(uint8_t data)
{
	if (!m_address_w_cb.isnull())
	{
		m_address_w_cb(0, data, 0xff);
	}
}

uint8_t S3C24_CLASS_NAME::iface_nand_data_r()
{
	if (!m_nand_data_r_cb.isnull())
		return m_nand_data_r_cb(0, 0xff);
	else
		return 0;
}

void S3C24_CLASS_NAME::iface_nand_data_w(uint8_t data)
{
	if (!m_nand_data_w_cb.isnull())
		m_nand_data_w_cb(0, data, 0xff);
}

void S3C24_CLASS_NAME::nand_update_mecc(uint8_t *ecc, int pos, uint8_t data)
{
	int bit[8];
	uint8_t temp;
	bit[0] = (data >> 0) & 1;
	bit[1] = (data >> 1) & 1;
	bit[2] = (data >> 2) & 1;
	bit[3] = (data >> 3) & 1;
	bit[4] = (data >> 4) & 1;
	bit[5] = (data >> 5) & 1;
	bit[6] = (data >> 6) & 1;
	bit[7] = (data >> 7) & 1;
	// column parity
	ecc[2] ^= ((bit[6] ^ bit[4] ^ bit[2] ^ bit[0]) << 2);
	ecc[2] ^= ((bit[7] ^ bit[5] ^ bit[3] ^ bit[1]) << 3);
	ecc[2] ^= ((bit[5] ^ bit[4] ^ bit[1] ^ bit[0]) << 4);
	ecc[2] ^= ((bit[7] ^ bit[6] ^ bit[3] ^ bit[2]) << 5);
	ecc[2] ^= ((bit[3] ^ bit[2] ^ bit[1] ^ bit[0]) << 6);
	ecc[2] ^= ((bit[7] ^ bit[6] ^ bit[5] ^ bit[4]) << 7);
	// line parity
	temp = bit[7] ^ bit[6] ^ bit[5] ^ bit[4] ^ bit[3] ^ bit[2] ^ bit[1] ^ bit[0];
	if (pos & 0x001) ecc[0] ^= (temp << 1); else ecc[0] ^= (temp << 0);
	if (pos & 0x002) ecc[0] ^= (temp << 3); else ecc[0] ^= (temp << 2);
	if (pos & 0x004) ecc[0] ^= (temp << 5); else ecc[0] ^= (temp << 4);
	if (pos & 0x008) ecc[0] ^= (temp << 7); else ecc[0] ^= (temp << 6);
	if (pos & 0x010) ecc[1] ^= (temp << 1); else ecc[1] ^= (temp << 0);
	if (pos & 0x020) ecc[1] ^= (temp << 3); else ecc[1] ^= (temp << 2);
	if (pos & 0x040) ecc[1] ^= (temp << 5); else ecc[1] ^= (temp << 4);
	if (pos & 0x080) ecc[1] ^= (temp << 7); else ecc[1] ^= (temp << 6);
	if (pos & 0x100) ecc[2] ^= (temp << 1); else ecc[2] ^= (temp << 0);
	if (pos & 0x200) ecc[3] ^= (temp << 5); else ecc[3] ^= (temp << 4);
	if (pos & 0x400) ecc[3] ^= (temp << 7); else ecc[3] ^= (temp << 6);
}

#if defined(DEVICE_S3C2440)

void S3C24_CLASS_NAME::nand_update_secc( uint8_t *ecc, int pos, uint8_t data)
{
	int bit[8];
	uint8_t temp;
	bit[0] = (data >> 0) & 1;
	bit[1] = (data >> 1) & 1;
	bit[2] = (data >> 2) & 1;
	bit[3] = (data >> 3) & 1;
	bit[4] = (data >> 4) & 1;
	bit[5] = (data >> 5) & 1;
	bit[6] = (data >> 6) & 1;
	bit[7] = (data >> 7) & 1;
	// column parity
	ecc[1] ^= ((bit[6] ^ bit[4] ^ bit[2] ^ bit[0]) << 6);
	ecc[1] ^= ((bit[7] ^ bit[5] ^ bit[3] ^ bit[1]) << 7);
	ecc[0] ^= ((bit[5] ^ bit[4] ^ bit[1] ^ bit[0]) << 0);
	ecc[0] ^= ((bit[7] ^ bit[6] ^ bit[3] ^ bit[2]) << 1);
	ecc[0] ^= ((bit[3] ^ bit[2] ^ bit[1] ^ bit[0]) << 2);
	ecc[0] ^= ((bit[7] ^ bit[6] ^ bit[5] ^ bit[4]) << 3);
	// line parity
	temp = bit[7] ^ bit[6] ^ bit[5] ^ bit[4] ^ bit[3] ^ bit[2] ^ bit[1] ^ bit[0];
	if (pos & 0x001) ecc[0] ^= (temp << 5); else ecc[0] ^= (temp << 4);
	if (pos & 0x002) ecc[0] ^= (temp << 7); else ecc[0] ^= (temp << 6);
	if (pos & 0x004) ecc[1] ^= (temp << 3); else ecc[1] ^= (temp << 2);
	if (pos & 0x008) ecc[1] ^= (temp << 5); else ecc[1] ^= (temp << 4);
}

#endif

void S3C24_CLASS_NAME::s3c24xx_nand_update_ecc(uint8_t data)
{
	uint8_t temp[4];
#if defined(DEVICE_S3C2410)
	temp[0] = m_nand.mecc[0];
	temp[1] = m_nand.mecc[1];
	temp[2] = m_nand.mecc[2];
	nand_update_mecc(m_nand.mecc, m_nand.ecc_pos++, data);
	verboselog(*this, 5, "NAND - MECC %03X - %02X %02X %02X -> %02X %02X %02X\n", m_nand.ecc_pos - 1, temp[0], temp[1], temp[2], m_nand.mecc[0], m_nand.mecc[1], m_nand.mecc[2]);
	if (m_nand.ecc_pos == 512)
		m_nand.ecc_pos = 0;
#else
	if (!BIT(m_nand.regs.nfcont, 5))
	{
		temp[0] = m_nand.mecc[0];
		temp[1] = m_nand.mecc[1];
		temp[2] = m_nand.mecc[2];
		temp[3] = m_nand.mecc[3];
		nand_update_mecc( m_nand.mecc, m_nand.ecc_pos++, data);
		verboselog( *this, 5, "NAND - MECC %03X - %02X %02X %02X %02X -> %02X %02X %02X %02X\n", m_nand.ecc_pos - 1, temp[0], temp[1], temp[2], temp[3], m_nand.mecc[0], m_nand.mecc[1], m_nand.mecc[2], m_nand.mecc[3]);
		if (m_nand.ecc_pos == 2048) m_nand.ecc_pos = 0;
	}
	if (!BIT(m_nand.regs.nfcont, 6))
	{
		temp[0] = m_nand.secc[0];
		temp[1] = m_nand.secc[1];
		nand_update_secc(m_nand.secc, m_nand.ecc_pos++, data);
		verboselog( *this, 5, "NAND - SECC %02X - %02X %02X -> %02X %02X\n", m_nand.ecc_pos - 1, temp[0], temp[1], m_nand.secc[0], m_nand.secc[1]);
		if (m_nand.ecc_pos == 16)
			m_nand.ecc_pos = 0;
	}
#endif
}

void S3C24_CLASS_NAME::s3c24xx_nand_command_w(uint8_t data)
{
	verboselog(*this, 5, "NAND write command %02X\n", data);
	m_nand.data_count = 0;
	iface_nand_command_w( data);
}

void S3C24_CLASS_NAME::s3c24xx_nand_address_w(uint8_t data)
{
	verboselog( *this, 5, "NAND write address %02X\n", data);
	m_nand.data_count = 0;
	iface_nand_address_w( data);
}

uint8_t S3C24_CLASS_NAME::s3c24xx_nand_data_r()
{
	uint8_t data = iface_nand_data_r();
	verboselog( *this, 5, "NAND read data %02X [%04X]\n", data, m_nand.data_count++);
	s3c24xx_nand_update_ecc( data);
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_nand_data_w(uint8_t data)
{
	verboselog( *this, 5, "NAND write data %02X [%04X]\n", data, m_nand.data_count++);
	iface_nand_data_w( data);
	s3c24xx_nand_update_ecc( data);
}

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_nand_r )
{
	uint32_t data = ((uint32_t*)&m_nand.regs)[offset];
	switch (offset)
	{
	case S3C24XX_NFDATA:
		data = 0;
#if defined(DEVICE_S3C2410)
		data = data | s3c24xx_nand_data_r();
#elif defined(DEVICE_S3C2440)
		if ((mem_mask & 0x000000FF) != 0) data = data | (s3c24xx_nand_data_r() <<  0);
		if ((mem_mask & 0x0000FF00) != 0) data = data | (s3c24xx_nand_data_r() <<  8);
		if ((mem_mask & 0x00FF0000) != 0) data = data | (s3c24xx_nand_data_r() << 16);
		if ((mem_mask & 0xFF000000) != 0) data = data | (s3c24xx_nand_data_r() << 24);
#endif
		break;
#if defined(DEVICE_S3C2410)
	case S3C24XX_NFECC :
		data = ((m_nand.mecc[2] << 16) | (m_nand.mecc[1] << 8) | (m_nand.mecc[0] << 0));
		break;
#endif
#if defined(DEVICE_S3C2440)
	case S3C24XX_NFMECC0 :
		data = (m_nand.mecc[3] << 24) | (m_nand.mecc[2] << 16) | (m_nand.mecc[1] << 8) | (m_nand.mecc[0] << 0);
		break;
	case S3C24XX_NFSECC :
		data = (m_nand.secc[1] << 8) | (m_nand.secc[0] << 0);
		break;
	case S3C24XX_NFESTAT0 :
		data &= ~0x000000F; // no main/spare ECC errors
		break;
	case S3C24XX_NFESTAT1 :
		data &= ~0x000000F; // no main/spare ECC errors
		break;
#endif
	}
	verboselog(*this, 9, "(NAND) %08X -> %08X (%08X)\n", S3C24XX_BASE_NAND + (offset << 2), data, mem_mask);
	return data;
}

void S3C24_CLASS_NAME::s3c24xx_nand_init_ecc()
{
	verboselog( *this, 5, "NAND - init ecc\n");
	m_nand.mecc[0] = 0xFF;
	m_nand.mecc[1] = 0xFF;
	m_nand.mecc[2] = 0xFF;
#if defined(DEVICE_S3C2440)
	m_nand.mecc[3] = 0xFF;
	m_nand.secc[0] = 0;
	m_nand.secc[1] = 0;
#endif
	m_nand.ecc_pos = 0;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_nand_w )
{
	uint32_t old_value = ((uint32_t*)&m_nand.regs)[offset];
	verboselog( *this, 9, "(NAND) %08X <- %08X (%08X)\n", S3C24XX_BASE_NAND + (offset << 2), data, mem_mask);
	COMBINE_DATA(&((uint32_t*)&m_nand.regs)[offset]);
	switch (offset)
	{
#if defined(DEVICE_S3C2410)
	case S3C24XX_NFCONF:
		if ((data & (1 << 12)) != 0)
			s3c24xx_nand_init_ecc();
		break;
#endif
#if defined(DEVICE_S3C2440)
	case S3C24XX_NFCONT:
		if ((data & (1 << 4)) != 0)
			s3c24xx_nand_init_ecc();
		break;
#endif
	case S3C24XX_NFSTAT:
		m_nand.regs.nfstat = (m_nand.regs.nfstat & ~0x03) | (old_value & 0x03); // read-only
#if defined(DEVICE_S3C2440)
		if ((data & (1 << 2)) != 0)
			m_nand.regs.nfstat &= ~(1 << 2); // "RnB_TransDetect, to clear this value write 1"
#endif
		break;
	case S3C24XX_NFCMD:
		s3c24xx_nand_command_w(data);
		break;
	case S3C24XX_NFADDR:
		s3c24xx_nand_address_w(data);
		break;
	case S3C24XX_NFDATA:
#if defined(DEVICE_S3C2410)
		s3c24xx_nand_data_w(data & 0xFF);
#elif defined(DEVICE_S3C2440)
		if ((mem_mask & 0x000000FF) != 0) s3c24xx_nand_data_w((data >>  0) & 0xFF);
		if ((mem_mask & 0x0000FF00) != 0) s3c24xx_nand_data_w((data >>  8) & 0xFF);
		if ((mem_mask & 0x00FF0000) != 0) s3c24xx_nand_data_w((data >> 16) & 0xFF);
		if ((mem_mask & 0xFF000000) != 0) s3c24xx_nand_data_w((data >> 24) & 0xFF);
#endif
		break;
	}
}

ATTR_UNUSED WRITE_LINE_MEMBER( S3C24_CLASS_NAME::s3c24xx_pin_frnb_w )
{
	verboselog( *this, 9, "s3c24xx_pin_frnb_w (%d)\n", state);
#if defined(DEVICE_S3C2440)
	if ((BIT( m_nand.regs.nfstat, 0) == 0) && (state != 0))
	{
		m_nand.regs.nfstat |= (1 << 2);
		if (BIT( m_nand.regs.nfcont, 9) != 0)
			s3c24xx_request_irq( S3C24XX_INT_NFCON);
	}
#endif
	if (state == 0)
		m_nand.regs.nfstat &= ~(1 << 0);
	else
		m_nand.regs.nfstat |= (1 << 0);
}

#endif

/* Camera Interface */

#if defined(DEVICE_S3C2440)

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_cam_r )
{
	uint32_t data = m_cam.regs.data[offset];
	verboselog(*this, 9, "(CAM) %08X -> %08X\n", S3C24XX_BASE_CAM + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_cam_w )
{
	verboselog(*this, 9, "(CAM) %08X <- %08X\n", S3C24XX_BASE_CAM + (offset << 2), data);
	COMBINE_DATA(&m_cam.regs.data[offset]);
}

#endif

/* AC97 Interface */

#if defined(DEVICE_S3C2440)

READ32_MEMBER( S3C24_CLASS_NAME::s3c24xx_ac97_r )
{
	uint32_t data = m_ac97.regs.data[offset];
	verboselog(*this, 9, "(AC97) %08X -> %08X\n", S3C24XX_BASE_AC97 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( S3C24_CLASS_NAME::s3c24xx_ac97_w )
{
	verboselog(*this, 9, "(AC97) %08X <- %08X\n", S3C24XX_BASE_AC97 + (offset << 2), data);
	COMBINE_DATA(&m_ac97.regs.data[offset]);
}

#endif

// ...

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

void S3C24_CLASS_NAME::s3c24xx_nand_auto_boot()
{
	int om0 = iface_core_pin_r(S3C24XX_CORE_PIN_OM0);
	int om1 = iface_core_pin_r(S3C24XX_CORE_PIN_OM1);
	if ((om0 == 0) && (om1 == 0))
	{
		int ncon = iface_core_pin_r(S3C24XX_CORE_PIN_NCON);
		uint8_t *ptr = m_steppingstone;
		int page_size, address_cycle;
#if defined(DEVICE_S3C2410)
		page_size = 512;
		if (ncon == 0)
			address_cycle = 3; // byte-page-page
		else
			address_cycle = 4; // byte-page-page-page
#elif defined(DEVICE_S3C2440)
		uint32_t port_g = iface_gpio_port_r( S3C24XX_GPIO_PORT_G, 0);
		if (ncon == 0)
		{
			if (BIT( port_g, 13) == 0)
			{
				page_size = 256;
				address_cycle = 3; // byte-page-page
			}
			else
			{
				page_size = 512;
				address_cycle = 4; // byte-page-page-page
			}
		}
		else
		{
			if (BIT( port_g, 13) == 0)
			{
				page_size = 1024;
				address_cycle = 4; // byte-byte-page-page or byte-page-page-page ??? assume latter
			}
			else
			{
				page_size = 2048;
				address_cycle = 5; // byte-byte-page-page-page
			}
		}
#endif
		iface_nand_command_w(0xFF);
		for (int page = 0; page < (4 * 1024) / page_size; page++)
		{
			iface_nand_command_w(0x00);
			iface_nand_address_w(0x00);
			if (address_cycle > 4)
			{
				iface_nand_address_w(0x00);
			}
			iface_nand_address_w((page >> 0) & 0xFF);
			iface_nand_address_w((page >> 8) & 0xFF);
			if (address_cycle > 3)
			{
				iface_nand_address_w((page >> 16) & 0xFF);
			}
			for (int i = 0; i < page_size; i++)
			{
				*ptr++ = iface_nand_data_r();
			}
		}
		iface_nand_command_w(0xFF);
	}
}

#endif

void S3C24_CLASS_NAME::s3c24xx_device_reset()
{
	verboselog( *this, 1, "s3c24xx device reset\n");
	for (uart_t &uart : m_uart)
		uart.reset();
	m_pwm.reset();
	s3c24xx_dma_reset();
	s3c24xx_iic_reset();
	m_iis.reset();
	s3c24xx_lcd_reset();
	m_rtc.reset();
	m_wdt.reset();
	s3c24xx_irq_reset();
	s3c24xx_gpio_reset();
	m_memcon.reset();
	s3c24xx_clkpow_reset();
	m_usbhost.reset();
	s3c24xx_usb_device_reset();
	s3c24xx_adc_reset();
	s3c24xx_spi_reset();
#if defined(DEVICE_S3C2400)
	m_mmc.reset();
#endif
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	s3c24xx_sdi_reset();
	s3c24xx_nand_reset();
#endif
#if defined(DEVICE_S3C2440)
	m_cam.reset();
	m_ac97.reset();
#endif
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	s3c24xx_nand_auto_boot();
#endif
}

void S3C24_CLASS_NAME::s3c24xx_device_start()
{
	verboselog(*this, 1, "s3c24xx device start\n");
	m_pin_r_cb.resolve();
	m_pin_w_cb.resolve_safe();
	m_port_r_cb.resolve();
	m_port_w_cb.resolve();
	m_scl_w_cb.resolve();
	m_sda_r_cb.resolve();
	m_sda_w_cb.resolve();
	m_data_r_cb.resolve();
	m_data_w_cb.resolve();
#if !defined(DEVICE_S3C2400)
	m_command_w_cb.resolve();
	m_address_w_cb.resolve();
	m_nand_data_r_cb.resolve();
	m_nand_data_w_cb.resolve();
#endif
	for (int i = 0; i < 5; i++)
		m_pwm.timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_pwm_timer_exp), this));
	for (auto & elem : m_dma)
		elem.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_dma_timer_exp), this));
	m_iic.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_iic_timer_exp), this));
	m_iis.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_iis_timer_exp), this));
	m_lcd.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_lcd_timer_exp), this));
	m_rtc.timer_tick_count = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_rtc_timer_tick_count_exp), this));
	m_rtc.timer_update = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_rtc_timer_update_exp), this));
	m_wdt.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(S3C24_CLASS_NAME::s3c24xx_wdt_timer_exp), this));

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	int om0 = iface_core_pin_r(S3C24XX_CORE_PIN_OM0);
	int om1 = iface_core_pin_r(S3C24XX_CORE_PIN_OM1);
	if ((om0 == 0) && (om1 == 0))
	{
		address_space &space = m_cpu->memory().space(AS_PROGRAM);
		space.install_ram(0x00000000, 0x00000fff, m_steppingstone);
		space.install_ram(0x40000000, 0x40000fff, m_steppingstone);
	}
#endif
}
