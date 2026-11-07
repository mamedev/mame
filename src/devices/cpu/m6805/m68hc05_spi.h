// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    Motorola M68HC05 serial peripheral interface (SPI)

    The synchronous SPI of the MC68HC05C4/C8 and relatives: SPCR, SPSR and
    SPDR at $0A-$0C of the host MCU.  The master bit rate is derived from
    the MCU's internal processor clock (E, half the oscillator frequency),
    which is this device's clock.

***************************************************************************/

#ifndef MAME_CPU_M6805_M68HC05_SPI_H
#define MAME_CPU_M6805_M68HC05_SPI_H

#pragma once


class m68hc05_spi_device : public device_t
{
public:
	m68hc05_spi_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// SCK and MOSI output pins (master mode), MISO input pin, and the interrupt
	// request towards the host MCU (level)
	auto sck_cb() { return m_sck_cb.bind(); }
	auto mosi_cb() { return m_mosi_cb.bind(); }
	auto miso_cb() { return m_miso_cb.bind(); }
	auto irq_cb() { return m_irq_cb.bind(); }

	// SS input pin (PD5), active low
	void ss_w(int state);

	void map(address_map &map) ATTR_COLD;

	// register access for the host's debugger state (no side effects)
	u8 spcr() const { return m_spcr; }
	u8 spsr() const { return m_spsr; }
	u8 spdr() const { return m_rdr; }
	void set_spcr(u8 data);
	void set_spsr(u8 data);
	void set_spdr(u8 data) { m_rdr = data; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 spcr_r();
	void spcr_w(u8 data);
	u8 spsr_r();
	u8 spdr_r();
	void spdr_w(u8 data);

	TIMER_CALLBACK_MEMBER(tick);

	bool spe() const { return BIT(m_spcr, 6); }
	bool mstr() const { return BIT(m_spcr, 4); }
	bool cpol() const { return BIT(m_spcr, 3); }
	bool cpha() const { return BIT(m_spcr, 2); }

	void set_flags(u8 mask);
	void clear_flags(u8 mask);
	void update_irq();
	void update_sck();
	void check_modf();
	void abort_transfer();
	void write_sck(int state);

	devcb_write_line    m_sck_cb;
	devcb_write_line    m_mosi_cb;
	devcb_read_line     m_miso_cb;
	devcb_write_line    m_irq_cb;
	emu_timer           *m_timer;

	u8                  m_spcr;
	u8                  m_spsr, m_spsr_seen;
	u8                  m_rdr;              // receive buffer, read through SPDR
	u8                  m_out;              // byte being shifted out
	u8                  m_in;               // byte being shifted in
	u8                  m_tick;             // half SCK periods elapsed in the transfer
	bool                m_busy;
	bool                m_ss;
	bool                m_irq_state;
	int                 m_sck;              // SCK level last driven, -1 if never
};

DECLARE_DEVICE_TYPE(M68HC05_SPI, m68hc05_spi_device)

#endif // MAME_CPU_M6805_M68HC05_SPI_H
