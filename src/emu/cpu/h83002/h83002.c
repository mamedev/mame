/***************************************************************************

 h83002.c: Hitachi H8/3002 microcontroller emulator

 Original by The_Author & DynaChicken for the ZiNc emulator.

 Rewrite for MAME by R. Belmont, including...
   - Changed types to MAME standards
   - Added MAME cpuintrf glue
   - Put all per-CPU state in a struct
   - Fixed ADD flag calcs
   - Added support for all 8 flag bits
   - Added cycle timing (probably not right yet)
   - Fixed and optimized interrupt handling
   - Optimized main execution loop
   - Added new opcodes for ND-1 and System 23 BIOS programs
   - Improved I/O and timer support
   - Fixed major error in 7Cxx/7Dxx series bit opcodes where the wrong
     instructions were being picked.

 TS 20060412 Added exts.l, sub.l, divxs.w (buggy), jsr @reg, rotxl.l reg, mov.l @(adr, reg), reg
 LE 20070903 Added divxu.b  shal.l  extu.w  dec.l #Imm,Rd  subx.b
 LE 20080202 Separated 3002/3044/3007, Added or.l  shal.l  rotl.l  not.l  neg.l  exts.w
             sub/or/xor.l #Imm:32,ERd  bset/bnot/bclr.b Rn,@ERd  bst/bist.b #Imm:3,@ERd  bnot.b #Imm:3,@ERd

 Note: The H8/3000 series is normally back-compatible to the 8-bit H8/300,
 but the 3002 does not include "emulation mode" - it always runs in full
 16/32-bit ("advanced") mode.  So this core is not suitable for general
 H8/300 emulation.

****************************************************************************/

#include "debugger.h"
#include "h83002.h"
#include "h8priv.h"

#define H8_SP	(7)

#define h8_mem_read8(x) program_read_byte_16be(x)
#define h8_mem_read16(x) program_read_word_16be(x)
#define h8_mem_write8(x, y)  program_write_byte_16be(x, y)
#define h8_mem_write16(x, y) program_write_word_16be(x, y)

// timing macros
// note: we assume a system 12 - type setup where external access is 3+1 states
// timing will be off somewhat for other configurations.
#define H8_IFETCH_TIMING(x)	h8_cyccnt -= (x) * 4;
#define H8_BRANCH_TIMING(x)	h8_cyccnt -= (x) * 4;
#define H8_STACK_TIMING(x)	h8_cyccnt -= (x) * 4;
#define H8_BYTE_TIMING(x, adr)	if (address24 >= 0xffff10) h8_cyccnt -= (x) * 3; else h8_cyccnt -= (x) * 4;
#define H8_WORD_TIMING(x, adr)	if (address24 >= 0xffff10) h8_cyccnt -= (x) * 3; else h8_cyccnt -= (x) * 4;
#define H8_IOP_TIMING(x)	h8_cyccnt -= (x);

INLINE UINT32 h8_mem_read32(offs_t address)
{
	UINT32 result = program_read_word_16be(address) << 16;
	return result | program_read_word_16be(address + 2);
}

INLINE void h8_mem_write32(offs_t address, UINT32 data)
{
	program_write_word_16be(address, data >> 16);
	program_write_word_16be(address + 2, data);
}


h83002_state h8;

static INT32 h8_cyccnt;

static UINT32 udata32, address24;
static INT32 sdata32;
static UINT16 udata16, ext16;
static INT16 sdata16;
static UINT8 udata8;
static INT8 sdata8;
static UINT8 srcreg, dstreg;

/* internal functions */

static void h8_check_irqs(void);

static void h8_group0(UINT16 opcode);
static void h8_group1(UINT16 opcode);
static void h8_group5(UINT16 opcode);
static void h8_group6(UINT16 opcode);
static void h8_group7(UINT16 opcode);

static int h8_branch(UINT8 condition);

static UINT8 h8_mov8(UINT8 src);
static UINT16 h8_mov16(UINT16 src);
static UINT32 h8_mov32(UINT32 src);

static UINT8 h8_add8(UINT8 src, UINT8 dst);
static UINT16 h8_add16(UINT16 src, UINT16 dst);
static UINT32 h8_add32(UINT32 src, UINT32 dst);

static UINT8 h8_sub8(UINT8 src, UINT8 dst);
static UINT16 h8_sub16(UINT16 src, UINT16 dst);
static UINT32 h8_sub32(UINT32 src, UINT32 dst);

static UINT8 h8_addx8(UINT8 src, UINT8 dst);

static void h8_cmp8(UINT8 src, UINT8 dst);
static void h8_cmp16(UINT16 src, UINT16 dst);
static void h8_cmp32(UINT32 src, UINT32 dst);
static UINT8 h8_subx8(UINT8 src, UINT8 dst);

static UINT8 h8_or8(UINT8 src, UINT8 dst);
static UINT16 h8_or16(UINT16 src, UINT16 dst);
static UINT32 h8_or32(UINT32 src, UINT32 dst);

static UINT8 h8_xor8(UINT8 src, UINT8 dst);
static UINT16 h8_xor16(UINT16 src, UINT16 dst);
static UINT32 h8_xor32(UINT32 src, UINT32 dst);

static UINT8 h8_and8(UINT8 src, UINT8 dst);
static UINT16 h8_and16(UINT16 src, UINT16 dst);
static UINT32 h8_and32(UINT32 src, UINT32 dst);

static INT8 h8_neg8(INT8 src);
static INT16 h8_neg16(INT16 src);
static INT32 h8_neg32(INT32 src);

static UINT16 h8_divxu8 (UINT16 dst, UINT8  src);
static UINT32 h8_divxu16(UINT32 dst, UINT16 src);

static UINT8 h8_not8(UINT8 src);
static UINT16 h8_not16(UINT16 src);
static UINT32 h8_not32(UINT32 src);

static UINT8 h8_rotl8(UINT8 src);
static UINT16 h8_rotl16(UINT16 src);
static UINT32 h8_rotl32(UINT32 src);

static UINT8 h8_rotxl8(UINT8 src);
static UINT16 h8_rotxl16(UINT16 src);
static UINT32 h8_rotxl32(UINT32 src);

static UINT8 h8_rotxr8(UINT8 src);
static UINT16 h8_rotxr16(UINT16 src);

static UINT8 h8_shll8(UINT8 src);
static UINT16 h8_shll16(UINT16 src);
static UINT32 h8_shll32(UINT32 src);

static UINT8 h8_shlr8(UINT8 src);
static UINT16 h8_shlr16(UINT16 src);
static UINT32 h8_shlr32(UINT32 src);

static INT8 h8_shal8(INT8 src);
static INT16 h8_shal16(INT16 src);
static INT32 h8_shal32(INT32 src);

static INT8 h8_shar8(INT8 src);
static INT16 h8_shar16(INT16 src);
static INT32 h8_shar32(INT32 src);

static UINT8 h8_dec8(UINT8 src);
static UINT16 h8_dec16(UINT16 src);
static UINT32 h8_dec32(UINT32 src);

static UINT8 h8_inc8(UINT8 src);
static UINT16 h8_inc16(UINT16 src);
static UINT32 h8_inc32(UINT32 src);

static UINT8 h8_bnot8(UINT8 src, UINT8 dst);
static UINT8 h8_bst8(UINT8 src, UINT8 dst);
static UINT8 h8_bist8(UINT8 src, UINT8 dst);
static UINT8 h8_bset8(UINT8 src, UINT8 dst);
static UINT8 h8_bclr8(UINT8 src, UINT8 dst);
static void h8_btst8(UINT8 src, UINT8 dst);
static void h8_bld8(UINT8 src, UINT8 dst); // loads to carry
static void h8_bor8(UINT8 src, UINT8 dst); // result in carry
//static void h8_bxor8(UINT8 src, UINT8 dst);

static INT32 h8_mulxs16(INT16 src, INT16 dst);
static UINT32 h8_divxs16(INT16 src, INT32 dst);

/* implementation */

extern CPU_DISASSEMBLE( h8 );

void h8_3002_InterruptRequest(UINT8 source)
{
	if(source>31)
	{
		h8.h8_IRQrequestH |= (1<<(source-32));
	}
	else
	{
		h8.h8_IRQrequestL |= (1<<source);
	}
}


static UINT8 h8_get_ccr(void)
{
	h8.ccr = 0;
	if(h8.h8nflag)h8.ccr |= NFLAG;
	if(h8.h8zflag)h8.ccr |= ZFLAG;
	if(h8.h8vflag)h8.ccr |= VFLAG;
	if(h8.h8cflag)h8.ccr |= CFLAG;
	if(h8.h8uflag)h8.ccr |= UFLAG;
	if(h8.h8hflag)h8.ccr |= HFLAG;
	if(h8.h8uiflag)h8.ccr |= UIFLAG;
	if(h8.h8iflag)h8.ccr |= IFLAG;
	return h8.ccr;
}

static char *h8_get_ccr_str(void)
{
	static char res[8];

	memset(res, 0, 8);
	if(h8.h8iflag) strcat(res, "I"); else strcat(res, "i");
	if(h8.h8uiflag)strcat(res, "U"); else strcat(res, "u");
	if(h8.h8hflag) strcat(res, "H"); else strcat(res, "h");
	if(h8.h8uflag) strcat(res, "U"); else strcat(res, "u");
	if(h8.h8nflag) strcat(res, "N"); else strcat(res, "n");
	if(h8.h8zflag) strcat(res, "Z"); else strcat(res, "z");
	if(h8.h8vflag) strcat(res, "V"); else strcat(res, "v");
	if(h8.h8cflag) strcat(res, "C"); else strcat(res, "c");

	return res;
}

static void h8_set_ccr(UINT8 data)
{
	h8.ccr = data;

	h8.h8nflag = 0;
	h8.h8zflag = 0;
	h8.h8vflag = 0;
	h8.h8cflag = 0;
	h8.h8hflag = 0;
	h8.h8iflag = 0;
	h8.h8uflag = 0;
	h8.h8uiflag = 0;

	if(h8.ccr & NFLAG) h8.h8nflag = 1;
	if(h8.ccr & ZFLAG) h8.h8zflag = 1;
	if(h8.ccr & VFLAG) h8.h8vflag = 1;
	if(h8.ccr & CFLAG) h8.h8cflag = 1;
	if(h8.ccr & HFLAG) h8.h8hflag = 1;
	if(h8.ccr & UFLAG) h8.h8uflag = 1;
	if(h8.ccr & UIFLAG) h8.h8uiflag = 1;
	if(h8.ccr & IFLAG) h8.h8iflag = 1;

	h8_check_irqs();
}

static INT16 h8_getreg16(UINT8 reg)
{
	if(reg > 7)
	{
		return h8.regs[reg-8]>>16;
	}
	else
	{
		return h8.regs[reg];
	}
}

static void h8_setreg16(UINT8 reg, UINT16 data)
{
	if(reg > 7)
	{
		h8.regs[reg-8] &= 0xffff;
		h8.regs[reg-8] |= data<<16;
	}
	else
	{
		h8.regs[reg] &= 0xffff0000;
		h8.regs[reg] |= data;
	}
}

static UINT8 h8_getreg8(UINT8 reg)
{
	if(reg > 7)
	{
		return h8.regs[reg-8];
	}
	else
	{
		return h8.regs[reg]>>8;
	}
}

static void h8_setreg8(UINT8 reg, UINT8 data)
{
	if(reg > 7)
	{
		h8.regs[reg-8] &= 0xffffff00;
		h8.regs[reg-8] |= data;
	}
	else
	{
		h8.regs[reg] &= 0xffff00ff;
		h8.regs[reg] |= data<<8;
	}
}

static UINT32 h8_getreg32(UINT8 reg)
{
	return h8.regs[reg];
}

static void h8_setreg32(UINT8 reg, UINT32 data)
{
	h8.regs[reg] = data;
}

static STATE_POSTLOAD( h8_onstateload )
{
	h8_set_ccr(h8.ccr);
}

static CPU_INIT( h8 )
{
	memset(&h8, 0, sizeof(h8));
	h8.h8iflag = 1;

	h8.irq_cb = irqcallback;
	h8.device = device;

	state_save_register_item("H8/3002", device->tag, 0, h8.h8err);
	state_save_register_item_array("H8/3002", device->tag, 0, h8.regs);
	state_save_register_item("H8/3002", device->tag, 0, h8.pc);
	state_save_register_item("H8/3002", device->tag, 0, h8.ppc);
	state_save_register_item("H8/3002", device->tag, 0, h8.h8_IRQrequestH);
	state_save_register_item("H8/3002", device->tag, 0, h8.h8_IRQrequestL);
	state_save_register_item("H8/3002", device->tag, 0, h8.ccr);

	state_save_register_item_array("H8/3002", device->tag, 0, h8.per_regs);
	state_save_register_item("H8/3002", device->tag, 0, h8.h8TSTR);
	state_save_register_item_array("H8/3002", device->tag, 0, h8.h8TCNT);

	state_save_register_postload(device->machine, h8_onstateload, NULL);

	h8_itu_init();
}

static CPU_INIT( h8_3007 )
{
	CPU_INIT_CALL(h8);
	h8_3007_itu_init();
}

static CPU_RESET( h8 )
{
	h8.h8err = 0;
	h8.pc = h8_mem_read32(0) & 0xffffff;
	change_pc(h8.pc);

	// disable timers
	h8.h8TSTR = 0;

	h8_itu_reset();
}

static void h8_GenException(UINT8 vectornr)
{
	// push PC on stack
	// extended mode stack push!
	h8_setreg32(H8_SP, h8_getreg32(H8_SP)-4);
	h8_mem_write32(h8_getreg32(H8_SP), h8.pc);
	// push ccr
	h8_setreg32(H8_SP, h8_getreg32(H8_SP)-2);
	h8_mem_write16(h8_getreg32(H8_SP), h8_get_ccr());

	// generate address from vector
	h8_set_ccr(h8_get_ccr() | 0x80);
	if (h8.h8uiflag == 0)
		h8_set_ccr(h8_get_ccr() | 0x40);
	h8.pc = h8_mem_read32(vectornr * 4) & 0xffffff;
	change_pc(h8.pc);

	// I couldn't find timing info for exceptions, so this is a guess (based on JSR/BSR)
	H8_IFETCH_TIMING(2);
	H8_STACK_TIMING(2);
}

static int h8_get_priority(UINT8 bit)
{
	int res = 0;
	switch(bit)
	{
	case 12: // IRQ0
		if (h8.per_regs[0xF8]&0x80) res = 1; break;
	case 13: // IRQ1
		if (h8.per_regs[0xF8]&0x40) res = 1; break;
	case 14: // IRQ2
	case 15: // IRQ3
		if (h8.per_regs[0xF8]&0x20) res = 1; break;
	case 16: // IRQ4
	case 17: // IRQ5
		if (h8.per_regs[0xF8]&0x10) res = 1; break;
	}
	return res;
}

static void h8_check_irqs(void)
{
	int lv = -1;
	if (h8.h8iflag == 0)
	{
		lv = 0;
	}
	else
	{
		if ((h8.per_regs[0xF2]&0x08)/*SYSCR*/ == 0)
		{
			if (h8.h8uiflag == 0)
				lv = 1;
		}
	}

	// any interrupts wanted and can accept ?
	if(((h8.h8_IRQrequestH != 0) || (h8.h8_IRQrequestL != 0)) && (lv >= 0))
	{
		UINT8 bit, source;
		// which one ?
		for(bit = 0, source = 0xff; source == 0xff && bit < 32; bit++)
		{
			if( h8.h8_IRQrequestL & (1<<bit) )
			{
				if (h8_get_priority(bit) >= lv)
				{
					// mask off
					h8.h8_IRQrequestL &= ~(1<<bit);
					source = bit;
				}
			}
		}
		// which one ?
		for(bit = 0; source == 0xff && bit < 32; bit++)
		{
			if( h8.h8_IRQrequestH & (1<<bit) )
			{
				if (h8_get_priority(bit + 32) >= lv)
				{
					// mask off
					h8.h8_IRQrequestH &= ~(1<<bit);
					source = bit + 32;
				}
			}
		}

		// call the MAME callback if it's one of the 6
		// external IRQs
		if (source >= 12 && source <= 17)
		{
			(*h8.irq_cb)(h8.device, source - 12 + H8_IRQ0);
		}

		if (source != 0xff)
			h8_GenException(source);
	}
}

static CPU_EXECUTE( h8 )
{
	UINT16 opcode=0;

	h8_cyccnt = cycles;

 	h8_check_irqs();

	while ((h8_cyccnt > 0) && (!h8.h8err))
	{
		h8.ppc = h8.pc;

		debugger_instruction_hook(device->machine, h8.pc);

		opcode = cpu_readop16(h8.pc);
//      mame_printf_debug("[%06x]: %04x => %x\n", h8.pc, opcode, (opcode>>12)&0xf);
		h8.pc += 2;

		switch((opcode>>12) & 0xf)
		{
		case 0x0:
			h8_group0(opcode);
			break;
		case 0x1:
			h8_group1(opcode);
			break;
		case 0x2:
			// mov.b @xx:8, Rd (abs)
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_mem_read8(0xffff00+(opcode & 0xff));
			h8_mov8(udata8); // flags calculation, dont care about others
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x3: // pass
			// mov.b Rs, @xx:8 (abs)
			srcreg = (opcode>>8) & 0xf;
			udata8 = h8_getreg8(srcreg);
			h8_mov8(udata8); // flags calculation, dont care about others
			h8_mem_write8(0xffff00+(opcode & 0xff), udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, 0xffff00+(opcode & 0xff));
			break;
		case 0x4:
			// bcc @xx:8
			sdata8 = (opcode & 0xff);
			if( h8_branch((opcode >> 8) & 0xf) == 1)h8.pc += sdata8;
			change_pc(h8.pc);
			break;
		case 0x5:
			h8_group5(opcode);
			break;
		case 0x6:
			h8_group6(opcode);
			break;
		case 0x7:
			h8_group7(opcode);
			break;
		case 0x8:
			// add.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_add8(opcode & 0xff, h8_getreg8(dstreg));
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1)
			break;
		case 0x9:
			// addx.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_addx8(opcode & 0xff, h8_getreg8(dstreg));
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1)
			break;
		case 0xa:
			// cmp.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			h8_cmp8(opcode & 0xff, h8_getreg8(dstreg));
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// subx.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_subx8(opcode & 0xff, h8_getreg8(dstreg));
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xc:
			// or.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_or8(opcode & 0xff, h8_getreg8(dstreg));
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// xor.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_xor8(opcode & 0xff, h8_getreg8(dstreg));
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xe: // pass
			// and.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_and8(opcode & 0xff, h8_getreg8(dstreg));
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1)
			break;
		case 0xf: // pass
			// mov.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_mov8(opcode & 0xff);
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		}
	}

	if (h8.h8err)
	{
		fatalerror("H8/3002: Unknown opcode (PC=%x)  %x", h8.ppc, opcode);

	}

	return cycles - h8_cyccnt;
}

static void h8_group0(UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0:
		// nop
		H8_IFETCH_TIMING(1);
		break;
	case 0x1:
		// 010x  where x should always be 0!
		if((opcode & 0xf) != 0)
		{
			h8.h8err = 1;
			break;
		}
		switch((opcode>>4) & 0xf)
		{
			// 0100 mov.l prefix
		case 0xf:     // and.l Rn, Rn
			ext16 = h8_mem_read16(h8.pc);
			h8.pc += 2;
			if (ext16 & 0x88)
			{
				h8.h8err = 1;
			}
			else
			{
				dstreg = ext16 & 0x7;
				switch((ext16>>8)&0xff)
				{
				case 0x64:	// or.l ERs, ERd
					udata32 = h8_or32(h8_getreg32((ext16>>4) & 0x7), h8_getreg32(dstreg));
					break;
				case 0x65:	// xor.l ERs, ERd
					udata32 = h8_xor32(h8_getreg32((ext16>>4) & 0x7), h8_getreg32(dstreg));
					break;
				case 0x66:	// and.l ERs, ERd
					udata32 = h8_and32(h8_getreg32((ext16>>4) & 0x7), h8_getreg32(dstreg));
					break;
				default:
					h8.h8err = 1;
					return;
				}
				h8_setreg32(dstreg, udata32);
				H8_IFETCH_TIMING(2);
			}
			break;

		case 0:
			ext16 = h8_mem_read16(h8.pc);
			h8.pc+=2;
			switch((ext16 >> 8) & 0xff)
			{
			case 0x69:
				if((ext16 & 0x80) == 0x80)
				{
					// mov.l rx, @rx
					udata32 = h8_mov32(h8_getreg32(ext16 & 7));
					h8_mem_write32(h8_getreg32((ext16 >> 4) & 7), udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, h8_getreg32((ext16 >> 4) & 7));
				}
				else
				{
					// mov.l @rx, rx
					udata32 = h8_mem_read32(h8_getreg32( (ext16 >> 4) &7));
					h8_mov32(udata32);
					h8_setreg32(ext16 & 7, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, h8_getreg32( (ext16 >> 4) &7));
				}
				break;
			case 0x6b:
				// mov.l rx, @xx / mov.l @xx, rx
				switch((ext16 >> 4)&0xf)
				{
				case 0x2:
					// mov.l @aa:24, erx
					address24=h8_mem_read32(h8.pc);
					h8.pc += 4;
					udata32=h8_mem_read32(address24);
					h8_mov32(udata32); // flags only
					h8_setreg32(ext16 & 0x7, udata32);
					H8_IFETCH_TIMING(4);
					H8_WORD_TIMING(2, address24);
					break;
				case 0xa:
					// mov.l erx, @aa:24
					address24=h8_mem_read32(h8.pc);
					h8.pc += 4;
					udata32=h8_getreg32(ext16 & 0x7);
					h8_mov32(udata32); // flags only
					h8_mem_write32(address24, udata32);
					H8_IFETCH_TIMING(4);
					H8_WORD_TIMING(2, address24);
					break;
				default:
					h8.h8err = 1;
					break;
				}
				break;
			case 0x6d:
				if(ext16 & 0x80)
				{
					// mov.l rs, @-erd
					srcreg = (ext16>>4)&7;
					h8_setreg32(srcreg, h8_getreg32(srcreg)-4);
					address24 = h8_getreg32(srcreg) & 0xffffff;
					udata32 = h8_getreg32(ext16 & 0x7);
					h8_mem_write32(address24, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, address24);
					H8_IOP_TIMING(2);
				}
				else
				{
					// mov.l @ers+, rd
					srcreg = (ext16 >>4)&7;
					address24 = h8_getreg32(srcreg) & 0xffffff;
					h8_setreg32(srcreg, h8_getreg32(srcreg)+4);
					udata32 = h8_mem_read32(address24);
					h8_setreg32(ext16 & 0x7, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, address24);
					H8_IOP_TIMING(2);
				}
				h8_mov32(udata32);
				break;
			case 0x6f:
				// mov.l @(displ16 + Rs), rd
				sdata16=h8_mem_read16(h8.pc); // sign extend displacements !
				h8.pc += 2;
				address24 = (h8_getreg32((ext16 >> 4)&7)) & 0xffffff;
				address24 += sdata16;
				H8_IFETCH_TIMING(3);
				H8_WORD_TIMING(2, address24);
				if(ext16 & 0x80)
				{
					udata32 = h8_getreg32(ext16 & 0x7);
					h8_mem_write32(address24, udata32);
				}
				else
				{
					udata32 = h8_mem_read32(address24);
					h8_setreg32(ext16 & 0x7, udata32);
				}
				h8_mov32(udata32);
				break;
			case 0x78:
				// prefix for
				// mov.l (@aa:x, rx), Rx
				//00000A10 010078606B2600201AC2 MOV.L   @($00201AC2,ER6),ER6
				// mov.l @(displ24 + Rs), rd
				srcreg = (ext16 >> 4) & 7;

				// 6b20
				udata16 = h8_mem_read16(h8.pc);
				h8.pc += 2;
				dstreg = udata16 & 7;

				address24 = h8_mem_read32(h8.pc);
				h8.pc += 4;
				address24 += h8_getreg32(srcreg);
				address24 &= 0xffffff;

				if ( (ext16 & 0x80) && ((udata16 & ~7) == 0x6ba0) )
				{
					udata32 = h8_getreg32(dstreg);
					h8_mem_write32(address24, udata32);
				}
				else if ( (!(ext16 & 0x80)) && ((udata16 & ~7) == 0x6b20) )
				{
					udata32 = h8_mem_read32(address24);
					h8_setreg32(dstreg, udata32);
				}
				else
				{
					h8.h8err = 1;
				}

				h8_mov32(udata32);

				H8_IFETCH_TIMING(5);
				H8_WORD_TIMING(2, address24);

				break;
			default:
				h8.h8err = 1;
				break;
			}
			break;
		case 0xc:
			// mulxs
			ext16 = h8_mem_read16(h8.pc);
			h8.pc+=2;
			if(((ext16>>8) & 0xf) == 0)
			{
				h8.h8err = 1;
			}
			else if(((ext16>>8) & 0xf) == 2)
			{
				sdata32 = h8_getreg32(ext16 & 0x7);
				sdata16 = h8_getreg16((ext16>>4) & 0xf);
				sdata32 = h8_mulxs16(sdata16, sdata32);
				h8_setreg32(ext16 & 0x7, sdata32);
				H8_IFETCH_TIMING(2);
				H8_IOP_TIMING(20);
			}
			else
			{
				logerror("H8/3002: Unk. group 0 mulxs %x\n", opcode);
				h8.h8err = 1;
			}
			break;

		case 0xd:
			//divxs - probbaly buggy (flags?)
			ext16 = h8_mem_read16(h8.pc);
			h8.pc+=2;
			if(((ext16>>8) & 0xf) == 0)
			{
				h8.h8err = 1;
			}
			else if(((ext16>>8) & 0xf) == 3)
			{
				sdata32 = h8_getreg32(ext16 & 0x7);
				sdata16 = h8_getreg16((ext16>>4) & 0xf);
				sdata32 = h8_divxs16(sdata16, sdata32);
				h8_setreg32(ext16 & 0x7, sdata32);
				H8_IFETCH_TIMING(2);
				H8_IOP_TIMING(20);
			}
			else
			{
				h8.h8err = 1;
			}


		break;

		default:
			h8.h8err = 1;
			break;
		}
		break;
	case 0x2:
		// stc ccr, rd
		if(((opcode>>4) & 0xf) == 0)
		{
			h8_setreg8(opcode & 0xf, h8_get_ccr());
			H8_IFETCH_TIMING(1);
		}
		else
		{
			logerror("H8/3002: Unk. group 0 2 %x\n", opcode);
			h8.h8err = 1;
		}
		break;
	case 0x3:
		// ldc rd, ccr
		if(((opcode>>4) & 0xf) == 0)
		{
			udata8 = h8_getreg8(opcode & 0xf);
			h8_set_ccr(udata8);
			H8_IFETCH_TIMING(1);
		}
		else
		{
			logerror("H8/3002: Unk. group 0 3 %x\n", opcode);
			h8.h8err = 1;
		}
		break;
	case 0x4: // pass
		// orc
		udata8 = h8_or8(opcode & 0xff, h8_get_ccr());
		h8_set_ccr(udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x6:
		// andc
		udata8 = h8_and8(opcode & 0xff, h8_get_ccr());
		h8_set_ccr(udata8);
		H8_IFETCH_TIMING(1)
		break;
		// ldc
	case 0x8:
		// add.b rx, ry
		dstreg = opcode & 0xf;
		udata8 = h8_add8(h8_getreg8((opcode>>4) &0xf), h8_getreg8(dstreg));
		h8_setreg8(dstreg, udata8);
		H8_IFETCH_TIMING(1)
		break;
		// add.w rx, ry
	case 0x9:
		dstreg = opcode & 0xf;
		udata16 = h8_add16(h8_getreg16((opcode>>4) &0xf), h8_getreg16(dstreg));
		h8_setreg16(dstreg, udata16);
		H8_IFETCH_TIMING(1)
		break;
		// inc.b rx
	case 0xA:
		if(opcode&0x80)
		{
			if(opcode & 0x8)
			{
				logerror("H8/3002: Unk. group 0 a %x\n", opcode);
				h8.h8err = 1;
			}
			else
			{
				dstreg = opcode & 0x7;
				udata32 = h8_add32(h8_getreg32((opcode>>4) &0x7), h8_getreg32(dstreg));
				h8_setreg32(dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
		}
		else
		{
			if(opcode & 0xf0)
			{
			logerror("H8/3002: Unk. group 0 a2 %x\n", opcode);
				h8.h8err =1;
			}
			else
			{
				dstreg = opcode & 0xf;
				udata8 = h8_inc8(h8_getreg8(dstreg));
				h8_setreg8(dstreg, udata8);
				H8_IFETCH_TIMING(1);
			}
		}
		break;
	case 0xb:
		switch((opcode>>4)& 0xf)
		{
		case 0:
			if(opcode & 8)
			{
				h8.h8err = 1;
			}
			else
			{
				dstreg = opcode & 7;
				udata32 = h8_getreg32(dstreg) + 1;
				h8_setreg32(dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
			break;
		case 5:
			dstreg = opcode & 0xf;
			udata16 = h8_inc16(h8_getreg16(dstreg));
			h8_setreg16(dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 7:
			dstreg = opcode & 0x7;
			udata32 = h8_inc32(h8_getreg32(dstreg));
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 8:
			if(opcode & 8)
			{
				h8.h8err = 1;
			}
			else
			{
				dstreg = opcode & 7;
				udata32 = h8_getreg32(dstreg) + 2;
				h8_setreg32(dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
			break;
		case 9:
			if(opcode & 8)
			{
				h8.h8err = 1;
			}
			else
			{
				dstreg = opcode & 7;
				udata32 = h8_getreg32(dstreg) + 4;
				h8_setreg32(dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
			break;
		case 0xd:
			dstreg = opcode & 0xf;
			udata16 = h8_inc16(h8_getreg16(dstreg));
			if(h8.h8vflag)
			{
				udata16 = h8_inc16(udata16); // slow and easy
				h8.h8vflag = 1;
			}
			else
				udata16 = h8_inc16(udata16); // slow and easy
			h8_setreg16(dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:
			dstreg = opcode & 0x7;
			udata32 = h8_inc32(h8_getreg32(dstreg));
			if(h8.h8vflag)
			{
				udata32 = h8_inc32(udata32); // slow and easy
				h8.h8vflag = 1;
			}
			else
				udata32 = h8_inc32(udata32); // slow and easy
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		default:
			logerror("H8/3002: Unk. group 0 b %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
		// mov.b rx, ry
	case 0xc: // pass
		dstreg = opcode & 0xf;
		udata8 = h8_mov8(h8_getreg8((opcode>>4) &0xf));
		h8_setreg8(dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0xd:
		// mov.w rx, ry
		dstreg = opcode & 0xf;
		udata16 = h8_mov16(h8_getreg16((opcode>>4) &0xf));
		h8_setreg16(dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
	case 0xf:
		if(opcode & 0x80)
		{
			if(opcode & 8)
			{
			logerror("H8/3002: Unk. group 0 f %x\n", opcode);
				h8.h8err = 1;
			}
			else
			{
				dstreg = opcode & 0x7;
				udata32 = h8_mov32(h8_getreg32((opcode>>4) &0x7));
				h8_setreg32(dstreg, udata32);
				H8_IFETCH_TIMING(1);
			}
		}
		else
		{
			h8.h8err = 1;
			logerror("H8/3002: Unk. group 0 f2 %x\n", opcode);
			if((opcode & 0xf0) !=0)
			{
				h8.h8err = 1;
			}
			else
			{
				h8.h8err = 1;
			}
		}
		break;
	default:
		logerror("H8/3002: Unk. group 0 tdef %x\n", opcode);
		h8.h8err = 1;
		break;
	}
}

static void h8_group1(UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0x0:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// shll.b Rx
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_shll8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// shll.w Rx
			udata16 = h8_getreg16(opcode & 0xf);
			udata16 = h8_shll16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0x3:
			// shal.l Rx
			udata32 = h8_getreg32(opcode & 0x7);
			udata32 = h8_shll32(udata32);
			h8_setreg32(opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;

		case 0x8:
			// shal.b Rx
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_shal8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// shal.w Rx
			udata16 = h8_getreg16(opcode & 0xf);
			udata16 = h8_shal16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// shal.l ERx
			udata32 = h8_getreg32(opcode & 0x7);
			udata32 = h8_shal32(udata32);
			h8_setreg32(opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;

		default:
			logerror("H8/3002: Unk. group 1 0 %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0x1:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// shlr.b rx
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_shlr8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// shlr.w rx
			udata16 = h8_getreg16(opcode & 0xf);
			udata16 = h8_shlr16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
			// shlr.l rx
		case 0x3:
			udata32 = h8_getreg32(opcode & 0x7);
			udata32 = h8_shlr32(udata32);
			h8_setreg32(opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0x8:
			// shar.b rx
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_shar8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// shar.w rx
			udata16 = h8_getreg16(opcode & 0xf);
			udata16 = h8_shar16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// shar.l rx
			udata32 = h8_getreg32(opcode & 0x7);
			udata32 = h8_shar32(udata32);
			h8_setreg32(opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		default:
			logerror("H8/3002: Unk. group 1 1 %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0x2:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// rotxl.b Rx
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_rotxl8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// rotxl.w Rx
			udata16 = h8_getreg16(opcode & 0xf);
			udata16 = h8_rotxl16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;

		case 0x3:
			// rotxl.l Rx
			udata32 = h8_getreg32(opcode & 0x7);
			udata32 = h8_rotxl32(udata32);
			h8_setreg32(opcode & 0xf, udata32);
			H8_IFETCH_TIMING(1);
			break;

		case 0x8:
			// rotl.b Rx
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_rotl8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// rotl.w Rx
			udata16 = h8_getreg16(opcode & 0xf);
			udata16 = h8_rotl16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// rotl.l ERx
			udata32 = h8_getreg32(opcode & 0x7);
			udata32 = h8_rotl32(udata32);
			h8_setreg32(opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;

		default:
			logerror("H8/3002: Unk. group 1 2 %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0x3:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// rotxr.b Rx
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_rotxr8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// rotxr.w Rx
			udata16 = h8_getreg16(opcode & 0xf);
			udata16 = h8_rotxr16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		default:
			logerror("H8/3002: Unk. group 1 3 %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0x4:
		// or.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_or8(h8_getreg8((opcode>>4) &0xf), h8_getreg8(dstreg));
		h8_setreg8(dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x5:
		// xor.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_xor8(h8_getreg8((opcode>>4) &0xf), h8_getreg8(dstreg));
		h8_setreg8(dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x6:
		// and.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_and8(h8_getreg8((opcode>>4) &0xf), h8_getreg8(dstreg));
		h8_setreg8(dstreg, udata8);
		H8_IFETCH_TIMING(1)
		break;
		// not
	case 0x7:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// not.b Rx
			dstreg = opcode & 0xf;
			udata8 = h8_not8(h8_getreg8(dstreg));
			h8_setreg8(dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// not.w Rx
			dstreg = opcode & 0xf;
			udata16 = h8_not16(h8_getreg16(dstreg));
			h8_setreg16(dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0x3:
			// not.l ERx
			dstreg = opcode & 0x7;
			udata32 = h8_not32(h8_getreg32(dstreg));
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;

		case 0x5:
			// extu.w Rx
			dstreg = opcode & 0xf;
			udata16 = h8_getreg16(dstreg) & 0x00ff;
			h8_setreg16(dstreg, udata16);
			h8.h8nflag = 0;
			h8.h8vflag = 0;
			h8.h8zflag = ((udata16 == 0) ? 1 : 0);
			H8_IFETCH_TIMING(1);
			break;
		case 0x7:
			// extu.l Rx
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(dstreg) & 0x0000ffff;
			h8_setreg32(dstreg, udata32);
			h8.h8nflag = 0;
			h8.h8vflag = 0;
			h8.h8zflag = ((udata32 == 0) ? 1 : 0);
			H8_IFETCH_TIMING(1);
			break;
		case 0x8:
			// neg.b Rx
			dstreg = opcode & 0xf;
			sdata8 = h8_neg8(h8_getreg8(dstreg));
			h8_setreg8(dstreg, sdata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// neg.w Rx
			dstreg = opcode & 0xf;
			sdata16 = h8_neg16(h8_getreg16(dstreg));
			h8_setreg16(dstreg, sdata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// neg.l ERx
			dstreg = opcode & 0x7;
			sdata32 = h8_neg32(h8_getreg32(dstreg));
			h8_setreg32(dstreg, sdata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// exts.w Rx
			dstreg = opcode & 0xf;
			udata16=h8_getreg16(dstreg)&0xff;
			if(udata16&0x80)
			{
				udata16|=0xff00;
			}
			h8_setreg16(dstreg, udata16);

			h8.h8vflag = 0;
			h8.h8nflag = (udata16 & 0xff00) ? 1 : 0;
			h8.h8zflag = (udata16) ? 0 : 1;

			H8_IFETCH_TIMING(1);
			break;

		case 0xf:
			// exts.l Rx
			dstreg = opcode & 0x7;
			udata32=h8_getreg32(dstreg)&0xffff;
			if(udata32&0x8000)
			{
				udata32|=0xffff0000;
			}
			h8_setreg32(dstreg, udata32);

			h8.h8vflag = 0;
			h8.h8nflag = (udata32 & 0xffff0000) ? 1 : 0;
			h8.h8zflag = (udata32) ? 0 : 1;

			H8_IFETCH_TIMING(1);
			break;

		default:
			logerror("H8/3002: Unk. group 1 7-9 %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0x8:
		// sub.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_sub8(h8_getreg8((opcode>>4) &0xf), h8_getreg8(dstreg));
		h8_setreg8(dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x9:
		// sub.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_sub16(h8_getreg16((opcode>>4) &0xf), h8_getreg16(dstreg));
		h8_setreg16(dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
		// sub.b rx
	case 0xA:
		if(opcode&0x80)
		{
			//logerror("H8/3002: Unk. group 1 A %x\n", opcode);

			// sub.l rs,rd
			dstreg = opcode & 0x7;
			udata32=h8_sub32(h8_getreg32((opcode>>4) &0x7), h8_getreg32(dstreg));
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(2);
			break;

		}
		else
		{
			if(opcode & 0xf0)
			{
				logerror("H8/3002: Unk. group A2 0 %x\n", opcode);
				h8.h8err = 1;
			}
			else
			{
				udata8 = h8_getreg8(opcode & 0xf);
				udata8 = h8_dec8(udata8);
				h8_setreg8(opcode & 0xf, udata8);
				H8_IFETCH_TIMING(1);
			}
		}
		break;
		//
	case 0xb:
		switch((opcode>>4)& 0xf)
		{
		case 0:	// subs.l #1, rN (decrement without touching flags)
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(dstreg);
			udata32--;
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 5:	// dec.w #1, rN
			dstreg = opcode & 0xf;
			udata16 = h8_dec16(h8_getreg16(dstreg));
			h8_setreg16(dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 7:	// dec.l #1, rN
			dstreg = opcode & 0x7;
			udata32 = h8_dec32(h8_getreg32(dstreg));
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 8:	// subs.l #2, rN (decrement without touching flags)
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(dstreg);
			udata32-=2;
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 9:	// subs.l #4, rN (decrement without touching flags)
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(dstreg);
			udata32-=4;
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:	// dec.w #2, rN
			dstreg = opcode & 0xf;
			udata16 = h8_dec16(h8_getreg16(dstreg));
			if (h8.h8vflag)
			{
				udata16 = h8_dec16(udata16);
				h8.h8vflag = 1;
			}
			else
				udata16 = h8_dec16(udata16);
			h8_setreg16(dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:	// dec.l #2, rN
			dstreg = opcode & 0x7;
			udata32 = h8_dec32(h8_getreg32(dstreg));
			if (h8.h8vflag)
			{
				udata32 = h8_dec32(udata32);
				h8.h8vflag = 1;
			}
			else
				udata32 = h8_dec32(udata32);
			h8_setreg32(dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;

		default:
			logerror("H8/3002: Unk. group 1 B %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0xc:
		// cmp.b rs, rd
		h8_cmp8(h8_getreg8((opcode>>4) &0xf), h8_getreg8(opcode & 0xf));
		H8_IFETCH_TIMING(1);
		break;
	case 0xd:
		// cmp.w rx, ry
		h8_cmp16(h8_getreg16((opcode>>4) &0xf), h8_getreg16(opcode & 0xf));
		H8_IFETCH_TIMING(1);
		break;
	case 0xe:
		// subx.b rx, ry
		dstreg = opcode & 0xf;
		udata8 = h8_subx8(h8_getreg8((opcode>>4) &0xf), h8_getreg8(dstreg));
		h8_setreg8(dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0xf:
		if(opcode & 0x80)
		{
			if(opcode & 8)
			{
				logerror("H8/3002: Unk. group 1 f %x\n", opcode);
				h8.h8err = 1;
			}
			else
			{
				h8_cmp32(h8_getreg32((opcode>>4) & 0x7), h8_getreg32(opcode & 0x7));
				H8_IFETCH_TIMING(1);
			}
		}
		else
		{
			logerror("H8/3002: Unk. group 1 f2 %x\n", opcode);
			h8.h8err = 1;
		}
		break;
	default:
		logerror("H8/3002: Unk. group 1 def %x\n", opcode);
		h8.h8err = 1;
		break;
	}
}


static void h8_group5(UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0x0:
		// mulxu.b
		udata8 = h8_getreg8((opcode>>4)&0xf);
		udata16 = h8_getreg16(opcode & 0xf);
		udata16 &= 0xff;
		udata16 = udata16*udata8;
		// no flags modified!
		h8_setreg16(opcode & 0xf, udata16);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(12);
		break;
	case 0x1:
		// divxu.b
		udata8 = h8_getreg8((opcode>>4)&0xf);
		udata16 = h8_getreg16(opcode & 0x0f);
		udata16 = h8_divxu8(udata16,udata8);
		h8_setreg16(opcode & 0xf, udata16);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(12);
		break;
	case 0x2:
		// mulxu.w
		udata16 = h8_getreg16((opcode>>4)&0xf);
		udata32 = h8_getreg32(opcode & 7);
		udata32 &= 0xffff;
		udata32 = udata32*udata16;
		// no flags modified!
		h8_setreg32(opcode & 7, udata32);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(20);
		break;
	case 0x3:
		// divxu.w
		udata16 = h8_getreg16((opcode>>4)&0xf);
		udata32 = h8_getreg32(opcode & 7);
		udata32 = h8_divxu16(udata32,udata16);
		h8_setreg32(opcode & 7, udata32);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(20);
		break;
	case 0x4:
		if(opcode == 0x5470)
		{
			// rts
			udata32 = h8_mem_read32(h8_getreg32(H8_SP));
			h8_setreg32(H8_SP, h8_getreg32(H8_SP)+4);
			// extended mode
			h8.pc = udata32;
			change_pc(h8.pc);
			H8_IFETCH_TIMING(2);
			H8_STACK_TIMING(2);
			H8_IOP_TIMING(2);
		}
		else
		{
			logerror("H8/3002: Unk. group 5 1 %x\n", opcode);
			h8.h8err = 1;
		}
		break;
	case 0x5:
		// bsr 8
		sdata8 = opcode & 0xff;
		// extended mode stack push!
		h8_setreg32(H8_SP, h8_getreg32(H8_SP)-4);
		h8_mem_write32(h8_getreg32(H8_SP), h8.pc);
		h8.pc = h8.pc + sdata8;
		change_pc(h8.pc);
		H8_IFETCH_TIMING(2); H8_STACK_TIMING(2);
		break;
	case 0x6:
		// rte
		if(opcode == 0x5670)
		{
			// restore CCR
			udata16 = h8_mem_read16(h8_getreg32(H8_SP));
			h8_setreg32(H8_SP, h8_getreg32(H8_SP)+2);
			// extended mode restore PC
			udata32 = h8_mem_read32(h8_getreg32(H8_SP));
			h8_setreg32(H8_SP, h8_getreg32(H8_SP)+4);
			// extended mode
			h8.pc = udata32;
			change_pc(h8.pc);
			// must do this last, because set_ccr() does a check_irq()
			h8_set_ccr((UINT8)udata16);
			H8_IFETCH_TIMING(2);
			H8_STACK_TIMING(2);
			H8_IOP_TIMING(2);
		}
		else
		{
			logerror("H8/3002: Unk. group 5 6 %x\n", opcode);
			h8.h8err = 1;
		}
		break;
		// trapa
	case 0x7:
		logerror("H8/3002: Unk. group 5 7 %x\n", opcode);
		h8.h8err = 1;
		break;
	case 0x8:
		// bcc @xx:16
		if(opcode & 0xf)
		{
			logerror("H8/3002: Unk. group 5 8 %x\n", opcode);
			h8.h8err = 1;
		}
		else
		{
			sdata16 = h8_mem_read16(h8.pc);
			h8.pc += 2;
			if( h8_branch((opcode >> 4) & 0xf) == 1)h8.pc += sdata16;
			change_pc(h8.pc);
			H8_IOP_TIMING(2)
		}
		break;
	case 0x9:
		// jmp @erd
		address24 = h8_getreg32((opcode>>4)&7);
		address24 &= 0xffffff;
		h8.pc = address24;
		change_pc(h8.pc);
		H8_IFETCH_TIMING(2);
		break;
		// jmp @aa:24
	case 0xa:
		address24=h8_mem_read32(h8.pc-2);
		address24 &= 0xffffff;
		h8.pc = address24;
		change_pc(h8.pc);
		H8_IFETCH_TIMING(2);
		H8_IOP_TIMING(2);
		break;
		// jmp @aa:8
	case 0xc:
		if(opcode & 0xff)
		{
			logerror("H8/3002: Unk. group 5 c %x\n", opcode);
			h8.h8err = 1;
		}
		else
		{
			// bsr d:16
			sdata16=h8_mem_read16(h8.pc);
			h8_setreg32(H8_SP, h8_getreg32(H8_SP)-4);
			h8_mem_write32(h8_getreg32(H8_SP), h8.pc+2);
			h8.pc += sdata16 + 2;
			change_pc(h8.pc);
			H8_IFETCH_TIMING(2); H8_STACK_TIMING(2); H8_IOP_TIMING(2);
		}
		break;
	case 0xd:
		// jsr @reg
		address24=h8_getreg32((opcode>>4)&7);
		address24 &= 0xffffff;
		// extended mode stack push!
		h8_setreg32(H8_SP, h8_getreg32(H8_SP)-4);
		h8_mem_write32(h8_getreg32(H8_SP), h8.pc);
		h8.pc = address24;
		change_pc(h8.pc);
		H8_STACK_TIMING(2);
		H8_IOP_TIMING(2);
		break;
	case 0xe:
		// jsr @aa:24
		address24=h8_mem_read32(h8.pc-2);
		address24 &= 0xffffff;
		// extended mode stack push!
		h8_setreg32(H8_SP, h8_getreg32(H8_SP)-4);
		h8_mem_write32(h8_getreg32(H8_SP), h8.pc+2);
		h8.pc = address24;
		change_pc(h8.pc);
		H8_IFETCH_TIMING(2);
		H8_STACK_TIMING(2);
		H8_IOP_TIMING(2);
		break;
		// jsr @aa:8
	default:
		logerror("H8/3002: Unk. group 5 def %x\n", opcode);
		h8.h8err = 1;
		break;
	}
}

static void h8_group6(UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0:case 1:case 2:case 3:
		{
			UINT8 bitnr;

			dstreg = opcode & 0xf;
			udata8 = h8_getreg8(dstreg);
			bitnr = h8_getreg8((opcode>>4)& 0xf)&7;

			switch((opcode>>8)&0xf)
			{
			case 0:	udata8 = h8_bset8(bitnr, udata8); h8_setreg8(dstreg, udata8); H8_IFETCH_TIMING(1); break;
			case 2:	udata8 = h8_bclr8(bitnr, udata8); h8_setreg8(dstreg, udata8); H8_IFETCH_TIMING(1); break;
			case 3:	h8_btst8(bitnr, udata8); H8_IFETCH_TIMING(1); break;
			default:
				logerror("H8/3002: Unk. group 6 def 0-3-0 %x\n", opcode);
				h8.h8err = 1;
				break;
			}
		}
		break;
	case 0x4:
		// or.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_getreg16(dstreg);
		udata16 = h8_or16(h8_getreg16((opcode>>4) & 0xf), udata16);
		h8_setreg16(dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
	case 0x5:
		// xor.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_getreg16(dstreg);
		udata16 = h8_xor16(h8_getreg16((opcode>>4) & 0xf), udata16);
		h8_setreg16(dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
	case 0x6:
		// and.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_getreg16(dstreg);
		udata16 = h8_and16(h8_getreg16((opcode>>4) & 0xf), udata16);
		h8_setreg16(dstreg, udata16);
		H8_IFETCH_TIMING(1)
		break;
	case 0x7:
		// bst/bist #imm, rd
		if(opcode & 0x80)
		{
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_bist8((opcode>>4) & 7, udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
		}
		else
		{
			udata8 = h8_getreg8(opcode & 0xf);
			udata8 = h8_bst8((opcode>>4) & 7, udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
		}
		break;
	case 0x8:
		if(opcode & 0x80)
		{
			// mov.b rx, @rx
			udata8 = h8_getreg8(opcode & 0xf);
			address24 = h8_getreg32((opcode>>4)&7) & 0xffffff;
			h8_mov8(udata8);
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
		}
		else
		{
			// mov.b @rx, rx
			address24 = h8_getreg32((opcode>>4)&7) & 0xffffff;
			udata8 = h8_mem_read8(address24);
			h8_mov8(udata8);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
		}
		break;
	case 0x9:
		if(opcode & 0x80)
		{
			// mov.w rx, @rx
			address24 = h8_getreg32((opcode>>4)&7) & 0xffffff;
			udata16 = h8_getreg16(opcode & 0xf);
			h8_mov16(udata16);
			h8_mem_write16(address24, udata16);
			H8_IFETCH_TIMING(1);
			H8_WORD_TIMING(1, address24);
		}
		else
		{
			// mov.w @rx, rx
			address24 = h8_getreg32((opcode>>4)&7) & 0xffffff;
			udata16 = h8_mem_read16(address24);
			h8_mov16(udata16);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			H8_WORD_TIMING(1, address24);
		}
		break;
	case 0xa:
		// mov.b rx, @xx
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			sdata16=h8_mem_read16(h8.pc);
			h8.pc += 2;
			address24 = sdata16 & 0xffffff;
			udata8=h8_mem_read8(address24);
			h8_mov8(udata8); // flags only
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			break;
		case 0x2:
			address24=h8_mem_read32(h8.pc);
			h8.pc += 4;
			udata8=h8_mem_read8(address24);
			h8_mov8(udata8); // flags only
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(2);
			H8_BYTE_TIMING(1, address24);
			break;
		case 0x8:
			sdata16=h8_mem_read16(h8.pc);
			h8.pc += 2;
			address24 = sdata16 & 0xffffff;
			udata8=h8_getreg8(opcode & 0xf);
			h8_mov8(udata8); // flags only
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(3);
			H8_BYTE_TIMING(1, address24);
			break;
		case 0xa: // pass
			address24=h8_mem_read32(h8.pc);
			h8.pc += 4;
			udata8=h8_getreg8(opcode & 0xf);
			h8_mov8(udata8); // flags only
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(3);
			H8_BYTE_TIMING(1, address24);
			break;
		default:
			logerror("H8/3002: Unk. group 6 a %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0xb:
		// mov.w rx, @xx / mov.w @xx, rx
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			sdata16=h8_mem_read16(h8.pc);
			address24 = sdata16;
			address24 &= 0xffffff;
			h8.pc += 2;
			udata16=h8_mem_read16(address24);
			h8_mov16(udata16); // flags only
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(2);
			H8_WORD_TIMING(1, address24);
			break;
		case 0x2:
			address24=h8_mem_read32(h8.pc);
			h8.pc += 4;
			udata16=h8_mem_read16(address24);
			h8_mov16(udata16); // flags only
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(4);
			H8_WORD_TIMING(1, address24);
			break;
		case 0x8:
			sdata16=h8_mem_read16(h8.pc);
			address24 = sdata16;
			address24 &= 0xffffff;
			h8.pc += 2;
			udata16=h8_getreg16(opcode & 0xf);
			h8_mov16(udata16); // flags only
			h8_mem_write16(address24, udata16);
			H8_IFETCH_TIMING(2);
			H8_WORD_TIMING(1, address24);
			break;
		case 0xa: // pass
			address24=h8_mem_read32(h8.pc);
			h8.pc += 4;
			udata16=h8_getreg16(opcode & 0xf);
			h8_mov16(udata16); // flags only
			h8_mem_write16(address24, udata16);
			H8_IFETCH_TIMING(4);
			H8_WORD_TIMING(1, address24);
			break;
		default:
			logerror("H8/3002: Unk. group 6b %x\n", opcode);
			h8.h8err = 1;
			break;
		}
		break;
	case 0xc:
		if(opcode & 0x80)
		{
			// mov.b rx, @-erx
			srcreg = (opcode>>4)&7;
			h8_setreg32(srcreg, h8_getreg32(srcreg)-1);
			address24 = h8_getreg32(srcreg) & 0xffffff;
			udata8 = h8_getreg8(opcode & 0xf);
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		else
		{
			// mov.b @erx+,rx
			srcreg = (opcode>>4)&7;
			address24 = h8_getreg32(srcreg) & 0xffffff;
			h8_setreg32(srcreg, h8_getreg32(srcreg)+1);
			udata8 = h8_mem_read8(address24);
			h8_setreg8(opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		h8_mov8(udata8);
		break;
	case 0xd:
		if(opcode & 0x80)
		{
			// mov.w rs, @-erd
			srcreg = (opcode>>4)&7;
			h8_setreg32(srcreg, h8_getreg32(srcreg)-2);
			address24 = h8_getreg32(srcreg) & 0xffffff;
			udata16 = h8_getreg16(opcode & 0xf);
			h8_mem_write16(address24, udata16);
			H8_IFETCH_TIMING(1);
			H8_WORD_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		else
		{
			// mov.w @ers+, rd
			srcreg = (opcode>>4)&7;
			address24 = h8_getreg32(srcreg) & 0xffffff;
			h8_setreg32(srcreg, h8_getreg32(srcreg)+2);
			udata16 = h8_mem_read16(address24);
			h8_setreg16(opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		h8_mov16(udata16);
		break;
	case 0xe: // pass
		// mov.b @(displ16 + Rs), rd
		sdata16=h8_mem_read16(h8.pc); // sign extend displacements !
		h8.pc += 2;
		address24 = (h8_getreg32((opcode>>4)&7)) & 0xffffff;
		address24 += sdata16;
		if(opcode & 0x80)
		{
			udata8 = h8_getreg8(opcode & 0xf);
			h8_mem_write8(address24, udata8);
		}
		else
		{
			udata8 = h8_mem_read8(address24);
			h8_setreg8(opcode & 0xf, udata8);
		}
		h8_mov8(udata8);
		H8_IFETCH_TIMING(2);
		H8_BYTE_TIMING(1, address24);
		break;
	case 0xf:
		// mov.w @(displ16 + Rs), rd
		sdata16=h8_mem_read16(h8.pc); // sign extend displacements !
		h8.pc += 2;
		address24 = (h8_getreg32((opcode>>4)&7)) & 0xffffff;
		address24 += sdata16;
		if(opcode & 0x80)
		{
			udata16 = h8_getreg16(opcode & 0xf);
			h8_mem_write16(address24, udata16);
		}
		else
		{
			udata16 = h8_mem_read16(address24);
			h8_setreg16(opcode & 0xf, udata16);
		}
		h8_mov16(udata16);
		H8_IFETCH_TIMING(2);
		H8_WORD_TIMING(1, address24);
		break;
	default:
		logerror("H8/3002: Unk. group 6 def %x\n", opcode);
		h8.h8err = 1;
		break;
	}
}

static void h8_group7(UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:
		{
			UINT8 bitnr;

			dstreg = opcode & 0xf;
			udata8 = h8_getreg8(dstreg);
			bitnr = (opcode>>4)&7;

			if(((opcode>>4)&0x8) == 0)
			{
				switch((opcode>>8)&7)
				{
				case 0:	udata8 = h8_bset8(bitnr, udata8); h8_setreg8(dstreg, udata8); H8_IFETCH_TIMING(1); break;
				case 2:	udata8 = h8_bclr8(bitnr, udata8); h8_setreg8(dstreg, udata8);H8_IFETCH_TIMING(1);break;
				case 3:	h8_btst8(bitnr, udata8); H8_IFETCH_TIMING(1); break;
				case 7:	h8_bld8(bitnr, udata8); H8_IFETCH_TIMING(1); break;
				default:
					logerror("H8/3002: Unk. group 7 0-7 def %x\n", opcode);
					h8.h8err = 1;
					break;
				}
			}
			else
			{
				switch((opcode>>8)&7)
				{
				default:
					logerror("H8/3002: Unk. group 7 0-7-1 def %x\n", opcode);
					h8.h8err = 1;
					break;
				}
			}
		}
		break;
	case 0x8:
		ext16 = h8_mem_read16(h8.pc);
		h8.pc += 2;
		udata32 = h8_mem_read32(h8.pc);
		h8.pc += 4;

		if(((ext16>>8) & 0xf) == 0xa)
		{
			if(((ext16>>4) & 0xf) == 0xa)
			{
				udata8 = h8_getreg8(ext16 & 0xf);
				h8_mov8(udata8); // update flags !
				udata32 += h8_getreg32((opcode >> 4) & 7);
				h8_mem_write8(udata32, udata8);
			}
			else
			{
				udata32 += h8_getreg32((opcode >> 4) & 7);
				udata8 = h8_mem_read8(udata32);
				h8_mov8(udata8); // update flags !
				h8_setreg8(ext16 & 0xf, udata8);
			}
			H8_BYTE_TIMING(1, udata32);
			H8_IFETCH_TIMING(4);
		}
		else if ((ext16 & 0xfff0) == 0x6b20) // mov.w @(24-bit direct, rN), rM
		{
			udata32 += h8_getreg32((opcode >> 4) & 7);
			udata16 = h8_mem_read16(udata32);
			h8_setreg16(ext16 & 0xf, udata16);
			h8_mov16(udata16); // update flags !
			H8_WORD_TIMING(1, udata32);
			H8_IFETCH_TIMING(4);
		}
		else if ((ext16 & 0xfff0) == 0x6ba0) // mov.w rM, @(24-bit direct, rN)
		{
			udata32 += h8_getreg32((opcode >> 4) & 7);
			udata16 = h8_getreg16(ext16 & 0xf);
			h8_mem_write16(udata32, udata16);
			h8_mov16(udata16); // update flags !
			H8_WORD_TIMING(1, udata32);
			H8_IFETCH_TIMING(4);
		}
		else
		{
			logerror("H8/3002: Unk. group 7 8 %x\n", opcode);
			h8.h8err = 1;
		}
		break;


		// xxx.w #aa:16, rd
	case 0x9:
		if( ((opcode>>4) & 0xf) > 0x6)
		{
			logerror("H8/3002: Unk. group 7 9 %x\n", opcode);
			h8.h8err = 1;
		}
		else
		{
			UINT16 dst16;
			udata16 = h8_mem_read16(h8.pc);
			h8.pc += 2;
			dstreg = opcode&0xf;
			dst16 = h8_getreg16(dstreg);

			switch((opcode>>4)&7)
			{
			case 0:	dst16 = h8_mov16(udata16); h8_setreg16(dstreg, dst16); H8_IFETCH_TIMING(2);break;
			case 1: dst16 = h8_add16(udata16, dst16); h8_setreg16(dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 2: h8_cmp16(udata16, dst16); H8_IFETCH_TIMING(2); break;
			case 3: dst16 = h8_sub16(udata16, dst16); h8_setreg16(dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 4: dst16 = h8_or16(udata16, dst16); h8_setreg16(dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 5: dst16 = h8_xor16(udata16, dst16); h8_setreg16(dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 6: dst16 = h8_and16(udata16, dst16); h8_setreg16(dstreg, dst16); H8_IFETCH_TIMING(2); break;
			default:
				logerror("H8/3002: Unk. group 7 9 %x\n", opcode);
				h8.h8err = 1;
				break;
			}
		}
		break;
		// xxx.l #aa:32, erd
	case 0xa:
		if( (((opcode>>4) & 0xf) > 0x6) || (opcode & 0x8))
		{
			logerror("H8/3002: Unk. group 7 a %x\n", opcode);
			h8.h8err = 1;
		}
		else
		{
			UINT32 dst32;
			udata32 = h8_mem_read32(h8.pc);
			dstreg = opcode&0x7;
			h8.pc +=4;
			dst32 = h8_getreg32(dstreg);

			switch((opcode>>4)&7)
			{
			case 0:	dst32 = h8_mov32(udata32); h8_setreg32(dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 1: dst32 = h8_add32(udata32, dst32); h8_setreg32(dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 2: h8_cmp32(udata32, dst32); H8_IFETCH_TIMING(3); break;
			case 3: dst32 = h8_sub32(udata32, dst32); h8_setreg32(dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 4: dst32 = h8_or32(udata32, dst32);  h8_setreg32(dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 5: dst32 = h8_xor32(udata32, dst32); h8_setreg32(dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 6: dst32 = h8_and32(udata32, dst32); h8_setreg32(dstreg, dst32); H8_IFETCH_TIMING(3); break;
			default:
				logerror("H8/3002: Unk. group 7 a2 %x\n", opcode);
				h8.h8err = 1;
				break;
			}
		}
		break;
		// eepmov
	case 0xb:
		if ((opcode & 0xff) == 0xd4)
		{
			UINT16 cnt = h8_getreg16(4);

			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING((2*cnt)+2, h8.regs[5]);

			// eepmov.w
			while (cnt > 0)
			{
				h8_mem_write8(h8.regs[6], h8_mem_read8(h8.regs[5]));
				h8.regs[5]++;
				h8.regs[6]++;
				cnt--;
			}
			h8_setreg16(4, 0);
			h8.pc += 2;
		}
		else
		{
			logerror("H8/3002: Unk. eepmov form\n");
			h8.h8err = 1;
		}
		break;
		// bxx.b #xx:3, @rd
	case 0xc:
		{
			UINT8 bitnr;

			address24 = h8_getreg32((opcode>>4) & 0x7);
			udata8 = h8_mem_read8(address24);
			H8_BYTE_TIMING(1, address24);

			ext16 = h8_mem_read16(h8.pc);
			h8.pc += 2;
			H8_IFETCH_TIMING(2);

			switch(ext16>>8)
			{
				// BTST Rn,@ERd
				case 0x63:
					srcreg = (ext16>>4)&0xf;
					bitnr = h8_getreg8(srcreg)&7;
					h8_btst8(bitnr, udata8);
					break;
				// btst.b #imm, @Rn
				case 0x73:
					bitnr = (ext16>>4)&7;
					h8_btst8(bitnr, udata8);
					break;
				default:
					h8.h8err=1;
			}
		}
		break;
	case 0xd:
		ext16 = h8_mem_read16(h8.pc);
		h8.pc += 2;
		address24 = h8_getreg32((opcode>>4) & 0x7);
		H8_IFETCH_TIMING(2);
		H8_BYTE_TIMING(1, address24);
		switch(ext16>>8)
		{
			// bset/bnot/bclr.b Rn, @ERd
			case 0x60:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8.h8err=1;	break;	}
				h8_mem_write8(address24, h8_bset8(h8_getreg16((ext16>>4)&0xf)&7, h8_mem_read8(address24)));
				break;
			case 0x61:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8.h8err=1;	break;	}
				h8_mem_write8(address24, h8_bnot8(h8_getreg16((ext16>>4)&0xf)&7, h8_mem_read8(address24)));
				break;
			case 0x62:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8.h8err=1;	break;	}
				h8_mem_write8(address24, h8_bclr8(h8_getreg16((ext16>>4)&0xf)&7, h8_mem_read8(address24)));
				break;

			case 0x67:	// bst/bist.b #Imm:3, @ERd
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8.h8err=1;	break;	}
				if ((ext16 & 0x80)!=0)
				{
					h8_mem_write8(address24, h8_bist8((ext16>>4)&7, h8_mem_read8(address24)));
				}
				else
				{
					h8_mem_write8(address24, h8_bst8((ext16>>4)&7, h8_mem_read8(address24)));
				}
				break;

			// bset/bnot/bclr.b #Imm:3, @ERd
			case 0x70:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x8f)!=0))	{	h8.h8err=1;	break;	}
				h8_mem_write8(address24, h8_bset8((ext16>>4)&7, h8_mem_read8(address24)));
				break;
			case 0x71:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x8f)!=0))	{	h8.h8err=1;	break;	}
				h8_mem_write8(address24, h8_bnot8((ext16>>4)&7, h8_mem_read8(address24)));
				break;
			case 0x72:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x8f)!=0))	{	h8.h8err=1;	break;	}
				h8_mem_write8(address24, h8_bclr8((ext16>>4)&7, h8_mem_read8(address24)));
				break;
		}
		break;

		// bxxx.b #imm, @aa:8
	case 0xe:
	case 0xf:
		{
			UINT8 bitnr=0;
			ext16 = h8_mem_read16(h8.pc);
			h8.pc += 2;
			address24 = 0xffff00 + (opcode & 0xff);
			udata8 = h8_mem_read8(address24);

			switch((ext16>>8)&0xff)
			{
			case 0x30:
			case 0x60:
				bitnr = (ext16>>4)&7;
				udata8 = h8_bset8(bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x70:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80) h8.h8err = 1;
				udata8 = h8_bset8(bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x32:
			case 0x62:
				bitnr = h8_getreg8((ext16>>4)&0xf)&7;
				udata8 = h8_bclr8(bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x72:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80) h8.h8err = 1;
				udata8 = h8_bclr8(bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x63:
				bitnr = h8_getreg8((ext16>>4)&0xf)&7;
				h8_btst8(bitnr, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(1, address24);
				break;
			case 0x73:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80) h8.h8err = 1;
				h8_btst8(bitnr, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(1, address24);
				break;
			case 0x74:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80)
				{
					// bior
					h8.h8err = 1;
				}
				else
				{
					h8_bor8(bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(1, address24);
				}
				break;
			case 0x67:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80)
				{
					// bist
					h8.h8err = 1;
				}
				else
				{
					h8_bst8(bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				}
				break;
			case 0x77:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80)
				{
					// bild
					h8.h8err = 1;
				}
				else
				{
					h8_bld8(bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				}
				break;
			default:
				h8.h8err = 1;
				break;
			}
			if(h8.h8err)
				logerror("H8/3002: Unk. group 7 e %x\n", opcode);
		}
		break;
	default:
		logerror("H8/3002: Unk. group 7 def %x\n", opcode);
		h8.h8err = 1;
		break;
	}
}


static UINT8 h8_mov8(UINT8 src)
{
	// N and Z modified
	h8.h8nflag = (src>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(src==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return src;
}

static UINT16 h8_mov16(UINT16 src)
{
	// N and Z modified
	h8.h8nflag = (src>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(src==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}
	return src;
}

static UINT32 h8_mov32(UINT32 src)
{
	// N and Z modified
	h8.h8nflag = (src>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(src==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return src;
}

static UINT8 h8_sub8(UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)dst - src;
	// H,N,Z,V,C modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = (((src^dst) & (res^dst))>>7) & 1;
	h8.h8cflag = (res >> 8) & 1;

	// zflag
	if((res&0xff)==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	h8.h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;

	return res;
}

static UINT16 h8_sub16(UINT16 src, UINT16 dst)
{
	UINT32 res;

	res = (UINT32)dst - src;
	// H,N,Z,V,C modified
	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = (((src^dst) & (res^dst))>>15) & 1;
	h8.h8cflag = (res >> 16) & 1;
	//  h8.h8hflag = (res>>28) & 1;

	// zflag
	if((res&0xffff)==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	h8.h8hflag = ((src^dst^res) & 0x1000) ? 1 : 0;

	return res;
}

static UINT32 h8_sub32(UINT32 src, UINT32 dst)
{
	UINT64 res;

	res = (UINT64)dst - src;
	// H,N,Z,V,C modified
	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = (((src^dst) & (res^dst))>>31) & 1;
	h8.h8cflag = (res >> 32) & 1;
	//  h8.h8hflag = (res>>28) & 1;

	// zflag
	if((res&0xffffffff)==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	h8.h8hflag = ((src^dst^res) & 0x10000000) ? 1 : 0;

	return res;
}




static UINT8 h8_add8(UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)src + dst;
	// H,N,Z,V,C modified
	h8.h8nflag = (res & 0x80) ? 1 : 0;
	h8.h8vflag = ((src^res) & (dst^res) & 0x80) ? 1 : 0;
	h8.h8cflag = (res & 0x100) ? 1 : 0;
	h8.h8zflag = (res & 0xff) ? 0 : 1;
	h8.h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;

	return (UINT8)res;
}

static UINT16 h8_add16(UINT16 src, UINT16 dst)
{
	UINT32 res;

	res = (UINT32)src + dst;
	// H,N,Z,V,C modified
	h8.h8nflag = (res & 0x8000) ? 1 : 0;
	h8.h8vflag = ((src^res) & (dst^res) & 0x8000) ? 1 : 0;
	h8.h8cflag = (res & 0x10000) ? 1 : 0;
	h8.h8zflag = (res & 0xffff) ? 0 : 1;
	h8.h8hflag = ((src^dst^res) & 0x1000) ? 1 : 0;

	return res;
}

static UINT32 h8_add32(UINT32 src, UINT32 dst)
{
	UINT64 res;

	res = (UINT64)src + dst;
	// H,N,Z,V,C modified
	h8.h8nflag = (res & 0x80000000) ? 1 : 0;
	h8.h8vflag = (((src^res) & (dst^res)) & 0x80000000) ? 1 : 0;
	h8.h8cflag = ((res) & (((UINT64)1) << 32)) ? 1 : 0;
	h8.h8zflag = (res & 0xffffffff) ? 0 : 1;
	h8.h8hflag = ((src^dst^res) & 0x10000000) ? 1 : 0;

	return res;
}


static UINT8 h8_addx8(UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)src + dst + h8.h8cflag;
	// H,N,Z,V,C modified
	h8.h8nflag = (res & 0x80) ? 1 : 0;
	h8.h8vflag = ((src^res) & (dst^res) & 0x80) ? 1 : 0;
	h8.h8cflag = (res >> 8) & 1;
	h8.h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;
	h8.h8zflag = (res & 0xff) ? 0 : h8.h8zflag;

	return (UINT8)res;
}

static void h8_cmp8(UINT8 src, UINT8 dst)
{
	UINT16 res = (UINT16)dst - src;

	h8.h8cflag = (res & 0x100) ? 1 : 0;
	h8.h8vflag = (((dst) ^ (src)) & ((dst) ^ (res)) & 0x80) ? 1 : 0;
	h8.h8zflag = ((res & 0xff) == 0) ? 1 : 0;
	h8.h8nflag = (res & 0x80) ? 1 : 0;
	h8.h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;
}

static void h8_cmp16(UINT16 src, UINT16 dst)
{
	UINT32 res = (UINT32)dst - src;

	h8.h8cflag = (res & 0x10000) ? 1 : 0;
	h8.h8vflag = (((dst) ^ (src)) & ((dst) ^ (res)) & 0x8000) ? 1 : 0;
	h8.h8zflag = ((res & 0xffff) == 0) ? 1 : 0;
	h8.h8nflag = (res & 0x8000) ? 1 : 0;
	h8.h8hflag = ((src^dst^res) & 0x1000) ? 1 : 0;
}

static void h8_cmp32(UINT32 src, UINT32 dst)
{
	UINT64 res = (UINT64)dst - src;

	h8.h8cflag = (res & (UINT64)U64(0x100000000)) ? 1 : 0;
	h8.h8vflag = (((dst) ^ (src)) & ((dst) ^ (res)) & 0x80000000) ? 1 : 0;
	h8.h8zflag = ((res & 0xffffffff) == 0) ? 1 : 0;
	h8.h8nflag = (res & 0x80000000) ? 1 : 0;
	h8.h8hflag = ((src^dst^res) & 0x10000000) ? 1 : 0;
}


static UINT8 h8_subx8(UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)dst - src - (h8.h8cflag) ? 1 : 0;
	// H,N,Z,V,C modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = (((src^dst) & (res^dst))>>7) & 1;
	h8.h8cflag = (res >> 8) & 1;

	// zflag
	if((res&0xff)==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	h8.h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;

	return res;
}

static UINT8 h8_or8(UINT8 src, UINT8 dst)
{
	UINT8 res;
	res = src | dst;

	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_or16(UINT16 src, UINT16 dst)
{
	UINT16 res;
	res = src | dst;

	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_or32(UINT32 src, UINT32 dst)
{
	UINT32 res;
	res = src | dst;

	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_xor8(UINT8 src, UINT8 dst)
{
	UINT8 res;
	res = src ^ dst;

	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_xor16(UINT16 src, UINT16 dst)
{
	UINT16 res;
	res = src ^ dst;

	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_xor32(UINT32 src, UINT32 dst)
{
	UINT32 res;
	res = src ^ dst;

	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_and8(UINT8 src, UINT8 dst)
{
	UINT8 res;

	res = src & dst;
	// N and Z modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_and16(UINT16 src, UINT16 dst)
{
	UINT16 res;

	res = src & dst;
	// N and Z modified
	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_and32(UINT32 src, UINT32 dst)
{
	UINT32 res;

	res = src & dst;
	// N and Z modified
	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static void h8_btst8(UINT8 bit, UINT8 dst)
{
	// test single bit and update Z flag
	if( (dst & (1<<bit)) == 0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}
}

static void h8_bld8(UINT8 bit, UINT8 dst)
{
	// load bit to carry
	h8.h8cflag = (dst >> bit) & 1;
}

static UINT8 h8_bnot8(UINT8 src, UINT8 dst)
{
	// invert single bit, no effect on C flag
	return dst ^ (1<<src);
}

static UINT8 h8_bst8(UINT8 src, UINT8 dst)
{
	UINT8 res;

	// store carry flag in bit position
	if(h8.h8cflag == 1)
	{
		res = dst | (1<<src);
	}
	else
	{
		res = dst & ~(1<<src); // mask off
	}
	return res;
}

static UINT8 h8_bist8(UINT8 src, UINT8 dst)
{
	UINT8 res;

	// store inverse of carry flag in bit position
	if(h8.h8cflag == 0)
	{
		res = dst | (1<<src);
	}
	else
	{
		res = dst & ~(1<<src); // mask off
	}
	return res;
}

static UINT8 h8_bset8(UINT8 src, UINT8 dst)
{
	// pass
	UINT8 res;
	res = dst | (1<<src);
	return res;
}

// does not affect result, res in C flag only
static void h8_bor8(UINT8 src, UINT8 dst)
{
	// pass
	UINT8 res;

	res = dst & (1<<src);
	h8.h8cflag |= res ? 1 : 0;
}

#ifdef UNUSED_FUNCTION
static void h8_bxor8(UINT8 src, UINT8 dst)
{
	dst >>= src;
	dst &= 0x1;
	h8.h8cflag ^= dst;
}
#endif

static UINT8 h8_bclr8(UINT8 src, UINT8 dst)
{
	// pass
	UINT8 res;
	res = dst & ~(1<<src);
	return res;
}

static INT8 h8_neg8(INT8 src)
{
	INT8 res;

	if((UINT8)src == 0x80)
	{
		// overflow !
		h8.h8vflag = 1;
		res = 0x80;
	}
	else
	{
		h8.h8vflag = 0;
		res = 0-src;
	}

	// N and Z modified
	h8.h8nflag = (res>>7) & 1;
	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	h8.h8hflag = ((src|res)&0x08) ? 1 : 0;
	h8.h8cflag = ((src|res)&0x80) ? 1 : 0;

	return res;
}

static INT16 h8_neg16(INT16 src)
{
	INT16 res;

	if((UINT16)src == 0x8000)
	{
		// overflow !
		h8.h8vflag = 1;
		res = 0x8000;
	}
	else
	{
		h8.h8vflag = 0;
		res = 0-src;
	}

	// N and Z modified
	h8.h8nflag = (res>>15) & 1;
	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	h8.h8hflag = ((src|res)&0x0800) ? 1 : 0;
	h8.h8cflag = ((src|res)&0x8000) ? 1 : 0;

	return res;
}

static INT32 h8_neg32(INT32 src)
{
	INT32 res;

	if((UINT32)src == 0x80000000)
	{
		// overflow !
		h8.h8vflag = 1;
		res = 0x80000000;
	}
	else
	{
		h8.h8vflag = 0;
		res = 0-src;
	}

	// N and Z modified
	h8.h8nflag = (res>>31) & 1;
	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	h8.h8hflag = ((src|res)&0x08000000) ? 1 : 0;
	h8.h8cflag = ((src|res)&0x80000000) ? 1 : 0;

	return res;
}

static UINT8 h8_not8(UINT8 src)
{
	UINT8 res;

	res = ~src;

	// N and Z modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_not16(UINT16 src)
{
	UINT16 res;

	res = ~src;

	// N and Z modified
	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_not32(UINT32 src)
{
	UINT32 res;

	res = ~src;

	// N and Z modified
	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_rotxr8(UINT8 src)
{
	UINT8 res;

	// rotate through carry right
	res = src>>1;
	if(h8.h8cflag)res |= 0x80; // put cflag in upper bit
	h8.h8cflag = src & 1;

	// N and Z modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_rotxr16(UINT16 src)
{
	UINT16 res;

	// rotate through carry right
	res = src>>1;
	if(h8.h8cflag)res |= 0x8000; // put cflag in upper bit
	h8.h8cflag = src & 1;

	// N and Z modified
	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_rotxl8(UINT8 src)
{
	UINT8 res;

	// rotate through carry
	res = src<<1;
	res |= (h8.h8cflag & 1);
	h8.h8cflag = (src>>7) & 1;

	// N and Z modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_rotxl16(UINT16 src)
{
	UINT16 res;

	// rotate through carry
	res = src<<1;
	res |= (h8.h8cflag & 1);
	h8.h8cflag = (src>>15) & 1;

	// N and Z modified
	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_rotxl32(UINT32 src)
{
	UINT32 res;

	// rotate through carry
	res = src<<1;
	res |= (h8.h8cflag & 1);
	h8.h8cflag = (src>>31) & 1;

	// N and Z modified
	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}


static UINT8 h8_rotl8(UINT8 src)
{
	UINT8 res;

	// rotate
	res = src<<1;
	h8.h8cflag = (src>>7) & 1;
	res |= (h8.h8cflag & 1);

	// N and Z modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_rotl16(UINT16 src)
{
	UINT16 res;

	// rotate
	res = src<<1;
	h8.h8cflag = (src>>15) & 1;
	res |= (h8.h8cflag & 1);

	// N and Z modified
	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_rotl32(UINT32 src)
{
	UINT32 res;

	// rotate
	res = src<<1;
	h8.h8cflag = (src>>31) & 1;
	res |= (h8.h8cflag & 1);

	// N and Z modified
	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_shll8(UINT8 src)
{
	UINT8 res;
	h8.h8cflag = (src>>7) & 1;
	res = src<<1;
	// N and Z modified
	h8.h8nflag = (res>>7) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_shll16(UINT16 src)
{
	UINT16 res;
	h8.h8cflag = (src>>15) & 1;
	res = src<<1;
	// N and Z modified
	h8.h8nflag = (res>>15) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_shll32(UINT32 src)
{
	UINT32 res;
	h8.h8cflag = (src>>31) & 1;
	res = src<<1;
	// N and Z modified
	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_shlr8(UINT8 src)
{
	UINT8 res;
	h8.h8cflag = src&1;
	res = src>>1;
	// N and Z modified
	h8.h8nflag = 0;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_shlr16(UINT16 src)
{
	UINT16 res;
	h8.h8cflag = src&1;
	res = src>>1;
	// N and Z modified
	h8.h8nflag = 0;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_shlr32(UINT32 src)
{
	UINT32 res;
	h8.h8cflag = src&1;
	res = src>>1;

	// N and Z modified, V always cleared
	h8.h8nflag = 0;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static INT8 h8_shar8(INT8 src)
{
	INT8 res;
	h8.h8cflag = src&1;
	res = (src>>1)|(src&0x80);
	// N and Z modified
	h8.h8nflag = (res>>7)&1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static INT16 h8_shar16(INT16 src)
{
	INT16 res;
	h8.h8cflag = src&1;
	res = (src>>1)|(src&0x8000);
	// N and Z modified
	h8.h8nflag = (res>>15)&1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static INT32 h8_shar32(INT32 src)
{
	INT32 res;

	h8.h8cflag = src&1;
	res = (src>>1)|(src&0x80000000);
	// N and Z modified
	h8.h8nflag = (res>>31) & 1;
	h8.h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static INT8 h8_shal8(INT8 src)
{
	INT8 res;

	h8.h8cflag = (src>>7)&1;
	res = src<<1;
	// N and Z modified
	h8.h8nflag = (res>>7)&1;
	h8.h8vflag = (src ^ res) >> 7;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static INT16 h8_shal16(INT16 src)
{
	INT16 res;

	h8.h8cflag = (src>>15)&1;
	res = src<<1;
	// N and Z modified
	h8.h8nflag = (res>>15)&1;
	h8.h8vflag = (src ^ res) >> 15;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static INT32 h8_shal32(INT32 src)
{
	INT32 res;

	h8.h8cflag = (src>>31)&1;
	res = src<<1;
	// N and Z modified
	h8.h8nflag = (res>>31)&1;
	h8.h8vflag = (src ^ res) >> 31;

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_dec8(UINT8 src)
{
	UINT8 res;

	res = src - 1;
	// N and Z modified
	h8.h8nflag = (res>>7)&1;
	if(src == 0x80)
	{
		h8.h8vflag = 1;
	}
	else
	{
		h8.h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_dec16(UINT16 src)
{
	UINT16 res;

	res = src - 1;
	// N and Z modified
	h8.h8nflag = (res>>15)&1;
	if(src == 0x8000)
	{
		h8.h8vflag = 1;
	}
	else
	{
		h8.h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_dec32(UINT32 src)
{
	UINT32 res;

	res = src - 1;
	// N and Z modified
	h8.h8nflag = (res>>31)&1;
	if(src == 0x80000000)
	{
		h8.h8vflag = 1;
	}
	else
	{
		h8.h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT8 h8_inc8(UINT8 src)
{
	UINT8 res;

	res = src + 1;
	// N and Z modified
	h8.h8nflag = (res>>7)&1;
	if(src == 0x7f)
	{
		h8.h8vflag = 1;
	}
	else
	{
		h8.h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT16 h8_inc16(UINT16 src)
{
	UINT16 res;

	res = src + 1;
	// N and Z modified
	h8.h8nflag = (res>>15)&1;
	if(src == 0x7fff)
	{
		h8.h8vflag = 1;
	}
	else
	{
		h8.h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static UINT32 h8_inc32(UINT32 src)
{
	UINT32 res;

	res = src + 1;
	// N and Z modified
	h8.h8nflag = (res>>31)&1;
	if(src == 0x7fffffff)
	{
		h8.h8vflag = 1;
	}
	else
	{
		h8.h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}

	return res;
}

static INT32 h8_mulxs16(INT16 src, INT16 dst)
{
	INT32 res;

	res = (INT32)src * dst;

	// N and Z modified
	h8.h8nflag = (res>>31)&1;
	// zflag
	if(res==0)
	{
		h8.h8zflag = 1;
	}
	else
	{
		h8.h8zflag = 0;
	}
	return res;
}

static UINT32 h8_divxs16(INT16 src, INT32 dst)
{
	// NOT tested !
	UINT32 res,r1,r2;
	INT16 remainder, quotient;

	if(src!=0)
	{
		quotient = dst/src;
		h8.h8zflag = 0;
	}
	else
	{
		quotient = 0;
		h8.h8zflag = 1;
	}
	remainder = dst%src;

	r1=*(&quotient);
	r2=*(&remainder);
	res=(r2<<16)|r1;

	h8.h8nflag = (quotient<0)?1:0;

	return res;

}

static UINT16 h8_divxu8(UINT16 dst, UINT8 src)
{
	UINT8 remainder, quotient;
	UINT16 res = 0;
	// N and Z modified
	h8.h8nflag = (src>>7)&1;
	// zflag
	if(src==0)
	{
		h8.h8zflag = 1;
		// dont do anything on division by zero !
	}
	else
	{
		h8.h8zflag = 0;
		quotient = dst / src;
		remainder = dst % src;
		res = (remainder << 8) | quotient;
	}
	return res;
}

static UINT32 h8_divxu16(UINT32 dst, UINT16 src)
{
	UINT16 remainder, quotient;
	UINT32 res = 0;
	// N and Z modified
	h8.h8nflag = (src>>15)&1;
	// zflag
	if(src==0)
	{
		h8.h8zflag = 1;
		// dont do anything on division by zero !
	}
	else
	{
		h8.h8zflag = 0;
		quotient = dst / src;
		remainder = dst % src;
		res = (remainder << 16) | quotient;
	}
	return res;
}

// input: branch condition
// output: 1 if condition met, 0 if not condition met
static int h8_branch(UINT8 condition)
{
	int taken = 0;

	// a branch always eats 2 ifetch states, regardless of if it's taken
	H8_IFETCH_TIMING(2)

	switch(condition)
	{
	case 0: // bt
		taken = 1;
		break;
	case 1: // bf
		break;
	case 2: // bhi (C | Z) == 0)
		if((h8.h8cflag | h8.h8zflag) == 0)taken = 1;
		break;
	case 3: // bls
		if((h8.h8cflag | h8.h8zflag) == 1)taken = 1;
		break;
	case 4: // bcc C = 0
		if(h8.h8cflag == 0)taken = 1;
		break;
	case 5: // bcs C = 1
		if(h8.h8cflag == 1)taken = 1;
		break;
	case 6: // bne Z = 0
		if(h8.h8zflag == 0)taken = 1;
		break;
	case 7: // beq Z = 1
		if(h8.h8zflag == 1)taken = 1;
		break;
	case 8: // bvc V = 0
		h8.h8err = 1;
		if(h8.h8vflag == 0)taken = 1;
		break;
	case 9: // bvs V = 1
		h8.h8err = 1;
		if(h8.h8vflag == 1)taken = 1;
		break;
	case 0xa: // bpl N = 0
		if(h8.h8nflag == 0)taken = 1;
		break;
	case 0xb: // bmi N = 1
		if(h8.h8nflag == 1)taken = 1;
		break;
	case 0xc: // bge (N ^ V) = 0
		if((h8.h8nflag ^ h8.h8vflag) == 0)taken = 1;
		break;
	case 0xd: // blt (N ^ V) = 1
		if((h8.h8nflag ^ h8.h8vflag) == 1)taken = 1;
		break;
	case 0xe: // bgt (Z | (N ^ V)) = 0
		if((h8.h8zflag | (h8.h8nflag ^ h8.h8vflag)) == 0)taken = 1;
		break;
	case 0xf: // ble (Z | (N ^ V)) = 1
		if((h8.h8zflag | (h8.h8nflag ^ h8.h8vflag)) == 1)taken = 1;
		break;
	}
	return taken;
}

// MAME interface stuff

static CPU_GET_CONTEXT( h8 )
{
	*(h83002_state *)dst = h8;
}

static CPU_SET_CONTEXT( h8 )
{
	h8 = *(h83002_state *)src;
}

static CPU_SET_INFO( h8 )
{
	switch(state) {
	case CPUINFO_INT_PC:						h8.pc = info->i; change_pc(h8.pc);				break;
	case CPUINFO_INT_REGISTER + H8_PC:			h8.pc = info->i; change_pc(h8.pc);				break;
	case CPUINFO_INT_REGISTER + H8_CCR:			h8_set_ccr(info->i);							break;

	case CPUINFO_INT_REGISTER + H8_E0:			h8.regs[0] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E1:			h8.regs[1] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E2:			h8.regs[2] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E3:			h8.regs[3] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E4:			h8.regs[4] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E5:			h8.regs[5] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E6:			h8.regs[6] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E7:			h8.regs[7] = info->i;							break;

	case CPUINFO_INT_INPUT_STATE + H8_IRQ0:		if (info->i) h8_3002_InterruptRequest(12);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ1:		if (info->i) h8_3002_InterruptRequest(13);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ2:		if (info->i) h8_3002_InterruptRequest(14);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ3:		if (info->i) h8_3002_InterruptRequest(15);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ4:		if (info->i) h8_3002_InterruptRequest(16);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ5:		if (info->i) h8_3002_InterruptRequest(17);		break;

	default:
		fatalerror("h8_set_info unknown request %x", state);
		break;
	}
}

static READ16_HANDLER( h8_itu_r )
{
	if (mem_mask == 0xffff)
	{
		// 16-bit read
		return h8_register_read8(offset*2 + 0xffff10)<<8 | h8_register_read8((offset*2) + 1 + 0xffff10);
	}
	else if (mem_mask == 0xff00)
	{
		return h8_register_read8(offset*2 + 0xffff10)<<8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8_register_read8((offset*2) + 1 + 0xffff10);
	}

	return 0;
}

static WRITE16_HANDLER( h8_itu_w )
{
	if (mem_mask == 0xffff)
	{
		// 16-bit write
		h8_register_write8(offset*2 + 0xffff10, data>>8);
		h8_register_write8((offset*2) + 1 + 0xffff10, data&0xff);
	}
	else if (mem_mask == 0xff00)
	{
		h8_register_write8(offset*2 + 0xffff10, data>>8);
	}
	else if (mem_mask == 0x00ff)
	{
		h8_register_write8((offset*2) + 1 + 0xffff10, data&0xff);
	}
}

static READ16_HANDLER( h8_3007_itu_r )
{
	if (mem_mask == 0xffff)
	{
		// 16-bit read
		return h8_3007_register_read8(offset*2 + 0xffff20)<<8 | h8_3007_register_read8((offset*2) + 1 + 0xffff20);
	}
	else if (mem_mask == 0xff00)
	{
		return h8_3007_register_read8(offset*2 + 0xffff20)<<8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8_3007_register_read8((offset*2) + 1 + 0xffff20);
	}

	return 0;
}
static WRITE16_HANDLER( h8_3007_itu_w )
{
	if (mem_mask == 0xffff)
	{
		// 16-bit write
		h8_3007_register_write8(offset*2 + 0xffff20, data>>8);
		h8_3007_register_write8((offset*2) + 1 + 0xffff20, data&0xff);
	}
	else if (mem_mask == 0xff00)
	{
		h8_3007_register_write8(offset*2 + 0xffff20, data>>8);
	}
	else if (mem_mask == 0x00ff)
	{
		h8_3007_register_write8((offset*2) + 1 + 0xffff20, data&0xff);
	}
}

static READ16_HANDLER( h8_3007_itu1_r )
{
	if (mem_mask == 0xffff)
	{
		// 16-bit read
		return h8_3007_register1_read8(offset*2 + 0xfee000)<<8 | h8_3007_register1_read8((offset*2) + 1 + 0xfee000);
	}
	else if (mem_mask == 0xff00)
	{
		return h8_3007_register1_read8(offset*2 + 0xfee000)<<8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8_3007_register1_read8((offset*2) + 1 + 0xfee000);
	}

	return 0;
}
static WRITE16_HANDLER( h8_3007_itu1_w )
{
	if (mem_mask == 0xffff)
	{
		// 16-bit write
		h8_3007_register1_write8(offset*2 + 0xfee000, data>>8);
		h8_3007_register1_write8((offset*2) + 1 + 0xfee000, data&0xff);
	}
	else if (mem_mask == 0xff00)
	{
		h8_3007_register1_write8(offset*2 + 0xfee000, data>>8);
	}
	else if (mem_mask == 0x00ff)
	{
		h8_3007_register1_write8((offset*2) + 1 + 0xfee000, data&0xff);
	}
}

// On-board RAM and peripherals
static ADDRESS_MAP_START( h8_3002_internal_map, ADDRESS_SPACE_PROGRAM, 16 )
	// 512B RAM
	AM_RANGE(0xfffd10, 0xffff0f) AM_RAM
	AM_RANGE(0xffff10, 0xffffff) AM_READWRITE( h8_itu_r, h8_itu_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8_3044_internal_map, ADDRESS_SPACE_PROGRAM, 16 )
	// 32k ROM, 2k RAM
	AM_RANGE(0xfff710, 0xffff0f) AM_RAM
	AM_RANGE(0xffff1c, 0xffffff) AM_READWRITE( h8_itu_r, h8_itu_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8_3007_internal_map, ADDRESS_SPACE_PROGRAM, 16 )
	// ROM-less, 4k RAM
	AM_RANGE(0xfee000, 0xfee0ff) AM_READWRITE( h8_3007_itu1_r, h8_3007_itu1_w )
	AM_RANGE(0xffef20, 0xffff1f) AM_RAM
	AM_RANGE(0xffff20, 0xffffe9) AM_READWRITE( h8_3007_itu_r, h8_3007_itu_w )
ADDRESS_MAP_END


CPU_GET_INFO( h8_3002 )
{
	switch(state) {
	// Interface functions and variables
	case CPUINFO_PTR_SET_INFO:					info->setinfo     = CPU_SET_INFO_NAME(h8);		break;
	case CPUINFO_PTR_GET_CONTEXT:				info->getcontext  = CPU_GET_CONTEXT_NAME(h8);	break;
	case CPUINFO_PTR_SET_CONTEXT:				info->setcontext  = CPU_SET_CONTEXT_NAME(h8);	break;
	case CPUINFO_PTR_INIT:						info->init        = CPU_INIT_NAME(h8);			break;
	case CPUINFO_PTR_RESET:						info->reset       = CPU_RESET_NAME(h8);			break;
	case CPUINFO_PTR_EXIT:						info->exit        = 0;							break;
	case CPUINFO_PTR_EXECUTE:					info->execute     = CPU_EXECUTE_NAME(h8);		break;
	case CPUINFO_PTR_BURN:						info->burn        = 0;							break;
	case CPUINFO_PTR_DISASSEMBLE:				info->disassemble = CPU_DISASSEMBLE_NAME(h8);	break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:		info->icount      = &h8_cyccnt;					break;
	case CPUINFO_INT_CONTEXT_SIZE:				info->i           = sizeof(h83002_state);		break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i           = 2;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i           = 10;							break;

		// Bus sizes
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 24;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;						break;

		// Internal maps
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8_3002_internal_map); break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map16 = NULL;	break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_IO:      info->internal_map16 = NULL;	break;

		// CPU misc parameters
	case CPUINFO_STR_NAME:						strcpy(info->s, "H8/3002");						break;
	case CPUINFO_STR_CORE_FILE:					strcpy(info->s, __FILE__);						break;
	case CPUINFO_STR_FLAGS:						strcpy(info->s, h8_get_ccr_str());				break;
	case CPUINFO_INT_ENDIANNESS:				info->i = CPU_IS_BE;							break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1;									break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;									break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 16;									break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:		info->i = -1;									break;

		// CPU main state
	case CPUINFO_INT_PC:						info->i = h8.pc;								break;
	case CPUINFO_INT_PREVIOUSPC:				info->i = h8.ppc;								break;

	case CPUINFO_INT_REGISTER + H8_PC:			info->i = h8.pc;								break;
	case CPUINFO_INT_REGISTER + H8_CCR:			info->i = h8_get_ccr();							break;

	case CPUINFO_INT_REGISTER + H8_E0:			info->i = h8.regs[0];							break;
	case CPUINFO_INT_REGISTER + H8_E1:			info->i = h8.regs[1];							break;
	case CPUINFO_INT_REGISTER + H8_E2:			info->i = h8.regs[2];							break;
	case CPUINFO_INT_REGISTER + H8_E3:			info->i = h8.regs[3];							break;
	case CPUINFO_INT_REGISTER + H8_E4:			info->i = h8.regs[4];							break;
	case CPUINFO_INT_REGISTER + H8_E5:			info->i = h8.regs[5];							break;
	case CPUINFO_INT_REGISTER + H8_E6:			info->i = h8.regs[6];							break;
	case CPUINFO_INT_REGISTER + H8_E7:			info->i = h8.regs[7];							break;

	// CPU debug stuff
	case CPUINFO_STR_REGISTER + H8_PC:			sprintf(info->s, "PC   :%08x", h8.pc);			break;
	case CPUINFO_STR_REGISTER + H8_CCR:			sprintf(info->s, "CCR  :%08x", h8_get_ccr());	break;

	case CPUINFO_STR_REGISTER + H8_E0:			sprintf(info->s, "ER0  :%08x", h8.regs[0]);		break;
	case CPUINFO_STR_REGISTER + H8_E1:			sprintf(info->s, "ER1  :%08x", h8.regs[1]);		break;
	case CPUINFO_STR_REGISTER + H8_E2:			sprintf(info->s, "ER2  :%08x", h8.regs[2]);		break;
	case CPUINFO_STR_REGISTER + H8_E3:			sprintf(info->s, "ER3  :%08x", h8.regs[3]);		break;
	case CPUINFO_STR_REGISTER + H8_E4:			sprintf(info->s, "ER4  :%08x", h8.regs[4]);		break;
	case CPUINFO_STR_REGISTER + H8_E5:			sprintf(info->s, "ER5  :%08x", h8.regs[5]);		break;
	case CPUINFO_STR_REGISTER + H8_E6:			sprintf(info->s, "ER6  :%08x", h8.regs[6]);		break;
	case CPUINFO_STR_REGISTER + H8_E7:			sprintf(info->s, " SP  :%08x", h8.regs[7]);		break;
	}
}

CPU_GET_INFO( h8_3044 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8_3044_internal_map);  break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8/3044");	 break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}

CPU_GET_INFO( h8_3007 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map16 = address_map_h8_3007_internal_map;  break;
		case CPUINFO_PTR_INIT:				info->init = CPU_INIT_NAME(h8_3007);		break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8/3007");		break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}
