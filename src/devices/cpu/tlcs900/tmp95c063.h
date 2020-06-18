// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_CPU_TLCS900_TMP95C063_H
#define MAME_CPU_TLCS900_TMP95C063_H

#pragma once

#include "tlcs900.h"

DECLARE_DEVICE_TYPE(TMP95C063, tmp95c063_device)


class tmp95c063_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto port1_read()  { return m_port1_read.bind(); }
	auto port1_write() { return m_port1_write.bind(); }
	auto port2_write() { return m_port2_write.bind(); }
	auto port5_read()  { return m_port5_read.bind(); }
	auto port5_write() { return m_port5_write.bind(); }
	auto port6_read()  { return m_port6_read.bind(); }
	auto port6_write() { return m_port6_write.bind(); }
	auto port7_read()  { return m_port7_read.bind(); }
	auto port7_write() { return m_port7_write.bind(); }
	auto port8_read()  { return m_port8_read.bind(); }
	auto port8_write() { return m_port8_write.bind(); }
	auto port9_read()  { return m_port9_read.bind(); }
	auto port9_write() { return m_port9_write.bind(); }
	auto porta_read()  { return m_porta_read.bind(); }
	auto porta_write() { return m_porta_write.bind(); }
	auto portb_read()  { return m_portb_read.bind(); }
	auto portb_write() { return m_portb_write.bind(); }
	auto portc_read()  { return m_portc_read.bind(); }
	auto portd_read()  { return m_portd_read.bind(); }
	auto portd_write() { return m_portd_write.bind(); }
	auto porte_read()  { return m_porte_read.bind(); }
	auto porte_write() { return m_porte_write.bind(); }
	template <size_t Bit> auto an_read() { return m_an_read[Bit].bind(); }

protected:
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

private:
	uint8_t internal_r(offs_t offset);
	void internal_w(offs_t offset, uint8_t data);

	void internal_mem(address_map &map);

	// Port 1: 8 bit I/O. Shared with d8-d15
	devcb_read8    m_port1_read;
	devcb_write8   m_port1_write;

	// Port 2: 8 bit output only. Shared with a16-a23
	devcb_write8   m_port2_write;

	// Port 5: 6 bit I/O
	devcb_read8    m_port5_read;
	devcb_write8   m_port5_write;

	// Port 6: 8 bit I/O. Shared with cs1, cs3 & dram control
	devcb_read8    m_port6_read;
	devcb_write8   m_port6_write;

	// Port 7: 8 bit I/O
	devcb_read8    m_port7_read;
	devcb_write8   m_port7_write;

	// Port 8: 8 bit I/O. Shared with SCOUT, WAIT, NMI2, INT0-INT3
	devcb_read8    m_port8_read;
	devcb_write8   m_port8_write;

	// Port 9: 8 bit I/O. Shared with clock input and output for the 8-bit timers
	devcb_read8    m_port9_read;
	devcb_write8   m_port9_write;

	// Port A: 8 bit I/O. Shared with serial channels 0/1
	devcb_read8    m_porta_read;
	devcb_write8   m_porta_write;

	// Port B: 8 bit I/O. Shared with 16bit timers
	devcb_read8    m_portb_read;
	devcb_write8   m_portb_write;

	// Port C: 8 bit input only. Shared with analogue inputs
	devcb_read8    m_portc_read;

	// Port D: 5 bit I/O. Shared with int8_t
	devcb_read8    m_portd_read;
	devcb_write8   m_portd_write;

	// Port E: 8 bit I/O.
	devcb_read8    m_porte_read;
	devcb_write8   m_porte_write;

	// analogue inputs, sampled at 10 bits
	devcb_read16::array<8> m_an_read;
};

#endif // MAME_CPU_TLCS900_TMP95C063_H
