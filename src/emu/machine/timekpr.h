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
#include "devhelpr.h"



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


// ======================> timekeeper_config

struct timekeeper_config
{
	const UINT8 *m_data;
};


// ======================> timekeeper_device

class timekeeper_device :	public device_t,
							public device_nvram_interface
{
protected:
	// construction/destruction
	timekeeper_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

public:
	void write(UINT16 offset, UINT8 data);
	UINT8 read(UINT16 offset);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

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

	UINT8 *m_data;
	UINT8 *m_default_data;

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



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

DECLARE_WRITE8_DEVICE_HANDLER( timekeeper_w );
DECLARE_READ8_DEVICE_HANDLER( timekeeper_r );

#endif // __TIMEKPR_H__
