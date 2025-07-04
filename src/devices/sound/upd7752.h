// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_SOUND_UPD7752_H
#define MAME_SOUND_UPD7752_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd7752_device

class upd7752_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	// construction/destruction
	upd7752_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	void upd7752_ram(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;
	virtual space_config_vector memory_space_config() const override;

private:
	sound_stream *m_stream;
	const address_space_config m_space_config;
	uint8_t m_status;
	uint16_t m_ram_addr;
	uint8_t m_mode;

	void status_change(uint8_t flag,bool type);
	inline uint8_t readbyte(offs_t address);
	inline void writebyte(offs_t address, uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(UPD7752, upd7752_device)

#endif // MAME_SOUND_UPD7752_H
