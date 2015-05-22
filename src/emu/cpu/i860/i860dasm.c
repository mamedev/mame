// license:BSD-3-Clause
// copyright-holders:Jason Eckhardt
#include "i860.h"

/* Sub-group decoders */
static void i860_dasm_core_dasm(const UINT32 op, char* buffer);
static void i860_dasm_floating_point_dasm(const UINT32 op, char* buffer);
static void i860_dasm_CTRL_dasm(const UINT32 op, char* buffer);

/* REG-Format Opcodes*/
static void i860_dasm_ldx(const UINT32 op, char* buffer);
static void i860_dasm_stx(const UINT32 op, char* buffer);
static void i860_dasm_ixfr(const UINT32 op, char* buffer);
static void i860_dasm_fid_fst(const UINT32 op, char* buffer);
static void i860_dasm_flush(const UINT32 op, char* buffer);
static void i860_dasm_pstd(const UINT32 op, char* buffer);
static void i860_dasm_ldc_sdc(const UINT32 op, char* buffer);
static void i860_dasm_bri(const UINT32 op, char* buffer);
static void i860_dasm_trap(const UINT32 op, char* buffer);
static void i860_dasm_bte_btne(const UINT32 op, char* buffer);
static void i860_dasm_pfidy(const UINT32 op, char* buffer);
static void i860_dasm_addu_subu(const UINT32 op, char* buffer);
static void i860_dasm_shl_shr(const UINT32 op, char* buffer);
static void i860_dasm_shrd(const UINT32 op, char* buffer);
static void i860_dasm_bla(const UINT32 op, char* buffer);
static void i860_dasm_shra(const UINT32 op, char* buffer);
static void i860_dasm_and_andh(const UINT32 op, char* buffer);
static void i860_dasm_andnot_andnoth(const UINT32 op, char* buffer);
static void i860_dasm_or_orh(const UINT32 op, char* buffer);
static void i860_dasm_xor_xorh(const UINT32 op, char* buffer);

/* CORE Escape Opcodes */
static void i860_dasm_CORE_lock(const UINT32 op, char* buffer);
static void i860_dasm_CORE_calli(const UINT32 op, char* buffer);
static void i860_dasm_CORE_intovr(const UINT32 op, char* buffer);
static void i860_dasm_CORE_unlock(const UINT32 op, char* buffer);

/* CTRL-Format Opcodes */
static void i860_dasm_CTRL_br(const UINT32 op, char* buffer);
static void i860_dasm_CTRL_call(const UINT32 op, char* buffer);
static void i860_dasm_CTRL_bc_bct(const UINT32 op, char* buffer);
static void i860_dasm_CTRL_bnc_bnct(const UINT32 op, char* buffer);

/* Floating-Point Instructions */


CPU_DISASSEMBLE( i860 )
{
	char tempB[1024] = "";

	/* Little Endian */
	const UINT32 op = (oprom[3] << 24) | (oprom[2] << 16) | (oprom[1] << 8) | (oprom[0] << 0);
	//const UINT32 op = (oprom[2] << 24) | (oprom[3] << 16) | (oprom[0] << 8) | (oprom[1] << 0);    /* Mixed Endian */
	//const UINT32 op = (oprom[0] << 24) | (oprom[1] << 16) | (oprom[2] << 8) | (oprom[3] << 0);    /* Big Endian */
	//const UINT32 op = (oprom[1] << 24) | (oprom[0] << 16) | (oprom[3] << 8) | (oprom[2] << 0);    /* Mixed Endian */

	/* The opcode is the top 6 bits */
	UINT8 opcode = (op >> 26) & 0x3f;

	/* DEBUG - print this out if you feel things are going a bit wonky */
	// sprintf(buffer, "%08x : oo %02x", op, opcode);

	/* Main decode */
	switch (opcode)
	{
		case 0x00:
		case 0x01:
		case 0x04:
		case 0x05: i860_dasm_ldx(op, tempB);        break;

		case 0x03:
		case 0x07: i860_dasm_stx(op, tempB);        break;

		case 0x02: i860_dasm_ixfr(op, tempB);       break;

		case 0x06: sprintf(tempB, "(reserved)");    break;

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b: i860_dasm_fid_fst(op, tempB);    break;

		case 0x0d: i860_dasm_flush(op, tempB);      break;

		case 0x0f: i860_dasm_pstd(op, tempB);       break;

		case 0x0c:
		case 0x0e: i860_dasm_ldc_sdc(op, tempB);    break;

		case 0x10: i860_dasm_bri(op, tempB);        break;

		case 0x11: i860_dasm_trap(op, tempB);       break;

		case 0x12: i860_dasm_floating_point_dasm(op, tempB); break; /* Floating point operation sub-group */

		case 0x13: i860_dasm_core_dasm(op, tempB);   break;         /* Core operation sub-group */

		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17: i860_dasm_bte_btne(op, tempB);   break;

		case 0x18:
		case 0x19: i860_dasm_pfidy(op, tempB);      break;

		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f: i860_dasm_CTRL_dasm(op, tempB);  break;          /* CTRL operation sub-group */

		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27: i860_dasm_addu_subu(op, tempB);  break;

		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b: i860_dasm_shl_shr(op, tempB);    break;

		case 0x2c: i860_dasm_shrd(op, tempB);       break;

		case 0x2d: i860_dasm_bla(op, tempB);        break;

		case 0x2e:
		case 0x2f: i860_dasm_shra(op, tempB);       break;

		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33: i860_dasm_and_andh(op, tempB);   break;

		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37: i860_dasm_andnot_andnoth(op, tempB); break;

		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b: i860_dasm_or_orh(op, tempB);     break;

		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f: i860_dasm_xor_xorh(op, tempB);   break;

		default: sprintf(tempB, "(reserved)");      break;
	}

	/* More Debug */
	//strcat(buffer, " : ");
	//strcat(buffer, tempB);
	sprintf(buffer, "%s", tempB);

	/* All opcodes are 32 bits */
	return (4 | DASMFLAG_SUPPORTED);
}


// BIT HELPER
// 31   27   23   19   15   11   7    3
// 0000 0011 1111 1111 0000 0111 1110 0000


/**********************/
/* Sub-group decoders */
/**********************/
static void i860_dasm_core_dasm(const UINT32 op, char* buffer)
{
	//UINT8 src1 = (op >> 11) & 0x0000001f;

	/* Reserved bits must be set to 0 */
	if ( (op & 0x000007e0) || (op & 0x03ff0000) )
	{
		//logerror("[i860] Reserved CORE bits must be set to 0.");
		printf("CORE baddie\n");
	}

	switch(op & 0x0000001f)
	{
		case 0x01: i860_dasm_CORE_lock(op, buffer);   break;
		case 0x02: i860_dasm_CORE_calli(op, buffer);  break;
		case 0x04: i860_dasm_CORE_intovr(op, buffer); break;
		case 0x07: i860_dasm_CORE_unlock(op, buffer); break;

		default: sprintf(buffer, "(reserved)");       break;
	}
}

static void i860_dasm_floating_point_dasm(const UINT32 op, char* buffer)
{
	sprintf(buffer, "[[F-P unit]]");
}

static void i860_dasm_CTRL_dasm(const UINT32 op, char* buffer)
{
	UINT8 opc = (op >> 26) & 0x07;

	switch(opc)
	{
		case 0x02:            i860_dasm_CTRL_br(op, buffer);       break;
		case 0x03:            i860_dasm_CTRL_call(op, buffer);     break;
		case 0x04: case 0x05: i860_dasm_CTRL_bc_bct(op, buffer);   break;
		case 0x06: case 0x07: i860_dasm_CTRL_bnc_bnct(op, buffer); break;

		default: sprintf(buffer, "(reserved)");                    break;
	}
}


/*********************/
/* REG-Format Opcodes*/
/*********************/
static void i860_dasm_ldx(const UINT32 op, char* buffer)
{
	sprintf(buffer, "ldx");
}

static void i860_dasm_stx(const UINT32 op, char* buffer)
{
	sprintf(buffer, "stx");
}

static void i860_dasm_ixfr(const UINT32 op, char* buffer)
{
//  UINT16 val = op & 0x7ff;
//  UINT8  opc = (op >> 26) & 0x3f;
//  UINT8 src2 = (op >> 21) & 0x1f;
//  UINT8 dest = (op >> 16) & 0x1f;
//  UINT8 src1 = (op >> 11) & 0x1f;

	sprintf(buffer, "ixfr");
}

static void i860_dasm_fid_fst(const UINT32 op, char* buffer)
{
	sprintf(buffer, "fst");
}

static void i860_dasm_flush(const UINT32 op, char* buffer)
{
	sprintf(buffer, "flush");
}

static void i860_dasm_pstd(const UINT32 op, char* buffer)
{
	sprintf(buffer, "pstd");
}

static void i860_dasm_ldc_sdc(const UINT32 op, char* buffer)
{
	sprintf(buffer, "ldc, sdc");
}

static void i860_dasm_bri(const UINT32 op, char* buffer)
{
	sprintf(buffer, "bri");
}

static void i860_dasm_trap(const UINT32 op, char* buffer)
{
	sprintf(buffer, "trap");
}

static void i860_dasm_bte_btne(const UINT32 op, char* buffer)
{
	sprintf(buffer, "bte, btne");
}

static void i860_dasm_pfidy(const UINT32 op, char* buffer)
{
	sprintf(buffer, "pfidy");
}

static void i860_dasm_addu_subu(const UINT32 op, char* buffer)
{
	sprintf(buffer, "addu, subu");
}

static void i860_dasm_shl_shr(const UINT32 op, char* buffer)
{
	sprintf(buffer, "shl, shr");
}

static void i860_dasm_shrd(const UINT32 op, char* buffer)
{
	sprintf(buffer, "shrd");
}

static void i860_dasm_bla(const UINT32 op, char* buffer)
{
	sprintf(buffer, "bla");
}

static void i860_dasm_shra(const UINT32 op, char* buffer)
{
	sprintf(buffer, "shra");
}

static void i860_dasm_and_andh(const UINT32 op, char* buffer)
{
	sprintf(buffer, "and, andh");
}

static void i860_dasm_andnot_andnoth(const UINT32 op, char* buffer)
{
	sprintf(buffer, "andnot, andnoth");
}

static void i860_dasm_or_orh(const UINT32 op, char* buffer)
{
	sprintf(buffer, "or, orh");
}

static void i860_dasm_xor_xorh(const UINT32 op, char* buffer)
{
	sprintf(buffer, "xor, xorh");
}


/***********************/
/* CORE Escape Opcodes */
/***********************/
static void i860_dasm_CORE_lock(const UINT32 op, char* buffer)
{
	sprintf(buffer, "lock");
}

static void i860_dasm_CORE_calli(const UINT32 op, char* buffer)
{
	sprintf(buffer, "calli");
}

static void i860_dasm_CORE_intovr(const UINT32 op, char* buffer)
{
	sprintf(buffer, "intovr");
}

static void i860_dasm_CORE_unlock(const UINT32 op, char* buffer)
{
	sprintf(buffer, "unlock");
}


/***********************/
/* CTRL-Format Opcodes */
/***********************/
static void i860_dasm_CTRL_br(const UINT32 op, char* buffer)
{
	sprintf(buffer, "br");
}

static void i860_dasm_CTRL_call(const UINT32 op, char* buffer)
{
	sprintf(buffer, "call");
}

static void i860_dasm_CTRL_bc_bct(const UINT32 op, char* buffer)
{
	sprintf(buffer, "bct");
}

static void i860_dasm_CTRL_bnc_bnct(const UINT32 op, char* buffer)
{
	sprintf(buffer, "bnct");
}


/*******************************/
/* Floating-Point Instructions */
/*******************************/
