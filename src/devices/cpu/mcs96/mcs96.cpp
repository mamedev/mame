// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    mcs96.h

    MCS96, 8098/8398/8798 branch

***************************************************************************/

#include "emu.h"
#include "mcs96.h"

mcs96_device::mcs96_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int data_width, address_map_constructor regs_map) :
	cpu_device(mconfig, type, tag, owner, clock),
	program_config("program", ENDIANNESS_LITTLE, data_width, 16),
	regs_config("register", ENDIANNESS_LITTLE, 16, 8, 0, regs_map),
	program(nullptr), regs(nullptr), register_file(*this, "register_file"),
	icount(0), bcount(0), inst_state(0), cycles_scaling(0), pending_irq(0),
	PC(0), PPC(0), PSW(0), OP1(0), OP2(0), OP3(0), OPI(0), TMP(0), irq_requested(false)
{
}

void mcs96_device::device_start()
{
	program = &space(AS_PROGRAM);
	if(program->data_width() == 8) {
		program->cache(m_cache8);
		m_pr8 = [this](offs_t address) -> u8 { return m_cache8.read_byte(address); };
	} else {
		program->cache(m_cache16);
		m_pr8 = [this](offs_t address) -> u8 { return m_cache16.read_byte(address); };
	}
	regs = &space(AS_DATA);

	set_icountptr(icount);

	auto register_file_bytes = util::little_endian_cast<u8>(register_file.target());
	state_add(STATE_GENPC,     "GENPC",     PC).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     PPC).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  PSW).formatstr("%16s").noshow();
	state_add(MCS96_PC,        "PC",        PC);
	state_add(MCS96_PSW,       "PSW",       PSW);
	state_add(MCS96_INT_PENDING, "INT_PENDING", pending_irq);
	state_add(MCS96_SP,        "SP",        register_file[0]);
	state_add(MCS96_AX,        "AX",        register_file[2]);
	state_add(MCS96_DX,        "DX",        register_file[3]);
	state_add(MCS96_BX,        "BX",        register_file[4]);
	state_add(MCS96_CX,        "CX",        register_file[5]);
	state_add(MCS96_AL,        "AL",        register_file_bytes[4]).noshow();
	state_add(MCS96_AH,        "AH",        register_file_bytes[5]).noshow();
	state_add(MCS96_DL,        "DL",        register_file_bytes[6]).noshow();
	state_add(MCS96_DH,        "DH",        register_file_bytes[7]).noshow();
	state_add(MCS96_BL,        "BL",        register_file_bytes[8]).noshow();
	state_add(MCS96_BH,        "BH",        register_file_bytes[9]).noshow();
	state_add(MCS96_CL,        "CL",        register_file_bytes[10]).noshow();
	state_add(MCS96_CH,        "CH",        register_file_bytes[11]).noshow();

	save_item(NAME(inst_state));
	save_item(NAME(pending_irq));
	save_item(NAME(PC));
	save_item(NAME(PPC));
	save_item(NAME(PSW));
	save_item(NAME(OP1));
	save_item(NAME(OP2));
	save_item(NAME(OP3));
	save_item(NAME(OPI));
	save_item(NAME(TMP));
	save_item(NAME(irq_requested));
}

void mcs96_device::device_reset()
{
	PC = 0x2080;
	PPC = PC;
	PSW = 0;
	irq_requested = false;
	inst_state = STATE_FETCH;
}

uint32_t mcs96_device::execute_min_cycles() const noexcept
{
	return 4;
}

uint32_t mcs96_device::execute_max_cycles() const noexcept
{
	return 33;
}

void mcs96_device::recompute_bcount(uint64_t event_time)
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

void mcs96_device::int_mask_w(u8 data)
{
	PSW = (PSW & 0xff00) | data;
	check_irq();
}

u8 mcs96_device::int_mask_r()
{
	return PSW;
}

void mcs96_device::int_pending_w(u8 data)
{
	pending_irq = data;
	check_irq();
}

u8 mcs96_device::int_pending_r()
{
	return pending_irq;
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

device_memory_interface::space_config_vector mcs96_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &program_config),
		std::make_pair(AS_DATA, &regs_config)
	};
}

void mcs96_device::state_import(const device_state_entry &entry)
{
}

void mcs96_device::state_export(const device_state_entry &entry)
{
}

void mcs96_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case MCS96_PSW:
		str = string_format("%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c",
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

void mcs96_device::reg_w8(u8 adr, u8 data)
{
	regs->write_byte(adr, data);
}

void mcs96_device::reg_w16(u8 adr, u16 data)
{
	regs->write_word(adr & 0xfe, data);
}

uint8_t mcs96_device::reg_r8(uint8_t adr)
{
	return regs->read_byte(adr);
}

uint16_t mcs96_device::reg_r16(uint8_t adr)
{
	return regs->read_word(adr & 0xfe);
}

void mcs96_device::any_w8(u16 adr, u8 data)
{
	if (adr < 0x100)
		regs->write_byte(adr, data);
	else
		program->write_byte(adr, data);
}

void mcs96_device::any_w16(u16 adr, u16 data)
{
	adr &= 0xfffe;
	if (adr < 0x100)
		regs->write_word(adr, data);
	else
		program->write_word(adr, data);
}

u8 mcs96_device::any_r8(u16 adr)
{
	if (adr < 0x100)
		return regs->read_byte(adr);
	else
		return program->read_byte(adr);
}

u16 mcs96_device::any_r16(u16 adr)
{
	adr &= 0xfffe;
	if (adr < 0x100)
		return regs->read_word(adr);
	else
		return program->read_word(adr);
}

uint8_t mcs96_device::do_addb(uint8_t v1, uint8_t v2)
{
	uint16_t sum = v1+v2;
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!uint8_t(sum))
		PSW |= F_Z;
	else if(int8_t(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x80)
		PSW |= F_V|F_VT;
	if(sum & 0xff00)
		PSW |= F_C;
	return sum;
}

uint16_t mcs96_device::do_add(uint16_t v1, uint16_t v2)
{
	uint32_t sum = v1+v2;
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!uint16_t(sum))
		PSW |= F_Z;
	else if(int16_t(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x8000)
		PSW |= F_V|F_VT;
	if(sum & 0xffff0000)
		PSW |= F_C;
	return sum;
}

uint8_t mcs96_device::do_subb(uint8_t v1, uint8_t v2)
{
	uint16_t diff = v1 - v2;
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(diff))
		PSW |= F_Z;
	else if(int8_t(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x80)
		PSW |= F_V;
	if(!(diff & 0xff00))
		PSW |= F_C;
	return diff;
}

uint16_t mcs96_device::do_sub(uint16_t v1, uint16_t v2)
{
	uint32_t diff = v1 - v2;
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!uint16_t(diff))
		PSW |= F_Z;
	else if(int16_t(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x8000)
		PSW |= F_V;
	if(!(diff & 0xffff0000))
		PSW |= F_C;
	return diff;
}

uint8_t mcs96_device::do_addcb(uint8_t v1, uint8_t v2)
{
	uint16_t sum = v1+v2+(PSW & F_C ? 1 : 0);
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!uint8_t(sum))
		PSW |= F_Z;
	else if(int8_t(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x80)
		PSW |= F_V|F_VT;
	if(sum & 0xff00)
		PSW |= F_C;
	return sum;
}

uint16_t mcs96_device::do_addc(uint16_t v1, uint16_t v2)
{
	uint32_t sum = v1+v2+(PSW & F_C ? 1 : 0);
	PSW &= ~(F_Z|F_N|F_C|F_V);
	if(!uint16_t(sum))
		PSW |= F_Z;
	else if(int16_t(sum) < 0)
		PSW |= F_N;
	if(~(v1^v2) & (v1^sum) & 0x8000)
		PSW |= F_V|F_VT;
	if(sum & 0xffff0000)
		PSW |= F_C;
	return sum;
}

uint8_t mcs96_device::do_subcb(uint8_t v1, uint8_t v2)
{
	uint16_t diff = v1 - v2 - (PSW & F_C ? 0 : 1);
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(diff))
		PSW |= F_Z;
	else if(int8_t(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x80)
		PSW |= F_V;
	if(!(diff & 0xff00))
		PSW |= F_C;
	return diff;
}

uint16_t mcs96_device::do_subc(uint16_t v1, uint16_t v2)
{
	uint32_t diff = v1 - v2 - (PSW & F_C ? 0 : 1);
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!uint16_t(diff))
		PSW |= F_Z;
	else if(int16_t(diff) < 0)
		PSW |= F_N;
	if((v1^v2) & (v1^diff) & 0x8000)
		PSW |= F_V;
	if(!(diff & 0xffff0000))
		PSW |= F_C;
	return diff;
}

void mcs96_device::set_nz8(uint8_t v)
{
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!v)
		PSW |= F_Z;
	else if(int8_t(v) < 0)
		PSW |= F_N;
}

void mcs96_device::set_nz16(uint16_t v)
{
	PSW &= ~(F_N|F_V|F_Z|F_C);
	if(!v)
		PSW |= F_Z;
	else if(int16_t(v) < 0)
		PSW |= F_N;
}

#include "cpu/mcs96/mcs96.hxx"
