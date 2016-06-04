// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
/***************************************************************************

    timekpr.h

    Various ST Microelectronics timekeeper SRAM implementations:
        - M48T02
        - M48T35
        - M48T37
        - M48T58
        - MK48T08

***************************************************************************/

#pragma once

#ifndef __TIMEKPR_H__
#define __TIMEKPR_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M48T02_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T02, 0)

#define MCFG_M48T35_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T35, 0)

#define MCFG_M48T37_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T37, 0)

#define MCFG_M48T58_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T58, 0)

#define MCFG_MK48T08_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MK48T08, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> timekeeper_device

class timekeeper_device :   public device_t,
							public device_nvram_interface
{
protected:
	// construction/destruction
	timekeeper_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int size);

public:
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	void counters_to_ram();
	void counters_from_ram();

	// internal state
	UINT8 m_control;
	UINT8 m_seconds;
	UINT8 m_minutes;
	UINT8 m_hours;
	UINT8 m_day;
	UINT8 m_date;
	UINT8 m_month;
	UINT8 m_year;
	UINT8 m_century;

	dynamic_buffer m_data;
	optional_region_ptr<UINT8> m_default_data;

protected:
	int m_size;
	int m_offset_control;
	int m_offset_seconds;
	int m_offset_minutes;
	int m_offset_hours;
	int m_offset_day;
	int m_offset_date;
	int m_offset_month;
	int m_offset_year;
	int m_offset_century;
	int m_offset_flags;
};

class m48t02_device : public timekeeper_device
{
public:
	m48t02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class m48t35_device : public timekeeper_device
{
public:
	m48t35_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class m48t37_device : public timekeeper_device
{
public:
	m48t37_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class m48t58_device : public timekeeper_device
{
public:
	m48t58_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mk48t08_device : public timekeeper_device
{
public:
	mk48t08_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// device type definition
extern const device_type M48T02;
extern const device_type M48T35;
extern const device_type M48T37;
extern const device_type M48T58;
extern const device_type MK48T08;

#endif // __TIMEKPR_H__
