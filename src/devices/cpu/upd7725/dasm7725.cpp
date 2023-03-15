// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu
/***************************************************************************

    dasm7725.c
    Disassembler for the portable uPD7725 emulator.
    Written by byuu
    MAME conversion by R. Belmont

***************************************************************************/

#include "emu.h"
#include "dasm7725.h"

u32 necdsp_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t necdsp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t opcode = opcodes.r32(pc) >> 8;
	uint32_t type = (opcode >> 22);
	offs_t flags = 0;

//  printf("dasm: PC %x opcode %08x\n", pc, opcode);

	if(type == 0 || type == 1) {  //OP,RT
		uint8_t pselect = (opcode >> 20)&0x3;  //P select
		uint8_t alu     = (opcode >> 16)&0xf;  //ALU operation mode
		uint8_t asl     = (opcode >> 15)&0x1;  //accumulator select
		uint8_t dpl     = (opcode >> 13)&0x3;  //DP low modify
		uint8_t dphm    = (opcode >>  9)&0xf;  //DP high XOR modify
		uint8_t rpdcr   = (opcode >>  8)&0x1;  //RP decrement
		uint8_t src     = (opcode >>  4)&0xf;  //move source
		uint8_t dst     = (opcode >>  0)&0xf;  //move destination

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

	if(dst) {
		stream << " | mov ";

		switch(src) {
		case  0: stream << "trb,"; break;
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
		case 14: stream << "trb"; break;
		case 15: stream << "mem"; break;
		}
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
		case  8: stream << " | m8"; break;
		case  9: stream << " | m9"; break;
		case 10: stream << " | ma"; break;
		case 11: stream << " | mb"; break;
		case 12: stream << " | mc"; break;
		case 13: stream << " | md"; break;
		case 14: stream << " | me"; break;
		case 15: stream << " | mf"; break;
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
		uint16_t brch = (opcode >> 13) & 0x1ff;  //branch
		uint16_t na  = (opcode >>  2) & 0x7ff;  //next address

	switch(brch) {
		case 0x000: stream << "jmpso "; break;
		case 0x080: stream << "jnca "; flags = STEP_COND; break;
		case 0x082: stream << "jca "; flags = STEP_COND; break;
		case 0x084: stream << "jncb "; flags = STEP_COND; break;
		case 0x086: stream << "jcb "; flags = STEP_COND; break;
		case 0x088: stream << "jnza "; flags = STEP_COND; break;
		case 0x08a: stream << "jza "; flags = STEP_COND; break;
		case 0x08c: stream << "jnzb "; flags = STEP_COND; break;
		case 0x08e: stream << "jzb "; flags = STEP_COND; break;
		case 0x090: stream << "jnova0 "; flags = STEP_COND; break;
		case 0x092: stream << "jova0 "; flags = STEP_COND; break;
		case 0x094: stream << "jnovb0 "; flags = STEP_COND; break;
		case 0x096: stream << "jovb0 "; flags = STEP_COND; break;
		case 0x098: stream << "jnova1 "; flags = STEP_COND; break;
		case 0x09a: stream << "jova1 "; flags = STEP_COND; break;
		case 0x09c: stream << "jnovb1 "; flags = STEP_COND; break;
		case 0x09e: stream << "jovb1 "; flags = STEP_COND; break;
		case 0x0a0: stream << "jnsa0 "; flags = STEP_COND; break;
		case 0x0a2: stream << "jsa0 "; flags = STEP_COND; break;
		case 0x0a4: stream << "jnsb0 "; flags = STEP_COND; break;
		case 0x0a6: stream << "jsb0 "; flags = STEP_COND; break;
		case 0x0a8: stream << "jnsa1 "; flags = STEP_COND; break;
		case 0x0aa: stream << "jsa1 "; flags = STEP_COND; break;
		case 0x0ac: stream << "jnsb1 "; flags = STEP_COND; break;
		case 0x0ae: stream << "jsb1 "; flags = STEP_COND; break;
		case 0x0b0: stream << "jdpl0 "; flags = STEP_COND; break;
		case 0x0b1: stream << "jdpln0 "; flags = STEP_COND; break;
		case 0x0b2: stream << "jdplf "; flags = STEP_COND; break;
		case 0x0b3: stream << "jdplnf "; flags = STEP_COND; break;
		case 0x0b4: stream << "jnsiak "; flags = STEP_COND; break;
		case 0x0b6: stream << "jsiak "; flags = STEP_COND; break;
		case 0x0b8: stream << "jnsoak "; flags = STEP_COND; break;
		case 0x0ba: stream << "jsoak "; flags = STEP_COND; break;
		case 0x0bc: stream << "jnrqm "; flags = STEP_COND; break;
		case 0x0be: stream << "jrqm "; flags = STEP_COND; break;
		case 0x100: stream << "ljmp "; break;
		case 0x101: stream << "hjmp "; break;
		case 0x140: stream << "lcall "; flags = STEP_OVER; break;
		case 0x141: stream << "hcall "; flags = STEP_OVER; break;
		default:    stream << "??????  "; break;
	}

	util::stream_format(stream, "$%x", na);
	}

	if(type == 3) {  //LD
	stream << "ld ";
	uint16_t id = opcode >> 6;
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
		case 14: stream << "trb"; break;
		case 15: stream << "mem"; break;
	}
	}

	return 1 | flags | SUPPORTED;
}
