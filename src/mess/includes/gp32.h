#ifndef _GP32_H_
#define _GP32_H_

#define INT_ADC       31
#define INT_RTC       30
#define INT_UTXD1     29
#define INT_UTXD0     28
#define INT_IIC       27
#define INT_USBH      26
#define INT_USBD      25
#define INT_URXD1     24
#define INT_URXD0     23
#define INT_SPI       22
#define INT_MMC       21
#define INT_DMA3      20
#define INT_DMA2      19
#define INT_DMA1      18
#define INT_DMA0      17
#define INT_RESERVED  16
#define INT_UERR      15
#define INT_TIMER4    14
#define INT_TIMER3    13
#define INT_TIMER2    12
#define INT_TIMER1    11
#define INT_TIMER0    10
#define INT_WDT        9
#define INT_TICK       8
#define INT_EINT7      7
#define INT_EINT6      6
#define INT_EINT5      5
#define INT_EINT4      4
#define INT_EINT3      3
#define INT_EINT2      2
#define INT_EINT1      1
#define INT_EINT0      0

struct s3c240x_lcd_t 
{
	UINT32 vramaddr_cur;
	UINT32 vramaddr_max;
	UINT32 offsize;
	UINT32 pagewidth_cur;
	UINT32 pagewidth_max;
	UINT32 bppmode;
	UINT32 bswp, hwswp;
	UINT32 hozval, lineval;
	int vpos, hpos;
};

struct smc_t 
{
	int add_latch;
	int chip;
	int cmd_latch;
	int do_read;
	int do_write;
	int read;
	int wp;
	int busy;
	UINT8 datarx;
	UINT8 datatx;
};

struct i2s_t 
{
	int l3d;
	int l3m;
	int l3c;
};

struct s3c240x_iic_t 
{
	UINT8 data[4];
	int data_index;
	UINT16 address;
};

struct s3c240x_iis_t 
{
	UINT16 fifo[16/2];
	int fifo_index;
};


class gp32_state : public driver_device
{
public:
	gp32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_s3c240x_ram(*this, "s3c240x_ram"){ }

	virtual void video_start();

	required_shared_ptr<UINT32> m_s3c240x_ram;
	UINT8 *m_eeprom_data;
	UINT32 m_s3c240x_lcd_regs[0x400/4];
	emu_timer *m_s3c240x_lcd_timer;
	s3c240x_lcd_t m_s3c240x_lcd;
	UINT32 m_s3c240x_lcd_palette[0x400/4];
	UINT32 m_s3c240x_clkpow_regs[0x18/4];
	UINT32 m_s3c240x_irq_regs[0x18/4];
	emu_timer *m_s3c240x_pwm_timer[5];
	UINT32 m_s3c240x_pwm_regs[0x44/4];
	emu_timer *m_s3c240x_dma_timer[4];
	UINT32 m_s3c240x_dma_regs[0x7c/4];
	smc_t m_smc;
	i2s_t m_i2s;
	UINT32 m_s3c240x_gpio[0x60/4];
	UINT32 m_s3c240x_memcon_regs[0x34/4];
	UINT32 m_s3c240x_usb_host_regs[0x5C/4];
	UINT32 m_s3c240x_uart_0_regs[0x2C/4];
	UINT32 m_s3c240x_uart_1_regs[0x2C/4];
	UINT32 m_s3c240x_usb_device_regs[0xBC/4];
	UINT32 m_s3c240x_watchdog_regs[0x0C/4];
	s3c240x_iic_t m_s3c240x_iic;
	emu_timer *m_s3c240x_iic_timer;
	UINT32 m_s3c240x_iic_regs[0x10/4];
	s3c240x_iis_t m_s3c240x_iis;
	emu_timer *m_s3c240x_iis_timer;
	UINT32 m_s3c240x_iis_regs[0x14/4];
	UINT32 m_s3c240x_rtc_regs[0x4C/4];
	UINT32 m_s3c240x_adc_regs[0x08/4];
	UINT32 m_s3c240x_spi_regs[0x18/4];
	UINT32 m_s3c240x_mmc_regs[0x40/4];
	bitmap_rgb32 m_bitmap;
	DECLARE_READ32_MEMBER(s3c240x_lcd_r);
	DECLARE_WRITE32_MEMBER(s3c240x_lcd_w);
	DECLARE_READ32_MEMBER(s3c240x_lcd_palette_r);
	DECLARE_WRITE32_MEMBER(s3c240x_lcd_palette_w);
	DECLARE_READ32_MEMBER(s3c240x_clkpow_r);
	DECLARE_WRITE32_MEMBER(s3c240x_clkpow_w);
	DECLARE_READ32_MEMBER(s3c240x_irq_r);
	DECLARE_WRITE32_MEMBER(s3c240x_irq_w);
	DECLARE_READ32_MEMBER(s3c240x_pwm_r);
	DECLARE_WRITE32_MEMBER(s3c240x_pwm_w);
	DECLARE_READ32_MEMBER(s3c240x_dma_r);
	DECLARE_WRITE32_MEMBER(s3c240x_dma_w);
	DECLARE_READ32_MEMBER(s3c240x_gpio_r);
	DECLARE_WRITE32_MEMBER(s3c240x_gpio_w);
	DECLARE_READ32_MEMBER(s3c240x_memcon_r);
	DECLARE_WRITE32_MEMBER(s3c240x_memcon_w);
	DECLARE_READ32_MEMBER(s3c240x_usb_host_r);
	DECLARE_WRITE32_MEMBER(s3c240x_usb_host_w);
	DECLARE_READ32_MEMBER(s3c240x_uart_0_r);
	DECLARE_WRITE32_MEMBER(s3c240x_uart_0_w);
	DECLARE_READ32_MEMBER(s3c240x_uart_1_r);
	DECLARE_WRITE32_MEMBER(s3c240x_uart_1_w);
	DECLARE_READ32_MEMBER(s3c240x_usb_device_r);
	DECLARE_WRITE32_MEMBER(s3c240x_usb_device_w);
	DECLARE_READ32_MEMBER(s3c240x_watchdog_r);
	DECLARE_WRITE32_MEMBER(s3c240x_watchdog_w);
	DECLARE_READ32_MEMBER(s3c240x_iic_r);
	DECLARE_WRITE32_MEMBER(s3c240x_iic_w);
	DECLARE_READ32_MEMBER(s3c240x_iis_r);
	DECLARE_WRITE32_MEMBER(s3c240x_iis_w);
	DECLARE_READ32_MEMBER(s3c240x_rtc_r);
	DECLARE_WRITE32_MEMBER(s3c240x_rtc_w);
	DECLARE_READ32_MEMBER(s3c240x_adc_r);
	DECLARE_WRITE32_MEMBER(s3c240x_adc_w);
	DECLARE_READ32_MEMBER(s3c240x_spi_r);
	DECLARE_WRITE32_MEMBER(s3c240x_spi_w);
	DECLARE_READ32_MEMBER(s3c240x_mmc_r);
	DECLARE_WRITE32_MEMBER(s3c240x_mmc_w);
	virtual void machine_start();
	virtual void machine_reset();
};


#endif
