// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8.h

    H8-300 base cpu emulation


***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h8.h"

h8_device::h8_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, bool mode_a16, address_map_delegate map_delegate) :
	cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
	program_config("program", ENDIANNESS_BIG, 16, mode_a16 ? 16 : 24, 0, map_delegate),
	io_config("io", ENDIANNESS_BIG, 16, 16, -1), program(nullptr), io(nullptr), direct(nullptr), PPC(0), NPC(0), PC(0), PIR(0), EXR(0), CCR(0), MAC(0), MACF(0),
	TMP1(0), TMP2(0), TMPR(0), inst_state(0), inst_substate(0), icount(0), bcount(0), irq_vector(0), taken_irq_vector(0), irq_level(0), taken_irq_level(0), irq_required(false), irq_nmi(false)
{
	supports_advanced = false;
	mode_advanced = false;
	has_exr = false;
	mac_saturating = false;
	has_trace = false;
}

void h8_device::device_start()
{
	program = &space(AS_PROGRAM);
	direct = &program->direct();
	io      = &space(AS_IO);

	state_add(STATE_GENPC,     "GENPC",     NPC).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", PPC).noshow();
	if(has_exr)
		state_add(STATE_GENFLAGS,  "GENFLAGS",  CCR).formatstr("%11s").noshow();
	else
		state_add(STATE_GENFLAGS,  "GENFLAGS",  CCR).formatstr("%8s").noshow();
	state_add(H8_PC,           "PC",        NPC);
	state_add(H8_CCR,          "CCR",       CCR);

	if(has_exr)
		state_add(H8_EXR,          "EXR",       EXR);
	if(!supports_advanced) {
		state_add(H8_R0,           "R0",        R[0]);
		state_add(H8_R1,           "R1",        R[1]);
		state_add(H8_R2,           "R2",        R[2]);
		state_add(H8_R3,           "R3",        R[3]);
		state_add(H8_R4,           "R4",        R[4]);
		state_add(H8_R5,           "R5",        R[5]);
		state_add(H8_R6,           "R6",        R[6]);
		state_add(H8_R7,           "R7",        R[7]);
	} else {
		state_add(H8_R0,           "R0",        R[0]).noshow();
		state_add(H8_R1,           "R1",        R[1]).noshow();
		state_add(H8_R2,           "R2",        R[2]).noshow();
		state_add(H8_R3,           "R3",        R[3]).noshow();
		state_add(H8_R4,           "R4",        R[4]).noshow();
		state_add(H8_R5,           "R5",        R[5]).noshow();
		state_add(H8_R6,           "R6",        R[6]).noshow();
		state_add(H8_R7,           "R7",        R[7]).noshow();
		state_add(H8_E0,           "E0",        R[8]).noshow();
		state_add(H8_E1,           "E1",        R[9]).noshow();
		state_add(H8_E2,           "E2",        R[10]).noshow();
		state_add(H8_E3,           "E3",        R[11]).noshow();
		state_add(H8_E4,           "E4",        R[12]).noshow();
		state_add(H8_E5,           "E5",        R[13]).noshow();
		state_add(H8_E6,           "E6",        R[14]).noshow();
		state_add(H8_E7,           "E7",        R[15]).noshow();
		state_add(H8_R0,           "ER0",       TMPR).callimport().formatstr("%9s");
		state_add(H8_R1,           "ER1",       TMPR).callimport().formatstr("%9s");
		state_add(H8_R2,           "ER2",       TMPR).callimport().formatstr("%9s");
		state_add(H8_R3,           "ER3",       TMPR).callimport().formatstr("%9s");
		state_add(H8_R4,           "ER4",       TMPR).callimport().formatstr("%9s");
		state_add(H8_R5,           "ER5",       TMPR).callimport().formatstr("%9s");
		state_add(H8_R6,           "ER6",       TMPR).callimport().formatstr("%9s");
		state_add(H8_R7,           "ER7",       TMPR).callimport().formatstr("%9s");
	}

	save_item(NAME(PPC));
	save_item(NAME(NPC));
	save_item(NAME(PC));
	save_item(NAME(PIR));
	save_item(NAME(IR));
	save_item(NAME(R));
	save_item(NAME(EXR));
	save_item(NAME(CCR));
	save_item(NAME(TMP1));
	save_item(NAME(TMP2));
	save_item(NAME(inst_state));
	save_item(NAME(inst_substate));
	save_item(NAME(irq_vector));
	save_item(NAME(taken_irq_vector));
	save_item(NAME(irq_level));
	save_item(NAME(taken_irq_level));
	save_item(NAME(irq_nmi));

	m_icountptr = &icount;

	PC = 0;
	PPC = 0;
	NPC = 0;
	memset(IR, 0, sizeof(IR));
	memset(R, 0, sizeof(R));
	EXR = 0;
	CCR = 0;
	MAC = 0;
	MACF = 0;
	inst_state = STATE_RESET;
	inst_substate = 0;
}

void h8_device::device_reset()
{
	inst_state = STATE_RESET;
	inst_substate = 0;

	irq_vector = 0;
	irq_level = -1;
	irq_nmi = false;
	taken_irq_vector = 0;
	taken_irq_level = -1;
}


UINT32 h8_device::execute_min_cycles() const
{
	return 1;
}

UINT32 h8_device::execute_max_cycles() const
{
	return 1;
}

UINT32 h8_device::execute_input_lines() const
{
	return 0;
}

void h8_device::recompute_bcount(UINT64 event_time)
{
	if(!event_time || event_time >= total_cycles() + icount) {
		bcount = 0;
		return;
	}
	bcount = total_cycles() + icount - event_time;
}

void h8_device::execute_run()
{
	internal_update(total_cycles());

	if(inst_substate)
		do_exec_partial();

	while(icount > 0) {
		while(icount > bcount) {
			if(inst_state < 0x10000) {
				PPC = NPC;
				if(machine().debug_flags & DEBUG_FLAG_ENABLED)
					debugger_instruction_hook(this, NPC);
			}
			do_exec_full();
		}
		while(bcount && icount && icount <= bcount)
			internal_update(total_cycles() + icount - bcount);
		if(inst_substate)
			do_exec_partial();
	}
}

void h8_device::add_event(UINT64 &event_time, UINT64 new_event)
{
	if(!new_event)
		return;
	if(!event_time || event_time > new_event)
		event_time = new_event;
}

void h8_device::internal_update()
{
	internal_update(total_cycles());
}

const address_space_config *h8_device::memory_space_config(address_spacenum spacenum) const
{
	return
		spacenum == AS_PROGRAM ? &program_config :
		spacenum == AS_IO ? &io_config : nullptr;
}


void h8_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case H8_R0:
	case H8_R1:
	case H8_R2:
	case H8_R3:
	case H8_R4:
	case H8_R5:
	case H8_R6:
	case H8_R7: {
		int r = entry.index() - H8_R0;
		R[r + 8] = TMPR >> 16;
		R[r] = TMPR;
		break;
	}
	}
}

void h8_device::state_export(const device_state_entry &entry)
{
}

void h8_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
		if(has_exr)
			strprintf(str, "%c%c %c%c%c%c%c%c%c%c",
							(EXR & EXR_T) ? 'T' : '-',
							'0' + (EXR & EXR_I),
							(CCR & F_I)  ? 'I' : '-',
							(CCR & F_UI) ? 'u' : '-',
							(CCR & F_H)  ? 'H' : '-',
							(CCR & F_U)  ? 'U' : '-',
							(CCR & F_N)  ? 'N' : '-',
							(CCR & F_Z)  ? 'Z' : '-',
							(CCR & F_V)  ? 'V' : '-',
							(CCR & F_C)  ? 'C' : '-');
		else
			strprintf(str, "%c%c%c%c%c%c%c%c",
							(CCR & F_I)  ? 'I' : '-',
							(CCR & F_UI) ? 'u' : '-',
							(CCR & F_H)  ? 'H' : '-',
							(CCR & F_U)  ? 'U' : '-',
							(CCR & F_N)  ? 'N' : '-',
							(CCR & F_Z)  ? 'Z' : '-',
							(CCR & F_V)  ? 'V' : '-',
							(CCR & F_C)  ? 'C' : '-');
		break;
	case H8_R0:
	case H8_R1:
	case H8_R2:
	case H8_R3:
	case H8_R4:
	case H8_R5:
	case H8_R6:
	case H8_R7: {
		int r = entry.index() - H8_R0;
		strprintf(str, "%04x %04x", R[r + 8], R[r]);
		break;
	}
	}
}


UINT32 h8_device::disasm_min_opcode_bytes() const
{
	return 2;
}

UINT32 h8_device::disasm_max_opcode_bytes() const
{
	return 10;
}

void h8_device::disassemble_am(char *&buffer, int am, offs_t pc, const UINT8 *oprom, UINT32 opcode, int offset)
{
	static const char *const r8_names[16] = {
		"r0h", "r1h",  "r2h", "r3h",  "r4h", "r5h",  "r6h", "r7h",
		"r0l", "r1l",  "r2l", "r3l",  "r4l", "r5l",  "r6l", "r7l"
	};

	static const char *const r16_names[16] = {
		"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
		"e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7",
	};

	static const char *const r32_names[88] = {
		"er0", "er1", "er2", "er3", "er4", "er5", "er6", "sp",
	};

	switch(am) {
	case DASM_r8l:
		buffer += sprintf(buffer, "%s", r8_names[opcode & 15]);
		break;

	case DASM_r8h:
		buffer += sprintf(buffer, "%s", r8_names[(opcode >> 4) & 15]);
		break;

	case DASM_r8u:
		buffer += sprintf(buffer, "%s", r8_names[(opcode >> 8) & 15]);
		break;

	case DASM_r16l:
		buffer += sprintf(buffer, "%s", r16_names[opcode & 15]);
		break;

	case DASM_r16h:
		buffer += sprintf(buffer, "%s", r16_names[(opcode >> 4) & 15]);
		break;

	case DASM_r32l:
		buffer += sprintf(buffer, "%s", r32_names[opcode & 7]);
		break;

	case DASM_r32h:
		buffer += sprintf(buffer, "%s", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16ih:
		buffer += sprintf(buffer, "@%s", r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16ihh:
		buffer += sprintf(buffer, "@%s", r16_names[(opcode >> 20) & 7]);
		break;

	case DASM_pr16h:
		buffer += sprintf(buffer, "@-%s", r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16ph:
		buffer += sprintf(buffer, "@%s+", r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16d16h:
		buffer += sprintf(buffer, "@(%x, %s)", (oprom[offset-2] << 8) | oprom[offset-1], r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32ih:
		buffer += sprintf(buffer, "@%s", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32ihh:
		buffer += sprintf(buffer, "@%s", r32_names[(opcode >> 20) & 7]);
		break;

	case DASM_pr32h:
		buffer += sprintf(buffer, "@-%s", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32pl:
		buffer += sprintf(buffer, "@%s+", r32_names[opcode & 7]);
		break;

	case DASM_r32ph:
		buffer += sprintf(buffer, "@%s+", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32d16h:
		buffer += sprintf(buffer, "@(%x, %s)", (oprom[offset-2] << 8) | oprom[offset-1], r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32d32hh:
		buffer += sprintf(buffer, "@(%x, %s)", (oprom[offset-4] << 24) | (oprom[offset-3] << 16) | (oprom[offset-2] << 8) | oprom[offset-1], r32_names[(opcode >> 20) & 7]);
		break;

	case DASM_psp:
		buffer += sprintf(buffer, "@-sp");
		break;

	case DASM_spp:
		buffer += sprintf(buffer, "@sp+");
		break;

	case DASM_r32n2l:
		buffer += sprintf(buffer, "%s-%s", r32_names[opcode & 6], r32_names[(opcode & 6) + 1]);
		break;

	case DASM_r32n3l:
		buffer += sprintf(buffer, "%s-%s", r32_names[opcode & 4], r32_names[(opcode & 4) + 2]);
		break;

	case DASM_r32n4l:
		buffer += sprintf(buffer, "%s-%s", r32_names[opcode & 4], r32_names[(opcode & 4) + 3]);
		break;

	case DASM_abs8:
		buffer += sprintf(buffer, "@%08x", 0xffffff00 | oprom[1]);
		break;

	case DASM_abs16:
		if(offset >= 6)
			buffer += sprintf(buffer, "@%08x", INT16((oprom[offset-4] << 8) | oprom[offset-3]));
		else
			buffer += sprintf(buffer, "@%08x", INT16((oprom[offset-2] << 8) | oprom[offset-1]));
		break;

	case DASM_abs32:
		if(offset >= 8)
			buffer += sprintf(buffer, "@%08x", (oprom[offset-6] << 24) | (oprom[offset-5] << 16) | (oprom[offset-4] << 8) | oprom[offset-3]);
		else
			buffer += sprintf(buffer, "@%08x", (oprom[offset-4] << 24) | (oprom[offset-3] << 16) | (oprom[offset-2] << 8) | oprom[offset-1]);
		break;

	case DASM_abs8i:
		buffer += sprintf(buffer, "@%02x", oprom[1]);
		break;

	case DASM_abs16e:
		buffer += sprintf(buffer, "%04x", (oprom[2] << 8) | oprom[3]);
		break;

	case DASM_abs24e:
		buffer += sprintf(buffer, "%08x", (oprom[1] << 16) | (oprom[2] << 8) | oprom[3]);
		break;

	case DASM_rel8:
		buffer += sprintf(buffer, "%08x", pc + 2 + INT8(oprom[1]));
		break;

	case DASM_rel16:
		buffer += sprintf(buffer, "%08x", pc + 4 + INT16((oprom[2] << 8) | oprom[3]));
		break;

	case DASM_one:
		buffer += sprintf(buffer, "#1");
		break;

	case DASM_two:
		buffer += sprintf(buffer, "#2");
		break;

	case DASM_four:
		buffer += sprintf(buffer, "#4");
		break;

	case DASM_imm2:
		buffer += sprintf(buffer, "#%x", (opcode >> 4) & 3);
		break;

	case DASM_imm3:
		buffer += sprintf(buffer, "#%x", (opcode >> 4) & 7);
		break;

	case DASM_imm8:
		buffer += sprintf(buffer, "#%02x", oprom[1]);
		break;

	case DASM_imm16:
		buffer += sprintf(buffer, "#%04x", (oprom[2] << 8) | oprom[3]);
		break;

	case DASM_imm32:
		buffer += sprintf(buffer, "#%08x", (oprom[2] << 16) | (oprom[3] << 16) | (oprom[4] << 8) | oprom[5]);
		break;

	case DASM_ccr:
		buffer += sprintf(buffer, "ccr");
		break;

	case DASM_exr:
		buffer += sprintf(buffer, "exr");
		break;

	case DASM_macl:
		buffer += sprintf(buffer, "macl");
		break;

	case DASM_mach:
		buffer += sprintf(buffer, "mach");
		break;

	default:
		buffer += sprintf(buffer, "<%d>", am);
		break;
	}
}

offs_t h8_device::disassemble_generic(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options, const disasm_entry *table)
{
	UINT32 slot[5];
	slot[0] = (oprom[0] << 8) | oprom[1];
	slot[1] = (oprom[0] << 24) | (oprom[1] << 16) | (oprom[2] << 8) | oprom[3];
	slot[2] = (oprom[0] << 24) | (oprom[1] << 16) | (oprom[4] << 8) | oprom[5];
	slot[3] = (oprom[0] << 24) | (oprom[1] << 16) | (oprom[6] << 8) | oprom[7];
	slot[4] = (oprom[2] << 24) | (oprom[3] << 16) | (oprom[4] << 8) | oprom[5];

	int inst;
	for(inst=0;; inst++) {
		const disasm_entry &e = table[inst];
		if((slot[e.slot] & e.mask) == e.val && (slot[0] & e.mask0) == e.val0)
			break;
	}
	const disasm_entry &e = table[inst];
	buffer += sprintf(buffer, "%s", e.opcode);

	if(e.am1 != DASM_none) {
		*buffer++ = ' ';
		disassemble_am(buffer, e.am1, pc, oprom, slot[e.slot], e.flags & DASMFLAG_LENGTHMASK);
	}
	if(e.am2 != DASM_none) {
		*buffer++ = ',';
		*buffer++ = ' ';
		disassemble_am(buffer, e.am2, pc, oprom, slot[e.slot], e.flags & DASMFLAG_LENGTHMASK);
	}
	return e.flags | DASMFLAG_SUPPORTED;
}

offs_t h8_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

UINT16 h8_device::read16i(UINT32 adr)
{
	icount--;
	return direct->read_word(adr & ~1);
}

UINT16 h8_device::fetch()
{
	UINT16 res = read16i(PC);
	PC += 2;
	return res;
}

UINT8 h8_device::read8(UINT32 adr)
{
	icount--;
	return program->read_byte(adr);
}

void h8_device::write8(UINT32 adr, UINT8 data)
{
	//  logerror("W %06x %02x\n", adr & 0xffffff, data);
	icount--;
	program->write_byte(adr, data);
}

UINT16 h8_device::read16(UINT32 adr)
{
	icount--;
	return program->read_word(adr & ~1);
}

void h8_device::write16(UINT32 adr, UINT16 data)
{
	//  logerror("W %06x %04x\n", adr & 0xfffffe, data);
	icount--;
	program->write_word(adr & ~1, data);
}

bool h8_device::exr_in_stack() const
{
	return false;
}

void h8_device::prefetch_done()
{
	if(irq_vector) {
		inst_state = STATE_IRQ;
		taken_irq_vector = irq_vector;
		taken_irq_level = irq_level;
	} else if(has_trace && (EXR & EXR_T) && exr_in_stack())
		inst_state = STATE_TRACE;
	else
		inst_state = IR[0] = PIR;
}

void h8_device::prefetch_done_noirq()
{
	if(has_trace && (EXR & EXR_T) && exr_in_stack())
		inst_state = STATE_TRACE;
	else
		inst_state = IR[0] = PIR;
}

void h8_device::prefetch_done_noirq_notrace()
{
	inst_state = IR[0] = PIR;
}

void h8_device::set_irq(int _irq_vector, int _irq_level, bool _irq_nmi)
{
	irq_vector = _irq_vector;
	irq_level = _irq_level;
	irq_nmi = _irq_nmi;
}

void h8_device::internal(int cycles)
{
	icount -= cycles;
}

void h8_device::illegal()
{
	throw emu_fatalerror("%s: Illegal instruction at address %x\n", tag().c_str(), PPC);
}

int h8_device::trace_setup()
{
	throw emu_fatalerror("%s: Trace setup called but unimplemented.\n", tag().c_str());
}

int h8_device::trapa_setup()
{
	throw emu_fatalerror("%s: Trapa setup called but unimplemented.\n", tag().c_str());
}

UINT8 h8_device::do_addx8(UINT8 v1, UINT8 v2)
{
	UINT16 res = v1 + v2 + (CCR & F_C ? 1 : 0);
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xf) + (v2 & 0xf) + (CCR & F_C ? 1 : 0)) & 0x10)
		CCR |= F_H;
	if(!UINT8(res))
		CCR |= F_Z;
	else if(INT8(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

UINT8 h8_device::do_subx8(UINT8 v1, UINT8 v2)
{
	UINT16 res = v1 - v2 - (CCR & F_C ? 1 : 0);
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xf) - (v2 & 0xf) - (CCR & F_C ? 1 : 0)) & 0x10)
		CCR |= F_H;
	if(!UINT8(res))
		CCR |= F_Z;
	else if(INT8(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

UINT8 h8_device::do_inc8(UINT8 v1, UINT8 v2)
{
	UINT8 res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(INT8(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	return res;
}

UINT16 h8_device::do_inc16(UINT16 v1, UINT16 v2)
{
	UINT16 res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(INT16(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	return res;
}

UINT32 h8_device::do_inc32(UINT32 v1, UINT32 v2)
{
	UINT32 res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(INT32(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	return res;
}

UINT8 h8_device::do_add8(UINT8 v1, UINT8 v2)
{
	UINT16 res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xf) + (v2 & 0xf)) & 0x10)
		CCR |= F_H;
	if(!UINT8(res))
		CCR |= F_Z;
	else if(INT8(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

UINT16 h8_device::do_add16(UINT16 v1, UINT16 v2)
{
	UINT32 res = v1 + v2;
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xfff) + (v2 & 0xffff)) & 0x1000)
		CCR |= F_H;
	if(!UINT16(res))
		CCR |= F_Z;
	else if(INT16(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	if(res & 0x10000)
		CCR |= F_C;
	return res;

}

UINT32 h8_device::do_add32(UINT32 v1, UINT32 v2)
{
	UINT64 res = UINT64(v1) + UINT64(v2);
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xfffffff) + (v2 & 0xfffffff)) & 0x10000000)
		CCR |= F_H;
	if(!UINT32(res))
		CCR |= F_Z;
	else if(INT32(res) < 0)
		CCR |= F_N;
	if(~(v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	if(res & U64(0x100000000))
		CCR |= F_C;
	return res;
}

UINT8 h8_device::do_dec8(UINT8 v1, UINT8 v2)
{
	UINT8 res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(INT8(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	return res;
}

UINT16 h8_device::do_dec16(UINT16 v1, UINT16 v2)
{
	UINT16 res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(INT16(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	return res;
}

UINT32 h8_device::do_dec32(UINT32 v1, UINT32 v2)
{
	UINT32 res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z);
	if(!res)
		CCR |= F_Z;
	else if(INT32(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	return res;
}

UINT8 h8_device::do_sub8(UINT8 v1, UINT8 v2)
{
	UINT16 res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xf) - (v2 & 0xf)) & 0x10)
		CCR |= F_H;
	if(!UINT8(res))
		CCR |= F_Z;
	else if(INT8(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80)
		CCR |= F_V;
	if(res & 0x100)
		CCR |= F_C;
	return res;

}

UINT16 h8_device::do_sub16(UINT16 v1, UINT16 v2)
{
	UINT32 res = v1 - v2;
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xfff) - (v2 & 0xffff)) & 0x1000)
		CCR |= F_H;
	if(!UINT16(res))
		CCR |= F_Z;
	else if(INT16(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x8000)
		CCR |= F_V;
	if(res & 0x10000)
		CCR |= F_C;
	return res;

}

UINT32 h8_device::do_sub32(UINT32 v1, UINT32 v2)
{
	UINT64 res = UINT64(v1) - UINT64(v2);
	CCR &= ~(F_N|F_V|F_Z|F_C|F_H);
	if(((v1 & 0xfffffff) - (v2 & 0xfffffff)) & 0x10000000)
		CCR |= F_H;
	if(!UINT32(res))
		CCR |= F_Z;
	else if(INT32(res) < 0)
		CCR |= F_N;
	if((v1^v2) & (v1^res) & 0x80000000)
		CCR |= F_V;
	if(res & U64(0x100000000))
		CCR |= F_C;
	return res;
}

UINT8 h8_device::do_shal8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	if((v & 0xc0) == 0x40 || (v & 0xc0) == 0x80)
		CCR |= F_V;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_shal16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	if((v & 0xc000) == 0x4000 || (v & 0xc000) == 0x8000)
		CCR |= F_V;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_shal32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	if((v & 0xc0000000) == 0x40000000 || (v & 0xc0000000) == 0x80000000)
		CCR |= F_V;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_shar8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x40) {
		v |= 0x80;
		CCR |= F_N;
	}
	return v;
}

UINT16 h8_device::do_shar16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x4000) {
		v |= 0x8000;
		CCR |= F_N;
	}
	return v;
}

UINT32 h8_device::do_shar32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x40000000) {
		v |= 0x80000000;
		CCR |= F_N;
	}
	return v;
}

UINT8 h8_device::do_shll8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_shll16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_shll32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	v <<= 1;
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_shlr8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	return v;
}

UINT16 h8_device::do_shlr16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	return v;
}

UINT32 h8_device::do_shlr32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 1)
		CCR |= F_C;
	v >>= 1;
	if(!v)
		CCR |= F_Z;
	return v;
}

UINT8 h8_device::do_shal2_8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	if((v & 0xc0) == 0x40 || (v & 0xc0) == 0x80 ||
		(v & 0x60) == 0x20 || (v & 0x60) == 0x40)
		CCR |= F_V;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_shal2_16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	if((v & 0xc000) == 0x4000 || (v & 0xc000) == 0x8000 ||
		(v & 0x6000) == 0x2000 || (v & 0x6000) == 0x4000)
		CCR |= F_V;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_shal2_32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	if((v & 0xc0000000) == 0x40000000 || (v & 0xc0000000) == 0x80000000 ||
		(v & 0x60000000) == 0x20000000 || (v & 0x60000000) == 0x40000000)
		CCR |= F_V;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_shar2_8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x20) {
		v |= 0xc0;
		CCR |= F_N;
	}
	return v;
}

UINT16 h8_device::do_shar2_16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x2000) {
		v |= 0xc000;
		CCR |= F_N;
	}
	return v;
}

UINT32 h8_device::do_shar2_32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	else if (v & 0x20000000) {
		v |= 0xc0000000;
		CCR |= F_N;
	}
	return v;
}

UINT8 h8_device::do_shll2_8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_shll2_16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_shll2_32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	v <<= 2;
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_shlr2_8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	return v;
}

UINT16 h8_device::do_shlr2_16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	return v;
}

UINT32 h8_device::do_shlr2_32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 2)
		CCR |= F_C;
	v >>= 2;
	if(!v)
		CCR |= F_Z;
	return v;
}

UINT8 h8_device::do_rotl8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	v = (v << 1) | (v >> 7);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotl16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	v = (v << 1) | (v >> 15);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotl32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	v = (v << 1) | (v >> 31);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_rotr8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x01)
		CCR |= F_C;
	v = (v << 7) | (v >> 1);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotr16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0001)
		CCR |= F_C;
	v = (v << 15) | (v >> 1);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotr32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000001)
		CCR |= F_C;
	v = (v << 31) | (v >> 1);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_rotxl8(UINT8 v)
{
	UINT8 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80)
		CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotxl16(UINT16 v)
{
	UINT16 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x8000)
		CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotxl32(UINT32 v)
{
	UINT32 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x80000000)
		CCR |= F_C;
	v = (v << 1) | c;
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_rotxr8(UINT8 v)
{
	UINT8 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x01)
		CCR |= F_C;
	v = (v >> 1) | (c << 7);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotxr16(UINT16 v)
{
	UINT8 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0001)
		CCR |= F_C;
	v = (v >> 1) | (c << 15);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotxr32(UINT32 v)
{
	UINT8 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000001)
		CCR |= F_C;
	v = (v >> 1) | (c << 31);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_rotl2_8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	v = (v << 2) | (v >> 6);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotl2_16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	v = (v << 2) | (v >> 14);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotl2_32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	v = (v << 2) | (v >> 30);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_rotr2_8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x02)
		CCR |= F_C;
	v = (v << 6) | (v >> 2);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotr2_16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0002)
		CCR |= F_C;
	v = (v << 14) | (v >> 2);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotr2_32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000002)
		CCR |= F_C;
	v = (v << 30) | (v >> 2);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_rotxl2_8(UINT8 v)
{
	UINT8 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40)
		CCR |= F_C;
	v = (v << 2) | (c << 1) | ((v >> 6) & 0x01);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotxl2_16(UINT16 v)
{
	UINT16 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x4000)
		CCR |= F_C;
	v = (v << 2) | (c << 1) | ((v >> 14) & 0x0001);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotxl2_32(UINT32 v)
{
	UINT32 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x40000000)
		CCR |= F_C;
	v = (v << 2) | (c << 1) | ((v >> 30) & 0x00000001);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

UINT8 h8_device::do_rotxr2_8(UINT8 v)
{
	UINT8 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x02)
		CCR |= F_C;
	v = (v >> 2) | (c << 6) | (v << 7);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
	return v;
}

UINT16 h8_device::do_rotxr2_16(UINT16 v)
{
	UINT16 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x0002)
		CCR |= F_C;
	v = (v >> 2) | (c << 14) | (v << 15);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
	return v;
}

UINT32 h8_device::do_rotxr2_32(UINT32 v)
{
	UINT32 c = CCR & F_C ? 1 : 0;
	CCR &= ~(F_N|F_V|F_Z|F_C);
	if(v & 0x00000002)
		CCR |= F_C;
	v = (v >> 2) | (c << 30) | (v << 31);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
	return v;
}

void h8_device::set_nzv8(UINT8 v)
{
	CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(INT8(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nzv16(UINT16 v)
{
	CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nzv32(UINT32 v)
{
	CCR &= ~(F_N|F_V|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nz16(UINT16 v)
{
	CCR &= ~(F_N|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(INT16(v) < 0)
		CCR |= F_N;
}

void h8_device::set_nz32(UINT32 v)
{
	CCR &= ~(F_N|F_Z);
	if(!v)
		CCR |= F_Z;
	else if(INT32(v) < 0)
		CCR |= F_N;
}

#include "cpu/h8/h8.inc"
