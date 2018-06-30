// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    mcs96.h

    MCS96, 8098/8398/8798 branch

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "mcs96.h"

mcs96_device::mcs96_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int data_width) :
	cpu_device(mconfig, type, tag, owner, clock),
	program_config("program", ENDIANNESS_LITTLE, data_width, 16),
	program(nullptr), icount(0), bcount(0), inst_state(0), cycles_scaling(0), pending_irq(0),
	PC(0), PPC(0), PSW(0), OP1(0), OP2(0), OP3(0), OPI(0), TMP(0), irq_requested(false)
{
}

void mcs96_device::device_start()
{
	program = &space(AS_PROGRAM);
	if(program->data_width() == 8) {
		auto cache = program->cache<0, 0, ENDIANNESS_LITTLE>();
		m_pr8 = [cache](offs_t address) -> u8 { return cache->read_byte(address); };
	} else {
		auto cache = program->cache<1, 0, ENDIANNESS_LITTLE>();
		m_pr8 = [cache](offs_t address) -> u8 { return cache->read_byte(address); };
	}

	set_icountptr(icount);

	state_add(STATE_GENPC,     "GENPC",     PC).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     PPC).noshow();
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

uint32_t mcs96_device::execute_min_cycles() const
{
	return 4;
}

uint32_t mcs96_device::execute_max_cycles() const
{
	return 33;
}

uint32_t mcs96_device::execute_input_lines() const
{
	return 1;
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

device_memory_interface::space_config_vector mcs96_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &program_config)
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

void mcs96_device::io_w8(uint8_t adr, uint8_t data)
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

void mcs96_device::io_w16(uint8_t adr, uint16_t data)
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

uint8_t mcs96_device::io_r8(uint8_t adr)
{
	switch(adr) {
	case 0x00:
		return 0x00;
	case 0x01:
		return 0x00;
	case 0x08:
		return PSW;
	}
	uint8_t data = 0x00;
	logerror("%s: io_r8 %02x, %02x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

uint16_t mcs96_device::io_r16(uint8_t adr)
{
	if(adr < 2)
		return 0x0000;
	uint16_t data = 0x0000;
	logerror("%s: io_r16 %02x, %04x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

void mcs96_device::reg_w8(uint8_t adr, uint8_t data)
{
	if(adr < 0x18)
		io_w8(adr, data);
	else {
		uint16_t &r = R[(adr - 0x18) >> 1];
		if(adr & 0x01)
			r = (r & 0x00ff) | (data << 8);
		else
			r = (r & 0xff00) | data;
	}
}

void mcs96_device::reg_w16(uint8_t adr, uint16_t data)
{
	adr &= 0xfe;
	if(adr < 0x18)
		io_w16(adr, data);
	else
		R[(adr-0x18) >> 1] = data;
}

uint8_t mcs96_device::reg_r8(uint8_t adr)
{
	if(adr < 0x18)
		return io_r8(adr);

	uint16_t data = R[(adr - 0x18) >> 1];
	if(adr & 0x01)
		return data >> 8;
	else
		return data;
}

uint16_t mcs96_device::reg_r16(uint8_t adr)
{
	adr &= 0xfe;
	if(adr < 0x18)
		return io_r16(adr);

	return R[(adr-0x18) >> 1];
}

void mcs96_device::any_w8(uint16_t adr, uint8_t data)
{
	if(adr < 0x18)
		io_w8(adr, data);
	else if(adr < 0x100) {
		uint16_t &r = R[(adr - 0x18) >> 1];
		if(adr & 0x01)
			r = (r & 0x00ff) | (data << 8);
		else
			r = (r & 0xff00) | data;
	} else
		program->write_byte(adr, data);
}

void mcs96_device::any_w16(uint16_t adr, uint16_t data)
{
	adr &= 0xfffe;
	if(adr < 0x18)
		io_w16(adr, data);
	else if(adr < 0x100)
		R[(adr-0x18) >> 1] = data;
	else
		program->write_word(adr, data);
}

uint8_t mcs96_device::any_r8(uint16_t adr)
{
	if(adr < 0x18)
		return io_r8(adr);
	else if(adr < 0x100) {
		uint16_t data = R[(adr - 0x18) >> 1];
		if(adr & 0x01)
			return data >> 8;
		else
			return data;
	} else
		return program->read_byte(adr);
}

uint16_t mcs96_device::any_r16(uint16_t adr)
{
	adr &= 0xfffe;
	if(adr < 0x18)
		return io_r16(adr);
	else if(adr < 0x100)
		return R[(adr-0x18) >> 1];
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
