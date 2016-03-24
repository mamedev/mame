// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#pragma once

#ifndef __M5074X_H__
#define __M5074X_H__

#include "m740.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// internal ROM region
#define M5074X_INTERNAL_ROM_REGION "internal"
#define M5074X_INTERNAL_ROM(_tag) (_tag ":" M5074X_INTERNAL_ROM_REGION)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M5074X_PORT0_READ_CALLBACK(_read) \
	devcb = &m5074x_device::set_p0_rd_callback(*device, DEVCB_##_read);

#define MCFG_M5074X_PORT1_READ_CALLBACK(_read) \
	devcb = &m5074x_device::set_p1_rd_callback(*device, DEVCB_##_read);

#define MCFG_M5074X_PORT2_READ_CALLBACK(_read) \
	devcb = &m5074x_device::set_p2_rd_callback(*device, DEVCB_##_read);

#define MCFG_M5074X_PORT3_READ_CALLBACK(_read) \
	devcb = &m5074x_device::set_p3_rd_callback(*device, DEVCB_##_read);

#define MCFG_M5074X_PORT0_WRITE_CALLBACK(_write) \
	devcb = &m5074x_device::set_p0_wr_callback(*device, DEVCB_##_write);

#define MCFG_M5074X_PORT1_WRITE_CALLBACK(_write) \
	devcb = &m5074x_device::set_p1_wr_callback(*device, DEVCB_##_write);

#define MCFG_M5074X_PORT2_WRITE_CALLBACK(_write) \
	devcb = &m5074x_device::set_p2_wr_callback(*device, DEVCB_##_write);

#define MCFG_M5074X_PORT3_WRITE_CALLBACK(_write) \
	devcb = &m5074x_device::set_p3_wr_callback(*device, DEVCB_##_write);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m5074x_device

class m5074x_device :  public m740_device
{
	friend class m50740_device;
	friend class m50741_device;

	enum
	{
		M5074X_INT1_LINE = INPUT_LINE_IRQ0,

		M5074X_SET_OVERFLOW = M740_SET_OVERFLOW
	};

	enum
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_X,

		NUM_TIMERS
	};

public:
	// construction/destruction
	m5074x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, address_map_constructor internal_map, const char *shortname, const char *source);

	const address_space_config m_program_config;

	template<class _Object> static devcb_base &set_p0_rd_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).read_p0.set_callback(object); }
	template<class _Object> static devcb_base &set_p1_rd_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).read_p1.set_callback(object); }
	template<class _Object> static devcb_base &set_p2_rd_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).read_p2.set_callback(object); }
	template<class _Object> static devcb_base &set_p3_rd_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).read_p3.set_callback(object); }
	template<class _Object> static devcb_base &set_p0_wr_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).write_p0.set_callback(object); }
	template<class _Object> static devcb_base &set_p1_wr_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).write_p1.set_callback(object); }
	template<class _Object> static devcb_base &set_p2_wr_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).write_p2.set_callback(object); }
	template<class _Object> static devcb_base &set_p3_wr_callback(device_t &device, _Object object) { return downcast<m5074x_device &>(device).write_p3.set_callback(object); }

	devcb_read8  read_p0, read_p1, read_p2, read_p3;
	devcb_write8 write_p0, write_p1, write_p2, write_p3;

	DECLARE_READ8_MEMBER(ports_r);
	DECLARE_WRITE8_MEMBER(ports_w);
	DECLARE_READ8_MEMBER(tmrirq_r);
	DECLARE_WRITE8_MEMBER(tmrirq_w);

	bool are_port_bits_output(UINT8 port, UINT8 mask) { return ((m_ddrs[port] & mask) == mask) ? true : false; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	void send_port(address_space &space, UINT8 offset, UINT8 data);
	UINT8 read_port(UINT8 offset);

	void recalc_irqs();
	void recalc_timer(int timer);

	UINT8 m_ports[6], m_ddrs[6];
	UINT8 m_intctrl, m_tmrctrl;
	UINT8 m_tmr12pre, m_tmr1, m_tmr2, m_tmrxpre, m_tmrx;
	UINT8 m_tmr1latch, m_tmr2latch, m_tmrxlatch;
	UINT8 m_last_all_ints;

private:
	emu_timer *m_timers[NUM_TIMERS];
};

class m50740_device : public m5074x_device
{
public:
	m50740_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m50740_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
};

class m50741_device : public m5074x_device
{
public:
	m50741_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m50741_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
};

extern const device_type M50740;
extern const device_type M50741;

#endif
