// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sanyo LC6554

    4-bit microcontroller belonging to Sanyo’s LC6500 series

    TODO:
    - Unimplemented instructions
    - External interrupts
    - Serial port
    - Pseudo-port functions
    - Control register flags other than CTL_TIMER_INT_ENABLE
    - Mask options (prescaler, port defaults, etc.)

***************************************************************************/

#include "emu.h"
#include "lc6554.h"
#include "lc6500_dasm.h"

#define LOG_GP  (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


//**************************************************************************
//  INSTRUCTION DECODE TABLE
//**************************************************************************

#define OP(name) &lc6554_cpu_device::op_##name

const lc6554_cpu_device::opcode_info lc6554_cpu_device::m_opcode_table[] =
{
	{ OP(nop), 1, 1 }, { OP(ral),  1, 1 }, { OP(s),       1, 1 }, { OP(tae),     1, 1 }, { OP(spb),  1, 2 }, { OP(spb),     1, 2 }, { OP(spb),  1, 2 }, { OP(spb), 1, 2 }, // 0x00
	{ OP(smb), 1, 1 }, { OP(smb),  1, 1 }, { OP(smb),     1, 1 }, { OP(smb),     1, 1 }, { OP(ip),   1, 1 }, { OP(xae),     1, 1 }, { OP(inc),  1, 1 }, { OP(dec), 1, 1 }, // 0x08
	{ OP(rfb), 1, 1 }, { OP(rfb),  1, 1 }, { OP(rfb),     1, 1 }, { OP(rfb),     1, 1 }, { OP(rfb),  1, 1 }, { OP(rfb),     1, 1 }, { OP(rfb),  1, 1 }, { OP(rfb), 1, 1 }, // 0x10
	{ OP(rfb), 1, 1 }, { OP(rfb),  1, 1 }, { OP(rfb),     1, 1 }, { OP(rfb),     1, 1 }, { OP(rfb),  1, 1 }, { OP(rfb),     1, 1 }, { OP(rfb),  1, 1 }, { OP(rfb), 1, 1 }, // 0x18
	{ OP(adc), 1, 1 }, { OP(l),    1, 1 }, { OP(rti),     1, 1 }, { OP(xah),     1, 1 }, { OP(rpb),  1, 2 }, { OP(rpb),     1, 2 }, { OP(rpb),  1, 2 }, { OP(rpb), 1, 2 }, // 0x20
	{ OP(rmb), 1, 1 }, { OP(rmb),  1, 1 }, { OP(rmb),     1, 1 }, { OP(rmb),     1, 1 }, { OP(2c),   2, 2 }, { OP(illegal), 1, 1 }, { OP(inm),  1, 1 }, { OP(dem), 1, 1 }, // 0x28
	{ OP(bna), 2, 2 }, { OP(bna),  2, 2 }, { OP(bna),     2, 2 }, { OP(bna),     2, 2 }, { OP(bnm),  2, 2 }, { OP(bnm),     2, 2 }, { OP(bnm),  2, 2 }, { OP(bnm), 2, 2 }, // 0x30
	{ OP(bnp), 2, 2 }, { OP(bnp),  2, 2 }, { OP(bnp),     2, 2 }, { OP(bnp),     2, 2 }, { OP(bntm), 2, 2 }, { OP(bni),     2, 2 }, { OP(bnz),  2, 2 }, { OP(bnc), 2, 2 }, // 0x38
	{ OP(lhi), 1, 1 }, { OP(lhi),  1, 1 }, { OP(lhi),     1, 1 }, { OP(lhi),     1, 1 }, { OP(lhi),  1, 1 }, { OP(lhi),     1, 1 }, { OP(lhi),  1, 1 }, { OP(lhi), 1, 1 }, // 0x40
	{ OP(lhi), 1, 1 }, { OP(lhi),  1, 1 }, { OP(lhi),     1, 1 }, { OP(lhi),     1, 1 }, { OP(lhi),  1, 1 }, { OP(lhi),     1, 1 }, { OP(lhi),  1, 1 }, { OP(lhi), 1, 1 }, // 0x48
	{ OP(sfb), 1, 1 }, { OP(sfb),  1, 1 }, { OP(sfb),     1, 1 }, { OP(sfb),     1, 1 }, { OP(sfb),  1, 1 }, { OP(sfb),     1, 1 }, { OP(sfb),  1, 1 }, { OP(sfb), 1, 1 }, // 0x50
	{ OP(sfb), 1, 1 }, { OP(sfb),  1, 1 }, { OP(sfb),     1, 1 }, { OP(sfb),     1, 1 }, { OP(sfb),  1, 1 }, { OP(sfb),     1, 1 }, { OP(sfb),  1, 1 }, { OP(sfb), 1, 1 }, // 0x58
	{ OP(ad),  1, 1 }, { OP(op),   1, 1 }, { OP(rt),      1, 1 }, { OP(rtbl),    1, 2 }, { OP(sb),   1, 1 }, { OP(sb),      1, 1 }, { OP(sb),   1, 1 }, { OP(sb),  1, 1 }, // 0x60
	{ OP(jmp), 2, 2 }, { OP(jmp),  2, 2 }, { OP(jmp),     2, 2 }, { OP(jmp),     2, 2 }, { OP(jmp),  2, 2 }, { OP(jmp),     2, 2 }, { OP(jmp),  2, 2 }, { OP(jmp), 2, 2 }, // 0x68
	{ OP(ba),  2, 2 }, { OP(ba),   2, 2 }, { OP(ba),      2, 2 }, { OP(ba),      2, 2 }, { OP(bm),   2, 2 }, { OP(bm),      2, 2 }, { OP(bm),   2, 2 }, { OP(bm),  2, 2 }, // 0x70
	{ OP(bp),  2, 2 }, { OP(bp),   2, 2 }, { OP(bp),      2, 2 }, { OP(bp),      2, 2 }, { OP(btm),  2, 2 }, { OP(bi),      2, 2 }, { OP(bz),   2, 2 }, { OP(bc),  2, 2 }, // 0x78
	{ OP(ldz), 1, 1 }, { OP(ldz),  1, 1 }, { OP(ldz),     1, 1 }, { OP(ldz),     1, 1 }, { OP(ldz),  1, 1 }, { OP(ldz),     1, 1 }, { OP(ldz),  1, 1 }, { OP(ldz), 1, 1 }, // 0x80
	{ OP(ldz), 1, 1 }, { OP(ldz),  1, 1 }, { OP(ldz),     1, 1 }, { OP(ldz),     1, 1 }, { OP(ldz),  1, 1 }, { OP(ldz),     1, 1 }, { OP(ldz),  1, 1 }, { OP(ldz), 1, 1 }, // 0x88
	{ OP(bnf), 2, 2 }, { OP(bnf),  2, 2 }, { OP(bnf),     2, 2 }, { OP(bnf),     2, 2 }, { OP(bnf),  2, 2 }, { OP(bnf),     2, 2 }, { OP(bnf),  2, 2 }, { OP(bnf), 2, 2 }, // 0x90
	{ OP(bnf), 2, 2 }, { OP(bnf),  2, 2 }, { OP(bnf),     2, 2 }, { OP(bnf),     2, 2 }, { OP(bnf),  2, 2 }, { OP(bnf),     2, 2 }, { OP(bnf),  2, 2 }, { OP(bnf), 2, 2 }, // 0x98
	{ OP(x),   1, 2 }, { OP(xm),   1, 2 }, { OP(xm),      1, 2 }, { OP(xm),      1, 2 }, { OP(xm),   1, 2 }, { OP(xm),      1, 2 }, { OP(xm),   1, 2 }, { OP(xm),  1, 2 }, // 0xa0
	{ OP(cal), 2, 2 }, { OP(cal),  2, 2 }, { OP(cal),     2, 2 }, { OP(cal),     2, 2 }, { OP(cal),  2, 2 }, { OP(cal),     2, 2 }, { OP(cal),  2, 2 }, { OP(cal), 2, 2 }, // 0xa8
	{ OP(czp), 1, 1 }, { OP(czp),  1, 1 }, { OP(czp),     1, 1 }, { OP(czp),     1, 1 }, { OP(czp),  1, 1 }, { OP(czp),     1, 1 }, { OP(czp),  1, 1 }, { OP(czp), 1, 1 }, // 0xb0
	{ OP(czp), 1, 1 }, { OP(czp),  1, 1 }, { OP(czp),     1, 1 }, { OP(czp),     1, 1 }, { OP(czp),  1, 1 }, { OP(czp),     1, 1 }, { OP(czp),  1, 1 }, { OP(czp), 1, 1 }, // 0xb8
	{ OP(cla), 1, 1 }, { OP(li),   1, 1 }, { OP(li),      1, 1 }, { OP(li),      1, 1 }, { OP(li),   1, 1 }, { OP(li),      1, 1 }, { OP(li),   1, 1 }, { OP(li),  1, 1 }, // 0xc0
	{ OP(li),  1, 1 }, { OP(li),   1, 1 }, { OP(li),      1, 1 }, { OP(li),      1, 1 }, { OP(li),   1, 1 }, { OP(li),      1, 1 }, { OP(li),   1, 1 }, { OP(li),  1, 1 }, // 0xc8
	{ OP(bf),  2, 2 }, { OP(bf),   2, 2 }, { OP(bf),      2, 2 }, { OP(bf),      2, 2 }, { OP(bf),   2, 2 }, { OP(bf),      2, 2 }, { OP(bf),   2, 2 }, { OP(bf),  2, 2 }, // 0xd0
	{ OP(bf),  2, 2 }, { OP(bf),   2, 2 }, { OP(bf),      2, 2 }, { OP(bf),      2, 2 }, { OP(bf),   2, 2 }, { OP(bf),      2, 2 }, { OP(bf),   2, 2 }, { OP(bf),  2, 2 }, // 0xd8
	{ OP(xa),  1, 1 }, { OP(clc),  1, 1 }, { OP(illegal), 1, 1 }, { OP(illegal), 1, 1 }, { OP(xa),   1, 1 }, { OP(or),      1, 1 }, { OP(daa),  1, 1 }, { OP(and), 1, 1 }, // 0xe0
	{ OP(xa),  1, 1 }, { OP(tla),  1, 1 }, { OP(das),     1, 1 }, { OP(cma),     1, 1 }, { OP(xa),   1, 1 }, { OP(illegal), 1, 1 }, { OP(ind),  1, 1 }, { OP(ded), 1, 1 }, // 0xe8
	{ OP(xl),  1, 1 }, { OP(stc),  1, 1 }, { OP(illegal), 1, 1 }, { OP(illegal), 1, 1 }, { OP(xl),   1, 1 }, { OP(exl),     1, 1 }, { OP(halt), 1, 1 }, { OP(tal), 1, 1 }, // 0xf0
	{ OP(xh),  1, 1 }, { OP(wttm), 1, 1 }, { OP(jpea),    1, 1 }, { OP(cm),      1, 1 }, { OP(xh),   1, 1 }, { OP(bank),    1, 1 }, { OP(xi),   1, 2 }, { OP(xd),  1, 2 }, // 0xf8
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(LC6554, lc6554_cpu_device, "lc6554", "Sanyo LC6554")

lc6554_cpu_device::lc6554_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, LC6554, tag, owner, clock),
	m_program_config("program", ENDIANNESS_BIG, 8, 12, 0, address_map_constructor(FUNC(lc6554_cpu_device::lc6554_program_map), this)),
	m_data_config("data", ENDIANNESS_BIG, 8, 7, 0, address_map_constructor(FUNC(lc6554_cpu_device::lc6554_data_map), this)),
	m_port_in_cb(*this, 0xff),
	m_port_out_cb(*this)
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void lc6554_cpu_device::lc6554_program_map(address_map &map)
{
	map(0x000, 0xfff).rom();
}

void lc6554_cpu_device::lc6554_data_map(address_map &map)
{
	map(0x00, 0x7f).ram();
}

void lc6554_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);

	// initialize registers
	m_bank = false;
	m_pc = 0x000;
	m_ppc = 0x000;
	m_status = 0;
	m_status_save = 0;
	m_ac = 0;
	m_e = 0;
	m_dp = 0;
	m_ctl = 0;
	m_tm = 0;
	m_sp = 0;
	m_gp_access = false;

	std::fill(std::begin(m_opcode), std::end(m_opcode), 0);
	std::fill(std::begin(m_stack), std::end(m_stack), 0);
	std::fill(std::begin(m_gp), std::end(m_gp), 0);

	// allocate timer
	m_timer = timer_alloc(FUNC(lc6554_cpu_device::timer_update), this);

	// register for savestates
	save_item(NAME(m_opcode));
	save_item(NAME(m_bank));
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_status));
	save_item(NAME(m_status_save));
	save_item(NAME(m_ac));
	save_item(NAME(m_e));
	save_item(NAME(m_dp));
	save_item(NAME(m_ctl));
	save_item(NAME(m_tm));
	save_item(NAME(m_stack));
	save_item(NAME(m_sp));
	save_item(NAME(m_gp));
	save_item(NAME(m_gp_access));

	state_add(STATE_GENPC,     "GENPC",    m_pc).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_pc).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS", m_status).callimport().formatstr("%4s").noshow();
	state_add(LC6554_PC,       "PC",       m_pc).callimport().formatstr("%03X");
	state_add(LC6554_AC,       "AC",       m_ac).callimport().formatstr("%01X");
	state_add(LC6554_E,        "E",        m_e).callimport().formatstr("%01X");
	state_add(LC6554_DP,       "DP",       m_dp).callimport();
	state_add(LC6554_CTL,      "CTL",      m_ctl).callimport().formatstr("%01X");
	state_add(LC6554_TM,       "TM",       m_tm).callimport().formatstr("%02X");

	set_icountptr(m_icount);
}

void lc6554_cpu_device::device_reset()
{
	m_pc = 0x000;
	m_bank = false;
	m_ctl = 0;
	m_status &= ~(FLAG_TMF | FLAG_EXTF);
	m_sp = 0; // not reset according to datasheet?
}

uint64_t lc6554_cpu_device::execute_clocks_to_cycles(uint64_t clocks) const noexcept
{
	return (clocks + 4 - 1) / 4;
}

uint64_t lc6554_cpu_device::execute_cycles_to_clocks(uint64_t cycles) const noexcept
{
	return (cycles * 4);
}

uint32_t lc6554_cpu_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t lc6554_cpu_device::execute_max_cycles() const noexcept
{
	return 2;
}

void lc6554_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		m_ppc = m_pc;

		// timer interrupt?
		if ((m_ctl & CTL_TIMER_INT_ENABLE) && (m_status & FLAG_TMF))
		{
			m_status &= ~FLAG_TMF;
			m_status_save = m_status & (FLAG_C | FLAG_Z);

			push_pc(VECTOR_TIMER);
		}

		debugger_instruction_hook(m_pc);

		// fetch next opcode byte(s)
		m_opcode[0] = m_program_cache.read_byte(m_pc++);
		const opcode_info &opcode = m_opcode_table[m_opcode[0]];

		if (opcode.bytes > 1)
			m_opcode[1] = m_program_cache.read_byte(m_pc++);

		// special handling for the BANK instruction
		if (m_bank)
		{
			// bank inverts PC11 but only if immediately followed by JMP
			if (opcode.func == OP(jmp))
				m_pc ^= 0x800;

			// BANK redirects IP and OP to the pseudo-port
			if (opcode.func == OP(ip) || opcode.func == OP(op))
				m_gp_access = true;

			m_bank = false;
		}

		(this->*opcode.func)();

		m_icount -= opcode.cycles;
	}
}

void lc6554_cpu_device::execute_set_input(int inputnum, int state)
{
	// TODO
}

device_memory_interface::space_config_vector lc6554_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
	};
}

std::unique_ptr<util::disasm_interface> lc6554_cpu_device::create_disassembler()
{
	return std::make_unique<lc6500_disassembler>();
}

void lc6554_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c",
				m_status & FLAG_C    ? 'C' : '.',
				m_status & FLAG_Z    ? 'Z' : '.',
				m_status & FLAG_TMF  ? 'T' : '.',
				m_status & FLAG_EXTF ? 'E' : '.');
			break;
	}
}

TIMER_CALLBACK_MEMBER(lc6554_cpu_device::timer_update)
{
	m_tm++;

	if (m_tm == 0x00)
	{
		m_status |= FLAG_TMF;
		m_timer->adjust(attotime::never);
	}
}


//**************************************************************************
//  HELPER FUNCTIONS
//**************************************************************************

void lc6554_cpu_device::ram_w(offs_t offset, uint8_t data)
{
	uint8_t idx = (offset >> 1) & 0x7f;

	if (offset & 1)
		m_data.write_byte(idx, (m_data.read_byte(idx) & 0xf0) | (data << 0));
	else
		m_data.write_byte(idx, (data << 4) | (m_data.read_byte(idx) & 0x0f));
}

uint8_t lc6554_cpu_device::ram_r(offs_t offset)
{
	uint8_t idx = (offset >> 1) & 0x7f;

	if (offset & 1)
		return m_data.read_byte(idx) >> 0 & 0x0f;
	else
		return m_data.read_byte(idx) >> 4 & 0x0f;
}

void lc6554_cpu_device::push_pc(uint16_t pc)
{
	if (m_sp == 7)
		logerror("Stack overflow at PC=%03X\n", m_ppc);

	m_stack[m_sp] = m_pc;
	m_sp = (m_sp + 1) & 0x07;
	m_pc = pc;
}

void lc6554_cpu_device::pop_pc()
{
	if (m_sp == 0)
		logerror("Stack underflow PC=%03X\n", m_ppc);

	m_sp = (m_sp - 1) & 0x07;
	m_pc = m_stack[m_sp];
}

void lc6554_cpu_device::set_cf(uint8_t data)
{
	if (data & 0x10)
		m_status |= FLAG_C;
	else
		m_status &= ~FLAG_C;
}

void lc6554_cpu_device::set_zf(uint8_t data)
{
	if ((data & 0x0f) == 0)
		m_status |= FLAG_Z;
	else
		m_status &= ~FLAG_Z;
}

void lc6554_cpu_device::set_cf_and_zf(uint8_t data)
{
	set_cf(data);
	set_zf(data);
}


//**************************************************************************
//  INSTRUCTIONS
//**************************************************************************

void lc6554_cpu_device::op_cla()
{
	m_ac = 0;
	set_zf(m_ac);
}

void lc6554_cpu_device::op_clc()
{
	m_status &= ~FLAG_C;
}

void lc6554_cpu_device::op_stc()
{
	m_status |= FLAG_C;
}

void lc6554_cpu_device::op_cma()
{
	fatalerror("CMA instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_inc()
{
	m_ac = m_ac + 1;
	set_cf_and_zf(m_ac);
	m_ac &= 0x0f;
}

void lc6554_cpu_device::op_dec()
{
	m_ac = m_ac + 0x0f;
	set_cf_and_zf(m_ac);
	m_ac &= 0x0f;
}

void lc6554_cpu_device::op_ral()
{
	m_ac = (m_ac << 1) | (m_status & FLAG_C ? 1 : 0);
	set_cf(m_ac);
	m_ac &= 0x0f;
}

void lc6554_cpu_device::op_tae()
{
	m_e = m_ac;
}

void lc6554_cpu_device::op_xae()
{
	uint8_t tmp = m_ac;
	m_ac = m_e;
	m_e = tmp;
}

void lc6554_cpu_device::op_inm()
{
	uint8_t data = ram_r(m_dp) + 1;
	set_cf_and_zf(data);
	ram_w(m_dp, data & 0x0f);
}

void lc6554_cpu_device::op_dem()
{
	uint8_t data = ram_r(m_dp) + 0x0f;
	set_cf_and_zf(data);
	ram_w(m_dp, data & 0x0f);
}

void lc6554_cpu_device::op_smb()
{
	uint8_t data = ram_r(m_dp);
	data |= (1 << (m_opcode[0] & 0x03));
	ram_w(m_dp, data);
}

void lc6554_cpu_device::op_rmb()
{
	uint8_t data = ram_r(m_dp);
	data &= ~(1 << (m_opcode[0] & 0x03));
	set_zf(data);
	ram_w(m_dp, data);
}

void lc6554_cpu_device::op_ad()
{
	fatalerror("AD instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_adc()
{
	fatalerror("ADC instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_daa()
{
	fatalerror("DAA instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_das()
{
	fatalerror("DAS instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_exl()
{
	m_ac = m_ac ^ ram_r(m_dp);
	set_zf(m_ac);
}

void lc6554_cpu_device::op_and()
{
	m_ac = m_ac & ram_r(m_dp);
	set_zf(m_ac);
}

void lc6554_cpu_device::op_or()
{
	m_ac = m_ac | ram_r(m_dp);
	set_zf(m_ac);
}

void lc6554_cpu_device::op_cm()
{
	uint8_t mem = ram_r(m_dp);
	uint8_t tmp = m_ac + ((~mem) & 0x0f) + 1;
	set_cf_and_zf(tmp);
}

void lc6554_cpu_device::op_ci(uint8_t arg)
{
	uint8_t tmp = m_ac + ((~arg) & 0x0f) + 1;
	set_cf_and_zf(tmp);
}

void lc6554_cpu_device::op_cli(uint8_t arg)
{
	fatalerror("CLI instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_li()
{
	m_ac = m_opcode[0] & 0x0f;
	set_zf(m_ac);
}

void lc6554_cpu_device::op_s()
{
	ram_w(m_dp, m_ac);
}

void lc6554_cpu_device::op_l()
{
	m_ac = ram_r(m_dp);
	set_zf(m_ac);
}

void lc6554_cpu_device::op_xm()
{
	fatalerror("XM instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_x()
{
	fatalerror("X instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_xi()
{
	uint8_t tmp = m_ac;
	m_ac = ram_r(m_dp);
	ram_w(m_dp, tmp);

	op_ind(); // increment DPL
}

void lc6554_cpu_device::op_xd()
{
	fatalerror("XD instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_rtbl()
{
	uint16_t addr = (m_pc & 0xf00) | (m_e << 4) | m_ac;
	uint8_t data = m_program_cache.read_byte(addr);
	m_ac = (data & 0xf0) >> 4;
	m_e = data & 0x0f;
}

void lc6554_cpu_device::op_ldz()
{
	m_dp = m_opcode[0] & 0x0f;
}

void lc6554_cpu_device::op_lhi()
{
	m_dp = (m_opcode[0] & 0x0f) << 4 | (m_dp & 0x0f);
}

void lc6554_cpu_device::op_ind()
{
	uint8_t dpl = ((m_dp & 0x0f) + 1) & 0x0f;
	m_dp = (m_dp & 0xf0) | dpl;
	set_zf(dpl);
}

void lc6554_cpu_device::op_ded()
{
	uint8_t dpl = ((m_dp & 0x0f) + 0x0f) & 0x0f;
	m_dp = (m_dp & 0xf0) | dpl;
	set_zf(dpl);
}

void lc6554_cpu_device::op_tal()
{
	m_dp = (m_dp & 0xf0) | m_ac;
}

void lc6554_cpu_device::op_tla()
{
	m_ac = (m_dp & 0x0f);
	set_zf(m_ac);
}

void lc6554_cpu_device::op_xah()
{
	uint8_t tmp = m_ac;
	m_ac = m_dp >> 4;
	m_dp = (tmp << 4) | (m_dp & 0x0f);
}

void lc6554_cpu_device::op_xa()
{
	uint8_t reg = (m_opcode[0] >> 2) & 0x03;
	uint8_t tmp = ram_r(RAM_OFFSET_A | reg);
	ram_w(RAM_OFFSET_A | reg, m_ac);
	m_ac = tmp;
}

void lc6554_cpu_device::op_xh()
{
	uint8_t reg = BIT(m_opcode[0], 2);
	uint8_t tmp = ram_r(RAM_OFFSET_H | reg);
	ram_w(RAM_OFFSET_H | reg, (m_dp >> 4) & 0x0f);
	m_dp = (tmp << 4) | (m_dp & 0x0f);
}

void lc6554_cpu_device::op_xl()
{
	uint8_t reg = BIT(m_opcode[0], 2);
	uint8_t tmp = ram_r(RAM_OFFSET_L | reg);
	ram_w(RAM_OFFSET_L | reg, m_dp & 0x0f);
	m_dp = (m_dp & 0xf0) | (tmp << 0);
}

void lc6554_cpu_device::op_sfb()
{
	fatalerror("SFB instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_rfb()
{
	fatalerror("RFB instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_jmp()
{
	m_pc = (m_pc & 0x800) | ((m_opcode[0] & 0x07) << 8) | m_opcode[1];
}

void lc6554_cpu_device::op_jpea()
{
	fatalerror("JPEA instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_czp()
{
	fatalerror("CZP instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_cal()
{
	push_pc(((m_opcode[0] & 0x07) << 8) | m_opcode[1]);
}

void lc6554_cpu_device::op_rt()
{
	pop_pc();
}

void lc6554_cpu_device::op_rti()
{
	m_status = (m_status & ~(FLAG_C | FLAG_Z)) | m_status_save;
	pop_pc();
}

void lc6554_cpu_device::op_bank()
{
	m_bank = true;
}

void lc6554_cpu_device::op_sb()
{
	fatalerror("SB instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_ba()
{
	fatalerror("BA instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bna()
{
	fatalerror("BNA instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bm()
{
	fatalerror("BM instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bnm()
{
	fatalerror("BNM instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bp()
{
	fatalerror("BP instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bnp()
{
	fatalerror("BNP instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_btm()
{
	fatalerror("BTM instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bntm()
{
	fatalerror("BNTM instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bi()
{
	fatalerror("BI instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bni()
{
	fatalerror("BNI instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bc()
{
	if ((m_status & FLAG_C) == FLAG_C)
		m_pc = (m_pc & 0xf00) | m_opcode[1];
}

void lc6554_cpu_device::op_bnc()
{
	if ((m_status & FLAG_C) == 0)
		m_pc = (m_pc & 0xf00) | m_opcode[1];
}

void lc6554_cpu_device::op_bz()
{
	if ((m_status & FLAG_Z) == FLAG_Z)
		m_pc = (m_pc & 0xf00) | m_opcode[1];
}

void lc6554_cpu_device::op_bnz()
{
	if ((m_status & FLAG_Z) == 0)
		m_pc = (m_pc & 0xf00) | m_opcode[1];
}

void lc6554_cpu_device::op_bf()
{
	fatalerror("BF instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_bnf()
{
	fatalerror("BNF instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_ip()
{
	if (m_gp_access)
	{
		LOGMASKED(LOG_GP, "R GP port %x\n", m_dp & 0x0f);
		m_ac = 0;
	}
	else
		m_ac = m_port_in_cb[m_dp & 0x0f](0);

	set_zf(m_ac);
	m_gp_access = false;
}

void lc6554_cpu_device::op_op()
{
	if (m_gp_access)
	{
		LOGMASKED(LOG_GP, "W GP port %x = %01x\n", m_ac, m_dp & 0x0f);
		m_gp[m_dp & 0x0f] = m_ac;
	}
	else
		m_port_out_cb[m_dp & 0x0f](0, m_ac);

	m_gp_access = false;
}

void lc6554_cpu_device::op_spb()
{
	fatalerror("SPB instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_rpb()
{
	fatalerror("RPB instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_sctl(uint8_t arg)
{
	m_ctl |= (arg & 0x0f);
}

void lc6554_cpu_device::op_rctl(uint8_t arg)
{
	m_ctl &= ~(arg & 0x0f);
	set_zf(m_ctl);
}

void lc6554_cpu_device::op_wttm()
{
	m_tm = (m_e << 4) | m_ac;
	m_status &= ~FLAG_TMF;

	// 4-bit prescaler, 4 clocks each cycle
	m_timer->adjust(attotime::from_ticks(16 * 4, clock()), 0, attotime::from_ticks(16 * 4, clock()));
}

void lc6554_cpu_device::op_halt()
{
	fatalerror("HALT instruction not implemented at PC=%03X\n", m_ppc);
}

void lc6554_cpu_device::op_nop()
{
}

void lc6554_cpu_device::op_2c()
{
	switch (m_opcode[1] & 0xf0)
	{
		case 0x40: op_ci(m_opcode[1] & 0x0f); break;
		case 0x50: op_cli(m_opcode[1] & 0x0f); break;
		case 0x80: op_sctl(m_opcode[1] & 0x0f); break;
		case 0x90: op_rctl(m_opcode[1] & 0x0f); break;
		default:   op_illegal(); break;
	}
}

void lc6554_cpu_device::op_illegal()
{
	logerror("Illegal opcode %02X at PC=%03X\n", m_opcode[0], m_ppc);
}
