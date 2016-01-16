// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eepromser.h

    Serial EEPROM devices.

***************************************************************************/

#pragma once

#ifndef __EEPROMSER_H__
#define __EEPROMSER_H__

#include "eeprom.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// standard 93CX6 class of 16-bit EEPROMs
#define MCFG_EEPROM_SERIAL_93C06_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C06_16BIT, 0)
#define MCFG_EEPROM_SERIAL_93C46_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C46_16BIT, 0)
#define MCFG_EEPROM_SERIAL_93C56_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C56_16BIT, 0)
#define MCFG_EEPROM_SERIAL_93C57_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C57_16BIT, 0)
#define MCFG_EEPROM_SERIAL_93C66_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C66_16BIT, 0)
#define MCFG_EEPROM_SERIAL_93C76_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C76_16BIT, 0)
#define MCFG_EEPROM_SERIAL_93C86_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C86_16BIT, 0)

// some manufacturers use pin 6 as an "ORG" pin which, when pulled low, configures memory for 8-bit accesses
#define MCFG_EEPROM_SERIAL_93C46_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C46_8BIT, 0)
#define MCFG_EEPROM_SERIAL_93C56_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C56_8BIT, 0)
#define MCFG_EEPROM_SERIAL_93C57_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C57_8BIT, 0)
#define MCFG_EEPROM_SERIAL_93C66_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C66_8BIT, 0)
#define MCFG_EEPROM_SERIAL_93C76_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C76_8BIT, 0)
#define MCFG_EEPROM_SERIAL_93C86_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_93C86_8BIT, 0)

// ER5911 has a separate ready pin, a reduced command set, and supports 8/16 bit out of the box
#define MCFG_EEPROM_SERIAL_ER5911_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_ER5911_8BIT, 0)
#define MCFG_EEPROM_SERIAL_ER5911_16BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_ER5911_16BIT, 0)

#define MCFG_EEPROM_SERIAL_MSM16911_8BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_MSM16911_8BIT, 0)
#define MCFG_EEPROM_SERIAL_MSM16911_16BIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_MSM16911_16BIT, 0)

// X24c44 16 bit ram/eeprom combo
#define MCFG_EEPROM_SERIAL_X24C44_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_SERIAL_X24C44_16BIT, 0)

// optional enable for streaming reads
#define MCFG_EEPROM_SERIAL_ENABLE_STREAMING() \
	eeprom_serial_base_device::static_enable_streaming(*device);
// pass-throughs to the base class for setting default data
#define MCFG_EEPROM_SERIAL_DATA MCFG_EEPROM_DATA
#define MCFG_EEPROM_SERIAL_DEFAULT_VALUE MCFG_EEPROM_DEFAULT_VALUE



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> eeprom_serial_base_device

class eeprom_serial_base_device : public eeprom_base_device
{
protected:
	// construction/destruction
	eeprom_serial_base_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source);

public:
	// inline configuration helpers
	static void static_set_address_bits(device_t &device, int addrbits);
	static void static_enable_streaming(device_t &device);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// read interfaces differ between implementations

	// commands
	enum eeprom_command
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
	enum eeprom_state
	{
		STATE_IN_RESET,
		STATE_WAIT_FOR_START_BIT,
		STATE_WAIT_FOR_COMMAND,
		STATE_READING_DATA,
		STATE_WAIT_FOR_DATA,
		STATE_WAIT_FOR_COMPLETION
	};

	// events
	enum eeprom_event
	{
		EVENT_CS_RISING_EDGE = 1 << 0,
		EVENT_CS_FALLING_EDGE = 1 << 1,
		EVENT_CLK_RISING_EDGE = 1 << 2,
		EVENT_CLK_FALLING_EDGE = 1 << 3
	};

	// internal helpers
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
	UINT8           m_command_address_bits;     // number of address bits in a command
	bool            m_streaming_enabled;        // true if streaming is enabled

	// runtime state
	eeprom_state    m_state;                    // current internal state
	UINT8           m_cs_state;                 // state of the CS line
	attotime        m_last_cs_rising_edge_time; // time of the last CS rising edge
	UINT8           m_oe_state;                 // state of the OE line
	UINT8           m_clk_state;                // state of the CLK line
	UINT8           m_di_state;                 // state of the DI line
	bool            m_locked;                   // are we locked against writes?
	UINT32          m_bits_accum;               // number of bits accumulated
	UINT32          m_command_address_accum;    // accumulator of command+address bits
	eeprom_command  m_command;                  // current command
	UINT32          m_address;                  // current address extracted from command
	UINT32          m_shift_register;           // holds data coming in/going out
};



// ======================> eeprom_serial_93cxx_device

class eeprom_serial_93cxx_device : public eeprom_serial_base_device
{
protected:
	// construction/destruction
	eeprom_serial_93cxx_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source);

public:
	// read handlers
	DECLARE_READ_LINE_MEMBER(do_read);  // combined DO+READY/BUSY

	// write handlers
	DECLARE_WRITE_LINE_MEMBER(cs_write);        // CS signal (active high)
	DECLARE_WRITE_LINE_MEMBER(clk_write);       // CLK signal (active high)
	DECLARE_WRITE_LINE_MEMBER(di_write);        // DI

protected:
	// subclass overrides
	virtual void parse_command_and_address() override;
};


// ======================> eeprom_serial_er5911_device

class eeprom_serial_er5911_device : public eeprom_serial_base_device
{
protected:
	// construction/destruction
	eeprom_serial_er5911_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source);

public:
	// read handlers
	DECLARE_READ_LINE_MEMBER(do_read);          // DO
	DECLARE_READ_LINE_MEMBER(ready_read);       // READY/BUSY only

	// write handlers
	DECLARE_WRITE_LINE_MEMBER(cs_write);        // CS signal (active high)
	DECLARE_WRITE_LINE_MEMBER(clk_write);       // CLK signal (active high)
	DECLARE_WRITE_LINE_MEMBER(di_write);        // DI

protected:
	// subclass overrides
	virtual void parse_command_and_address() override;
};


// ======================> eeprom_serial_x24c44_device

class eeprom_serial_x24c44_device : public eeprom_serial_base_device
{
		//async recall not implemented
		//async store not implemented
protected:
	// construction/destruction
	eeprom_serial_x24c44_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source);

public:
	// read handlers
	DECLARE_READ_LINE_MEMBER(do_read);          // DO

	// write handlers
	DECLARE_WRITE_LINE_MEMBER(cs_write);        // CS signal (active high)
	DECLARE_WRITE_LINE_MEMBER(clk_write);       // CLK signal (active high)
	DECLARE_WRITE_LINE_MEMBER(di_write);        // DI

protected:
	// subclass overrides
	virtual void parse_command_and_address() override;
	void handle_event(eeprom_event event) override;
	virtual void parse_command_and_address_2_bit();
	void execute_command() override;
	void copy_ram_to_eeprom();
	void copy_eeprom_to_ram();
	void device_start() override;
	UINT8 m_ram_length;
	UINT16 m_ram_data[16];
	UINT16 m_reading;
	UINT8 m_store_latch;
};



//**************************************************************************
//  DERIVED TYPES
//**************************************************************************

// macro for declaring a new device class
#define DECLARE_SERIAL_EEPROM_DEVICE(_baseclass, _lowercase, _uppercase, _bits) \
class eeprom_serial_##_lowercase##_##_bits##bit_device : public eeprom_serial_##_baseclass##_device \
{ \
public: \
	eeprom_serial_##_lowercase##_##_bits##bit_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock); \
}; \
extern const device_type EEPROM_SERIAL_##_uppercase##_##_bits##BIT;
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

// X24c44 8 bit 32byte ram/eeprom combo
DECLARE_SERIAL_EEPROM_DEVICE(x24c44, x24c44, X24C44, 16)
#endif
