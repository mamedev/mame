// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_MACHINE_SDA2006_H
#define MAME_MACHINE_SDA2006_H

#pragma once



// sda2006_device

class sda2006_device : public device_t,
	public device_nvram_interface
{
public:
	// construction/destruction
	sda2006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// I/O operations
	int read_data();
	void write_data(int state);
	void write_clock(int state);
	void write_enable(int state);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	enum {
		EEPROM_READ = 0,
		EEPROM_WRITE
	};

	uint8_t m_latch;
	uint8_t m_current_address;
	uint32_t m_eeprom_state;
	uint8_t m_read_stream_pos;
	bool m_is_end_o_stream;
	uint8_t m_write_stream_length;
	uint32_t m_write_stream;
	uint8_t m_write_state;
	uint8_t m_clock_state;

	optional_memory_region m_region;
	uint16_t m_eeprom_data[0x20];
};


// device type definition
DECLARE_DEVICE_TYPE(SDA2006, sda2006_device)

#endif // MAME_MACHINE_SDA2006_H
