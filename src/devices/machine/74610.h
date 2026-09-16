// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*****************************************************************************

    74610: Memory mappers

    Variant   Output       Type
    ----------------------------------------
    74610     latched      tristate
    74611     latched      open collector
    74612     direct       tristate
    74613     direct       open collector

    Michael Zapf
    June 2025

**********************************************************************/

#ifndef MAME_MACHINE_74610_H
#define MAME_MACHINE_74610_H

#pragma once

/*
    Overall base class
*/
class ttl7461x_device : public device_t
{
public:
	auto map_output_cb() { return m_map_output.bind(); }

	void set_register(int num, uint16_t value);
	uint16_t get_register(int num);

	void mapper_output_rz(uint8_t num, uint16_t& value);
	uint16_t get_mapper_output(uint8_t num);

	void map_mode_w(int mapping);
	void map_enable_w(int enable);

protected:
	ttl7461x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	bool m_has_tristate;
	bool m_enabled;
	devcb_write16 m_map_output;
	uint16_t m_map[16];
	bool m_map_mode;
};

/*
    Base class for the latched variants
*/
class ttl7461x_latched_device : public ttl7461x_device
{
public:
	void set_map_address(uint8_t num);
	uint16_t get_mapper_output();
	void mapper_output_rz(uint16_t& value);
	void latch_enable_w(int enable);

protected:
	ttl7461x_latched_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	bool m_latch_enabled;
	uint16_t m_latched_output;

	virtual void device_start() override ATTR_COLD;
};


/*
    Variants
*/
class ttl74610_device : public ttl7461x_latched_device
{
public:
	ttl74610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ttl74611_device : public ttl7461x_latched_device
{
public:
	ttl74611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ttl74612_device : public ttl7461x_device
{
public:
	ttl74612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ttl74613_device : public ttl7461x_device
{
public:
	ttl74613_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// device type definition
DECLARE_DEVICE_TYPE(TTL74610, ttl74610_device)
DECLARE_DEVICE_TYPE(TTL74611, ttl74611_device)
DECLARE_DEVICE_TYPE(TTL74612, ttl74612_device)
DECLARE_DEVICE_TYPE(TTL74613, ttl74613_device)

#endif // MAME_MACHINE_74160_H
