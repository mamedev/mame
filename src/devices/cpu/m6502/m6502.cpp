// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.cpp

    MOS Technology 6502, original NMOS variant

***************************************************************************/

#include "emu.h"
#include "m6502.h"
#include "m6502d.h"

DEFINE_DEVICE_TYPE(M6502, m6502_device, "m6502", "MOS Technology 6502")
DEFINE_DEVICE_TYPE(M6512, m6512_device, "m6512", "MOS Technology 6512")

m6502_device::m6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502, tag, owner, clock)
{
}

m6512_device::m6512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6512, tag, owner, clock)
{
}

m6502_device::m6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, type, tag, owner, clock),
	sync_w(*this),
	program_config("program", ENDIANNESS_LITTLE, 8, 16),
	sprogram_config("decrypted_opcodes", ENDIANNESS_LITTLE, 8, 16),
	mintf(nullptr),
	uses_custom_memory_interface(false)
{
}

void m6502_device::device_start()
{
	if(!uses_custom_memory_interface)
		mintf = space(AS_PROGRAM).addr_width() > 14 ? std::make_unique<mi_default>() : std::make_unique<mi_default14>();

	init();
}

void m6502_device::init()
{
	if(mintf) {
		space(AS_PROGRAM).cache(mintf->cprogram);
		space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(mintf->csprogram);

		// specific group 1-14 or 15-31
		if(space(AS_PROGRAM).addr_width() > 14)
			space(AS_PROGRAM).specific(mintf->program);
		else
			space(AS_PROGRAM).specific(mintf->program14);
	}

	state_add(STATE_GENPC,     "GENPC",     XPC).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     XPC).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  P).callimport().formatstr("%6s").noshow();
	state_add(M6502_PC,        "PC",        NPC).callimport();
	state_add(M6502_A,         "A",         A);
	state_add(M6502_X,         "X",         X);
	state_add(M6502_Y,         "Y",         Y);
	state_add(M6502_P,         "P",         P).callimport();
	state_add(M6502_S,         "SP",        SP);
	state_add(M6502_IR,        "IR",        IR);

	save_item(NAME(PC));
	save_item(NAME(NPC));
	save_item(NAME(PPC));
	save_item(NAME(A));
	save_item(NAME(X));
	save_item(NAME(Y));
	save_item(NAME(P));
	save_item(NAME(SP));
	save_item(NAME(TMP));
	save_item(NAME(TMP2));
	save_item(NAME(IR));
	save_item(NAME(nmi_state));
	save_item(NAME(irq_state));
	save_item(NAME(apu_irq_state));
	save_item(NAME(v_state));
	save_item(NAME(nmi_pending));
	save_item(NAME(irq_taken));
	save_item(NAME(inst_state));
	save_item(NAME(inst_substate));
	save_item(NAME(inst_state_base));
	save_item(NAME(inhibit_interrupts));

	set_icountptr(icount);

	XPC = 0x0000;
	PPC = 0x0000;
	PC = 0x0000;
	NPC = 0x0000;
	A = 0x00;
	X = 0x80;
	Y = 0x00;
	P = 0x36;
	SP = 0x0100;
	TMP = 0x0000;
	TMP2 = 0x00;
	IR = 0x00;
	nmi_state = false;
	irq_state = false;
	apu_irq_state = false;
	v_state = false;
	nmi_pending = false;
	irq_taken = false;
	inst_state = STATE_RESET;
	inst_substate = 0;
	inst_state_base = 0;
	sync = false;
	inhibit_interrupts = false;
	count_before_instruction_step = 0;
}

void m6502_device::device_reset()
{
	inst_state = STATE_RESET;
	inst_substate = 0;
	inst_state_base = 0;
	nmi_pending = false;
	irq_taken = false;
	sync = false;
	sync_w(CLEAR_LINE);
	inhibit_interrupts = false;
}


uint32_t m6502_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t m6502_device::execute_max_cycles() const noexcept
{
	return 10;
}

bool m6502_device::execute_input_edge_triggered(int inputnum) const noexcept
{
	return inputnum == NMI_LINE || inputnum == V_LINE;
}

void m6502_device::do_adc_d(uint8_t val)
{
	uint8_t c = P & F_C ? 1 : 0;
	P &= ~(F_N|F_V|F_Z|F_C);
	uint8_t al = (A & 15) + (val & 15) + c;
	if(al > 9)
		al += 6;
	uint8_t ah = (A >> 4) + (val >> 4) + (al > 15);
	if(!uint8_t(A + val + c))
		P |= F_Z;
	else if(ah & 8)
		P |= F_N;
	if(~(A^val) & (A^(ah << 4)) & 0x80)
		P |= F_V;
	if(ah > 9)
		ah += 6;
	if(ah > 15)
		P |= F_C;
	A = (ah << 4) | (al & 15);
}

void m6502_device::do_adc_nd(uint8_t val)
{
	uint16_t sum;
	sum = A + val + (P & F_C ? 1 : 0);
	P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(sum))
		P |= F_Z;
	else if(int8_t(sum) < 0)
		P |= F_N;
	if(~(A^val) & (A^sum) & 0x80)
		P |= F_V;
	if(sum & 0xff00)
		P |= F_C;
	A = sum;
}

void m6502_device::do_adc(uint8_t val)
{
	if(P & F_D)
		do_adc_d(val);
	else
		do_adc_nd(val);
}

void m6502_device::do_arr_nd()
{
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C|F_V);
	A >>= 1;
	if(c)
		A |= 0x80;
	if(!A)
		P |= F_Z;
	else if(int8_t(A)<0)
		P |= F_N;
	if(A & 0x40)
		P |= F_V|F_C;
	if(A & 0x20)
		P ^= F_V;
}

void m6502_device::do_arr_d()
{
	// The adc/ror interaction gives an extremely weird result
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C|F_V);
	uint8_t a = A >> 1;
	if(c)
		a |= 0x80;
	if(!a)
		P |= F_Z;
	else if(int8_t(a) < 0)
		P |= F_N;
	if((a ^ A) & 0x40)
		P |= F_V;

	if((A & 0x0f) >= 0x05)
		a = ((a + 6) & 0x0f) | (a & 0xf0);

	if((A & 0xf0) >= 0x50) {
		a += 0x60;
		P |= F_C;
	}
	A = a;
}

void m6502_device::do_arr()
{
	if(P & F_D)
		do_arr_d();
	else
		do_arr_nd();
}

void m6502_device::do_cmp(uint8_t val1, uint8_t val2)
{
	P &= ~(F_N|F_Z|F_C);
	uint16_t r = val1-val2;
	if(!r)
		P |= F_Z;
	else if(int8_t(r) < 0)
		P |= F_N;
	if(!(r & 0xff00))
		P |= F_C;
}

void m6502_device::do_sbc_d(uint8_t val)
{
	uint8_t c = P & F_C ? 0 : 1;
	P &= ~(F_N|F_V|F_Z|F_C);
	uint16_t diff = A - val - c;
	uint8_t al = (A & 15) - (val & 15) - c;
	if(int8_t(al) < 0)
		al -= 6;
	uint8_t ah = (A >> 4) - (val >> 4) - (int8_t(al) < 0);
	if(!uint8_t(diff))
		P |= F_Z;
	else if(diff & 0x80)
		P |= F_N;
	if((A^val) & (A^diff) & 0x80)
		P |= F_V;
	if(!(diff & 0xff00))
		P |= F_C;
	if(int8_t(ah) < 0)
		ah -= 6;
	A = (ah << 4) | (al & 15);
}

void m6502_device::do_sbc_nd(uint8_t val)
{
	uint16_t diff = A - val - (P & F_C ? 0 : 1);
	P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(diff))
		P |= F_Z;
	else if(int8_t(diff) < 0)
		P |= F_N;
	if((A^val) & (A^diff) & 0x80)
		P |= F_V;
	if(!(diff & 0xff00))
		P |= F_C;
	A = diff;
}

void m6502_device::do_sbc(uint8_t val)
{
	if(P & F_D)
		do_sbc_d(val);
	else
		do_sbc_nd(val);
}

void m6502_device::do_bit(uint8_t val)
{
	P &= ~(F_N|F_Z|F_V);
	uint8_t r = A & val;
	if(!r)
		P |= F_Z;
	if(val & 0x80)
		P |= F_N;
	if(val & 0x40)
		P |= F_V;
}

uint8_t m6502_device::do_asl(uint8_t v)
{
	P &= ~(F_N|F_Z|F_C);
	uint8_t r = v<<1;
	if(!r)
		P |= F_Z;
	else if(int8_t(r) < 0)
		P |= F_N;
	if(v & 0x80)
		P |= F_C;
	return r;
}

uint8_t m6502_device::do_lsr(uint8_t v)
{
	P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		P |= F_C;
	v >>= 1;
	if(!v)
		P |= F_Z;
	return v;
}

uint8_t m6502_device::do_ror(uint8_t v)
{
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		P |= F_C;
	v >>= 1;
	if(c)
		v |= 0x80;
	if(!v)
		P |= F_Z;
	else if(int8_t(v)<0)
		P |= F_N;
	return v;
}

uint8_t m6502_device::do_rol(uint8_t v)
{
	bool c = P & F_C;
	P &= ~(F_N|F_Z|F_C);
	if(v & 0x80)
		P |= F_C;
	v <<= 1;
	if(c)
		v |= 0x01;
	if(!v)
		P |= F_Z;
	else if(int8_t(v)<0)
		P |= F_N;
	return v;
}

uint8_t m6502_device::do_asr(uint8_t v)
{
	P &= ~(F_N|F_Z|F_C);
	if(v & 1)
		P |= F_C;
	v >>= 1;
	if(!v)
		P |= F_Z;
	else if(v & 0x40) {
		P |= F_N;
		v |= 0x80;
	}
	return v;
}

offs_t m6502_device::pc_to_external(u16 pc)
{
	return pc;
}

void m6502_device::execute_run()
{
	if(inst_substate)
		do_exec_partial();

	while(icount > 0) {
		if(inst_state < 0xff00) {
			PPC = NPC;
			inst_state = IR | inst_state_base;
			if(machine().debug_flags & DEBUG_FLAG_ENABLED)
				debugger_instruction_hook(pc_to_external(NPC));
		}
		do_exec_full();
	}
}

void m6502_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum) {
	case IRQ_LINE: irq_state = state == ASSERT_LINE; break;
	case APU_IRQ_LINE: apu_irq_state = state == ASSERT_LINE; break;
	case NMI_LINE:
		if(machine().time() > attotime::zero && !nmi_state && state == ASSERT_LINE)
			nmi_pending = true;
		nmi_state = state == ASSERT_LINE;
		break;
	case V_LINE:
		if(machine().time() > attotime::zero && !v_state && state == ASSERT_LINE)
			P |= F_V;
		v_state = state == ASSERT_LINE;
		break;
	}
}


device_memory_interface::space_config_vector m6502_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_OPCODES, &sprogram_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config)
		};
}


void m6502_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		P = P | (F_B|F_E);
		break;
	case M6502_PC:
		PC = NPC;
		irq_taken = false;
		prefetch_start();
		IR = mintf->read_sync(PC);
		prefetch_end();
		PPC = NPC;
		inst_state = IR | inst_state_base;
		break;
	}
}

void m6502_device::state_export(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENPC:     XPC = pc_to_external(PPC); break;
	case STATE_GENPCBASE: XPC = pc_to_external(NPC); break;
	}
}

void m6502_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		str = string_format("%c%c%c%c%c%c",
						P & F_N ? 'N' : '.',
						P & F_V ? 'V' : '.',
						P & F_D ? 'D' : '.',
						P & F_I ? 'I' : '.',
						P & F_Z ? 'Z' : '.',
						P & F_C ? 'C' : '.');
		break;
	}
}

void m6502_device::prefetch_start()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
}

void m6502_device::prefetch_end()
{
	sync = false;
	sync_w(CLEAR_LINE);

	if((nmi_pending || ((irq_state || apu_irq_state) && !(P & F_I))) && !inhibit_interrupts) {
		irq_taken = true;
		IR = 0x00;
	} else
		PC++;
}

void m6502_device::prefetch_end_noirq()
{
	sync = false;
	sync_w(CLEAR_LINE);
	PC++;
}

void m6502_device::set_nz(uint8_t v)
{
	P &= ~(F_Z|F_N);
	if(v & 0x80)
		P |= F_N;
	if(!v)
		P |= F_Z;
}

std::unique_ptr<util::disasm_interface> m6502_device::create_disassembler()
{
	return std::make_unique<m6502_disassembler>();
}

uint8_t m6502_device::memory_interface::read_9(uint16_t adr)
{
	return read(adr);
}

void m6502_device::memory_interface::write_9(uint16_t adr, uint8_t val)
{
	write(adr, val);
}


uint8_t m6502_device::mi_default::read(uint16_t adr)
{
	return program.read_byte(adr);
}

uint8_t m6502_device::mi_default::read_sync(uint16_t adr)
{
	return csprogram.read_byte(adr);
}

uint8_t m6502_device::mi_default::read_arg(uint16_t adr)
{
	return cprogram.read_byte(adr);
}

void m6502_device::mi_default::write(uint16_t adr, uint8_t val)
{
	program.write_byte(adr, val);
}

uint8_t m6502_device::mi_default14::read(uint16_t adr)
{
	return program14.read_byte(adr);
}

void m6502_device::mi_default14::write(uint16_t adr, uint8_t val)
{
	program14.write_byte(adr, val);
}


#include "cpu/m6502/m6502.hxx"
