// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Generic 8bit and 16 bit latch devices

***************************************************************************/

#pragma once

#ifndef __GEN_LATCH_H__
#define __GEN_LATCH_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GENERIC_LATCH_8_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GENERIC_LATCH_8, 0)

#define MCFG_GENERIC_LATCH_16_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GENERIC_LATCH_16, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> generic_latch_8_device

class generic_latch_8_device : public device_t
{
public:
	// construction/destruction
	generic_latch_8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( preset_w );
	DECLARE_WRITE8_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( preset_w );
	DECLARE_WRITE_LINE_MEMBER( clear_w );

	void preset_w(uint16_t value) { m_latched_value = value; }

protected:
	virtual void device_start() override;

	void sync_callback(void *ptr, int32_t param);
private:
	uint16_t                  m_latched_value;
	uint8_t                   m_latch_read;
};

// device type definition
extern const device_type GENERIC_LATCH_8;


// ======================> generic_latch_16_device

class generic_latch_16_device : public device_t
{
public:
	// construction/destruction
	generic_latch_16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	DECLARE_WRITE16_MEMBER( preset_w );
	DECLARE_WRITE16_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( preset_w );
	DECLARE_WRITE_LINE_MEMBER( clear_w );

protected:
	virtual void device_start() override;

	void sync_callback(void *ptr, int32_t param);
private:
	uint16_t                  m_latched_value;
	uint8_t                   m_latch_read;
};

// device type definition
extern const device_type GENERIC_LATCH_16;


#endif  /* __GEN_LATCH_H__ */
