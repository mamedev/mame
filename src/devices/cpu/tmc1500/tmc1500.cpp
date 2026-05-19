// license:BSD-3-Clause
// copyright-holders:baco, Jeff Parsons
/***************************************************************************

    Texas Instruments TMC1500 family (TMC1501, TMC1502, TMC1503, ...)

    The TMC1500 family are 4-bit digit-serial microcomputers used in TI 
    calculators like the TI-57, TI-58, and TI-59.

    Architecture Overview:
    - 4-bit operational registers (A, B, C, D) of 16 digits (64 bits each)
    - 4-bit storage registers (X, Y) grouped in banks of 8 (64 bits each)
    - 13-bit instruction word
    - 11-bit program counter (2048 words address space)
    - 3-level subroutine stack
    - Digit-serial ALU with BCD support (selectable base 10 or 16)
    - Serial-access memory (SAM) architecture

    Register Mapping:
    - O Registers (Operational): A (0), B (1), C (2), D (3)
    - S Registers (Storage): X0-X7, Y0-Y7 (indexed by RAB)
    - RAB (Register Address Buffer): 3 bits
    - R5 Register: 8-bit latch used for keyboard input and status
    - COND: 1-bit condition latch

***************************************************************************/

#include "emu.h"
#include "tmc1500.h"
#include "tmc1500_dasm.h"

// Digit range mapping based on the Mask Field (MF) of the Instruction Word
static const int ranges[16][2] = {
	{12, 12}, // 0: MMSD (Mantissa MSD)
	{0, 15},  // 1: ALL
	{2, 12},  // 2: MANT (Mantissa)
	{0, 12},  // 3: MAEX (Mantissa and Exponent)
	{2, 2},   // 4: LLSD (Mantissa LSD)
	{0, 1},   // 5: EXP (Exponent)
	{0, 0},   // 6: (Reserved)
	{0, 13},  // 7: FMAEX (Fraction Mantissa and Exponent)
	{14, 14}, // 8: D14
	{13, 15}, // 9: FLAG (Registers 13-15)
	{14, 15}, // A: DIGIT
	{0, 0},   // B: (Reserved)
	{0, 0},   // C: (FF Group - Bit Ops)
	{13, 13}, // D: D13
	{0, 0},   // E: (PF Group - Misc Ops)
	{15, 15}  // F: D15
};

tmc1500_base_device::tmc1500_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 11, -1)
	, m_read_k(*this, 0)
	, m_write_o(*this)
	, m_write_r(*this)
{
}

void tmc1500_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_pc_last));
	save_item(NAME(m_opcode));
	save_item(NAME(m_stack));
	save_item(NAME(m_rab));
	save_item(NAME(m_base));
	save_item(NAME(m_cond));
	save_item(NAME(m_regR5));
	save_item(NAME(m_regsO));
	save_item(NAME(m_regsX));
	save_item(NAME(m_regsY));
	save_item(NAME(m_disp_activity));
	save_item(NAME(m_last_display_on));

	state_add(STATE_GENPC, "PC", m_pc).formatstr("%03X");
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(1, "RAB", m_rab).formatstr("%1X");
	state_add(2, "R5", m_regR5).formatstr("%02X");
	state_add(3, "COND", m_cond).formatstr("%1u");

	m_icountptr = &m_icount;
}

void tmc1500_base_device::device_reset()
{
	m_pc = 0;
	m_pc_last = 0;
	m_opcode = 0;
	m_rab = 0; 
	m_base = 10; 
	m_cond = false; 	
	m_regR5 = 0;
	m_scan_idx = 0;
	m_disp_activity = 0;
	m_last_display_on = false;

	memset(m_regsO, 0, sizeof(m_regsO));
	memset(m_regsX, 0, sizeof(m_regsX));
	memset(m_regsY, 0, sizeof(m_regsY));
	memset(m_stack, 0, sizeof(m_stack));
}

device_memory_interface::space_config_vector tmc1500_base_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_program_config) };
}

void tmc1500_base_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		m_pc_last = m_pc;
		m_opcode = m_program->read_word(m_pc);
		m_pc = (m_pc + 1) & 0x7ff;

		execute_instruction();
		m_icount -= 1;
	}
}

void tmc1500_base_device::execute_instruction() {
	u16 op = m_opcode;

	// Update display activity and blanking
	bool is_disp = (op & 0x0f00) == 0x0e00 && (op & 0x000f) == 0x07 && (op & 0x1000) == 0;
	if (is_disp) {
		for (int c = 0; c < 32; c++) {
			m_disp_activity = (m_disp_activity * 999) / 1000 + 1000;
		}
	} else {
		m_disp_activity = (m_disp_activity * 999) / 1000;
	}

	bool display_on = (m_disp_activity > 500000);
	if (display_on != m_last_display_on) {
		m_last_display_on = display_on;
		if (!display_on) {
			// Blank display
			for (int i = 0; i < 12; i++) {
				m_write_o(i, 0x0f, 0xffff);
			}
		} else {
			// Redraw display with current register values
			refresh_display();
		}
	}

	// Control Flow (Bit 12 = 1)
	if (op & 0x1000) {
		if (op & 0x0800) { // BRC/BRNC
			bool cond_needed = (op & 0x0400) != 0;
			if (m_cond == cond_needed) {
				m_pc = (m_pc & 0x0400) | (op & 0x03ff);
			}
		} else { // CALL
			push(m_pc);
			m_pc = op & 0x07ff;
		}
		m_cond = false;
		return;
	}

	// Masked Operations (Bit 12 = 0)
	int mask = (op & 0x0f00);
	int j = (op >> 6) & 0x03;
	int k = (op >> 3) & 0x07;
	int l = (op >> 1) & 0x03;
	int n = op & 0x01;
	
	int lo = ranges[mask >> 8][0];
	int hi = ranges[mask >> 8][1];

	// FF Group: Bit Operations
	if (mask == 0x0c00) {
		int d_field = (op >> 4) & 0x03;
		if (d_field != 0) {
			int bit = 1 << ((op >> 2) & 0x03);
			int mode = op & 0x03;
			int d = d_field + 12; // 13, 14, 15
			
			switch(mode) {
				case 0: m_regsO[j][d] |= bit; break; // SET
				case 1: m_regsO[j][d] &= ~bit; break; // RESET
				case 2: if (m_regsO[j][d] & bit) m_cond = true; break; // TEST
				case 3: m_regsO[j][d] ^= bit; break; // TOGGLE
			}
			return;
		}
	}
	// PF Group: Miscellaneous Operations
	else if (mask == 0x0e00) {
		int pf = op & 0x000f;
		switch(pf) {
			case 0x00: reg_store(m_regsO[0], m_regsY[m_rab]); break; // STYA
			case 0x01: m_rab = (op >> 4) & 0x07; break; // RABI
			case 0x02: m_pc = m_regR5; break; // BRR5
			case 0x03: opRET(); break; // RET
			case 0x04: reg_store(m_regsX[m_rab], m_regsO[0]); break; // STAX
			case 0x05: reg_store(m_regsO[0], m_regsX[m_rab]); break; // STXA
			case 0x06: reg_store(m_regsY[m_rab], m_regsO[0]); break; // STAY
			case 0x07: opDISP(); break; // DISP
			case 0x08: m_base = 10; break; // BCDS
			case 0x09: m_base = 16; break; // BCDR
			case 0x0a: m_rab = m_regR5 & 0x07; break; // RABR5
		}
		return;
	}

	// Standard ALU Operations
	int base = (mask >= 0x0800) ? 16 : m_base;
	uint8_t src2[16] = {0};

	// Prepare second operand
	switch(k) {
		case 0: case 1: case 2: case 3: 
			for(int i=0; i<16; i++) src2[i] = m_regsO[k][i]; 
			break;
		case 4: src2[lo] = 1; break; 
		case 5: break; // Shift
		case 6: src2[lo] = m_regR5 & 0x0f; break; // R5L
		case 7: 
			src2[lo] = m_regR5 & 0x0f; 
			if (lo < 15) src2[lo+1] = (m_regR5 >> 4) & 0x0f;
			break;
	}

	uint8_t *p_dst = nullptr;
	uint8_t dummy[16] = {0};

	// Determine destination
	switch(l) {
		case 0: p_dst = m_regsO[j]; break;
		case 1: if (k < 4) p_dst = m_regsO[k]; break;
		case 2: 
			if (k < 5) p_dst = dummy; 
			else if (k == 5) p_dst = m_regsO[j]; 
			break;
		case 3: // XCHG or MOVE
			if (n == 0) { // XCHG
				if (j == 0 && k < 4) reg_xchg(m_regsO[0], m_regsO[k], lo, hi);
			} else { // MOVE
				if (k != 5) reg_move(m_regsO[j], src2, lo, hi, base);
			}
			return;
	}

	if (p_dst == nullptr) return;

	// Execute operation
	if (k == 5) { // Shift
		if (n == 0) reg_shl(p_dst, m_regsO[j], lo, hi);
		else reg_shr(p_dst, m_regsO[j], lo, hi);
	} else { // Arithmetic
		if (n == 0) reg_add(p_dst, m_regsO[j], src2, lo, hi, base);
		else reg_sub(p_dst, m_regsO[j], src2, lo, hi, base);
	}
}

// Stack helpers
void tmc1500_base_device::push(uint16_t addr) {
	m_stack[2] = m_stack[1];
	m_stack[1] = m_stack[0];
	m_stack[0] = addr;
}

uint16_t tmc1500_base_device::pop() {
	uint16_t addr = m_stack[0];
	m_stack[0] = m_stack[1];
	m_stack[1] = m_stack[2];
	m_stack[2] = 0;
	return addr;
}

void tmc1500_base_device::opRET() {
	m_cond = false;
	m_pc = pop();
}

// Architectural helpers for R5
void tmc1500_base_device::update_r5(int lo, int hi, const uint8_t *data) {
	m_regR5 = data[lo] & 0x0f;
	if (lo < hi) {
		m_regR5 |= (data[lo + 1] & 0x0f) << 4;
	}
}

// Digit-serial Arithmetic
void tmc1500_base_device::reg_add(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int lo, int hi, int base) {
	int carry = 0; 
	for (int i = lo; i <= hi; i++) { 
		int r = src1[i] + src2[i] + carry; 
		if (r >= base) { r -= base; carry = 1; } 
		else carry = 0; 
		dst[i] = r & 0x0f; 
	}
	if (carry) m_cond = true; 
	update_r5(lo, hi, dst);
}

void tmc1500_base_device::reg_sub(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int lo, int hi, int base) {
	int carry = 0;
	for (int i = lo; i <= hi; i++) { 
		int r = src1[i] - src2[i] - carry; 
		if (r < 0) { r += base; carry = 1; } 
		else carry = 0; 
		dst[i] = r & 0x0f; 
	}
	if (carry) m_cond = true; 
	update_r5(lo, hi, dst);
}

void tmc1500_base_device::reg_move(uint8_t *dst, const uint8_t *src, int lo, int hi, int base) {
	int carry = 0;
	for (int i = lo; i <= hi; i++) {
		int r = src[i] + carry;
		if (r >= base) { r -= base; carry = 1; }
		else carry = 0;
		dst[i] = r & 0x0f;
	}
	if (carry) m_cond = true; 
	update_r5(lo, hi, src); 
}

// Shift operations
void tmc1500_base_device::reg_shl(uint8_t *dst, const uint8_t *src, int lo, int hi) {
	for (int i = hi; i > lo; i--) {
		dst[i] = src[i - 1];
	}
	dst[lo] = 0;
	update_r5(lo, hi, dst);
}

void tmc1500_base_device::reg_shr(uint8_t *dst, const uint8_t *src, int lo, int hi) {
	for (int i = lo; i < hi; i++) {
		dst[i] = src[i + 1];
	}
	dst[hi] = 0;
	update_r5(lo, hi, dst);
}

void tmc1500_base_device::reg_store(uint8_t *dst, const uint8_t *src) {
	for (int i = 0; i < 16; i++) dst[i] = src[i];
}

void tmc1500_base_device::reg_xchg(uint8_t *r1, uint8_t *r2, int lo, int hi) {
	for (int i = lo; i <= hi; i++) {
		uint8_t tmp = r1[i];
		r1[i] = r2[i];
		r2[i] = tmp;
	}
	update_r5(lo, hi, r1); // Update from the first register after swap
}

// Display and Keyboard scanning
void tmc1500_base_device::refresh_display() {
	for (int i = 11; i >= 0; i--) {
		uint8_t a = m_regsO[0][i];
		uint8_t b = m_regsO[1][i];
		
		uint16_t data = 0;
		if (b & 0x08) data = 0x0f; // Blank
		else if (b & 0x01) data = 0x0e; // Minus sign
		else data = a & 0x0f;
		if (b & 0x02) data |= 0x10; // Decimal point
		
		m_write_o(11 - i, data, 0xffff);
	}
}

void tmc1500_base_device::opDISP() {
	m_cond = false;
	m_regR5 = 0; 

	bool display_on = (m_disp_activity > 500000);

	for (int i = 11; i >= 0; i--) {
		m_scan_idx = i;
		
		uint16_t data = 0x0f; // Blank by default if display is off
		if (display_on) {
			uint8_t a = m_regsO[0][i];
			uint8_t b = m_regsO[1][i];
			
			if (b & 0x08) data = 0x0f; // Blank
			else if (b & 0x01) data = 0x0e; // Minus sign
			else data = a & 0x0f;
			if (b & 0x02) data |= 0x10; // Decimal point
		}
		
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
			if (col != 0) {
				if (i < 8) {
					m_regR5 = (7 - i) | (col << 4);
					m_cond = true;
				}
			}
		}
	}
	
	m_icount -= 31;
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
