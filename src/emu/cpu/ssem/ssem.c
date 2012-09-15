/*
    Manchester Small-Scale Experimental Machine (SSEM) emulator

    Written by MooglyGuy
*/

#include "emu.h"
#include "debugger.h"
#include "ssem.h"

CPU_DISASSEMBLE( ssem );


#define SSEM_DISASM_ON_UNIMPL           0
#define SSEM_DUMP_MEM_ON_UNIMPL         0

struct ssem_state
{
    UINT32 pc;
    UINT32 a;
    UINT32 halt;

    legacy_cpu_device *device;
    address_space *program;
    int icount;
};

INLINE ssem_state *get_safe_token(device_t *device)
{
    assert(device != NULL);
    assert(device->type() == SSEM);
    return (ssem_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define INSTR       ((op >> 13) & 7)
#define ADDR        (op & 0x1f)

/*****************************************************************************/

// The SSEM stores its data, visually, with the leftmost bit corresponding to the least significant bit.
// The de facto snapshot format for other SSEM simulators stores the data physically in that format as well.
// Therefore, in MESS, every 32-bit word has its bits reversed, too, and as a result the values must be
// un-reversed before being used.
INLINE UINT32 reverse(UINT32 v)
{
    // Taken from http://www-graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
    // swap odd and even bits
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    // swap consecutive pairs
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    // swap nibbles ...
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    // swap bytes
    v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
    // swap 2-byte long pairs
    v = ( v >> 16             ) | ( v               << 16);

    return v;
}

INLINE UINT32 READ32(ssem_state *cpustate, UINT32 address)
{
    UINT32 v = 0;
    // The MAME core does not have a good way of specifying a minimum datum size that is more than
    // 8 bits in width.  The minimum datum width on the SSEM is 32 bits, so we need to quadruple
    // the address value to get the appropriate byte index.
    address <<= 2;

    v |= cpustate->program->read_byte(address + 0) << 24;
    v |= cpustate->program->read_byte(address + 1) << 16;
    v |= cpustate->program->read_byte(address + 2) <<  8;
    v |= cpustate->program->read_byte(address + 3) <<  0;

    return reverse(v);
}

INLINE void WRITE32(ssem_state *cpustate, UINT32 address, UINT32 data)
{
    UINT32 v = reverse(data);

    // The MAME core does not have a good way of specifying a minimum datum size that is more than
    // 8 bits in width.  The minimum datum width on the SSEM is 32 bits, so we need to quadruple
    // the address value to get the appropriate byte index.
    address <<= 2;

    cpustate->program->write_byte(address + 0, (v >> 24) & 0x000000ff);
    cpustate->program->write_byte(address + 1, (v >> 16) & 0x000000ff);
    cpustate->program->write_byte(address + 2, (v >>  8) & 0x000000ff);
    cpustate->program->write_byte(address + 3, (v >>  0) & 0x000000ff);
    return;
}

/*****************************************************************************/

static void unimplemented_opcode(ssem_state *cpustate, UINT32 op)
{
    if((cpustate->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
    {
        char string[200];
        ssem_dasm_one(string, cpustate->pc-1, op);
        mame_printf_debug("%08X: %s\n", cpustate->pc-1, string);
    }

#if SSEM_DISASM_ON_UNIMPL
    {
        char string[200] = { 0 };
        UINT32 i = 0;
        FILE *disasm = fopen("ssemdasm.txt", "wt");

        if(disasm)
        {
            for(i = 0; i < 0x20; i++)
            {
                UINT32 opcode = reverse(READ32(cpustate, i));
                ssem_dasm_one(string, i, opcode);
                fprintf(disasm, "%02X: %08X    %s\n", i, opcode, string);
            }

            fclose(disasm);
        }
    }
#endif
#if SSEM_DUMP_MEM_ON_UNIMPL
    {
        UINT32 i = 0;
        FILE *store = fopen("ssemmem.bin", "wb");

        if(store)
        {
            for( i = 0; i < 0x80; i++ )
            {
                fputc(cpustate->program->read_byte(i), store);
            }
            fclose(store);
        }
    }
#endif

    fatalerror("SSEM: unknown opcode %d (%08X) at %d\n", reverse(op) & 7, reverse(op), cpustate->pc);
}

/*****************************************************************************/

static CPU_INIT( ssem )
{
    ssem_state *cpustate = get_safe_token(device);
    cpustate->pc = 1;
    cpustate->a = 0;
    cpustate->halt = 0;

    cpustate->device = device;
    cpustate->program = device->space(AS_PROGRAM);
}

static CPU_EXIT( ssem )
{
}

static CPU_RESET( ssem )
{
    ssem_state *cpustate = get_safe_token(device);

    cpustate->pc = 1;
    cpustate->a = 0;
    cpustate->halt = 0;
}

static CPU_EXECUTE( ssem )
{
    ssem_state *cpustate = get_safe_token(device);
    UINT32 op;

    cpustate->pc &= 0x1f;

    while (cpustate->icount > 0)
    {
        debugger_instruction_hook(device, cpustate->pc);

        op = READ32(cpustate, cpustate->pc);

        if( !cpustate->halt )
        {
            cpustate->pc++;
        }
        else
        {
            op = 0x0000e000;
        }

        switch (INSTR)
        {
            case 0:
                // JMP: Move the value at the specified address into the Program Counter.
                cpustate->pc = READ32(cpustate, ADDR) + 1;
                break;
            case 1:
                // JRP: Add the value at the specified address to the Program Counter.
                cpustate->pc += (INT32)READ32(cpustate, ADDR);
                break;
            case 2:
                // LDN: Load the accumulator with the two's-complement negation of the value at the specified address.
                cpustate->a = (UINT32)(0 - (INT32)READ32(cpustate, ADDR));
                break;
            case 3:
                // STO: Store the value in the accumulator at the specified address.
                WRITE32(cpustate, ADDR, cpustate->a);
                break;
            case 4:
            case 5:
                // SUB: Subtract the value at the specified address from the accumulator.
                cpustate->a -= READ32(cpustate, ADDR);
                break;
            case 6:
                // CMP: If the accumulator is less than zero, skip the next opcode.
                if((INT32)(cpustate->a) < 0)
                {
                    cpustate->pc++;
                }
                break;
            case 7:
                // STP: Halt the computer.
                cpustate->halt = 1;
                break;
            default:
                // This is impossible, but it's better to be safe than sorry.
                unimplemented_opcode(cpustate, op);
        }

        --cpustate->icount;
    }
}


/*****************************************************************************/

static CPU_SET_INFO( ssem )
{
    ssem_state *cpustate = get_safe_token(device);

    switch (state)
    {
        /* --- the following bits of info are set as 64-bit signed integers --- */
        case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + SSEM_PC:            cpustate->pc = info->i;         break;
        case CPUINFO_INT_REGISTER + SSEM_A:             cpustate->a = info->i;          break;
        case CPUINFO_INT_REGISTER + SSEM_HALT:          cpustate->halt = info->i;       break;
    }
}

CPU_GET_INFO( ssem )
{
    ssem_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

    switch(state)
    {
        /* --- the following bits of info are returned as 64-bit signed integers --- */
        case CPUINFO_INT_CONTEXT_SIZE:          info->i = sizeof(ssem_state);   break;
        case CPUINFO_INT_INPUT_LINES:           info->i = 0;                    break;
        case CPUINFO_INT_DEFAULT_IRQ_VECTOR:    info->i = 0;                    break;
        case CPUINFO_INT_ENDIANNESS:            info->i = ENDIANNESS_LITTLE;    break;
        case CPUINFO_INT_CLOCK_MULTIPLIER:      info->i = 1;                    break;
        case CPUINFO_INT_CLOCK_DIVIDER:         info->i = 1;                    break;
        case CPUINFO_INT_MIN_INSTRUCTION_BYTES: info->i = 4;                    break;
        case CPUINFO_INT_MAX_INSTRUCTION_BYTES: info->i = 4;                    break;
        case CPUINFO_INT_MIN_CYCLES:            info->i = 1;                    break;
        case CPUINFO_INT_MAX_CYCLES:            info->i = 1;                    break;

        case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM: info->i = 8;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;                   break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_IO:      info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:      info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:      info->i = 0;                    break;

        case CPUINFO_INT_PC:    /* intentional fallthrough */
        case CPUINFO_INT_REGISTER + SSEM_PC:    info->i = cpustate->pc << 2;    break;
        case CPUINFO_INT_REGISTER + SSEM_A:     info->i = cpustate->a;          break;
        case CPUINFO_INT_REGISTER + SSEM_HALT:  info->i = cpustate->halt;       break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
        case CPUINFO_FCT_SET_INFO:              info->setinfo = CPU_SET_INFO_NAME(ssem);        break;
        case CPUINFO_FCT_INIT:                  info->init = CPU_INIT_NAME(ssem);               break;
        case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(ssem);             break;
        case CPUINFO_FCT_EXIT:                  info->exit = CPU_EXIT_NAME(ssem);               break;
        case CPUINFO_FCT_EXECUTE:               info->execute = CPU_EXECUTE_NAME(ssem);         break;
        case CPUINFO_FCT_BURN:                  info->burn = NULL;                              break;
        case CPUINFO_FCT_DISASSEMBLE:           info->disassemble = CPU_DISASSEMBLE_NAME(ssem); break;
        case CPUINFO_PTR_INSTRUCTION_COUNTER:   info->icount = &cpustate->icount;               break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:                          strcpy(info->s, "SSEM");                break;
        case CPUINFO_STR_FAMILY:                   strcpy(info->s, "SSEM");                break;
        case CPUINFO_STR_VERSION:                  strcpy(info->s, "1.0");                 break;
        case CPUINFO_STR_SOURCE_FILE:                     strcpy(info->s, __FILE__);              break;
        case CPUINFO_STR_CREDITS:                  strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

        case CPUINFO_STR_FLAGS:                         strcpy(info->s, " ");                   break;

        case CPUINFO_STR_REGISTER + SSEM_PC:            sprintf(info->s, "PC: %08X", cpustate->pc);     break;
        case CPUINFO_STR_REGISTER + SSEM_A:             sprintf(info->s, "A: %08X", cpustate->a);       break;
        case CPUINFO_STR_REGISTER + SSEM_HALT:          sprintf(info->s, "HALT: %d", cpustate->halt);   break;
    }
}

DEFINE_LEGACY_CPU_DEVICE(SSEM, ssem);
