// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Original XaviX 6502-derived CPU core, with custom opcodes for far calls
    and interrupts and independent code/data bank latches. Graphics and
    sound are integrated into the SoC but modelled outside this CPU device.

    Banks 0x00-0x7f use low bus below 0x8000 and external bus above it. Bank
    bit 7 selects external bus throughout the 64 KiB window and is removed
    from the 23-bit address. Zero-page instructions, pointer cycles and stack
    cycles always use low bus; addresses 0xfe and 0xff expose the code and
    data bank latches.

    Known Die Markings:
    SSD 97 PA7270-107 (only seen on Ping Pong)
    SSD 98 PA7351-107
    SSD 98 PL7351-181

    TODO:
    - Confirm the physical bank-stack arrangement used by far calls and
      interrupts.

***************************************************************************/

#include "emu.h"
#include "xavix.h"
#include "xavixd.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(XAVIX, xavix_device, "xavix", "XaviX (SSD 97 / SSD 98)")

xavix_device::xavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_device(mconfig, XAVIX, tag, owner, clock, false, true)
{
}

xavix_device::xavix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool full_bank_register_decode, bool use_separate_codebank_stack) :
	m6502_device(mconfig, type, tag, owner, clock),
	m_data_address_mask(0x7fffff),
	m_full_bank_register_decode(full_bank_register_decode),
	m_lowbus_config("lowbus", ENDIANNESS_LITTLE, 8, 15),
	m_extbus_config("extbus", ENDIANNESS_LITTLE, 8, 24),
	m_interrupt_vector_callback(*this),
	m_use_separate_codebank_stack(use_separate_codebank_stack)
{
	m_program_config.m_addr_width = 24;
	m_program_config.m_logaddr_width = 24;
	m_sprogram_config.m_addr_width = 24;
	m_sprogram_config.m_logaddr_width = 24;
	// XaviX specific spaces
	m_lowbus_config.m_addr_width = 15;
	m_lowbus_config.m_logaddr_width = 15;
	m_extbus_config.m_addr_width = 24;
	m_extbus_config.m_logaddr_width = 24;
}



xavix_device::mi_xavix::mi_xavix(xavix_device *_base)
{
	m_base = _base;
}

device_memory_interface::space_config_vector xavix_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_sprogram_config),
			std::make_pair(AS_LOW_BUS, &m_lowbus_config),
			std::make_pair(AS_EXTERNAL_BUS, &m_extbus_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_LOW_BUS, &m_lowbus_config),
			std::make_pair(AS_EXTERNAL_BUS, &m_extbus_config)
		};
}

void xavix_device::device_start()
{
	if (!m_mintf)
		m_mintf = std::make_unique<mi_xavix>(this);

	// bind delegates
	m_interrupt_vector_callback.resolve();

	init();

	space(AS_LOW_BUS).specific(m_lowbus);
	space(AS_EXTERNAL_BUS).specific(m_extbus);

	state_add(XAVIX_DATABANK, "DATBNK", m_databank).formatstr("%2s");
	state_add(XAVIX_CODEBANK, "CODBNK", m_codebank).formatstr("%2s");

	save_item(NAME(m_databank));
	save_item(NAME(m_codebank));
	save_item(NAME(m_codebank_stack));
	save_item(NAME(m_codebank_stack_pointer));
}

void xavix_device::device_reset()
{
	m_codebank_stack_pointer = 0xff;
	std::ranges::fill(m_codebank_stack, 0x00);

	set_codebank(0);
	set_databank(0);
	m6502_device::device_reset();
}



std::unique_ptr<util::disasm_interface> xavix_device::create_disassembler()
{
	return std::make_unique<xavix_disassembler>();
}


offs_t xavix_device::pc_to_external(u16 pc)
{
	return code_address(pc);
}

void xavix_device::write_codebank_stack(uint32_t addr, uint8_t data)
{
	if (m_use_separate_codebank_stack)
	{
		m_codebank_stack[m_codebank_stack_pointer & 0xff] = data;
	}
	else
	{
		write_stack(addr, data);
	}
}

void xavix_device::dec_codebank_stack_pointer()
{
	if (m_use_separate_codebank_stack)
	{
		m_codebank_stack_pointer--;
	}
	else
	{
		dec_SP();
	}
}

void xavix_device::inc_codebank_stack_pointer()
{
	if (m_use_separate_codebank_stack)
	{
		m_codebank_stack_pointer++;
	}
	else
	{
		inc_SP();
	}
}

uint8_t xavix_device::read_codebank_stack(uint32_t addr)
{
	if (m_use_separate_codebank_stack)
	{
		return m_codebank_stack[m_codebank_stack_pointer & 0xff];
	}
	else
	{
		return read_stack(addr);
	}
}



uint8_t xavix_device::read_banked_data(uint32_t addr)
{
	return downcast<mi_xavix &>(*m_mintf).read_banked(addr);
}

uint8_t xavix_device::read_banked_data(uint8_t databank, uint16_t adr)
{
	return read_banked_data((uint32_t(databank) << 16) | adr);
}

uint8_t xavix_device::read_banked_data_raw(uint32_t addr)
{
	return read_banked_data_raw(BIT(addr, 16, 8), addr & 0xffff);
}

inline uint8_t xavix_device::read_banked_data_raw(uint8_t databank, uint16_t adr)
{
	if (databank < 0x80)
	{
		if (adr < 0x8000)
		{
			if (adr == 0xfe && (!m_full_bank_register_decode || databank == 0))
			{
				return m_codebank;
			}
			else if (adr == 0xff && (!m_full_bank_register_decode || databank == 0))
			{
				return m_databank;
			}

			return m_lowbus.read_interruptible(adr);
		}
		else
		{
			return m_extbus.read_interruptible(((databank << 16) | adr) & m_data_address_mask);
		}
	}
	else
	{
		return m_extbus.read_interruptible(((databank << 16) | adr) & m_data_address_mask);
	}
}

// data reads
inline uint8_t xavix_device::mi_xavix::read(uint16_t adr)
{
	uint8_t databank = m_base->databank();
	return read_banked((uint32_t(databank) << 16) | adr);
}

uint8_t xavix_device::mi_xavix::read_banked(uint32_t adr)
{
	return m_base->read_banked_data_raw(adr);
}

// opcode reads
uint8_t xavix_device::mi_xavix::read_sync(uint16_t adr)
{
	if (m_base->m_full_bank_register_decode && m_base->codebank() == 0 && (adr == 0xfe || adr == 0xff))
		return m_base->read_zeropage(adr);

	return m_csprogram.read_interruptible(m_base->code_address(adr));
}

// oprand reads
uint8_t xavix_device::mi_xavix::read_arg(uint16_t adr)
{
	if (m_base->m_full_bank_register_decode && m_base->codebank() == 0 && (adr == 0xfe || adr == 0xff))
		return m_base->read_zeropage(adr);

	return m_cprogram.read_interruptible(m_base->code_address(adr));
}

// data writes
void xavix_device::write_banked_data(uint32_t addr, uint8_t val)
{
	downcast<mi_xavix &>(*m_mintf).write_banked(addr, val);
}

void xavix_device::write_banked_data(uint8_t databank, uint16_t adr, uint8_t val)
{
	write_banked_data((uint32_t(databank) << 16) | adr, val);
}

void xavix_device::write_banked_data_raw(uint32_t addr, uint8_t val)
{
	write_banked_data_raw(BIT(addr, 16, 8), addr & 0xffff, val);
}

uint8_t xavix_device::read_stack(uint32_t addr)
{
	// address is always 0x100-0x1ff
	return read_banked_data((addr & 0xff) + 0x100);
}

void xavix_device::write_stack(uint32_t addr, uint8_t val)
{
	// address is always 0x100-0x1ff
	write_banked_data((addr & 0xff) + 0x100, val);
}

uint8_t xavix_device::read_zeropage(uint32_t addr)
{
	// address is always 0x00-0xff
	return read_banked_data(addr & 0xff);
}

void xavix_device::write_zeropage(uint32_t addr, uint8_t val)
{
	// address is always 0x00-0xff
	write_banked_data(addr & 0xff, val);
}


// data writes
inline void xavix_device::write_banked_data_raw(uint8_t databank, uint16_t adr, uint8_t val)
{
	if (databank < 0x80)
	{
		if (adr < 0x8000)
		{
			if (adr == 0xfe && (!m_full_bank_register_decode || databank == 0))
			{
				m_codebank = val;
				return;
			}
			else if (adr == 0xff && (!m_full_bank_register_decode || databank == 0))
			{
				m_databank = val;
				return;
			}

			m_lowbus.write_interruptible(adr, val);
		}
		else
		{
			m_extbus.write_interruptible(((databank << 16) | adr) & m_data_address_mask, val);
		}
	}
	else
	{
		m_extbus.write_interruptible(((databank << 16) | adr) & m_data_address_mask, val);
	}
}

void xavix_device::mi_xavix::write(uint16_t adr, uint8_t val)
{
	uint8_t databank = m_base->databank();
	write_banked((uint32_t(databank) << 16) | adr, val);
}

void xavix_device::mi_xavix::write_banked(uint32_t adr, uint8_t val)
{
	m_base->write_banked_data_raw(adr, val);
}

void xavix_device::set_codebank(uint8_t bank)
{
	m_codebank = bank;
}

inline uint8_t xavix_device::codebank()
{
	return m_codebank;
}

inline void xavix_device::set_databank(uint8_t bank)
{
	m_databank = bank;
}

inline uint8_t xavix_device::databank()
{
	return m_databank;
}



void xavix_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	m6502_device::state_string_export(entry, str);

	switch(entry.index())
	{
	case XAVIX_DATABANK:
		str = string_format("%02x", m_databank);
		break;
	case XAVIX_CODEBANK:
		str = string_format("%02x", m_codebank);
		break;
	}
}



#include "cpu/m6502/xavix.hxx"
