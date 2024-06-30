// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_F2MC16_MB9061X_H
#define MAME_CPU_F2MC16_MB9061X_H

#pragma once

#include "f2mc16.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb9061x_device

class mb9061x_device :  public f2mc16_device
{
	friend class mb90610_device;
	friend class mb90611_device;
	friend class mb90641_device;

public:
	const address_space_config m_program_config;

	// port registers
	enum
	{
		PDR1 = 0, PDR2, PDR3, PDR4, PDR5, PDR6, PDR7, PDR8, PDR9, PDRA
	};

	// interrupts handled by the interrupt controller
	enum
	{
		ICR0 = 0, ICR1, ICR2, ICR3, ICR4, ICR5, ICR6, ICR7, ICR8, ICR9, ICR10, ICR11, ICR12, ICR13, ICR14, ICR15
	};

	auto read_pdr1()  { return m_read_port [PDR1].bind(); }
	auto write_pdr1() { return m_write_port[PDR1].bind(); }
	auto read_pdr2()  { return m_read_port [PDR2].bind(); }
	auto write_pdr2() { return m_write_port[PDR2].bind(); }
	auto read_pdr3()  { return m_read_port [PDR3].bind(); }
	auto write_pdr3() { return m_write_port[PDR3].bind(); }
	auto read_pdr4()  { return m_read_port [PDR4].bind(); }
	auto write_pdr4() { return m_write_port[PDR4].bind(); }
	auto read_pdr5()  { return m_read_port [PDR5].bind(); }
	auto write_pdr5() { return m_write_port[PDR5].bind(); }
	auto read_pdr6()  { return m_read_port [PDR6].bind(); }
	auto write_pdr6() { return m_write_port[PDR6].bind(); }
	auto read_pdr7()  { return m_read_port [PDR7].bind(); }
	auto write_pdr7() { return m_write_port[PDR7].bind(); }
	auto read_pdr8()  { return m_read_port [PDR8].bind(); }
	auto write_pdr8() { return m_write_port[PDR8].bind(); }
	auto read_pdr9()  { return m_read_port [PDR9].bind(); }
	auto write_pdr9() { return m_write_port[PDR9].bind(); }
	auto read_pdra()  { return m_read_port [PDRA].bind(); }
	auto write_pdra() { return m_write_port[PDRA].bind(); }
	auto read_adcs()  { return m_read_adcs .bind(); }
	auto write_adcs() { return m_write_adcs.bind(); }
	auto read_adcr()  { return m_read_adcr .bind(); }
	auto write_adcr() { return m_write_adcr.bind(); }

	// port functions
	u8 read_port(offs_t offset) { return m_ports[offset]; }
	void write_port(offs_t offset, u8 data) { m_ports[offset] = data; }
	u8 port_r(offs_t offset) { return m_read_port[offset](); }
	void port_w(offs_t offset, u8 data) { write_port(offset, data); m_write_port[offset](0, data); }

	// analog/digital control status functions
	u16 read_reg_adcs() { return m_adcs; }
	void write_reg_adcs(offs_t offset, u8 data) { m_adcs = (offset == 0) ? (data | (m_adcs & 0xff00)) : ((data << 8) | (m_adcs & 0x00ff)); }
	u8 adcs_r(offs_t offset) { return (offset == 0 ? m_read_adcs() : m_read_adcs() >> 8) & 0xff; }
	void adcs_w(offs_t offset, u8 data) { write_reg_adcs(offset, data); m_write_adcs(offset, data); }

	// analog/digital data functions
	u16 read_reg_adcr() { return m_adcr; }
	void write_reg_adcr(offs_t offset, u8 data) { m_adcr = (offset == 0) ? (data | (m_adcr & 0xff00)) : ((data << 8) | (m_adcr & 0x00ff)); }
	u8 adcr_r(offs_t offset) { return (offset == 0 ? m_read_adcr() : m_read_adcr() >> 8) & 0xff; }
	void adcr_w(offs_t offset, u8 data) { write_reg_adcr(offset, data); m_write_adcr(offset, data); }

	// timer external counter tick functions
	void tin0_w(int state);
	void tin1_w(int state);

protected:
	// construction/destruction
	mb9061x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;

private:
	static inline constexpr u8 PORT_COUNT = 10;

	// TBC
	TIMER_CALLBACK_MEMBER(tbtc_tick);
	u8 tbtc_r();
	void tbtc_w(u8 data);

	// INTC
	u8 intc_r(offs_t offset);
	void intc_w(offs_t offset, u8 data);
	void intc_trigger_irq(int icr, int vector);
	void intc_clear_irq(int icr, int vector);

	// TIMERS
	TIMER_CALLBACK_MEMBER(timer0_tick);
	TIMER_CALLBACK_MEMBER(timer1_tick);
	u8 timer_r(offs_t offset);
	void timer_w(offs_t offset, u8 data);
	void recalc_timer(int tnum);
	void tin_common(int timer, int base, int state);

	// PORTS
	// TODO: Control input and output values with direction registers
	u8 m_ports[PORT_COUNT];
	devcb_read8::array<PORT_COUNT> m_read_port;
	devcb_write8::array<PORT_COUNT> m_write_port;

	// AD
	u16 m_adcs;
	u16 m_adcr;
	devcb_read16 m_read_adcs;
	devcb_write8 m_write_adcs;
	devcb_read16 m_read_adcr;
	devcb_write8 m_write_adcr;

	u8 m_timer_regs[8];
	u32 m_timer_hz[2];
	emu_timer *m_timer[2];
	int m_event_state[2];
	u16 m_event_count[2];

	u8 m_tbtc;
	emu_timer *m_tbtc_timer;

	u8 m_intc[0x10];
};

class mb90610_device : public mb9061x_device
{
public:
	mb90610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90610_map(address_map &map);
protected:
	mb90610_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class mb90611_device : public mb9061x_device
{
public:
	mb90611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90611_map(address_map &map);
protected:
	mb90611_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class mb90641_device : public mb9061x_device
{
public:
	mb90641_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90641_map(address_map &map);
protected:
	mb90641_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MB90610A, mb90610_device)
DECLARE_DEVICE_TYPE(MB90611A, mb90611_device)
DECLARE_DEVICE_TYPE(MB90641A, mb90641_device)

#endif // MAME_CPU_F2MC16_MB9061X_H
