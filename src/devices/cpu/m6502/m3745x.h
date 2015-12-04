// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#pragma once

#ifndef __M3745X_H__
#define __M3745X_H__

#include "m740.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M3745X_ADC14_CALLBACKS(_ad0, _ad1, _ad2, _ad3) \
	downcast<m3745x_device *>(device)->set_ad14_callbacks(DEVCB_##_ad0, DEVCB_##_ad1, DEVCB_##_ad2, DEVCB_##_ad3);

#define MCFG_M3745X_ADC58_CALLBACKS(_ad0, _ad1, _ad2, _ad3) \
	downcast<m3745x_device *>(device)->set_ad58_callbacks(DEVCB_##_ad0, DEVCB_##_ad1, DEVCB_##_ad2, DEVCB_##_ad3);

#define MCFG_M3745X_PORT3_CALLBACKS(_read, _write) \
	downcast<m3745x_device *>(device)->set_p3_callbacks(DEVCB_##_read, DEVCB_##_write);

#define MCFG_M3745X_PORT4_CALLBACKS(_read, _write) \
	downcast<m3745x_device *>(device)->set_p4_callbacks(DEVCB_##_read, DEVCB_##_write);

#define MCFG_M3745X_PORT5_CALLBACKS(_read, _write) \
	downcast<m3745x_device *>(device)->set_p5_callbacks(DEVCB_##_read, DEVCB_##_write);

#define MCFG_M3745X_PORT6_CALLBACKS(_read, _write) \
	downcast<m3745x_device *>(device)->set_p6_callbacks(DEVCB_##_read, DEVCB_##_write);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m3745x_device

class m3745x_device :  public m740_device
{
	friend class m37450_device;

	enum
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_3,

		TIMER_ADC,

		NUM_TIMERS
	};

public:
	enum
	{
		M3745X_INT1_LINE = INPUT_LINE_IRQ0,
		M3745X_INT2_LINE,
		M3745X_INT3_LINE,

		M3745X_SET_OVERFLOW = M740_SET_OVERFLOW
	};

	// construction/destruction
	m3745x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, address_map_constructor internal_map, const char *shortname, const char *source);

	const address_space_config m_program_config;

	template<class _read, class _write> void set_p3_callbacks(_read rd, _write wr)
	{
		read_p3.set_callback(rd);
		write_p3.set_callback(wr);
	}

	template<class _read, class _write> void set_p4_callbacks(_read rd, _write wr)
	{
		read_p4.set_callback(rd);
		write_p4.set_callback(wr);
	}

	template<class _read, class _write> void set_p5_callbacks(_read rd, _write wr)
	{
		read_p5.set_callback(rd);
		write_p5.set_callback(wr);
	}

	template<class _read, class _write> void set_p6_callbacks(_read rd, _write wr)
	{
		read_p6.set_callback(rd);
		write_p6.set_callback(wr);
	}

	template<class _read, class _read2, class _read3, class _read4> void set_ad14_callbacks(_read rd, _read2 rd2, _read3 rd3, _read4 rd4)
	{
		read_ad_0.set_callback(rd);
		read_ad_1.set_callback(rd2);
		read_ad_2.set_callback(rd3);
		read_ad_3.set_callback(rd4);
	}

	template<class _read, class _read2, class _read3, class _read4> void set_ad58_callbacks(_read rd, _read2 rd2, _read3 rd3, _read4 rd4)
	{
		read_ad_4.set_callback(rd);
		read_ad_5.set_callback(rd2);
		read_ad_6.set_callback(rd3);
		read_ad_7.set_callback(rd4);
	}

	devcb_read8  read_p3, read_p4, read_p5, read_p6;
	devcb_write8 write_p3, write_p4, write_p5, write_p6;
	devcb_read8  read_ad_0, read_ad_1, read_ad_2, read_ad_3;
	devcb_read8  read_ad_4, read_ad_5, read_ad_6, read_ad_7;

	DECLARE_READ8_MEMBER(ports_r);
	DECLARE_WRITE8_MEMBER(ports_w);
	DECLARE_READ8_MEMBER(adc_r);
	DECLARE_WRITE8_MEMBER(adc_w);
	DECLARE_READ8_MEMBER(intregs_r);
	DECLARE_WRITE8_MEMBER(intregs_w);

	bool are_port_bits_output(UINT8 port, UINT8 mask) { return ((m_ddrs[port] & mask) == mask) ? true : false; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void execute_set_input(int inputnum, int state);
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	void send_port(address_space &space, UINT8 offset, UINT8 data);
	UINT8 read_port(UINT8 offset);

	void recalc_irqs();

	UINT8 m_ports[6], m_ddrs[6];
	UINT8 m_intreq1, m_intreq2, m_intctrl1, m_intctrl2;
	UINT8 m_adctrl;
	UINT16 m_last_all_ints;

private:
	emu_timer *m_timers[NUM_TIMERS];
};

class m37450_device : public m3745x_device
{
public:
	m37450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m37450_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:

private:
};

extern const device_type M37450;

#endif
