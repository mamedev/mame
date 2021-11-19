// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland RF5C36 (15229840) & SA-16 (15229874) Sampler Custom ICs

***************************************************************************/

#ifndef MAME_MACHINE_SA16_H
#define MAME_MACHINE_SA16_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sa16_base_device

class sa16_base_device : public device_t
{
public:
	// callback configuration
	auto int_callback() { return m_int_callback.bind(); }
	auto sh_callback() { return m_sh_callback.bind(); }

	// CPU read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// base type constructor
	sa16_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// line callbacks
	devcb_write_line m_int_callback;
	devcb_write_line m_sh_callback;

	// internal state (TODO)
};

// ======================> rf5c36_device

class rf5c36_device : public sa16_base_device
{
public:
	// device type constructor
	rf5c36_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> sa16_device

class sa16_device : public sa16_base_device
{
public:
	// device type constructor
	sa16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(RF5C36, rf5c36_device)
DECLARE_DEVICE_TYPE(SA16, sa16_device)

#endif // MAME_MACHINE_SA16_H
