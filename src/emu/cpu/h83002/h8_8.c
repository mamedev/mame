/***************************************************************************

 h8_8.c: Hitachi H8/3xx 8/16-bit microcontroller emulator

 Based on H8/300 series 16/32-bit emulator h83002.c.
 Reference: Renesas Technology H8/3337 Group Hardware Manual

 By R. Belmont

****************************************************************************/

#include "debugger.h"
#include "deprecat.h"
#include "h8.h"
#include "h8priv.h"

#define H8_SP	(7)

#define h8_mem_read8(x) program_read_byte_8be(x)
#define h8_mem_write8(x, y)  program_write_byte_8be(x, y)

// timing macros
#define H8_IFETCH_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_BRANCH_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_STACK_TIMING(x)	h8->cyccnt -= (x) * 4;
#define H8_BYTE_TIMING(x, adr)	if (address24 >= 0xff90) h8->cyccnt -= (x) * 3; else h8->cyccnt -= (x) * 4;
#define H8_WORD_TIMING(x, adr)	if (address24 >= 0xff90) h8->cyccnt -= (x) * 3; else h8->cyccnt -= (x) * 4;
#define H8_IOP_TIMING(x)	h8->cyccnt -= (x);

INLINE UINT16 h8_mem_read16(offs_t address)
{
	UINT16 result =  program_read_byte_8be(address)<<8;
	return result | program_read_byte_8be(address+1);
}

INLINE UINT16 h8_readop16(offs_t address)
{
	UINT16 result =  program_decrypted_read_byte(address)<<8;
	return result | program_decrypted_read_byte(address+1);
}

INLINE void h8_mem_write16(offs_t address, UINT16 data)
{
	program_write_byte_8be(address, data >> 8);
	program_write_byte_8be(address+1, data);
}

INLINE UINT32 h8_mem_read32(offs_t address)
{
	UINT32 result = program_read_byte_8be(address) << 24;
	result |= program_read_byte_8be(address+1) << 16; 
	result |= program_read_byte_8be(address+2) << 8; 
	result |= program_read_byte_8be(address+3);

	return result;		
}

INLINE void h8_mem_write32(offs_t address, UINT32 data)
{
	program_write_byte_8be(address, data >> 24);
	program_write_byte_8be(address+1, data >> 16);
	program_write_byte_8be(address+2, data >> 8);
	program_write_byte_8be(address+3, data);
}

static void *token;
static void h8_check_irqs(h83xx_state *h8);

/* implementation */

extern offs_t h8_disasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 addrmask);

static CPU_DISASSEMBLE(h8)
{
	return h8_disasm(buffer, pc, oprom, opram, 0xffff);
}

void h8_300_InterruptRequest(h83xx_state *h8, UINT8 source)
{
	if(source>31)
	{
		h8->h8_IRQrequestH |= (1<<(source-32));
	}
	else
	{
		h8->h8_IRQrequestL |= (1<<source);
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

static char *h8_get_ccr_str(h83xx_state *h8)
{
	static char res[8];

	memset(res, 0, 8);
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

	h8_check_irqs(h8);
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

static STATE_POSTLOAD( h8_onstateload )
{
	h83xx_state *h8 = (h83xx_state *)param;

	h8_set_ccr(h8, h8->ccr);
}

static CPU_INIT(h8bit)
{
	h83xx_state *h8 = device->token;

	h8->h8iflag = 1;

	h8->irq_cb = irqcallback;
	h8->device = device;

	h8->h8300_mode = 1;

	state_save_register_item("H8/300", device->tag, 0, h8->h8err);
	state_save_register_item_array("H8/300", device->tag, 0, h8->regs);
	state_save_register_item("H8/300", device->tag, 0, h8->pc);
	state_save_register_item("H8/300", device->tag, 0, h8->ppc);
	state_save_register_item("H8/300", device->tag, 0, h8->h8_IRQrequestH);
	state_save_register_item("H8/300", device->tag, 0, h8->h8_IRQrequestL);
	state_save_register_item("H8/300", device->tag, 0, h8->ccr);
	state_save_register_item("H8/300", device->tag, 0, h8->h8300_mode);

	state_save_register_item_array("H8/300", device->tag, 0, h8->per_regs);
	state_save_register_item("H8/300", device->tag, 0, h8->h8TSTR);
	state_save_register_item_array("H8/300", device->tag, 0, h8->h8TCNT);

	state_save_register_postload(Machine, h8_onstateload, h8);
}

static CPU_RESET(h8bit)
{
	h83xx_state *h8 = device->token;

	h8->h8err = 0;
	h8->pc = h8_mem_read16(0);
	change_pc(h8->pc);

	// disable timers
	h8->h8TSTR = 0;
}

static void h8_GenException(h83xx_state *h8, UINT8 vectornr)
{
	// push PC on stack
	h8_setreg16(h8, H8_SP, h8_getreg16(h8, H8_SP)-2);
	h8_mem_write16(h8_getreg16(h8, H8_SP), h8->pc);
	// push ccr
	h8_setreg16(h8, H8_SP, h8_getreg16(h8, H8_SP)-2);
	h8_mem_write16(h8_getreg16(h8, H8_SP), h8_get_ccr(h8));

	// generate address from vector
	h8_set_ccr(h8, h8_get_ccr(h8) | 0x80);
	if (h8->h8uiflag == 0)
		h8_set_ccr(h8, h8_get_ccr(h8) | 0x40);
	h8->pc = h8_mem_read16(vectornr * 2) & 0xffff;
	change_pc(h8->pc);

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
	}
	return res;
}

static void h8_check_irqs(h83xx_state *h8)
{
	int lv = -1;

	if (h8->h8iflag == 0)
	{
		lv = 0;
	}

	// any interrupts wanted and can accept ?
	if(((h8->h8_IRQrequestH != 0) || (h8->h8_IRQrequestL != 0)) && (lv >= 0))
	{
		UINT8 bit, source;
		// which one ?
		for(bit = 0, source = 0xff; source == 0xff && bit < 32; bit++)
		{
			if( h8->h8_IRQrequestL & (1<<bit) )
			{
				if (h8_get_priority(h8, bit) >= lv)
				{
					// mask off
					h8->h8_IRQrequestL &= ~(1<<bit);
					source = bit;
				}
			}
		}
		// which one ?
		for(bit = 0; source == 0xff && bit < 32; bit++)
		{
			if( h8->h8_IRQrequestH & (1<<bit) )
			{
				if (h8_get_priority(h8, bit + 32) >= lv)
				{
					// mask off
					h8->h8_IRQrequestH &= ~(1<<bit);
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
}

#define H8_ADDR_MASK 0xffff
#include "h8ops.h"

// MAME interface stuff

static CPU_GET_CONTEXT( h8 )
{
}

static CPU_SET_CONTEXT( h8 )
{
	if (src)
	{
		token = src;
	}
}

static CPU_SET_INFO( h8 )
{
	h83xx_state *h8 = device->token;

	switch(state) {
	case CPUINFO_INT_PC:			      		h8->pc = info->i; change_pc(h8->pc);				break;
	case CPUINFO_INT_REGISTER + H8_PC:			h8->pc = info->i; change_pc(h8->pc);				break;
	case CPUINFO_INT_REGISTER + H8_CCR:			h8_set_ccr(h8, info->i);							break;

	case CPUINFO_INT_REGISTER + H8_E0:			h8->regs[0] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E1:			h8->regs[1] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E2:			h8->regs[2] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E3:			h8->regs[3] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E4:			h8->regs[4] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E5:			h8->regs[5] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E6:			h8->regs[6] = info->i;							break;
	case CPUINFO_INT_REGISTER + H8_E7:			h8->regs[7] = info->i;							break;

	case CPUINFO_INT_INPUT_STATE + H8_NMI:		if (info->i) h8_300_InterruptRequest(h8, 3);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ0:		if (info->i) h8_300_InterruptRequest(h8, 4);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ1:		if (info->i) h8_300_InterruptRequest(h8, 5);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ2:		if (info->i) h8_300_InterruptRequest(h8, 6);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ3:		if (info->i) h8_300_InterruptRequest(h8, 7);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ4:		if (info->i) h8_300_InterruptRequest(h8, 8);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ5:		if (info->i) h8_300_InterruptRequest(h8, 9);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ6:		if (info->i) h8_300_InterruptRequest(h8, 10);		break;
	case CPUINFO_INT_INPUT_STATE + H8_IRQ7:		if (info->i) h8_300_InterruptRequest(h8, 11);		break;

	case CPUINFO_INT_INPUT_STATE + H8_SCI_0_RX:	if (info->i) h8_300_InterruptRequest(h8, 28);		break;
	case CPUINFO_INT_INPUT_STATE + H8_SCI_1_RX:	if (info->i) h8_300_InterruptRequest(h8, 32);		break;

	default:
		fatalerror("h8_set_info unknown request %x", state);
		break;
	}
}

static READ8_HANDLER( h8330_itu_r )
{
	UINT8 val;
	UINT8 reg;
	h83xx_state *h8 = (h83xx_state *)space->cpu->token;

	reg = (offset + 0x88) & 0xff;

	switch(reg)
	{
	case 0x8d:		// serial Rx 1
		val = io_read_byte(H8_SERIAL_1);
		break;
	case 0xb2:    		// port 1 data
		val = io_read_byte(H8_PORT_1);
		break;
	case 0xb3:    		// port 2 data
		val = io_read_byte(H8_PORT_2);
		break;
	case 0xb6:		// port 3 data
		val = io_read_byte(H8_PORT_3);
		break;
	case 0xb7:		// port 4 data
		val = io_read_byte(H8_PORT_4);
		break;
	case 0xba:		// port 5 data
		val = io_read_byte(H8_PORT_5);
		break;
	case 0xbb:		// port 6 data
		val = io_read_byte(H8_PORT_6);
		break;
	case 0xbe:		// port 7 data
		val = io_read_byte(H8_PORT_7);
		break;
	case 0xbf:		// port 8 data
		val = io_read_byte(H8_PORT_8);
		break;
	case 0xc1:		// port 9 data
		val = io_read_byte(H8_PORT_9);
		break;
	case 0xdc:	// serial status
		val = 0x87;
		break;
	case 0xdd:		// serial Rx 0
		val = io_read_byte(H8_SERIAL_0);
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
	h83xx_state *h8 = (h83xx_state *)space->cpu->token;

	reg = (offset + 0x88) & 0xff;

	switch (reg)
	{
	case 0x8b:		// serial Tx 1
		io_write_byte(H8_SERIAL_1, data);
		break;
	case 0xb2:    		// port 1 data
		io_write_byte(H8_PORT_1, data);
		break;
	case 0xb3:    		// port 2 data
		io_write_byte(H8_PORT_2, data);
		break;
	case 0xb6:		// port 3 data
		io_write_byte(H8_PORT_3, data);
		break;
	case 0xb7:		// port 4 data
		io_write_byte(H8_PORT_4, data);
		break;
	case 0xba:		// port 5 data
		io_write_byte(H8_PORT_5, data);
		break;
	case 0xbb:		// port 6 data
		io_write_byte(H8_PORT_6, data);
		break;
	case 0xbe:		// port 7 data
		io_write_byte(H8_PORT_7, data);
		break;
	case 0xbf:		// port 8 data
		io_write_byte(H8_PORT_8, data);
		break;
	case 0xc1:		// port 9 data
		io_write_byte(H8_PORT_9, data);
		break;
	case 0xdb:		// serial Tx 0
		io_write_byte(H8_SERIAL_0, data);
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

	case 0xc3:
		break;

	case 0xc7:
		break;
	}

	h8->per_regs[reg] = data;
}

static ADDRESS_MAP_START( h8_3334_internal_map, ADDRESS_SPACE_PROGRAM, 8 )
	// 512B RAM
	AM_RANGE(0xfb80, 0xff7f) AM_RAM
	AM_RANGE(0xff88, 0xffff) AM_READWRITE( h8330_itu_r, h8330_itu_w )
ADDRESS_MAP_END

CPU_GET_INFO( h8_3334 )
{
	h83xx_state *h8 = (device != NULL) ? device->token : NULL; 

	switch(state) {
	// Interface functions and variables
	case CPUINFO_PTR_SET_INFO:			info->setinfo     = CPU_SET_INFO_NAME(h8);				break;
	case CPUINFO_PTR_GET_CONTEXT:			info->getcontext  = CPU_GET_CONTEXT_NAME(h8);	break;
	case CPUINFO_PTR_SET_CONTEXT:			info->setcontext  = CPU_SET_CONTEXT_NAME(h8);	break;
	case CPUINFO_PTR_INIT:				info->init        = CPU_INIT_NAME(h8bit);					break;
	case CPUINFO_PTR_RESET:				info->reset       = CPU_RESET_NAME(h8bit);					break;
	case CPUINFO_PTR_EXIT:				info->exit        = 0;							break;
	case CPUINFO_PTR_EXECUTE:			info->execute     = CPU_EXECUTE_NAME(h8);					break;
	case CPUINFO_PTR_BURN:				info->burn        = 0;							break;
	case CPUINFO_PTR_DISASSEMBLE:			info->disassemble = CPU_DISASSEMBLE_NAME(h8);					break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:		info->icount      = &h8->cyccnt;					break;
	case CPUINFO_INT_CONTEXT_SIZE:			info->i           = sizeof(h83xx_state);		break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i           = 2;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i           = 10;							break;

		// Bus sizes
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 8;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 16;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:	info->i = 0;						break;

		// Internal maps
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map8 = address_map_h8_3334_internal_map; break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = NULL;	break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_IO:      info->internal_map16 = NULL;	break;

		// CPU misc parameters
	case CPUINFO_STR_NAME:					strcpy(info->s, "H8/3334");						break;
	case CPUINFO_STR_CORE_FILE:				strcpy(info->s, __FILE__);						break;
	case CPUINFO_STR_FLAGS:					strcpy(info->s, h8_get_ccr_str(h8));				break;
	case CPUINFO_INT_ENDIANNESS:				info->i = CPU_IS_BE;							break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1;									break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;									break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 16;									break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;									break;

		// CPU main state
	case CPUINFO_INT_PC:					info->i = h8->pc;								break;
	case CPUINFO_INT_PREVIOUSPC:				info->i = h8->ppc;								break;

	case CPUINFO_INT_REGISTER + H8_PC:			info->i = h8->pc;								break;
	case CPUINFO_INT_REGISTER + H8_CCR:			info->i = h8_get_ccr(h8);							break;

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

