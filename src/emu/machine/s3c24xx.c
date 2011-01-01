/*******************************************************************************

    Samsung S3C2400 / S3C2410 / S3C2440

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
//#include "includes/s3c24xx.h"
#include "coreutil.h"

/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

//#define UART_PRINTF

#define CLOCK_MULTIPLIER 1

#define BIT(x,n) (((x)>>(n))&1)
#define BITS(x,m,n) (((x)>>(n))&(((UINT32)1<<((m)-(n)+1))-1))
#define CLR_BITS(x,m,n) ((x) & ~((((UINT32)1 << ((m) - (n) + 1)) - 1) << n))

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
    TYPE DEFINITIONS
***************************************************************************/

#if defined(DEVICE_S3C2400)
typedef s3c2400_interface s3c24xx_interface;
#elif defined(DEVICE_S3C2410)
typedef s3c2410_interface s3c24xx_interface;
#elif defined(DEVICE_S3C2440)
typedef s3c2440_interface s3c24xx_interface;
#endif

/***************************************************************************
    PROTOTYPES
***************************************************************************/

static UINT32 s3c24xx_get_fclk( device_t *device);
static UINT32 s3c24xx_get_hclk( device_t *device);
static UINT32 s3c24xx_get_pclk( device_t *device);

static void s3c24xx_dma_request_iis( device_t *device);
static void s3c24xx_dma_request_pwm( device_t *device);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE s3c24xx_t *get_token( device_t *device)
{
	assert(device != NULL);
	return (s3c24xx_t *)downcast<legacy_device_base *>(device)->token();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* ... */

static void s3c24xx_reset( device_t *device)
{
	fatalerror( "s3c24xx_reset\n");
}

/* LCD Controller */

static rgb_t s3c24xx_get_color_tft_16( device_t *device, UINT16 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if ((s3c24xx->lcd.regs.lcdcon5 & (1 << 11)) == 0)
	{
		UINT8 r, g, b, i;
		r = (BITS( data, 15, 11) << 3);
		g = (BITS( data, 10, 6) << 3);
		b = (BITS( data, 5, 1) << 3);
		i = BIT( data, 1) << 2;
		return MAKE_RGB( r | i, g | i, b | i);
	}
	else
	{
		UINT8 r, g, b;
		r = BITS( data, 15, 11) << 3;
		g = BITS( data, 10, 5) << 2;
		b = BITS( data, 4, 0) << 3;
		return MAKE_RGB( r, g, b);
	}
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static rgb_t s3c24xx_get_color_tft_24( device_t *device, UINT32 data)
{
	UINT8 r, g, b;
	r = BITS( data, 23, 16);
	g = BITS( data, 15, 8);
	b = BITS( data, 7, 0);
	return MAKE_RGB( r, g, b);
}

#endif

static rgb_t s3c24xx_get_color_stn_12( device_t *device, UINT16 data)
{
	UINT8 r, g, b;
	r = BITS( data, 11, 8) << 4;
	g = BITS( data, 7, 4) << 4;
	b = BITS( data, 3, 0) << 4;
	return MAKE_RGB( r, g, b);
}

static rgb_t s3c24xx_get_color_stn_08( device_t *device, UINT8 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT8 r, g, b;
	r = ((s3c24xx->lcd.regs.redlut   >> (BITS( data, 7, 5) << 2)) & 0xF) << 4;
	g = ((s3c24xx->lcd.regs.greenlut >> (BITS( data, 4, 2) << 2)) & 0xF) << 4;
	b = ((s3c24xx->lcd.regs.bluelut  >> (BITS( data, 1, 0) << 2)) & 0xF) << 4;
	return MAKE_RGB( r, g, b);
}

static rgb_t s3c24xx_get_color_stn_01( device_t *device, UINT8 data)
{
	if ((data & 1) == 0)
	{
		return RGB_BLACK;
	}
	else
	{
		return RGB_WHITE;
	}
}

static rgb_t s3c24xx_get_color_stn_02( device_t *device, UINT8 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT8 r, g, b;
	r = g = b = ((s3c24xx->lcd.regs.bluelut >> (BITS( data, 1, 0) << 2)) & 0xF) << 4;
	return MAKE_RGB( r, g, b);
}

static rgb_t s3c24xx_get_color_stn_04( device_t *device, UINT8 data)
{
	UINT8 r, g, b;
	r = g = b = BITS( data, 3, 0) << 4;
	return MAKE_RGB( r, g, b);
}

static rgb_t s3c24xx_get_color_tpal( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
#if defined(DEVICE_S3C2400)
	return s3c24xx_get_color_tft_16( device, S3C24XX_TPAL_GET_TPALVAL( s3c24xx->lcd.tpal));
#else
	return s3c24xx_get_color_tft_24( device, S3C24XX_TPAL_GET_TPALVAL( s3c24xx->lcd.tpal));
#endif
}

static void s3c24xx_lcd_dma_reload( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx->lcd.vramaddr_cur = s3c24xx->lcd.regs.lcdsaddr1 << 1;
	s3c24xx->lcd.vramaddr_max = ((s3c24xx->lcd.regs.lcdsaddr1 & 0xFFE00000) | s3c24xx->lcd.regs.lcdsaddr2) << 1;
	s3c24xx->lcd.offsize = BITS( s3c24xx->lcd.regs.lcdsaddr3, 21, 11);
	s3c24xx->lcd.pagewidth_cur = 0;
	s3c24xx->lcd.pagewidth_max = BITS( s3c24xx->lcd.regs.lcdsaddr3, 10, 0);
	verboselog( device->machine, 3, "LCD - vramaddr %08X %08X offsize %08X pagewidth %08X\n", s3c24xx->lcd.vramaddr_cur, s3c24xx->lcd.vramaddr_max, s3c24xx->lcd.offsize, s3c24xx->lcd.pagewidth_max);
	s3c24xx->lcd.dma_data = 0;
	s3c24xx->lcd.dma_bits = 0;
}

static void s3c24xx_lcd_dma_init( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx_lcd_dma_reload( device);
	s3c24xx->lcd.bppmode = BITS( s3c24xx->lcd.regs.lcdcon1, 4, 1);
	s3c24xx->lcd.bswp = BIT( s3c24xx->lcd.regs.lcdcon5, 1);
	s3c24xx->lcd.hwswp = BIT( s3c24xx->lcd.regs.lcdcon5, 0);
	s3c24xx->lcd.tpal = s3c24xx->lcd.regs.tpal;
	verboselog( device->machine, 3, "LCD - bppmode %d hwswp %d bswp %d\n", s3c24xx->lcd.bppmode, s3c24xx->lcd.hwswp, s3c24xx->lcd.bswp);
	s3c24xx->lcd.dma_data = 0;
	s3c24xx->lcd.dma_bits = 0;
}

#if 0
static UINT32 s3c24xx_lcd_dma_read( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	address_space* space = cputag_get_address_space( device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT8 *vram, data[4];
	vram = (UINT8 *)space->get_read_ptr( s3c24xx->lcd.vramaddr_cur);
	for (int i = 0; i < 2; i++)
	{
		data[i*2+0] = *vram++;
		data[i*2+1] = *vram++;
		s3c24xx->lcd.vramaddr_cur += 2;
		s3c24xx->lcd.pagewidth_cur++;
		if (s3c24xx->lcd.pagewidth_cur >= s3c24xx->lcd.pagewidth_max)
		{
			s3c24xx->lcd.vramaddr_cur += s3c24xx->lcd.offsize << 1;
			s3c24xx->lcd.pagewidth_cur = 0;
			vram = (UINT8 *)space->get_read_ptr( s3c24xx->lcd.vramaddr_cur);
		}
	}
	if (s3c24xx->lcd.hwswp == 0)
	{
		if (s3c24xx->lcd.bswp == 0)
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
		if (s3c24xx->lcd.bswp == 0)
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

static UINT32 s3c24xx_lcd_dma_read( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	address_space* space = cputag_get_address_space( device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT8 *vram, data[4];
	vram = (UINT8 *)space->get_read_ptr( s3c24xx->lcd.vramaddr_cur);
	for (int i = 0; i < 2; i++)
	{
		if (s3c24xx->lcd.hwswp == 0)
		{
			if (s3c24xx->lcd.bswp == 0)
			{
				if ((s3c24xx->lcd.vramaddr_cur & 2) == 0)
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
			if (s3c24xx->lcd.bswp == 0)
			{
				data[i*2+0] = *(vram + 1);
				data[i*2+1] = *(vram + 0);
			}
			else
			{
				if ((s3c24xx->lcd.vramaddr_cur & 2) == 0)
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
		s3c24xx->lcd.vramaddr_cur += 2;
		s3c24xx->lcd.pagewidth_cur++;
		if (s3c24xx->lcd.pagewidth_cur >= s3c24xx->lcd.pagewidth_max)
		{
			s3c24xx->lcd.vramaddr_cur += s3c24xx->lcd.offsize << 1;
			s3c24xx->lcd.pagewidth_cur = 0;
			vram = (UINT8 *)space->get_read_ptr( s3c24xx->lcd.vramaddr_cur);
		}
		else
		{
			vram += 2;
		}
	}
	if (s3c24xx->iface->lcd.flags & S3C24XX_INTERFACE_LCD_REVERSE)
	{
		return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0] << 0);
	}
	else
	{
		return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
	}
}

static UINT32 s3c24xx_lcd_dma_read_bits( device_t *device, int count)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data;
	if (count <= s3c24xx->lcd.dma_bits)
	{
		s3c24xx->lcd.dma_bits -= count;
		data = BITS( s3c24xx->lcd.dma_data, 31, 32 - count);
		s3c24xx->lcd.dma_data = s3c24xx->lcd.dma_data << count;
	}
	else
	{
		if (s3c24xx->lcd.dma_bits == 0)
		{
			if (count == 32)
			{
				data = s3c24xx_lcd_dma_read( device);
			}
			else
			{
				UINT32 temp = s3c24xx_lcd_dma_read( device);
				data = BITS( temp, 31, 32 - count);
				s3c24xx->lcd.dma_data = temp << count;
				s3c24xx->lcd.dma_bits = 32 - count;
			}
		}
		else
		{
			UINT32 temp = s3c24xx_lcd_dma_read( device);
			data = (s3c24xx->lcd.dma_data >> (32 - count)) | BITS( temp, 31, 32 - (count - s3c24xx->lcd.dma_bits));
			s3c24xx->lcd.dma_data = temp << (count - s3c24xx->lcd.dma_bits);
			s3c24xx->lcd.dma_bits = 32 - (count - s3c24xx->lcd.dma_bits);
		}
	}
	return data;
}

static void s3c24xx_lcd_render_tpal( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 color = s3c24xx_get_color_tpal( device);
	for (int y = s3c24xx->lcd.vpos_min; y <= s3c24xx->lcd.vpos_max; y++)
	{
		UINT32 *scanline = BITMAP_ADDR32( bitmap, y, s3c24xx->lcd.hpos_min);
		for (int x = s3c24xx->lcd.hpos_min; x <= s3c24xx->lcd.hpos_max; x++)
		{
			*scanline++ = color;
		}
	}
}

static void s3c24xx_lcd_render_stn_01( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 32; j++)
		{
			if (s3c24xx->iface->lcd.flags & S3C24XX_INTERFACE_LCD_REVERSE)
			{
				*scanline++ = s3c24xx_get_color_stn_01( device, data & 0x01);
				data = data >> 1;
			}
			else
			{
				*scanline++ = s3c24xx_get_color_stn_01( device, (data >> 31) & 0x01);
				data = data << 1;
			}
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 4))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_stn_02( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 16; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_02( device, (data >> 30) & 0x03);
			data = data << 2;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 3))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_stn_04( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 8; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_04( device, (data >> 28) & 0x0F);
			data = data << 4;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 2))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_stn_08( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 4; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_08( device, (data >> 24) & 0xFF);
			data = data << 8;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 1))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_stn_12_p( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 16; i++)
	{
		*scanline++ = s3c24xx_get_color_stn_12( device, s3c24xx_lcd_dma_read_bits( device, 12));
		s3c24xx->lcd.hpos++;
		if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max * 16 / 12))
		{
			s3c24xx->lcd.vpos++;
			if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
			s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
			scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
		}
	}
}

static void s3c24xx_lcd_render_stn_12_u( device_t *device) // not tested
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 2; j++)
		{
			*scanline++ = s3c24xx_get_color_stn_12( device, (data >> 16) & 0x0FFF);
			data = data << 16;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 0))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_tft_01( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 32; j++)
		{
			*scanline++ = palette_get_color( device->machine, (data >> 31) & 0x01);
			data = data << 1;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 4))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_tft_02( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 16; j++)
		{
			*scanline++ = palette_get_color( device->machine, (data >> 30) & 0x03);
			data = data << 2;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 3))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_tft_04( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 8; j++)
		{
			*scanline++ = palette_get_color( device->machine, (data >> 28) & 0x0F);
			data = data << 4;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 2))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_tft_08( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 4; j++)
		{
			*scanline++ = palette_get_color( device->machine, (data >> 24) & 0xFF);
			data = data << 8;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 1))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static void s3c24xx_lcd_render_tft_16( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	bitmap_t *bitmap = s3c24xx->lcd.bitmap[0];
	UINT32 *scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	for (int i = 0; i < 4; i++)
	{
		UINT32 data = s3c24xx_lcd_dma_read( device);
		for (int j = 0; j < 2; j++)
		{
			*scanline++ = s3c24xx_get_color_tft_16( device, (data >> 16) & 0xFFFF);
			data = data << 16;
			s3c24xx->lcd.hpos++;
			if (s3c24xx->lcd.hpos >= s3c24xx->lcd.hpos_min + (s3c24xx->lcd.pagewidth_max << 0))
			{
				s3c24xx->lcd.vpos++;
				if (s3c24xx->lcd.vpos > s3c24xx->lcd.vpos_max) s3c24xx->lcd.vpos = s3c24xx->lcd.vpos_min;
				s3c24xx->lcd.hpos = s3c24xx->lcd.hpos_min;
				scanline = BITMAP_ADDR32( bitmap, s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
			}
		}
	}
}

static TIMER_CALLBACK( s3c24xx_lcd_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c24xx_t *s3c24xx = get_token( device);
	screen_device *screen = machine->primary_screen;
	UINT32 tpalen;
	verboselog( machine, 2, "LCD timer callback\n");
	s3c24xx->lcd.vpos = screen->vpos();
	s3c24xx->lcd.hpos = screen->hpos();
	verboselog( machine, 3, "LCD - vpos %d hpos %d\n", s3c24xx->lcd.vpos, s3c24xx->lcd.hpos);
	tpalen = S3C24XX_TPAL_GET_TPALEN( s3c24xx->lcd.tpal);
	if (tpalen == 0)
	{
		if (s3c24xx->lcd.vramaddr_cur >= s3c24xx->lcd.vramaddr_max)
		{
			s3c24xx_lcd_dma_reload( device);
		}
		verboselog( machine, 3, "LCD - vramaddr %08X\n", s3c24xx->lcd.vramaddr_cur);
		while (s3c24xx->lcd.vramaddr_cur < s3c24xx->lcd.vramaddr_max)
		{
			switch (s3c24xx->lcd.bppmode)
			{
				case S3C24XX_BPPMODE_STN_01   : s3c24xx_lcd_render_stn_01( device); break;
				case S3C24XX_BPPMODE_STN_02   : s3c24xx_lcd_render_stn_02( device); break;
				case S3C24XX_BPPMODE_STN_04   : s3c24xx_lcd_render_stn_04( device); break;
				case S3C24XX_BPPMODE_STN_08   : s3c24xx_lcd_render_stn_08( device); break;
				case S3C24XX_BPPMODE_STN_12_P : s3c24xx_lcd_render_stn_12_p( device); break;
				case S3C24XX_BPPMODE_STN_12_U : s3c24xx_lcd_render_stn_12_u( device); break;
				case S3C24XX_BPPMODE_TFT_01   : s3c24xx_lcd_render_tft_01( device); break;
				case S3C24XX_BPPMODE_TFT_02   : s3c24xx_lcd_render_tft_02( device); break;
				case S3C24XX_BPPMODE_TFT_04   : s3c24xx_lcd_render_tft_04( device); break;
				case S3C24XX_BPPMODE_TFT_08   : s3c24xx_lcd_render_tft_08( device); break;
				case S3C24XX_BPPMODE_TFT_16   : s3c24xx_lcd_render_tft_16( device); break;
				default : verboselog( machine, 0, "s3c24xx_lcd_timer_exp: bppmode %d not supported\n", s3c24xx->lcd.bppmode); break;
			}
			if ((s3c24xx->lcd.vpos == s3c24xx->lcd.vpos_min) && (s3c24xx->lcd.hpos == s3c24xx->lcd.hpos_min)) break;
		}
	}
	else
	{
		s3c24xx_lcd_render_tpal( device);
	}
	timer_adjust_oneshot( s3c24xx->lcd.timer, screen->time_until_pos( s3c24xx->lcd.vpos, s3c24xx->lcd.hpos), 0);
}

static void s3c24xx_video_start( device_t *device, running_machine *machine)
{
	s3c24xx_t *s3c24xx = get_token( device);
	screen_device *screen = machine->primary_screen;
	s3c24xx->lcd.bitmap[0] = screen->alloc_compatible_bitmap();
	s3c24xx->lcd.bitmap[1] = screen->alloc_compatible_bitmap();
}

static void bitmap_blend( bitmap_t *bitmap_dst, bitmap_t *bitmap_src_1, bitmap_t *bitmap_src_2)
{
	for (int y = 0; y < bitmap_dst->height; y++)
	{
		UINT32 *line0 = BITMAP_ADDR32( bitmap_src_1, y, 0);
		UINT32 *line1 = BITMAP_ADDR32( bitmap_src_2, y, 0);
		UINT32 *line2 = BITMAP_ADDR32( bitmap_dst, y, 0);
		for (int x = 0; x < bitmap_dst->width; x++)
		{
				UINT32 color0 = line0[x];
				UINT32 color1 = line1[x];
				UINT16 r0 = (color0 >> 16) & 0x000000ff;
				UINT16 g0 = (color0 >>  8) & 0x000000ff;
				UINT16 b0 = (color0 >>  0) & 0x000000ff;
				UINT16 r1 = (color1 >> 16) & 0x000000ff;
				UINT16 g1 = (color1 >>  8) & 0x000000ff;
				UINT16 b1 = (color1 >>  0) & 0x000000ff;
				UINT8 r = (UINT8)((r0 + r1) >> 1);
				UINT8 g = (UINT8)((g0 + g1) >> 1);
				UINT8 b = (UINT8)((b0 + b1) >> 1);
				line2[x] = (r << 16) | (g << 8) | b;
			}
		}
}

static UINT32 s3c24xx_video_update( device_t *device, screen_device *screen, bitmap_t *bitmap, const rectangle *cliprect)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->lcd.framerate >= 1195)
	{
		bitmap_blend( bitmap, s3c24xx->lcd.bitmap[0], s3c24xx->lcd.bitmap[1]);
		copybitmap( s3c24xx->lcd.bitmap[1], s3c24xx->lcd.bitmap[0], 0, 0, 0, 0, cliprect);
	}
	else
	{
		copybitmap( bitmap, s3c24xx->lcd.bitmap[0], 0, 0, 0, 0, cliprect);
	}
	s3c24xx_lcd_dma_init( device);
	return 0;
}

#if defined(DEVICE_S3C2400)
READ32_DEVICE_HANDLER( s3c2400_lcd_r )
#elif defined(DEVICE_S3C2410)
READ32_DEVICE_HANDLER( s3c2410_lcd_r )
#elif defined(DEVICE_S3C2440)
READ32_DEVICE_HANDLER( s3c2440_lcd_r )
#endif
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->lcd.regs)[offset];
	switch (offset)
	{
		case S3C24XX_LCDCON1 :
		{
			// make sure line counter is going
			UINT32 vpos = device->machine->primary_screen->vpos();
			if (vpos < s3c24xx->lcd.vpos_min) vpos = s3c24xx->lcd.vpos_min;
			if (vpos > s3c24xx->lcd.vpos_max) vpos = s3c24xx->lcd.vpos_max;
			data = (data & ~0xFFFC0000) | ((s3c24xx->lcd.vpos_max - vpos) << 18);
		}
		break;
		case S3C24XX_LCDCON5 :
		{
			UINT32 vpos = device->machine->primary_screen->vpos();
			data = data & ~0x00018000;
			if (vpos < s3c24xx->lcd.vpos_min) data = data | 0x00000000;
			if (vpos > s3c24xx->lcd.vpos_max) data = data | 0x00018000;
			// todo: 00 = VSYNC, 01 = BACK Porch, 10 = ACTIVE, 11 = FRONT Porch
		}
		break;
	}
	verboselog( device->machine, 9, "(LCD) %08X -> %08X\n", S3C24XX_BASE_LCD + (offset << 2), data);
	return data;
}

static void s3c24xx_lcd_configure_tft( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	screen_device *screen = device->machine->primary_screen;
	UINT32 vspw, vbpd, lineval, vfpd, hspw, hbpd, hfpd, hozval, clkval, hclk;
	double framerate, vclk;
	UINT32 width, height;
	rectangle visarea;
	verboselog( device->machine, 5, "s3c24xx_lcd_configure_tft\n");
	vspw = BITS( s3c24xx->lcd.regs.lcdcon2, 5, 0);
	vbpd = BITS( s3c24xx->lcd.regs.lcdcon2, 31, 24);
	lineval = BITS( s3c24xx->lcd.regs.lcdcon2, 23, 14);
	vfpd = BITS( s3c24xx->lcd.regs.lcdcon2, 13, 6);
	hspw = BITS( s3c24xx->lcd.regs.lcdcon4, 7, 0);
	hbpd = BITS( s3c24xx->lcd.regs.lcdcon3, 25, 19);
	hfpd = BITS( s3c24xx->lcd.regs.lcdcon3, 7, 0);
	hozval = BITS( s3c24xx->lcd.regs.lcdcon3, 18, 8);
	clkval = BITS( s3c24xx->lcd.regs.lcdcon1, 17, 8);
	hclk = s3c24xx_get_hclk( device);
	verboselog( device->machine, 3, "LCD - vspw %d vbpd %d lineval %d vfpd %d hspw %d hbpd %d hfpd %d hozval %d clkval %d hclk %d\n", vspw, vbpd, lineval, vfpd, hspw, hbpd, hfpd, hozval, clkval, hclk);
	vclk = (double)(hclk / ((clkval + 1) * 2));
	verboselog( device->machine, 3, "LCD - vclk %f\n", vclk);
	framerate = vclk / (((vspw + 1) + (vbpd + 1) + (lineval + 1) + (vfpd + 1)) * ((hspw + 1) + (hbpd + 1) + (hozval + 1) + (hfpd + 1)));
	verboselog( device->machine, 3, "LCD - framerate %f\n", framerate);
	s3c24xx->lcd.framerate = framerate;
	width = (hspw + 1) + (hbpd + 1) + (hozval + 1) + (hfpd + 1);
	height = (vspw + 1) + (vbpd + 1) + (lineval + 1) + (vfpd + 1);
	visarea.min_x = (hspw + 1) + (hbpd + 1);
	visarea.min_y = (vspw + 1) + (vbpd + 1);
	visarea.max_x = visarea.min_x + (hozval + 1) - 1;
	visarea.max_y = visarea.min_y + (lineval + 1) - 1;
	verboselog( device->machine, 3, "LCD - visarea min_x %d min_y %d max_x %d max_y %d\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	verboselog( device->machine, 3, "video_screen_configure %d %d %f\n", width, height, s3c24xx->lcd.framerate);
	s3c24xx->lcd.hpos_min = (hspw + 1) + (hbpd + 1);
	s3c24xx->lcd.hpos_max = s3c24xx->lcd.hpos_min + (hozval + 1) - 1;
	s3c24xx->lcd.vpos_min = (vspw + 1) + (vbpd + 1);
	s3c24xx->lcd.vpos_max = s3c24xx->lcd.vpos_min + (lineval + 1) - 1;
	screen->configure( width, height, visarea, HZ_TO_ATTOSECONDS( s3c24xx->lcd.framerate));
}

static void s3c24xx_lcd_configure_stn( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	screen_device *screen = device->machine->primary_screen;
	UINT32 pnrmode, bppmode, clkval, lineval, wdly, hozval, lineblank, wlh, hclk;
	double vclk, framerate;
	UINT32 width, height;
	rectangle visarea;
	verboselog( device->machine, 5, "s3c24xx_lcd_configure_stn\n");
	pnrmode = BITS( s3c24xx->lcd.regs.lcdcon1, 6, 5);
	bppmode = BITS( s3c24xx->lcd.regs.lcdcon1, 4, 1);
	clkval = BITS( s3c24xx->lcd.regs.lcdcon1, 17, 8);
	lineval = BITS( s3c24xx->lcd.regs.lcdcon2, 23, 14);
	wdly = BITS( s3c24xx->lcd.regs.lcdcon3, 20, 19);
	hozval = BITS( s3c24xx->lcd.regs.lcdcon3, 18, 8);
	lineblank = BITS( s3c24xx->lcd.regs.lcdcon3, 7, 0);
	wlh = BITS( s3c24xx->lcd.regs.lcdcon4, 1, 0);
	hclk = s3c24xx_get_hclk( device);
	verboselog( device->machine, 3, "LCD - pnrmode %d bppmode %d clkval %d lineval %d wdly %d hozval %d lineblank %d wlh %d hclk %d\n", pnrmode, bppmode, clkval, lineval, wdly, hozval, lineblank, wlh, hclk);
	vclk = (double)(hclk / ((clkval + 0) * 2));
	verboselog( device->machine, 3, "LCD - vclk %f\n", vclk);
	framerate = 1 / (((1 / vclk) * (hozval + 1) + (1 / hclk) * ((1 << (4 + wlh)) + (1 << (4 + wdly)) + (lineblank * 8))) * (lineval + 1));
	verboselog( device->machine, 3, "LCD - framerate %f\n", framerate);
	switch (pnrmode)
	{
		case S3C24XX_PNRMODE_STN_04_SS : width = ((hozval + 1) * 4); break;
		case S3C24XX_PNRMODE_STN_04_DS : width = ((hozval + 1) * 4); break;
		case S3C24XX_PNRMODE_STN_08_SS : width = ((hozval + 1) * 8 / 3); break;
		default : width = 0; break;
	}
	height = lineval + 1;
	s3c24xx->lcd.framerate = framerate;
	visarea.min_x = 0;
	visarea.min_y = 0;
	visarea.max_x = width - 1;
	visarea.max_y = height - 1;
	verboselog( device->machine, 3, "LCD - visarea min_x %d min_y %d max_x %d max_y %d\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	verboselog( device->machine, 3, "video_screen_configure %d %d %f\n", width, height, s3c24xx->lcd.framerate);
	s3c24xx->lcd.hpos_min = 0;
	s3c24xx->lcd.hpos_max = width - 1;
	s3c24xx->lcd.vpos_min = 0;
	s3c24xx->lcd.vpos_max = height - 1;
	screen->configure( width, height, visarea, HZ_TO_ATTOSECONDS( s3c24xx->lcd.framerate));
}

static void s3c24xx_lcd_configure( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 bppmode;
	verboselog( device->machine, 5, "s3c24xx_lcd_configure\n");
	bppmode = BITS( s3c24xx->lcd.regs.lcdcon1, 4, 1);
	if ((bppmode & (1 << 3)) == 0)
	{
		s3c24xx_lcd_configure_stn( device);
	}
	else
	{
		s3c24xx_lcd_configure_tft( device);
	}
}

static void s3c24xx_lcd_start( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	screen_device *screen = device->machine->primary_screen;
	verboselog( device->machine, 1, "LCD start\n");
	s3c24xx_lcd_configure( device);
	s3c24xx_lcd_dma_init( device);
	timer_adjust_oneshot( s3c24xx->lcd.timer, screen->time_until_pos( s3c24xx->lcd.vpos_min, s3c24xx->lcd.hpos_min), 0);
}

static void s3c24xx_lcd_stop( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 1, "LCD stop\n");
	timer_adjust_oneshot( s3c24xx->lcd.timer, attotime_never, 0);
}

static void s3c24xx_lcd_recalc( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->lcd.regs.lcdcon1 & (1 << 0))
	{
		s3c24xx_lcd_start( device);
	}
	else
	{
		s3c24xx_lcd_stop( device);
	}
}

static WRITE32_DEVICE_HANDLER( s3c24xx_lcd_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->lcd.regs)[offset];
	verboselog( device->machine, 9, "(LCD) %08X <- %08X\n", S3C24XX_BASE_LCD + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->lcd.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_LCDCON1 :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				s3c24xx_lcd_recalc( device);
			}
		}
		break;
	}
}

/* LCD Palette */

static READ32_DEVICE_HANDLER( s3c24xx_lcd_palette_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->lcdpal.regs.data[offset];
	verboselog( device->machine, 9, "(LCD) %08X -> %08X\n", S3C24XX_BASE_LCDPAL + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_lcd_palette_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(LCD) %08X <- %08X\n", S3C24XX_BASE_LCDPAL + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->lcdpal.regs.data[offset]);
	if (mem_mask != 0xffffffff)
	{
		verboselog( device->machine, 0, "s3c24xx_lcd_palette_w: unknown mask %08x\n", mem_mask);
	}
	palette_set_color( device->machine, offset, s3c24xx_get_color_tft_16( device, data & 0xFFFF));
}

/* Clock & Power Management */

static UINT32 s3c24xx_get_fclk( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data, mdiv, pdiv, sdiv;
	data = s3c24xx->clkpow.regs.mpllcon;
	mdiv = BITS( data, 19, 12);
	pdiv = BITS( data, 9, 4);
	sdiv = BITS( data, 1, 0);
#if defined(DEVICE_S3C2400) || defined(DEVICE_S3C2410)
	return (UINT32)((double)(1 * (mdiv + 8) * device->clock()) / (double)((pdiv + 2) * (1 << sdiv)));
#else
	return (UINT32)((double)(2 * (mdiv + 8) * device->clock()) / (double)((pdiv + 2) * (1 << sdiv)));
#endif
}

static UINT32 s3c24xx_get_hclk( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
#if defined(DEVICE_S3C2400) || defined(DEVICE_S3C2410)
	return s3c24xx_get_fclk( device) / (BIT( s3c24xx->clkpow.regs.clkdivn, 1) + 1);
#else
	switch (BITS( s3c24xx->clkpow.regs.clkdivn, 2, 1))
	{
		case 0 : return s3c24xx_get_fclk( device) / 1;
		case 1 : return s3c24xx_get_fclk( device) / 2;
		case 2 : return s3c24xx_get_fclk( device) / (4 * (BIT( s3c24xx->clkpow.regs.camdivn, 9) + 1));
		case 3 : return s3c24xx_get_fclk( device) / (3 * (BIT( s3c24xx->clkpow.regs.camdivn, 8) + 1));
	}
	return 0;
#endif
}

static UINT32 s3c24xx_get_pclk( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	return s3c24xx_get_hclk( device) / (1 << BIT( s3c24xx->clkpow.regs.clkdivn, 0));
}

static READ32_DEVICE_HANDLER( s3c24xx_clkpow_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->clkpow.regs)[offset];
	verboselog( device->machine, 9, "(CLKPOW) %08X -> %08X\n", S3C24XX_BASE_CLKPOW + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_clkpow_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(CLKPOW) %08X <- %08X\n", S3C24XX_BASE_CLKPOW + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->clkpow.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_MPLLCON :
		{
			verboselog( device->machine, 5, "CLKPOW - fclk %d hclk %d pclk %d\n", s3c24xx_get_fclk( device), s3c24xx_get_hclk( device), s3c24xx_get_pclk( device));
			cputag_set_clock( device->machine, "maincpu", s3c24xx_get_fclk( device) * CLOCK_MULTIPLIER);
		}
		break;
	}
}

/* Interrupt Controller */

static void s3c24xx_check_pending_irq( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 temp;
	// normal irq
	temp = (s3c24xx->irq.regs.srcpnd & ~s3c24xx->irq.regs.intmsk) & ~s3c24xx->irq.regs.intmod;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		s3c24xx->irq.regs.intpnd |= (1 << int_type);
		s3c24xx->irq.regs.intoffset = int_type;
		if (s3c24xx->irq.line_irq != ASSERT_LINE)
		{
			cputag_set_input_line( device->machine, "maincpu", ARM7_IRQ_LINE, ASSERT_LINE);
			s3c24xx->irq.line_irq = ASSERT_LINE;
		}
	}
	else
	{
		if (s3c24xx->irq.line_irq != CLEAR_LINE)
		{
			cputag_set_input_line( device->machine, "maincpu", ARM7_IRQ_LINE, CLEAR_LINE);
			s3c24xx->irq.line_irq = CLEAR_LINE;
		}
	}
	// fast irq
	temp = (s3c24xx->irq.regs.srcpnd & ~s3c24xx->irq.regs.intmsk) & s3c24xx->irq.regs.intmod;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		if (s3c24xx->irq.line_fiq != ASSERT_LINE)
		{
			cputag_set_input_line( device->machine, "maincpu", ARM7_FIRQ_LINE, ASSERT_LINE);
			s3c24xx->irq.line_fiq = ASSERT_LINE;
		}
	}
	else
	{
		if (s3c24xx->irq.line_fiq != CLEAR_LINE)
		{
			cputag_set_input_line( device->machine, "maincpu", ARM7_FIRQ_LINE, CLEAR_LINE);
			s3c24xx->irq.line_fiq = CLEAR_LINE;
		}
	}
}

static void s3c24xx_request_irq( device_t *device, UINT32 int_type)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 5, "request irq %d\n", int_type);
	s3c24xx->irq.regs.srcpnd |= (1 << int_type);
	s3c24xx_check_pending_irq( device);
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static void s3c24xx_check_pending_subirq( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 temp = s3c24xx->irq.regs.subsrcpnd & ~s3c24xx->irq.regs.intsubmsk;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		s3c24xx_request_irq( device, MAP_SUBINT_TO_INT[int_type]);
	}
}

ATTR_UNUSED static void s3c24xx_request_subirq( device_t *device, UINT32 int_type)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 5, "request subirq %d\n", int_type);
	s3c24xx->irq.regs.subsrcpnd |= (1 << int_type);
	s3c24xx_check_pending_subirq( device);
}

static void s3c24xx_check_pending_eint( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 temp = s3c24xx->gpio.regs.eintpend & ~s3c24xx->gpio.regs.eintmask;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		if (int_type < 8)
		{
			s3c24xx_request_irq( device, S3C24XX_INT_EINT4_7);
		}
		else
		{
			s3c24xx_request_irq( device, S3C24XX_INT_EINT8_23);
		}
	}
}

ATTR_UNUSED static void s3c24xx_request_eint( device_t *device, UINT32 number)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 5, "request external interrupt %d\n", number);
	if (number < 4)
	{
		s3c24xx_request_irq( device, S3C24XX_INT_EINT0 + number);
	}
	else
	{
		s3c24xx->gpio.regs.eintpend |= (1 << number);
		s3c24xx_check_pending_eint( device);
	}
}

#endif

static READ32_DEVICE_HANDLER( s3c24xx_irq_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->irq.regs)[offset];
	verboselog( device->machine, 9, "(IRQ) %08X -> %08X\n", S3C24XX_BASE_INT + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_irq_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->irq.regs)[offset];
	verboselog( device->machine, 9, "(IRQ) %08X <- %08X\n", S3C24XX_BASE_INT + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->irq.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_SRCPND :
		{
			s3c24xx->irq.regs.srcpnd = (old_value & ~data); // clear only the bit positions of SRCPND corresponding to those set to one in the data
			s3c24xx_check_pending_irq( device);
		}
		break;
		case S3C24XX_INTMSK :
		{
			s3c24xx_check_pending_irq( device);
		}
		break;
		case S3C24XX_INTPND :
		{
			s3c24xx->irq.regs.intpnd = (old_value & ~data); // clear only the bit positions of INTPND corresponding to those set to one in the data
		}
		break;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
		case S3C24XX_SUBSRCPND :
		{
			s3c24xx->irq.regs.subsrcpnd = (old_value & ~data); // clear only the bit positions of SRCPND corresponding to those set to one in the data
			s3c24xx_check_pending_subirq( device);
		}
		break;
		case S3C24XX_INTSUBMSK :
		{
			s3c24xx_check_pending_subirq( device);
		}
		break;
#endif
	}
}

/* PWM Timer */

static UINT16 s3c24xx_pwm_calc_observation( device_t *device, int ch)
{
	s3c24xx_t *s3c24xx = get_token( device);
	double timeleft, x1, x2;
	UINT32 cnto;
	timeleft = attotime_to_double( timer_timeleft( s3c24xx->pwm.timer[ch]));
//  printf( "timeleft %f freq %d cntb %d cmpb %d\n", timeleft, s3c24xx->pwm.freq[ch], s3c24xx->pwm.cnt[ch], s3c24xx->pwm.cmp[ch]);
	x1 = 1 / ((double)s3c24xx->pwm.freq[ch] / (s3c24xx->pwm.cnt[ch]- s3c24xx->pwm.cmp[ch] + 1));
	x2 = x1 / timeleft;
//  printf( "x1 %f\n", x1);
	cnto = s3c24xx->pwm.cmp[ch] + ((s3c24xx->pwm.cnt[ch]- s3c24xx->pwm.cmp[ch]) / x2);
//  printf( "cnto %d\n", cnto);
	return cnto;
}

static READ32_DEVICE_HANDLER( s3c24xx_pwm_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->pwm.regs)[offset];
	switch (offset)
	{
		case S3C24XX_TCNTO0 :
		{
			data = (data & ~0x0000FFFF) | s3c24xx_pwm_calc_observation( device, 0);
		}
		break;
		case S3C24XX_TCNTO1 :
		{
			data = (data & ~0x0000FFFF) | s3c24xx_pwm_calc_observation( device, 1);
		}
		break;
		case S3C24XX_TCNTO2 :
		{
			data = (data & ~0x0000FFFF) | s3c24xx_pwm_calc_observation( device, 2);
		}
		break;
		case S3C24XX_TCNTO3 :
		{
			data = (data & ~0x0000FFFF) | s3c24xx_pwm_calc_observation( device, 3);
		}
		break;
		case S3C24XX_TCNTO4 :
		{
			data = (data & ~0x0000FFFF) | s3c24xx_pwm_calc_observation( device, 4);
		}
		break;
	}
	verboselog( device->machine, 9, "(PWM) %08X -> %08X\n", S3C24XX_BASE_PWM + (offset << 2), data);
	return data;
}

static void s3c24xx_pwm_start( device_t *device, int timer)
{
	s3c24xx_t *s3c24xx = get_token( device);
	const int mux_table[] = { 2, 4, 8, 16};
	const int prescaler_shift[] = { 0, 0, 8, 8, 8};
	const int mux_shift[] = { 0, 4, 8, 12, 16};
	UINT32 pclk, prescaler, mux, cnt, cmp, auto_reload;
	double freq, hz;
	verboselog( device->machine, 1, "PWM %d start\n", timer);
	pclk = s3c24xx_get_pclk( device);
	prescaler = (s3c24xx->pwm.regs.tcfg0 >> prescaler_shift[timer]) & 0xFF;
	mux = (s3c24xx->pwm.regs.tcfg1 >> mux_shift[timer]) & 0x0F;
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
		case 0 :
		{
			cnt = BITS( s3c24xx->pwm.regs.tcntb0, 15, 0);
			cmp = BITS( s3c24xx->pwm.regs.tcmpb0, 15, 0);
			auto_reload = BIT( s3c24xx->pwm.regs.tcon, 3);
		}
		break;
		case 1 :
		{
			cnt = BITS( s3c24xx->pwm.regs.tcntb1, 15, 0);
			cmp = BITS( s3c24xx->pwm.regs.tcmpb1, 15, 0);
			auto_reload = BIT( s3c24xx->pwm.regs.tcon, 11);
		}
		break;
		case 2 :
		{
			cnt = BITS( s3c24xx->pwm.regs.tcntb2, 15, 0);
			cmp = BITS( s3c24xx->pwm.regs.tcmpb2, 15, 0);
			auto_reload = BIT( s3c24xx->pwm.regs.tcon, 15);
		}
		break;
		case 3 :
		{
			cnt = BITS( s3c24xx->pwm.regs.tcntb3, 15, 0);
			cmp = BITS( s3c24xx->pwm.regs.tcmpb3, 15, 0);
			auto_reload = BIT( s3c24xx->pwm.regs.tcon, 19);
		}
		break;
		case 4 :
		{
			cnt = BITS( s3c24xx->pwm.regs.tcntb4, 15, 0);
			cmp = 0;
			auto_reload = BIT( s3c24xx->pwm.regs.tcon, 22);
		}
		break;
		default :
		{
			cnt = cmp = auto_reload = 0;
		}
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
	verboselog( device->machine, 5, "PWM %d - pclk=%d prescaler=%d div=%d freq=%f cnt=%d cmp=%d auto_reload=%d hz=%f\n", timer, pclk, prescaler, mux_table[mux], freq, cnt, cmp, auto_reload, hz);
	s3c24xx->pwm.cnt[timer] = cnt;
	s3c24xx->pwm.cmp[timer] = cmp;
	s3c24xx->pwm.freq[timer] = freq;
	if (auto_reload)
	{
		timer_adjust_periodic( s3c24xx->pwm.timer[timer], ATTOTIME_IN_HZ( hz), timer, ATTOTIME_IN_HZ( hz));
	}
	else
	{
		timer_adjust_oneshot( s3c24xx->pwm.timer[timer], ATTOTIME_IN_HZ( hz), timer);
	}
}

static void s3c24xx_pwm_stop( device_t *device, int timer)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 1, "PWM %d stop\n", timer);
	timer_adjust_oneshot( s3c24xx->pwm.timer[timer], attotime_never, 0);
}

static void s3c24xx_pwm_recalc( device_t *device, int timer)
{
	s3c24xx_t *s3c24xx = get_token( device);
	const int tcon_shift[] = { 0, 8, 12, 16, 20};
	if (s3c24xx->pwm.regs.tcon & (1 << tcon_shift[timer]))
	{
		s3c24xx_pwm_start( device, timer);
	}
	else
	{
		s3c24xx_pwm_stop( device, timer);
	}
}

static WRITE32_DEVICE_HANDLER( s3c24xx_pwm_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->pwm.regs)[offset];
	verboselog( device->machine, 9, "(PWM) %08X <- %08X\n", S3C24XX_BASE_PWM + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->pwm.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_TCON :
		{
			if ((data & (1 << 0)) != (old_value & (1 << 0)))
			{
				s3c24xx_pwm_recalc( device, 0);
			}
			if ((data & (1 << 8)) != (old_value & (1 << 8)))
			{
				s3c24xx_pwm_recalc( device, 1);
			}
			if ((data & (1 << 12)) != (old_value & (1 << 12)))
			{
				s3c24xx_pwm_recalc( device, 2);
			}
			if ((data & (1 << 16)) != (old_value & (1 << 16)))
			{
				s3c24xx_pwm_recalc( device, 3);
			}
			if ((data & (1 << 20)) != (old_value & (1 << 20)))
			{
				s3c24xx_pwm_recalc( device, 4);
			}
		}
		break;
	}
}

static TIMER_CALLBACK( s3c24xx_pwm_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c24xx_t *s3c24xx = get_token( device);
	int ch = param;
	const int ch_int[] = { S3C24XX_INT_TIMER0, S3C24XX_INT_TIMER1, S3C24XX_INT_TIMER2, S3C24XX_INT_TIMER3, S3C24XX_INT_TIMER4 };
	verboselog( machine, 2, "PWM %d timer callback\n", ch);
	if (BITS( s3c24xx->pwm.regs.tcfg1, 23, 20) == (ch + 1))
	{
		s3c24xx_dma_request_pwm( device);
	}
	else
	{
		s3c24xx_request_irq( device, ch_int[ch]);
	}
}

/* DMA */

static void s3c24xx_dma_reload( device_t *device, int ch)
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx_dma_regs_t *regs = &s3c24xx->dma[ch].regs;
	regs->dstat = S3C24XX_DSTAT_SET_CURR_TC( regs->dstat, S3C24XX_DCON_GET_TC( regs->dcon));
	regs->dcsrc = S3C24XX_DCSRC_SET_CURR_SRC( regs->dcsrc, S3C24XX_DISRC_GET_SADDR( regs->disrc));
	regs->dcdst = S3C24XX_DCDST_SET_CURR_DST( regs->dcdst, S3C24XX_DIDST_GET_DADDR( regs->didst));
}

static void s3c24xx_dma_trigger( device_t *device, int ch)
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx_dma_regs_t *regs = &s3c24xx->dma[ch].regs;
	UINT32 curr_tc, curr_src, curr_dst;
	address_space *space = cputag_get_address_space( device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int dsz, inc_src, inc_dst, servmode, tsz;
	const UINT32 ch_int[] = { S3C24XX_INT_DMA0, S3C24XX_INT_DMA1, S3C24XX_INT_DMA2, S3C24XX_INT_DMA3};
	verboselog( device->machine, 5, "DMA %d trigger\n", ch);
	curr_tc = S3C24XX_DSTAT_GET_CURR_TC( regs->dstat);
	dsz = S3C24XX_DCON_GET_DSZ( regs->dcon);
	curr_src = S3C24XX_DCSRC_GET_CURR_SRC( regs->dcsrc);
	curr_dst = S3C24XX_DCDST_GET_CURR_DST( regs->dcdst);
	servmode = S3C24XX_DCON_GET_SERVMODE( regs->dcon);
	tsz = S3C24XX_DCON_GET_TSZ( regs->dcon);
#if defined(DEVICE_S3C2400)
	inc_src = BIT( regs->disrc, 29);
	inc_dst = BIT( regs->didst, 29);
#else
	inc_src = BIT( regs->disrcc, 0);
	inc_dst = BIT( regs->didstc, 0);
#endif
	verboselog( device->machine, 5, "DMA %d - curr_src %08X curr_dst %08X curr_tc %d dsz %d\n", ch, curr_src, curr_dst, curr_tc, dsz);
	while (curr_tc > 0)
	{
		curr_tc--;
		for (int i = 0; i < 1 << (tsz << 1); i++)
		{
			switch (dsz)
			{
				case 0 : space->write_byte( curr_dst, space->read_byte( curr_src)); break;
				case 1 : space->write_word( curr_dst, space->read_word( curr_src)); break;
				case 2 : space->write_dword( curr_dst, space->read_dword( curr_src)); break;
			}
			if (inc_src == 0) curr_src += (1 << dsz);
			if (inc_dst == 0) curr_dst += (1 << dsz);
		}
		if (servmode == 0) break;
	}
	regs->dcsrc = S3C24XX_DCSRC_SET_CURR_SRC( regs->dcsrc, curr_src);
	regs->dcdst = S3C24XX_DCDST_SET_CURR_DST( regs->dcdst, curr_dst);
	regs->dstat = S3C24XX_DSTAT_SET_CURR_TC( regs->dstat, curr_tc);
	if (curr_tc == 0)
	{
		if (S3C24XX_DCON_GET_RELOAD( regs->dcon) == 0)
		{
			s3c24xx_dma_reload( device, ch);
		}
		else
		{
			regs->dmasktrig &= ~(1 << 1); // clear on/off
		}
		if (S3C24XX_DCON_GET_INT( regs->dcon) != 0)
		{
			s3c24xx_request_irq( device, ch_int[ch]);
		}
	}
}

static void s3c24xx_dma_request_iis( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx_dma_regs_t *regs = &s3c24xx->dma[2].regs;
	verboselog( device->machine, 5, "s3c24xx_dma_request_iis\n");
	if ((S3C24XX_DMASKTRIG_GET_ON_OFF( regs->dmasktrig) != 0) && (S3C24XX_DCON_GET_SWHWSEL( regs->dcon) != 0) && (S3C24XX_DCON_GET_HWSRCSEL( regs->dcon) == 0))
	{
		s3c24xx_dma_trigger( device, 2);
	}
}

static void s3c24xx_dma_request_pwm( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 5, "s3c24xx_dma_request_pwm\n");
	for (int i = 0; i < 4; i++)
	{
		if (i != 1)
		{
			s3c24xx_dma_regs_t *regs = &s3c24xx->dma[i].regs;
			if ((S3C24XX_DMASKTRIG_GET_ON_OFF( regs->dmasktrig) != 0) && (S3C24XX_DCON_GET_SWHWSEL( regs->dcon) != 0) && (S3C24XX_DCON_GET_HWSRCSEL( regs->dcon) == 3))
			{
				s3c24xx_dma_trigger( device, i);
			}
		}
	}
}

static void s3c24xx_dma_start( device_t *device, int ch)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 addr_src, addr_dst, tc;
	s3c24xx_dma_regs_t *regs = &s3c24xx->dma[ch].regs;
	UINT32 dsz, tsz, reload;
	int inc_src, inc_dst, _int, servmode, swhwsel, hwsrcsel;
	verboselog( device->machine, 1, "DMA %d start\n", ch);
	addr_src = S3C24XX_DISRC_GET_SADDR( regs->disrc);
	addr_dst = S3C24XX_DIDST_GET_DADDR( regs->didst);
	tc = S3C24XX_DCON_GET_TC( regs->dcon);
	_int = S3C24XX_DCON_GET_INT( regs->dcon);
	servmode = S3C24XX_DCON_GET_SERVMODE( regs->dcon);
	hwsrcsel = S3C24XX_DCON_GET_HWSRCSEL( regs->dcon);
	swhwsel = S3C24XX_DCON_GET_SWHWSEL( regs->dcon);
	reload = S3C24XX_DCON_GET_RELOAD( regs->dcon);
	dsz = S3C24XX_DCON_GET_DSZ( regs->dcon);
	tsz = S3C24XX_DCON_GET_TSZ( regs->dcon);
#if defined(DEVICE_S3C2400)
	inc_src = BIT( regs->disrc, 29);
	inc_dst = BIT( regs->didst, 29);
#else
	inc_src = BIT( regs->disrcc, 0);
	inc_dst = BIT( regs->didstc, 0);
#endif
	verboselog( device->machine, 5, "DMA %d - addr_src %08X inc_src %d addr_dst %08X inc_dst %d int %d tsz %d servmode %d hwsrcsel %d swhwsel %d reload %d dsz %d tc %d\n", ch, addr_src, inc_src, addr_dst, inc_dst, _int, tsz, servmode, hwsrcsel, swhwsel, reload, dsz, tc);
	verboselog( device->machine, 5, "DMA %d - copy %08X bytes from %08X (%s) to %08X (%s)\n", ch, (tc << dsz) << (tsz << 1), addr_src, inc_src ? "fix" : "inc", addr_dst, inc_dst ? "fix" : "inc");
	s3c24xx_dma_reload( device, ch);
	if (swhwsel == 0)
	{
		s3c24xx_dma_trigger( device, ch);
	}
}

static void s3c24xx_dma_stop( device_t *device, int ch)
{
	verboselog( device->machine, 1, "DMA %d stop\n", ch);
}

static void s3c24xx_dma_recalc( device_t *device, int ch)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if ((s3c24xx->dma[ch].regs.dmasktrig & (1 << 1)) != 0)
	{
		s3c24xx_dma_start( device, ch);
	}
	else
	{
		s3c24xx_dma_stop( device, ch);
	}
}

static UINT32 s3c24xx_dma_r( device_t *device, UINT32 ch, UINT32 offset)
{
	s3c24xx_t *s3c24xx = get_token( device);
	return ((UINT32*)&s3c24xx->dma[ch].regs)[offset];
}

static void s3c24xx_dma_w( device_t *device, UINT32 ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->dma[ch].regs)[offset];
	COMBINE_DATA(&((UINT32*)&s3c24xx->dma[ch].regs)[offset]);
	switch (offset)
	{
		case S3C24XX_DCON :
		{
			#if 0 // is this code necessary ???
			if ((data & (1 << 22)) != 0) // reload
			{
				s3c24xx_dma_regs_t *regs = &s3c24xx->dma[ch].regs;
				regs->dmasktrig &= ~(1 << 1); // clear on/off
			}
			#endif
		}
		break;
		case S3C24XX_DMASKTRIG :
		{
			if ((old_value & (1 << 1)) != (data & (1 << 1)))
			{
				s3c24xx_dma_recalc( device, ch);
			}
		}
		break;
	}
}

static READ32_DEVICE_HANDLER( s3c24xx_dma_0_r )
{
	UINT32 data = s3c24xx_dma_r( device, 0, offset);
	verboselog( device->machine, 9, "(DMA 0) %08X -> %08X\n", S3C24XX_BASE_DMA_0 + (offset << 2), data);
	return data;
}

static READ32_DEVICE_HANDLER( s3c24xx_dma_1_r )
{
	UINT32 data = s3c24xx_dma_r( device, 1, offset);
	verboselog( device->machine, 9, "(DMA 1) %08X -> %08X\n", S3C24XX_BASE_DMA_1 + (offset << 2), data);
	return data;
}

static READ32_DEVICE_HANDLER( s3c24xx_dma_2_r )
{
	UINT32 data = s3c24xx_dma_r( device, 2, offset);
	verboselog( device->machine, 9, "(DMA 2) %08X -> %08X\n", S3C24XX_BASE_DMA_2 + (offset << 2), data);
	return data;
}

static READ32_DEVICE_HANDLER( s3c24xx_dma_3_r )
{
	UINT32 data = s3c24xx_dma_r( device, 3, offset);
	verboselog( device->machine, 9, "(DMA 3) %08X -> %08X\n", S3C24XX_BASE_DMA_3 + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_dma_0_w )
{
	verboselog( device->machine, 9, "(DMA 0) %08X <- %08X\n", S3C24XX_BASE_DMA_0 + (offset << 2), data);
	s3c24xx_dma_w( device, 0, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( s3c24xx_dma_1_w )
{
	verboselog( device->machine, 9, "(DMA 1) %08X <- %08X\n", S3C24XX_BASE_DMA_1 + (offset << 2), data);
	s3c24xx_dma_w( device, 1, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( s3c24xx_dma_2_w )
{
	verboselog( device->machine, 9, "(DMA 2) %08X <- %08X\n", S3C24XX_BASE_DMA_2 + (offset << 2), data);
	s3c24xx_dma_w( device, 2, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( s3c24xx_dma_3_w )
{
	verboselog( device->machine, 9, "(DMA 3) %08X <- %08X\n", S3C24XX_BASE_DMA_3 + (offset << 2), data);
	s3c24xx_dma_w( device, 3, offset, data, mem_mask);
}

static TIMER_CALLBACK( s3c24xx_dma_timer_exp )
{
	int ch = param;
	verboselog( machine, 2, "DMA %d timer callback\n", ch);
}

/* I/O Port */

INLINE UINT32 iface_gpio_port_r( device_t *device, int port)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->gpio.port_r)
	{
		return (s3c24xx->iface->gpio.port_r)( device, port);
	}
	else
	{
		return 0;
	}
}

INLINE void iface_gpio_port_w( device_t *device, int port, UINT32 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->gpio.port_w)
	{
		(s3c24xx->iface->gpio.port_w)( device, port, data);
	}
}

static READ32_DEVICE_HANDLER( s3c24xx_gpio_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->gpio.regs)[offset];
	switch (offset)
	{
		case S3C24XX_GPADAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_A) & S3C24XX_GPADAT_MASK;
		}
		break;
		case S3C24XX_GPBDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_B) & S3C24XX_GPBDAT_MASK;
		}
		break;
		case S3C24XX_GPCDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_C) & S3C24XX_GPCDAT_MASK;
		}
		break;
		case S3C24XX_GPDDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_D) & S3C24XX_GPDDAT_MASK;
		}
		break;
		case S3C24XX_GPEDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_E) & S3C24XX_GPEDAT_MASK;
		}
		break;
		case S3C24XX_GPFDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_F) & S3C24XX_GPFDAT_MASK;
		}
		break;
		case S3C24XX_GPGDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_G) & S3C24XX_GPGDAT_MASK;
		}
		break;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
		case S3C24XX_GPHDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_H) & S3C24XX_GPHDAT_MASK;
		}
		break;
#endif
#if defined(DEVICE_S3C2440)
		case S3C24XX_GPJDAT :
		{
			data = iface_gpio_port_r( device, S3C24XX_GPIO_PORT_J) & S3C24XX_GPJDAT_MASK;
		}
		break;
#endif
	}
	verboselog( device->machine, 9, "(GPIO) %08X -> %08X\n", S3C24XX_BASE_GPIO + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_gpio_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	UINT32 old_value = ((UINT32*)&s3c24xx->gpio.regs)[offset];
#endif
	verboselog( device->machine, 9, "(GPIO) %08X <- %08X\n", S3C24XX_BASE_GPIO + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->gpio.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_GPADAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_A, data & S3C24XX_GPADAT_MASK);
		}
		break;
		case S3C24XX_GPBDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_B, data & S3C24XX_GPBDAT_MASK);
		}
		break;
		case S3C24XX_GPCDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_C, data & S3C24XX_GPCDAT_MASK);
		}
		break;
		case S3C24XX_GPDDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_D, data & S3C24XX_GPDDAT_MASK);
		}
		break;
		case S3C24XX_GPEDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_E, data & S3C24XX_GPEDAT_MASK);
		}
		break;
		case S3C24XX_GPFDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_F, data & S3C24XX_GPFDAT_MASK);
		}
		break;
		case S3C24XX_GPGDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_G, data & S3C24XX_GPGDAT_MASK);
		}
		break;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
		case S3C24XX_GPHDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_H, data & S3C24XX_GPHDAT_MASK);
		}
		break;
		case S3C24XX_EINTPEND :
		{
			s3c24xx->gpio.regs.eintpend = (old_value & ~data);
			s3c24xx_check_pending_eint( device);
		}
		break;
		case S3C24XX_EINTMASK :
		{
			s3c24xx_check_pending_eint( device);
		}
		break;
#endif
#if defined(DEVICE_S3C2440)
		case S3C24XX_GPJDAT :
		{
			 iface_gpio_port_w( device, S3C24XX_GPIO_PORT_J, data & S3C24XX_GPJDAT_MASK);
		}
		break;
#endif
	}
}

/* Memory Controller */

static READ32_DEVICE_HANDLER( s3c24xx_memcon_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->memcon.regs.data[offset];
	verboselog( device->machine, 9, "(MEMCON) %08X -> %08X\n", S3C24XX_BASE_MEMCON + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_memcon_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(MEMCON) %08X <- %08X\n", S3C24XX_BASE_MEMCON + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->memcon.regs.data[offset]);
}

/* USB Host Controller */

static READ32_DEVICE_HANDLER( s3c24xx_usb_host_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->usbhost.regs.data[offset];
	switch (offset)
	{
		// HcCommandStatus
		case 0x08 / 4 :
		{
			data = data & ~(1 << 0); // [bit 0] HostControllerReset
		}
		break;
	}
	verboselog( device->machine, 9, "(USB H) %08X -> %08X\n", S3C24XX_BASE_USBHOST + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_usb_host_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(USB H) %08X <- %08X\n", S3C24XX_BASE_USBHOST + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->usbhost.regs.data[offset]);
}

/* UART */

static UINT32 s3c24xx_uart_r( device_t *device, UINT32 ch, UINT32 offset)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->uart[ch].regs)[offset];
	switch (offset)
	{
		case S3C24XX_UTRSTAT :
		{
			data = (data & ~0x00000006) | 0x00000004 | 0x00000002; // [bit 2] Transmitter empty / [bit 1] Transmit buffer empty
		}
		break;
		case S3C24XX_URXH :
		{
			UINT8 rxdata = data & 0xFF;
			verboselog( device->machine, 5, "UART %d read %02X (%c)\n", ch, rxdata, ((rxdata >= 32) && (rxdata < 128)) ? (char)rxdata : '?');
			s3c24xx->uart[ch].regs.utrstat &= ~1; // [bit 0] Receive buffer data ready
		}
		break;
	}
	return data;
}

static void s3c24xx_uart_w( device_t *device, UINT32 ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	s3c24xx_t *s3c24xx = get_token( device);
	COMBINE_DATA(&((UINT32*)&s3c24xx->uart[ch].regs)[offset]);
	switch (offset)
	{
		case S3C24XX_UTXH :
		{
			UINT8 txdata = data & 0xFF;
			verboselog( device->machine, 5, "UART %d write %02X (%c)\n", ch, txdata, ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#ifdef UART_PRINTF
			printf( "%c", ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#endif
		}
		break;
	}
}

static READ32_DEVICE_HANDLER( s3c24xx_uart_0_r )
{
	UINT32 data = s3c24xx_uart_r( device, 0, offset);
//  verboselog( device->machine, 9, "(UART 0) %08X -> %08X\n", S3C24XX_BASE_UART_0 + (offset << 2), data);
	return data;
}

static READ32_DEVICE_HANDLER( s3c24xx_uart_1_r )
{
	UINT32 data = s3c24xx_uart_r( device, 1, offset);
//  verboselog( device->machine, 9, "(UART 1) %08X -> %08X\n", S3C24XX_BASE_UART_1 + (offset << 2), data);
	return data;
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static READ32_DEVICE_HANDLER( s3c24xx_uart_2_r )
{
	UINT32 data = s3c24xx_uart_r( device, 2, offset);
//  verboselog( device->machine, 9, "(UART 2) %08X -> %08X\n", S3C24XX_BASE_UART_2 + (offset << 2), data);
	return data;
}

#endif

static WRITE32_DEVICE_HANDLER( s3c24xx_uart_0_w )
{
//  verboselog( device->machine, 9, "(UART 0) %08X <- %08X\n", S3C24XX_BASE_UART_0 + (offset << 2), data);
	s3c24xx_uart_w( device, 0, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( s3c24xx_uart_1_w )
{
//  verboselog( device->machine, 9, "(UART 1) %08X <- %08X\n", S3C24XX_BASE_UART_1 + (offset << 2), data);
	s3c24xx_uart_w( device, 1, offset, data, mem_mask);
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static WRITE32_DEVICE_HANDLER( s3c24xx_uart_2_w )
{
//  verboselog( device->machine, 9, "(UART 2) %08X <- %08X\n", S3C24XX_BASE_UART_2 + (offset << 2), data);
	s3c24xx_uart_w( device, 2, offset, data, mem_mask);
}

#endif

static void s3c24xx_uart_fifo_w( device_t *device, int uart, UINT8 data)
{
//  printf( "s3c24xx_uart_fifo_w (%c)\n", data);
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx->uart[uart].regs.urxh = data;
	s3c24xx->uart[uart].regs.utrstat |= 1; // [bit 0] Receive buffer data ready
}

/* USB Device */

static READ32_DEVICE_HANDLER( s3c24xx_usb_device_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->usbdev.regs.data[offset];
	verboselog( device->machine, 9, "(USB D) %08X -> %08X\n", S3C24XX_BASE_USBDEV + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_usb_device_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(USB D) %08X <- %08X\n", S3C24XX_BASE_USBDEV + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->usbdev.regs.data[offset]);
}

/* Watchdog Timer */

#if defined(DEVICE_S3C2410)

static UINT16 s3c24xx_wdt_calc_current_count( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	double timeleft, x1, x2;
	UINT32 cnt;
	timeleft = attotime_to_double( timer_timeleft( s3c24xx->wdt.timer));
//  printf( "timeleft %f freq %d cnt %d\n", timeleft, s3c24xx->wdt.freq, s3c24xx->wdt.cnt);
	x1 = 1 / ((double)s3c24xx->wdt.freq / s3c24xx->wdt.cnt);
	x2 = x1 / timeleft;
//  printf( "x1 %f\n", x1);
	cnt = s3c24xx->wdt.cnt / x2;
//  printf( "cnt %d\n", cnt);
	return cnt;
}

#else

static UINT16 s3c24xx_wdt_calc_current_count( device_t *device)
{
	return 0;
}

#endif

static READ32_DEVICE_HANDLER( s3c24xx_wdt_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->wdt.regs)[offset];
	switch (offset)
	{
		case S3C24XX_WTCNT :
		{
			// is wdt active?
			if ((s3c24xx->wdt.regs.wtcon & (1 << 5)) != 0)
			{
				data = s3c24xx_wdt_calc_current_count( device);
			}
		}
		break;
	}
	verboselog( device->machine, 9, "(WDT) %08X -> %08X\n", S3C24XX_BASE_WDT + (offset << 2), data);
	return data;
}

static void s3c24xx_wdt_start( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 pclk, prescaler, clock;
	double freq, hz;
	verboselog( device->machine, 1, "WDT start\n");
	pclk = s3c24xx_get_pclk( device);
	prescaler = BITS( s3c24xx->wdt.regs.wtcon, 15, 8);
	clock = 16 << BITS( s3c24xx->wdt.regs.wtcon, 4, 3);
	freq = (double)pclk / (prescaler + 1) / clock;
	hz = freq / s3c24xx->wdt.regs.wtcnt;
	verboselog( device->machine, 5, "WDT pclk %d prescaler %d clock %d freq %f hz %f\n", pclk, prescaler, clock, freq, hz);
	timer_adjust_periodic( s3c24xx->wdt.timer, ATTOTIME_IN_HZ( hz), 0, ATTOTIME_IN_HZ( hz));
#if defined(DEVICE_S3C2410)
	s3c24xx->wdt.freq = freq;
	s3c24xx->wdt.cnt = s3c24xx->wdt.regs.wtcnt;
#endif
}

static void s3c24xx_wdt_stop( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 1, "WDT stop\n");
	s3c24xx->wdt.regs.wtcnt = s3c24xx_wdt_calc_current_count( device);
	timer_adjust_oneshot( s3c24xx->wdt.timer, attotime_never, 0);
}

static void s3c24xx_wdt_recalc( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if ((s3c24xx->wdt.regs.wtcon & (1 << 5)) != 0)
	{
		s3c24xx_wdt_start( device);
	}
	else
	{
		s3c24xx_wdt_stop( device);
	}
}

static WRITE32_DEVICE_HANDLER( s3c24xx_wdt_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->wdt.regs)[offset];
	verboselog( device->machine, 9, "(WDT) %08X <- %08X\n", S3C24XX_BASE_WDT + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->wdt.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_WTCON :
		{
			if ((data & (1 << 5)) != (old_value & (1 << 5)))
			{
				s3c24xx_wdt_recalc( device);
			}
		}
		break;
	}
}

static TIMER_CALLBACK( s3c24xx_wdt_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( machine, 2, "WDT timer callback\n");
	if ((s3c24xx->wdt.regs.wtcon & (1 << 2)) != 0)
	{
#if defined(DEVICE_S3C2400) || defined(DEVICE_S3C2410)
		s3c24xx_request_irq( device, S3C24XX_INT_WDT);
#else
		s3c24xx_request_subirq( device, S3C24XX_SUBINT_WDT);
#endif
	}
	if ((s3c24xx->wdt.regs.wtcon & (1 << 0)) != 0)
	{
		s3c24xx_reset( device);
	}
}

/* IIC */

INLINE void iface_i2c_scl_w( device_t *device, int state)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->i2c.scl_w)
	{
		(s3c24xx->iface->i2c.scl_w)( device, state);
	}
}

INLINE void iface_i2c_sda_w( device_t *device, int state)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->i2c.sda_w)
	{
		(s3c24xx->iface->i2c.sda_w)( device, state);
	}
}

INLINE int iface_i2c_sda_r( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->i2c.sda_r)
	{
		return (s3c24xx->iface->i2c.sda_r)( device);
	}
	else
	{
		return 0;
	}
}

static void i2c_send_start( device_t *device)
{
	verboselog( device->machine, 5, "i2c_send_start\n");
	iface_i2c_sda_w( device, 1);
	iface_i2c_scl_w( device, 1);
	iface_i2c_sda_w( device, 0);
	iface_i2c_scl_w( device, 0);
}

static void i2c_send_stop( device_t *device)
{
	verboselog( device->machine, 5, "i2c_send_stop\n");
	iface_i2c_sda_w( device, 0);
	iface_i2c_scl_w( device, 1);
	iface_i2c_sda_w( device, 1);
	iface_i2c_scl_w( device, 0);
}

static UINT8 i2c_receive_byte( device_t *device, int ack)
{
	UINT8 data = 0;
	verboselog( device->machine, 5, "i2c_receive_byte ...\n");
	iface_i2c_sda_w( device, 1);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_scl_w( device, 1);
		data = (data << 1) + (iface_i2c_sda_r( device) ? 1 : 0);
		iface_i2c_scl_w( device, 0);
	}
	verboselog( device->machine, 5, "recv data %02X\n", data);
	verboselog( device->machine, 5, "send ack %d\n", ack);
	iface_i2c_sda_w( device, ack ? 0 : 1);
	iface_i2c_scl_w( device, 1);
	iface_i2c_scl_w( device, 0);
	return data;
}

static int i2c_send_byte( device_t *device, UINT8 data)
{
	int ack;
	verboselog( device->machine, 5, "i2c_send_byte ...\n");
	verboselog( device->machine, 5, "send data %02X\n", data);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_sda_w( device, (data & 0x80) ? 1 : 0);
		data = data << 1;
		iface_i2c_scl_w( device, 1);
		iface_i2c_scl_w( device, 0);
	}
	iface_i2c_sda_w( device, 1); // ack bit
	iface_i2c_scl_w( device, 1);
	ack = iface_i2c_sda_r( device);
	verboselog( device->machine, 5, "recv ack %d\n", ack);
	iface_i2c_scl_w( device, 0);
	return ack;
}

static void iic_start( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	int mode_selection;
	verboselog( device->machine, 1, "IIC start\n");
	i2c_send_start( device);
	mode_selection = BITS( s3c24xx->iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : i2c_send_byte( device, s3c24xx->iic.regs.iicds | 0x01); break;
		case 3 : i2c_send_byte( device, s3c24xx->iic.regs.iicds & 0xFE); break;
	}
	timer_adjust_oneshot( s3c24xx->iic.timer, ATTOTIME_IN_USEC( 1), 0);
}

static void iic_stop( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 1, "IIC stop\n");
	i2c_send_stop( device);
	timer_adjust_oneshot( s3c24xx->iic.timer, attotime_never, 0);
}

static void iic_resume( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	int mode_selection;
	verboselog( device->machine, 1, "IIC resume\n");
	mode_selection = BITS( s3c24xx->iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : s3c24xx->iic.regs.iicds = i2c_receive_byte( device, BIT( s3c24xx->iic.regs.iiccon, 7)); break;
		case 3 : i2c_send_byte( device, s3c24xx->iic.regs.iicds & 0xFF); break;
	}
	timer_adjust_oneshot( s3c24xx->iic.timer, ATTOTIME_IN_USEC( 1), 0);
}

static READ32_DEVICE_HANDLER( s3c24xx_iic_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->iic.regs)[offset];
	switch (offset)
	{
		case S3C24XX_IICSTAT :
		{
			data = data & ~0x0000000F;
		}
		break;
	}
	verboselog( device->machine, 9, "(IIC) %08X -> %08X\n", S3C24XX_BASE_IIC + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_iic_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->iic.regs)[offset];
	verboselog( device->machine, 9, "(IIC) %08X <- %08X\n", S3C24XX_BASE_IIC + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->iic.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_IICCON :
		{
			int interrupt_pending_flag;
#if 0
			const int div_table[] = { 16, 512};
			int enable_interrupt, transmit_clock_value, tx_clock_source_selection
			double clock;
			transmit_clock_value = (data >> 0) & 0xF;
			tx_clock_source_selection = (data >> 6) & 1;
			enable_interrupt = (data >> 5) & 1;
			clock = (double)s3c24xx_get_pclk( device) / div_table[tx_clock_source_selection] / (transmit_clock_value + 1);
#endif
			interrupt_pending_flag = BIT( old_value, 4);
			if (interrupt_pending_flag != 0)
			{
				interrupt_pending_flag = BIT( data, 4);
				if (interrupt_pending_flag == 0)
				{
					int start_stop_condition;
					start_stop_condition = BIT( s3c24xx->iic.regs.iicstat, 5);
					if (start_stop_condition != 0)
					{
						if (s3c24xx->iic.count == 0)
						{
							iic_start( device);

						}
						else
						{
							iic_resume( device);
						}
					}
					else
					{
						iic_stop( device);
					}
				}
			}
		}
		break;
		case  S3C24XX_IICSTAT :
		{
			int interrupt_pending_flag;
			s3c24xx->iic.count = 0;
			interrupt_pending_flag = BIT( s3c24xx->iic.regs.iiccon, 4);
			if (interrupt_pending_flag == 0)
			{
				int start_stop_condition;
				start_stop_condition = BIT( data, 5);
				if (start_stop_condition != 0)
				{
					if (s3c24xx->iic.count == 0)
					{
						iic_start( device);

					}
					else
					{
						iic_resume( device);
					}
				}
				else
				{
					iic_stop( device);
				}
			}
		}
		break;
	}
}

static TIMER_CALLBACK( s3c24xx_iic_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c24xx_t *s3c24xx = get_token( device);
	int enable_interrupt;
	verboselog( machine, 2, "IIC timer callback\n");
	s3c24xx->iic.count++;
	enable_interrupt = BIT( s3c24xx->iic.regs.iiccon, 5);
	if (enable_interrupt)
	{
		s3c24xx->iic.regs.iiccon |= (1 << 4); // [bit 4] interrupt is pending
		s3c24xx_request_irq( device, S3C24XX_INT_IIC);
	}
}

/* IIS */

INLINE void iface_i2s_data_w( device_t *device, int ch, UINT16 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->i2s.data_w)
	{
		(s3c24xx->iface->i2s.data_w)( device, ch, data, 0);
	}
}

static void s3c24xx_iis_start( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	const UINT32 codeclk_table[] = { 256, 384};
	double freq;
	int pclk, prescaler_enable, prescaler_control_a, prescaler_control_b, codeclk;
	verboselog( device->machine, 1, "IIS start\n");
	prescaler_enable = BIT( s3c24xx->iis.regs.iiscon, 1);
	prescaler_control_a = BITS( s3c24xx->iis.regs.iispsr, 9, 5);
	prescaler_control_b = BITS( s3c24xx->iis.regs.iispsr, 4, 0);
	codeclk = BIT( s3c24xx->iis.regs.iismod, 2);
	pclk = s3c24xx_get_pclk( device);
	freq = ((double)pclk / (prescaler_control_a + 1) / codeclk_table[codeclk]) * 2; // why do I have to multiply by two?
	verboselog( device->machine, 5, "IIS - pclk %d psc_enable %d psc_a %d psc_b %d codeclk %d freq %f\n", pclk, prescaler_enable, prescaler_control_a, prescaler_control_b, codeclk_table[codeclk], freq);
	timer_adjust_periodic( s3c24xx->iis.timer, ATTOTIME_IN_HZ( freq), 0, ATTOTIME_IN_HZ( freq));
}

static void s3c24xx_iis_stop( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 1, "IIS stop\n");
	timer_adjust_oneshot( s3c24xx->iis.timer, attotime_never, 0);
}

static void s3c24xx_iis_recalc( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if ((s3c24xx->iis.regs.iiscon & (1 << 0)) != 0)
	{
		s3c24xx_iis_start( device);
	}
	else
	{
		s3c24xx_iis_stop( device);
	}
}

static READ32_DEVICE_HANDLER( s3c24xx_iis_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->iis.regs)[offset];
#if 0
	switch (offset)
	{
		case S3C24XX_IISCON :
		{
			data = data & ~1; // hack for mp3 player
		}
		break;
	}
#endif
	verboselog( device->machine, 9, "(IIS) %08X -> %08X\n", S3C24XX_BASE_IIS + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_iis_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->iis.regs)[offset];
	verboselog( device->machine, 9, "(IIS) %08X <- %08X\n", S3C24XX_BASE_IIS + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->iis.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_IISCON :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				s3c24xx_iis_recalc( device);
			}
		}
		break;
		case S3C24XX_IISFIFO :
		{
			if (ACCESSING_BITS_16_31)
			{
				s3c24xx->iis.fifo[s3c24xx->iis.fifo_index++] = BITS( data, 31, 16);
			}
			if (ACCESSING_BITS_0_15)
			{
				s3c24xx->iis.fifo[s3c24xx->iis.fifo_index++] = BITS( data, 15, 0);
			}
			if (s3c24xx->iis.fifo_index == 2)
			{
				s3c24xx->iis.fifo_index = 0;
				iface_i2s_data_w( device, 0, s3c24xx->iis.fifo[0]);
				iface_i2s_data_w( device, 1, s3c24xx->iis.fifo[1]);
			}
		}
		break;
	}
}

static TIMER_CALLBACK( s3c24xx_iis_timer_exp )
{
	device_t *device = (device_t *)ptr;
	verboselog( machine, 2, "IIS timer callback\n");
	s3c24xx_dma_request_iis( device);
}

/* RTC */

static void s3c24xx_rtc_init( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx->rtc.regs.bcdsec = dec_2_bcd( 0);
	s3c24xx->rtc.regs.bcdmin = dec_2_bcd( 0);
	s3c24xx->rtc.regs.bcdhour = dec_2_bcd( 0);
	s3c24xx->rtc.regs.bcdday = dec_2_bcd( 1);
	s3c24xx->rtc.regs.bcddow = dec_2_bcd( 1);
	s3c24xx->rtc.regs.bcdmon = dec_2_bcd( 1);
	s3c24xx->rtc.regs.bcdyear = dec_2_bcd( 4);
}

static READ32_DEVICE_HANDLER( s3c24xx_rtc_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->rtc.regs)[offset];
	verboselog( device->machine, 9, "(RTC) %08X -> %08X\n", S3C24XX_BASE_RTC + (offset << 2), data);
	return data;
}

static void s3c24xx_rtc_recalc( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->rtc.regs.ticnt & (1 << 7))
	{
		UINT32 ttc;
		double freq;
		ttc = BITS( s3c24xx->rtc.regs.ticnt, 6, 0);
		freq = 128 / (ttc + 1);
//      printf( "ttc %d freq %f\n", ttc, freq);
		timer_adjust_periodic( s3c24xx->rtc.timer_tick_count, ATTOTIME_IN_HZ( freq), 0, ATTOTIME_IN_HZ( freq));
	}
	else
	{
		timer_adjust_oneshot( s3c24xx->rtc.timer_tick_count, attotime_never, 0);
	}
}

static WRITE32_DEVICE_HANDLER( s3c24xx_rtc_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(RTC) %08X <- %08X\n", S3C24XX_BASE_RTC + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->rtc.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_TICNT :
		{
			s3c24xx_rtc_recalc( device);
		}
		break;
	}
}

static TIMER_CALLBACK( s3c24xx_rtc_timer_tick_count_exp )
{
	device_t *device = (device_t *)ptr;
	verboselog( machine, 2, "RTC timer callback (tick count)\n");
	s3c24xx_request_irq( device, S3C24XX_INT_TICK);
}

static void s3c24xx_rtc_update( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 bcdday_max;
	// increase second
	s3c24xx->rtc.regs.bcdsec = bcd_adjust( s3c24xx->rtc.regs.bcdsec + 1);
	if (s3c24xx->rtc.regs.bcdsec >= 0x60)
	{
		s3c24xx->rtc.regs.bcdsec = 0;
		// increase minute
		s3c24xx->rtc.regs.bcdmin = bcd_adjust( s3c24xx->rtc.regs.bcdmin + 1);
		if (s3c24xx->rtc.regs.bcdmin >= 0x60)
		{
			s3c24xx->rtc.regs.bcdmin = 0;
			// increase hour
			s3c24xx->rtc.regs.bcdhour = bcd_adjust( s3c24xx->rtc.regs.bcdhour + 1);
			if (s3c24xx->rtc.regs.bcdhour >= 0x24)
			{
				s3c24xx->rtc.regs.bcdhour = 0;
				// increase day-of-week
				s3c24xx->rtc.regs.bcddow = (s3c24xx->rtc.regs.bcddow % 7) + 1;
				// increase day
				s3c24xx->rtc.regs.bcdday = bcd_adjust( s3c24xx->rtc.regs.bcdday + 1);
				bcdday_max = dec_2_bcd( gregorian_days_in_month( bcd_2_dec( s3c24xx->rtc.regs.bcdmon), bcd_2_dec( s3c24xx->rtc.regs.bcdyear) + 2000));
				if (s3c24xx->rtc.regs.bcdday > bcdday_max)
				{
					s3c24xx->rtc.regs.bcdday = 1;
					// increase month
					s3c24xx->rtc.regs.bcdmon = bcd_adjust( s3c24xx->rtc.regs.bcdmon + 1);
					if (s3c24xx->rtc.regs.bcdmon >= 0x12)
					{
						s3c24xx->rtc.regs.bcdmon = 1;
						// increase year
						s3c24xx->rtc.regs.bcdyear = bcd_adjust( s3c24xx->rtc.regs.bcdyear + 1);
						if (s3c24xx->rtc.regs.bcdyear >= 0x100)
						{
							s3c24xx->rtc.regs.bcdyear = 0;
						}
					}
				}
			}
		}
	}
	verboselog( device->machine, 5, "RTC - %04d/%02d/%02d %02d:%02d:%02d\n", bcd_2_dec( s3c24xx->rtc.regs.bcdyear) + 2000, bcd_2_dec( s3c24xx->rtc.regs.bcdmon), bcd_2_dec( s3c24xx->rtc.regs.bcdday), bcd_2_dec( s3c24xx->rtc.regs.bcdhour), bcd_2_dec( s3c24xx->rtc.regs.bcdmin), bcd_2_dec( s3c24xx->rtc.regs.bcdsec));
}

static void s3c24xx_rtc_check_alarm( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->rtc.regs.rtcalm & 0x40)
	{
		int isalarm = 1;
		isalarm = isalarm && (((s3c24xx->rtc.regs.rtcalm & 0x20) == 0) || (s3c24xx->rtc.regs.almyear == s3c24xx->rtc.regs.bcdyear));
		isalarm = isalarm && (((s3c24xx->rtc.regs.rtcalm & 0x10) == 0) || (s3c24xx->rtc.regs.almmon == s3c24xx->rtc.regs.bcdmon));
		isalarm = isalarm && (((s3c24xx->rtc.regs.rtcalm & 0x08) == 0) || (s3c24xx->rtc.regs.almday == s3c24xx->rtc.regs.bcdday));
		isalarm = isalarm && (((s3c24xx->rtc.regs.rtcalm & 0x04) == 0) || (s3c24xx->rtc.regs.almhour == s3c24xx->rtc.regs.bcdhour));
		isalarm = isalarm && (((s3c24xx->rtc.regs.rtcalm & 0x02) == 0) || (s3c24xx->rtc.regs.almmin == s3c24xx->rtc.regs.bcdmin));
		isalarm = isalarm && (((s3c24xx->rtc.regs.rtcalm & 0x01) == 0) || (s3c24xx->rtc.regs.almsec == s3c24xx->rtc.regs.bcdsec));
		if (isalarm != 0)
		{
			s3c24xx_request_irq( device, S3C24XX_INT_RTC);
		}
	}
}

static TIMER_CALLBACK( s3c24xx_rtc_timer_update_exp )
{
	device_t *device = (device_t *)ptr;
	verboselog( machine, 2, "RTC timer callback (update)\n");
	s3c24xx_rtc_update( device);
	s3c24xx_rtc_check_alarm( device);
}

/* A/D Converter */

static UINT32 iface_adc_data_r( device_t *device, int ch)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->adc.data_r)
	{
		return (s3c24xx->iface->adc.data_r)( device, ch, 0);
	}
	else
	{
		return 0;
	}
}

static READ32_DEVICE_HANDLER( s3c24xx_adc_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->adc.regs)[offset];
	switch (offset)
	{
#if defined(DEVICE_S3C2400)
		case S3C24XX_ADCDAT :
		{
			data = (data & ~0x3FF) | (iface_adc_data_r( device, 0) & 0x3FF);
		}
		break;
#else
		case S3C24XX_ADCDAT0 :
		{
			data = (data & ~0x3FF) | (iface_adc_data_r( device, 0) & 0x3FF);
		}
		break;
		case S3C24XX_ADCDAT1 :
		{
			data = (data & ~0x3FF) | (iface_adc_data_r( device, 1) & 0x3FF);
		}
		break;
#endif
	}
	verboselog( device->machine, 9, "(ADC) %08X -> %08X\n", S3C24XX_BASE_ADC + (offset << 2), data);
	return data;
}

static void s3c24xx_adc_start( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 1, "ADC start\n");
	s3c24xx->adc.regs.adccon &= ~(1 << 0); // A/D conversion is completed
	s3c24xx->adc.regs.adccon |= (1 << 15); // End of A/D conversion
}

static WRITE32_DEVICE_HANDLER( s3c24xx_adc_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->adc.regs)[offset];
	verboselog( device->machine, 9, "(ADC) %08X <- %08X\n", S3C24XX_BASE_ADC + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->adc.regs)[offset]);
	switch (offset)
	{
		case S3C24XX_ADCCON :
		{
			if (((old_value & (1 << 0)) == 0) && ((data & (1 << 0)) != 0))
			{
				s3c24xx_adc_start( device);
			}
		}
		break;
	}
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static void s3c24xx_touch_screen( device_t *device, int state)
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx->adc.regs.adcdat0 = ((state ? 0 : 1) << 15);
	s3c24xx->adc.regs.adcdat1 = ((state ? 0 : 1) << 15);
	s3c24xx_request_subirq( device, S3C24XX_SUBINT_TC);
}

#endif

/* SPI */

static UINT32 s3c24xx_spi_r( device_t *device, UINT32 ch, UINT32 offset)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->spi[ch].regs)[offset];
	switch (offset)
	{
		case S3C24XX_SPSTA :
		{
			data = data | (1 << 0); // [bit 0] Transfer Ready Flag
		}
		break;
	}
	return data;
}

static void s3c24xx_spi_w( device_t *device, UINT32 ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	s3c24xx_t *s3c24xx = get_token( device);
	COMBINE_DATA(&((UINT32*)&s3c24xx->spi[ch].regs)[offset]);
}

static READ32_DEVICE_HANDLER( s3c24xx_spi_0_r )
{
	UINT32 data = s3c24xx_spi_r( device, 0, offset);
	verboselog( device->machine, 9, "(SPI 0) %08X -> %08X\n", S3C24XX_BASE_SPI_0 + (offset << 2), data);
	return data;
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static READ32_DEVICE_HANDLER( s3c24xx_spi_1_r )
{
	UINT32 data = s3c24xx_spi_r( device, 1, offset);
	verboselog( device->machine, 9, "(SPI 1) %08X -> %08X\n", S3C24XX_BASE_SPI_1 + (offset << 2), data);
	return data;
}

#endif

static WRITE32_DEVICE_HANDLER( s3c24xx_spi_0_w )
{
	verboselog( device->machine, 9, "(SPI 0) %08X <- %08X\n", S3C24XX_BASE_SPI_0 + (offset << 2), data);
	s3c24xx_spi_w( device, 0, offset, data, mem_mask);
}

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static WRITE32_DEVICE_HANDLER( s3c24xx_spi_1_w )
{
	verboselog( device->machine, 9, "(SPI 1) %08X <- %08X\n", S3C24XX_BASE_SPI_1 + (offset << 2), data);
	s3c24xx_spi_w( device, 1, offset, data, mem_mask);
}

#endif

/* MMC Interface */

#if defined(DEVICE_S3C2400)

static READ32_DEVICE_HANDLER( s3c24xx_mmc_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->mmc.regs.data[offset];
	verboselog( device->machine, 9, "(MMC) %08X -> %08X\n", S3C24XX_BASE_MMC + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_mmc_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(MMC) %08X <- %08X\n", S3C24XX_BASE_MMC + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->mmc.regs.data[offset]);
}

#endif

/* SD Interface */

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

static READ32_DEVICE_HANDLER( s3c24xx_sdi_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->sdi.regs.data[offset];
	verboselog( device->machine, 9, "(SDI) %08X -> %08X\n", S3C24XX_BASE_SDI + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_sdi_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(SDI) %08X <- %08X\n", S3C24XX_BASE_SDI + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->sdi.regs.data[offset]);
}

#endif

/* NAND Flash */

#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)

INLINE void iface_nand_command_w( device_t *device, UINT8 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->nand.command_w)
	{
		(s3c24xx->iface->nand.command_w)( device, 0, data);
	}
}

INLINE void iface_nand_address_w( device_t *device, UINT8 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->nand.address_w)
	{
		(s3c24xx->iface->nand.address_w)( device, 0, data);
	}
}

INLINE UINT8 iface_nand_data_r( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->nand.data_r)
	{
		return (s3c24xx->iface->nand.data_r)( device, 0);
	}
	else
	{
		return 0;
	}
}

INLINE void iface_nand_data_w( device_t *device, UINT8 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	if (s3c24xx->iface->nand.data_w)
	{
		(s3c24xx->iface->nand.data_w)( device, 0, data);
	}
}

static void nand_update_mecc( UINT8 *ecc, int pos, UINT8 data)
{
	int bit[8];
	UINT8 temp;
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

static void nand_update_secc( UINT8 *ecc, int pos, UINT8 data)
{
	int bit[8];
	UINT8 temp;
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

static void s3c24xx_nand_update_ecc( device_t *device, UINT8 data)
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT8 temp[4];
#if defined(DEVICE_S3C2410)
	temp[0] = s3c24xx->nand.mecc[0];
	temp[1] = s3c24xx->nand.mecc[1];
	temp[2] = s3c24xx->nand.mecc[2];
	nand_update_mecc( s3c24xx->nand.mecc, s3c24xx->nand.pos++, data);
	verboselog( device->machine, 5, "NAND - MECC %03X - %02X %02X %02X ->  %02X %02X %02X\n", s3c24xx->nand.pos - 1, temp[0], temp[1], temp[2], s3c24xx->nand.mecc[0], s3c24xx->nand.mecc[1], s3c24xx->nand.mecc[2]);
	if (s3c24xx->nand.pos == 512) s3c24xx->nand.pos = 0;
#else
	if ((s3c24xx->nand.regs.nfcont & (1 << 5)) == 0)
	{
		temp[0] = s3c24xx->nand.mecc[0];
		temp[1] = s3c24xx->nand.mecc[1];
		temp[2] = s3c24xx->nand.mecc[2];
		temp[3] = s3c24xx->nand.mecc[3];
		nand_update_mecc( s3c24xx->nand.mecc, s3c24xx->nand.pos++, data);
		verboselog( device->machine, 5, "NAND - MECC %03X - %02X %02X %02X %02X ->  %02X %02X %02X %02X\n", s3c24xx->nand.pos - 1, temp[0], temp[1], temp[2], temp[3], s3c24xx->nand.mecc[0], s3c24xx->nand.mecc[1], s3c24xx->nand.mecc[2], s3c24xx->nand.mecc[3]);
		if (s3c24xx->nand.pos == 2048) s3c24xx->nand.pos = 0;
	}
	if ((s3c24xx->nand.regs.nfcont & (1 << 6)) == 0)
	{
		temp[0] = s3c24xx->nand.secc[0];
		temp[1] = s3c24xx->nand.secc[1];
		nand_update_secc( s3c24xx->nand.secc, s3c24xx->nand.pos++, data);
		verboselog( device->machine, 5, "NAND - SECC %02X - %02X %02X ->  %02X %02X\n", s3c24xx->nand.pos - 1, temp[0], temp[1], s3c24xx->nand.secc[0], s3c24xx->nand.secc[1]);
		if (s3c24xx->nand.pos == 16) s3c24xx->nand.pos = 0;
	}
#endif
}

static void s3c24xx_nand_command_w( device_t *device, UINT8 data)
{
	verboselog( device->machine, 5, "NAND write command %02X\n", data);
	iface_nand_command_w( device, data);
}

static void s3c24xx_nand_address_w( device_t *device, UINT8 data)
{
	verboselog( device->machine, 5, "NAND write address %02X\n", data);
	iface_nand_address_w( device, data);
}

static UINT8 s3c24xx_nand_data_r( device_t *device)
{
	UINT8 data = iface_nand_data_r( device);
	verboselog( device->machine, 5, "NAND data read %02X\n", data);
	s3c24xx_nand_update_ecc( device, data);
	return data;
}

static void s3c24xx_nand_data_w( device_t *device, UINT8 data)
{
	verboselog( device->machine, 5, "NAND write data %02X\n", data);
	iface_nand_data_w( device, data);
	s3c24xx_nand_update_ecc( device, data);
}

static READ32_DEVICE_HANDLER( s3c24xx_nand_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = ((UINT32*)&s3c24xx->nand.regs)[offset];
	switch (offset)
	{
		case S3C24XX_NFDATA :
		{
			data = 0;
			if ((mem_mask & 0x000000FF) != 0) data = data | (s3c24xx_nand_data_r( device) <<  0);
			if ((mem_mask & 0x0000FF00) != 0) data = data | (s3c24xx_nand_data_r( device) <<  8);
			if ((mem_mask & 0x00FF0000) != 0) data = data | (s3c24xx_nand_data_r( device) << 16);
			if ((mem_mask & 0xFF000000) != 0) data = data | (s3c24xx_nand_data_r( device) << 24);
		}
		break;
#if defined(DEVICE_S3C2410)
		case S3C24XX_NFECC :
		{
			data = (s3c24xx->nand.mecc[2] << 16) | (s3c24xx->nand.mecc[1] << 8) | (s3c24xx->nand.mecc[0] << 0);
			//data = 0xFFFFFFFF;
		}
		break;
#endif
#if defined(DEVICE_S3C2440)
		case S3C24XX_NFMECC0 :
		{

			data = (s3c24xx->nand.mecc[3] << 24) | (s3c24xx->nand.mecc[2] << 16) | (s3c24xx->nand.mecc[1] << 8) | (s3c24xx->nand.mecc[0] << 0);
		}
		break;
		case S3C24XX_NFSECC :
		{
			data = (s3c24xx->nand.secc[1] << 8) | (s3c24xx->nand.secc[0] << 0);
		}
		break;
		case S3C24XX_NFESTAT0 :
		{
			data &= ~0x000000F; // no main/spare ECC errors
		}
		break;
		case S3C24XX_NFESTAT1 :
		{
			data &= ~0x000000F; // no main/spare ECC errors
		}
		break;
#endif
	}
	verboselog( device->machine, 9, "(NAND) %08X -> %08X (%08X)\n", S3C24XX_BASE_NAND + (offset << 2), data, mem_mask);
	return data;
}

static void s3c24xx_nand_init_ecc( device_t *device)
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 5, "NAND - init ecc\n");
#if defined(DEVICE_S3C2410)
	s3c24xx->nand.mecc[0] = 0;
	s3c24xx->nand.mecc[1] = 0;
	s3c24xx->nand.mecc[2] = 0;
#else
	s3c24xx->nand.mecc[0] = 0;
	s3c24xx->nand.mecc[1] = 0;
	s3c24xx->nand.mecc[2] = 0;
	s3c24xx->nand.mecc[3] = 0;
	s3c24xx->nand.secc[0] = 0;
	s3c24xx->nand.secc[1] = 0;
#endif
	s3c24xx->nand.pos = 0;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_nand_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c24xx->nand.regs)[offset];
	verboselog( device->machine, 9, "(NAND) %08X <- %08X\n", S3C24XX_BASE_NAND + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c24xx->nand.regs)[offset]);
	switch (offset)
	{
#if defined(DEVICE_S3C2410)
		case S3C24XX_NFCONF :
		{
			if ((data & (1 << 12)) != 0)
			{
				s3c24xx_nand_init_ecc( device);
			}
		}
		break;
#endif
#if defined(DEVICE_S3C2440)
		case S3C24XX_NFCONT :
		{
			if ((data & (1 << 4)) != 0)
			{
				s3c24xx_nand_init_ecc( device);
			}
		}
		break;
#endif
		case S3C24XX_NFSTAT :
		{
			s3c24xx->nand.regs.nfstat = (s3c24xx->nand.regs.nfstat & ~0x03) | (old_value & 0x03);
#if defined(DEVICE_S3C2440)
			if ((data & (1 << 2)) != 0)
			{
				s3c24xx->nand.regs.nfstat &= ~(1 << 2); // "RnB_TransDetect, to clear this value write 1"
			}
#endif
		}
		break;
		case S3C24XX_NFCMD :
		{
			s3c24xx_nand_command_w( device, data);
		}
		break;
		case S3C24XX_NFADDR :
		{
			s3c24xx_nand_address_w( device, data);
		}
		break;
		case S3C24XX_NFDATA :
		{
			if ((mem_mask & 0x000000FF) != 0) s3c24xx_nand_data_w( device, (data >>  0) & 0xFF);
			if ((mem_mask & 0x0000FF00) != 0) s3c24xx_nand_data_w( device, (data >>  8) & 0xFF);
			if ((mem_mask & 0x00FF0000) != 0) s3c24xx_nand_data_w( device, (data >> 16) & 0xFF);
			if ((mem_mask & 0xFF000000) != 0) s3c24xx_nand_data_w( device, (data >> 24) & 0xFF);
		}
		break;
	}
}

ATTR_UNUSED static WRITE_LINE_DEVICE_HANDLER( s3c24xx_pin_frnb_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
#if defined(DEVICE_S3C2440)
	if ((BIT( s3c24xx->nand.regs.nfstat, 0) == 0) && (state != 0))
	{
		s3c24xx->nand.regs.nfstat |= (1 << 2);
		if (BIT( s3c24xx->nand.regs.nfcont, 9) != 0)
		{
			s3c24xx_request_irq( device, S3C24XX_INT_NFCON);
		}
	}
#endif
	if (state == 0)
	{
		s3c24xx->nand.regs.nfstat &= ~(1 << 0);
	}
	else
	{
		s3c24xx->nand.regs.nfstat |= (1 << 0);
	}
}

#endif

/* Camera Interface */

#if defined(DEVICE_S3C2440)

static READ32_DEVICE_HANDLER( s3c24xx_cam_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->cam.regs.data[offset];
	verboselog( device->machine, 9, "(CAM) %08X -> %08X\n", S3C24XX_BASE_CAM + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_cam_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(CAM) %08X <- %08X\n", S3C24XX_BASE_CAM + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->cam.regs.data[offset]);
}

#endif

/* AC97 Interface */

#if defined(DEVICE_S3C2440)

static READ32_DEVICE_HANDLER( s3c24xx_ac97_r )
{
	s3c24xx_t *s3c24xx = get_token( device);
	UINT32 data = s3c24xx->ac97.regs.data[offset];
	verboselog( device->machine, 9, "(AC97) %08X -> %08X\n", S3C24XX_BASE_AC97 + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c24xx_ac97_w )
{
	s3c24xx_t *s3c24xx = get_token( device);
	verboselog( device->machine, 9, "(AC97) %08X <- %08X\n", S3C24XX_BASE_AC97 + (offset << 2), data);
	COMBINE_DATA(&s3c24xx->ac97.regs.data[offset]);
}

#endif

// ...

static DEVICE_RESET( s3c24xx )
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx->iis.fifo_index = 0;
//  s3c24xx->iic.data_index = 0;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	s3c24xx->gpio.regs.gstatus2 = 0x00000001; // Boot is caused by power on reset
#endif
	s3c24xx->irq.line_irq = s3c24xx->irq.line_fiq = CLEAR_LINE;
}

static DEVICE_START( s3c24xx )
{
	s3c24xx_t *s3c24xx = get_token( device);
	s3c24xx->iface = (const s3c24xx_interface *)device->baseconfig().static_config();
	for (int i = 0; i < 5; i++) s3c24xx->pwm.timer[i] = timer_alloc( device->machine, s3c24xx_pwm_timer_exp, (void*)device);
	for (int i = 0; i < 4; i++) s3c24xx->dma[i].timer = timer_alloc( device->machine, s3c24xx_dma_timer_exp, (void*)device);
	s3c24xx->iic.timer = timer_alloc( device->machine, s3c24xx_iic_timer_exp, (void*)device);
	s3c24xx->iis.timer = timer_alloc( device->machine, s3c24xx_iis_timer_exp, (void*)device);
	s3c24xx->lcd.timer = timer_alloc( device->machine, s3c24xx_lcd_timer_exp, (void*)device);
	s3c24xx->rtc.timer_tick_count = timer_alloc( device->machine, s3c24xx_rtc_timer_tick_count_exp, (void*)device);
	s3c24xx->rtc.timer_update = timer_alloc( device->machine, s3c24xx_rtc_timer_update_exp, (void*)device);
	s3c24xx->wdt.timer = timer_alloc( device->machine, s3c24xx_wdt_timer_exp, (void*)device);
	timer_adjust_periodic( s3c24xx->rtc.timer_update, ATTOTIME_IN_MSEC( 1000), 0, ATTOTIME_IN_MSEC( 1000));
	s3c24xx_rtc_init( device);
}

static DEVICE_GET_INFO( s3c24xx )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:           info->i = sizeof(s3c24xx_t);                    break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:   info->i = 0;                                    break;
//      case DEVINFO_INT_CLASS:                 info->i = DEVICE_CLASS_PERIPHERAL;              break;
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:                 info->start = DEVICE_START_NAME(s3c24xx);       break;
		case DEVINFO_FCT_RESET:                 info->reset = DEVICE_RESET_NAME(s3c24xx);       break;
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_FAMILY:                strcpy(info->s, "S3C24XX");                     break;
		case DEVINFO_STR_VERSION:               strcpy(info->s, "1.00");                        break;
		case DEVINFO_STR_SOURCE_FILE:           strcpy(info->s, __FILE__);                      break;
		case DEVINFO_STR_CREDITS:               strcpy(info->s, "Copyright the MESS Team");	break;
	}
}
