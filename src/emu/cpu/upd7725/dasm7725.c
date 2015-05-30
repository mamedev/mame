// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu
/***************************************************************************

    dasm7725.c
    Disassembler for the portable uPD7725 emulator.
    Written by byuu
    MAME conversion by R. Belmont

***************************************************************************/

#include "emu.h"
#include "upd7725.h"

CPU_DISASSEMBLE( upd7725 )
{
	static char output[256];
	UINT32 opcode = oprom[2] | (oprom[1] << 8) | (oprom[0] << 16);
	UINT32 type = (opcode >> 22);

//  printf("dasm: PC %x opcode %08x\n", pc, opcode);

	memset(output, 0, sizeof(output));

	if(type == 0 || type == 1) {  //OP,RT
		UINT8 pselect = (opcode >> 20)&0x3;  //P select
		UINT8 alu     = (opcode >> 16)&0xf;  //ALU operation mode
		UINT8 asl     = (opcode >> 15)&0x1;  //accumulator select
		UINT8 dpl     = (opcode >> 13)&0x3;  //DP low modify
		UINT8 dphm    = (opcode >>  9)&0xf;  //DP high XOR modify
		UINT8 rpdcr   = (opcode >>  8)&0x1;  //RP decrement
		UINT8 src     = (opcode >>  4)&0xf;  //move source
		UINT8 dst     = (opcode >>  0)&0xf;  //move destination

	switch(alu) {
		case  0: strcat(output, "nop  "); break;
		case  1: strcat(output, "or   "); break;
		case  2: strcat(output, "and  "); break;
		case  3: strcat(output, "xor  "); break;
		case  4: strcat(output, "sub  "); break;
		case  5: strcat(output, "add  "); break;
		case  6: strcat(output, "sbb  "); break;
		case  7: strcat(output, "adc  "); break;
		case  8: strcat(output, "dec  "); break;
		case  9: strcat(output, "inc  "); break;
		case 10: strcat(output, "cmp  "); break;
		case 11: strcat(output, "shr1 "); break;
		case 12: strcat(output, "shl1 "); break;
		case 13: strcat(output, "shl2 "); break;
		case 14: strcat(output, "shl4 "); break;
		case 15: strcat(output, "xchg "); break;
	}

	if(alu < 8) {
		switch(pselect) {
		case 0: strcat(output, "ram,"); break;
		case 1: strcat(output, "idb,"); break;
		case 2: strcat(output, "m,"); break;
		case 3: strcat(output, "n,"); break;
		}
	}

	switch(asl) {
		case 0: strcat(output, "a"); break;
		case 1: strcat(output, "b"); break;
	}

	if(dst) {
		strcat(output, " | mov ");

		switch(src) {
		case  0: strcat(output, "trb,"); break;
		case  1: strcat(output, "a,"); break;
		case  2: strcat(output, "b,"); break;
		case  3: strcat(output, "tr,"); break;
		case  4: strcat(output, "dp,"); break;
		case  5: strcat(output, "rp,"); break;
		case  6: strcat(output, "ro,"); break;
		case  7: strcat(output, "sgn,"); break;
		case  8: strcat(output, "dr,"); break;
		case  9: strcat(output, "drnf,"); break;
		case 10: strcat(output, "sr,"); break;
		case 11: strcat(output, "sim,"); break;
		case 12: strcat(output, "sil,"); break;
		case 13: strcat(output, "k,"); break;
		case 14: strcat(output, "l,"); break;
		case 15: strcat(output, "mem,"); break;
		}

		switch(dst) {
		case  0: strcat(output, "non"); break;
		case  1: strcat(output, "a"); break;
		case  2: strcat(output, "b"); break;
		case  3: strcat(output, "tr"); break;
		case  4: strcat(output, "dp"); break;
		case  5: strcat(output, "rp"); break;
		case  6: strcat(output, "dr"); break;
		case  7: strcat(output, "sr"); break;
		case  8: strcat(output, "sol"); break;
		case  9: strcat(output, "som"); break;
		case 10: strcat(output, "k"); break;
		case 11: strcat(output, "klr"); break;
		case 12: strcat(output, "klm"); break;
		case 13: strcat(output, "l"); break;
		case 14: strcat(output, "trb"); break;
		case 15: strcat(output, "mem"); break;
		}
	}

	if(dpl) {
		switch(dpl) {
		case 0: strcat(output, " | dpnop"); break;
		case 1: strcat(output, " | dpinc"); break;
		case 2: strcat(output, " | dpdec"); break;
		case 3: strcat(output, " | dpclr"); break;
		}
	}

	if(dphm) {
		switch(dphm) {
		case  0: strcat(output, " | m0"); break;
		case  1: strcat(output, " | m1"); break;
		case  2: strcat(output, " | m2"); break;
		case  3: strcat(output, " | m3"); break;
		case  4: strcat(output, " | m4"); break;
		case  5: strcat(output, " | m5"); break;
		case  6: strcat(output, " | m6"); break;
		case  7: strcat(output, " | m7"); break;
		case  8: strcat(output, " | m8"); break;
		case  9: strcat(output, " | m9"); break;
		case 10: strcat(output, " | ma"); break;
		case 11: strcat(output, " | mb"); break;
		case 12: strcat(output, " | mc"); break;
		case 13: strcat(output, " | md"); break;
		case 14: strcat(output, " | me"); break;
		case 15: strcat(output, " | mf"); break;
		}
	}

	if(rpdcr == 1) {
		strcat(output, " | rpdec");
	}

	if(type == 1) {
		strcat(output, " | ret");
	}
	}

	if(type == 2) {  //JP
		UINT16 brch = (opcode >> 13) & 0x1ff;  //branch
		UINT16 na  = (opcode >>  2) & 0x7ff;  //next address

	switch(brch) {
		case 0x000: strcat(output, "jmpso "); break;
		case 0x080: strcat(output, "jnca "); break;
		case 0x082: strcat(output, "jca "); break;
		case 0x084: strcat(output, "jncb "); break;
		case 0x086: strcat(output, "jcb "); break;
		case 0x088: strcat(output, "jnza "); break;
		case 0x08a: strcat(output, "jza "); break;
		case 0x08c: strcat(output, "jnzb "); break;
		case 0x08e: strcat(output, "jzb "); break;
		case 0x090: strcat(output, "jnova0 "); break;
		case 0x092: strcat(output, "jova0 "); break;
		case 0x094: strcat(output, "jnovb0 "); break;
		case 0x096: strcat(output, "jovb0 "); break;
		case 0x098: strcat(output, "jnova1 "); break;
		case 0x09a: strcat(output, "jova1 "); break;
		case 0x09c: strcat(output, "jnovb1 "); break;
		case 0x09e: strcat(output, "jovb1 "); break;
		case 0x0a0: strcat(output, "jnsa0 "); break;
		case 0x0a2: strcat(output, "jsa0 "); break;
		case 0x0a4: strcat(output, "jnsb0 "); break;
		case 0x0a6: strcat(output, "jsb0 "); break;
		case 0x0a8: strcat(output, "jnsa1 "); break;
		case 0x0aa: strcat(output, "jsa1 "); break;
		case 0x0ac: strcat(output, "jnsb1 "); break;
		case 0x0ae: strcat(output, "jsb1 "); break;
		case 0x0b0: strcat(output, "jdpl0 "); break;
		case 0x0b1: strcat(output, "jdpln0 "); break;
		case 0x0b2: strcat(output, "jdplf "); break;
		case 0x0b3: strcat(output, "jdplnf "); break;
		case 0x0b4: strcat(output, "jnsiak "); break;
		case 0x0b6: strcat(output, "jsiak "); break;
		case 0x0b8: strcat(output, "jnsoak "); break;
		case 0x0ba: strcat(output, "jsoak "); break;
		case 0x0bc: strcat(output, "jnrqm "); break;
		case 0x0be: strcat(output, "jrqm "); break;
		case 0x100: strcat(output, "ljmp "); break;
		case 0x101: strcat(output, "hjmp "); break;
		case 0x140: strcat(output, "lcall "); break;
		case 0x141: strcat(output, "hcall "); break;
		default:    strcat(output, "??????  "); break;
	}

	char temp[16];

	sprintf(temp, "$%x", na);
	strcat(output, temp);
	}

	if(type == 3) {  //LD
	strcat(output, "ld ");
	UINT16 id = opcode >> 6;
	UINT8 dst = (opcode >> 0) & 0xf;  //destination

	char temp[16];
	sprintf(temp, "$%x,", id);
	strcat(output, temp);

	switch(dst) {
		case  0: strcat(output, "non"); break;
		case  1: strcat(output, "a"); break;
		case  2: strcat(output, "b"); break;
		case  3: strcat(output, "tr"); break;
		case  4: strcat(output, "dp"); break;
		case  5: strcat(output, "rp"); break;
		case  6: strcat(output, "dr"); break;
		case  7: strcat(output, "sr"); break;
		case  8: strcat(output, "sol"); break;
		case  9: strcat(output, "som"); break;
		case 10: strcat(output, "k"); break;
		case 11: strcat(output, "klr"); break;
		case 12: strcat(output, "klm"); break;
		case 13: strcat(output, "l"); break;
		case 14: strcat(output, "trb"); break;
		case 15: strcat(output, "mem"); break;
	}
	}

	strcpy(buffer, output);

	return 1 | DASMFLAG_SUPPORTED;
}
