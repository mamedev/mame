// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu,Jonathan Gevaryahu
/***************************************************************************

    dasm7720.cpp
    Disassembler for the portable uPD7720 emulator.
    Written by byuu
    MAME conversion by R. Belmont
    Adapted for uPD7720 by Lord Nightmare

***************************************************************************/

#include "emu.h"
#include "dasm7720.h"

u32 upd7720_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t upd7720_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t opcode = opcodes.r32(pc) >> 0; // 0 for little endian, 8 for big endian
	uint32_t type = (opcode >> 21);
	offs_t flags = 0;

//  printf("dasm: PC %x opcode %08x\n", pc, opcode);

	if(type == 0 || type == 1) {  //OP,RT
		uint8_t pselect = (opcode >> 19)&0x3;  //P select
		uint8_t alu     = (opcode >> 15)&0xf;  //ALU operation mode
		uint8_t asl     = (opcode >> 14)&0x1;  //accumulator select
		uint8_t dpl     = (opcode >> 12)&0x3;  //DP low modify
		uint8_t dphm    = (opcode >>  9)&0x7;  //DP high XOR modify
		uint8_t rpdcr   = (opcode >>  8)&0x1;  //RP decrement
		uint8_t src     = (opcode >>  4)&0xf;  //move source
		uint8_t dst     = (opcode >>  0)&0xf;  //move destination

	if(dst || (pselect == 1)) {
		stream << "mov  ";

		switch(src) {
		case  0: stream << "non,"; break;
		case  1: stream << "a,"; break;
		case  2: stream << "b,"; break;
		case  3: stream << "tr,"; break;
		case  4: stream << "dp,"; break;
		case  5: stream << "rp,"; break;
		case  6: stream << "ro,"; break;
		case  7: stream << "sgn,"; break;
		case  8: stream << "dr,"; break;
		case  9: stream << "drnf,"; break;
		case 10: stream << "sr,"; break;
		case 11: stream << "sim,"; break;
		case 12: stream << "sil,"; break;
		case 13: stream << "k,"; break;
		case 14: stream << "l,"; break;
		case 15: stream << "mem,"; break;
		}

		switch(dst) {
		case  0: stream << "@non"; break;
		case  1: stream << "@a"; break;
		case  2: stream << "@b"; break;
		case  3: stream << "@tr"; break;
		case  4: stream << "@dp"; break;
		case  5: stream << "@rp"; break;
		case  6: stream << "@dr"; break;
		case  7: stream << "@sr"; break;
		case  8: stream << "@sol"; break;
		case  9: stream << "@som"; break;
		case 10: stream << "@k"; break;
		case 11: stream << "@klr"; break;
		case 12: stream << "@klm"; break;
		case 13: stream << "@l"; break;
		case 14: stream << "@non2"; break;
		case 15: stream << "@mem"; break;
		}
		stream << " | ";
	}

	switch(alu) {
		case  0: stream << "nop  "; break;
		case  1: stream << "or   "; break;
		case  2: stream << "and  "; break;
		case  3: stream << "xor  "; break;
		case  4: stream << "sub  "; break;
		case  5: stream << "add  "; break;
		case  6: stream << "sbb  "; break;
		case  7: stream << "adc  "; break;
		case  8: stream << "dec  "; break;
		case  9: stream << "inc  "; break;
		case 10: stream << "cmp  "; break;
		case 11: stream << "shr1 "; break;
		case 12: stream << "shl1 "; break;
		case 13: stream << "shl2 "; break;
		case 14: stream << "shl4 "; break;
		case 15: stream << "xchg "; break;
	}

	if(alu < 8) {
		switch(pselect) {
		case 0: stream << "ram,"; break;
		case 1: stream << "idb,"; break;
		case 2: stream << "m,"; break;
		case 3: stream << "n,"; break;
		}
	}

	switch(asl) {
		case 0: stream << "a"; break;
		case 1: stream << "b"; break;
	}

	if(dpl) {
		switch(dpl) {
		case 0: stream << " | dpnop"; break;
		case 1: stream << " | dpinc"; break;
		case 2: stream << " | dpdec"; break;
		case 3: stream << " | dpclr"; break;
		}
	}

	if(dphm) {
		switch(dphm) {
		case  0: stream << " | m0"; break;
		case  1: stream << " | m1"; break;
		case  2: stream << " | m2"; break;
		case  3: stream << " | m3"; break;
		case  4: stream << " | m4"; break;
		case  5: stream << " | m5"; break;
		case  6: stream << " | m6"; break;
		case  7: stream << " | m7"; break;
		}
	}

	if(rpdcr == 1) {
		stream << " | rpdec";
	}

	if(type == 1) {
		stream << " | ret";
		flags = STEP_OUT;
	}
	}

	if(type == 2) {  //JP
		uint16_t brch = (opcode >> 13) & 0xff;  //branch
		uint16_t na  = (opcode >>  4) & 0x1ff;  //next address

	switch(brch) {
		case 0x40: stream << "jnca "; flags = STEP_COND; break;
		case 0x41: stream << "jca "; flags = STEP_COND; break;
		case 0x42: stream << "jncb "; flags = STEP_COND; break;
		case 0x43: stream << "jcb "; flags = STEP_COND; break;
		case 0x44: stream << "jnza "; flags = STEP_COND; break;
		case 0x45: stream << "jza "; flags = STEP_COND; break;
		case 0x46: stream << "jnzb "; flags = STEP_COND; break;
		case 0x47: stream << "jzb "; flags = STEP_COND; break;
		case 0x48: stream << "jnova0 "; flags = STEP_COND; break;
		case 0x49: stream << "jova0 "; flags = STEP_COND; break;
		case 0x4a: stream << "jnovb0 "; flags = STEP_COND; break;
		case 0x4b: stream << "jovb0 "; flags = STEP_COND; break;
		case 0x4c: stream << "jnova1 "; flags = STEP_COND; break;
		case 0x4d: stream << "jova1 "; flags = STEP_COND; break;
		case 0x4e: stream << "jnovb1 "; flags = STEP_COND; break;
		case 0x4f: stream << "jovb1 "; flags = STEP_COND; break;
		case 0x50: stream << "jnsa0 "; flags = STEP_COND; break;
		case 0x51: stream << "jsa0 "; flags = STEP_COND; break;
		case 0x52: stream << "jnsb0 "; flags = STEP_COND; break;
		case 0x53: stream << "jsb0 "; flags = STEP_COND; break;
		case 0x54: stream << "jnsa1 "; flags = STEP_COND; break;
		case 0x55: stream << "jsa1 "; flags = STEP_COND; break;
		case 0x56: stream << "jnsb1 "; flags = STEP_COND; break;
		case 0x57: stream << "jsb1 "; flags = STEP_COND; break;
		case 0x58: stream << "jdpl0 "; flags = STEP_COND; break;
		case 0x59: stream << "jdplf "; flags = STEP_COND; break;
		case 0x5a: stream << "jnsiak "; flags = STEP_COND; break;
		case 0x5b: stream << "jsiak "; flags = STEP_COND; break;
		case 0x5c: stream << "jnsoak "; flags = STEP_COND; break;
		case 0x5d: stream << "jsoak "; flags = STEP_COND; break;
		case 0x5e: stream << "jnrqm "; flags = STEP_COND; break;
		case 0x5f: stream << "jrqm "; flags = STEP_COND; break;
		case 0x80: stream << "jmp "; break;
		case 0xa0: stream << "call "; flags = STEP_OVER; break;
		default:    stream << "??????  "; break;
	}

	util::stream_format(stream, "$%x", na);
	}

	if(type == 3) {  //LD
	stream << "ld ";
	uint16_t id = opcode >> 5;
	uint8_t dst = (opcode >> 0) & 0xf;  //destination

	util::stream_format(stream, "$%x,", id);

	switch(dst) {
		case  0: stream << "non"; break;
		case  1: stream << "a"; break;
		case  2: stream << "b"; break;
		case  3: stream << "tr"; break;
		case  4: stream << "dp"; break;
		case  5: stream << "rp"; break;
		case  6: stream << "dr"; break;
		case  7: stream << "sr"; break;
		case  8: stream << "sol"; break;
		case  9: stream << "som"; break;
		case 10: stream << "k"; break;
		case 11: stream << "klr"; break;
		case 12: stream << "klm"; break;
		case 13: stream << "l"; break;
		case 14: stream << "non2"; break;
		case 15: stream << "mem"; break;
	}
	}

	return 1 | flags | SUPPORTED;
}
