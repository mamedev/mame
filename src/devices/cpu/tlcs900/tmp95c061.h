// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_CPU_TLCS900_TMP95C061_H
#define MAME_CPU_TLCS900_TMP95C061_H

#pragma once

#include "tlcs900.h"

DECLARE_DEVICE_TYPE(TMP95C061, tmp95c061_device)


class tmp95c061_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c061_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto port1_read()  { return m_port1_read.bind(); }
	auto port1_write() { return m_port1_write.bind(); }
	auto port2_write() { return m_port2_write.bind(); }
	auto port5_read()  { return m_port5_read.bind(); }
	auto port5_write() { return m_port5_write.bind(); }
	auto port6_write() { return m_port6_write.bind(); }
	auto port7_read()  { return m_port7_read.bind(); }
	auto port7_write() { return m_port7_write.bind(); }
	auto port8_read()  { return m_port8_read.bind(); }
	auto port8_write() { return m_port8_write.bind(); }
	auto port9_read()  { return m_port9_read.bind(); }
	auto porta_read()  { return m_porta_read.bind(); }
	auto porta_write() { return m_porta_write.bind(); }
	auto portb_read()  { return m_portb_read.bind(); }
	auto portb_write() { return m_portb_write.bind(); }

protected:
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

	void tlcs900_change_tff( int which, int change );
	int tlcs900_process_hdma( int channel );
	void update_porta();

private:
	uint8_t internal_r(offs_t offset);
	void internal_w(offs_t offset, uint8_t data);

	void internal_mem(address_map &map);

	uint8_t   m_to1;
	uint8_t   m_to3;

	// Port 1: 8 bit I/O. Shared with D8-D15
	devcb_read8    m_port1_read;
	devcb_write8   m_port1_write;

	// Port 2: 8 bit output only. Shared with A16-A23
	devcb_write8   m_port2_write;

	// Port 5: 4 bit I/O. Shared with HWR, BUSRQ, BUSAK, RW
	devcb_read8    m_port5_read;
	devcb_write8   m_port5_write;

	// Port 6: 6 bit I/O. Shared with CS0, CS1, CS3/LCAS, RAS, REFOUT
	devcb_read8    m_port6_read;
	devcb_write8   m_port6_write;

	// Port 7: 8 bit I/O. Shared with PG0-OUT, PG1-OUT
	devcb_read8    m_port7_read;
	devcb_write8   m_port7_write;

	// Port 8: 6 bit I/O. Shared with TXD0, TXD1, RXD0, RXD1, CTS0, SCLK0, SCLK1
	devcb_read8    m_port8_read;
	devcb_write8   m_port8_write;

	// Port 9: 4 bit input only. Shared with AN0-AN3
	devcb_read8    m_port9_read;

	// Port A: 4 bit I/O. Shared with WAIT, TI0, TO1, TO2
	devcb_read8    m_porta_read;
	devcb_write8   m_porta_write;

	// Port B: 8 bit I/O. Shared with TI4/INT4, TI5/INT5, TI6/INT6, TI7/INT7, TO4, TO5, TO6
	devcb_read8    m_portb_read;
	devcb_write8   m_portb_write;
};

#endif // MAME_CPU_TLCS900_TMP95C061_H
