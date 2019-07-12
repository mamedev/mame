// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha MEG - Multiple effects generator
//
// Audio dsp dedicated to effects generation

#include "emu.h"
#include "debugger.h"
#include "meg.h"

DEFINE_DEVICE_TYPE(MEG, meg_device, "meg", "Multiple Effects Generator (HD62098 / XM309A00)")
DEFINE_DEVICE_TYPE(MEGEMB, meg_embedded_device, "megemb", "Multiple Effects Generator (embedded)")

void meg_base_device::prg_map(address_map &map)
{
	map(0, m_prg_size - 1).ram();
}

void meg_base_device::fp_map(address_map &map)
{
	map(0, m_prg_size - 1).ram();
}

void meg_base_device::offsets_map(address_map &map)
{
	map(0, 0x7f).ram();
}

meg_base_device::meg_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u32 prg_size) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_BIG, 64, prg_size > 256 ? 9 : 8, -3, address_map_constructor(FUNC(meg_base_device::prg_map), this)),
	m_fp_config("fp", ENDIANNESS_BIG, 16, prg_size > 256 ? 9 : 8, -1, address_map_constructor(FUNC(meg_base_device::fp_map), this)),
	m_offsets_config("offsets", ENDIANNESS_BIG, 16, prg_size > 256 ? 7 : 7, -1, address_map_constructor(FUNC(meg_base_device::offsets_map), this)),
	m_prg_size(prg_size)
{
}


void meg_base_device::prg_w(u16 address, u64 opcode)
{
	m_program->write_qword(address, opcode);
}

void meg_base_device::fp_w(u16 address, u16 value)
{
	m_fp->write_word(address, value);
}

void meg_base_device::offset_w(u16 address, u16 value)
{
	m_offsets->write_word(address, value);
}

void meg_base_device::lfo_w(u8 reg, u16 value)
{
	m_lfo[reg] = value;

	static const int dt[8] = { 0, 32, 64, 128, 256, 512,  1024, 2048 };
	static const int sh[8] = { 0,  0,  1,   2,   3,   4,     5,    6 };

	int scale = (value >> 5) & 7;
	int step = ((value & 31) << sh[scale]) + dt[scale];
	logerror("lfo_w %02x freq=%5.2f phase=%6.4f\n", reg, step * 44100.0/4194304, (value >> 8)/256.0);
}

void meg_base_device::map_w(u8 reg, u16 value)
{
	m_map[reg] = value;
	logerror("map %d: start = %06x size = %06x extra = %x\n", reg, (value & 0xff) << 10, 1 << (10 + ((value & 0x0700) >> 8)), (value & 0xf800) >> 11);
}

u64 meg_base_device::prg_r(u16 address) const
{
	return m_program->read_qword(address);
}

u16 meg_base_device::fp_r(u16 address) const
{
	return m_fp->read_word(address);
}

u16 meg_base_device::offset_r(u16 address) const
{
	return m_offsets->read_word(address);
}

u16 meg_base_device::lfo_r(u8 reg) const
{
	return m_lfo[reg];
}

u16 meg_base_device::map_r(u8 reg) const
{
	return m_map[reg];
}


void meg_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_fp      = &space(AS_FP);
	m_offsets = &space(AS_OFFSETS);

	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	state_add(0,               "PC",        m_pc);

	set_icountptr(m_icount);

	save_item(NAME(m_lfo));
	save_item(NAME(m_map));
	save_item(NAME(m_pc));
}

void meg_base_device::device_reset()
{
	memset(m_lfo, 0, sizeof(m_lfo));
	memset(m_map, 0, sizeof(m_map));
	m_pc = 0;
}

uint32_t meg_base_device::execute_min_cycles() const
{
	return 1;
}

uint32_t meg_base_device::execute_max_cycles() const
{
	return 1;
}

uint32_t meg_base_device::execute_input_lines() const
{
	return 0;
}

void meg_base_device::execute_run()
{
	if(machine().debug_flags & DEBUG_FLAG_ENABLED)
		debugger_instruction_hook(m_pc);
	m_icount = 0;
}

device_memory_interface::space_config_vector meg_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_FP,      &m_fp_config),
		std::make_pair(AS_OFFSETS, &m_offsets_config)
	};
}

void meg_base_device::state_import(const device_state_entry &entry)
{
}

void meg_base_device::state_export(const device_state_entry &entry)
{
}

void meg_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

std::unique_ptr<util::disasm_interface> meg_base_device::create_disassembler()
{
	return std::make_unique<meg_disassembler>(this);
}

meg_embedded_device::meg_embedded_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	meg_base_device(mconfig, MEGEMB, tag, owner, clock, 384)
{
}

meg_device::meg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	meg_base_device(mconfig, MEG, tag, owner, clock, 256)
{
}


//   vl70:
// 6d1e: write 1, r0l
// 6d26: write 2, r0l
// 6d2e: read 2
// 6d36: write 3, r0l
// 6d3e: write reg 4:r0h, r0l
// 6d52: write reg 5:r0h, r0l-1
// 6d68: write 7, r0l
// 6d70: write reg 8:r0h, r0l
// 6d84: write reg 9:r0h, r0l
// 6dac: write a, r0l
// 6db4: write reg cd:r1l, r0
// 6dd4: write reg e:r0h, r0l
// 6dee: write reg f:r0h, r0l
// 6e08: read 10,11
// 6e1c: write reg 1213:r1l, r0
// 6e3c: write reg 14:r0h, r0l
// 6e50: write 15, r0l
// 6e58: write reg 16:r0h, r0l
// 6e6c: write reg 17:r0h, r0l
// 6e80: write reg 18:e0h, e0l

void meg_device::map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(meg_device::select_w));
	map(0x01, 0x01).w(FUNC(meg_device::s1_w));
	map(0x02, 0x02).rw(FUNC(meg_device::s2_r), FUNC(meg_device::s2_w));
	map(0x03, 0x03).w(FUNC(meg_device::s3_w));
	map(0x04, 0x04).w(FUNC(meg_device::s4_w));
	map(0x05, 0x05).w(FUNC(meg_device::s5_w));
	map(0x07, 0x07).w(FUNC(meg_device::s7_w));
	map(0x08, 0x08).w(FUNC(meg_device::s8_w));
	map(0x09, 0x09).w(FUNC(meg_device::s9_w));
	map(0x0a, 0x0a).w(FUNC(meg_device::sa_w));
	map(0x0c, 0x0c).w(FUNC(meg_device::fph_w));
	map(0x0d, 0x0d).w(FUNC(meg_device::fpl_w));
	map(0x0e, 0x0e).w(FUNC(meg_device::se_w));
	map(0x0f, 0x0f).w(FUNC(meg_device::sf_w));
	map(0x10, 0x10).r(FUNC(meg_device::s10_r));
	map(0x11, 0x11).r(FUNC(meg_device::s11_r));
	map(0x12, 0x12).w(FUNC(meg_device::offseth_w));
	map(0x13, 0x13).w(FUNC(meg_device::offsetl_w));
	map(0x14, 0x14).w(FUNC(meg_device::s14_w));
	map(0x15, 0x15).w(FUNC(meg_device::s15_w));
	map(0x16, 0x16).w(FUNC(meg_device::s16_w));
	map(0x17, 0x17).w(FUNC(meg_device::s17_w));
	map(0x18, 0x18).w(FUNC(meg_device::s18_w));
}

u8 meg_device::s2_r()
{
	logerror("read r2 %s\n", machine().describe_context());
	return 0x00;
}

void meg_device::select_w(u8 data)
{
	m_reg = data;
}

void meg_device::s1_w(u8 data)
{
	logerror("r1 %02x %s\n", data, machine().describe_context());
}

void meg_device::s2_w(u8 data)
{
	logerror("r2 %02x %s\n", data, machine().describe_context());
}

void meg_device::s3_w(u8 data)
{
	logerror("r3 %02x %s\n", data, machine().describe_context());
}

void meg_device::s4_w(u8 data)
{
	if(m_r4[m_reg] != data) {
		m_r4[m_reg] = data;
		logerror("r4[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s5_w(u8 data)
{
	if(m_r5[m_reg] != data) {
		m_r5[m_reg] = data;
		logerror("r5[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s7_w(u8 data)
{
	logerror("r7 %02x %s\n", data, machine().describe_context());
}

void meg_device::s8_w(u8 data)
{
	if(m_r8[m_reg] != data) {
		m_r8[m_reg] = data;
		logerror("r8[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}


void meg_device::s9_w(u8 data)
{
	if(m_r9[m_reg] != data) {
		m_r9[m_reg] = data;
		logerror("r9[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::sa_w(u8 data)
{
	logerror("ra %02x %s\n", data, machine().describe_context());
}

void meg_device::fph_w(u8 data)
{
	fp_w(m_reg, (fp_r(m_reg) & 0x00ff) | (data << 8));
}


void meg_device::fpl_w(u8 data)
{
	fp_w(m_reg, (fp_r(m_reg) & 0xff00) | data);
}

void meg_device::se_w(u8 data)
{
	if(m_re[m_reg] != data) {
		m_re[m_reg] = data;
		logerror("re[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}


void meg_device::sf_w(u8 data)
{
	if(m_rf[m_reg] != data) {
		m_rf[m_reg] = data;
		logerror("rf[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

u8 meg_device::s10_r()
{
	logerror("read r10 %s\n", machine().describe_context());
	return 0x00;
}

u8 meg_device::s11_r()
{
	logerror("read r11 %s\n", machine().describe_context());
	return 0x00;
}

void meg_device::offseth_w(u8 data)
{
	offset_w(m_reg, (offset_r(m_reg) & 0x00ff) | (data << 8));
}

void meg_device::offsetl_w(u8 data)
{
	offset_w(m_reg, (offset_r(m_reg) & 0xff00) | data);
}

void meg_device::s14_w(u8 data)
{
	if(m_r14[m_reg] != data) {
		m_r14[m_reg] = data;
		logerror("r14[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s15_w(u8 data)
{
	logerror("r15 %02x %s\n", data, machine().describe_context());
}

void meg_device::s16_w(u8 data)
{
	if(m_r16[m_reg] != data) {
		m_r16[m_reg] = data;
		logerror("r16[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s17_w(u8 data)
{
	if(m_r17[m_reg] != data) {
		m_r17[m_reg] = data;
		logerror("r17[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s18_w(u8 data)
{
	if(m_r18[m_reg] != data) {
		m_r18[m_reg] = data;
		logerror("r18[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}
