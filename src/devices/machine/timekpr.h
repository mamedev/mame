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
        - MK48T12

***************************************************************************/

#ifndef MAME_MACHINE_TIMEKPR_H
#define MAME_MACHINE_TIMEKPR_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M48T02_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T02, 0)

#define MCFG_M48T35_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T35, 0)

#define MCFG_M48T37_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T37, 0)

#define MCFG_M48T37_RESET_HANDLER(_devcb) \
	devcb = &downcast<timekeeper_device &>(*device).set_reset_handler(DEVCB_##_devcb);

#define MCFG_M48T37_IRQ_HANDLER(_devcb) \
	devcb = &downcast<timekeeper_device &>(*device).set_irq_handler(DEVCB_##_devcb);

#define MCFG_M48T58_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M48T58, 0)

#define MCFG_MK48T08_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MK48T08, 0)

#define MCFG_MK48T12_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MK48T12, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> timekeeper_device

class timekeeper_device : public device_t, public device_nvram_interface
{
public:
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER(watchdog_write);
	template <class Object> devcb_base &set_reset_handler(Object &&cb) { return m_reset_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_irq_handler(Object &&cb) { return m_irq_cb.set_callback(std::forward<Object>(cb)); }

protected:
	// construction/destruction
	timekeeper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int size);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	TIMER_CALLBACK_MEMBER(watchdog_callback);
	devcb_write_line m_reset_cb;
	devcb_write_line m_irq_cb;

private:
	void counters_to_ram();
	void counters_from_ram();

	// internal state
	uint8_t m_control;
	uint8_t m_seconds;
	uint8_t m_minutes;
	uint8_t m_hours;
	uint8_t m_day;
	uint8_t m_date;
	uint8_t m_month;
	uint8_t m_year;
	uint8_t m_century;

	std::vector<uint8_t> m_data;
	optional_region_ptr<uint8_t> m_default_data;

	emu_timer* m_watchdog_timer;
	attotime m_watchdog_delay;
protected:
	int const m_size;
	int m_offset_watchdog;
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
	m48t02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class m48t35_device : public timekeeper_device
{
public:
	m48t35_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class m48t37_device : public timekeeper_device
{
public:
	m48t37_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class m48t58_device : public timekeeper_device
{
public:
	m48t58_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mk48t08_device : public timekeeper_device
{
public:
	mk48t08_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mk48t12_device : public timekeeper_device
{
public:
	mk48t12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(M48T02,  m48t02_device)
DECLARE_DEVICE_TYPE(M48T35,  m48t35_device)
DECLARE_DEVICE_TYPE(M48T37,  m48t37_device)
DECLARE_DEVICE_TYPE(M48T58,  m48t58_device)
DECLARE_DEVICE_TYPE(MK48T08, mk48t08_device)
DECLARE_DEVICE_TYPE(MK48T12, mk48t12_device)

#endif // MAME_MACHINE_TIMEKPR_H
