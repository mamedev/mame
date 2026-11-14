// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_DSP563XX_SHI_H
#define MAME_CPU_DSP563XX_SHI_H

#pragma once

// forward declaration
class dsp563xx_device;

class dsp5636x_shi_device : public device_t
{
public:
	// device type constructor
	dsp5636x_shi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// I/O register map
	void map(address_map &map);

	// SPI slave signal interface
	void ss_w(int state);
	void sck_w(int state);
	void mosi_w(int state);
	auto miso_callback() { return m_miso_cb.bind(); }
	auto hreq_callback() { return m_hreq_cb.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// I/O register handlers
	u32 hckr_r();
	void hckr_w(u32 data);
	u32 hcsr_r();
	void hcsr_w(u32 data);
	u32 hsar_r();
	void hsar_w(u32 data);
	void htx_w(u32 data);
	u32 hrx_r();

	// HCSR bits
	enum {
		HCSR_HEN   = 0,
		HCSR_HI2C  = 1,
		HCSR_HM0   = 2,
		HCSR_HM1   = 3,
		HCSR_HCKFR = 4,
		HCSR_HFIFO = 5,
		HCSR_HMST  = 6,
		HCSR_HRQE0 = 7,
		HCSR_HRQE1 = 8,
		HCSR_HIDLE = 9,
		HCSR_HBIE  = 10,
		HCSR_HTIE  = 11,
		HCSR_HRIE0 = 12,
		HCSR_HRIE1 = 13,
		HCSR_HTUE  = 14,
		HCSR_HTDE  = 15,
		HCSR_HRNE  = 17,
		HCSR_HRFF  = 19,
		HCSR_HROE  = 20,
		HCSR_HBER  = 21,
		HCSR_HBUSY = 22,
	};

	// HCKR bits
	enum {
		HCKR_CPHA = 0,
		HCKR_CPOL = 1,
	};

	// SPI slave state
	bool spi_active() const;
	int spi_word_bits() const;
	int fifo_depth() const;
	void spi_rx_bit();
	void spi_tx_bit();
	void spi_rx_word_done();
	void update_status();
	void update_hreq();

	// reference to CPU
	required_device<dsp563xx_device> m_cpu;

	// registers
	u32 m_hsar;
	u32 m_hckr;
	u32 m_hcsr;
	u32 m_htx;

	// SPI slave state
	bool m_ss;
	bool m_sck;
	bool m_mosi;
	bool m_miso;
	bool m_hreq;
	u32 m_rx_shift;
	int m_rx_bits;
	u32 m_iosr;
	int m_tx_bits;
	u32 m_hrx[10];
	int m_hrx_head;
	int m_hrx_tail;
	int m_hrx_count;

	devcb_write_line m_miso_cb;
	devcb_write_line m_hreq_cb;
};

// device type declaration
DECLARE_DEVICE_TYPE(DSP5636X_SHI, dsp5636x_shi_device)

#endif // MAME_CPU_DSP563XX_SHI_H
