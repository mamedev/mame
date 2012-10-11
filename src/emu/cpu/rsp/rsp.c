/*
    Nintendo/SGI Reality Signal Processor (RSP) emulator

    Written by Ville Linde
*/

#include "emu.h"
#include "debugger.h"
#include "rsp.h"
#include "rspdiv.h"

CPU_DISASSEMBLE( rsp );

#ifndef USE_RSPDRC

#define LOG_INSTRUCTION_EXECUTION		0
#define SAVE_DISASM						0
#define SAVE_DMEM						0
#define RSP_TEST_SYNC                   0

#define PRINT_VECREG(x)		mame_printf_debug("V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X\n", (x), \
							(UINT16)VREG_S((x),0), (UINT16)VREG_S((x),1), \
							(UINT16)VREG_S((x),2), (UINT16)VREG_S((x),3), \
							(UINT16)VREG_S((x),4), (UINT16)VREG_S((x),5), \
							(UINT16)VREG_S((x),6), (UINT16)VREG_S((x),7))

#define PRINT_ACCUM(x)     mame_printf_debug("A%d: %08X|%08X\n", (x), \
                            (UINT32)( ( ACCUM(x) >> 32 ) & 0x00000000ffffffff ),    \
                            (UINT32)(   ACCUM(x)         & 0x00000000ffffffff ))

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

INLINE rsp_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == RSP);
	return (rsp_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define SIMM16		((INT32)(INT16)(op))
#define UIMM16		((UINT16)(op))
#define UIMM26		(op & 0x03ffffff)

#define JUMP_ABS(addr)			{ rsp->nextpc = 0x04001000 | (((addr) << 2) & 0xfff); }
#define JUMP_ABS_L(addr,l)		{ rsp->nextpc = 0x04001000 | (((addr) << 2) & 0xfff); rsp->r[l] = rsp->pc + 4; }
#define JUMP_REL(offset)		{ rsp->nextpc = 0x04001000 | ((rsp->pc + ((offset) << 2)) & 0xfff); }
#define JUMP_REL_L(offset,l)	{ rsp->nextpc = 0x04001000 | ((rsp->pc + ((offset) << 2)) & 0xfff); rsp->r[l] = rsp->pc + 4; }
#define JUMP_PC(addr)			{ rsp->nextpc = 0x04001000 | ((addr) & 0xfff); }
#define JUMP_PC_L(addr,l)		{ rsp->nextpc = 0x04001000 | ((addr) & 0xfff); rsp->r[l] = rsp->pc + 4; }
#define LINK(l) 				{ rsp->r[l] = rsp->pc + 4; }

#define VREG_B(reg, offset)		rsp->v[(reg)].b[(offset)^1]
#define VREG_S(reg, offset)		rsp->v[(reg)].s[(offset)]
#define VREG_L(reg, offset)		rsp->v[(reg)].l[(offset)]

#define R_VREG_B(reg, offset)		rsp->v[(reg)].b[(offset)^1]
#define R_VREG_S(reg, offset)		(INT16)rsp->v[(reg)].s[(offset)]
#define R_VREG_L(reg, offset)		rsp->v[(reg)].l[(offset)]

#define W_VREG_B(reg, offset, val)	(rsp->v[(reg)].b[(offset)^1] = val)
#define W_VREG_S(reg, offset, val)	(rsp->v[(reg)].s[(offset)] = val)
#define W_VREG_L(reg, offset, val)	(rsp->v[(reg)].l[(offset)] = val)

#define VEC_EL_2(x,z)			(vector_elements[(x)][(z)])

#define ACCUM(x)		rsp->accum[((x))].q
#define ACCUM_H(x)		rsp->accum[((x))].w[3]
#define ACCUM_M(x)		rsp->accum[((x))].w[2]
#define ACCUM_L(x)		rsp->accum[((x))].w[1]

#define CARRY_FLAG(x)			((rsp->flag[0] & (1 << ((x)))) ? 1 : 0)
#define CLEAR_CARRY_FLAGS()		{ rsp->flag[0] &= ~0xff; }
#define SET_CARRY_FLAG(x)		{ rsp->flag[0] |= (1 << ((x))); }
#define CLEAR_CARRY_FLAG(x)		{ rsp->flag[0] &= ~(1 << ((x))); }

#define COMPARE_FLAG(x)			((rsp->flag[1] & (1 << ((x)))) ? 1 : 0)
#define CLEAR_COMPARE_FLAGS()	{ rsp->flag[1] &= ~0xff; }
#define SET_COMPARE_FLAG(x)		{ rsp->flag[1] |= (1 << ((x))); }
#define CLEAR_COMPARE_FLAG(x)	{ rsp->flag[1] &= ~(1 << ((x))); }

#define ZERO_FLAG(x)			((rsp->flag[0] & (1 << (8+(x)))) ? 1 : 0)
#define CLEAR_ZERO_FLAGS()		{ rsp->flag[0] &= ~0xff00; }
#define SET_ZERO_FLAG(x)		{ rsp->flag[0] |= (1 << (8+(x))); }
#define CLEAR_ZERO_FLAG(x)		{ rsp->flag[0] &= ~(1 << (8+(x))); }

#define EXTENSION_FLAG(x)		((rsp.flag[2] & (1 << ((x)))) ? 1 : 0)

#define ROPCODE(pc)		rsp->program->read_dword(pc)

INLINE UINT8 READ8(rsp_state *rsp, UINT32 address)
{
	UINT8 ret;
	address = 0x04000000 | (address & 0xfff);
	ret = rsp->program->read_byte(address);
	printf("%04xr%02x\n", address & 0x0000ffff, ret);
	return ret;
}

INLINE UINT16 READ16(rsp_state *rsp, UINT32 address)
{
	UINT16 ret;
	address = 0x04000000 | (address & 0xfff);

	if(address & 1)
	{
		ret = ((rsp->program->read_byte(address + 0) & 0xff) << 8) | (rsp->program->read_byte(address + 1) & 0xff);
	}
	else
	{
		ret = rsp->program->read_word(address);
	}

	printf("%04xr%04x\n", address & 0x0000ffff, ret);

	return ret;
}

INLINE UINT32 READ32(rsp_state *rsp, UINT32 address)
{
	UINT32 ret;
	address = 0x04000000 | (address & 0xfff);

	if(address & 3)
	{
		ret =  ((rsp->program->read_byte(address + 0) & 0xff) << 24) |
			   ((rsp->program->read_byte(address + 1) & 0xff) << 16) |
			   ((rsp->program->read_byte(address + 2) & 0xff) << 8) |
			   ((rsp->program->read_byte(address + 3) & 0xff) << 0);
	}
	else
	{
		ret = rsp->program->read_dword(address);
	}

	printf("%04xr%08x\n", address & 0x0000ffff, ret);
	return ret;
}

INLINE void WRITE8(rsp_state *rsp, UINT32 address, UINT8 data)
{
	address = 0x04000000 | (address & 0xfff);
	printf("%04x:%02x\n", address & 0x0000ffff, data);
	rsp->program->write_byte(address, data);
}

INLINE void WRITE16(rsp_state *rsp, UINT32 address, UINT16 data)
{
	address = 0x04000000 | (address & 0xfff);
	printf("%04x:%04x\n", address & 0x0000ffff, data);

	if(address & 1)
	{
		rsp->program->write_byte(address + 0, (data >> 8) & 0xff);
		rsp->program->write_byte(address + 1, (data >> 0) & 0xff);
		return;
	}

	rsp->program->write_word(address, data);
}

INLINE void WRITE32(rsp_state *rsp, UINT32 address, UINT32 data)
{
	address = 0x04000000 | (address & 0xfff);
	printf("%04x:%08x\n", address & 0x0000ffff, data);

	if(address & 3)
	{
		rsp->program->write_byte(address + 0, (data >> 24) & 0xff);
		rsp->program->write_byte(address + 1, (data >> 16) & 0xff);
		rsp->program->write_byte(address + 2, (data >> 8) & 0xff);
		rsp->program->write_byte(address + 3, (data >> 0) & 0xff);
		return;
	}

	rsp->program->write_dword(address, data);
}

/*****************************************************************************/

static UINT32 get_cop0_reg(rsp_state *rsp, int reg)
{
	reg &= 0xf;
	if (reg < 8)
	{
		return (rsp->sp_reg_r_func)(reg, 0x00000000);
	}
	else if (reg >= 8 && reg < 16)
	{
		return (rsp->dp_reg_r_func)(reg - 8, 0x00000000);
	}

	return 0;
}

static void set_cop0_reg(rsp_state *rsp, int reg, UINT32 data)
{
	reg &= 0xf;
	if (reg < 8)
	{
		(rsp->sp_reg_w_func)(reg, data, 0x00000000);
	}
	else if (reg >= 8 && reg < 16)
	{
		(rsp->dp_reg_w_func)(reg - 8, data, 0x00000000);
	}
}

static void unimplemented_opcode(rsp_state *rsp, UINT32 op)
{
	if ((rsp->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		char string[200];
		rsp_dasm_one(string, rsp->ppc, op);
		mame_printf_debug("%08X: %s\n", rsp->ppc, string);
	}

#if SAVE_DISASM
	{
		char string[200];
		int i;
		FILE *dasm;
		dasm = fopen("rsp_disasm.txt", "wt");

		for (i=0; i < 0x1000; i+=4)
		{
			UINT32 opcode = ROPCODE(0x04001000 + i);
			rsp_dasm_one(string, 0x04001000 + i, opcode);
			fprintf(dasm, "%08X: %08X   %s\n", 0x04001000 + i, opcode, string);
		}
		fclose(dasm);
	}
#endif
#if SAVE_DMEM
	{
		int i;
		FILE *dmem;
		dmem = fopen("rsp_dmem.bin", "wb");

		for (i=0; i < 0x1000; i++)
		{
			fputc(READ8(rsp, 0x04000000 + i), dmem);
		}
		fclose(dmem);
	}
#endif

	fatalerror("RSP: unknown opcode %02X (%08X) at %08X\n", op >> 26, op, rsp->ppc);
}

/*****************************************************************************/

static const int vector_elements[16][8] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7 },		// none
	{ 0, 1, 2, 3, 4, 5, 6, 7 },		// ???
	{ 0, 0, 2, 2, 4, 4, 6, 6 },		// 0q
	{ 1, 1, 3, 3, 5, 5, 7, 7 },		// 1q
	{ 0, 0, 0, 0, 4, 4, 4, 4 },		// 0h
	{ 1, 1, 1, 1, 5, 5, 5, 5 },		// 1h
	{ 2, 2, 2, 2, 6, 6, 6, 6 },		// 2h
	{ 3, 3, 3, 3, 7, 7, 7, 7 },		// 3h
	{ 0, 0, 0, 0, 0, 0, 0, 0 },		// 0
	{ 1, 1, 1, 1, 1, 1, 1, 1 },		// 1
	{ 2, 2, 2, 2, 2, 2, 2, 2 },		// 2
	{ 3, 3, 3, 3, 3, 3, 3, 3 },		// 3
	{ 4, 4, 4, 4, 4, 4, 4, 4 },		// 4
	{ 5, 5, 5, 5, 5, 5, 5, 5 },		// 5
	{ 6, 6, 6, 6, 6, 6, 6, 6 },		// 6
	{ 7, 7, 7, 7, 7, 7, 7, 7 },		// 7
};

static CPU_INIT( rsp )
{
	rsp_state *rsp = get_safe_token(device);
	int regIdx;
	int accumIdx;
	const rsp_config *config = (const rsp_config *)device->static_config();
	// resolve callbacks
	rsp->dp_reg_r_func.resolve(config->dp_reg_r_cb, *device);
	rsp->dp_reg_w_func.resolve(config->dp_reg_w_cb, *device);
	rsp->sp_reg_r_func.resolve(config->sp_reg_r_cb, *device);
	rsp->sp_reg_w_func.resolve(config->sp_reg_w_cb, *device);
	rsp->sp_set_status_func.resolve(config->sp_set_status_cb, *device);

	if (LOG_INSTRUCTION_EXECUTION)
		rsp->exec_output = fopen("rsp_execute.txt", "wt");

	rsp->irq_callback = irqcallback;
	rsp->device = device;
	rsp->program = &device->space(AS_PROGRAM);
	rsp->direct = &rsp->program->direct();

#if 1
	// Inaccurate.  RSP registers power on to a random state...
	for(regIdx = 0; regIdx < 32; regIdx++ )
	{
		rsp->r[regIdx] = 0;
		rsp->v[regIdx].d[0] = 0;
		rsp->v[regIdx].d[1] = 0;
	}
	rsp->flag[0] = 0;
	rsp->flag[1] = 0;
	rsp->flag[2] = 0;
	rsp->flag[3] = 0;
	rsp->square_root_res = 0;
	rsp->square_root_high = 0;
	rsp->reciprocal_res = 0;
	rsp->reciprocal_high = 0;
#endif

	// ...except for the accumulators.
	// We're not calling machine.rand() because initializing something with machine.rand()
	//   makes me retch uncontrollably.
	for(accumIdx = 0; accumIdx < 8; accumIdx++ )
	{
		rsp->accum[accumIdx].q = 0;
	}

	rsp->sr = RSP_STATUS_HALT;
	rsp->step_count = 0;
}

static CPU_EXIT( rsp )
{
	rsp_state *rsp = get_safe_token(device);

#if SAVE_DISASM
	{
		char string[200];
		int i;
		FILE *dasm;
		dasm = fopen("rsp_disasm.txt", "wt");

		for (i=0; i < 0x1000; i+=4)
		{
			UINT32 opcode = ROPCODE(0x04001000 + i);
			rsp_dasm_one(string, 0x04001000 + i, opcode);
			fprintf(dasm, "%08X: %08X   %s\n", 0x04001000 + i, opcode, string);
		}
		fclose(dasm);
	}
#endif
#if SAVE_DMEM
	{
		int i;
		FILE *dmem;
#if 0
		dmem = fopen("rsp_dmem.txt", "wt");

		for (i=0; i < 0x1000; i+=4)
		{
			fprintf(dmem, "%08X: %08X\n", 0x04000000 + i, READ32(rsp, 0x04000000 + i));
		}
		fclose(dmem);
#endif
		dmem = fopen("rsp_dmem.bin", "wb");

		for (i=0; i < 0x1000; i++)
		{
			fputc(READ8(rsp, 0x04000000 + i), dmem);
		}
		fclose(dmem);
	}
#endif

	if (rsp->exec_output)
		fclose(rsp->exec_output);
	rsp->exec_output = NULL;
}

static CPU_RESET( rsp )
{
	rsp_state *rsp = get_safe_token(device);
	rsp->nextpc = ~0;
}

static void handle_lwc2(rsp_state *rsp, UINT32 op)
{
	int i, end;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:		/* LBV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Load 1 byte to vector byte index

			ea = (base) ? rsp->r[base] + offset : offset;
			VREG_B(dest, index) = READ8(rsp, ea);
			break;
		}
		case 0x01:		/* LSV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 2 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(rsp, ea);
				ea++;
			}
			break;
		}
		case 0x02:		/* LLV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 4 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(rsp, ea);
				ea++;
			}
			break;
		}
		case 0x03:		/* LDV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 8 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(rsp, ea);
				ea++;
			}
			break;
		}
		case 0x04:		/* LQV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00100 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads up to 16 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));
			if (end > 16) end = 16;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(rsp, ea);
				ea++;
			}
			break;
		}
		case 0x05:		/* LRV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00101 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from right side until 16-byte boundary

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			index = 16 - ((ea & 0xf) - index);
			end = 16;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(rsp, ea);
				ea++;
			}
			break;
		}
		case 0x06:		/* LPV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00110 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the upper 8 bits of each element

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = READ8(rsp, ea + (((16-index) + i) & 0xf)) << 8;
			}
			break;
		}
		case 0x07:		/* LUV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00111 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of each element

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = READ8(rsp, ea + (((16-index) + i) & 0xf)) << 7;
			}
			break;
		}
		case 0x08:		/* LHV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of each element, with 2-byte stride

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = READ8(rsp, ea + (((16-index) + (i<<1)) & 0xf)) << 7;
			}
			break;
		}
		case 0x09:		/* LFV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of upper or lower quad, with 4-byte stride

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			// not sure what happens if 16-byte boundary is crossed...

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				VREG_S(dest, i) = READ8(rsp, ea) << 7;
				ea += 4;
			}
			break;
		}
		case 0x0a:		/* LWV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			// not sure what happens if 16-byte boundary is crossed...
			if ((ea & 0xf) > 0) fatalerror("RSP: LWV: 16-byte boundary crossing at %08X, recheck this!\n", rsp->ppc);

			end = (16 - index) + 16;

			for (i=(16 - index); i < end; i++)
			{
				VREG_B(dest, i & 0xf) = READ8(rsp, ea);
				ea += 4;
			}
			break;
		}
		case 0x0b:		/* LTV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads one element to maximum of 8 vectors, while incrementing element index

			// FIXME: has a small problem with odd indices

			int element;
			int vs = dest;
			int ve = dest + 8;
			if (ve > 32)
				ve = 32;

			element = 7 - (index >> 1);

			if (index & 1)	fatalerror("RSP: LTV: index = %d\n", index);

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			ea = ((ea + 8) & ~0xf) + (index & 1);
			for (i=vs; i < ve; i++)
			{
				element = ((8 - (index >> 1) + (i-vs)) << 1);
				VREG_B(i, (element & 0xf)) = READ8(rsp, ea);
				VREG_B(i, ((element + 1) & 0xf)) = READ8(rsp, ea + 1);

				ea += 2;
			}
			break;
		}

		default:
		{
			unimplemented_opcode(rsp, op);
			break;
		}
	}
}

static void handle_swc2(rsp_state *rsp, UINT32 op)
{
	int i, end;
	int eaoffset;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:		/* SBV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 1 byte from vector byte index

			ea = (base) ? rsp->r[base] + offset : offset;
			WRITE8(rsp, ea, VREG_B(dest, index));
			break;
		}
		case 0x01:		/* SSV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 2 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea, VREG_B(dest, i));
				ea++;
			}
			break;
		}
		case 0x02:		/* SLV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 4 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea, VREG_B(dest, i));
				ea++;
			}
			break;
		}
		case 0x03:		/* SDV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 8 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea, VREG_B(dest, i));
				ea++;
			}
			break;
		}
		case 0x04:		/* SQV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00100 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from vector byte index until 16-byte boundary

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea, VREG_B(dest, i & 0xf));
				ea++;
			}
			break;
		}
		case 0x05:		/* SRV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00101 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from right side until 16-byte boundary

			int o;
			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			end = index + (ea & 0xf);
			o = (16 - (ea & 0xf)) & 0xf;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea, VREG_B(dest, ((i + o) & 0xf)));
				ea++;
			}
			break;
		}
		case 0x06:		/* SPV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00110 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores upper 8 bits of each element

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					WRITE8(rsp, ea, VREG_B(dest, ((i & 0xf) << 1)));
				}
				else
				{
					WRITE8(rsp, ea, VREG_S(dest, (i & 0x7)) >> 7);
				}
				ea++;
			}
			break;
		}
		case 0x07:		/* SUV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00111 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					WRITE8(rsp, ea, VREG_S(dest, (i & 0x7)) >> 7);
				}
				else
				{
					WRITE8(rsp, ea, VREG_B(dest, ((i & 0x7) << 1)));
				}
				ea++;
			}
			break;
		}
		case 0x08:		/* SHV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element, with 2-byte stride

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				UINT8 d = ((VREG_B(dest, ((index + (i << 1) + 0) & 0xf))) << 1) |
						  ((VREG_B(dest, ((index + (i << 1) + 1) & 0xf))) >> 7);

				WRITE8(rsp, ea, d);
				ea += 2;
			}
			break;
		}
		case 0x09:		/* SFV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of upper or lower quad, with 4-byte stride

			// FIXME: only works for index 0 and index 8

			if (index & 0x7)	mame_printf_debug("RSP: SFV: index = %d at %08X\n", index, rsp->ppc);

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				WRITE8(rsp, ea + (eaoffset & 0xf), VREG_S(dest, i) >> 7);
				eaoffset += 4;
			}
			break;
		}
		case 0x0a:		/* SWV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = index + 16;

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
				eaoffset++;
			}
			break;
		}
		case 0x0b:		/* STV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores one element from maximum of 8 vectors, while incrementing element index

			int element;
			int vs = dest;
			int ve = dest + 8;
			if (ve > 32)
				ve = 32;

			element = 8 - (index >> 1);

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			eaoffset = (ea & 0xf) + (element * 2);
			ea &= ~0xf;

			for (i=vs; i < ve; i++)
			{
				WRITE16(rsp, ea + (eaoffset & 0xf), VREG_S(i, element & 0x7));
				eaoffset += 2;
				element++;
			}
			break;
		}

		default:
		{
			unimplemented_opcode(rsp, op);
			break;
		}
	}
}

INLINE UINT16 SATURATE_ACCUM(rsp_state *rsp, int accum, int slice, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else if (slice == 1)
				{
					return ACCUM_M(accum);
				}
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else
				{
					return ACCUM_M(accum);
				}
			}
		}
	}

	return 0;
}

INLINE UINT16 SATURATE_ACCUM1(rsp_state *rsp, int accum, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
        		return ACCUM_M(accum);
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
        		return ACCUM_M(accum);
			}
		}
	}

	return 0;
}

#define WRITEBACK_RESULT() {memcpy(&rsp->v[VDREG].s[0], &vres[0], 16);}

#if 0
static float float_round(float input)
{
	INT32 integer = (INT32)input;
	float fraction = input - (float)integer;
	float output = 0.0f;
	if( fraction >= 0.5f )
	{
		output = (float)( integer + 1 );
	}
	else
	{
		output = (float)integer;
	}
	return output;
}
#endif

static void handle_vector_ops(rsp_state *rsp, UINT32 op)
{
	int i;
	UINT32 VS1REG = (op >> 11) & 0x1f;
	UINT32 VS2REG = (op >> 16) & 0x1f;
	UINT32 VDREG = (op >> 6) & 0x1f;
	UINT32 EL = (op >> 21) & 0xf;
	INT16 vres[8];

	// Opcode legend:
	//    E = VS2 element type
	//    S = VS1, Source vector 1
	//    T = VS2, Source vector 2
	//    D = Destination vector

	switch (op & 0x3f)
	{
		case 0x00:		/* VMULF */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000000 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer * 2

			int sel;
			INT32 s1, s2;
			INT64 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				if (s1 == -32768 && s2 == -32768)
				{
					// overflow
					ACCUM_H(i) = 0;
					ACCUM_M(i) = -32768;
					ACCUM_L(i) = -32768;
					vres[i] = 0x7fff;
				}
				else
				{
					r =  s1 * s2 * 2;
					r += 0x8000;	// rounding ?
					ACCUM_H(i) = (r < 0) ? 0xffff : 0;		// sign-extend to 48-bit
					ACCUM_M(i) = (INT16)(r >> 16);
					ACCUM_L(i) = (UINT16)(r);
					vres[i] = ACCUM_M(i);
				}
			}
			WRITEBACK_RESULT();

			break;
		}

		case 0x01:		/* VMULU */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000001 |
			// ------------------------------------------------------
			//

			int sel;
			INT32 s1, s2;
			INT64 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2 * 2;
				r += 0x8000;	// rounding ?

				ACCUM_H(i) = (UINT16)(r >> 32);
				ACCUM_M(i) = (UINT16)(r >> 16);
				ACCUM_L(i) = (UINT16)(r);

				if (r < 0)
				{
					vres[i] = 0;
				}
				else if (((INT16)(ACCUM_H(i)) ^ (INT16)(ACCUM_M(i))) < 0)
				{
					vres[i] = -1;
				}
				else
				{
					vres[i] = ACCUM_M(i);
				}
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x04:		/* VMUDL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000100 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by unsigned fraction
			// Stores the higher 16 bits of the 32-bit result to accumulator
			// The low slice of accumulator is stored into destination element

			int sel;
			UINT32 s1, s2;
			UINT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM_H(i) = 0;
				ACCUM_M(i) = 0;
				ACCUM_L(i) = (UINT16)(r >> 16);

				vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x05:		/* VMUDM */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000101 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by unsigned fraction
			// The result is stored into accumulator
			// The middle slice of accumulator is stored into destination element

			int sel;
			INT32 s1, s2;
			INT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (UINT16)VREG_S(VS2REG, sel);	// not sign-extended
				r =  s1 * s2;

				ACCUM_H(i) = (r < 0) ? 0xffff : 0;		// sign-extend to 48-bit
				ACCUM_M(i) = (INT16)(r >> 16);
				ACCUM_L(i) = (UINT16)(r);

				vres[i] = ACCUM_M(i);
			}
			WRITEBACK_RESULT();
			break;

		}

		case 0x06:		/* VMUDN */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000110 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by signed integer
			// The result is stored into accumulator
			// The low slice of accumulator is stored into destination element

			int sel;
			INT32 s1, s2;
			INT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT16)VREG_S(VS1REG, i);		// not sign-extended
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM_H(i) = (r < 0) ? 0xffff : 0;		// sign-extend to 48-bit
				ACCUM_M(i) = (INT16)(r >> 16);
				ACCUM_L(i) = (UINT16)(r);

				vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x07:		/* VMUDH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000111 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer
			// The result is stored into highest 32 bits of accumulator, the low slice is zero
			// The highest 32 bits of accumulator is saturated into destination element

			int sel;
			INT32 s1, s2;
			INT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM_H(i) = (INT16)(r >> 16);
				ACCUM_M(i) = (UINT16)(r);
				ACCUM_L(i) = 0;

				if (r < -32768) r = -32768;
				if (r >  32767)	r = 32767;
				vres[i] = (INT16)(r);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x08:		/* VMACF */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001000 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer * 2
			// The result is added to accumulator

			int sel;
			INT32 s1, s2;
			INT32 r;
			UINT16 res;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM(i) += (INT64)(r) << 17;
				res = SATURATE_ACCUM(rsp, i, 1, 0x8000, 0x7fff);

				vres[i] = res;
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x09:		/* VMACU */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001001 |
			// ------------------------------------------------------
			//

			UINT16 res;
			int sel;
			INT32 s1, s2, r1;
			UINT32 r2, r3;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r1 = s1 * s2;
				r2 = (UINT16)ACCUM_L(i) + ((UINT16)(r1) * 2);
				r3 = (UINT16)ACCUM_M(i) + (UINT16)((r1 >> 16) * 2) + (UINT16)(r2 >> 16);

				ACCUM_L(i) = (UINT16)(r2);
				ACCUM_M(i) = (UINT16)(r3);
				ACCUM_H(i) += (UINT16)(r3 >> 16) + (UINT16)(r1 >> 31);

				//res = SATURATE_ACCUM(i, 1, 0x0000, 0xffff);
				if ((INT16)ACCUM_H(i) < 0)
				{
					res = 0;
				}
				else
				{
					if (ACCUM_H(i) != 0)
					{
						res = 0xffff;
					}
					else
					{
						if ((INT16)ACCUM_M(i) < 0)
						{
							res = 0xffff;
						}
						else
						{
							res = ACCUM_M(i);
						}
					}
				}

				vres[i] = res;
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x0c:		/* VMADL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001100 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by unsigned fraction
			// Adds the higher 16 bits of the 32-bit result to accumulator
			// The low slice of accumulator is stored into destination element

			UINT16 res;
			int sel;
			UINT32 s1, s2, r1;
			UINT32 r2, r3;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r1 = s1 * s2;
				r2 = (UINT16)ACCUM_L(i) + (r1 >> 16);
				r3 = (UINT16)ACCUM_M(i) + (r2 >> 16);

				ACCUM_L(i) = (UINT16)(r2);
				ACCUM_M(i) = (UINT16)(r3);
				ACCUM_H(i) += (INT16)(r3 >> 16);

				res = SATURATE_ACCUM(rsp, i, 0, 0x0000, 0xffff);

				vres[i] = res;
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x0d:		/* VMADM */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by unsigned fraction
			// The result is added into accumulator
			// The middle slice of accumulator is stored into destination element

			UINT16 res;
			int sel;
			UINT32 s1, s2, r1;
			UINT32 r2, r3;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (UINT16)VREG_S(VS2REG, sel);	// not sign-extended
				r1 = s1 * s2;
				r2 = (UINT16)ACCUM_L(i) + (UINT16)(r1);
				r3 = (UINT16)ACCUM_M(i) + (r1 >> 16) + (r2 >> 16);

				ACCUM_L(i) = (UINT16)(r2);
				ACCUM_M(i) = (UINT16)(r3);
				ACCUM_H(i) += (UINT16)(r3 >> 16);
				if ((INT32)(r1) < 0)
					ACCUM_H(i) -= 1;

				res = SATURATE_ACCUM(rsp, i, 1, 0x8000, 0x7fff);

				vres[i] = res;
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x0e:		/* VMADN */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by signed integer
			// The result is added into accumulator
			// The low slice of accumulator is stored into destination element

			INT32 s1, s2;
			UINT16 res;
			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT16)VREG_S(VS1REG, i);		// not sign-extended
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);

				ACCUM(i) += (INT64)(s1*s2)<<16;

				res = SATURATE_ACCUM(rsp, i, 0, 0x0000, 0xffff);
				vres[i] = res;
			}
			WRITEBACK_RESULT();

			break;
		}

		case 0x0f:		/* VMADH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001111 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer
			// The result is added into highest 32 bits of accumulator, the low slice is zero
			// The highest 32 bits of accumulator is saturated into destination element

			UINT16 res;
			int sel;
			INT32 s1, s2;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);

				rsp->accum[i].l[1] += s1*s2;

				res = SATURATE_ACCUM1(rsp, i, 0x8000, 0x7fff);

				vres[i] = res;
			}
			WRITEBACK_RESULT();

			break;
		}

		case 0x10:		/* VADD */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010000 |
			// ------------------------------------------------------
			//
			// Adds two vector registers and carry flag, the result is saturated to 32767

			// TODO: check VS2REG == VDREG

			int sel;
			INT32 s1, s2, r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 + s2 + CARRY_FLAG(i);

				ACCUM_L(i) = (INT16)(r);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;
				vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x11:		/* VSUB */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010001 |
			// ------------------------------------------------------
			//
			// Subtracts two vector registers and carry flag, the result is saturated to -32768

			// TODO: check VS2REG == VDREG

			int sel;
			INT32 s1, s2, r;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 - s2 - CARRY_FLAG(i);

				ACCUM_L(i) = (INT16)(r);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;

				vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x13:		/* VABS */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010011 |
			// ------------------------------------------------------
			//
			// Changes the sign of source register 2 if source register 1 is negative and stores
			// the result to destination register

			int sel;
			INT16 s1, s2;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT16)VREG_S(VS1REG, i);
				s2 = (INT16)VREG_S(VS2REG, sel);

				if (s1 < 0)
				{
					if (s2 == -32768)
					{
						vres[i] = 32767;
					}
					else
					{
						vres[i] = -s2;
					}
				}
				else if (s1 > 0)
				{
					vres[i] = s2;
				}
				else
				{
					vres[i] = 0;
				}

				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x14:		/* VADDC */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010100 |
			// ------------------------------------------------------
			//
			// Adds two vector registers, the carry out is stored into carry register

			// TODO: check VS2REG = VDREG

			int sel;
			INT32 s1, s2, r;
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r = s1 + s2;

				vres[i] = (INT16)(r);
				ACCUM_L(i) = (INT16)(r);

				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
				}
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x15:		/* VSUBC */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010101 |
			// ------------------------------------------------------
			//
			// Subtracts two vector registers, the carry out is stored into carry register

			// TODO: check VS2REG = VDREG

			int sel;
			INT32 s1, s2, r;
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r = s1 - s2;

				vres[i] = (INT16)(r);
				ACCUM_L(i) = (UINT16)(r);

				if ((UINT16)(r) != 0)
				{
					SET_ZERO_FLAG(i);
				}
				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
				}
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x1d:		/* VSAW */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 011101 |
			// ------------------------------------------------------
			//
			// Stores high, middle or low slice of accumulator to destination vector

			switch (EL)
			{
				case 0x08:		// VSAWH
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_H(i);
					}
					break;
				}
				case 0x09:		// VSAWM
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_M(i);
					}
					break;
				}
				case 0x0a:		// VSAWL
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_L(i);
					}
					break;
				}
				default:	//fatalerror("RSP: VSAW: el = %d\n", EL);//???????
					printf("RSP: VSAW: el = %d\n", EL);//??? ???
					exit(0);
			}
			break;
		}

		case 0x20:		/* VLT */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100000 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are less than VS2
			// Moves the element in VS2 to destination vector

			int sel;
			rsp->flag[1] = 0;

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);

				if (VREG_S(VS1REG, i) < VREG_S(VS2REG, sel))
				{
					SET_COMPARE_FLAG(i);
				}
				else if (VREG_S(VS1REG, i) == VREG_S(VS2REG, sel))
				{
					if (ZERO_FLAG(i) == 1 && CARRY_FLAG(i) != 0)
					{
						SET_COMPARE_FLAG(i);
					}
				}

				if (COMPARE_FLAG(i))
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}

				ACCUM_L(i) = vres[i];
			}

			rsp->flag[0] = 0;
			WRITEBACK_RESULT();
			break;
		}

		case 0x21:		/* VEQ */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100001 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are equal with VS2
			// Moves the element in VS2 to destination vector

			int sel;
			rsp->flag[1] = 0;

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);

				if ((VREG_S(VS1REG, i) == VREG_S(VS2REG, sel)) && ZERO_FLAG(i) == 0)
				{
					SET_COMPARE_FLAG(i);
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}
				ACCUM_L(i) = vres[i];
			}

			rsp->flag[0] = 0;
			WRITEBACK_RESULT();
			break;
		}

		case 0x22:		/* VNE */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100010 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are not equal with VS2
			// Moves the element in VS2 to destination vector

			int sel;
			rsp->flag[1] = 0;

			for (i=0; i < 8; i++)//?????????? ????
			{
				sel = VEC_EL_2(EL, i);

				if (VREG_S(VS1REG, i) != VREG_S(VS2REG, sel))
				{
					SET_COMPARE_FLAG(i);
				}
				else
				{
					if (ZERO_FLAG(i) == 1)
					{
						SET_COMPARE_FLAG(i);
					}
				}
				if (COMPARE_FLAG(i))
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}
				ACCUM_L(i) = vres[i];
			}

			rsp->flag[0] = 0;
			WRITEBACK_RESULT();
			break;
		}

		case 0x23:		/* VGE */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100011 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are greater or equal with VS2
			// Moves the element in VS2 to destination vector

			int sel;
			rsp->flag[1] = 0;

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);

				if (VREG_S(VS1REG, i) == VREG_S(VS2REG, sel))
				{
					if (ZERO_FLAG(i) == 0 || CARRY_FLAG(i) == 0)
					{
						SET_COMPARE_FLAG(i);
					}
				}
				else if (VREG_S(VS1REG, i) > VREG_S(VS2REG, sel))
				{
					SET_COMPARE_FLAG(i);
				}

				if (COMPARE_FLAG(i) != 0)
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}

				ACCUM_L(i) = vres[i];
			}

			rsp->flag[0] = 0;
			WRITEBACK_RESULT();
			break;
		}

		case 0x24:		/* VCL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100100 |
			// ------------------------------------------------------
			//
			// Vector clip low

			int sel;
			INT16 s1, s2;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, sel);

				if (CARRY_FLAG(i) != 0)
				{

					if (ZERO_FLAG(i) != 0)
					{

						if (COMPARE_FLAG(i) != 0)
						{
							ACCUM_L(i) = -(UINT16)s2;
						}
						else
						{
							ACCUM_L(i) = s1;
						}
					}
					else//ZERO_FLAG(i)==0
					{

						if (rsp->flag[2] & (1 << (i)))
						{

							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
							{//proper fix for Harvest Moon 64, r4

								ACCUM_L(i) = s1;
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{

								ACCUM_L(i) = -((UINT16)s2);
								SET_COMPARE_FLAG(i);
							}
						}
						else
						{
							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) != 0)
							{
								ACCUM_L(i) = s1;
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{
								ACCUM_L(i) = -((UINT16)s2);
								SET_COMPARE_FLAG(i);
							}
						}
					}
				}//
				else//CARRY_FLAG(i)==0
				{

					if (ZERO_FLAG(i) != 0)
					{

						if (rsp->flag[1] & (1 << (8+i)))
						{
							ACCUM_L(i) = s2;
						}
						else
						{
							ACCUM_L(i) = s1;
						}
					}
					else
					{
						if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
						{
							ACCUM_L(i) = s2;
							rsp->flag[1] |= (1 << (8+i));
						}
						else
						{
							ACCUM_L(i) = s1;
							rsp->flag[1] &= ~(1 << (8+i));
						}
					}
				}

				vres[i] = ACCUM_L(i);
			}
			rsp->flag[0] = 0;
			rsp->flag[2] = 0;
			WRITEBACK_RESULT();
			break;
		}

		case 0x25:		/* VCH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100101 |
			// ------------------------------------------------------
			//
			// Vector clip high

			int sel;
			INT16 s1, s2;
			rsp->flag[0] = 0;
			rsp->flag[1] = 0;
			rsp->flag[2] = 0;
			UINT32 vce = 0;

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, sel);

				if ((s1 ^ s2) < 0)
				{
					vce = (s1 + s2 == -1);
					SET_CARRY_FLAG(i);
					if (s2 < 0)
					{
						rsp->flag[1] |= (1 << (8+i));
					}

					if (s1 + s2 <= 0)
					{
						SET_COMPARE_FLAG(i);
						vres[i] = -((UINT16)s2);
					}
					else
					{
						vres[i] = s1;
					}

					if (s1 + s2 != 0)
					{
						if (s1 != ~s2)
						{
							SET_ZERO_FLAG(i);
						}
					}
				}//sign
				else
				{
					vce = 0;
					if (s2 < 0)
					{
						SET_COMPARE_FLAG(i);
					}
					if (s1 - s2 >= 0)
					{
						rsp->flag[1] |= (1 << (8+i));
						vres[i] = s2;
					}
					else
					{
						vres[i] = s1;
					}

					if ((s1 - s2) != 0)
					{
						if (s1 != ~s2)
						{
							SET_ZERO_FLAG(i);
						}
					}
				}
				rsp->flag[2] |= (vce << (i));
				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x26:		/* VCR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100110 |
			// ------------------------------------------------------
			//
			// Vector clip reverse

			int sel;
			INT16 s1, s2;
			rsp->flag[0] = 0;
			rsp->flag[1] = 0;
			rsp->flag[2] = 0;

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, sel);

				if ((INT16)(s1 ^ s2) < 0)
				{
					if (s2 < 0)
					{
						rsp->flag[1] |= (1 << (8+i));
					}
					if ((s1 + s2) <= 0)
					{
						ACCUM_L(i) = ~((UINT16)s2);
						SET_COMPARE_FLAG(i);
					}
					else
					{
						ACCUM_L(i) = s1;
					}
				}
				else
				{
					if (s2 < 0)
					{
						SET_COMPARE_FLAG(i);
					}
					if ((s1 - s2) >= 0)
					{
						ACCUM_L(i) = s2;
						rsp->flag[1] |= (1 << (8+i));
					}
					else
					{
						ACCUM_L(i) = s1;
					}
				}

				vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x27:		/* VMRG */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100111 |
			// ------------------------------------------------------
			//
			// Merges two vectors according to compare flags

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				if (COMPARE_FLAG(i) != 0)
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);//??? ???????????
				}

				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x28:		/* VAND */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
			// ------------------------------------------------------
			//
			// Bitwise AND of two vector registers

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = VREG_S(VS1REG, i) & VREG_S(VS2REG, sel);
				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x29:		/* VNAND */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101001 |
			// ------------------------------------------------------
			//
			// Bitwise NOT AND of two vector registers

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = ~((VREG_S(VS1REG, i) & VREG_S(VS2REG, sel)));
				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2a:		/* VOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101010 |
			// ------------------------------------------------------
			//
			// Bitwise OR of two vector registers

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = VREG_S(VS1REG, i) | VREG_S(VS2REG, sel);
				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2b:		/* VNOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101011 |
			// ------------------------------------------------------
			//
			// Bitwise NOT OR of two vector registers

			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = ~((VREG_S(VS1REG, i) | VREG_S(VS2REG, sel)));
				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2c:		/* VXOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101100 |
			// ------------------------------------------------------
			//
			// Bitwise XOR of two vector registers

			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = VREG_S(VS1REG, i) ^ VREG_S(VS2REG, sel);
				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2d:		/* VNXOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
			// ------------------------------------------------------
			//
			// Bitwise NOT XOR of two vector registers

			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = ~((VREG_S(VS1REG, i) ^ VREG_S(VS2REG, sel)));
				ACCUM_L(i) = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x30:		/* VRCP */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110000 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal
			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, sel));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
					{
						shifter = i;
						break;
					}
				}
			}
			else
			{
				shifter = 0x10;
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
			if (rec < 0)
			{
				temp = ~temp;
			}
			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			rsp->reciprocal_res = rec;
			rsp->dp_allowed = 0;

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}


			break;
		}

		case 0x31:		/* VRCPL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110001 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal low part

			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = ((UINT16)(VREG_S(VS2REG, sel)) | ((UINT32)(rsp->reciprocal_high) & 0xffff0000));

			INT32 datainput = rec;

			if (rec < 0)
			{
				if (rsp->dp_allowed)
				{
					if (rec < -32768)
					{
						datainput = ~datainput;
					}
					else
					{
						datainput = -datainput;
					}
				}
				else
				{
					datainput = -datainput;
				}
			}


			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
					{
						shifter = i;
						break;
					}
				}
			}
			else
			{
				if (rsp->dp_allowed)
				{
					shifter = 0;
				}
				else
				{
					shifter = 0x10;
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
			if (rec < 0)
			{
				temp = ~temp;
			}
			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			rsp->reciprocal_res = rec;
			rsp->dp_allowed = 0;

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}

			break;
		}

		case 0x32:		/* VRCPH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110010 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal high part

			int del = VS1REG & 7;
			int sel = EL & 7;

			rsp->reciprocal_high = (VREG_S(VS2REG, sel)) << 16;
			rsp->dp_allowed = 1;

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}

			VREG_S(VDREG, del) = (INT16)(rsp->reciprocal_res >> 16);

			break;
		}

		case 0x33:		/* VMOV */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110011 |
			// ------------------------------------------------------
			//
			// Moves element from vector to destination vector

			int del = VS1REG & 7;
			int sel = EL & 7;

			VREG_S(VDREG, del) = VREG_S(VS2REG, sel);
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}
			break;
		}

		case 0x34:		/* VRSQ */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110100 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root

			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, sel));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
					{
						shifter = i;
						break;
					}
				}
			}
			else
			{
				shifter = 0x10;
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
			if (rec < 0)
			{
				temp = ~temp;
			}
			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			rsp->reciprocal_res = rec;
			rsp->dp_allowed = 0;

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}

			break;
		}

		case 0x35:		/* VRSQL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110101 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root low part

			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = ((UINT16)(VREG_S(VS2REG, sel)) | ((UINT32)(rsp->reciprocal_high) & 0xffff0000));

			INT32 datainput = rec;

			if (rec < 0)
			{
				if (rsp->dp_allowed)
				{
					if (rec < -32768)//VDIV.C,208
					{
						datainput = ~datainput;
					}
					else
					{
						datainput = -datainput;
					}
				}
				else
				{
					datainput = -datainput;
				}
			}

			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))
					{
						shifter = i;
						break;
					}
				}
			}
			else
			{
				if (rsp->dp_allowed)
				{
					shifter = 0;
				}
				else
				{
					shifter = 0x10;
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
			if (rec < 0)
			{
				temp = ~temp;
			}
			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			rsp->reciprocal_res = rec;
			rsp->dp_allowed = 0;

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}

			break;
		}

		case 0x36:		/* VRSQH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110110 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root high part

			int del = VS1REG & 7;
			int sel = EL & 7;

			rsp->reciprocal_high = (VREG_S(VS2REG, sel)) << 16;
			rsp->dp_allowed = 1;

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}

			VREG_S(VDREG, del) = (INT16)(rsp->reciprocal_res >> 16);	// store high part
			break;
		}

		case 0x37:		/* VNOP */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110111 |
			// ------------------------------------------------------
			//
			// Vector null instruction

			break;
		}

		default:	unimplemented_opcode(rsp, op); break;
	}
}

static CPU_EXECUTE( rsp )
{
	rsp_state *rsp = get_safe_token(device);
	UINT32 op;

	rsp->pc = 0x4001000 | (rsp->pc & 0xfff);

	if( rsp->sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
	{
		rsp->icount = MIN(rsp->icount, 0);
	}

	while (rsp->icount > 0)
	{
		rsp->ppc = rsp->pc;
		debugger_instruction_hook(device, rsp->pc);

		op = ROPCODE(rsp->pc);
		if (rsp->nextpc != ~0)
		{
			rsp->pc = rsp->nextpc;
			rsp->nextpc = ~0;
		}
		else
		{
			rsp->pc += 4;
		}

		switch (op >> 26)
		{
			case 0x00:	/* SPECIAL */
			{
				switch (op & 0x3f)
				{
					case 0x00:	/* SLL */		if (RDREG) RDVAL = (UINT32)RTVAL << SHIFT; break;
					case 0x02:	/* SRL */		if (RDREG) RDVAL = (UINT32)RTVAL >> SHIFT; break;
					case 0x03:	/* SRA */		if (RDREG) RDVAL = (INT32)RTVAL >> SHIFT; break;
					case 0x04:	/* SLLV */		if (RDREG) RDVAL = (UINT32)RTVAL << (RSVAL & 0x1f); break;
					case 0x06:	/* SRLV */		if (RDREG) RDVAL = (UINT32)RTVAL >> (RSVAL & 0x1f); break;
					case 0x07:	/* SRAV */		if (RDREG) RDVAL = (INT32)RTVAL >> (RSVAL & 0x1f); break;
					case 0x08:	/* JR */		JUMP_PC(RSVAL); break;
					case 0x09:	/* JALR */		JUMP_PC_L(RSVAL, RDREG); break;
					case 0x0d:	/* BREAK */
					{
						(rsp->sp_set_status_func)(0, 0x3);
						rsp->icount = MIN(rsp->icount, 1);

						if (LOG_INSTRUCTION_EXECUTION) fprintf(rsp->exec_output, "\n---------- break ----------\n\n");

						break;
					}
					case 0x20:	/* ADD */		if (RDREG) RDVAL = (INT32)(RSVAL + RTVAL); break;
					case 0x21:	/* ADDU */		if (RDREG) RDVAL = (INT32)(RSVAL + RTVAL); break;
					case 0x22:	/* SUB */		if (RDREG) RDVAL = (INT32)(RSVAL - RTVAL); break;
					case 0x23:	/* SUBU */		if (RDREG) RDVAL = (INT32)(RSVAL - RTVAL); break;
					case 0x24:	/* AND */		if (RDREG) RDVAL = RSVAL & RTVAL; break;
					case 0x25:	/* OR */		if (RDREG) RDVAL = RSVAL | RTVAL; break;
					case 0x26:	/* XOR */		if (RDREG) RDVAL = RSVAL ^ RTVAL; break;
					case 0x27:	/* NOR */		if (RDREG) RDVAL = ~(RSVAL | RTVAL); break;
					case 0x2a:	/* SLT */		if (RDREG) RDVAL = (INT32)RSVAL < (INT32)RTVAL; break;
					case 0x2b:	/* SLTU */		if (RDREG) RDVAL = (UINT32)RSVAL < (UINT32)RTVAL; break;
					default:	unimplemented_opcode(rsp, op); break;
				}
				break;
			}

			case 0x01:	/* REGIMM */
			{
				switch (RTREG)
				{
					case 0x00:	/* BLTZ */		if ((INT32)(RSVAL) < 0) JUMP_REL(SIMM16); break;
					case 0x01:	/* BGEZ */		if ((INT32)(RSVAL) >= 0) JUMP_REL(SIMM16); break;
					case 0x10:	/* BLTZAL */	if ((INT32)(RSVAL) < 0) JUMP_REL_L(SIMM16, 31); break;
					case 0x11:	/* BGEZAL */	if ((INT32)(RSVAL) >= 0) JUMP_REL_L(SIMM16, 31); break;
					default:	unimplemented_opcode(rsp, op); break;
				}
				break;
			}

			case 0x02:	/* J */			JUMP_ABS(UIMM26); break;
			case 0x03:	/* JAL */		JUMP_ABS_L(UIMM26, 31); break;
			case 0x04:	/* BEQ */		if (RSVAL == RTVAL) JUMP_REL(SIMM16); break;
			case 0x05:	/* BNE */		if (RSVAL != RTVAL) JUMP_REL(SIMM16); break;
			case 0x06:	/* BLEZ */		if ((INT32)RSVAL <= 0) JUMP_REL(SIMM16); break;
			case 0x07:	/* BGTZ */		if ((INT32)RSVAL > 0) JUMP_REL(SIMM16); break;
			case 0x08:	/* ADDI */		if (RTREG) RTVAL = (INT32)(RSVAL + SIMM16); break;
			case 0x09:	/* ADDIU */		if (RTREG) RTVAL = (INT32)(RSVAL + SIMM16); break;
			case 0x0a:	/* SLTI */		if (RTREG) RTVAL = (INT32)(RSVAL) < ((INT32)SIMM16); break;
			case 0x0b:	/* SLTIU */		if (RTREG) RTVAL = (UINT32)(RSVAL) < (UINT32)((INT32)SIMM16); break;
			case 0x0c:	/* ANDI */		if (RTREG) RTVAL = RSVAL & UIMM16; break;
			case 0x0d:	/* ORI */		if (RTREG) RTVAL = RSVAL | UIMM16; break;
			case 0x0e:	/* XORI */		if (RTREG) RTVAL = RSVAL ^ UIMM16; break;
			case 0x0f:	/* LUI */		if (RTREG) RTVAL = UIMM16 << 16; break;

			case 0x10:	/* COP0 */
			{
				switch ((op >> 21) & 0x1f)
				{
					case 0x00:	/* MFC0 */		if (RTREG) RTVAL = get_cop0_reg(rsp, RDREG); break;
					case 0x04:	/* MTC0 */		set_cop0_reg(rsp, RDREG, RTVAL); break;
					default:	unimplemented_opcode(rsp, op); break;
				}
				break;
			}

			case 0x12:	/* COP2 */
			{
				switch ((op >> 21) & 0x1f)
				{
					case 0x00:	/* MFC2 */
					{
						// 31       25      20      15      10     6         0
						// ---------------------------------------------------
						// | 010010 | 00000 | TTTTT | DDDDD | IIII | 0000000 |
						// ---------------------------------------------------
						//

						int el = (op >> 7) & 0xf;
						UINT16 b1 = VREG_B(RDREG, (el+0) & 0xf);
						UINT16 b2 = VREG_B(RDREG, (el+1) & 0xf);
						if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
						break;
					}
					case 0x02:	/* CFC2 */
					{
						// 31       25      20      15      10            0
						// ------------------------------------------------
						// | 010010 | 00010 | TTTTT | DDDDD | 00000000000 |
						// ------------------------------------------------
						//

						if (RTREG)
						{
							if (RDREG == 2)
							{
								// Anciliary clipping flags
								RTVAL = rsp->flag[RDREG] & 0x00ff;
							}
							else
							{
								// All other flags are 16 bits but sign-extended at retrieval
								RTVAL = (UINT32)rsp->flag[RDREG] | ( ( rsp->flag[RDREG] & 0x8000 ) ? 0xffff0000 : 0 );
							}
						}
						break;
					}
					case 0x04:	/* MTC2 */
					{
						// 31       25      20      15      10     6         0
						// ---------------------------------------------------
						// | 010010 | 00100 | TTTTT | DDDDD | IIII | 0000000 |
						// ---------------------------------------------------
						//

						int el = (op >> 7) & 0xf;
						W_VREG_B(RDREG, (el+0) & 0xf, (RTVAL >> 8) & 0xff);
						W_VREG_B(RDREG, (el+1) & 0xf, (RTVAL >> 0) & 0xff);
						break;
					}
					case 0x06:	/* CTC2 */
					{
						// 31       25      20      15      10            0
						// ------------------------------------------------
						// | 010010 | 00110 | TTTTT | DDDDD | 00000000000 |
						// ------------------------------------------------
						//

						rsp->flag[RDREG] = RTVAL & 0xffff;
						break;
					}

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
					{
						handle_vector_ops(rsp, op);
						break;
					}

					default:	unimplemented_opcode(rsp, op); break;
				}
				break;
			}

			case 0x20:	/* LB */		if (RTREG) RTVAL = (INT32)(INT8)READ8(rsp, RSVAL + SIMM16); break;
			case 0x21:	/* LH */		if (RTREG) RTVAL = (INT32)(INT16)READ16(rsp, RSVAL + SIMM16); break;
			case 0x23:	/* LW */		if (RTREG) RTVAL = READ32(rsp, RSVAL + SIMM16); break;
			case 0x24:	/* LBU */		if (RTREG) RTVAL = (UINT8)READ8(rsp, RSVAL + SIMM16); break;
			case 0x25:	/* LHU */		if (RTREG) RTVAL = (UINT16)READ16(rsp, RSVAL + SIMM16); break;
			case 0x28:	/* SB */		WRITE8(rsp, RSVAL + SIMM16, RTVAL); break;
			case 0x29:	/* SH */		WRITE16(rsp, RSVAL + SIMM16, RTVAL); break;
			case 0x2b:	/* SW */		WRITE32(rsp, RSVAL + SIMM16, RTVAL); break;
			case 0x32:	/* LWC2 */		handle_lwc2(rsp, op); break;
			case 0x3a:	/* SWC2 */		handle_swc2(rsp, op); break;

			default:
			{
				unimplemented_opcode(rsp, op);
				break;
			}
		}

		if (LOG_INSTRUCTION_EXECUTION)
		{
			int i, l;
			static UINT32 prev_regs[32];
			static VECTOR_REG prev_vecs[32];
			char string[200];
			rsp_dasm_one(string, rsp->ppc, op);

			fprintf(rsp->exec_output, "%08X: %s", rsp->ppc, string);

			l = strlen(string);
			if (l < 36)
			{
				for (i=l; i < 36; i++)
				{
					fprintf(rsp->exec_output, " ");
				}
			}

			fprintf(rsp->exec_output, "| ");

			for (i=0; i < 32; i++)
			{
				if (rsp->r[i] != prev_regs[i])
				{
					fprintf(rsp->exec_output, "R%d: %08X ", i, rsp->r[i]);
				}
				prev_regs[i] = rsp->r[i];
			}

			for (i=0; i < 32; i++)
			{
				if (rsp->v[i].d[0] != prev_vecs[i].d[0] || rsp->v[i].d[1] != prev_vecs[i].d[1])
				{
					fprintf(rsp->exec_output, "V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X ", i,
					(UINT16)VREG_S(i,0), (UINT16)VREG_S(i,1), (UINT16)VREG_S(i,2), (UINT16)VREG_S(i,3), (UINT16)VREG_S(i,4), (UINT16)VREG_S(i,5), (UINT16)VREG_S(i,6), (UINT16)VREG_S(i,7));
				}
				prev_vecs[i].d[0] = rsp->v[i].d[0];
				prev_vecs[i].d[1] = rsp->v[i].d[1];
			}

			fprintf(rsp->exec_output, "\n");

		}

		--rsp->icount;

		if( rsp->sr & RSP_STATUS_SSTEP )
		{
			if( rsp->step_count )
			{
				rsp->step_count--;
			}
			else
			{
				rsp->sr |= RSP_STATUS_BROKE;
			}
		}

		if( rsp->sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
		{
			rsp->icount = MIN(rsp->icount, 0);
		}

	}
}




/*****************************************************************************/

static CPU_SET_INFO( rsp )
{
	rsp_state *rsp = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + RSP_PC:             rsp->pc = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R0:             rsp->r[0] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R1:             rsp->r[1] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R2:             rsp->r[2] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R3:             rsp->r[3] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R4:             rsp->r[4] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R5:             rsp->r[5] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R6:             rsp->r[6] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R7:             rsp->r[7] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R8:             rsp->r[8] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R9:             rsp->r[9] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R10:            rsp->r[10] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R11:            rsp->r[11] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R12:            rsp->r[12] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R13:            rsp->r[13] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R14:            rsp->r[14] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R15:            rsp->r[15] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R16:            rsp->r[16] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R17:            rsp->r[17] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R18:            rsp->r[18] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R19:            rsp->r[19] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R20:            rsp->r[20] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R21:            rsp->r[21] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R22:            rsp->r[22] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R23:            rsp->r[23] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R24:            rsp->r[24] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R25:            rsp->r[25] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R26:            rsp->r[26] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R27:            rsp->r[27] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R28:            rsp->r[28] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R29:            rsp->r[29] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_R30:            rsp->r[30] = info->i;        break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + RSP_R31:            rsp->r[31] = info->i;        break;
		case CPUINFO_INT_REGISTER + RSP_SR:             rsp->sr = info->i;           break;
		case CPUINFO_INT_REGISTER + RSP_NEXTPC:         rsp->nextpc = info->i;       break;
		case CPUINFO_INT_REGISTER + RSP_STEPCNT:        rsp->step_count = info->i;   break;
	}
}

CPU_GET_INFO( rsp )
{
	rsp_state *rsp = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(rsp_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = CLEAR_LINE;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = rsp->ppc;						break;

		case CPUINFO_INT_PC:	/* intentional fallthrough */
		case CPUINFO_INT_REGISTER + RSP_PC:				info->i = rsp->pc;						break;

		case CPUINFO_INT_REGISTER + RSP_R0:				info->i = rsp->r[0];						break;
		case CPUINFO_INT_REGISTER + RSP_R1:				info->i = rsp->r[1];						break;
		case CPUINFO_INT_REGISTER + RSP_R2:				info->i = rsp->r[2];						break;
		case CPUINFO_INT_REGISTER + RSP_R3:				info->i = rsp->r[3];						break;
		case CPUINFO_INT_REGISTER + RSP_R4:				info->i = rsp->r[4];						break;
		case CPUINFO_INT_REGISTER + RSP_R5:				info->i = rsp->r[5];						break;
		case CPUINFO_INT_REGISTER + RSP_R6:				info->i = rsp->r[6];						break;
		case CPUINFO_INT_REGISTER + RSP_R7:				info->i = rsp->r[7];						break;
		case CPUINFO_INT_REGISTER + RSP_R8:				info->i = rsp->r[8];						break;
		case CPUINFO_INT_REGISTER + RSP_R9:				info->i = rsp->r[9];						break;
		case CPUINFO_INT_REGISTER + RSP_R10:			info->i = rsp->r[10];					break;
		case CPUINFO_INT_REGISTER + RSP_R11:			info->i = rsp->r[11];					break;
		case CPUINFO_INT_REGISTER + RSP_R12:			info->i = rsp->r[12];					break;
		case CPUINFO_INT_REGISTER + RSP_R13:			info->i = rsp->r[13];					break;
		case CPUINFO_INT_REGISTER + RSP_R14:			info->i = rsp->r[14];					break;
		case CPUINFO_INT_REGISTER + RSP_R15:			info->i = rsp->r[15];					break;
		case CPUINFO_INT_REGISTER + RSP_R16:			info->i = rsp->r[16];					break;
		case CPUINFO_INT_REGISTER + RSP_R17:			info->i = rsp->r[17];					break;
		case CPUINFO_INT_REGISTER + RSP_R18:			info->i = rsp->r[18];					break;
		case CPUINFO_INT_REGISTER + RSP_R19:			info->i = rsp->r[19];					break;
		case CPUINFO_INT_REGISTER + RSP_R20:			info->i = rsp->r[20];					break;
		case CPUINFO_INT_REGISTER + RSP_R21:			info->i = rsp->r[21];					break;
		case CPUINFO_INT_REGISTER + RSP_R22:			info->i = rsp->r[22];					break;
		case CPUINFO_INT_REGISTER + RSP_R23:			info->i = rsp->r[23];					break;
		case CPUINFO_INT_REGISTER + RSP_R24:			info->i = rsp->r[24];					break;
		case CPUINFO_INT_REGISTER + RSP_R25:			info->i = rsp->r[25];					break;
		case CPUINFO_INT_REGISTER + RSP_R26:			info->i = rsp->r[26];					break;
		case CPUINFO_INT_REGISTER + RSP_R27:			info->i = rsp->r[27];					break;
		case CPUINFO_INT_REGISTER + RSP_R28:			info->i = rsp->r[28];					break;
		case CPUINFO_INT_REGISTER + RSP_R29:			info->i = rsp->r[29];					break;
		case CPUINFO_INT_REGISTER + RSP_R30:			info->i = rsp->r[30];					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + RSP_R31:			info->i = rsp->r[31];					break;
		case CPUINFO_INT_REGISTER + RSP_SR:             info->i = rsp->sr;                       break;
		case CPUINFO_INT_REGISTER + RSP_NEXTPC:         info->i = rsp->nextpc;                   break;
		case CPUINFO_INT_REGISTER + RSP_STEPCNT:        info->i = rsp->step_count;               break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(rsp);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(rsp);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(rsp);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(rsp);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(rsp);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(rsp);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &rsp->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "RSP");					break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "RSP");					break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team");	break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + RSP_PC:				sprintf(info->s, "PC: %08X", rsp->pc);	break;

		case CPUINFO_STR_REGISTER + RSP_R0:				sprintf(info->s, "R0: %08X", rsp->r[0]); break;
		case CPUINFO_STR_REGISTER + RSP_R1:				sprintf(info->s, "R1: %08X", rsp->r[1]); break;
		case CPUINFO_STR_REGISTER + RSP_R2:				sprintf(info->s, "R2: %08X", rsp->r[2]); break;
		case CPUINFO_STR_REGISTER + RSP_R3:				sprintf(info->s, "R3: %08X", rsp->r[3]); break;
		case CPUINFO_STR_REGISTER + RSP_R4:				sprintf(info->s, "R4: %08X", rsp->r[4]); break;
		case CPUINFO_STR_REGISTER + RSP_R5:				sprintf(info->s, "R5: %08X", rsp->r[5]); break;
		case CPUINFO_STR_REGISTER + RSP_R6:				sprintf(info->s, "R6: %08X", rsp->r[6]); break;
		case CPUINFO_STR_REGISTER + RSP_R7:				sprintf(info->s, "R7: %08X", rsp->r[7]); break;
		case CPUINFO_STR_REGISTER + RSP_R8:				sprintf(info->s, "R8: %08X", rsp->r[8]); break;
		case CPUINFO_STR_REGISTER + RSP_R9:				sprintf(info->s, "R9: %08X", rsp->r[9]); break;
		case CPUINFO_STR_REGISTER + RSP_R10:			sprintf(info->s, "R10: %08X", rsp->r[10]); break;
		case CPUINFO_STR_REGISTER + RSP_R11:			sprintf(info->s, "R11: %08X", rsp->r[11]); break;
		case CPUINFO_STR_REGISTER + RSP_R12:			sprintf(info->s, "R12: %08X", rsp->r[12]); break;
		case CPUINFO_STR_REGISTER + RSP_R13:			sprintf(info->s, "R13: %08X", rsp->r[13]); break;
		case CPUINFO_STR_REGISTER + RSP_R14:			sprintf(info->s, "R14: %08X", rsp->r[14]); break;
		case CPUINFO_STR_REGISTER + RSP_R15:			sprintf(info->s, "R15: %08X", rsp->r[15]); break;
		case CPUINFO_STR_REGISTER + RSP_R16:			sprintf(info->s, "R16: %08X", rsp->r[16]); break;
		case CPUINFO_STR_REGISTER + RSP_R17:			sprintf(info->s, "R17: %08X", rsp->r[17]); break;
		case CPUINFO_STR_REGISTER + RSP_R18:			sprintf(info->s, "R18: %08X", rsp->r[18]); break;
		case CPUINFO_STR_REGISTER + RSP_R19:			sprintf(info->s, "R19: %08X", rsp->r[19]); break;
		case CPUINFO_STR_REGISTER + RSP_R20:			sprintf(info->s, "R20: %08X", rsp->r[20]); break;
		case CPUINFO_STR_REGISTER + RSP_R21:			sprintf(info->s, "R21: %08X", rsp->r[21]); break;
		case CPUINFO_STR_REGISTER + RSP_R22:			sprintf(info->s, "R22: %08X", rsp->r[22]); break;
		case CPUINFO_STR_REGISTER + RSP_R23:			sprintf(info->s, "R23: %08X", rsp->r[23]); break;
		case CPUINFO_STR_REGISTER + RSP_R24:			sprintf(info->s, "R24: %08X", rsp->r[24]); break;
		case CPUINFO_STR_REGISTER + RSP_R25:			sprintf(info->s, "R25: %08X", rsp->r[25]); break;
		case CPUINFO_STR_REGISTER + RSP_R26:			sprintf(info->s, "R26: %08X", rsp->r[26]); break;
		case CPUINFO_STR_REGISTER + RSP_R27:			sprintf(info->s, "R27: %08X", rsp->r[27]); break;
		case CPUINFO_STR_REGISTER + RSP_R28:			sprintf(info->s, "R28: %08X", rsp->r[28]); break;
		case CPUINFO_STR_REGISTER + RSP_R29:			sprintf(info->s, "R29: %08X", rsp->r[29]); break;
		case CPUINFO_STR_REGISTER + RSP_R30:			sprintf(info->s, "R30: %08X", rsp->r[30]); break;
		case CPUINFO_STR_REGISTER + RSP_R31:			sprintf(info->s, "R31: %08X", rsp->r[31]); break;
		case CPUINFO_STR_REGISTER + RSP_SR:             sprintf(info->s, "SR: %08X",  rsp->sr);    break;
		case CPUINFO_STR_REGISTER + RSP_NEXTPC:         sprintf(info->s, "NPC: %08X", rsp->nextpc);break;
		case CPUINFO_STR_REGISTER + RSP_STEPCNT:        sprintf(info->s, "STEP: %d",  rsp->step_count);  break;
	}
}

void rspdrc_flush_drc_cache(device_t *device)
{
}

void rspdrc_set_options(device_t *device, UINT32 options)
{
}

void rspdrc_add_imem(device_t *device, UINT32 *base)
{
}

void rspdrc_add_dmem(device_t *device, UINT32 *base)
{
}

DEFINE_LEGACY_CPU_DEVICE(RSP, rsp);

#endif // USE_RSPDRC
