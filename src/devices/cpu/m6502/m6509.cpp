// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6509.cpp

    6502 with banking and extended address bus

***************************************************************************/

#include "emu.h"
#include "m6509.h"
#include "m6509d.h"

DEFINE_DEVICE_TYPE(M6509, m6509_device, "m6509", "MOS Technology 6509")

m6509_device::m6509_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6509, tag, owner, clock), m_bank_i(0), m_bank_y(0)
{
	m_program_config.m_addr_width = 20;
	m_program_config.m_logaddr_width = 20;
	m_sprogram_config.m_addr_width = 20;
	m_sprogram_config.m_logaddr_width = 20;
}

void m6509_device::device_start()
{
	m_mintf = std::make_unique<mi_6509>(this);

	init();

	state_add(STATE_GENPC, "GENPC", m_XPC).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_XPC).callexport().noshow();
	state_add(M6509_BI, "BI", m_bank_i);
	state_add(M6509_BY, "BY", m_bank_y);
}

void m6509_device::device_reset()
{
	m6502_device::device_reset();
	m_bank_i = 0x0f;
	m_bank_y = 0x0f;
}

offs_t m6509_device::pc_to_external(u16 pc)
{
	return adr_in_bank_i(pc);
}

std::unique_ptr<util::disasm_interface> m6509_device::create_disassembler()
{
	return std::make_unique<m6509_disassembler>();
}

m6509_device::mi_6509::mi_6509(m6509_device *_base)
{
	m_base = _base;
}

uint8_t m6509_device::mi_6509::read(uint16_t adr)
{
	uint8_t res = m_program.read_byte(m_base->adr_in_bank_i(adr));
	if(adr == 0x0000)
		res = m_base->bank_i_r();
	else if(adr == 0x0001)
		res = m_base->bank_y_r();
	return res;
}

uint8_t m6509_device::mi_6509::read_sync(uint16_t adr)
{
	uint8_t res = m_csprogram.read_byte(m_base->adr_in_bank_i(adr));
	if(adr == 0x0000)
		res = m_base->bank_i_r();
	else if(adr == 0x0001)
		res = m_base->bank_y_r();
	return res;
}

uint8_t m6509_device::mi_6509::read_arg(uint16_t adr)
{
	uint8_t res = m_cprogram.read_byte(m_base->adr_in_bank_i(adr));
	if(adr == 0x0000)
		res = m_base->bank_i_r();
	else if(adr == 0x0001)
		res = m_base->bank_y_r();
	return res;
}

uint8_t m6509_device::mi_6509::read_9(uint16_t adr)
{
	uint8_t res = m_program.read_byte(m_base->adr_in_bank_y(adr));
	if(adr == 0x0000)
		res = m_base->bank_i_r();
	else if(adr == 0x0001)
		res = m_base->bank_y_r();
	return res;
}

void m6509_device::mi_6509::write(uint16_t adr, uint8_t val)
{
	m_program.write_byte(m_base->adr_in_bank_i(adr), val);
	if(adr == 0x0000)
		m_base->bank_i_w(val);
	else if(adr == 0x0001)
		m_base->bank_y_w(val);
}

void m6509_device::mi_6509::write_9(uint16_t adr, uint8_t val)
{
	m_program.write_byte(m_base->adr_in_bank_y(adr), val);
	if(adr == 0x0000)
		m_base->bank_i_w(val);
	else if(adr == 0x0001)
		m_base->bank_y_w(val);
}

#include "cpu/m6502/m6509.hxx"
