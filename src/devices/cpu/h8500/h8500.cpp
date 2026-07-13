// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR, Olivier Galibert
/***************************************************************************

	Hitachi H8/500 family base CPU device.
	Disassembler by AJR, execution core by R. Belmont

	The H8/500 is a 16-bit microcontroller architecture that's kind of a
	Thumb version of the H8/300, to draw an ARM analogy.  Instead of fixed
	16-bit instruction words, instructions vary from 1 to 6 bytes, and a set
	of smaller, faster instructions that go to a movable special area in
	the first 64K help to optimize code size.

***************************************************************************/

#include "emu.h"
#include "h8500.h"
#include "h8500dasm.h"

h8500_device::h8500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, int buswidth, int ramsize, int defmode, address_map_constructor map)
	: h8_cpu_base(mconfig, type, tag, owner, clock)
	, m_pc(0), m_ppc(0), m_sr(0)
	, m_cp(0), m_dp(0), m_ep(0), m_tp(0), m_br(0)
	, m_r{0, 0, 0, 0, 0, 0, 0, 0}
	, m_npc(0), m_pir(0), m_ir{0}, m_ea_op_bytes(0)
	, m_ea_addr_cache(0), m_ea_addr_cached(false), m_ea_committed(false)
	, m_tmp1(0), m_tmp2(0)
	, m_inst_state(STATE_RESET), m_inst_substate(0), m_requested_state(-1)
	, m_irq_vector(0), m_taken_irq_vector(0), m_irq_level(0), m_taken_irq_level(0), m_irq_nmi(false)
	, m_standby_pending(false), m_standby_time(0)
	, m_icount(0), m_bcount(0), m_count_before_instruction_step(0)
	, m_read_adc(*this, 0)
	, m_read_port(*this, 0xff)
	, m_write_port(*this)
	, m_sci(*this, "sci%u", 0)
	, m_sci_tx(*this)
	, m_sci_clk(*this)
	, m_standby_cb(*this)
	, m_program_config("program", ENDIANNESS_BIG, buswidth, addrbits, 0, map)
	, m_program(nullptr)
	, m_mode_control(defmode)
	, m_ram_size((1 << ramsize) - 1)
{
	for (unsigned int i = 0; i != m_read_adc.size(); i++) {
		m_read_adc[i].bind().set([this, i]()
								 { return adc_default(i); });
	}
	for(int i=0; i != PORT_COUNT; i++) {
		m_read_port[i].bind().set([this, i]() { return port_default_r(i); });
		m_write_port[i].bind().set([this, i](u8 data) { port_default_w(i, data); });
	}
}

void h8500_device::device_config_complete()
{
	if (!h8_maximum_mode())
		m_program_config.m_addr_width = 16;
}

std::unique_ptr<util::disasm_interface> h8500_device::create_disassembler()
{
	return std::make_unique<h8500_disassembler>(h8_maximum_mode());
}

device_memory_interface::space_config_vector h8500_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void h8500_device::debug_set_pc(offs_t pc) noexcept
{
	m_pc = m_ppc = pc & 0xffff;
	if (h8_maximum_mode()) {
		m_cp = u8(pc >> 16);
	}
}

void h8500_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	set_icountptr(m_icount);

	// Control registers
	state_add(H8500_PC, "PC", m_ppc);
	if (h8_maximum_mode())
	{
		state_add<u32>(STATE_GENPC, "GENPC", [this]() { return u32(m_cp) << 16 | m_ppc; }, [this](u32 data) { debug_set_pc(data); }).noshow().mask(0xffffff);
		state_add<u32>(STATE_GENPCBASE, "CURPC", [this]() { return u32(m_cp) << 16 | m_ppc; }, [this](u32 data) { debug_set_pc(data); }).noshow().mask(0xffffff);
	}
	else
	{
		state_add<u16>(STATE_GENPC, "GENPC", [this]() { return m_ppc; }, [this](u16 data) { debug_set_pc(data); }).noshow();
		state_add<u16>(STATE_GENPCBASE, "CURPC", [this]() { return m_ppc; }, [this](u16 data) { debug_set_pc(data); }).noshow();
	}
	state_add(H8500_SR, "SR", m_sr).mask(0x870f);
	state_add<u8>(H8500_CCR, "CCR", [this]() { return m_sr & 0xff; }, [this](u8 data) { m_sr = (m_sr & 0xff00) | data; }).mask(0x0f).noshow();
	state_add<u8>(STATE_GENFLAGS, "FLAGS", [this]() { return m_sr & 0xff; }, [this](u8 data) { m_sr = (m_sr & 0xff00) | data; }).mask(0x0f).formatstr("%4s").noshow();
	if (h8_maximum_mode())
	{
		state_add(H8500_CP, "CP", m_cp);
		state_add(H8500_DP, "DP", m_dp);
		state_add(H8500_EP, "EP", m_ep);
		state_add(H8500_TP, "TP", m_tp);
	}
	state_add(H8500_BR, "BR", m_br);

	// General registers
	for (int n = 0; n < 6; n++) {
		state_add(H8500_R0 + n, string_format("R%d", n).c_str(), m_r[n]);
	}
	state_add(H8500_FP, "FP", m_r[6]);
	state_add(H8500_SP, "SP", m_r[7]);

	// Save state
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_sr));
	if (h8_maximum_mode())
		save_item(NAME(m_cp));
	save_item(NAME(m_dp));
	save_item(NAME(m_ep));
	save_item(NAME(m_tp));
	save_item(NAME(m_br));
	save_item(NAME(m_r));
	save_item(NAME(m_npc));
	save_item(NAME(m_pir));
	save_item(NAME(m_ir));
	save_item(NAME(m_ea_op_bytes));
	save_item(NAME(m_ea_addr_cache));
	save_item(NAME(m_ea_addr_cached));
	save_item(NAME(m_ea_committed));
	save_item(NAME(m_tmp1));
	save_item(NAME(m_tmp2));
	save_item(NAME(m_tmp3));
	save_item(NAME(m_tmp4));
	save_item(NAME(m_inst_state));
	save_item(NAME(m_inst_substate));
	save_item(NAME(m_requested_state));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_taken_irq_vector));
	save_item(NAME(m_irq_level));
	save_item(NAME(m_taken_irq_level));
	save_item(NAME(m_irq_nmi));
	save_item(NAME(m_standby_pending));
	save_item(NAME(m_standby_time));
	save_item(NAME(m_bcount));
	save_item(NAME(m_count_before_instruction_step));
}

void h8500_device::device_reset()
{
	m_sr = 0x0700 | (m_sr & 0x000f);
	m_inst_state = STATE_RESET;
	m_inst_substate = 0;
	m_requested_state = -1;
	m_count_before_instruction_step = 0;

	m_irq_vector = 0;
	m_irq_level = -1;
	m_irq_nmi = false;
	m_taken_irq_vector = 0;
	m_taken_irq_level = -1;

	m_standby_pending = false;
	m_standby_cb(0);
}

u8 h8500_device::read8i(u32 adr)
{
	internal(1);
	return m_program->read_byte(adr);
}

u16 h8500_device::read16i(u32 adr)
{
	internal(1);
	if (adr & 1) {
		return m_program->read_byte(adr) << 8 | m_program->read_byte(adr + 1);
	}

	return m_program->read_word(adr);
}

u8 h8500_device::read_imm8()
{
	u8 val = read8i(pc24());
	if (!access_to_be_redone_noclear()) {
		m_pc = (m_pc + 1) & 0xffff;
	}
	internal(1);
	return val;
}

u16 h8500_device::read_imm16()
{
	u16 val = read16i(pc24());
	if (!access_to_be_redone_noclear()) {
		m_pc = (m_pc + 2) & 0xffff;
	}
	internal(1);
	return val;
}

u8 h8500_device::read8(u32 adr)
{
	internal(1);
	return m_program->read_byte(adr);
}

void h8500_device::write8(u32 adr, u8 data)
{
	internal(1);
	m_program->write_byte(adr, data);
}

u16 h8500_device::read16(u32 adr)
{
	internal(1);
	if (adr & 1) {
		return m_program->read_byte(adr) << 8 | m_program->read_byte(adr + 1);
	}

	return m_program->read_word(adr);
}

void h8500_device::write16(u32 adr, u16 data)
{
	internal(1);
	if (adr & 1) {
		m_program->write_byte(adr, data >> 8);
		m_program->write_byte(adr + 1, data & 0xff);
		return;
	}

	m_program->write_word(adr, data);
}

u32 h8500_device::ea_addr()
{
	// We cache the calculated effective address to save some effort on RMW instructions
	if (m_ea_addr_cached) {
		return m_ea_addr_cache;
	}

	const u8 ea = m_ir[0];
	u32 page = 0;
	if (h8_maximum_mode()) {
		switch (ea & 0x7) {
			case 0: case 1: case 2: case 3:
				page = m_dp << 16;
				break;

			case 4: case 5:
				page = m_ep << 16;
				break;

			case 6: case 7:
				page = m_tp << 16;
				break;
		}
	}

	// If there is a side effect (increment/decrement), that's done later by ea_commit() in
	// case the access needs to be restarted
	u32 addr;
	switch (ea & 0xf0) {
	case 0xb0: // @-Rn: address is Rn - step (pre-decrement, deferred commit)
		addr = ((r16_r(ea & 0x07) - (BIT(ea, OP_WIDTH) ? 2 : 1)) & 0xffff) | page;
		break;
	case 0xc0: // @Rn+: address is current Rn (post-increment, deferred commit)
		addr = r16_r(ea & 0x07) | page;
		break;
	case 0xd0: // @Rn
		addr = r16_r(ea & 0x07) | page;
		break;
	case 0xe0: // @(d:8, Rn)
		addr = ((r16_r(ea & 0x07) + s8(m_ir[1])) & 0xffff) | page;
		break;
	case 0xf0: // @(d:16, Rn)
		addr = ((r16_r(ea & 0x07) + s16((u16(m_ir[1]) << 8) | m_ir[2])) & 0xffff) | page;
		break;
	default:
		if (ea == 0x05 || ea == 0x0d) {
			addr = (m_br << 8) | m_ir[1];   // @aa:8 page-zero short
		} else if (ea == 0x15 || ea == 0x1d) {
			addr = (u16(m_ir[1]) << 8) | m_ir[2] | (m_dp << 16);   // @aa:16
		} else {
			logerror("h8500: ea_addr called on non-memory EA 0x%02x\n", ea);
			addr = 0;
		}
		break;
	}

	m_ea_addr_cache = addr;
	m_ea_addr_cached = true;
	return addr;
}

void h8500_device::ea_commit()
{
	// Prevent RMW instructions from incrementing/decrementing the register twice
	if (m_ea_committed)
		return;
	m_ea_committed = true;

	const u8 ea = m_ir[0];
	const int n = ea & 0x07;
	const int step = BIT(ea, OP_WIDTH) ? 2 : 1;
	switch (ea & 0xf0) {
	case 0xb0: // @-Rn: commit the predecrement
		r16_w(n, (r16_r(n) - step) & 0xffff);
		break;
	case 0xc0: // @Rn+: commit the postincrement
		r16_w(n, (r16_r(n) + step) & 0xffff);
		break;
	}
}

// @-R7 and @R7+ accesses always push/pop a full word so the stack stays
// word-aligned.  Detect those two EA byte values and forward to the
// word-size helpers - flipping bit 3 of m_ir[0] in the process so that
// ea_addr/ea_commit step by 2 and the access actually goes through
// read16/write16.  m_ir[0] is restored after the call so the opcode body
// still sees the EA byte it dispatched on.
static constexpr bool is_byte_sp_pushpop(u8 ea) { return ea == 0xb7 || ea == 0xc7; }

u8 h8500_device::read_ea_8()
{
	const u8 ea = m_ir[0];
	if (ea == 0x04)                               // #imm8 immediate
		return m_ir[1];
	if ((ea & 0xf8) == 0xa0)                      // Rn register direct, byte
		return r8_r((ea & 0x07) | 8);
	if (is_byte_sp_pushpop(ea)) {
		m_ir[0] = ea | (1 << OP_WIDTH);           // promote to word for the call
		u16 val = read_ea_16();
		m_ir[0] = ea;
		return val & 0xff;                        // returned byte is the low half
	}
	u8 val = read8(ea_addr());
	if (!access_to_be_redone_noclear())
		ea_commit();
	return val;
}

void h8500_device::write_ea_8(u8 val)
{
	const u8 ea = m_ir[0];
	if ((ea & 0xf8) == 0xa0) {
		r8_w((ea & 0x07) | 8, val);
		return;
	}
	if (is_byte_sp_pushpop(ea)) {
		m_ir[0] = ea | (1 << OP_WIDTH);           // promote to word for the call
		write_ea_16(val);                         // zero-extended to 16 bits
		m_ir[0] = ea;
		return;
	}
	write8(ea_addr(), val);
	if (!access_to_be_redone_noclear())
		ea_commit();
}

u16 h8500_device::read_ea_16()
{
	const u8 ea = m_ir[0];
	if (ea == 0x0c)                               // #imm16 immediate
		return (u16(m_ir[1]) << 8) | m_ir[2];
	if ((ea & 0xf8) == 0xa8)                      // Rn register direct, word
		return r16_r(ea & 0x07);
	u16 val = read16(ea_addr());
	if (!access_to_be_redone_noclear())
		ea_commit();
	return val;
}

void h8500_device::write_ea_16(u16 val)
{
	const u8 ea = m_ir[0];
	if ((ea & 0xf8) == 0xa8) {
		r16_w(ea & 0x07, val);
		return;
	}
	write16(ea_addr(), val);
	if (!access_to_be_redone_noclear())
		ea_commit();
}

// ---------------------------------------------------------------------------
//  Prefetch / dispatch helpers
// ---------------------------------------------------------------------------

void h8500_device::prefetch_done()
{
	// Instruction boundary: invalidate the EA address cache and clear
	// the one-shot ea_commit flag so the next instruction starts fresh.
	m_ea_addr_cached = false;
	m_ea_committed = false;

	// IRQ-aware prefetch completion: take a pending interrupt at the
	// instruction boundary if the interrupt controller presented one.
	if (m_requested_state != -1) {
		m_inst_state = m_requested_state;
		m_requested_state = -1;
	} else if (m_irq_vector) {
		take_interrupt();
	} else {
		m_inst_state = m_ir[0] = m_pir;
	}
}

void h8500_device::prefetch_done_noirq()
{
	m_ea_addr_cached = false;
	m_ea_committed = false;
	m_inst_state = m_ir[0] = m_pir;
}

void h8500_device::take_interrupt()
{
	m_inst_state = STATE_IRQ;
	m_taken_irq_vector = m_irq_vector;
	m_taken_irq_level = m_irq_level;
}

void h8500_device::illegal()
{
	// m_ppc was set by execute_run to the in-page address of m_ir[0]
	// (the first byte of this instruction).  For post-EA dispatch that
	// is the EA prefix byte, NOT the op byte that triggered illegal -
	// reporting the instruction start is what's actually useful for
	// finding the unimplemented opcode in a ROM disassembly.
	const u32 start = (u32(m_cp) << 16) | m_ppc;

	// Bytes captured for this instruction:
	//   - top-level standalone dispatch: just m_ir[0]
	//   - post-EA / deeper dispatch: EA prefix + m_ea_op_bytes operand
	//     bytes + op selector byte at m_ir[1 + m_ea_op_bytes]
	const int consumed =
		((m_inst_state >> 16) == 0) ? 1 : (2 + m_ea_op_bytes);
	std::string bytes;
	for (int i = 0; i < consumed && i < int(std::size(m_ir)); i++)
		bytes += util::string_format(" %02X", m_ir[i]);

	// TODO: raise the illegal instruction exception once all instructions
	// are implemented.  Until then, this lets us know when they're hit.
	fatalerror("h8500: unimplemented instruction at %06X (bytes%s)\n",
			   start, bytes.c_str());
}

void h8500_device::execute_set_input(int /*inputnum*/, int /*state*/)
{
}

u16 h8500_device::adc_default(int adc)
{
	if(!machine().side_effects_disabled())
		logerror("read of un-hooked adc %d\n", adc);
	return 0;
}

const char h8500_device::port_names[] = "123456789";
u8 h8500_device::port_default_r(int port)
{
	if (!machine().side_effects_disabled())
		logerror("read of un-hooked port %c (PC=%X)\n", port_names[port], m_ppc);
	return 0xff;
}

void h8500_device::port_default_w(int port, u8 data)
{
	logerror("write of un-hooked port %c %02x (PC=%X)\n", port_names[port], data, m_ppc);
}

u16 h8500_device::nz8(u8 val)
{
	u16 rv = 0;

	if (BIT(val, 7)) {
		rv |= SR_N;
	}
	if (val == 0) {
		rv |= SR_Z;
	}
	return rv;
}

u16 h8500_device::nz16(u16 val)
{
	u16 rv = 0;

	if (BIT(val, 15)) {
		rv |= SR_N;
	}
	if (val == 0) {
		rv |= SR_Z;
	}
	return rv;
}

u16 h8500_device::nz32(u32 val)
{
	u16 rv = 0;

	if (BIT(val, 31)) {
		rv |= SR_N;
	}
	if (val == 0) {
		rv |= SR_Z;
	}
	return rv;
}

u8 h8500_device::do_add8(u8 v1, u8 v2)
{
	const u16 res = v1 + v2;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (!u8(res))
		m_sr |= SR_Z;
	else if (s8(res) < 0)
		m_sr |= SR_N;
	if (~(v1 ^ v2) & (v1 ^ res) & 0x80)
		m_sr |= SR_V;
	if (res & 0x100)
		m_sr |= SR_C;
	return res;
}

u16 h8500_device::do_add16(u16 v1, u16 v2)
{
	const u32 res = v1 + v2;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (!u16(res))
		m_sr |= SR_Z;
	else if (s16(res) < 0)
		m_sr |= SR_N;
	if (~(v1 ^ v2) & (v1 ^ res) & 0x8000)
		m_sr |= SR_V;
	if (res & 0x10000)
		m_sr |= SR_C;
	return res;
}

u8 h8500_device::do_sub8(u8 v1, u8 v2)
{
	const u16 res = v1 - v2;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);

	if (!u8(res))
		m_sr |= SR_Z;
	else if (s8(res) < 0)
		m_sr |= SR_N;
	if ((v1 ^ v2) & (v1 ^ res) & 0x80)
		m_sr |= SR_V;
	if (res & 0x100)
		m_sr |= SR_C;
	return res;
}

u16 h8500_device::do_sub16(u16 v1, u16 v2)
{
	const u32 res = v1 - v2;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);

	if (!u16(res))
		m_sr |= SR_Z;
	else if (s16(res) < 0)
		m_sr |= SR_N;
	if ((v1 ^ v2) & (v1 ^ res) & 0x8000)
		m_sr |= SR_V;
	if (res & 0x10000)
		m_sr |= SR_C;
	return res;
}

u8 h8500_device::do_shll8(u8 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x80)
		m_sr |= SR_C;
	v <<= 1;
	if (!v)
		m_sr |= SR_Z;
	else if (s8(v) < 0)
		m_sr |= SR_N;
	return v;
}

u16 h8500_device::do_shll16(u16 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x8000)
		m_sr |= SR_C;
	v <<= 1;
	if (!v)
		m_sr |= SR_Z;
	else if (s16(v) < 0)
		m_sr |= SR_N;
	return v;
}

u8 h8500_device::do_shlr8(u8 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 1)
		m_sr |= SR_C;
	v >>= 1;
	if (!v)
		m_sr |= SR_Z;
	return v;
}

u16 h8500_device::do_shlr16(u16 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 1)
		m_sr |= SR_C;
	v >>= 1;
	if (!v)
		m_sr |= SR_Z;
	return v;
}

u8 h8500_device::do_rotl8(u8 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x80)
		m_sr |= SR_C;
	v = (v << 1) | (v >> 7);
	if (!v)
		m_sr |= SR_Z;
	else if (s8(v) < 0)
		m_sr |= SR_N;
	return v;
}

u16 h8500_device::do_rotl16(u16 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x8000)
		m_sr |= SR_C;
	v = (v << 1) | (v >> 15);
	if (!v)
		m_sr |= SR_Z;
	else if (s16(v) < 0)
		m_sr |= SR_N;
	return v;
}

u8 h8500_device::do_rotr8(u8 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x01)
		m_sr |= SR_C;
	v = (v << 7) | (v >> 1);
	if (!v)
		m_sr |= SR_Z;
	else if (s8(v) < 0)
		m_sr |= SR_N;
	return v;
}

u16 h8500_device::do_rotr16(u16 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x0001)
		m_sr |= SR_C;
	v = (v << 15) | (v >> 1);
	if (!v)
		m_sr |= SR_Z;
	else if (s16(v) < 0)
		m_sr |= SR_N;
	return v;
}

u8 h8500_device::do_addx8(u8 v1, u8 v2)
{
	const u8 c = m_sr & SR_C ? 1 : 0;
	const u16 res = v1 + v2 + c;
	m_sr &= ~(SR_N | SR_V | SR_C);
	if(u8(res))
		m_sr &= ~SR_Z;
	if(s8(res) < 0)
		m_sr |= SR_N;
	if(~(v1^v2) & (v1^res) & 0x80)
		m_sr |= SR_V;
	if(res & 0x100)
		m_sr |= SR_C;
	return res;
}

u16 h8500_device::do_addx16(u16 v1, u16 v2)
{
	const u8 c = m_sr & SR_C ? 1 : 0;
	const u32 res = v1 + v2 + c;
	m_sr &= ~(SR_N | SR_V | SR_C);
	if (u16(res))
		m_sr &= ~SR_Z;
	if (s16(res) < 0)
		m_sr |= SR_N;
	if (~(v1 ^ v2) & (v1 ^ res) & 0x8000)
		m_sr |= SR_V;
	if (res & 0x10000)
		m_sr |= SR_C;
	return res;
}

u16 h8500_device::do_adds16(u16 v1, u16 v2)
{
	const u32 res = v1 + v2;
	return res;
}

u8 h8500_device::do_subx8(u8 v1, u8 v2)
{
	const u8 c = m_sr & SR_C ? 1 : 0;
	const u16 res = v1 - v2 - c;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if(u8(res))
		m_sr &= ~SR_Z;
	if(s8(res) < 0)
		m_sr |= SR_N;
	if((v1^v2) & (v1^res) & 0x80)
		m_sr |= SR_V;
	if(res & 0x100)
		m_sr |= SR_C;
	return res;
}

u16 h8500_device::do_subx16(u16 v1, u16 v2)
{
	const u8 c = m_sr & SR_C ? 1 : 0;
	const u32 res = v1 - v2 - c;
	m_sr &= ~(SR_N | SR_V | SR_C);

	if (u16(res))
		m_sr &= ~SR_Z;
	if (s16(res) < 0)
		m_sr |= SR_N;
	if ((v1 ^ v2) & (v1 ^ res) & 0x8000)
		m_sr |= SR_V;
	if (res & 0x10000)
		m_sr |= SR_C;
	return res;
}

u8 h8500_device::do_subs8(u8 v1, u8 v2)
{
	const u16 res = v1 - (s8)v2;
	return res;
}

u16 h8500_device::do_subs16(u16 v1, u16 v2)
{
	const u32 res = v1 - v2;
	return res;
}

u8 h8500_device::do_dadd8(u8 v1, u8 v2)
{
	const u8 c = m_sr & SR_C ? 1 : 0;
	u16 res = (v1 & 0x0f) + (v2 & 0x0f) + c;
	if (res >= 0x0a)
		res += 0x06;
	res += (v1 & 0xf0) + (v2 & 0xf0);
	if (res >= 0xa0)
		res += 0x60;
	m_sr &= ~SR_C;
	if (u8(res))
		m_sr &= ~SR_Z;
	if (res & 0x100)
		m_sr |= SR_C;
	return res;
}

u8 h8500_device::do_rotxl8(u8 v)
{
	const u8 c = m_sr & SR_C ? 1 : 0;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x80)
		m_sr |= SR_C;
	v = (v << 1) | c;
	if (!v)
		m_sr |= SR_Z;
	else if (s8(v) < 0)
		m_sr |= SR_N;
	return v;
}

u16 h8500_device::do_rotxl16(u16 v)
{
	const u16 c = m_sr & SR_C ? 1 : 0;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x8000)
		m_sr |= SR_C;
	v = (v << 1) | c;
	if (!v)
		m_sr |= SR_Z;
	else if (s16(v) < 0)
		m_sr |= SR_N;
	return v;
}

u8 h8500_device::do_rotxr8(u8 v)
{
	const u8 c = m_sr & SR_C ? 1 : 0;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x01)
		m_sr |= SR_C;
	v = (v >> 1) | (c << 7);
	if (!v)
		m_sr |= SR_Z;
	else if (s8(v) < 0)
		m_sr |= SR_N;
	return v;
}

u16 h8500_device::do_rotxr16(u16 v)
{
	const u16 c = m_sr & SR_C ? 1 : 0;
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x0001)
		m_sr |= SR_C;
	v = (v >> 1) | (c << 15);
	if (!v)
		m_sr |= SR_Z;
	else if (s16(v) < 0)
		m_sr |= SR_N;
	return v;
}

u8 h8500_device::do_shal8(u8 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x80)
		m_sr |= SR_C;
	if ((v & 0xc0) == 0x40 || (v & 0xc0) == 0x80)
		m_sr |= SR_V;
	v <<= 1;
	if (!v)
		m_sr |= SR_Z;
	else if (s8(v) < 0)
		m_sr |= SR_N;
	return v;
}

u16 h8500_device::do_shal16(u16 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 0x8000)
		m_sr |= SR_C;
	if ((v & 0xc000) == 0x4000 || (v & 0xc000) == 0x8000)
		m_sr |= SR_V;
	v <<= 1;
	if (!v)
		m_sr |= SR_Z;
	else if (s16(v) < 0)
		m_sr |= SR_N;
	return v;
}

u8 h8500_device::do_shar8(u8 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 1)
		m_sr |= SR_C;
	v >>= 1;
	if (!v)
		m_sr |= SR_Z;
	else if (v & 0x40)
	{
		v |= 0x80;
		m_sr |= SR_N;
	}
	return v;
}

u16 h8500_device::do_shar16(u16 v)
{
	m_sr &= ~(SR_N | SR_V | SR_Z | SR_C);
	if (v & 1)
		m_sr |= SR_C;
	v >>= 1;
	if (!v)
		m_sr |= SR_Z;
	else if (v & 0x4000)
	{
		v |= 0x8000;
		m_sr |= SR_N;
	}
	return v;
}

// ---------------------------------------------------------------------------
//  Execution loop
// ---------------------------------------------------------------------------

void h8500_device::execute_run()
{
	internal_update(total_cycles());

	m_icount -= m_count_before_instruction_step;
	if (m_icount < 0) {
		m_count_before_instruction_step = -m_icount;
		m_icount = 0;
	} else {
		m_count_before_instruction_step = 0;
	}

	while (m_bcount && m_icount <= m_bcount)
		internal_update(total_cycles() + m_icount - m_bcount);

	if (m_icount > 0 && m_inst_substate)
		do_exec_partial();

	while (m_icount > 0) {
		while(m_icount > m_bcount) {
			if (m_inst_state < 0x10000) {
				// m_pc has already been advanced past the prefetched byte;
				// m_npc holds the address of the byte we're about to execute.
				m_ppc = m_npc & 0xffff;
				if (debugger_enabled())
					debugger_instruction_hook(m_npc);
			}
			do_exec_full();
		}
		if (m_icount > 0)
			while (m_bcount && m_icount <= m_bcount)
				internal_update(total_cycles() + m_icount - m_bcount);
		if (m_icount > 0 && m_inst_substate)
			do_exec_partial();
	}
	if (m_icount < 0) {
		m_count_before_instruction_step = -m_icount;
		m_icount = 0;
	}
}

void h8500_device::add_event(u64 &event_time, u64 new_event)
{
	if (!new_event)
		return;
	if (!event_time || event_time > new_event)
		event_time = new_event;
}

void h8500_device::recompute_bcount(u64 event_time)
{
	if(!event_time || event_time >= total_cycles() + m_icount) {
		m_bcount = 0;
		return;
	}
	m_bcount = total_cycles() + m_icount - event_time;
}


void h8500_device::internal_update()
{
	internal_update(total_cycles());
}

void h8500_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c",
			BIT(m_sr, 3) ? 'N' : '.',
			BIT(m_sr, 2) ? 'Z' : '.',
			BIT(m_sr, 1) ? 'V' : '.',
			BIT(m_sr, 0) ? 'C' : '.');
		break;
	}
}

void h8500_device::set_irq(int irq_vector, int irq_level, bool irq_nmi)
{
	m_irq_vector = irq_vector;
	m_irq_level = irq_level;
	m_irq_nmi = irq_nmi;

	// Only NMI brings the chip out of software standby mode
	if (standby() && irq_nmi)
	{
		notify_standby(0);
		resume(SUSPEND_REASON_CLOCK);
		m_standby_cb(0);
		take_interrupt();
	}
}

bool h8500_device::trigger_dma(int vector)
{
	return false;
}

void h8500_device::update_active_dma_channel()
{
}

void h8500_device::request_state(int state)
{
}

// ---------------------------------------------------------------------------
//  Generated dispatcher & opcode bodies
// ---------------------------------------------------------------------------

#include "cpu/h8500/h8500.hxx"
