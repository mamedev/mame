// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eepromser.h

    Serial EEPROM devices.

***************************************************************************/

#ifndef MAME_MACHINE_EEPROMSER_H
#define MAME_MACHINE_EEPROMSER_H

#pragma once

#include "eeprom.h"

enum class eeprom_serial_streaming : bool
{
	DISABLE = false,
	ENABLE = true
};

// ======================> eeprom_serial_base_device

class eeprom_serial_base_device : public eeprom_base_device
{
public:
	// inline configuration helpers
	void enable_streaming(bool enable) { m_streaming_enabled = enable; }
	void enable_output_on_falling_clock(bool enable) { m_output_on_falling_clock_enabled = enable; }
	auto do_callback() { return m_do_cb.bind(); }

protected:
	// construction/destruction
	eeprom_serial_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, eeprom_serial_streaming enable_streaming);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// read interfaces differ between implementations

	// commands
	enum eeprom_command : u8
	{
		COMMAND_INVALID,
		COMMAND_READ,
		COMMAND_WRITE,
		COMMAND_ERASE,
		COMMAND_LOCK,
		COMMAND_UNLOCK,
		COMMAND_WRITEALL,
		COMMAND_ERASEALL,
		COMMAND_COPY_EEPROM_TO_RAM,
		COMMAND_COPY_RAM_TO_EEPROM
	};

	// states
	enum eeprom_state : u8
	{
		STATE_IN_RESET,
		STATE_WAIT_FOR_START_BIT,
		STATE_WAIT_FOR_COMMAND,
		STATE_READING_DATA,
		STATE_WAIT_FOR_DATA,
		STATE_WAIT_FOR_COMPLETION
	};

	// events
	enum eeprom_event : u8
	{
		EVENT_CS_RISING_EDGE = 1 << 0,
		EVENT_CS_FALLING_EDGE = 1 << 1,
		EVENT_CLK_RISING_EDGE = 1 << 2,
		EVENT_CLK_FALLING_EDGE = 1 << 3
	};

	// internal helpers
	void set_address_bits(int addrbits) { m_command_address_bits = addrbits; }
	void set_state(eeprom_state newstate);
	void execute_write_command();

	// subclass helpers
	void base_cs_write(int state);
	void base_clk_write(int state);
	void base_di_write(int state);
	int base_do_read();
	int base_ready_read();

	// subclass overrides
	virtual void handle_event(eeprom_event event);
	virtual void parse_command_and_address() = 0;
	virtual void execute_command();


	// configuration state
	uint8_t         m_command_address_bits;     // number of address bits in a command
	bool            m_streaming_enabled;        // true if streaming is enabled
	bool            m_output_on_falling_clock_enabled;  // true if the output pin is updated on the falling edge of the clock
	devcb_write_line m_do_cb;                   // callback to push state of DO line

	// runtime state
	eeprom_state    m_state;                    // current internal state
	uint8_t         m_cs_state;                 // state of the CS line
	attotime        m_last_cs_rising_edge_time; // time of the last CS rising edge
	uint8_t         m_oe_state;                 // state of the OE line
	uint8_t         m_clk_state;                // state of the CLK line
	uint8_t         m_di_state;                 // state of the DI line
	bool            m_locked;                   // are we locked against writes?
	uint32_t        m_bits_accum;               // number of bits accumulated
	uint32_t        m_command_address_accum;    // accumulator of command+address bits
	eeprom_command  m_command;                  // current command
	uint32_t        m_address;                  // current address extracted from command
	uint32_t        m_shift_register;           // holds data coming in/going out
};



// ======================> eeprom_serial_93cxx_device

class eeprom_serial_93cxx_device : public eeprom_serial_base_device
{
public:
	// read handlers
	int do_read();  // combined DO+READY/BUSY

	// write handlers
	void cs_write(int state);        // CS signal (active high)
	void clk_write(int state);       // CLK signal (active high)
	void di_write(int state);        // DI

protected:
	// construction/destruction
	using eeprom_serial_base_device::eeprom_serial_base_device;

	// subclass overrides
	virtual void parse_command_and_address() override;
};


// ======================> eeprom_serial_s29x90_device

class eeprom_serial_s29x90_device : public eeprom_serial_93cxx_device
{
protected:
	// construction/destruction
	eeprom_serial_s29x90_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, eeprom_serial_streaming ignored);
};


// ======================> eeprom_serial_er5911_device

class eeprom_serial_er5911_device : public eeprom_serial_base_device
{
public:
	// read handlers
	int do_read();          // DO
	int ready_read();       // READY/BUSY only

	// write handlers
	void cs_write(int state);        // CS signal (active high)
	void clk_write(int state);       // CLK signal (active high)
	void di_write(int state);        // DI

protected:
	// construction/destruction
	using eeprom_serial_base_device::eeprom_serial_base_device;

	// subclass overrides
	virtual void parse_command_and_address() override;
};


// ======================> eeprom_serial_x24c44_device

class eeprom_serial_x24c44_device : public eeprom_serial_base_device
{
		//async recall not implemented
		//async store not implemented
public:
	// read handlers
	int do_read();          // DO

	// write handlers
	void cs_write(int state);        // CS signal (active high)
	void clk_write(int state);       // CLK signal (active high)
	void di_write(int state);        // DI

protected:
	// construction/destruction
	using eeprom_serial_base_device::eeprom_serial_base_device;

	// subclass overrides
	virtual void parse_command_and_address() override;
	void handle_event(eeprom_event event) override;
	virtual void parse_command_and_address_2_bit();
	void execute_command() override;
	void copy_ram_to_eeprom();
	void copy_eeprom_to_ram();
	void device_start() override ATTR_COLD;
	uint8_t m_ram_length;
	uint16_t m_ram_data[16];
	uint16_t m_reading;
	uint8_t m_store_latch;
};



//**************************************************************************
//  DERIVED TYPES
//**************************************************************************

// macro for declaring a new device class
#define DECLARE_SERIAL_EEPROM_DEVICE(_baseclass, _lowercase, _uppercase, _bits) \
class eeprom_serial_##_lowercase##_##_bits##bit_device : public eeprom_serial_##_baseclass##_device \
{ \
public: \
	eeprom_serial_##_lowercase##_##_bits##bit_device(const machine_config &mconfig, const char *tag, device_t *owner, eeprom_serial_streaming enable_streaming = eeprom_serial_streaming::DISABLE); \
	eeprom_serial_##_lowercase##_##_bits##bit_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock); \
}; \
DECLARE_DEVICE_TYPE(EEPROM_##_uppercase##_##_bits##BIT, eeprom_serial_##_lowercase##_##_bits##bit_device)

// standard 93CX6 class of 16-bit EEPROMs
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c06, 93C06, 16)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c46, 93C46, 16)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c56, 93C56, 16)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c57, 93C57, 16)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c66, 93C66, 16)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c76, 93C76, 16)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c86, 93C86, 16)

// some manufacturers use pin 6 as an "ORG" pin which, when pulled low, configures memory for 8-bit accesses
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c46, 93C46, 8)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c56, 93C56, 8)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c57, 93C57, 8)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c66, 93C66, 8)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c76, 93C76, 8)
DECLARE_SERIAL_EEPROM_DEVICE(93cxx, 93c86, 93C86, 8)

// ER5911 has a separate ready pin, a reduced command set, and supports 8/16 bit out of the box
DECLARE_SERIAL_EEPROM_DEVICE(er5911, er5911, ER5911, 8)
DECLARE_SERIAL_EEPROM_DEVICE(er5911, er5911, ER5911, 16)
DECLARE_SERIAL_EEPROM_DEVICE(er5911, msm16911, MSM16911, 8)
DECLARE_SERIAL_EEPROM_DEVICE(er5911, msm16911, MSM16911, 16)

// Seiko S-29X90 class of 16-bit EEPROMs. They always use 13 address bits, despite needing only 6-8.
// The output is updated on the falling edge of the clock. Streaming is enabled
DECLARE_SERIAL_EEPROM_DEVICE(s29x90, s29190, S29190, 16)
DECLARE_SERIAL_EEPROM_DEVICE(s29x90, s29290, S29290, 16)
DECLARE_SERIAL_EEPROM_DEVICE(s29x90, s29390, S29390, 16)

// X24c44 16 bit 32byte ram/eeprom combo
DECLARE_SERIAL_EEPROM_DEVICE(x24c44, x24c44, X24C44, 16)

#endif // MAME_MACHINE_EEPROMSER_H
