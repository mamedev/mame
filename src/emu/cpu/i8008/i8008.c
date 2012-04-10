/*****************************************************************************
 *
 *   i8008.c
 *
 *   Intel 8008 CPU
 *
 *   Initial version by Miodrag Milanovic
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "i8008.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

static UINT8 PARITY[256];

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8008_state i8008_state;
struct _i8008_state
{
	UINT8	A,B,C,D,E,H,L;
	PAIR	PC; // It is in fact one of ADDR regs
	PAIR	ADDR[8]; // Address registers
	UINT8	CF; // Carry flag
	UINT8	ZF; // Zero flag
	UINT8	SF; // Sign flag
	UINT8	PF; // Parity flag
	UINT8	HALT;
	UINT8	flags; // temporary I/O only
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int					icount;
	int 				pc_pos; // PC possition in ADDR

	device_irq_acknowledge_callback irq_callback;
	UINT8			irq_state;
};

/***************************************************************************
    MACROS
***************************************************************************/
#define REG_1					((opcode >> 3) & 7)
#define REG_2					(opcode & 7)
#define GET_PC					(cpustate->ADDR[cpustate->pc_pos])

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE i8008_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I8008);
	return (i8008_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE void PUSH_STACK(i8008_state *cpustate)
{
	cpustate->pc_pos = (cpustate->pc_pos + 1) & 7;
}

INLINE void POP_STACK(i8008_state *cpustate)
{
	cpustate->ADDR[cpustate->pc_pos].d = 0;
	cpustate->pc_pos = (cpustate->pc_pos - 1) & 7;
}

INLINE UINT8 ROP(i8008_state *cpustate)
{
	UINT8 retVal = cpustate->direct->read_decrypted_byte(GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x3fff;
	cpustate->PC = GET_PC;
	return retVal;
}

INLINE UINT8 GET_REG(i8008_state *cpustate,UINT8 reg)
{
	UINT8 retVal;
	switch(reg) {
		case 0 : retVal = cpustate->A; break;
		case 1 : retVal = cpustate->B; break;
		case 2 : retVal = cpustate->C; break;
		case 3 : retVal = cpustate->D; break;
		case 4 : retVal = cpustate->E; break;
		case 5 : retVal = cpustate->H; break;
		case 6 : retVal = cpustate->L; break;
		default: retVal = cpustate->program->read_byte((cpustate->H << 8) + cpustate->L); break;
	}
	return retVal;
}

INLINE void SET_REG(i8008_state *cpustate,UINT8 reg, UINT8 val)
{
	switch(reg) {
		case 0 : cpustate->A = val; break;
		case 1 : cpustate->B = val; break;
		case 2 : cpustate->C = val; break;
		case 3 : cpustate->D = val; break;
		case 4 : cpustate->E = val; break;
		case 5 : cpustate->H = val; break;
		case 6 : cpustate->L = val; break;
		default: cpustate->program->write_byte((cpustate->H << 8) + cpustate->L, val); break;
	}
}

INLINE UINT8 ARG(i8008_state *cpustate)
{
	UINT8 retVal = cpustate->direct->read_raw_byte(GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x3fff;
	cpustate->PC = GET_PC;
	return retVal;
}

INLINE void UPDATE_FLAGS(i8008_state *cpustate,UINT8 val)
{
	cpustate->ZF = (val == 0) ? 1 : 0;
	cpustate->SF = (val & 0x80) ? 1 : 0;
	cpustate->PF = PARITY[val];
}

INLINE UINT8 DO_CONDITION(i8008_state *cpustate, UINT8 val)
{
	UINT8 v = (val >> 5) & 1;
	UINT8 cond = 0;
	switch((val>> 3) & 0x03) {
		case 0 :
				if (cpustate->CF==v) cond = 1;
				break;
		case 1 :
				if (cpustate->ZF==v) cond = 1;
				break;
		case 2 :
				if (cpustate->SF==v) cond = 1;
				break;
		case 3 :
				if (cpustate->PF==v) cond = 1;
				break;
	}
	return cond;
}

INLINE UINT16 GET_ADDR(i8008_state *cpustate)
{
	UINT8 lo = ARG(cpustate);
	UINT8 hi = ARG(cpustate);
	return ((hi & 0x3f) << 8) + lo;
}

INLINE void illegal(i8008_state *cpustate,UINT8 opcode)
{
#if VERBOSE
	UINT16 pc = cpustate->PC.w.l;
	LOG(("I8008 illegal instruction %04X $%02X\n", pc, opcode));
#endif
}

static void execute_one(i8008_state *cpustate, int opcode)
{
	UINT16 tmp;

	switch (opcode >> 6)
	{
		case 0x03:	// starting with 11
					if (opcode==0xff) {
						// HLT
						cpustate->icount -= 4;
						GET_PC.w.l = GET_PC.w.l - 1;
						cpustate->PC = GET_PC;
						cpustate->HALT = 1;
					} else {
						// Lrr
						cpustate->icount -= 5;
						if (REG_1==7) cpustate->icount -= 2;
						if (REG_2==7) cpustate->icount -= 3;
						SET_REG(cpustate,REG_1, GET_REG(cpustate,REG_2));
					}
					break;
		case 0x00:	// starting with 00
					switch(opcode & 7) {
						case 0 :	if(((opcode >> 3) & 7)==0) {
							        	// HLT
										cpustate->icount -= 4;
										GET_PC.w.l = GET_PC.w.l - 1;
										cpustate->PC = GET_PC;
										cpustate->HALT = 1;
									} else {
										if(((opcode >> 3) & 7)==7) {
											// ILLEGAL
											cpustate->icount -= 5;
											illegal(cpustate,opcode);
										} else {
											// INr
											cpustate->icount -= 5;
											tmp = GET_REG(cpustate,REG_1) + 1;
											SET_REG(cpustate,REG_1, tmp & 0xff);
											UPDATE_FLAGS(cpustate,tmp & 0xff);
										}
									}
									break;
						case 1 :	if(((opcode >> 3) & 7)==0) {
							        	// HLT
										cpustate->icount -= 4;
										GET_PC.w.l = GET_PC.w.l - 1;
										cpustate->PC = GET_PC;
										cpustate->HALT = 1;
									} else {
										if(((opcode >> 3) & 7)==7) {
											// ILLEGAL
											cpustate->icount -= 5;
											illegal(cpustate,opcode);
										} else {
											// DCr
											cpustate->icount -= 5;
											tmp = GET_REG(cpustate,REG_1) - 1;
											SET_REG(cpustate,REG_1, tmp & 0xff);
											UPDATE_FLAGS(cpustate,tmp & 0xff);
										}
									}
									break;
						case 2 :	{
										// All instuction from this group have same timing
										cpustate->icount -= 5;
										switch((opcode >> 3) & 7) {
											case 0 :
												// RLC
												tmp = cpustate->A;
												cpustate->A = (cpustate->A << 1) | BIT(tmp,7);
												cpustate->CF = BIT(tmp,7);
												break;
											case 1 :
												// RRC
												tmp = cpustate->A;
												cpustate->A = (cpustate->A >> 1) | (BIT(tmp,0) ? 0x80 : 0x00);
												cpustate->CF = BIT(tmp,0);
												break;
											case 2 :
												// RAL
												tmp = cpustate->A;
												cpustate->A = (cpustate->A << 1) | cpustate->CF;
												cpustate->CF = BIT(tmp,7);
												break;
											case 3 :
												// RAR
												tmp = cpustate->A;
												cpustate->A = (cpustate->A >> 1) | (cpustate->CF ? 0x80 : 0x00);
												cpustate->CF = BIT(tmp,0);
												break;
											default :
												// ILLEGAL
												illegal(cpustate,opcode);
												break;
										}
									}
									break;
						case 3 :
									// Rcc
									{
										cpustate->icount -= 3;
										if (DO_CONDITION(cpustate,opcode)==1) {
											cpustate->icount -= 2;
											POP_STACK(cpustate);
											cpustate->PC = GET_PC;
										}
									}
									break;
						case 4 :	{
										cpustate->icount -= 8;
										switch((opcode >> 3) & 7) {
											case 0 :
												// ADI
												tmp = GET_REG(cpustate,0) + ARG(cpustate);
												SET_REG(cpustate,0,tmp & 0xff);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = (tmp >> 8) & 1;
												break;
											case 1 :
												// ACI
												tmp = GET_REG(cpustate,0) + ARG(cpustate) + cpustate->CF;
												SET_REG(cpustate,0,tmp & 0xff);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = (tmp >> 8) & 1;
												break;
											case 2 :
												// SUI
												tmp = GET_REG(cpustate,0) - ARG(cpustate);
												SET_REG(cpustate,0,tmp & 0xff);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = (tmp >> 8) & 1;
												break;
											case 3 :
												// SBI
												tmp = GET_REG(cpustate,0) - ARG(cpustate) - cpustate->CF;
												SET_REG(cpustate,0,tmp & 0xff);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = (tmp >> 8) & 1;
												break;
											case 4 :
												// NDI
												tmp = GET_REG(cpustate,0) & ARG(cpustate);
												SET_REG(cpustate,0,tmp & 0xff);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = 0;
												break;
											case 5 :
												// XRI
												tmp = GET_REG(cpustate,0) ^ ARG(cpustate);
												SET_REG(cpustate,0,tmp & 0xff);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = 0;
												break;
											case 6 :
												// ORI
												tmp = GET_REG(cpustate,0) | ARG(cpustate);
												SET_REG(cpustate,0,tmp & 0xff);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = 0;
												break;
											case 7 :
												// CPI
												tmp = GET_REG(cpustate,0) - ARG(cpustate);
												UPDATE_FLAGS(cpustate,tmp & 0xff);
												cpustate->CF = (tmp >> 8) & 1;
												break;
										}
									}
									break;
						case 5 :	// RST
									cpustate->icount -= 5;
									PUSH_STACK(cpustate);
									GET_PC.w.l = opcode & 0x38;
									cpustate->PC = GET_PC;
									break;
						case 6 :	// LrI
									cpustate->icount -= 8;
									if (REG_1==7) cpustate->icount -= 1; // LMI
									SET_REG(cpustate,REG_1, ARG(cpustate));
									break;
						case 7 :	// RET
									cpustate->icount -= 5;
									POP_STACK(cpustate);
									cpustate->PC = GET_PC;
									break;
					}
					break;

		case 0x01:	// starting with 01
					switch(opcode & 7) {
						case 0 :
							// Jcc
							cpustate->icount -= 9;
							tmp = GET_ADDR(cpustate);
							if (DO_CONDITION(cpustate,opcode)==1) {
								cpustate->icount -= 2;
								GET_PC.w.l = tmp;
								cpustate->PC = GET_PC;
							}
							break;
						case 2 :
							// Ccc
							cpustate->icount -= 9;
							tmp = GET_ADDR(cpustate);
							if (DO_CONDITION(cpustate,opcode)==1) {
								cpustate->icount -= 2;
								PUSH_STACK(cpustate);
								GET_PC.w.l = tmp;
								cpustate->PC = GET_PC;
							}
							break;
						case 4 :
							// JMP
							cpustate->icount -= 11;
							GET_PC.w.l = GET_ADDR(cpustate);
							cpustate->PC = GET_PC;
							break;
						case 6 :
							// CAL
							cpustate->icount -= 11;
							tmp = GET_ADDR(cpustate);
							PUSH_STACK(cpustate);
							GET_PC.w.l = tmp;
							cpustate->PC = GET_PC;
							break;
						default :
							if (((opcode>>4)&3)==0) {
								// INP
								cpustate->icount -= 8;
								cpustate->A = cpustate->io->read_byte((opcode >> 1) & 0x1f);
							} else {
								// OUT
								cpustate->icount -= 6;
								cpustate->io->write_byte((opcode >> 1) & 0x1f, cpustate->A);
							}
							break;
					}
					break;
		case 0x02:	// starting with 10
					cpustate->icount -= 5;
					if ((opcode & 7)==7) cpustate->icount -= 3; // operations with memory
					switch((opcode >> 3) & 7) {
						case 0 :
							// ADx
							tmp = GET_REG(cpustate,0) + GET_REG(cpustate,opcode & 7);
							SET_REG(cpustate,0,tmp & 0xff);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = (tmp >> 8) & 1;
							break;
						case 1 :
							// ACx
							tmp = GET_REG(cpustate,0) + GET_REG(cpustate,opcode & 7) + cpustate->CF;
							SET_REG(cpustate,0,tmp & 0xff);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = (tmp >> 8) & 1;
							break;
						case 2 :
							// SUx
							tmp = GET_REG(cpustate,0) - GET_REG(cpustate,opcode & 7);
							SET_REG(cpustate,0,tmp & 0xff);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = (tmp >> 8) & 1;
							break;
						case 3 :
							// SBx
							tmp = GET_REG(cpustate,0) - GET_REG(cpustate,opcode & 7) - cpustate->CF;
							SET_REG(cpustate,0,tmp & 0xff);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = (tmp >> 8) & 1;
							break;
						case 4 :
							// NDx
							tmp = GET_REG(cpustate,0) & GET_REG(cpustate,opcode & 7);
							SET_REG(cpustate,0,tmp & 0xff);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = 0;
							break;
						case 5 :
							// XRx
							tmp = GET_REG(cpustate,0) ^ GET_REG(cpustate,opcode & 7);
							SET_REG(cpustate,0,tmp & 0xff);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = 0;
							break;
						case 6 :
							// ORx
							tmp = GET_REG(cpustate,0) | GET_REG(cpustate,opcode & 7);
							SET_REG(cpustate,0,tmp & 0xff);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = 0;
							break;
						case 7 :
							// CPx
							tmp = GET_REG(cpustate,0) - GET_REG(cpustate,opcode & 7);
							UPDATE_FLAGS(cpustate,tmp & 0xff);
							cpustate->CF = (tmp >> 8) & 1;
							break;
					}
					break;
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/
static void take_interrupt(i8008_state *cpustate)
{
	if (cpustate->HALT) {
		GET_PC.w.l = (GET_PC.w.l + 1) & 0x3fff;
		cpustate->PC = GET_PC;
		cpustate->HALT = 0;
	}
	// For now only support one byte operation to be executed
	execute_one(cpustate,(*cpustate->irq_callback)(cpustate->device, 0));
}

static CPU_EXECUTE( i8008 )
{
	i8008_state *cpustate = get_safe_token(device);

	do
	{
		if (cpustate->irq_state != CLEAR_LINE) {
			take_interrupt(cpustate);
		}
		debugger_instruction_hook(device, cpustate->PC.d);
		execute_one(cpustate, ROP(cpustate));

	} while (cpustate->icount > 0);
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/
static void init_tables (void)
{
	int i;
	UINT8 p;
	for (i = 0; i < 256; i++)
	{
		p = 0;
		if (BIT(i,0)) p++;
		if (BIT(i,1)) p++;
		if (BIT(i,2)) p++;
		if (BIT(i,3)) p++;
		if (BIT(i,4)) p++;
		if (BIT(i,5)) p++;
		if (BIT(i,6)) p++;
		if (BIT(i,7)) p++;
		PARITY[i] = ((p&1) ? 0 : 1);
	}
}

static CPU_INIT( i8008 )
{
	i8008_state *cpustate = get_safe_token(device);

	/* set up the state table */
	{
		device_state_interface *state;
		device->interface(state);
		state->state_add(I8008_PC,    "PC",    cpustate->PC.w.l).mask(0x3fff);
		state->state_add(STATE_GENPC, "GENPC", cpustate->PC.w.l).mask(0x3fff).noshow();
		state->state_add(STATE_GENFLAGS, "GENFLAGS", cpustate->flags).mask(0x0f).callimport().callexport().noshow().formatstr("%4s");
		state->state_add(I8008_A,     "A",     cpustate->A);
		state->state_add(I8008_B,     "B",     cpustate->B);
		state->state_add(I8008_C,     "C",     cpustate->C);
		state->state_add(I8008_D,     "D",     cpustate->D);
		state->state_add(I8008_E,     "E",     cpustate->E);
		state->state_add(I8008_H,     "H",     cpustate->H);
		state->state_add(I8008_L,     "L",     cpustate->L);

		astring tempstr;
		for (int addrnum = 0; addrnum < 8; addrnum++)
			state->state_add(I8008_ADDR1 + addrnum, tempstr.format("ADDR%d", addrnum + 1), cpustate->ADDR[addrnum].w.l).mask(0xfff);
	}

	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	cpustate->irq_callback = irqcallback;

	init_tables();

	device->save_item(NAME(cpustate->PC));
	device->save_item(NAME(cpustate->A));
	device->save_item(NAME(cpustate->B));
	device->save_item(NAME(cpustate->C));
	device->save_item(NAME(cpustate->D));
	device->save_item(NAME(cpustate->E));
	device->save_item(NAME(cpustate->H));
	device->save_item(NAME(cpustate->L));
	device->save_item(NAME(cpustate->CF));
	device->save_item(NAME(cpustate->SF));
	device->save_item(NAME(cpustate->ZF));
	device->save_item(NAME(cpustate->PF));
	device->save_item(NAME(cpustate->pc_pos));
	device->save_item(NAME(cpustate->ADDR[0]));
	device->save_item(NAME(cpustate->ADDR[1]));
	device->save_item(NAME(cpustate->ADDR[2]));
	device->save_item(NAME(cpustate->ADDR[3]));
	device->save_item(NAME(cpustate->ADDR[4]));
	device->save_item(NAME(cpustate->ADDR[5]));
	device->save_item(NAME(cpustate->ADDR[6]));
	device->save_item(NAME(cpustate->ADDR[7]));
	device->save_item(NAME(cpustate->HALT));
	device->save_item(NAME(cpustate->irq_state));
}



/***************************************************************************
    COMMON RESET
***************************************************************************/

static CPU_RESET( i8008 )
{
	i8008_state *cpustate = get_safe_token(device);

	cpustate->CF = cpustate->SF = cpustate->ZF = cpustate->PF = 0;
	cpustate->A = cpustate->B = cpustate->C = cpustate->D = cpustate->E = cpustate->H = cpustate->L = 0;
	cpustate->PC.d = 0;
	cpustate->pc_pos = 0;
	cpustate->HALT = 0;
	cpustate->irq_state = CLEAR_LINE;
	memset(cpustate->ADDR,0,sizeof(cpustate->ADDR));

}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(i8008_state *cpustate, int irqline, int state)
{
	cpustate->irq_state = state;
}

/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

static CPU_IMPORT_STATE( i8008 )
{
	i8008_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			cpustate->CF = (cpustate->flags >> 3) & 1;
			cpustate->ZF = (cpustate->flags >> 2) & 1;
			cpustate->SF = (cpustate->flags >> 1) & 1;
			cpustate->PF = (cpustate->flags >> 0) & 1;
			break;
	}
}

static CPU_EXPORT_STATE( i8008 )
{
	i8008_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			cpustate->flags = (cpustate->CF ? 0x08 : 0x00) |
							  (cpustate->ZF ? 0x04 : 0x00) |
							  (cpustate->SF ? 0x02 : 0x00) |
							  (cpustate->PF ? 0x01 : 0x00);
			break;
	}
}

static CPU_EXPORT_STRING( i8008 )
{
	i8008_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c",
				cpustate->CF ? 'C':'.',
				cpustate->ZF ? 'Z':'.',
				cpustate->SF ? 'S':'.',
				cpustate->PF ? 'P':'.');
			break;
	}
}

/***************************************************************************
    COMMON SET INFO
***************************************************************************/
static CPU_SET_INFO( i8008 )
{
	i8008_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:					set_irq_line(cpustate, 0, info->i);				break;
	}
}

/***************************************************************************
    8008 GET INFO
***************************************************************************/

CPU_GET_INFO( i8008 )
{
	i8008_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i8008_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 8;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 14;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 0;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 0;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(i8008);				break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(i8008);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(i8008);					break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(i8008);				break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(i8008);		break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(i8008);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(i8008);		break;
		case CPUINFO_FCT_EXPORT_STRING:	info->export_string = CPU_EXPORT_STRING_NAME(i8008);	break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "8008");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 8008");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Miodrag Milanovic"); break;

		case CPUINFO_IS_OCTAL:						info->i = true;							break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(I8008, i8008);
