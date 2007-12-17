/**************************************************************************
 *               National Semiconductor COP420 Emulator                   *
 *                                                                        *
 *                   Copyright (C) 2006 MAME Team                         *
 **************************************************************************/

/*

    TODO:

    - serial I/O
    - interrupt

*/

#include "cpuintrf.h"
#include "debugger.h"
#include "cop400.h"

/* The opcode table now is a combination of cycle counts and function pointers */
typedef struct {
	unsigned cycles;
	void (*function) (void);
}	s_opcode;

#define UINT1	UINT8
#define UINT4	UINT8
#define UINT6	UINT8
#define UINT10	UINT16

typedef struct
{
	UINT10 	PC;
	UINT10	PREVPC;
	UINT4	A;
	UINT6	B;
	UINT1	C;
	UINT4	EN;
	UINT4	G;
	UINT8	Q;
	UINT10	SA, SB, SC;
	UINT4	SIO;
	UINT1	SKL;
	UINT8   skip, skipLBI;
	UINT1	timerlatch;
	UINT16	counter;
	UINT8	G_mask;
	UINT8	D_mask;
} COP420_Regs;

static COP420_Regs R;
static int    cop420_ICount;

static int InstLen[256];
static int LBIops[256];
static int LBIops33[256];

#include "420ops.c"

static s_opcode cop420_opcode_op33[256]=
{
	{1, inil 		},{1, skgbz0 	},{1, illegal 	},{1, skgbz2 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, skgbz1 	},{1, illegal 	},{1, skgbz3 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, skgz	 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, inin 		},{1, illegal 	},{1, ing	 	},{1, illegal 	},{1, cqma	 	},{1, illegal 	},{1, inl	 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, omg	 	},{1, illegal 	},{1, camq	 	},{1, illegal 	},{1, obd	 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, ogi0	 	},{1, ogi1	 	},{1, ogi2	 	},{1, ogi3	 	},{1, ogi4	 	},{1, ogi5	 	},{1, ogi6	 	},{1, ogi7	 	},
	{1, ogi8	 	},{1, ogi9	 	},{1, ogi10	 	},{1, ogi11	 	},{1, ogi12	 	},{1, ogi13	 	},{1, ogi14	 	},{1, ogi15	 	},
	{1, lei0	 	},{1, lei1	 	},{1, lei2	 	},{1, lei3	 	},{1, lei4	 	},{1, lei5	 	},{1, lei6	 	},{1, lei7	 	},
	{1, lei8	 	},{1, lei9	 	},{1, lei10	 	},{1, lei11	 	},{1, lei12	 	},{1, lei13	 	},{1, lei14	 	},{1, lei15	 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, lbi0_1 	},{1, lbi0_2 	},{1, lbi0_3 	},{1, lbi0_4 	},{1, lbi0_5 	},{1, lbi0_6 	},{1, lbi0_7 	},
	{1, lbi0_8	 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, lbi1_1 	},{1, lbi1_2 	},{1, lbi1_3 	},{1, lbi1_4 	},{1, lbi1_5 	},{1, lbi1_6 	},{1, lbi1_7 	},
	{1, lbi1_8	 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, lbi2_1 	},{1, lbi2_2 	},{1, lbi2_3 	},{1, lbi2_4 	},{1, lbi2_5 	},{1, lbi2_6 	},{1, lbi2_7 	},
	{1, lbi2_8	 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, lbi3_1 	},{1, lbi3_2 	},{1, lbi3_3 	},{1, lbi3_4 	},{1, lbi3_5 	},{1, lbi3_6 	},{1, lbi3_7 	},
	{1, lbi3_8	 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	}
};

static void cop420_op33(void)
{
	(*(cop420_opcode_op33[ROM(PC++)].function))();
}

static s_opcode cop420_opcode_main[256]=
{
	{1, clra		},{1, skmbz0	},{1, xor		},{1, skmbz2		},{1, xis0		},{1, ld0		},{1, x0		},{1, xds0		},
	{1, lbi0_9		},{1, lbi0_10	},{1, lbi0_11	},{1, lbi0_12		},{1, lbi0_13	},{1, lbi0_14	},{1, lbi0_15	},{1, lbi0_0	},
	{1, casc		},{1, skmbz1	},{1, xabr		},{1, skmbz3		},{1, xis0		},{1, ld1		},{1, x1		},{1, xds1		},
	{1, lbi1_9		},{1, lbi1_10	},{1, lbi1_11	},{1, lbi1_12		},{1, lbi1_13	},{1, lbi1_14	},{1, lbi1_15	},{1, lbi1_0	},
	{1, skc			},{1, ske		},{1, sc		},{2, xad			},{1, xis2		},{1, ld2		},{1, x2		},{1, xds2 		},
	{1,	lbi2_9		},{1, lbi2_10	},{1, lbi2_11	},{1, lbi2_12		},{1, lbi2_13	},{1, lbi2_14	},{1, lbi2_15	},{1, lbi2_0	},
	{1, asc			},{1, add		},{1, rc		},{2, cop420_op33  	},{1, xis3		},{1, ld3		},{1, x3		},{1, xds3		},
	{1,	lbi3_9		},{1, lbi3_10	},{1, lbi3_11	},{1, lbi3_12		},{1, lbi3_13	},{1, lbi3_14	},{1, lbi3_15	},{1, lbi3_0	},
	{1, comp		},{1, skt		},{1, rmb2		},{1, rmb2			},{1, nop		},{1, rmb1		},{1, smb2		},{1, smb1		},
	{1,	ret			},{1, retsk		},{1, adt		},{1, smb3			},{1, rmb0		},{1, smb0		},{1, cba		},{1, xas		},
	{1, cab			},{1, aisc1		},{1, aisc2		},{1, aisc3			},{1, aisc4		},{1, aisc5		},{1, aisc6		},{1, aisc7		},
	{1, aisc8		},{1, aisc9		},{1, aisc10	},{1, aisc11		},{1, aisc12	},{1, aisc13	},{1, aisc14	},{1, aisc15	},
	{2, jmp0		},{2, jmp1		},{2, jmp2		},{2, jmp3			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal   },
	{2, jsr0		},{2, jsr1		},{2, jsr2		},{2, jsr3			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
	{1, stii0		},{1, stii1		},{1, stii2		},{1, stii3			},{1, stii4		},{1, stii5		},{1, stii6		},{1, stii7		},
	{1, stii8		},{1, stii9		},{1, stii10	},{1, stii11		},{1, stii12	},{1, stii13	},{1, stii14	},{1, stii15	},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, lqid		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jid		}
};

static ADDRESS_MAP_START( cop420_RAM, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x40) AM_RAM
ADDRESS_MAP_END

/****************************************************************************
 * Initialize emulation
 ****************************************************************************/
static void cop420_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	int i;

	memset(&R, 0, sizeof(R));
	R.G_mask = 0x0F;
	R.D_mask = 0x0F;

	R.counter = 0x00;

	for (i=0; i<256; i++) InstLen[i]=1;

	InstLen[0x60] = InstLen[0x61] = InstLen[0x62] = InstLen[0x63] =
	InstLen[0x68] = InstLen[0x69] = InstLen[0x6a] = InstLen[0x6b] =
	InstLen[0x33] = InstLen[0x23] = 2;

	for (i=0; i<256; i++) LBIops[i] = 0;
	for (i=0x08; i<0x10; i++) LBIops[i] = 1;
	for (i=0x18; i<0x20; i++) LBIops[i] = 1;
	for (i=0x28; i<0x30; i++) LBIops[i] = 1;
	for (i=0x38; i<0x40; i++) LBIops[i] = 1;

	for (i=0; i<256; i++) LBIops33[i] = 0;
	for (i=0x81; i<0x89; i++) LBIops33[i] = 1;
	for (i=0x91; i<0x99; i++) LBIops33[i] = 1;
	for (i=0xa1; i<0xa9; i++) LBIops33[i] = 1;
	for (i=0xb1; i<0xb9; i++) LBIops33[i] = 1;

	state_save_register_item("cop420", index, PC);
	state_save_register_item("cop420", index, R.PREVPC);
	state_save_register_item("cop420", index, A);
	state_save_register_item("cop420", index, B);
	state_save_register_item("cop420", index, C);
	state_save_register_item("cop420", index, EN);
	state_save_register_item("cop420", index, G);
	state_save_register_item("cop420", index, Q);
	state_save_register_item("cop420", index, SA);
	state_save_register_item("cop420", index, SB);
	state_save_register_item("cop420", index, SC);
	state_save_register_item("cop420", index, SIO);
	state_save_register_item("cop420", index, SKL);
	state_save_register_item("cop420", index, skip);
	state_save_register_item("cop420", index, skipLBI);
	state_save_register_item("cop420", index, R.timerlatch);
	state_save_register_item("cop420", index, R.counter);
	state_save_register_item("cop420", index, R.G_mask);
	state_save_register_item("cop420", index, R.D_mask);
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static void cop420_reset(void)
{
	PC = 0;
	A = 0;
	B = 0;
	C = 0;
	OUT_D(0);
	EN = 0;
	WRITE_G(0);
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static int cop420_execute(int cycles)
{
	unsigned opcode;

	cop420_ICount = cycles;

	do
	{
		prevPC = PC;

		CALL_MAME_DEBUG;

		opcode = ROM(PC);

		if (skipLBI == 1)
		{
			int is_lbi = 0;

			if (opcode == 0x33)
			{
				is_lbi = LBIops33[ROM(PC+1)];
			}
			else
			{
				is_lbi = LBIops[opcode];
			}

			if (is_lbi == 0)
			{
				skipLBI = 0;
			}
			else {
				cop420_ICount -= cop420_opcode_main[opcode].cycles;

				PC += InstLen[opcode];
			}
		}

		if (skipLBI == 0)
		{
			int inst_cycles = cop420_opcode_main[opcode].cycles;
			PC++;
			(*(cop420_opcode_main[opcode].function))();
			cop420_ICount -= inst_cycles;

			if (skip == 1) {
				opcode=ROM(PC);
				cop420_ICount -= cop420_opcode_main[opcode].cycles;
				PC += InstLen[opcode];
				skip = 0;
			}
		}

		/* counter handling */
		R.counter += cop420_opcode_main[opcode].cycles;
		if (R.counter > 1024)
		{
			R.timerlatch = 1;
			R.counter = 0;
		}

	} while (cop420_ICount > 0);

	return cycles - cop420_ICount;
}

/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
static void cop420_get_context (void *dst)
{
	if( dst )
		*(COP420_Regs*)dst = R;
}


/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
static void cop420_set_context (void *src)
{
	if( src )
		R = *(COP420_Regs*)src;
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void cop420_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + COP400_PC:			PC = info->i;							break;

	    case CPUINFO_INT_REGISTER + COP400_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_B:			B = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_C:			C = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_EN:			EN = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_G:			G = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_Q:			Q = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SA:			SA = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SB:			SB = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SC:			SC = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SIO:			SIO = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SKL:			SKL = info->i;							break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void cop420_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(R);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = COP400_CLOCK_DIVIDER;			break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 8;	/* Really 6 */	break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 9;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;						    break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = prevPC;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + COP400_PC:			info->i = PC;							break;
		case CPUINFO_INT_REGISTER + COP400_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + COP400_B:			info->i = B;							break;
		case CPUINFO_INT_REGISTER + COP400_C:			info->i = C;							break;
		case CPUINFO_INT_REGISTER + COP400_EN:			info->i = EN;							break;
		case CPUINFO_INT_REGISTER + COP400_G:			info->i = G;							break;
		case CPUINFO_INT_REGISTER + COP400_Q:			info->i = Q;							break;
		case CPUINFO_INT_REGISTER + COP400_SA:			info->i = SA;							break;
		case CPUINFO_INT_REGISTER + COP400_SB:			info->i = SB;							break;
		case CPUINFO_INT_REGISTER + COP400_SC:			info->i = SC;							break;
		case CPUINFO_INT_REGISTER + COP400_SIO:			info->i = SIO;							break;
		case CPUINFO_INT_REGISTER + COP400_SKL:			info->i = SKL;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = cop420_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = cop420_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = cop420_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = cop420_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = cop420_reset;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = cop420_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = cop420_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cop420_ICount;			break;
 		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:
 			info->internal_map = construct_map_cop420_RAM;										break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP420");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COP420"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C) 2006 MAME Team"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, " ");
			break;

		case CPUINFO_STR_REGISTER + COP400_PC:			sprintf(info->s, "PC:%04X", PC);		break;
		case CPUINFO_STR_REGISTER + COP400_A:			sprintf(info->s, "A:%01X", A );			break;
		case CPUINFO_STR_REGISTER + COP400_B:			sprintf(info->s, "B:%02X", B );			break;
		case CPUINFO_STR_REGISTER + COP400_C:			sprintf(info->s, "C:%01X", C);			break;
		case CPUINFO_STR_REGISTER + COP400_EN:			sprintf(info->s, "EN:%01X", EN);		break;
		case CPUINFO_STR_REGISTER + COP400_G:			sprintf(info->s, "G:%01X", G);			break;
		case CPUINFO_STR_REGISTER + COP400_Q:			sprintf(info->s, "Q:%02X", Q);			break;
		case CPUINFO_STR_REGISTER + COP400_SA:			sprintf(info->s, "SA:%04X", SA);		break;
		case CPUINFO_STR_REGISTER + COP400_SB:			sprintf(info->s, "SB:%04X", SB);		break;
		case CPUINFO_STR_REGISTER + COP400_SC:			sprintf(info->s, "SC:%04X", SC);		break;
		case CPUINFO_STR_REGISTER + COP400_SIO:			sprintf(info->s, "SIO:%01X", SIO);		break;
		case CPUINFO_STR_REGISTER + COP400_SKL:			sprintf(info->s, "SKL:%01X", SKL);		break;
	}
}
