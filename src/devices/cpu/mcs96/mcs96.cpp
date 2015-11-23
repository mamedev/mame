// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    mcs96.h

    MCS96, 8098/8398/8798 branch

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "mcs96.h"

mcs96_device::mcs96_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int data_width, const char *shortname, const char *source) :
	cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
	program_config("program", ENDIANNESS_LITTLE, data_width, 16), program(nullptr), direct(nullptr), icount(0), bcount(0), inst_state(0), cycles_scaling(0), pending_irq(0), 
	PC(0), PPC(0), PSW(0), OP1(0), OP2(0), OP3(0), OPI(0), TMP(0), irq_requested(false)
{
}

void mcs96_device::device_start()
{
	program = &space(AS_PROGRAM);
	direct = &program->direct();
	m_icountptr = &icount;

	state_add(STATE_GENPC,     "GENPC",     PC).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", PPC).noshow();
	state_add(STATE_GENSP,     "GENSP",     R[0]).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  PSW).formatstr("%16s").noshow();
	state_add(MCS96_PC,        "PC",        PC);
	state_add(MCS96_PSW,       "PSW",       PSW);
	state_add(MCS96_R,         "SP",        R[0]);
	for(int i=1; i<0x74; i++) {
		char buf[10];
		sprintf(buf, "R%02x", i*2+0x18);
		state_add(MCS96_R+i,   buf,         R[i]);
	}

	memset(R, 0, sizeof(R));
}

void mcs96_device::device_reset()
{
	PC = 0x2080;
	PPC = PC;
	PSW = 0;
	pending_irq = 0x00;
	irq_requested = false;
	inst_state = STATE_FETCH;
}

UINT32 mcs96_device::execute_min_cycles() const
{
	return 4;
}

UINT32 mcs96_device::execute_max_cycles() const
{
	return 33;
}

UINT32 mcs96_device::execute_input_lines() const
{
	return 1;
}

void mcs96_device::recompute_bcount(UINT64 event_time)
{
	if(!event_time || event_time >= total_cycles() + icount) {
		bcount = 0;
		return;
	}
	bcount = total_cycles() + icount - event_time;
}

void mcs96_device::check_irq()
{
	irq_requested = (PSW & pending_irq) && (PSW & F_I);
}

void mcs96_device::execute_run()
{
	internal_update(total_cycles());

	//  if(inst_substate)
	//      do_exec_partial();

	while(icount > 0) {
		while(icount > bcount) {
			int picount = inst_state >= 0x200 ? -1 : icount;
			do_exec_full();
			if(icount == picount) {
				fatalerror("Unhandled %x (%04x)\n", inst_state, PPC);
			}
		}
		while(bcount && icount <= bcount)
			internal_update(total_cycles() + icount - bcount);
		//      if(inst_substate)
		//          do_exec_partial();
	}
}

void mcs96_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum) {
	case EXINT_LINE:
		if(state)
			pending_irq |= 0x80;
		else
			pending_irq &= 0x7f;
		check_irq();
		break;
	}
}

const address_space_config *mcs96_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &program_config : NULL;
}

void mcs96_device::state_import(const device_state_entry &entry)
{
}

void mcs96_device::state_export(const device_state_entry &entry)
{
}

void mcs96_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case MCS96_PSW:
		strprintf(str, "%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c",
						PSW & F_Z  ? 'Z' : '.',
						PSW & F_N  ? 'N' : '.',
						PSW & F_V  ? 'V' : '.',
						PSW & F_VT ? 'v' : '.',
						PSW & F_C  ? 'C' : '.',
						PSW & F_I  ? 'I' : '.',
						PSW & F_ST ? 'S' : '.',
						PSW & 0x80 ? '7' : '.',
						PSW & 0x40 ? '6' : '.',
						PSW & 0x20 ? '5' : '.',
						PSW & 0x10 ? '4' : '.',
						PSW & 0x08 ? '3' : '.',
						PSW & 0x04 ? '2' : '.',
						PSW & 0x02 ? '1' : '.',
						PSW & 0x01 ? '0' : '.');
		break;
	}
}

std::string mcs96_device::regname(UINT8 reg)
{
	char res[32];
	switch(reg) {
	case 0x18:
		strcpy(res, "sp");
		break;

	case 0x19:
		strcpy(res, "sph");
		break;

	default:
		sprintf(res, "%02x", reg);
		break;
	}
	return res;
}

offs_t mcs96_device::disasm_generic(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options, const disasm_entry *entries)
{
	bool prefix_fe = false;
	int off = 0;
	if(oprom[0] == 0xfe && entries[oprom[1]].opcode_fe) {
		prefix_fe = true;
		pc++;
		off++;
		oprom++;
	}
	const disasm_entry &e = entries[oprom[0]];
	UINT32 flags = e.flags | DASMFLAG_SUPPORTED;
	buffer += sprintf(buffer, "%s", prefix_fe ? e.opcode_fe : e.opcode);

	switch(e.mode) {
	case DASM_none:
		flags |= 1;
		break;

	case DASM_nop_2:
		sprintf(buffer, " %02x", oprom[1]);
		flags |= 2;
		break;

	case DASM_rel8: {
		int delta = oprom[1];
		if(delta & 0x80)
			delta -= 0x100;
		sprintf(buffer, " %04x", (pc+2+delta) & 0xffff);
		flags |= 2;
		break;
	}

	case DASM_rel11: {
		int delta = ((oprom[0] << 8) | oprom[1]) & 0x7ff;
		if(delta & 0x400)
			delta -= 0x800;
		sprintf(buffer, " %04x", (pc+2+delta) & 0xffff);
		flags |= 2;
		break;
	}

	case DASM_rel16: {
		int delta = oprom[1] | (oprom[2] << 8);
		sprintf(buffer, " %04x", (pc+3+delta) & 0xffff);
		flags |= 3;
		break;
	}

	case DASM_rrel8: {
		int delta = oprom[2];
		if(delta & 0x80)
			delta -= 0x100;
		sprintf(buffer, " %s, %04x", regname(oprom[1]).c_str(), (pc+3+delta) & 0xffff);
		flags |= 3;
		break;
	}

	case DASM_brrel8: {
		int delta = oprom[2];
		if(delta & 0x80)
			delta -= 0x100;
		sprintf(buffer, " %d, %s, %04x", oprom[0] & 7, regname(oprom[1]).c_str(), (pc+3+delta) & 0xffff);
		flags |= 3;
		break;
	}

	case DASM_direct_1:
		sprintf(buffer, " %s", regname(oprom[1]).c_str());
		flags |= 2;
		break;

	case DASM_direct_2:
		sprintf(buffer, " %s, %s", regname(oprom[2]).c_str(), regname(oprom[1]).c_str());
		flags |= 3;
		break;

	case DASM_direct_3:
		sprintf(buffer, " %s, %s, %s", regname(oprom[3]).c_str(), regname(oprom[2]).c_str(), regname(oprom[1]).c_str());
		flags |= 4;
		break;

	case DASM_immed_1b:
		sprintf(buffer, " #%02x", oprom[1]);
		flags |= 2;
		break;

	case DASM_immed_2b:
		sprintf(buffer, " %s, #%02x", regname(oprom[2]).c_str(), oprom[1]);
		flags |= 3;
		break;

	case DASM_immed_or_reg_2b:
		if(oprom[1] >= 0x10)
			sprintf(buffer, " %s, %s", regname(oprom[2]).c_str(), regname(oprom[1]).c_str());
		else
			sprintf(buffer, " %s, #%02x", regname(oprom[2]).c_str(), oprom[1]);
		flags |= 3;
		break;

	case DASM_immed_3b:
		sprintf(buffer, " %s, %s, #%02x", regname(oprom[3]).c_str(), regname(oprom[2]).c_str(), oprom[1]);
		flags |= 4;
		break;

	case DASM_immed_1w:
		sprintf(buffer, " #%02x%02x", oprom[2], oprom[1]);
		flags |= 3;
		break;

	case DASM_immed_2w:
		sprintf(buffer, " %s, #%02x%02x", regname(oprom[3]).c_str(), oprom[2], oprom[1]);
		flags |= 4;
		break;

	case DASM_immed_3w:
		sprintf(buffer, " %s, %s, #%02x%02x", regname(oprom[4]).c_str(), regname(oprom[3]).c_str(), oprom[2], oprom[1]);
		flags |= 5;
		break;

	case DASM_indirect_1n:
		sprintf(buffer, " [%s]", regname(oprom[1]).c_str());
		flags |= 2;
		break;

	case DASM_indirect_1:
		if(oprom[1] & 0x01) {
			sprintf(buffer, " [%s]+", regname(oprom[1]-1).c_str());
			flags |= 2;
		} else {
			sprintf(buffer, " [%s]", regname(oprom[1]).c_str());
			flags |= 2;
		}
		break;

	case DASM_indirect_2:
		if(oprom[1] & 0x01) {
			sprintf(buffer, " %s, [%s]+", regname(oprom[2]).c_str(), regname(oprom[1]-1).c_str());
			flags |= 3;
		} else {
			sprintf(buffer, " %s, [%s]", regname(oprom[2]).c_str(), regname(oprom[1]).c_str());
			flags |= 3;
		}
		break;

	case DASM_indirect_3:
		if(oprom[1] & 0x01) {
			sprintf(buffer, " %s, %s, [%s]+", regname(oprom[3]).c_str(), regname(oprom[2]).c_str(), regname(oprom[1]-1).c_str());
			flags |= 4;
		} else {
			sprintf(buffer, " %s, %s, [%s]", regname(oprom[3]).c_str(), regname(oprom[2]).c_str(), regname(oprom[1]).c_str());
			flags |= 4;
		}
		break;

	case DASM_indexed_1:
		if(oprom[1] & 0x01) {
			if(oprom[1] == 0x01)
				sprintf(buffer, " %02x%02x", oprom[3], oprom[2]);
			else
				sprintf(buffer, " %02x%02x[%s]", oprom[3], oprom[2], regname(oprom[1]-1).c_str());
			flags |= 4;
		} else {
			int delta = oprom[2];
			if(delta & 0x80)
				delta -= 0x100;
			if(oprom[1] == 0x00) {
				if(delta < 0)
					sprintf(buffer, " %04x", delta & 0xffff);
				else
					sprintf(buffer, " %02x", delta);
			} else {
				if(delta < 0)
					sprintf(buffer, " -%02x[%s]", -delta, regname(oprom[1]).c_str());
				else
					sprintf(buffer, " %02x[%s]", delta, regname(oprom[1]).c_str());
			}
			flags |= 3;
		}
		break;

	case DASM_indexed_2:
		if(oprom[1] & 0x01) {
			if(oprom[1] == 0x01)
				sprintf(buffer, " %s, %02x%02x", regname(oprom[4]).c_str(), oprom[3], oprom[2]);
			else
				sprintf(buffer, " %s, %02x%02x[%s]", regname(oprom[4]).c_str(), oprom[3], oprom[2], regname(oprom[1]-1).c_str());
			flags |= 5;
		} else {
			int delta = oprom[2];
			if(delta & 0x80)
				delta -= 0x100;
			if(oprom[1] == 0x00) {
				if(delta < 0)
					sprintf(buffer, " %s, %04x", regname(oprom[3]).c_str(), delta & 0xffff);
				else
					sprintf(buffer, " %s, %02x", regname(oprom[3]).c_str(), delta);
			} else {
				if(delta < 0)
					sprintf(buffer, " %s, -%02x[%s]", regname(oprom[3]).c_str(), -delta, regname(oprom[1]).c_str());
				else
					sprintf(buffer, " %s, %02x[%s]", regname(oprom[3]).c_str(), delta, regname(oprom[1]).c_str());
			}
			flags |= 4;
		}
		break;

	case DASM_indexed_3:
		if(oprom[1] & 0x01) {
			if(oprom[1] == 0x01)
				sprintf(buffer, " %s, %s, %02x%02x", regname(oprom[5]).c_str(),  regname(oprom[4]).c_str(), oprom[3], oprom[2]);
			else
				sprintf(buffer, " %s, %s, %02x%02x[%s]", regname(oprom[5]).c_str(), regname(oprom[4]).c_str(), oprom[3], oprom[2], regname(oprom[1]-1).c_str());
			flags |= 6;
		} else {
			int delta = oprom[2];
			if(delta & 0x80)
				delta -= 0x100;
			if(oprom[1] == 0x00) {
				if(delta < 0)
					sprintf(buffer, " %s, %s, %04x", regname(oprom[4]).c_str(), regname(oprom[3]).c_str(), delta & 0xffff);
				else
					sprintf(buffer, " %s, %s, %02x", regname(oprom[4]).c_str(), regname(oprom[3]).c_str(), delta);
			} else {
				if(delta < 0)
					sprintf(buffer, " %s, %s, -%02x[%s]", regname(oprom[4]).c_str(), regname(oprom[3]).c_str(), -delta, regname(oprom[1]).c_str());
				else
					sprintf(buffer, " %s, %s, %02x[%s]", regname(oprom[4]).c_str(), regname(oprom[3]).c_str(), delta, regname(oprom[1]).c_str());
			}
			flags |= 5;
		}
		break;

	default:
		fprintf(stderr, "Unhandled dasm mode %d\n", e.mode);
		abort();
	};

	return flags+off;
}

UINT32 mcs96_device::disasm_min_opcode_bytes() const
{
	return 1;
}

UINT32 mcs96_device::disasm_max_opcode_bytes() const
{
	return 7;
}

void mcs96_device::io_w8(UINT8 adr, UINT8 data)
{
	switch(adr) {
	case 0x02:
		logerror("%s: ad_command %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x03:
		logerror("%s: hsi_mode %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x04:
		logerror("%s: hso_time.l %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x05:
		logerror("%s: hso_time.h %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x06:
		logerror("%s: hso_command %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x07:
		logerror("%s: sbuf %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x08:
		PSW = (PSW & 0xff00) | data;
		break;
	case 0x09:
		logerror("%s: int_pending %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x0a:
		logerror("%s: watchdog %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x0e:
		logerror("%s: baud rate %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x0f:
		logerror("%s: io port 1 %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x10:
		logerror("%s: io port 2 %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x11:
		logerror("%s: sp con %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x15:
		logerror("%s: ioc0 %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x16:
		logerror("%s: ioc1 %02x (%04x)\n", tag(), data, PPC);
		break;
	case 0x17:
		logerror("%s: pwm control %02x (%04x)\n", tag(), data, PPC);
		break;
	}
	return;
}

void mcs96_device::io_w16(UINT8 adr, UINT16 data)
{
	switch(adr) {
	case 0:
		break;
	case 4:
		logerror("%s: hso_time %04x (%04x)\n", tag(), data, PPC);
		break;
	default:
		io_w8(adr, data);
		io_w8(adr+1, data>>8);
		break;
	}
	return;
}

UINT8 mcs96_device::io_r8(UINT8 adr)
{
	switch(adr) {
	case 0x00:
		return 0x00;
	case 0x01:
		return 0x00;
	case 0x08:
		return PSW;
	}
	UINT8 data = 0x00;
	logerror("%s: io_r8 %02x, %02x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

UINT16 mcs96_device::io_r16(UINT8 adr)
{
	if(adr < 2)
		return 0x0000;
	UINT16 data = 0x0000;
	logerror("%s: io_r16 %02x, %04x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

void mcs96_device::reg_w8(UINT8 adr, UINT8 data)
{
	if(adr < 0x18)
		io_w8(adr, data);
	else {
		UINT16 &r = R[(adr - 0x18) >> 1];
		if(adr & 0x01)
			r = (r & 0x00ff) | (data << 8);
		else
			r = (r & 0xff00) | data;
	}
}

void mcs96_device::reg_w16(UINT8 adr, UINT16 data)
{
	adr &= 0xfe;
	if(adr < 0x18)
		io_w16(adr, data);
	else
		R[(adr-0x18) >> 1] = data;
}

UINT8 mcs96_device::reg_r8(UINT8 adr)
{
	if(adr < 0x18)
		return io_r8(adr);

	UINT16 data = R[(adr - 0x18) >> 1];
	if(adr & 0x01)
		return data >> 8;
	else
		return data;
}

UINT16 mcs96_device::reg_r16(UINT8 adr)
{
	adr &= 0xfe;
	if(adr < 0x18)
		return io_r16(adr);

	return R[(adr-0x18) >> 1];
}

void mcs96_device::any_w8(UINT16 adr, UINT8 data)
{
	if(adr < 0x18)
		io_w8(adr, data);
	else if(adr < 0x100) {
		UINT16 &r = R[(adr - 0x18) >> 1];
		if(adr & 0x01)
			r = (r & 0x00ff) | (data << 8);
		else
			r = (r & 0xff00) | data;
	} else
		program->write_byte(adr, data);
}

void mcs96_device::any_w16(UINT16 adr, UINT16 data)
{
	adr &= 0xfffe;
	if(adr < 0x18)
		io_w16(adr, data);
	else if(adr < 0x100)
		R[(adr-0x18) >> 1] = data;
	else
		program->write_word(adr, data);
}

UINT8 mcs96_device::any_r8(UINT16 adr)
{
	if(adr < 0x18)
		return io_r8(adr);
	else if(adr < 0x100) {
		UINT16 data = R[(adr - 0x18) >> 1];
		if(adr & 0x01)
			return data >> 8;
		else
			return data;
	} else
		return program->read_byte(adr);
}

UINT16 mcs96_device::any_r16(UINT16 adr)
{
	adr &= 0xfffe;
	if(adr < 0x18)
		return io_r16(adr);
	else if(adr < 0x100)
		return R[(adr-0x18) >> 1];
	else
		return program->read_word(adr);
}

UINT8 mcs96_device::do_addb(UINT8 v1, UINT8 v2)
{
	UINT16 sum = v1+v2;
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!UINT8(sum))
		PSW |= F_Z;
	else if(INT8(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x80)
		PSW |= F_V|F_VT;
	if(sum & 0xff00)
		PSW |= F_C;
	return sum;
}

UINT16 mcs96_device::do_add(UINT16 v1, UINT16 v2)
{
	UINT32 sum = v1+v2;
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!UINT16(sum))
		PSW |= F_Z;
	else if(INT16(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x8000)
		PSW |= F_V|F_VT;
	if(sum & 0xffff0000)
		PSW |= F_C;
	return sum;
}

UINT8 mcs96_device::do_subb(UINT8 v1, UINT8 v2)
{
	UINT16 diff = v1 - v2;
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!UINT8(diff))
		PSW |= F_Z;
	else if(INT8(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x80)
		PSW |= F_V;
	if(!(diff & 0xff00))
		PSW |= F_C;
	return diff;
}

UINT16 mcs96_device::do_sub(UINT16 v1, UINT16 v2)
{
	UINT32 diff = v1 - v2;
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!UINT16(diff))
		PSW |= F_Z;
	else if(INT16(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x8000)
		PSW |= F_V;
	if(!(diff & 0xffff0000))
		PSW |= F_C;
	return diff;
}

UINT8 mcs96_device::do_addcb(UINT8 v1, UINT8 v2)
{
	UINT16 sum = v1+v2+(PSW & F_C ? 1 : 0);
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!UINT8(sum))
		PSW |= F_Z;
	else if(INT8(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x80)
		PSW |= F_V|F_VT;
	if(sum & 0xff00)
		PSW |= F_C;
	return sum;
}

UINT16 mcs96_device::do_addc(UINT16 v1, UINT16 v2)
{
	UINT32 sum = v1+v2+(PSW & F_C ? 1 : 0);
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!UINT16(sum))
		PSW |= F_Z;
	else if(INT16(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x8000)
		PSW |= F_V|F_VT;
	if(sum & 0xffff0000)
		PSW |= F_C;
	return sum;
}

UINT8 mcs96_device::do_subcb(UINT8 v1, UINT8 v2)
{
	UINT16 diff = v1 - v2 - (PSW & F_C ? 0 : 1);
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!UINT8(diff))
		PSW |= F_Z;
	else if(INT8(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x80)
		PSW |= F_V;
	if(!(diff & 0xff00))
		PSW |= F_C;
	return diff;
}

UINT16 mcs96_device::do_subc(UINT16 v1, UINT16 v2)
{
	UINT32 diff = v1 - v2 - (PSW & F_C ? 0 : 1);
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!UINT16(diff))
		PSW |= F_Z;
	else if(INT16(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x8000)
		PSW |= F_V;
	if(!(diff & 0xffff0000))
		PSW |= F_C;
	return diff;
}

void mcs96_device::set_nz8(UINT8 v)
{
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!v)
		PSW |= F_Z;
	else if(INT8(v) < 0)
		PSW |= F_N;
}

void mcs96_device::set_nz16(UINT16 v)
{
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!v)
		PSW |= F_Z;
	else if(INT16(v) < 0)
		PSW |= F_N;
}

#include "cpu/mcs96/mcs96.inc"
