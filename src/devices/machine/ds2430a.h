// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Dallas Semiconductor DS2430A 256-Bit 1-Wire EEPROM

**********************************************************************/

#ifndef MAME_MACHINE_DS2430A_H
#define MAME_MACHINE_DS2430A_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ds1wire_device

class ds1wire_device : public device_t
{
protected:
	enum class state : u8
	{
		PRESENCE = 0,
		ROM_COMMAND,
		ROM_READ,
		ROM_MATCH,
		ROM_SEARCH,
		ROM_SEARCH_COMPLEMENT,
		ROM_SEARCH_WRITE,
		MEMORY_COMMAND,
		MEMORY_READ,
		MEMORY_WRITE,
		MEMORY_COPY,
		DONE
	};

	ds1wire_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// Dallas 1-Wire CRC algorithm (use with std::accumulate)
	static u8 ds1wire_crc(u8 crc, u8 data) noexcept
	{
		for (int i = 8; i > 0; --i)
		{
			crc = (crc >> 1) ^ (BIT(data ^ crc, 0) ? 0x8c : 0);
			data >>= 1;
		}
		return crc;
	}

public:
	void set_timing_scale(double scale) { m_timing_scale = scale; }

	// serial data line handlers
	int data_r();
	void data_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual state ds1wire_next_state(state prev_state, u8 command, u16 index, u8 data) = 0;
	virtual u8 ds1wire_read_rom(u16 index) const = 0;
	virtual u8 ds1wire_read_memory(u8 command, u16 index) const = 0;
	virtual void ds1wire_memory_copy(u8 command) = 0;

private:
	// internal helpers
	attotime scaled_time(attoseconds_t tconst) const { return attotime(0, tconst * m_timing_scale); }
	bool set_state(state new_state);
	void pulse_start(attotime time);
	void pulse_end(attotime time);
	TIMER_CALLBACK_MEMBER(update_state);

	// misc. configuration
	double m_timing_scale;

	// timer object
	emu_timer *m_slot_timer;

	// line state
	bool m_data_in;
	bool m_data_out;

	// internal state
	u8 m_shift_data;
	u8 m_command;
	u32 m_bit_count;
	attotime m_pulse_start_time;
	state m_current_state;
};


// ======================> ds2430a_device

class ds2430a_device : public ds1wire_device, public device_nvram_interface
{
public:
	// device type constructor
	ds2430a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	ds2430a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void nvram_default() override;

	// ds1wire_device overrides
	virtual state ds1wire_next_state(state prev_state, u8 command, u16 index, u8 data) override;
	virtual u8 ds1wire_read_rom(u16 index) const override;
	virtual u8 ds1wire_read_memory(u8 command, u16 index) const override;
	virtual void ds1wire_memory_copy(u8 command) override;

private:
	optional_region_ptr<u8> m_default_data;

	u8 m_rom[8];
	u8 m_eeprom[0x20];
	u8 m_scratchpad[0x20];
	u8 m_app_scratchpad[8];

	u8 m_start_address;
};

// ======================> ds1971_device

class ds1971_device : public ds2430a_device
{
public:
	// device type constructor
	ds1971_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// device type declarations
DECLARE_DEVICE_TYPE(DS2430A, ds2430a_device)
DECLARE_DEVICE_TYPE(DS1971, ds1971_device)

#endif // MAME_MACHINE_DS2430A_H
