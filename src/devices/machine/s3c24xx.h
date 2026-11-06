// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen, Ryan Holtz
/*******************************************************************************

    Samsung S3C2400/S3C2410/S3C2440

*******************************************************************************/

#ifndef MAME_MACHINE_S3C24XX_H
#define MAME_MACHINE_S3C24XX_H

#pragma once

#include "emupal.h"
#include "cpu/arm7/arm7.h"


/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

/* LCD Interface */

class s3c24xx_device : public device_t
{
public:
	// configuration
	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	auto om0_r_callback() { return m_om0_r_cb.bind(); }
	auto om1_r_callback() { return m_om1_r_cb.bind(); }
	auto ncon_r_callback() { return m_ncon_r_cb.bind(); }
	auto core_pin_w_callback() { return m_pin_w_cb.bind(); }
	auto gpio_port_r_callback() { return m_port_r_cb.bind(); }
	auto gpio_port_w_callback() { return m_port_w_cb.bind(); }
	auto i2c_scl_w_callback() { return m_scl_w_cb.bind(); }
	auto i2c_sda_r_callback() { return m_sda_r_cb.bind(); }
	auto i2c_sda_w_callback() { return m_sda_w_cb.bind(); }
	auto adc_data_r_callback() { return m_data_r_cb.bind(); }
	auto i2s_data_w_callback() { return m_iis_data_w_cb.bind(); }
	auto i2s_clockchange_w_callback() { return m_iis_clockchange_w_cb.bind(); }
	void set_lcd_flags(u32 lcd_flags) { m_lcd_flags = lcd_flags; }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	template <int Which> void uart_fifo_w(u8 data);

	enum
	{
		PIN_NCON = 0,
		PIN_OM0,
		PIN_OM1
	};

	enum
	{
		GPIO_PORT_A = 0,
		GPIO_PORT_B,
		GPIO_PORT_C,
		GPIO_PORT_D,
		GPIO_PORT_E,
		GPIO_PORT_F,
		GPIO_PORT_G,
		GPIO_PORT_H,
		GPIO_PORT_J
	};

	enum : u32
	{
		INTERFACE_LCD_REVERSE
	};

protected:
	s3c24xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void memcon_reset();
	u32 memcon_bwscon_r(offs_t offset, u32 mem_mask = ~0);
	void memcon_bwscon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 memcon_bankcon_r(offs_t offset, u32 mem_mask = ~0);
	template <int which> void memcon_bankcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 memcon_refresh_r(offs_t offset, u32 mem_mask = ~0);
	void memcon_refresh_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 memcon_banksize_r(offs_t offset, u32 mem_mask = ~0);
	void memcon_banksize_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 memcon_mrsrb6_r(offs_t offset, u32 mem_mask = ~0);
	void memcon_mrsrb6_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 memcon_mrsrb7_r(offs_t offset, u32 mem_mask = ~0);
	void memcon_mrsrb7_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void usbhost_reset();
	u32 usbhost_rev_r(offs_t offset, u32 mem_mask = ~0);
	u32 usbhost_ctrl_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_common_stat_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_common_stat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_int_stat_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_int_stat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_int_en_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_int_en_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_int_dis_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_int_dis_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_hcca_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_hcca_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_period_curr_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_period_curr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_ctrl_head_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_ctrl_head_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_ctrl_curr_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_ctrl_curr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_bulk_head_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_bulk_head_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_bulk_curr_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_bulk_curr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_done_head_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_done_head_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_rm_interval_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_rm_interval_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_fm_remaining_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_fm_remaining_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_fm_number_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_fm_number_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_periodic_start_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_periodic_start_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_ls_thresh_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_ls_thresh_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_rh_desc_a_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_rh_desc_a_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_rh_desc_b_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_rh_desc_b_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_rh_status_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_rh_status_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_rh_port_stat_1_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_rh_port_stat_1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbhost_rh_port_stat_2_r(offs_t offset, u32 mem_mask = ~0);
	void usbhost_rh_port_stat_2_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	enum : u32
	{
		INT_ADC,
		INT_RTC,
		INT_SPI,
		INT_SPI0,
		INT_SPI1,
		INT_UART0,
		INT_UART1,
		INT_UART2,
		INT_UTXD0,
		INT_UTXD1,
		INT_URXD0,
		INT_URXD1,
		INT_UERR,
		INT_IIC,
		INT_USBH,
		INT_USBD,
		INT_24,
		INT_SDI,
		INT_MMC,
		INT_DMA0,
		INT_DMA1,
		INT_DMA2,
		INT_DMA3,
		INT_LCD,
		INT_TIMER0,
		INT_TIMER1,
		INT_TIMER2,
		INT_TIMER3,
		INT_TIMER4,
		INT_WDT,
		INT_NFCON,
		INT_TICK,
		INT_BATT_FLT,
		INT_6,
		INT_CAM,
		INT_EINT0,
		INT_EINT1,
		INT_EINT2,
		INT_EINT3,
		INT_EINT4,
		INT_EINT5,
		INT_EINT6,
		INT_EINT7,
		INT_EINT4_7,
		INT_EINT8_23,

		INT_COUNT
	};

	virtual void irq_reset();
	void check_pending_irq();
	virtual u32 get_irq_bit_for_int_type(u32 int_type) const = 0;
	void request_irq(u32 int_type);
	u32 irq_srcpnd_r(offs_t offset, u32 mem_mask = ~0);
	void irq_srcpnd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irq_intmod_r(offs_t offset, u32 mem_mask = ~0);
	void irq_intmod_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irq_intmsk_r(offs_t offset, u32 mem_mask = ~0);
	void irq_intmsk_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irq_priority_r(offs_t offset, u32 mem_mask = ~0);
	void irq_priority_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irq_intpnd_r(offs_t offset, u32 mem_mask = ~0);
	void irq_intpnd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irq_intoffset_r(offs_t offset, u32 mem_mask = ~0);
	void irq_intoffset_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual u32 dma_dcon_get_tsz(const u32 value);
	virtual u32 dma_dcon_get_hwsrcsel(const u32 value);
	virtual u32 dma_dcon_get_servmode(const u32 value);
	virtual u32 dma_dcon_get_int(const u32 value);
	virtual u32 dma_daddr_get_addr(const u32 value);
	virtual void dma_dc_set_curr_addr(u32 &reg, const u32 value);
	virtual u32 dma_get_src_inc_flag(const int which);
	virtual u32 dma_get_dst_inc_flag(const int which);
	template <int Which> void dma_reload();
	template <int Which> void dma_trigger();
	void dma_request_iis();
	void dma_request_pwm();
	template <int Which> void dma_start();
	template <int Which> void dma_stop();
	template <int Which> void dma_recalc();
	TIMER_CALLBACK_MEMBER(dma_timer_exp);
	template <int Which> u32 dma_disrc_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_disrc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 dma_didst_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_didst_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 dma_dcon_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_dcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 dma_dstat_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_dstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 dma_dcsrc_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_dcsrc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 dma_dcdst_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_dcdst_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 dma_dmasktrig_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_dmasktrig_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void clkpow_reset();
	virtual u32 get_clk_mult() const;
	u64 get_full_clock() const;
	u64 get_fclk_ticks() const;
	attotime get_fclk() const;
	virtual u64 get_hclk_ticks() const;
	attotime get_hclk() const;
	u64 get_pclk_ticks() const;
	attotime get_pclk() const;
	u32 clkpow_locktime_r(offs_t offset, u32 mem_mask = ~0);
	void clkpow_locktime_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 clkpow_mpllcon_r(offs_t offset, u32 mem_mask = ~0);
	void clkpow_mpllcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 clkpow_upllcon_r(offs_t offset, u32 mem_mask = ~0);
	void clkpow_upllcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 clkpow_clkcon_r(offs_t offset, u32 mem_mask = ~0);
	void clkpow_clkcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 clkpow_clkslow_r(offs_t offset, u32 mem_mask = ~0);
	void clkpow_clkslow_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 clkpow_clkdivn_r(offs_t offset, u32 mem_mask = ~0);
	virtual void clkpow_clkdivn_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	enum : u32
	{
		BPPMODE_STN_01      = 0x00,
		BPPMODE_STN_02      = 0x01,
		BPPMODE_STN_04      = 0x02,
		BPPMODE_STN_08      = 0x03,
		BPPMODE_STN_12_P    = 0x04,
		BPPMODE_STN_12_U    = 0x05,
		BPPMODE_STN_16      = 0x06,
		BPPMODE_TFT_01      = 0x08,
		BPPMODE_TFT_02      = 0x09,
		BPPMODE_TFT_04      = 0x0a,
		BPPMODE_TFT_08      = 0x0b,
		BPPMODE_TFT_16      = 0x0c,
		BPPMODE_TFT_24      = 0x0d,

		PNRMODE_STN_04_DS   = 0,
		PNRMODE_STN_04_SS   = 1,
		PNRMODE_STN_08_SS   = 2,
		PNRMODE_TFT         = 3
	};

	virtual void lcd_reset();
	rgb_t lcd_get_color_tft_16(u16 data);
	rgb_t lcd_get_color_stn_12(u16 data);
	rgb_t lcd_get_color_stn_08(u8 data);
	rgb_t lcd_get_color_stn_01(u8 data);
	rgb_t lcd_get_color_stn_02(u8 data);
	rgb_t lcd_get_color_stn_04(u8 data);
	virtual u32 lcd_get_tpalval();
	virtual rgb_t lcd_get_color_tpal();
	virtual bool lcd_get_tpalen();
	void lcd_dma_reload();
	void lcd_dma_init();
	u32 lcd_dma_read();
	u32 lcd_dma_read_bits(int count);
	void lcd_render_tpal();
	void lcd_render_stn_01();
	void lcd_render_stn_02();
	void lcd_render_stn_04();
	void lcd_render_stn_08();
	void lcd_render_stn_12_p();
	void lcd_render_stn_12_u(); // not tested
	void lcd_render_tft_01();
	void lcd_render_tft_02();
	void lcd_render_tft_04();
	void lcd_render_tft_08();
	void lcd_render_tft_16();
	TIMER_CALLBACK_MEMBER(lcd_timer_exp);
	void video_start();
	void bitmap_blend(bitmap_rgb32 &bitmap_dst, bitmap_rgb32 &bitmap_src_1, bitmap_rgb32 &bitmap_src_2);
	void lcd_configure_tft();
	bool lcd_configure_stn();
	bool lcd_configure();
	void lcd_start();
	void lcd_stop();
	void lcd_recalc();
	u32 lcd_lcdcon1_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdcon1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdcon2_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdcon2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdcon3_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdcon3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdcon4_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdcon4_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdcon5_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdcon5_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdsaddr1_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdsaddr1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdsaddr2_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdsaddr2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdsaddr3_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdsaddr3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_redlut_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_redlut_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_greenlut_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_greenlut_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_bluelut_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_bluelut_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp12_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp12_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp47_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp47_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp35_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp35_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp23_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp23_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp57_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp57_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp34_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp34_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp45_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp45_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dp67_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dp67_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_dithmode_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_dithmode_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_tpal_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_tpal_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void uart_reset();
	virtual void uart_request_irq(int which);
	template <int Which> u32 uart_ulcon_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_ulcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_ucon_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_ucon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_ufcon_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_ufcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_umcon_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_umcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_utrstat_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_utrstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_uerstat_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_uerstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_ufstat_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_ufstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_umstat_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_umstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_utxh_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_utxh_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_urxh_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_urxh_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 uart_ubrdiv_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void uart_ubrdiv_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void pwm_reset();
	u16 pwm_calc_observation(unsigned ch) const;
	TIMER_CALLBACK_MEMBER(pwm_timer_exp);
	void pwm_start(int timer);
	void pwm_stop(int timer);
	void pwm_recalc(int timer);
	u32 pwm_tcfg0_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcfg0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcfg1_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcfg1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcon_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcntb0_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcntb0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcmpb0_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcmpb0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcnto0_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcnto0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcntb1_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcntb1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcmpb1_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcmpb1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcnto1_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcnto1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcntb2_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcntb2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcmpb2_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcmpb2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcnto2_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcnto2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcntb3_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcntb3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcmpb3_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcmpb3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcnto3_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcnto3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcntb4_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcntb4_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pwm_tcnto4_r(offs_t offset, u32 mem_mask = ~0);
	void pwm_tcnto4_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void usb_device_reset();
	u32 usbdev_func_addr_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_func_addr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_pwr_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_pwr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_int_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_int_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_int_mask_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_int_mask_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_frame_num_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_frame_num_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_resume_con_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_resume_con_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_csr_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_csr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_maxp_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_maxp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_out_cnt_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_out_cnt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_ep0_fifo_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_ep0_fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_ep1_fifo_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_ep1_fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_ep2_fifo_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_ep2_fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_ep3_fifo_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_ep3_fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_ep4_fifo_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_ep4_fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_dma_con_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_dma_con_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_dma_unit_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_dma_unit_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_dma_fifo_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_dma_fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_dma_tx_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_dma_tx_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_test_mode_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_test_mode_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_in_con_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_in_con_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void wdt_reset();
	u16 wdt_calc_current_count();
	virtual void wdt_start();
	virtual void wdt_stop();
	void wdt_recalc();
	TIMER_CALLBACK_MEMBER(wdt_timer_exp);
	virtual void request_wdt_irq();
	u32 wdt_wtcon_r(offs_t offset, u32 mem_mask = ~0);
	void wdt_wtcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 wdt_wtdat_r(offs_t offset, u32 mem_mask = ~0);
	void wdt_wtdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	virtual u32 wdt_wtcnt_r(offs_t offset, u32 mem_mask = ~0);
	void wdt_wtcnt_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void iic_reset();
	void i2c_send_start();
	void i2c_send_stop();
	u8 i2c_receive_byte(int ack);
	int i2c_send_byte(u8 data);
	void iic_start();
	void iic_stop();
	void iic_resume();
	TIMER_CALLBACK_MEMBER(iic_timer_exp);
	u32 iic_iiccon_r(offs_t offset, u32 mem_mask = ~0);
	void iic_iiccon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iic_iicstat_r(offs_t offset, u32 mem_mask = ~0);
	void iic_iicstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iic_iicadd_r(offs_t offset, u32 mem_mask = ~0);
	void iic_iicadd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iic_iicds_r(offs_t offset, u32 mem_mask = ~0);
	void iic_iicds_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void iis_reset();
	void iis_start();
	void iis_stop();
	void iis_recalc();
	TIMER_CALLBACK_MEMBER(iis_timer_exp);
	u32 iis_iiscon_r(offs_t offset, u32 mem_mask = ~0);
	void iis_iiscon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iis_iismod_r(offs_t offset, u32 mem_mask = ~0);
	void iis_iismod_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iis_iispsr_r(offs_t offset, u32 mem_mask = ~0);
	void iis_iispsr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iis_iisfcon_r(offs_t offset, u32 mem_mask = ~0);
	void iis_iisfcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iis_iisfifo_r(offs_t offset, u32 mem_mask = ~0);
	void iis_iisfifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void gpio_reset();
	virtual u32 gpio_get_data_mask(int port) = 0;
	u32 gpio_get_mask(u32 con, int val);
	u32 gpio_gpacon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpacon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpadat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpadat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpbcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpbcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpbdat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpbdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpbup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpbup_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpccon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpccon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpcdat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpcdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpcup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpcup_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpdcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpdcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpddat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpddat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpdup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpdup_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpecon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpecon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpedat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpedat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpeup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpeup_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpfcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpfcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpfdat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpfdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpfup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpfup_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpgcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpgcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpgdat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpgdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpgup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpgup_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_misccr_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_misccr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_opencr_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_opencr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 gpio_extint_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void gpio_extint_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void rtc_reset();
	TIMER_CALLBACK_MEMBER(rtc_timer_tick_count_exp);
	void rtc_update();
	void rtc_check_alarm();
	TIMER_CALLBACK_MEMBER(rtc_timer_update_exp);
	u32 rtc_rtccon_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_rtccon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_ticnt_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_ticnt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_rtcalm_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_rtcalm_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_almsec_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_almsec_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_almmin_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_almmin_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_almhour_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_almhour_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_almday_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_almday_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_almmon_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_almmon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_almyear_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_almyear_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_rtcrst_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_rtcrst_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_bcdsec_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_bcdsec_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_bcdmin_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_bcdmin_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_bcdhour_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_bcdhour_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_bcddate_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_bcddate_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_bcdday_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_bcdday_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_bcdmon_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_bcdmon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_bcdyear_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_bcdyear_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void adc_reset();
	virtual void adc_start();
	u32 adc_adccon_r(offs_t offset, u32 mem_mask = ~0);
	void adc_adccon_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 lcd_palette_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_palette_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void spi_reset();
	template <int Which> u32 spi_spcon_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void spi_spcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 spi_spsta_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void spi_spsta_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 spi_sppin_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void spi_sppin_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 spi_sppre_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void spi_sppre_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 spi_sptdat_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void spi_sptdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 spi_sprdat_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void spi_sprdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <typename A, typename B, typename C> static constexpr auto CLR_BITS(A &&x, B &&m, C &&n)
	{
		return x & ~(((u32(1) << (m - n + 1)) - 1) << n);
	}

	// internal state
	required_device<arm7_cpu_device> m_cpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	memory_access<24, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;

	devcb_read_line m_om0_r_cb;
	devcb_read_line m_om1_r_cb;
	devcb_read_line m_ncon_r_cb;
	devcb_write_line m_pin_w_cb;
	devcb_read32 m_port_r_cb;
	devcb_write32 m_port_w_cb;
	devcb_write_line m_scl_w_cb;
	devcb_read_line m_sda_r_cb;
	devcb_write_line m_sda_w_cb;
	devcb_read32 m_data_r_cb;
	devcb_write16 m_iis_data_w_cb;
	devcb_write32 m_iis_clockchange_w_cb;

	u32 m_memcon_bwscon;
	u32 m_memcon_bankcon[8];
	u32 m_memcon_refresh;
	u32 m_memcon_banksize;
	u32 m_memcon_mrsrb6;
	u32 m_memcon_mrsrb7;

	u32 m_usbhost_rev;
	u32 m_usbhost_ctrl;
	u32 m_usbhost_common_stat;
	u32 m_usbhost_int_stat;
	u32 m_usbhost_int_en;
	u32 m_usbhost_int_dis;
	u32 m_usbhost_hcca;
	u32 m_usbhost_period_curr;
	u32 m_usbhost_ctrl_head;
	u32 m_usbhost_ctrl_curr;
	u32 m_usbhost_bulk_head;
	u32 m_usbhost_bulk_curr;
	u32 m_usbhost_done_head;
	u32 m_usbhost_rm_interval;
	u32 m_usbhost_fm_remaining;
	u32 m_usbhost_fm_number;
	u32 m_usbhost_periodic_start;
	u32 m_usbhost_ls_thresh;
	u32 m_usbhost_rh_desc[2];
	u32 m_usbhost_rh_status;
	u32 m_usbhost_rh_port_status[2];

	u32 m_irq_srcpnd;
	u32 m_irq_intmod;
	u32 m_irq_intmsk;
	u32 m_irq_priority;
	u32 m_irq_intpnd;
	u32 m_irq_intoffset;
	int m_irq_line;
	int m_fiq_line;

	u32 m_dma_disrc[4];
	u32 m_dma_didst[4];
	u32 m_dma_dcon[4];
	u32 m_dma_dstat[4];
	u32 m_dma_dcsrc[4];
	u32 m_dma_dcdst[4];
	u32 m_dma_dmasktrig[4];

	u32 m_clkpow_locktime;
	u32 m_clkpow_mpllcon;
	u32 m_clkpow_upllcon;
	u32 m_clkpow_clkcon;
	u32 m_clkpow_clkslow;
	u32 m_clkpow_clkdivn;

	u32 m_lcd_lcdcon1;
	u32 m_lcd_lcdcon2;
	u32 m_lcd_lcdcon3;
	u32 m_lcd_lcdcon4;
	u32 m_lcd_lcdcon5;
	u32 m_lcd_lcdsaddr1;
	u32 m_lcd_lcdsaddr2;
	u32 m_lcd_lcdsaddr3;
	u32 m_lcd_redlut;
	u32 m_lcd_greenlut;
	u32 m_lcd_bluelut;
	u32 m_lcd_dithpat[8];
	u32 m_lcd_dithmode;
	u32 m_lcd_tpal;
	emu_timer *m_lcd_timer;
	std::unique_ptr<bitmap_rgb32> m_lcd_bitmap[2];
	u32 m_lcd_vramaddr_cur;
	u32 m_lcd_vramaddr_max;
	u32 m_lcd_offsize;
	u32 m_lcd_pagewidth_cur;
	u32 m_lcd_pagewidth_max;
	u32 m_lcd_bppmode;
	u32 m_lcd_bswp;
	u32 m_lcd_hwswp;
	int m_lcd_vpos;
	int m_lcd_hpos;
	attotime m_lcd_framerate;
	u32 m_lcd_hpos_min;
	u32 m_lcd_hpos_max;
	u32 m_lcd_vpos_min;
	u32 m_lcd_vpos_max;
	u32 m_lcd_dma_data;
	u32 m_lcd_dma_bits;
	u32 m_lcd_flags;

	u32 m_uart_ulcon[3];
	u32 m_uart_ucon[3];
	u32 m_uart_ufcon[3];
	u32 m_uart_umcon[3];
	u32 m_uart_utrstat[3];
	u32 m_uart_uerstat[3];
	u32 m_uart_ufstat[3];
	u32 m_uart_umstat[3];
	u32 m_uart_utxh[3];
	u32 m_uart_urxh[3];
	u32 m_uart_ubrdiv[3];

	u32 m_pwm_tcfg0;
	u32 m_pwm_tcfg1;
	u32 m_pwm_tcon;
	u32 m_pwm_tcntb0;
	u32 m_pwm_tcmpb0;
	u32 m_pwm_tcnto0;
	u32 m_pwm_tcntb1;
	u32 m_pwm_tcmpb1;
	u32 m_pwm_tcnto1;
	u32 m_pwm_tcntb2;
	u32 m_pwm_tcmpb2;
	u32 m_pwm_tcnto2;
	u32 m_pwm_tcntb3;
	u32 m_pwm_tcmpb3;
	u32 m_pwm_tcnto3;
	u32 m_pwm_tcntb4;
	u32 m_pwm_tcnto4;
	emu_timer *m_pwm_timer[5];
	u32 m_pwm_cnt[5];
	u32 m_pwm_cmp[5];
	u32 m_pwm_freq[5];

	u32 m_usbdev_func_addr;
	u32 m_usbdev_pwr;
	u32 m_usbdev_int;
	u32 m_usbdev_int_mask;
	u32 m_usbdev_frame_num;
	u32 m_usbdev_resume_con;
	u32 m_usbdev_ep_csr[5];
	u32 m_usbdev_ep_maxp[5];
	u32 m_usbdev_ep_out_cnt[5];
	u32 m_usbdev_ep_fifo[5];
	u32 m_usbdev_dma_con;
	u32 m_usbdev_dma_unit;
	u32 m_usbdev_dma_fifo;
	u32 m_usbdev_dma_tx;
	u32 m_usbdev_test_mode;
	u32 m_usbdev_in_con;

	u32 m_wdt_wtcon;
	u32 m_wdt_wtdat;
	u32 m_wdt_wtcnt;
	double m_wdt_freq;
	emu_timer *m_wdt_timer;
	u32 m_wdt_cnt;

	u32 m_iic_iiccon;
	u32 m_iic_iicstat;
	u32 m_iic_iicadd;
	u32 m_iic_iicds;
	emu_timer *m_iic_timer;
	int m_iic_count;

	u32 m_iis_iiscon;
	u32 m_iis_iismod;
	u32 m_iis_iispsr;
	u32 m_iis_iisfcon;
	u32 m_iis_iisfifo;
	emu_timer *m_iis_timer;
	u16 m_iis_tx_fifo[16/2];
	int m_iis_fifo_index;

	u32 m_gpio_gpacon;
	u32 m_gpio_gpbcon;
	u32 m_gpio_gpbup;
	u32 m_gpio_gpccon;
	u32 m_gpio_gpcup;
	u32 m_gpio_gpdcon;
	u32 m_gpio_gpdup;
	u32 m_gpio_gpecon;
	u32 m_gpio_gpeup;
	u32 m_gpio_gpfcon;
	u32 m_gpio_gpfup;
	u32 m_gpio_gpgcon;
	u32 m_gpio_gpgup;
	u32 m_gpio_gphcon;
	u32 m_gpio_gphup;
	u32 m_gpio_misccr;
	u32 m_gpio_opencr;
	u32 m_gpio_extint[3];

	u32 m_rtc_rtccon;
	u32 m_rtc_ticnt;
	u32 m_rtc_rtcalm;
	u32 m_rtc_almsec;
	u32 m_rtc_almmin;
	u32 m_rtc_almhour;
	u32 m_rtc_almday;
	u32 m_rtc_almmon;
	u32 m_rtc_almyear;
	u32 m_rtc_rtcrst;
	u32 m_rtc_bcdsec;
	u32 m_rtc_bcdmin;
	u32 m_rtc_bcdhour;
	u32 m_rtc_bcddate;
	u32 m_rtc_bcdday;
	u32 m_rtc_bcdmon;
	u32 m_rtc_bcdyear;
	emu_timer *m_rtc_timer_tick_count;
	emu_timer *m_rtc_timer_update;

	u32 m_adc_adccon;

	u32 m_lcdpal_data[0x400/4];

	u32 m_spi_spcon[2];
	u32 m_spi_spsta[2];
	u32 m_spi_sppin[2];
	u32 m_spi_sppre[2];
	u32 m_spi_sptdat[2];
	u32 m_spi_sprdat[2];
};

class s3c2400_device : public s3c24xx_device
{
public:
	s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map);

protected:
	s3c2400_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_reset() override ATTR_COLD;
	virtual void clkpow_reset() override;
	virtual void gpio_reset() override;
	virtual void usb_device_reset() override;

	virtual u32 get_irq_bit_for_int_type(u32 int_type) const override;

	u32 adc_adcdat_r(offs_t offset, u32 mem_mask = ~0);
	void adc_adcdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual u32 gpio_get_data_mask(int port) override;

	void mmc_reset();
	u32 mmc_mmcon_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmcrr_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmcrr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmfcon_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmfcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmsta_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmsta_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmfsta_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmfsta_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmpre_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmpre_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmlen_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmlen_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmcr7_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmcr7_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 mmc_mmrsp_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void mmc_mmrsp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 mmc_mmcmd_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void mmc_mmcmd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmcr16_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmcr16_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mmc_mmdat_r(offs_t offset, u32 mem_mask = ~0);
	void mmc_mmdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);

private:
	u32 m_mmc_mmcon;
	u32 m_mmc_mmcrr;
	u32 m_mmc_mmfcon;
	u32 m_mmc_mmsta;
	u32 m_mmc_mmfsta;
	u32 m_mmc_mmpre;
	u32 m_mmc_mmlen;
	u32 m_mmc_mmcr7;
	u32 m_mmc_mmrsp[4];
	u32 m_mmc_mmcmd[2];
	u32 m_mmc_mmcr16;
	u32 m_mmc_mmdat;
};

class s3c2410_device : public s3c2400_device
{
public:
	s3c2410_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto nand_command_w_callback() { return m_nand_command_w_cb.bind(); }
	auto nand_address_w_callback() { return m_nand_address_w_cb.bind(); }
	auto nand_data_r_callback() { return m_nand_data_r_cb.bind(); }
	auto nand_update_ecc_r_callback() { return m_nand_update_ecc_r_cb.bind(); }
	auto nand_data_w_callback() { return m_nand_data_w_cb.bind(); }

	virtual void map(address_map &map) override;

	virtual void frnb_w(int state);

	void touch_screen(int state);
	void gpio_set_extint(u32 line, int state);
	void nand_calculate_mecc(u8 *data, u32 size, u8 *mecc);

protected:
	s3c2410_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	void request_eint(u32 number);
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	enum : u32
	{
		SUBINT_RXD0,
		SUBINT_TXD0,
		SUBINT_ERR0,
		SUBINT_RXD1,
		SUBINT_TXD1,
		SUBINT_ERR1,
		SUBINT_RXD2,
		SUBINT_TXD2,
		SUBINT_ERR2,
		SUBINT_TC,
		SUBINT_ADC,
		SUBINT_CAM_C,
		SUBINT_CAM_P,
		SUBINT_WDT,
		SUBINT_AC97,

		SUBINT_COUNT
	};

	virtual void irq_reset() override;
	virtual u32 get_irq_bit_for_int_type(u32 int_type) const override;
	virtual u32 get_int_type_for_subint(u32 subint) const;
	void check_pending_subirq();
	void request_subirq(u32 int_type);
	void check_pending_eint();
	u32 irq_subsrcpnd_r(offs_t offset, u32 mem_mask = ~0);
	void irq_subsrcpnd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irq_intsubmsk_r(offs_t offset, u32 mem_mask = ~0);
	void irq_intsubmsk_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual u32 dma_dcon_get_tsz(const u32 value) override;
	virtual u32 dma_dcon_get_hwsrcsel(const u32 value) override;
	virtual u32 dma_dcon_get_servmode(const u32 value) override;
	virtual u32 dma_dcon_get_int(const u32 value) override;
	virtual u32 dma_daddr_get_addr(const u32 value) override;
	virtual void dma_dc_set_curr_addr(u32 &reg, const u32 value) override;
	virtual u32 dma_get_src_inc_flag(const int which) override;
	virtual u32 dma_get_dst_inc_flag(const int which) override;
	template <int Which> u32 dma_disrcc_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_disrcc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 dma_didstc_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void dma_didstc_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void clkpow_reset() override;

	rgb_t lcd_get_color_tft_24(u32 data);
	virtual rgb_t lcd_get_color_tpal() override;
	virtual void lcd_reset() override;
	virtual bool lcd_get_tpalen() override;
	virtual u32 lcd_get_tpalval() override;
	u32 lcd_lcdintpnd_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdintpnd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdsrcpnd_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdsrcpnd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lcdintmsk_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lcdintmsk_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 lcd_lpcsel_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_lpcsel_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void nand_reset();
	void nand_update_mecc(u8 *ecc, int pos, u8 data);
	void nand_update_ecc(u8 data);
	u8 nand_data_r();
	void nand_data_w(u8 data);
	virtual void nand_init_ecc();
	u32 nand_nfconf_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfconf_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfcmd_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfcmd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfaddr_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfaddr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	virtual u32 nand_nfdata_r(offs_t offset, u32 mem_mask = ~0);
	virtual void nand_nfdata_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfstat_r(offs_t offset, u32 mem_mask = ~0);
	virtual void nand_nfstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfecc_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfecc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	virtual void get_nand_params(int ncon, int &out_page_size, int &out_address_cycle);
	virtual void nand_auto_boot();

	virtual void uart_request_irq(int which) override;

	virtual void usb_device_reset() override;
	u32 usbdev_ep_int_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_ep_int_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_usb_int_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_usb_int_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_ep_int_en_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_ep_int_en_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_usb_int_en_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_usb_int_en_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_frame_num1_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_frame_num1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_frame_num2_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_frame_num2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_index_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_index_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_in_csr2_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_in_csr2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_maxp_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_maxp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_out_csr1_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_out_csr1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_out_csr2_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_out_csr2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_out_fifo_cnt1_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_out_fifo_cnt1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 usbdev_out_fifo_cnt2_r(offs_t offset, u32 mem_mask = ~0);
	void usbdev_out_fifo_cnt2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_dma_con_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_dma_con_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_dma_unit_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_dma_unit_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_dma_fifo_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_dma_fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_dma_ttc_l_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_dma_ttc_l_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_dma_ttc_m_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_dma_ttc_m_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 usbdev_ep_dma_ttc_h_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void usbdev_ep_dma_ttc_h_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void gpio_reset() override;
	virtual u32 gpio_get_data_mask(int port) override;
	u32 gpio_gphcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gphcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gphdat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gphdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gphup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gphup_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_dclkcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_dclkcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <int Which> u32 gpio_eintflt_r(offs_t offset, u32 mem_mask = ~0);
	template <int Which> void gpio_eintflt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_eintmask_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_eintmask_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_eintpend_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_eintpend_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gstatus0_r(offs_t offset, u32 mem_mask = ~0);
	u32 gpio_gstatus1_r(offs_t offset, u32 mem_mask = ~0);
	u32 gpio_gstatus2_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gstatus2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gstatus3_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gstatus3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gstatus4_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gstatus4_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void wdt_reset() override;
	virtual void wdt_start() override;
	virtual void wdt_stop() override;
	virtual u32 wdt_wtcnt_r(offs_t offset, u32 mem_mask = ~0) override;

	virtual void adc_reset() override;
	virtual void adc_start() override;
	u32 adc_adctsc_r(offs_t offset, u32 mem_mask = ~0);
	void adc_adctsc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 adc_adcdly_r(offs_t offset, u32 mem_mask = ~0);
	void adc_adcdly_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 adc_adcdat0_r(offs_t offset, u32 mem_mask = ~0);
	void adc_adcdat0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 adc_adcdat1_r(offs_t offset, u32 mem_mask = ~0);
	void adc_adcdat1_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void sdi_reset();
	u32 sdi_con_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_con_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_pre_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_pre_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_cmdarg_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_cmdarg_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_cmdcon_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_cmdcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_cmdsta_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_cmdsta_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_rsp0_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_rsp0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_rsp1_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_rsp1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_rsp2_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_rsp2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_rsp3_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_rsp3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_dtimer_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_dtimer_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_bsize_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_bsize_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_datcon_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_datcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_adidatcnt_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_adidatcnt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_adidatsta_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_adidatsta_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_fsta_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_fsta_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_dat_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_dat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdi_intmsk_r(offs_t offset, u32 mem_mask = ~0);
	void sdi_intmsk_w(offs_t offset, u32 data, u32 mem_mask = ~0);

protected:
	devcb_write8 m_nand_command_w_cb;
	devcb_write8 m_nand_address_w_cb;
	devcb_read8  m_nand_data_r_cb;
	devcb_read_line m_nand_update_ecc_r_cb;
	devcb_write8 m_nand_data_w_cb;

	u32 m_irq_subsrcpnd;
	u32 m_irq_intsubmsk;

	u32 m_dma_disrcc[4];
	u32 m_dma_didstc[4];

	u32 m_lcd_lpcsel;
	u32 m_lcd_lcdintpnd;
	u32 m_lcd_lcdsrcpnd;
	u32 m_lcd_lcdintmsk;

	u32 m_nand_nfconf;
	u32 m_nand_nfcmd;
	u32 m_nand_nfaddr;
	u32 m_nand_nfdata;
	u32 m_nand_nfstat;
	u32 m_nand_nfecc;
	u8 m_nand_mecc[4];
	int m_nand_ecc_pos;
	int m_nand_data_count;

	u32 m_gpio_dclkcon;
	u32 m_gpio_eintflt[4];
	u32 m_gpio_eintmask;
	u32 m_gpio_eintpend;
	u32 m_gpio_gstatus[5];
	u32 m_gpio_extint_lines;

	u8 m_steppingstone[4 * 1024];

	u32 m_usbdev_ep_int;
	u32 m_usbdev_usb_int;
	u32 m_usbdev_ep_int_en;
	u32 m_usbdev_usb_int_en;
	u32 m_usbdev_frame_num1;
	u32 m_usbdev_frame_num2;
	u32 m_usbdev_index;
	u32 m_usbdev_in_csr2;
	u32 m_usbdev_maxp;
	u32 m_usbdev_out_csr1;
	u32 m_usbdev_out_csr2;
	u32 m_usbdev_out_fifo_cnt1;
	u32 m_usbdev_out_fifo_cnt2;
	u32 m_usbdev_ep_dma_con[5];
	u32 m_usbdev_ep_dma_unit[5];
	u32 m_usbdev_ep_dma_fifo[5];
	u32 m_usbdev_ep_dma_ttc_l[5];
	u32 m_usbdev_ep_dma_ttc_m[5];
	u32 m_usbdev_ep_dma_ttc_h[5];

	u32 m_adc_adctsc;
	u32 m_adc_adcdly;
	u32 m_adc_adcdat[2];

	u32 m_sdi_con;
	u32 m_sdi_pre;
	u32 m_sdi_cmdarg;
	u32 m_sdi_cmdcon;
	u32 m_sdi_cmdsta;
	u32 m_sdi_rsp0;
	u32 m_sdi_rsp1;
	u32 m_sdi_rsp2;
	u32 m_sdi_rsp3;
	u32 m_sdi_dtimer;
	u32 m_sdi_bsize;
	u32 m_sdi_datcon;
	u32 m_sdi_adidatcnt;
	u32 m_sdi_adidatsta;
	u32 m_sdi_fsta;
	u32 m_sdi_intmsk;
	u32 m_sdi_dat;
};

class s3c2440_device : public s3c2410_device
{
public:
	s3c2440_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) override;

	virtual void frnb_w(int state) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void lcd_reset() override;
	u32 lcd_tconsel_r(offs_t offset, u32 mem_mask = ~0);
	void lcd_tconsel_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void clkpow_reset() override;
	virtual void clkpow_clkdivn_w(offs_t offset, u32 data, u32 mem_mask = ~0) override;

	virtual u32 get_clk_mult() const override;
	virtual u64 get_hclk_ticks() const override;

	u32 clkpow_camdivn_r(offs_t offset, u32 mem_mask = ~0);
	void clkpow_camdivn_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void irq_reset() override;
	virtual u32 get_irq_bit_for_int_type(u32 int_type) const override;
	virtual u32 get_int_type_for_subint(u32 subint) const override;

	virtual void gpio_reset() override;
	virtual u32 gpio_get_data_mask(int port) override;
	u32 gpio_mslcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_mslcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpjcon_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpjcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpjdat_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpjdat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gpio_gpjup_r(offs_t offset, u32 mem_mask = ~0);
	void gpio_gpjup_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void request_wdt_irq() override;

	virtual void iic_reset() override;
	u32 iic_iiclc_r(offs_t offset, u32 mem_mask = ~0);
	void iic_iiclc_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 adc_adcupdn_r(offs_t offset, u32 mem_mask = ~0);
	void adc_adcupdn_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void spi_reset() override;

	virtual void sdi_reset() override;

	virtual void nand_reset() override;
	void nand_update_secc(u8 *ecc, int pos, u8 data);
	void nand_update_ecc(const u8 data);
	virtual void nand_init_ecc() override;
	u32 nand_nfcont_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfcont_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	virtual void nand_nfstat_w(offs_t offset, u32 data, u32 mem_mask = ~0) override;
	virtual u32 nand_nfdata_r(offs_t offset, u32 mem_mask = ~0) override;
	virtual void nand_nfdata_w(offs_t offset, u32 data, u32 mem_mask = ~0) override;
	u32 nand_nfmeccd0_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfmeccd0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfmeccd1_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfmeccd1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfseccd_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfseccd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfestat0_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfestat0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfestat1_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfestat1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfmecc0_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfmecc0_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfmecc1_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfmecc1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfsecc_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfsecc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfsblk_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfsblk_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 nand_nfeblk_r(offs_t offset, u32 mem_mask = ~0);
	void nand_nfeblk_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	virtual void get_nand_params(int ncon, int &out_page_size, int &out_address_cycle) override;

	void cam_reset();
	u32 cam_srcfmt_r(offs_t offset, u32 mem_mask = ~0);
	void cam_srcfmt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_wdofst_r(offs_t offset, u32 mem_mask = ~0);
	void cam_wdofst_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_gctrl_r(offs_t offset, u32 mem_mask = ~0);
	void cam_gctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coysa1_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coysa1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coysa2_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coysa2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coysa3_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coysa3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coysa4_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coysa4_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocbsa1_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocbsa1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocbsa2_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocbsa2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocbsa3_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocbsa3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocbsa4_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocbsa4_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocrsa1_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocrsa1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocrsa2_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocrsa2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocrsa3_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocrsa3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cocrsa4_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cocrsa4_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cotrgfmt_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cotrgfmt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coctrl_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coscpreratio_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coscpreratio_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coscpredst_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coscpredst_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_coscctrl_r(offs_t offset, u32 mem_mask = ~0);
	void cam_coscctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_cotarea_r(offs_t offset, u32 mem_mask = ~0);
	void cam_cotarea_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_costatus_r(offs_t offset, u32 mem_mask = ~0);
	void cam_costatus_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prclrsa1_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prclrsa1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prclrsa2_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prclrsa2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prclrsa3_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prclrsa3_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prclrsa4_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prclrsa4_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prtrgfmt_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prtrgfmt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prctrl_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prscpreratio_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prscpreratio_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prscpredst_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prscpredst_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prscctrl_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prscctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prtarea_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prtarea_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_prstatus_r(offs_t offset, u32 mem_mask = ~0);
	void cam_prstatus_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cam_imgcpt_r(offs_t offset, u32 mem_mask = ~0);
	void cam_imgcpt_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void ac97_reset();
	u32 ac97_glbctrl_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_glbctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 ac97_glbstat_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_glbstat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 ac97_codec_cmd_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_codec_cmd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 ac97_codec_stat_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_codec_stat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 ac97_pcmaddr_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_pcmaddr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 ac97_micaddr_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_micaddr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 ac97_pcmdata_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_pcmdata_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 ac97_micdata_r(offs_t offset, u32 mem_mask = ~0);
	void ac97_micdata_w(offs_t offset, u32 data, u32 mem_mask = ~0);

private:
	u32 m_gpio_mslcon;
	u32 m_gpio_gpjcon;
	u32 m_gpio_gpjup;

	u32 m_lcd_tconsel;

	u32 m_adc_adcupdn;

	u32 m_iic_iiclc;

	u32 m_nand_nfcont;
	u32 m_nand_nfmeccd[2];
	u32 m_nand_nfseccd;
	u32 m_nand_nfestat[2];
	u32 m_nand_nfmecc[2];
	u32 m_nand_nfsecc;
	u32 m_nand_nfsblk;
	u32 m_nand_nfeblk;
	u8 m_nand_secc[2];
	int m_nand_ecc_pos;
	int m_nand_data_count;

	u32 m_cam_srcfmt;
	u32 m_cam_wdofst;
	u32 m_cam_gctrl;
	u32 m_cam_coysa1;
	u32 m_cam_coysa2;
	u32 m_cam_coysa3;
	u32 m_cam_coysa4;
	u32 m_cam_cocbsa1;
	u32 m_cam_cocbsa2;
	u32 m_cam_cocbsa3;
	u32 m_cam_cocbsa4;
	u32 m_cam_cocrsa1;
	u32 m_cam_cocrsa2;
	u32 m_cam_cocrsa3;
	u32 m_cam_cocrsa4;
	u32 m_cam_cotrgfmt;
	u32 m_cam_coctrl;
	u32 m_cam_coscpreratio;
	u32 m_cam_coscpredst;
	u32 m_cam_coscctrl;
	u32 m_cam_cotarea;
	u32 m_cam_costatus;
	u32 m_cam_prclrsa1;
	u32 m_cam_prclrsa2;
	u32 m_cam_prclrsa3;
	u32 m_cam_prclrsa4;
	u32 m_cam_prtrgfmt;
	u32 m_cam_prctrl;
	u32 m_cam_prscpreratio;
	u32 m_cam_prscpredst;
	u32 m_cam_prscctrl;
	u32 m_cam_prtarea;
	u32 m_cam_prstatus;
	u32 m_cam_imgcpt;

	u32 m_ac97_glbctrl;
	u32 m_ac97_glbstat;
	u32 m_ac97_codec_cmd;
	u32 m_ac97_codec_stat;
	u32 m_ac97_pcmaddr;
	u32 m_ac97_micaddr;
	u32 m_ac97_pcmdata;
	u32 m_ac97_micdata;

	u32 m_clkpow_camdivn;
};

DECLARE_DEVICE_TYPE(S3C2400, s3c2400_device)
DECLARE_DEVICE_TYPE(S3C2410, s3c2410_device)
DECLARE_DEVICE_TYPE(S3C2440, s3c2440_device)

#endif // MAME_MACHINE_S3C24XX_H
