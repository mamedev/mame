/*******************************************************************************

    Samsung S3C44B0

    (c) 2011 Tim Schuerewegen

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/s3c44b0.h"
#include "sound/dac.h"
#include "coreutil.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context(), buf);
	}
}

/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

#define UART_PRINTF

#define CLOCK_MULTIPLIER 1

#define BIT(x,n) (((x)>>(n))&1)
#define BITS(x,m,n) (((x)>>(n))&(((UINT32)1<<((m)-(n)+1))-1))
#define CLR_BITS(x,m,n) ((x) & ~((((UINT32)1 << ((m) - (n) + 1)) - 1) << n))

/***************************************************************************
    PROTOTYPES
***************************************************************************/

static UINT32 s3c44b0_get_mclk( device_t *device);

static void s3c44b0_bdma_request_iis( device_t *device);
//static void s3c44b0_dma_request_pwm( device_t *device);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE s3c44b0_t *get_token( device_t *device)
{
	assert(device != NULL);
	return (s3c44b0_t *)downcast<s3c44b0_device *>(device)->token();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* ... */

static void s3c44b0_reset( device_t *device)
{
	fatalerror( "s3c44b0_reset\n");
}

/* LCD Controller */

static rgb_t s3c44b0_get_color_stn_04( device_t *device, UINT8 data)
{
	UINT8 r, g, b;
	r = g = b = BITS( data, 3, 0) << 4;
	return MAKE_RGB( r, g, b);
}

static UINT8 s3c44b0_get_color_stn_08_r( device_t *device, UINT8 data)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	return ((lcd->regs.redlut >> (BITS( data, 7, 5) << 2)) & 0xF) << 4;
}

static UINT8 s3c44b0_get_color_stn_08_g( device_t *device, UINT8 data)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	return ((lcd->regs.greenlut >> (BITS( data, 4, 2) << 2)) & 0xF) << 4;
}

static UINT8 s3c44b0_get_color_stn_08_b( device_t *device, UINT8 data)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	return ((lcd->regs.bluelut >> (BITS( data, 1, 0) << 2)) & 0xF) << 4;
}

static void s3c44b0_lcd_dma_reload( device_t *device)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	int lcdbank, lcdbaseu, lcdbasel;
	lcdbank = BITS( lcd->regs.lcdsaddr1, 26, 21);
	lcdbaseu = BITS( lcd->regs.lcdsaddr1, 20, 0);
	lcdbasel = BITS( lcd->regs.lcdsaddr2, 20, 0);
	lcd->vramaddr_cur = (lcdbank << 22) | (lcdbaseu << 1);
	lcd->vramaddr_max = (lcdbank << 22) | (lcdbasel << 1);
	if (lcdbasel == 0) lcd->vramaddr_max += 1 << 22;
	lcd->offsize = BITS( lcd->regs.lcdsaddr3, 19, 9);
	lcd->pagewidth_cur = 0;
	lcd->pagewidth_max = BITS( lcd->regs.lcdsaddr3, 8, 0);
	lcd->bswp = BIT( lcd->regs.lcdsaddr2, 29); // note: juicebox changes bswp when video playback starts
//  verboselog( device->machine(), 3, "LCD - vramaddr %08X %08X offsize %08X pagewidth %08X\n", lcd->vramaddr_cur, lcd->vramaddr_max, lcd->offsize, lcd->pagewidth_max);
}

static void s3c44b0_lcd_dma_init( device_t *device)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	lcd->modesel = BITS( lcd->regs.lcdsaddr1, 28, 27);
//  verboselog( device->machine(), 3, "LCD - modesel %d bswp %d\n", lcd->modesel, lcd->bswp);
	s3c44b0_lcd_dma_reload( device);
}

static void s3c44b0_lcd_dma_read( device_t *device, int count, UINT8 *data)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0_lcd_t *lcd = &s3c44b0->lcd;
	UINT8 *vram;
	vram = (UINT8 *)s3c44b0->space->get_read_ptr( lcd->vramaddr_cur);
	for (int i = 0; i < count / 2; i++)
	{
		if (lcd->bswp == 0)
		{
			if ((lcd->vramaddr_cur & 2) == 0)
			{
				data[0] = *(vram + 3);
				data[1] = *(vram + 2);
			}
			else
			{
				data[0] = *(vram - 1);
				data[1] = *(vram - 2);
			}
		}
		else
		{
			data[0] = *(vram + 0);
			data[1] = *(vram + 1);
		}
		lcd->vramaddr_cur += 2;
		lcd->pagewidth_cur++;
		if (lcd->pagewidth_cur >= lcd->pagewidth_max)
		{
			lcd->vramaddr_cur += lcd->offsize << 1;
			if (lcd->vramaddr_cur >= lcd->vramaddr_max)
			{
				s3c44b0_lcd_dma_reload( device);
			}
			lcd->pagewidth_cur = 0;
			vram = (UINT8 *)s3c44b0->space->get_read_ptr( lcd->vramaddr_cur);
		}
		else
		{
			vram += 2;
		}
		data += 2;
	}
}

static void s3c44b0_lcd_render_stn_04( device_t *device)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	UINT8 *bitmap = lcd->bitmap + ((lcd->vpos - lcd->vpos_min) * (lcd->hpos_max - lcd->hpos_min + 1)) + (lcd->hpos - lcd->hpos_min);
	UINT8 data[16];
	s3c44b0_lcd_dma_read( device, 16, data);
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			*bitmap++ = s3c44b0_get_color_stn_04( device, (data[i] >> 4) & 0x0F);
			data[i] = data[i] << 4;
			lcd->hpos++;
			if (lcd->hpos >= lcd->hpos_min + (lcd->pagewidth_max << 2))
			{
				lcd->vpos++;
				if (lcd->vpos > lcd->vpos_max)
				{
					lcd->vpos = lcd->vpos_min;
					bitmap = lcd->bitmap;
				}
				lcd->hpos = lcd->hpos_min;
			}
		}
	}
}

static void s3c44b0_lcd_render_stn_08( device_t *device)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	UINT8 *bitmap = lcd->bitmap + ((lcd->vpos - lcd->vpos_min) * (lcd->hpos_max - lcd->hpos_min + 1)) + (lcd->hpos - lcd->hpos_min);
	UINT8 data[16];
	s3c44b0_lcd_dma_read( device, 16, data);
	for (int i = 0; i < 16; i++)
	{
		UINT8 xxx[3];
		xxx[0] = s3c44b0_get_color_stn_08_r( device, data[i]);
		xxx[1] = s3c44b0_get_color_stn_08_g( device, data[i]);
		xxx[2] = s3c44b0_get_color_stn_08_b( device, data[i]);
		for (int j = 0; j < 3; j++)
		{
			*bitmap++ = xxx[j];
			lcd->hpos++;
			if (lcd->hpos >= lcd->hpos_min + (lcd->pagewidth_max * 6))
			{
				lcd->vpos++;
				if (lcd->vpos > lcd->vpos_max)
				{
					lcd->vpos = lcd->vpos_min;
					bitmap = lcd->bitmap;
				}
				lcd->hpos = lcd->hpos_min;
			}
		}
	}
}

static attotime s3c44b0_time_until_pos( device_t *device, int vpos, int hpos)
{
	running_machine &machine = device->machine();
	s3c44b0_lcd_t *lcd = (s3c44b0_lcd_t *)&(get_token( device)->lcd);
	attoseconds_t time1, time2;
	attotime retval;
	verboselog( device->machine(), 3, "s3c44b0_time_until_pos - vpos %d hpos %d\n", vpos, hpos);
	time1 = (attoseconds_t)vpos * lcd->scantime + (attoseconds_t)hpos * lcd->pixeltime;
	time2 = (machine.time() - lcd->frame_time).as_attoseconds();
	verboselog( device->machine(), 3, "machine %f frametime %f time1 %f time2 %f\n", machine.time().as_double(), lcd->frame_time.as_double(), attotime( 0, time1).as_double(), attotime( 0, time2).as_double());
	while (time1 <= time2) time1 += lcd->frame_period;
	retval = attotime( 0, time1 - time2);
	verboselog( device->machine(), 3, "result %f\n", retval.as_double());
	return retval;
}

static int s3c44b0_lcd_get_vpos( device_t *device)
{
	running_machine &machine = device->machine();
	s3c44b0_lcd_t *lcd = (s3c44b0_lcd_t *)&(get_token( device)->lcd);
	attoseconds_t delta;
	int vpos;
	delta = (machine.time() - lcd->frame_time).as_attoseconds();
	delta = delta + (lcd->pixeltime / 2);
	vpos = delta / lcd->scantime;
	return (lcd->vpos_min + vpos) % lcd->vpos_end;
}

static int s3c44b0_lcd_get_hpos( device_t *device)
{
	running_machine &machine = device->machine();
	s3c44b0_lcd_t *lcd = (s3c44b0_lcd_t *)&(get_token( device)->lcd);
	attoseconds_t delta;
	int vpos;
	delta = (machine.time() - lcd->frame_time).as_attoseconds();
	delta = delta + (lcd->pixeltime / 2);
	vpos = delta / lcd->scantime;
	delta = delta - (vpos * lcd->scantime);
	return delta / lcd->pixeltime;
}

static TIMER_CALLBACK( s3c44b0_lcd_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	int vpos = lcd->vpos;
	verboselog( machine, 2, "LCD timer callback (%f)\n", machine.time().as_double());
	verboselog( machine, 3, "LCD - (1) vramaddr %08X vpos %d hpos %d\n", lcd->vramaddr_cur, lcd->vpos, lcd->hpos);
	switch (lcd->modesel)
	{
		case S3C44B0_MODESEL_04 : s3c44b0_lcd_render_stn_04( device); break;
		case S3C44B0_MODESEL_08 : s3c44b0_lcd_render_stn_08( device); break;
		default : verboselog( machine, 0, "s3c44b0_lcd_timer_exp: modesel %d not supported\n", lcd->modesel); break;
	}
	verboselog( machine, 3, "LCD - (2) vramaddr %08X vpos %d hpos %d\n", lcd->vramaddr_cur, lcd->vpos, lcd->hpos);
	if (lcd->vpos < vpos)
	{
//      verboselog( machine, 3, "LCD - (1) frame_time %f\n", attotime_to_double( lcd->frame_time));
		lcd->frame_time = machine.time() + s3c44b0_time_until_pos( device, lcd->vpos_min, lcd->hpos_min);
//      verboselog( machine, 3, "LCD - (2) frame_time %f\n", attotime_to_double( lcd->frame_time));
	}
	lcd->timer->adjust( s3c44b0_time_until_pos( device, lcd->vpos, lcd->hpos), 0);
}

static void s3c44b0_video_start( device_t *device, running_machine &machine)
{
	// do nothing
}

static UINT32 s3c44b0_video_update( device_t *device, screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	if (lcd->regs.lcdcon1 & (1 << 0))
	{
		if (lcd->bitmap)
		{
			for (int y = 0; y < screen.height(); y++)
			{
				UINT32 *scanline = &bitmap.pix32(y);
				UINT8 *vram = lcd->bitmap + y * (lcd->hpos_max - lcd->hpos_min + 1);
				for (int x = 0; x < screen.width(); x++)
				{
					*scanline++ = MAKE_RGB( vram[0], vram[1], vram[2]);
					vram += 3;
				}
			}
		}
	}
	else
	{
		for (int y = 0; y < screen.height(); y++)
		{
			UINT32 *scanline = &bitmap.pix32(y);
			memset( scanline, 0, screen.width() * 4);
		}
	}
	return 0;
}

static READ32_DEVICE_HANDLER( s3c44b0_lcd_r )
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	UINT32 data = ((UINT32*)&lcd->regs)[offset];
	switch (offset)
	{
		case S3C44B0_LCDCON1 :
		{
			int vpos = 0;
			// make sure line counter is going
			if (lcd->regs.lcdcon1 & (1 << 0))
			{
				vpos = s3c44b0_lcd_get_vpos( device);
				int hpos = s3c44b0_lcd_get_hpos( device);
				if (hpos < lcd->hpos_min) vpos = vpos - 1;
				if ((vpos < lcd->vpos_min) || (vpos > lcd->vpos_max)) vpos = lcd->vpos_max;
				vpos = lcd->vpos_max - vpos;
			}
			data = (data & ~0xFFC00000) | (vpos << 22);
		}
		break;
	}
//  verboselog( space.machine(), 9, "(LCD) %08X -> %08X\n", S3C44B0_BASE_LCD + (offset << 2), data);
	return data;
}

static void s3c44b0_lcd_configure( device_t *device)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	screen_device *screen = device->machine().primary_screen;
	int dismode, clkval, lineval, wdly, hozval, lineblank, wlh, mclk;
	double vclk, framerate;
	int width, height;
	verboselog( device->machine(), 5, "s3c44b0_lcd_configure\n");
	dismode = BITS( lcd->regs.lcdcon1, 6, 5);
	clkval = BITS( lcd->regs.lcdcon1, 21, 12);
	lineval = BITS( lcd->regs.lcdcon2, 9, 0);
	wdly = BITS( lcd->regs.lcdcon1, 9, 8);
	hozval = BITS( lcd->regs.lcdcon2, 20, 10);
	lineblank = BITS( lcd->regs.lcdcon2, 31, 21);
	wlh = BITS( lcd->regs.lcdcon1, 11, 10);
	mclk = s3c44b0_get_mclk( device);
	verboselog( device->machine(), 3, "LCD - dismode %d clkval %d lineval %d wdly %d hozval %d lineblank %d wlh %d mclk %d\n", dismode, clkval, lineval, wdly, hozval, lineblank, wlh, mclk);
	vclk = (double)(mclk / (clkval * 2));
	verboselog( device->machine(), 3, "LCD - vclk %f\n", vclk);
	framerate = 1 / (((1 / vclk) * (hozval + 1) + (1 / mclk) * (wlh + wdly + lineblank)) * (lineval + 1));
	framerate = framerate / 3; // ???
	verboselog( device->machine(), 3, "LCD - framerate %f\n", framerate);
	switch (dismode)
	{
		case S3C44B0_PNRMODE_STN_04_SS : width = ((hozval + 1) * 4); break;
		case S3C44B0_PNRMODE_STN_04_DS : width = ((hozval + 1) * 4); break;
		case S3C44B0_PNRMODE_STN_08_SS : width = ((hozval + 1) * 8); break;
		default : fatalerror( "invalid display mode (%d)\n", dismode); break;
	}
	height = lineval + 1;
	lcd->framerate = framerate;
	verboselog( device->machine(), 3, "video_screen_configure %d %d %f\n", width, height, lcd->framerate);
	screen->configure( screen->width(), screen->height(), screen->visible_area(), HZ_TO_ATTOSECONDS( lcd->framerate));
	lcd->hpos_min = 25;
	lcd->hpos_max = 25 + width - 1;
	lcd->hpos_end = 25 + width - 1 + 25;
	lcd->vpos_min = 25;
	lcd->vpos_max = 25 + height - 1;
	lcd->vpos_end = 25 + height - 1 + 25;
	verboselog( device->machine(), 3, "LCD - min_x %d min_y %d max_x %d max_y %d\n", lcd->hpos_min, lcd->vpos_min, lcd->hpos_max, lcd->vpos_max);
	if (lcd->bitmap)
	{
		auto_free( device->machine(), lcd->bitmap);
	}
	lcd->bitmap = auto_alloc_array( device->machine(), UINT8, (lcd->hpos_max - lcd->hpos_min + 1) * (lcd->vpos_max - lcd->vpos_min + 1) * 3);
	lcd->frame_period = HZ_TO_ATTOSECONDS( lcd->framerate);
	lcd->scantime = lcd->frame_period / lcd->vpos_end;
	lcd->pixeltime = lcd->frame_period / (lcd->vpos_end * lcd->hpos_end);
//  printf( "frame_period %f\n", attotime( 0, lcd->frame_period).as_double());
//  printf( "scantime %f\n", attotime( 0, lcd->scantime).as_double());
//  printf( "pixeltime %f\n", attotime( 0, lcd->pixeltime).as_double());
}

static void s3c44b0_lcd_start( device_t *device)
{
	running_machine &machine = device->machine();
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	screen_device *screen = device->machine().primary_screen;
	verboselog( device->machine(), 1, "LCD start\n");
	s3c44b0_lcd_configure( device);
	s3c44b0_lcd_dma_init( device);
	lcd->vpos = lcd->vpos_min;
	lcd->hpos = lcd->hpos_min;
	lcd->frame_time = screen->time_until_pos( 0, 0);
	lcd->timer->adjust( lcd->frame_time, 0);
	lcd->frame_time = machine.time() + lcd->frame_time;
}

static void s3c44b0_lcd_stop( device_t *device)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	verboselog( device->machine(), 1, "LCD stop\n");
	lcd->timer->adjust( attotime::never, 0);
}

static void s3c44b0_lcd_recalc( device_t *device)
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	if (lcd->regs.lcdcon1 & (1 << 0))
	{
		s3c44b0_lcd_start( device);
	}
	else
	{
		s3c44b0_lcd_stop( device);
	}
}

static WRITE32_DEVICE_HANDLER( s3c44b0_lcd_w )
{
	s3c44b0_lcd_t *lcd = &(get_token( device)->lcd);
	UINT32 old_value = ((UINT32*)&lcd->regs)[offset];
//  verboselog( space.machine(), 9, "(LCD) %08X <- %08X\n", S3C44B0_BASE_LCD + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&lcd->regs)[offset]);
	switch (offset)
	{
		case S3C44B0_LCDCON1 :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				s3c44b0_lcd_recalc( device);
			}
		}
		break;
	}
}

/* Clock & Power Management */

static UINT32 s3c44b0_get_mclk( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data, mdiv, pdiv, sdiv;
	data = s3c44b0->clkpow.regs.pllcon;
	mdiv = BITS( data, 19, 12);
	pdiv = BITS( data, 9, 4);
	sdiv = BITS( data, 1, 0);
	return (UINT32)((double)((mdiv + 8) * device->clock()) / (double)((pdiv + 2) * (1 << sdiv)));
}

static READ32_DEVICE_HANDLER( s3c44b0_clkpow_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->clkpow.regs)[offset];
	verboselog( space.machine(), 9, "(CLKPOW) %08X -> %08X\n", S3C44B0_BASE_CLKPOW + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_clkpow_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( space.machine(), 9, "(CLKPOW) %08X <- %08X\n", S3C44B0_BASE_CLKPOW + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->clkpow.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_PLLCON :
		{
			verboselog( space.machine(), 5, "CLKPOW - mclk %d\n", s3c44b0_get_mclk( device));
			s3c44b0->cpu->set_unscaled_clock( s3c44b0_get_mclk( device) * CLOCK_MULTIPLIER);
		}
		break;
		case S3C44B0_CLKCON :
		{
			if (data & (1 << 2))
			{
				s3c44b0->cpu->spin_until_interrupt();
			}
		}
		break;
	}
}

/* Interrupt Controller */

static void s3c44b0_check_pending_irq( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 temp;
	// normal irq
	temp = (s3c44b0->irq.regs.intpnd & ~s3c44b0->irq.regs.intmsk) & ~s3c44b0->irq.regs.intmod;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		s3c44b0->irq.regs.i_ispr |= (1 << int_type);
		if (s3c44b0->irq.line_irq != ASSERT_LINE)
		{
			s3c44b0->cpu->set_input_line( ARM7_IRQ_LINE, ASSERT_LINE);
			s3c44b0->irq.line_irq = ASSERT_LINE;
		}
	}
	else
	{
		if (s3c44b0->irq.line_irq != CLEAR_LINE)
		{
			s3c44b0->cpu->set_input_line( ARM7_IRQ_LINE, CLEAR_LINE);
			s3c44b0->irq.line_irq = CLEAR_LINE;
		}
	}
	// fast irq
	temp = (s3c44b0->irq.regs.intpnd & ~s3c44b0->irq.regs.intmsk) & s3c44b0->irq.regs.intmod;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		if (s3c44b0->irq.line_fiq != ASSERT_LINE)
		{
			s3c44b0->cpu->set_input_line( ARM7_FIRQ_LINE, ASSERT_LINE);
			s3c44b0->irq.line_fiq = ASSERT_LINE;
		}
	}
	else
	{
		if (s3c44b0->irq.line_fiq != CLEAR_LINE)
		{
			s3c44b0->cpu->set_input_line( ARM7_FIRQ_LINE, CLEAR_LINE);
			s3c44b0->irq.line_fiq = CLEAR_LINE;
		}
	}
}

static void s3c44b0_request_irq( device_t *device, UINT32 int_type)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 5, "request irq %d\n", int_type);
	s3c44b0->irq.regs.intpnd |= (1 << int_type);
	s3c44b0_check_pending_irq( device);
}

static void s3c44b0_check_pending_eint( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 temp = s3c44b0->gpio.regs.extintpnd;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		s3c44b0_request_irq( device, S3C44B0_INT_EINT4_7);
	}
}

void s3c44b0_request_eint( device_t *device, UINT32 number)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 5, "request external interrupt %d\n", number);
	if (number < 4)
	{
		s3c44b0_request_irq( device, S3C44B0_INT_EINT0 + number);
	}
	else
	{
		s3c44b0->gpio.regs.extintpnd |= (1 << (number - 4));
		s3c44b0_check_pending_eint( device);
	}
}

static READ32_DEVICE_HANDLER( s3c44b0_irq_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->irq.regs)[offset];
	verboselog( space.machine(), 9, "(IRQ) %08X -> %08X\n", S3C44B0_BASE_INT + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_irq_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( space.machine(), 9, "(IRQ) %08X <- %08X\n", S3C44B0_BASE_INT + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->irq.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_INTMSK :
		{
			s3c44b0_check_pending_irq( device);
		}
		break;
		case S3C44B0_I_ISPC :
		{
			s3c44b0->irq.regs.intpnd = (s3c44b0->irq.regs.intpnd & ~data); // The bit of INTPND bit is cleared to zero by writing '1' on I_ISPC/F_ISPC
			s3c44b0->irq.regs.i_ispr = (s3c44b0->irq.regs.i_ispr & ~data); // The pending bit in I_ISPR register should be cleared by writing I_ISPC register.
			s3c44b0_check_pending_irq( device);
		}
		break;
		case S3C44B0_F_ISPC :
		{
			s3c44b0->irq.regs.intpnd = (s3c44b0->irq.regs.intpnd & ~data); // The bit of INTPND bit is cleared to zero by writing '1' on I_ISPC/F_ISPC
			s3c44b0_check_pending_irq( device);
		}
		break;
	}
}

/* PWM Timer */

static UINT16 s3c44b0_pwm_calc_observation( device_t *device, int ch)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	double timeleft, x1, x2;
	UINT32 cnto;
	timeleft = (s3c44b0->pwm.timer[ch]->remaining()).as_double();
//  printf( "timeleft %f freq %d cntb %d cmpb %d\n", timeleft, s3c44b0->pwm.freq[ch], s3c44b0->pwm.cnt[ch], s3c44b0->pwm.cmp[ch]);
	x1 = 1 / ((double)s3c44b0->pwm.freq[ch] / (s3c44b0->pwm.cnt[ch]- s3c44b0->pwm.cmp[ch] + 1));
	x2 = x1 / timeleft;
//  printf( "x1 %f\n", x1);
	cnto = s3c44b0->pwm.cmp[ch] + ((s3c44b0->pwm.cnt[ch]- s3c44b0->pwm.cmp[ch]) / x2);
//  printf( "cnto %d\n", cnto);
	return cnto;
}

static READ32_DEVICE_HANDLER( s3c44b0_pwm_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->pwm.regs)[offset];
	switch (offset)
	{
		case S3C44B0_TCNTO0 :
		{
			data = (data & ~0x0000FFFF) | s3c44b0_pwm_calc_observation( device, 0);
		}
		break;
		case S3C44B0_TCNTO1 :
		{
			data = (data & ~0x0000FFFF) | s3c44b0_pwm_calc_observation( device, 1);
		}
		break;
		case S3C44B0_TCNTO2 :
		{
			data = (data & ~0x0000FFFF) | s3c44b0_pwm_calc_observation( device, 2);
		}
		break;
		case S3C44B0_TCNTO3 :
		{
			data = (data & ~0x0000FFFF) | s3c44b0_pwm_calc_observation( device, 3);
		}
		break;
		case S3C44B0_TCNTO4 :
		{
			data = (data & ~0x0000FFFF) | s3c44b0_pwm_calc_observation( device, 4);
		}
		break;
		case S3C44B0_TCNTO5 :
		{
			data = (data & ~0x0000FFFF) | s3c44b0_pwm_calc_observation( device, 5);
		}
		break;
	}
	verboselog( space.machine(), 9, "(PWM) %08X -> %08X\n", S3C44B0_BASE_PWM + (offset << 2), data);
	return data;
}

static void s3c44b0_pwm_start( device_t *device, int timer)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	const int mux_table[] = { 2, 4, 8, 16};
	const int prescaler_shift[] = { 0, 0, 8, 8, 16, 16};
	const int mux_shift[] = { 0, 4, 8, 12, 16, 20};
	UINT32 mclk, prescaler, mux, cnt, cmp, auto_reload;
	double freq, hz;
	verboselog( device->machine(), 1, "PWM %d start\n", timer);
	mclk = s3c44b0_get_mclk( device);
	prescaler = (s3c44b0->pwm.regs.tcfg0 >> prescaler_shift[timer]) & 0xFF;
	mux = (s3c44b0->pwm.regs.tcfg1 >> mux_shift[timer]) & 0x0F;
	if (mux < 4)
	{
		freq = (double)mclk / (prescaler + 1) / mux_table[mux];
	}
	else
	{
		// todo
		freq = (double)mclk / (prescaler + 1) / 1;
	}
	switch (timer)
	{
		case 0 :
		{
			cnt = BITS( s3c44b0->pwm.regs.tcntb0, 15, 0);
			cmp = BITS( s3c44b0->pwm.regs.tcmpb0, 15, 0);
			auto_reload = BIT( s3c44b0->pwm.regs.tcon, 3);
		}
		break;
		case 1 :
		{
			cnt = BITS( s3c44b0->pwm.regs.tcntb1, 15, 0);
			cmp = BITS( s3c44b0->pwm.regs.tcmpb1, 15, 0);
			auto_reload = BIT( s3c44b0->pwm.regs.tcon, 11);
		}
		break;
		case 2 :
		{
			cnt = BITS( s3c44b0->pwm.regs.tcntb2, 15, 0);
			cmp = BITS( s3c44b0->pwm.regs.tcmpb2, 15, 0);
			auto_reload = BIT( s3c44b0->pwm.regs.tcon, 15);
		}
		break;
		case 3 :
		{
			cnt = BITS( s3c44b0->pwm.regs.tcntb3, 15, 0);
			cmp = BITS( s3c44b0->pwm.regs.tcmpb3, 15, 0);
			auto_reload = BIT( s3c44b0->pwm.regs.tcon, 19);
		}
		break;
		case 4 :
		{
			cnt = BITS( s3c44b0->pwm.regs.tcntb4, 15, 0);
			cmp = BITS( s3c44b0->pwm.regs.tcmpb4, 15, 0);
			auto_reload = BIT( s3c44b0->pwm.regs.tcon, 23);
		}
		break;
		case 5 :
		{
			cnt = BITS( s3c44b0->pwm.regs.tcntb5, 15, 0);
			cmp = 0;
			auto_reload = BIT( s3c44b0->pwm.regs.tcon, 26);
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
	verboselog( device->machine(), 5, "PWM %d - mclk=%d prescaler=%d div=%d freq=%f cnt=%d cmp=%d auto_reload=%d hz=%f\n", timer, mclk, prescaler, mux_table[mux], freq, cnt, cmp, auto_reload, hz);
	s3c44b0->pwm.cnt[timer] = cnt;
	s3c44b0->pwm.cmp[timer] = cmp;
	s3c44b0->pwm.freq[timer] = freq;
	if (cnt == 0)
	{
		s3c44b0->pwm.timer[timer]->adjust( attotime::never, 0);
	}
	else
	{
		if (auto_reload)
		{
			s3c44b0->pwm.timer[timer]->adjust( attotime::from_hz( hz), timer, attotime::from_hz( hz));
		}
		else
		{
			s3c44b0->pwm.timer[timer]->adjust( attotime::from_hz( hz), timer);
		}
	}
}

static void s3c44b0_pwm_stop( device_t *device, int timer)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 1, "PWM %d stop\n", timer);
	s3c44b0->pwm.timer[timer]->adjust( attotime::never, 0);
}

static void s3c44b0_pwm_recalc( device_t *device, int timer)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	const int tcon_shift[] = { 0, 8, 12, 16, 20, 24};
	if (s3c44b0->pwm.regs.tcon & (1 << tcon_shift[timer]))
	{
		s3c44b0_pwm_start( device, timer);
	}
	else
	{
		s3c44b0_pwm_stop( device, timer);
	}
}

static WRITE32_DEVICE_HANDLER( s3c44b0_pwm_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->pwm.regs)[offset];
	verboselog( space.machine(), 9, "(PWM) %08X <- %08X\n", S3C44B0_BASE_PWM + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->pwm.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_TCON :
		{
			if ((data & (1 << 0)) != (old_value & (1 << 0)))
			{
				s3c44b0_pwm_recalc( device, 0);
			}
			if ((data & (1 << 8)) != (old_value & (1 << 8)))
			{
				s3c44b0_pwm_recalc( device, 1);
			}
			if ((data & (1 << 12)) != (old_value & (1 << 12)))
			{
				s3c44b0_pwm_recalc( device, 2);
			}
			if ((data & (1 << 16)) != (old_value & (1 << 16)))
			{
				s3c44b0_pwm_recalc( device, 3);
			}
			if ((data & (1 << 20)) != (old_value & (1 << 20)))
			{
				s3c44b0_pwm_recalc( device, 4);
			}
			if ((data & (1 << 24)) != (old_value & (1 << 24)))
			{
				s3c44b0_pwm_recalc( device, 5);
			}
		}
		break;
	}
}

static TIMER_CALLBACK( s3c44b0_pwm_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_t *s3c44b0 = get_token( device);
	int ch = param;
	const int ch_int[] = { S3C44B0_INT_TIMER0, S3C44B0_INT_TIMER1, S3C44B0_INT_TIMER2, S3C44B0_INT_TIMER3, S3C44B0_INT_TIMER4, S3C44B0_INT_TIMER5 };
	verboselog( machine, 2, "PWM %d timer callback\n", ch);
	if (BITS( s3c44b0->pwm.regs.tcfg1, 27, 24) == (ch + 1))
	{
		fatalerror( "s3c44b0_dma_request_pwm( device)\n");
	}
	else
	{
		s3c44b0_request_irq( device, ch_int[ch]);
	}
}

/* IIC */

INLINE void iface_i2c_scl_w( device_t *device, int state)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if (s3c44b0->iface->i2c.scl_w)
	{
		(s3c44b0->iface->i2c.scl_w)( device, state);
	}
}

INLINE void iface_i2c_sda_w( device_t *device, int state)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if (s3c44b0->iface->i2c.sda_w)
	{
		(s3c44b0->iface->i2c.sda_w)( device, state);
	}
}

INLINE int iface_i2c_sda_r( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if (s3c44b0->iface->i2c.sda_r)
	{
		return (s3c44b0->iface->i2c.sda_r)( device);
	}
	else
	{
		return 0;
	}
}

static void i2c_send_start( device_t *device)
{
	verboselog( device->machine(), 5, "i2c_send_start\n");
	iface_i2c_sda_w( device, 1);
	iface_i2c_scl_w( device, 1);
	iface_i2c_sda_w( device, 0);
	iface_i2c_scl_w( device, 0);
}

static void i2c_send_stop( device_t *device)
{
	verboselog( device->machine(), 5, "i2c_send_stop\n");
	iface_i2c_sda_w( device, 0);
	iface_i2c_scl_w( device, 1);
	iface_i2c_sda_w( device, 1);
	iface_i2c_scl_w( device, 0);
}

static UINT8 i2c_receive_byte( device_t *device, int ack)
{
	UINT8 data = 0;
	verboselog( device->machine(), 5, "i2c_receive_byte ...\n");
	iface_i2c_sda_w( device, 1);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_scl_w( device, 1);
		data = (data << 1) + (iface_i2c_sda_r( device) ? 1 : 0);
		iface_i2c_scl_w( device, 0);
	}
	verboselog( device->machine(), 5, "recv data %02X\n", data);
	verboselog( device->machine(), 5, "send ack %d\n", ack);
	iface_i2c_sda_w( device, ack ? 0 : 1);
	iface_i2c_scl_w( device, 1);
	iface_i2c_scl_w( device, 0);
	return data;
}

static int i2c_send_byte( device_t *device, UINT8 data)
{
	int ack;
	verboselog( device->machine(), 5, "i2c_send_byte ...\n");
	verboselog( device->machine(), 5, "send data %02X\n", data);
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
	verboselog( device->machine(), 5, "recv ack %d\n", ack);
	iface_i2c_scl_w( device, 0);
	return ack;
}

static void iic_start( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	int mode_selection;
	verboselog( device->machine(), 1, "IIC start\n");
	i2c_send_start( device);
	mode_selection = BITS( s3c44b0->iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : i2c_send_byte( device, s3c44b0->iic.regs.iicds | 0x01); break;
		case 3 : i2c_send_byte( device, s3c44b0->iic.regs.iicds & 0xFE); break;
	}
	s3c44b0->iic.timer->adjust( attotime::from_usec( 1), 0);
}

static void iic_stop( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 1, "IIC stop\n");
	i2c_send_stop( device);
	s3c44b0->iic.timer->adjust( attotime::never, 0);
}

static void iic_resume( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	int mode_selection;
	verboselog( device->machine(), 1, "IIC resume\n");
	mode_selection = BITS( s3c44b0->iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : s3c44b0->iic.regs.iicds = i2c_receive_byte( device, BIT( s3c44b0->iic.regs.iiccon, 7)); break;
		case 3 : i2c_send_byte( device, s3c44b0->iic.regs.iicds & 0xFF); break;
	}
	s3c44b0->iic.timer->adjust( attotime::from_usec( 1), 0);
}

static READ32_DEVICE_HANDLER( s3c44b0_iic_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->iic.regs)[offset];
	switch (offset)
	{
		case S3C44B0_IICSTAT :
		{
			data = data & ~0x0000000F;
		}
		break;
	}
	verboselog( space.machine(), 9, "(IIC) %08X -> %08X\n", S3C44B0_BASE_IIC + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_iic_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->iic.regs)[offset];
	verboselog( space.machine(), 9, "(IIC) %08X <- %08X\n", S3C44B0_BASE_IIC + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->iic.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_IICCON :
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
					start_stop_condition = BIT( s3c44b0->iic.regs.iicstat, 5);
					if (start_stop_condition != 0)
					{
						if (s3c44b0->iic.count == 0)
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
		case  S3C44B0_IICSTAT :
		{
			int interrupt_pending_flag;
			s3c44b0->iic.count = 0;
			interrupt_pending_flag = BIT( s3c44b0->iic.regs.iiccon, 4);
			if (interrupt_pending_flag == 0)
			{
				int start_stop_condition;
				start_stop_condition = BIT( data, 5);
				if (start_stop_condition != 0)
				{
					if (s3c44b0->iic.count == 0)
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

static TIMER_CALLBACK( s3c44b0_iic_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_t *s3c44b0 = get_token( device);
	int enable_interrupt;
	verboselog( machine, 2, "IIC timer callback\n");
	s3c44b0->iic.count++;
	enable_interrupt = BIT( s3c44b0->iic.regs.iiccon, 5);

	s3c44b0->iic.regs.iicds = 0xFF; // TEST

	if (enable_interrupt)
	{
		s3c44b0->iic.regs.iiccon |= (1 << 4); // [bit 4] interrupt is pending
		s3c44b0_request_irq( device, S3C44B0_INT_IIC);
	}
}

/* I/O Port */

INLINE UINT32 iface_gpio_port_r( device_t *device, int port)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if (s3c44b0->iface->gpio.port_r)
	{
		return (s3c44b0->iface->gpio.port_r)( device, port);
	}
	else
	{
		return 0;
	}
}

INLINE void iface_gpio_port_w( device_t *device, int port, UINT32 data)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if (s3c44b0->iface->gpio.port_w)
	{
		(s3c44b0->iface->gpio.port_w)( device, port, data);
	}
}

static READ32_DEVICE_HANDLER( s3c44b0_gpio_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->gpio.regs)[offset];
	switch (offset)
	{
		case S3C44B0_GPADAT :
		{
			data = iface_gpio_port_r( device, S3C44B0_GPIO_PORT_A) & S3C44B0_GPADAT_MASK;
		}
		break;
		case S3C44B0_GPBDAT :
		{
			data = iface_gpio_port_r( device, S3C44B0_GPIO_PORT_B) & S3C44B0_GPBDAT_MASK;
		}
		break;
		case S3C44B0_GPCDAT :
		{
			data = iface_gpio_port_r( device, S3C44B0_GPIO_PORT_C) & S3C44B0_GPCDAT_MASK;
		}
		break;
		case S3C44B0_GPDDAT :
		{
			data = iface_gpio_port_r( device, S3C44B0_GPIO_PORT_D) & S3C44B0_GPDDAT_MASK;
		}
		break;
		case S3C44B0_GPEDAT :
		{
			data = iface_gpio_port_r( device, S3C44B0_GPIO_PORT_E) & S3C44B0_GPEDAT_MASK;
		}
		break;
		case S3C44B0_GPFDAT :
		{
			data = iface_gpio_port_r( device, S3C44B0_GPIO_PORT_F) & S3C44B0_GPFDAT_MASK;
		}
		break;
		case S3C44B0_GPGDAT :
		{
			data = iface_gpio_port_r( device, S3C44B0_GPIO_PORT_G) & S3C44B0_GPGDAT_MASK;
		}
		break;
	}
	verboselog( space.machine(), 9, "(GPIO) %08X -> %08X\n", S3C44B0_BASE_GPIO + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_gpio_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->gpio.regs)[offset];
	verboselog( space.machine(), 9, "(GPIO) %08X <- %08X\n", S3C44B0_BASE_GPIO + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->gpio.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_GPADAT :
		{
			 iface_gpio_port_w( device, S3C44B0_GPIO_PORT_A, data & S3C44B0_GPADAT_MASK);
		}
		break;
		case S3C44B0_GPBDAT :
		{
			 iface_gpio_port_w( device, S3C44B0_GPIO_PORT_B, data & S3C44B0_GPBDAT_MASK);
		}
		break;
		case S3C44B0_GPCDAT :
		{
			 iface_gpio_port_w( device, S3C44B0_GPIO_PORT_C, data & S3C44B0_GPCDAT_MASK);
		}
		break;
		case S3C44B0_GPDDAT :
		{
			 iface_gpio_port_w( device, S3C44B0_GPIO_PORT_D, data & S3C44B0_GPDDAT_MASK);
		}
		break;
		case S3C44B0_GPEDAT :
		{
			 iface_gpio_port_w( device, S3C44B0_GPIO_PORT_E, data & S3C44B0_GPEDAT_MASK);
		}
		break;
		case S3C44B0_GPFDAT :
		{
			 iface_gpio_port_w( device, S3C44B0_GPIO_PORT_F, data & S3C44B0_GPFDAT_MASK);
		}
		break;
		case S3C44B0_GPGDAT :
		{
			 iface_gpio_port_w( device, S3C44B0_GPIO_PORT_G, data & S3C44B0_GPGDAT_MASK);
		}
		break;
		case S3C44B0_EXTINTPND :
		{
			s3c44b0->gpio.regs.extintpnd = (old_value & ~data);
			s3c44b0_check_pending_eint( device);
		}
		break;
	}
}

/* UART */

static UINT32 s3c44b0_uart_r( device_t *device, int ch, UINT32 offset)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->uart[ch].regs)[offset];
	switch (offset)
	{
		case S3C44B0_UTRSTAT :
		{
			data = (data & ~0x00000006) | 0x00000004 | 0x00000002; // [bit 2] Transmitter empty / [bit 1] Transmit buffer empty
		}
		break;
		case S3C44B0_URXH :
		{
			UINT8 rxdata = data & 0xFF;
			verboselog( device->machine(), 5, "UART %d read %02X (%c)\n", ch, rxdata, ((rxdata >= 32) && (rxdata < 128)) ? (char)rxdata : '?');
			s3c44b0->uart[ch].regs.utrstat &= ~1; // [bit 0] Receive buffer data ready
		}
		break;
	}
	return data;
}

static void s3c44b0_uart_w( device_t *device, int ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	COMBINE_DATA(&((UINT32*)&s3c44b0->uart[ch].regs)[offset]);
	s3c44b0_uart_t *uart = &s3c44b0->uart[ch];
	switch (offset)
	{
		case S3C44B0_UTXH :
		{
			UINT8 txdata = data & 0xFF;
			verboselog( device->machine(), 5, "UART %d write %02X (%c)\n", ch, txdata, ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#ifdef UART_PRINTF
			printf( "%c", ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#endif
		}
		break;
		case S3C44B0_UBRDIV :
		{
			UINT32 mclk, hz;
			mclk = s3c44b0_get_mclk( device);
			hz = (mclk / (uart->regs.ubrdiv + 1)) / 16;
			verboselog( device->machine(), 5, "UART %d - mclk %08X hz %08X\n", ch, mclk, hz);
			uart->timer->adjust( attotime::from_hz( hz), ch, attotime::from_hz( hz));
		}
		break;
	}
}

static READ32_DEVICE_HANDLER( s3c44b0_uart_0_r )
{
	UINT32 data = s3c44b0_uart_r( device, 0, offset);
//  verboselog( space.machine(), 9, "(UART 0) %08X -> %08X\n", S3C44B0_BASE_UART_0 + (offset << 2), data);
	return data;
}

static READ32_DEVICE_HANDLER( s3c44b0_uart_1_r )
{
	UINT32 data = s3c44b0_uart_r( device, 1, offset);
//  verboselog( space.machine(), 9, "(UART 1) %08X -> %08X\n", S3C44B0_BASE_UART_1 + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_uart_0_w )
{
	verboselog( space.machine(), 9, "(UART 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_UART_0 + (offset << 2), data, mem_mask);
	s3c44b0_uart_w( device, 0, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( s3c44b0_uart_1_w )
{
	verboselog( space.machine(), 9, "(UART 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_UART_1 + (offset << 2), data, mem_mask);
	s3c44b0_uart_w( device, 1, offset, data, mem_mask);
}

void s3c44b0_uart_fifo_w( device_t *device, int uart, UINT8 data)
{
//  printf( "s3c44b0_uart_fifo_w (%c)\n", data);
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0->uart[uart].regs.urxh = data;
	s3c44b0->uart[uart].regs.utrstat |= 1; // [bit 0] Receive buffer data ready
}

static TIMER_CALLBACK( s3c44b0_uart_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_t *s3c44b0 = get_token( device);
	int ch = param;
	s3c44b0_uart_t *uart = &s3c44b0->uart[ch];
	verboselog( machine, 2, "UART %d timer callback\n", ch);
	if ((uart->regs.ucon & (1 << 9)) != 0)
	{
		const int ch_int[] = { S3C44B0_INT_UTXD0, S3C44B0_INT_UTXD1 };
		s3c44b0_request_irq( device, ch_int[ch]);
	}
}

/* Watchdog Timer */

static UINT16 s3c44b0_wdt_calc_current_count( device_t *device)
{
	return 0;
}

static READ32_DEVICE_HANDLER( s3c44b0_wdt_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->wdt.regs)[offset];
	switch (offset)
	{
		case S3C44B0_WTCNT :
		{
			// is wdt active?
			if ((s3c44b0->wdt.regs.wtcon & (1 << 5)) != 0)
			{
				data = s3c44b0_wdt_calc_current_count( device);
			}
		}
		break;
	}
	verboselog( space.machine(), 9, "(WDT) %08X -> %08X\n", S3C44B0_BASE_WDT + (offset << 2), data);
	return data;
}

static void s3c44b0_wdt_start( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 mclk, prescaler, clock;
	double freq, hz;
	verboselog( device->machine(), 1, "WDT start\n");
	mclk = s3c44b0_get_mclk( device);
	prescaler = BITS( s3c44b0->wdt.regs.wtcon, 15, 8);
	clock = 16 << BITS( s3c44b0->wdt.regs.wtcon, 4, 3);
	freq = (double)mclk / (prescaler + 1) / clock;
	hz = freq / s3c44b0->wdt.regs.wtcnt;
	verboselog( device->machine(), 5, "WDT mclk %d prescaler %d clock %d freq %f hz %f\n", mclk, prescaler, clock, freq, hz);
	s3c44b0->wdt.timer->adjust( attotime::from_hz( hz), 0, attotime::from_hz( hz));
}

static void s3c44b0_wdt_stop( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 1, "WDT stop\n");
	s3c44b0->wdt.regs.wtcnt = s3c44b0_wdt_calc_current_count( device);
	s3c44b0->wdt.timer->adjust( attotime::never, 0);
}

static void s3c44b0_wdt_recalc( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if ((s3c44b0->wdt.regs.wtcon & (1 << 5)) != 0)
	{
		s3c44b0_wdt_start( device);
	}
	else
	{
		s3c44b0_wdt_stop( device);
	}
}

static WRITE32_DEVICE_HANDLER( s3c44b0_wdt_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->wdt.regs)[offset];
	verboselog( space.machine(), 9, "(WDT) %08X <- %08X\n", S3C44B0_BASE_WDT + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->wdt.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_WTCON :
		{
			if ((data & (1 << 5)) != (old_value & (1 << 5)))
			{
				s3c44b0_wdt_recalc( device);
			}
		}
		break;
	}
}

static TIMER_CALLBACK( s3c44b0_wdt_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( machine, 2, "WDT timer callback\n");
	if ((s3c44b0->wdt.regs.wtcon & (1 << 2)) != 0)
	{
		s3c44b0_request_irq( device, S3C44B0_INT_WDT);
	}
	if ((s3c44b0->wdt.regs.wtcon & (1 << 0)) != 0)
	{
		s3c44b0_reset( device);
	}
}

/* CPU Wrapper */

static READ32_DEVICE_HANDLER( s3c44b0_cpuwrap_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->cpuwrap.regs)[offset];
	verboselog( space.machine(), 9, "(CPUWRAP) %08X -> %08X\n", S3C44B0_BASE_CPU_WRAPPER + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_cpuwrap_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( space.machine(), 9, "(CPUWRAP) %08X <- %08X\n", S3C44B0_BASE_CPU_WRAPPER + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->cpuwrap.regs)[offset]);
}

/* A/D Converter */

static READ32_DEVICE_HANDLER( s3c44b0_adc_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->adc.regs)[offset];
	verboselog( space.machine(), 9, "(ADC) %08X -> %08X\n", S3C44B0_BASE_ADC + (offset << 2), data);
	return data;
}

static void s3c44b0_adc_start( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 mclk, prescaler;
	double freq, hz;
	verboselog( device->machine(), 1, "ADC start\n");
	mclk = s3c44b0_get_mclk( device);
	prescaler = BITS( s3c44b0->adc.regs.adcpsr, 7, 0);
	freq = (double)mclk / (2 * (prescaler + 1)) / 16;
	hz = freq / 1; //s3c44b0->wdt.regs.wtcnt;
	verboselog( device->machine(), 5, "ADC mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	s3c44b0->adc.timer->adjust( attotime::from_hz( hz), 0);
}

static void s3c44b0_adc_stop( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 1, "ADC stop\n");
	s3c44b0->adc.timer->adjust( attotime::never, 0);
}

static void s3c44b0_adc_recalc( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if ((s3c44b0->adc.regs.adccon & (1 << 0)) != 0)
	{
		s3c44b0_adc_start( device);
	}
	else
	{
		s3c44b0_adc_stop( device);
	}
}

static WRITE32_DEVICE_HANDLER( s3c44b0_adc_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->wdt.regs)[offset];
	verboselog( space.machine(), 9, "(ADC) %08X <- %08X\n", S3C44B0_BASE_ADC + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->adc.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_ADCCON :
		{
			if ((data & (1 << 0)) != (old_value & (1 << 0)))
			{
				s3c44b0_adc_recalc( device);
			}
			s3c44b0->adc.regs.adccon &= ~(1 << 0); // "this bit is cleared after the start-up"
		}
		break;
	}
}

static TIMER_CALLBACK( s3c44b0_adc_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( machine, 2, "ADC timer callback\n");
	s3c44b0->adc.regs.adccon |= (1 << 6);
	s3c44b0_request_irq( device, S3C44B0_INT_ADC);
}

/* SIO */

static READ32_DEVICE_HANDLER( s3c44b0_sio_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->sio.regs)[offset];
	verboselog( space.machine(), 9, "(SIO) %08X -> %08X\n", S3C44B0_BASE_SIO + (offset << 2), data);
	return data;
}

static void s3c44b0_sio_start( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 mclk, prescaler;
	double freq, hz;
	verboselog( device->machine(), 1, "SIO start\n");
	mclk = s3c44b0_get_mclk( device);
	prescaler = BITS( s3c44b0->sio.regs.sbrdr, 11, 0);
	freq = (double)mclk / 2 / (prescaler + 1);
	hz = freq / 1; //s3c44b0->wdt.regs.wtcnt;
	verboselog( device->machine(), 5, "SIO mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	s3c44b0->sio.timer->adjust( attotime::from_hz( hz), 0);
//  printf( "SIO transmit %02X (%c)\n", s3c44b0->sio.regs.siodat, ((s3c44b0->sio.regs.siodat >= 32) && (s3c44b0->sio.regs.siodat < 128)) ? (char)s3c44b0->sio.regs.siodat : '?');
}

static void s3c44b0_sio_stop( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 1, "SIO stop\n");
//  s3c44b0->wdt.regs.wtcnt = s3c44b0_wdt_calc_current_count( device);
	s3c44b0->sio.timer->adjust( attotime::never, 0);
}

static void s3c44b0_sio_recalc( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if ((s3c44b0->sio.regs.siocon & (1 << 3)) != 0)
	{
		s3c44b0_sio_start( device);
	}
	else
	{
		s3c44b0_sio_stop( device);
	}
}

static WRITE32_DEVICE_HANDLER( s3c44b0_sio_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->sio.regs)[offset];
	verboselog( space.machine(), 9, "(SIO) %08X <- %08X\n", S3C44B0_BASE_SIO + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->sio.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_SIOCON :
		{
			if ((old_value & (1 << 3)) != (data & (1 << 3)))
			{
				s3c44b0_sio_recalc( device);
			}
			s3c44b0->sio.regs.siocon &= ~(1 << 3); // "This bit is cleared just after writing this bit as 1."
		}
		break;
	}
}

static TIMER_CALLBACK( s3c44b0_sio_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( machine, 2, "SIO timer callback\n");

	s3c44b0->sio.regs.siodat = 0x00; // TEST

	if ((s3c44b0->sio.regs.siocon & (1 << 0)) != 0)
	{
		s3c44b0_request_irq( device, S3C44B0_INT_SIO);
	}
}

/* IIS */

INLINE void iface_i2s_data_w( device_t *device, address_space &space, int ch, UINT16 data)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	if (s3c44b0->iface->i2s.data_w)
	{
		(s3c44b0->iface->i2s.data_w)( device, space, ch, data, 0);
	}
}

static void s3c44b0_iis_start( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 mclk;
	int prescaler;
	double freq, hz;
	const int div[] = { 2, 4, 6, 8, 10, 12, 14, 16, 1, 0, 3, 0, 5, 0, 7, 0 };
	verboselog( device->machine(), 1, "IIS start\n");
	mclk = s3c44b0_get_mclk( device);
	prescaler = BITS( s3c44b0->iis.regs.iispsr, 3, 0);
	freq = (double)mclk / div[prescaler];
	hz = freq / 256 * 2;
	verboselog( device->machine(), 5, "IIS mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	s3c44b0->iis.timer->adjust( attotime::from_hz( hz), 0, attotime::from_hz( hz));
}

static void s3c44b0_iis_stop( device_t *device)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 1, "IIS stop\n");
	s3c44b0->iis.timer->adjust( attotime::never, 0);
}

static READ32_DEVICE_HANDLER( s3c44b0_iis_r )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->iis.regs)[offset];
	verboselog( space.machine(), 9, "(IIS) %08X -> %08X\n", S3C44B0_BASE_IIS + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_iis_w )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->iis.regs)[offset];
	verboselog( space.machine(), 9, "(IIS) %08X <- %08X\n", S3C44B0_BASE_IIS + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&s3c44b0->iis.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_IISCON :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				if ((data & (1 << 0)) != 0)
				{
					s3c44b0_iis_start( device);
				}
				else
				{
					s3c44b0_iis_stop( device);
				}
			}
		}
		break;
		case S3C44B0_IISFIFO :
		{
			if (ACCESSING_BITS_16_31)
			{
				s3c44b0->iis.fifo[s3c44b0->iis.fifo_index++] = BITS( data, 31, 16);
			}
			if (ACCESSING_BITS_0_15)
			{
				s3c44b0->iis.fifo[s3c44b0->iis.fifo_index++] = BITS( data, 15, 0);
			}
			if (s3c44b0->iis.fifo_index == 2)
			{
				s3c44b0->iis.fifo_index = 0;
				iface_i2s_data_w( device, space, 0, s3c44b0->iis.fifo[0]);
				iface_i2s_data_w( device, space, 1, s3c44b0->iis.fifo[1]);
			}
		}
		break;
	}
}

static TIMER_CALLBACK( s3c44b0_iis_timer_exp )
{
	device_t *device = (device_t *)ptr;
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0_iis_t *iis = &s3c44b0->iis;
	verboselog( machine, 2, "IIS timer callback\n");
	if ((iis->regs.iiscon & (1 << 5)) != 0)
	{
		s3c44b0_bdma_request_iis( device);
	}
}

/* ZDMA */

static void s3c44b0_zdma_trigger( device_t *device, int ch)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0_dma_t *dma = &s3c44b0->zdma[ch];
	UINT32 saddr, daddr;
	int dal, dst, opt, das, cnt;
	verboselog( device->machine(), 5, "s3c44b0_zdma_trigger %d\n", ch);
	dst = BITS( dma->regs.dcsrc, 31, 30);
	dal = BITS( dma->regs.dcsrc, 29, 28);
	saddr = BITS( dma->regs.dcsrc, 27, 0);
	verboselog( device->machine(), 5, "dst %d dal %d saddr %08X\n", dst, dal, saddr);
	opt = BITS( dma->regs.dcdst, 31, 30);
	das = BITS( dma->regs.dcdst, 29, 28);
	daddr = BITS( dma->regs.dcdst, 27, 0);
	verboselog( device->machine(), 5, "opt %d das %d daddr %08X\n", opt, das, daddr);
	cnt = BITS( dma->regs.dccnt, 19, 0);
	verboselog( device->machine(), 5, "icnt %08X\n", cnt);
	while (cnt > 0)
	{
		verboselog( device->machine(), 9, "[%08X] -> [%08X]\n", saddr, daddr);
		switch (dst)
		{
			case 0 : s3c44b0->space->write_byte( daddr, s3c44b0->space->read_byte( saddr)); break;
			case 1 : s3c44b0->space->write_word( daddr, s3c44b0->space->read_word( saddr)); break;
			case 2 : s3c44b0->space->write_dword( daddr, s3c44b0->space->read_dword( saddr)); break;
		}
		switch (dal)
		{
			case 1 : saddr += (1 << dst); break;
			case 2 : saddr -= (1 << dst); break;
		}
		switch (das)
		{
			case 1 : daddr += (1 << dst); break;
			case 2 : daddr -= (1 << dst); break;
		}
		cnt -= (1 << dst);
	}
	dma->regs.dcsrc = CLR_BITS( dma->regs.dcsrc, 27, 0) | saddr;
	dma->regs.dcdst = CLR_BITS( dma->regs.dcdst, 27, 0) | daddr;
	dma->regs.dccnt = CLR_BITS( dma->regs.dcdst, 19, 0) | cnt;
	if (cnt == 0)
	{
		if ((dma->regs.dccnt & (1 << 23)) != 0)
		{
			const int ch_int[] = { S3C44B0_INT_ZDMA0, S3C44B0_INT_ZDMA1 };
			s3c44b0_request_irq( device, ch_int[ch]);
		}
	}
}

static void s3c44b0_zdma_start( device_t *device, int ch)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0_dma_t *dma = &s3c44b0->zdma[ch];
	verboselog( device->machine(), 5, "ZDMA %d start\n", ch);
	dma->regs.dcsrc = dma->regs.disrc;
	dma->regs.dcdst = dma->regs.didst;
	dma->regs.dccnt = dma->regs.dicnt;
	s3c44b0_zdma_trigger( device, ch);
}

static UINT32 s3c44b0_zdma_r( device_t *device, int ch, UINT32 offset)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->zdma[ch].regs)[offset];
	return data;
}

static void s3c44b0_zdma_w( device_t *device, int ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->zdma[ch].regs)[offset];
	COMBINE_DATA(&((UINT32*)&s3c44b0->zdma[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_DCON :
		{
			if ((old_value & 3) != (data & 3))
			{
				switch (data & 3)
				{
					case 1 : s3c44b0_zdma_start( device, ch); break;
				}
			}
			s3c44b0->zdma[ch].regs.dcon &= ~3; // "After writing 01,10,11, CMD bit is cleared automatically"
		}
		break;
	}
}

static READ32_DEVICE_HANDLER( s3c44b0_zdma_0_r )
{
	UINT32 data = s3c44b0_zdma_r( device, 0, offset);
	verboselog( space.machine(), 9, "(ZDMA 0) %08X -> %08X\n", S3C44B0_BASE_ZDMA_0 + (offset << 2), data);
	return data;
}

static READ32_DEVICE_HANDLER( s3c44b0_zdma_1_r )
{
	UINT32 data = s3c44b0_zdma_r( device, 1, offset);
	verboselog( space.machine(), 9, "(ZDMA 1) %08X -> %08X\n", S3C44B0_BASE_ZDMA_1 + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_zdma_0_w )
{
	verboselog( space.machine(), 9, "(ZDMA 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_ZDMA_0 + (offset << 2), data, mem_mask);
	s3c44b0_zdma_w( device, 0, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( s3c44b0_zdma_1_w )
{
	verboselog( space.machine(), 9, "(ZDMA 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_ZDMA_1 + (offset << 2), data, mem_mask);
	s3c44b0_zdma_w( device, 1, offset, data, mem_mask);
}

static TIMER_CALLBACK( s3c44b0_zdma_timer_exp )
{
	int ch = param;
	verboselog( machine, 2, "ZDMA %d timer callback\n", ch);
}

/* BDMA */

static void s3c44b0_bdma_trigger( device_t *device, int ch)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0_dma_t *dma = &s3c44b0->bdma[ch];
	UINT32 saddr, daddr;
	int dal, dst, tdm, das, cnt;
	verboselog( device->machine(), 5, "s3c44b0_bdma_trigger %d\n", ch);
	dst = BITS( dma->regs.dcsrc, 31, 30);
	dal = BITS( dma->regs.dcsrc, 29, 28);
	saddr = BITS( dma->regs.dcsrc, 27, 0);
	verboselog( device->machine(), 5, "dst %d dal %d saddr %08X\n", dst, dal, saddr);
	tdm = BITS( dma->regs.dcdst, 31, 30);
	das = BITS( dma->regs.dcdst, 29, 28);
	daddr = BITS( dma->regs.dcdst, 27, 0);
	verboselog( device->machine(), 5, "tdm %d das %d daddr %08X\n", tdm, das, daddr);
	cnt = BITS( dma->regs.dccnt, 19, 0);
	verboselog( device->machine(), 5, "icnt %08X\n", cnt);
	verboselog( device->machine(), 9, "[%08X] -> [%08X]\n", saddr, daddr);
	switch (dst)
	{
		case 0 : s3c44b0->space->write_byte( daddr, s3c44b0->space->read_byte( saddr)); break;
		case 1 : s3c44b0->space->write_word( daddr, s3c44b0->space->read_word( saddr)); break;
		case 2 : s3c44b0->space->write_dword( daddr, s3c44b0->space->read_dword( saddr)); break;
	}
	switch (dal)
	{
		case 1 : saddr += (1 << dst); break;
		case 2 : saddr -= (1 << dst); break;
	}
	switch (das)
	{
		case 1 : daddr += (1 << dst); break;
		case 2 : daddr -= (1 << dst); break;
	}
	cnt -= (1 << dst);
	dma->regs.dcsrc = CLR_BITS( dma->regs.dcsrc, 27, 0) | saddr;
	dma->regs.dcdst = CLR_BITS( dma->regs.dcdst, 27, 0) | daddr;
	dma->regs.dccnt = CLR_BITS( dma->regs.dcdst, 19, 0) | cnt;
	if (cnt == 0)
	{
		if ((dma->regs.dccnt & (1 << 23)) != 0)
		{
			const int ch_int[] = { S3C44B0_INT_BDMA0, S3C44B0_INT_BDMA1 };
			s3c44b0_request_irq( device, ch_int[ch]);
		}
	}
}

static void s3c44b0_bdma_request_iis( device_t *device)
{
//  s3c44b0_t *s3c44b0 = get_token( device);
//  s3c44b0_dma_regs_t *regs = &s3c24xx->bdma[0].regs;
	verboselog( device->machine(), 5, "s3c44b0_bdma_request_iis\n");
	s3c44b0_bdma_trigger( device, 0);
}

static UINT32 s3c44b0_bdma_r( device_t *device, int ch, UINT32 offset)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 data = ((UINT32*)&s3c44b0->bdma[ch].regs)[offset];
	return data;
}

static void s3c44b0_bdma_start( device_t *device, int ch)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0_dma_t *dma = &s3c44b0->bdma[ch];
	int qsc;
	verboselog( device->machine(), 5, "BDMA %d start\n", ch);
	qsc = BITS( dma->regs.dicnt, 31, 30);
	if ((ch == 0) && (qsc == 1))
	{
		// IIS
	}
	else
	{
		printf( "s3c44b0_bdma_start - todo\n");
	}
	dma->regs.dcsrc = dma->regs.disrc;
	dma->regs.dcdst = dma->regs.didst;
	dma->regs.dccnt = dma->regs.dicnt;
}

static void s3c44b0_bdma_stop( device_t *device, int ch)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	verboselog( device->machine(), 5, "BDMA %d stop\n", ch);
	s3c44b0->bdma[ch].timer->adjust( attotime::never, ch);
}

static void s3c44b0_bdma_w( device_t *device, int ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	s3c44b0_t *s3c44b0 = get_token( device);
	UINT32 old_value = ((UINT32*)&s3c44b0->bdma[ch].regs)[offset];
	COMBINE_DATA(&((UINT32*)&s3c44b0->bdma[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_DICNT :
		{
			if ((old_value & (1 << 20)) != (data & (1 << 20)))
			{
				if ((data & (1 << 20)) != 0)
				{
					s3c44b0_bdma_start( device, ch);
				}
				else
				{
					s3c44b0_bdma_stop( device, ch);
				}
			}
		}
		break;
	}
}

static READ32_DEVICE_HANDLER( s3c44b0_bdma_0_r )
{
	UINT32 data = s3c44b0_bdma_r( device, 0, offset);
	verboselog( space.machine(), 9, "(BDMA 0) %08X -> %08X\n", S3C44B0_BASE_BDMA_0 + (offset << 2), data);
	return data;
}

static READ32_DEVICE_HANDLER( s3c44b0_bdma_1_r )
{
	UINT32 data = s3c44b0_bdma_r( device, 1, offset);
	verboselog( space.machine(), 9, "(BDMA 1) %08X -> %08X\n", S3C44B0_BASE_BDMA_1 + (offset << 2), data);
	return data;
}

static WRITE32_DEVICE_HANDLER( s3c44b0_bdma_0_w )
{
	verboselog( space.machine(), 9, "(BDMA 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_BDMA_0 + (offset << 2), data, mem_mask);
	s3c44b0_bdma_w( device, 0, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( s3c44b0_bdma_1_w )
{
	verboselog( space.machine(), 9, "(BDMA 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_BDMA_1 + (offset << 2), data, mem_mask);
	s3c44b0_bdma_w( device, 1, offset, data, mem_mask);
}

static TIMER_CALLBACK( s3c44b0_bdma_timer_exp )
{
//  device_t *device = (device_t *)ptr;
//  s3c44b0_t *s3c44b0 = get_token( device);
	int ch = param;
	verboselog( machine, 2, "BDMA %d timer callback\n", ch);
}

// ...

static DEVICE_RESET( s3c44b0 )
{
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0->iis.fifo_index = 0;
//  s3c44b0->iic.data_index = 0;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	s3c44b0->gpio.regs.gstatus2 = 0x00000001; // Boot is caused by power on reset
#endif
	s3c44b0->irq.line_irq = s3c44b0->irq.line_fiq = CLEAR_LINE;
}

VIDEO_START( s3c44b0 )
{
	device_t *device = machine.device( S3C44B0_TAG);
	s3c44b0_video_start( device, machine);
}

SCREEN_UPDATE_RGB32( s3c44b0 )
{
	device_t *device = screen.machine().device( S3C44B0_TAG);
	return s3c44b0_video_update( device, screen, bitmap, cliprect);
}

DEVICE_START( s3c44b0 )
{
	running_machine &machine = device->machine();
	address_space &space = machine.device( "maincpu")->memory().space( AS_PROGRAM);
	s3c44b0_t *s3c44b0 = get_token( device);
	s3c44b0->iface = (const s3c44b0_interface *)device->static_config();
	s3c44b0->space = &space;
	s3c44b0->cpu = downcast<cpu_device *>(device->machine().device( "maincpu"));
	for (int i = 0; i < 6; i++) s3c44b0->pwm.timer[i] = machine.scheduler().timer_alloc(FUNC(s3c44b0_pwm_timer_exp), (void*)device);
	for (int i = 0; i < 2; i++) s3c44b0->uart[i].timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_uart_timer_exp), (void*)device);
	for (int i = 0; i < 2; i++) s3c44b0->zdma[i].timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_zdma_timer_exp), (void*)device);
	for (int i = 0; i < 2; i++) s3c44b0->bdma[i].timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_bdma_timer_exp), (void*)device);
	s3c44b0->lcd.timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_lcd_timer_exp), (void*)device);
	s3c44b0->wdt.timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_wdt_timer_exp), (void*)device);
	s3c44b0->sio.timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_sio_timer_exp), (void*)device);
	s3c44b0->adc.timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_adc_timer_exp), (void*)device);
	s3c44b0->iic.timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_iic_timer_exp), (void*)device);
	s3c44b0->iis.timer = machine.scheduler().timer_alloc(FUNC(s3c44b0_iis_timer_exp), (void*)device);
	space.install_legacy_readwrite_handler( *device, 0x01c00000, 0x01c0000b, 0, 0, FUNC(s3c44b0_cpuwrap_r), FUNC(s3c44b0_cpuwrap_w));
	space.install_legacy_readwrite_handler( *device, 0x01d00000, 0x01d0002b, 0, 0, FUNC(s3c44b0_uart_0_r), FUNC(s3c44b0_uart_0_w));
	space.install_legacy_readwrite_handler( *device, 0x01d04000, 0x01d0402b, 0, 0, FUNC(s3c44b0_uart_1_r), FUNC(s3c44b0_uart_1_w));
	space.install_legacy_readwrite_handler( *device, 0x01d14000, 0x01d14013, 0, 0, FUNC(s3c44b0_sio_r), FUNC(s3c44b0_sio_w));
	space.install_legacy_readwrite_handler( *device, 0x01d18000, 0x01d18013, 0, 0, FUNC(s3c44b0_iis_r), FUNC(s3c44b0_iis_w));
	space.install_legacy_readwrite_handler( *device, 0x01d20000, 0x01d20057, 0, 0, FUNC(s3c44b0_gpio_r), FUNC(s3c44b0_gpio_w));
	space.install_legacy_readwrite_handler( *device, 0x01d30000, 0x01d3000b, 0, 0, FUNC(s3c44b0_wdt_r), FUNC(s3c44b0_wdt_w));
	space.install_legacy_readwrite_handler( *device, 0x01d40000, 0x01d4000b, 0, 0, FUNC(s3c44b0_adc_r), FUNC(s3c44b0_adc_w));
	space.install_legacy_readwrite_handler( *device, 0x01d50000, 0x01d5004f, 0, 0, FUNC(s3c44b0_pwm_r), FUNC(s3c44b0_pwm_w));
	space.install_legacy_readwrite_handler( *device, 0x01d60000, 0x01d6000f, 0, 0, FUNC(s3c44b0_iic_r), FUNC(s3c44b0_iic_w));
	space.install_legacy_readwrite_handler( *device, 0x01d80000, 0x01d8000f, 0, 0, FUNC(s3c44b0_clkpow_r), FUNC(s3c44b0_clkpow_w));
	space.install_legacy_readwrite_handler( *device, 0x01e00000, 0x01e0003f, 0, 0, FUNC(s3c44b0_irq_r), FUNC(s3c44b0_irq_w));
	space.install_legacy_readwrite_handler( *device, 0x01e80000, 0x01e8001b, 0, 0, FUNC(s3c44b0_zdma_0_r), FUNC(s3c44b0_zdma_0_w));
	space.install_legacy_readwrite_handler( *device, 0x01e80020, 0x01e8003b, 0, 0, FUNC(s3c44b0_zdma_1_r), FUNC(s3c44b0_zdma_1_w));
	space.install_legacy_readwrite_handler( *device, 0x01f00000, 0x01f00047, 0, 0, FUNC(s3c44b0_lcd_r), FUNC(s3c44b0_lcd_w));
	space.install_legacy_readwrite_handler( *device, 0x01f80000, 0x01f8001b, 0, 0, FUNC(s3c44b0_bdma_0_r), FUNC(s3c44b0_bdma_0_w));
	space.install_legacy_readwrite_handler( *device, 0x01f80020, 0x01f8003b, 0, 0, FUNC(s3c44b0_bdma_1_r), FUNC(s3c44b0_bdma_1_w));
}

const device_type S3C44B0 = &device_creator<s3c44b0_device>;

s3c44b0_device::s3c44b0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S3C44B0, "Samsung S3C44B0", tag, owner, clock)
{
	m_token = global_alloc_clear(s3c44b0_t);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void s3c44b0_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3c44b0_device::device_start()
{
	DEVICE_START_NAME( s3c44b0 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3c44b0_device::device_reset()
{
	DEVICE_RESET_NAME( s3c44b0 )(this);
}


