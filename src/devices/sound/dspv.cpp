// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha DSPV, dsp used for acoustic simulation

#include "emu.h"
#include "dspv.h"

DEFINE_DEVICE_TYPE(DSPV, dspv_device, "dspv", "Yamaha DSPV audio simulation DSP (YSS217-F)")

dspv_device::dspv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, DSPV, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_prg1_config("prg1", ENDIANNESS_BIG, 64, 8, -3, address_map_constructor(FUNC(dspv_device::prg1_map), this)),
	  m_prg2_config("prg2", ENDIANNESS_BIG, 64, 8, -3, address_map_constructor(FUNC(dspv_device::prg2_map), this)),
	  m_data_config("data", ENDIANNESS_BIG, 16, 32, -1, address_map_constructor(FUNC(dspv_device::data_map), this)),
	  m_prg1(*this, "prg1"),
	  m_prg2(*this, "prg2")
{
}

void dspv_device::map(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(dspv_device::snd_r), FUNC(dspv_device::snd_w));

	map(0x02, 0x03).r(FUNC(dspv_device::status_r));
	map(0x06, 0x07).w(FUNC(dspv_device::prg_adr_w));
	map(0x12, 0x13).lw16(NAME([](u16 data) { }));
	map(0x20, 0x21).w(FUNC(dspv_device::data_adrh_w));
	map(0x22, 0x23).w(FUNC(dspv_device::data_adrl_w));
	map(0x24, 0x25).w(FUNC(dspv_device::data_data_w));
	map(0x26, 0x27).w(FUNC(dspv_device::data_zero_w));
	map(0x38, 0x39).lr16(NAME([]() -> u16 { return 0; }));
	map(0x40, 0x7f).rw(FUNC(dspv_device::prg_data_r), FUNC(dspv_device::prg_data_w));
}

void dspv_device::prg1_map(address_map &map)
{
	map(0x00, 0xff).ram().share(m_prg1);
}

void dspv_device::prg2_map(address_map &map)
{
	map(0x00, 0xff).ram().share(m_prg2);
}

void dspv_device::data_map(address_map &map)
{
	map(0x00000, 0x03fff).ram();
	map(0x1c000, 0x1dfff).ram();
}

void dspv_device::data_adrh_w(u16 data)
{
	m_data_adr = (m_data_adr & 0x0000ffff) | (data << 16);
}

void dspv_device::data_adrl_w(u16 data)
{
	m_data_adr = (m_data_adr & 0xffff0000) | data;
}

void dspv_device::data_data_w(u16 data)
{
	m_data->write_word(m_data_adr, data);
	m_data_adr++;
}

void dspv_device::data_zero_w(u16 data)
{
	logerror("data_zero_w %04x\n", data);
}

void dspv_device::prg_adr_w(u16 data)
{
	u32 slot = BIT(data, 13, 3);
	u32 len = BIT(data, 8, 5);
	u32 address = BIT(data, 0, 8);
	if(len == 0)
		len = 0x20;
	auto &prg = slot >= 4 ? m_prg2 : m_prg1;
	u32 shift = (slot & 3)*16;
	u64 mask = ~(u64(0xffff) << shift);
	for(u32 i=0; i != len; i++)
		prg[(i + address) & 0xff] = (prg[(i + address) & 0xff] & mask) | (u64(m_buffer[i]) << shift);
}

void dspv_device::prg_data_w(offs_t offset, u16 data)
{
	m_buffer[offset] = data;
}

u16 dspv_device::prg_data_r(offs_t offset)
{
	return m_buffer[offset];
}

u16 dspv_device::status_r()
{
	if(!machine().side_effects_disabled())
		m_status ^= 0xffff;
	return m_status;
}

u16 dspv_device::snd_r(offs_t offset)
{
	logerror("r %04x %s\n", offset, machine().describe_context());
	return 0;
}

void dspv_device::snd_w(offs_t offset, u16 data)
{
	logerror("w %02x, %04x %s\n", offset, data, machine().describe_context());
}

void dspv_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(0);
}

void dspv_device::device_start()
{
	m_data = &space(AS_DATA);
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	state_add(0,               "PC",        m_pc);

	set_icountptr(m_icount);

	save_item(NAME(m_pc));
	save_item(NAME(m_status));
	save_item(NAME(m_data_adr));
	save_item(NAME(m_buffer));
}

void dspv_device::device_reset()
{
	m_pc = 0;
	m_status = 0;
	m_data_adr = 0;
	std::fill(m_buffer.begin(), m_buffer.end(), 0);
}

uint32_t dspv_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t dspv_device::execute_max_cycles() const noexcept
{
	return 1;
}

void dspv_device::execute_run()
{
	if(machine().debug_flags & DEBUG_FLAG_ENABLED)
		debugger_instruction_hook(m_pc);
	m_icount = 0;
}

device_memory_interface::space_config_vector dspv_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_OPCODES, &m_prg1_config),
		std::make_pair(AS_PROGRAM, &m_prg2_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void dspv_device::state_import(const device_state_entry &entry)
{
}

void dspv_device::state_export(const device_state_entry &entry)
{
}

void dspv_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

std::unique_ptr<util::disasm_interface> dspv_device::create_disassembler()
{
	return std::make_unique<dspv_disassembler>();
}
