// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    m950x0.h

    STmicro M95010/20/40 SPI-bus EEPROM

***************************************************************************/

#ifndef MAME_MACHINE_M950X0_H
#define MAME_MACHINE_M950X0_H

#pragma once

#include "machine/eeprom.h"

class m950x0_device : public eeprom_base_device
{
public:
	uint8_t access(uint8_t data);
	void select_w(int selected);

protected:
	m950x0_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, int capacity);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void process_instruction(const uint8_t insn);

	enum : uint32_t
	{
		STATE_IDLE,

		// Status-register accesses expect two sequential bytes (instruction + status)
		STATE_RDSR,
		STATE_WRSR,

		// Reads and writes expect a minimum of 3 bytes (instruction + address + data)
		STATE_READ_ADDR,
		STATE_READ_DATA,
		STATE_WRITE_ADDR,
		STATE_WRITE_DATA
	};

	enum : uint8_t
	{
		INSN_WRSR0      = 0x01,
		INSN_WRITE0     = 0x02,
		INSN_READ0      = 0x03,
		INSN_WRDI0      = 0x04,
		INSN_RDSR0      = 0x05,
		INSN_WREN0      = 0x06,

		INSN_WRSR1      = 0x09,
		INSN_WRITE1     = 0x0a,
		INSN_READ1      = 0x0b,
		INSN_WRDI1      = 0x0c,
		INSN_RDSR1      = 0x0d,
		INSN_WREN1      = 0x0e,

		STATUS_WEL_BIT  = 1
	};

	const bool m_check_a8;
	const uint16_t m_addr_mask;

	uint32_t m_state;
	bool m_selected;
	uint8_t m_status;
	uint16_t m_addr;
};

class m95010_device : public m950x0_device
{
public:
	m95010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class m95020_device : public m950x0_device
{
public:
	m95020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class m95040_device : public m950x0_device
{
public:
	m95040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(M95010, m95010_device)
DECLARE_DEVICE_TYPE(M95020, m95020_device)
DECLARE_DEVICE_TYPE(M95040, m95040_device)

#endif // MAME_MACHINE_M950X0_H
