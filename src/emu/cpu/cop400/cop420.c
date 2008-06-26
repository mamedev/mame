/***************************************************************************

    cop420.c

    National Semiconductor COP420 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*

    TODO:

    - when is the microbus int cleared?
    - CKO sync input
    - save internal RAM when CKO is RAM power supply pin
    - run interrupt test suite
    - run production test suite
    - run microbus test suite
    - remove LBIops
    - move cop440 stuff to cop440.c

*/

#include "cpuintrf.h"
#include "debugger.h"
#include "cop400.h"

/* The opcode table now is a combination of cycle counts and function pointers */
typedef struct {
	unsigned cycles;
	void (*function) (UINT8 opcode);
}	s_opcode;

#define UINT1	UINT8
#define UINT4	UINT8
#define UINT6	UINT8
#define UINT10	UINT16

typedef struct
{
	const cop400_interface *intf;

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
	UINT8	IN_mask;
	UINT4	IL;
	UINT4	in[4];
	UINT8	si;
	int		last_skip;
	int		microbus_int;
	int		halt;
} COP420_Regs;

static COP420_Regs R;
static int    cop420_ICount;

static int InstLen[256];
static int LBIops[256];
static int LBIops33[256];

static emu_timer *cop420_serial_timer;
static emu_timer *cop420_counter_timer;
static emu_timer *cop420_inil_timer;
static emu_timer *cop420_microbus_timer;

#include "420ops.c"

/* Opcode Maps */

static const s_opcode opcode_23_map[256]=
{
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},

	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	}
};

static void cop420_op23(UINT8 opcode)
{
	UINT8 opcode23 = ROM(PC++);

	(*(opcode_23_map[opcode23].function))(opcode23);
}

static const s_opcode opcode_33_map[256]=
{
	{1, illegal		},{1, skgbz0 	},{1, illegal 	},{1, skgbz2 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, skgbz1 	},{1, illegal 	},{1, skgbz3 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, skgz	 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, inin 		},{1, inil 		},{1, ing	 	},{1, illegal 	},{1, cqma	 	},{1, illegal 	},{1, inl	 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, omg	 	},{1, illegal 	},{1, camq	 	},{1, illegal 	},{1, obd	 	},{1, illegal 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, ogi	 		},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},
	{1, ogi	 		},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},
	{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},
	{1, lei 		},{1, lei 		},{1, lei	 	},{1, lei	 	},{1, lei	 	},{1, lei	 	},{1, lei	 	},{1, lei	 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},

	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	}
};

static void cop420_op33(UINT8 opcode)
{
	UINT8 opcode33 = ROM(PC++);

	(*(opcode_33_map[opcode33].function))(opcode33);
}

static const s_opcode opcode_map[256]=
{
	{1, clra		},{1, skmbz0	},{1, xor		},{1, skmbz2		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, casc		},{1, skmbz1	},{1, xabr		},{1, skmbz3		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, skc			},{1, ske		},{1, sc		},{1, cop420_op23	},{1, xis		},{1, ld		},{1, x			},{1, xds 		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, asc			},{1, add		},{1, rc		},{1, cop420_op33  	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},

	{1, comp		},{1, skt		},{1, rmb2		},{1, rmb3			},{1, nop		},{1, rmb1		},{1, smb2		},{1, smb1		},
	{1,	cop420_ret	},{1, retsk		},{1, adt		},{1, smb3			},{1, rmb0		},{1, smb0		},{1, cba		},{1, xas		},
	{1, cab			},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{2, jmp			},{2, jmp		},{2, jmp		},{2, jmp			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal   },
	{2, jsr			},{2, jsr		},{2, jsr		},{2, jsr			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},

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
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, jid		}
};

/* Memory Maps */

static ADDRESS_MAP_START( cop420_internal_rom, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cop420_internal_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

/* Binary Counter */

static TIMER_CALLBACK(cop420_counter_tick)
{
	cpuintrf_push_context(param);

	R.counter++;

	if (R.counter == 1024)
	{
		R.counter = 0;
		R.timerlatch = 1;
	}

	cpuintrf_pop_context();
}

/* IN Latches */

static TIMER_CALLBACK(cop420_inil_tick)
{
	UINT8 in;
	int i;

	cpuintrf_push_context(param);

	in = IN_IN();
	for (i = 0; i < 4; i++)
	{
		R.in[i] = (R.in[i] << 1) | BIT(in, i);

		if ((R.in[i] & 0x07) == 0x04) // 100
		{
			IL |= (1 << i);
		}
	}

	cpuintrf_pop_context();
}

/* Microbus */

static TIMER_CALLBACK(cop420_microbus_tick)
{
	UINT8 in;

	cpuintrf_push_context(param);

	in = IN_IN();

	if (!BIT(in, 2))
	{
		// chip select

		if (!BIT(in, 1))
		{
			// read strobe

			OUT_L(Q);

			R.microbus_int = 1;
		}
		else if (!BIT(in, 3))
		{
			// write strobe

			Q = IN_L();

			R.microbus_int = 0;
		}
	}

	cpuintrf_pop_context();
}

/****************************************************************************
 * Initialize emulation
 ****************************************************************************/
static void cop420_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	int i;

	memset(&R, 0, sizeof(R));

	R.intf = (cop400_interface *) config;

	assert(R.intf != NULL);

	/* set output pin masks */

	R.G_mask = 0x0f;	// G0-G3 available
	R.D_mask = 0x0f;	// D0-D3 available
	R.IN_mask = 0x0f;	// IN0-IN3 available

	/* set clock divider */

	cpunum_set_info_int(index, CPUINFO_INT_CLOCK_DIVIDER, R.intf->cki);

	/* allocate serial timer */

	cop420_serial_timer = timer_alloc(cop410_serial_tick, NULL);
	timer_adjust_periodic(cop420_serial_timer, attotime_zero, index, ATTOTIME_IN_HZ(clock));

	/* allocate counter timer */

	cop420_counter_timer = timer_alloc(cop420_counter_tick, NULL);
	timer_adjust_periodic(cop420_counter_timer, attotime_zero, index, ATTOTIME_IN_HZ(clock));

	/* allocate IN latch timer */

	cop420_inil_timer = timer_alloc(cop420_inil_tick, NULL);
	timer_adjust_periodic(cop420_inil_timer, attotime_zero, index, ATTOTIME_IN_HZ(clock));

	/* allocate Microbus timer */

	if (R.intf->microbus == COP400_MICROBUS_ENABLED)
	{
		cop420_microbus_timer = timer_alloc(cop420_microbus_tick, NULL);
		timer_adjust_periodic(cop420_microbus_timer, attotime_zero, index, ATTOTIME_IN_HZ(clock));
	}

	/* initialize instruction length array */

	for (i=0; i<256; i++) InstLen[i]=1;

	InstLen[0x60] = InstLen[0x61] = InstLen[0x62] = InstLen[0x63] =
	InstLen[0x68] = InstLen[0x69] = InstLen[0x6a] = InstLen[0x6b] =
	InstLen[0x33] = InstLen[0x23] = 2;

	/* initialize LBI opcode array */

	for (i=0; i<256; i++) LBIops[i] = 0;
	for (i=0x08; i<0x10; i++) LBIops[i] = 1;
	for (i=0x18; i<0x20; i++) LBIops[i] = 1;
	for (i=0x28; i<0x30; i++) LBIops[i] = 1;
	for (i=0x38; i<0x40; i++) LBIops[i] = 1;

	for (i=0; i<256; i++) LBIops33[i] = 0;
	for (i=0x80; i<0xc0; i++) LBIops33[i] = 1;

	/* register for state saving */

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
	state_save_register_item("cop420", index, R.IN_mask);
	state_save_register_item("cop420", index, R.si);
	state_save_register_item("cop420", index, R.last_skip);
	state_save_register_item_array("cop420", index, R.in);
	state_save_register_item("cop420", index, R.microbus_int);
	state_save_register_item("cop420", index, R.halt);
}

static void cop421_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	cop420_init(index, clock, config, irqcallback);

	/* microbus option is not available */

	assert(R.intf->microbus != COP400_MICROBUS_ENABLED);

	/* set output pin masks */

	R.IN_mask = 0; // IN lines not available
}

static void cop422_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	cop420_init(index, clock, config, irqcallback);

	/* microbus option is not available */

	assert(R.intf->microbus != COP400_MICROBUS_ENABLED);

	/* set output pin masks */

	R.G_mask = 0x0e; // only G2, G3 available
	R.D_mask = 0x0e; // only D2, D3 available
	R.IN_mask = 0; // IN lines not available
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

	R.counter = 0;
	R.timerlatch = 1;
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static int cop420_execute(int cycles)
{
	UINT8 opcode;

	cop420_ICount = cycles;

	do
	{
		prevPC = PC;

		debugger_instruction_hook(Machine, PC);

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
			else
			{
				cop420_ICount -= opcode_map[opcode].cycles;

				PC += InstLen[opcode];
			}
		}

		if (skipLBI == 0)
		{
			int inst_cycles = opcode_map[opcode].cycles;

			PC++;

			(*(opcode_map[opcode].function))(opcode);
			cop420_ICount -= inst_cycles;

			// check for interrupt

			if (BIT(EN, 1) && BIT(IL, 1))
			{
				void *function = opcode_map[ROM(PC)].function;

				if ((function != jp) &&	(function != jmp) && (function != jsr))
				{
					// store skip logic
					R.last_skip = skip;
					skip = 0;

					// push next PC
					PUSH(PC);

					// jump to interrupt service routine
					PC = 0x0ff;

					// disable interrupt
					EN &= ~0x02;
				}

				IL &= ~2;
			}

			// skip next instruction?

			if (skip == 1)
			{
				void *function = opcode_map[ROM(PC)].function;

				opcode = ROM(PC);

				if ((function == lqid) || (function == jid))
				{
					cop420_ICount -= 1;
				}
				else
				{
					cop420_ICount -= opcode_map[opcode].cycles;
				}
				PC += InstLen[opcode];

				skip = 0;
			}
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
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 16;							break;
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
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = cop420_dasm;		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cop420_ICount;			break;

/*      case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:
            info->internal_map8 = address_map_cop420_internal_rom;                              break;*/
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:
 			info->internal_map8 = address_map_cop420_internal_ram;								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP420");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright MAME Team"); break;

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

void cop421_get_info(UINT32 state, cpuinfo *info)
{
	// COP421 is a 24-pin package version of the COP420, lacking the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = cop421_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP421");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop420_get_info(state, info); break;
	}
}

void cop422_get_info(UINT32 state, cpuinfo *info)
{
	// COP422 is a 20-pin package version of the COP420, lacking G0/G1, D0/D1, and the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = cop422_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP422");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop420_get_info(state, info); break;
	}
}

void cop402_get_info(UINT32 state, cpuinfo *info)
{
	// COP402 is a ROMless version of the COP420

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:
 			info->internal_map8 = NULL;															break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP402");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop420_get_info(state, info); break;
	}
}

/* COP44x */

static void cop426_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	cop420_init(index, clock, config, irqcallback);

	R.G_mask = 0x0e; // only G2, G3 available
	R.D_mask = 0x0e; // only D2, D3 available
}

static void cop445_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	cop420_init(index, clock, config, irqcallback);

	R.G_mask = 0x07;
	R.D_mask = 0x03;
}

static ADDRESS_MAP_START( cop424_internal_rom, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cop424_internal_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cop444_internal_rom, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cop444_internal_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END

void cop444_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:
 			info->internal_map8 = address_map_cop444_internal_rom;								break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:
 			info->internal_map8 = address_map_cop444_internal_ram;								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP444");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop420_get_info(state, info); break;
	}
}

void cop445_get_info(UINT32 state, cpuinfo *info)
{
	// COP445 is a 24-pin package version of the COP444, lacking the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = cop445_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP445");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop444_get_info(state, info); break;
	}
}

void cop424_get_info(UINT32 state, cpuinfo *info)
{
	// COP424 is functionally equivalent to COP444, with only 1K ROM and 64x4 bytes RAM

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:
 			info->internal_map8 = address_map_cop424_internal_rom;								break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:
 			info->internal_map8 = address_map_cop424_internal_ram;								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP424");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop444_get_info(state, info); break;
	}
}

void cop425_get_info(UINT32 state, cpuinfo *info)
{
	// COP425 is a 24-pin package version of the COP424, lacking the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP425");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop444_get_info(state, info); break;
	}
}

void cop426_get_info(UINT32 state, cpuinfo *info)
{
	// COP426 is a 20-pin package version of the COP424, with only L0-L7, G2-G3, D2-D3 ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = cop426_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP426");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop444_get_info(state, info); break;
	}
}

void cop404_get_info(UINT32 state, cpuinfo *info)
{
	// COP404 is a ROMless version of the COP444, which can emulate a COP410 or a COP424

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:
 			info->internal_map8 = NULL;															break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP404");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: cop444_get_info(state, info); break;
	}
}
