// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen

#ifndef MAME_GAMEPARK_GP32_H
#define MAME_GAMEPARK_GP32_H

#pragma once

#include "machine/nvram.h"
#include "machine/smartmed.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"

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
	uint32_t vramaddr_cur = 0;
	uint32_t vramaddr_max = 0;
	uint32_t offsize = 0;
	uint32_t pagewidth_cur = 0;
	uint32_t pagewidth_max = 0;
	uint32_t bppmode = 0;
	uint32_t bswp = 0, hwswp = 0;
	uint32_t hozval = 0, lineval = 0;
	int vpos = 0, hpos = 0;
};

struct smc_t
{
	int add_latch = 0;
	int chip = 0;
	int cmd_latch = 0;
	int do_read = 0;
	int do_write = 0;
	int read = 0;
	int wp = 0;
	int busy = 0;
	uint8_t datarx = 0;
	uint8_t datatx = 0;
};

struct i2s_t
{
	int l3d = 0;
	int l3m = 0;
	int l3c = 0;
};

struct s3c240x_iic_t
{
	uint8_t data[4]{};
	int data_index = 0;
	uint16_t address = 0;
};

struct s3c240x_iis_t
{
	uint16_t fifo[16/2]{};
	int fifo_index = 0;
};


class gp32_state : public driver_device
{
public:
	gp32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_s3c240x_ram(*this, "s3c240x_ram"),
		m_maincpu(*this, "maincpu"),
		m_smartmedia(*this, "smartmedia"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_nvram(*this, "nvram"),
		m_io_in0(*this, "IN0"),
		m_io_in1(*this, "IN1"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
		{
			std::fill(std::begin(m_s3c240x_lcd_regs), std::end(m_s3c240x_lcd_regs), 0);
			std::fill(std::begin(m_s3c240x_uart_0_regs), std::end(m_s3c240x_uart_0_regs), 0);
			std::fill(std::begin(m_s3c240x_uart_1_regs), std::end(m_s3c240x_uart_1_regs), 0);
		}

	void gp32(machine_config &config);

private:
	virtual void video_start() override ATTR_COLD;

	required_shared_ptr<uint32_t> m_s3c240x_ram;
	std::unique_ptr<uint8_t[]> m_eeprom_data;
	uint32_t m_s3c240x_lcd_regs[0x400/4];
	emu_timer *m_s3c240x_lcd_timer = nullptr;
	s3c240x_lcd_t m_s3c240x_lcd;
	uint32_t m_s3c240x_lcd_palette[0x400/4]{};
	uint32_t m_s3c240x_clkpow_regs[0x18/4]{};
	uint32_t m_s3c240x_irq_regs[0x18/4]{};
	emu_timer *m_s3c240x_pwm_timer[5]{};
	uint32_t m_s3c240x_pwm_regs[0x44/4]{};
	emu_timer *m_s3c240x_dma_timer[4]{};
	uint32_t m_s3c240x_dma_regs[0x7c/4]{};
	smc_t m_smc;
	i2s_t m_i2s;
	uint32_t m_s3c240x_gpio[0x60/4]{};
	uint32_t m_s3c240x_memcon_regs[0x34/4]{};
	uint32_t m_s3c240x_usb_host_regs[0x5C/4]{};
	uint32_t m_s3c240x_uart_0_regs[0x2C/4];
	uint32_t m_s3c240x_uart_1_regs[0x2C/4];
	uint32_t m_s3c240x_usb_device_regs[0xBC/4]{};
	uint32_t m_s3c240x_watchdog_regs[0x0C/4]{};
	s3c240x_iic_t m_s3c240x_iic;
	emu_timer *m_s3c240x_iic_timer = nullptr;
	uint32_t m_s3c240x_iic_regs[0x10/4]{};
	s3c240x_iis_t m_s3c240x_iis;
	emu_timer *m_s3c240x_iis_timer = nullptr;
	uint32_t m_s3c240x_iis_regs[0x14/4]{};
	uint32_t m_s3c240x_rtc_regs[0x4C/4]{};
	uint32_t m_s3c240x_adc_regs[0x08/4]{};
	uint32_t m_s3c240x_spi_regs[0x18/4]{};
	uint32_t m_s3c240x_mmc_regs[0x40/4]{};
	bitmap_rgb32 m_bitmap;
	uint32_t s3c240x_lcd_r(offs_t offset);
	void s3c240x_lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_lcd_palette_r(offs_t offset);
	void s3c240x_lcd_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_clkpow_r(offs_t offset);
	void s3c240x_clkpow_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_irq_r(offs_t offset);
	void s3c240x_irq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_pwm_r(offs_t offset);
	void s3c240x_pwm_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_dma_r(offs_t offset);
	void s3c240x_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_gpio_r(offs_t offset);
	void s3c240x_gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_memcon_r(offs_t offset);
	void s3c240x_memcon_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_usb_host_r(offs_t offset);
	void s3c240x_usb_host_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_uart_0_r(offs_t offset);
	void s3c240x_uart_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_uart_1_r(offs_t offset);
	void s3c240x_uart_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_usb_device_r(offs_t offset);
	void s3c240x_usb_device_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_watchdog_r(offs_t offset);
	void s3c240x_watchdog_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_iic_r(offs_t offset);
	void s3c240x_iic_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_iis_r(offs_t offset);
	void s3c240x_iis_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_rtc_r(offs_t offset);
	void s3c240x_rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_adc_r(offs_t offset);
	void s3c240x_adc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_spi_r(offs_t offset);
	void s3c240x_spi_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c240x_mmc_r(offs_t offset);
	void s3c240x_mmc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_gp32(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(s3c240x_lcd_timer_exp);
	TIMER_CALLBACK_MEMBER(s3c240x_pwm_timer_exp);
	TIMER_CALLBACK_MEMBER(s3c240x_dma_timer_exp);
	TIMER_CALLBACK_MEMBER(s3c240x_iic_timer_exp);
	TIMER_CALLBACK_MEMBER(s3c240x_iis_timer_exp);
	void gp32_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<smartmedia_image_device> m_smartmedia;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_device<nvram_device> m_nvram;
	required_ioport m_io_in0;
	required_ioport m_io_in1;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint32_t s3c240x_get_fclk(int reg);
	uint32_t s3c240x_get_hclk(int reg);
	uint32_t s3c240x_get_pclk(int reg);
	void s3c240x_lcd_dma_reload();
	void s3c240x_lcd_dma_init();
	void s3c240x_lcd_configure();
	void s3c240x_lcd_start();
	void s3c240x_lcd_stop();
	void s3c240x_lcd_recalc();
	void s3c240x_check_pending_irq();
	void s3c240x_request_irq(uint32_t int_type);
	void s3c240x_dma_reload(int dma);
	void s3c240x_dma_trigger(int dma);
	void s3c240x_dma_request_iis();
	void s3c240x_dma_request_pwm();
	void s3c240x_dma_start(int dma);
	void s3c240x_dma_stop(int dma);
	void s3c240x_dma_recalc(int dma);
	void s3c240x_pwm_start(int timer);
	void s3c240x_pwm_stop(int timer);
	void s3c240x_pwm_recalc(int timer);
	void s3c240x_iis_start();
	void s3c240x_iis_stop();
	void s3c240x_iis_recalc();
	void smc_reset();
	void smc_init();
	uint8_t smc_read();
	void smc_write(uint8_t data);
	void smc_update();
	void i2s_reset();
	void i2s_init();
	void i2s_write(int line, int data);
	uint8_t eeprom_read(uint16_t address);
	void eeprom_write(uint16_t address, uint8_t data);
	void iic_start();
	void iic_stop();
	void iic_resume();
	void s3c240x_machine_start();
	void s3c240x_machine_reset();
	inline rgb_t s3c240x_get_color_5551( uint16_t data);
	uint32_t s3c240x_lcd_dma_read( );
	void s3c240x_lcd_render_01( );
	void s3c240x_lcd_render_02( );
	void s3c240x_lcd_render_04( );
	void s3c240x_lcd_render_08( );
	void s3c240x_lcd_render_16( );
};


#endif // MAME_GAMEPARK_GP32_H
