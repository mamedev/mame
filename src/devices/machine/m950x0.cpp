// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    m950x0.cpp

    STmicro M95010/20/40 SPI-bus EEPROM

    Common characteristics:
    - 16-byte page size
    - Write protection selectable in quarter, half, or full sizes

    Part variants with a -DF designation have additional support for an
    identification page, which is not currently emulated.

    Sizes:
    M95010 - 1kbit
    M95020 - 2kbit
    M95040 - 4kbit, slightly altered instructions for 9th address bit

    Current issues:
    - Implementation currently operates in a parallel manner, rather than
      serial.

***************************************************************************/

#include "emu.h"
#include "m950x0.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(M95010, m95010_device, "m95010", "STmicro M95010 1kbit SPI EEPROM")
DEFINE_DEVICE_TYPE(M95020, m95020_device, "m95020", "STmicro M95020 2kbit SPI EEPROM")
DEFINE_DEVICE_TYPE(M95040, m95040_device, "m95040", "STmicro M95040 4kbit SPI EEPROM")

m950x0_device::m950x0_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, int capacity)
	: eeprom_base_device(mconfig, type, tag, owner)
	, m_check_a8(capacity > 0x100)
	, m_addr_mask((uint16_t)capacity - 1)
	, m_state(STATE_IDLE)
{
	size(capacity, 8);
}

m95010_device::m95010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m950x0_device(mconfig, M95010, tag, owner, 0x80)
{
}

m95020_device::m95020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m950x0_device(mconfig, M95020, tag, owner, 0x100)
{
}

m95040_device::m95040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m950x0_device(mconfig, M95040, tag, owner, 0x200)
{
}

void m950x0_device::device_start()
{
	eeprom_base_device::device_start();

	save_item(NAME(m_state));
	save_item(NAME(m_selected));
	save_item(NAME(m_status));
	save_item(NAME(m_addr));
}

void m950x0_device::device_reset()
{
	eeprom_base_device::device_reset();

	m_state = STATE_IDLE;
	m_selected = false;
	m_status = 0xf0;
	m_addr = 0;
}

uint8_t m950x0_device::access(uint8_t data)
{
	if (!m_selected)
		return 0;

	uint8_t response = 0;

	switch (m_state)
	{
	case STATE_IDLE:
		process_instruction(data);
		break;

	case STATE_RDSR:
		response = m_status;
		m_state = STATE_IDLE;
		LOG("Status Register read, value: %02x\n", response);
		break;

	case STATE_WRSR:
		m_status &= 0xf0;
		m_status |= data & 0x0f;
		m_state = STATE_IDLE;
		LOG("Status Register write, new value: %02x\n", m_status);
		break;

	case STATE_READ_ADDR:
		m_addr |= data;
		m_state = STATE_READ_DATA;
		LOG("Read command, starting address: %03x, entering read-data state\n", m_addr);
		break;

	case STATE_WRITE_ADDR:
		m_addr |= data;
		m_state = STATE_WRITE_DATA;
		LOG("Write command, starting address: %03x, entering write-data state\n", m_addr);
		break;

	case STATE_READ_DATA:
		response = (uint8_t)internal_read(m_addr);
		LOG("Read command, data: %02x, at address %03x\n", response, m_addr);
		m_addr++;
		m_addr &= m_addr_mask;
		break;

	case STATE_WRITE_DATA:
		internal_write(m_addr, data);
		LOG("Write command, data: %02x to address %03x\n", data, m_addr);
		m_addr++;
		m_addr &= m_addr_mask;
		break;
	}

	return response;
}

void m950x0_device::process_instruction(const uint8_t insn)
{
	switch (insn)
	{
	case INSN_WREN0:
	case INSN_WREN1:
		LOG("Instruction: Write enable\n");
		m_status |= (1 << STATUS_WEL_BIT);
		break;

	case INSN_WRDI0:
	case INSN_WRDI1:
		LOG("Instruction: Write disable\n");
		m_status &= ~(1 << STATUS_WEL_BIT);
		break;

	case INSN_RDSR0:
	case INSN_RDSR1:
		LOG("Instruction: Read status register\n");
		m_state = STATE_RDSR;
		break;

	case INSN_WRSR0:
	case INSN_WRSR1:
		LOG("Instruction: Write status register\n");
		m_state = STATE_WRSR;
		break;

	case INSN_READ0:
		LOG("Instruction: Read, A8=0\n");
		m_state = STATE_READ_ADDR;
		break;
	case INSN_READ1:
		LOG("Instruction: Read, A8=1\n");
		m_state = STATE_READ_ADDR;
		if (m_check_a8)
			m_addr |= 0x100;
		break;

	case INSN_WRITE0:
		LOG("Instruction: Write, A8=0\n");
		m_state = STATE_WRITE_ADDR;
		break;
	case INSN_WRITE1:
		LOG("Instruction: Write, A8=1\n");
		m_state = STATE_WRITE_ADDR;
		if (m_check_a8)
			m_addr |= 0x100;
		break;

	default:
		LOG("Unrecognized instruction byte: %02x, deselecting\n", insn);
		m_selected = false;
		break;
	}
}

void m950x0_device::select_w(int selected)
{
	if (m_selected == (bool)selected)
		return;

	m_selected = (bool)selected;

	if (!selected)
	{
		LOG("Deselected, resetting address to 0 and entering idle state.\n");
		m_state = STATE_IDLE;
		m_addr = 0;
	}
}
