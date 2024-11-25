// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Nathan Gilbert

#include "emu.h"
#include "xavix2.h"
#include "xavix2d.h"

DEFINE_DEVICE_TYPE(XAVIX2, xavix2_device, "xavix2", "Xavix 2 CPU")

const u8 xavix2_device::bpo[8] = { 4, 3, 3, 2, 2, 2, 2, 1 };

xavix2_device::xavix2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, XAVIX2, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32)
{
}

void xavix2_device::device_start()
{
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_PROGRAM).specific(m_program);

	state_add(STATE_GENPC,     "GENPC",     m_pc).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_hr[4]).callimport().formatstr("%5s").noshow();
	state_add(XAVIX2_PC,       "PC",        m_pc).callimport();
	state_add(XAVIX2_FLAGS,    "FLAGS",     m_hr[4]).callimport();
	state_add(XAVIX2_R0,       "R0",        m_r[0]);
	state_add(XAVIX2_R1,       "R1",        m_r[1]);
	state_add(XAVIX2_R2,       "R2",        m_r[2]);
	state_add(XAVIX2_R3,       "R3",        m_r[3]);
	state_add(XAVIX2_R4,       "R4",        m_r[4]);
	state_add(XAVIX2_R5,       "R5",        m_r[5]);
	state_add(XAVIX2_SP,       "SP",        m_r[6]);
	state_add(XAVIX2_LR,       "LR",        m_r[7]);
	state_add(XAVIX2_ILR1,     "ILR1",      m_ilr1);
	state_add(XAVIX2_IF1,      "IF1",       m_if1);
	state_add(XAVIX2_HR00,     "HR00",      m_hr[0x00]);
	state_add(XAVIX2_HR01,     "HR01",      m_hr[0x01]);
	state_add(XAVIX2_HR02,     "HR02",      m_hr[0x02]);
	state_add(XAVIX2_HR03,     "HR03",      m_hr[0x03]);
	state_add(XAVIX2_HR04,     "HR04",      m_hr[0x04]);
	state_add(XAVIX2_HR05,     "HR05",      m_hr[0x05]);
	state_add(XAVIX2_HR06,     "HR06",      m_hr[0x06]);
	state_add(XAVIX2_HR07,     "HR07",      m_hr[0x07]);
	state_add(XAVIX2_HR08,     "HR08",      m_hr[0x08]);
	state_add(XAVIX2_HR09,     "HR09",      m_hr[0x09]);
	state_add(XAVIX2_HR0A,     "HR0A",      m_hr[0x0a]);
	state_add(XAVIX2_HR0B,     "HR0B",      m_hr[0x0b]);
	state_add(XAVIX2_HR0C,     "HR0C",      m_hr[0x0c]);
	state_add(XAVIX2_HR0D,     "HR0D",      m_hr[0x0d]);
	state_add(XAVIX2_HR0E,     "HR0E",      m_hr[0x0e]);
	state_add(XAVIX2_HR0F,     "HR0F",      m_hr[0x0f]);
	state_add(XAVIX2_HR10,     "HR10",      m_hr[0x10]);
	state_add(XAVIX2_HR11,     "HR11",      m_hr[0x11]);
	state_add(XAVIX2_HR12,     "HR12",      m_hr[0x12]);
	state_add(XAVIX2_HR13,     "HR13",      m_hr[0x13]);
	state_add(XAVIX2_HR14,     "HR14",      m_hr[0x14]);
	state_add(XAVIX2_HR15,     "HR15",      m_hr[0x15]);
	state_add(XAVIX2_HR16,     "HR16",      m_hr[0x16]);
	state_add(XAVIX2_HR17,     "HR17",      m_hr[0x17]);
	state_add(XAVIX2_HR18,     "HR18",      m_hr[0x18]);
	state_add(XAVIX2_HR19,     "HR19",      m_hr[0x19]);
	state_add(XAVIX2_HR1A,     "HR1A",      m_hr[0x1a]);
	state_add(XAVIX2_HR1B,     "HR1B",      m_hr[0x1b]);
	state_add(XAVIX2_HR1C,     "HR1C",      m_hr[0x1c]);
	state_add(XAVIX2_HR1D,     "HR1D",      m_hr[0x1d]);
	state_add(XAVIX2_HR1E,     "HR1E",      m_hr[0x1e]);
	state_add(XAVIX2_HR1F,     "HR1F",      m_hr[0x1f]);
	state_add(XAVIX2_HR20,     "HR20",      m_hr[0x20]);
	state_add(XAVIX2_HR21,     "HR21",      m_hr[0x21]);
	state_add(XAVIX2_HR22,     "HR22",      m_hr[0x22]);
	state_add(XAVIX2_HR23,     "HR23",      m_hr[0x23]);
	state_add(XAVIX2_HR24,     "HR24",      m_hr[0x24]);
	state_add(XAVIX2_HR25,     "HR25",      m_hr[0x25]);
	state_add(XAVIX2_HR26,     "HR26",      m_hr[0x26]);
	state_add(XAVIX2_HR27,     "HR27",      m_hr[0x27]);
	state_add(XAVIX2_HR28,     "HR28",      m_hr[0x28]);
	state_add(XAVIX2_HR29,     "HR29",      m_hr[0x29]);
	state_add(XAVIX2_HR2A,     "HR2A",      m_hr[0x2a]);
	state_add(XAVIX2_HR2B,     "HR2B",      m_hr[0x2b]);
	state_add(XAVIX2_HR2C,     "HR2C",      m_hr[0x2c]);
	state_add(XAVIX2_HR2D,     "HR2D",      m_hr[0x2d]);
	state_add(XAVIX2_HR2E,     "HR2E",      m_hr[0x2e]);
	state_add(XAVIX2_HR2F,     "HR2F",      m_hr[0x2f]);
	state_add(XAVIX2_HR30,     "HR30",      m_hr[0x30]);
	state_add(XAVIX2_HR31,     "HR31",      m_hr[0x31]);
	state_add(XAVIX2_HR32,     "HR32",      m_hr[0x32]);
	state_add(XAVIX2_HR33,     "HR33",      m_hr[0x33]);
	state_add(XAVIX2_HR34,     "HR34",      m_hr[0x34]);
	state_add(XAVIX2_HR35,     "HR35",      m_hr[0x35]);
	state_add(XAVIX2_HR36,     "HR36",      m_hr[0x36]);
	state_add(XAVIX2_HR37,     "HR37",      m_hr[0x37]);
	state_add(XAVIX2_HR38,     "HR38",      m_hr[0x38]);
	state_add(XAVIX2_HR39,     "HR39",      m_hr[0x39]);
	state_add(XAVIX2_HR3A,     "HR3A",      m_hr[0x3a]);
	state_add(XAVIX2_HR3B,     "HR3B",      m_hr[0x3b]);
	state_add(XAVIX2_HR3C,     "HR3C",      m_hr[0x3c]);
	state_add(XAVIX2_HR3D,     "HR3D",      m_hr[0x3d]);
	state_add(XAVIX2_HR3E,     "HR3E",      m_hr[0x3e]);
	state_add(XAVIX2_HR3F,     "HR3F",      m_hr[0x3f]);


	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_ilr1));
	save_item(NAME(m_if1));
	save_item(NAME(m_hr));
	save_item(NAME(m_int_line));
	save_item(NAME(m_wait));
	save_item(NAME(m_ei_count));

	set_icountptr(m_icount);

	m_pc = 0;
	m_int_line = false;
	m_wait = false;

	memset(m_r, 0, sizeof(m_r));
	memset(m_hr, 0, sizeof(m_hr));
}

void xavix2_device::device_reset()
{
	m_hr[4] = 0;
	m_pc = 0x40000000;
}

uint32_t xavix2_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t xavix2_device::execute_max_cycles() const noexcept
{
	return 5;
}

u32 xavix2_device::check_interrupt(u32 cpc)
{
	if(m_int_line && ((m_hr[4] & F_I) || m_wait)) {
		m_ilr1 = m_wait ? cpc + 1 : cpc;
		standard_irq_callback(0, m_ilr1);
		m_wait = false;
		m_ei_count = 0;
		m_if1 = m_hr[4];
		m_hr[4] &= ~F_I;
		return 0x40000010;
	}
	return cpc;
}

void xavix2_device::execute_set_input(int inputnum, int state)
{
	m_int_line = state;
}

device_memory_interface::space_config_vector xavix2_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

std::unique_ptr<util::disasm_interface> xavix2_device::create_disassembler()
{
	return std::make_unique<xavix2_disassembler>();
}

void xavix2_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c%c",
								  m_hr[4] & F_I ? 'I' : '-',
								  m_hr[4] & F_V ? 'V' : '-',
								  m_hr[4] & F_C ? 'C' : '-',
								  m_hr[4] & F_N ? 'N' : '-',
								  m_hr[4] & F_Z ? 'Z' : '-');
		break;
	}
}

void xavix2_device::execute_run()
{
	if(!m_ei_count)
		m_pc = check_interrupt(m_pc);
	while(m_icount > 0) {
		if(m_ei_count) {
			m_ei_count--;
			if(!m_ei_count)
				m_pc = check_interrupt(m_pc);
		}
		if(machine().debug_flags & DEBUG_FLAG_ENABLED)
			debugger_instruction_hook(m_pc);

		u32 opcode = m_program_cache.read_byte(m_pc) << 24;
		m_icount --;

		u8 nb = bpo[opcode >> 29];
		u32 npc = m_pc + nb;
		for(u8 i=1; i != nb; i++) {
			opcode |= m_program_cache.read_byte(m_pc + i) << (24 - 8*i);
			m_icount --;
		}

		switch(opcode >> 24) {
		case 0x00: case 0x01: m_r[r1(opcode)] = do_add(m_r[r2(opcode)], val19s(opcode)); break;
		case 0x02: case 0x03: m_r[r1(opcode)] = val22h(opcode); break;
		case 0x04: case 0x05: m_r[r1(opcode)] = do_sub(m_r[r2(opcode)], val19s(opcode)); break;
		case 0x06: case 0x07: m_r[r1(opcode)] = val22s(opcode); break;
		case 0x08:            npc = val24u(opcode) | (m_pc & 0xff000000); break;
		case 0x09:            m_r[7] = npc; npc = val24u(opcode) | (m_pc & 0xff000000); break;
		case 0x0a: case 0x0b: m_r[r1(opcode)] = snz(m_r[r2(opcode)] & val19u(opcode)); break;
		case 0x0c: case 0x0d: m_r[r1(opcode)] = snz(m_r[r2(opcode)] | val19u(opcode)); break;
		case 0x0e: case 0x0f: m_r[r1(opcode)] = snz(m_r[r2(opcode)] ^ val19u(opcode)); break;

		case 0x10: case 0x11: m_r[r1(opcode)] = (s8)m_program.read_byte(m_r[r2(opcode)] + val19s(opcode)); break;
		case 0x12: case 0x13: m_r[r1(opcode)] = m_program.read_byte(m_r[r2(opcode)] + val19s(opcode)); break;
		case 0x14: case 0x15: m_r[r1(opcode)] = (s16)m_program.read_word(m_r[r2(opcode)] + val19s(opcode)); break;
		case 0x16: case 0x17: m_r[r1(opcode)] = m_program.read_word(m_r[r2(opcode)] + val19s(opcode)); break;
		case 0x18: case 0x19: m_r[r1(opcode)] = m_program.read_dword(m_r[r2(opcode)] + val19s(opcode)); break;
		case 0x1a: case 0x1b: m_program.write_byte(m_r[r2(opcode)] + val19s(opcode), m_r[r1(opcode)]); break;
		case 0x1c: case 0x1d: m_program.write_word(m_r[r2(opcode)] + val19s(opcode), m_r[r1(opcode)]); break;
		case 0x1e: case 0x1f: m_program.write_dword(m_r[r2(opcode)] + val19s(opcode), m_r[r1(opcode)]); break;

		case 0x20: case 0x21: m_r[r1(opcode)] = do_add(m_r[r2(opcode)], val11s(opcode)); break;
		case 0x22: case 0x23: m_r[r1(opcode)] = val14h(opcode); break;
		case 0x24: case 0x25: m_r[r1(opcode)] = do_sub(m_r[r2(opcode)], val11s(opcode)); break;
		case 0x26: case 0x27: do_sub(m_r[r1(opcode)], val14s(opcode)); break;
		case 0x28:            npc = m_pc + val16s(opcode); break;
		case 0x29:            m_r[7] = npc; npc = m_pc + val16s(opcode); break;
		case 0x2a: case 0x2b: m_r[r1(opcode)] = snz(m_r[r2(opcode)] & val11u(opcode)); break;
		case 0x2c: case 0x2d: m_r[r1(opcode)] = snz(m_r[r2(opcode)] | val11u(opcode)); break;
		case 0x2e: case 0x2f: m_r[r1(opcode)] = snz(m_r[r2(opcode)] ^ val11u(opcode)); break;

		case 0x30: case 0x31: m_r[r1(opcode)] = (s8)m_program.read_byte(m_r[6] + val14s(opcode)); break;
		case 0x32: case 0x33: m_r[r1(opcode)] = m_program.read_byte(m_r[6] + val14s(opcode)); break;
		case 0x34: case 0x35: m_r[r1(opcode)] = (s16)m_program.read_word(m_r[6] + val14s(opcode)); break;
		case 0x36: case 0x37: m_r[r1(opcode)] = m_program.read_word(m_r[6] + val14s(opcode)); break;
		case 0x38: case 0x39: m_r[r1(opcode)] = m_program.read_dword(m_r[6] + val14s(opcode)); break;
		case 0x3a: case 0x3b: m_program.write_byte(m_r[6] + val14s(opcode), m_r[r1(opcode)]); break;
		case 0x3c: case 0x3d: m_program.write_word(m_r[6] + val14s(opcode), m_r[r1(opcode)]); break;
		case 0x3e: case 0x3f: m_program.write_dword(m_r[6] + val14s(opcode), m_r[r1(opcode)]); break;

		case 0x40: case 0x41: m_r[r1(opcode)] = (s8)m_program.read_byte(m_r[r2(opcode)] + val11s(opcode)); break;
		case 0x42: case 0x43: m_r[r1(opcode)] = m_program.read_byte(m_r[r2(opcode)] + val11s(opcode)); break;
		case 0x44: case 0x45: m_r[r1(opcode)] = (s16)m_program.read_word(m_r[r2(opcode)] + val11s(opcode)); break;
		case 0x46: case 0x47: m_r[r1(opcode)] = m_program.read_word(m_r[r2(opcode)] + val11s(opcode)); break;
		case 0x48: case 0x49: m_r[r1(opcode)] = m_program.read_dword(m_r[r2(opcode)] + val11s(opcode)); break;
		case 0x4a: case 0x4b: m_program.write_byte(m_r[r2(opcode)] + val11s(opcode), m_r[r1(opcode)]); break;
		case 0x4c: case 0x4d: m_program.write_word(m_r[r2(opcode)] + val11s(opcode), m_r[r1(opcode)]); break;
		case 0x4e: case 0x4f: m_program.write_dword(m_r[r2(opcode)] + val11s(opcode), m_r[r1(opcode)]); break;

		case 0x50: case 0x51: m_r[r1(opcode)] = (s8)m_program.read_byte(val14s(opcode)); break;
		case 0x52: case 0x53: m_r[r1(opcode)] = m_program.read_byte(val14s(opcode)); break;
		case 0x54: case 0x55: m_r[r1(opcode)] = (s16)m_program.read_word(val14s(opcode)); break;
		case 0x56: case 0x57: m_r[r1(opcode)] = m_program.read_word(val14s(opcode)); break;
		case 0x58: case 0x59: m_r[r1(opcode)] = m_program.read_dword(val14s(opcode)); break;
		case 0x5a: case 0x5b: m_program.write_byte(val14s(opcode), m_r[r1(opcode)]); break;
		case 0x5c: case 0x5d: m_program.write_word(val14s(opcode), m_r[r1(opcode)]); break;
		case 0x5e: case 0x5f: m_program.write_dword(val14s(opcode), m_r[r1(opcode)]); break;

		case 0x60: case 0x61: m_r[r1(opcode)] = do_add(m_r[r1(opcode)], val6s(opcode)); break;
		case 0x62: case 0x63: m_r[r1(opcode)] = val6s(opcode); break;
		case 0x64: case 0x65: m_r[r1(opcode)] = do_sub(m_r[r1(opcode)], val6s(opcode)); break;
		case 0x66: case 0x67: do_sub(m_r[r1(opcode)], val6s(opcode)); break;
			// 68-69
		case 0x6a: case 0x6b: m_r[r1(opcode)] = do_asr(m_r[r2(opcode)], val3u(opcode)); break;
		case 0x6c: case 0x6d: m_r[r1(opcode)] = do_lsr(m_r[r2(opcode)], val3u(opcode)); break;
		case 0x6e: case 0x6f: m_r[r1(opcode)] = do_lsl(m_r[r2(opcode)], val3u(opcode)); break;

		case 0x70: case 0x71: m_r[r1(opcode)] = (s8)m_program.read_byte(m_r[6] + val6s(opcode)); break;
		case 0x72: case 0x73: m_r[r1(opcode)] = m_program.read_byte(m_r[6] + val6s(opcode)); break;
		case 0x74: case 0x75: m_r[r1(opcode)] = (s16)m_program.read_word(m_r[6] + val6s(opcode)); break;
		case 0x76: case 0x77: m_r[r1(opcode)] = m_program.read_word(m_r[6] + val6s(opcode)); break;
		case 0x78: case 0x79: m_r[r1(opcode)] = m_program.read_dword(m_r[6] + val6s(opcode)); break;
		case 0x7a: case 0x7b: m_program.write_byte(m_r[6] + val6s(opcode), m_r[r1(opcode)]); break;
		case 0x7c: case 0x7d: m_program.write_word(m_r[6] + val6s(opcode), m_r[r1(opcode)]); break;
		case 0x7e: case 0x7f: m_program.write_dword(m_r[6] + val6s(opcode), m_r[r1(opcode)]); break;

		case 0x80: case 0x81: m_r[r1(opcode)] = do_add(m_r[r2(opcode)], m_r[r3(opcode)]); break;
			// 82-83
		case 0x84: case 0x85: m_r[r1(opcode)] = do_sub(m_r[r2(opcode)], m_r[r3(opcode)]); break;
			// 86-87
		case 0x88:            npc = m_r[r2(opcode)]; break;
			// 89
		case 0x8a: case 0x8b: m_r[r1(opcode)] = snz(m_r[r2(opcode)] & m_r[r3(opcode)]); break;
		case 0x8c: case 0x8d: m_r[r1(opcode)] = snz(m_r[r2(opcode)] | m_r[r3(opcode)]); break;
		case 0x8e: case 0x8f: m_r[r1(opcode)] = snz(m_r[r2(opcode)] ^ m_r[r3(opcode)]); break;

		case 0x90: case 0x91: m_r[r1(opcode)] = (s8)m_program.read_byte(m_r[r2(opcode)] + val3s(opcode)); break;
		case 0x92: case 0x93: m_r[r1(opcode)] = m_program.read_byte(m_r[r2(opcode)] + val3s(opcode)); break;
		case 0x94: case 0x95: m_r[r1(opcode)] = (s16)m_program.read_word(m_r[r2(opcode)] + val3s(opcode)); break;
		case 0x96: case 0x97: m_r[r1(opcode)] = m_program.read_word(m_r[r2(opcode)] + val3s(opcode)); break;
		case 0x98: case 0x99: m_r[r1(opcode)] = m_program.read_dword(m_r[r2(opcode)] + val3s(opcode)); break;
		case 0x9a: case 0x9b: m_program.write_byte(m_r[r2(opcode)] + val3s(opcode), m_r[r1(opcode)]); break;
		case 0x9c: case 0x9d: m_program.write_word(m_r[r2(opcode)] + val3s(opcode), m_r[r1(opcode)]); break;
		case 0x9e: case 0x9f: m_program.write_dword(m_r[r2(opcode)] + val3s(opcode), m_r[r1(opcode)]); break;

		case 0xa0: case 0xa1: m_r[r1(opcode)] = ~m_r[r2(opcode)]; break;
		case 0xa2: case 0xa3: m_r[r1(opcode)] = m_r[r2(opcode)]; break;
		case 0xa4: case 0xa5: m_r[r1(opcode)] = -m_r[r2(opcode)]; break;
		case 0xa6: case 0xa7: do_sub(m_r[r1(opcode)], m_r[r2(opcode)]); break;
		case 0xa8:            m_r[7] = npc; npc = m_r[r2(opcode)]; break;
			// a9
		case 0xaa: case 0xab: m_r[r1(opcode)] = do_asr(m_r[r2(opcode)], m_r[r3(opcode)]); break;
		case 0xac: case 0xad: m_r[r1(opcode)] = do_lsr(m_r[r2(opcode)], m_r[r3(opcode)]); break;
		case 0xae: case 0xaf: m_r[r1(opcode)] = do_lsl(m_r[r2(opcode)], m_r[r3(opcode)]); break;

		case 0xb0: case 0xb1: {
			m_hr[0] = m_r[r1(opcode)] * m_r[r2(opcode)];
			break;
		}
		case 0xb2: case 0xb3: {
			m_hr[0] = s32(m_r[r1(opcode)]) * s32(m_r[r2(opcode)]);
			break;
		}
		case 0xb4: case 0xb5: {
			u64 res = m_r[r1(opcode)] * m_r[r2(opcode)];
			m_hr[0] = res;
			m_hr[1] = res >> 32;
			break;
		}
		case 0xb6: case 0xb7: {
			u64 res = s32(m_r[r1(opcode)]) * s32(m_r[r2(opcode)]);
			m_hr[0] = res;
			m_hr[1] = res >> 32;
			break;
		}

		case 0xbc: case 0xbd: {
			s32 xr1 = m_r[r1(opcode)];
			s32 xr2 = m_r[r2(opcode)];
			if(xr2) {
				m_hr[2] = xr1 / xr2;
				m_hr[3] = xr1 % xr2;
			}
			break;
		}

		case 0xbe: case 0xbf: {
			u32 xr1 = m_r[r1(opcode)];
			u32 xr2 = m_r[r2(opcode)];
			if(xr2) {
				m_hr[2] = xr1 / xr2;
				m_hr[3] = xr1 % xr2;
			}
			break;
		}

		case 0xc8: case 0xc9: m_r[r1(opcode)] = m_hr[val6u(opcode)]; break;
		case 0xca: case 0xcb: m_hr[val6u(opcode)] = m_r[r1(opcode)]; break;

		case 0xd0:            if(m_hr[4] & F_V) npc = m_pc + val8s(opcode); break;
		case 0xd1:            if(m_hr[4] & F_C) npc = m_pc + val8s(opcode); break;
		case 0xd2:            if(m_hr[4] & F_Z) npc = m_pc + val8s(opcode); break;
		case 0xd3:            if((m_hr[4] & F_Z) || (m_hr[4] & F_C)) npc = m_pc + val8s(opcode); break;
		case 0xd4:            if(m_hr[4] & F_N) npc = m_pc + val8s(opcode); break;
		case 0xd5:            npc = m_pc + val8s(opcode); break;
		case 0xd6:            if(((m_hr[4] & F_N) && !(m_hr[4] & F_V)) || ((m_hr[4] & F_V) && !(m_hr[4] & F_N))) npc = m_pc + val8s(opcode); break;
		case 0xd7:            if((m_hr[4] & F_Z) || ((m_hr[4] & F_N) && !(m_hr[4] & F_V)) || ((m_hr[4] & F_V) && !(m_hr[4] & F_N))) npc = m_pc + val8s(opcode); break;
		case 0xd8:            if(!(m_hr[4] & F_V)) npc = m_pc + val8s(opcode); break;
		case 0xd9:            if(!(m_hr[4] & F_C)) npc = m_pc + val8s(opcode); break;
		case 0xda:            if(!(m_hr[4] & F_Z)) npc = m_pc + val8s(opcode); break;
		case 0xdb:            if(!(m_hr[4] & F_Z) && !(m_hr[4] & F_C)) npc = m_pc + val8s(opcode); break;
		case 0xdc:            if(!(m_hr[4] & F_N)) npc = m_pc + val8s(opcode); break;
		case 0xdd:            break;
		case 0xde:            if(((m_hr[4] & F_N) && (m_hr[4] & F_V)) || (!(m_hr[4] & F_V) && !(m_hr[4] & F_N))) npc = m_pc + val8s(opcode); break;
		case 0xdf:            if((!(m_hr[4] & F_Z) && (m_hr[4] & F_N) && (m_hr[4] & F_V)) || (!(m_hr[4] & F_Z) && !(m_hr[4] & F_V) && !(m_hr[4] & F_N))) npc = m_pc + val8s(opcode); break;

		case 0xe0:            npc = m_r[7]; break;
		case 0xe1:            m_hr[4] = m_if1; npc = m_ilr1; break;
			// e2
		case 0xe3:            /* rti2 */ break;
			// e4-ef

		case 0xf0:            m_hr[4] &= ~F_C; break;
		case 0xf1:            m_hr[4] |= F_C; break;
		case 0xf2:            m_hr[4] &= ~F_Z; break;
		case 0xf3:            m_hr[4] |= F_Z; break;
		case 0xf4:            m_hr[4] &= ~F_N; break;
		case 0xf5:            m_hr[4] |= F_N; break;
		case 0xf6:            m_hr[4] &= ~F_V; break;
		case 0xf7:            m_hr[4] |= F_V; break;
		case 0xf8:            m_hr[4] &= ~F_I; break;
		case 0xf9:            m_hr[4] |= F_I; m_ei_count = 2; break;
			// fa-fb
		case 0xfc:            break;
			// fd
		case 0xfe:            m_wait = true; npc = check_interrupt(npc-1); if(m_wait) m_icount = 0; break;
			// ff
		}

		m_pc = npc;
	}
}
