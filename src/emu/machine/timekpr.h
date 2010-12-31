/***************************************************************************

    timekpr.h

    Various ST Microelectronics timekeeper SRAM implementations:
        - M48T02
        - M48T35
        - M48T58
        - MK48T08

***************************************************************************/

#pragma once

#ifndef __TIMEKPR_H__
#define __TIMEKPR_H__

#include "emu.h"
#include "devhelpr.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M48T02_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T02, 0)

#define MCFG_M48T35_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T35, 0)

#define MCFG_M48T58_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T58, 0)

#define MCFG_MK48T08_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MK48T08, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> timekeeper_config

struct timekeeper_config
{
	const UINT8 *m_data;
};

// ======================> timekeeper_device_config

class timekeeper_device_config :	public device_config,
									public device_config_nvram_interface,
									public timekeeper_config
{
	friend class timekeeper_device;
	friend class m48t02_device_config;
	friend class m48t35_device_config;
	friend class m48t58_device_config;
	friend class mk48t08_device_config;

protected:
	// construction/destruction
	timekeeper_device_config(const machine_config &mconfig, const char *type, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
};


// ======================> timekeeper_device

class timekeeper_device :	public device_t,
							public device_nvram_interface
{
	friend class timekeeper_device_config;
	friend class m48t02_device;
	friend class m48t35_device;
	friend class m48t58_device;
	friend class mk48t08_device;

	// construction/destruction
	timekeeper_device(running_machine &_machine, const timekeeper_device_config &config);

public:
	void timekeeper_tick();

	void write(UINT16 offset, UINT8 data);
	UINT8 read(UINT16 offset);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(mame_file &file);
	virtual void nvram_write(mame_file &file);

	// internal state
	const timekeeper_device_config &m_config;

	static TIMER_CALLBACK( timekeeper_tick_callback );

private:
	void counters_to_ram();
	void counters_from_ram();

	UINT8 m_control;
	UINT8 m_seconds;
	UINT8 m_minutes;
	UINT8 m_hours;
	UINT8 m_day;
	UINT8 m_date;
	UINT8 m_month;
	UINT8 m_year;
	UINT8 m_century;

	UINT8 *m_data;
	UINT8 *m_default_data;

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

GENERIC_DEVICE_DERIVED_CONFIG(timekeeper, m48t02)
GENERIC_DEVICE_DERIVED_CONFIG(timekeeper, m48t35)
GENERIC_DEVICE_DERIVED_CONFIG(timekeeper, m48t58)
GENERIC_DEVICE_DERIVED_CONFIG(timekeeper, mk48t08)

// device type definition
extern const device_type M48T02;
extern const device_type M48T35;
extern const device_type M48T58;
extern const device_type MK48T08;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_DEVICE_HANDLER( timekeeper_w );
READ8_DEVICE_HANDLER( timekeeper_r );

#endif // __TIMEKPR_H__
