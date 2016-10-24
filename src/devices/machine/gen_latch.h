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

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void preset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void preset_w(int state);
	void clear_w(int state);

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

	uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void preset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void clear_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void preset_w(int state);
	void clear_w(int state);

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
