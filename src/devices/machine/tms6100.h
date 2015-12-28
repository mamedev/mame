// license:BSD-3-Clause
// copyright-holders:Couriersud

/* TMS 6100 memory controller */

#pragma once

#ifndef __TMS6100_H__
#define __TMS6100_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// 4-bit mode (mask option)
// note: in 4-bit mode, use data_r, otherwise use data_line_r

#define MCFG_TMS6100_4BIT_MODE() \
	tms6100_device::enable_4bit_mode(*device);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tms6100_device : public device_t
{
public:
	tms6100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms6100_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static void enable_4bit_mode(device_t &device) { downcast<tms6100_device &>(device).m_4bit_read = true; }

	DECLARE_WRITE_LINE_MEMBER(m0_w);
	DECLARE_WRITE_LINE_MEMBER(m1_w);
	DECLARE_WRITE_LINE_MEMBER(romclock_w);

	DECLARE_WRITE8_MEMBER(addr_w);
	DECLARE_READ8_MEMBER(data_r); // 4bit
	DECLARE_READ_LINE_MEMBER(data_line_r);

protected:
	// device-level overrides
	virtual void device_start() override;

	// internal state
	required_region_ptr<UINT8> m_rom;
	bool m_reverse_bits;
	bool m_4bit_read;
	UINT32 m_address;
	UINT32 m_address_latch;
	UINT8 m_loadptr;
	int m_m0;
	int m_m1;
	UINT8 m_addr_bits;
	int m_tms_clock;
	UINT8 m_data;
	UINT8 m_state;
};

extern const device_type TMS6100;

class m58819_device : public tms6100_device
{
public:
	m58819_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
};

extern const device_type M58819;


#endif /* __TMS6100_H__ */
