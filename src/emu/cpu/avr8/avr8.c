/*
    Atmel 8-bit AVR emulator

    (Skeleton)

    DONE:
    - Disassembler
    - [lft]'s "Craft" depends on on-chip device support now instead of opcodes (it requires unusually few in order to boot)

    TODO:
    - Everything else
      * Finish opcode implementation
      * Add proper cycle timing
      * Add Interrupts
      * Add on-chip hardware (machine driver)

    Written by MooglyGuy
*/

#include "emu.h"
#include "debugger.h"
#include "avr8.h"

struct avr8_state
{
    UINT32 pc;

    legacy_cpu_device *device;
    address_space *program;
    address_space *io;
    int icount;
	UINT32 addr_mask;
};

enum
{
    AVR8_SREG_C = 0,
    AVR8_SREG_Z,
    AVR8_SREG_N,
    AVR8_SREG_V,
    AVR8_SREG_S,
    AVR8_SREG_H,
    AVR8_SREG_T,
    AVR8_SREG_I,
};

enum
{
    AVR8_IO_SPL = 0x5d,
    AVR8_IO_SPH = 0x5e,
    AVR8_IO_SREG = 0x5f,
};

#define SREG_R(b) ((READ_IO_8(cpustate, AVR8_IO_SREG) & (1 << (b))) >> (b))
#define SREG_W(b,v) WRITE_IO_8(cpustate, AVR8_IO_SREG, (READ_IO_8(cpustate, AVR8_IO_SREG) &~ (1 << (b))) | (((v) ? 1 : 0) << (b)))
#define NOT(x) (1 - (x))

#define RD2(op)         (((op) >> 4) & 0x0003)
#define RD3(op)         (((op) >> 4) & 0x0007)
#define RD4(op)         (((op) >> 4) & 0x000f)
#define RD5(op)         (((op) >> 4) & 0x001f)
#define RR3(op)         ((op) & 0x0007)
#define RR4(op)         ((op) & 0x000f)
#define RR5(op)         ((((op) >> 5) & 0x0010) | ((op) & 0x000f))
#define KCONST6(op)     ((((op) >> 2) & 0x0030) | ((op) & 0x000f))
#define KCONST7(op)     (((op) >> 3) & 0x007f)
#define KCONST8(op)     ((((op) >> 4) & 0x00f0) | ((op) & 0x000f))
#define KCONST22(op)    (((((UINT32)(op) >> 3) & 0x003e) | ((UINT32)(op) & 0x0001)) << 16)
#define QCONST6(op)     ((((op) >> 8) & 0x0020) | (((op) >> 7) & 0x0018) | ((op) & 0x0007))
#define ACONST5(op)     (((op) >> 3) & 0x001f)
#define ACONST6(op)     ((((op) >> 5) & 0x0030) | ((op) & 0x000f))

#define XREG            ((READ_IO_8(cpustate, 27) << 8) | READ_IO_8(cpustate, 26))
#define YREG            ((READ_IO_8(cpustate, 29) << 8) | READ_IO_8(cpustate, 28))
#define ZREG            ((READ_IO_8(cpustate, 31) << 8) | READ_IO_8(cpustate, 30))
#define SPREG           ((READ_IO_8(cpustate, AVR8_IO_SPH) << 8) | READ_IO_8(cpustate, AVR8_IO_SPL))

INLINE avr8_state *get_safe_token(device_t *device)
{
    assert(device != NULL);
    assert(device->type() == ATMEGA88 || device->type() == ATMEGA644);
    return (avr8_state *)downcast<legacy_cpu_device *>(device)->token();
}

/*****************************************************************************/

static void unimplemented_opcode(avr8_state *cpustate, UINT32 op)
{
    fatalerror("AVR8: unknown opcode (%08x) at %08x\n", op, cpustate->pc);
}

/*****************************************************************************/

INLINE bool avr8_is_long_opcode(UINT16 op)
{
	if((op & 0xf000) == 0x9000)
	{
		if((op & 0x0f00) < 0x0400)
		{
			if((op & 0x000f) == 0x0000)
			{
				return true;
			}
		}
		else if((op & 0x0f00) < 0x0600)
		{
			if((op & 0x000f) >= 0x000c)
			{
				return true;
			}
		}
	}
	return false;
}

INLINE UINT8 READ_PRG_8(avr8_state *cpustate, UINT32 address)
{
    return cpustate->program->read_byte(address);
}

INLINE UINT16 READ_PRG_16(avr8_state *cpustate, UINT32 address)
{
    return cpustate->program->read_word(address << 1);
}

INLINE void WRITE_PRG_8(avr8_state *cpustate, UINT32 address, UINT8 data)
{
    cpustate->program->write_byte(address, data);
}

INLINE void WRITE_PRG_16(avr8_state *cpustate, UINT32 address, UINT16 data)
{
    cpustate->program->write_word(address, data);
}

INLINE UINT8 READ_IO_8(avr8_state *cpustate, UINT16 address)
{
    return cpustate->io->read_byte(address);
}

INLINE void WRITE_IO_8(avr8_state *cpustate, UINT16 address, UINT8 data)
{
    cpustate->io->write_byte(address, data);
}

INLINE void PUSH(avr8_state *cpustate, UINT8 val)
{
    UINT16 sp = SPREG;
    WRITE_IO_8(cpustate, sp, val);
    sp--;
    //printf( "PUSH %02x, new SP = %04x\n", val, sp );
    WRITE_IO_8(cpustate, AVR8_IO_SPH, (sp >> 8) & 0x00ff);
    WRITE_IO_8(cpustate, AVR8_IO_SPL, sp & 0x00ff);
}

INLINE UINT8 POP(avr8_state *cpustate)
{
    UINT16 sp = SPREG;
    sp++;
    WRITE_IO_8(cpustate, AVR8_IO_SPH, (sp >> 8) & 0x00ff);
    WRITE_IO_8(cpustate, AVR8_IO_SPL, sp & 0x00ff);
    //printf( "POP %02x, new SP = %04x\n", READ_IO_8(cpustate, sp), sp );
    return READ_IO_8(cpustate, sp);
}

static void avr8_set_irq_line(avr8_state *cpustate, UINT16 vector, int state)
{
    //printf( "OMFG SETTING IRQ LINE\n" );
    // Horrible hack, not accurate
    if(state)
    {
		if(SREG_R(AVR8_SREG_I))
		{
			SREG_W(AVR8_SREG_I, 0);
			PUSH(cpustate, (cpustate->pc >> 8) & 0x00ff);
			PUSH(cpustate, cpustate->pc & 0x00ff);
			cpustate->pc = vector;
		}
    }
}

/*****************************************************************************/

static CPU_INIT( avr8 )
{
    avr8_state *cpustate = get_safe_token(device);

    cpustate->pc = 0;

    cpustate->device = device;
    cpustate->program = device->space(AS_PROGRAM);
    cpustate->io = device->space(AS_IO);

    WRITE_IO_8(cpustate, AVR8_IO_SREG, 0);

	device->save_item(NAME(cpustate->pc));
}

static CPU_EXIT( avr8 )
{
}

static CPU_RESET( avr8 )
{
    avr8_state *cpustate = get_safe_token(device);

    WRITE_IO_8(cpustate, AVR8_IO_SREG, 0);
    cpustate->pc = 0;
}

static CPU_EXECUTE( avr8 )
{
    UINT32 op = 0;
    INT32 offs = 0;
    UINT8 rd = 0;
    UINT8 rr = 0;
    UINT8 res = 0;
    UINT16 pd = 0;
    INT16 sd = 0;
    INT32 opcycles = 1;
    //UINT16 pr = 0;
    avr8_state *cpustate = get_safe_token(device);

    while (cpustate->icount > 0)
    {
        cpustate->pc &= cpustate->addr_mask;

        debugger_instruction_hook(device, cpustate->pc << 1);

        op = (UINT32)READ_PRG_16(cpustate, cpustate->pc);

        switch(op & 0xf000)
        {
            case 0x0000:
                switch(op & 0x0f00)
                {
                    case 0x0000:    // NOP
                        break;
                    case 0x0100:    // MOVW Rd+1:Rd,Rr+1:Rd
                        WRITE_IO_8(cpustate, (RD4(op) << 1)+1, READ_IO_8(cpustate, (RR4(op) << 1)+1));
                        WRITE_IO_8(cpustate, RD4(op) << 1, READ_IO_8(cpustate, RR4(op) << 1));
                        break;
                    case 0x0200:    // MULS Rd,Rr
                        //output += sprintf( output, "MULS    R%d, R%d", 16+RD4(op), 16+RR4(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0300:    // MULSU Rd,Rr
                        sd = (INT8)READ_IO_8(cpustate, 16+RD4(op)) * (UINT8)READ_IO_8(cpustate, 16+RR4(op));
                        WRITE_IO_8(cpustate, 1, (sd >> 8) & 0x00ff);
                        WRITE_IO_8(cpustate, 0, sd & 0x00ff);
                        SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
                        SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
                        opcycles = 2;
                        break;
                    case 0x0400:
                    case 0x0500:
                    case 0x0600:
                    case 0x0700:    // CPC Rd,Rr
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        res = rd - (rr + SREG_R(AVR8_SREG_C));
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0800:
                    case 0x0900:
                    case 0x0a00:
                    case 0x0b00:    // SBC Rd,Rr
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        res = rd - (rr + SREG_R(AVR8_SREG_C));
                        WRITE_IO_8(cpustate, RD5(op), res);
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0c00:
                    case 0x0d00:
                    case 0x0e00:
                    case 0x0f00:    // ADD Rd,Rr
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        res = rd + rr;
                        WRITE_IO_8(cpustate, RD5(op), res);
                        SREG_W(AVR8_SREG_H, (BIT(rd,3) & BIT(rr,3)) | (BIT(rr,3) & NOT(BIT(res,3))) | (NOT(BIT(res,3)) & BIT(rd,3)));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & NOT(BIT(rr,7)) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (BIT(rd,7) & BIT(rr,7)) | (BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(res,7)) & BIT(rd,7)));
                        break;
                }
                break;
            case 0x1000:
                switch(op & 0x0c00)
                {
                    case 0x0000:    // CPSR Rd,Rr
                        //output += sprintf( output, "CPSE    R%d, R%d", RD5(op), RR5(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0400:    // CP Rd,Rr
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        res = rd - rr;
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0800:    // SUB Rd,Rr
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        res = rd - rr;
                        WRITE_IO_8(cpustate, RD5(op), res);
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0c00:    // ADC Rd,Rr
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        res = rd + rr + SREG_R(AVR8_SREG_C);
                        WRITE_IO_8(cpustate, RD5(op), res);
                        SREG_W(AVR8_SREG_H, (BIT(rd,3) & BIT(rr,3)) | (BIT(rr,3) & NOT(BIT(res,3))) | (NOT(BIT(res,3)) & BIT(rd,3)));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & NOT(BIT(rr,7)) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (BIT(rd,7) & BIT(rr,7)) | (BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(res,7)) & BIT(rd,7)));
                        break;
                }
                break;
            case 0x2000:
                switch(op & 0x0c00)
                {
                    case 0x0000:    // AND Rd,Rr
                        //output += sprintf( output, "AND     R%d, R%d", RD5(op), RR5(op) );
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        rd &= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, rd & 0x80);
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        WRITE_IO_8(cpustate, RD5(op), rd);
                        break;
                    case 0x0400:    // EOR Rd,Rr
                        rd = READ_IO_8(cpustate, RD5(op));
                        rr = READ_IO_8(cpustate, RR5(op));
                        rd ^= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, rd & 0x80);
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        WRITE_IO_8(cpustate, RD5(op), rd);
                        break;
                    case 0x0800:    // OR Rd,Rr
                        //output += sprintf( output, "OR      R%d, R%d", RD5(op), RR5(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0c00:    // MOV Rd,Rr
                        WRITE_IO_8(cpustate, RD5(op), READ_IO_8(cpustate, RR5(op)));
                        break;
                }
                break;
            case 0x3000:    // CPI Rd,K
                rd = READ_IO_8(cpustate, 16+RD4(op));
                rr = KCONST8(op);
                res = rd - rr;
                SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                SREG_W(AVR8_SREG_N, BIT(res,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                break;
            case 0x4000:    // SBCI Rd,K
                //output += sprintf( output, "SBCI    R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
                unimplemented_opcode(cpustate, op);
                break;
            case 0x5000:    // SUBI Rd,K
                rd = READ_IO_8(cpustate, 16+RD4(op));
                rr = KCONST8(op);
                res = rd - rr;
                WRITE_IO_8(cpustate, 16+RD4(op), res);
                SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                SREG_W(AVR8_SREG_N, BIT(res,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                break;
            case 0x6000:    // ORI Rd,K
                //output += sprintf( output, "ORI     R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
                unimplemented_opcode(cpustate, op);
                break;
            case 0x7000:    // ANDI Rd,K
                //output += sprintf( output, "ANDI    R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
                unimplemented_opcode(cpustate, op);
                break;
            case 0x8000:
            case 0xa000:
                switch(op & 0x0208)
                {
                    case 0x0000:    // LDD Rd,Z+q
                        //output += sprintf( output, "LD(D)   R%d, Z+%02x", RD5(op), QCONST6(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0008:    // LDD Rd,Y+q
                        WRITE_IO_8(cpustate, RD5(op), YREG + QCONST6(op));
                        opcycles = 2;
                        break;
                    case 0x0200:    // STD Z+q,Rr
                        //output += sprintf( output, "ST(D)   Z+%02x, R%d", QCONST6(op), RD5(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0208:    // STD Y+q,Rr
                        WRITE_IO_8(cpustate, YREG + QCONST6(op), READ_IO_8(cpustate, RD5(op)));
                        opcycles = 2;
                        break;
                }
                break;
            case 0x9000:
                switch(op & 0x0f00)
                {
                    case 0x0000:
                    case 0x0100:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // LDS Rd,k
                                op <<= 16;
                                cpustate->pc++;
                                op |= READ_PRG_16(cpustate, cpustate->pc);
                                WRITE_IO_8(cpustate, RD5(op >> 16), READ_IO_8(cpustate, op & 0x0000ffff));
                                opcycles = 2;
                                break;
                            case 0x0001:    // LD Rd,Z+
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0002:    // LD Rd,-Z
                                //output += sprintf( output, "LD      R%d,-Z", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0004:    // LPM Rd,Z
                                WRITE_IO_8(cpustate, RD5(op), READ_PRG_8(cpustate, ZREG));
                                opcycles = 3;
                                break;
                            case 0x0005:    // LPM Rd,Z+
                                pd = ZREG;
                                WRITE_IO_8(cpustate, RD5(op), READ_PRG_8(cpustate, pd));
                                pd++;
                                WRITE_IO_8(cpustate, 31, (pd >> 8) & 0x00ff);
                                WRITE_IO_8(cpustate, 30, pd & 0x00ff);
                                opcycles = 3;
                                break;
                            case 0x0006:    // ELPM Rd,Z
                                //output += sprintf( output, "ELPM    R%d, Z", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0007:    // ELPM Rd,Z+
                                //output += sprintf( output, "ELPM    R%d, Z+", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0009:    // LD Rd,Y+
                                //output += sprintf( output, "LD      R%d, Y+", RD5(op) );
                                 unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000a:    // LD Rd,-Y
                                //output += sprintf( output, "LD      R%d,-Y", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000c:    // LD Rd,X
                                //output += sprintf( output, "LD      R%d, X", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000d:    // LD Rd,X+
                                pd = XREG;
                                WRITE_IO_8(cpustate, RD5(op), READ_IO_8(cpustate, pd));
                                pd++;
                                WRITE_IO_8(cpustate, 27, (pd >> 8) & 0x00ff);
                                WRITE_IO_8(cpustate, 26, pd & 0x00ff);
                                opcycles = 2;
                                break;
                            case 0x000e:    // LD Rd,-X
                                //output += sprintf( output, "LD      R%d,-X", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000f:    // POP Rd
                                WRITE_IO_8(cpustate, RD5(op), POP(cpustate));
                                opcycles = 2;
                                break;
                            default:
                                unimplemented_opcode(cpustate, op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0200:
                    case 0x0300:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // STS k,Rr
                                op <<= 16;
                                cpustate->pc++;
                                op |= READ_PRG_16(cpustate, cpustate->pc);
                                WRITE_IO_8(cpustate, op & 0x0000ffff, READ_IO_8(cpustate, RD5(op >> 16)));
                                opcycles = 2;
                                break;
                            case 0x0001:    // ST Z+,Rd
                                //output += sprintf( output, "ST       Z+, R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0002:    // ST -Z,Rd
                                //output += sprintf( output, "ST      -Z , R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0009:    // ST Y+,Rd
                                //output += sprintf( output, "ST       Y+, R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000a:    // ST -Z,Rd
                                //output += sprintf( output, "ST      -Y , R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000c:    // ST X,Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                WRITE_IO_8(cpustate, XREG, rd);
                                opcycles = 2;
                                break;
                            case 0x000d:    // ST X+,Rd
                                pd = XREG;
                                WRITE_IO_8(cpustate, pd, READ_IO_8(cpustate, RD5(op)));
                                pd++;
                                WRITE_IO_8(cpustate, 27, (pd >> 8) & 0x00ff);
                                WRITE_IO_8(cpustate, 26, pd & 0x00ff);
                                opcycles = 2;
                                break;
                            case 0x000e:    // ST -X,Rd
                                //output += sprintf( output, "ST      -X , R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000f:    // PUSH Rd
                                PUSH(cpustate, READ_IO_8(cpustate, RD5(op)));
                                opcycles = 2;
                                break;
                            default:
                                unimplemented_opcode(cpustate, op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0400:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // COM Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                rd = ~rd;
                                SREG_W(AVR8_SREG_C, 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) | SREG_R(AVR8_SREG_V));
                                break;
                            case 0x0001:    // NEG Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = 0 - rd;
                                SREG_W(AVR8_SREG_C, (res == 0) ? 0 : 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, (res == 0x80) ? 1 : 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) | SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_H, BIT(res,3) | BIT(rd,3));
                                break;
                            case 0x0002:    // SWAP Rd
                                //output += sprintf( output, "SWAP    R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0003:    // INC Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = rd + 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                WRITE_IO_8(cpustate, RD5(op), res);
                                break;
                            case 0x0005:    // ASR Rd
                                //output += sprintf( output, "ASR     R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0006:    // LSR Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = rd >> 1;
                                SREG_W(AVR8_SREG_C, rd & 0x01);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, 0);
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                break;
                            case 0x0007:    // ROR Rd
                                //output += sprintf( output, "ROR     R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0008:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // SEC
                                    case 0x0010:    // SEZ
                                    case 0x0020:    // SEN
                                    case 0x0030:    // SEV
                                    case 0x0040:    // SES
                                    case 0x0050:    // SEH
                                    case 0x0060:    // SET
                                    case 0x0070:    // SEI
                                        SREG_W((op >> 4) & 0x07, 1);
                                        break;
                                    case 0x0080:    // CLC
                                    case 0x0090:    // CLZ
                                    case 0x00a0:    // CLN
                                    case 0x00b0:    // CLV
                                    case 0x00c0:    // CLS
                                    case 0x00d0:    // CLH
                                    case 0x00e0:    // CLT
                                    case 0x00f0:    // CLI
                                        SREG_W((op >> 4) & 0x07, 0);
                                        break;
                                }
                                break;
                            case 0x0009:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // IJMP
                                        cpustate->pc = ZREG - 1;
                                        opcycles = 2;
                                        break;
                                    case 0x0010:    // EIJMP
                                        //output += sprintf( output, "EIJMP" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    default:
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                }
                                break;
                            case 0x000a:    // DEC Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = rd - 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                WRITE_IO_8(cpustate, RD5(op), res);
                                break;
                            case 0x000c:
                            case 0x000d:    // JMP k
								offs = KCONST22(op) << 16;
                                cpustate->pc++;
                                offs |= READ_PRG_16(cpustate, cpustate->pc);
                                cpustate->pc = offs;
								cpustate->pc--;
								opcycles = 4;
                                break;
                            case 0x000e:    // CALL k
                            case 0x000f:
								PUSH(cpustate, ((cpustate->pc + 1) >> 8) & 0x00ff);
								PUSH(cpustate, (cpustate->pc + 1) & 0x00ff);
								offs = KCONST22(op) << 16;
                                cpustate->pc++;
                                offs |= READ_PRG_16(cpustate, cpustate->pc);
                                cpustate->pc = offs;
								cpustate->pc--;
                                break;
                            default:
                                unimplemented_opcode(cpustate, op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0500:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // COM Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                rd = ~rd;
                                SREG_W(AVR8_SREG_C, 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) | SREG_R(AVR8_SREG_V));
                                break;
                            case 0x0001:    // NEG Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = 0 - rd;
                                WRITE_IO_8(cpustate, RD5(op), res);
                                SREG_W(AVR8_SREG_C, (res == 0) ? 0 : 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, (res == 0x80) ? 1 : 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) | SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_H, BIT(res,3) | BIT(rd,3));
                                break;
                            case 0x0002:    // SWAP Rd
                                //output += sprintf( output, "SWAP    R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0003:    // INC Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = rd + 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                WRITE_IO_8(cpustate, RD5(op), res);
                                break;
                            case 0x0005:    // ASR Rd
                                //output += sprintf( output, "ASR     R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0006:    // LSR Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = rd >> 1;
                                SREG_W(AVR8_SREG_C, rd & 0x01);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, 0);
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                break;
                            case 0x0007:    // ROR Rd
                                //output += sprintf( output, "ROR     R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0008:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // RET
                                        cpustate->pc = POP(cpustate);
                                        cpustate->pc |= POP(cpustate) << 8;
                                        cpustate->pc--;
                                        opcycles = 4;
                                        break;
                                    case 0x0010:    // RETI
                                        cpustate->pc = POP(cpustate);
                                        cpustate->pc |= POP(cpustate) << 8;
                                        cpustate->pc--;
                                        SREG_W(AVR8_SREG_I, 1);
                                        opcycles = 4;
                                        break;
                                    case 0x0080:    // SLEEP
                                        //output += sprintf( output, "SLEEP" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x0090:    // BREAK
                                        //output += sprintf( output, "BREAK" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00a0:    // WDR
                                        //output += sprintf( output, "WDR" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00c0:    // LPM
                                        WRITE_IO_8(cpustate, 0, READ_PRG_8(cpustate, ZREG));
                                        opcycles = 3;
                                        break;
                                    case 0x00d0:    // ELPM
                                        //output += sprintf( output, "ELPM" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00e0:    // SPM
                                        //output += sprintf( output, "SPM" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00f0:    // SPM Z+
                                        //output += sprintf( output, "SPM     Z+" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    default:
                                        unimplemented_opcode(cpustate, op);
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        break;
                                }
                                break;
                            case 0x0009:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // ICALL
                                        //output += sprintf( output, "ICALL" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x0010:    // EICALL
                                        //output += sprintf( output, "EICALL" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    default:
                                        unimplemented_opcode(cpustate, op);
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        break;
                                }
                                break;
                            case 0x000a:    // DEC Rd
                                rd = READ_IO_8(cpustate, RD5(op));
                                res = rd - 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                WRITE_IO_8(cpustate, RD5(op), res);
                                break;
                            case 0x000c:
                            case 0x000d:    // JMP k
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //output += sprintf( output, "JMP     0x%06x", KCONST22(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000e:
                            case 0x000f:    // CALL k
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //output += sprintf( output, "CALL    0x%06x", KCONST22(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                        }
                        break;
                    case 0x0600:    // ADIW Rd+1:Rd,K
                        //output += sprintf( output, "ADIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0700:    // SBIW Rd+1:Rd,K
                        //output += sprintf( output, "SBIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0800:    // CBI A,b
                        //output += sprintf( output, "CBI     0x%02x, %d", ACONST5(op), RR3(op) );
                        WRITE_IO_8(cpustate, ACONST5(op), READ_IO_8(cpustate, ACONST5(op)) &~ (1 << RR3(op)));
                        opcycles = 2;
                        break;
                    case 0x0900:    // SBIC A,b
                		if(!(READ_IO_8(cpustate, ACONST5(op)) & (1 << RR3(op))))
                		{
							opcycles += avr8_is_long_opcode(op) ? 2 : 1;
                            cpustate->pc += avr8_is_long_opcode(op) ? 2 : 1;
						}
                        break;
                    case 0x0a00:    // SBI A,b
                        //output += sprintf( output, "SBI     0x%02x, %d", ACONST5(op), RR3(op) );
                        WRITE_IO_8(cpustate, ACONST5(op), READ_IO_8(cpustate, ACONST5(op)) | (1 << RR3(op)));
                        opcycles = 2;
                        break;
                    case 0x0b00:    // SBIS A,b
                        //output += sprintf( output, "SBIS    0x%02x, %d", ACONST5(op), RR3(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0c00:
                    case 0x0d00:
                    case 0x0e00:
                    case 0x0f00:    // MUL Rd,Rr
                        sd = (UINT8)READ_IO_8(cpustate, RD5(op)) * (UINT8)READ_IO_8(cpustate, RR5(op));
                        WRITE_IO_8(cpustate, 1, (sd >> 8) & 0x00ff);
                        WRITE_IO_8(cpustate, 0, sd & 0x00ff);
                        SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
                        SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
                        opcycles = 2;
                        //output += sprintf( output, "MUL     R%d, R%d", RD5(op), RR5(op) );
                        break;
                }
                break;
            case 0xb000:
                if(op & 0x0800) // OUT A,Rr
                {
                    WRITE_IO_8(cpustate, 0x20 + ACONST6(op), READ_IO_8(cpustate, RD5(op)));
                }
                else            // IN Rd,A
                {
                    WRITE_IO_8(cpustate, RD5(op), READ_IO_8(cpustate, 0x20 + ACONST6(op)));
                }
                break;
            case 0xc000:    // RJMP k
                offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
                cpustate->pc += offs;
                opcycles = 2;
                break;
            case 0xd000:    // RCALL k
                offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
                PUSH(cpustate, ((cpustate->pc + 1) >> 8) & 0x00ff);
                PUSH(cpustate, (cpustate->pc + 1) & 0x00ff);
                cpustate->pc += offs;
                opcycles = 3;
                break;
            case 0xe000:    // LDI Rd,K
                rd = KCONST8(op);
                WRITE_IO_8(cpustate, 16 + RD4(op), rd);
                break;
            case 0xf000:
                switch(op & 0x0c00)
                {
                    case 0x0000: // BRLO through BRIE
                        if(SREG_R(op & 0x0007))
                        {
                            offs = (INT32)(KCONST7(op));
                            if(offs & 0x40)
                            {
                                offs |= 0xffffff80;
                            }
                            cpustate->pc += offs;
                            opcycles = 2;
                        }
                        break;
                    case 0x0400: // BRSH through BRID
                        if(SREG_R(op & 0x0007) == 0)
                        {
                            offs = (INT32)(KCONST7(op));
                            if(offs & 0x40)
                            {
                                offs |= 0xffffff80;
                            }
                            cpustate->pc += offs;
                            opcycles = 2;
                        }
                        break;
                    case 0x0800:
                        if(op & 0x0200) // BST Rd, b
                        {
                            SREG_W(AVR8_SREG_T, (BIT(READ_IO_8(cpustate, RD5(op)),RR3(op))) ? 1 : 0);
                        }
                        else            // BLD Rd, b
                        {
                            if(SREG_R(AVR8_SREG_T))
                            {
                                WRITE_IO_8(cpustate, RD5(op), READ_IO_8(cpustate, RD5(op)) | (1 << RR3(op)));
                            }
                            else
                            {
                                WRITE_IO_8(cpustate, RD5(op), READ_IO_8(cpustate, RD5(op)) &~ (1 << RR3(op)));
                            }
                        }
                        break;
                    case 0x0c00:
                        if(op & 0x0200) // SBRS Rd, b
                        {
                            if(BIT(READ_IO_8(cpustate, RD5(op)),RR3(op)))
                            {
                                op = (UINT32)READ_PRG_16(cpustate, cpustate->pc++);
                                opcycles = 2;
                                if((op & 0xfe0c) == 0x940c ||
                                   (op & 0xfe0f) == 0xfe0f)
                                {
                                    cpustate->pc++;
                                    opcycles = 3;
                                }
                            }
                        }
                        else            // SBRC Rd, b
                        {
                            if(!BIT(READ_IO_8(cpustate, RD5(op)),RR3(op)))
                            {
                                op = (UINT32)READ_PRG_16(cpustate, cpustate->pc++);
                                opcycles = 2;
                                if((op & 0xfe0c) == 0x940c ||
                                   (op & 0xfc0f) == 0x9000)
                                {
                                    cpustate->pc++;
                                    opcycles = 3;
                                }
                            }
                        }
                        break;
                }
                break;
        }

        cpustate->pc++;

        cpustate->icount -= opcycles;
    }
}

/*****************************************************************************/

static CPU_SET_INFO( avr8 )
{
    avr8_state *cpustate = get_safe_token(device);

    switch (state)
    {

        /* interrupt lines/exceptions */
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_RESET:          avr8_set_irq_line(cpustate, AVR8_INT_RESET, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_INT0:           avr8_set_irq_line(cpustate, AVR8_INT_INT0, info->i);        break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_INT1:           avr8_set_irq_line(cpustate, AVR8_INT_INT1, info->i);        break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_PCINT0:         avr8_set_irq_line(cpustate, AVR8_INT_PCINT0, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_PCINT1:         avr8_set_irq_line(cpustate, AVR8_INT_PCINT1, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_PCINT2:         avr8_set_irq_line(cpustate, AVR8_INT_PCINT2, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_WDT:            avr8_set_irq_line(cpustate, AVR8_INT_WDT, info->i);         break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T2COMPA:        avr8_set_irq_line(cpustate, AVR8_INT_T2COMPA, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T2COMPB:        avr8_set_irq_line(cpustate, AVR8_INT_T2COMPB, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T2OVF:          avr8_set_irq_line(cpustate, AVR8_INT_T2OVF, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1CAPT:         avr8_set_irq_line(cpustate, AVR8_INT_T1CAPT, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1COMPA:        avr8_set_irq_line(cpustate, AVR8_INT_T1COMPA, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1COMPB:        avr8_set_irq_line(cpustate, AVR8_INT_T1COMPB, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1OVF:          avr8_set_irq_line(cpustate, AVR8_INT_T1OVF, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T0COMPA:        avr8_set_irq_line(cpustate, AVR8_INT_T0COMPA, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T0COMPB:        avr8_set_irq_line(cpustate, AVR8_INT_T0COMPB, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T0OVF:          avr8_set_irq_line(cpustate, AVR8_INT_T0OVF, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_SPI_STC:        avr8_set_irq_line(cpustate, AVR8_INT_SPI_STC, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_USART_RX:       avr8_set_irq_line(cpustate, AVR8_INT_USART_RX, info->i);    break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_USART_UDRE:     avr8_set_irq_line(cpustate, AVR8_INT_USART_UDRE, info->i);  break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_USART_TX:       avr8_set_irq_line(cpustate, AVR8_INT_USART_TX, info->i);    break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_ADC:            avr8_set_irq_line(cpustate, AVR8_INT_ADC, info->i);         break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_EE_RDY:         avr8_set_irq_line(cpustate, AVR8_INT_EE_RDY, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_ANALOG_COMP:    avr8_set_irq_line(cpustate, AVR8_INT_ANALOG_COMP, info->i); break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_TWI:            avr8_set_irq_line(cpustate, AVR8_INT_TWI, info->i);         break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_SPM_RDY:        avr8_set_irq_line(cpustate, AVR8_INT_SPM_RDY, info->i);     break;

        /* --- the following bits of info are set as 64-bit signed integers --- */
        case CPUINFO_INT_PC:    /* intentional fallthrough */
        case CPUINFO_INT_REGISTER + AVR8_PC:            cpustate->pc = info->i;                         break;
        case CPUINFO_INT_REGISTER + AVR8_SREG:          WRITE_IO_8(cpustate, AVR8_IO_SREG, info->i);    break;
        case CPUINFO_INT_REGISTER + AVR8_R0:            WRITE_IO_8(cpustate, 0, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R1:            WRITE_IO_8(cpustate, 1, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R2:            WRITE_IO_8(cpustate, 2, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R3:            WRITE_IO_8(cpustate, 3, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R4:            WRITE_IO_8(cpustate, 4, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R5:            WRITE_IO_8(cpustate, 5, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R6:            WRITE_IO_8(cpustate, 6, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R7:            WRITE_IO_8(cpustate, 7, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R8:            WRITE_IO_8(cpustate, 8, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R9:            WRITE_IO_8(cpustate, 9, info->i);               break;
        case CPUINFO_INT_REGISTER + AVR8_R10:           WRITE_IO_8(cpustate, 10, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R11:           WRITE_IO_8(cpustate, 11, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R12:           WRITE_IO_8(cpustate, 12, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R13:           WRITE_IO_8(cpustate, 13, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R14:           WRITE_IO_8(cpustate, 14, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R15:           WRITE_IO_8(cpustate, 15, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R16:           WRITE_IO_8(cpustate, 16, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R17:           WRITE_IO_8(cpustate, 17, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R18:           WRITE_IO_8(cpustate, 18, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R19:           WRITE_IO_8(cpustate, 19, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R20:           WRITE_IO_8(cpustate, 20, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R21:           WRITE_IO_8(cpustate, 21, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R22:           WRITE_IO_8(cpustate, 22, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R23:           WRITE_IO_8(cpustate, 23, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R24:           WRITE_IO_8(cpustate, 24, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R25:           WRITE_IO_8(cpustate, 25, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R26:           WRITE_IO_8(cpustate, 26, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R27:           WRITE_IO_8(cpustate, 27, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R28:           WRITE_IO_8(cpustate, 28, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R29:           WRITE_IO_8(cpustate, 29, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R30:           WRITE_IO_8(cpustate, 30, info->i);              break;
        case CPUINFO_INT_REGISTER + AVR8_R31:           WRITE_IO_8(cpustate, 31, info->i);              break;
    }
}

CPU_GET_INFO( avr8 )
{
    avr8_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

    switch(state)
    {
        /* --- the following bits of info are returned as 64-bit signed integers --- */
        case CPUINFO_INT_CONTEXT_SIZE:          info->i = sizeof(avr8_state);   break;
        case CPUINFO_INT_INPUT_LINES:           info->i = 0;                    break;
        case CPUINFO_INT_DEFAULT_IRQ_VECTOR:    info->i = 0;                    break;
        case CPUINFO_INT_ENDIANNESS:            info->i = ENDIANNESS_LITTLE;    break;
        case CPUINFO_INT_CLOCK_MULTIPLIER:      info->i = 1;                    break;
        case CPUINFO_INT_CLOCK_DIVIDER:         info->i = 1;                    break;
        case CPUINFO_INT_MIN_INSTRUCTION_BYTES: info->i = 2;                    break;
        case CPUINFO_INT_MAX_INSTRUCTION_BYTES: info->i = 4;                    break;
        case CPUINFO_INT_MIN_CYCLES:            info->i = 1;                    break;
        case CPUINFO_INT_MAX_CYCLES:            info->i = 4;                    break;

        case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM: info->i = 8;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 22;                   break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_IO:      info->i = 8;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:      info->i = 11;                   break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:      info->i = 0;                    break;

        case CPUINFO_INT_PC:    /* intentional fallthrough */
        case CPUINFO_INT_REGISTER + AVR8_PC:    info->i = cpustate->pc << 1;                    break;
        case CPUINFO_INT_REGISTER + AVR8_SREG:  info->i = READ_IO_8(cpustate, AVR8_IO_SREG);    break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
        case CPUINFO_FCT_SET_INFO:              info->setinfo = CPU_SET_INFO_NAME(avr8);        break;
        case CPUINFO_FCT_INIT:                  info->init = CPU_INIT_NAME(avr8);               break;
        case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(avr8);             break;
        case CPUINFO_FCT_EXIT:                  info->exit = CPU_EXIT_NAME(avr8);               break;
        case CPUINFO_FCT_EXECUTE:               info->execute = CPU_EXECUTE_NAME(avr8);         break;
        case CPUINFO_FCT_BURN:                  info->burn = NULL;                              break;
        case CPUINFO_FCT_DISASSEMBLE:           info->disassemble = CPU_DISASSEMBLE_NAME(avr8); break;
        case CPUINFO_PTR_INSTRUCTION_COUNTER:   info->icount = &cpustate->icount;               break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:                          strcpy(info->s, "Atmel 8-bit AVR");     break;
        case CPUINFO_STR_FAMILY:                   strcpy(info->s, "AVR8");                break;
        case CPUINFO_STR_VERSION:                  strcpy(info->s, "1.0");                 break;
        case CPUINFO_STR_SOURCE_FILE:                     strcpy(info->s, __FILE__);              break;
        case CPUINFO_STR_CREDITS:                  strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

        case CPUINFO_STR_FLAGS:                         strcpy(info->s, " ");                   break;

        case CPUINFO_STR_REGISTER + AVR8_SREG:          sprintf(info->s, "SREG: %c%c%c%c%c%c%c%c", (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x80) ? 'I' : '-', (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x40) ? 'T' : '-', (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x20) ? 'H' : '-', (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x10) ? 'S' : '-', (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x08) ? 'V' : '-', (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x04) ? 'N' : '-', (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x02) ? 'Z' : '-', (READ_IO_8(cpustate, AVR8_IO_SREG) & 0x01) ? 'C' : '-'); break;
        case CPUINFO_STR_REGISTER + AVR8_R0:            sprintf(info->s, "R0:  %02x", READ_IO_8(cpustate, 0) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R1:            sprintf(info->s, "R1:  %02x", READ_IO_8(cpustate, 1) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R2:            sprintf(info->s, "R2:  %02x", READ_IO_8(cpustate, 2) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R3:            sprintf(info->s, "R3:  %02x", READ_IO_8(cpustate, 3) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R4:            sprintf(info->s, "R4:  %02x", READ_IO_8(cpustate, 4) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R5:            sprintf(info->s, "R5:  %02x", READ_IO_8(cpustate, 5) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R6:            sprintf(info->s, "R6:  %02x", READ_IO_8(cpustate, 6) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R7:            sprintf(info->s, "R7:  %02x", READ_IO_8(cpustate, 7) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R8:            sprintf(info->s, "R8:  %02x", READ_IO_8(cpustate, 8) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R9:            sprintf(info->s, "R9:  %02x", READ_IO_8(cpustate, 9) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R10:           sprintf(info->s, "R10: %02x", READ_IO_8(cpustate, 10) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R11:           sprintf(info->s, "R11: %02x", READ_IO_8(cpustate, 11) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R12:           sprintf(info->s, "R12: %02x", READ_IO_8(cpustate, 12) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R13:           sprintf(info->s, "R13: %02x", READ_IO_8(cpustate, 13) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R14:           sprintf(info->s, "R14: %02x", READ_IO_8(cpustate, 14) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R15:           sprintf(info->s, "R15: %02x", READ_IO_8(cpustate, 15) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R16:           sprintf(info->s, "R16: %02x", READ_IO_8(cpustate, 16) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R17:           sprintf(info->s, "R17: %02x", READ_IO_8(cpustate, 17) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R18:           sprintf(info->s, "R18: %02x", READ_IO_8(cpustate, 18) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R19:           sprintf(info->s, "R19: %02x", READ_IO_8(cpustate, 19) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R20:           sprintf(info->s, "R20: %02x", READ_IO_8(cpustate, 20) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R21:           sprintf(info->s, "R21: %02x", READ_IO_8(cpustate, 21) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R22:           sprintf(info->s, "R22: %02x", READ_IO_8(cpustate, 22) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R23:           sprintf(info->s, "R23: %02x", READ_IO_8(cpustate, 23) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R24:           sprintf(info->s, "R24: %02x", READ_IO_8(cpustate, 24) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R25:           sprintf(info->s, "R25: %02x", READ_IO_8(cpustate, 25) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R26:           sprintf(info->s, "R26: %02x", READ_IO_8(cpustate, 26) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R27:           sprintf(info->s, "R27: %02x", READ_IO_8(cpustate, 27) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R28:           sprintf(info->s, "R28: %02x", READ_IO_8(cpustate, 28) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R29:           sprintf(info->s, "R29: %02x", READ_IO_8(cpustate, 29) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R30:           sprintf(info->s, "R30: %02x", READ_IO_8(cpustate, 30) ); break;
        case CPUINFO_STR_REGISTER + AVR8_R31:           sprintf(info->s, "R31: %02x", READ_IO_8(cpustate, 31) ); break;
        case CPUINFO_STR_REGISTER + AVR8_X:             sprintf(info->s, "X: %04x", XREG ); break;
        case CPUINFO_STR_REGISTER + AVR8_Y:             sprintf(info->s, "Y: %04x", YREG ); break;
        case CPUINFO_STR_REGISTER + AVR8_Z:             sprintf(info->s, "Z: %04x", ZREG ); break;
        case CPUINFO_STR_REGISTER + AVR8_SP:            sprintf(info->s, "SP: %04x", SPREG ); break;
    }
}

static CPU_INIT( atmega88 )
{
	CPU_INIT_CALL(avr8);
    avr8_state *cpustate = get_safe_token(device);
	cpustate->addr_mask = 0x0fff;
}

static CPU_INIT( atmega644 )
{
	CPU_INIT_CALL(avr8);
    avr8_state *cpustate = get_safe_token(device);
	cpustate->addr_mask = 0xffff;
}

CPU_GET_INFO( atmega88 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(atmega88);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "ATmega88");				break;

		default:										CPU_GET_INFO_CALL(avr8); break;
	}
}

CPU_GET_INFO( atmega644 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(atmega644);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "ATmega644");				break;

		default:										CPU_GET_INFO_CALL(avr8); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(ATMEGA88, atmega88);
DEFINE_LEGACY_CPU_DEVICE(ATMEGA644, atmega644);
