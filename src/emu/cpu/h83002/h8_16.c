/***************************************************************************

 h8_16.c: Hitachi H8/3xx series 16/32-bit microcontroller emulator

 Original by The_Author & DynaChicken for the ZiNc emulator.
 MAME changes by R. Belmont, Luca Elia, and Tomasz Slanina.

 TS 20060412 Added exts.l, sub.l, divxs.w (buggy), jsr @reg, rotxl.l reg, mov.l @(adr, reg), reg
 LE 20070903 Added divxu.b  shal.l  extu.w  dec.l #Imm,Rd  subx.b
 LE 20080202 Separated 3002/3044/3007, Added or.l  shal.l  rotl.l  not.l  neg.l  exts.w
             sub/or/xor.l #Imm:32,ERd  bset/bnot/bclr.b Rn,@ERd  bst/bist.b #Imm:3,@ERd  bnot.b #Imm:3,@ERd
 LE 20090128 Added mov.l ers,@aa:16;  bild #xx:3,rd;  eepmov.b;  bnot #xx:3,@aa:8

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h8.h"
#include "h8priv.h"

CPU_DISASSEMBLE(h8_24);
CPU_DISASSEMBLE(h8_32);

#define H8_SP	(7)

#define h8_mem_read8(x) h8->program->read_byte(x)
#define h8_mem_read16(z, x) h8->program->read_word(x)
#define h8_mem_write8(x, y)  h8->program->write_byte(x, y)
#define h8_mem_write16(z, x, y) h8->program->write_word(x, y)
#define h8_readop16(x, y) x->direct->read_decrypted_word(y)

// timing macros
// note: we assume a system 12 - type setup where external access is 3+1 states
// timing will be off somewhat for other configurations.
#define H8_IFETCH_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_BRANCH_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_STACK_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_BYTE_TIMING(x, adr)	if (address24 >= 0xffff10) h8->cyccnt -= (x) * 3; else h8->cyccnt -= (x) * 4;
#define H8_WORD_TIMING(x, adr)	if (address24 >= 0xffff10) h8->cyccnt -= (x) * 3; else h8->cyccnt -= (x) * 4;
#define H8_IOP_TIMING(x)	h8->cyccnt -= (x);

INLINE UINT32 h8_mem_read32(h83xx_state *h8, offs_t address)
{
	UINT32 result = h8->program->read_word(address) << 16;
	return result | h8->program->read_word(address + 2);
}

INLINE void h8_mem_write32(h83xx_state *h8, offs_t address, UINT32 data)
{
	h8->program->write_word(address, data >> 16);
	h8->program->write_word(address + 2, data);
}

static void h8_check_irqs(h83xx_state *h8);

/* implementation */

void h8_3002_InterruptRequest(h83xx_state *h8, UINT8 source, UINT8 state)
{
	int request = source / 32;
	int bit = source % 32;

	// don't allow clear on external interrupts
	if ((source <= 17) && (state == 0)) return;

	if (state)
	{
		h8->irq_req[request] |= (1<<bit);
	}
	else
	{
		h8->irq_req[request] &= ~(1<<bit);
	}
}


static UINT8 h8_get_ccr(h83xx_state *h8)
{
	h8->ccr = 0;
	if(h8->h8nflag)h8->ccr |= NFLAG;
	if(h8->h8zflag)h8->ccr |= ZFLAG;
	if(h8->h8vflag)h8->ccr |= VFLAG;
	if(h8->h8cflag)h8->ccr |= CFLAG;
	if(h8->h8uflag)h8->ccr |= UFLAG;
	if(h8->h8hflag)h8->ccr |= HFLAG;
	if(h8->h8uiflag)h8->ccr |= UIFLAG;
	if(h8->h8iflag)h8->ccr |= IFLAG;
	return h8->ccr;
}

static UINT8 h8_get_exr(h83xx_state *h8)
{
    return h8->exr;
}

static char *h8_get_ccr_str(h83xx_state *h8)
{
	static char res[10];

	memset(res, 0, sizeof(res));
	if(h8->h8iflag) strcat(res, "I"); else strcat(res, "i");
	if(h8->h8uiflag)strcat(res, "U"); else strcat(res, "u");
	if(h8->h8hflag) strcat(res, "H"); else strcat(res, "h");
	if(h8->h8uflag) strcat(res, "U"); else strcat(res, "u");
	if(h8->h8nflag) strcat(res, "N"); else strcat(res, "n");
	if(h8->h8zflag) strcat(res, "Z"); else strcat(res, "z");
	if(h8->h8vflag) strcat(res, "V"); else strcat(res, "v");
	if(h8->h8cflag) strcat(res, "C"); else strcat(res, "c");

	return res;
}

static void h8_set_ccr(h83xx_state *h8, UINT8 data)
{
	h8->ccr = data;

	h8->h8nflag = 0;
	h8->h8zflag = 0;
	h8->h8vflag = 0;
	h8->h8cflag = 0;
	h8->h8hflag = 0;
	h8->h8iflag = 0;
	h8->h8uflag = 0;
	h8->h8uiflag = 0;

	if(h8->ccr & NFLAG) h8->h8nflag = 1;
	if(h8->ccr & ZFLAG) h8->h8zflag = 1;
	if(h8->ccr & VFLAG) h8->h8vflag = 1;
	if(h8->ccr & CFLAG) h8->h8cflag = 1;
	if(h8->ccr & HFLAG) h8->h8hflag = 1;
	if(h8->ccr & UFLAG) h8->h8uflag = 1;
	if(h8->ccr & UIFLAG) h8->h8uiflag = 1;
	if(h8->ccr & IFLAG) h8->h8iflag = 1;

	if (!h8->incheckirqs) h8_check_irqs(h8);
}

static void h8_set_exr(h83xx_state *h8, UINT8 data)
{
	h8->exr = data;
}

static INT16 h8_getreg16(h83xx_state *h8, UINT8 reg)
{
	if(reg > 7)
	{
		return h8->regs[reg-8]>>16;
	}
	else
	{
		return h8->regs[reg];
	}
}

static void h8_setreg16(h83xx_state *h8, UINT8 reg, UINT16 data)
{
	if(reg > 7)
	{
		h8->regs[reg-8] &= 0xffff;
		h8->regs[reg-8] |= data<<16;
	}
	else
	{
		h8->regs[reg] &= 0xffff0000;
		h8->regs[reg] |= data;
	}
}

static UINT8 h8_getreg8(h83xx_state *h8, UINT8 reg)
{
	if(reg > 7)
	{
		return h8->regs[reg-8];
	}
	else
	{
		return h8->regs[reg]>>8;
	}
}

static void h8_setreg8(h83xx_state *h8, UINT8 reg, UINT8 data)
{
	if(reg > 7)
	{
		h8->regs[reg-8] &= 0xffffff00;
		h8->regs[reg-8] |= data;
	}
	else
	{
		h8->regs[reg] &= 0xffff00ff;
		h8->regs[reg] |= data<<8;
	}
}

static UINT32 h8_getreg32(h83xx_state *h8, UINT8 reg)
{
	return h8->regs[reg];
}

static void h8_setreg32(h83xx_state *h8, UINT8 reg, UINT32 data)
{
	h8->regs[reg] = data;
}

static void h8_onstateload(h83xx_state *h8)
{
	h8_set_ccr(h8, h8->ccr);
}

static CPU_INIT(h8)
{
	h83xx_state *h8 = get_safe_token(device);

	h8->h8iflag = 1;
	h8->irq_cb = irqcallback;
	h8->device = device;

	h8->mode_8bit = 0;

	h8->program = &device->space(AS_PROGRAM);
	h8->direct = &h8->program->direct();
	h8->io = &device->space(AS_IO);

	device->save_item(NAME(h8->h8err));
	device->save_item(NAME(h8->regs));
	device->save_item(NAME(h8->pc));
	device->save_item(NAME(h8->ppc));
	device->save_item(NAME(h8->irq_req));
	device->save_item(NAME(h8->ccr));
	device->save_item(NAME(h8->mode_8bit));

	device->save_item(NAME(h8->per_regs));
	device->save_item(NAME(h8->h8TSTR));
	device->save_item(NAME(h8->h8TCNT));

	device->machine().save().register_postload(save_prepost_delegate(FUNC(h8_onstateload), h8));

	h8_itu_init(h8);
}

static CPU_INIT(h8_3007)
{
	h83xx_state *h8 = get_safe_token(device);

	CPU_INIT_CALL(h8);
	h8_3007_itu_init(h8);
}

static CPU_INIT(h8s_2xxx)
{
	h83xx_state *h8 = get_safe_token(device);

	CPU_INIT_CALL(h8);

	h8s_tmr_init(h8);
	h8s_tpu_init(h8);
	h8s_sci_init(h8);
}

static CPU_RESET(h8)
{
	h83xx_state *h8 = get_safe_token(device);

	h8->h8err = 0;
	h8->pc = h8_mem_read32(h8, 0) & 0xffffff;

	h8->incheckirqs = 0;
    h8->exr = 0;
    h8->has_exr = false;

	// disable timers
	h8->h8TSTR = 0;

	h8_itu_reset(h8);

	h8->has_h8speriphs = false;
}

static CPU_RESET(h8s_2xxx)
{
	h83xx_state *h8 = get_safe_token(device);

	CPU_RESET_CALL(h8);

    h8->exr = 7;        // set the 3 interrupt bits, clear TRACE

	h8s_periph_reset(h8);
	h8->has_h8speriphs = true;

}

static CPU_RESET(h8s_2394)
{
	h83xx_state *h8 = get_safe_token(device);

	CPU_RESET_CALL(h8);

    h8->exr = 7;        // set the 3 interrupt bits, clear TRACE
    h8->has_exr = true;

    // port 4 is fixed to input only
    h8->drs[3] = 0;
    h8->ddrs[3] = 0;

	h8s_periph_reset(h8);
	h8->has_h8speriphs = true;

}

static void h8_GenException(h83xx_state *h8, UINT8 vectornr)
{
	// push PC on stack
	// extended mode stack push!
	h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)-4);
	h8_mem_write32(h8, h8_getreg32(h8, H8_SP), h8->pc);
	// push ccr
	h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)-2);
	h8_mem_write16(h8, h8_getreg32(h8, H8_SP), h8_get_ccr(h8));

	// generate address from vector
	h8_set_ccr(h8, h8_get_ccr(h8) | 0x80);
	if (h8->h8uiflag == 0)
		h8_set_ccr(h8, h8_get_ccr(h8) | 0x40);
	h8->pc = h8_mem_read32(h8, vectornr * 4) & 0xffffff;

	// I couldn't find timing info for exceptions, so this is a guess (based on JSR/BSR)
	H8_IFETCH_TIMING(2);
	H8_STACK_TIMING(2);
}

static int h8_get_priority(h83xx_state *h8, UINT8 bit)
{
	int res = 0;
	switch(bit)
	{
	case 12: // IRQ0
		if (h8->per_regs[0xF8]&0x80) res = 1; break;
	case 13: // IRQ1
		if (h8->per_regs[0xF8]&0x40) res = 1; break;
	case 14: // IRQ2
	case 15: // IRQ3
		if (h8->per_regs[0xF8]&0x20) res = 1; break;
	case 16: // IRQ4
	case 17: // IRQ5
		if (h8->per_regs[0xF8]&0x10) res = 1; break;
	case 53: // SCI0 Rx
		if (!(h8->per_regs[0xB2]&0x40)) res = -2;
		else if (h8->per_regs[0xF9]&0x08) res = 1; break;
	case 54: // SCI0 Tx Empty
		if (!(h8->per_regs[0xB2]&0x80)) res = -2;
		else if (h8->per_regs[0xF9]&0x08) res = 1; break;
	case 55: // SCI0 Tx End
		if (!(h8->per_regs[0xB2]&0x04)) res = -2;
		else if (h8->per_regs[0xF9]&0x08) res = 1; break;
	case 57: // SCI1 Rx
		if (!(h8->per_regs[0xBA]&0x40)) res = -2;
		else if (h8->per_regs[0xF9]&0x04) res = 1; break;
	case 58: // SCI1 Tx Empty
		if (!(h8->per_regs[0xBA]&0x80)) res = -2;
		else if (h8->per_regs[0xF9]&0x04) res = 1; break;
	case 59: // SCI1 Tx End
		if (!(h8->per_regs[0xBA]&0x04)) res = -2;
		else if (h8->per_regs[0xF9]&0x04) res = 1; break;
	}
	return res;
}

static void h8_check_irqs(h83xx_state *h8)
{
	int lv = -1;

	h8->incheckirqs = 1;

    if (h8->has_exr)
    {
        lv = (h8->exr & 7);
    }
    else
    {
        if (h8->h8iflag == 0)
        {
            lv = 0;
        }
        else
        {
            if ((h8->per_regs[0xF2]&0x08)/*SYSCR*/ == 0)
            {
                if (h8->h8uiflag == 0)
                    lv = 1;
            }
        }
    }

	// any interrupts wanted and can accept ?
	if(((h8->irq_req[0] != 0) || (h8->irq_req[1]!= 0) || (h8->irq_req[2] != 0)) && (lv >= 0))
	{
		UINT8 bit, source;
		// which one ?
		for(bit = 0, source = 0xff; source == 0xff && bit < 32; bit++)
		{
			if( h8->irq_req[0] & (1<<bit) )
			{
				if (h8_get_priority(h8, bit) >= lv)
				{
					// mask off
					h8->irq_req[0] &= ~(1<<bit);
					source = bit;
				}
			}
		}
		// which one ?
		for(bit = 0; source == 0xff && bit < 32; bit++)
		{
			if( h8->irq_req[1] & (1<<bit) )
			{
				if (h8_get_priority(h8, bit + 32) >= lv)
				{
					// mask off
					h8->irq_req[1] &= ~(1<<bit);
					source = bit + 32;
				}
			}
		}
		// which one ?
		for(bit = 0; source == 0xff && bit < 32; bit++)
		{
			if( h8->irq_req[2] & (1<<bit) )
			{
				if (h8_get_priority(h8, bit + 64) >= lv)
				{
					// mask off
					h8->irq_req[2] &= ~(1<<bit);
					source = bit + 64;
				}
			}
		}

		// call the MAME callback if it's one of the 6
		// external IRQs
		if (source >= 12 && source <= 17)
		{
			(*h8->irq_cb)(h8->device, source - 12 + H8_IRQ0);
		}

		if (source != 0xff)
		{
			if (h8->has_h8speriphs)
			{
				h8s_dtce_check(h8, source);
			}
			h8_GenException(h8, source);
		}
	}

	h8->incheckirqs = 0;
}

#define H8_ADDR_MASK 0xffffff
#include "h8ops.h"

// MAME interface stuff

static CPU_SET_INFO( h8 )
{
	h83xx_state *h8 = get_safe_token(device);

	switch(state) {
	case CPUINFO_INT_PC:						h8->pc = info->i;								break;
	case CPUINFO_INT_REGISTER + H8_PC:			h8->pc = info->i;								break;
	case CPUINFO_INT_REGISTER + H8_CCR:			h8_set_ccr(h8, info->i);						break;
	case CPUINFO_INT_REGISTER + H8_EXR:			h8_set_exr(h8, info->i);						break;

	case CPUINFO_INT_REGISTER + H8_E0:			h8->regs[0] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E1:			h8->regs[1] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E2:			h8->regs[2] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E3:			h8->regs[3] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E4:			h8->regs[4] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E5:			h8->regs[5] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E6:			h8->regs[6] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E7:			h8->regs[7] = info->i;							break;

	case CPUINFO_INT_INPUT_STATE + H8_IRQ0:		h8_3002_InterruptRequest(h8, 12, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ1:		h8_3002_InterruptRequest(h8, 13, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ2:		h8_3002_InterruptRequest(h8, 14, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ3:		h8_3002_InterruptRequest(h8, 15, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ4:		h8_3002_InterruptRequest(h8, 16, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ5:		h8_3002_InterruptRequest(h8, 17, info->i);		break;

	case CPUINFO_INT_INPUT_STATE + H8_METRO_TIMER_HACK:	h8_3002_InterruptRequest(h8, 24, info->i);		break;

	case CPUINFO_INT_INPUT_STATE + H8_SCI_0_RX:	h8_3002_InterruptRequest(h8, 53, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_SCI_1_RX:	h8_3002_InterruptRequest(h8, 57, info->i);		break;

	default:
		fatalerror("h8_set_info unknown request %x\n", state);
		break;
	}
}

static CPU_SET_INFO( h8s_2394 )
{
	h83xx_state *h8 = get_safe_token(device);

	switch(state) {
	case CPUINFO_INT_PC:						h8->pc = info->i;								break;
	case CPUINFO_INT_REGISTER + H8_PC:			h8->pc = info->i;								break;
	case CPUINFO_INT_REGISTER + H8_CCR:			h8_set_ccr(h8, info->i);						break;
	case CPUINFO_INT_REGISTER + H8_EXR:			h8_set_exr(h8, info->i);						break;

	case CPUINFO_INT_REGISTER + H8_E0:			h8->regs[0] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E1:			h8->regs[1] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E2:			h8->regs[2] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E3:			h8->regs[3] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E4:			h8->regs[4] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E5:			h8->regs[5] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E6:			h8->regs[6] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E7:			h8->regs[7] = info->i;							break;

	case CPUINFO_INT_INPUT_STATE + H8_IRQ0:		h8_3002_InterruptRequest(h8, 16, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ1:		h8_3002_InterruptRequest(h8, 17, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ2:		h8_3002_InterruptRequest(h8, 18, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ3:		h8_3002_InterruptRequest(h8, 19, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ4:		h8_3002_InterruptRequest(h8, 20, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ5:		h8_3002_InterruptRequest(h8, 21, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ6:		h8_3002_InterruptRequest(h8, 22, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ7:		h8_3002_InterruptRequest(h8, 23, info->i);		break;

	case CPUINFO_INT_INPUT_STATE + H8_SCI_0_RX:	h8_3002_InterruptRequest(h8, 81, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_SCI_1_RX:	h8_3002_InterruptRequest(h8, 85, info->i);		break;

	default:
		fatalerror("h8_set_info unknown request %x\n", state);
		break;
	}
}

static READ16_HANDLER( h8_itu_r )
{
	h83xx_state *h8 = get_safe_token(&space.device());

	if (mem_mask == 0xffff)
	{
		// 16-bit read
		return h8_register_read8(h8, offset*2 + 0xffff10)<<8 | h8_register_read8(h8, (offset*2) + 1 + 0xffff10);
	}
	else if (mem_mask == 0xff00)
	{
		return h8_register_read8(h8, offset*2 + 0xffff10)<<8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8_register_read8(h8, (offset*2) + 1 + 0xffff10);
	}

	return 0;
}

static WRITE16_HANDLER( h8_itu_w )
{
	h83xx_state *h8 = get_safe_token(&space.device());

	if (mem_mask == 0xffff)
	{
		// 16-bit write
		h8_register_write8(h8, offset*2 + 0xffff10, data>>8);
		h8_register_write8(h8, (offset*2) + 1 + 0xffff10, data&0xff);
	}
	else if (mem_mask == 0xff00)
	{
		h8_register_write8(h8, offset*2 + 0xffff10, data>>8);
	}
	else if (mem_mask == 0x00ff)
	{
		h8_register_write8(h8, (offset*2) + 1 + 0xffff10, data&0xff);
	}
}

static READ16_HANDLER( h8_3007_itu_r )
{
	h83xx_state *h8 = get_safe_token(&space.device());

	if (mem_mask == 0xffff)
	{
		// 16-bit read
		return h8_3007_register_read8(h8, offset*2 + 0xffff20)<<8 | h8_3007_register_read8(h8, (offset*2) + 1 + 0xffff20);
	}
	else if (mem_mask == 0xff00)
	{
		return h8_3007_register_read8(h8, offset*2 + 0xffff20)<<8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8_3007_register_read8(h8, (offset*2) + 1 + 0xffff20);
	}

	return 0;
}
static WRITE16_HANDLER( h8_3007_itu_w )
{
	h83xx_state *h8 = get_safe_token(&space.device());

	if (mem_mask == 0xffff)
	{
		// 16-bit write
		h8_3007_register_write8(h8, offset*2 + 0xffff20, data>>8);
		h8_3007_register_write8(h8, (offset*2) + 1 + 0xffff20, data&0xff);
	}
	else if (mem_mask == 0xff00)
	{
		h8_3007_register_write8(h8, offset*2 + 0xffff20, data>>8);
	}
	else if (mem_mask == 0x00ff)
	{
		h8_3007_register_write8(h8, (offset*2) + 1 + 0xffff20, data&0xff);
	}
}

static READ16_HANDLER( h8_3007_itu1_r )
{
	h83xx_state *h8 = get_safe_token(&space.device());

	if (mem_mask == 0xffff)
	{
		// 16-bit read
		return h8_3007_register1_read8(h8, offset*2 + 0xfee000)<<8 | h8_3007_register1_read8(h8, (offset*2) + 1 + 0xfee000);
	}
	else if (mem_mask == 0xff00)
	{
		return h8_3007_register1_read8(h8, offset*2 + 0xfee000)<<8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8_3007_register1_read8(h8, (offset*2) + 1 + 0xfee000);
	}

	return 0;
}
static WRITE16_HANDLER( h8_3007_itu1_w )
{
	h83xx_state *h8 = get_safe_token(&space.device());

	if (mem_mask == 0xffff)
	{
		// 16-bit write
		h8_3007_register1_write8(h8, offset*2 + 0xfee000, data>>8);
		h8_3007_register1_write8(h8, (offset*2) + 1 + 0xfee000, data&0xff);
	}
	else if (mem_mask == 0xff00)
	{
		h8_3007_register1_write8(h8, offset*2 + 0xfee000, data>>8);
	}
	else if (mem_mask == 0x00ff)
	{
		h8_3007_register1_write8(h8, (offset*2) + 1 + 0xfee000, data&0xff);
	}
}

static WRITE16_HANDLER( h8s2241_per_regs_w )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		h8s2241_per_regs_write_16(h8, (offset << 1), data);
	}
	else if (mem_mask & 0xff00)
	{
		h8s2241_per_regs_write_8(h8, (offset << 1), (data >> 8) & 0xff);
	}
	else if (mem_mask == 0x00ff)
	{
		h8s2241_per_regs_write_8(h8, (offset << 1) + 1, data & 0xff);
	}
}

static WRITE16_HANDLER( h8s2246_per_regs_w )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		h8s2246_per_regs_write_16(h8, (offset << 1), data);
	}
	else if (mem_mask == 0xff00)
	{
		h8s2246_per_regs_write_8(h8, (offset << 1), (data >> 8) & 0xff);
	}
	else if (mem_mask == 0x00ff)
	{
		h8s2246_per_regs_write_8(h8, (offset << 1) + 1, data & 0xff);
	}
}

static WRITE16_HANDLER( h8s2323_per_regs_w )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		h8s2323_per_regs_write_16(h8, (offset << 1), data);
	}
	else if (mem_mask & 0xff00)
	{
		h8s2323_per_regs_write_8(h8, (offset << 1), (data >> 8) & 0xff);
	}
	else if (mem_mask == 0x00ff)
	{
		h8s2323_per_regs_write_8(h8, (offset << 1) + 1, data & 0xff);
	}
}

static WRITE16_HANDLER( h8s2394_per_regs_w )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		h8s2394_per_regs_write_16(h8, (offset << 1), data);
	}
	else if (mem_mask & 0xff00)
	{
		h8s2394_per_regs_write_8(h8, (offset << 1), (data >> 8) & 0xff);
	}
	else if (mem_mask == 0x00ff)
	{
		h8s2394_per_regs_write_8(h8, (offset << 1) + 1, data & 0xff);
	}
}

static READ16_HANDLER( h8s2241_per_regs_r )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		return h8s2241_per_regs_read_16(h8, (offset << 1));
	}
	else if (mem_mask == 0xff00)
	{
		return h8s2241_per_regs_read_8(h8, (offset << 1)) << 8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8s2241_per_regs_read_8(h8, (offset << 1) + 1);
	}
	return 0;
}

static READ16_HANDLER( h8s2246_per_regs_r )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		return h8s2246_per_regs_read_16(h8, (offset << 1));
	}
	else if (mem_mask == 0xff00)
	{
		return h8s2246_per_regs_read_8(h8, (offset << 1)) << 8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8s2246_per_regs_read_8(h8, (offset << 1) + 1);
	}
	return 0;
}

static READ16_HANDLER( h8s2323_per_regs_r )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		return h8s2323_per_regs_read_16(h8, (offset << 1));
	}
	else if (mem_mask == 0xff00)
	{
		return h8s2323_per_regs_read_8(h8, (offset << 1)) << 8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8s2323_per_regs_read_8(h8, (offset << 1) + 1);
	}
	return 0;
}

static READ16_HANDLER( h8s2394_per_regs_r )
{
	h83xx_state *h8 = get_safe_token(&space.device());
	if (mem_mask == 0xffff)
	{
		return h8s2394_per_regs_read_16(h8, (offset << 1));
	}
	else if (mem_mask == 0xff00)
	{
		return h8s2394_per_regs_read_8(h8, (offset << 1)) << 8;
	}
	else if (mem_mask == 0x00ff)
	{
		return h8s2394_per_regs_read_8(h8, (offset << 1) + 1);
	}
	return 0;
}

// On-board RAM and peripherals
static ADDRESS_MAP_START( h8_3002_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	// 512B RAM
	AM_RANGE(0xfffd10, 0xffff0f) AM_RAM
	AM_RANGE(0xffff10, 0xffffff) AM_READWRITE_LEGACY( h8_itu_r, h8_itu_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8_3044_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	// 32k ROM, 2k RAM
	AM_RANGE(0xfff710, 0xffff0f) AM_RAM
	AM_RANGE(0xffff1c, 0xffffff) AM_READWRITE_LEGACY( h8_itu_r, h8_itu_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8_3007_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	// ROM-less, 4k RAM
	AM_RANGE(0xfee000, 0xfee0ff) AM_READWRITE_LEGACY( h8_3007_itu1_r, h8_3007_itu1_w )
	AM_RANGE(0xffef20, 0xffff1f) AM_RAM
	AM_RANGE(0xffff20, 0xffffe9) AM_READWRITE_LEGACY( h8_3007_itu_r, h8_3007_itu_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8s_2241_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE( 0xFFEC00, 0xFFFBFF ) AM_RAM // on-chip ram
	AM_RANGE( 0xFFFE40, 0xFFFFFF ) AM_READWRITE_LEGACY( h8s2241_per_regs_r, h8s2241_per_regs_w ) // internal i/o registers
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8s_2246_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE( 0xFFDC00, 0xFFFBFF ) AM_RAM // on-chip ram
	AM_RANGE( 0xFFFE40, 0xFFFFFF ) AM_READWRITE_LEGACY( h8s2246_per_regs_r, h8s2246_per_regs_w ) // internal i/o registers
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8s_2323_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE( 0xFFDC00, 0xFFFBFF ) AM_RAM // on-chip ram
	AM_RANGE( 0xFFFE40, 0xFFFFFF ) AM_READWRITE_LEGACY( h8s2323_per_regs_r, h8s2323_per_regs_w ) // internal i/o registers
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8s_2394_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE( 0xFF7C00, 0xFFFBFF ) AM_RAM // 32K of on-chip ram
	AM_RANGE( 0xFFFE40, 0xFFFFFF ) AM_READWRITE_LEGACY( h8s2394_per_regs_r, h8s2394_per_regs_w ) // internal i/o registers
ADDRESS_MAP_END

CPU_GET_INFO( h8_3002 )
{
	h83xx_state *h8 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state) {
	// Interface functions and variables
	case CPUINFO_FCT_SET_INFO:					info->setinfo     = CPU_SET_INFO_NAME(h8);		break;
	case CPUINFO_FCT_INIT:						info->init        = CPU_INIT_NAME(h8);			break;
	case CPUINFO_FCT_RESET:						info->reset       = CPU_RESET_NAME(h8);			break;
	case CPUINFO_FCT_EXIT:						info->exit        = 0;							break;
	case CPUINFO_FCT_EXECUTE:					info->execute     = CPU_EXECUTE_NAME(h8);		break;
	case CPUINFO_FCT_BURN:						info->burn        = 0;							break;
	case CPUINFO_FCT_DISASSEMBLE:				info->disassemble = CPU_DISASSEMBLE_NAME(h8_32);	break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:		info->icount      = &h8->cyccnt;					break;
	case CPUINFO_INT_CONTEXT_SIZE:				info->i           = sizeof(h83xx_state);		break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i           = 2;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i           = 10;							break;

		// Bus sizes
	case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 24;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;						break;

		// Internal maps
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8_3002_internal_map); break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map16 = NULL;	break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:      info->internal_map16 = NULL;	break;

		// CPU misc parameters
	case CPUINFO_STR_NAME:						strcpy(info->s, "H8/3002");						break;
	case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	case CPUINFO_STR_FLAGS:						strcpy(info->s, h8_get_ccr_str(h8));				break;
	case CPUINFO_INT_ENDIANNESS:				info->i = ENDIANNESS_BIG;							break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1;									break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;									break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 16;									break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:		info->i = -1;									break;

		// CPU main state
	case CPUINFO_INT_PC:						info->i = h8->pc;								break;
	case CPUINFO_INT_PREVIOUSPC:				info->i = h8->ppc;								break;

	case CPUINFO_INT_REGISTER + H8_PC:			info->i = h8->pc;								break;
	case CPUINFO_INT_REGISTER + H8_CCR:			info->i = h8_get_ccr(h8);							break;
	case CPUINFO_INT_REGISTER + H8_EXR:			info->i = h8_get_exr(h8);							break;

	case CPUINFO_INT_REGISTER + H8_E0:			info->i = h8->regs[0];							break;
	case CPUINFO_INT_REGISTER + H8_E1:			info->i = h8->regs[1];							break;
	case CPUINFO_INT_REGISTER + H8_E2:			info->i = h8->regs[2];							break;
	case CPUINFO_INT_REGISTER + H8_E3:			info->i = h8->regs[3];							break;
	case CPUINFO_INT_REGISTER + H8_E4:			info->i = h8->regs[4];							break;
	case CPUINFO_INT_REGISTER + H8_E5:			info->i = h8->regs[5];							break;
	case CPUINFO_INT_REGISTER + H8_E6:			info->i = h8->regs[6];							break;
	case CPUINFO_INT_REGISTER + H8_E7:			info->i = h8->regs[7];							break;

	// CPU debug stuff
	case CPUINFO_STR_REGISTER + H8_PC:			sprintf(info->s, "PC   :%08x", h8->pc);			break;
	case CPUINFO_STR_REGISTER + H8_CCR:			sprintf(info->s, "CCR  :%08x", h8_get_ccr(h8));	break;
	case CPUINFO_STR_REGISTER + H8_EXR:			sprintf(info->s, "EXR  :%02x", h8_get_exr(h8));	break;

	case CPUINFO_STR_REGISTER + H8_E0:			sprintf(info->s, "ER0  :%08x", h8->regs[0]);		break;
	case CPUINFO_STR_REGISTER + H8_E1:			sprintf(info->s, "ER1  :%08x", h8->regs[1]);		break;
	case CPUINFO_STR_REGISTER + H8_E2:			sprintf(info->s, "ER2  :%08x", h8->regs[2]);		break;
	case CPUINFO_STR_REGISTER + H8_E3:			sprintf(info->s, "ER3  :%08x", h8->regs[3]);		break;
	case CPUINFO_STR_REGISTER + H8_E4:			sprintf(info->s, "ER4  :%08x", h8->regs[4]);		break;
	case CPUINFO_STR_REGISTER + H8_E5:			sprintf(info->s, "ER5  :%08x", h8->regs[5]);		break;
	case CPUINFO_STR_REGISTER + H8_E6:			sprintf(info->s, "ER6  :%08x", h8->regs[6]);		break;
	case CPUINFO_STR_REGISTER + H8_E7:			sprintf(info->s, " SP  :%08x", h8->regs[7]);		break;
	}
}

CPU_GET_INFO( h8_3044 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8_3044_internal_map);  break;
		case CPUINFO_FCT_DISASSEMBLE:				info->disassemble = CPU_DISASSEMBLE_NAME(h8_24);					break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8/3044");	 break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}

CPU_GET_INFO( h8_3007 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8_3007_internal_map);  break;
		case CPUINFO_FCT_INIT:				info->init = CPU_INIT_NAME(h8_3007);		break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8/3007");		break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}

CPU_GET_INFO( h8s_2241 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8s_2241_internal_map);  break;
		case CPUINFO_FCT_INIT:				info->init = CPU_INIT_NAME(h8s_2xxx);		break;
		case CPUINFO_FCT_RESET:				info->reset= CPU_RESET_NAME(h8s_2xxx);			break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8S/2241");		break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}

CPU_GET_INFO( h8s_2246 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8s_2246_internal_map);  break;
		case CPUINFO_FCT_INIT:				info->init = CPU_INIT_NAME(h8s_2xxx);		break;
		case CPUINFO_FCT_RESET:				info->reset= CPU_RESET_NAME(h8s_2xxx);			break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8S/2246");		break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}

CPU_GET_INFO( h8s_2323 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8s_2323_internal_map);  break;
		case CPUINFO_FCT_INIT:				info->init = CPU_INIT_NAME(h8s_2xxx);		break;
		case CPUINFO_FCT_RESET:				info->reset= CPU_RESET_NAME(h8s_2xxx);			break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8S/2323");		break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}

CPU_GET_INFO( h8s_2394 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(h8s_2394_internal_map);  break;
        case CPUINFO_FCT_SET_INFO:			info->setinfo = CPU_SET_INFO_NAME(h8s_2394);		break;
		case CPUINFO_FCT_INIT:				info->init = CPU_INIT_NAME(h8s_2xxx);		break;
		case CPUINFO_FCT_RESET:				info->reset= CPU_RESET_NAME(h8s_2394);			break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "H8S/2394");		break;
		default:
			CPU_GET_INFO_CALL(h8_3002);
	}
}

DEFINE_LEGACY_CPU_DEVICE(H83002, h8_3002);
DEFINE_LEGACY_CPU_DEVICE(H83007, h8_3007);
DEFINE_LEGACY_CPU_DEVICE(H83044, h8_3044);

DEFINE_LEGACY_CPU_DEVICE(H8S2241, h8s_2241);
DEFINE_LEGACY_CPU_DEVICE(H8S2246, h8s_2246);
DEFINE_LEGACY_CPU_DEVICE(H8S2323, h8s_2323);
DEFINE_LEGACY_CPU_DEVICE(H8S2394, h8s_2394);

