// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Generic 8bit and 16 bit latch devices

***************************************************************************/

#pragma once

#ifndef __GEN_LATCH_H__
#define __GEN_LATCH_H__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GENERIC_LATCH_8_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GENERIC_LATCH_8, 0)

#define MCFG_GENERIC_LATCH_16_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GENERIC_LATCH_16, 0)

#define MCFG_GENERIC_LATCH_DATA_PENDING_CB(_devcb) \
	devcb = &generic_latch_base_device::set_data_pending_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> generic_latch_base_device

class generic_latch_base_device : public device_t
{
protected:
	// construction/destruction
	generic_latch_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source);

public:
	// static configuration
	template<class _Object> static devcb_base &set_data_pending_callback(device_t &device, _Object object) { return downcast<generic_latch_base_device &>(device).m_data_pending_cb.set_callback(object); }

	DECLARE_READ_LINE_MEMBER(pending_r);

protected:
	virtual void device_start() override;

	bool is_latch_read() const { return m_latch_read; }
	void set_latch_read(bool latch_read);

private:
	bool                    m_latch_read;
	devcb_write_line        m_data_pending_cb;				
};


// ======================> generic_latch_8_device

class generic_latch_8_device : public generic_latch_base_device
{
public:
	// construction/destruction
	generic_latch_8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( preset_w );
	DECLARE_WRITE8_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( preset_w );
	DECLARE_WRITE_LINE_MEMBER( clear_w );

	void preset_w(u8 value) { m_latched_value = value; }

protected:
	virtual void device_start() override;

	void sync_callback(void *ptr, s32 param);

private:
	u8                      m_latched_value;
};

// device type definition
extern const device_type GENERIC_LATCH_8;


// ======================> generic_latch_16_device

class generic_latch_16_device : public generic_latch_base_device
{
public:
	// construction/destruction
	generic_latch_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	DECLARE_WRITE16_MEMBER( preset_w );
	DECLARE_WRITE16_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( preset_w );
	DECLARE_WRITE_LINE_MEMBER( clear_w );

	void preset_w(u16 value) { m_latched_value = value; }

protected:
	virtual void device_start() override;

	void sync_callback(void *ptr, s32 param);

private:
	u16                     m_latched_value;
};

// device type definition
extern const device_type GENERIC_LATCH_16;


#endif  /* __GEN_LATCH_H__ */
