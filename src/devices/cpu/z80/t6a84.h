// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Toshiba T6A84, TLCS-Z80 ASSP Family

    Unknown specs. Disassembled code suggests that this processor uses
    separate code and data address spaces. Mapped pages on each space are
    configured by writing to I/O ports. Values 0 to 7 map the corresponding
    0x10000 sized page from ROM offset 0 to 0x7ffff, while value 8 seems to
    map full contents of RAM.

    Pinout: https://patents.google.com/patent/CN2280961Y/en

***************************************************************************/

#ifndef MAME_CPU_Z80_T6A84_H
#define MAME_CPU_Z80_T6A84_H

#pragma once

#include "z80.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class t6a84_device : public z80_device
{
public:
	enum address_spaces : uint8_t
	{
		AS_STACK = AS_OPCODES + 1
	};

	t6a84_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t code_address(uint16_t address);
	uint32_t data_address(uint16_t address);
	uint32_t stack_address(uint16_t address);

protected:
	static inline constexpr uint32_t PAGE_SIZE = 0x10000;

	t6a84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor io_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// z80 overrides
	virtual uint8_t stack_read(uint16_t addr) override;
	virtual void stack_write(uint16_t addr, uint8_t value) override;

	uint8_t code_page_r();
	uint8_t data_page_r();
	uint8_t stack_page_r();
	uint8_t vector_page_r();
	void code_page_w(uint8_t page);
	void data_page_w(uint8_t page);
	void stack_page_w(uint8_t page);
	void vector_page_w(uint8_t page);

	void internal_io_map(address_map &map) const;
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	const address_space_config m_program_space_config;
	const address_space_config m_data_space_config;
	const address_space_config m_stack_space_config;
	const address_space_config m_io_space_config;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_stack;

private:
	uint8_t m_code_page;
	uint8_t m_delay_code_page;
	bool m_is_delay_code_page_set;
	uint8_t m_prev_code_page;
	uint8_t m_data_page;
	uint8_t m_stack_page;
	uint8_t m_vector_page;
};


// device type definition
DECLARE_DEVICE_TYPE(T6A84, t6a84_device)


#endif // MAME_CPU_Z80_T6A84_H
