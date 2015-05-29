// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
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
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("%s: %s", machine.describe_context(), buf);
	}
}

const device_type S3C44B0 = &device_creator<s3c44b0_device>;

s3c44b0_device::s3c44b0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, S3C44B0, "Samsung S3C44B0", tag, owner, clock, "s3c44b0", __FILE__),
					m_port_r_cb(*this),
					m_port_w_cb(*this),
					m_scl_w_cb(*this),
					m_sda_r_cb(*this),
					m_sda_w_cb(*this),
					m_data_r_cb(*this),
					m_data_w_cb(*this)
{
	memset(&m_irq, 0, sizeof(s3c44b0_irq_t));
	memset(m_zdma, 0, sizeof(s3c44b0_dma_t)*2);
	memset(m_bdma, 0, sizeof(s3c44b0_dma_t)*2);
	memset(&m_clkpow, 0, sizeof(s3c44b0_clkpow_t));
	memset(&m_lcd, 0, sizeof(s3c44b0_lcd_t));
	memset(m_uart, 0, sizeof(s3c44b0_uart_t)*2);
	memset(&m_sio, 0, sizeof(s3c44b0_sio_t));
	memset(&m_pwm, 0, sizeof(s3c44b0_pwm_t));
	memset(&m_wdt, 0, sizeof(s3c44b0_wdt_t));
	memset(&m_iic, 0, sizeof(s3c44b0_iic_t));
	memset(&m_iis, 0, sizeof(s3c44b0_iis_t));
	memset(&m_gpio, 0, sizeof(s3c44b0_gpio_t));
	memset(&m_adc, 0, sizeof(s3c44b0_adc_t));
	memset(&m_cpuwrap, 0, sizeof(s3c44b0_cpuwrap_t));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3c44b0_device::device_start()
{
	m_cpu = machine().device<cpu_device>("maincpu");

	m_port_r_cb.resolve();
	m_port_w_cb.resolve();
	m_scl_w_cb.resolve();
	m_sda_r_cb.resolve();
	m_sda_w_cb.resolve();
	m_data_r_cb.resolve_safe(0);
	m_data_w_cb.resolve();


	for (int i = 0; i < 6; i++) m_pwm.timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::pwm_timer_exp),this));
	for (int i = 0; i < 2; i++) m_uart[i].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::uart_timer_exp),this));
	for (int i = 0; i < 2; i++) m_zdma[i].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::zdma_timer_exp),this));
	for (int i = 0; i < 2; i++) m_bdma[i].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::bdma_timer_exp),this));

	m_lcd.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::lcd_timer_exp),this));
	m_wdt.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::wdt_timer_exp),this));
	m_sio.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::sio_timer_exp),this));
	m_adc.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::adc_timer_exp),this));
	m_iic.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::iic_timer_exp),this));
	m_iis.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3c44b0_device::iis_timer_exp),this));

	video_start();

	save_item(NAME(m_irq.regs.intcon));
	save_item(NAME(m_irq.regs.intpnd));
	save_item(NAME(m_irq.regs.intmod));
	save_item(NAME(m_irq.regs.intmsk));
	save_item(NAME(m_irq.regs.i_pslv));
	save_item(NAME(m_irq.regs.i_pmst));
	save_item(NAME(m_irq.regs.i_cslv));
	save_item(NAME(m_irq.regs.i_cmst));
	save_item(NAME(m_irq.regs.i_ispr));
	save_item(NAME(m_irq.regs.i_ispc));
	save_item(NAME(m_irq.regs.reserved));
	save_item(NAME(m_irq.regs.f_ispr));
	save_item(NAME(m_irq.regs.f_ispc));
	save_item(NAME(m_irq.line_irq));
	save_item(NAME(m_irq.line_fiq));

	save_item(NAME(m_clkpow.regs.pllcon));
	save_item(NAME(m_clkpow.regs.clkcon));
	save_item(NAME(m_clkpow.regs.clkslow));
	save_item(NAME(m_clkpow.regs.locktime));

	// FIXME: how to save m_lcd.bitmap which gets allocated/freed during emulation?
	save_item(NAME(m_lcd.regs.lcdcon1));
	save_item(NAME(m_lcd.regs.lcdcon2));
	save_item(NAME(m_lcd.regs.lcdsaddr1));
	save_item(NAME(m_lcd.regs.lcdsaddr2));
	save_item(NAME(m_lcd.regs.lcdsaddr3));
	save_item(NAME(m_lcd.regs.redlut));
	save_item(NAME(m_lcd.regs.greenlut));
	save_item(NAME(m_lcd.regs.bluelut));
	save_item(NAME(m_lcd.regs.reserved));
	save_item(NAME(m_lcd.regs.lcdcon3));
	save_item(NAME(m_lcd.regs.dithmode));
	save_item(NAME(m_lcd.vramaddr_cur));
	save_item(NAME(m_lcd.vramaddr_max));
	save_item(NAME(m_lcd.offsize));
	save_item(NAME(m_lcd.pagewidth_cur));
	save_item(NAME(m_lcd.pagewidth_max));
	save_item(NAME(m_lcd.modesel));
	save_item(NAME(m_lcd.bswp));
	save_item(NAME(m_lcd.vpos));
	save_item(NAME(m_lcd.hpos));
	save_item(NAME(m_lcd.framerate));
	save_item(NAME(m_lcd.hpos_min));
	save_item(NAME(m_lcd.hpos_max));
	save_item(NAME(m_lcd.hpos_end));
	save_item(NAME(m_lcd.vpos_min));
	save_item(NAME(m_lcd.vpos_max));
	save_item(NAME(m_lcd.vpos_end));
	save_item(NAME(m_lcd.frame_time));

	machine().save().register_postload(save_prepost_delegate(FUNC(s3c44b0_device::s3c44b0_postload), this));

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_zdma[i].regs.dcon), i);
		save_item(NAME(m_zdma[i].regs.disrc), i);
		save_item(NAME(m_zdma[i].regs.didst), i);
		save_item(NAME(m_zdma[i].regs.dicnt), i);
		save_item(NAME(m_zdma[i].regs.dcsrc), i);
		save_item(NAME(m_zdma[i].regs.dcdst), i);
		save_item(NAME(m_zdma[i].regs.dccnt), i);

		save_item(NAME(m_bdma[i].regs.dcon), i);
		save_item(NAME(m_bdma[i].regs.disrc), i);
		save_item(NAME(m_bdma[i].regs.didst), i);
		save_item(NAME(m_bdma[i].regs.dicnt), i);
		save_item(NAME(m_bdma[i].regs.dcsrc), i);
		save_item(NAME(m_bdma[i].regs.dcdst), i);
		save_item(NAME(m_bdma[i].regs.dccnt), i);

		save_item(NAME(m_uart[i].regs.ulcon), i);
		save_item(NAME(m_uart[i].regs.ucon), i);
		save_item(NAME(m_uart[i].regs.ufcon), i);
		save_item(NAME(m_uart[i].regs.umcon), i);
		save_item(NAME(m_uart[i].regs.utrstat), i);
		save_item(NAME(m_uart[i].regs.uerstat), i);
		save_item(NAME(m_uart[i].regs.ufstat), i);
		save_item(NAME(m_uart[i].regs.umstat), i);
		save_item(NAME(m_uart[i].regs.utxh), i);
		save_item(NAME(m_uart[i].regs.urxh), i);
		save_item(NAME(m_uart[i].regs.ubrdiv), i);
	}

	save_item(NAME(m_sio.regs.siocon));
	save_item(NAME(m_sio.regs.siodat));
	save_item(NAME(m_sio.regs.sbrdr));
	save_item(NAME(m_sio.regs.itvcnt));
	save_item(NAME(m_sio.regs.dcntz));

	save_item(NAME(m_pwm.regs.tcfg0));
	save_item(NAME(m_pwm.regs.tcfg1));
	save_item(NAME(m_pwm.regs.tcon));
	save_item(NAME(m_pwm.regs.tcntb0));
	save_item(NAME(m_pwm.regs.tcmpb0));
	save_item(NAME(m_pwm.regs.tcnto0));
	save_item(NAME(m_pwm.regs.tcntb1));
	save_item(NAME(m_pwm.regs.tcmpb1));
	save_item(NAME(m_pwm.regs.tcnto1));
	save_item(NAME(m_pwm.regs.tcntb2));
	save_item(NAME(m_pwm.regs.tcmpb2));
	save_item(NAME(m_pwm.regs.tcnto2));
	save_item(NAME(m_pwm.regs.tcntb3));
	save_item(NAME(m_pwm.regs.tcmpb3));
	save_item(NAME(m_pwm.regs.tcnto3));
	save_item(NAME(m_pwm.regs.tcntb4));
	save_item(NAME(m_pwm.regs.tcmpb4));
	save_item(NAME(m_pwm.regs.tcnto4));
	save_item(NAME(m_pwm.regs.tcntb5));
	save_item(NAME(m_pwm.regs.tcnto5));
	save_item(NAME(m_pwm.cnt));
	save_item(NAME(m_pwm.cmp));
	save_item(NAME(m_pwm.freq));

	save_item(NAME(m_wdt.regs.wtcon));
	save_item(NAME(m_wdt.regs.wtdat));
	save_item(NAME(m_wdt.regs.wtcnt));

	save_item(NAME(m_iic.regs.iiccon));
	save_item(NAME(m_iic.regs.iicstat));
	save_item(NAME(m_iic.regs.iicadd));
	save_item(NAME(m_iic.regs.iicds));
	save_item(NAME(m_iic.count));

	save_item(NAME(m_iis.regs.iiscon));
	save_item(NAME(m_iis.regs.iismod));
	save_item(NAME(m_iis.regs.iispsr));
	save_item(NAME(m_iis.regs.iisfcon));
	save_item(NAME(m_iis.regs.iisfifo));
	save_item(NAME(m_iis.fifo));
	save_item(NAME(m_iis.fifo_index));

	save_item(NAME(m_gpio.regs.gpacon));
	save_item(NAME(m_gpio.regs.gpadat));
	save_item(NAME(m_gpio.regs.gpbcon));
	save_item(NAME(m_gpio.regs.gpbdat));
	save_item(NAME(m_gpio.regs.gpccon));
	save_item(NAME(m_gpio.regs.gpcdat));
	save_item(NAME(m_gpio.regs.gpcup));
	save_item(NAME(m_gpio.regs.gpdcon));
	save_item(NAME(m_gpio.regs.gpddat));
	save_item(NAME(m_gpio.regs.gpdup));
	save_item(NAME(m_gpio.regs.gpecon));
	save_item(NAME(m_gpio.regs.gpedat));
	save_item(NAME(m_gpio.regs.gpeup));
	save_item(NAME(m_gpio.regs.gpfcon));
	save_item(NAME(m_gpio.regs.gpfdat));
	save_item(NAME(m_gpio.regs.gpfup));
	save_item(NAME(m_gpio.regs.gpgcon));
	save_item(NAME(m_gpio.regs.gpgdat));
	save_item(NAME(m_gpio.regs.gpgup));
	save_item(NAME(m_gpio.regs.spucr));
	save_item(NAME(m_gpio.regs.extint));
	save_item(NAME(m_gpio.regs.extintpnd));

	save_item(NAME(m_adc.regs.adccon));
	save_item(NAME(m_adc.regs.adcpsr));
	save_item(NAME(m_adc.regs.adcdat));

	save_item(NAME(m_cpuwrap.regs.syscfg));
	save_item(NAME(m_cpuwrap.regs.ncachbe0));
	save_item(NAME(m_cpuwrap.regs.ncachbe1));
}


void s3c44b0_device::s3c44b0_postload()
{
	m_lcd.frame_period = HZ_TO_ATTOSECONDS(m_lcd.framerate);
	m_lcd.scantime = m_lcd.frame_period / m_lcd.vpos_end;
	m_lcd.pixeltime = m_lcd.frame_period / (m_lcd.vpos_end * m_lcd.hpos_end);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3c44b0_device::device_reset()
{
	m_iis.fifo_index = 0;
	//  m_iic.data_index = 0;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	m_gpio.regs.gstatus2 = 0x00000001; // Boot is caused by power on reset
#endif
	m_irq.line_irq = m_irq.line_fiq = CLEAR_LINE;
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
    IMPLEMENTATION
***************************************************************************/

/* LCD Controller */

rgb_t s3c44b0_device::lcd_get_color_stn_04(UINT8 data)
{
	UINT8 r, g, b;
	r = g = b = BITS(data, 3, 0) << 4;
	return rgb_t(r, g, b);
}

UINT8 s3c44b0_device::lcd_get_color_stn_08_r(UINT8 data)
{
	return ((m_lcd.regs.redlut >> (BITS(data, 7, 5) << 2)) & 0xf) << 4;
}

UINT8 s3c44b0_device::lcd_get_color_stn_08_g(UINT8 data)
{
	return ((m_lcd.regs.greenlut >> (BITS(data, 4, 2) << 2)) & 0xf) << 4;
}

UINT8 s3c44b0_device::lcd_get_color_stn_08_b(UINT8 data)
{
	return ((m_lcd.regs.bluelut >> (BITS(data, 1, 0) << 2)) & 0xf) << 4;
}

void s3c44b0_device::lcd_dma_reload()
{
	int lcdbank, lcdbaseu, lcdbasel;
	lcdbank  = BITS(m_lcd.regs.lcdsaddr1, 26, 21);
	lcdbaseu = BITS(m_lcd.regs.lcdsaddr1, 20, 0);
	lcdbasel = BITS(m_lcd.regs.lcdsaddr2, 20, 0);

	m_lcd.vramaddr_cur = (lcdbank << 22) | (lcdbaseu << 1);
	m_lcd.vramaddr_max = (lcdbank << 22) | (lcdbasel << 1);
	if (lcdbasel == 0) m_lcd.vramaddr_max += 1 << 22;
	m_lcd.offsize = BITS(m_lcd.regs.lcdsaddr3, 19, 9);
	m_lcd.pagewidth_cur = 0;
	m_lcd.pagewidth_max = BITS(m_lcd.regs.lcdsaddr3, 8, 0);
	m_lcd.bswp = BIT(m_lcd.regs.lcdsaddr2, 29); // note: juicebox changes bswp when video playback starts
//  verboselog(machine(), 3, "LCD - vramaddr %08X %08X offsize %08X pagewidth %08X\n", m_lcd.vramaddr_cur, m_lcd.vramaddr_max, m_lcd.offsize, m_lcd.pagewidth_max);
}

void s3c44b0_device::lcd_dma_init()
{
	m_lcd.modesel = BITS(m_lcd.regs.lcdsaddr1, 28, 27);
//  verboselog(machine(), 3, "LCD - modesel %d bswp %d\n", m_lcd.modesel, m_lcd.bswp);
	lcd_dma_reload();
}

void s3c44b0_device::lcd_dma_read(int count, UINT8 *data)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	UINT8 *vram = (UINT8 *)space.get_read_ptr(m_lcd.vramaddr_cur);
	for (int i = 0; i < count / 2; i++)
	{
		if (m_lcd.bswp == 0)
		{
			if ((m_lcd.vramaddr_cur & 2) == 0)
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
		m_lcd.vramaddr_cur += 2;
		m_lcd.pagewidth_cur++;
		if (m_lcd.pagewidth_cur >= m_lcd.pagewidth_max)
		{
			m_lcd.vramaddr_cur += m_lcd.offsize << 1;
			if (m_lcd.vramaddr_cur >= m_lcd.vramaddr_max)
			{
				lcd_dma_reload();
			}
			m_lcd.pagewidth_cur = 0;
			vram = (UINT8 *)space.get_read_ptr(m_lcd.vramaddr_cur);
		}
		else
		{
			vram += 2;
		}
		data += 2;
	}
}

void s3c44b0_device::lcd_render_stn_04()
{
	UINT8 *bitmap = m_lcd.bitmap + ((m_lcd.vpos - m_lcd.vpos_min) * (m_lcd.hpos_max - m_lcd.hpos_min + 1)) + (m_lcd.hpos - m_lcd.hpos_min);
	UINT8 data[16];
	lcd_dma_read(16, data);
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			*bitmap++ = lcd_get_color_stn_04((data[i] >> 4) & 0x0F);
			data[i] = data[i] << 4;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 2))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max)
				{
					m_lcd.vpos = m_lcd.vpos_min;
					bitmap = m_lcd.bitmap;
				}
				m_lcd.hpos = m_lcd.hpos_min;
			}
		}
	}
}

void s3c44b0_device::lcd_render_stn_08()
{
	UINT8 *bitmap = m_lcd.bitmap + ((m_lcd.vpos - m_lcd.vpos_min) * (m_lcd.hpos_max - m_lcd.hpos_min + 1)) + (m_lcd.hpos - m_lcd.hpos_min);
	UINT8 data[16];
	lcd_dma_read(16, data);
	for (int i = 0; i < 16; i++)
	{
		UINT8 xxx[3];
		xxx[0] = lcd_get_color_stn_08_r(data[i]);
		xxx[1] = lcd_get_color_stn_08_g(data[i]);
		xxx[2] = lcd_get_color_stn_08_b(data[i]);
		for (int j = 0; j < 3; j++)
		{
			*bitmap++ = xxx[j];
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max * 6))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max)
				{
					m_lcd.vpos = m_lcd.vpos_min;
					bitmap = m_lcd.bitmap;
				}
				m_lcd.hpos = m_lcd.hpos_min;
			}
		}
	}
}

attotime s3c44b0_device::time_until_pos(int vpos, int hpos)
{
	attoseconds_t time1, time2;
	attotime retval;
	verboselog(machine(), 3, "s3c44b0_time_until_pos - vpos %d hpos %d\n", vpos, hpos);
	time1 = (attoseconds_t)vpos * m_lcd.scantime + (attoseconds_t)hpos * m_lcd.pixeltime;
	time2 = (machine().time() - m_lcd.frame_time).as_attoseconds();
	verboselog(machine(), 3, "machine %f frametime %f time1 %f time2 %f\n", machine().time().as_double(), m_lcd.frame_time.as_double(), attotime(0, time1).as_double(), attotime(0, time2).as_double());
	while (time1 <= time2) time1 += m_lcd.frame_period;
	retval = attotime( 0, time1 - time2);
	verboselog(machine(), 3, "result %f\n", retval.as_double());
	return retval;
}

int s3c44b0_device::lcd_get_vpos()
{
	attoseconds_t delta;
	int vpos;
	delta = (machine().time() - m_lcd.frame_time).as_attoseconds();
	delta = delta + (m_lcd.pixeltime / 2);
	vpos = delta / m_lcd.scantime;
	return (m_lcd.vpos_min + vpos) % m_lcd.vpos_end;
}

int s3c44b0_device::lcd_get_hpos()
{
	attoseconds_t delta;
	int vpos;
	delta = (machine().time() - m_lcd.frame_time).as_attoseconds();
	delta = delta + (m_lcd.pixeltime / 2);
	vpos = delta / m_lcd.scantime;
	delta = delta - (vpos * m_lcd.scantime);
	return delta / m_lcd.pixeltime;
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::lcd_timer_exp )
{
	int vpos = m_lcd.vpos;
	verboselog(machine(), 2, "LCD timer callback (%f)\n", machine().time().as_double());
	verboselog(machine(), 3, "LCD - (1) vramaddr %08X vpos %d hpos %d\n", m_lcd.vramaddr_cur, m_lcd.vpos, m_lcd.hpos);
	switch (m_lcd.modesel)
	{
		case S3C44B0_MODESEL_04 : lcd_render_stn_04(); break;
		case S3C44B0_MODESEL_08 : lcd_render_stn_08(); break;
		default : verboselog(machine(), 0, "s3c44b0_lcd_timer_exp: modesel %d not supported\n", m_lcd.modesel); break;
	}
	verboselog(machine(), 3, "LCD - (2) vramaddr %08X vpos %d hpos %d\n", m_lcd.vramaddr_cur, m_lcd.vpos, m_lcd.hpos);
	if (m_lcd.vpos < vpos)
	{
//      verboselog(machine(), 3, "LCD - (1) frame_time %f\n", attotime_to_double(m_lcd.frame_time));
		m_lcd.frame_time = machine().time() + time_until_pos(m_lcd.vpos_min, m_lcd.hpos_min);
//      verboselog(machine(), 3, "LCD - (2) frame_time %f\n", attotime_to_double(m_lcd.frame_time));
	}
	m_lcd.timer->adjust(time_until_pos(m_lcd.vpos, m_lcd.hpos), 0);
}

void s3c44b0_device::video_start()
{
	// do nothing
}

UINT32 s3c44b0_device::video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_lcd.regs.lcdcon1 & (1 << 0))
	{
		if (m_lcd.bitmap)
		{
			for (int y = 0; y < screen.height(); y++)
			{
				UINT32 *scanline = &bitmap.pix32(y);
				UINT8 *vram = m_lcd.bitmap + y * (m_lcd.hpos_max - m_lcd.hpos_min + 1);
				for (int x = 0; x < screen.width(); x++)
				{
					*scanline++ = rgb_t(vram[0], vram[1], vram[2]);
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
			memset(scanline, 0, screen.width() * 4);
		}
	}
	return 0;
}

READ32_MEMBER( s3c44b0_device::lcd_r )
{
	UINT32 data = ((UINT32*)&m_lcd.regs)[offset];
	switch (offset)
	{
		case S3C44B0_LCDCON1 :
		{
			int vpos = 0;
			// make sure line counter is going
			if (m_lcd.regs.lcdcon1 & (1 << 0))
			{
				vpos = lcd_get_vpos();
				int hpos = lcd_get_hpos();
				if (hpos < m_lcd.hpos_min) vpos = vpos - 1;
				if ((vpos < m_lcd.vpos_min) || (vpos > m_lcd.vpos_max)) vpos = m_lcd.vpos_max;
				vpos = m_lcd.vpos_max - vpos;
			}
			data = (data & ~0xFFC00000) | (vpos << 22);
		}
		break;
	}
//  verboselog(machine(), 9, "(LCD) %08X -> %08X\n", S3C44B0_BASE_LCD + (offset << 2), data);
	return data;
}

void s3c44b0_device::lcd_configure()
{
	screen_device *screen = machine().first_screen();
	int dismode, clkval, lineval, wdly, hozval, lineblank, wlh, mclk;
	double vclk, framerate;
	int width, height;
	verboselog(machine(), 5, "s3c44b0_lcd_configure\n");
	dismode = BITS(m_lcd.regs.lcdcon1, 6, 5);
	clkval = BITS(m_lcd.regs.lcdcon1, 21, 12);
	lineval = BITS(m_lcd.regs.lcdcon2, 9, 0);
	wdly = BITS(m_lcd.regs.lcdcon1, 9, 8);
	hozval = BITS(m_lcd.regs.lcdcon2, 20, 10);
	lineblank = BITS(m_lcd.regs.lcdcon2, 31, 21);
	wlh = BITS(m_lcd.regs.lcdcon1, 11, 10);
	mclk = get_mclk();
	verboselog(machine(), 3, "LCD - dismode %d clkval %d lineval %d wdly %d hozval %d lineblank %d wlh %d mclk %d\n", dismode, clkval, lineval, wdly, hozval, lineblank, wlh, mclk);
	vclk = (double)(mclk / (clkval * 2));
	verboselog(machine(), 3, "LCD - vclk %f\n", vclk);
	framerate = 1 / (((1 / vclk) * (hozval + 1) + (1 / mclk) * (wlh + wdly + lineblank)) * (lineval + 1));
	framerate = framerate / 3; // ???
	verboselog(machine(), 3, "LCD - framerate %f\n", framerate);
	switch (dismode)
	{
		case S3C44B0_PNRMODE_STN_04_SS : width = ((hozval + 1) * 4); break;
		case S3C44B0_PNRMODE_STN_04_DS : width = ((hozval + 1) * 4); break;
		case S3C44B0_PNRMODE_STN_08_SS : width = ((hozval + 1) * 8); break;
		default : fatalerror("invalid display mode (%d)\n", dismode);
	}
	height = lineval + 1;
	m_lcd.framerate = framerate;
	verboselog(machine(), 3, "video_screen_configure %d %d %f\n", width, height, m_lcd.framerate);
	screen->configure(screen->width(), screen->height(), screen->visible_area(), HZ_TO_ATTOSECONDS(m_lcd.framerate));
	m_lcd.hpos_min = 25;
	m_lcd.hpos_max = 25 + width - 1;
	m_lcd.hpos_end = 25 + width - 1 + 25;
	m_lcd.vpos_min = 25;
	m_lcd.vpos_max = 25 + height - 1;
	m_lcd.vpos_end = 25 + height - 1 + 25;
	verboselog(machine(), 3, "LCD - min_x %d min_y %d max_x %d max_y %d\n", m_lcd.hpos_min, m_lcd.vpos_min, m_lcd.hpos_max, m_lcd.vpos_max);
	if (m_lcd.bitmap)
	{
		auto_free(machine(), m_lcd.bitmap);
	}
	m_lcd.bitmap = auto_alloc_array(machine(), UINT8, (m_lcd.hpos_max - m_lcd.hpos_min + 1) * (m_lcd.vpos_max - m_lcd.vpos_min + 1) * 3);
	m_lcd.frame_period = HZ_TO_ATTOSECONDS(m_lcd.framerate);
	m_lcd.scantime = m_lcd.frame_period / m_lcd.vpos_end;
	m_lcd.pixeltime = m_lcd.frame_period / (m_lcd.vpos_end * m_lcd.hpos_end);
//  printf("frame_period %f\n", attotime( 0, m_lcd.frame_period).as_double());
//  printf("scantime %f\n", attotime( 0, m_lcd.scantime).as_double());
//  printf("pixeltime %f\n", attotime( 0, m_lcd.pixeltime).as_double());
}


void s3c44b0_device::lcd_start()
{
	screen_device *screen = machine().first_screen();
	verboselog(machine(), 1, "LCD start\n");
	lcd_configure();
	lcd_dma_init();
	m_lcd.vpos = m_lcd.vpos_min;
	m_lcd.hpos = m_lcd.hpos_min;
	m_lcd.frame_time = screen->time_until_pos( 0, 0);
	m_lcd.timer->adjust(m_lcd.frame_time, 0);
	m_lcd.frame_time = machine().time() + m_lcd.frame_time;
}

void s3c44b0_device::lcd_stop()
{
	verboselog(machine(), 1, "LCD stop\n");
	m_lcd.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::lcd_recalc()
{
	if (m_lcd.regs.lcdcon1 & (1 << 0))
		lcd_start();
	else
		lcd_stop();
}

WRITE32_MEMBER( s3c44b0_device::lcd_w )
{
	UINT32 old_value = ((UINT32*)&m_lcd.regs)[offset];
//  verboselog(machine(), 9, "(LCD) %08X <- %08X\n", S3C44B0_BASE_LCD + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_lcd.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_LCDCON1 :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				lcd_recalc();
			}
		}
		break;
	}
}


/* Clock & Power Management */

UINT32 s3c44b0_device::get_mclk()
{
	UINT32 data, mdiv, pdiv, sdiv;
	data = m_clkpow.regs.pllcon;
	mdiv = BITS(data, 19, 12);
	pdiv = BITS(data, 9, 4);
	sdiv = BITS(data, 1, 0);
	return (UINT32)((double)((mdiv + 8) * clock()) / (double)((pdiv + 2) * (1 << sdiv)));
}

READ32_MEMBER( s3c44b0_device::clkpow_r )
{
	UINT32 data = ((UINT32*)&m_clkpow.regs)[offset];
	verboselog(machine(), 9, "(CLKPOW) %08X -> %08X\n", S3C44B0_BASE_CLKPOW + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::clkpow_w )
{
	verboselog(machine(), 9, "(CLKPOW) %08X <- %08X\n", S3C44B0_BASE_CLKPOW + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_clkpow.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_PLLCON :
		{
			verboselog(machine(), 5, "CLKPOW - mclk %d\n", get_mclk());
			m_cpu->set_unscaled_clock(get_mclk() * CLOCK_MULTIPLIER);
		}
		break;
		case S3C44B0_CLKCON :
		{
			if (data & (1 << 2))
			{
				m_cpu->spin_until_interrupt();
			}
		}
		break;
	}
}

/* Interrupt Controller */

void s3c44b0_device::check_pending_irq()
{
	// normal irq
	UINT32 temp = (m_irq.regs.intpnd & ~m_irq.regs.intmsk) & ~m_irq.regs.intmod;

	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		m_irq.regs.i_ispr |= (1 << int_type);
		if (m_irq.line_irq != ASSERT_LINE)
		{
			m_cpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
			m_irq.line_irq = ASSERT_LINE;
		}
	}
	else
	{
		if (m_irq.line_irq != CLEAR_LINE)
		{
			m_cpu->set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);
			m_irq.line_irq = CLEAR_LINE;
		}
	}
	// fast irq
	temp = (m_irq.regs.intpnd & ~m_irq.regs.intmsk) & m_irq.regs.intmod;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		if (m_irq.line_fiq != ASSERT_LINE)
		{
			m_cpu->set_input_line(ARM7_FIRQ_LINE, ASSERT_LINE);
			m_irq.line_fiq = ASSERT_LINE;
		}
	}
	else
	{
		if (m_irq.line_fiq != CLEAR_LINE)
		{
			m_cpu->set_input_line(ARM7_FIRQ_LINE, CLEAR_LINE);
			m_irq.line_fiq = CLEAR_LINE;
		}
	}
}

void s3c44b0_device::request_irq(UINT32 int_type)
{
	verboselog(machine(), 5, "request irq %d\n", int_type);
	m_irq.regs.intpnd |= (1 << int_type);
	check_pending_irq();
}

void s3c44b0_device::check_pending_eint()
{
	UINT32 temp = m_gpio.regs.extintpnd;
	if (temp != 0)
	{
		UINT32 int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		request_irq(S3C44B0_INT_EINT4_7);
	}
}

void s3c44b0_device::request_eint(UINT32 number)
{
	verboselog(machine(), 5, "request external interrupt %d\n", number);
	if (number < 4)
	{
		request_irq(S3C44B0_INT_EINT0 + number);
	}
	else
	{
		m_gpio.regs.extintpnd |= (1 << (number - 4));
		check_pending_eint();
	}
}

READ32_MEMBER( s3c44b0_device::irq_r )
{
	UINT32 data = ((UINT32*)&m_irq.regs)[offset];
	verboselog(machine(), 9, "(IRQ) %08X -> %08X\n", S3C44B0_BASE_INT + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::irq_w )
{
	verboselog(machine(), 9, "(IRQ) %08X <- %08X\n", S3C44B0_BASE_INT + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_irq.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_INTMSK :
		{
			check_pending_irq();
		}
		break;
		case S3C44B0_I_ISPC :
		{
			m_irq.regs.intpnd = (m_irq.regs.intpnd & ~data); // The bit of INTPND bit is cleared to zero by writing '1' on I_ISPC/F_ISPC
			m_irq.regs.i_ispr = (m_irq.regs.i_ispr & ~data); // The pending bit in I_ISPR register should be cleared by writing I_ISPC register.
			check_pending_irq();
		}
		break;
		case S3C44B0_F_ISPC :
		{
			m_irq.regs.intpnd = (m_irq.regs.intpnd & ~data); // The bit of INTPND bit is cleared to zero by writing '1' on I_ISPC/F_ISPC
			check_pending_irq();
		}
		break;
	}
}

/* PWM Timer */

UINT16 s3c44b0_device::pwm_calc_observation(int ch)
{
	double timeleft, x1, x2;
	UINT32 cnto;
	timeleft = (m_pwm.timer[ch]->remaining()).as_double();
//  printf( "timeleft %f freq %d cntb %d cmpb %d\n", timeleft, m_pwm.freq[ch], m_pwm.cnt[ch], m_pwm.cmp[ch]);
	x1 = 1 / ((double)m_pwm.freq[ch] / (m_pwm.cnt[ch]- m_pwm.cmp[ch] + 1));
	x2 = x1 / timeleft;
//  printf( "x1 %f\n", x1);
	cnto = m_pwm.cmp[ch] + ((m_pwm.cnt[ch]- m_pwm.cmp[ch]) / x2);
//  printf( "cnto %d\n", cnto);
	return cnto;
}

READ32_MEMBER( s3c44b0_device::pwm_r )
{
	UINT32 data = ((UINT32*)&m_pwm.regs)[offset];
	switch (offset)
	{
		case S3C44B0_TCNTO0 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(0);
		}
		break;
		case S3C44B0_TCNTO1 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(1);
		}
		break;
		case S3C44B0_TCNTO2 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(2);
		}
		break;
		case S3C44B0_TCNTO3 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(3);
		}
		break;
		case S3C44B0_TCNTO4 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(4);
		}
		break;
		case S3C44B0_TCNTO5 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(5);
		}
		break;
	}
	verboselog(machine(), 9, "(PWM) %08X -> %08X\n", S3C44B0_BASE_PWM + (offset << 2), data);
	return data;
}

void s3c44b0_device::pwm_start(int timer)
{
	const int mux_table[] = { 2, 4, 8, 16};
	const int prescaler_shift[] = { 0, 0, 8, 8, 16, 16};
	const int mux_shift[] = { 0, 4, 8, 12, 16, 20};
	UINT32 mclk, prescaler, mux, cnt, cmp, auto_reload;
	double freq, hz;
	verboselog(machine(), 1, "PWM %d start\n", timer);
	mclk = get_mclk();
	prescaler = (m_pwm.regs.tcfg0 >> prescaler_shift[timer]) & 0xFF;
	mux = (m_pwm.regs.tcfg1 >> mux_shift[timer]) & 0x0F;
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
			cnt = BITS(m_pwm.regs.tcntb0, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb0, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 3);
		}
		break;
		case 1 :
		{
			cnt = BITS(m_pwm.regs.tcntb1, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb1, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 11);
		}
		break;
		case 2 :
		{
			cnt = BITS(m_pwm.regs.tcntb2, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb2, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 15);
		}
		break;
		case 3 :
		{
			cnt = BITS(m_pwm.regs.tcntb3, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb3, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 19);
		}
		break;
		case 4 :
		{
			cnt = BITS(m_pwm.regs.tcntb4, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb4, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 23);
		}
		break;
		case 5 :
		{
			cnt = BITS(m_pwm.regs.tcntb5, 15, 0);
			cmp = 0;
			auto_reload = BIT(m_pwm.regs.tcon, 26);
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
	verboselog(machine(), 5, "PWM %d - mclk=%d prescaler=%d div=%d freq=%f cnt=%d cmp=%d auto_reload=%d hz=%f\n", timer, mclk, prescaler, mux_table[mux], freq, cnt, cmp, auto_reload, hz);
	m_pwm.cnt[timer] = cnt;
	m_pwm.cmp[timer] = cmp;
	m_pwm.freq[timer] = freq;
	if (cnt == 0)
	{
		m_pwm.timer[timer]->adjust(attotime::never, 0);
	}
	else
	{
		if (auto_reload)
		{
			m_pwm.timer[timer]->adjust(attotime::from_hz(hz), timer, attotime::from_hz(hz));
		}
		else
		{
			m_pwm.timer[timer]->adjust(attotime::from_hz(hz), timer);
		}
	}
}

void s3c44b0_device::pwm_stop(int timer)
{
	verboselog(machine(), 1, "PWM %d stop\n", timer);
	m_pwm.timer[timer]->adjust(attotime::never, 0);
}

void s3c44b0_device::pwm_recalc(int timer)
{
	const int tcon_shift[] = { 0, 8, 12, 16, 20, 24};
	if (m_pwm.regs.tcon & (1 << tcon_shift[timer]))
		pwm_start(timer);
	else
		pwm_stop(timer);
}

WRITE32_MEMBER( s3c44b0_device::pwm_w )
{
	UINT32 old_value = ((UINT32*)&m_pwm.regs)[offset];
	verboselog(machine(), 9, "(PWM) %08X <- %08X\n", S3C44B0_BASE_PWM + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_pwm.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_TCON :
		{
			if ((data & (1 << 0)) != (old_value & (1 << 0)))
			{
				pwm_recalc(0);
			}
			if ((data & (1 << 8)) != (old_value & (1 << 8)))
			{
				pwm_recalc(1);
			}
			if ((data & (1 << 12)) != (old_value & (1 << 12)))
			{
				pwm_recalc(2);
			}
			if ((data & (1 << 16)) != (old_value & (1 << 16)))
			{
				pwm_recalc(3);
			}
			if ((data & (1 << 20)) != (old_value & (1 << 20)))
			{
				pwm_recalc(4);
			}
			if ((data & (1 << 24)) != (old_value & (1 << 24)))
			{
				pwm_recalc(5);
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::pwm_timer_exp )
{
	int ch = param;
	const int ch_int[] = { S3C44B0_INT_TIMER0, S3C44B0_INT_TIMER1, S3C44B0_INT_TIMER2, S3C44B0_INT_TIMER3, S3C44B0_INT_TIMER4, S3C44B0_INT_TIMER5 };
	verboselog(machine(), 2, "PWM %d timer callback\n", ch);
	if (BITS(m_pwm.regs.tcfg1, 27, 24) == (ch + 1))
	{
		fatalerror("s3c44b0_dma_request_pwm( device)\n");
	}
	else
	{
		request_irq(ch_int[ch]);
	}
}

/* IIC */

inline void s3c44b0_device::iface_i2c_scl_w(int state)
{
	if (!m_scl_w_cb.isnull())
		(m_scl_w_cb)( state);
}

inline void s3c44b0_device::iface_i2c_sda_w(int state)
{
	if (!m_sda_w_cb.isnull())
		(m_sda_w_cb)( state);
}

inline int s3c44b0_device::iface_i2c_sda_r()
{
	if (!m_sda_r_cb.isnull())
		return (m_sda_r_cb)();
	else
		return 0;
}

void s3c44b0_device::i2c_send_start()
{
	verboselog(machine(), 5, "i2c_send_start\n");
	iface_i2c_sda_w(1);
	iface_i2c_scl_w(1);
	iface_i2c_sda_w(0);
	iface_i2c_scl_w(0);
}

void s3c44b0_device::i2c_send_stop()
{
	verboselog(machine(), 5, "i2c_send_stop\n");
	iface_i2c_sda_w(0);
	iface_i2c_scl_w(1);
	iface_i2c_sda_w(1);
	iface_i2c_scl_w(0);
}

UINT8 s3c44b0_device::i2c_receive_byte(int ack)
{
	UINT8 data = 0;
	verboselog(machine(), 5, "i2c_receive_byte ...\n");
	iface_i2c_sda_w(1);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_scl_w(1);
		data = (data << 1) + (iface_i2c_sda_r() ? 1 : 0);
		iface_i2c_scl_w(0);
	}
	verboselog(machine(), 5, "recv data %02X\n", data);
	verboselog(machine(), 5, "send ack %d\n", ack);
	iface_i2c_sda_w(ack ? 0 : 1);
	iface_i2c_scl_w(1);
	iface_i2c_scl_w(0);
	return data;
}

int s3c44b0_device::i2c_send_byte(UINT8 data)
{
	int ack;
	verboselog(machine(), 5, "i2c_send_byte ...\n");
	verboselog(machine(), 5, "send data %02X\n", data);
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
	verboselog(machine(), 5, "recv ack %d\n", ack);
	iface_i2c_scl_w(0);
	return ack;
}

void s3c44b0_device::iic_start()
{
	int mode_selection;
	verboselog(machine(), 1, "IIC start\n");
	i2c_send_start();
	mode_selection = BITS(m_iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : i2c_send_byte(m_iic.regs.iicds | 0x01); break;
		case 3 : i2c_send_byte(m_iic.regs.iicds & 0xFE); break;
	}
	m_iic.timer->adjust(attotime::from_usec( 1), 0);
}

void s3c44b0_device::iic_stop()
{
	verboselog(machine(), 1, "IIC stop\n");
	i2c_send_stop();
	m_iic.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::iic_resume()
{
	int mode_selection;
	verboselog(machine(), 1, "IIC resume\n");
	mode_selection = BITS(m_iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : m_iic.regs.iicds = i2c_receive_byte(BIT(m_iic.regs.iiccon, 7)); break;
		case 3 : i2c_send_byte(m_iic.regs.iicds & 0xFF); break;
	}
	m_iic.timer->adjust(attotime::from_usec( 1), 0);
}

READ32_MEMBER( s3c44b0_device::iic_r )
{
	UINT32 data = ((UINT32*)&m_iic.regs)[offset];
	switch (offset)
	{
		case S3C44B0_IICSTAT :
		{
			data = data & ~0x0000000F;
		}
		break;
	}
	verboselog(machine(), 9, "(IIC) %08X -> %08X\n", S3C44B0_BASE_IIC + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::iic_w )
{
	UINT32 old_value = ((UINT32*)&m_iic.regs)[offset];
	verboselog(machine(), 9, "(IIC) %08X <- %08X\n", S3C44B0_BASE_IIC + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_iic.regs)[offset]);
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
			clock = (double)get_pclk() / div_table[tx_clock_source_selection] / (transmit_clock_value + 1);
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
		case  S3C44B0_IICSTAT :
		{
			int interrupt_pending_flag;
			m_iic.count = 0;
			interrupt_pending_flag = BIT(m_iic.regs.iiccon, 4);
			if (interrupt_pending_flag == 0)
			{
				int start_stop_condition;
				start_stop_condition = BIT(data, 5);
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

TIMER_CALLBACK_MEMBER( s3c44b0_device::iic_timer_exp )
{
	int enable_interrupt;
	verboselog(machine(), 2, "IIC timer callback\n");
	m_iic.count++;
	enable_interrupt = BIT(m_iic.regs.iiccon, 5);

	m_iic.regs.iicds = 0xFF; // TEST

	if (enable_interrupt)
	{
		m_iic.regs.iiccon |= (1 << 4); // [bit 4] interrupt is pending
		request_irq(S3C44B0_INT_IIC);
	}
}

/* I/O Port */

inline UINT32 s3c44b0_device::iface_gpio_port_r(int port)
{
	if (!m_port_r_cb.isnull())
		return (m_port_r_cb)(port);
	else
		return 0;
}

inline void s3c44b0_device::iface_gpio_port_w(int port, UINT32 data)
{
	if (!m_port_w_cb.isnull())
		(m_port_w_cb)(port, data, 0xffff);
}

READ32_MEMBER( s3c44b0_device::gpio_r )
{
	UINT32 data = ((UINT32*)&m_gpio.regs)[offset];
	switch (offset)
	{
		case S3C44B0_GPADAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_A) & S3C44B0_GPADAT_MASK;
		}
		break;
		case S3C44B0_GPBDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_B) & S3C44B0_GPBDAT_MASK;
		}
		break;
		case S3C44B0_GPCDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_C) & S3C44B0_GPCDAT_MASK;
		}
		break;
		case S3C44B0_GPDDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_D) & S3C44B0_GPDDAT_MASK;
		}
		break;
		case S3C44B0_GPEDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_E) & S3C44B0_GPEDAT_MASK;
		}
		break;
		case S3C44B0_GPFDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_F) & S3C44B0_GPFDAT_MASK;
		}
		break;
		case S3C44B0_GPGDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_G) & S3C44B0_GPGDAT_MASK;
		}
		break;
	}
	verboselog(machine(), 9, "(GPIO) %08X -> %08X\n", S3C44B0_BASE_GPIO + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::gpio_w )
{
	UINT32 old_value = ((UINT32*)&m_gpio.regs)[offset];
	verboselog(machine(), 9, "(GPIO) %08X <- %08X\n", S3C44B0_BASE_GPIO + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_gpio.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_GPADAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_A, data & S3C44B0_GPADAT_MASK);
		}
		break;
		case S3C44B0_GPBDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_B, data & S3C44B0_GPBDAT_MASK);
		}
		break;
		case S3C44B0_GPCDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_C, data & S3C44B0_GPCDAT_MASK);
		}
		break;
		case S3C44B0_GPDDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_D, data & S3C44B0_GPDDAT_MASK);
		}
		break;
		case S3C44B0_GPEDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_E, data & S3C44B0_GPEDAT_MASK);
		}
		break;
		case S3C44B0_GPFDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_F, data & S3C44B0_GPFDAT_MASK);
		}
		break;
		case S3C44B0_GPGDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_G, data & S3C44B0_GPGDAT_MASK);
		}
		break;
		case S3C44B0_EXTINTPND :
		{
			m_gpio.regs.extintpnd = (old_value & ~data);
			check_pending_eint();
		}
		break;
	}
}

/* UART */

UINT32 s3c44b0_device::uart_r(int ch, UINT32 offset)
{
	UINT32 data = ((UINT32*)&m_uart[ch].regs)[offset];
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
			verboselog(machine(), 5, "UART %d read %02X (%c)\n", ch, rxdata, ((rxdata >= 32) && (rxdata < 128)) ? (char)rxdata : '?');
			m_uart[ch].regs.utrstat &= ~1; // [bit 0] Receive buffer data ready
		}
		break;
	}
	return data;
}

void s3c44b0_device::uart_w(int ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&((UINT32*)&m_uart[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_UTXH :
		{
			UINT8 txdata = data & 0xFF;
			verboselog(machine(), 5, "UART %d write %02X (%c)\n", ch, txdata, ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#ifdef UART_PRINTF
			printf( "%c", ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#endif
		}
		break;
		case S3C44B0_UBRDIV :
		{
			UINT32 mclk, hz;
			mclk = get_mclk();
			hz = (mclk / (m_uart->regs.ubrdiv + 1)) / 16;
			verboselog(machine(), 5, "UART %d - mclk %08X hz %08X\n", ch, mclk, hz);
			m_uart->timer->adjust(attotime::from_hz(hz), ch, attotime::from_hz(hz));
		}
		break;
	}
}

READ32_MEMBER( s3c44b0_device::uart_0_r )
{
	UINT32 data = uart_r(0, offset);
//  verboselog(machine(), 9, "(UART 0) %08X -> %08X\n", S3C44B0_BASE_UART_0 + (offset << 2), data);
	return data;
}

READ32_MEMBER( s3c44b0_device::uart_1_r )
{
	UINT32 data = uart_r(1, offset);
//  verboselog(machine(), 9, "(UART 1) %08X -> %08X\n", S3C44B0_BASE_UART_1 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::uart_0_w )
{
	verboselog(machine(), 9, "(UART 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_UART_0 + (offset << 2), data, mem_mask);
	uart_w(0, offset, data, mem_mask);
}

WRITE32_MEMBER( s3c44b0_device::uart_1_w )
{
	verboselog(machine(), 9, "(UART 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_UART_1 + (offset << 2), data, mem_mask);
	uart_w(1, offset, data, mem_mask);
}

void s3c44b0_device::uart_fifo_w(int uart, UINT8 data)
{
//  printf("s3c44b0_uart_fifo_w (%c)\n", data);
	m_uart[uart].regs.urxh = data;
	m_uart[uart].regs.utrstat |= 1; // [bit 0] Receive buffer data ready
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::uart_timer_exp )
{
	int ch = param;
	verboselog(machine(), 2, "UART %d timer callback\n", ch);
	if ((m_uart->regs.ucon & (1 << 9)) != 0)
	{
		const int ch_int[] = { S3C44B0_INT_UTXD0, S3C44B0_INT_UTXD1 };
		request_irq(ch_int[ch]);
	}
}

/* Watchdog Timer */

UINT16 s3c44b0_device::wdt_calc_current_count()
{
	return 0;
}

READ32_MEMBER( s3c44b0_device::wdt_r )
{
	UINT32 data = ((UINT32*)&m_wdt.regs)[offset];
	switch (offset)
	{
		case S3C44B0_WTCNT :
		{
			// is wdt active?
			if ((m_wdt.regs.wtcon & (1 << 5)) != 0)
			{
				data = wdt_calc_current_count();
			}
		}
		break;
	}
	verboselog(machine(), 9, "(WDT) %08X -> %08X\n", S3C44B0_BASE_WDT + (offset << 2), data);
	return data;
}

void s3c44b0_device::wdt_start()
{
	UINT32 mclk, prescaler, clock;
	double freq, hz;
	verboselog(machine(), 1, "WDT start\n");
	mclk = get_mclk();
	prescaler = BITS(m_wdt.regs.wtcon, 15, 8);
	clock = 16 << BITS(m_wdt.regs.wtcon, 4, 3);
	freq = (double)mclk / (prescaler + 1) / clock;
	hz = freq / m_wdt.regs.wtcnt;
	verboselog(machine(), 5, "WDT mclk %d prescaler %d clock %d freq %f hz %f\n", mclk, prescaler, clock, freq, hz);
	m_wdt.timer->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
}

void s3c44b0_device::wdt_stop()
{
	verboselog(machine(), 1, "WDT stop\n");
	m_wdt.regs.wtcnt = wdt_calc_current_count();
	m_wdt.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::wdt_recalc()
{
	if ((m_wdt.regs.wtcon & (1 << 5)) != 0)
		wdt_start();
	else
		wdt_stop();
}

WRITE32_MEMBER( s3c44b0_device::wdt_w )
{
	UINT32 old_value = ((UINT32*)&m_wdt.regs)[offset];
	verboselog(machine(), 9, "(WDT) %08X <- %08X\n", S3C44B0_BASE_WDT + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_wdt.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_WTCON :
		{
			if ((data & (1 << 5)) != (old_value & (1 << 5)))
			{
				wdt_recalc();
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::wdt_timer_exp )
{
	verboselog(machine(), 2, "WDT timer callback\n");
	if ((m_wdt.regs.wtcon & (1 << 2)) != 0)
	{
		request_irq(S3C44B0_INT_WDT);
	}
	if ((m_wdt.regs.wtcon & (1 << 0)) != 0)
	{
		//s3c44b0_reset();
		fatalerror("s3c44b0_reset\n");
	}
}

/* CPU Wrapper */

READ32_MEMBER( s3c44b0_device::cpuwrap_r )
{
	UINT32 data = ((UINT32*)&m_cpuwrap.regs)[offset];
	verboselog(machine(), 9, "(CPUWRAP) %08X -> %08X\n", S3C44B0_BASE_CPU_WRAPPER + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::cpuwrap_w )
{
	verboselog(machine(), 9, "(CPUWRAP) %08X <- %08X\n", S3C44B0_BASE_CPU_WRAPPER + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_cpuwrap.regs)[offset]);
}

/* A/D Converter */

READ32_MEMBER( s3c44b0_device::adc_r )
{
	UINT32 data = ((UINT32*)&m_adc.regs)[offset];
	verboselog(machine(), 9, "(ADC) %08X -> %08X\n", S3C44B0_BASE_ADC + (offset << 2), data);
	return data;
}

void s3c44b0_device::adc_start()
{
	UINT32 mclk, prescaler;
	double freq, hz;
	verboselog(machine(), 1, "ADC start\n");
	mclk = get_mclk();
	prescaler = BITS(m_adc.regs.adcpsr, 7, 0);
	freq = (double)mclk / (2 * (prescaler + 1)) / 16;
	hz = freq / 1; //m_wdt.regs.wtcnt;
	verboselog(machine(), 5, "ADC mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	m_adc.timer->adjust(attotime::from_hz(hz), 0);
}

void s3c44b0_device::adc_stop()
{
	verboselog(machine(), 1, "ADC stop\n");
	m_adc.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::adc_recalc()
{
	if ((m_adc.regs.adccon & (1 << 0)) != 0)
		adc_start();
	else
		adc_stop();
}

WRITE32_MEMBER( s3c44b0_device::adc_w )
{
	UINT32 old_value = ((UINT32*)&m_wdt.regs)[offset];
	verboselog(machine(), 9, "(ADC) %08X <- %08X\n", S3C44B0_BASE_ADC + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_adc.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_ADCCON :
		{
			if ((data & (1 << 0)) != (old_value & (1 << 0)))
			{
				adc_recalc();
			}
			m_adc.regs.adccon &= ~(1 << 0); // "this bit is cleared after the start-up"
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::adc_timer_exp )
{
	verboselog(machine(), 2, "ADC timer callback\n");
	m_adc.regs.adccon |= (1 << 6);
	request_irq(S3C44B0_INT_ADC);
}

/* SIO */

READ32_MEMBER( s3c44b0_device::sio_r )
{
	UINT32 data = ((UINT32*)&m_sio.regs)[offset];
	verboselog(machine(), 9, "(SIO) %08X -> %08X\n", S3C44B0_BASE_SIO + (offset << 2), data);
	return data;
}

void s3c44b0_device::sio_start()
{
	UINT32 mclk, prescaler;
	double freq, hz;
	verboselog(machine(), 1, "SIO start\n");
	mclk = get_mclk();
	prescaler = BITS(m_sio.regs.sbrdr, 11, 0);
	freq = (double)mclk / 2 / (prescaler + 1);
	hz = freq / 1; //m_wdt.regs.wtcnt;
	verboselog(machine(), 5, "SIO mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	m_sio.timer->adjust(attotime::from_hz(hz), 0);
//  printf("SIO transmit %02X (%c)\n", m_sio.regs.siodat, ((m_sio.regs.siodat >= 32) && (m_sio.regs.siodat < 128)) ? (char)m_sio.regs.siodat : '?');
}

void s3c44b0_device::sio_stop()
{
	verboselog(machine(), 1, "SIO stop\n");
//  m_wdt.regs.wtcnt = s3c44b0_wdt_calc_current_count( device);
	m_sio.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::sio_recalc()
{
	if ((m_sio.regs.siocon & (1 << 3)) != 0)
		sio_start();
	else
		sio_stop();
}

WRITE32_MEMBER( s3c44b0_device::sio_w )
{
	UINT32 old_value = ((UINT32*)&m_sio.regs)[offset];
	verboselog(machine(), 9, "(SIO) %08X <- %08X\n", S3C44B0_BASE_SIO + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_sio.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_SIOCON :
		{
			if ((old_value & (1 << 3)) != (data & (1 << 3)))
			{
				sio_recalc();
			}
			m_sio.regs.siocon &= ~(1 << 3); // "This bit is cleared just after writing this bit as 1."
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::sio_timer_exp )
{
	verboselog(machine(), 2, "SIO timer callback\n");

	m_sio.regs.siodat = 0x00; // TEST

	if ((m_sio.regs.siocon & (1 << 0)) != 0)
	{
		request_irq(S3C44B0_INT_SIO);
	}
}

/* IIS */

inline void s3c44b0_device::iface_i2s_data_w(address_space &space, int ch, UINT16 data)
{
	if (!m_data_w_cb.isnull())
		(m_data_w_cb)(ch, data, 0);
}

void s3c44b0_device::iis_start()
{
	UINT32 mclk;
	int prescaler;
	double freq, hz;
	const int div[] = { 2, 4, 6, 8, 10, 12, 14, 16, 1, 0, 3, 0, 5, 0, 7, 0 };
	verboselog(machine(), 1, "IIS start\n");
	mclk = get_mclk();
	prescaler = BITS(m_iis.regs.iispsr, 3, 0);
	freq = (double)mclk / div[prescaler];
	hz = freq / 256 * 2;
	verboselog(machine(), 5, "IIS mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	m_iis.timer->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
}

void s3c44b0_device::iis_stop()
{
	verboselog(machine(), 1, "IIS stop\n");
	m_iis.timer->adjust(attotime::never, 0);
}

READ32_MEMBER( s3c44b0_device::iis_r )
{
	UINT32 data = ((UINT32*)&m_iis.regs)[offset];
	verboselog(machine(), 9, "(IIS) %08X -> %08X\n", S3C44B0_BASE_IIS + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::iis_w )
{
	UINT32 old_value = ((UINT32*)&m_iis.regs)[offset];
	verboselog(machine(), 9, "(IIS) %08X <- %08X\n", S3C44B0_BASE_IIS + (offset << 2), data);
	COMBINE_DATA(&((UINT32*)&m_iis.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_IISCON :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				if ((data & (1 << 0)) != 0)
				{
					iis_start();
				}
				else
				{
					iis_stop();
				}
			}
		}
		break;
		case S3C44B0_IISFIFO :
		{
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
				iface_i2s_data_w(space, 0, m_iis.fifo[0]);
				iface_i2s_data_w(space, 1, m_iis.fifo[1]);
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::iis_timer_exp )
{
	verboselog(machine(), 2, "IIS timer callback\n");
	if ((m_iis.regs.iiscon & (1 << 5)) != 0)
	{
		bdma_request_iis();
	}
}

/* ZDMA */

void s3c44b0_device::zdma_trigger(int ch)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	UINT32 saddr, daddr;
	int dal, dst, opt, das, cnt;
	verboselog(machine(), 5, "s3c44b0_zdma_trigger %d\n", ch);
	dst = BITS(m_zdma->regs.dcsrc, 31, 30);
	dal = BITS(m_zdma->regs.dcsrc, 29, 28);
	saddr = BITS(m_zdma->regs.dcsrc, 27, 0);
	verboselog(machine(), 5, "dst %d dal %d saddr %08X\n", dst, dal, saddr);
	opt = BITS(m_zdma->regs.dcdst, 31, 30);
	das = BITS(m_zdma->regs.dcdst, 29, 28);
	daddr = BITS(m_zdma->regs.dcdst, 27, 0);
	verboselog(machine(), 5, "opt %d das %d daddr %08X\n", opt, das, daddr);
	cnt = BITS(m_zdma->regs.dccnt, 19, 0);
	verboselog(machine(), 5, "icnt %08X\n", cnt);
	while (cnt > 0)
	{
		verboselog(machine(), 9, "[%08X] -> [%08X]\n", saddr, daddr);
		switch (dst)
		{
			case 0 : space.write_byte(daddr, space.read_byte(saddr)); break;
			case 1 : space.write_word(daddr, space.read_word(saddr)); break;
			case 2 : space.write_dword(daddr, space.read_dword(saddr)); break;
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
	m_zdma->regs.dcsrc = CLR_BITS(m_zdma->regs.dcsrc, 27, 0) | saddr;
	m_zdma->regs.dcdst = CLR_BITS(m_zdma->regs.dcdst, 27, 0) | daddr;
	m_zdma->regs.dccnt = CLR_BITS(m_zdma->regs.dcdst, 19, 0) | cnt;
	if (cnt == 0)
	{
		if ((m_zdma->regs.dccnt & (1 << 23)) != 0)
		{
			const int ch_int[] = { S3C44B0_INT_ZDMA0, S3C44B0_INT_ZDMA1 };
			request_irq(ch_int[ch]);
		}
	}
}

void s3c44b0_device::zdma_start(int ch)
{
	verboselog(machine(), 5, "ZDMA %d start\n", ch);
	m_zdma->regs.dcsrc = m_zdma->regs.disrc;
	m_zdma->regs.dcdst = m_zdma->regs.didst;
	m_zdma->regs.dccnt = m_zdma->regs.dicnt;
	zdma_trigger(ch);
}

UINT32 s3c44b0_device::zdma_r(int ch, UINT32 offset)
{
	UINT32 data = ((UINT32*)&m_zdma[ch].regs)[offset];
	return data;
}

void s3c44b0_device::zdma_w(int ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	UINT32 old_value = ((UINT32*)&m_zdma[ch].regs)[offset];
	COMBINE_DATA(&((UINT32*)&m_zdma[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_DCON :
		{
			if ((old_value & 3) != (data & 3))
			{
				switch (data & 3)
				{
					case 1 : zdma_start(ch); break;
				}
			}
			m_zdma[ch].regs.dcon &= ~3; // "After writing 01,10,11, CMD bit is cleared automatically"
		}
		break;
	}
}

READ32_MEMBER( s3c44b0_device::zdma_0_r )
{
	UINT32 data = zdma_r(0, offset);
	verboselog(machine(), 9, "(ZDMA 0) %08X -> %08X\n", S3C44B0_BASE_ZDMA_0 + (offset << 2), data);
	return data;
}

READ32_MEMBER( s3c44b0_device::zdma_1_r )
{
	UINT32 data = zdma_r(1, offset);
	verboselog(machine(), 9, "(ZDMA 1) %08X -> %08X\n", S3C44B0_BASE_ZDMA_1 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::zdma_0_w )
{
	verboselog(machine(), 9, "(ZDMA 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_ZDMA_0 + (offset << 2), data, mem_mask);
	zdma_w(0, offset, data, mem_mask);
}

WRITE32_MEMBER( s3c44b0_device::zdma_1_w )
{
	verboselog(machine(), 9, "(ZDMA 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_ZDMA_1 + (offset << 2), data, mem_mask);
	zdma_w(1, offset, data, mem_mask);
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::zdma_timer_exp )
{
	int ch = param;
	verboselog(machine(), 2, "ZDMA %d timer callback\n", ch);
}

/* BDMA */

void s3c44b0_device::bdma_trigger(int ch)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	UINT32 saddr, daddr;
	int dal, dst, tdm, das, cnt;
	verboselog(machine(), 5, "s3c44b0_bdma_trigger %d\n", ch);
	dst = BITS(m_bdma->regs.dcsrc, 31, 30);
	dal = BITS(m_bdma->regs.dcsrc, 29, 28);
	saddr = BITS(m_bdma->regs.dcsrc, 27, 0);
	verboselog(machine(), 5, "dst %d dal %d saddr %08X\n", dst, dal, saddr);
	tdm = BITS(m_bdma->regs.dcdst, 31, 30);
	das = BITS(m_bdma->regs.dcdst, 29, 28);
	daddr = BITS(m_bdma->regs.dcdst, 27, 0);
	verboselog(machine(), 5, "tdm %d das %d daddr %08X\n", tdm, das, daddr);
	cnt = BITS(m_bdma->regs.dccnt, 19, 0);
	verboselog(machine(), 5, "icnt %08X\n", cnt);
	verboselog(machine(), 9, "[%08X] -> [%08X]\n", saddr, daddr);
	switch (dst)
	{
		case 0 : space.write_byte(daddr, space.read_byte(saddr)); break;
		case 1 : space.write_word(daddr, space.read_word(saddr)); break;
		case 2 : space.write_dword(daddr, space.read_dword(saddr)); break;
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
	m_bdma->regs.dcsrc = CLR_BITS(m_bdma->regs.dcsrc, 27, 0) | saddr;
	m_bdma->regs.dcdst = CLR_BITS(m_bdma->regs.dcdst, 27, 0) | daddr;
	m_bdma->regs.dccnt = CLR_BITS(m_bdma->regs.dcdst, 19, 0) | cnt;
	if (cnt == 0)
	{
		if ((m_bdma->regs.dccnt & (1 << 23)) != 0)
		{
			const int ch_int[] = { S3C44B0_INT_BDMA0, S3C44B0_INT_BDMA1 };
			request_irq(ch_int[ch]);
		}
	}
}

void s3c44b0_device::bdma_request_iis()
{
	verboselog(machine(), 5, "s3c44b0_bdma_request_iis\n");
	bdma_trigger(0);
}

UINT32 s3c44b0_device::bdma_r(int ch, UINT32 offset)
{
	UINT32 data = ((UINT32*)&m_bdma[ch].regs)[offset];
	return data;
}

void s3c44b0_device::bdma_start(int ch)
{
	verboselog(machine(), 5, "BDMA %d start\n", ch);
	int qsc = BITS(m_bdma->regs.dicnt, 31, 30);
	if ((ch == 0) && (qsc == 1))
	{
		// IIS
	}
	else
	{
		printf( "s3c44b0_bdma_start - todo\n");
	}
	m_bdma->regs.dcsrc = m_bdma->regs.disrc;
	m_bdma->regs.dcdst = m_bdma->regs.didst;
	m_bdma->regs.dccnt = m_bdma->regs.dicnt;
}

void s3c44b0_device::bdma_stop(int ch)
{
	verboselog(machine(), 5, "BDMA %d stop\n", ch);
	m_bdma[ch].timer->adjust(attotime::never, ch);
}

void s3c44b0_device::bdma_w(int ch, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	UINT32 old_value = ((UINT32*)&m_bdma[ch].regs)[offset];
	COMBINE_DATA(&((UINT32*)&m_bdma[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_DICNT :
		{
			if ((old_value & (1 << 20)) != (data & (1 << 20)))
			{
				if ((data & (1 << 20)) != 0)
				{
					bdma_start(ch);
				}
				else
				{
					bdma_stop(ch);
				}
			}
		}
		break;
	}
}

READ32_MEMBER( s3c44b0_device::bdma_0_r )
{
	UINT32 data = bdma_r(0, offset);
	verboselog(machine(), 9, "(BDMA 0) %08X -> %08X\n", S3C44B0_BASE_BDMA_0 + (offset << 2), data);
	return data;
}

READ32_MEMBER( s3c44b0_device::bdma_1_r )
{
	UINT32 data = bdma_r(1, offset);
	verboselog(machine(), 9, "(BDMA 1) %08X -> %08X\n", S3C44B0_BASE_BDMA_1 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( s3c44b0_device::bdma_0_w )
{
	verboselog(machine(), 9, "(BDMA 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_BDMA_0 + (offset << 2), data, mem_mask);
	bdma_w(0, offset, data, mem_mask);
}

WRITE32_MEMBER( s3c44b0_device::bdma_1_w )
{
	verboselog(machine(), 9, "(BDMA 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_BDMA_1 + (offset << 2), data, mem_mask);
	bdma_w(1, offset, data, mem_mask);
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::bdma_timer_exp )
{
	int ch = param;
	verboselog(machine(), 2, "BDMA %d timer callback\n", ch);
}
