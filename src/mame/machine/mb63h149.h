// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB63H149 gate array

***************************************************************************/

#ifndef MAME_MACHINE_MB63H149_H
#define MAME_MACHINE_MB63H149_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb63h149_device

class mb63h149_device : public device_t
{
public:
	// device type constructor
	mb63h149_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto int_callback() { return m_int_callback.bind(); }

	// CPU read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	mb63h149_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// callback objects
	devcb_write_line m_int_callback;
};

// ======================> mb63h130_device

class mb63h130_device : public mb63h149_device
{
public:
	// device type constructor
	mb63h130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// device type declarations
DECLARE_DEVICE_TYPE(MB63H149, mb63h149_device)
DECLARE_DEVICE_TYPE(MB63H130, mb63h130_device)

#endif // MAME_MACHINE_MB63H149_H
