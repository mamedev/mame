// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen, Ryan Holtz, Peter Wilhelmsen
#include "emu.h"
#include "s3c24xx.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "coreutil.h"
#include "screen.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>


DEFINE_DEVICE_TYPE(S3C2400, s3c2400_device, "s3c2400", "Samsung S3C2400 SoC")
DEFINE_DEVICE_TYPE(S3C2410, s3c2410_device, "s3c2410", "Samsung S3C2410 SoC")
DEFINE_DEVICE_TYPE(S3C2440, s3c2440_device, "s3c2440", "Samsung S3C2440 SoC")

/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

//#define UART_PRINTF

#define CLOCK_MULTIPLIER 1

#define LOG_RESET           (u64(1) << 1)
#define LOG_ADC             (u64(1) << 2)
#define LOG_LCD_DMA         (u64(1) << 3)
#define LOG_LCD_TIMER       (u64(1) << 4)
#define LOG_LCD_REGS        (u64(1) << 5)
#define LOG_SPI             (u64(1) << 6)
#define LOG_LCD_CFG         (u64(1) << 7)
#define LOG_LCD_TFT         (u64(1) << 8)
#define LOG_LCD_STN         (u64(1) << 9)
#define LOG_CLKPOW          (u64(1) << 10)
#define LOG_MMC             (u64(1) << 11)
#define LOG_IRQS            (u64(1) << 12)
#define LOG_IRQ_REGS        (u64(1) << 13)
#define LOG_FLASH           (u64(1) << 14)
#define LOG_PWM             (u64(1) << 15)
#define LOG_PWM_REGS        (u64(1) << 16)
#define LOG_CAM             (u64(1) << 17)
#define LOG_DMA             (u64(1) << 18)
#define LOG_DMA_REQS        (u64(1) << 19)
#define LOG_DMA_REGS        (u64(1) << 20)
#define LOG_AC97            (u64(1) << 21)
#define LOG_DMA_TIMERS      (u64(1) << 22)
#define LOG_GPIO            (u64(1) << 23)
#define LOG_MEMCON          (u64(1) << 24)
#define LOG_USBHOST         (u64(1) << 25)
#define LOG_UART            (u64(1) << 26)
#define LOG_USB             (u64(1) << 27)
#define LOG_WDT             (u64(1) << 28)
#define LOG_I2C             (u64(1) << 29)
#define LOG_I2S             (u64(1) << 30)
#define LOG_RTC             (u64(1) << 31)
#define LOG_SDI             (u64(1) << 32)

#define LOG_ALL             (LOG_RESET | LOG_ADC | LOG_LCD_DMA | LOG_LCD_TIMER | LOG_LCD_REGS | LOG_SPI | LOG_LCD_CFG | LOG_LCD_TFT | LOG_LCD_STN \
							| LOG_CLKPOW | LOG_MMC | LOG_IRQS | LOG_IRQ_REGS | LOG_FLASH | LOG_PWM | LOG_PWM_REGS | LOG_CAM | LOG_DMA | LOG_DMA_REQS \
							| LOG_DMA_REGS | LOG_AC97 | LOG_DMA_TIMERS | LOG_GPIO | LOG_MEMCON | LOG_USBHOST | LOG_UART | LOG_USB \
							| LOG_WDT | LOG_I2C | LOG_I2S | LOG_RTC | LOG_SDI)

#define VERBOSE             ( 0 )
#include "logmacro.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

s3c24xx_device::s3c24xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_cpu(*this, ":maincpu")
	, m_palette(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_om0_r_cb(*this, 0)
	, m_om1_r_cb(*this, 0)
	, m_ncon_r_cb(*this, 0)
	, m_pin_w_cb(*this)
	, m_port_r_cb(*this, 0)
	, m_port_w_cb(*this)
	, m_scl_w_cb(*this)
	, m_sda_r_cb(*this, 1)
	, m_sda_w_cb(*this)
	, m_data_r_cb(*this, 0)
	, m_iis_data_w_cb(*this)
	, m_iis_clockchange_w_cb(*this)
	, m_lcd_flags(0)
{
}

s3c2400_device::s3c2400_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: s3c24xx_device(mconfig, type, tag, owner, clock)
{
}

s3c2400_device::s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: s3c2400_device(mconfig, S3C2400, tag, owner, clock)
{
}

s3c2410_device::s3c2410_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: s3c24xx_device(mconfig, type, tag, owner, clock)
	, m_nand_command_w_cb(*this)
	, m_nand_address_w_cb(*this)
	, m_nand_data_r_cb(*this, 0)
	, m_nand_update_ecc_r_cb(*this, 0)
	, m_nand_data_w_cb(*this)
{
}

s3c2410_device::s3c2410_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: s3c2410_device(mconfig, S3C2410, tag, owner, clock)
{
}

s3c2440_device::s3c2440_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: s3c2410_device(mconfig, S3C2440, tag, owner, clock)
{
}

void s3c24xx_device::device_start()
{
	LOG("s3c24xx_device::device_start\n");
	LOGMASKED(LOG_RESET, "s3c24xx_device::device_start\n");
	for (int i = 0; i < 5; i++)
		m_pwm_timer[i] = timer_alloc(FUNC(s3c24xx_device::pwm_timer_exp), this);
	m_iic_timer = timer_alloc(FUNC(s3c24xx_device::iic_timer_exp), this);
	m_iis_timer = timer_alloc(FUNC(s3c24xx_device::iis_timer_exp), this);
	m_lcd_timer = timer_alloc(FUNC(s3c24xx_device::lcd_timer_exp), this);
	m_rtc_timer_tick_count = timer_alloc(FUNC(s3c24xx_device::rtc_timer_tick_count_exp), this);
	m_rtc_timer_update = timer_alloc(FUNC(s3c24xx_device::rtc_timer_update_exp), this);
	m_wdt_timer = timer_alloc(FUNC(s3c24xx_device::wdt_timer_exp), this);

	for (auto &timer : m_uart_tx_timer)
		timer = timer_alloc(FUNC(s3c24xx_device::uart_tx_complete), this);

	save_item(NAME(m_memcon_bwscon));
	save_item(NAME(m_memcon_bankcon));
	save_item(NAME(m_memcon_refresh));
	save_item(NAME(m_memcon_banksize));
	save_item(NAME(m_memcon_mrsrb6));
	save_item(NAME(m_memcon_mrsrb7));

	save_item(NAME(m_usbhost_rev));
	save_item(NAME(m_usbhost_ctrl));
	save_item(NAME(m_usbhost_common_stat));
	save_item(NAME(m_usbhost_int_stat));
	save_item(NAME(m_usbhost_int_en));
	save_item(NAME(m_usbhost_int_dis));
	save_item(NAME(m_usbhost_hcca));
	save_item(NAME(m_usbhost_period_curr));
	save_item(NAME(m_usbhost_ctrl_head));
	save_item(NAME(m_usbhost_ctrl_curr));
	save_item(NAME(m_usbhost_bulk_head));
	save_item(NAME(m_usbhost_bulk_curr));
	save_item(NAME(m_usbhost_done_head));
	save_item(NAME(m_usbhost_rm_interval));
	save_item(NAME(m_usbhost_fm_remaining));
	save_item(NAME(m_usbhost_fm_number));
	save_item(NAME(m_usbhost_periodic_start));
	save_item(NAME(m_usbhost_ls_thresh));
	save_item(NAME(m_usbhost_rh_desc));
	save_item(NAME(m_usbhost_rh_status));
	save_item(NAME(m_usbhost_rh_port_status));

	save_item(NAME(m_irq_srcpnd));
	save_item(NAME(m_irq_intmod));
	save_item(NAME(m_irq_intmsk));
	save_item(NAME(m_irq_priority));
	save_item(NAME(m_irq_intpnd));
	save_item(NAME(m_irq_intoffset));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_fiq_line));

	save_item(NAME(m_dma_disrc));
	save_item(NAME(m_dma_didst));
	save_item(NAME(m_dma_dcon));
	save_item(NAME(m_dma_dstat));
	save_item(NAME(m_dma_dcsrc));
	save_item(NAME(m_dma_dcdst));
	save_item(NAME(m_dma_dmasktrig));

	save_item(NAME(m_clkpow_locktime));
	save_item(NAME(m_clkpow_mpllcon));
	save_item(NAME(m_clkpow_upllcon));
	save_item(NAME(m_clkpow_clkcon));
	save_item(NAME(m_clkpow_clkslow));
	save_item(NAME(m_clkpow_clkdivn));

	save_item(NAME(m_lcd_lcdcon1));
	save_item(NAME(m_lcd_lcdcon2));
	save_item(NAME(m_lcd_lcdcon3));
	save_item(NAME(m_lcd_lcdcon4));
	save_item(NAME(m_lcd_lcdcon5));
	save_item(NAME(m_lcd_lcdsaddr1));
	save_item(NAME(m_lcd_lcdsaddr2));
	save_item(NAME(m_lcd_lcdsaddr3));
	save_item(NAME(m_lcd_redlut));
	save_item(NAME(m_lcd_greenlut));
	save_item(NAME(m_lcd_bluelut));
	save_item(NAME(m_lcd_dithmode));
	save_item(NAME(m_lcd_tpal));
	save_item(NAME(m_lcd_vramaddr_cur));
	save_item(NAME(m_lcd_vramaddr_max));
	save_item(NAME(m_lcd_offsize));
	save_item(NAME(m_lcd_pagewidth_cur));
	save_item(NAME(m_lcd_pagewidth_max));
	save_item(NAME(m_lcd_bppmode));
	save_item(NAME(m_lcd_bswp));
	save_item(NAME(m_lcd_hwswp));
	save_item(NAME(m_lcd_vpos));
	save_item(NAME(m_lcd_hpos));
	save_item(NAME(m_lcd_framerate));
	save_item(NAME(m_lcd_hpos_min));
	save_item(NAME(m_lcd_hpos_max));
	save_item(NAME(m_lcd_vpos_min));
	save_item(NAME(m_lcd_vpos_max));
	save_item(NAME(m_lcd_dma_data));
	save_item(NAME(m_lcd_dma_bits));

	save_item(NAME(m_uart_ulcon));
	save_item(NAME(m_uart_ucon));
	save_item(NAME(m_uart_ufcon));
	save_item(NAME(m_uart_umcon));
	save_item(NAME(m_uart_utrstat));
	save_item(NAME(m_uart_uerstat));
	save_item(NAME(m_uart_ufstat));
	save_item(NAME(m_uart_umstat));
	save_item(NAME(m_uart_utxh));
	save_item(NAME(m_uart_urxh));
	save_item(NAME(m_uart_ubrdiv));
	save_item(NAME(m_uart_tx_fifo));
	save_item(NAME(m_uart_tx_head));
	save_item(NAME(m_uart_tx_count));
	save_item(NAME(m_uart_tx_shift));
	save_item(NAME(m_uart_tx_busy));
	save_item(NAME(m_uart_tx_irq));

	save_item(NAME(m_pwm_tcfg0));
	save_item(NAME(m_pwm_tcfg1));
	save_item(NAME(m_pwm_tcon));
	save_item(NAME(m_pwm_tcntb0));
	save_item(NAME(m_pwm_tcmpb0));
	save_item(NAME(m_pwm_tcnto0));
	save_item(NAME(m_pwm_tcntb1));
	save_item(NAME(m_pwm_tcmpb1));
	save_item(NAME(m_pwm_tcnto1));
	save_item(NAME(m_pwm_tcntb2));
	save_item(NAME(m_pwm_tcmpb2));
	save_item(NAME(m_pwm_tcnto2));
	save_item(NAME(m_pwm_tcntb3));
	save_item(NAME(m_pwm_tcmpb3));
	save_item(NAME(m_pwm_tcnto3));
	save_item(NAME(m_pwm_tcntb4));
	save_item(NAME(m_pwm_tcnto4));
	save_item(NAME(m_pwm_cnt));
	save_item(NAME(m_pwm_cmp));
	save_item(NAME(m_pwm_freq));

	save_item(NAME(m_usbdev_func_addr));
	save_item(NAME(m_usbdev_pwr));
	save_item(NAME(m_usbdev_int));
	save_item(NAME(m_usbdev_int_mask));
	save_item(NAME(m_usbdev_frame_num));
	save_item(NAME(m_usbdev_resume_con));
	save_item(NAME(m_usbdev_ep_csr));
	save_item(NAME(m_usbdev_ep_maxp));
	save_item(NAME(m_usbdev_ep_out_cnt));
	save_item(NAME(m_usbdev_ep_fifo));
	save_item(NAME(m_usbdev_dma_con));
	save_item(NAME(m_usbdev_dma_unit));
	save_item(NAME(m_usbdev_dma_fifo));
	save_item(NAME(m_usbdev_dma_tx));
	save_item(NAME(m_usbdev_test_mode));
	save_item(NAME(m_usbdev_in_con));

	save_item(NAME(m_wdt_wtcon));
	save_item(NAME(m_wdt_wtdat));
	save_item(NAME(m_wdt_wtcnt));
	save_item(NAME(m_wdt_freq));
	save_item(NAME(m_wdt_cnt));

	save_item(NAME(m_iic_iiccon));
	save_item(NAME(m_iic_iicstat));
	save_item(NAME(m_iic_iicadd));
	save_item(NAME(m_iic_iicds));
	save_item(NAME(m_iic_count));

	save_item(NAME(m_iis_iiscon));
	save_item(NAME(m_iis_iismod));
	save_item(NAME(m_iis_iispsr));
	save_item(NAME(m_iis_iisfcon));
	save_item(NAME(m_iis_tx_fifo));
	save_item(NAME(m_iis_fifo_head));
	save_item(NAME(m_iis_fifo_count));
	save_item(NAME(m_iis_channel));

	save_item(NAME(m_gpio_gpacon));
	save_item(NAME(m_gpio_gpbcon));
	save_item(NAME(m_gpio_gpbup));
	save_item(NAME(m_gpio_gpccon));
	save_item(NAME(m_gpio_gpcup));
	save_item(NAME(m_gpio_gpdcon));
	save_item(NAME(m_gpio_gpdup));
	save_item(NAME(m_gpio_gpecon));
	save_item(NAME(m_gpio_gpeup));
	save_item(NAME(m_gpio_gpfcon));
	save_item(NAME(m_gpio_gpfup));
	save_item(NAME(m_gpio_gpgcon));
	save_item(NAME(m_gpio_gpgup));
	save_item(NAME(m_gpio_gphcon));
	save_item(NAME(m_gpio_gphup));
	save_item(NAME(m_gpio_misccr));
	save_item(NAME(m_gpio_opencr));
	save_item(NAME(m_gpio_extint));

	save_item(NAME(m_rtc_rtccon));
	save_item(NAME(m_rtc_ticnt));
	save_item(NAME(m_rtc_rtcalm));
	save_item(NAME(m_rtc_almsec));
	save_item(NAME(m_rtc_almmin));
	save_item(NAME(m_rtc_almhour));
	save_item(NAME(m_rtc_almday));
	save_item(NAME(m_rtc_almmon));
	save_item(NAME(m_rtc_almyear));
	save_item(NAME(m_rtc_rtcrst));
	save_item(NAME(m_rtc_bcdsec));
	save_item(NAME(m_rtc_bcdmin));
	save_item(NAME(m_rtc_bcdhour));
	save_item(NAME(m_rtc_bcddate));
	save_item(NAME(m_rtc_bcdday));
	save_item(NAME(m_rtc_bcdmon));
	save_item(NAME(m_rtc_bcdyear));

	save_item(NAME(m_adc_adccon));

	save_item(NAME(m_spi_spcon));
	save_item(NAME(m_spi_spsta));
	save_item(NAME(m_spi_sppin));
	save_item(NAME(m_spi_sppre));
	save_item(NAME(m_spi_sptdat));
	save_item(NAME(m_spi_sprdat));

	video_start();
}

void s3c2400_device::device_start()
{
	s3c24xx_device::device_start();

	save_item(NAME(m_mmc_mmcon));
	save_item(NAME(m_mmc_mmcrr));
	save_item(NAME(m_mmc_mmfcon));
	save_item(NAME(m_mmc_mmsta));
	save_item(NAME(m_mmc_mmfsta));
	save_item(NAME(m_mmc_mmpre));
	save_item(NAME(m_mmc_mmlen));
	save_item(NAME(m_mmc_mmcr7));
	save_item(NAME(m_mmc_mmrsp));
	save_item(NAME(m_mmc_mmcmd));
	save_item(NAME(m_mmc_mmcr16));
	save_item(NAME(m_mmc_mmdat));
}

void s3c2410_device::device_start()
{
	s3c24xx_device::device_start();
	m_gpio_extint_lines = 0;
	std::fill(std::begin(m_adc_adcdat), std::end(m_adc_adcdat), 0);
	std::fill(std::begin(m_steppingstone), std::end(m_steppingstone), 0);

	save_item(NAME(m_irq_subsrcpnd));
	save_item(NAME(m_irq_intsubmsk));

	save_item(NAME(m_dma_disrcc));
	save_item(NAME(m_dma_didstc));

	save_item(NAME(m_lcd_lpcsel));
	save_item(NAME(m_lcd_lcdintpnd));
	save_item(NAME(m_lcd_lcdsrcpnd));
	save_item(NAME(m_lcd_lcdintmsk));

	save_item(NAME(m_nand_nfconf));
	save_item(NAME(m_nand_nfcmd));
	save_item(NAME(m_nand_nfaddr));
	save_item(NAME(m_nand_nfstat));
	save_item(NAME(m_nand_nfecc));
	save_item(NAME(m_nand_mecc));
	save_item(NAME(m_nand_ecc_pos));
	save_item(NAME(m_nand_data_count));

	save_item(NAME(m_gpio_dclkcon));
	save_item(NAME(m_gpio_eintflt));
	save_item(NAME(m_gpio_eintmask));
	save_item(NAME(m_gpio_eintpend));
	save_item(NAME(m_gpio_gstatus));
	save_item(NAME(m_gpio_extint_lines));

	save_item(NAME(m_steppingstone));

	save_item(NAME(m_usbdev_ep_int));
	save_item(NAME(m_usbdev_usb_int));
	save_item(NAME(m_usbdev_ep_int_en));
	save_item(NAME(m_usbdev_usb_int_en));
	save_item(NAME(m_usbdev_frame_num1));
	save_item(NAME(m_usbdev_frame_num2));
	save_item(NAME(m_usbdev_index));
	save_item(NAME(m_usbdev_in_csr2));
	save_item(NAME(m_usbdev_maxp));
	save_item(NAME(m_usbdev_out_csr1));
	save_item(NAME(m_usbdev_out_csr2));
	save_item(NAME(m_usbdev_out_fifo_cnt1));
	save_item(NAME(m_usbdev_out_fifo_cnt2));
	save_item(NAME(m_usbdev_ep_dma_con));
	save_item(NAME(m_usbdev_ep_dma_unit));
	save_item(NAME(m_usbdev_ep_dma_fifo));
	save_item(NAME(m_usbdev_ep_dma_ttc_l));
	save_item(NAME(m_usbdev_ep_dma_ttc_m));
	save_item(NAME(m_usbdev_ep_dma_ttc_h));

	save_item(NAME(m_adc_adctsc));
	save_item(NAME(m_adc_adcdly));
	save_item(NAME(m_adc_adcdat));

	save_item(NAME(m_sdi_con));
	save_item(NAME(m_sdi_pre));
	save_item(NAME(m_sdi_cmdarg));
	save_item(NAME(m_sdi_cmdcon));
	save_item(NAME(m_sdi_cmdsta));
	save_item(NAME(m_sdi_rsp0));
	save_item(NAME(m_sdi_rsp1));
	save_item(NAME(m_sdi_rsp2));
	save_item(NAME(m_sdi_rsp3));
	save_item(NAME(m_sdi_dtimer));
	save_item(NAME(m_sdi_bsize));
	save_item(NAME(m_sdi_datcon));
	save_item(NAME(m_sdi_adidatcnt));
	save_item(NAME(m_sdi_adidatsta));
	save_item(NAME(m_sdi_fsta));
	save_item(NAME(m_sdi_intmsk));
	save_item(NAME(m_sdi_dat));

	const int om0 = m_om0_r_cb();
	const int om1 = m_om1_r_cb();
	LOGMASKED(LOG_RESET, "OM01 %d,%d\n", om0, om1);
	if (om0 == 0 && om1 == 0)
	{
		address_space &space = m_cpu->space(AS_PROGRAM);
		space.install_ram(0x00000000, 0x00000fff, m_steppingstone);
		space.install_ram(0x40000000, 0x40000fff, m_steppingstone);
	}
}

void s3c2440_device::device_start()
{
	s3c2410_device::device_start();
	m_adc_adcupdn = 0;

	save_item(NAME(m_gpio_mslcon));
	save_item(NAME(m_gpio_gpjcon));
	save_item(NAME(m_gpio_gpjup));

	save_item(NAME(m_lcd_tconsel));

	save_item(NAME(m_adc_adcupdn));

	save_item(NAME(m_iic_iiclc));

	save_item(NAME(m_nand_nfcont));
	save_item(NAME(m_nand_nfmeccd));
	save_item(NAME(m_nand_nfseccd));
	save_item(NAME(m_nand_nfestat));
	save_item(NAME(m_nand_nfmecc));
	save_item(NAME(m_nand_nfsecc));
	save_item(NAME(m_nand_nfsblk));
	save_item(NAME(m_nand_nfeblk));
	save_item(NAME(m_nand_secc));

	save_item(NAME(m_cam_srcfmt));
	save_item(NAME(m_cam_wdofst));
	save_item(NAME(m_cam_gctrl));
	save_item(NAME(m_cam_coysa1));
	save_item(NAME(m_cam_coysa2));
	save_item(NAME(m_cam_coysa3));
	save_item(NAME(m_cam_coysa4));
	save_item(NAME(m_cam_cocbsa1));
	save_item(NAME(m_cam_cocbsa2));
	save_item(NAME(m_cam_cocbsa3));
	save_item(NAME(m_cam_cocbsa4));
	save_item(NAME(m_cam_cocrsa1));
	save_item(NAME(m_cam_cocrsa2));
	save_item(NAME(m_cam_cocrsa3));
	save_item(NAME(m_cam_cocrsa4));
	save_item(NAME(m_cam_cotrgfmt));
	save_item(NAME(m_cam_coctrl));
	save_item(NAME(m_cam_coscpreratio));
	save_item(NAME(m_cam_coscpredst));
	save_item(NAME(m_cam_coscctrl));
	save_item(NAME(m_cam_cotarea));
	save_item(NAME(m_cam_costatus));
	save_item(NAME(m_cam_prclrsa1));
	save_item(NAME(m_cam_prclrsa2));
	save_item(NAME(m_cam_prclrsa3));
	save_item(NAME(m_cam_prclrsa4));
	save_item(NAME(m_cam_prtrgfmt));
	save_item(NAME(m_cam_prctrl));
	save_item(NAME(m_cam_prscpreratio));
	save_item(NAME(m_cam_prscpredst));
	save_item(NAME(m_cam_prscctrl));
	save_item(NAME(m_cam_prtarea));
	save_item(NAME(m_cam_prstatus));
	save_item(NAME(m_cam_imgcpt));

	save_item(NAME(m_ac97_glbctrl));
	save_item(NAME(m_ac97_glbstat));
	save_item(NAME(m_ac97_codec_cmd));
	save_item(NAME(m_ac97_codec_stat));
	save_item(NAME(m_ac97_pcmaddr));
	save_item(NAME(m_ac97_micaddr));
	save_item(NAME(m_ac97_pcmdata));
	save_item(NAME(m_ac97_micdata));

	save_item(NAME(m_clkpow_camdivn));
}

void s3c24xx_device::device_post_load()
{
	// Scanout and LCD persistence are regenerated by the restored LCD timer.
	for (auto &bitmap : m_lcd_bitmap)
	{
		bitmap.fill(0);
	}
}

void s3c24xx_device::device_reset()
{
	LOG("s3c24xx_device::device_reset\n");
	LOGMASKED(LOG_RESET, "s3c24xx device reset\n");

	std::fill(std::begin(m_dma_disrc), std::end(m_dma_disrc), 0);
	std::fill(std::begin(m_dma_didst), std::end(m_dma_didst), 0);
	std::fill(std::begin(m_dma_dcon), std::end(m_dma_dcon), 0);
	std::fill(std::begin(m_dma_dstat), std::end(m_dma_dstat), 0);
	std::fill(std::begin(m_dma_dcsrc), std::end(m_dma_dcsrc), 0);
	std::fill(std::begin(m_dma_dcdst), std::end(m_dma_dcdst), 0);
	std::fill(std::begin(m_dma_dmasktrig), std::end(m_dma_dmasktrig), 0);

	uart_reset();
	pwm_reset();
	iic_reset();
	iis_reset();
	lcd_reset();
	rtc_reset();
	wdt_reset();
	irq_reset();
	gpio_reset();
	memcon_reset();
	clkpow_reset();
	usbhost_reset();
	usb_device_reset();
	adc_reset();
	spi_reset();
}

void s3c2400_device::device_reset()
{
	s3c24xx_device::device_reset();
	LOG("s3c2400_device::device_reset\n");
	mmc_reset();
}

void s3c2410_device::device_reset()
{
	s3c24xx_device::device_reset();
	LOG("s3c2410_device::device_reset\n");
	std::fill(std::begin(m_dma_disrcc), std::end(m_dma_disrcc), 0);
	std::fill(std::begin(m_dma_didstc), std::end(m_dma_didstc), 0);
	sdi_reset();
	nand_reset();
	nand_auto_boot();
}

void s3c2440_device::device_reset()
{
	s3c2410_device::device_reset();
	LOG("s3c2440_device::device_reset\n");
	cam_reset();
	ac97_reset();
}

void s3c2400_device::map(address_map &map)
{
	map(0x00000000, 0x00000003).rw(FUNC(s3c2400_device::memcon_bwscon_r), FUNC(s3c2400_device::memcon_bwscon_w));
	map(0x00000004, 0x00000007).rw(FUNC(s3c2400_device::memcon_bankcon_r<0>), FUNC(s3c2400_device::memcon_bankcon_w<0>));
	map(0x00000008, 0x0000000b).rw(FUNC(s3c2400_device::memcon_bankcon_r<1>), FUNC(s3c2400_device::memcon_bankcon_w<1>));
	map(0x0000000c, 0x0000000f).rw(FUNC(s3c2400_device::memcon_bankcon_r<2>), FUNC(s3c2400_device::memcon_bankcon_w<2>));
	map(0x00000010, 0x00000013).rw(FUNC(s3c2400_device::memcon_bankcon_r<3>), FUNC(s3c2400_device::memcon_bankcon_w<3>));
	map(0x00000014, 0x00000017).rw(FUNC(s3c2400_device::memcon_bankcon_r<4>), FUNC(s3c2400_device::memcon_bankcon_w<4>));
	map(0x00000018, 0x0000001b).rw(FUNC(s3c2400_device::memcon_bankcon_r<5>), FUNC(s3c2400_device::memcon_bankcon_w<5>));
	map(0x0000001c, 0x0000001f).rw(FUNC(s3c2400_device::memcon_bankcon_r<6>), FUNC(s3c2400_device::memcon_bankcon_w<6>));
	map(0x00000020, 0x00000023).rw(FUNC(s3c2400_device::memcon_bankcon_r<7>), FUNC(s3c2400_device::memcon_bankcon_w<7>));
	map(0x00000024, 0x00000027).rw(FUNC(s3c2400_device::memcon_refresh_r), FUNC(s3c2400_device::memcon_refresh_w));
	map(0x00000028, 0x0000002b).rw(FUNC(s3c2400_device::memcon_banksize_r), FUNC(s3c2400_device::memcon_banksize_w));
	map(0x0000002c, 0x0000002f).rw(FUNC(s3c2400_device::memcon_mrsrb6_r), FUNC(s3c2400_device::memcon_mrsrb6_w));
	map(0x00000030, 0x00000033).rw(FUNC(s3c2400_device::memcon_mrsrb7_r), FUNC(s3c2400_device::memcon_mrsrb7_w));

	map(0x00200000, 0x00200003).r(FUNC(s3c2400_device::usbhost_rev_r));
	map(0x00200004, 0x00200007).rw(FUNC(s3c2400_device::usbhost_ctrl_r), FUNC(s3c2400_device::usbhost_ctrl_w));
	map(0x00200008, 0x0020000b).rw(FUNC(s3c2400_device::usbhost_common_stat_r), FUNC(s3c2400_device::usbhost_common_stat_w));
	map(0x0020000c, 0x0020000f).rw(FUNC(s3c2400_device::usbhost_int_stat_r), FUNC(s3c2400_device::usbhost_int_stat_w));
	map(0x00200010, 0x00200013).rw(FUNC(s3c2400_device::usbhost_int_en_r), FUNC(s3c2400_device::usbhost_int_en_w));
	map(0x00200014, 0x00200017).rw(FUNC(s3c2400_device::usbhost_int_dis_r), FUNC(s3c2400_device::usbhost_int_dis_w));
	map(0x00200018, 0x0020001b).rw(FUNC(s3c2400_device::usbhost_hcca_r), FUNC(s3c2400_device::usbhost_hcca_w));
	map(0x0020001c, 0x0020001f).rw(FUNC(s3c2400_device::usbhost_period_curr_r), FUNC(s3c2400_device::usbhost_period_curr_w));
	map(0x00200020, 0x00200023).rw(FUNC(s3c2400_device::usbhost_ctrl_head_r), FUNC(s3c2400_device::usbhost_ctrl_head_w));
	map(0x00200024, 0x00200027).rw(FUNC(s3c2400_device::usbhost_ctrl_curr_r), FUNC(s3c2400_device::usbhost_ctrl_curr_w));
	map(0x00200028, 0x0020002b).rw(FUNC(s3c2400_device::usbhost_bulk_head_r), FUNC(s3c2400_device::usbhost_bulk_head_w));
	map(0x0020002c, 0x0020002f).rw(FUNC(s3c2400_device::usbhost_bulk_curr_r), FUNC(s3c2400_device::usbhost_bulk_curr_w));
	map(0x00200030, 0x00200033).rw(FUNC(s3c2400_device::usbhost_done_head_r), FUNC(s3c2400_device::usbhost_done_head_w));
	map(0x00200034, 0x00200037).rw(FUNC(s3c2400_device::usbhost_rm_interval_r), FUNC(s3c2400_device::usbhost_rm_interval_w));
	map(0x00200038, 0x0020003b).rw(FUNC(s3c2400_device::usbhost_fm_remaining_r), FUNC(s3c2400_device::usbhost_fm_remaining_w));
	map(0x0020003c, 0x0020003f).rw(FUNC(s3c2400_device::usbhost_fm_number_r), FUNC(s3c2400_device::usbhost_fm_number_w));
	map(0x00200040, 0x00200043).rw(FUNC(s3c2400_device::usbhost_periodic_start_r), FUNC(s3c2400_device::usbhost_periodic_start_w));
	map(0x00200044, 0x00200047).rw(FUNC(s3c2400_device::usbhost_ls_thresh_r), FUNC(s3c2400_device::usbhost_ls_thresh_w));
	map(0x00200048, 0x0020004b).rw(FUNC(s3c2400_device::usbhost_rh_desc_a_r), FUNC(s3c2400_device::usbhost_rh_desc_a_w));
	map(0x0020004c, 0x0020004f).rw(FUNC(s3c2400_device::usbhost_rh_desc_b_r), FUNC(s3c2400_device::usbhost_rh_desc_b_w));
	map(0x00200050, 0x00200053).rw(FUNC(s3c2400_device::usbhost_rh_status_r), FUNC(s3c2400_device::usbhost_rh_status_w));
	map(0x00200054, 0x00200057).rw(FUNC(s3c2400_device::usbhost_rh_port_stat_1_r), FUNC(s3c2400_device::usbhost_rh_port_stat_1_w));
	map(0x00200058, 0x0020005b).rw(FUNC(s3c2400_device::usbhost_rh_port_stat_2_r), FUNC(s3c2400_device::usbhost_rh_port_stat_2_w));

	map(0x00400000, 0x00400003).rw(FUNC(s3c2400_device::irq_srcpnd_r), FUNC(s3c2400_device::irq_srcpnd_w));
	map(0x00400004, 0x00400007).rw(FUNC(s3c2400_device::irq_intmod_r), FUNC(s3c2400_device::irq_intmod_w));
	map(0x00400008, 0x0040000b).rw(FUNC(s3c2400_device::irq_intmsk_r), FUNC(s3c2400_device::irq_intmsk_w));
	map(0x0040000c, 0x0040000f).rw(FUNC(s3c2400_device::irq_priority_r), FUNC(s3c2400_device::irq_priority_w));
	map(0x00400010, 0x00400013).rw(FUNC(s3c2400_device::irq_intpnd_r), FUNC(s3c2400_device::irq_intpnd_w));
	map(0x00400014, 0x00400017).rw(FUNC(s3c2400_device::irq_intoffset_r), FUNC(s3c2400_device::irq_intoffset_w));

	map(0x00600000, 0x00600003).rw(FUNC(s3c2400_device::dma_disrc_r<0>), FUNC(s3c2400_device::dma_disrc_w<0>));
	map(0x00600004, 0x00600007).rw(FUNC(s3c2400_device::dma_didst_r<0>), FUNC(s3c2400_device::dma_didst_w<0>));
	map(0x00600008, 0x0060000b).rw(FUNC(s3c2400_device::dma_dcon_r<0>), FUNC(s3c2400_device::dma_dcon_w<0>));
	map(0x0060000c, 0x0060000f).rw(FUNC(s3c2400_device::dma_dstat_r<0>), FUNC(s3c2400_device::dma_dstat_w<0>));
	map(0x00600010, 0x00600013).rw(FUNC(s3c2400_device::dma_dcsrc_r<0>), FUNC(s3c2400_device::dma_dcsrc_w<0>));
	map(0x00600014, 0x00600017).rw(FUNC(s3c2400_device::dma_dcdst_r<0>), FUNC(s3c2400_device::dma_dcdst_w<0>));
	map(0x00600018, 0x0060001b).rw(FUNC(s3c2400_device::dma_dmasktrig_r<0>), FUNC(s3c2400_device::dma_dmasktrig_w<0>));
	map(0x00600020, 0x00600023).rw(FUNC(s3c2400_device::dma_disrc_r<1>), FUNC(s3c2400_device::dma_disrc_w<1>));
	map(0x00600024, 0x00600027).rw(FUNC(s3c2400_device::dma_didst_r<1>), FUNC(s3c2400_device::dma_didst_w<1>));
	map(0x00600028, 0x0060002b).rw(FUNC(s3c2400_device::dma_dcon_r<1>), FUNC(s3c2400_device::dma_dcon_w<1>));
	map(0x0060002c, 0x0060002f).rw(FUNC(s3c2400_device::dma_dstat_r<1>), FUNC(s3c2400_device::dma_dstat_w<1>));
	map(0x00600030, 0x00600033).rw(FUNC(s3c2400_device::dma_dcsrc_r<1>), FUNC(s3c2400_device::dma_dcsrc_w<1>));
	map(0x00600034, 0x00600037).rw(FUNC(s3c2400_device::dma_dcdst_r<1>), FUNC(s3c2400_device::dma_dcdst_w<1>));
	map(0x00600038, 0x0060003b).rw(FUNC(s3c2400_device::dma_dmasktrig_r<1>), FUNC(s3c2400_device::dma_dmasktrig_w<1>));
	map(0x00600040, 0x00600043).rw(FUNC(s3c2400_device::dma_disrc_r<2>), FUNC(s3c2400_device::dma_disrc_w<2>));
	map(0x00600044, 0x00600047).rw(FUNC(s3c2400_device::dma_didst_r<2>), FUNC(s3c2400_device::dma_didst_w<2>));
	map(0x00600048, 0x0060004b).rw(FUNC(s3c2400_device::dma_dcon_r<2>), FUNC(s3c2400_device::dma_dcon_w<2>));
	map(0x0060004c, 0x0060004f).rw(FUNC(s3c2400_device::dma_dstat_r<2>), FUNC(s3c2400_device::dma_dstat_w<2>));
	map(0x00600050, 0x00600053).rw(FUNC(s3c2400_device::dma_dcsrc_r<2>), FUNC(s3c2400_device::dma_dcsrc_w<2>));
	map(0x00600054, 0x00600057).rw(FUNC(s3c2400_device::dma_dcdst_r<2>), FUNC(s3c2400_device::dma_dcdst_w<2>));
	map(0x00600058, 0x0060005b).rw(FUNC(s3c2400_device::dma_dmasktrig_r<2>), FUNC(s3c2400_device::dma_dmasktrig_w<2>));
	map(0x00600060, 0x00600063).rw(FUNC(s3c2400_device::dma_disrc_r<3>), FUNC(s3c2400_device::dma_disrc_w<3>));
	map(0x00600064, 0x00600067).rw(FUNC(s3c2400_device::dma_didst_r<3>), FUNC(s3c2400_device::dma_didst_w<3>));
	map(0x00600068, 0x0060006b).rw(FUNC(s3c2400_device::dma_dcon_r<3>), FUNC(s3c2400_device::dma_dcon_w<3>));
	map(0x0060006c, 0x0060006f).rw(FUNC(s3c2400_device::dma_dstat_r<3>), FUNC(s3c2400_device::dma_dstat_w<3>));
	map(0x00600070, 0x00600073).rw(FUNC(s3c2400_device::dma_dcsrc_r<3>), FUNC(s3c2400_device::dma_dcsrc_w<3>));
	map(0x00600074, 0x00600077).rw(FUNC(s3c2400_device::dma_dcdst_r<3>), FUNC(s3c2400_device::dma_dcdst_w<3>));
	map(0x00600078, 0x0060007b).rw(FUNC(s3c2400_device::dma_dmasktrig_r<3>), FUNC(s3c2400_device::dma_dmasktrig_w<3>));

	map(0x00800000, 0x00800003).rw(FUNC(s3c2400_device::clkpow_locktime_r), FUNC(s3c2400_device::clkpow_locktime_w));
	map(0x00800004, 0x00800007).rw(FUNC(s3c2400_device::clkpow_mpllcon_r), FUNC(s3c2400_device::clkpow_mpllcon_w));
	map(0x00800008, 0x0080000b).rw(FUNC(s3c2400_device::clkpow_upllcon_r), FUNC(s3c2400_device::clkpow_upllcon_w));
	map(0x0080000c, 0x0080000f).rw(FUNC(s3c2400_device::clkpow_clkcon_r), FUNC(s3c2400_device::clkpow_clkcon_w));
	map(0x00800010, 0x00800013).rw(FUNC(s3c2400_device::clkpow_clkslow_r), FUNC(s3c2400_device::clkpow_clkslow_w));
	map(0x00800014, 0x00800017).rw(FUNC(s3c2400_device::clkpow_clkdivn_r), FUNC(s3c2400_device::clkpow_clkdivn_w));

	map(0x00a00000, 0x00a00003).rw(FUNC(s3c2400_device::lcd_lcdcon1_r), FUNC(s3c2400_device::lcd_lcdcon1_w));
	map(0x00a00004, 0x00a00007).rw(FUNC(s3c2400_device::lcd_lcdcon2_r), FUNC(s3c2400_device::lcd_lcdcon2_w));
	map(0x00a00008, 0x00a0000b).rw(FUNC(s3c2400_device::lcd_lcdcon3_r), FUNC(s3c2400_device::lcd_lcdcon3_w));
	map(0x00a0000c, 0x00a0000f).rw(FUNC(s3c2400_device::lcd_lcdcon4_r), FUNC(s3c2400_device::lcd_lcdcon4_w));
	map(0x00a00010, 0x00a00013).rw(FUNC(s3c2400_device::lcd_lcdcon5_r), FUNC(s3c2400_device::lcd_lcdcon5_w));
	map(0x00a00014, 0x00a00017).rw(FUNC(s3c2400_device::lcd_lcdsaddr1_r), FUNC(s3c2400_device::lcd_lcdsaddr1_w));
	map(0x00a00018, 0x00a0001b).rw(FUNC(s3c2400_device::lcd_lcdsaddr2_r), FUNC(s3c2400_device::lcd_lcdsaddr2_w));
	map(0x00a0001c, 0x00a0001f).rw(FUNC(s3c2400_device::lcd_lcdsaddr3_r), FUNC(s3c2400_device::lcd_lcdsaddr3_w));
	map(0x00a00020, 0x00a00023).rw(FUNC(s3c2400_device::lcd_redlut_r), FUNC(s3c2400_device::lcd_redlut_w));
	map(0x00a00024, 0x00a00027).rw(FUNC(s3c2400_device::lcd_greenlut_r), FUNC(s3c2400_device::lcd_greenlut_w));
	map(0x00a00028, 0x00a0002b).rw(FUNC(s3c2400_device::lcd_bluelut_r), FUNC(s3c2400_device::lcd_bluelut_w));
	map(0x00a0002c, 0x00a0002f).rw(FUNC(s3c2400_device::lcd_dp12_r), FUNC(s3c2400_device::lcd_dp12_w));
	map(0x00a00030, 0x00a00033).rw(FUNC(s3c2400_device::lcd_dp47_r), FUNC(s3c2400_device::lcd_dp47_w));
	map(0x00a00034, 0x00a00037).rw(FUNC(s3c2400_device::lcd_dp35_r), FUNC(s3c2400_device::lcd_dp35_w));
	map(0x00a00038, 0x00a0003b).rw(FUNC(s3c2400_device::lcd_dp23_r), FUNC(s3c2400_device::lcd_dp23_w));
	map(0x00a0003c, 0x00a0003f).rw(FUNC(s3c2400_device::lcd_dp57_r), FUNC(s3c2400_device::lcd_dp57_w));
	map(0x00a00040, 0x00a00043).rw(FUNC(s3c2400_device::lcd_dp34_r), FUNC(s3c2400_device::lcd_dp34_w));
	map(0x00a00044, 0x00a00047).rw(FUNC(s3c2400_device::lcd_dp45_r), FUNC(s3c2400_device::lcd_dp45_w));
	map(0x00a00048, 0x00a0004b).rw(FUNC(s3c2400_device::lcd_dp67_r), FUNC(s3c2400_device::lcd_dp67_w));
	map(0x00a0004c, 0x00a0004f).rw(FUNC(s3c2400_device::lcd_dithmode_r), FUNC(s3c2400_device::lcd_dithmode_w));
	map(0x00a00050, 0x00a00053).rw(FUNC(s3c2400_device::lcd_tpal_r), FUNC(s3c2400_device::lcd_tpal_w));
	map(0x00a00400, 0x00a007ff).rw(FUNC(s3c2400_device::lcd_palette_r), FUNC(s3c2400_device::lcd_palette_w));

	map(0x01000000, 0x01000003).rw(FUNC(s3c2400_device::uart_ulcon_r<0>), FUNC(s3c2400_device::uart_ulcon_w<0>));
	map(0x01000004, 0x01000007).rw(FUNC(s3c2400_device::uart_ucon_r<0>), FUNC(s3c2400_device::uart_ucon_w<0>));
	map(0x01000008, 0x0100000b).rw(FUNC(s3c2400_device::uart_ufcon_r<0>), FUNC(s3c2400_device::uart_ufcon_w<0>));
	map(0x0100000c, 0x0100000f).rw(FUNC(s3c2400_device::uart_umcon_r<0>), FUNC(s3c2400_device::uart_umcon_w<0>));
	map(0x01000010, 0x01000013).rw(FUNC(s3c2400_device::uart_utrstat_r<0>), FUNC(s3c2400_device::uart_utrstat_w<0>));
	map(0x01000014, 0x01000017).rw(FUNC(s3c2400_device::uart_uerstat_r<0>), FUNC(s3c2400_device::uart_uerstat_w<0>));
	map(0x01000018, 0x0100001b).rw(FUNC(s3c2400_device::uart_ufstat_r<0>), FUNC(s3c2400_device::uart_ufstat_w<0>));
	map(0x0100001c, 0x0100001f).rw(FUNC(s3c2400_device::uart_umstat_r<0>), FUNC(s3c2400_device::uart_umstat_w<0>));
	map(0x01000020, 0x01000023).rw(FUNC(s3c2400_device::uart_utxh_r<0>), FUNC(s3c2400_device::uart_utxh_w<0>));
	map(0x01000024, 0x01000027).rw(FUNC(s3c2400_device::uart_urxh_r<0>), FUNC(s3c2400_device::uart_urxh_w<0>));
	map(0x01000028, 0x0100002b).rw(FUNC(s3c2400_device::uart_ubrdiv_r<0>), FUNC(s3c2400_device::uart_ubrdiv_w<0>));
	map(0x01004000, 0x01004003).rw(FUNC(s3c2400_device::uart_ulcon_r<1>), FUNC(s3c2400_device::uart_ulcon_w<1>));
	map(0x01004004, 0x01004007).rw(FUNC(s3c2400_device::uart_ucon_r<1>), FUNC(s3c2400_device::uart_ucon_w<1>));
	map(0x01004008, 0x0100400b).rw(FUNC(s3c2400_device::uart_ufcon_r<1>), FUNC(s3c2400_device::uart_ufcon_w<1>));
	map(0x0100400c, 0x0100400f).rw(FUNC(s3c2400_device::uart_umcon_r<1>), FUNC(s3c2400_device::uart_umcon_w<1>));
	map(0x01004010, 0x01004013).rw(FUNC(s3c2400_device::uart_utrstat_r<1>), FUNC(s3c2400_device::uart_utrstat_w<1>));
	map(0x01004014, 0x01004017).rw(FUNC(s3c2400_device::uart_uerstat_r<1>), FUNC(s3c2400_device::uart_uerstat_w<1>));
	map(0x01004018, 0x0100401b).rw(FUNC(s3c2400_device::uart_ufstat_r<1>), FUNC(s3c2400_device::uart_ufstat_w<1>));
	map(0x0100401c, 0x0100401f).rw(FUNC(s3c2400_device::uart_umstat_r<1>), FUNC(s3c2400_device::uart_umstat_w<1>));
	map(0x01004020, 0x01004023).rw(FUNC(s3c2400_device::uart_utxh_r<1>), FUNC(s3c2400_device::uart_utxh_w<1>));
	map(0x01004024, 0x01004027).rw(FUNC(s3c2400_device::uart_urxh_r<1>), FUNC(s3c2400_device::uart_urxh_w<1>));
	map(0x01004028, 0x0100402b).rw(FUNC(s3c2400_device::uart_ubrdiv_r<1>), FUNC(s3c2400_device::uart_ubrdiv_w<1>));

	map(0x01100000, 0x01100003).rw(FUNC(s3c2400_device::pwm_tcfg0_r), FUNC(s3c2400_device::pwm_tcfg0_w));
	map(0x01100004, 0x01100007).rw(FUNC(s3c2400_device::pwm_tcfg1_r), FUNC(s3c2400_device::pwm_tcfg1_w));
	map(0x01100008, 0x0110000b).rw(FUNC(s3c2400_device::pwm_tcon_r), FUNC(s3c2400_device::pwm_tcon_w));
	map(0x0110000c, 0x0110000f).rw(FUNC(s3c2400_device::pwm_tcntb0_r), FUNC(s3c2400_device::pwm_tcntb0_w));
	map(0x01100010, 0x01100013).rw(FUNC(s3c2400_device::pwm_tcmpb0_r), FUNC(s3c2400_device::pwm_tcmpb0_w));
	map(0x01100014, 0x01100017).rw(FUNC(s3c2400_device::pwm_tcnto0_r), FUNC(s3c2400_device::pwm_tcnto0_w));
	map(0x01100018, 0x0110001b).rw(FUNC(s3c2400_device::pwm_tcntb1_r), FUNC(s3c2400_device::pwm_tcntb1_w));
	map(0x0110001c, 0x0110001f).rw(FUNC(s3c2400_device::pwm_tcmpb1_r), FUNC(s3c2400_device::pwm_tcmpb1_w));
	map(0x01100020, 0x01100023).rw(FUNC(s3c2400_device::pwm_tcnto1_r), FUNC(s3c2400_device::pwm_tcnto1_w));
	map(0x01100024, 0x01100027).rw(FUNC(s3c2400_device::pwm_tcntb2_r), FUNC(s3c2400_device::pwm_tcntb2_w));
	map(0x01100028, 0x0110002b).rw(FUNC(s3c2400_device::pwm_tcmpb2_r), FUNC(s3c2400_device::pwm_tcmpb2_w));
	map(0x0110002c, 0x0110002f).rw(FUNC(s3c2400_device::pwm_tcnto2_r), FUNC(s3c2400_device::pwm_tcnto2_w));
	map(0x01100030, 0x01100033).rw(FUNC(s3c2400_device::pwm_tcntb3_r), FUNC(s3c2400_device::pwm_tcntb3_w));
	map(0x01100034, 0x01100037).rw(FUNC(s3c2400_device::pwm_tcmpb3_r), FUNC(s3c2400_device::pwm_tcmpb3_w));
	map(0x01100038, 0x0110003b).rw(FUNC(s3c2400_device::pwm_tcnto3_r), FUNC(s3c2400_device::pwm_tcnto3_w));
	map(0x0110003c, 0x0110003f).rw(FUNC(s3c2400_device::pwm_tcntb4_r), FUNC(s3c2400_device::pwm_tcntb4_w));
	map(0x01100040, 0x01100043).rw(FUNC(s3c2400_device::pwm_tcnto4_r), FUNC(s3c2400_device::pwm_tcnto4_w));

	map(0x01200140, 0x01200143).rw(FUNC(s3c2400_device::usbdev_func_addr_r), FUNC(s3c2400_device::usbdev_func_addr_w));
	map(0x01200144, 0x01200147).rw(FUNC(s3c2400_device::usbdev_pwr_r), FUNC(s3c2400_device::usbdev_pwr_w));
	map(0x01200148, 0x0120014b).rw(FUNC(s3c2400_device::usbdev_int_r), FUNC(s3c2400_device::usbdev_int_w));
	map(0x0120014c, 0x0120014f).rw(FUNC(s3c2400_device::usbdev_int_mask_r), FUNC(s3c2400_device::usbdev_int_mask_w));
	map(0x01200150, 0x01200153).rw(FUNC(s3c2400_device::usbdev_frame_num_r), FUNC(s3c2400_device::usbdev_frame_num_w));
	map(0x01200154, 0x01200157).rw(FUNC(s3c2400_device::usbdev_resume_con_r), FUNC(s3c2400_device::usbdev_resume_con_w));
	map(0x01200160, 0x01200163).rw(FUNC(s3c2400_device::usbdev_ep_csr_r<0>), FUNC(s3c2400_device::usbdev_ep_csr_w<0>));
	map(0x01200164, 0x01200167).rw(FUNC(s3c2400_device::usbdev_ep_maxp_r<0>), FUNC(s3c2400_device::usbdev_ep_maxp_w<0>));
	map(0x01200168, 0x0120016b).rw(FUNC(s3c2400_device::usbdev_ep_out_cnt_r<0>), FUNC(s3c2400_device::usbdev_ep_out_cnt_w<0>));
	map(0x0120016c, 0x0120016f).rw(FUNC(s3c2400_device::usbdev_ep0_fifo_r), FUNC(s3c2400_device::usbdev_ep0_fifo_w));
	map(0x01200180, 0x01200183).rw(FUNC(s3c2400_device::usbdev_ep_csr_r<1>), FUNC(s3c2400_device::usbdev_ep_csr_w<1>));
	map(0x01200184, 0x01200187).rw(FUNC(s3c2400_device::usbdev_ep_maxp_r<1>), FUNC(s3c2400_device::usbdev_ep_maxp_w<1>));
	map(0x01200188, 0x0120018b).rw(FUNC(s3c2400_device::usbdev_ep1_fifo_r), FUNC(s3c2400_device::usbdev_ep1_fifo_w));
	map(0x01200190, 0x01200193).rw(FUNC(s3c2400_device::usbdev_ep_csr_r<2>), FUNC(s3c2400_device::usbdev_ep_csr_w<2>));
	map(0x01200194, 0x01200197).rw(FUNC(s3c2400_device::usbdev_ep_maxp_r<2>), FUNC(s3c2400_device::usbdev_ep_maxp_w<2>));
	map(0x01200198, 0x0120019b).rw(FUNC(s3c2400_device::usbdev_ep2_fifo_r), FUNC(s3c2400_device::usbdev_ep2_fifo_w));
	map(0x012001a0, 0x012001a3).rw(FUNC(s3c2400_device::usbdev_ep_csr_r<3>), FUNC(s3c2400_device::usbdev_ep_csr_w<3>));
	map(0x012001a4, 0x012001a7).rw(FUNC(s3c2400_device::usbdev_ep_maxp_r<3>), FUNC(s3c2400_device::usbdev_ep_maxp_w<3>));
	map(0x012001a8, 0x012001ab).rw(FUNC(s3c2400_device::usbdev_ep_out_cnt_r<3>), FUNC(s3c2400_device::usbdev_ep_out_cnt_w<3>));
	map(0x012001ac, 0x012001af).rw(FUNC(s3c2400_device::usbdev_ep3_fifo_r), FUNC(s3c2400_device::usbdev_ep3_fifo_w));
	map(0x012001b0, 0x012001b3).rw(FUNC(s3c2400_device::usbdev_ep_csr_r<4>), FUNC(s3c2400_device::usbdev_ep_csr_w<4>));
	map(0x012001b4, 0x012001b7).rw(FUNC(s3c2400_device::usbdev_ep_maxp_r<4>), FUNC(s3c2400_device::usbdev_ep_maxp_w<4>));
	map(0x012001b8, 0x012001bb).rw(FUNC(s3c2400_device::usbdev_ep_out_cnt_r<4>), FUNC(s3c2400_device::usbdev_ep_out_cnt_w<4>));
	map(0x012001bc, 0x012001bf).rw(FUNC(s3c2400_device::usbdev_ep4_fifo_r), FUNC(s3c2400_device::usbdev_ep4_fifo_w));
	map(0x012001c0, 0x012001c3).rw(FUNC(s3c2400_device::usbdev_dma_con_r), FUNC(s3c2400_device::usbdev_dma_con_w));
	map(0x012001c4, 0x012001c7).rw(FUNC(s3c2400_device::usbdev_dma_unit_r), FUNC(s3c2400_device::usbdev_dma_unit_w));
	map(0x012001c8, 0x012001cb).rw(FUNC(s3c2400_device::usbdev_dma_fifo_r), FUNC(s3c2400_device::usbdev_dma_fifo_w));
	map(0x012001cc, 0x012001cf).rw(FUNC(s3c2400_device::usbdev_dma_tx_r), FUNC(s3c2400_device::usbdev_dma_tx_w));
	map(0x012001f4, 0x012001f7).rw(FUNC(s3c2400_device::usbdev_test_mode_r), FUNC(s3c2400_device::usbdev_test_mode_w));
	map(0x012001f8, 0x012001fb).rw(FUNC(s3c2400_device::usbdev_in_con_r), FUNC(s3c2400_device::usbdev_in_con_w));

	map(0x01300000, 0x01300003).rw(FUNC(s3c2400_device::wdt_wtcon_r), FUNC(s3c2400_device::wdt_wtcon_w));
	map(0x01300004, 0x01300007).rw(FUNC(s3c2400_device::wdt_wtdat_r), FUNC(s3c2400_device::wdt_wtdat_w));
	map(0x01300008, 0x0130000b).rw(FUNC(s3c2400_device::wdt_wtcnt_r), FUNC(s3c2400_device::wdt_wtcnt_w));

	map(0x01400000, 0x01400003).rw(FUNC(s3c2400_device::iic_iiccon_r), FUNC(s3c2400_device::iic_iiccon_w));
	map(0x01400004, 0x01400007).rw(FUNC(s3c2400_device::iic_iicstat_r), FUNC(s3c2400_device::iic_iicstat_w));
	map(0x01400008, 0x0140000b).rw(FUNC(s3c2400_device::iic_iicadd_r), FUNC(s3c2400_device::iic_iicadd_w));
	map(0x0140000c, 0x0140000f).rw(FUNC(s3c2400_device::iic_iicds_r), FUNC(s3c2400_device::iic_iicds_w));

	map(0x01508000, 0x01508003).rw(FUNC(s3c2400_device::iis_iiscon_r), FUNC(s3c2400_device::iis_iiscon_w));
	map(0x01508004, 0x01508007).rw(FUNC(s3c2400_device::iis_iismod_r), FUNC(s3c2400_device::iis_iismod_w));
	map(0x01508008, 0x0150800b).rw(FUNC(s3c2400_device::iis_iispsr_r), FUNC(s3c2400_device::iis_iispsr_w));
	map(0x0150800c, 0x0150800f).rw(FUNC(s3c2400_device::iis_iisfcon_r), FUNC(s3c2400_device::iis_iisfcon_w));
	map(0x01508010, 0x01508013).rw(FUNC(s3c2400_device::iis_iisfifo_r), FUNC(s3c2400_device::iis_iisfifo_w));

	map(0x01600000, 0x01600003).rw(FUNC(s3c2400_device::gpio_gpacon_r), FUNC(s3c2400_device::gpio_gpacon_w));
	map(0x01600004, 0x01600007).rw(FUNC(s3c2400_device::gpio_gpadat_r), FUNC(s3c2400_device::gpio_gpadat_w));
	map(0x01600008, 0x0160000b).rw(FUNC(s3c2400_device::gpio_gpbcon_r), FUNC(s3c2400_device::gpio_gpbcon_w));
	map(0x0160000c, 0x0160000f).rw(FUNC(s3c2400_device::gpio_gpbdat_r), FUNC(s3c2400_device::gpio_gpbdat_w));
	map(0x01600010, 0x01600013).rw(FUNC(s3c2400_device::gpio_gpbup_r), FUNC(s3c2400_device::gpio_gpbup_w));
	map(0x01600014, 0x01600017).rw(FUNC(s3c2400_device::gpio_gpccon_r), FUNC(s3c2400_device::gpio_gpccon_w));
	map(0x01600018, 0x0160001b).rw(FUNC(s3c2400_device::gpio_gpcdat_r), FUNC(s3c2400_device::gpio_gpcdat_w));
	map(0x0160001c, 0x0160001f).rw(FUNC(s3c2400_device::gpio_gpcup_r), FUNC(s3c2400_device::gpio_gpcup_w));
	map(0x01600020, 0x01600023).rw(FUNC(s3c2400_device::gpio_gpdcon_r), FUNC(s3c2400_device::gpio_gpdcon_w));
	map(0x01600024, 0x01600027).rw(FUNC(s3c2400_device::gpio_gpddat_r), FUNC(s3c2400_device::gpio_gpddat_w));
	map(0x01600028, 0x0160002b).rw(FUNC(s3c2400_device::gpio_gpdup_r), FUNC(s3c2400_device::gpio_gpdup_w));
	map(0x0160002c, 0x0160002f).rw(FUNC(s3c2400_device::gpio_gpecon_r), FUNC(s3c2400_device::gpio_gpecon_w));
	map(0x01600030, 0x01600033).rw(FUNC(s3c2400_device::gpio_gpedat_r), FUNC(s3c2400_device::gpio_gpedat_w));
	map(0x01600034, 0x01600037).rw(FUNC(s3c2400_device::gpio_gpeup_r), FUNC(s3c2400_device::gpio_gpeup_w));
	map(0x01600038, 0x0160003b).rw(FUNC(s3c2400_device::gpio_gpfcon_r), FUNC(s3c2400_device::gpio_gpfcon_w));
	map(0x0160003c, 0x0160003f).rw(FUNC(s3c2400_device::gpio_gpfdat_r), FUNC(s3c2400_device::gpio_gpfdat_w));
	map(0x01600040, 0x01600043).rw(FUNC(s3c2400_device::gpio_gpfup_r), FUNC(s3c2400_device::gpio_gpfup_w));
	map(0x01600044, 0x01600047).rw(FUNC(s3c2400_device::gpio_gpgcon_r), FUNC(s3c2400_device::gpio_gpgcon_w));
	map(0x01600048, 0x0160004b).rw(FUNC(s3c2400_device::gpio_gpgdat_r), FUNC(s3c2400_device::gpio_gpgdat_w));
	map(0x0160004c, 0x0160004f).rw(FUNC(s3c2400_device::gpio_gpgup_r), FUNC(s3c2400_device::gpio_gpgup_w));
	map(0x01600050, 0x01600053).rw(FUNC(s3c2400_device::gpio_opencr_r), FUNC(s3c2400_device::gpio_opencr_w));
	map(0x01600054, 0x01600057).rw(FUNC(s3c2400_device::gpio_misccr_r), FUNC(s3c2400_device::gpio_misccr_w));
	map(0x01600058, 0x0160005b).rw(FUNC(s3c2400_device::gpio_extint_r<0>), FUNC(s3c2400_device::gpio_extint_w<0>));

	map(0x01700040, 0x01700043).rw(FUNC(s3c2400_device::rtc_rtccon_r), FUNC(s3c2400_device::rtc_rtccon_w));
	map(0x01700044, 0x01700047).rw(FUNC(s3c2400_device::rtc_ticnt_r), FUNC(s3c2400_device::rtc_ticnt_w));
	map(0x01700050, 0x01700053).rw(FUNC(s3c2400_device::rtc_rtcalm_r), FUNC(s3c2400_device::rtc_rtcalm_w));
	map(0x01700054, 0x01700057).rw(FUNC(s3c2400_device::rtc_almsec_r), FUNC(s3c2400_device::rtc_almsec_w));
	map(0x01700058, 0x0170005b).rw(FUNC(s3c2400_device::rtc_almmin_r), FUNC(s3c2400_device::rtc_almmin_w));
	map(0x0170005c, 0x0170005f).rw(FUNC(s3c2400_device::rtc_almhour_r), FUNC(s3c2400_device::rtc_almhour_w));
	map(0x01700060, 0x01700063).rw(FUNC(s3c2400_device::rtc_almday_r), FUNC(s3c2400_device::rtc_almday_w));
	map(0x01700064, 0x01700067).rw(FUNC(s3c2400_device::rtc_almmon_r), FUNC(s3c2400_device::rtc_almmon_w));
	map(0x01700068, 0x0170006b).rw(FUNC(s3c2400_device::rtc_almyear_r), FUNC(s3c2400_device::rtc_almyear_w));
	map(0x0170006c, 0x0170006f).rw(FUNC(s3c2400_device::rtc_rtcrst_r), FUNC(s3c2400_device::rtc_rtcrst_w));
	map(0x01700070, 0x01700073).rw(FUNC(s3c2400_device::rtc_bcdsec_r), FUNC(s3c2400_device::rtc_bcdsec_w));
	map(0x01700074, 0x01700077).rw(FUNC(s3c2400_device::rtc_bcdmin_r), FUNC(s3c2400_device::rtc_bcdmin_w));
	map(0x01700078, 0x0170007b).rw(FUNC(s3c2400_device::rtc_bcdhour_r), FUNC(s3c2400_device::rtc_bcdhour_w));
	map(0x0170007c, 0x0170007f).rw(FUNC(s3c2400_device::rtc_bcddate_r), FUNC(s3c2400_device::rtc_bcddate_w));
	map(0x01700080, 0x01700083).rw(FUNC(s3c2400_device::rtc_bcdday_r), FUNC(s3c2400_device::rtc_bcdday_w));
	map(0x01700084, 0x01700087).rw(FUNC(s3c2400_device::rtc_bcdmon_r), FUNC(s3c2400_device::rtc_bcdmon_w));
	map(0x01700088, 0x0170008b).rw(FUNC(s3c2400_device::rtc_bcdyear_r), FUNC(s3c2400_device::rtc_bcdyear_w));

	map(0x01800000, 0x01800003).rw(FUNC(s3c2400_device::adc_adccon_r), FUNC(s3c2400_device::adc_adccon_w));
	map(0x01800004, 0x01800007).rw(FUNC(s3c2400_device::adc_adcdat_r), FUNC(s3c2400_device::adc_adcdat_w));

	map(0x01900000, 0x01900003).rw(FUNC(s3c2400_device::spi_spcon_r<0>), FUNC(s3c2400_device::spi_spcon_w<0>));
	map(0x01900004, 0x01900007).rw(FUNC(s3c2400_device::spi_spsta_r<0>), FUNC(s3c2400_device::spi_spsta_w<0>));
	map(0x01900008, 0x0190000b).rw(FUNC(s3c2400_device::spi_sppin_r<0>), FUNC(s3c2400_device::spi_sppin_w<0>));
	map(0x0190000c, 0x0190000f).rw(FUNC(s3c2400_device::spi_sppre_r<0>), FUNC(s3c2400_device::spi_sppre_w<0>));
	map(0x01900010, 0x01900013).rw(FUNC(s3c2400_device::spi_sptdat_r<0>), FUNC(s3c2400_device::spi_sptdat_w<0>));
	map(0x01900014, 0x01900017).rw(FUNC(s3c2400_device::spi_sprdat_r<0>), FUNC(s3c2400_device::spi_sprdat_w<0>));

	map(0x01a00000, 0x01a00003).rw(FUNC(s3c2400_device::mmc_mmcon_r), FUNC(s3c2400_device::mmc_mmcon_w));
	map(0x01a00004, 0x01a00007).rw(FUNC(s3c2400_device::mmc_mmcrr_r), FUNC(s3c2400_device::mmc_mmcrr_w));
	map(0x01a00008, 0x01a0000b).rw(FUNC(s3c2400_device::mmc_mmfcon_r), FUNC(s3c2400_device::mmc_mmfcon_w));
	map(0x01a0000c, 0x01a0000f).rw(FUNC(s3c2400_device::mmc_mmsta_r), FUNC(s3c2400_device::mmc_mmsta_w));
	map(0x01a00010, 0x01a00013).rw(FUNC(s3c2400_device::mmc_mmfsta_r), FUNC(s3c2400_device::mmc_mmfsta_w));
	map(0x01a00014, 0x01a00017).rw(FUNC(s3c2400_device::mmc_mmpre_r), FUNC(s3c2400_device::mmc_mmpre_w));
	map(0x01a00018, 0x01a0001b).rw(FUNC(s3c2400_device::mmc_mmlen_r), FUNC(s3c2400_device::mmc_mmlen_w));
	map(0x01a0001c, 0x01a0001f).rw(FUNC(s3c2400_device::mmc_mmcr7_r), FUNC(s3c2400_device::mmc_mmcr7_w));
	map(0x01a00020, 0x01a00023).rw(FUNC(s3c2400_device::mmc_mmrsp_r<0>), FUNC(s3c2400_device::mmc_mmrsp_w<0>));
	map(0x01a00024, 0x01a00027).rw(FUNC(s3c2400_device::mmc_mmrsp_r<1>), FUNC(s3c2400_device::mmc_mmrsp_w<1>));
	map(0x01a00028, 0x01a0002b).rw(FUNC(s3c2400_device::mmc_mmrsp_r<2>), FUNC(s3c2400_device::mmc_mmrsp_w<2>));
	map(0x01a0002c, 0x01a0002f).rw(FUNC(s3c2400_device::mmc_mmrsp_r<3>), FUNC(s3c2400_device::mmc_mmrsp_w<3>));
	map(0x01a00030, 0x01a00033).rw(FUNC(s3c2400_device::mmc_mmcmd_r<0>), FUNC(s3c2400_device::mmc_mmcmd_w<0>));
	map(0x01a00034, 0x01a00037).rw(FUNC(s3c2400_device::mmc_mmcmd_r<1>), FUNC(s3c2400_device::mmc_mmcmd_w<1>));
	map(0x01a00038, 0x01a0003b).rw(FUNC(s3c2400_device::mmc_mmcr16_r), FUNC(s3c2400_device::mmc_mmcr16_w));
	map(0x01a0003c, 0x01a0003f).rw(FUNC(s3c2400_device::mmc_mmdat_r), FUNC(s3c2400_device::mmc_mmdat_w));
}

void s3c2410_device::map(address_map &map)
{
	map(0x08000000, 0x08000003).rw(FUNC(s3c2410_device::memcon_bwscon_r), FUNC(s3c2410_device::memcon_bwscon_w));
	map(0x08000004, 0x08000007).rw(FUNC(s3c2410_device::memcon_bankcon_r<0>), FUNC(s3c2410_device::memcon_bankcon_w<0>));
	map(0x08000008, 0x0800000b).rw(FUNC(s3c2410_device::memcon_bankcon_r<1>), FUNC(s3c2410_device::memcon_bankcon_w<1>));
	map(0x0800000c, 0x0800000f).rw(FUNC(s3c2410_device::memcon_bankcon_r<2>), FUNC(s3c2410_device::memcon_bankcon_w<2>));
	map(0x08000010, 0x08000013).rw(FUNC(s3c2410_device::memcon_bankcon_r<3>), FUNC(s3c2410_device::memcon_bankcon_w<3>));
	map(0x08000014, 0x08000017).rw(FUNC(s3c2410_device::memcon_bankcon_r<4>), FUNC(s3c2410_device::memcon_bankcon_w<4>));
	map(0x08000018, 0x0800001b).rw(FUNC(s3c2410_device::memcon_bankcon_r<5>), FUNC(s3c2410_device::memcon_bankcon_w<5>));
	map(0x0800001c, 0x0800001f).rw(FUNC(s3c2410_device::memcon_bankcon_r<6>), FUNC(s3c2410_device::memcon_bankcon_w<6>));
	map(0x08000020, 0x08000023).rw(FUNC(s3c2410_device::memcon_bankcon_r<7>), FUNC(s3c2410_device::memcon_bankcon_w<7>));
	map(0x08000024, 0x08000027).rw(FUNC(s3c2410_device::memcon_refresh_r), FUNC(s3c2410_device::memcon_refresh_w));
	map(0x08000028, 0x0800002b).rw(FUNC(s3c2410_device::memcon_banksize_r), FUNC(s3c2410_device::memcon_banksize_w));
	map(0x0800002c, 0x0800002f).rw(FUNC(s3c2410_device::memcon_mrsrb6_r), FUNC(s3c2410_device::memcon_mrsrb6_w));
	map(0x08000030, 0x08000033).rw(FUNC(s3c2410_device::memcon_mrsrb7_r), FUNC(s3c2410_device::memcon_mrsrb7_w));

	map(0x09000000, 0x09000003).r(FUNC(s3c2410_device::usbhost_rev_r));
	map(0x09000004, 0x09000007).rw(FUNC(s3c2410_device::usbhost_ctrl_r), FUNC(s3c2410_device::usbhost_ctrl_w));
	map(0x09000008, 0x0900000b).rw(FUNC(s3c2410_device::usbhost_common_stat_r), FUNC(s3c2410_device::usbhost_common_stat_w));
	map(0x0900000c, 0x0900000f).rw(FUNC(s3c2410_device::usbhost_int_stat_r), FUNC(s3c2410_device::usbhost_int_stat_w));
	map(0x09000010, 0x09000013).rw(FUNC(s3c2410_device::usbhost_int_en_r), FUNC(s3c2410_device::usbhost_int_en_w));
	map(0x09000014, 0x09000017).rw(FUNC(s3c2410_device::usbhost_int_dis_r), FUNC(s3c2410_device::usbhost_int_dis_w));
	map(0x09000018, 0x0900001b).rw(FUNC(s3c2410_device::usbhost_hcca_r), FUNC(s3c2410_device::usbhost_hcca_w));
	map(0x0900001c, 0x0900001f).rw(FUNC(s3c2410_device::usbhost_period_curr_r), FUNC(s3c2410_device::usbhost_period_curr_w));
	map(0x09000020, 0x09000023).rw(FUNC(s3c2410_device::usbhost_ctrl_head_r), FUNC(s3c2410_device::usbhost_ctrl_head_w));
	map(0x09000024, 0x09000027).rw(FUNC(s3c2410_device::usbhost_ctrl_curr_r), FUNC(s3c2410_device::usbhost_ctrl_curr_w));
	map(0x09000028, 0x0900002b).rw(FUNC(s3c2410_device::usbhost_bulk_head_r), FUNC(s3c2410_device::usbhost_bulk_head_w));
	map(0x0900002c, 0x0900002f).rw(FUNC(s3c2410_device::usbhost_bulk_curr_r), FUNC(s3c2410_device::usbhost_bulk_curr_w));
	map(0x09000030, 0x09000033).rw(FUNC(s3c2410_device::usbhost_done_head_r), FUNC(s3c2410_device::usbhost_done_head_w));
	map(0x09000034, 0x09000037).rw(FUNC(s3c2410_device::usbhost_rm_interval_r), FUNC(s3c2410_device::usbhost_rm_interval_w));
	map(0x09000038, 0x0900003b).rw(FUNC(s3c2410_device::usbhost_fm_remaining_r), FUNC(s3c2410_device::usbhost_fm_remaining_w));
	map(0x0900003c, 0x0900003f).rw(FUNC(s3c2410_device::usbhost_fm_number_r), FUNC(s3c2410_device::usbhost_fm_number_w));
	map(0x09000040, 0x09000043).rw(FUNC(s3c2410_device::usbhost_periodic_start_r), FUNC(s3c2410_device::usbhost_periodic_start_w));
	map(0x09000044, 0x09000047).rw(FUNC(s3c2410_device::usbhost_ls_thresh_r), FUNC(s3c2410_device::usbhost_ls_thresh_w));
	map(0x09000048, 0x0900004b).rw(FUNC(s3c2410_device::usbhost_rh_desc_a_r), FUNC(s3c2410_device::usbhost_rh_desc_a_w));
	map(0x0900004c, 0x0900004f).rw(FUNC(s3c2410_device::usbhost_rh_desc_b_r), FUNC(s3c2410_device::usbhost_rh_desc_b_w));
	map(0x09000050, 0x09000053).rw(FUNC(s3c2410_device::usbhost_rh_status_r), FUNC(s3c2410_device::usbhost_rh_status_w));
	map(0x09000054, 0x09000057).rw(FUNC(s3c2410_device::usbhost_rh_port_stat_1_r), FUNC(s3c2410_device::usbhost_rh_port_stat_1_w));
	map(0x09000058, 0x0900005b).rw(FUNC(s3c2410_device::usbhost_rh_port_stat_2_r), FUNC(s3c2410_device::usbhost_rh_port_stat_2_w));

	map(0x0a000000, 0x0a000003).rw(FUNC(s3c2410_device::irq_srcpnd_r), FUNC(s3c2410_device::irq_srcpnd_w));
	map(0x0a000004, 0x0a000007).rw(FUNC(s3c2410_device::irq_intmod_r), FUNC(s3c2410_device::irq_intmod_w));
	map(0x0a000008, 0x0a00000b).rw(FUNC(s3c2410_device::irq_intmsk_r), FUNC(s3c2410_device::irq_intmsk_w));
	map(0x0a00000c, 0x0a00000f).rw(FUNC(s3c2410_device::irq_priority_r), FUNC(s3c2410_device::irq_priority_w));
	map(0x0a000010, 0x0a000013).rw(FUNC(s3c2410_device::irq_intpnd_r), FUNC(s3c2410_device::irq_intpnd_w));
	map(0x0a000014, 0x0a000017).rw(FUNC(s3c2410_device::irq_intoffset_r), FUNC(s3c2410_device::irq_intoffset_w));
	map(0x0a000018, 0x0a00001b).rw(FUNC(s3c2410_device::irq_subsrcpnd_r), FUNC(s3c2410_device::irq_subsrcpnd_w));
	map(0x0a00001c, 0x0a00001f).rw(FUNC(s3c2410_device::irq_intsubmsk_r), FUNC(s3c2410_device::irq_intsubmsk_w));

	map(0x0b000000, 0x0b000003).rw(FUNC(s3c2410_device::dma_disrc_r<0>), FUNC(s3c2410_device::dma_disrc_w<0>));
	map(0x0b000004, 0x0b000007).rw(FUNC(s3c2410_device::dma_disrcc_r<0>), FUNC(s3c2410_device::dma_disrcc_w<0>));
	map(0x0b000008, 0x0b00000b).rw(FUNC(s3c2410_device::dma_didst_r<0>), FUNC(s3c2410_device::dma_didst_w<0>));
	map(0x0b00000c, 0x0b00000f).rw(FUNC(s3c2410_device::dma_didstc_r<0>), FUNC(s3c2410_device::dma_didstc_w<0>));
	map(0x0b000010, 0x0b000013).rw(FUNC(s3c2410_device::dma_dcon_r<0>), FUNC(s3c2410_device::dma_dcon_w<0>));
	map(0x0b000014, 0x0b000017).rw(FUNC(s3c2410_device::dma_dstat_r<0>), FUNC(s3c2410_device::dma_dstat_w<0>));
	map(0x0b000018, 0x0b00001b).rw(FUNC(s3c2410_device::dma_dcsrc_r<0>), FUNC(s3c2410_device::dma_dcsrc_w<0>));
	map(0x0b00001c, 0x0b00001f).rw(FUNC(s3c2410_device::dma_dcdst_r<0>), FUNC(s3c2410_device::dma_dcdst_w<0>));
	map(0x0b000020, 0x0b000023).rw(FUNC(s3c2410_device::dma_dmasktrig_r<0>), FUNC(s3c2410_device::dma_dmasktrig_w<0>));
	map(0x0b000040, 0x0b000043).rw(FUNC(s3c2410_device::dma_disrc_r<1>), FUNC(s3c2410_device::dma_disrc_w<1>));
	map(0x0b000044, 0x0b000047).rw(FUNC(s3c2410_device::dma_disrcc_r<1>), FUNC(s3c2410_device::dma_disrcc_w<1>));
	map(0x0b000048, 0x0b00004b).rw(FUNC(s3c2410_device::dma_didst_r<1>), FUNC(s3c2410_device::dma_didst_w<1>));
	map(0x0b00004c, 0x0b00004f).rw(FUNC(s3c2410_device::dma_didstc_r<1>), FUNC(s3c2410_device::dma_didstc_w<1>));
	map(0x0b000050, 0x0b000053).rw(FUNC(s3c2410_device::dma_dcon_r<1>), FUNC(s3c2410_device::dma_dcon_w<1>));
	map(0x0b000054, 0x0b000057).rw(FUNC(s3c2410_device::dma_dstat_r<1>), FUNC(s3c2410_device::dma_dstat_w<1>));
	map(0x0b000058, 0x0b00005b).rw(FUNC(s3c2410_device::dma_dcsrc_r<1>), FUNC(s3c2410_device::dma_dcsrc_w<1>));
	map(0x0b00005c, 0x0b00005f).rw(FUNC(s3c2410_device::dma_dcdst_r<1>), FUNC(s3c2410_device::dma_dcdst_w<1>));
	map(0x0b000060, 0x0b000063).rw(FUNC(s3c2410_device::dma_dmasktrig_r<1>), FUNC(s3c2410_device::dma_dmasktrig_w<1>));
	map(0x0b000080, 0x0b000083).rw(FUNC(s3c2410_device::dma_disrc_r<2>), FUNC(s3c2410_device::dma_disrc_w<2>));
	map(0x0b000084, 0x0b000087).rw(FUNC(s3c2410_device::dma_disrcc_r<2>), FUNC(s3c2410_device::dma_disrcc_w<2>));
	map(0x0b000088, 0x0b00008b).rw(FUNC(s3c2410_device::dma_didst_r<2>), FUNC(s3c2410_device::dma_didst_w<2>));
	map(0x0b00008c, 0x0b00008f).rw(FUNC(s3c2410_device::dma_didstc_r<2>), FUNC(s3c2410_device::dma_didstc_w<2>));
	map(0x0b000090, 0x0b000093).rw(FUNC(s3c2410_device::dma_dcon_r<2>), FUNC(s3c2410_device::dma_dcon_w<2>));
	map(0x0b000094, 0x0b000097).rw(FUNC(s3c2410_device::dma_dstat_r<2>), FUNC(s3c2410_device::dma_dstat_w<2>));
	map(0x0b000098, 0x0b00009b).rw(FUNC(s3c2410_device::dma_dcsrc_r<2>), FUNC(s3c2410_device::dma_dcsrc_w<2>));
	map(0x0b00009c, 0x0b00009f).rw(FUNC(s3c2410_device::dma_dcdst_r<2>), FUNC(s3c2410_device::dma_dcdst_w<2>));
	map(0x0b0000a0, 0x0b0000a3).rw(FUNC(s3c2410_device::dma_dmasktrig_r<2>), FUNC(s3c2410_device::dma_dmasktrig_w<2>));
	map(0x0b0000c0, 0x0b0000c3).rw(FUNC(s3c2410_device::dma_disrc_r<3>), FUNC(s3c2410_device::dma_disrc_w<3>));
	map(0x0b0000c4, 0x0b0000c7).rw(FUNC(s3c2410_device::dma_disrcc_r<3>), FUNC(s3c2410_device::dma_disrcc_w<3>));
	map(0x0b0000c8, 0x0b0000cb).rw(FUNC(s3c2410_device::dma_didst_r<3>), FUNC(s3c2410_device::dma_didst_w<3>));
	map(0x0b0000cc, 0x0b0000cf).rw(FUNC(s3c2410_device::dma_didstc_r<3>), FUNC(s3c2410_device::dma_didstc_w<3>));
	map(0x0b0000d0, 0x0b0000d3).rw(FUNC(s3c2410_device::dma_dcon_r<3>), FUNC(s3c2410_device::dma_dcon_w<3>));
	map(0x0b0000d4, 0x0b0000d7).rw(FUNC(s3c2410_device::dma_dstat_r<3>), FUNC(s3c2410_device::dma_dstat_w<3>));
	map(0x0b0000d8, 0x0b0000db).rw(FUNC(s3c2410_device::dma_dcsrc_r<3>), FUNC(s3c2410_device::dma_dcsrc_w<3>));
	map(0x0b0000dc, 0x0b0000df).rw(FUNC(s3c2410_device::dma_dcdst_r<3>), FUNC(s3c2410_device::dma_dcdst_w<3>));
	map(0x0b0000e0, 0x0b0000e3).rw(FUNC(s3c2410_device::dma_dmasktrig_r<3>), FUNC(s3c2410_device::dma_dmasktrig_w<3>));

	map(0x0c000000, 0x0c000003).rw(FUNC(s3c2410_device::clkpow_locktime_r), FUNC(s3c2410_device::clkpow_locktime_w));
	map(0x0c000004, 0x0c000007).rw(FUNC(s3c2410_device::clkpow_mpllcon_r), FUNC(s3c2410_device::clkpow_mpllcon_w));
	map(0x0c000008, 0x0c00000b).rw(FUNC(s3c2410_device::clkpow_upllcon_r), FUNC(s3c2410_device::clkpow_upllcon_w));
	map(0x0c00000c, 0x0c00000f).rw(FUNC(s3c2410_device::clkpow_clkcon_r), FUNC(s3c2410_device::clkpow_clkcon_w));
	map(0x0c000010, 0x0c000013).rw(FUNC(s3c2410_device::clkpow_clkslow_r), FUNC(s3c2410_device::clkpow_clkslow_w));
	map(0x0c000014, 0x0c000017).rw(FUNC(s3c2410_device::clkpow_clkdivn_r), FUNC(s3c2410_device::clkpow_clkdivn_w));

	map(0x0d000000, 0x0d000003).rw(FUNC(s3c2410_device::lcd_lcdcon1_r), FUNC(s3c2410_device::lcd_lcdcon1_w));
	map(0x0d000004, 0x0d000007).rw(FUNC(s3c2410_device::lcd_lcdcon2_r), FUNC(s3c2410_device::lcd_lcdcon2_w));
	map(0x0d000008, 0x0d00000b).rw(FUNC(s3c2410_device::lcd_lcdcon3_r), FUNC(s3c2410_device::lcd_lcdcon3_w));
	map(0x0d00000c, 0x0d00000f).rw(FUNC(s3c2410_device::lcd_lcdcon4_r), FUNC(s3c2410_device::lcd_lcdcon4_w));
	map(0x0d000010, 0x0d000013).rw(FUNC(s3c2410_device::lcd_lcdcon5_r), FUNC(s3c2410_device::lcd_lcdcon5_w));
	map(0x0d000014, 0x0d000017).rw(FUNC(s3c2410_device::lcd_lcdsaddr1_r), FUNC(s3c2410_device::lcd_lcdsaddr1_w));
	map(0x0d000018, 0x0d00001b).rw(FUNC(s3c2410_device::lcd_lcdsaddr2_r), FUNC(s3c2410_device::lcd_lcdsaddr2_w));
	map(0x0d00001c, 0x0d00001f).rw(FUNC(s3c2410_device::lcd_lcdsaddr3_r), FUNC(s3c2410_device::lcd_lcdsaddr3_w));
	map(0x0d000020, 0x0d000023).rw(FUNC(s3c2410_device::lcd_redlut_r), FUNC(s3c2410_device::lcd_redlut_w));
	map(0x0d000024, 0x0d000027).rw(FUNC(s3c2410_device::lcd_greenlut_r), FUNC(s3c2410_device::lcd_greenlut_w));
	map(0x0d000028, 0x0d00002b).rw(FUNC(s3c2410_device::lcd_bluelut_r), FUNC(s3c2410_device::lcd_bluelut_w));
	map(0x0d00004c, 0x0d00004f).rw(FUNC(s3c2410_device::lcd_dithmode_r), FUNC(s3c2410_device::lcd_dithmode_w));
	map(0x0d000050, 0x0d000053).rw(FUNC(s3c2410_device::lcd_tpal_r), FUNC(s3c2410_device::lcd_tpal_w));
	map(0x0d000054, 0x0d000057).rw(FUNC(s3c2410_device::lcd_lcdintpnd_r), FUNC(s3c2410_device::lcd_lcdintpnd_w));
	map(0x0d000058, 0x0d00005b).rw(FUNC(s3c2410_device::lcd_lcdsrcpnd_r), FUNC(s3c2410_device::lcd_lcdsrcpnd_w));
	map(0x0d00005c, 0x0d00005f).rw(FUNC(s3c2410_device::lcd_lcdintmsk_r), FUNC(s3c2410_device::lcd_lcdintmsk_w));
	map(0x0d000060, 0x0d000063).rw(FUNC(s3c2410_device::lcd_lpcsel_r), FUNC(s3c2410_device::lcd_lpcsel_w));
	map(0x0d000400, 0x0d0007ff).rw(FUNC(s3c2410_device::lcd_palette_r), FUNC(s3c2410_device::lcd_palette_w));

	map(0x0e000000, 0x0e000003).rw(FUNC(s3c2410_device::nand_nfconf_r), FUNC(s3c2410_device::nand_nfconf_w));
	map(0x0e000004, 0x0e000007).rw(FUNC(s3c2410_device::nand_nfcmd_r), FUNC(s3c2410_device::nand_nfcmd_w));
	map(0x0e000008, 0x0e00000b).rw(FUNC(s3c2410_device::nand_nfaddr_r), FUNC(s3c2410_device::nand_nfaddr_w));
	map(0x0e00000c, 0x0e00000f).rw(FUNC(s3c2410_device::nand_nfdata_r), FUNC(s3c2410_device::nand_nfdata_w));
	map(0x0e000010, 0x0e000013).rw(FUNC(s3c2410_device::nand_nfstat_r), FUNC(s3c2410_device::nand_nfstat_w));
	map(0x0e000014, 0x0e000017).rw(FUNC(s3c2410_device::nand_nfecc_r), FUNC(s3c2410_device::nand_nfecc_w));

	map(0x10000000, 0x10000003).rw(FUNC(s3c2410_device::uart_ulcon_r<0>), FUNC(s3c2410_device::uart_ulcon_w<0>));
	map(0x10000004, 0x10000007).rw(FUNC(s3c2410_device::uart_ucon_r<0>), FUNC(s3c2410_device::uart_ucon_w<0>));
	map(0x10000008, 0x1000000b).rw(FUNC(s3c2410_device::uart_ufcon_r<0>), FUNC(s3c2410_device::uart_ufcon_w<0>));
	map(0x1000000c, 0x1000000f).rw(FUNC(s3c2410_device::uart_umcon_r<0>), FUNC(s3c2410_device::uart_umcon_w<0>));
	map(0x10000010, 0x10000013).rw(FUNC(s3c2410_device::uart_utrstat_r<0>), FUNC(s3c2410_device::uart_utrstat_w<0>));
	map(0x10000014, 0x10000017).rw(FUNC(s3c2410_device::uart_uerstat_r<0>), FUNC(s3c2410_device::uart_uerstat_w<0>));
	map(0x10000018, 0x1000001b).rw(FUNC(s3c2410_device::uart_ufstat_r<0>), FUNC(s3c2410_device::uart_ufstat_w<0>));
	map(0x1000001c, 0x1000001f).rw(FUNC(s3c2410_device::uart_umstat_r<0>), FUNC(s3c2410_device::uart_umstat_w<0>));
	map(0x10000020, 0x10000023).rw(FUNC(s3c2410_device::uart_utxh_r<0>), FUNC(s3c2410_device::uart_utxh_w<0>));
	map(0x10000024, 0x10000027).rw(FUNC(s3c2410_device::uart_urxh_r<0>), FUNC(s3c2410_device::uart_urxh_w<0>));
	map(0x10000028, 0x1000002b).rw(FUNC(s3c2410_device::uart_ubrdiv_r<0>), FUNC(s3c2410_device::uart_ubrdiv_w<0>));
	map(0x10004000, 0x10004003).rw(FUNC(s3c2410_device::uart_ulcon_r<1>), FUNC(s3c2410_device::uart_ulcon_w<1>));
	map(0x10004004, 0x10004007).rw(FUNC(s3c2410_device::uart_ucon_r<1>), FUNC(s3c2410_device::uart_ucon_w<1>));
	map(0x10004008, 0x1000400b).rw(FUNC(s3c2410_device::uart_ufcon_r<1>), FUNC(s3c2410_device::uart_ufcon_w<1>));
	map(0x1000400c, 0x1000400f).rw(FUNC(s3c2410_device::uart_umcon_r<1>), FUNC(s3c2410_device::uart_umcon_w<1>));
	map(0x10004010, 0x10004013).rw(FUNC(s3c2410_device::uart_utrstat_r<1>), FUNC(s3c2410_device::uart_utrstat_w<1>));
	map(0x10004014, 0x10004017).rw(FUNC(s3c2410_device::uart_uerstat_r<1>), FUNC(s3c2410_device::uart_uerstat_w<1>));
	map(0x10004018, 0x1000401b).rw(FUNC(s3c2410_device::uart_ufstat_r<1>), FUNC(s3c2410_device::uart_ufstat_w<1>));
	map(0x1000401c, 0x1000401f).rw(FUNC(s3c2410_device::uart_umstat_r<1>), FUNC(s3c2410_device::uart_umstat_w<1>));
	map(0x10004020, 0x10004023).rw(FUNC(s3c2410_device::uart_utxh_r<1>), FUNC(s3c2410_device::uart_utxh_w<1>));
	map(0x10004024, 0x10004027).rw(FUNC(s3c2410_device::uart_urxh_r<1>), FUNC(s3c2410_device::uart_urxh_w<1>));
	map(0x10004028, 0x1000402b).rw(FUNC(s3c2410_device::uart_ubrdiv_r<1>), FUNC(s3c2410_device::uart_ubrdiv_w<1>));
	map(0x10008000, 0x10008003).rw(FUNC(s3c2410_device::uart_ulcon_r<2>), FUNC(s3c2410_device::uart_ulcon_w<2>));
	map(0x10008004, 0x10508007).rw(FUNC(s3c2410_device::uart_ucon_r<2>), FUNC(s3c2410_device::uart_ucon_w<2>));
	map(0x10008008, 0x1000800b).rw(FUNC(s3c2410_device::uart_ufcon_r<2>), FUNC(s3c2410_device::uart_ufcon_w<2>));
	map(0x1000800c, 0x1000800f).rw(FUNC(s3c2410_device::uart_umcon_r<2>), FUNC(s3c2410_device::uart_umcon_w<2>));
	map(0x10008010, 0x10008013).rw(FUNC(s3c2410_device::uart_utrstat_r<2>), FUNC(s3c2410_device::uart_utrstat_w<2>));
	map(0x10008014, 0x10008017).rw(FUNC(s3c2410_device::uart_uerstat_r<2>), FUNC(s3c2410_device::uart_uerstat_w<2>));
	map(0x10008018, 0x1000801b).rw(FUNC(s3c2410_device::uart_ufstat_r<2>), FUNC(s3c2410_device::uart_ufstat_w<2>));
	map(0x1000801c, 0x1000801f).rw(FUNC(s3c2410_device::uart_umstat_r<2>), FUNC(s3c2410_device::uart_umstat_w<2>));
	map(0x10008020, 0x10008023).rw(FUNC(s3c2410_device::uart_utxh_r<2>), FUNC(s3c2410_device::uart_utxh_w<2>));
	map(0x10008024, 0x10008027).rw(FUNC(s3c2410_device::uart_urxh_r<2>), FUNC(s3c2410_device::uart_urxh_w<2>));
	map(0x10008028, 0x1000802b).rw(FUNC(s3c2410_device::uart_ubrdiv_r<2>), FUNC(s3c2410_device::uart_ubrdiv_w<2>));

	map(0x11000000, 0x11000003).rw(FUNC(s3c2410_device::pwm_tcfg0_r), FUNC(s3c2410_device::pwm_tcfg0_w));
	map(0x11000004, 0x11000007).rw(FUNC(s3c2410_device::pwm_tcfg1_r), FUNC(s3c2410_device::pwm_tcfg1_w));
	map(0x11000008, 0x1100000b).rw(FUNC(s3c2410_device::pwm_tcon_r), FUNC(s3c2410_device::pwm_tcon_w));
	map(0x1100000c, 0x1100000f).rw(FUNC(s3c2410_device::pwm_tcntb0_r), FUNC(s3c2410_device::pwm_tcntb0_w));
	map(0x11000010, 0x11000013).rw(FUNC(s3c2410_device::pwm_tcmpb0_r), FUNC(s3c2410_device::pwm_tcmpb0_w));
	map(0x11000014, 0x11000017).rw(FUNC(s3c2410_device::pwm_tcnto0_r), FUNC(s3c2410_device::pwm_tcnto0_w));
	map(0x11000018, 0x1100001b).rw(FUNC(s3c2410_device::pwm_tcntb1_r), FUNC(s3c2410_device::pwm_tcntb1_w));
	map(0x1100001c, 0x1100001f).rw(FUNC(s3c2410_device::pwm_tcmpb1_r), FUNC(s3c2410_device::pwm_tcmpb1_w));
	map(0x11000020, 0x11000023).rw(FUNC(s3c2410_device::pwm_tcnto1_r), FUNC(s3c2410_device::pwm_tcnto1_w));
	map(0x11000024, 0x11000027).rw(FUNC(s3c2410_device::pwm_tcntb2_r), FUNC(s3c2410_device::pwm_tcntb2_w));
	map(0x11000028, 0x1100002b).rw(FUNC(s3c2410_device::pwm_tcmpb2_r), FUNC(s3c2410_device::pwm_tcmpb2_w));
	map(0x1100002c, 0x1100002f).rw(FUNC(s3c2410_device::pwm_tcnto2_r), FUNC(s3c2410_device::pwm_tcnto2_w));
	map(0x11000030, 0x11000033).rw(FUNC(s3c2410_device::pwm_tcntb3_r), FUNC(s3c2410_device::pwm_tcntb3_w));
	map(0x11000034, 0x11000037).rw(FUNC(s3c2410_device::pwm_tcmpb3_r), FUNC(s3c2410_device::pwm_tcmpb3_w));
	map(0x11000038, 0x1100003b).rw(FUNC(s3c2410_device::pwm_tcnto3_r), FUNC(s3c2410_device::pwm_tcnto3_w));
	map(0x1100003c, 0x1100003f).rw(FUNC(s3c2410_device::pwm_tcntb4_r), FUNC(s3c2410_device::pwm_tcntb4_w));
	map(0x11000040, 0x11000043).rw(FUNC(s3c2410_device::pwm_tcnto4_r), FUNC(s3c2410_device::pwm_tcnto4_w));

	map(0x12000140, 0x12000143).rw(FUNC(s3c2410_device::usbdev_func_addr_r), FUNC(s3c2410_device::usbdev_func_addr_w));
	map(0x12000144, 0x12000147).rw(FUNC(s3c2410_device::usbdev_pwr_r), FUNC(s3c2410_device::usbdev_pwr_w));
	map(0x12000148, 0x1200014b).rw(FUNC(s3c2410_device::usbdev_ep_int_r), FUNC(s3c2410_device::usbdev_ep_int_w));
	map(0x12000158, 0x1200015b).rw(FUNC(s3c2410_device::usbdev_usb_int_r), FUNC(s3c2410_device::usbdev_usb_int_w));
	map(0x1200015c, 0x1200015f).rw(FUNC(s3c2410_device::usbdev_ep_int_en_r), FUNC(s3c2410_device::usbdev_ep_int_en_w));
	map(0x1200016c, 0x1200016f).rw(FUNC(s3c2410_device::usbdev_usb_int_en_r), FUNC(s3c2410_device::usbdev_usb_int_en_w));
	map(0x12000170, 0x12000173).rw(FUNC(s3c2410_device::usbdev_frame_num1_r), FUNC(s3c2410_device::usbdev_frame_num1_w));
	map(0x12000174, 0x12000177).rw(FUNC(s3c2410_device::usbdev_frame_num2_r), FUNC(s3c2410_device::usbdev_frame_num2_w));
	map(0x12000178, 0x1200017b).rw(FUNC(s3c2410_device::usbdev_index_r), FUNC(s3c2410_device::usbdev_index_w));
	map(0x120001c0, 0x120001c3).rw(FUNC(s3c2410_device::usbdev_ep0_fifo_r), FUNC(s3c2410_device::usbdev_ep0_fifo_w));
	map(0x120001c4, 0x120001c7).rw(FUNC(s3c2410_device::usbdev_ep1_fifo_r), FUNC(s3c2410_device::usbdev_ep1_fifo_w));
	map(0x120001c8, 0x120001cb).rw(FUNC(s3c2410_device::usbdev_ep2_fifo_r), FUNC(s3c2410_device::usbdev_ep2_fifo_w));
	map(0x120001cc, 0x120001cf).rw(FUNC(s3c2410_device::usbdev_ep3_fifo_r), FUNC(s3c2410_device::usbdev_ep3_fifo_w));
	map(0x120001d0, 0x120001d3).rw(FUNC(s3c2410_device::usbdev_ep4_fifo_r), FUNC(s3c2410_device::usbdev_ep4_fifo_w));
	map(0x12000200, 0x12000203).rw(FUNC(s3c2410_device::usbdev_ep_dma_con_r<1>), FUNC(s3c2410_device::usbdev_ep_dma_con_w<1>));
	map(0x12000204, 0x12000207).rw(FUNC(s3c2410_device::usbdev_ep_dma_unit_r<1>), FUNC(s3c2410_device::usbdev_ep_dma_unit_w<1>));
	map(0x12000208, 0x1200020b).rw(FUNC(s3c2410_device::usbdev_ep_dma_fifo_r<1>), FUNC(s3c2410_device::usbdev_ep_dma_fifo_w<1>));
	map(0x1200020c, 0x1200020f).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_r<1>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_w<1>));
	map(0x12000210, 0x12000213).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_r<1>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_w<1>));
	map(0x12000214, 0x12000217).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_r<1>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_w<1>));
	map(0x12000218, 0x1200021b).rw(FUNC(s3c2410_device::usbdev_ep_dma_con_r<2>), FUNC(s3c2410_device::usbdev_ep_dma_con_w<2>));
	map(0x1200021c, 0x1200021f).rw(FUNC(s3c2410_device::usbdev_ep_dma_unit_r<2>), FUNC(s3c2410_device::usbdev_ep_dma_unit_w<2>));
	map(0x12000220, 0x12000223).rw(FUNC(s3c2410_device::usbdev_ep_dma_fifo_r<2>), FUNC(s3c2410_device::usbdev_ep_dma_fifo_w<2>));
	map(0x12000224, 0x12000227).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_r<2>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_w<2>));
	map(0x12000228, 0x1200022b).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_r<2>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_w<2>));
	map(0x1200022c, 0x1200022f).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_r<2>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_w<2>));
	map(0x12000240, 0x12000243).rw(FUNC(s3c2410_device::usbdev_ep_dma_con_r<3>), FUNC(s3c2410_device::usbdev_ep_dma_con_w<3>));
	map(0x12000244, 0x12000247).rw(FUNC(s3c2410_device::usbdev_ep_dma_unit_r<3>), FUNC(s3c2410_device::usbdev_ep_dma_unit_w<3>));
	map(0x12000248, 0x1200024b).rw(FUNC(s3c2410_device::usbdev_ep_dma_fifo_r<3>), FUNC(s3c2410_device::usbdev_ep_dma_fifo_w<3>));
	map(0x1200024c, 0x1200024f).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_r<3>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_w<3>));
	map(0x12000250, 0x12000253).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_r<3>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_w<3>));
	map(0x12000254, 0x12000257).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_r<3>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_w<3>));
	map(0x12000258, 0x1200025b).rw(FUNC(s3c2410_device::usbdev_ep_dma_con_r<4>), FUNC(s3c2410_device::usbdev_ep_dma_con_w<4>));
	map(0x1200025c, 0x1200025f).rw(FUNC(s3c2410_device::usbdev_ep_dma_unit_r<4>), FUNC(s3c2410_device::usbdev_ep_dma_unit_w<4>));
	map(0x12000260, 0x12000263).rw(FUNC(s3c2410_device::usbdev_ep_dma_fifo_r<4>), FUNC(s3c2410_device::usbdev_ep_dma_fifo_w<4>));
	map(0x12000264, 0x12000267).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_r<4>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_l_w<4>));
	map(0x12000268, 0x1200026b).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_r<4>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_m_w<4>));
	map(0x1200026c, 0x1200026f).rw(FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_r<4>), FUNC(s3c2410_device::usbdev_ep_dma_ttc_h_w<4>));

	map(0x13000000, 0x13000003).rw(FUNC(s3c2410_device::wdt_wtcon_r), FUNC(s3c2410_device::wdt_wtcon_w));
	map(0x13000004, 0x13000007).rw(FUNC(s3c2410_device::wdt_wtdat_r), FUNC(s3c2410_device::wdt_wtdat_w));
	map(0x13000008, 0x1300000b).rw(FUNC(s3c2410_device::wdt_wtcnt_r), FUNC(s3c2410_device::wdt_wtcnt_w));

	map(0x14000000, 0x14000003).rw(FUNC(s3c2410_device::iic_iiccon_r), FUNC(s3c2410_device::iic_iiccon_w));
	map(0x14000004, 0x14000007).rw(FUNC(s3c2410_device::iic_iicstat_r), FUNC(s3c2410_device::iic_iicstat_w));
	map(0x14000008, 0x1400000b).rw(FUNC(s3c2410_device::iic_iicadd_r), FUNC(s3c2410_device::iic_iicadd_w));
	map(0x1400000c, 0x1400000f).rw(FUNC(s3c2410_device::iic_iicds_r), FUNC(s3c2410_device::iic_iicds_w));

	map(0x15000000, 0x15000003).rw(FUNC(s3c2410_device::iis_iiscon_r), FUNC(s3c2410_device::iis_iiscon_w));
	map(0x15000004, 0x15000007).rw(FUNC(s3c2410_device::iis_iismod_r), FUNC(s3c2410_device::iis_iismod_w));
	map(0x15000008, 0x1500000b).rw(FUNC(s3c2410_device::iis_iispsr_r), FUNC(s3c2410_device::iis_iispsr_w));
	map(0x1500000c, 0x1500000f).rw(FUNC(s3c2410_device::iis_iisfcon_r), FUNC(s3c2410_device::iis_iisfcon_w));
	map(0x15000010, 0x15000013).rw(FUNC(s3c2410_device::iis_iisfifo_r), FUNC(s3c2410_device::iis_iisfifo_w));

	map(0x16000000, 0x16000003).rw(FUNC(s3c2410_device::gpio_gpacon_r), FUNC(s3c2410_device::gpio_gpacon_w));
	map(0x16000004, 0x16000007).rw(FUNC(s3c2410_device::gpio_gpadat_r), FUNC(s3c2410_device::gpio_gpadat_w));
	map(0x16000010, 0x16000013).rw(FUNC(s3c2410_device::gpio_gpbcon_r), FUNC(s3c2410_device::gpio_gpbcon_w));
	map(0x16000014, 0x16000017).rw(FUNC(s3c2410_device::gpio_gpbdat_r), FUNC(s3c2410_device::gpio_gpbdat_w));
	map(0x16000018, 0x1600001b).rw(FUNC(s3c2410_device::gpio_gpbup_r), FUNC(s3c2410_device::gpio_gpbup_w));
	map(0x16000020, 0x16000023).rw(FUNC(s3c2410_device::gpio_gpccon_r), FUNC(s3c2410_device::gpio_gpccon_w));
	map(0x16000024, 0x16000027).rw(FUNC(s3c2410_device::gpio_gpcdat_r), FUNC(s3c2410_device::gpio_gpcdat_w));
	map(0x16000028, 0x1600002b).rw(FUNC(s3c2410_device::gpio_gpcup_r), FUNC(s3c2410_device::gpio_gpcup_w));
	map(0x16000030, 0x16000033).rw(FUNC(s3c2410_device::gpio_gpdcon_r), FUNC(s3c2410_device::gpio_gpdcon_w));
	map(0x16000034, 0x16000037).rw(FUNC(s3c2410_device::gpio_gpddat_r), FUNC(s3c2410_device::gpio_gpddat_w));
	map(0x16000038, 0x1600003b).rw(FUNC(s3c2410_device::gpio_gpdup_r), FUNC(s3c2410_device::gpio_gpdup_w));
	map(0x16000040, 0x16000043).rw(FUNC(s3c2410_device::gpio_gpecon_r), FUNC(s3c2410_device::gpio_gpecon_w));
	map(0x16000044, 0x16000047).rw(FUNC(s3c2410_device::gpio_gpedat_r), FUNC(s3c2410_device::gpio_gpedat_w));
	map(0x16000048, 0x1600004b).rw(FUNC(s3c2410_device::gpio_gpeup_r), FUNC(s3c2410_device::gpio_gpeup_w));
	map(0x16000050, 0x16000053).rw(FUNC(s3c2410_device::gpio_gpfcon_r), FUNC(s3c2410_device::gpio_gpfcon_w));
	map(0x16000054, 0x16000057).rw(FUNC(s3c2410_device::gpio_gpfdat_r), FUNC(s3c2410_device::gpio_gpfdat_w));
	map(0x16000058, 0x1600005b).rw(FUNC(s3c2410_device::gpio_gpfup_r), FUNC(s3c2410_device::gpio_gpfup_w));
	map(0x16000060, 0x16000063).rw(FUNC(s3c2410_device::gpio_gpgcon_r), FUNC(s3c2410_device::gpio_gpgcon_w));
	map(0x16000064, 0x16000067).rw(FUNC(s3c2410_device::gpio_gpgdat_r), FUNC(s3c2410_device::gpio_gpgdat_w));
	map(0x16000068, 0x1600006b).rw(FUNC(s3c2410_device::gpio_gpgup_r), FUNC(s3c2410_device::gpio_gpgup_w));
	map(0x16000070, 0x16000073).rw(FUNC(s3c2410_device::gpio_gphcon_r), FUNC(s3c2410_device::gpio_gphcon_w));
	map(0x16000074, 0x16000077).rw(FUNC(s3c2410_device::gpio_gphdat_r), FUNC(s3c2410_device::gpio_gphdat_w));
	map(0x16000078, 0x1600007b).rw(FUNC(s3c2410_device::gpio_gphup_r), FUNC(s3c2410_device::gpio_gphup_w));
	map(0x16000080, 0x16000083).rw(FUNC(s3c2410_device::gpio_misccr_r), FUNC(s3c2410_device::gpio_misccr_w));
	map(0x16000084, 0x16000087).rw(FUNC(s3c2410_device::gpio_dclkcon_r), FUNC(s3c2410_device::gpio_dclkcon_w));
	map(0x16000088, 0x1600008b).rw(FUNC(s3c2410_device::gpio_extint_r<0>), FUNC(s3c2410_device::gpio_extint_w<0>));
	map(0x1600008c, 0x1600008f).rw(FUNC(s3c2410_device::gpio_extint_r<1>), FUNC(s3c2410_device::gpio_extint_w<1>));
	map(0x16000090, 0x16000093).rw(FUNC(s3c2410_device::gpio_extint_r<2>), FUNC(s3c2410_device::gpio_extint_w<2>));
	map(0x16000094, 0x16000097).rw(FUNC(s3c2410_device::gpio_eintflt_r<0>), FUNC(s3c2410_device::gpio_eintflt_w<0>));
	map(0x16000098, 0x1600009b).rw(FUNC(s3c2410_device::gpio_eintflt_r<1>), FUNC(s3c2410_device::gpio_eintflt_w<1>));
	map(0x1600009c, 0x1600009f).rw(FUNC(s3c2410_device::gpio_eintflt_r<2>), FUNC(s3c2410_device::gpio_eintflt_w<2>));
	map(0x160000a0, 0x160000a3).rw(FUNC(s3c2410_device::gpio_eintflt_r<3>), FUNC(s3c2410_device::gpio_eintflt_w<3>));
	map(0x160000a4, 0x160000a7).rw(FUNC(s3c2410_device::gpio_eintmask_r), FUNC(s3c2410_device::gpio_eintmask_w));
	map(0x160000a8, 0x160000ab).rw(FUNC(s3c2410_device::gpio_eintpend_r), FUNC(s3c2410_device::gpio_eintpend_w));
	map(0x160000ac, 0x160000af).r(FUNC(s3c2410_device::gpio_gstatus0_r));
	map(0x160000b0, 0x160000b3).r(FUNC(s3c2410_device::gpio_gstatus1_r));
	map(0x160000b4, 0x160000b7).rw(FUNC(s3c2410_device::gpio_gstatus2_r), FUNC(s3c2410_device::gpio_gstatus2_w));
	map(0x160000b8, 0x160000bb).rw(FUNC(s3c2410_device::gpio_gstatus3_r), FUNC(s3c2410_device::gpio_gstatus3_w));
	map(0x160000bc, 0x160000bf).rw(FUNC(s3c2410_device::gpio_gstatus4_r), FUNC(s3c2410_device::gpio_gstatus4_w));

	map(0x17000040, 0x17000043).rw(FUNC(s3c2410_device::rtc_rtccon_r), FUNC(s3c2410_device::rtc_rtccon_w));
	map(0x17000044, 0x17000047).rw(FUNC(s3c2410_device::rtc_ticnt_r), FUNC(s3c2410_device::rtc_ticnt_w));
	map(0x17000050, 0x17000053).rw(FUNC(s3c2410_device::rtc_rtcalm_r), FUNC(s3c2410_device::rtc_rtcalm_w));
	map(0x17000054, 0x17000057).rw(FUNC(s3c2410_device::rtc_almsec_r), FUNC(s3c2410_device::rtc_almsec_w));
	map(0x17000058, 0x1700005b).rw(FUNC(s3c2410_device::rtc_almmin_r), FUNC(s3c2410_device::rtc_almmin_w));
	map(0x1700005c, 0x1700005f).rw(FUNC(s3c2410_device::rtc_almhour_r), FUNC(s3c2410_device::rtc_almhour_w));
	map(0x17000060, 0x17000063).rw(FUNC(s3c2410_device::rtc_almday_r), FUNC(s3c2410_device::rtc_almday_w));
	map(0x17000064, 0x17000067).rw(FUNC(s3c2410_device::rtc_almmon_r), FUNC(s3c2410_device::rtc_almmon_w));
	map(0x17000068, 0x1700006b).rw(FUNC(s3c2410_device::rtc_almyear_r), FUNC(s3c2410_device::rtc_almyear_w));
	map(0x1700006c, 0x1700006f).rw(FUNC(s3c2410_device::rtc_rtcrst_r), FUNC(s3c2410_device::rtc_rtcrst_w));
	map(0x17000070, 0x17000073).rw(FUNC(s3c2410_device::rtc_bcdsec_r), FUNC(s3c2410_device::rtc_bcdsec_w));
	map(0x17000074, 0x17000077).rw(FUNC(s3c2410_device::rtc_bcdmin_r), FUNC(s3c2410_device::rtc_bcdmin_w));
	map(0x17000078, 0x1700007b).rw(FUNC(s3c2410_device::rtc_bcdhour_r), FUNC(s3c2410_device::rtc_bcdhour_w));
	map(0x1700007c, 0x1700007f).rw(FUNC(s3c2410_device::rtc_bcddate_r), FUNC(s3c2410_device::rtc_bcddate_w));
	map(0x17000080, 0x17000083).rw(FUNC(s3c2410_device::rtc_bcdday_r), FUNC(s3c2410_device::rtc_bcdday_w));
	map(0x17000084, 0x17000087).rw(FUNC(s3c2410_device::rtc_bcdmon_r), FUNC(s3c2410_device::rtc_bcdmon_w));
	map(0x17000088, 0x1700008b).rw(FUNC(s3c2410_device::rtc_bcdyear_r), FUNC(s3c2410_device::rtc_bcdyear_w));

	map(0x18000000, 0x18000003).rw(FUNC(s3c2410_device::adc_adccon_r), FUNC(s3c2410_device::adc_adccon_w));
	map(0x18000004, 0x18000007).rw(FUNC(s3c2410_device::adc_adctsc_r), FUNC(s3c2410_device::adc_adctsc_w));
	map(0x18000008, 0x1800000b).rw(FUNC(s3c2410_device::adc_adcdly_r), FUNC(s3c2410_device::adc_adcdly_w));
	map(0x1800000c, 0x1800000f).rw(FUNC(s3c2410_device::adc_adcdat0_r), FUNC(s3c2410_device::adc_adcdat0_w));
	map(0x18000010, 0x18000013).rw(FUNC(s3c2410_device::adc_adcdat1_r), FUNC(s3c2410_device::adc_adcdat1_w));

	map(0x19000000, 0x19000003).rw(FUNC(s3c2410_device::spi_spcon_r<0>), FUNC(s3c2410_device::spi_spcon_w<0>));
	map(0x19000004, 0x19000007).rw(FUNC(s3c2410_device::spi_spsta_r<0>), FUNC(s3c2410_device::spi_spsta_w<0>));
	map(0x19000008, 0x1900000b).rw(FUNC(s3c2410_device::spi_sppin_r<0>), FUNC(s3c2410_device::spi_sppin_w<0>));
	map(0x1900000c, 0x1900000f).rw(FUNC(s3c2410_device::spi_sppre_r<0>), FUNC(s3c2410_device::spi_sppre_w<0>));
	map(0x19000010, 0x19000013).rw(FUNC(s3c2410_device::spi_sptdat_r<0>), FUNC(s3c2410_device::spi_sptdat_w<0>));
	map(0x19000014, 0x19000017).rw(FUNC(s3c2410_device::spi_sprdat_r<0>), FUNC(s3c2410_device::spi_sprdat_w<0>));
	map(0x19000020, 0x19000023).rw(FUNC(s3c2410_device::spi_spcon_r<1>), FUNC(s3c2410_device::spi_spcon_w<1>));
	map(0x19000024, 0x19000027).rw(FUNC(s3c2410_device::spi_spsta_r<1>), FUNC(s3c2410_device::spi_spsta_w<1>));
	map(0x19000028, 0x1900002b).rw(FUNC(s3c2410_device::spi_sppin_r<1>), FUNC(s3c2410_device::spi_sppin_w<1>));
	map(0x1900002c, 0x1900002f).rw(FUNC(s3c2410_device::spi_sppre_r<1>), FUNC(s3c2410_device::spi_sppre_w<1>));
	map(0x19000030, 0x19000033).rw(FUNC(s3c2410_device::spi_sptdat_r<1>), FUNC(s3c2410_device::spi_sptdat_w<1>));
	map(0x19000034, 0x19000037).rw(FUNC(s3c2410_device::spi_sprdat_r<1>), FUNC(s3c2410_device::spi_sprdat_w<1>));

	map(0x1a000000, 0x1a000003).rw(FUNC(s3c2410_device::sdi_con_r), FUNC(s3c2410_device::sdi_con_w));
	map(0x1a000004, 0x1a000007).rw(FUNC(s3c2410_device::sdi_pre_r), FUNC(s3c2410_device::sdi_pre_w));
	map(0x1a000008, 0x1a00000b).rw(FUNC(s3c2410_device::sdi_cmdarg_r), FUNC(s3c2410_device::sdi_cmdarg_w));
	map(0x1a00000c, 0x1a00000f).rw(FUNC(s3c2410_device::sdi_cmdcon_r), FUNC(s3c2410_device::sdi_cmdcon_w));
	map(0x1a000010, 0x1a000013).rw(FUNC(s3c2410_device::sdi_cmdsta_r), FUNC(s3c2410_device::sdi_cmdsta_w));
	map(0x1a000014, 0x1a000017).rw(FUNC(s3c2410_device::sdi_rsp0_r), FUNC(s3c2410_device::sdi_rsp0_w));
	map(0x1a000018, 0x1a00001b).rw(FUNC(s3c2410_device::sdi_rsp1_r), FUNC(s3c2410_device::sdi_rsp1_w));
	map(0x1a00001c, 0x1a00001f).rw(FUNC(s3c2410_device::sdi_rsp2_r), FUNC(s3c2410_device::sdi_rsp2_w));
	map(0x1a000020, 0x1a000023).rw(FUNC(s3c2410_device::sdi_rsp3_r), FUNC(s3c2410_device::sdi_rsp3_w));
	map(0x1a000024, 0x1a000027).rw(FUNC(s3c2410_device::sdi_dtimer_r), FUNC(s3c2410_device::sdi_dtimer_w));
	map(0x1a000028, 0x1a00002b).rw(FUNC(s3c2410_device::sdi_bsize_r), FUNC(s3c2410_device::sdi_bsize_w));
	map(0x1a00002c, 0x1a00002f).rw(FUNC(s3c2410_device::sdi_datcon_r), FUNC(s3c2410_device::sdi_datcon_w));
	map(0x1a000030, 0x1a000033).rw(FUNC(s3c2410_device::sdi_adidatcnt_r), FUNC(s3c2410_device::sdi_adidatcnt_w));
	map(0x1a000034, 0x1a000037).rw(FUNC(s3c2410_device::sdi_adidatsta_r), FUNC(s3c2410_device::sdi_adidatsta_w));
	map(0x1a000038, 0x1a00003b).rw(FUNC(s3c2410_device::sdi_fsta_r), FUNC(s3c2410_device::sdi_fsta_w));
	map(0x1a00003c, 0x1a00003f).rw(FUNC(s3c2410_device::sdi_dat_r), FUNC(s3c2410_device::sdi_dat_w));
	map(0x1a000040, 0x1a000043).rw(FUNC(s3c2410_device::sdi_intmsk_r), FUNC(s3c2410_device::sdi_intmsk_w));
}

void s3c2440_device::map(address_map &map)
{
	map(0x08000000, 0x08000003).rw(FUNC(s3c2440_device::memcon_bwscon_r), FUNC(s3c2440_device::memcon_bwscon_w));
	map(0x08000004, 0x08000007).rw(FUNC(s3c2440_device::memcon_bankcon_r<0>), FUNC(s3c2440_device::memcon_bankcon_w<0>));
	map(0x08000008, 0x0800000b).rw(FUNC(s3c2440_device::memcon_bankcon_r<1>), FUNC(s3c2440_device::memcon_bankcon_w<1>));
	map(0x0800000c, 0x0800000f).rw(FUNC(s3c2440_device::memcon_bankcon_r<2>), FUNC(s3c2440_device::memcon_bankcon_w<2>));
	map(0x08000010, 0x08000013).rw(FUNC(s3c2440_device::memcon_bankcon_r<3>), FUNC(s3c2440_device::memcon_bankcon_w<3>));
	map(0x08000014, 0x08000017).rw(FUNC(s3c2440_device::memcon_bankcon_r<4>), FUNC(s3c2440_device::memcon_bankcon_w<4>));
	map(0x08000018, 0x0800001b).rw(FUNC(s3c2440_device::memcon_bankcon_r<5>), FUNC(s3c2440_device::memcon_bankcon_w<5>));
	map(0x0800001c, 0x0800001f).rw(FUNC(s3c2440_device::memcon_bankcon_r<6>), FUNC(s3c2440_device::memcon_bankcon_w<6>));
	map(0x08000020, 0x08000023).rw(FUNC(s3c2440_device::memcon_bankcon_r<7>), FUNC(s3c2440_device::memcon_bankcon_w<7>));
	map(0x08000024, 0x08000027).rw(FUNC(s3c2440_device::memcon_refresh_r), FUNC(s3c2440_device::memcon_refresh_w));
	map(0x08000028, 0x0800002b).rw(FUNC(s3c2440_device::memcon_banksize_r), FUNC(s3c2440_device::memcon_banksize_w));
	map(0x0800002c, 0x0800002f).rw(FUNC(s3c2440_device::memcon_mrsrb6_r), FUNC(s3c2440_device::memcon_mrsrb6_w));
	map(0x08000030, 0x08000033).rw(FUNC(s3c2440_device::memcon_mrsrb7_r), FUNC(s3c2440_device::memcon_mrsrb7_w));

	map(0x09000000, 0x09000003).r(FUNC(s3c2440_device::usbhost_rev_r));
	map(0x09000004, 0x09000007).rw(FUNC(s3c2440_device::usbhost_ctrl_r), FUNC(s3c2440_device::usbhost_ctrl_w));
	map(0x09000008, 0x0900000b).rw(FUNC(s3c2440_device::usbhost_common_stat_r), FUNC(s3c2440_device::usbhost_common_stat_w));
	map(0x0900000c, 0x0900000f).rw(FUNC(s3c2440_device::usbhost_int_stat_r), FUNC(s3c2440_device::usbhost_int_stat_w));
	map(0x09000010, 0x09000013).rw(FUNC(s3c2440_device::usbhost_int_en_r), FUNC(s3c2440_device::usbhost_int_en_w));
	map(0x09000014, 0x09000017).rw(FUNC(s3c2440_device::usbhost_int_dis_r), FUNC(s3c2440_device::usbhost_int_dis_w));
	map(0x09000018, 0x0900001b).rw(FUNC(s3c2440_device::usbhost_hcca_r), FUNC(s3c2440_device::usbhost_hcca_w));
	map(0x0900001c, 0x0900001f).rw(FUNC(s3c2440_device::usbhost_period_curr_r), FUNC(s3c2440_device::usbhost_period_curr_w));
	map(0x09000020, 0x09000023).rw(FUNC(s3c2440_device::usbhost_ctrl_head_r), FUNC(s3c2440_device::usbhost_ctrl_head_w));
	map(0x09000024, 0x09000027).rw(FUNC(s3c2440_device::usbhost_ctrl_curr_r), FUNC(s3c2440_device::usbhost_ctrl_curr_w));
	map(0x09000028, 0x0900002b).rw(FUNC(s3c2440_device::usbhost_bulk_head_r), FUNC(s3c2440_device::usbhost_bulk_head_w));
	map(0x0900002c, 0x0900002f).rw(FUNC(s3c2440_device::usbhost_bulk_curr_r), FUNC(s3c2440_device::usbhost_bulk_curr_w));
	map(0x09000030, 0x09000033).rw(FUNC(s3c2440_device::usbhost_done_head_r), FUNC(s3c2440_device::usbhost_done_head_w));
	map(0x09000034, 0x09000037).rw(FUNC(s3c2440_device::usbhost_rm_interval_r), FUNC(s3c2440_device::usbhost_rm_interval_w));
	map(0x09000038, 0x0900003b).rw(FUNC(s3c2440_device::usbhost_fm_remaining_r), FUNC(s3c2440_device::usbhost_fm_remaining_w));
	map(0x0900003c, 0x0900003f).rw(FUNC(s3c2440_device::usbhost_fm_number_r), FUNC(s3c2440_device::usbhost_fm_number_w));
	map(0x09000040, 0x09000043).rw(FUNC(s3c2440_device::usbhost_periodic_start_r), FUNC(s3c2440_device::usbhost_periodic_start_w));
	map(0x09000044, 0x09000047).rw(FUNC(s3c2440_device::usbhost_ls_thresh_r), FUNC(s3c2440_device::usbhost_ls_thresh_w));
	map(0x09000048, 0x0900004b).rw(FUNC(s3c2440_device::usbhost_rh_desc_a_r), FUNC(s3c2440_device::usbhost_rh_desc_a_w));
	map(0x0900004c, 0x0900004f).rw(FUNC(s3c2440_device::usbhost_rh_desc_b_r), FUNC(s3c2440_device::usbhost_rh_desc_b_w));
	map(0x09000050, 0x09000053).rw(FUNC(s3c2440_device::usbhost_rh_status_r), FUNC(s3c2440_device::usbhost_rh_status_w));
	map(0x09000054, 0x09000057).rw(FUNC(s3c2440_device::usbhost_rh_port_stat_1_r), FUNC(s3c2440_device::usbhost_rh_port_stat_1_w));
	map(0x09000058, 0x0900005b).rw(FUNC(s3c2440_device::usbhost_rh_port_stat_2_r), FUNC(s3c2440_device::usbhost_rh_port_stat_2_w));

	map(0x0a000000, 0x0a000003).rw(FUNC(s3c2440_device::irq_srcpnd_r), FUNC(s3c2440_device::irq_srcpnd_w));
	map(0x0a000004, 0x0a000007).rw(FUNC(s3c2440_device::irq_intmod_r), FUNC(s3c2440_device::irq_intmod_w));
	map(0x0a000008, 0x0a00000b).rw(FUNC(s3c2440_device::irq_intmsk_r), FUNC(s3c2440_device::irq_intmsk_w));
	map(0x0a00000c, 0x0a00000f).rw(FUNC(s3c2440_device::irq_priority_r), FUNC(s3c2440_device::irq_priority_w));
	map(0x0a000010, 0x0a000013).rw(FUNC(s3c2440_device::irq_intpnd_r), FUNC(s3c2440_device::irq_intpnd_w));
	map(0x0a000014, 0x0a000017).rw(FUNC(s3c2440_device::irq_intoffset_r), FUNC(s3c2440_device::irq_intoffset_w));
	map(0x0a000018, 0x0a00001b).rw(FUNC(s3c2440_device::irq_subsrcpnd_r), FUNC(s3c2440_device::irq_subsrcpnd_w));
	map(0x0a00001c, 0x0a00001f).rw(FUNC(s3c2440_device::irq_intsubmsk_r), FUNC(s3c2440_device::irq_intsubmsk_w));

	map(0x0b000000, 0x0b000003).rw(FUNC(s3c2440_device::dma_disrc_r<0>), FUNC(s3c2440_device::dma_disrc_w<0>));
	map(0x0b000004, 0x0b000007).rw(FUNC(s3c2440_device::dma_disrcc_r<0>), FUNC(s3c2440_device::dma_disrcc_w<0>));
	map(0x0b000008, 0x0b00000b).rw(FUNC(s3c2440_device::dma_didst_r<0>), FUNC(s3c2440_device::dma_didst_w<0>));
	map(0x0b00000c, 0x0b00000f).rw(FUNC(s3c2440_device::dma_didstc_r<0>), FUNC(s3c2440_device::dma_didstc_w<0>));
	map(0x0b000010, 0x0b000013).rw(FUNC(s3c2440_device::dma_dcon_r<0>), FUNC(s3c2440_device::dma_dcon_w<0>));
	map(0x0b000014, 0x0b000017).rw(FUNC(s3c2440_device::dma_dstat_r<0>), FUNC(s3c2440_device::dma_dstat_w<0>));
	map(0x0b000018, 0x0b00001b).rw(FUNC(s3c2440_device::dma_dcsrc_r<0>), FUNC(s3c2440_device::dma_dcsrc_w<0>));
	map(0x0b00001c, 0x0b00001f).rw(FUNC(s3c2440_device::dma_dcdst_r<0>), FUNC(s3c2440_device::dma_dcdst_w<0>));
	map(0x0b000020, 0x0b000023).rw(FUNC(s3c2440_device::dma_dmasktrig_r<0>), FUNC(s3c2440_device::dma_dmasktrig_w<0>));
	map(0x0b000040, 0x0b000043).rw(FUNC(s3c2440_device::dma_disrc_r<1>), FUNC(s3c2440_device::dma_disrc_w<1>));
	map(0x0b000044, 0x0b000047).rw(FUNC(s3c2440_device::dma_disrcc_r<1>), FUNC(s3c2440_device::dma_disrcc_w<1>));
	map(0x0b000048, 0x0b00004b).rw(FUNC(s3c2440_device::dma_didst_r<1>), FUNC(s3c2440_device::dma_didst_w<1>));
	map(0x0b00004c, 0x0b00004f).rw(FUNC(s3c2440_device::dma_didstc_r<1>), FUNC(s3c2440_device::dma_didstc_w<1>));
	map(0x0b000050, 0x0b000053).rw(FUNC(s3c2440_device::dma_dcon_r<1>), FUNC(s3c2440_device::dma_dcon_w<1>));
	map(0x0b000054, 0x0b000057).rw(FUNC(s3c2440_device::dma_dstat_r<1>), FUNC(s3c2440_device::dma_dstat_w<1>));
	map(0x0b000058, 0x0b00005b).rw(FUNC(s3c2440_device::dma_dcsrc_r<1>), FUNC(s3c2440_device::dma_dcsrc_w<1>));
	map(0x0b00005c, 0x0b00005f).rw(FUNC(s3c2440_device::dma_dcdst_r<1>), FUNC(s3c2440_device::dma_dcdst_w<1>));
	map(0x0b000060, 0x0b000063).rw(FUNC(s3c2440_device::dma_dmasktrig_r<1>), FUNC(s3c2440_device::dma_dmasktrig_w<1>));
	map(0x0b000080, 0x0b000083).rw(FUNC(s3c2440_device::dma_disrc_r<2>), FUNC(s3c2440_device::dma_disrc_w<2>));
	map(0x0b000084, 0x0b000087).rw(FUNC(s3c2440_device::dma_disrcc_r<2>), FUNC(s3c2440_device::dma_disrcc_w<2>));
	map(0x0b000088, 0x0b00008b).rw(FUNC(s3c2440_device::dma_didst_r<2>), FUNC(s3c2440_device::dma_didst_w<2>));
	map(0x0b00008c, 0x0b00008f).rw(FUNC(s3c2440_device::dma_didstc_r<2>), FUNC(s3c2440_device::dma_didstc_w<2>));
	map(0x0b000090, 0x0b000093).rw(FUNC(s3c2440_device::dma_dcon_r<2>), FUNC(s3c2440_device::dma_dcon_w<2>));
	map(0x0b000094, 0x0b000097).rw(FUNC(s3c2440_device::dma_dstat_r<2>), FUNC(s3c2440_device::dma_dstat_w<2>));
	map(0x0b000098, 0x0b00009b).rw(FUNC(s3c2440_device::dma_dcsrc_r<2>), FUNC(s3c2440_device::dma_dcsrc_w<2>));
	map(0x0b00009c, 0x0b00009f).rw(FUNC(s3c2440_device::dma_dcdst_r<2>), FUNC(s3c2440_device::dma_dcdst_w<2>));
	map(0x0b0000a0, 0x0b0000a3).rw(FUNC(s3c2440_device::dma_dmasktrig_r<2>), FUNC(s3c2440_device::dma_dmasktrig_w<2>));
	map(0x0b0000c0, 0x0b0000c3).rw(FUNC(s3c2440_device::dma_disrc_r<3>), FUNC(s3c2440_device::dma_disrc_w<3>));
	map(0x0b0000c4, 0x0b0000c7).rw(FUNC(s3c2440_device::dma_disrcc_r<3>), FUNC(s3c2440_device::dma_disrcc_w<3>));
	map(0x0b0000c8, 0x0b0000cb).rw(FUNC(s3c2440_device::dma_didst_r<3>), FUNC(s3c2440_device::dma_didst_w<3>));
	map(0x0b0000cc, 0x0b0000cf).rw(FUNC(s3c2440_device::dma_didstc_r<3>), FUNC(s3c2440_device::dma_didstc_w<3>));
	map(0x0b0000d0, 0x0b0000d3).rw(FUNC(s3c2440_device::dma_dcon_r<3>), FUNC(s3c2440_device::dma_dcon_w<3>));
	map(0x0b0000d4, 0x0b0000d7).rw(FUNC(s3c2440_device::dma_dstat_r<3>), FUNC(s3c2440_device::dma_dstat_w<3>));
	map(0x0b0000d8, 0x0b0000db).rw(FUNC(s3c2440_device::dma_dcsrc_r<3>), FUNC(s3c2440_device::dma_dcsrc_w<3>));
	map(0x0b0000dc, 0x0b0000df).rw(FUNC(s3c2440_device::dma_dcdst_r<3>), FUNC(s3c2440_device::dma_dcdst_w<3>));
	map(0x0b0000e0, 0x0b0000e3).rw(FUNC(s3c2440_device::dma_dmasktrig_r<3>), FUNC(s3c2440_device::dma_dmasktrig_w<3>));

	map(0x0c000000, 0x0c000003).rw(FUNC(s3c2440_device::clkpow_locktime_r), FUNC(s3c2440_device::clkpow_locktime_w));
	map(0x0c000004, 0x0c000007).rw(FUNC(s3c2440_device::clkpow_mpllcon_r), FUNC(s3c2440_device::clkpow_mpllcon_w));
	map(0x0c000008, 0x0c00000b).rw(FUNC(s3c2440_device::clkpow_upllcon_r), FUNC(s3c2440_device::clkpow_upllcon_w));
	map(0x0c00000c, 0x0c00000f).rw(FUNC(s3c2440_device::clkpow_clkcon_r), FUNC(s3c2440_device::clkpow_clkcon_w));
	map(0x0c000010, 0x0c000013).rw(FUNC(s3c2440_device::clkpow_clkslow_r), FUNC(s3c2440_device::clkpow_clkslow_w));
	map(0x0c000014, 0x0c000017).rw(FUNC(s3c2440_device::clkpow_clkdivn_r), FUNC(s3c2440_device::clkpow_clkdivn_w));
	map(0x0c000018, 0x0c00001b).rw(FUNC(s3c2440_device::clkpow_camdivn_r), FUNC(s3c2440_device::clkpow_camdivn_w));

	map(0x0d000000, 0x0d000003).rw(FUNC(s3c2440_device::lcd_lcdcon1_r), FUNC(s3c2440_device::lcd_lcdcon1_w));
	map(0x0d000004, 0x0d000007).rw(FUNC(s3c2440_device::lcd_lcdcon2_r), FUNC(s3c2440_device::lcd_lcdcon2_w));
	map(0x0d000008, 0x0d00000b).rw(FUNC(s3c2440_device::lcd_lcdcon3_r), FUNC(s3c2440_device::lcd_lcdcon3_w));
	map(0x0d00000c, 0x0d00000f).rw(FUNC(s3c2440_device::lcd_lcdcon4_r), FUNC(s3c2440_device::lcd_lcdcon4_w));
	map(0x0d000010, 0x0d000013).rw(FUNC(s3c2440_device::lcd_lcdcon5_r), FUNC(s3c2440_device::lcd_lcdcon5_w));
	map(0x0d000014, 0x0d000017).rw(FUNC(s3c2440_device::lcd_lcdsaddr1_r), FUNC(s3c2440_device::lcd_lcdsaddr1_w));
	map(0x0d000018, 0x0d00001b).rw(FUNC(s3c2440_device::lcd_lcdsaddr2_r), FUNC(s3c2440_device::lcd_lcdsaddr2_w));
	map(0x0d00001c, 0x0d00001f).rw(FUNC(s3c2440_device::lcd_lcdsaddr3_r), FUNC(s3c2440_device::lcd_lcdsaddr3_w));
	map(0x0d000020, 0x0d000023).rw(FUNC(s3c2440_device::lcd_redlut_r), FUNC(s3c2440_device::lcd_redlut_w));
	map(0x0d000024, 0x0d000027).rw(FUNC(s3c2440_device::lcd_greenlut_r), FUNC(s3c2440_device::lcd_greenlut_w));
	map(0x0d000028, 0x0d00002b).rw(FUNC(s3c2440_device::lcd_bluelut_r), FUNC(s3c2440_device::lcd_bluelut_w));
	map(0x0d00004c, 0x0d00004f).rw(FUNC(s3c2440_device::lcd_dithmode_r), FUNC(s3c2440_device::lcd_dithmode_w));
	map(0x0d000050, 0x0d000053).rw(FUNC(s3c2440_device::lcd_tpal_r), FUNC(s3c2440_device::lcd_tpal_w));
	map(0x0d000054, 0x0d000057).rw(FUNC(s3c2440_device::lcd_lcdintpnd_r), FUNC(s3c2440_device::lcd_lcdintpnd_w));
	map(0x0d000058, 0x0d00005b).rw(FUNC(s3c2440_device::lcd_lcdsrcpnd_r), FUNC(s3c2440_device::lcd_lcdsrcpnd_w));
	map(0x0d00005c, 0x0d00005f).rw(FUNC(s3c2440_device::lcd_lcdintmsk_r), FUNC(s3c2440_device::lcd_lcdintmsk_w));
	map(0x0d000060, 0x0d000063).rw(FUNC(s3c2440_device::lcd_tconsel_r), FUNC(s3c2440_device::lcd_tconsel_w));
	map(0x0d000400, 0x0d0007ff).rw(FUNC(s3c2440_device::lcd_palette_r), FUNC(s3c2440_device::lcd_palette_w));

	map(0x0e000000, 0x0e000003).rw(FUNC(s3c2440_device::nand_nfconf_r), FUNC(s3c2440_device::nand_nfconf_w));
	map(0x0e000004, 0x0e000007).rw(FUNC(s3c2440_device::nand_nfcont_r), FUNC(s3c2440_device::nand_nfcont_w));
	map(0x0e000008, 0x0e00000b).rw(FUNC(s3c2440_device::nand_nfcmd_r), FUNC(s3c2440_device::nand_nfcmd_w));
	map(0x0e00000c, 0x0e00000f).rw(FUNC(s3c2440_device::nand_nfaddr_r), FUNC(s3c2440_device::nand_nfaddr_w));
	map(0x0e000010, 0x0e000013).rw(FUNC(s3c2440_device::nand_nfdata_r), FUNC(s3c2440_device::nand_nfdata_w));
	map(0x0e000014, 0x0e000017).rw(FUNC(s3c2440_device::nand_nfmeccd0_r), FUNC(s3c2440_device::nand_nfmeccd0_w));
	map(0x0e000018, 0x0e00001b).rw(FUNC(s3c2440_device::nand_nfmeccd1_r), FUNC(s3c2440_device::nand_nfmeccd1_w));
	map(0x0e00001c, 0x0e00001f).rw(FUNC(s3c2440_device::nand_nfseccd_r), FUNC(s3c2440_device::nand_nfseccd_w));
	map(0x0e000020, 0x0e000023).rw(FUNC(s3c2440_device::nand_nfstat_r), FUNC(s3c2440_device::nand_nfstat_w));
	map(0x0e000024, 0x0e000027).rw(FUNC(s3c2440_device::nand_nfestat0_r), FUNC(s3c2440_device::nand_nfestat0_w));
	map(0x0e000028, 0x0e00002b).rw(FUNC(s3c2440_device::nand_nfestat1_r), FUNC(s3c2440_device::nand_nfestat1_w));
	map(0x0e00002c, 0x0e00002f).rw(FUNC(s3c2440_device::nand_nfmecc0_r), FUNC(s3c2440_device::nand_nfmecc0_w));
	map(0x0e000030, 0x0e000033).rw(FUNC(s3c2440_device::nand_nfmecc1_r), FUNC(s3c2440_device::nand_nfmecc1_w));
	map(0x0e000034, 0x0e000037).rw(FUNC(s3c2440_device::nand_nfsecc_r), FUNC(s3c2440_device::nand_nfsecc_w));
	map(0x0e000038, 0x0e00003b).rw(FUNC(s3c2440_device::nand_nfsblk_r), FUNC(s3c2440_device::nand_nfsblk_w));
	map(0x0e00003c, 0x0e00003f).rw(FUNC(s3c2440_device::nand_nfeblk_r), FUNC(s3c2440_device::nand_nfeblk_w));

	map(0x0f000000, 0x0f000003).rw(FUNC(s3c2440_device::cam_srcfmt_r), FUNC(s3c2440_device::cam_srcfmt_w));
	map(0x0f000004, 0x0f000007).rw(FUNC(s3c2440_device::cam_wdofst_r), FUNC(s3c2440_device::cam_wdofst_w));
	map(0x0f000008, 0x0f00000b).rw(FUNC(s3c2440_device::cam_gctrl_r), FUNC(s3c2440_device::cam_gctrl_w));
	map(0x0f000018, 0x0f00001b).rw(FUNC(s3c2440_device::cam_coysa1_r), FUNC(s3c2440_device::cam_coysa1_w));
	map(0x0f00001c, 0x0f00001f).rw(FUNC(s3c2440_device::cam_coysa2_r), FUNC(s3c2440_device::cam_coysa2_w));
	map(0x0f000020, 0x0f000023).rw(FUNC(s3c2440_device::cam_coysa3_r), FUNC(s3c2440_device::cam_coysa3_w));
	map(0x0f000024, 0x0f000027).rw(FUNC(s3c2440_device::cam_coysa4_r), FUNC(s3c2440_device::cam_coysa4_w));
	map(0x0f000028, 0x0f00002b).rw(FUNC(s3c2440_device::cam_cocbsa1_r), FUNC(s3c2440_device::cam_cocbsa1_w));
	map(0x0f00002c, 0x0f00002f).rw(FUNC(s3c2440_device::cam_cocbsa2_r), FUNC(s3c2440_device::cam_cocbsa2_w));
	map(0x0f000030, 0x0f000033).rw(FUNC(s3c2440_device::cam_cocbsa3_r), FUNC(s3c2440_device::cam_cocbsa3_w));
	map(0x0f000034, 0x0f000037).rw(FUNC(s3c2440_device::cam_cocbsa4_r), FUNC(s3c2440_device::cam_cocbsa4_w));
	map(0x0f000038, 0x0f00003b).rw(FUNC(s3c2440_device::cam_cocrsa1_r), FUNC(s3c2440_device::cam_cocrsa1_w));
	map(0x0f00003c, 0x0f00003f).rw(FUNC(s3c2440_device::cam_cocrsa2_r), FUNC(s3c2440_device::cam_cocrsa2_w));
	map(0x0f000040, 0x0f000043).rw(FUNC(s3c2440_device::cam_cocrsa3_r), FUNC(s3c2440_device::cam_cocrsa3_w));
	map(0x0f000044, 0x0f000047).rw(FUNC(s3c2440_device::cam_cocrsa4_r), FUNC(s3c2440_device::cam_cocrsa4_w));
	map(0x0f000048, 0x0f00004b).rw(FUNC(s3c2440_device::cam_cotrgfmt_r), FUNC(s3c2440_device::cam_cotrgfmt_w));
	map(0x0f00004c, 0x0f00004f).rw(FUNC(s3c2440_device::cam_coctrl_r), FUNC(s3c2440_device::cam_coctrl_w));
	map(0x0f000050, 0x0f000053).rw(FUNC(s3c2440_device::cam_coscpreratio_r), FUNC(s3c2440_device::cam_coscpreratio_w));
	map(0x0f000054, 0x0f000057).rw(FUNC(s3c2440_device::cam_coscpredst_r), FUNC(s3c2440_device::cam_coscpredst_w));
	map(0x0f000058, 0x0f00005b).rw(FUNC(s3c2440_device::cam_coscctrl_r), FUNC(s3c2440_device::cam_coscctrl_w));
	map(0x0f00005c, 0x0f00005f).rw(FUNC(s3c2440_device::cam_cotarea_r), FUNC(s3c2440_device::cam_cotarea_w));
	map(0x0f000064, 0x0f000067).rw(FUNC(s3c2440_device::cam_costatus_r), FUNC(s3c2440_device::cam_costatus_w));
	map(0x0f00006c, 0x0f00006f).rw(FUNC(s3c2440_device::cam_prclrsa1_r), FUNC(s3c2440_device::cam_prclrsa1_w));
	map(0x0f000070, 0x0f000073).rw(FUNC(s3c2440_device::cam_prclrsa2_r), FUNC(s3c2440_device::cam_prclrsa2_w));
	map(0x0f000074, 0x0f000077).rw(FUNC(s3c2440_device::cam_prclrsa3_r), FUNC(s3c2440_device::cam_prclrsa3_w));
	map(0x0f000078, 0x0f00007b).rw(FUNC(s3c2440_device::cam_prclrsa4_r), FUNC(s3c2440_device::cam_prclrsa4_w));
	map(0x0f00007c, 0x0f00007f).rw(FUNC(s3c2440_device::cam_prtrgfmt_r), FUNC(s3c2440_device::cam_prtrgfmt_w));
	map(0x0f000080, 0x0f000083).rw(FUNC(s3c2440_device::cam_prctrl_r), FUNC(s3c2440_device::cam_prctrl_w));
	map(0x0f000084, 0x0f000087).rw(FUNC(s3c2440_device::cam_prscpreratio_r), FUNC(s3c2440_device::cam_prscpreratio_w));
	map(0x0f000088, 0x0f00008b).rw(FUNC(s3c2440_device::cam_prscpredst_r), FUNC(s3c2440_device::cam_prscpredst_w));
	map(0x0f00008c, 0x0f00008f).rw(FUNC(s3c2440_device::cam_prscctrl_r), FUNC(s3c2440_device::cam_prscctrl_w));
	map(0x0f000090, 0x0f000093).rw(FUNC(s3c2440_device::cam_prtarea_r), FUNC(s3c2440_device::cam_prtarea_w));
	map(0x0f000098, 0x0f00009b).rw(FUNC(s3c2440_device::cam_prstatus_r), FUNC(s3c2440_device::cam_prstatus_w));
	map(0x0f0000a0, 0x0f0000a3).rw(FUNC(s3c2440_device::cam_imgcpt_r), FUNC(s3c2440_device::cam_imgcpt_w));

	map(0x10000000, 0x10000003).rw(FUNC(s3c2440_device::uart_ulcon_r<0>), FUNC(s3c2440_device::uart_ulcon_w<0>));
	map(0x10000004, 0x10000007).rw(FUNC(s3c2440_device::uart_ucon_r<0>), FUNC(s3c2440_device::uart_ucon_w<0>));
	map(0x10000008, 0x1000000b).rw(FUNC(s3c2440_device::uart_ufcon_r<0>), FUNC(s3c2440_device::uart_ufcon_w<0>));
	map(0x1000000c, 0x1000000f).rw(FUNC(s3c2440_device::uart_umcon_r<0>), FUNC(s3c2440_device::uart_umcon_w<0>));
	map(0x10000010, 0x10000013).rw(FUNC(s3c2440_device::uart_utrstat_r<0>), FUNC(s3c2440_device::uart_utrstat_w<0>));
	map(0x10000014, 0x10000017).rw(FUNC(s3c2440_device::uart_uerstat_r<0>), FUNC(s3c2440_device::uart_uerstat_w<0>));
	map(0x10000018, 0x1000001b).rw(FUNC(s3c2440_device::uart_ufstat_r<0>), FUNC(s3c2440_device::uart_ufstat_w<0>));
	map(0x1000001c, 0x1000001f).rw(FUNC(s3c2440_device::uart_umstat_r<0>), FUNC(s3c2440_device::uart_umstat_w<0>));
	map(0x10000020, 0x10000023).rw(FUNC(s3c2440_device::uart_utxh_r<0>), FUNC(s3c2440_device::uart_utxh_w<0>));
	map(0x10000024, 0x10000027).rw(FUNC(s3c2440_device::uart_urxh_r<0>), FUNC(s3c2440_device::uart_urxh_w<0>));
	map(0x10000028, 0x1000002b).rw(FUNC(s3c2440_device::uart_ubrdiv_r<0>), FUNC(s3c2440_device::uart_ubrdiv_w<0>));
	map(0x10004000, 0x10004003).rw(FUNC(s3c2440_device::uart_ulcon_r<1>), FUNC(s3c2440_device::uart_ulcon_w<1>));
	map(0x10004004, 0x10004007).rw(FUNC(s3c2440_device::uart_ucon_r<1>), FUNC(s3c2440_device::uart_ucon_w<1>));
	map(0x10004008, 0x1000400b).rw(FUNC(s3c2440_device::uart_ufcon_r<1>), FUNC(s3c2440_device::uart_ufcon_w<1>));
	map(0x1000400c, 0x1000400f).rw(FUNC(s3c2440_device::uart_umcon_r<1>), FUNC(s3c2440_device::uart_umcon_w<1>));
	map(0x10004010, 0x10004013).rw(FUNC(s3c2440_device::uart_utrstat_r<1>), FUNC(s3c2440_device::uart_utrstat_w<1>));
	map(0x10004014, 0x10004017).rw(FUNC(s3c2440_device::uart_uerstat_r<1>), FUNC(s3c2440_device::uart_uerstat_w<1>));
	map(0x10004018, 0x1000401b).rw(FUNC(s3c2440_device::uart_ufstat_r<1>), FUNC(s3c2440_device::uart_ufstat_w<1>));
	map(0x1000401c, 0x1000401f).rw(FUNC(s3c2440_device::uart_umstat_r<1>), FUNC(s3c2440_device::uart_umstat_w<1>));
	map(0x10004020, 0x10004023).rw(FUNC(s3c2440_device::uart_utxh_r<1>), FUNC(s3c2440_device::uart_utxh_w<1>));
	map(0x10004024, 0x10004027).rw(FUNC(s3c2440_device::uart_urxh_r<1>), FUNC(s3c2440_device::uart_urxh_w<1>));
	map(0x10004028, 0x1000402b).rw(FUNC(s3c2440_device::uart_ubrdiv_r<1>), FUNC(s3c2440_device::uart_ubrdiv_w<1>));
	map(0x10008000, 0x10008003).rw(FUNC(s3c2440_device::uart_ulcon_r<2>), FUNC(s3c2440_device::uart_ulcon_w<2>));
	map(0x10008004, 0x10508007).rw(FUNC(s3c2440_device::uart_ucon_r<2>), FUNC(s3c2440_device::uart_ucon_w<2>));
	map(0x10008008, 0x1000800b).rw(FUNC(s3c2440_device::uart_ufcon_r<2>), FUNC(s3c2440_device::uart_ufcon_w<2>));
	map(0x1000800c, 0x1000800f).rw(FUNC(s3c2440_device::uart_umcon_r<2>), FUNC(s3c2440_device::uart_umcon_w<2>));
	map(0x10008010, 0x10008013).rw(FUNC(s3c2440_device::uart_utrstat_r<2>), FUNC(s3c2440_device::uart_utrstat_w<2>));
	map(0x10008014, 0x10008017).rw(FUNC(s3c2440_device::uart_uerstat_r<2>), FUNC(s3c2440_device::uart_uerstat_w<2>));
	map(0x10008018, 0x1000801b).rw(FUNC(s3c2440_device::uart_ufstat_r<2>), FUNC(s3c2440_device::uart_ufstat_w<2>));
	map(0x1000801c, 0x1000801f).rw(FUNC(s3c2440_device::uart_umstat_r<2>), FUNC(s3c2440_device::uart_umstat_w<2>));
	map(0x10008020, 0x10008023).rw(FUNC(s3c2440_device::uart_utxh_r<2>), FUNC(s3c2440_device::uart_utxh_w<2>));
	map(0x10008024, 0x10008027).rw(FUNC(s3c2440_device::uart_urxh_r<2>), FUNC(s3c2440_device::uart_urxh_w<2>));
	map(0x10008028, 0x1000802b).rw(FUNC(s3c2440_device::uart_ubrdiv_r<2>), FUNC(s3c2440_device::uart_ubrdiv_w<2>));

	map(0x11000000, 0x11000003).rw(FUNC(s3c2440_device::pwm_tcfg0_r), FUNC(s3c2440_device::pwm_tcfg0_w));
	map(0x11000004, 0x11000007).rw(FUNC(s3c2440_device::pwm_tcfg1_r), FUNC(s3c2440_device::pwm_tcfg1_w));
	map(0x11000008, 0x1100000b).rw(FUNC(s3c2440_device::pwm_tcon_r), FUNC(s3c2440_device::pwm_tcon_w));
	map(0x1100000c, 0x1100000f).rw(FUNC(s3c2440_device::pwm_tcntb0_r), FUNC(s3c2440_device::pwm_tcntb0_w));
	map(0x11000010, 0x11000013).rw(FUNC(s3c2440_device::pwm_tcmpb0_r), FUNC(s3c2440_device::pwm_tcmpb0_w));
	map(0x11000014, 0x11000017).rw(FUNC(s3c2440_device::pwm_tcnto0_r), FUNC(s3c2440_device::pwm_tcnto0_w));
	map(0x11000018, 0x1100001b).rw(FUNC(s3c2440_device::pwm_tcntb1_r), FUNC(s3c2440_device::pwm_tcntb1_w));
	map(0x1100001c, 0x1100001f).rw(FUNC(s3c2440_device::pwm_tcmpb1_r), FUNC(s3c2440_device::pwm_tcmpb1_w));
	map(0x11000020, 0x11000023).rw(FUNC(s3c2440_device::pwm_tcnto1_r), FUNC(s3c2440_device::pwm_tcnto1_w));
	map(0x11000024, 0x11000027).rw(FUNC(s3c2440_device::pwm_tcntb2_r), FUNC(s3c2440_device::pwm_tcntb2_w));
	map(0x11000028, 0x1100002b).rw(FUNC(s3c2440_device::pwm_tcmpb2_r), FUNC(s3c2440_device::pwm_tcmpb2_w));
	map(0x1100002c, 0x1100002f).rw(FUNC(s3c2440_device::pwm_tcnto2_r), FUNC(s3c2440_device::pwm_tcnto2_w));
	map(0x11000030, 0x11000033).rw(FUNC(s3c2440_device::pwm_tcntb3_r), FUNC(s3c2440_device::pwm_tcntb3_w));
	map(0x11000034, 0x11000037).rw(FUNC(s3c2440_device::pwm_tcmpb3_r), FUNC(s3c2440_device::pwm_tcmpb3_w));
	map(0x11000038, 0x1100003b).rw(FUNC(s3c2440_device::pwm_tcnto3_r), FUNC(s3c2440_device::pwm_tcnto3_w));
	map(0x1100003c, 0x1100003f).rw(FUNC(s3c2440_device::pwm_tcntb4_r), FUNC(s3c2440_device::pwm_tcntb4_w));
	map(0x11000040, 0x11000043).rw(FUNC(s3c2440_device::pwm_tcnto4_r), FUNC(s3c2440_device::pwm_tcnto4_w));

	map(0x12000140, 0x12000143).rw(FUNC(s3c2440_device::usbdev_func_addr_r), FUNC(s3c2440_device::usbdev_func_addr_w));
	map(0x12000144, 0x12000147).rw(FUNC(s3c2440_device::usbdev_pwr_r), FUNC(s3c2440_device::usbdev_pwr_w));
	map(0x12000148, 0x1200014b).rw(FUNC(s3c2440_device::usbdev_ep_int_r), FUNC(s3c2440_device::usbdev_ep_int_w));
	map(0x12000158, 0x1200015b).rw(FUNC(s3c2440_device::usbdev_usb_int_r), FUNC(s3c2440_device::usbdev_usb_int_w));
	map(0x1200015c, 0x1200015f).rw(FUNC(s3c2440_device::usbdev_ep_int_en_r), FUNC(s3c2440_device::usbdev_ep_int_en_w));
	map(0x1200016c, 0x1200016f).rw(FUNC(s3c2440_device::usbdev_usb_int_en_r), FUNC(s3c2440_device::usbdev_usb_int_en_w));
	map(0x12000170, 0x12000173).rw(FUNC(s3c2440_device::usbdev_frame_num1_r), FUNC(s3c2440_device::usbdev_frame_num1_w));
	map(0x12000174, 0x12000177).rw(FUNC(s3c2440_device::usbdev_frame_num2_r), FUNC(s3c2440_device::usbdev_frame_num2_w));
	map(0x12000178, 0x1200017b).rw(FUNC(s3c2440_device::usbdev_index_r), FUNC(s3c2440_device::usbdev_index_w));
	map(0x12000180, 0x12000183).rw(FUNC(s3c2440_device::usbdev_maxp_r), FUNC(s3c2440_device::usbdev_maxp_w));
	map(0x12000184, 0x12000187).rw(FUNC(s3c2440_device::usbdev_ep_csr_r<0>), FUNC(s3c2440_device::usbdev_ep_csr_w<0>));
	map(0x12000188, 0x1200018b).rw(FUNC(s3c2440_device::usbdev_in_csr2_r), FUNC(s3c2440_device::usbdev_in_csr2_w));
	map(0x12000190, 0x12000193).rw(FUNC(s3c2440_device::usbdev_out_csr1_r), FUNC(s3c2440_device::usbdev_out_csr1_w));
	map(0x12000194, 0x12000197).rw(FUNC(s3c2440_device::usbdev_out_csr2_r), FUNC(s3c2440_device::usbdev_out_csr2_w));
	map(0x12000198, 0x1200019b).rw(FUNC(s3c2440_device::usbdev_out_fifo_cnt1_r), FUNC(s3c2440_device::usbdev_out_fifo_cnt1_w));
	map(0x1200019c, 0x1200019f).rw(FUNC(s3c2440_device::usbdev_out_fifo_cnt2_r), FUNC(s3c2440_device::usbdev_out_fifo_cnt2_w));
	map(0x120001c0, 0x120001c3).rw(FUNC(s3c2440_device::usbdev_ep0_fifo_r), FUNC(s3c2440_device::usbdev_ep0_fifo_w));
	map(0x120001c4, 0x120001c7).rw(FUNC(s3c2440_device::usbdev_ep1_fifo_r), FUNC(s3c2440_device::usbdev_ep1_fifo_w));
	map(0x120001c8, 0x120001cb).rw(FUNC(s3c2440_device::usbdev_ep2_fifo_r), FUNC(s3c2440_device::usbdev_ep2_fifo_w));
	map(0x120001cc, 0x120001cf).rw(FUNC(s3c2440_device::usbdev_ep3_fifo_r), FUNC(s3c2440_device::usbdev_ep3_fifo_w));
	map(0x120001d0, 0x120001d3).rw(FUNC(s3c2440_device::usbdev_ep4_fifo_r), FUNC(s3c2440_device::usbdev_ep4_fifo_w));
	map(0x12000200, 0x12000203).rw(FUNC(s3c2440_device::usbdev_ep_dma_con_r<1>), FUNC(s3c2440_device::usbdev_ep_dma_con_w<1>));
	map(0x12000204, 0x12000207).rw(FUNC(s3c2440_device::usbdev_ep_dma_unit_r<1>), FUNC(s3c2440_device::usbdev_ep_dma_unit_w<1>));
	map(0x12000208, 0x1200020b).rw(FUNC(s3c2440_device::usbdev_ep_dma_fifo_r<1>), FUNC(s3c2440_device::usbdev_ep_dma_fifo_w<1>));
	map(0x1200020c, 0x1200020f).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_r<1>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_w<1>));
	map(0x12000210, 0x12000213).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_r<1>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_w<1>));
	map(0x12000214, 0x12000217).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_r<1>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_w<1>));
	map(0x12000218, 0x1200021b).rw(FUNC(s3c2440_device::usbdev_ep_dma_con_r<2>), FUNC(s3c2440_device::usbdev_ep_dma_con_w<2>));
	map(0x1200021c, 0x1200021f).rw(FUNC(s3c2440_device::usbdev_ep_dma_unit_r<2>), FUNC(s3c2440_device::usbdev_ep_dma_unit_w<2>));
	map(0x12000220, 0x12000223).rw(FUNC(s3c2440_device::usbdev_ep_dma_fifo_r<2>), FUNC(s3c2440_device::usbdev_ep_dma_fifo_w<2>));
	map(0x12000224, 0x12000227).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_r<2>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_w<2>));
	map(0x12000228, 0x1200022b).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_r<2>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_w<2>));
	map(0x1200022c, 0x1200022f).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_r<2>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_w<2>));
	map(0x12000240, 0x12000243).rw(FUNC(s3c2440_device::usbdev_ep_dma_con_r<3>), FUNC(s3c2440_device::usbdev_ep_dma_con_w<3>));
	map(0x12000244, 0x12000247).rw(FUNC(s3c2440_device::usbdev_ep_dma_unit_r<3>), FUNC(s3c2440_device::usbdev_ep_dma_unit_w<3>));
	map(0x12000248, 0x1200024b).rw(FUNC(s3c2440_device::usbdev_ep_dma_fifo_r<3>), FUNC(s3c2440_device::usbdev_ep_dma_fifo_w<3>));
	map(0x1200024c, 0x1200024f).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_r<3>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_w<3>));
	map(0x12000250, 0x12000253).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_r<3>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_w<3>));
	map(0x12000254, 0x12000257).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_r<3>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_w<3>));
	map(0x12000258, 0x1200025b).rw(FUNC(s3c2440_device::usbdev_ep_dma_con_r<4>), FUNC(s3c2440_device::usbdev_ep_dma_con_w<4>));
	map(0x1200025c, 0x1200025f).rw(FUNC(s3c2440_device::usbdev_ep_dma_unit_r<4>), FUNC(s3c2440_device::usbdev_ep_dma_unit_w<4>));
	map(0x12000260, 0x12000263).rw(FUNC(s3c2440_device::usbdev_ep_dma_fifo_r<4>), FUNC(s3c2440_device::usbdev_ep_dma_fifo_w<4>));
	map(0x12000264, 0x12000267).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_r<4>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_l_w<4>));
	map(0x12000268, 0x1200026b).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_r<4>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_m_w<4>));
	map(0x1200026c, 0x1200026f).rw(FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_r<4>), FUNC(s3c2440_device::usbdev_ep_dma_ttc_h_w<4>));

	map(0x13000000, 0x13000003).rw(FUNC(s3c2440_device::wdt_wtcon_r), FUNC(s3c2440_device::wdt_wtcon_w));
	map(0x13000004, 0x13000007).rw(FUNC(s3c2440_device::wdt_wtdat_r), FUNC(s3c2440_device::wdt_wtdat_w));
	map(0x13000008, 0x1300000b).rw(FUNC(s3c2440_device::wdt_wtcnt_r), FUNC(s3c2440_device::wdt_wtcnt_w));

	map(0x14000000, 0x14000003).rw(FUNC(s3c2440_device::iic_iiccon_r), FUNC(s3c2440_device::iic_iiccon_w));
	map(0x14000004, 0x14000007).rw(FUNC(s3c2440_device::iic_iicstat_r), FUNC(s3c2440_device::iic_iicstat_w));
	map(0x14000008, 0x1400000b).rw(FUNC(s3c2440_device::iic_iicadd_r), FUNC(s3c2440_device::iic_iicadd_w));
	map(0x1400000c, 0x1400000f).rw(FUNC(s3c2440_device::iic_iicds_r), FUNC(s3c2440_device::iic_iicds_w));
	map(0x14000010, 0x14000013).rw(FUNC(s3c2440_device::iic_iiclc_r), FUNC(s3c2440_device::iic_iiclc_w));

	map(0x15000000, 0x15000003).rw(FUNC(s3c2440_device::iis_iiscon_r), FUNC(s3c2440_device::iis_iiscon_w));
	map(0x15000004, 0x15000007).rw(FUNC(s3c2440_device::iis_iismod_r), FUNC(s3c2440_device::iis_iismod_w));
	map(0x15000008, 0x1500000b).rw(FUNC(s3c2440_device::iis_iispsr_r), FUNC(s3c2440_device::iis_iispsr_w));
	map(0x1500000c, 0x1500000f).rw(FUNC(s3c2440_device::iis_iisfcon_r), FUNC(s3c2440_device::iis_iisfcon_w));
	map(0x15000010, 0x15000013).rw(FUNC(s3c2440_device::iis_iisfifo_r), FUNC(s3c2440_device::iis_iisfifo_w));

	map(0x16000000, 0x16000003).rw(FUNC(s3c2440_device::gpio_gpacon_r), FUNC(s3c2440_device::gpio_gpacon_w));
	map(0x16000004, 0x16000007).rw(FUNC(s3c2440_device::gpio_gpadat_r), FUNC(s3c2440_device::gpio_gpadat_w));
	map(0x16000010, 0x16000013).rw(FUNC(s3c2440_device::gpio_gpbcon_r), FUNC(s3c2440_device::gpio_gpbcon_w));
	map(0x16000014, 0x16000017).rw(FUNC(s3c2440_device::gpio_gpbdat_r), FUNC(s3c2440_device::gpio_gpbdat_w));
	map(0x16000018, 0x1600001b).rw(FUNC(s3c2440_device::gpio_gpbup_r), FUNC(s3c2440_device::gpio_gpbup_w));
	map(0x16000020, 0x16000023).rw(FUNC(s3c2440_device::gpio_gpccon_r), FUNC(s3c2440_device::gpio_gpccon_w));
	map(0x16000024, 0x16000027).rw(FUNC(s3c2440_device::gpio_gpcdat_r), FUNC(s3c2440_device::gpio_gpcdat_w));
	map(0x16000028, 0x1600002b).rw(FUNC(s3c2440_device::gpio_gpcup_r), FUNC(s3c2440_device::gpio_gpcup_w));
	map(0x16000030, 0x16000033).rw(FUNC(s3c2440_device::gpio_gpdcon_r), FUNC(s3c2440_device::gpio_gpdcon_w));
	map(0x16000034, 0x16000037).rw(FUNC(s3c2440_device::gpio_gpddat_r), FUNC(s3c2440_device::gpio_gpddat_w));
	map(0x16000038, 0x1600003b).rw(FUNC(s3c2440_device::gpio_gpdup_r), FUNC(s3c2440_device::gpio_gpdup_w));
	map(0x16000040, 0x16000043).rw(FUNC(s3c2440_device::gpio_gpecon_r), FUNC(s3c2440_device::gpio_gpecon_w));
	map(0x16000044, 0x16000047).rw(FUNC(s3c2440_device::gpio_gpedat_r), FUNC(s3c2440_device::gpio_gpedat_w));
	map(0x16000048, 0x1600004b).rw(FUNC(s3c2440_device::gpio_gpeup_r), FUNC(s3c2440_device::gpio_gpeup_w));
	map(0x16000050, 0x16000053).rw(FUNC(s3c2440_device::gpio_gpfcon_r), FUNC(s3c2440_device::gpio_gpfcon_w));
	map(0x16000054, 0x16000057).rw(FUNC(s3c2440_device::gpio_gpfdat_r), FUNC(s3c2440_device::gpio_gpfdat_w));
	map(0x16000058, 0x1600005b).rw(FUNC(s3c2440_device::gpio_gpfup_r), FUNC(s3c2440_device::gpio_gpfup_w));
	map(0x16000060, 0x16000063).rw(FUNC(s3c2440_device::gpio_gpgcon_r), FUNC(s3c2440_device::gpio_gpgcon_w));
	map(0x16000064, 0x16000067).rw(FUNC(s3c2440_device::gpio_gpgdat_r), FUNC(s3c2440_device::gpio_gpgdat_w));
	map(0x16000068, 0x1600006b).rw(FUNC(s3c2440_device::gpio_gpgup_r), FUNC(s3c2440_device::gpio_gpgup_w));
	map(0x16000070, 0x16000073).rw(FUNC(s3c2440_device::gpio_gphcon_r), FUNC(s3c2440_device::gpio_gphcon_w));
	map(0x16000074, 0x16000077).rw(FUNC(s3c2440_device::gpio_gphdat_r), FUNC(s3c2440_device::gpio_gphdat_w));
	map(0x16000078, 0x1600007b).rw(FUNC(s3c2440_device::gpio_gphup_r), FUNC(s3c2440_device::gpio_gphup_w));
	map(0x16000080, 0x16000083).rw(FUNC(s3c2440_device::gpio_misccr_r), FUNC(s3c2440_device::gpio_misccr_w));
	map(0x16000084, 0x16000087).rw(FUNC(s3c2440_device::gpio_dclkcon_r), FUNC(s3c2440_device::gpio_dclkcon_w));
	map(0x16000088, 0x1600008b).rw(FUNC(s3c2440_device::gpio_extint_r<0>), FUNC(s3c2440_device::gpio_extint_w<0>));
	map(0x1600008c, 0x1600008f).rw(FUNC(s3c2440_device::gpio_extint_r<1>), FUNC(s3c2440_device::gpio_extint_w<1>));
	map(0x16000090, 0x16000093).rw(FUNC(s3c2440_device::gpio_extint_r<2>), FUNC(s3c2440_device::gpio_extint_w<2>));
	map(0x16000094, 0x16000097).rw(FUNC(s3c2440_device::gpio_eintflt_r<0>), FUNC(s3c2440_device::gpio_eintflt_w<0>));
	map(0x16000098, 0x1600009b).rw(FUNC(s3c2440_device::gpio_eintflt_r<1>), FUNC(s3c2440_device::gpio_eintflt_w<1>));
	map(0x1600009c, 0x1600009f).rw(FUNC(s3c2440_device::gpio_eintflt_r<2>), FUNC(s3c2440_device::gpio_eintflt_w<2>));
	map(0x160000a0, 0x160000a3).rw(FUNC(s3c2440_device::gpio_eintflt_r<3>), FUNC(s3c2440_device::gpio_eintflt_w<3>));
	map(0x160000a4, 0x160000a7).rw(FUNC(s3c2440_device::gpio_eintmask_r), FUNC(s3c2440_device::gpio_eintmask_w));
	map(0x160000a8, 0x160000ab).rw(FUNC(s3c2440_device::gpio_eintpend_r), FUNC(s3c2440_device::gpio_eintpend_w));
	map(0x160000ac, 0x160000af).r(FUNC(s3c2440_device::gpio_gstatus0_r));
	map(0x160000b0, 0x160000b3).r(FUNC(s3c2440_device::gpio_gstatus1_r));
	map(0x160000b4, 0x160000b7).rw(FUNC(s3c2440_device::gpio_gstatus2_r), FUNC(s3c2440_device::gpio_gstatus2_w));
	map(0x160000b8, 0x160000bb).rw(FUNC(s3c2440_device::gpio_gstatus3_r), FUNC(s3c2440_device::gpio_gstatus3_w));
	map(0x160000bc, 0x160000bf).rw(FUNC(s3c2440_device::gpio_gstatus4_r), FUNC(s3c2440_device::gpio_gstatus4_w));
	map(0x160000cc, 0x160000cf).rw(FUNC(s3c2440_device::gpio_mslcon_r), FUNC(s3c2440_device::gpio_mslcon_w));
	map(0x160000d0, 0x160000d3).rw(FUNC(s3c2440_device::gpio_gpjcon_r), FUNC(s3c2440_device::gpio_gpjcon_w));
	map(0x160000d4, 0x160000d7).rw(FUNC(s3c2440_device::gpio_gpjdat_r), FUNC(s3c2440_device::gpio_gpjdat_w));
	map(0x160000d8, 0x160000db).rw(FUNC(s3c2440_device::gpio_gpjup_r), FUNC(s3c2440_device::gpio_gpjup_w));

	map(0x17000040, 0x17000043).rw(FUNC(s3c2440_device::rtc_rtccon_r), FUNC(s3c2440_device::rtc_rtccon_w));
	map(0x17000044, 0x17000047).rw(FUNC(s3c2440_device::rtc_ticnt_r), FUNC(s3c2440_device::rtc_ticnt_w));
	map(0x17000050, 0x17000053).rw(FUNC(s3c2440_device::rtc_rtcalm_r), FUNC(s3c2440_device::rtc_rtcalm_w));
	map(0x17000054, 0x17000057).rw(FUNC(s3c2440_device::rtc_almsec_r), FUNC(s3c2440_device::rtc_almsec_w));
	map(0x17000058, 0x1700005b).rw(FUNC(s3c2440_device::rtc_almmin_r), FUNC(s3c2440_device::rtc_almmin_w));
	map(0x1700005c, 0x1700005f).rw(FUNC(s3c2440_device::rtc_almhour_r), FUNC(s3c2440_device::rtc_almhour_w));
	map(0x17000060, 0x17000063).rw(FUNC(s3c2440_device::rtc_almday_r), FUNC(s3c2440_device::rtc_almday_w));
	map(0x17000064, 0x17000067).rw(FUNC(s3c2440_device::rtc_almmon_r), FUNC(s3c2440_device::rtc_almmon_w));
	map(0x17000068, 0x1700006b).rw(FUNC(s3c2440_device::rtc_almyear_r), FUNC(s3c2440_device::rtc_almyear_w));
	map(0x17000070, 0x17000073).rw(FUNC(s3c2440_device::rtc_bcdsec_r), FUNC(s3c2440_device::rtc_bcdsec_w));
	map(0x17000074, 0x17000077).rw(FUNC(s3c2440_device::rtc_bcdmin_r), FUNC(s3c2440_device::rtc_bcdmin_w));
	map(0x17000078, 0x1700007b).rw(FUNC(s3c2440_device::rtc_bcdhour_r), FUNC(s3c2440_device::rtc_bcdhour_w));
	map(0x1700007c, 0x1700007f).rw(FUNC(s3c2440_device::rtc_bcddate_r), FUNC(s3c2440_device::rtc_bcddate_w));
	map(0x17000080, 0x17000083).rw(FUNC(s3c2440_device::rtc_bcdday_r), FUNC(s3c2440_device::rtc_bcdday_w));
	map(0x17000084, 0x17000087).rw(FUNC(s3c2440_device::rtc_bcdmon_r), FUNC(s3c2440_device::rtc_bcdmon_w));
	map(0x17000088, 0x1700008b).rw(FUNC(s3c2440_device::rtc_bcdyear_r), FUNC(s3c2440_device::rtc_bcdyear_w));

	map(0x18000000, 0x18000003).rw(FUNC(s3c2440_device::adc_adccon_r), FUNC(s3c2440_device::adc_adccon_w));
	map(0x18000004, 0x18000007).rw(FUNC(s3c2440_device::adc_adctsc_r), FUNC(s3c2440_device::adc_adctsc_w));
	map(0x18000008, 0x1800000b).rw(FUNC(s3c2440_device::adc_adcdly_r), FUNC(s3c2440_device::adc_adcdly_w));
	map(0x1800000c, 0x1800000f).rw(FUNC(s3c2440_device::adc_adcdat0_r), FUNC(s3c2440_device::adc_adcdat0_w));
	map(0x18000010, 0x18000013).rw(FUNC(s3c2440_device::adc_adcdat1_r), FUNC(s3c2440_device::adc_adcdat1_w));
	map(0x18000014, 0x18000017).rw(FUNC(s3c2440_device::adc_adcupdn_r), FUNC(s3c2440_device::adc_adcupdn_w));

	map(0x19000000, 0x19000003).rw(FUNC(s3c2440_device::spi_spcon_r<0>), FUNC(s3c2440_device::spi_spcon_w<0>));
	map(0x19000004, 0x19000007).rw(FUNC(s3c2440_device::spi_spsta_r<0>), FUNC(s3c2440_device::spi_spsta_w<0>));
	map(0x19000008, 0x1900000b).rw(FUNC(s3c2440_device::spi_sppin_r<0>), FUNC(s3c2440_device::spi_sppin_w<0>));
	map(0x1900000c, 0x1900000f).rw(FUNC(s3c2440_device::spi_sppre_r<0>), FUNC(s3c2440_device::spi_sppre_w<0>));
	map(0x19000010, 0x19000013).rw(FUNC(s3c2440_device::spi_sptdat_r<0>), FUNC(s3c2440_device::spi_sptdat_w<0>));
	map(0x19000014, 0x19000017).rw(FUNC(s3c2440_device::spi_sprdat_r<0>), FUNC(s3c2440_device::spi_sprdat_w<0>));
	map(0x19000020, 0x19000023).rw(FUNC(s3c2440_device::spi_spcon_r<1>), FUNC(s3c2440_device::spi_spcon_w<1>));
	map(0x19000024, 0x19000027).rw(FUNC(s3c2440_device::spi_spsta_r<1>), FUNC(s3c2440_device::spi_spsta_w<1>));
	map(0x19000028, 0x1900002b).rw(FUNC(s3c2440_device::spi_sppin_r<1>), FUNC(s3c2440_device::spi_sppin_w<1>));
	map(0x1900002c, 0x1900002f).rw(FUNC(s3c2440_device::spi_sppre_r<1>), FUNC(s3c2440_device::spi_sppre_w<1>));
	map(0x19000030, 0x19000033).rw(FUNC(s3c2440_device::spi_sptdat_r<1>), FUNC(s3c2440_device::spi_sptdat_w<1>));
	map(0x19000034, 0x19000037).rw(FUNC(s3c2440_device::spi_sprdat_r<1>), FUNC(s3c2440_device::spi_sprdat_w<1>));

	map(0x1a000000, 0x1a000003).rw(FUNC(s3c2440_device::sdi_con_r), FUNC(s3c2440_device::sdi_con_w));
	map(0x1a000004, 0x1a000007).rw(FUNC(s3c2440_device::sdi_pre_r), FUNC(s3c2440_device::sdi_pre_w));
	map(0x1a000008, 0x1a00000b).rw(FUNC(s3c2440_device::sdi_cmdarg_r), FUNC(s3c2440_device::sdi_cmdarg_w));
	map(0x1a00000c, 0x1a00000f).rw(FUNC(s3c2440_device::sdi_cmdcon_r), FUNC(s3c2440_device::sdi_cmdcon_w));
	map(0x1a000010, 0x1a000013).rw(FUNC(s3c2440_device::sdi_cmdsta_r), FUNC(s3c2440_device::sdi_cmdsta_w));
	map(0x1a000014, 0x1a000017).rw(FUNC(s3c2440_device::sdi_rsp0_r), FUNC(s3c2440_device::sdi_rsp0_w));
	map(0x1a000018, 0x1a00001b).rw(FUNC(s3c2440_device::sdi_rsp1_r), FUNC(s3c2440_device::sdi_rsp1_w));
	map(0x1a00001c, 0x1a00001f).rw(FUNC(s3c2440_device::sdi_rsp2_r), FUNC(s3c2440_device::sdi_rsp2_w));
	map(0x1a000020, 0x1a000023).rw(FUNC(s3c2440_device::sdi_rsp3_r), FUNC(s3c2440_device::sdi_rsp3_w));
	map(0x1a000024, 0x1a000027).rw(FUNC(s3c2440_device::sdi_dtimer_r), FUNC(s3c2440_device::sdi_dtimer_w));
	map(0x1a000028, 0x1a00002b).rw(FUNC(s3c2440_device::sdi_bsize_r), FUNC(s3c2440_device::sdi_bsize_w));
	map(0x1a00002c, 0x1a00002f).rw(FUNC(s3c2440_device::sdi_datcon_r), FUNC(s3c2440_device::sdi_datcon_w));
	map(0x1a000030, 0x1a000033).rw(FUNC(s3c2440_device::sdi_adidatcnt_r), FUNC(s3c2440_device::sdi_adidatcnt_w));
	map(0x1a000034, 0x1a000037).rw(FUNC(s3c2440_device::sdi_adidatsta_r), FUNC(s3c2440_device::sdi_adidatsta_w));
	map(0x1a000038, 0x1a00003b).rw(FUNC(s3c2440_device::sdi_fsta_r), FUNC(s3c2440_device::sdi_fsta_w));
	map(0x1a00003c, 0x1a00003f).rw(FUNC(s3c2440_device::sdi_dat_r), FUNC(s3c2440_device::sdi_dat_w));
	map(0x1a000040, 0x1a000043).rw(FUNC(s3c2440_device::sdi_intmsk_r), FUNC(s3c2440_device::sdi_intmsk_w));

	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_glbctrl_r), FUNC(s3c2440_device::ac97_glbctrl_w));
	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_glbstat_r), FUNC(s3c2440_device::ac97_glbstat_w));
	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_codec_cmd_r), FUNC(s3c2440_device::ac97_codec_cmd_w));
	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_codec_stat_r), FUNC(s3c2440_device::ac97_codec_stat_w));
	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_pcmaddr_r), FUNC(s3c2440_device::ac97_pcmaddr_w));
	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_micaddr_r), FUNC(s3c2440_device::ac97_micaddr_w));
	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_pcmdata_r), FUNC(s3c2440_device::ac97_pcmdata_w));
	map(0x1b000000, 0x1b000003).rw(FUNC(s3c2440_device::ac97_micdata_r), FUNC(s3c2440_device::ac97_micdata_w));
}

/* LCD Controller */

void s3c24xx_device::lcd_reset()
{
	m_lcd_lcdcon1 = 0;
	m_lcd_lcdcon2 = 0;
	m_lcd_lcdcon3 = 0;
	m_lcd_lcdcon4 = 0;
	m_lcd_lcdcon5 = 0;
	m_lcd_lcdsaddr1 = 0;
	m_lcd_lcdsaddr2 = 0;
	m_lcd_lcdsaddr3 = 0;
	m_lcd_redlut = 0;
	m_lcd_greenlut = 0;
	m_lcd_bluelut = 0;
	m_lcd_dithmode = 0;
	m_lcd_tpal = 0;

	m_lcd_vramaddr_cur = 0;
	m_lcd_vramaddr_max = 0;
	m_lcd_offsize = 0;
	m_lcd_pagewidth_cur = 0;
	m_lcd_pagewidth_max = 0;
	m_lcd_bppmode = 0;
	m_lcd_bswp = 0;
	m_lcd_hwswp = 0;
	m_lcd_vpos = 0;
	m_lcd_hpos = 0;
	m_lcd_framerate = attotime::never;
	m_lcd_tpal = 0;
	m_lcd_hpos_min = 0;
	m_lcd_hpos_max = 0;
	m_lcd_vpos_min = 0;
	m_lcd_vpos_max = 0;
	m_lcd_dma_data = 0;
	m_lcd_dma_bits = 0;
	m_lcd_timer->adjust(attotime::never);
}

void s3c2410_device::lcd_reset()
{
	s3c24xx_device::lcd_reset();
	m_lcd_lpcsel = 4;
	m_lcd_lcdintpnd = 0;
	m_lcd_lcdsrcpnd = 0;
	m_lcd_lcdintmsk = 3;
}

void s3c2440_device::lcd_reset()
{
	s3c2410_device::lcd_reset();
	m_lcd_tconsel = 0x0f84;
	m_lcd_lcdintmsk = 3;
}

rgb_t s3c24xx_device::lcd_get_color_tft_16(u16 data)
{
	if (!BIT(m_lcd_lcdcon5, 11))
	{
		const u8 r = (BIT(data, 11, 5) << 3);
		const u8 g = (BIT(data, 6, 5) << 3);
		const u8 b = (BIT(data, 1, 5) << 3);
		const u8 i = BIT(data, 1) << 2;
		return rgb_t(r | i, g | i, b | i);
	}
	else
	{
		const u8 r = BIT(data, 11, 5) << 3;
		const u8 g = BIT(data, 5, 6) << 2;
		const u8 b = BIT(data, 0, 5) << 3;
		return rgb_t(r, g, b);
	}
}

rgb_t s3c2410_device::lcd_get_color_tft_24(u32 data)
{
	const u8 r = BIT(data, 16, 8);
	const u8 g = BIT(data, 8, 8);
	const u8 b = BIT(data, 0, 8);
	return rgb_t(r, g, b);
}

rgb_t s3c24xx_device::lcd_get_color_stn_12(u16 data)
{
	const u8 r = BIT(data, 8, 4) << 4;
	const u8 g = BIT(data, 4, 4) << 4;
	const u8 b = BIT(data, 0, 4) << 4;
	return rgb_t(r, g, b);
}

rgb_t s3c24xx_device::lcd_get_color_stn_08(u8 data)
{
	const u8 r = ((m_lcd_redlut   >> (BIT(data, 5, 3) << 2)) & 0xf) << 4;
	const u8 g = ((m_lcd_greenlut >> (BIT(data, 2, 3) << 2)) & 0xf) << 4;
	const u8 b = ((m_lcd_bluelut  >> (BIT(data, 0, 2) << 2)) & 0xf) << 4;
	return rgb_t(r, g, b);
}

rgb_t s3c24xx_device::lcd_get_color_stn_01(u8 data)
{
	if (BIT(data, 0))
		return rgb_t::white();
	else
		return rgb_t::black();
}

rgb_t s3c24xx_device::lcd_get_color_stn_02(u8 data)
{
	const u8 i = ((m_lcd_bluelut >> (BIT(data, 0, 2) << 2)) & 0xf) << 4;
	return rgb_t(i, i, i);
}

rgb_t s3c24xx_device::lcd_get_color_stn_04(u8 data)
{
	const u8 i = BIT(data, 0, 4) << 4;
	return rgb_t(i, i, i);
}

u32 s3c24xx_device::lcd_get_tpalval()
{
	return BIT(m_lcd_tpal, 0, 16);
}

u32 s3c2410_device::lcd_get_tpalval()
{
	return BIT(m_lcd_tpal, 0, 24);
}

rgb_t s3c24xx_device::lcd_get_color_tpal()
{
	return lcd_get_color_tft_16(lcd_get_tpalval());
}

rgb_t s3c2410_device::lcd_get_color_tpal()
{
	return lcd_get_color_tft_24(lcd_get_tpalval());
}

bool s3c24xx_device::lcd_get_tpalen()
{
	return BIT(m_lcd_tpal, 16);
}

bool s3c2410_device::lcd_get_tpalen()
{
	return BIT(m_lcd_tpal, 24);
}

void s3c24xx_device::lcd_dma_reload()
{
	m_lcd_vramaddr_cur = m_lcd_lcdsaddr1 << 1;
	m_lcd_vramaddr_max = ((m_lcd_lcdsaddr1 & 0xffe00000) | m_lcd_lcdsaddr2) << 1;
	m_lcd_offsize = BIT(m_lcd_lcdsaddr3, 11, 11);
	m_lcd_pagewidth_cur = 0;
	m_lcd_pagewidth_max = BIT(m_lcd_lcdsaddr3, 0, 11);
	if (m_lcd_pagewidth_max == 0)
	{
		if (m_lcd_bppmode == BPPMODE_STN_12_P)
		{
			m_lcd_pagewidth_max = (m_lcd_hpos_max - m_lcd_hpos_min + 1) / 16 * 12;
		}
	}
	//LOGMASKED(LOG_LCD_DMA, "LCD - vramaddr %08X %08X offsize %08X pagewidth %08X\n", m_lcd_vramaddr_cur, m_lcd_vramaddr_max, m_lcd_offsize, m_lcd_pagewidth_max);
	m_lcd_dma_data = 0;
	m_lcd_dma_bits = 0;
}

void s3c24xx_device::lcd_dma_init()
{
	m_lcd_bppmode = BIT(m_lcd_lcdcon1, 1, 4);
	lcd_dma_reload();
	m_lcd_bswp = BIT(m_lcd_lcdcon5, 1);
	m_lcd_hwswp = BIT(m_lcd_lcdcon5, 0);
	m_lcd_tpal = 0;
	//LOGMASKED(LOG_LCD_DMA, "LCD - bppmode %d hwswp %d bswp %d\n", m_lcd_bppmode, m_lcd_hwswp, m_lcd_bswp);
	m_lcd_dma_data = 0;
	m_lcd_dma_bits = 0;
}

u32 s3c24xx_device::lcd_dma_read()
{
	u8 data[4];
	u8 *dst = &data[0];
	for (int i = 0; i < 2; i++)
	{
		if (m_lcd_hwswp == 0)
		{
			if (m_lcd_bswp == 0)
			{
				if ((m_lcd_vramaddr_cur & 2) == 0)
				{
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 3);
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 2);
				}
				else
				{
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur - 1);
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur - 2);
				}
			}
			else
			{
				*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 0);
				*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 1);
			}
		}
		else
		{
			if (m_lcd_bswp == 0)
			{
				*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 1);
				*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 0);
			}
			else
			{
				if ((m_lcd_vramaddr_cur & 2) == 0)
				{
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 2);
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur + 3);
				}
				else
				{
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur - 2);
					*dst++ = m_cache.read_byte(m_lcd_vramaddr_cur - 1);
				}
			}
		}
		m_lcd_vramaddr_cur += 2;
		m_lcd_pagewidth_cur++;
		if (m_lcd_pagewidth_cur >= m_lcd_pagewidth_max)
		{
			m_lcd_vramaddr_cur += m_lcd_offsize << 1;
			m_lcd_pagewidth_cur = 0;
		}
	}
	if (m_lcd_flags & INTERFACE_LCD_REVERSE)
	{
		return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0] << 0);
	}
	else
	{
		return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
	}
}

u32 s3c24xx_device::lcd_dma_read_bits(int count)
{
	u32 data;
	if (count <= m_lcd_dma_bits)
	{
		m_lcd_dma_bits -= count;
		data = BIT(m_lcd_dma_data, 32 - count, count);
		m_lcd_dma_data <<= count;
	}
	else
	{
		if (m_lcd_dma_bits == 0)
		{
			if (count == 32)
			{
				data = lcd_dma_read();
			}
			else
			{
				u32 temp = lcd_dma_read();
				data = BIT(temp, 32 - count, count);
				m_lcd_dma_data = temp << count;
				m_lcd_dma_bits = 32 - count;
			}
		}
		else
		{
			u32 temp = lcd_dma_read();
			data = (m_lcd_dma_data >> (32 - count)) | BIT(temp, 32 - (count - m_lcd_dma_bits), count - m_lcd_dma_bits);
			m_lcd_dma_data = temp << (count - m_lcd_dma_bits);
			m_lcd_dma_bits = 32 - (count - m_lcd_dma_bits);
		}
	}
	return data;
}

void s3c24xx_device::lcd_render_tpal()
{
	const u32 color = lcd_get_color_tpal();
	for (int y = m_lcd_vpos_min; y <= m_lcd_vpos_max; y++)
	{
		u32 *scanline = &m_lcd_bitmap[0].pix(y, m_lcd_hpos_min);
		for (int x = m_lcd_hpos_min; x <= m_lcd_hpos_max; x++)
		{
			*scanline++ = color;
		}
	}
}

void s3c24xx_device::lcd_render_stn_01()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 32; j++)
		{
			if (m_lcd_flags & INTERFACE_LCD_REVERSE)
			{
				*scanline++ = lcd_get_color_stn_01(data & 0x01);
				data >>= 1;
			}
			else
			{
				*scanline++ = lcd_get_color_stn_01((data >> 31) & 0x01);
				data <<= 1;
			}
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 4))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max) m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_stn_02()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 16; j++)
		{
			*scanline++ = lcd_get_color_stn_02((data >> 30) & 0x03);
			data <<= 2;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 3))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_stn_04()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 8; j++)
		{
			*scanline++ = lcd_get_color_stn_04((data >> 28) & 0x0f);
			data <<= 4;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 2))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_stn_08()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 4; j++)
		{
			*scanline++ = lcd_get_color_stn_08((data >> 24) & 0xff);
			data <<= 8;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 1))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_stn_12_p()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 16; i++)
	{
		*scanline++ = lcd_get_color_stn_12(lcd_dma_read_bits(12));
		m_lcd_hpos++;
		if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max * 16 / 12))
		{
			m_lcd_vpos++;
			if (m_lcd_vpos > m_lcd_vpos_max)
				m_lcd_vpos = m_lcd_vpos_min;
			m_lcd_hpos = m_lcd_hpos_min;
			scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
		}
	}
}

void s3c24xx_device::lcd_render_stn_12_u() // not tested
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 2; j++)
		{
			*scanline++ = lcd_get_color_stn_12((data >> 16) & 0x0fff);
			data <<= 16;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 0))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max) m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_tft_01()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 32; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 31) & 0x01);
			data <<= 1;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 4))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_tft_02()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 16; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 30) & 0x03);
			data <<= 2;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 3))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_tft_04()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 8; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 28) & 0x0f);
			data <<= 4;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 2))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_tft_08()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 4; j++)
		{
			*scanline++ = m_palette->pen_color((data >> 24) & 0xff);
			data <<= 8;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 1))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

void s3c24xx_device::lcd_render_tft_16()
{
	u32 *scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
	for (int i = 0; i < 4; i++)
	{
		u32 data = lcd_dma_read();
		for (int j = 0; j < 2; j++)
		{
			*scanline++ = lcd_get_color_tft_16(data >> 16);
			data <<= 16;
			m_lcd_hpos++;
			if (m_lcd_hpos >= m_lcd_hpos_min + (m_lcd_pagewidth_max << 0))
			{
				m_lcd_vpos++;
				if (m_lcd_vpos > m_lcd_vpos_max)
					m_lcd_vpos = m_lcd_vpos_min;
				m_lcd_hpos = m_lcd_hpos_min;
				scanline = &m_lcd_bitmap[0].pix(m_lcd_vpos, m_lcd_hpos);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::lcd_timer_exp)
{
	//LOGMASKED(LOG_LCD_TIMER, "LCD timer callback\n");
	m_lcd_vpos = m_screen->vpos();
	m_lcd_hpos = m_screen->hpos();
	//LOGMASKED(LOG_LCD_TIMER, "LCD - vpos %d hpos %d\n", m_lcd_vpos, m_lcd_hpos);
	const u32 tpalen = lcd_get_tpalen();
	if (tpalen == 0)
	{
		if (m_lcd_vramaddr_cur >= m_lcd_vramaddr_max)
		{
			lcd_dma_reload();
		}
		//LOGMASKED(LOG_LCD_TIMER, "LCD - vramaddr %08x bpp %d\n", m_lcd_vramaddr_cur, m_lcd_bppmode);
		while (m_lcd_vramaddr_cur < m_lcd_vramaddr_max)
		{
			switch (m_lcd_bppmode)
			{
			case BPPMODE_STN_01:    lcd_render_stn_01();   break;
			case BPPMODE_STN_02:    lcd_render_stn_02();   break;
			case BPPMODE_STN_04:    lcd_render_stn_04();   break;
			case BPPMODE_STN_08:    lcd_render_stn_08();   break;
			case BPPMODE_STN_12_P:  lcd_render_stn_12_p(); break;
			case BPPMODE_STN_12_U:  lcd_render_stn_12_u(); break;
			case BPPMODE_TFT_01:    lcd_render_tft_01();   break;
			case BPPMODE_TFT_02:    lcd_render_tft_02();   break;
			case BPPMODE_TFT_04:    lcd_render_tft_04();   break;
			case BPPMODE_TFT_08:    lcd_render_tft_08();   break;
			case BPPMODE_TFT_16:    lcd_render_tft_16();   break;
			default:  LOGMASKED(LOG_LCD_TIMER, "lcd_timer_exp: bppmode %d not supported\n", m_lcd_bppmode); break;
			}
			if ((m_lcd_vpos == m_lcd_vpos_min) && (m_lcd_hpos == m_lcd_hpos_min)) break;
		}
	}
	else
	{
		lcd_render_tpal();
	}
	m_lcd_timer->adjust(m_screen->time_until_pos(m_lcd_vpos, m_lcd_hpos));
}

void s3c24xx_device::video_start()
{
	std::fill(std::begin(m_lcdpal_data), std::end(m_lcdpal_data), 0);
	std::fill(std::begin(m_lcd_dithpat), std::end(m_lcd_dithpat), 0);
	save_item(NAME(m_lcdpal_data));
	save_item(NAME(m_lcd_dithpat));

	for (auto &bitmap : m_lcd_bitmap)
	{
		m_screen->register_screen_bitmap(bitmap);
		bitmap.fill(0);
	}

	m_cpu->space(AS_PROGRAM).cache(m_cache);
}

void s3c24xx_device::bitmap_blend(bitmap_rgb32 &bitmap_dst, bitmap_rgb32 &bitmap_src_1, bitmap_rgb32 &bitmap_src_2)
{
	for (int y = 0; y < bitmap_dst.height(); y++)
	{
		u32 const *const line0 = &bitmap_src_1.pix(y);
		u32 const *const line1 = &bitmap_src_2.pix(y);
		u32 *const line2 = &bitmap_dst.pix(y);
		for (int x = 0; x < bitmap_dst.width(); x++)
		{
			const u32 color0 = line0[x];
			const u32 color1 = line1[x];
			const u16 r0 = (color0 >> 16) & 0x000000ff;
			const u16 g0 = (color0 >>  8) & 0x000000ff;
			const u16 b0 = (color0 >>  0) & 0x000000ff;
			const u16 r1 = (color1 >> 16) & 0x000000ff;
			const u16 g1 = (color1 >>  8) & 0x000000ff;
			const u16 b1 = (color1 >>  0) & 0x000000ff;
			const u8 r = u8((r0 + r1) >> 1);
			const u8 g = u8((g0 + g1) >> 1);
			const u8 b = u8((b0 + b1) >> 1);
			line2[x] = (r << 16) | (g << 8) | b;
		}
	}
}

u32 s3c24xx_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_lcd_lcdcon1, 0))
	{
		if (m_lcd_framerate.as_hz() >= 1195)
		{
			bitmap_blend(bitmap, m_lcd_bitmap[0], m_lcd_bitmap[1]);
			copybitmap(m_lcd_bitmap[1], m_lcd_bitmap[0], 0, 0, 0, 0, cliprect);
		}
		else
		{
			copybitmap(bitmap, m_lcd_bitmap[0], 0, 0, 0, 0, cliprect);
		}
		lcd_dma_init();
	}
	return 0;
}

void s3c24xx_device::lcd_configure_tft()
{
	LOGMASKED(LOG_LCD_TFT, "lcd_configure_tft\n");
	const u32 vspw = BIT(m_lcd_lcdcon2, 0, 6);
	const u32 vbpd = BIT(m_lcd_lcdcon2, 24, 8);
	const u32 lineval = BIT(m_lcd_lcdcon2, 14, 10);
	const u32 vfpd = BIT(m_lcd_lcdcon2, 6, 8);
	const u32 hspw = BIT(m_lcd_lcdcon4, 0, 8);
	const u32 hbpd = BIT(m_lcd_lcdcon3, 19, 7);
	const u32 hfpd = BIT(m_lcd_lcdcon3, 0, 8);
	const u32 hozval = BIT(m_lcd_lcdcon3, 8, 11);
	const u32 clkval = BIT(m_lcd_lcdcon1, 8, 10);
	const u64 full_clock = get_full_clock();
	const u32 hclk_ticks = get_hclk_ticks();
	LOGMASKED(LOG_LCD_TFT, "LCD - vspw %d vbpd %d lineval %d vfpd %d hspw %d hbpd %d hfpd %d hozval %d clkval %d hclk %d\n", vspw, vbpd, lineval, vfpd, hspw, hbpd, hfpd, hozval, clkval, (int)(full_clock / hclk_ticks));

	const u32 vclk_ticks = hclk_ticks * ((clkval + 1) * 2);
	LOGMASKED(LOG_LCD_TFT, "LCD - vclk %f\n", full_clock / vclk_ticks);

	const u32 framerate_ticks = vclk_ticks * (((vspw + 1) + (vbpd + 1) + (lineval + 1) + (vfpd + 1)) * ((hspw + 1) + (hbpd + 1) + (hozval + 1) + (hfpd + 1)));
	LOGMASKED(LOG_LCD_TFT, "LCD - framerate %f\n", full_clock / framerate_ticks);
	m_lcd_framerate = attotime::from_ticks(framerate_ticks, full_clock);

	u32 width = (hspw + 1) + (hbpd + 1) + (hozval + 1) + (hfpd + 1);
	u32 height = (vspw + 1) + (vbpd + 1) + (lineval + 1) + (vfpd + 1);

	rectangle visarea;
	visarea.min_x = (hspw + 1) + (hbpd + 1);
	visarea.min_y = (vspw + 1) + (vbpd + 1);
	visarea.max_x = visarea.min_x + (hozval + 1) - 1;
	visarea.max_y = visarea.min_y + (lineval + 1) - 1;
	LOGMASKED(LOG_LCD_TFT, "LCD - visarea min_x %d min_y %d max_x %d max_y %d\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	LOGMASKED(LOG_LCD_TFT, "screen->configure %d %d %f\n", width, height, m_lcd_framerate.as_hz());

	m_lcd_hpos_min = (hspw + 1) + (hbpd + 1);
	m_lcd_hpos_max = m_lcd_hpos_min + (hozval + 1) - 1;
	m_lcd_vpos_min = (vspw + 1) + (vbpd + 1);
	m_lcd_vpos_max = m_lcd_vpos_min + (lineval + 1) - 1;
	m_screen->configure(width, height, visarea, m_lcd_framerate);
}

bool s3c24xx_device::lcd_configure_stn()
{
	LOGMASKED(LOG_LCD_STN, "lcd_configure_stn\n");

	const u32 pnrmode = BIT(m_lcd_lcdcon1, 5, 2);
	const u32 bppmode = BIT(m_lcd_lcdcon1, 1, 4);
	const u32 clkval = BIT(m_lcd_lcdcon1, 8, 10);
	const u32 lineval = BIT(m_lcd_lcdcon2, 14, 10);
	const u32 wdly = BIT(m_lcd_lcdcon3, 19, 2);
	const u32 hozval = BIT(m_lcd_lcdcon3, 8, 11);
	const u32 lineblank = BIT(m_lcd_lcdcon3, 0, 8);
	const u32 wlh = BIT(m_lcd_lcdcon4, 0, 2);
	const u64 full_clock = get_full_clock();
	const u32 hclk_ticks = get_hclk_ticks();
	LOGMASKED(LOG_LCD_STN, "LCD - pnrmode %d bppmode %d clkval %d lineval %d wdly %d hozval %d lineblank %d wlh %d hclk %d\n", pnrmode, bppmode, clkval, lineval, wdly, hozval, lineblank, wlh, full_clock / hclk_ticks);
	if (clkval == 0)
	{
		return false;
	}

	const u32 vclk_ticks = hclk_ticks * (clkval * 2);
	LOGMASKED(LOG_LCD_STN, "LCD - vclk %f\n", full_clock / vclk_ticks);

	const u32 framerate_ticks = (vclk_ticks * (hozval + 1) + hclk_ticks * ((1 << (4 + wlh)) + (1 << (4 + wdly)) + (lineblank * 8))) * (lineval + 1);
	LOGMASKED(LOG_LCD_STN, "LCD - framerate %f\n", full_clock / framerate_ticks);
	m_lcd_framerate = attotime::from_ticks(framerate_ticks, full_clock);

	u32 width = 0;
	switch (pnrmode)
	{
		case PNRMODE_STN_04_SS: width = ((hozval + 1) * 4); break;
		case PNRMODE_STN_04_DS: width = ((hozval + 1) * 4); break;
		case PNRMODE_STN_08_SS: width = ((hozval + 1) * 8 / 3); break;
		default: break;
	}

	u32 height = lineval + 1;

	rectangle visarea;
	visarea.set(0, width - 1, 0, height - 1);
	LOGMASKED(LOG_LCD_STN, "LCD - visarea min_x %d min_y %d max_x %d max_y %d\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	LOGMASKED(LOG_LCD_STN, "screen->configure %d %d %f\n", width, height, m_lcd_framerate.as_hz());

	m_lcd_hpos_min = 0;
	m_lcd_hpos_max = width - 1;
	m_lcd_vpos_min = 0;
	m_lcd_vpos_max = height - 1;
	m_screen->configure(width, height, visarea, m_lcd_framerate);
	return true;
}

bool s3c24xx_device::lcd_configure()
{
	LOGMASKED(LOG_LCD_CFG, "lcd_configure\n");
	u32 bppmode = BIT(m_lcd_lcdcon1, 1, 4);
	if ((bppmode & (1 << 3)) == 0)
	{
		return lcd_configure_stn();
	}
	lcd_configure_tft();
	return true;
}

void s3c24xx_device::lcd_start()
{
	LOGMASKED(LOG_LCD_CFG, "LCD start\n");
	if (lcd_configure())
	{
		lcd_dma_init();
		m_lcd_timer->adjust(m_screen->time_until_pos(m_lcd_vpos_min, m_lcd_hpos_min));
	}
}

void s3c24xx_device::lcd_stop()
{
	LOGMASKED(LOG_LCD_CFG, "LCD stop\n");
	m_lcd_timer->adjust(attotime::never);
}

void s3c24xx_device::lcd_recalc()
{
	if (m_lcd_lcdcon1 & (1 << 0))
		lcd_start();
	else
		lcd_stop();
}

u32 s3c24xx_device::lcd_lcdcon1_r(offs_t offset, u32 mem_mask)
{
	u32 vpos = m_screen->vpos();
	if (vpos < m_lcd_vpos_min)
		vpos = m_lcd_vpos_min;
	if (vpos > m_lcd_vpos_max)
		vpos = m_lcd_vpos_max;

	const u32 data = (m_lcd_lcdcon1 & ~0xfffc0000) | ((m_lcd_vpos_max - vpos) << 18);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDCON1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdcon1_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_lcd_lcdcon1;
	COMBINE_DATA(&m_lcd_lcdcon1);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDCON1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (BIT(old, 0) != BIT(m_lcd_lcdcon1, 0))
	{
		lcd_recalc();
	}
}

u32 s3c24xx_device::lcd_lcdcon2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdcon2;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDCON2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdcon2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdcon2);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDCON2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_lcdcon3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdcon3;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDCON3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdcon3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdcon3);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDCON3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_lcdcon4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdcon4;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDCON4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdcon4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdcon4);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDCON4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_lcdcon5_r(offs_t offset, u32 mem_mask)
{
	const u32 vpos = m_screen->vpos();
	u32 data = m_lcd_lcdcon5 & ~0x00018000;
	if (vpos < m_lcd_vpos_min)
		data = data | 0x00000000;
	if (vpos > m_lcd_vpos_max)
		data = data | 0x00018000;
	// todo: 00 = VSYNC, 01 = BACK Porch, 10 = ACTIVE, 11 = FRONT Porch
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDCON5: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdcon5_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdcon5);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDCON5 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_lcdsaddr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdsaddr1;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDSADDR1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdsaddr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdsaddr1);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDSADDR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_lcdsaddr2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdsaddr2;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDSADDR2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdsaddr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdsaddr2);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDSADDR2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_lcdsaddr3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdsaddr3;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDSADDR3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_lcdsaddr3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdsaddr3);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDSADDR3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_redlut_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_redlut;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: REDLUT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_redlut_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_redlut);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: REDLUT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_greenlut_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_greenlut;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: GREENLUT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_greenlut_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_greenlut);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: GREENLUT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_bluelut_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_bluelut;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: BLUELUT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_bluelut_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_bluelut);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: BLUELUT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp12_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[0];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP1_2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp12_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[0]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP1_2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp47_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[1];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP4_7: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp47_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[1]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP4_7 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp35_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[2];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP3_5: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp35_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[2]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP3_5 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp23_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[3];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP2_3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp23_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[3]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP2_3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp57_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[4];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP5_7: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp57_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[4]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP5_7 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp34_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[5];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP3_4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp34_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[5]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP3_4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp45_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[6];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP4_5: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp45_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[6]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP4_5 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dp67_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithpat[7];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DP6_7: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dp67_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithpat[7]);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DP6_7 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_dithmode_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_dithmode;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: DITHMODE: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_dithmode_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_dithmode);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: DITHMODE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::lcd_tpal_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_tpal;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: TPAL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_tpal_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_tpal);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: TPAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::lcd_lcdintpnd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdintpnd;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDINTPND: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::lcd_lcdintpnd_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdintpnd);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDINTPND = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::lcd_lcdsrcpnd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdsrcpnd;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDSRCPND: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::lcd_lcdsrcpnd_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdsrcpnd);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDSRCPND = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::lcd_lcdintmsk_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lcdintmsk;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LCDINTMSK: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::lcd_lcdintmsk_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lcdintmsk);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LCDINTMSK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::lcd_lpcsel_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_lpcsel;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: LPCSEL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::lcd_lpcsel_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_lpcsel);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: LPCSEL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::lcd_tconsel_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_tconsel;
	LOGMASKED(LOG_LCD_REGS, "%s: lcd read: TCONSEL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::lcd_tconsel_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd_tconsel);
	LOGMASKED(LOG_LCD_REGS, "%s: lcd write: TCONSEL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* LCD Palette */

u32 s3c24xx_device::lcd_palette_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcdpal_data[offset];
	LOGMASKED(LOG_LCD_REGS, "%s: lcd palette read: %08X = %08X & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
	return data;
}

void s3c24xx_device::lcd_palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD_REGS, "%s: lcd palette write: %08X = %08X & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
	COMBINE_DATA(&m_lcdpal_data[offset]);
	if (mem_mask != 0xffffffff)
	{
		LOGMASKED(LOG_LCD_REGS, "lcd_palette_w: unknown mask %08x\n", mem_mask);
	}
	m_palette->set_pen_color(offset, lcd_get_color_tft_16(data & 0xffff));
}

/* Clock & Power Management */

void s3c24xx_device::clkpow_reset()
{
	m_clkpow_clkslow = 4;
}

void s3c2400_device::clkpow_reset()
{
	s3c24xx_device::clkpow_reset();
	m_clkpow_locktime = 0x00ffffff;
	m_clkpow_mpllcon  = 0x0005c080;
	m_clkpow_upllcon  = 0x00028080;
	m_clkpow_clkcon   = 0x0000fff8;
	m_clkpow_clkdivn  = 0;
}

void s3c2410_device::clkpow_reset()
{
	s3c24xx_device::clkpow_reset();
	m_clkpow_locktime = 0x00ffffff;
	m_clkpow_mpllcon  = 0x0005c080;
	m_clkpow_upllcon  = 0x00028080;
	m_clkpow_clkcon   = 0x0007fff0;
	m_clkpow_clkdivn  = 0;
}

void s3c2440_device::clkpow_reset()
{
	s3c24xx_device::clkpow_reset();
	m_clkpow_locktime = 0xffffffff;
	m_clkpow_mpllcon  = 0x00096030;
	m_clkpow_upllcon  = 0x0004d030;
	m_clkpow_clkcon   = 0x00fffff0;
	m_clkpow_clkdivn  = 0;
	m_clkpow_camdivn  = 0;
}

u32 s3c24xx_device::get_clk_mult() const
{
	return 1;
}

u32 s3c2440_device::get_clk_mult() const
{
	return 2;
}

u64 s3c24xx_device::get_full_clock() const
{
	const u64 mdiv = (m_clkpow_mpllcon >> 12) & 0xff;
	return get_clk_mult() * (mdiv + 8) * 12000000;
}

u64 s3c24xx_device::get_fclk_ticks() const
{
	const u32 sdiv = m_clkpow_mpllcon & 0x03;
	const u32 pdiv = ((m_clkpow_mpllcon >> 4) & 0x1f) + 2;
	u32 ticks = pdiv * (1 << sdiv);

	if (BIT(m_clkpow_clkslow, 4))
	{
		const u32 slow_val = BIT(m_clkpow_clkslow, 0, 3);
		if (slow_val > 0)
		{
			ticks *= 2 * slow_val;
		}
	}
	return ticks;
}

attotime s3c24xx_device::get_fclk() const
{
	LOGMASKED(LOG_CLKPOW, "%s: get_fclk(): %d ticks, %d clock\n", machine().describe_context(), (u32)get_fclk_ticks(), (u32)get_full_clock());
	return attotime::from_ticks(get_fclk_ticks(), get_full_clock());
}

u64 s3c24xx_device::get_hclk_ticks() const
{
	return get_fclk_ticks() * (BIT(m_clkpow_clkdivn, 1) + 1);
}

u64 s3c2440_device::get_hclk_ticks() const
{
	const u32 divisors[4] = { 1, 2, 4 * (BIT(m_clkpow_camdivn, 9) + 1), 3 * (BIT(m_clkpow_camdivn, 8) + 1) };
	return get_fclk_ticks() * divisors[(m_clkpow_clkdivn >> 1) & 3];
}

attotime s3c24xx_device::get_hclk() const
{
	return attotime::from_ticks(get_hclk_ticks(), get_full_clock());
}

u64 s3c24xx_device::get_pclk_ticks() const
{
	return get_hclk_ticks() * (1 << BIT(m_clkpow_clkdivn, 0));
}

attotime s3c24xx_device::get_pclk() const
{
	return attotime::from_ticks(get_pclk_ticks(), get_full_clock());
}

u32 s3c24xx_device::clkpow_locktime_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clkpow_locktime;
	LOGMASKED(LOG_CLKPOW, "%s: clock/power read: LOCKTIME: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::clkpow_locktime_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_clkpow_locktime);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: LOCKTIME = %08x & %08x - fclk %d hclk %d pclk %d\n", machine().describe_context(), data, mem_mask, (int)get_fclk().as_hz(), (int)get_hclk().as_hz(), (int)get_pclk().as_hz());
}

u32 s3c24xx_device::clkpow_mpllcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clkpow_mpllcon;
	LOGMASKED(LOG_CLKPOW, "%s: clock/power read: MPLLCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::clkpow_mpllcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_clkpow_mpllcon);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: MPLLCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_CLKPOW, "%s:                    Post Divider: %d\n", machine().describe_context(), data & 0x03);
	LOGMASKED(LOG_CLKPOW, "%s:                    Pre-Divider: %d\n", machine().describe_context(), (data >> 4) & 0x1f);
	LOGMASKED(LOG_CLKPOW, "%s:                    Main Divider: %d\n", machine().describe_context(), (data >> 12) & 0xff);
	LOGMASKED(LOG_CLKPOW, "%s:                    FCLK: %9.1fHz\n", machine().describe_context(), get_fclk().as_hz());
	LOGMASKED(LOG_CLKPOW, "%s:                    HCLK: %9.1fHz\n", machine().describe_context(), get_hclk().as_hz());
	LOGMASKED(LOG_CLKPOW, "%s:                    PCLK: %9.1fHz\n", machine().describe_context(), get_pclk().as_hz());
	m_cpu->set_unscaled_clock((u32)get_fclk().as_hz() * CLOCK_MULTIPLIER);
	iis_recalc();
}

u32 s3c24xx_device::clkpow_upllcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clkpow_upllcon;
	LOGMASKED(LOG_CLKPOW, "%s: clock/power read: UPLLCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::clkpow_upllcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_clkpow_upllcon);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: UPLLCON = %08x & %08x - fclk %d hclk %d pclk %d\n", machine().describe_context(), data, mem_mask, (int)get_fclk().as_hz(), (int)get_hclk().as_hz(), (int)get_pclk().as_hz());
}

u32 s3c24xx_device::clkpow_clkcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clkpow_clkcon;
	//LOGMASKED(LOG_CLKPOW, "%s: clock/power read: CLKCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::clkpow_clkcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_clkpow_clkcon;
	COMBINE_DATA(&m_clkpow_clkcon);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: CLKCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	// An interrupt may arrive between the CPU's last check and entering idle.
	if (BIT(m_clkpow_clkcon, 2) && !BIT(old, 2) && (m_irq_line == CLEAR_LINE) && (m_fiq_line == CLEAR_LINE))
		m_cpu->suspend(SUSPEND_REASON_HALT, 1);
}

u32 s3c24xx_device::clkpow_clkslow_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clkpow_clkslow;
	LOGMASKED(LOG_CLKPOW, "%s: clock/power read: CLKSLOW: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::clkpow_clkslow_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_clkpow_clkslow);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: CLKSLOW = %08x & %08x - fclk %d hclk %d pclk %d\n", machine().describe_context(), data, mem_mask, (int)get_fclk().as_hz(), (int)get_hclk().as_hz(), (int)get_pclk().as_hz());
	m_cpu->set_unscaled_clock((u32)get_fclk().as_hz() * CLOCK_MULTIPLIER);
	iis_recalc();
}

u32 s3c24xx_device::clkpow_clkdivn_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clkpow_clkdivn;
	LOGMASKED(LOG_CLKPOW, "%s: clock/power read: CLKDIVN: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::clkpow_clkdivn_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_clkpow_clkdivn);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: CLKDIVN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_CLKPOW, "%s:                    HDIVN: HCLK = %s\n", machine().describe_context(), BIT(data, 1) ? "FCLK/2" : "FCLK");
	LOGMASKED(LOG_CLKPOW, "%s:                    PDIVN: %s\n", machine().describe_context(), BIT(data, 0) ? "PCLK = HCLK/2" : "PCLK = HCLK");
	LOGMASKED(LOG_CLKPOW, "%s:                    FCLK: %9.1fHz\n", machine().describe_context(), get_fclk().as_hz());
	LOGMASKED(LOG_CLKPOW, "%s:                    HCLK: %9.1fHz\n", machine().describe_context(), get_hclk().as_hz());
	LOGMASKED(LOG_CLKPOW, "%s:                    PCLK: %9.1fHz\n", machine().describe_context(), get_pclk().as_hz());
	iis_recalc();
}

void s3c2440_device::clkpow_clkdivn_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_clkpow_clkdivn);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: CLKDIVN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	switch ((data >> 1) & 3)
	{
	case 0:
		LOGMASKED(LOG_CLKPOW, "%s:                    HDIVN: HCLK = FCLK\n", machine().describe_context());
		break;
	case 1:
		LOGMASKED(LOG_CLKPOW, "%s:                    HDIVN: HCLK = FCLK/2\n", machine().describe_context());
		break;
	case 2:
		LOGMASKED(LOG_CLKPOW, "%s:                    HDIVN: HCLK = %s\n", machine().describe_context(), BIT(m_clkpow_camdivn, 9) ? "FCLK/8" : "FCLK/4");
		break;
	case 3:
		LOGMASKED(LOG_CLKPOW, "%s:                    HDIVN: HCLK = %s\n", machine().describe_context(), BIT(m_clkpow_camdivn, 8) ? "FCLK/6" : "FCLK/3");
		break;
	}
	LOGMASKED(LOG_CLKPOW, "%s:                    PDIVN: %s\n", machine().describe_context(), BIT(data, 0) ? "PCLK = HCLK/2" : "PCLK = HCLK");
	iis_recalc();
}

u32 s3c2440_device::clkpow_camdivn_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clkpow_camdivn;
	LOGMASKED(LOG_CLKPOW, "%s: clock/power read: CAMDIVN: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::clkpow_camdivn_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_clkpow_camdivn);
	LOGMASKED(LOG_CLKPOW, "%s: clock/power write: CAMDIVN = %08x & %08x - fclk %d hclk %d pclk %d\n", machine().describe_context(), data, mem_mask, (int)get_fclk().as_hz(), (int)get_hclk().as_hz(), (int)get_pclk().as_hz());
	iis_recalc();
}

/* Interrupt Controller */

void s3c24xx_device::irq_reset()
{
	m_irq_srcpnd = 0;
	m_irq_intmod = 0;
	m_irq_intmsk = ~0u;
	m_irq_priority = 0x7f;
	m_irq_intpnd = 0;
	m_irq_intoffset = 0;
	m_irq_line = CLEAR_LINE;
	m_fiq_line = CLEAR_LINE;
}

void s3c2410_device::irq_reset()
{
	s3c24xx_device::irq_reset();
	m_irq_subsrcpnd = 0;
	m_irq_intsubmsk = 0x07ff;
}

void s3c2440_device::irq_reset()
{
	s3c24xx_device::irq_reset();
	m_irq_subsrcpnd = 0;
	m_irq_intsubmsk = 0xffff;
}

void s3c24xx_device::check_pending_irq()
{
	// normal irq
	LOGMASKED(LOG_IRQS, "IRQ: intpnd %08X intoffs %08X srcpnd %08X intmsk %08X intmod %08X\n", m_irq_intpnd, m_irq_intoffset, m_irq_srcpnd, m_irq_intmsk, m_irq_intmod);
	if ((m_irq_intpnd == 0) && (m_irq_intoffset == 0)) // without this "touryuu" crashes
	{
		u32 temp = (m_irq_srcpnd & ~m_irq_intmsk) & ~m_irq_intmod;
		if (temp != 0)
		{
			u32 int_type = 0;
			LOGMASKED(LOG_IRQS, "IRQ: srcpnd %08X intmsk %08X intmod %08X\n", m_irq_srcpnd, m_irq_intmsk, m_irq_intmod);
			while ((temp & 1) == 0)
			{
				int_type++;
				temp >>= 1;
			}
			LOGMASKED(LOG_IRQS, "IRQ: intpnd set bit %d\n", int_type);
			m_irq_intpnd |= (1 << int_type);
			m_irq_intoffset = int_type;
			if (m_irq_line != ASSERT_LINE)
			{
				LOGMASKED(LOG_IRQS, "triggering IRQ line\n");
				m_cpu->resume(SUSPEND_REASON_HALT);
				m_cpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
				m_irq_line = ASSERT_LINE;
			}
		}
		else
		{
			if (m_irq_line != CLEAR_LINE)
			{
				LOGMASKED(LOG_IRQS, "IRQ: srcpnd %08X intmsk %08X intmod %08X\n", m_irq_srcpnd, m_irq_intmsk, m_irq_intmod);
				LOGMASKED(LOG_IRQS, "clearing IRQ line\n");
				m_cpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);
				m_irq_line = CLEAR_LINE;
			}
		}
	}

	// fast irq
	u32 temp = (m_irq_srcpnd & ~m_irq_intmsk) & m_irq_intmod;
	if (temp != 0)
	{
		if (m_fiq_line != ASSERT_LINE)
		{
			LOGMASKED(LOG_IRQS, "asserting FIQ line\n");
			m_cpu->resume(SUSPEND_REASON_HALT);
			m_cpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, ASSERT_LINE);
			m_fiq_line = ASSERT_LINE;
		}
	}
	else
	{
		if (m_fiq_line != CLEAR_LINE)
		{
			LOGMASKED(LOG_IRQS, "clearing FIQ line\n");
			m_cpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, CLEAR_LINE);
			m_fiq_line = CLEAR_LINE;
		}
	}
}

u32 s3c2400_device::get_irq_bit_for_int_type(u32 int_type) const
{
	static constexpr u32 INT_TYPE_TO_BIT[INT_COUNT] = {
		31,     // INT_ADC
		30,     // INT_RTC
		22,     // INT_SPI
		~0u,    // INT_SPI0
		~0u,    // INT_SPI1
		~0u,    // INT_UART0
		~0u,    // INT_UART1
		~0u,    // INT_UART2
		28,     // INT_UTXD0
		29,     // INT_UTXD1
		23,     // INT_URXD0
		24,     // INT_URXD1
		15,     // INT_UERR
		28,     // INT_IIC
		26,     // INT_USBH
		25,     // INT_USBD
		~0u,    // INT_24
		~0u,    // INT_SDI
		21,     // INT_MMC
		17,     // INT_DMA0
		18,     // INT_DMA1
		19,     // INT_DMA2
		20,     // INT_DMA3
		~0u,    // INT_LCD
		10,     // INT_TIMER0
		11,     // INT_TIMER1
		12,     // INT_TIMER2
		13,     // INT_TIMER3
		14,     // INT_TIMER4
		9,      // INT_WDT
		~0u,    // INT_NFCON
		8,      // INT_TICK
		~0u,    // INT_BATT_FLT
		~0u,    // INT_6
		~0u,    // INT_CAM
		0,      // INT_EINT0
		1,      // INT_EINT1
		2,      // INT_EINT2
		3,      // INT_EINT3
		4,      // INT_EINT4
		5,      // INT_EINT5
		6,      // INT_EINT6
		7,      // INT_EINT7
		~0u,    // INT_EINT4_7
		~0u     // INT_EINT8_23
	};
	return INT_TYPE_TO_BIT[int_type];
}

u32 s3c2410_device::get_irq_bit_for_int_type(u32 int_type) const
{
	static constexpr u32 INT_TYPE_TO_BIT[INT_COUNT] = {
		31,     // INT_ADC
		30,     // INT_RTC
		~0u,    // INT_SPI
		22,     // INT_SPI0
		29,     // INT_SPI1
		28,     // INT_UART0
		23,     // INT_UART1
		15,     // INT_UART2
		~0u,    // INT_UTXD0
		~0u,    // INT_UTXD1
		~0u,    // INT_URXD0
		~0u,    // INT_URXD1
		~0u,    // INT_UERR
		27,     // INT_IIC
		26,     // INT_USBH
		25,     // INT_USBD
		24,     // INT_24
		21,     // INT_SDI
		~0u,    // INT_MMC
		17,     // INT_DMA0
		18,     // INT_DMA1
		19,     // INT_DMA2
		20,     // INT_DMA3
		16,     // INT_LCD
		10,     // INT_TIMER0
		11,     // INT_TIMER1
		12,     // INT_TIMER2
		13,     // INT_TIMER3
		14,     // INT_TIMER4
		9,      // INT_WDT
		~0u,    // INT_NFCON
		8,      // INT_TICK
		7,      // INT_BATT_FLT
		6,      // INT_6
		~0u,    // INT_CAM
		0,      // INT_EINT0
		1,      // INT_EINT1
		2,      // INT_EINT2
		3,      // INT_EINT3
		~0u,    // INT_EINT4
		~0u,    // INT_EINT5
		~0u,    // INT_EINT6
		~0u,    // INT_EINT7
		4,      // INT_EINT4_7
		5       // INT_EINT8_23
	};
	return INT_TYPE_TO_BIT[int_type];
}

u32 s3c2440_device::get_irq_bit_for_int_type(u32 int_type) const
{
	static constexpr u32 INT_TYPE_TO_BIT[INT_COUNT] = {
		31,     // INT_ADC
		30,     // INT_RTC
		~0u,    // INT_SPI
		22,     // INT_SPI0
		29,     // INT_SPI1
		28,     // INT_UART0
		23,     // INT_UART1
		15,     // INT_UART2
		~0u,    // INT_UTXD0
		~0u,    // INT_UTXD1
		~0u,    // INT_URXD0
		~0u,    // INT_URXD1
		~0u,    // INT_UERR
		27,     // INT_IIC
		26,     // INT_USBH
		25,     // INT_USBD
		~0u,    // INT_24
		21,     // INT_SDI
		~0u,    // INT_MMC
		17,     // INT_DMA0
		18,     // INT_DMA1
		19,     // INT_DMA2
		20,     // INT_DMA3
		16,     // INT_LCD
		10,     // INT_TIMER0
		11,     // INT_TIMER1
		12,     // INT_TIMER2
		13,     // INT_TIMER3
		14,     // INT_TIMER4
		9,      // INT_WDT
		24,     // INT_NFCON
		8,      // INT_TICK
		7,      // INT_BATT_FLT
		6,      // INT_CAM
		0,      // INT_EINT0
		1,      // INT_EINT1
		2,      // INT_EINT2
		3,      // INT_EINT3
		~0u,    // INT_EINT4
		~0u,    // INT_EINT5
		~0u,    // INT_EINT6
		~0u,    // INT_EINT7
		4,      // INT_EINT4_7
		5       // INT_EINT8_23
	};
	return INT_TYPE_TO_BIT[int_type];
}

void s3c24xx_device::request_irq(u32 int_type)
{
	const u32 int_bit = get_irq_bit_for_int_type(int_type);
	if (int_bit != 14 && int_bit != 10)
		LOGMASKED(LOG_IRQS, "request irq %d\n", int_bit);
	m_irq_srcpnd |= (1 << int_bit);
	check_pending_irq();
}

u32 s3c2410_device::get_int_type_for_subint(u32 subint) const
{
	static constexpr u32 SUBINT_TO_INT_TYPE[INT_COUNT] = {
		INT_UART0,  // SUBINT_RXD0
		INT_UART0,  // SUBINT_TXD0
		INT_UART0,  // SUBINT_ERR0
		INT_UART1,  // SUBINT_RXD1
		INT_UART1,  // SUBINT_TXD1
		INT_UART1,  // SUBINT_ERR1
		INT_UART2,  // SUBINT_RXD2
		INT_UART2,  // SUBINT_TXD2
		INT_UART2,  // SUBINT_ERR2
		INT_ADC,    // SUBINT_TC
		INT_ADC,    // SUBINT_ADC
		~0u,        // SUBINT_CAM_C
		~0u,        // SUBINT_CAM_P
		~0u,        // SUBINT_WDT
		~0u,        // SUBINT_AC97
	};
	return SUBINT_TO_INT_TYPE[subint];
}

u32 s3c2440_device::get_int_type_for_subint(u32 subint) const
{
	static constexpr u32 SUBINT_TO_INT_TYPE[INT_COUNT] = {
		INT_UART0,  // SUBINT_RXD0
		INT_UART0,  // SUBINT_TXD0
		INT_UART0,  // SUBINT_ERR0
		INT_UART1,  // SUBINT_RXD1
		INT_UART1,  // SUBINT_TXD1
		INT_UART1,  // SUBINT_ERR1
		INT_UART2,  // SUBINT_RXD2
		INT_UART2,  // SUBINT_TXD2
		INT_UART2,  // SUBINT_ERR2
		INT_ADC,    // SUBINT_TC
		INT_ADC,    // SUBINT_ADC
		INT_CAM,    // SUBINT_CAM_C
		INT_CAM,    // SUBINT_CAM_P
		INT_WDT,    // SUBINT_WDT
		INT_WDT,    // SUBINT_AC97
	};
	return SUBINT_TO_INT_TYPE[subint];
}

void s3c2410_device::check_pending_irq()
{
	// Each UART has three sub-interrupts sharing one level-sensitive parent.
	const u32 pending = m_irq_subsrcpnd & ~m_irq_intsubmsk;
	for (u32 bit = 0; bit < 15; bit++)
	{
		if (BIT(pending, bit))
		{
			const u32 type = get_int_type_for_subint(bit);
			if (type != ~0U)
				m_irq_srcpnd |= 1U << get_irq_bit_for_int_type(type);
		}
	}
	s3c24xx_device::check_pending_irq();
}

void s3c2410_device::request_subirq(u32 int_type)
{
	LOGMASKED(LOG_IRQS, "request subirq %d\n", int_type);
	m_irq_subsrcpnd |= 1 << int_type;
	check_pending_irq();
}

void s3c2410_device::check_pending_eint()
{
	u32 temp = m_gpio_eintpend & ~m_gpio_eintmask;
	if (temp != 0)
	{
		u32 int_idx = 0;
		while ((temp & 1) == 0)
		{
			int_idx++;
			temp >>= 1;
		}

		if (int_idx < 8)
		{
			request_irq(INT_EINT4_7);
		}
		else
		{
			request_irq(INT_EINT8_23);
		}
	}
}

void s3c2410_device::gpio_set_extint(u32 line, int state)
{
	if (line >= 24)
	{
		return;
	}

	const u32 old = m_gpio_extint_lines;
	m_gpio_extint_lines &= ~(1 << line);
	m_gpio_extint_lines |= state << line;

	const u32 raised = ~old & m_gpio_extint_lines;
	const u32 lowered = old & ~m_gpio_extint_lines;
	const u32 changed = raised | lowered;
	if (!changed)
	{
		return;
	}

	const u32 mode = (m_gpio_extint[line >> 3] >> (4 * (line & 7))) & 7;

	switch (mode)
	{
		case 0: // Trigger EINT on low level
			if (BIT(m_gpio_extint_lines, line))
			{
				return;
			}
			break;
		case 1: // Trigger EINT on high level
			if (!BIT(m_gpio_extint_lines, line))
			{
				return;
			}
			break;
		case 2:
		case 3: // Trigger EINT on falling edge
			if (!BIT(lowered, line))
			{
				return;
			}
			break;
		case 4:
		case 5: // Trigger EINT on rising edge
			if (!BIT(raised, line))
			{
				return;
			}
			break;
		case 6:
		case 7: // Trigger EINT on either edge
			if (!BIT(changed, line))
			{
				return;
			}
			break;
	}

	request_eint(line);
}

void s3c2410_device::request_eint(u32 number)
{
	LOGMASKED(LOG_IRQS, "request external interrupt %d\n", number);
	if (number < 4)
	{
		request_irq(INT_EINT0 + number);
	}
	else
	{
		m_gpio_eintpend |= (1 << number);
		check_pending_eint();
	}
}

u32 s3c24xx_device::irq_srcpnd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_srcpnd;
	LOGMASKED(LOG_IRQ_REGS, "%s: irq read: SRCPND: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::irq_srcpnd_w(offs_t offset, u32 data, u32 mem_mask)
{
	//const u32 old = m_irq_srcpnd;
	m_irq_srcpnd &= ~(data & mem_mask); // clear only the bit positions of SRCPND corresponding to those set to one in the data
	//LOGMASKED(LOG_IRQ_REGS, "%s: irq write: SRCPND = %08x & %08x (old %08x)\n", machine().describe_context(), data, mem_mask, old);
	check_pending_irq();
}

u32 s3c24xx_device::irq_intmod_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_intmod;
	LOGMASKED(LOG_IRQ_REGS, "%s: irq read: INTMOD: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::irq_intmod_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_irq_intmod);
	LOGMASKED(LOG_IRQ_REGS, "%s: irq write: INTMOD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::irq_intmsk_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_intmsk;
	LOGMASKED(LOG_IRQ_REGS, "%s: irq read: INTMSK: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::irq_intmsk_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_irq_intmsk);
	LOGMASKED(LOG_IRQ_REGS, "%s: irq write: INTMSK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	check_pending_irq();
}

u32 s3c24xx_device::irq_priority_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_priority;
	LOGMASKED(LOG_IRQ_REGS, "%s: irq read: PRIORITY: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::irq_priority_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_irq_priority);
	LOGMASKED(LOG_IRQ_REGS, "%s: irq write: PRIORITY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::irq_intpnd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_intpnd;
	//LOGMASKED(LOG_IRQ_REGS, "%s: irq read: INTPND: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::irq_intpnd_w(offs_t offset, u32 data, u32 mem_mask)
{
	//const u32 old = m_irq_intpnd;
	m_irq_intpnd &= ~(data & mem_mask); // clear only the bit positions of INTPND corresponding to those set to one in the data
	if (!m_irq_intpnd)
		m_irq_intoffset = 0;
	//LOGMASKED(LOG_IRQ_REGS, "%s: irq write: INTPND = %08x & %08x (old %08x)\n", machine().describe_context(), data, mem_mask, old);
	check_pending_irq();
}

u32 s3c24xx_device::irq_intoffset_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_intoffset;
	//LOGMASKED(LOG_IRQ_REGS, "%s: irq read: INTOFFSET: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::irq_intoffset_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_irq_intoffset);
	LOGMASKED(LOG_IRQ_REGS, "%s: irq write: INTOFFSET = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::irq_subsrcpnd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_subsrcpnd;
	LOGMASKED(LOG_IRQ_REGS, "%s: irq read: SUBSRCPND: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::irq_subsrcpnd_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_IRQ_REGS, "%s: irq write: SUBSRCPND = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_irq_subsrcpnd &= ~(data & mem_mask); // clear only the bit positions of SRCPND corresponding to those set to one in the data
	for (int which = 0; which < 3; which++)
		uart_update_tx_irq(which);
	check_pending_irq();
}

u32 s3c2410_device::irq_intsubmsk_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_irq_intsubmsk;
	LOGMASKED(LOG_IRQ_REGS, "%s: irq read: INTSUBMSK: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::irq_intsubmsk_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_irq_intsubmsk);
	LOGMASKED(LOG_IRQ_REGS, "%s: irq write: INTSUBMSK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	check_pending_irq();
}

/* PWM Timer */

void s3c24xx_device::pwm_reset()
{
	m_pwm_tcfg0 = 0;
	m_pwm_tcfg1 = 0;
	m_pwm_tcon = 0;
	m_pwm_tcntb0 = 0;
	m_pwm_tcmpb0 = 0;
	m_pwm_tcnto0 = 0;
	m_pwm_tcntb1 = 0;
	m_pwm_tcmpb1 = 0;
	m_pwm_tcnto1 = 0;
	m_pwm_tcntb2 = 0;
	m_pwm_tcmpb2 = 0;
	m_pwm_tcnto2 = 0;
	m_pwm_tcntb3 = 0;
	m_pwm_tcmpb3 = 0;
	m_pwm_tcnto3 = 0;
	m_pwm_tcntb4 = 0;
	m_pwm_tcnto4 = 0;

	std::fill(std::begin(m_pwm_cnt), std::end(m_pwm_cnt), 0);
	std::fill(std::begin(m_pwm_cmp), std::end(m_pwm_cmp), 0);
	std::fill(std::begin(m_pwm_freq), std::end(m_pwm_freq), 0);
	for (emu_timer *&tmr : m_pwm_timer)
		tmr->adjust(attotime::never);
}

u16 s3c24xx_device::pwm_calc_observation(unsigned ch) const
{
	double const timeleft = m_pwm_timer[ch]->remaining().as_double();
	LOG("timeleft %f freq %d cntb %d cmpb %d\n", timeleft, m_pwm_freq[ch], m_pwm_cnt[ch], m_pwm_cmp[ch]);
	double const x1 = 1 / (double(m_pwm_freq[ch]) / (m_pwm_cnt[ch] - m_pwm_cmp[ch] + 1));
	double const x2 = x1 / timeleft;
	LOG("x1 %f\n", x1);
	u32 cnto = m_pwm_cmp[ch] + ((m_pwm_cnt[ch]- m_pwm_cmp[ch]) / x2);
	LOG("cnto %u\n", cnto);
	return cnto;
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::pwm_timer_exp)
{
	static constexpr int CH_INT[] = { INT_TIMER0, INT_TIMER1, INT_TIMER2, INT_TIMER3, INT_TIMER4 };
	if (param != 4 && param != 0)
		LOGMASKED(LOG_PWM, "PWM %d timer callback\n", param);
	if (BIT(m_pwm_tcfg1, 20, 4) == (param + 1))
		dma_request_pwm();
	else
		request_irq(CH_INT[param]);
}

void s3c24xx_device::pwm_start(int timer)
{
	static constexpr int MUX_TABLE[] = { 2, 4, 8, 16 };
	static constexpr int PRESCALER_SHIFT[] = { 0, 0, 8, 8, 8 };
	static constexpr int MUX_SHIFT[] = { 0, 4, 8, 12, 16 };
	LOGMASKED(LOG_PWM, "PWM %d start\n", timer);

	const u32 pclk_ticks = get_pclk_ticks();
	const u32 prescaler = (m_pwm_tcfg0 >> PRESCALER_SHIFT[timer]) & 0xff;
	const u32 mux = (m_pwm_tcfg1 >> MUX_SHIFT[timer]) & 0x0f;
	const u64 full_clock = get_full_clock();

	u32 freq = 0;
	if (mux < 4)
	{
		freq = pclk_ticks * (prescaler + 1) * MUX_TABLE[mux];
	}
	else
	{
		// todo
		freq = pclk_ticks * (prescaler + 1) / 1;
	}

	u32 cnt = 0;
	u32 cmp = 0;
	u32 auto_reload = 0;
	switch (timer)
	{
	case 0:
		cnt = BIT(m_pwm_tcntb0, 0, 16);
		cmp = BIT(m_pwm_tcmpb0, 0, 16);
		auto_reload = BIT(m_pwm_tcon, 3);
		break;
	case 1:
		cnt = BIT(m_pwm_tcntb1, 0, 16);
		cmp = BIT(m_pwm_tcmpb1, 0, 16);
		auto_reload = BIT(m_pwm_tcon, 11);
		break;
	case 2:
		cnt = BIT(m_pwm_tcntb2, 0, 16);
		cmp = BIT(m_pwm_tcmpb2, 0, 16);
		auto_reload = BIT(m_pwm_tcon, 15);
		break;
	case 3:
		cnt = BIT(m_pwm_tcntb3, 0, 16);
		cmp = BIT(m_pwm_tcmpb3, 0, 16);
		auto_reload = BIT(m_pwm_tcon, 19);
		break;
	case 4:
		cnt = BIT(m_pwm_tcntb4, 0, 16);
		cmp = 0;
		auto_reload = BIT(m_pwm_tcon, 22);
		break;
	default:
		cnt = cmp = auto_reload = 0;
		break;
	}

	if (cnt >= 2)
	{
		freq *= cnt;
	}

	attotime period = attotime::from_ticks(freq, full_clock);
	LOGMASKED(LOG_PWM, "PWM %d - pclk=%d prescaler=%d div=%d freq=%f cnt=%d cmp=%d auto_reload=%d hz=%f\n", timer, pclk_ticks, prescaler, MUX_TABLE[mux], freq, cnt, cmp, auto_reload, period.as_hz());

	m_pwm_cnt[timer] = cnt;
	m_pwm_cmp[timer] = cmp;
	m_pwm_freq[timer] = freq;

	if (auto_reload)
		m_pwm_timer[timer]->adjust(period, timer, period);
	else
		m_pwm_timer[timer]->adjust(period, timer);
}

void s3c24xx_device::pwm_stop(int timer)
{
	LOGMASKED(LOG_PWM, "PWM %d stop\n", timer);
	m_pwm_timer[timer]->adjust(attotime::never);
}

void s3c24xx_device::pwm_recalc(int timer)
{
	static constexpr int TCON_SHIFT[] = { 0, 8, 12, 16, 20 };
	if (m_pwm_tcon & (1 << TCON_SHIFT[timer]))
		pwm_start(timer);
	else
		pwm_stop(timer);
}

u32 s3c24xx_device::pwm_tcfg0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcfg0;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCFG0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcfg0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcfg0);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCFG0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcfg1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcfg1;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCFG1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcfg1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcfg1);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcon;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_pwm_tcon;
	COMBINE_DATA(&m_pwm_tcon);
	const u32 changed = old ^ m_pwm_tcon;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (BIT(changed, 0))
		pwm_recalc(0);
	if (BIT(changed, 8))
		pwm_recalc(1);
	if (BIT(changed, 12))
		pwm_recalc(2);
	if (BIT(changed, 16))
		pwm_recalc(3);
	if (BIT(changed, 20))
		pwm_recalc(4);
}

u32 s3c24xx_device::pwm_tcntb0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcntb0;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTB0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcntb0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcntb0);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTB0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcmpb0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcmpb0;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCMPB0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcmpb0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcmpb0);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCMPB0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcnto0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_pwm_tcnto0 & ~0x0000ffff) | pwm_calc_observation(0);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTO0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcnto0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcnto0);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTO0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcntb1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcntb1;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTB1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcntb1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcntb1);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTB1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcmpb1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcmpb1;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCMPB1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcmpb1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcmpb1);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCMPB1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcnto1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_pwm_tcnto1 & ~0x0000ffff) | pwm_calc_observation(1);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTO1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcnto1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcnto1);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTO1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcntb2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcntb2;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTB2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcntb2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcntb2);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTB2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcmpb2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcmpb2;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCMPB2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcmpb2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcmpb2);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCMPB2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcnto2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_pwm_tcnto2 & ~0x0000ffff) | pwm_calc_observation(2);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTO2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcnto2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcnto2);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTO2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcntb3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcntb3;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTB3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcntb3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcntb3);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTB3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcmpb3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcmpb3;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCMPB3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcmpb3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcmpb3);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCMPB3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcnto3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_pwm_tcnto3 & ~0x0000ffff) | pwm_calc_observation(3);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTO3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcnto3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcnto3);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTO3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcntb4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_pwm_tcntb4;
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTB4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcntb4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcntb4);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTB4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::pwm_tcnto4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_pwm_tcnto4 & ~0x0000ffff) | pwm_calc_observation(4);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm read: TCNTO4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::pwm_tcnto4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pwm_tcnto4);
	LOGMASKED(LOG_PWM_REGS, "%s: pwm write: TCNTO4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* DMA */

u32 s3c24xx_device::dma_dcon_get_tsz(const u32 value)
{
	return BIT(value, 27);
}

u32 s3c2410_device::dma_dcon_get_tsz(const u32 value)
{
	return BIT(value, 28);
}

u32 s3c24xx_device::dma_dcon_get_hwsrcsel(const u32 value)
{
	return BIT(value, 24, 2);
}

u32 s3c2410_device::dma_dcon_get_hwsrcsel(const u32 value)
{
	return BIT(value, 24, 3);
}

u32 s3c24xx_device::dma_dcon_get_servmode(const u32 value)
{
	return BIT(value, 26);
}

u32 s3c2410_device::dma_dcon_get_servmode(const u32 value)
{
	return BIT(value, 27);
}

u32 s3c24xx_device::dma_dcon_get_int(const u32 value)
{
	return BIT(value, 28);
}

u32 s3c2410_device::dma_dcon_get_int(const u32 value)
{
	return BIT(value, 29);
}

u32 s3c24xx_device::dma_daddr_get_addr(const u32 value)
{
	return BIT(value, 0, 29);
}

u32 s3c2410_device::dma_daddr_get_addr(const u32 value)
{
	return BIT(value, 0, 31);
}

void s3c24xx_device::dma_dc_set_curr_addr(u32 &reg, const u32 value)
{
	reg = CLR_BITS(reg, 28, 0) | value;
}

void s3c2410_device::dma_dc_set_curr_addr(u32 &reg, const u32 value)
{
	reg = CLR_BITS(reg, 30, 0) | value;
}

u32 s3c24xx_device::dma_get_src_inc_flag(const int which)
{
	return BIT(m_dma_disrc[which], 29);
}

u32 s3c24xx_device::dma_get_dst_inc_flag(const int which)
{
	return BIT(m_dma_didst[which], 29);
}

u32 s3c2410_device::dma_get_src_inc_flag(const int which)
{
	return BIT(~m_dma_disrcc[which], 0);
}

u32 s3c2410_device::dma_get_dst_inc_flag(const int which)
{
	return BIT(~m_dma_didstc[which], 0);
}

template <int Which>
void s3c24xx_device::dma_reload()
{
	const u32 old = m_dma_dstat[Which];
	m_dma_dstat[Which] = CLR_BITS(m_dma_dstat[Which], 19, 0) | BIT(m_dma_dcon[Which], 0, 20);
	if (Which == 2)
		LOGMASKED(LOG_DMA, "dma_reload dstat now %08x (old %08x)\n", m_dma_dstat[Which], old);
	dma_dc_set_curr_addr(m_dma_dcsrc[Which], m_dma_disrc[Which]);
	dma_dc_set_curr_addr(m_dma_dcdst[Which], m_dma_didst[Which]);
}

template <int Which>
void s3c24xx_device::dma_trigger()
{
	static constexpr u32 CH_INT[] = { INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3 };
	if (Which != 2)
		LOGMASKED(LOG_DMA, "DMA %d trigger\n", Which);

	address_space &space = m_cpu->space(AS_PROGRAM);

	u32 curr_tc = BIT(m_dma_dstat[Which], 0, 20);
	const u32 dsz = BIT(m_dma_dcon[Which], 20, 2);
	u32 curr_src = dma_daddr_get_addr(m_dma_dcsrc[Which]);
	u32 curr_dst = dma_daddr_get_addr(m_dma_dcdst[Which]);
	int servmode = dma_dcon_get_servmode(m_dma_dcon[Which]);
	int tsz = dma_dcon_get_tsz(m_dma_dcon[Which]);
	const u32 inc_src = dma_get_src_inc_flag(Which) << dsz;
	const u32 inc_dst = dma_get_dst_inc_flag(Which) << dsz;
	if (Which != 2)
		LOGMASKED(LOG_DMA, "DMA %d - curr_src %08X curr_dst %08X curr_tc %d dsz %d\n", Which, curr_src, curr_dst, curr_tc, dsz);

	while (curr_tc > 0)
	{
		curr_tc--;
		for (int i = 0; i < 1 << (tsz << 1); i++)
		{
			switch (dsz)
			{
			case 0: space.write_byte(curr_dst, space.read_byte(curr_src)); break;
			case 1: space.write_word(curr_dst, space.read_word(curr_src)); break;
			case 2: space.write_dword(curr_dst, space.read_dword(curr_src)); break;
			}
			curr_src += inc_src;
			curr_dst += inc_dst;
		}
		if (servmode == 0) break;
	}
	dma_dc_set_curr_addr(m_dma_dcsrc[Which], curr_src);
	dma_dc_set_curr_addr(m_dma_dcdst[Which], curr_dst);
	m_dma_dstat[Which] = CLR_BITS(m_dma_dstat[Which], 19, 0) | curr_tc;
	if (curr_tc == 0)
	{
		if (!BIT(m_dma_dcon[Which], 22)) // Reload bit
		{
			dma_reload<Which>();
		}
		else
		{
			m_dma_dmasktrig[Which] &= ~(1 << 1); // clear on/off
		}
		if (dma_dcon_get_int(m_dma_dcon[Which]))
		{
			request_irq(CH_INT[Which]);
		}
	}
}

void s3c24xx_device::dma_request_iis()
{
	//LOGMASKED(LOG_DMA_REQS, "dma_request_iis\n");

	if (!BIT(m_dma_dmasktrig[2], 2) && BIT(m_dma_dmasktrig[2], 1) && BIT(m_dma_dcon[2], 23) && dma_dcon_get_hwsrcsel(m_dma_dcon[2]) == 0)
		dma_trigger<2>();
}

void s3c24xx_device::dma_request_pwm()
{
	LOGMASKED(LOG_DMA_REQS, "dma_request_pwm\n");
	if (BIT(m_dma_dmasktrig[0], 1) && BIT(m_dma_dcon[0], 23) && dma_dcon_get_hwsrcsel(m_dma_dcon[0]) == 3)
		dma_trigger<0>();
	if (BIT(m_dma_dmasktrig[2], 1) && BIT(m_dma_dcon[2], 23) && dma_dcon_get_hwsrcsel(m_dma_dcon[2]) == 3)
		dma_trigger<2>();
	if (BIT(m_dma_dmasktrig[3], 1) && BIT(m_dma_dcon[3], 23) && dma_dcon_get_hwsrcsel(m_dma_dcon[3]) == 3)
		dma_trigger<3>();
}

template <int Which>
void s3c24xx_device::dma_start()
{
	const u32 swhwsel = BIT(m_dma_dcon[Which], 23);

	if (VERBOSE & LOG_DMA)
	{
		LOGMASKED(LOG_DMA, "DMA %d start\n", Which);
		const u32 addr_src = dma_daddr_get_addr(m_dma_disrc[Which]);
		const u32 addr_dst = dma_daddr_get_addr(m_dma_didst[Which]);
		const u32 tc = BIT(m_dma_dcon[Which], 0, 20);
		const int _int = dma_dcon_get_int(m_dma_dcon[Which]);
		const int servmode = dma_dcon_get_servmode(m_dma_dcon[Which]);
		const int hwsrcsel = dma_dcon_get_hwsrcsel(m_dma_dcon[Which]);
		const u32 reload = BIT(m_dma_dcon[Which], 22);
		const u32 dsz = BIT(m_dma_dcon[Which], 20, 2);
		const u32 tsz = dma_dcon_get_tsz(m_dma_dcon[Which]);
		const int inc_src = dma_get_src_inc_flag(Which);
		const int inc_dst = dma_get_dst_inc_flag(Which);
		LOGMASKED(LOG_DMA, "DMA %d - addr_src %08X inc_src %d addr_dst %08X inc_dst %d int %d tsz %d servmode %d hwsrcsel %d swhwsel %d reload %d dsz %d tc %d\n", Which, addr_src, inc_src, addr_dst, inc_dst, _int, tsz, servmode, hwsrcsel, swhwsel, reload, dsz, tc);
		LOGMASKED(LOG_DMA, "DMA %d - copy %08X bytes from %08X (%s) to %08X (%s)\n", Which, (tc << dsz) << (tsz << 1), addr_src, inc_src ? "fix" : "inc", addr_dst, inc_dst ? "fix" : "inc");
	}

	dma_reload<Which>();
}

template <int Which>
void s3c24xx_device::dma_stop()
{
	LOGMASKED(LOG_DMA, "DMA %d stop\n", Which);
	// Transfers are atomic here, so STOP takes effect immediately.
	m_dma_dmasktrig[Which] &= ~2U;
	m_dma_dstat[Which] = CLR_BITS(m_dma_dstat[Which], 19, 0);
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::dma_timer_exp)
{
	int ch = param;
	LOGMASKED(LOG_DMA_TIMERS, "DMA %d timer callback\n", ch);
}

template <int Which>
u32 s3c24xx_device::dma_disrc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_disrc[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DISRC: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::dma_disrc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_disrc[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DISRC = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::dma_didst_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_didst[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DIDST: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::dma_didst_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_didst[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DIDST = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::dma_dcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_dcon[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DCON: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::dma_dcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_dcon[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DCON = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
#if 0 // is this code necessary ???
	if (BIT(data, 22)) // reload
		m_dma_dmasktrig[Which] &= ~(1 << 1); // clear on/off
#endif
}

template <int Which>
u32 s3c24xx_device::dma_dstat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_dstat[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DSTAT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::dma_dstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_dstat[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DSTAT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::dma_dcsrc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_dcsrc[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DCSRC: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::dma_dcsrc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_dcsrc[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DCSRC = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::dma_dcdst_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_dcdst[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DCDST: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::dma_dcdst_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_dcdst[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DCDST = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::dma_dmasktrig_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_dmasktrig[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DMASKTRIG: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::dma_dmasktrig_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_dma_dmasktrig[Which];
	COMBINE_DATA(&m_dma_dmasktrig[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DMASKTRIG = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	if (BIT(m_dma_dmasktrig[Which], 2))
	{
		dma_stop<Which>();
	}
	else if (BIT(m_dma_dmasktrig[Which], 1))
	{
		if (!BIT(old, 1))
			dma_start<Which>();
		if (BIT(m_dma_dmasktrig[Which], 0) && !BIT(m_dma_dcon[Which], 23))
		{
			m_dma_dmasktrig[Which] &= ~1U;
			dma_trigger<Which>();
		}
	}
}

template <int Which>
u32 s3c2410_device::dma_disrcc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_disrcc[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DISRCC: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::dma_disrcc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_disrcc[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DISRCC = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c2410_device::dma_didstc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_didstc[Which];
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d read: DIDSTC: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::dma_didstc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_didstc[Which]);
	LOGMASKED(LOG_DMA_REGS, "%s: dma %d write: DIDSTC = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

/* I/O Port */

void s3c24xx_device::gpio_reset()
{
	m_gpio_gpacon = 0;
	m_gpio_gpbcon = 0;
	m_gpio_gpbup = 0;
	m_gpio_gpccon = 0;
	m_gpio_gpcup = 0;
	m_gpio_gpdcon = 0;
	m_gpio_gpdup = 0xf000;
	m_gpio_gpecon = 0;
	m_gpio_gpeup = 0;
	m_gpio_gpfcon = 0;
	m_gpio_gpfup = 0;
	m_gpio_gpgcon = 0;
	m_gpio_gpgup = 0;
	m_gpio_gphcon = 0;
	m_gpio_gphup = 0;
	m_gpio_misccr = 0;
	m_gpio_opencr = 0;
	std::fill(std::begin(m_gpio_extint), std::end(m_gpio_extint), 0);
}

void s3c2400_device::gpio_reset()
{
	s3c24xx_device::gpio_reset();
	m_gpio_gpacon = 0x0003ffff;
	m_gpio_gpbcon = 0xaaaaaaaa;
	m_gpio_gpdup = 0x0620;
	m_gpio_gpeup = 0x0003;
}

void s3c2410_device::gpio_reset()
{
	s3c24xx_device::gpio_reset();
	m_gpio_dclkcon = 0;
	std::fill(std::begin(m_gpio_gstatus), std::end(m_gpio_gstatus), 0);
	m_gpio_gpacon = 0x007fffff;
	m_gpio_gpgup = 0xf800;
	m_gpio_misccr = 0x00010330;
	m_gpio_eintmask = 0x00fffff0;
	m_gpio_gstatus[1] = 0x32410000;
	m_gpio_gstatus[2] = 1 << 0; // Boot is caused by power on reset
	m_gpio_extint_lines = (m_port_r_cb(GPIO_PORT_F) & 0xff) | ((m_port_r_cb(GPIO_PORT_G) & 0xffff) << 8);
	m_gpio_eintpend = 0;
	std::fill(std::begin(m_gpio_eintflt), std::end(m_gpio_eintflt), 0);
}

void s3c2440_device::gpio_reset()
{
	s3c2410_device::gpio_reset();
	m_gpio_gpacon = 0x00ffffff;
	m_gpio_gpgup = 0xfc00;
	m_gpio_mslcon = 0;
	m_gpio_gpjcon = 0;
	m_gpio_gpjup = 0;
	m_gpio_misccr = 0x00010020;
	m_gpio_eintmask = 0x000fffff;
	m_gpio_gstatus[1] = 0x32440001;
	m_gpio_gstatus[2] = 1 << 0; // Boot is caused by power on reset
}

u32 s3c2400_device::gpio_get_data_mask(int port)
{
	static constexpr u32 GPIO_MASKS[9] = {
		0x0003ffff,
		0x0000ffff,
		0x0000ffff,
		0x000007ff,
		0x00000fff,
		0x0000007f,
		0x000003ff,
		~0u,
		~0u
	};
	return GPIO_MASKS[port];
}

u32 s3c2410_device::gpio_get_data_mask(int port)
{
	static constexpr u32 GPIO_MASKS[9] = {
		0x007fffff,
		0x000007ff,
		0x0000ffff,
		0x0000ffff,
		0x0000ffff,
		0x000000ff,
		0x0000ffff,
		0x000007ff,
		~0u
	};
	return GPIO_MASKS[port];
}

u32 s3c2440_device::gpio_get_data_mask(int port)
{
	static constexpr u32 GPIO_MASKS[9] = {
		0x01ffffff,
		0x000007ff,
		0x0000ffff,
		0x0000ffff,
		0x0000ffff,
		0x000000ff,
		0x0000ffff,
		0x000007ff,
		0x0000ffff
	};
	return GPIO_MASKS[port];
}

u32 s3c24xx_device::gpio_get_mask(u32 con, int val)
{
	u32 mask = 0;
	for (int i = 0; i < 16; i++)
	{
		if ((con & 3) == val)
			mask |= 1 << i;
		con >>= 2;
	}
	return mask;
}

u32 s3c24xx_device::gpio_gpacon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpacon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPACON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpacon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpacon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPACON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpadat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_A, 0xffffffff) & gpio_get_data_mask(GPIO_PORT_A);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPADAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpadat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPADAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_A, data & gpio_get_data_mask(GPIO_PORT_A), m_gpio_gpacon ^ 0xffffffff);
}

u32 s3c24xx_device::gpio_gpbcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpbcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPBCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpbcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpbcon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPBCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpbdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_B, gpio_get_mask(m_gpio_gpbcon, 0) & gpio_get_data_mask(GPIO_PORT_B)) & gpio_get_data_mask(GPIO_PORT_B);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPBDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpbdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPBDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_B, data & gpio_get_data_mask(GPIO_PORT_B), gpio_get_mask(m_gpio_gpbcon, 1) & gpio_get_data_mask(GPIO_PORT_B));
}

u32 s3c24xx_device::gpio_gpbup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpbup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPBUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpbup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpbup);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPBUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpccon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpccon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPCCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpccon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpccon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPCCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpcdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_C, gpio_get_mask(m_gpio_gpccon, 0) & gpio_get_data_mask(GPIO_PORT_C)) & gpio_get_data_mask(GPIO_PORT_C);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPCDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpcdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPCDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_C, data & gpio_get_data_mask(GPIO_PORT_C), gpio_get_mask(m_gpio_gpccon, 1) & gpio_get_data_mask(GPIO_PORT_C));
}

u32 s3c24xx_device::gpio_gpcup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpcup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPCUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpcup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpcup);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPCUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpdcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpdcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPDCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpdcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpdcon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPDCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpddat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_D, gpio_get_mask(m_gpio_gpdcon, 0) & gpio_get_data_mask(GPIO_PORT_D)) & gpio_get_data_mask(GPIO_PORT_D);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPDDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpddat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPDDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_D, data & gpio_get_data_mask(GPIO_PORT_D), gpio_get_mask(m_gpio_gpdcon, 1) & gpio_get_data_mask(GPIO_PORT_D));
}

u32 s3c24xx_device::gpio_gpdup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpdup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPDUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpdup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpdup);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPDUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpecon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpecon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPECON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpecon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpecon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPECON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpedat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_E, gpio_get_mask(m_gpio_gpecon, 0) & gpio_get_data_mask(GPIO_PORT_E)) & gpio_get_data_mask(GPIO_PORT_E);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPEDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpedat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPEDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_E, data & gpio_get_data_mask(GPIO_PORT_E), gpio_get_mask(m_gpio_gpecon, 1) & gpio_get_data_mask(GPIO_PORT_E));
}

u32 s3c24xx_device::gpio_gpeup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpeup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPEUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpeup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpeup);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPEUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpfcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpfcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPFCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpfcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpfcon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPFCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpfdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_F, gpio_get_mask(m_gpio_gpfcon, 0) & gpio_get_data_mask(GPIO_PORT_F)) & gpio_get_data_mask(GPIO_PORT_F);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPFDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpfdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPFDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_F, data & gpio_get_data_mask(GPIO_PORT_F), gpio_get_mask(m_gpio_gpfcon, 1) & gpio_get_data_mask(GPIO_PORT_F));
}

u32 s3c24xx_device::gpio_gpfup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpfup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPFUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpfup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpfup);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPFUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpgcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpgcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPGCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpgcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpgcon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPGCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_gpgdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_G, gpio_get_mask(m_gpio_gpgcon, 0) & gpio_get_data_mask(GPIO_PORT_G)) & gpio_get_data_mask(GPIO_PORT_G);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPGDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpgdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPGDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_G, data & gpio_get_data_mask(GPIO_PORT_G), gpio_get_mask(m_gpio_gpgcon, 1) & gpio_get_data_mask(GPIO_PORT_G));
}

u32 s3c24xx_device::gpio_gpgup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpgup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPGUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_gpgup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpgup);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPGUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_misccr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_misccr;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: MISCCR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_misccr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_misccr);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: MISCCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::gpio_opencr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_opencr;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: OPENCR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::gpio_opencr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_opencr);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: OPENCR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::gpio_extint_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_extint[Which];
	LOGMASKED(LOG_GPIO, "%s: GPIO read: EXTINT%d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::gpio_extint_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_extint[Which]);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: EXTINT%d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

u32 s3c2410_device::gpio_gphcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gphcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPHCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_gphcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gphcon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPHCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::gpio_gphdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_H, gpio_get_mask(m_gpio_gphcon, 0) & gpio_get_data_mask(GPIO_PORT_H)) & gpio_get_data_mask(GPIO_PORT_H);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPHDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_gphdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPHDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_H, data & gpio_get_data_mask(GPIO_PORT_H), gpio_get_mask(m_gpio_gphcon, 1) & gpio_get_data_mask(GPIO_PORT_H));
}

u32 s3c2410_device::gpio_gphup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gphup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPHUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_gphup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gphup);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GPHUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::gpio_dclkcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_dclkcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: DCLKCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_dclkcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_dclkcon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: DCLKCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

template <int Which>
u32 s3c2410_device::gpio_eintflt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_eintflt[Which];
	LOGMASKED(LOG_GPIO, "%s: GPIO read: EINTFLT%d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::gpio_eintflt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_eintflt[Which]);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: EINTFLT%D = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

u32 s3c2410_device::gpio_eintmask_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_eintmask;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: EINTMASK: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_eintmask_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_eintmask);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: EINTMASK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	check_pending_eint();
}

u32 s3c2410_device::gpio_eintpend_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_eintpend;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: EINTPEND: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_eintpend_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_gpio_eintpend &= ~(data & mem_mask);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: EINTPEND = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	check_pending_eint();
}

u32 s3c2410_device::gpio_gstatus0_r(offs_t offset, u32 mem_mask)
{
	// nWAIT and nBATT_FLT are inactive; NCON and RnB reflect their input pins.
	const u32 data = 0x09 | (m_ncon_r_cb() ? 0x04 : 0) | (BIT(m_nand_nfstat, 0) ? 0x02 : 0);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GSTATUS0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

u32 s3c2410_device::gpio_gstatus1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gstatus[1];
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GSTATUS1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

u32 s3c2410_device::gpio_gstatus2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gstatus[2];
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GSTATUS2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_gstatus2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gstatus[2]);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GSTATUS2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_gpio_gstatus[2] = (m_gpio_gstatus[2] & ~data) & 7; // "The setting is cleared by writing '1' to this bit"
}

u32 s3c2410_device::gpio_gstatus3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gstatus[3];
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GSTATUS3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_gstatus3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gstatus[3]);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GSTATUS3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::gpio_gstatus4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gstatus[4];
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GSTATUS4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::gpio_gstatus4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gstatus[4]);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: GSTATUS4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::gpio_mslcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_mslcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: MSLCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::gpio_mslcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_mslcon);
	LOGMASKED(LOG_GPIO, "%s: GPIO write: MSLCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::gpio_gpjcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpjcon;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPJCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::gpio_gpjcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpjcon);
	//LOGMASKED(LOG_GPIO, "%s: GPIO write: GPJCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::gpio_gpjdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_port_r_cb(GPIO_PORT_J, gpio_get_mask(m_gpio_gpjcon, 0) & gpio_get_data_mask(GPIO_PORT_J)) & gpio_get_data_mask(GPIO_PORT_J);
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPJDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::gpio_gpjdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	//LOGMASKED(LOG_GPIO, "%s: GPIO write: GPJDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_port_w_cb(GPIO_PORT_J, data & gpio_get_data_mask(GPIO_PORT_J), gpio_get_mask(m_gpio_gpjcon, 1) & gpio_get_data_mask(GPIO_PORT_J));
}

u32 s3c2440_device::gpio_gpjup_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_gpjup;
	LOGMASKED(LOG_GPIO, "%s: GPIO read: GPJUP: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::gpio_gpjup_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_gpio_gpjup);
	//LOGMASKED(LOG_GPIO, "%s: GPIO write: GPJUP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* Memory Controller */

void s3c24xx_device::memcon_reset()
{
	m_memcon_bwscon = 0;
	m_memcon_bankcon[0] = 0x00000700;
	m_memcon_bankcon[1] = 0x00000700;
	m_memcon_bankcon[2] = 0x00000700;
	m_memcon_bankcon[3] = 0x00000700;
	m_memcon_bankcon[4] = 0x00000700;
	m_memcon_bankcon[5] = 0x00000700;
	m_memcon_bankcon[6] = 0x00018008;
	m_memcon_bankcon[7] = 0x00018008;
	m_memcon_refresh = 0x00ac0000;
	m_memcon_banksize = 0;
	m_memcon_mrsrb6 = 0;
	m_memcon_mrsrb7 = 0;
}

u32 s3c24xx_device::memcon_bwscon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_memcon_bwscon;
	LOGMASKED(LOG_MEMCON, "%s: memcon read: BWSCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::memcon_bwscon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memcon_bwscon);
	LOGMASKED(LOG_MEMCON, "%s: memcon write: BWSCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::memcon_bankcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_memcon_bankcon[Which];
	LOGMASKED(LOG_MEMCON, "%s: memcon read: BANKCON%d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::memcon_bankcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memcon_bankcon[Which]);
	LOGMASKED(LOG_MEMCON, "%s: memcon write: BANKCON%d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

u32 s3c24xx_device::memcon_refresh_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_memcon_refresh;
	LOGMASKED(LOG_MEMCON, "%s: memcon read: REFRESH: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::memcon_refresh_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memcon_refresh);
	LOGMASKED(LOG_MEMCON, "%s: memcon write: REFRESH = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::memcon_banksize_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_memcon_banksize;
	LOGMASKED(LOG_MEMCON, "%s: memcon read: BANKSIZE: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::memcon_banksize_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memcon_banksize);
	LOGMASKED(LOG_MEMCON, "%s: memcon write: BANKSIZE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::memcon_mrsrb6_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_memcon_mrsrb6;
	LOGMASKED(LOG_MEMCON, "%s: memcon read: MRSRB6: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::memcon_mrsrb6_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memcon_mrsrb6);
	LOGMASKED(LOG_MEMCON, "%s: memcon write: MRSRB6 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::memcon_mrsrb7_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_memcon_mrsrb7;
	LOGMASKED(LOG_MEMCON, "%s: memcon read: MRSRB7: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::memcon_mrsrb7_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memcon_mrsrb7);
	LOGMASKED(LOG_MEMCON, "%s: memcon write: MRSRB7 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* USB Host Controller */

void s3c24xx_device::usbhost_reset()
{
	m_usbhost_rev = 0;
	m_usbhost_ctrl = 0;
	m_usbhost_common_stat = 0;
	m_usbhost_int_stat = 0;
	m_usbhost_int_en = 0;
	m_usbhost_int_dis = 0;
	m_usbhost_hcca = 0;
	m_usbhost_period_curr = 0;
	m_usbhost_ctrl_head = 0;
	m_usbhost_ctrl_curr = 0;
	m_usbhost_bulk_head = 0;
	m_usbhost_bulk_curr = 0;
	m_usbhost_done_head = 0;
	m_usbhost_rm_interval = 0;
	m_usbhost_fm_remaining = 0;
	m_usbhost_fm_number = 0;
	m_usbhost_periodic_start = 0;
	m_usbhost_ls_thresh = 0;
	std::fill(std::begin(m_usbhost_rh_desc), std::end(m_usbhost_rh_desc), 0);
	m_usbhost_rh_status = 0;
	std::fill(std::begin(m_usbhost_rh_port_status), std::end(m_usbhost_rh_port_status), 0);
}

u32 s3c24xx_device::usbhost_rev_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_rev;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcRevision: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

u32 s3c24xx_device::usbhost_ctrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_ctrl;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcControl: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_ctrl);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcControl = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_common_stat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_common_stat & ~(1 << 0); // [bit 0] HostControllerReset
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcCommonStatus: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_common_stat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_common_stat);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcCommonStatus = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_int_stat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_int_stat;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcInterruptstatus: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_int_stat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_int_stat);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcInterruptStatus = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_int_en_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_int_en;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcInterruptEnable: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_int_en_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_int_en);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcInterruptEnable = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_int_dis_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_int_dis;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcInterruptDisable: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_int_dis_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_int_dis);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcInterruptDisable = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_hcca_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_hcca;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcHCCA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_hcca_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_hcca);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcHCCA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_period_curr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_period_curr;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcPeriodCurrentED: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_period_curr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_period_curr);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcPeriodCurrentED = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_ctrl_head_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_ctrl_head;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcControlHeadED: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_ctrl_head_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_ctrl_head);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcControlHeadED = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_ctrl_curr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_ctrl_curr;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcControlCurrentED: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_ctrl_curr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_ctrl_curr);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcControlCurrentED = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_bulk_head_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_bulk_head;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcBulkHeadED: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_bulk_head_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_bulk_head);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcBulkHeadED = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_bulk_curr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_bulk_curr;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcBulkCurrentED: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_bulk_curr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_bulk_curr);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcBulkCurrentED = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_done_head_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_done_head;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcDoneHead: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_done_head_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_done_head);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcDoneHead = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_rm_interval_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_rm_interval;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcRmInterval: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_rm_interval_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_rm_interval);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcRmInterval = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_fm_remaining_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_fm_remaining;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcFmRemaining: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_fm_remaining_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_fm_remaining);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcFmRemaining = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_fm_number_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_fm_number;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcFmNumber: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_fm_number_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_fm_number);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcFmNumber = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_periodic_start_r(offs_t offset, u32 mem_mask)
{
	// "After a hardware reset, this field is cleared. This is then set by"
	// "HCD during the HC initialization. The value is calculated"
	// "roughly as 10% off from HcFmInterval.. A typical value will be 3E67h."
	const u32 data = (m_usbhost_periodic_start & ~0x00003fff) | 0x3e67;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcPeriodicStart: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_periodic_start_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_periodic_start);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcPeriodicStart = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_ls_thresh_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_ls_thresh;
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcLsThreshold: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_ls_thresh_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_ls_thresh);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcLsThreshold = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_rh_desc_a_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_usbhost_rh_desc[0] & ~0xff) | 2; // number of ports
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcRhDescriptorA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_rh_desc_a_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_rh_desc[0]);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcRhDescriptorA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_rh_desc_b_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_rh_desc[1];
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcRhDescriptorB: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_rh_desc_b_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_rh_desc[1]);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcRhDescriptorB = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_rh_status_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_rh_status & ~(1 << 16); // "The Root Hub does not support the local power status feature; thus, this bit is always read as 0."
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcRhStatus: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_rh_status_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_rh_status);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcRhStatus = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_rh_port_stat_1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_rh_port_status[0];
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcRhPortStatus1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_rh_port_stat_1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_rh_port_status[0]);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcRhPortStatus1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbhost_rh_port_stat_2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbhost_rh_port_status[1];
	LOGMASKED(LOG_USBHOST, "%s: USB host read: HcRhPortStatus2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbhost_rh_port_stat_2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbhost_rh_port_status[1]);
	LOGMASKED(LOG_USBHOST, "%s: USB host write: HcRhPortStatus2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* UART */

void s3c24xx_device::uart_reset()
{
	std::fill(std::begin(m_uart_ulcon), std::end(m_uart_ulcon), 0);
	std::fill(std::begin(m_uart_ucon), std::end(m_uart_ucon), 0);
	std::fill(std::begin(m_uart_ufcon), std::end(m_uart_ufcon), 0);
	std::fill(std::begin(m_uart_umcon), std::end(m_uart_umcon), 0);
	std::fill(std::begin(m_uart_uerstat), std::end(m_uart_uerstat), 0);
	std::fill(std::begin(m_uart_ufstat), std::end(m_uart_ufstat), 0);
	std::fill(std::begin(m_uart_umstat), std::end(m_uart_umstat), 0);
	std::fill(std::begin(m_uart_utxh), std::end(m_uart_utxh), 0);
	std::fill(std::begin(m_uart_urxh), std::end(m_uart_urxh), 0);
	std::fill(std::begin(m_uart_ubrdiv), std::end(m_uart_ubrdiv), 0);
	std::fill(std::begin(m_uart_utrstat), std::end(m_uart_utrstat), 6);
	for (int which = 0; which < 3; which++)
	{
		std::fill(std::begin(m_uart_tx_fifo[which]), std::end(m_uart_tx_fifo[which]), 0);
		m_uart_tx_head[which] = 0;
		m_uart_tx_count[which] = 0;
		m_uart_tx_shift[which] = 0;
		m_uart_tx_busy[which] = false;
		m_uart_tx_irq[which] = false;
		m_uart_tx_timer[which]->adjust(attotime::never);
	}
}

template <int Which>
u32 s3c24xx_device::uart_ulcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_ulcon[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: ULCON: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_ulcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_ulcon[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: ULCON = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_ucon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_ucon[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: UCON: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_ucon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_ucon[Which]);
	uart_tx_start(Which);
	uart_update_tx_irq(Which);
	LOGMASKED(LOG_UART, "%s: UART %d write: UCON = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_ufcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_ufcon[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: UFCON: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_ufcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_ufcon[Which]);
	if (BIT(m_uart_ufcon[Which], 2))
	{
		m_uart_tx_head[Which] = 0;
		m_uart_tx_count[Which] = 0;
	}
	uart_update_tx_irq(Which);
	m_uart_ufcon[Which] &= ~((1 << 2) | (1 << 1)); // bits 1 and 2 are auto-cleared after resetting FIFO
	LOGMASKED(LOG_UART, "%s: UART %d write: UFCON = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_umcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_umcon[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: UMCON: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_umcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_umcon[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: UMCON = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_utrstat_r(offs_t offset, u32 mem_mask)
{
	u32 data = m_uart_utrstat[Which] & ~6U;
	if (!m_uart_tx_count[Which])
		data |= m_uart_tx_busy[Which] ? 2 : 6;
	LOGMASKED(LOG_UART, "%s: UART %d read: UTRSTAT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_utrstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_utrstat[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: UTRSTAT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_uerstat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_uerstat[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: UERSTAT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_uerstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_uerstat[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: UERSTAT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_ufstat_r(offs_t offset, u32 mem_mask)
{
	const u32 count = m_uart_tx_count[Which];
	const u32 depth = uart_fifo_depth();
	const u32 tx = (count == depth) ? ((depth == 64) ? 0x4000 : 0x0200) : (count << ((depth == 64) ? 8 : 4));
	const u32 data = (m_uart_ufstat[Which] & ((depth == 64) ? ~0x7f00U : ~0x02f0U)) | tx;
	LOGMASKED(LOG_UART, "%s: UART %d read: UFSTAT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_ufstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_ufstat[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: UFSTAT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_umstat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_umstat[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: UMSTAT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_umstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_umstat[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: UMSTAT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_utxh_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_utxh[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: UTXH: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_utxh_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_utxh[Which]);
	const u8 txdata = m_uart_utxh[Which] & 0xff;
	if ((mem_mask & 0xff) && BIT(m_uart_ucon[Which], 2, 2))
	{
		const u32 capacity = BIT(m_uart_ufcon[Which], 0) ? uart_fifo_depth() : 1;
		if (m_uart_tx_count[Which] < capacity)
		{
			m_uart_tx_fifo[Which][(m_uart_tx_head[Which] + m_uart_tx_count[Which]) & 63] = txdata;
			m_uart_tx_count[Which]++;
			uart_update_tx_irq(Which);
			uart_tx_start(Which);
		}
	}
#ifdef UART_PRINTF
	if (txdata == 0x0d)
		printf("\n");
	else if (txdata == 0x08 || txdata == 0x09 || (txdata >= 0x20 && txdata < 0x7f))
		printf("%c", txdata);
#endif
	LOGMASKED(LOG_UART, "%s: UART %d write: UTXH = %08x & %08x (%c)\n", machine().describe_context(), Which, data, mem_mask, ((txdata >= 32 && txdata < 128) ? (char)txdata : '?'));
}

template <int Which>
u32 s3c24xx_device::uart_urxh_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_urxh[Which];
	const u8 rxdata = data & 0xff;
	if (!machine().side_effects_disabled())
		m_uart_utrstat[Which] &= ~1; // [bit 0] Receive buffer data ready
	LOGMASKED(LOG_UART, "%s: UART %d read: URXH: %08x (%c) & %08x\n", machine().describe_context(), Which, data, ((rxdata >= 0x20 && rxdata < 0x7f) ? (char)rxdata : ' '), mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_urxh_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_urxh[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: URXH = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::uart_ubrdiv_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_ubrdiv[Which];
	LOGMASKED(LOG_UART, "%s: UART %d read: UBRDIV: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::uart_ubrdiv_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_uart_ubrdiv[Which]);
	LOGMASKED(LOG_UART, "%s: UART %d write: UBRDIV = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

u32 s3c24xx_device::uart_fifo_depth() const
{
	return 16;
}

u32 s3c2440_device::uart_fifo_depth() const
{
	return 64;
}

void s3c24xx_device::uart_update_tx_irq(int which)
{
	const u32 trigger = BIT(m_uart_ufcon[which], 0) ? BIT(m_uart_ufcon[which], 6, 2) * (uart_fifo_depth() / 4) : 0;
	const bool state = (BIT(m_uart_ucon[which], 2, 2) == 1) && (m_uart_tx_count[which] <= trigger);
	const bool old = m_uart_tx_irq[which];
	m_uart_tx_irq[which] = state;
	if (state && (!old || BIT(m_uart_ucon[which], 9)))
		uart_request_tx_irq(which);
}

void s3c24xx_device::uart_tx_start(int which)
{
	if (m_uart_tx_busy[which] || !m_uart_tx_count[which] || !BIT(m_uart_ucon[which], 2, 2))
		return;

	m_uart_tx_shift[which] = m_uart_tx_fifo[which][m_uart_tx_head[which]];
	m_uart_tx_head[which] = (m_uart_tx_head[which] + 1) & 63;
	m_uart_tx_count[which]--;
	m_uart_tx_busy[which] = true;
	const u32 bits = 1 + 5 + BIT(m_uart_ulcon[which], 0, 2) + BIT(m_uart_ulcon[which], 5) + 1 + BIT(m_uart_ulcon[which], 2);
	// TODO: external UART clock selection and automatic flow control.
	m_uart_tx_timer[which]->adjust(get_pclk() * (16 * (BIT(m_uart_ubrdiv[which], 0, 16) + 1) * bits), which);
	uart_update_tx_irq(which);
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::uart_tx_complete)
{
	m_uart_tx_busy[param] = false;
	if (BIT(m_uart_ucon[param], 5))
	{
		switch (param)
		{
		case 0: uart_fifo_w<0>(m_uart_tx_shift[param]); break;
		case 1: uart_fifo_w<1>(m_uart_tx_shift[param]); break;
		case 2: uart_fifo_w<2>(m_uart_tx_shift[param]); break;
		}
	}
	uart_tx_start(param);
	uart_update_tx_irq(param);
}

void s3c24xx_device::uart_request_tx_irq(int which)
{
	static constexpr u32 INT_BITS[2] = { INT_UTXD0, INT_UTXD1 };
	if (which < 2)
		request_irq(INT_BITS[which]);
}

void s3c2410_device::uart_request_tx_irq(int which)
{
	static constexpr u32 INT_BITS[3] = { SUBINT_TXD0, SUBINT_TXD1, SUBINT_TXD2 };
	request_subirq(INT_BITS[which]);
}

void s3c24xx_device::uart_request_irq(int which)
{
	static constexpr u32 INT_BITS[2] = { INT_URXD0, INT_URXD1 };
	request_irq(INT_BITS[which]);
}

void s3c2410_device::uart_request_irq(int which)
{
	static constexpr u32 INT_BITS[3] = { SUBINT_RXD0, SUBINT_RXD1, SUBINT_RXD2 };
	request_subirq(INT_BITS[which]);
}

template <int Which>
void s3c24xx_device::uart_fifo_w(u8 data)
{
	m_uart_urxh[Which] = data;
	m_uart_utrstat[Which] |= 1; // [bit 0] Receive buffer data ready

	bool request_irq = true;
	if (BIT(m_uart_ufcon[Which], 0))
	{
		request_irq = false;
	}

	if (request_irq)
	{
		uart_request_irq(Which);
	}
}

/* USB Device */

void s3c24xx_device::usb_device_reset()
{
	m_usbdev_func_addr = 0;
	m_usbdev_pwr = 0;
	m_usbdev_int = 0;
	m_usbdev_int_mask = 0;
	m_usbdev_frame_num = 0;
	m_usbdev_resume_con = 0;
	std::fill(std::begin(m_usbdev_ep_csr), std::end(m_usbdev_ep_csr), 0);
	std::fill(std::begin(m_usbdev_ep_maxp), std::end(m_usbdev_ep_maxp), 0);
	std::fill(std::begin(m_usbdev_ep_out_cnt), std::end(m_usbdev_ep_out_cnt), 0);
	std::fill(std::begin(m_usbdev_ep_fifo), std::end(m_usbdev_ep_fifo), 0);
	m_usbdev_dma_con = 0;
	m_usbdev_dma_unit = 0;
	m_usbdev_dma_fifo = 0;
	m_usbdev_dma_tx = 0;
	m_usbdev_test_mode = 0;
	m_usbdev_in_con = 0;
}

void s3c2400_device::usb_device_reset()
{
	s3c24xx_device::usb_device_reset();
	m_usbdev_int_mask = 0x033f;
	m_usbdev_resume_con = 0x000a;
	m_usbdev_ep_maxp[0] = 0x0001;
	m_usbdev_ep_maxp[1] = 0x0001;
	m_usbdev_ep_maxp[2] = 0x0001;
	m_usbdev_ep_maxp[3] = 0x0001;
	m_usbdev_ep_maxp[4] = 0x0001;
	m_usbdev_in_con = 0x00ff;
}

void s3c2410_device::usb_device_reset()
{
	s3c24xx_device::usb_device_reset();
	m_usbdev_ep_int = 0;
	m_usbdev_usb_int = 0;
	m_usbdev_ep_int_en = 0xff;
	m_usbdev_usb_int_en = 0x04;
	m_usbdev_frame_num1 = 0;
	m_usbdev_frame_num2 = 0;
	m_usbdev_index = 0;
	m_usbdev_maxp = 0x01;
	m_usbdev_in_csr2 = 0x20;
	m_usbdev_out_csr1 = 0;
	m_usbdev_out_csr2 = 0;
	m_usbdev_out_fifo_cnt1 = 0;
	m_usbdev_out_fifo_cnt2 = 0;
	std::fill(std::begin(m_usbdev_ep_dma_con), std::end(m_usbdev_ep_dma_con), 0);
	std::fill(std::begin(m_usbdev_ep_dma_unit), std::end(m_usbdev_ep_dma_unit), 0);
	std::fill(std::begin(m_usbdev_ep_dma_fifo), std::end(m_usbdev_ep_dma_fifo), 0);
	std::fill(std::begin(m_usbdev_ep_dma_ttc_l), std::end(m_usbdev_ep_dma_ttc_l), 0);
	std::fill(std::begin(m_usbdev_ep_dma_ttc_m), std::end(m_usbdev_ep_dma_ttc_m), 0);
	std::fill(std::begin(m_usbdev_ep_dma_ttc_h), std::end(m_usbdev_ep_dma_ttc_h), 0);
}

u32 s3c24xx_device::usbdev_func_addr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_func_addr;
	LOGMASKED(LOG_UART, "%s: USB device read: FUNC_ADDR_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_func_addr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_func_addr);
	LOGMASKED(LOG_UART, "%s: USB device write: FUNC_ADDR_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_pwr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_pwr;
	LOGMASKED(LOG_UART, "%s: USB device read: PWR_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_pwr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_pwr);
	LOGMASKED(LOG_UART, "%s: USB device write: PWR_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_int_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_int;
	LOGMASKED(LOG_UART, "%s: USB device read: INT_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_int_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_int);
	LOGMASKED(LOG_UART, "%s: USB device write: INT_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_int_mask_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_int_mask;
	LOGMASKED(LOG_UART, "%s: USB device read: INT_MASK_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_int_mask_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_int_mask);
	LOGMASKED(LOG_UART, "%s: USB device write: INT_MASK_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_frame_num_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_frame_num;
	LOGMASKED(LOG_UART, "%s: USB device read: FRAME_NUM_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_frame_num_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_frame_num);
	LOGMASKED(LOG_UART, "%s: USB device write: FRAME_NUM_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_resume_con_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_resume_con;
	LOGMASKED(LOG_UART, "%s: USB device read: RESUME_CON_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_resume_con_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_resume_con);
	LOGMASKED(LOG_UART, "%s: USB device write: RESUME_CON_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::usbdev_ep_csr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_csr[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_CSR: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::usbdev_ep_csr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_csr[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_CSR = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::usbdev_ep_maxp_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_maxp[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_MAXP: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::usbdev_ep_maxp_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_maxp[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_MAXP = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::usbdev_ep_out_cnt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_out_cnt[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_OUT_CNT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::usbdev_ep_out_cnt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_out_cnt[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_OUT_CNT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

u32 s3c24xx_device::usbdev_ep0_fifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_fifo[0];
	LOGMASKED(LOG_UART, "%s: USB device read: EP0_FIFO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_ep0_fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_fifo[0]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP0_FIFO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_ep1_fifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_fifo[1];
	LOGMASKED(LOG_UART, "%s: USB device read: EP1_FIFO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_ep1_fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_fifo[1]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP1_FIFO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_ep2_fifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_fifo[2];
	LOGMASKED(LOG_UART, "%s: USB device read: EP2_FIFO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_ep2_fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_fifo[2]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP2_FIFO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_ep3_fifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_fifo[3];
	LOGMASKED(LOG_UART, "%s: USB device read: EP3_FIFO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_ep3_fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_fifo[3]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP3_FIFO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_ep4_fifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_fifo[4];
	LOGMASKED(LOG_UART, "%s: USB device read: EP4_FIFO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_ep4_fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_fifo[4]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP4_FIFO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_dma_con_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_dma_con;
	LOGMASKED(LOG_UART, "%s: USB device read: DMA_CON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_dma_con_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_dma_con);
	LOGMASKED(LOG_UART, "%s: USB device write: DMA_CON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_dma_unit_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_dma_unit;
	LOGMASKED(LOG_UART, "%s: USB device read: DMA_UNIT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_dma_unit_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_dma_unit);
	LOGMASKED(LOG_UART, "%s: USB device write: DMA_UNIT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_dma_fifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_dma_fifo;
	LOGMASKED(LOG_UART, "%s: USB device read: DMA_FIFO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_dma_fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_dma_fifo);
	LOGMASKED(LOG_UART, "%s: USB device write: DMA_FIFO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_dma_tx_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_dma_tx;
	LOGMASKED(LOG_UART, "%s: USB device read: DMA_TX: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_dma_tx_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_dma_tx);
	LOGMASKED(LOG_UART, "%s: USB device write: DMA_TX = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_test_mode_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_test_mode;
	LOGMASKED(LOG_UART, "%s: USB device read: TEST_MODE: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_test_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_test_mode);
	LOGMASKED(LOG_UART, "%s: USB device write: TEST_MODE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::usbdev_in_con_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_in_con;
	LOGMASKED(LOG_UART, "%s: USB device read: IN_CON_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::usbdev_in_con_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_in_con);
	LOGMASKED(LOG_UART, "%s: USB device write: IN_CON_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_ep_int_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_int;
	LOGMASKED(LOG_UART, "%s: USB device read: EP_INT_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_ep_int_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_int);
	LOGMASKED(LOG_UART, "%s: USB device write: EP_INT_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_usb_int_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_usb_int;
	LOGMASKED(LOG_UART, "%s: USB device read: USB_INT_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_usb_int_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_usb_int);
	LOGMASKED(LOG_UART, "%s: USB device write: USB_INT_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_ep_int_en_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_int_en;
	LOGMASKED(LOG_UART, "%s: USB device read: EP_INT_EN_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_ep_int_en_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_int_en);
	LOGMASKED(LOG_UART, "%s: USB device write: EP_INT_EN_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_usb_int_en_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_usb_int_en;
	LOGMASKED(LOG_UART, "%s: USB device read: USB_INT_EN_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_usb_int_en_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_usb_int_en);
	LOGMASKED(LOG_UART, "%s: USB device write: USB_INT_EN_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_frame_num1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_frame_num1;
	LOGMASKED(LOG_UART, "%s: USB device read: FRAME_NUM1_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_frame_num1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_frame_num1);
	LOGMASKED(LOG_UART, "%s: USB device write: FRAME_NUM1_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_frame_num2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_frame_num2;
	LOGMASKED(LOG_UART, "%s: USB device read: FRAME_NUM2_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_frame_num2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_frame_num2);
	LOGMASKED(LOG_UART, "%s: USB device write: FRAME_NUM2_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_index_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_index;
	LOGMASKED(LOG_UART, "%s: USB device read: INDEX_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_index_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_index);
	LOGMASKED(LOG_UART, "%s: USB device write: INDEX_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_in_csr2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_in_csr2;
	LOGMASKED(LOG_UART, "%s: USB device read: IN_CSR2_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_in_csr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_in_csr2);
	LOGMASKED(LOG_UART, "%s: USB device write: IN_CSR2_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_maxp_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_maxp;
	LOGMASKED(LOG_UART, "%s: USB device read: MAXP_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_maxp_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_maxp);
	LOGMASKED(LOG_UART, "%s: USB device write: MAXP_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_out_csr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_out_csr1;
	LOGMASKED(LOG_UART, "%s: USB device read: OUT_CSR1_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_out_csr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_out_csr1);
	LOGMASKED(LOG_UART, "%s: USB device write: OUT_CSR1_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_out_csr2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_out_csr2;
	LOGMASKED(LOG_UART, "%s: USB device read: OUT_CSR2_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_out_csr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_out_csr2);
	LOGMASKED(LOG_UART, "%s: USB device write: OUT_CSR2_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_out_fifo_cnt1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_out_fifo_cnt1;
	LOGMASKED(LOG_UART, "%s: USB device read: OUT_FIFO_CNT1_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_out_fifo_cnt1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_out_fifo_cnt1);
	LOGMASKED(LOG_UART, "%s: USB device write: OUT_FIFO_CNT1_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::usbdev_out_fifo_cnt2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_out_fifo_cnt2;
	LOGMASKED(LOG_UART, "%s: USB device read: OUT_FIFO_CNT2_REG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::usbdev_out_fifo_cnt2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_out_fifo_cnt2);
	LOGMASKED(LOG_UART, "%s: USB device write: OUT_FIFO_CNT2_REG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

template <int Which>
u32 s3c2410_device::usbdev_ep_dma_con_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_dma_con[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_DMA_CON: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::usbdev_ep_dma_con_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_dma_con[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_DMA_CON = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c2410_device::usbdev_ep_dma_unit_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_dma_unit[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_DMA_UNIT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::usbdev_ep_dma_unit_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_dma_unit[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_DMA_UNIT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c2410_device::usbdev_ep_dma_fifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_dma_fifo[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_DMA_FIFO: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::usbdev_ep_dma_fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_dma_fifo[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_DMA_FIFO = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c2410_device::usbdev_ep_dma_ttc_l_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_dma_ttc_l[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_DMA_TTC_L: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::usbdev_ep_dma_ttc_l_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_dma_ttc_l[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_DMA_TTC_L = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c2410_device::usbdev_ep_dma_ttc_m_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_dma_ttc_m[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_DMA_TTC_M: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::usbdev_ep_dma_ttc_m_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_dma_ttc_m[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_DMA_TTC_M = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c2410_device::usbdev_ep_dma_ttc_h_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_usbdev_ep_dma_ttc_h[Which];
	LOGMASKED(LOG_UART, "%s: USB device read: EP%d_DMA_TTC_H: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2410_device::usbdev_ep_dma_ttc_h_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_usbdev_ep_dma_ttc_h[Which]);
	LOGMASKED(LOG_UART, "%s: USB device write: EP%d_DMA_TTC_H = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}


/* Watchdog Timer */

void s3c24xx_device::wdt_reset()
{
	m_wdt_wtcon = 0x8021;
	m_wdt_wtdat = 0x8000;
	m_wdt_wtcnt = 0x8000;
	m_wdt_cnt = 0;
	m_wdt_freq = 0;
	m_wdt_timer->adjust(attotime::never);
}

void s3c2410_device::wdt_reset()
{
	s3c24xx_device::wdt_reset();
	m_gpio_gstatus[2] = 1 << 2; // Watchdog reset
}

u16 s3c24xx_device::wdt_calc_current_count()
{
	if (!m_wdt_timer->enabled() || (m_wdt_freq <= 0))
		return m_wdt_wtcnt;

	return std::min<u32>(0xffff, u32(std::ceil(m_wdt_timer->remaining().as_double() * m_wdt_cnt / m_wdt_freq)));
}

void s3c24xx_device::wdt_start()
{
	LOGMASKED(LOG_WDT, "WDT start\n");
	const u64 full_clock = get_full_clock();
	const u32 pclk_ticks = get_pclk_ticks();
	const u32 prescaler = BIT(m_wdt_wtcon, 8, 8);
	const u32 clock = 16 << BIT(m_wdt_wtcon, 3, 2);
	const u32 freq_ticks = pclk_ticks * (prescaler + 1) * clock * m_wdt_wtcnt;
	attotime period = attotime::from_ticks(freq_ticks, full_clock);
	m_wdt_freq = period.as_double();
	LOGMASKED(LOG_WDT, "watchdog start: pclk %d prescaler %d clock %d hz %f\n", pclk_ticks, prescaler, clock, period.as_hz());
	m_wdt_timer->adjust(period, 0, period);
}

void s3c2410_device::wdt_start()
{
	s3c24xx_device::wdt_start();
	m_wdt_cnt = m_wdt_wtcnt;
}

void s3c24xx_device::wdt_stop()
{
	LOGMASKED(LOG_WDT, "watchdog stop\n");
	m_wdt_wtcnt = 0;
	m_wdt_timer->adjust(attotime::never);
}

void s3c2410_device::wdt_stop()
{
	LOGMASKED(LOG_WDT, "watchdog stop\n");
	m_wdt_wtcnt = wdt_calc_current_count();
	m_wdt_timer->adjust(attotime::never);
}

void s3c24xx_device::wdt_recalc()
{
	if (BIT(m_wdt_wtcon, 5))
		wdt_start();
	else
		wdt_stop();
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::wdt_timer_exp)
{
	LOGMASKED(LOG_WDT, "WDT timer callback\n");
	if (BIT(m_wdt_wtcon, 2))
		request_wdt_irq();
	if (BIT(m_wdt_wtcon, 0))
	{
		LOGMASKED(LOG_RESET, "WDT reset\n");
		m_cpu->reset();
		reset();
	}
}

void s3c24xx_device::request_wdt_irq()
{
	request_irq(INT_WDT);
}

void s3c2440_device::request_wdt_irq()
{
	request_irq(INT_WDT);
}

u32 s3c24xx_device::wdt_wtcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_wdt_wtcon;
	LOGMASKED(LOG_WDT, "%s: watchdog read: WTCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::wdt_wtcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_wdt_wtcon;
	COMBINE_DATA(&m_wdt_wtcon);
	LOGMASKED(LOG_WDT, "%s: watchdog write: WTCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (BIT(data ^ old, 5))
		wdt_recalc();
}

u32 s3c24xx_device::wdt_wtdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_wdt_wtdat;
	LOGMASKED(LOG_WDT, "%s: watchdog read: WTDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::wdt_wtdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_wdt_wtdat);
	LOGMASKED(LOG_WDT, "%s: watchdog write: WTDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::wdt_wtcnt_r(offs_t offset, u32 mem_mask)
{
	u32 data = m_wdt_wtcnt;
	// is wdt active?
	if (BIT(m_wdt_wtcon, 5))
		data = 0;
	LOGMASKED(LOG_WDT, "%s: watchdog read: WTCNT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

u32 s3c2410_device::wdt_wtcnt_r(offs_t offset, u32 mem_mask)
{
	u32 data = m_wdt_wtcnt;
	// is wdt active?
	if (BIT(m_wdt_wtcon, 5))
		data = wdt_calc_current_count();
	LOGMASKED(LOG_WDT, "%s: watchdog read: WTCNT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::wdt_wtcnt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_wdt_wtcnt);
	LOGMASKED(LOG_WDT, "%s: watchdog write: WTCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* IIC */

void s3c24xx_device::iic_reset()
{
	m_iic_iiccon = 0;
	m_iic_iicstat = 0;
	m_iic_iicadd = 0;
	m_iic_iicds = 0;
	m_iic_count = 0;
	m_iic_timer->adjust(attotime::never);
}

void s3c2440_device::iic_reset()
{
	s3c24xx_device::iic_reset();
	m_iic_iiclc = 0;
}

void s3c24xx_device::i2c_send_start()
{
	// FIXME: this needs to sense busy condition and use realistic timing
	//LOGMASKED(LOG_I2C, "i2c_send_start\n");
	m_sda_w_cb(1);
	m_scl_w_cb(1);
	m_sda_w_cb(0);
	m_scl_w_cb(0);
}

void s3c24xx_device::i2c_send_stop()
{
	// FIXME: this needs realistic timing
	//LOGMASKED(LOG_I2C, "i2c_send_stop\n");
	m_sda_w_cb(0);
	m_scl_w_cb(1);
	m_sda_w_cb(1);
	m_scl_w_cb(0);
}

u8 s3c24xx_device::i2c_receive_byte(int ack)
{
	u8 data = 0;
	//LOGMASKED(LOG_I2C, "i2c_receive_byte ...\n");
	m_sda_w_cb(1);
	for (int i = 0; i < 8; i++)
	{
		m_scl_w_cb(1);
		data <<= 1;
		data |= m_sda_r_cb();
		m_scl_w_cb(0);
	}
	LOGMASKED(LOG_I2C, "recv data %02X\n", data);
	//LOGMASKED(LOG_I2C, "send ack %d\n", ack);
	m_sda_w_cb(ack ? 0 : 1);
	m_scl_w_cb(1);
	m_scl_w_cb(0);
	return data;
}

int s3c24xx_device::i2c_send_byte(u8 data)
{
	//LOGMASKED(LOG_I2C, "i2c_send_byte ...\n");
	LOGMASKED(LOG_I2C, "send data %02X\n", data);
	for (int i = 0; i < 8; i++)
	{
		m_sda_w_cb(BIT(data, 7));
		data <<= 1;
		m_scl_w_cb(1);
		m_scl_w_cb(0);
	}
	m_sda_w_cb(1); // ack bit
	m_scl_w_cb(1);
	int ack = m_sda_r_cb();
	//LOGMASKED(LOG_I2C, "recv ack %d\n", ack);
	m_scl_w_cb(0);
	return ack;
}

void s3c24xx_device::iic_start()
{
	//LOGMASKED(LOG_I2C, "I2C start\n");
	i2c_send_start();
	int mode_selection = BIT(m_iic_iicstat, 6, 2);
	switch (mode_selection)
	{
		case 2: m_iic_iicstat = (m_iic_iicstat & ~1U) | i2c_send_byte(m_iic_iicds | 0x01); break;
		case 3: m_iic_iicstat = (m_iic_iicstat & ~1U) | i2c_send_byte(m_iic_iicds & ~0x01); break;
	}
	m_iic_timer->adjust(get_pclk() * (9 * (BIT(m_iic_iiccon, 6) ? 512 : 16) * (BIT(m_iic_iiccon, 0, 4) + 1)));
}

void s3c24xx_device::iic_stop()
{
	//LOGMASKED(LOG_I2C, "IIC stop\n");
	i2c_send_stop();
	m_iic_timer->adjust(attotime::never);
}

void s3c24xx_device::iic_resume()
{
	//LOGMASKED(LOG_I2C, "IIC resume\n");
	int mode_selection = BIT(m_iic_iicstat, 6, 2);
	switch (mode_selection)
	{
		case 2: m_iic_iicds = i2c_receive_byte(BIT(m_iic_iiccon, 7)); break;
		case 3: m_iic_iicstat = (m_iic_iicstat & ~1U) | i2c_send_byte(m_iic_iicds & 0xff); break;
	}
	m_iic_timer->adjust(get_pclk() * (9 * (BIT(m_iic_iiccon, 6) ? 512 : 16) * (BIT(m_iic_iiccon, 0, 4) + 1)));
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::iic_timer_exp)
{
	//LOGMASKED(LOG_I2C, "I2C timer callback\n");
	m_iic_count++;
	if (BIT(m_iic_iiccon, 5))
	{
		m_iic_iiccon |= (1 << 4); // [bit 4] interrupt is pending
		request_irq(INT_IIC);
	}
}

u32 s3c24xx_device::iic_iiccon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iic_iiccon;
	//LOGMASKED(LOG_I2C, "%s: i2c read: IICCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iic_iiccon_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_iic_iiccon;
	COMBINE_DATA(&m_iic_iiccon);
	//LOGMASKED(LOG_I2C, "%s: i2c write: IICCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (BIT(old, 4) && !BIT(data, 4))
	{
		const bool start_stop_condition = BIT(m_iic_iicstat, 5);
		if (start_stop_condition)
		{
			if (m_iic_count == 0)
				iic_start();
			else
				iic_resume();
		}
		else
			iic_stop();
	}
}

u32 s3c24xx_device::iic_iicstat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iic_iicstat;
	//LOGMASKED(LOG_I2C, "%s: i2c read: IICSTAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iic_iicstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	mem_mask &= 0xf0;
	COMBINE_DATA(&m_iic_iicstat);
	//LOGMASKED(LOG_I2C, "%s: i2c write: IICSTAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_iic_count = 0;
	if (!BIT(m_iic_iiccon, 4))
	{
		const bool start_stop_condition = BIT(data, 5);
		if (start_stop_condition)
		{
			if (m_iic_count == 0)
				iic_start();
			else
				iic_resume();
		}
		else
			iic_stop();
	}
}

u32 s3c24xx_device::iic_iicadd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iic_iicadd;
	//LOGMASKED(LOG_I2C, "%s: i2c read: IICADD: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iic_iicadd_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_iic_iicadd);
	//LOGMASKED(LOG_I2C, "%s: i2c write: IICADD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::iic_iicds_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iic_iicds;
	//LOGMASKED(LOG_I2C, "%s: i2c read: IICDS: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iic_iicds_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_iic_iicds);
	//LOGMASKED(LOG_I2C, "%s: i2c write: IICDS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::iic_iiclc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iic_iiclc;
	//LOGMASKED(LOG_I2C, "%s: i2c read: IICLC: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::iic_iiclc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_iic_iiclc);
	//LOGMASKED(LOG_I2C, "%s: i2c write: IICLC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* IIS */

u32 s3c24xx_device::iis_fifo_count_bits() const
{
	return 4;
}

u32 s3c2410_device::iis_fifo_count_bits() const
{
	return 6;
}

void s3c24xx_device::iis_reset()
{
	m_iis_iiscon = 0x0100;
	m_iis_iismod = 0;
	m_iis_iispsr = 0;
	m_iis_iisfcon = 0;
	m_iis_timer->adjust(attotime::never);
	memset(m_iis_tx_fifo, 0, sizeof(m_iis_tx_fifo));
	m_iis_fifo_head = 0;
	m_iis_fifo_count = 0;
	m_iis_channel = 1;
}

void s3c24xx_device::iis_start()
{
	LOGMASKED(LOG_I2S, "IIS start\n");
	static constexpr u32 CODECLK_TABLE[2] = { 256, 384 };

	const int prescaler_enable = BIT(m_iis_iiscon, 1);
	const int prescaler_control_a = (m_iis_iispsr >> 5) & 0x1f;
	const int prescaler_control_b = m_iis_iispsr & 0x1f;
	const int codeclk = BIT(m_iis_iismod, 2);

	const attotime pclk = get_pclk();
	const attotime period = pclk * (prescaler_enable ? (prescaler_control_a + 1) : 1) * CODECLK_TABLE[codeclk];

	LOGMASKED(LOG_I2S, "IIS - pclk is %dHz, psc_enable is %d, psc_a is %d, psc_b is %d, codeclk is %d, sample freq is %fHz\n", (u32)pclk.as_hz(), prescaler_enable, prescaler_control_a, prescaler_control_b, CODECLK_TABLE[codeclk], period.as_hz());
	// One timer tick shifts one channel; the DAC clock is the stereo frame rate.
	m_iis_timer->adjust(period / 2, 0, period / 2);
	m_iis_clockchange_w_cb((u32)period.as_hz());
}

void s3c24xx_device::iis_stop()
{
	LOGMASKED(LOG_I2S, "IIS stop\n");
	m_iis_timer->adjust(attotime::never);
}

void s3c24xx_device::iis_recalc()
{
	// TODO: external clock input in slave mode and receive transfers.
	if (BIT(m_iis_iiscon, 0) && !BIT(m_iis_iismod, 8) && BIT(m_iis_iismod, 7) && !BIT(m_iis_iiscon, 3))
		iis_start();
	else
		iis_stop();
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::iis_timer_exp)
{
	const u32 count_bits = iis_fifo_count_bits();
	const u32 depth = 1U << (count_bits - 1);
	const bool fifo_enabled = BIT(m_iis_iisfcon, 2 * count_bits + 1);
	if (fifo_enabled && BIT(m_iis_iiscon, 5) && BIT(m_iis_iisfcon, 2 * count_bits + 3) && (m_iis_fifo_count < depth))
		dma_request_iis();

	m_iis_channel ^= 1;
	u16 data = 0;
	if (fifo_enabled && m_iis_fifo_count)
	{
		data = m_iis_tx_fifo[m_iis_fifo_head];
		m_iis_fifo_head = (m_iis_fifo_head + 1) & 31;
		m_iis_fifo_count--;
	}
	if (!BIT(m_iis_iismod, 3))
		data <<= 8;
	m_iis_data_w_cb(m_iis_channel, data, 0xffff);
}

u32 s3c24xx_device::iis_iiscon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_iis_iiscon & 0x3f) | (m_iis_channel << 8) | (m_iis_fifo_count ? 0x80 : 0) | 0x40;
	LOGMASKED(LOG_I2S, "%s: i2s read: IISCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iis_iiscon_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_iis_iiscon;
	mem_mask &= 0x3f;
	COMBINE_DATA(&m_iis_iiscon);
	LOGMASKED(LOG_I2S, "%s: i2s write: IISCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_I2S, "%s:            IIS Interface: %s\n", machine().describe_context(), BIT(data, 0) ? "Enabled" : "Disabled");
	LOGMASKED(LOG_I2S, "%s:            IIS Prescaler: %s\n", machine().describe_context(), BIT(data, 1) ? "Enabled" : "Disabled");
	LOGMASKED(LOG_I2S, "%s:            Rx Idle: %s\n", machine().describe_context(), BIT(data, 2) ? "Idle" : "Active");
	LOGMASKED(LOG_I2S, "%s:            Tx Idle: %s\n", machine().describe_context(), BIT(data, 3) ? "Idle" : "Active");
	LOGMASKED(LOG_I2S, "%s:            Rx DREQ: %s\n", machine().describe_context(), BIT(data, 4) ? "Enabled" : "Disabled");
	LOGMASKED(LOG_I2S, "%s:            Tx DREQ: %s\n", machine().describe_context(), BIT(data, 5) ? "Enabled" : "Disabled");
	if ((m_iis_iiscon ^ old) & 0x0b)
		iis_recalc();
}

u32 s3c24xx_device::iis_iismod_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iis_iismod;
	LOGMASKED(LOG_I2S, "%s: i2s read: IISMOD: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iis_iismod_w(offs_t offset, u32 data, u32 mem_mask)
{
	constexpr std::string_view SCLK_NAMES[4] = { "16fs", "32fs", "48fs", "N/A" };
	COMBINE_DATA(&m_iis_iismod);
	LOGMASKED(LOG_I2S, "%s: i2s write: IISMOD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_I2S, "%s:            Serial bit clock frequency select: %s\n", machine().describe_context(), SCLK_NAMES[data & 0x03]);
	LOGMASKED(LOG_I2S, "%s:            Master clock frequency select: %s\n", machine().describe_context(), BIT(data, 2) ? "384fs" : "256fs");
	LOGMASKED(LOG_I2S, "%s:            Serial bits per channel: %d\n", machine().describe_context(), BIT(data, 3) ? "16" : "8");
	LOGMASKED(LOG_I2S, "%s:            Serial format: %s\n", machine().describe_context(), BIT(data, 4) ? "MSB-justified" : "I2S-compatible");
	LOGMASKED(LOG_I2S, "%s:            Active channel: %s\n", machine().describe_context(), BIT(data, 5) ? "Left-high/Right-low" : "Right-high/Left-low");
	LOGMASKED(LOG_I2S, "%s:            Receive: %s\n", machine().describe_context(), BIT(data, 6) ? "Yes" : "No");
	LOGMASKED(LOG_I2S, "%s:            Transmit: %s\n", machine().describe_context(), BIT(data, 7) ? "Yes" : "No");
	LOGMASKED(LOG_I2S, "%s:            Output/Input: %s\n", machine().describe_context(), BIT(data, 8) ? "Input" : "Output");
	iis_recalc();
}

u32 s3c24xx_device::iis_iispsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iis_iispsr;
	LOGMASKED(LOG_I2S, "%s: i2s read: IISPSR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iis_iispsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_iis_iispsr);
	LOGMASKED(LOG_I2S, "%s: i2s write: IISPSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_I2S, "%s:            Prescaler A: %d\n", machine().describe_context(), ((data >> 5) & 0x1f) + 1);
	LOGMASKED(LOG_I2S, "%s:            Prescaler B: %d\n", machine().describe_context(), (data & 0x1f) + 1);
	iis_recalc();
}

u32 s3c24xx_device::iis_iisfcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_iis_iisfcon | (m_iis_fifo_count << iis_fifo_count_bits());
	LOGMASKED(LOG_I2S, "%s: i2s read: IISFCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iis_iisfcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 count_bits = iis_fifo_count_bits();
	mem_mask &= 0xfU << (2 * count_bits);
	COMBINE_DATA(&m_iis_iisfcon);
	LOGMASKED(LOG_I2S, "%s: i2s write: IISFCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (!BIT(m_iis_iisfcon, 2 * count_bits + 1))
	{
		m_iis_fifo_head = 0;
		m_iis_fifo_count = 0;
	}
}

u32 s3c24xx_device::iis_iisfifo_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0;
	LOGMASKED(LOG_I2S, "%s: i2s read: IISFIFO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::iis_iisfifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	//LOGMASKED(LOG_I2S, "%s: i2s write: IISFIFO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (ACCESSING_BITS_0_15)
		iis_fifo_push(BIT(data, 0, 16));
	if (ACCESSING_BITS_16_31)
		iis_fifo_push(BIT(data, 16, 16));
}

void s3c24xx_device::iis_fifo_push(u16 data)
{
	const u32 count_bits = iis_fifo_count_bits();
	if (BIT(m_iis_iisfcon, 2 * count_bits + 1) && (m_iis_fifo_count < (1U << (count_bits - 1))))
	{
		m_iis_tx_fifo[(m_iis_fifo_head + m_iis_fifo_count) & 31] = data;
		m_iis_fifo_count++;
	}
}

/* RTC */

void s3c24xx_device::rtc_reset()
{
	m_rtc_rtccon = 0;
	m_rtc_ticnt = 0;
	m_rtc_rtcalm = 0;
	m_rtc_almsec = 0;
	m_rtc_almmin = 0;
	m_rtc_almhour = 0;
	m_rtc_almday = 1;
	m_rtc_almmon = 1;
	m_rtc_almyear = 0;
	m_rtc_rtcrst = 0;
	m_rtc_bcdsec = 0;
	m_rtc_bcdmin = 0;
	m_rtc_bcdhour = 0;
	m_rtc_bcddate = 0;
	m_rtc_bcdday = 0;
	m_rtc_bcdmon = 0;
	m_rtc_bcdyear = 0;
	m_rtc_timer_update->adjust(attotime::from_msec(1000), 0, attotime::from_msec(1000));
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::rtc_timer_tick_count_exp)
{
	LOGMASKED(LOG_RTC, "RTC timer callback (tick count)\n");
	request_irq(INT_TICK);
}

void s3c24xx_device::rtc_update()
{
	LOGMASKED(LOG_RTC, "RTC - %04d/%02d/%02d %02d:%02d:%02d\n", bcd_2_dec(m_rtc_bcdyear) + 2000, bcd_2_dec(m_rtc_bcdmon), bcd_2_dec(m_rtc_bcdday), bcd_2_dec(m_rtc_bcdhour), bcd_2_dec(m_rtc_bcdmin), bcd_2_dec(m_rtc_bcdsec));
	// increase second
	m_rtc_bcdsec = bcd_adjust(m_rtc_bcdsec + 1);
	if (m_rtc_bcdsec >= 0x60)
	{
		m_rtc_bcdsec = 0;
		// increase minute
		m_rtc_bcdmin = bcd_adjust(m_rtc_bcdmin + 1);
		if (m_rtc_bcdmin >= 0x60)
		{
			m_rtc_bcdmin = 0;
			// increase hour
			m_rtc_bcdhour = bcd_adjust(m_rtc_bcdhour + 1);
			if (m_rtc_bcdhour >= 0x24)
			{
				m_rtc_bcdhour = 0;
				// increase day-of-week
				m_rtc_bcdday = (m_rtc_bcdday % 7) + 1;
				// increase day
				m_rtc_bcddate = bcd_adjust(m_rtc_bcddate + 1);
				u32 const bcdday_max = dec_2_bcd(gregorian_days_in_month(bcd_2_dec(m_rtc_bcdmon), bcd_2_dec(m_rtc_bcdyear) + 2000));
				if (m_rtc_bcdday > bcdday_max)
				{
					m_rtc_bcdday = 1;
					// increase month
					m_rtc_bcdmon = bcd_adjust(m_rtc_bcdmon + 1);
					if (m_rtc_bcdmon >= 0x12)
					{
						m_rtc_bcdmon = 1;
						// increase year
						m_rtc_bcdyear = bcd_adjust(m_rtc_bcdyear + 1);
						if (m_rtc_bcdyear >= 0x100)
						{
							m_rtc_bcdyear = 0;
						}
					}
				}
			}
		}
	}
}

void s3c24xx_device::rtc_check_alarm()
{
	if ((m_rtc_rtcalm & 0x40) &&
		(!(m_rtc_rtcalm & 0x20) || (m_rtc_almyear == m_rtc_bcdyear)) &&
		(!(m_rtc_rtcalm & 0x10) || (m_rtc_almmon == m_rtc_bcdmon)) &&
		(!(m_rtc_rtcalm & 0x08) || (m_rtc_almday == m_rtc_bcdday)) &&
		(!(m_rtc_rtcalm & 0x04) || (m_rtc_almhour == m_rtc_bcdhour)) &&
		(!(m_rtc_rtcalm & 0x02) || (m_rtc_almmin == m_rtc_bcdmin)) &&
		(!(m_rtc_rtcalm & 0x01) || (m_rtc_almsec == m_rtc_bcdsec)))
	{
		request_irq(INT_RTC);
	}
}

TIMER_CALLBACK_MEMBER(s3c24xx_device::rtc_timer_update_exp)
{
	LOGMASKED(LOG_RTC, "RTC timer callback (update)\n");
	rtc_update();
	rtc_check_alarm();
}

u32 s3c24xx_device::rtc_rtccon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_rtccon;
	LOGMASKED(LOG_RTC, "%s: rtc read: RTCCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_rtccon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_rtccon);
	LOGMASKED(LOG_RTC, "%s: rtc write: RTCCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_ticnt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_ticnt;
	LOGMASKED(LOG_RTC, "%s: rtc read: TICNT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_ticnt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_ticnt);
	LOGMASKED(LOG_RTC, "%s: rtc write: TICNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (BIT(m_rtc_ticnt, 7))
	{
		const u32 ttc = BIT(m_rtc_ticnt, 0, 7);
		const double freq = 128.0 / (ttc + 1);
		LOG("ttc %u freq %f\n", ttc, freq);
		m_rtc_timer_tick_count->adjust(attotime::from_hz(freq), 0, attotime::from_hz(freq));
	}
	else
	{
		m_rtc_timer_tick_count->adjust(attotime::never);
	}
}

u32 s3c24xx_device::rtc_rtcalm_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_rtcalm;
	LOGMASKED(LOG_RTC, "%s: rtc read: RTCALM: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_rtcalm_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_rtcalm);
	LOGMASKED(LOG_RTC, "%s: rtc write: RTCALM = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_almsec_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_almsec;
	LOGMASKED(LOG_RTC, "%s: rtc read: ALMSEC: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_almsec_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_almsec);
	LOGMASKED(LOG_RTC, "%s: rtc write: ALMSEC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_almmin_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_almmin;
	LOGMASKED(LOG_RTC, "%s: rtc read: ALMMIN: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_almmin_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_almmin);
	LOGMASKED(LOG_RTC, "%s: rtc write: ALMMIN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_almhour_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_almhour;
	LOGMASKED(LOG_RTC, "%s: rtc read: ALMHOUR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_almhour_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_almhour);
	LOGMASKED(LOG_RTC, "%s: rtc write: ALMHOUR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_almday_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_almday;
	LOGMASKED(LOG_RTC, "%s: rtc read: ALMDAY: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_almday_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_almday);
	LOGMASKED(LOG_RTC, "%s: rtc write: ALMDAY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_almmon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_almmon;
	LOGMASKED(LOG_RTC, "%s: rtc read: ALMMON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_almmon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_almmon);
	LOGMASKED(LOG_RTC, "%s: rtc write: ALMMON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_almyear_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_almyear;
	LOGMASKED(LOG_RTC, "%s: rtc read: ALMYEAR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_almyear_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_almyear);
	LOGMASKED(LOG_RTC, "%s: rtc write: ALMYEAR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_rtcrst_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_rtcrst;
	LOGMASKED(LOG_RTC, "%s: rtc read: RTCRST: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_rtcrst_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_rtcrst);
	LOGMASKED(LOG_RTC, "%s: rtc write: RTCRST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_bcdsec_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_bcdsec;
	LOGMASKED(LOG_RTC, "%s: rtc read: BCDSEC: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_bcdsec_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_bcdsec);
	LOGMASKED(LOG_RTC, "%s: rtc write: BCDSEC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_bcdmin_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_bcdmin;
	LOGMASKED(LOG_RTC, "%s: rtc read: BCDMIN: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_bcdmin_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_bcdmin);
	LOGMASKED(LOG_RTC, "%s: rtc write: BCDMIN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_bcdhour_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_bcdhour;
	LOGMASKED(LOG_RTC, "%s: rtc read: BCDHOUR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_bcdhour_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_bcdhour);
	LOGMASKED(LOG_RTC, "%s: rtc write: BCDHOUR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_bcddate_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_bcddate;
	LOGMASKED(LOG_RTC, "%s: rtc read: BCDDATE: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_bcddate_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_bcddate);
	LOGMASKED(LOG_RTC, "%s: rtc write: BCDDATE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_bcdday_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_bcdday;
	LOGMASKED(LOG_RTC, "%s: rtc read: BCDDAY: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_bcdday_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_bcdday);
	LOGMASKED(LOG_RTC, "%s: rtc write: BCDDAY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_bcdmon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_bcdmon;
	LOGMASKED(LOG_RTC, "%s: rtc read: BCDMON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_bcdmon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_bcdmon);
	LOGMASKED(LOG_RTC, "%s: rtc write: BCDMON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c24xx_device::rtc_bcdyear_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_bcdyear;
	LOGMASKED(LOG_RTC, "%s: rtc read: BCDYEAR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::rtc_bcdyear_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rtc_bcdyear);
	LOGMASKED(LOG_RTC, "%s: rtc write: BCDYEAR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* A/D Converter */

void s3c24xx_device::adc_reset()
{
	m_adc_adccon = 0x3fc4;
}

void s3c2410_device::adc_reset()
{
	s3c24xx_device::adc_reset();
	m_adc_adctsc = 0x58;
	m_adc_adcdly = 0xff;
}

void s3c24xx_device::adc_start()
{
	LOGMASKED(LOG_ADC, "ADC start\n");
	m_adc_adccon &= ~(1 << 0); // A/D conversion is completed
	m_adc_adccon |= (1 << 15); // End of A/D conversion
}

void s3c2410_device::adc_start()
{
	s3c24xx_device::adc_start();
	request_subirq(SUBINT_ADC);
}

u32 s3c24xx_device::adc_adccon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_adc_adccon;
	LOGMASKED(LOG_ADC, "%s: ADC read: ADCCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c24xx_device::adc_adccon_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_adc_adccon;
	COMBINE_DATA(&m_adc_adccon);
	if (!BIT(old, 0) && BIT(m_adc_adccon, 0))
		adc_start();
	LOGMASKED(LOG_ADC, "%s: ADC write: ADCCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::adc_adcdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_data_r_cb((m_adc_adccon >> 3) & 7);
	LOGMASKED(LOG_ADC, "%s: ADC read: ADCDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::adc_adcdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ADC, "%s: ADC write: ADCDAT (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::adc_adctsc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_adc_adctsc;
	LOGMASKED(LOG_ADC, "%s: ADC read: ADCTSC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::adc_adctsc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_adc_adctsc);
	LOGMASKED(LOG_ADC, "%s: ADC write: ADCTSC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::adc_adcdly_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_adc_adcdly;
	LOGMASKED(LOG_ADC, "%s: ADC read: ADCDLY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::adc_adcdly_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_adc_adcdly);
	LOGMASKED(LOG_ADC, "%s: ADC write: ADCDLY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::adc_adcdat0_r(offs_t offset, u32 mem_mask)
{
	const u32 channel = (m_adc_adccon >> 3) & 7;
	m_adc_adcdat[0] = m_data_r_cb(channel << 1);
	LOGMASKED(LOG_ADC, "%s: ADC read: ADCDAT0 = %08x & %08x\n", machine().describe_context(), m_adc_adcdat[0], mem_mask);
	return m_adc_adcdat[0];
}

void s3c2410_device::adc_adcdat0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ADC, "%s: ADC write: ADCDAT0 (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::adc_adcdat1_r(offs_t offset, u32 mem_mask)
{
	const u32 channel = (m_adc_adccon >> 3) & 7;
	m_adc_adcdat[1] = m_data_r_cb((channel << 1) | 1);
	LOGMASKED(LOG_ADC, "%s: ADC read: ADCDAT1 = %08x & %08x\n", machine().describe_context(), m_adc_adcdat[1], mem_mask);
	return m_adc_adcdat[1];
}

void s3c2410_device::adc_adcdat1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ADC, "%s: ADC write: ADCDAT1 (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::adc_adcupdn_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_adc_adcupdn;
	LOGMASKED(LOG_ADC, "%s: ADC read: ADCUPDN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::adc_adcupdn_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_adc_adcupdn);
	LOGMASKED(LOG_ADC, "%s: ADC write: ADCUPDN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

void s3c2410_device::touch_screen(int state)
{
	m_adc_adcdat[0] = ((state ? 0 : 1) << 15);
	m_adc_adcdat[1] = ((state ? 0 : 1) << 15);
	request_subirq(SUBINT_TC);
}

/* SPI */

void s3c24xx_device::spi_reset()
{
	std::fill(std::begin(m_spi_spcon), std::end(m_spi_spcon), 0);
	std::fill(std::begin(m_spi_spsta), std::end(m_spi_spsta), 1);
	std::fill(std::begin(m_spi_sppin), std::end(m_spi_sppin), 2);
	std::fill(std::begin(m_spi_sppre), std::end(m_spi_sppre), 0);
	std::fill(std::begin(m_spi_sptdat), std::end(m_spi_sptdat), 0);
	std::fill(std::begin(m_spi_sprdat), std::end(m_spi_sprdat), 0);
}

void s3c2440_device::spi_reset()
{
	s3c24xx_device::spi_reset();
	std::fill(std::begin(m_spi_sppin), std::end(m_spi_sppin), 0);
}

template <int Which>
u32 s3c24xx_device::spi_spcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_spi_spcon[Which];
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPCON: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::spi_spcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spi_spcon[Which]);
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPCON = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::spi_spsta_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_spi_spsta[Which] | (1 << 0); // [bit 0] Transfer Ready Flag
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPSTA: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::spi_spsta_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spi_spsta[Which]);
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPSTA = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::spi_sppin_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_spi_sppin[Which];
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPPIN: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::spi_sppin_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spi_sppin[Which]);
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPPIN = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::spi_sppre_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_spi_sppre[Which];
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPPRE: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::spi_sppre_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spi_sppre[Which]);
	LOGMASKED(LOG_SPI, "%s: SPI %d write, SPPRE = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::spi_sptdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_spi_sptdat[Which];
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPTDAT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::spi_sptdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spi_sptdat[Which]);
	LOGMASKED(LOG_SPI, "%s: SPI %d write, SPTDAT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c24xx_device::spi_sprdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_spi_sprdat[Which];
	LOGMASKED(LOG_SPI, "%s: SPI %d read, SPRDAT: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c24xx_device::spi_sprdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spi_sprdat[Which]);
	LOGMASKED(LOG_SPI, "%s: SPI %d write, SPRDAT = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

/* MMC Interface */

void s3c2400_device::mmc_reset()
{
	m_mmc_mmcon = 0;
	m_mmc_mmcrr = 0;
	m_mmc_mmfcon = 0;
	m_mmc_mmsta = 0;
	m_mmc_mmfsta = 0;
	m_mmc_mmpre = 0;
	m_mmc_mmlen = 0;
	m_mmc_mmcr7 = 0;
	std::fill(std::begin(m_mmc_mmrsp), std::end(m_mmc_mmrsp), 0);
	std::fill(std::begin(m_mmc_mmcmd), std::end(m_mmc_mmcmd), 0);
	m_mmc_mmcr16 = 0;
	m_mmc_mmdat = 0;
}

u32 s3c2400_device::mmc_mmcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmcon;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmcon);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmcrr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmcrr;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMCRR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmcrr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmcrr);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMCRR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmfcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmfcon;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMFCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmfcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmfcon);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMFCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmsta_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmsta;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMSTA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmsta_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmsta);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMSTA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmfsta_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmfsta;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMFSTA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmfsta_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmfsta);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMFSTA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmpre_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmpre;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMPRE: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmpre_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmpre);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMPRE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmlen_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmlen;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMLEN: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmlen_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmlen);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMLEN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmcr7_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmcr7;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMCR7: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmcr7_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmcr7);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMCR7 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

template <int Which>
u32 s3c2400_device::mmc_mmrsp_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmrsp[Which];
	LOGMASKED(LOG_MMC, "%s: MMC read: MMRSP%d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2400_device::mmc_mmrsp_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmrsp[Which]);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMRSP%d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 s3c2400_device::mmc_mmcmd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmcmd[Which];
	LOGMASKED(LOG_MMC, "%s: MMC read: MMCMD%d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void s3c2400_device::mmc_mmcmd_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmcmd[Which]);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMCMD%d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

u32 s3c2400_device::mmc_mmcr16_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmcr16;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMCR16: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmcr16_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmcr16);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMCR16 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2400_device::mmc_mmdat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mmc_mmdat;
	LOGMASKED(LOG_MMC, "%s: MMC read: MMDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2400_device::mmc_mmdat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mmc_mmdat);
	LOGMASKED(LOG_MMC, "%s: MMC write: MMDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* SD Interface */

void s3c2410_device::sdi_reset()
{
	m_sdi_con = 0;
	m_sdi_pre = 0;
	m_sdi_cmdarg = 0;
	m_sdi_cmdcon = 0;
	m_sdi_cmdsta = 0;
	m_sdi_rsp0 = 0;
	m_sdi_rsp1 = 0;
	m_sdi_rsp2 = 0;
	m_sdi_rsp3 = 0;
	m_sdi_dtimer = 0x2000;
	m_sdi_bsize = 0;
	m_sdi_datcon = 0;
	m_sdi_adidatcnt = 0;
	m_sdi_adidatsta = 0;
	m_sdi_fsta = 0;
	m_sdi_intmsk = 0;
	m_sdi_dat = 0;
}

void s3c2440_device::sdi_reset()
{
	s3c2410_device::sdi_reset();
	m_sdi_pre = 1;
	m_sdi_dtimer = 0x10000;
}

u32 s3c2410_device::sdi_con_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_con;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDICON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_con_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_con);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDICON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_pre_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_pre;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIPRE: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_pre_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_pre);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIPRE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_cmdarg_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_cmdarg;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDICMDARG: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_cmdarg_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_cmdarg);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDICMDARG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_cmdcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_cmdcon;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDICMDCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_cmdcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_cmdcon);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDICMDCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_cmdsta_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_cmdsta;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDICMDSTA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_cmdsta_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_cmdsta);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDICMDSTA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_rsp0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_rsp0;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIRSP0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_rsp0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_rsp0);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIRSP0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_rsp1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_rsp1;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIRSP1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_rsp1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_rsp1);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIRSP1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_rsp2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_rsp2;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIRSP2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_rsp2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_rsp2);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIRSP2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_rsp3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_rsp3;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIRSP3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_rsp3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_rsp2);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIRSP3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_dtimer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_dtimer;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIDTIMER: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_dtimer_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_dtimer);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIDTIMER = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_bsize_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_bsize;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIBSIZE: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_bsize_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_bsize);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIBSIZE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_datcon_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_datcon;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIDATCON: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_datcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_datcon);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIDATCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_adidatcnt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_adidatcnt;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIADIDATCNT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_adidatcnt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_adidatcnt);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIDATCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_adidatsta_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_adidatsta;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIADIDATSTA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_adidatsta_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_adidatsta);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIADIDATSTA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_fsta_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_fsta;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIFSTA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_fsta_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_fsta);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIFSTA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_intmsk_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_intmsk;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIINTMSK: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_intmsk_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_intmsk);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIINTMSK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2410_device::sdi_dat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_sdi_dat;
	LOGMASKED(LOG_SDI, "%s: SDI read: SDIDAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::sdi_dat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sdi_dat);
	LOGMASKED(LOG_SDI, "%s: SDI write: SDIDAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* NAND Flash */

void s3c2410_device::nand_reset()
{
	m_nand_nfconf = 0;
	m_nand_nfcmd = 0;
	m_nand_nfaddr = 0;
	m_nand_nfstat = 0;
	m_nand_nfecc = 0;
	std::fill(std::begin(m_nand_mecc), std::end(m_nand_mecc), 0);
	m_nand_ecc_pos = 0;
	m_nand_data_count = 0;
}

void s3c2440_device::nand_reset()
{
	s3c2410_device::nand_reset();
	m_nand_nfconf = 0x1000;
	m_nand_nfcont = 0x0384;
	std::fill(std::begin(m_nand_nfmeccd), std::end(m_nand_nfmeccd), 0);
	m_nand_nfseccd = 0;
	std::fill(std::begin(m_nand_nfestat), std::end(m_nand_nfestat), 0);
	std::fill(std::begin(m_nand_nfmecc), std::end(m_nand_nfmecc), 0);
	m_nand_nfsecc = 0;
	m_nand_nfsblk = 0;
	m_nand_nfeblk = 0;
	std::fill(std::begin(m_nand_secc), std::end(m_nand_secc), 0);
	m_nand_ecc_pos = 0;
	m_nand_data_count = 0;
}

void s3c2410_device::nand_calculate_mecc(u8 *data, u32 size, u8 *mecc)
{
	mecc[0] = mecc[1] = mecc[2] = mecc[3] = 0xff;
	for (int i = 0; i < size; i++)
		nand_update_mecc(mecc, i, data[i]);
}

void s3c2410_device::nand_update_mecc(u8 *ecc, int pos, u8 data)
{
	int bit[8];
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
	u8 temp = bit[7] ^ bit[6] ^ bit[5] ^ bit[4] ^ bit[3] ^ bit[2] ^ bit[1] ^ bit[0];
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

void s3c2440_device::nand_update_secc(u8 *ecc, int pos, u8 data)
{
	int bit[8];
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
	u8 temp = bit[7] ^ bit[6] ^ bit[5] ^ bit[4] ^ bit[3] ^ bit[2] ^ bit[1] ^ bit[0];
	if (pos & 0x001) ecc[0] ^= (temp << 5); else ecc[0] ^= (temp << 4);
	if (pos & 0x002) ecc[0] ^= (temp << 7); else ecc[0] ^= (temp << 6);
	if (pos & 0x004) ecc[1] ^= (temp << 3); else ecc[1] ^= (temp << 2);
	if (pos & 0x008) ecc[1] ^= (temp << 5); else ecc[1] ^= (temp << 4);
}

void s3c2410_device::nand_update_ecc(u8 data)
{
	u8 temp[3] = { m_nand_mecc[0], m_nand_mecc[1], m_nand_mecc[2] };
	nand_update_mecc(m_nand_mecc, m_nand_ecc_pos++, data);
	LOGMASKED(LOG_FLASH, "%s: NAND - MECC %03x - %02x %02x %02x -> %02x %02x %02x\n", machine().describe_context(), m_nand_ecc_pos - 1, temp[0], temp[1], temp[2], m_nand_mecc[0], m_nand_mecc[1], m_nand_mecc[2]);
	if (m_nand_ecc_pos == 512)
		m_nand_ecc_pos = 0;
}

void s3c2440_device::nand_update_ecc(const u8 data)
{
	u8 temp[4];
	if (!BIT(m_nand_nfcont, 5))
	{
		temp[0] = m_nand_mecc[0];
		temp[1] = m_nand_mecc[1];
		temp[2] = m_nand_mecc[2];
		temp[3] = m_nand_mecc[3];
		nand_update_mecc(m_nand_mecc, m_nand_ecc_pos++, data);
		LOGMASKED(LOG_FLASH, "%s: NAND - MECC %03x - %02x %02x %02x %02x -> %02x %02x %02x %02x\n", machine().describe_context(), m_nand_ecc_pos - 1, temp[0], temp[1], temp[2], temp[3], m_nand_mecc[0], m_nand_mecc[1], m_nand_mecc[2], m_nand_mecc[3]);
		if (m_nand_ecc_pos == 2048)
			m_nand_ecc_pos = 0;
	}
	if (!BIT(m_nand_nfcont, 6))
	{
		temp[0] = m_nand_secc[0];
		temp[1] = m_nand_secc[1];
		nand_update_secc(m_nand_secc, m_nand_ecc_pos++, data);
		LOGMASKED(LOG_FLASH, "%s: NAND - SECC %02x - %02x %02x -> %02x %02x\n", machine().describe_context(), m_nand_ecc_pos - 1, temp[0], temp[1], m_nand_secc[0], m_nand_secc[1]);
		if (m_nand_ecc_pos == 16)
			m_nand_ecc_pos = 0;
	}
}

u8 s3c2410_device::nand_data_r()
{
	const bool update_ecc = m_nand_update_ecc_r_cb();
	const u8 data = m_nand_data_r_cb(0, 0xff);
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_FLASH, "%s: NAND read data %02X [%04X]\n", machine().describe_context(), data, m_nand_data_count);
		m_nand_data_count++;
		if (update_ecc)
			nand_update_ecc(data);
	}
	return data;
}

void s3c2410_device::nand_data_w(u8 data)
{
	LOGMASKED(LOG_FLASH, "%s: NAND write data %02X [%04X]\n", machine().describe_context(), data, m_nand_data_count);
	m_nand_data_count++;
	m_nand_data_w_cb(0, data, 0xff);
	nand_update_ecc(data);
}

void s3c2410_device::nand_init_ecc()
{
	LOGMASKED(LOG_FLASH, "NAND - init ecc\n");
	m_nand_mecc[0] = 0xff;
	m_nand_mecc[1] = 0xff;
	m_nand_mecc[2] = 0xff;
	m_nand_ecc_pos = 0;
}

void s3c2440_device::nand_init_ecc()
{
	s3c2410_device::nand_init_ecc();
	m_nand_mecc[3] = 0xff;
	m_nand_secc[0] = 0;
	m_nand_secc[1] = 0;
}

u32 s3c2410_device::nand_nfconf_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfconf;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFCONF: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::nand_nfconf_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfconf);

	LOGMASKED(LOG_FLASH, "%s: NAND write: NFCONF = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (data & (1 << 12))
		nand_init_ecc();
}

u32 s3c2410_device::nand_nfcmd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfcmd;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFCMD: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::nand_nfcmd_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfcmd);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFCMD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_nand_data_count = 0;
	m_nand_command_w_cb(0, data, 0xff);
}

u32 s3c2440_device::nand_nfcont_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfcont;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFCONT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfcont_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfcont);

	LOGMASKED(LOG_FLASH, "%s: NAND write: NFCONT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (data & (1 << 4))
		nand_init_ecc();
}

u32 s3c2410_device::nand_nfaddr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfaddr;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFADDR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::nand_nfaddr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfaddr);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_nand_data_count = 0;
	m_nand_address_w_cb(0, data, 0xff);
}

u32 s3c2410_device::nand_nfdata_r(offs_t offset, u32 mem_mask)
{
	const u32 data = nand_data_r();
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFDATA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

u32 s3c2440_device::nand_nfdata_r(offs_t offset, u32 mem_mask)
{
	u32 data = 0;
	if ((mem_mask & 0x000000ff) != 0) data |= m_nand_data_r_cb() <<  0;
	if ((mem_mask & 0x0000ff00) != 0) data |= m_nand_data_r_cb() <<  8;
	if ((mem_mask & 0x00ff0000) != 0) data |= m_nand_data_r_cb() << 16;
	if ((mem_mask & 0xff000000) != 0) data |= m_nand_data_r_cb() << 24;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFDATA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::nand_nfdata_w(offs_t offset, u32 data, u32 mem_mask)
{
	nand_data_w(data & 0xff);
}

void s3c2440_device::nand_nfdata_w(offs_t offset, u32 data, u32 mem_mask)
{
	if ((mem_mask & 0x000000ff) != 0) m_nand_data_w_cb((data >>  0) & 0xff);
	if ((mem_mask & 0x0000ff00) != 0) m_nand_data_w_cb((data >>  8) & 0xff);
	if ((mem_mask & 0x00ff0000) != 0) m_nand_data_w_cb((data >> 16) & 0xff);
	if ((mem_mask & 0xff000000) != 0) m_nand_data_w_cb((data >> 24) & 0xff);
}

u32 s3c2410_device::nand_nfstat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfstat;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFSTAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::nand_nfstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFSTAT (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	//m_nand_nfstat = ((m_nand_nfstat & ~mem_mask) | (data | mem_mask)) & ~0x03) (m_nand_nfstat & 0x03); // read-only bits
}

void s3c2440_device::nand_nfstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	s3c2410_device::nand_nfstat_w(offset, data, mem_mask);
	if (data & (1 << 2))
		m_nand_nfstat &= ~(1 << 2); // "RnB_TransDetect, to clear this value write 1"
}

u32 s3c2410_device::nand_nfecc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = ((m_nand_mecc[2] << 16) | (m_nand_mecc[1] << 8) | (m_nand_mecc[0] << 0));
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFECC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2410_device::nand_nfecc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfecc);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFECC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfmeccd0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfmeccd[0];
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFMECCD0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfmeccd0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfmeccd[0]);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFMECCD0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfmeccd1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfmeccd[1];
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFMECCD1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfmeccd1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfmeccd[1]);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFMECCD1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfseccd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfseccd;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFSECCD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfseccd_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfseccd);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFMECCD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfestat0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfestat[0] & ~0x000000f; // no main/spare ECC errors
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFESTAT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfestat0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfestat[0]);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFESTAT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfestat1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfestat[1] & ~0x000000f; // no main/spare ECC errors
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFESTAT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfestat1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfestat[1]);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFESTAT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfmecc0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_nand_mecc[3] << 24) | (m_nand_mecc[2] << 16) | (m_nand_mecc[1] << 8) | (m_nand_mecc[0] << 0);
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFMECC0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfmecc0_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfmecc[0]);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFMECC0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfmecc1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfmecc[1];
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFMECC1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfmecc1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfmecc[1]);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFMECC1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfsecc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_nand_secc[1] << 8) | (m_nand_secc[0] << 0);
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFSECC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfsecc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfsecc);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFSECC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfsblk_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfsblk;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFSBLK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfsblk_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfsblk);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFSBLK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::nand_nfeblk_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_nand_nfeblk;
	LOGMASKED(LOG_FLASH, "%s: NAND read: NFEBLK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::nand_nfeblk_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nand_nfeblk);
	LOGMASKED(LOG_FLASH, "%s: NAND write: NFEBLK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

void s3c2410_device::frnb_w(int state)
{
	LOGMASKED(LOG_FLASH, "frnb_w (%d)\n", state);
	if (state == 0)
		m_nand_nfstat &= ~(1 << 0);
	else
		m_nand_nfstat |= (1 << 0);
}

void s3c2440_device::frnb_w(int state)
{
	if (BIT(m_nand_nfstat, 0) == 0 && state != 0)
	{
		m_nand_nfstat |= (1 << 2);
		if (BIT(m_nand_nfcont, 9) != 0)
			request_irq(INT_NFCON);
	}
	s3c2410_device::frnb_w(state);
}

void s3c2410_device::get_nand_params(int ncon, int &out_page_size, int &out_address_cycle)
{
	out_page_size = 512;
	if (ncon == 0)
		out_address_cycle = 3; // byte-page-page
	else
		out_address_cycle = 4; // byte-page-page-page
}

void s3c2440_device::get_nand_params(int ncon, int &out_page_size, int &out_address_cycle)
{
	u32 port_g = m_port_r_cb(GPIO_PORT_G, 0xffffffff);
	if (ncon == 0)
	{
		if (BIT(port_g, 13) == 0)
		{
			out_page_size = 256;
			out_address_cycle = 3; // byte-page-page
		}
		else
		{
			out_page_size = 512;
			out_address_cycle = 4; // byte-page-page-page
		}
	}
	else
	{
		if (BIT(port_g, 13) == 0)
		{
			out_page_size = 1024;
			out_address_cycle = 4; // byte-byte-page-page or byte-page-page-page ??? assume latter
		}
		else
		{
			out_page_size = 2048;
			out_address_cycle = 5; // byte-byte-page-page-page
		}
	}
}

void s3c2410_device::nand_auto_boot()
{
	int om0 = m_om0_r_cb();
	int om1 = m_om1_r_cb();
	if (om0 == 0 && om1 == 0)
	{
		int ncon = m_ncon_r_cb();
		u8 *ptr = m_steppingstone;

		int page_size = 512;
		int address_cycle = 3;
		get_nand_params(ncon, page_size, address_cycle);

		m_nand_command_w_cb(0, 0xff, 0xff);
		for (int page = 0; page < (4 * 1024) / page_size; page++)
		{
			m_nand_command_w_cb(0, 0x00, 0xff);
			m_nand_address_w_cb(0, 0x00, 0xff);
			if (address_cycle > 4)
			{
				m_nand_address_w_cb(0, 0x00, 0xff);
			}
			m_nand_address_w_cb(0, (page >> 0) & 0xff, 0xff);
			m_nand_address_w_cb(0, (page >> 8) & 0xff, 0xff);
			if (address_cycle > 3)
			{
				m_nand_address_w_cb(0, (page >> 16) & 0xff, 0xff);
			}
			for (int i = 0; i < page_size; i++)
			{
				*ptr++ = m_nand_data_r_cb(0, 0xff);
			}
		}
		m_nand_command_w_cb(0, 0xff, 0xff);
	}
}

/* Camera Interface */

void s3c2440_device::cam_reset()
{
	m_cam_srcfmt = 0;
	m_cam_wdofst = 0;
	m_cam_gctrl = 0;
	m_cam_coysa1 = 0;
	m_cam_coysa2 = 0;
	m_cam_coysa3 = 0;
	m_cam_coysa4 = 0;
	m_cam_cocbsa1 = 0;
	m_cam_cocbsa2 = 0;
	m_cam_cocbsa3 = 0;
	m_cam_cocbsa4 = 0;
	m_cam_cocrsa1 = 0;
	m_cam_cocrsa2 = 0;
	m_cam_cocrsa3 = 0;
	m_cam_cocrsa4 = 0;
	m_cam_cotrgfmt = 0;
	m_cam_coctrl = 0;
	m_cam_coscpreratio = 0;
	m_cam_coscpredst = 0;
	m_cam_coscctrl = 0;
	m_cam_cotarea = 0;
	m_cam_costatus = 0;
	m_cam_prclrsa1 = 0;
	m_cam_prclrsa2 = 0;
	m_cam_prclrsa3 = 0;
	m_cam_prclrsa4 = 0;
	m_cam_prtrgfmt = 0;
	m_cam_prctrl = 0;
	m_cam_prscpreratio = 0;
	m_cam_prscpredst = 0;
	m_cam_prscctrl = 0;
	m_cam_prtarea = 0;
	m_cam_prstatus = 0;
	m_cam_imgcpt = 0;
}

u32 s3c2440_device::cam_srcfmt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_srcfmt;
	LOGMASKED(LOG_CAM, "%s: camera read: CISRCFMT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_srcfmt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_srcfmt);
	LOGMASKED(LOG_CAM, "%s: camera write: CISRCFMT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_wdofst_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_wdofst;
	LOGMASKED(LOG_CAM, "%s: camera read: CIWDOFST: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_wdofst_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_wdofst);
	LOGMASKED(LOG_CAM, "%s: camera write: CIWDOFST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_gctrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_gctrl;
	LOGMASKED(LOG_CAM, "%s: camera read: CIGCTRL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_gctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_gctrl);
	LOGMASKED(LOG_CAM, "%s: camera write: CIGCTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coysa1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coysa1;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOYSA1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coysa1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coysa1);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOYSA1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coysa2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coysa2;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOYSA2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coysa2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coysa2);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOYSA2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coysa3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coysa3;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOYSA3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coysa3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coysa3);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOYSA3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coysa4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coysa4;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOYSA4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coysa4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coysa4);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOYSA4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocbsa1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocbsa1;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCBSA1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocbsa1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocbsa1);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCBSA1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocbsa2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocbsa2;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCBSA2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocbsa2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocbsa2);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCBSA2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocbsa3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocbsa3;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCBSA3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocbsa3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocbsa3);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCBSA3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocbsa4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocbsa4;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCBSA4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocbsa4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocbsa4);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCBSA4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocrsa1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocrsa1;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCRSA1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocrsa1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocrsa1);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCRSA1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocrsa2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocrsa2;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCRSA2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocrsa2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocrsa2);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCRSA2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocrsa3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocrsa3;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCRSA3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocrsa3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocrsa3);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCRSA3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cocrsa4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cocrsa4;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCRSA4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cocrsa4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cocrsa4);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCRSA4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cotrgfmt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cotrgfmt;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOTRGFMT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cotrgfmt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cotrgfmt);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOTRGFMT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coctrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coctrl;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOCTRL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coctrl);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOCTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coscpreratio_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coscpreratio;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOSCPRERATIO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coscpreratio_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coscpreratio);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOSCPRERATIO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coscpredst_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coscpredst;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOSCPREDST: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coscpredst_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coscpredst);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOSCPREDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_coscctrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_coscctrl;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOSCCTRL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_coscctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_coscctrl);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOSCCTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_cotarea_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_cotarea;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOTAREA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_cotarea_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_cotarea);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOTAREA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_costatus_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_costatus;
	LOGMASKED(LOG_CAM, "%s: camera read: CICOSTATUS: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_costatus_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_costatus);
	LOGMASKED(LOG_CAM, "%s: camera write: CICOSTATUS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prclrsa1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prclrsa1;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRCLRSA1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prclrsa1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prclrsa1);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRCLRSA1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prclrsa2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prclrsa2;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRCLRSA2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prclrsa2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prclrsa2);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRCLRSA2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prclrsa3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prclrsa3;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRCLRSA3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prclrsa3_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prclrsa3);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRCLRSA3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prclrsa4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prclrsa4;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRCLRSA4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prclrsa4_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prclrsa4);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRCLRSA4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prtrgfmt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prtrgfmt;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRTRGFMT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prtrgfmt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prtrgfmt);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRTRGFMT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prctrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prctrl;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRCTRL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prctrl);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRCTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prscpreratio_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prscpreratio;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRSCPRERATIO: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prscpreratio_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prscpreratio);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRSCPRERATIO = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prscpredst_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prscpredst;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRSCPREDST: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prscpredst_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prscpredst);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRSCPREDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prscctrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prscctrl;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRSCCTRL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prscctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prscctrl);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRSCCTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prtarea_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prtarea;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRTAREA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prtarea_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prtarea);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRTAREA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_prstatus_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_prstatus;
	LOGMASKED(LOG_CAM, "%s: camera read: CIPRSTATUS: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_prstatus_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_prstatus);
	LOGMASKED(LOG_CAM, "%s: camera write: CIPRSTATUS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::cam_imgcpt_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_cam_imgcpt;
	LOGMASKED(LOG_CAM, "%s: camera read: CIIMGCPT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::cam_imgcpt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cam_imgcpt);
	LOGMASKED(LOG_CAM, "%s: camera write: CIIMGCPT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

/* AC'97 Interface */

void s3c2440_device::ac97_reset()
{
	m_ac97_glbctrl = 0;
	m_ac97_glbstat = 0;
	m_ac97_codec_cmd = 0;
	m_ac97_codec_stat = 0;
	m_ac97_pcmaddr = 0;
	m_ac97_micaddr = 0;
	m_ac97_pcmdata = 0;
	m_ac97_micdata = 0;
}

u32 s3c2440_device::ac97_glbctrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_glbctrl;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_GLBCTRL: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_glbctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_glbctrl);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_GLBCTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::ac97_glbstat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_glbstat;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_GLBSTAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_glbstat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_glbstat);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_GLBSTAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::ac97_codec_cmd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_codec_cmd;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_CODEC_CMD: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_codec_cmd_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_codec_cmd);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_CODEC_CMD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::ac97_codec_stat_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_codec_stat;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_CODEC_STAT: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_codec_stat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_codec_stat);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_CODEC_STAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::ac97_pcmaddr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_pcmaddr;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_PCMADDR: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_pcmaddr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_pcmaddr);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_PCMADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::ac97_micaddr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_micaddr;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_micaddr: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_micaddr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_micaddr);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_MICADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::ac97_pcmdata_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_pcmdata;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_PCMDATA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_pcmdata_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_pcmdata);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_PCMDATA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 s3c2440_device::ac97_micdata_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ac97_micdata;
	LOGMASKED(LOG_AC97, "%s: AC97 read: AC_MICDATA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void s3c2440_device::ac97_micdata_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ac97_micdata);
	LOGMASKED(LOG_AC97, "%s: AC97 write: AC_MICDATA = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}
