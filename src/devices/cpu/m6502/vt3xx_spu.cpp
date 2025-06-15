// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    vt3xx_spu.cpp

    6502 derived sound/sub CPU for VT3xx series chips

	has some additional opcodes for reading/writing the 24-bit 'onebus'
	with an extra register to store the upper 8-bits of the access address

	- should this instead derive from the rp2a03 as is the case for the
	  main CPUs on many of the VT chips, or is using the m6502 as a base
	  correct in this case?

***************************************************************************/

#include "emu.h"
#include "vt3xx_spu.h"
#include "vt3xx_spud.h"

DEFINE_DEVICE_TYPE(VT3XX_SPU, vt3xx_spu_device, "vt3xx_spu", "VT3xx SPU")

vt3xx_spu_device::vt3xx_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vt3xx_spu_device(mconfig, VT3XX_SPU, tag, owner, clock)
{
}

vt3xx_spu_device::vt3xx_spu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock),
	m_extdata_config("extdata", ENDIANNESS_LITTLE, 8, 24)
{
}


std::unique_ptr<util::disasm_interface> vt3xx_spu_device::create_disassembler()
{
	return std::make_unique<vt3xx_spu_disassembler>();
}

void vt3xx_spu_device::device_start()
{
	m6502_device::device_start();

	m_extdata_space = &space(5);

	state_add(VT3XX_SPU_DATABANK, "DATBNK", m_databank).callimport().formatstr("%2s");

	save_item(NAME(m_databank));
}

void vt3xx_spu_device::device_reset()
{
	set_databank(0);
	m6502_device::device_reset();
}


inline void vt3xx_spu_device::set_databank(uint8_t bank)
{
	m_databank = bank;
}

inline uint8_t vt3xx_spu_device::get_databank()
{
	return m_databank;
}

device_memory_interface::space_config_vector vt3xx_spu_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_OPCODES, &sprogram_config),
			std::make_pair(5, &m_extdata_config),
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(5, &m_extdata_config),
		};
}

void vt3xx_spu_device::state_import(const device_state_entry &entry)
{
	m6502_device::state_import(entry);

	switch(entry.index())
	{
	case VT3XX_SPU_DATABANK:
		break;
	}
}

void vt3xx_spu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	m6502_device::state_string_export(entry, str);

	switch(entry.index())
	{
	case VT3XX_SPU_DATABANK:
		str = string_format("%02x", m_databank);
		break;
	}
}

#include "cpu/m6502/vt3xx_spu.hxx"
