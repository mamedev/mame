// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Ingenic JZ4755 SoC on-chip peripherals

    The XBurst CPU core itself is a separate device (see cpu/mips/mips3.h);
    this device supplies the APB and AHB peripheral blocks behind it:
    clock/power manager, interrupt controller, timers, RTC, GPIO, UARTs,
    external memory controller with its NAND interface, BCH ECC engine,
    DMA controller, audio interface with the internal codec (playback
    only; it is a sound device), IPU scaler, LCD/TV controller, and
    register-storage stubs for the rest.

***************************************************************************/

#ifndef MAME_MACHINE_JZ4755_H
#define MAME_MACHINE_JZ4755_H

#pragma once

#include "machine/nandflash.h"
#include "screen.h"

#include <deque>
#include <string>
#include <vector>

class jz4755_device : public device_t, public device_sound_interface
{
public:
	// GPIO ports
	enum : unsigned
	{
		GPIO_A = 0, GPIO_B, GPIO_C, GPIO_D, GPIO_E, GPIO_F,
		GPIO_PORTS
	};

	// interrupt controller sources (ICSR/ICMR/ICPR bit numbers)
	enum : unsigned
	{
		IRQ_ETH   = 0,
		IRQ_SFT   = 1,
		IRQ_I2C   = 2,
		IRQ_RTC   = 3,
		IRQ_UART2 = 7,
		IRQ_UART1 = 8,
		IRQ_UART0 = 9,
		IRQ_AIC   = 10,
		IRQ_GPIO5 = 11,
		IRQ_GPIO4 = 12,
		IRQ_GPIO3 = 13,
		IRQ_GPIO2 = 14,
		IRQ_GPIO1 = 15,
		IRQ_GPIO0 = 16,
		IRQ_BCH   = 17,
		IRQ_SADC  = 18,
		IRQ_CIM   = 19,
		IRQ_TSSI  = 20,
		IRQ_TCU2  = 21,
		IRQ_TCU1  = 22,
		IRQ_TCU0  = 23,
		IRQ_MSC1  = 24,
		IRQ_MSC0  = 25,
		IRQ_SSI   = 26,
		IRQ_UDC   = 27,
		IRQ_DMA1  = 28,
		IRQ_DMA0  = 29,
		IRQ_IPU   = 30,
		IRQ_LCD   = 31
	};

	// the clock is EXCLK, the external oscillator
	jz4755_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_nand_tag(T &&tag) { m_nand.set_tag(std::forward<T>(tag)); }

	template <unsigned N> auto uart_tx() { return m_uart_tx[N].bind(); }
	// driven GPIO output levels per port, with pins configured as inputs reported high
	template <unsigned N> auto gpio_out() { return m_gpio_out[N].bind(); }

	// the GPIO pin the NAND ready/busy line is wired to
	void set_nand_rb(unsigned port, unsigned bit) { m_nand_rb_port = port; m_nand_rb_bit = bit; }

	void uart_rx(unsigned n, uint8_t data);
	// external GPIO input levels (inputs default high)
	void gpio_input(unsigned port, unsigned bit, int state);
	void gpio_input_mask(unsigned port, uint32_t mask, uint32_t state);

	// 0x10000000-0x13ffffff: APB and AHB peripheral registers
	void map(address_map &map) ATTR_COLD;
	// static memory bank 1: NAND flash data/command/address ports
	void nand_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	// clock tree
	uint32_t pll_clock() const;
	uint32_t cpu_clock() const;
	uint32_t pclk() const;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned TCU_CHANNELS = 6;
	static constexpr unsigned UARTS = 3;

	struct uart_t
	{
		std::deque<uint8_t> rx;
		std::string line;
		uint8_t dll, dlh, ier, fcr, lcr, mcr, spr, isr, umr, uacr;
		bool thre_pending;
	};

	struct tcu_channel_t
	{
		uint16_t tdfr, tdhr, tcsr;
		uint32_t tcnt;      // count at base_time
		attotime base_time;
		emu_timer *timer;
	};

	// clock and power manager
	uint32_t cpm_r(offs_t offset, uint32_t mem_mask);
	void cpm_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// interrupt controller
	uint32_t intc_r(offs_t offset, uint32_t mem_mask);
	void intc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void set_irq(unsigned source, bool state);
	void update_irq();

	// timer/counter unit, OS timer and watchdog
	uint32_t tcu_r(offs_t offset, uint32_t mem_mask);
	void tcu_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t timer_clock(uint16_t tcsr) const;
	uint32_t tcu_count(unsigned ch) const;
	void tcu_restart(unsigned ch);
	void tcu_reschedule(unsigned ch);
	void tcu_update_irqs();
	TIMER_CALLBACK_MEMBER(tcu_tick);
	uint32_t ost_count() const;
	void ost_restart();
	void ost_reschedule();
	TIMER_CALLBACK_MEMBER(ost_tick);
	void wdt_reschedule();
	TIMER_CALLBACK_MEMBER(wdt_tick);

	// real time clock
	uint32_t rtc_r(offs_t offset, uint32_t mem_mask);
	void rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t rtc_seconds() const;

	// general purpose I/O
	uint32_t gpio_r(offs_t offset, uint32_t mem_mask);
	void gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpio_pins(unsigned port);
	void gpio_update(unsigned port);
	void gpio_output_changed(unsigned port);
	TIMER_CALLBACK_MEMBER(nand_ready);

	// UARTs
	template <unsigned N> uint32_t uart_r(offs_t offset, uint32_t mem_mask);
	template <unsigned N> void uart_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t uart_read(unsigned n, offs_t offset);
	void uart_write(unsigned n, offs_t offset, uint8_t data);
	void uart_update_irq(unsigned n);

	// external memory controller and NAND interface
	uint32_t emc_r(offs_t offset, uint32_t mem_mask);
	void emc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t nand_r(offs_t offset, uint32_t mem_mask);
	void nand_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// BCH error correction engine
	uint32_t bch_r(offs_t offset, uint32_t mem_mask);
	void bch_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void bch_data_w(uint8_t data);
	void bch_reset();
	void bch_encode();
	void bch_decode();
	static void bch_init_tables();
	static uint16_t gf_mul(uint16_t a, uint16_t b);

	// LCD controller with TV encoder
	uint32_t lcd_r(offs_t offset, uint32_t mem_mask);
	void lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void lcd_update_irq();
	void draw_foreground(bitmap_rgb32 &bitmap, const rectangle &cliprect, unsigned fg);

	// audio interface and internal codec
	uint32_t aic_r(offs_t offset, uint32_t mem_mask);
	void aic_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void aic_reset();
	void aic_set_rate();
	void aic_tx_push(uint32_t data);
	bool aic_tx_service() const;
	int16_t aic_tx_sample(uint32_t raw) const;
	void aic_update();
	TIMER_CALLBACK_MEMBER(aic_tick);

	// register-storage stubs
	template <unsigned N> uint32_t msc_r(offs_t offset, uint32_t mem_mask);
	template <unsigned N> void msc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t i2c_r(offs_t offset, uint32_t mem_mask);
	void i2c_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sadc_r(offs_t offset, uint32_t mem_mask);
	void sadc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t ipu_r(offs_t offset, uint32_t mem_mask);
	void ipu_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void ipu_run();
	TIMER_CALLBACK_MEMBER(ipu_done);
	// DMA controller
	uint32_t dmac_r(offs_t offset, uint32_t mem_mask);
	void dmac_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void dma_start(unsigned ch);
	void dma_run(unsigned ch);
	void dma_service(unsigned drt);
	bool dma_load_descriptor(unsigned ch);
	bool dma_transfer(unsigned ch);
	void dma_unit(unsigned ch);
	void dma_sync_aic();
	void dma_update_irq();
	template <unsigned N> uint32_t stub_r(offs_t offset, uint32_t mem_mask);
	template <unsigned N> void stub_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t read_phys(uint32_t address);

	required_device<cpu_device> m_cpu;
	required_device<nand_device> m_nand;
	devcb_write8::array<UARTS> m_uart_tx;
	devcb_write32::array<GPIO_PORTS> m_gpio_out;

	// CPM
	uint32_t m_cpm[0x40];

	// INTC
	uint32_t m_icsr, m_icmr;

	// TCU/OST/WDT
	tcu_channel_t m_tcu[TCU_CHANNELS];
	uint32_t m_ter, m_tfr, m_tmr, m_tsr;
	uint32_t m_ostdr, m_ostcnt, m_ostcsr;
	attotime m_ost_base;
	emu_timer *m_ost_timer;
	uint16_t m_wdt_tdr, m_wdt_tcer, m_wdt_tcnt, m_wdt_tcsr;
	attotime m_wdt_base;
	emu_timer *m_wdt_timer;

	// RTC
	uint32_t m_rtc[0x10];
	attotime m_rtc_base;

	// GPIO
	struct gpio_port_t
	{
		uint32_t dat, im, pe, fun, sel, dir, trg, flg;
		uint32_t ext;       // external input levels
		uint32_t prev;      // levels at the last interrupt evaluation
		uint32_t out;       // driven levels last reported through the output callback
	};
	gpio_port_t m_gpio[GPIO_PORTS];
	unsigned m_nand_rb_port, m_nand_rb_bit;
	emu_timer *m_nand_timer;

	// UART
	uart_t m_uart[UARTS];

	// EMC
	uint32_t m_emc[0x40];
	uint32_t m_sdmr;

	// BCH
	uint32_t m_bhcr, m_bhcnt, m_bhint, m_bhinte;
	uint32_t m_bhpar[4], m_bherr[4];
	uint32_t m_bch_count;
	bool m_bch_all_ff, m_bch_all_00;
	std::vector<uint8_t> m_bch_buf;

	// GF(2^13) tables and the generator polynomials for 4- and 8-bit correction, shared by all instances
	static constexpr unsigned GF_ORDER = 8191;
	static uint16_t s_gf_exp[2 * GF_ORDER];
	static uint16_t s_gf_log[GF_ORDER + 1];
	static uint64_t s_bch_gen[2][2];    // [t == 8][low/high 64 bits], the x^(13t) term left out
	static bool s_bch_tables_ready;

	// LCDC
	uint32_t m_lcd[0x80];
	uint32_t m_lcd_desc[2][8];

	// AIC: the transmit FIFO feeds the sound stream at the codec's DAC rate
	static constexpr unsigned AIC_FIFO_DEPTH = 32;
	sound_stream *m_stream;
	uint32_t m_aic[0x40];
	uint8_t m_codec[0x80];
	uint32_t m_aic_txfifo[AIC_FIFO_DEPTH];
	uint8_t m_aic_txrd, m_aic_txlevel;
	int16_t m_aic_out[2];       // the samples last sent to the DAC
	emu_timer *m_aic_timer;     // fires when the DMA burst in flight will have drained

	// stubs
	uint32_t m_msc[2][0x20];
	uint32_t m_i2c[4];
	bool m_i2c_reading, m_i2c_address_phase;
	uint32_t m_sadc[0x10];
	uint32_t m_stub[8][0x100];

	// IPU: the last frame streamed to the LCD controller, as 0x00RRGGBB
	uint32_t m_ipu[0x40];
	std::vector<uint32_t> m_ipu_frame;
	uint32_t m_ipu_frame_w, m_ipu_frame_h;
	emu_timer *m_ipu_timer;
	bool m_ipu_frame_pending;    // a frame handed to the LCD controller, completed at the next vblank

	// DMAC: two groups of six channels
	static constexpr unsigned DMA_CHANNELS = 12;
	struct dma_channel_t
	{
		uint32_t dsa, dta, dtc, drt, dcs, dcm, dda, dsd;
		bool active;        // a transfer is under way, possibly waiting for its peripheral
	};
	dma_channel_t m_dma[DMA_CHANNELS];
	uint32_t m_dmac_ctrl[2], m_dirqp[2], m_ddr[2], m_dcke[2];
};

DECLARE_DEVICE_TYPE(JZ4755, jz4755_device)

#endif // MAME_MACHINE_JZ4755_H
