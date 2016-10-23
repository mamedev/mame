// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __UPD7752DEV_H__
#define __UPD7752DEV_H__

/* status flags */
#define BSY 1<<7
#define REQ 1<<6
#define EXT 1<<5
#define ERR 1<<4


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD7752_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, UPD7752, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd7752_device

class upd7752_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	// construction/destruction
	upd7752_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream  *m_stream;
	const address_space_config      m_space_config;
	uint8_t m_status;
	uint16_t m_ram_addr;
	uint8_t m_mode;
	void status_change(uint8_t flag,bool type);
	inline uint8_t readbyte(offs_t address);
	inline void writebyte(offs_t address, uint8_t data);
};


// device type definition
extern const device_type UPD7752;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
