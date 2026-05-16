// license:BSD-3-Clause
// copyright-holders:baco, Antigravity
/***************************************************************************

    Texas Instruments TMC1500 family (TMC1501, TMC1502, TMC1503)

    Architecture: 
    - Digit-serial 4-bit processor
    - SAM (Serial Access Memory) structure:
      - 4 Operational Registers (A, B, C, D) of 16 digits each.
      - 8 Storage Registers X (X0-X7) and 8 Storage Registers Y (Y0-Y7).
    - 13-bit instruction word.
    - 12-digit LED multiplexed display output.

***************************************************************************/

#include "emu.h"
#include "tmc1500.h"
#include "tmc1500_dasm.h"

// Device definitions
tmc1500_base_device::tmc1500_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 12, -1)
	, m_read_k(*this)
	, m_write_o(*this)
	, m_write_r(*this)
{
}

void tmc1500_base_device::device_start() {
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_stack));
	save_item(NAME(m_rab));
	save_item(NAME(m_base));
	save_item(NAME(m_cond));
	save_item(NAME(m_regR5));
	save_item(NAME(m_regsO));
	save_item(NAME(m_regsX));
	save_item(NAME(m_regsY));

	state_add(STATE_GENPC, "PC", m_pc).formatstr("%03X");
	state_add(TMC1500_PC, "PC", m_pc).formatstr("%03X");
	state_add(TMC1500_A, "A", m_regsO[0][0]); // Simplified display
	state_add(TMC1500_R5, "R5", m_regR5).formatstr("%02X");

	m_icountptr = &m_icount;
}

void tmc1500_base_device::device_reset() {
	m_pc = 0;
	m_cond = false;
	m_regR5 = 0;
	m_base = 10;
	m_rab = 0;
	memset(m_stack, 0, sizeof(m_stack));
	memset(m_regsO, 0, sizeof(m_regsO));
	memset(m_regsX, 0, sizeof(m_regsX));
	memset(m_regsY, 0, sizeof(m_regsY));
}

void tmc1500_base_device::execute_run() {
	while (m_icount > 0) {
		m_pc_last = m_pc;
		debugger_instruction_hook(m_pc);
		m_opcode = m_program->read_word(m_pc) & 0x1fff;
		m_pc = (m_pc + 1) & 0x7ff;
		m_icount -= 1;
		execute_instruction();
	}
}

device_memory_interface::space_config_vector tmc1500_base_device::memory_space_config() const {
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_program_config) };
}

// Instruction Decoding
void tmc1500_base_device::execute_instruction() {
	uint16_t op = m_opcode;

	// 1. Control Flow (Bit 12 = 1)
	if (op & 0x1000) {
		if (op & 0x0800) { // Branch
			bool target_cond = (op & 0x0400) ? m_cond : !m_cond;
			if (target_cond) m_pc = (m_pc_last & 0x0400) | (op & 0x03ff);
		} else { // CALL
			opCALL(op & 0x07ff);
		}
		return;
	}

	// 2. Masked Arithmetic/Move (Bit 12 = 0)
	int mask = (op >> 8) & 0x0f;
	int j    = (op >> 6) & 0x03;
	int k    = (op >> 3) & 0x07;
	int l    = (op >> 1) & 0x03;
	int n    = op & 0x01;

	// Define lo/hi digit masks based on mask field
	static const int masks_lo[16] = { 15, 0, 2, 2, 0, 0, 0, 2, 14, 0, 0, 0, 0, 13, 0, 15 };
	static const int masks_hi[16] = { 15, 15, 11, 13, 0, 1, 0, 15, 14, 0, 0, 0, 0, 13, 0, 15 };
	
	int lo = masks_lo[mask];
	int hi = masks_hi[mask];

	// Handle special groups PF (PF0-PF15) and FF (FF0-FF15)
	if (mask == 0x0e) { // PF Group
		int pf = op & 0x0f;
		switch (pf) {
			case 2: // BRR5
				if (m_cond) m_pc = (m_pc_last & 0x0400) | (m_regR5 & 0x03ff);
				break;
			case 3: opRET(); break;
			case 7: opDISP(); break;
			case 8: m_base = 10; break;
			case 9: m_base = 16; break;
			// Other PF instructions can be added here
		}
		return;
	}

	// Arithmetic Logic
	uint8_t *p_dst = (l == 0) ? m_regsO[j] : (l == 1 ? (k < 4 ? m_regsO[k] : nullptr) : (k < 5 ? nullptr : m_regsO[j]));
	uint8_t src2_buf[16];
	uint8_t *src2 = nullptr;

	if (k < 4) src2 = m_regsO[k];
	else if (k == 4) { memset(src2_buf, 0, 16); src2_buf[lo] = 1; src2 = src2_buf; }
	else if (k == 5) { /* Shift - handled below */ }
	else if (k == 6) { src2 = m_regsO[1]; /* R5L - simplified */ }
	else if (k == 7) { src2 = m_regsO[1]; /* R5 - simplified */ }

	if (!p_dst && k != 5) return;

	if (k == 5) { // Shift
		if (n == 0) reg_shl(p_dst, m_regsO[j], lo, hi);
		else reg_shr(p_dst, m_regsO[j], lo, hi);
	} else { // Arithmetic
		if (n == 0) reg_add(p_dst, m_regsO[j], src2, lo, hi, m_base);
		else reg_sub(p_dst, m_regsO[j], src2, lo, hi, m_base);
	}
}

// Arithmetic Core (Digit-Serial)
void tmc1500_base_device::reg_add(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int lo, int hi, int base) {
	int carry = 0;
	for (int i = lo; i <= hi; i++) {
		int sum = src1[i] + src2[i] + carry;
		if (sum >= base) { sum -= base; carry = 1; }
		else carry = 0;
		if (dst) dst[i] = sum & 0x0f;
	}
	m_cond = (carry != 0);
}

void tmc1500_base_device::reg_sub(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int lo, int hi, int base) {
	int borrow = 0;
	for (int i = lo; i <= hi; i++) {
		int diff = src1[i] - src2[i] - borrow;
		if (diff < 0) { diff += base; borrow = 1; }
		else borrow = 0;
		if (dst) dst[i] = diff & 0x0f;
	}
	m_cond = (borrow == 0); // TI-57 uses "No Borrow" logic for condition
}

// Stack helpers
void tmc1500_base_device::opCALL(uint16_t addr) {
	push(m_pc);
	m_pc = addr;
}

void tmc1500_base_device::push(uint16_t addr) {
	m_stack[2] = m_stack[1];
	m_stack[1] = m_stack[0];
	m_stack[0] = addr;
}

uint16_t tmc1500_base_device::pop() {
	uint16_t addr = m_stack[0];
	m_stack[1] = m_stack[0]; // Simplified
	m_stack[2] = m_stack[1];
	return addr;
}

void tmc1500_base_device::opRET() {
	m_pc = pop();
}

// Shift operations
void tmc1500_base_device::reg_shl(uint8_t *dst, const uint8_t *src, int lo, int hi) {
	for (int i = hi; i > lo; i--) dst[i] = src[i - 1];
	dst[lo] = 0;
}

void tmc1500_base_device::reg_shr(uint8_t *dst, const uint8_t *src, int lo, int hi) {
	for (int i = lo; i < hi; i++) dst[i] = src[i + 1];
	dst[hi] = 0;
}

// Display and Keyboard scanning
void tmc1500_base_device::opDISP() {
	m_regR5 = 0; 
	m_icount -= 32; // DISP takes exactly 32 cycles

	for (int i = 0; i < 16; i++) {
		m_scan_idx = i;
		
		uint8_t a = m_regsO[0][i];
		uint8_t b = m_regsO[1][i];
		
		uint16_t data = 0;
		if (b & 0x08) data = 0x0f; // Blank
		else if (b & 0x01) data = 0x0e; // Minus sign
		else data = a & 0x0f;
		if (b & 0x02) data |= 0x10; // Decimal point
		
		m_write_o(11 - i, data, 0xffff);

		uint8_t k = m_read_k(0);
		if (k != 0) {
			int col = 0;
			for (int b_idx = 0; b_idx < 5; b_idx++) {
				if (k & (1 << b_idx)) {
					col = b_idx + 1;
					break;
				}
			}
			if (col != 0 && i < 8) {
				m_regR5 = (7 - i) | (col << 4);
				m_cond = true;
			}
		}
	}
	// Blanking at the end of scan to ensure display turns off if DISP is not called regularly
	m_write_o(0x100, 0, 0xffff); // Special code for "End of scan / Blank all"
}

std::unique_ptr<util::disasm_interface> tmc1500_base_device::create_disassembler() { return std::make_unique<tmc1500_disassembler>(); }

void tmc1500_base_device::state_string_export(const device_state_entry &entry, std::string &str) const {
	switch (entry.index()) {
		case STATE_GENPC: str = util::string_format("%03X", m_pc); break;
	}
}

tmc1501_cpu_device::tmc1501_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : tmc1500_base_device(mconfig, TMC1501, tag, owner, clock) { }
tmc1502_cpu_device::tmc1502_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : tmc1500_base_device(mconfig, TMC1502, tag, owner, clock) { }
tmc1503_cpu_device::tmc1503_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : tmc1500_base_device(mconfig, TMC1503, tag, owner, clock) { }

DEFINE_DEVICE_TYPE(TMC1501, tmc1501_cpu_device, "tmc1501", "TI TMC1501")
DEFINE_DEVICE_TYPE(TMC1502, tmc1502_cpu_device, "tmc1502", "TI TMC1502")
DEFINE_DEVICE_TYPE(TMC1503, tmc1503_cpu_device, "tmc1503", "TI TMC1503")
