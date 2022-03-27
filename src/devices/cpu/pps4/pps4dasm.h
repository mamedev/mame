// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   pps4dasm.c
 *
 *   Rockwell PPS-4 CPU Disassembly
 *
 *
 * TODO: double verify all opcodes with t_Ixx flags
 *
 *****************************************************************************/

#ifndef MAME_CPU_PPS4_PPS4DASM_H
#define MAME_CPU_PPS4_PPS4DASM_H

#pragma once

class pps4_disassembler : public util::disasm_interface
{
public:
	pps4_disassembler() = default;
	virtual ~pps4_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	typedef enum pps4_token_e : uint32_t {
		t_AD,       t_ADC,      t_ADSK,     t_ADCSK,    t_ADI,
		t_DC,       t_AND,      t_OR,       t_EOR,      t_COMP,
		t_SC,       t_RC,       t_SF1,      t_RF1,      t_SF2,
		t_RF2,      t_LD,       t_EX,       t_EXD,      t_LDI,
		t_LAX,      t_LXA,      t_LABL,     t_LBMX,     t_LBUA,
		t_XABL,     t_XBMX,     t_XAX,      t_XS,       t_CYS,
		t_LB,       t_LBL,      t_INCB,     t_DECB,     t_T,
		t_TM,       t_TL,       t_TML,      t_SKC,      t_SKZ,
		t_SKBI,     t_SKF1,     t_SKF2,     t_RTN,      t_RTNSK,
		t_IOL,      t_DIA,      t_DIB,      t_DOA,      t_SAG,
		t_COUNT,
		t_MASK = (1 << 6) - 1,
		t_I3c  = 1 <<  6,   /* immediate 3 bit constant, complemented */
		t_I4   = 1 <<  7,   /* immediate 4 bit constant */
		t_I4c  = 1 <<  8,   /* immediate 4 bit constant, complemented */
		t_I4p  = 1 <<  9,   /* immediate 4 bit offset into page 3 */
		t_I6p  = 1 << 10,   /* immediate 6 bit constant; address in current page */
		t_I6i  = 1 << 11,   /* immediate 6 bit indirect page 3 offset (16 ... 63) + followed by page 1 address */
		t_I8   = 1 << 12,   /* immediate 8 bit constant (I/O port number) */
		t_I8c  = 1 << 13,   /* immediate 8 bit constant inverted */
		t_OVER = 1 << 14,   /* Debugger step over (CALL) */
		t_OUT  = 1 << 15,   /* Debugger step out (RETURN) */
		t_COND = 1 << 16    /* Debugger conditional branch */
	}   pps4_token_e;

	static char const *const token_str[t_COUNT];
	static uint32_t const table[];

};

#endif
