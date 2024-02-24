// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#ifndef MAME_CPU_SH_SH7042_H
#define MAME_CPU_SH_SH7042_H

#pragma once

#include "sh2.h"

class sh7042_device : public sh2_device
{
public:
	sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template<int Port> auto read_adc() { return m_read_adc[Port].bind(); }
	template<int Sci> void sci_rx_w(int state) { do_sci_w(Sci, state); }
	template<int Sci> auto write_sci_tx() { return m_sci_tx[Sci].bind(); }


protected:
	enum {
		CSR_ADF  = 0x80,
		CSR_ADIE = 0x40,
		CSR_ADST = 0x20,
		CSR_CKS  = 0x10,
		CSR_GRP  = 0x08,
		CSR_CHAN = 0x07
	};

	enum {
		CR_PWR   = 0x40,
		CR_TRGS  = 0x30,
		CR_SCAN  = 0x08,
		CR_SIM   = 0x04,
		CR_BUF   = 0x03
	};

	sh7042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read16::array<8> m_read_adc;
	devcb_write_line::array<2> m_sci_tx;

	void map(address_map &map);

	// ADC section
	uint16_t m_addr[8];
	uint8_t m_adcsr, m_adcr;

	u16 adc_default(int adc);
	u16 addr_r(offs_t offset);
	u8 adcsr_r();
	u8 adcr_r();
	void adcsr_w(u8 data);
	void adcr_w(u8 data);

	void do_sci_w(int sci, int state);
};

class sh7043_device : public sh7042_device
{
public:
	sh7043_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(SH7042, sh7042_device)
DECLARE_DEVICE_TYPE(SH7043, sh7043_device)

#endif // MAME_CPU_SH_SH7042_H
