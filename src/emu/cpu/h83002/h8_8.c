/***************************************************************************

 h8_8.c: Hitachi H8/3xx 8/16-bit microcontroller emulator

 Based on H8/300 series 16/32-bit emulator h83002.c.
 Reference: Renesas Technology H8/3337 Group Hardware Manual

 By R. Belmont

****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h8.h"
#include "h8priv.h"

CPU_DISASSEMBLE(h8);

#define H8_SP	(7)

#define h8_mem_read8(x)		h8->program->read_byte(x)
#define h8_mem_write8(x, y)	h8->program->write_byte(x, y)

// timing macros
#define H8_IFETCH_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_BRANCH_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_STACK_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_BYTE_TIMING(x, adr)	if (address24 >= 0xff90) h8->cyccnt -= (x) * 3; else h8->cyccnt -= (x) * 4;
#define H8_WORD_TIMING(x, adr)	if (address24 >= 0xff90) h8->cyccnt -= (x) * 3; else h8->cyccnt -= (x) * 4;
#define H8_IOP_TIMING(x)	h8->cyccnt -= (x);

static TIMER_CALLBACK( h8_timer_0_cb );
static TIMER_CALLBACK( h8_timer_1_cb );
static TIMER_CALLBACK( h8_timer_2_cb );
static TIMER_CALLBACK( h8_timer_3_cb );

INLINE UINT16 h8_mem_read16(h83xx_state *h8, offs_t address)
{
	UINT16 result =  h8->program->read_byte(address)<<8;
	return result | h8->program->read_byte(address+1);
}

INLINE UINT16 h8_readop16(h83xx_state *h8, offs_t address)
{
	UINT16 result =  h8->direct->read_decrypted_byte(address)<<8;
	return result | h8->direct->read_decrypted_byte(address+1);
}

INLINE void h8_mem_write16(h83xx_state *h8, offs_t address, UINT16 data)
{
	h8->program->write_byte(address, data >> 8);
	h8->program->write_byte(address+1, data);
}

INLINE UINT32 h8_mem_read32(h83xx_state *h8, offs_t address)
{
	UINT32 result = h8->program->read_byte(address) << 24;
	result |= h8->program->read_byte(address+1) << 16;
	result |= h8->program->read_byte(address+2) << 8;
	result |= h8->program->read_byte(address+3);

	return result;
}

INLINE void h8_mem_write32(h83xx_state *h8, offs_t address, UINT32 data)
{
	h8->program->write_byte(address, data >> 24);
	h8->program->write_byte(address+1, data >> 16);
	h8->program->write_byte(address+2, data >> 8);
	h8->program->write_byte(address+3, data);
}

static void h8_check_irqs(h83xx_state *h8);

/* implementation */

static void h8_300_InterruptRequest(h83xx_state *h8, UINT8 source, UINT8 state)
{
	int request = source / 32;
	int bit = source % 32;

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

static CPU_INIT(h8bit)
{
	h83xx_state *h8 = get_safe_token(device);

	h8->h8iflag = 1;

	h8->irq_cb = irqcallback;
	h8->device = device;

	h8->mode_8bit = 1;

	h8->program = device->space(AS_PROGRAM);
	h8->direct = &h8->program->direct();
	h8->io = device->space(AS_IO);

	h8->timer[0] = h8->device->machine().scheduler().timer_alloc(FUNC(h8_timer_0_cb), h8);
	h8->timer[1] = h8->device->machine().scheduler().timer_alloc(FUNC(h8_timer_1_cb), h8);
	h8->timer[2] = h8->device->machine().scheduler().timer_alloc(FUNC(h8_timer_2_cb), h8);
	h8->timer[3] = h8->device->machine().scheduler().timer_alloc(FUNC(h8_timer_3_cb), h8);

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

	h8->device->machine().save().register_postload(save_prepost_delegate(FUNC(h8_onstateload), h8));
}

static CPU_RESET(h8bit)
{
	h83xx_state *h8 = get_safe_token(device);

	h8->h8err = 0;
	h8->pc = h8_mem_read16(h8, 0);

	h8->incheckirqs = 0;

	// disable timers
	h8->h8TSTR = 0;
	h8->FRC = 0;
	h8->STCR = 0;
	h8->TCR[0] = h8->TCR[1] = 0;
	h8->TCORA[0] = h8->TCORB[0] = 0;
	h8->TCORA[1] = h8->TCORB[1] = 0;
	h8->TCNT[0] = h8->TCNT[1] = 0;

	h8->has_h8speriphs = false;
}

static void h8_GenException(h83xx_state *h8, UINT8 vectornr)
{
	// push PC on stack
	h8_setreg16(h8, H8_SP, h8_getreg16(h8, H8_SP)-2);
	h8_mem_write16(h8, h8_getreg16(h8, H8_SP), h8->pc);
	// push ccr
	h8_setreg16(h8, H8_SP, h8_getreg16(h8, H8_SP)-2);
	h8_mem_write16(h8, h8_getreg16(h8, H8_SP), h8_get_ccr(h8));

	// generate address from vector
	h8_set_ccr(h8, h8_get_ccr(h8) | 0x80);
	if (h8->h8uiflag == 0)
		h8_set_ccr(h8, h8_get_ccr(h8) | 0x40);
	h8->pc = h8_mem_read16(h8, vectornr * 2) & 0xffff;

	// these timings are still approximations but much better than before
	H8_IFETCH_TIMING(8);	// 24 cycles
	H8_STACK_TIMING(3);	// 12 cycles
}

static int h8_get_priority(h83xx_state *h8, UINT8 bit)
{
	int res = 0;
	switch(bit)
	{
	case 3: // NMI
		res = 2; break;
	case 4: // IRQ0
		if (h8->per_regs[0xc7]&0x01) res = 1; break;
	case 5: // IRQ1
		if (h8->per_regs[0xc7]&0x02) res = 1; break;
	case 6: // IRQ2
		if (h8->per_regs[0xc7]&0x04) res = 1; break;
	case 7: // IRQ3
		if (h8->per_regs[0xc7]&0x08) res = 1; break;
	case 8: // IRQ4
		if (h8->per_regs[0xc7]&0x10) res = 1; break;
	case 9: // IRQ5
		if (h8->per_regs[0xc7]&0x20) res = 1; break;
	case 10: // IRQ6
		if (h8->per_regs[0xc7]&0x40) res = 1; break;
	case 11: // IRQ7
		if (h8->per_regs[0xc7]&0x80) res = 1; break;
	case 19: // 8-bit timer 0 match A
		if (h8->TCR[0] & 0x40) res = 1; break;
	case 20: // 8-bit timer 0 match B
		if (h8->TCR[0] & 0x80) res = 1; break;
	case 22: // 8-bit timer 1 match A
		if (h8->TCR[1] & 0x40) res = 1; break;
	case 23: // 8-bit timer 1 match B
		if (h8->TCR[1] & 0x80) res = 1; break;
	case 28: // SCI0 Rx
		if (h8->per_regs[0xda]&0x40) res = 1; break;
	case 32: // SCI1 Rx
		if (h8->per_regs[0x8a]&0x40) res = 1; break;
	}
	return res;
}

static void h8_check_irqs(h83xx_state *h8)
{
	int lv = 0;

	h8->incheckirqs = 1;

	if (h8->h8iflag != 0)
	{
		lv = 2;
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
					source = bit + 32;
				}
			}
		}

		// call the MAME callback if it's one of the external IRQs
		if (source >= 3 && source <= 11)
		{
			(*h8->irq_cb)(h8->device, source - 3 + H8_NMI);
		}

		if (source != 0xff)
		{
			h8_GenException(h8, source);
		}
	}

	h8->incheckirqs = 0;
}

#define H8_ADDR_MASK 0xffff
#include "h8ops.h"

//  peripherals
static void recalc_8bit_timer(h83xx_state *h8, int t)
{
	static const INT32 dividers[8] = { 0, 0, 8, 2, 64, 32, 1024, 256 };
	int div;
	INT32 time;

	div = (h8->STCR & 1) | ((h8->TCR[t] & 3)<<1);

	// if "no clock source", stop
	if (div < 2)
	{
		h8->timer[(t*2)]->adjust(attotime::never);
		h8->timer[(t*2)+1]->adjust(attotime::never);
		return;
	}

	if (h8->TCORA[t])
	{
		time = (h8->device->unscaled_clock() / dividers[div]) / (h8->TCORA[t] - h8->TCNT[t]);
		h8->timer[(t*2)]->adjust(attotime::from_hz(time));
	}

	if (h8->TCORB[t])
	{
		time = (h8->device->unscaled_clock() / dividers[div]) / (h8->TCORB[t] - h8->TCNT[t]);
		h8->timer[(t*2)+1]->adjust(attotime::from_hz(time));
	}
}

// IRQs: timer 0: 19 A 20 B 21 OV  timer1: 22 A 23 B 24 OV
static void timer_8bit_expire(h83xx_state *h8, int t, int sel)
{
	static const int irqbase[2] = { 19, 22 };

	h8->timer[(t*2)+sel]->adjust(attotime::never);

	h8->TCSR[t] |= ((0x40)<<sel);

	// check for interrupts
	if (h8->TCR[t] & (0x40<<sel))
	{
		h8->irq_req[0] |= (1 << (irqbase[t] + sel));
	}

	switch ((h8->TCR[t]>>3) & 3)
	{
		case 0:	// no clear
			break;

		case 1: // clear on match A
			if (!sel)
			{
				h8->TCNT[t] = 0;
				recalc_8bit_timer(h8, t);
			}
			break;

		case 2: // clear on match B
			if (sel)
			{
				h8->TCNT[t] = 0;
				recalc_8bit_timer(h8, t);
			}
			break;

		case 3:	// clear on external reset input signal (not implemented)
			logerror("H8: external reset not implemented for 8-bit timers\n");
			break;
	}
}


// MAME interface stuff

static CPU_SET_INFO( h8 )
{
	h83xx_state *h8 = get_safe_token(device);

	switch(state) {
	case CPUINFO_INT_PC:			    		h8->pc = info->i;								break;
	case CPUINFO_INT_REGISTER + H8_PC:			h8->pc = info->i;								break;
    case CPUINFO_INT_REGISTER + H8_CCR:			h8_set_ccr(h8, info->i);						break;
    case CPUINFO_INT_REGISTER + H8_EXR:         h8_set_exr(h8, info->i);                        break;

	case CPUINFO_INT_REGISTER + H8_E0:			h8->regs[0] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E1:			h8->regs[1] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E2:			h8->regs[2] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E3:			h8->regs[3] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E4:			h8->regs[4] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E5:			h8->regs[5] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E6:			h8->regs[6] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E7:			h8->regs[7] = info->i;							break;

	case CPUINFO_INT_INPUT_STATE + H8_NMI:		h8_300_InterruptRequest(h8, 3, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ0:		h8_300_InterruptRequest(h8, 4, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ1:		h8_300_InterruptRequest(h8, 5, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ2:		h8_300_InterruptRequest(h8, 6, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ3:		h8_300_InterruptRequest(h8, 7, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ4:		h8_300_InterruptRequest(h8, 8, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ5:		h8_300_InterruptRequest(h8, 9, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ6:		h8_300_InterruptRequest(h8, 10, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ7:		h8_300_InterruptRequest(h8, 11, info->i);		break;

	case CPUINFO_INT_INPUT_STATE + H8_SCI_0_RX:	h8_300_InterruptRequest(h8, 28, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + H8_SCI_1_RX:	h8_300_InterruptRequest(h8, 32, info->i);		break;

	default:
		fatalerror("h8_set_info unknown request %x", state);
		break;
	}
}

static READ8_HANDLER( h8330_itu_r )
{
	UINT8 val;
	UINT8 reg;
	UINT64 frc;
	static const UINT64 divider[4] = { 2, 8, 32, 1 };
	h83xx_state *h8 = get_safe_token(&space->device());

	reg = (offset + 0x88) & 0xff;

	switch(reg)
	{
	case 0x8d:		// serial Rx 1
		val = h8->io->read_byte(H8_SERIAL_1);
		break;
	case 0x92:  		// FRC H
		frc = h8->device->total_cycles() / divider[h8->per_regs[0x96]];
		frc %= 65536;
		return frc>>8;
	case 0x93:		// FRC L
		frc = h8->device->total_cycles() / divider[h8->per_regs[0x96]];
		frc %= 65536;
		return frc&0xff;
	case 0xb2:  		// port 1 data
		val = h8->io->read_byte(H8_PORT_1);
		break;
	case 0xb3:  		// port 2 data
		val = h8->io->read_byte(H8_PORT_2);
		break;
	case 0xb6:		// port 3 data
		val = h8->io->read_byte(H8_PORT_3);
		break;
	case 0xb7:		// port 4 data
		val = h8->io->read_byte(H8_PORT_4);
		break;
	case 0xba:		// port 5 data
		val = h8->io->read_byte(H8_PORT_5);
		break;
	case 0xbb:		// port 6 data
		val = h8->io->read_byte(H8_PORT_6);
		break;
	case 0xbe:		// port 7 data
		val = h8->io->read_byte(H8_PORT_7);
		break;
	case 0xbf:		// port 8 data
		val = h8->io->read_byte(H8_PORT_8);
		break;
	case 0xc1:		// port 9 data
		val = h8->io->read_byte(H8_PORT_9);
		break;
	case 0xdc:	// serial status
		val = 0x87;
		break;
	case 0xdd:		// serial Rx 0
		val = h8->io->read_byte(H8_SERIAL_0);
		break;
	case 0xe0:	// ADC 0 low byte
		val = h8->io->read_byte(H8_ADC_0_L);
		break;
	case 0xe1:	// ADC 0 high byte
		val = h8->io->read_byte(H8_ADC_0_H);
		break;
	case 0xe2:	// ADC 1 low byte
		val = h8->io->read_byte(H8_ADC_1_L);
		break;
	case 0xe3:	// ADC 1 high byte
		val = h8->io->read_byte(H8_ADC_1_H);
		break;
	case 0xe4:	// ADC 2 low byte
		val = h8->io->read_byte(H8_ADC_2_L);
		break;
	case 0xe5:	// ADC 2 high byte
		val = h8->io->read_byte(H8_ADC_2_H);
		break;
	case 0xe6:	// ADC 3 low byte
		val = h8->io->read_byte(H8_ADC_3_L);
		break;
	case 0xe7:	// ADC 3 high byte
		val = h8->io->read_byte(H8_ADC_3_H);
		break;
	case 0xe8:	// ADCSR: A/D control/status
		val = 0x80;	// return conversion completed
		break;
	default:
		val = h8->per_regs[reg];
		break;
	}

	return val;
}

static WRITE8_HANDLER( h8330_itu_w )
{
	UINT8 reg;
	h83xx_state *h8 = get_safe_token(&space->device());

	reg = (offset + 0x88) & 0xff;

	switch (reg)
	{
	case 0x80:
		printf("%02x to flash control or external\n", data);
		break;
	case 0x8b:		// serial Tx 1
		h8->io->write_byte(H8_SERIAL_1, data);
		break;
	case 0xb2:  		// port 1 data
		h8->io->write_byte(H8_PORT_1, data);
		break;
	case 0xb3:  		// port 2 data
		h8->io->write_byte(H8_PORT_2, data);
		break;
	case 0xb6:		// port 3 data
		h8->io->write_byte(H8_PORT_3, data);
		break;
	case 0xb7:		// port 4 data
		h8->io->write_byte(H8_PORT_4, data);
		break;
	case 0xba:		// port 5 data
		h8->io->write_byte(H8_PORT_5, data);
		break;
	case 0xbb:		// port 6 data
		h8->io->write_byte(H8_PORT_6, data);
		break;
	case 0xbe:		// port 7 data
		h8->io->write_byte(H8_PORT_7, data);
		break;
	case 0xbf:		// port 8 data
		h8->io->write_byte(H8_PORT_8, data);
		break;
	case 0xc1:		// port 9 data
		h8->io->write_byte(H8_PORT_9, data);
		break;
	case 0xdb:		// serial Tx 0
		h8->io->write_byte(H8_SERIAL_0, data);
		break;

	case 0xd8:
	case 0xda:
	case 0xdc:
	case 0xd9:
		break;

	case 0x88:
	case 0x8a:
	case 0x8c:
	case 0x89:
		break;

	case 0xc7:
		break;

	case 0xc8:
	    	h8->TCR[0] = data;
		recalc_8bit_timer(h8, 0);
		break;
	case 0xc9:
		h8->TCSR[0] = data;
		h8->irq_req[0] &= ~(1 << 19);
		h8->irq_req[0] &= ~(1 << 20);
		h8->irq_req[0] &= ~(1 << 21);
		recalc_8bit_timer(h8, 0);
		break;
	case 0xca:
		h8->TCORA[0] = data;
		recalc_8bit_timer(h8, 0);
		break;
	case 0xcb:
		h8->TCORB[0] = data;
		recalc_8bit_timer(h8, 0);
		break;
	case 0xcc:
		h8->TCNT[0] = data;
		recalc_8bit_timer(h8, 0);
		break;

	case 0xc3:
		h8->STCR = data;
		recalc_8bit_timer(h8, 0);
		recalc_8bit_timer(h8, 1);
		break;

	case 0xd0:
	    	h8->TCR[1] = data;
		recalc_8bit_timer(h8, 1);
		break;
	case 0xd1:
		h8->TCSR[1] = data;
		h8->irq_req[0] &= ~(1 << 22);
		h8->irq_req[0] &= ~(1 << 23);
		h8->irq_req[0] &= ~(1 << 24);
		recalc_8bit_timer(h8, 1);
		break;
	case 0xd2:
		h8->TCORA[1] = data;
		recalc_8bit_timer(h8, 1);
		break;
	case 0xd3:
		h8->TCORB[1] = data;
		recalc_8bit_timer(h8, 1);
		break;
	case 0xd4:
		h8->TCNT[1] = data;
		recalc_8bit_timer(h8, 1);
		break;
	}

	h8->per_regs[reg] = data;
}

static TIMER_CALLBACK( h8_timer_0_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	timer_8bit_expire(h8, 0, 0);
}

static TIMER_CALLBACK( h8_timer_1_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	timer_8bit_expire(h8, 0, 1);
}

static TIMER_CALLBACK( h8_timer_2_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	timer_8bit_expire(h8, 1, 0);
}

static TIMER_CALLBACK( h8_timer_3_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	timer_8bit_expire(h8, 1, 1);
}

static ADDRESS_MAP_START( h8_3334_internal_map, AS_PROGRAM, 8, legacy_cpu_device )
	// 512B RAM
	AM_RANGE(0xfb80, 0xff7f) AM_RAM
	AM_RANGE(0xff88, 0xffff) AM_READWRITE_LEGACY( h8330_itu_r, h8330_itu_w )
ADDRESS_MAP_END

CPU_GET_INFO( h8_3334 )
{
	h83xx_state *h8 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state) {
	// Interface functions and variables
	case CPUINFO_FCT_SET_INFO:			info->setinfo     = CPU_SET_INFO_NAME(h8);				break;
	case CPUINFO_FCT_INIT:				info->init        = CPU_INIT_NAME(h8bit);					break;
	case CPUINFO_FCT_RESET:				info->reset       = CPU_RESET_NAME(h8bit);					break;
	case CPUINFO_FCT_EXIT:				info->exit        = 0;							break;
	case CPUINFO_FCT_EXECUTE:			info->execute     = CPU_EXECUTE_NAME(h8);					break;
	case CPUINFO_FCT_BURN:				info->burn        = 0;							break;
	case CPUINFO_FCT_DISASSEMBLE:			info->disassemble = CPU_DISASSEMBLE_NAME(h8);					break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:		info->icount      = &h8->cyccnt;					break;
	case CPUINFO_INT_CONTEXT_SIZE:			info->i           = sizeof(h83xx_state);		break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i           = 2;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i           = 10;							break;

		// Bus sizes
	case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;						break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 16;						break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;						break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;						break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;						break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;						break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_IO:	info->i = 8;						break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:	info->i = 16;						break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:	info->i = 0;						break;

		// Internal maps
	case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(h8_3334_internal_map); break;
	case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map8 = NULL;	break;
	case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:      info->internal_map16 = NULL;	break;

		// CPU misc parameters
	case DEVINFO_STR_NAME:					strcpy(info->s, "H8/3334");						break;
	case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);						break;
	case CPUINFO_STR_FLAGS:					strcpy(info->s, h8_get_ccr_str(h8));				break;
	case DEVINFO_INT_ENDIANNESS:				info->i = ENDIANNESS_BIG;							break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1;									break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;									break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 16;									break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;									break;

		// CPU main state
	case CPUINFO_INT_PC:					info->i = h8->pc;								break;
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

	case CPUINFO_STR_REGISTER + H8_E0:			sprintf(info->s, " R0  :%08x", h8->regs[0]);		break;
	case CPUINFO_STR_REGISTER + H8_E1:			sprintf(info->s, " R1  :%08x", h8->regs[1]);		break;
	case CPUINFO_STR_REGISTER + H8_E2:			sprintf(info->s, " R2  :%08x", h8->regs[2]);		break;
	case CPUINFO_STR_REGISTER + H8_E3:			sprintf(info->s, " R3  :%08x", h8->regs[3]);		break;
	case CPUINFO_STR_REGISTER + H8_E4:			sprintf(info->s, " R4  :%08x", h8->regs[4]);		break;
	case CPUINFO_STR_REGISTER + H8_E5:			sprintf(info->s, " R5  :%08x", h8->regs[5]);		break;
	case CPUINFO_STR_REGISTER + H8_E6:			sprintf(info->s, " R6  :%08x", h8->regs[6]);		break;
	case CPUINFO_STR_REGISTER + H8_E7:			sprintf(info->s, " SP  :%08x", h8->regs[7]);		break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(H83334, h8_3334);
