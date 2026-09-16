// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_XC1700E_H
#define MAME_MACHINE_XC1700E_H

#pragma once

class base_xc1700e_device : public device_t
{
public:
	// configuration
	auto cascade_r() { return m_cascade_cb.bind(); }

	// input/output lines
	void reset_w(int state);
	int data_r();

protected:
	base_xc1700e_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 capacity);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;

private:
	// device configuration
	const u32 m_capacity;
	required_memory_region m_region;

	devcb_read_line m_cascade_cb;

	// device state
	bool m_reset;
	unsigned m_address;
};

DECLARE_DEVICE_TYPE(XC1736E,  xc1736e_device)
DECLARE_DEVICE_TYPE(XC1765E,  xc1765e_device)
DECLARE_DEVICE_TYPE(XC17128E, xc17128e_device)
DECLARE_DEVICE_TYPE(XC17256E, xc17256e_device)
DECLARE_DEVICE_TYPE(XC17512L, xc17512l_device)
DECLARE_DEVICE_TYPE(XC1701,   xc1701_device)
DECLARE_DEVICE_TYPE(XC1702L,  xc1702l_device)
DECLARE_DEVICE_TYPE(XC1704L,  xc1704l_device)

class xc1736e_device : public base_xc1700e_device
{
public:
	xc1736e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC1736E, tag, owner, clock, 36'288UL)
	{
	}
};

class xc1765e_device : public base_xc1700e_device
{
public:
	xc1765e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC1765E, tag, owner, clock, 65'536UL)
	{
	}
};

class xc17128e_device : public base_xc1700e_device
{
public:
	xc17128e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC17128E, tag, owner, clock, 131'072UL)
	{
	}
};

class xc17256e_device : public base_xc1700e_device
{
public:
	xc17256e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC17256E, tag, owner, clock, 262'144UL)
	{
	}
};

class xc17512l_device : public base_xc1700e_device
{
public:
	xc17512l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC17512L, tag, owner, clock, 524'288UL)
	{
	}
};

class xc1701_device : public base_xc1700e_device
{
public:
	xc1701_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC1701, tag, owner, clock, 1'048'576UL)
	{
	}
};

class xc1702l_device : public base_xc1700e_device
{
public:
	xc1702l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC1702L, tag, owner, clock, 2'097'152UL)
	{
	}
};

class xc1704l_device : public base_xc1700e_device
{
public:
	xc1704l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: base_xc1700e_device(mconfig, XC1704L, tag, owner, clock, 4'194'304UL)
	{
	}
};

#endif // MAME_MACHINE_XC1700E_H
