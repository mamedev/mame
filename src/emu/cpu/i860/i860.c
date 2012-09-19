/***************************************************************************

    i860.c

    Interface file for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)
    Released for general non-commercial use under the MAME license
    with the additional requirement that you are free to use and
    redistribute this code in modified or unmodified form, provided
    you list me in the credits.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*
TODO: Separate out i860XR and i860XP (make different types, etc).
      Hook IRQ lines into MAME core (they're custom functions atm).
*/

#include "emu.h"
#include "debugger.h"
#include "i860.h"

/**************************************************************************
 * Functions specified by GET_INFO
 **************************************************************************/

static CPU_INIT( i860 )
{
	i860_state_t *cpustate = get_safe_token(device);
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	reset_i860(cpustate);
	i860_set_pin(device, DEC_PIN_BUS_HOLD, 0);
	i860_set_pin(device, DEC_PIN_RESET, 0);
	cpustate->single_stepping = 0;

	device->save_item(NAME(cpustate->iregs));
	device->save_item(NAME(cpustate->cregs));
	device->save_item(NAME(cpustate->frg));
	device->save_item(NAME(cpustate->pc));
}

static CPU_RESET( i860 )
{
	i860_state_t *cpustate = get_safe_token(device);
	reset_i860(cpustate);
}


/***************************************************************************
 *  Disassembler hook
 ***************************************************************************/

static CPU_DISASSEMBLE( i860 )
{
	extern unsigned disasm_i860(char*, unsigned, UINT32);

	/* Hard-coded little endian for now.  */
	return disasm_i860(buffer, pc, (oprom[0] << 0)  |
								   (oprom[1] << 8)  |
								   (oprom[2] << 16) |
								   (oprom[3] << 24));
}


/**************************************************************************
 * The actual decode and execute code.
 **************************************************************************/
#include "i860dec.c"


/**************************************************************************
 * Generic set_info/get_info
 **************************************************************************/

#define CPU_SET_INFO_F(fnum) cpustate->frg[0+(4*fnum)] = (info->i & 0x000000ff);       \
							 cpustate->frg[1+(4*fnum)] = (info->i & 0x0000ff00) >> 8;  \
							 cpustate->frg[2+(4*fnum)] = (info->i & 0x00ff0000) >> 16; \
							 cpustate->frg[3+(4*fnum)] = (info->i & 0xff000000) >> 24;

static CPU_SET_INFO( i860 )
{
	i860_state_t *cpustate = get_safe_token(device);

	switch(state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I860_PC:		cpustate->pc = info->i & 0xffffffff;		break;

		case CPUINFO_INT_REGISTER + I860_FIR:		cpustate->cregs[CR_FIR] 	= info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_PSR:		cpustate->cregs[CR_PSR] 	= info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_DIRBASE:	cpustate->cregs[CR_DIRBASE] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_DB:		cpustate->cregs[CR_DB]		= info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_FSR:		cpustate->cregs[CR_FSR] 	= info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_EPSR:		cpustate->cregs[CR_EPSR]	= info->i & 0xffffffff;	break;

		case CPUINFO_INT_REGISTER + I860_R0:		cpustate->iregs[0]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R1:		cpustate->iregs[1]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R2:		cpustate->iregs[2]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R3:		cpustate->iregs[3]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R4:		cpustate->iregs[4]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R5:		cpustate->iregs[5]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R6:		cpustate->iregs[6]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R7:		cpustate->iregs[7]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R8:		cpustate->iregs[8]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R9:		cpustate->iregs[9]  = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R10:		cpustate->iregs[10] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R11:		cpustate->iregs[11] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R12:		cpustate->iregs[12] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R13:		cpustate->iregs[13] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R14:		cpustate->iregs[14] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R15:		cpustate->iregs[15] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R16:		cpustate->iregs[16] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R17:		cpustate->iregs[17] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R18:		cpustate->iregs[18] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R19:		cpustate->iregs[19] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R20:		cpustate->iregs[20] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R21:		cpustate->iregs[21] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R22:		cpustate->iregs[22] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R23:		cpustate->iregs[23] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R24:		cpustate->iregs[24] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R25:		cpustate->iregs[25] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R26:		cpustate->iregs[26] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R27:		cpustate->iregs[27] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R28:		cpustate->iregs[28] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R29:		cpustate->iregs[29] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R30:		cpustate->iregs[30] = info->i & 0xffffffff;	break;
		case CPUINFO_INT_REGISTER + I860_R31:		cpustate->iregs[31] = info->i & 0xffffffff;	break;

		case CPUINFO_INT_REGISTER + I860_F0:		CPU_SET_INFO_F(0);	break;
		case CPUINFO_INT_REGISTER + I860_F1:		CPU_SET_INFO_F(1);	break;
		case CPUINFO_INT_REGISTER + I860_F2:		CPU_SET_INFO_F(2);	break;
		case CPUINFO_INT_REGISTER + I860_F3:		CPU_SET_INFO_F(3);	break;
		case CPUINFO_INT_REGISTER + I860_F4:		CPU_SET_INFO_F(4);	break;
		case CPUINFO_INT_REGISTER + I860_F5:		CPU_SET_INFO_F(5);	break;
		case CPUINFO_INT_REGISTER + I860_F6:		CPU_SET_INFO_F(6);	break;
		case CPUINFO_INT_REGISTER + I860_F7:		CPU_SET_INFO_F(7);	break;
		case CPUINFO_INT_REGISTER + I860_F8:		CPU_SET_INFO_F(8);	break;
		case CPUINFO_INT_REGISTER + I860_F9:		CPU_SET_INFO_F(9);	break;
		case CPUINFO_INT_REGISTER + I860_F10:		CPU_SET_INFO_F(10);	break;
		case CPUINFO_INT_REGISTER + I860_F11:		CPU_SET_INFO_F(11);	break;
		case CPUINFO_INT_REGISTER + I860_F12:		CPU_SET_INFO_F(12);	break;
		case CPUINFO_INT_REGISTER + I860_F13:		CPU_SET_INFO_F(13);	break;
		case CPUINFO_INT_REGISTER + I860_F14:		CPU_SET_INFO_F(14);	break;
		case CPUINFO_INT_REGISTER + I860_F15:		CPU_SET_INFO_F(15);	break;
		case CPUINFO_INT_REGISTER + I860_F16:		CPU_SET_INFO_F(16);	break;
		case CPUINFO_INT_REGISTER + I860_F17:		CPU_SET_INFO_F(17);	break;
		case CPUINFO_INT_REGISTER + I860_F18:		CPU_SET_INFO_F(18);	break;
		case CPUINFO_INT_REGISTER + I860_F19:		CPU_SET_INFO_F(19);	break;
		case CPUINFO_INT_REGISTER + I860_F20:		CPU_SET_INFO_F(20);	break;
		case CPUINFO_INT_REGISTER + I860_F21:		CPU_SET_INFO_F(21);	break;
		case CPUINFO_INT_REGISTER + I860_F22:		CPU_SET_INFO_F(22);	break;
		case CPUINFO_INT_REGISTER + I860_F23:		CPU_SET_INFO_F(23);	break;
		case CPUINFO_INT_REGISTER + I860_F24:		CPU_SET_INFO_F(24);	break;
		case CPUINFO_INT_REGISTER + I860_F25:		CPU_SET_INFO_F(25);	break;
		case CPUINFO_INT_REGISTER + I860_F26:		CPU_SET_INFO_F(26);	break;
		case CPUINFO_INT_REGISTER + I860_F27:		CPU_SET_INFO_F(27);	break;
		case CPUINFO_INT_REGISTER + I860_F28:		CPU_SET_INFO_F(28);	break;
		case CPUINFO_INT_REGISTER + I860_F29:		CPU_SET_INFO_F(29);	break;
		case CPUINFO_INT_REGISTER + I860_F30:		CPU_SET_INFO_F(30);	break;
		case CPUINFO_INT_REGISTER + I860_F31:		CPU_SET_INFO_F(31);	break;
	}
}

#define CPU_GET_INFO_INT_F(fnum) (info->i = cpustate->frg[0+(4*fnum)]       | \
											cpustate->frg[1+(4*fnum)] << 8  | \
											cpustate->frg[2+(4*fnum)] << 16 | \
											cpustate->frg[3+(4*fnum)] << 24)

#define CPU_GET_INFO_STR_F(fnum) (sprintf(info->s, "F%d : %08x", fnum, cpustate->frg[0+(4*fnum)]       | \
																	   cpustate->frg[1+(4*fnum)] << 8  | \
																	   cpustate->frg[2+(4*fnum)] << 16 | \
																	   cpustate->frg[3+(4*fnum)] << 24))

CPU_GET_INFO( i860 )
{
	i860_state_t *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i860_state_t);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;						break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0x00000000;				break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;		break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;						break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;						break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;						break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;						break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;						break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 8;						break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 64;	break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 32;	break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0;	break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 0;	break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 0;	break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;	break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 0;	break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 0;	break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I860_PC:			info->i = cpustate->pc;					break;
		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc;				break;

		case CPUINFO_INT_REGISTER + I860_FIR:			info->i = cpustate->cregs[CR_FIR];		break;
		case CPUINFO_INT_REGISTER + I860_PSR:			info->i = cpustate->cregs[CR_PSR];		break;
		case CPUINFO_INT_REGISTER + I860_DIRBASE:		info->i = cpustate->cregs[CR_DIRBASE];	break;
		case CPUINFO_INT_REGISTER + I860_DB:			info->i = cpustate->cregs[CR_DB];		break;
		case CPUINFO_INT_REGISTER + I860_FSR:			info->i = cpustate->cregs[CR_FSR];		break;
		case CPUINFO_INT_REGISTER + I860_EPSR:			info->i = cpustate->cregs[CR_EPSR];		break;

		case CPUINFO_INT_REGISTER + I860_R0:			info->i = cpustate->iregs[0];			break;
		case CPUINFO_INT_REGISTER + I860_R1:			info->i = cpustate->iregs[1];			break;
		case CPUINFO_INT_REGISTER + I860_R2:			info->i = cpustate->iregs[2];			break;
		case CPUINFO_INT_REGISTER + I860_R3:			info->i = cpustate->iregs[3];			break;
		case CPUINFO_INT_REGISTER + I860_R4:			info->i = cpustate->iregs[4];			break;
		case CPUINFO_INT_REGISTER + I860_R5:			info->i = cpustate->iregs[5];			break;
		case CPUINFO_INT_REGISTER + I860_R6:			info->i = cpustate->iregs[6];			break;
		case CPUINFO_INT_REGISTER + I860_R7:			info->i = cpustate->iregs[7];			break;
		case CPUINFO_INT_REGISTER + I860_R8:			info->i = cpustate->iregs[8];			break;
		case CPUINFO_INT_REGISTER + I860_R9:			info->i = cpustate->iregs[9];			break;
		case CPUINFO_INT_REGISTER + I860_R10:			info->i = cpustate->iregs[10];			break;
		case CPUINFO_INT_REGISTER + I860_R11:			info->i = cpustate->iregs[11];			break;
		case CPUINFO_INT_REGISTER + I860_R12:			info->i = cpustate->iregs[12];			break;
		case CPUINFO_INT_REGISTER + I860_R13:			info->i = cpustate->iregs[13];			break;
		case CPUINFO_INT_REGISTER + I860_R14:			info->i = cpustate->iregs[14];			break;
		case CPUINFO_INT_REGISTER + I860_R15:			info->i = cpustate->iregs[15];			break;
		case CPUINFO_INT_REGISTER + I860_R16:			info->i = cpustate->iregs[16];			break;
		case CPUINFO_INT_REGISTER + I860_R17:			info->i = cpustate->iregs[17];			break;
		case CPUINFO_INT_REGISTER + I860_R18:			info->i = cpustate->iregs[18];			break;
		case CPUINFO_INT_REGISTER + I860_R19:			info->i = cpustate->iregs[19];			break;
		case CPUINFO_INT_REGISTER + I860_R20:			info->i = cpustate->iregs[20];			break;
		case CPUINFO_INT_REGISTER + I860_R21:			info->i = cpustate->iregs[21];			break;
		case CPUINFO_INT_REGISTER + I860_R22:			info->i = cpustate->iregs[22];			break;
		case CPUINFO_INT_REGISTER + I860_R23:			info->i = cpustate->iregs[23];			break;
		case CPUINFO_INT_REGISTER + I860_R24:			info->i = cpustate->iregs[24];			break;
		case CPUINFO_INT_REGISTER + I860_R25:			info->i = cpustate->iregs[25];			break;
		case CPUINFO_INT_REGISTER + I860_R26:			info->i = cpustate->iregs[26];			break;
		case CPUINFO_INT_REGISTER + I860_R27:			info->i = cpustate->iregs[27];			break;
		case CPUINFO_INT_REGISTER + I860_R28:			info->i = cpustate->iregs[28];			break;
		case CPUINFO_INT_REGISTER + I860_R29:			info->i = cpustate->iregs[29];			break;
		case CPUINFO_INT_REGISTER + I860_R30:			info->i = cpustate->iregs[30];			break;
		case CPUINFO_INT_REGISTER + I860_R31:			info->i = cpustate->iregs[31];			break;

		case CPUINFO_INT_REGISTER + I860_F0:			CPU_GET_INFO_INT_F(0);	break;
		case CPUINFO_INT_REGISTER + I860_F1:			CPU_GET_INFO_INT_F(1);	break;
		case CPUINFO_INT_REGISTER + I860_F2:			CPU_GET_INFO_INT_F(2);	break;
		case CPUINFO_INT_REGISTER + I860_F3:			CPU_GET_INFO_INT_F(3);	break;
		case CPUINFO_INT_REGISTER + I860_F4:			CPU_GET_INFO_INT_F(4);	break;
		case CPUINFO_INT_REGISTER + I860_F5:			CPU_GET_INFO_INT_F(5);	break;
		case CPUINFO_INT_REGISTER + I860_F6:			CPU_GET_INFO_INT_F(6);	break;
		case CPUINFO_INT_REGISTER + I860_F7:			CPU_GET_INFO_INT_F(7);	break;
		case CPUINFO_INT_REGISTER + I860_F8:			CPU_GET_INFO_INT_F(8);	break;
		case CPUINFO_INT_REGISTER + I860_F9:			CPU_GET_INFO_INT_F(9);	break;
		case CPUINFO_INT_REGISTER + I860_F10:			CPU_GET_INFO_INT_F(10);	break;
		case CPUINFO_INT_REGISTER + I860_F11:			CPU_GET_INFO_INT_F(11);	break;
		case CPUINFO_INT_REGISTER + I860_F12:			CPU_GET_INFO_INT_F(12);	break;
		case CPUINFO_INT_REGISTER + I860_F13:			CPU_GET_INFO_INT_F(13);	break;
		case CPUINFO_INT_REGISTER + I860_F14:			CPU_GET_INFO_INT_F(14);	break;
		case CPUINFO_INT_REGISTER + I860_F15:			CPU_GET_INFO_INT_F(15);	break;
		case CPUINFO_INT_REGISTER + I860_F16:			CPU_GET_INFO_INT_F(16);	break;
		case CPUINFO_INT_REGISTER + I860_F17:			CPU_GET_INFO_INT_F(17);	break;
		case CPUINFO_INT_REGISTER + I860_F18:			CPU_GET_INFO_INT_F(18);	break;
		case CPUINFO_INT_REGISTER + I860_F19:			CPU_GET_INFO_INT_F(19);	break;
		case CPUINFO_INT_REGISTER + I860_F20:			CPU_GET_INFO_INT_F(20);	break;
		case CPUINFO_INT_REGISTER + I860_F21:			CPU_GET_INFO_INT_F(21);	break;
		case CPUINFO_INT_REGISTER + I860_F22:			CPU_GET_INFO_INT_F(22);	break;
		case CPUINFO_INT_REGISTER + I860_F23:			CPU_GET_INFO_INT_F(23);	break;
		case CPUINFO_INT_REGISTER + I860_F24:			CPU_GET_INFO_INT_F(24);	break;
		case CPUINFO_INT_REGISTER + I860_F25:			CPU_GET_INFO_INT_F(25);	break;
		case CPUINFO_INT_REGISTER + I860_F26:			CPU_GET_INFO_INT_F(26);	break;
		case CPUINFO_INT_REGISTER + I860_F27:			CPU_GET_INFO_INT_F(27);	break;
		case CPUINFO_INT_REGISTER + I860_F28:			CPU_GET_INFO_INT_F(28);	break;
		case CPUINFO_INT_REGISTER + I860_F29:			CPU_GET_INFO_INT_F(29);	break;
		case CPUINFO_INT_REGISTER + I860_F30:			CPU_GET_INFO_INT_F(30);	break;
		case CPUINFO_INT_REGISTER + I860_F31:			CPU_GET_INFO_INT_F(31);	break;


		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo	  = CPU_SET_INFO_NAME(i860);	break;
		case CPUINFO_FCT_INIT:							info->init		  = CPU_INIT_NAME(i860);		break;
		case CPUINFO_FCT_RESET:							info->reset 	  = CPU_RESET_NAME(i860);		break;
		case CPUINFO_FCT_EXIT:							info->exit		  = NULL;						break;
		case CPUINFO_FCT_EXECUTE:						info->execute	  = CPU_EXECUTE_NAME(i860);		break;
		case CPUINFO_FCT_BURN:							info->burn		  = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i860);	break;
		case CPUINFO_FCT_DEBUG_INIT:					info->debug_init  = NULL;						break;
		case CPUINFO_FCT_TRANSLATE:						info->translate	  = NULL;						break;
		case CPUINFO_FCT_READ:							info->read		  = NULL;						break;
		case CPUINFO_FCT_WRITE:							info->write 	  = NULL;						break;
		case CPUINFO_FCT_READOP:						info->readop	  = NULL;						break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount	  = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "i860XR");			break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Intel i860");		break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "0.1");				break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);			break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Jason Eckhardt");	break;

		case CPUINFO_STR_FLAGS:
			strcpy(info->s, ""); break;

		case CPUINFO_STR_REGISTER + I860_PC:			sprintf(info->s, "PC : %08x", cpustate->pc);					break;
		case CPUINFO_STR_REGISTER + I860_FIR:			sprintf(info->s, "FIR : %08x", cpustate->cregs[CR_FIR]);		break;
		case CPUINFO_STR_REGISTER + I860_PSR:			sprintf(info->s, "PSR : %08x", cpustate->cregs[CR_PSR]);		break;
		case CPUINFO_STR_REGISTER + I860_DIRBASE:		sprintf(info->s, "DIRBASE : %08x", cpustate->cregs[CR_DIRBASE]);break;
		case CPUINFO_STR_REGISTER + I860_DB:			sprintf(info->s, "DB : %08x", cpustate->cregs[CR_DB]);			break;
		case CPUINFO_STR_REGISTER + I860_FSR:			sprintf(info->s, "FSR : %08x", cpustate->cregs[CR_FSR]);		break;
		case CPUINFO_STR_REGISTER + I860_EPSR:			sprintf(info->s, "EPSR : %08x", cpustate->cregs[CR_EPSR]);		break;

		case CPUINFO_STR_REGISTER + I860_R0:			sprintf(info->s, "R0 : %08x",  cpustate->iregs[0]);		break;
		case CPUINFO_STR_REGISTER + I860_R1:			sprintf(info->s, "R1 : %08x",  cpustate->iregs[1]);		break;
		case CPUINFO_STR_REGISTER + I860_R2:			sprintf(info->s, "R2 : %08x",  cpustate->iregs[2]);		break;
		case CPUINFO_STR_REGISTER + I860_R3:			sprintf(info->s, "R3 : %08x",  cpustate->iregs[3]);		break;
		case CPUINFO_STR_REGISTER + I860_R4:			sprintf(info->s, "R4 : %08x",  cpustate->iregs[4]);		break;
		case CPUINFO_STR_REGISTER + I860_R5:			sprintf(info->s, "R5 : %08x",  cpustate->iregs[5]);		break;
		case CPUINFO_STR_REGISTER + I860_R6:			sprintf(info->s, "R6 : %08x",  cpustate->iregs[6]);		break;
		case CPUINFO_STR_REGISTER + I860_R7:			sprintf(info->s, "R7 : %08x",  cpustate->iregs[7]);		break;
		case CPUINFO_STR_REGISTER + I860_R8:			sprintf(info->s, "R8 : %08x",  cpustate->iregs[8]);		break;
		case CPUINFO_STR_REGISTER + I860_R9:			sprintf(info->s, "R9 : %08x",  cpustate->iregs[9]);		break;
		case CPUINFO_STR_REGISTER + I860_R10:			sprintf(info->s, "R10 : %08x", cpustate->iregs[10]);	break;
		case CPUINFO_STR_REGISTER + I860_R11:			sprintf(info->s, "R11 : %08x", cpustate->iregs[11]);	break;
		case CPUINFO_STR_REGISTER + I860_R12:			sprintf(info->s, "R12 : %08x", cpustate->iregs[12]);	break;
		case CPUINFO_STR_REGISTER + I860_R13:			sprintf(info->s, "R13 : %08x", cpustate->iregs[13]);	break;
		case CPUINFO_STR_REGISTER + I860_R14:			sprintf(info->s, "R14 : %08x", cpustate->iregs[14]);	break;
		case CPUINFO_STR_REGISTER + I860_R15:			sprintf(info->s, "R15 : %08x", cpustate->iregs[15]);	break;
		case CPUINFO_STR_REGISTER + I860_R16:			sprintf(info->s, "R16 : %08x", cpustate->iregs[16]);	break;
		case CPUINFO_STR_REGISTER + I860_R17:			sprintf(info->s, "R17 : %08x", cpustate->iregs[17]);	break;
		case CPUINFO_STR_REGISTER + I860_R18:			sprintf(info->s, "R18 : %08x", cpustate->iregs[18]);	break;
		case CPUINFO_STR_REGISTER + I860_R19:			sprintf(info->s, "R19 : %08x", cpustate->iregs[19]);	break;
		case CPUINFO_STR_REGISTER + I860_R20:			sprintf(info->s, "R20 : %08x", cpustate->iregs[20]);	break;
		case CPUINFO_STR_REGISTER + I860_R21:			sprintf(info->s, "R21 : %08x", cpustate->iregs[21]);	break;
		case CPUINFO_STR_REGISTER + I860_R22:			sprintf(info->s, "R22 : %08x", cpustate->iregs[22]);	break;
		case CPUINFO_STR_REGISTER + I860_R23:			sprintf(info->s, "R23 : %08x", cpustate->iregs[23]);	break;
		case CPUINFO_STR_REGISTER + I860_R24:			sprintf(info->s, "R24 : %08x", cpustate->iregs[24]);	break;
		case CPUINFO_STR_REGISTER + I860_R25:			sprintf(info->s, "R25 : %08x", cpustate->iregs[25]);	break;
		case CPUINFO_STR_REGISTER + I860_R26:			sprintf(info->s, "R26 : %08x", cpustate->iregs[26]);	break;
		case CPUINFO_STR_REGISTER + I860_R27:			sprintf(info->s, "R27 : %08x", cpustate->iregs[27]);	break;
		case CPUINFO_STR_REGISTER + I860_R28:			sprintf(info->s, "R28 : %08x", cpustate->iregs[28]);	break;
		case CPUINFO_STR_REGISTER + I860_R29:			sprintf(info->s, "R29 : %08x", cpustate->iregs[29]);	break;
		case CPUINFO_STR_REGISTER + I860_R30:			sprintf(info->s, "R30 : %08x", cpustate->iregs[30]);	break;
		case CPUINFO_STR_REGISTER + I860_R31:			sprintf(info->s, "R31 : %08x", cpustate->iregs[31]);	break;

		case CPUINFO_STR_REGISTER + I860_F0:			CPU_GET_INFO_STR_F(0);	break;
		case CPUINFO_STR_REGISTER + I860_F1:			CPU_GET_INFO_STR_F(1);	break;
		case CPUINFO_STR_REGISTER + I860_F2:			CPU_GET_INFO_STR_F(2);	break;
		case CPUINFO_STR_REGISTER + I860_F3:			CPU_GET_INFO_STR_F(3);	break;
		case CPUINFO_STR_REGISTER + I860_F4:			CPU_GET_INFO_STR_F(4);	break;
		case CPUINFO_STR_REGISTER + I860_F5:			CPU_GET_INFO_STR_F(5);	break;
		case CPUINFO_STR_REGISTER + I860_F6:			CPU_GET_INFO_STR_F(6);	break;
		case CPUINFO_STR_REGISTER + I860_F7:			CPU_GET_INFO_STR_F(7);	break;
		case CPUINFO_STR_REGISTER + I860_F8:			CPU_GET_INFO_STR_F(8);	break;
		case CPUINFO_STR_REGISTER + I860_F9:			CPU_GET_INFO_STR_F(9);	break;
		case CPUINFO_STR_REGISTER + I860_F10:			CPU_GET_INFO_STR_F(10);	break;
		case CPUINFO_STR_REGISTER + I860_F11:			CPU_GET_INFO_STR_F(11);	break;
		case CPUINFO_STR_REGISTER + I860_F12:			CPU_GET_INFO_STR_F(12);	break;
		case CPUINFO_STR_REGISTER + I860_F13:			CPU_GET_INFO_STR_F(13);	break;
		case CPUINFO_STR_REGISTER + I860_F14:			CPU_GET_INFO_STR_F(14);	break;
		case CPUINFO_STR_REGISTER + I860_F15:			CPU_GET_INFO_STR_F(15);	break;
		case CPUINFO_STR_REGISTER + I860_F16:			CPU_GET_INFO_STR_F(16);	break;
		case CPUINFO_STR_REGISTER + I860_F17:			CPU_GET_INFO_STR_F(17);	break;
		case CPUINFO_STR_REGISTER + I860_F18:			CPU_GET_INFO_STR_F(18);	break;
		case CPUINFO_STR_REGISTER + I860_F19:			CPU_GET_INFO_STR_F(19);	break;
		case CPUINFO_STR_REGISTER + I860_F20:			CPU_GET_INFO_STR_F(20);	break;
		case CPUINFO_STR_REGISTER + I860_F21:			CPU_GET_INFO_STR_F(21);	break;
		case CPUINFO_STR_REGISTER + I860_F22:			CPU_GET_INFO_STR_F(22);	break;
		case CPUINFO_STR_REGISTER + I860_F23:			CPU_GET_INFO_STR_F(23);	break;
		case CPUINFO_STR_REGISTER + I860_F24:			CPU_GET_INFO_STR_F(24);	break;
		case CPUINFO_STR_REGISTER + I860_F25:			CPU_GET_INFO_STR_F(25);	break;
		case CPUINFO_STR_REGISTER + I860_F26:			CPU_GET_INFO_STR_F(26);	break;
		case CPUINFO_STR_REGISTER + I860_F27:			CPU_GET_INFO_STR_F(27);	break;
		case CPUINFO_STR_REGISTER + I860_F28:			CPU_GET_INFO_STR_F(28);	break;
		case CPUINFO_STR_REGISTER + I860_F29:			CPU_GET_INFO_STR_F(29);	break;
		case CPUINFO_STR_REGISTER + I860_F30:			CPU_GET_INFO_STR_F(30);	break;
		case CPUINFO_STR_REGISTER + I860_F31:			CPU_GET_INFO_STR_F(31);	break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(I860, i860);
