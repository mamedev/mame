/***************************************************************************

    ppccom.c

    Common PowerPC definitions and functions

***************************************************************************/

#include "emu.h"
#include "ppccom.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_SPU				(0)
#define PRINTF_DECREMENTER		(0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DOUBLE_SIGN		(U64(0x8000000000000000))
#define DOUBLE_EXP		(U64(0x7ff0000000000000))
#define DOUBLE_FRAC		(U64(0x000fffffffffffff))
#define DOUBLE_ZERO		(0)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( ppc4xx_fit_callback );
static TIMER_CALLBACK( ppc4xx_pit_callback );
static TIMER_CALLBACK( ppc4xx_spu_callback );
static TIMER_CALLBACK( decrementer_int_callback );

static void ppc4xx_set_irq_line(powerpc_state *ppc, UINT32 bitmask, int state);

static void ppc4xx_dma_update_irq_states(powerpc_state *ppc);
static int ppc4xx_dma_fetch_transmit_byte(powerpc_state *ppc, int dmachan, UINT8 *byte);
static int ppc4xx_dma_handle_receive_byte(powerpc_state *ppc, int dmachan, UINT8 byte);
static void ppc4xx_dma_exec(powerpc_state *ppc, int dmachan);

static void ppc4xx_spu_update_irq_states(powerpc_state *ppc);
static void ppc4xx_spu_timer_reset(powerpc_state *ppc);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    page_access_allowed - return true if we are
    allowed to access memory based on the type
    of access and the protection bits
-------------------------------------------------*/

INLINE int page_access_allowed(int transtype, UINT8 key, UINT8 protbits)
{
	if (key == 0)
		return (transtype == TRANSLATE_WRITE) ? (protbits != 3) : TRUE;
	else
		return (transtype == TRANSLATE_WRITE) ? (protbits == 2) : (protbits != 0);
}


/*-------------------------------------------------
    get_cr - return the current CR value
-------------------------------------------------*/

INLINE UINT32 get_cr(powerpc_state *ppc)
{
	return	((ppc->cr[0] & 0x0f) << 28) |
			((ppc->cr[1] & 0x0f) << 24) |
			((ppc->cr[2] & 0x0f) << 20) |
			((ppc->cr[3] & 0x0f) << 16) |
			((ppc->cr[4] & 0x0f) << 12) |
			((ppc->cr[5] & 0x0f) << 8) |
			((ppc->cr[6] & 0x0f) << 4) |
			((ppc->cr[7] & 0x0f) << 0);
}


/*-------------------------------------------------
    set_cr - set the current CR value
-------------------------------------------------*/

INLINE void set_cr(powerpc_state *ppc, UINT32 value)
{
	ppc->cr[0] = value >> 28;
	ppc->cr[1] = value >> 24;
	ppc->cr[2] = value >> 20;
	ppc->cr[3] = value >> 16;
	ppc->cr[4] = value >> 12;
	ppc->cr[5] = value >> 8;
	ppc->cr[6] = value >> 4;
	ppc->cr[7] = value >> 0;
}


/*-------------------------------------------------
    get_xer - return the current XER value
-------------------------------------------------*/

INLINE UINT32 get_xer(powerpc_state *ppc)
{
	return ppc->spr[SPR_XER] | (ppc->xerso << 31);
}


/*-------------------------------------------------
    set_xer - set the current XER value
-------------------------------------------------*/

INLINE void set_xer(powerpc_state *ppc, UINT32 value)
{
	ppc->spr[SPR_XER] = value & ~XER_SO;
	ppc->xerso = value >> 31;
}


/*-------------------------------------------------
    get_timebase - return the current timebase
    value
-------------------------------------------------*/

INLINE UINT64 get_timebase(powerpc_state *ppc)
{
	if (!ppc->tb_divisor)
	{
		return (ppc->device->total_cycles() - ppc->tb_zero_cycles);
	}

	return (ppc->device->total_cycles() - ppc->tb_zero_cycles) / ppc->tb_divisor;
}


/*-------------------------------------------------
    set_timebase - set the timebase
-------------------------------------------------*/

INLINE void set_timebase(powerpc_state *ppc, UINT64 newtb)
{
	ppc->tb_zero_cycles = ppc->device->total_cycles() - newtb * ppc->tb_divisor;
}


/*-------------------------------------------------
    get_decremeter - return the current
    decrementer value
-------------------------------------------------*/

INLINE UINT32 get_decrementer(powerpc_state *ppc)
{
	INT64 cycles_until_zero = ppc->dec_zero_cycles - ppc->device->total_cycles();
	cycles_until_zero = MAX(cycles_until_zero, 0);

	if (!ppc->tb_divisor)
	{
		return 0;
	}

	return cycles_until_zero / ppc->tb_divisor;
}


/*-------------------------------------------------
    set_decrementer - set the decremeter
-------------------------------------------------*/

INLINE void set_decrementer(powerpc_state *ppc, UINT32 newdec)
{
	UINT64 cycles_until_done = ((UINT64)newdec + 1) * ppc->tb_divisor;
	UINT32 curdec = get_decrementer(ppc);

	if (!ppc->tb_divisor)
	{
		return;
	}

	if (PRINTF_DECREMENTER)
	{
		UINT64 total = ppc->device->total_cycles();
		mame_printf_debug("set_decrementer: olddec=%08X newdec=%08X divisor=%d totalcyc=%08X%08X timer=%08X%08X\n",
				curdec, newdec, ppc->tb_divisor,
				(UINT32)(total >> 32), (UINT32)total, (UINT32)(cycles_until_done >> 32), (UINT32)cycles_until_done);
	}

	ppc->dec_zero_cycles = ppc->device->total_cycles() + cycles_until_done;
	ppc->decrementer_int_timer->adjust(ppc->device->cycles_to_attotime(cycles_until_done));

	if ((INT32)curdec >= 0 && (INT32)newdec < 0)
		ppc->irq_pending |= 0x02;
}


/*-------------------------------------------------
    is_nan_double - is a double value a NaN
-------------------------------------------------*/

INLINE int is_nan_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) );
}


/*-------------------------------------------------
    is_qnan_double - is a double value a
    quiet NaN
-------------------------------------------------*/

INLINE int is_qnan_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & U64(0x0007fffffffffff)) == U64(0x000000000000000)) &&
			((xi & U64(0x000800000000000)) == U64(0x000800000000000)) );
}


/*-------------------------------------------------
    is_snan_double - is a double value a
    signaling NaN
-------------------------------------------------*/

INLINE int is_snan_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) &&
			((xi & U64(0x0008000000000000)) == DOUBLE_ZERO) );
}


/*-------------------------------------------------
    is_infinity_double - is a double value
    infinity
-------------------------------------------------*/

INLINE int is_infinity_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) == DOUBLE_ZERO) );
}


/*-------------------------------------------------
    is_normalized_double - is a double value
    normalized
-------------------------------------------------*/

INLINE int is_normalized_double(double x)
{
	UINT64 exp;
	UINT64 xi = *(UINT64*)&x;
	exp = (xi & DOUBLE_EXP) >> 52;

	return (exp >= 1) && (exp <= 2046);
}


/*-------------------------------------------------
    is_denormalized_double - is a double value
    denormalized
-------------------------------------------------*/

INLINE int is_denormalized_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == 0) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) );
}


/*-------------------------------------------------
    sign_double - return sign of a double value
-------------------------------------------------*/

INLINE int sign_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return ((xi & DOUBLE_SIGN) != 0);
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

/*-------------------------------------------------
    ppccom_init - initialize the powerpc_state
    structure based on the configured type
-------------------------------------------------*/

void ppccom_init(powerpc_state *ppc, powerpc_flavor flavor, UINT8 cap, int tb_divisor, legacy_cpu_device *device, device_irq_callback irqcallback)
{
	const powerpc_config *config = (const powerpc_config *)device->static_config();

	/* initialize based on the config */
	memset(ppc, 0, sizeof(*ppc));
	ppc->flavor = flavor;
	ppc->cap = cap;
	ppc->cache_line_size = 32;
	ppc->tb_divisor = tb_divisor;
	ppc->cpu_clock = device->clock();
	ppc->irq_callback = irqcallback;
	ppc->device = device;
	ppc->program = device->space(AS_PROGRAM);
	ppc->direct = &ppc->program->direct();
	ppc->system_clock = (config != NULL) ? config->bus_frequency : device->clock();
	ppc->tb_divisor = (ppc->tb_divisor * device->clock() + ppc->system_clock / 2 - 1) / ppc->system_clock;
	ppc->codexor = 0;
	if (!(cap & PPCCAP_4XX) && device->space_config()->m_endianness != ENDIANNESS_NATIVE)
		ppc->codexor = 4;

	/* allocate the virtual TLB */
	ppc->vtlb = vtlb_alloc(device, AS_PROGRAM, (cap & PPCCAP_603_MMU) ? PPC603_FIXED_TLB_ENTRIES : 0, POWERPC_TLB_ENTRIES);

	/* allocate a timer for the compare interrupt */
	if ((cap & PPCCAP_OEA) && (ppc->tb_divisor))
		ppc->decrementer_int_timer = device->machine().scheduler().timer_alloc(FUNC(decrementer_int_callback), ppc);

	/* and for the 4XX interrupts if needed */
	if (cap & PPCCAP_4XX)
	{
		ppc->fit_timer = device->machine().scheduler().timer_alloc(FUNC(ppc4xx_fit_callback), ppc);
		ppc->pit_timer = device->machine().scheduler().timer_alloc(FUNC(ppc4xx_pit_callback), ppc);
		ppc->spu.timer = device->machine().scheduler().timer_alloc(FUNC(ppc4xx_spu_callback), ppc);
	}

	/* register for save states */
	device->save_item(NAME(ppc->pc));
	device->save_item(NAME(ppc->r));
	device->save_item(NAME(ppc->f));
	device->save_item(NAME(ppc->cr));
	device->save_item(NAME(ppc->xerso));
	device->save_item(NAME(ppc->fpscr));
	device->save_item(NAME(ppc->msr));
	device->save_item(NAME(ppc->sr));
	device->save_item(NAME(ppc->spr));
	device->save_item(NAME(ppc->dcr));
	if (cap & PPCCAP_4XX)
	{
		device->save_item(NAME(ppc->spu.regs));
		device->save_item(NAME(ppc->spu.txbuf));
		device->save_item(NAME(ppc->spu.rxbuf));
		device->save_item(NAME(ppc->spu.rxbuffer));
		device->save_item(NAME(ppc->spu.rxin));
		device->save_item(NAME(ppc->spu.rxout));
		device->save_item(NAME(ppc->pit_reload));
		device->save_item(NAME(ppc->irqstate));
	}
	if (cap & PPCCAP_603_MMU)
	{
		device->save_item(NAME(ppc->mmu603_cmp));
		device->save_item(NAME(ppc->mmu603_hash));
		device->save_item(NAME(ppc->mmu603_r));
	}
	device->save_item(NAME(ppc->irq_pending));
	device->save_item(NAME(ppc->tb_zero_cycles));
	device->save_item(NAME(ppc->dec_zero_cycles));
}


/*-------------------------------------------------
    ppccom_exit - common cleanup/exit
-------------------------------------------------*/

void ppccom_exit(powerpc_state *ppc)
{
	if (ppc->vtlb != NULL)
		vtlb_free(ppc->vtlb);
}


/*-------------------------------------------------
    ppccom_reset - reset the state of all the
    registers
-------------------------------------------------*/

void ppccom_reset(powerpc_state *ppc)
{
	int tlbindex;

	/* initialize the OEA state */
	if (ppc->cap & PPCCAP_OEA)
	{
		/* PC to the reset vector; MSR has IP set to start */
		ppc->pc = 0xfff00100;
		ppc->msr = MSROEA_IP;

		/* reset the decrementer */
		ppc->dec_zero_cycles = ppc->device->total_cycles();
		if (ppc->tb_divisor)
		{
			decrementer_int_callback(ppc->device->machine(), ppc, 0);
		}
	}

	/* initialize the 4XX state */
	if (ppc->cap & PPCCAP_4XX)
	{
		/* PC to the last word; MSR to 0 */
		ppc->pc = 0xfffffffc;
		ppc->msr = 0;

		/* reset the SPU status */
		ppc->spr[SPR4XX_TCR] &= ~PPC4XX_TCR_WRC_MASK;
		ppc->spu.regs[SPU4XX_LINE_STATUS] = 0x06;
	}

	/* initialize the 602 HID0 register */
	if (ppc->flavor == PPC_MODEL_602)
		ppc->spr[SPR603_HID0] = 1;

	/* time base starts here */
	ppc->tb_zero_cycles = ppc->device->total_cycles();

	/* clear interrupts */
	ppc->irq_pending = 0;

	/* flush the TLB */
	vtlb_flush_dynamic(ppc->vtlb);
	if (ppc->cap & PPCCAP_603_MMU)
		for (tlbindex = 0; tlbindex < PPC603_FIXED_TLB_ENTRIES; tlbindex++)
			vtlb_load(ppc->vtlb, tlbindex, 0, 0, 0);
}


/*-------------------------------------------------
    ppccom_dasm - handle disassembly for a
    CPU
-------------------------------------------------*/

offs_t ppccom_dasm(powerpc_state *ppc, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	extern offs_t ppc_dasm_one(char *buffer, UINT32 pc, UINT32 op);
	UINT32 op = *(UINT32 *)oprom;
	op = BIG_ENDIANIZE_INT32(op);
	return ppc_dasm_one(buffer, pc, op);
}



/***************************************************************************
    TLB HANDLING
***************************************************************************/

/*-------------------------------------------------
    ppccom_translate_address_internal - translate
    an address from logical to physical; shared
    between external requests and internal TLB
    filling
-------------------------------------------------*/

static UINT32 ppccom_translate_address_internal(powerpc_state *ppc, int intention, offs_t *address)
{
	int transpriv = ((intention & TRANSLATE_USER_MASK) == 0);	// 1 for supervisor, 0 for user
	int transtype = intention & TRANSLATE_TYPE_MASK;
	offs_t hash, hashbase, hashmask;
	int batbase, batnum, hashnum;
	UINT32 segreg;

	/* 4xx case: "TLB" really just caches writes and checks compare registers */
	if (ppc->cap & PPCCAP_4XX)
	{
		/* we don't support the MMU of the 403GCX */
		if (ppc->flavor == PPC_MODEL_403GCX && (ppc->msr & MSROEA_DR))
			fatalerror("MMU enabled but not supported!");

		/* only check if PE is enabled */
		if (transtype == TRANSLATE_WRITE && (ppc->msr & MSR4XX_PE))
		{
			/* are we within one of the protection ranges? */
			int inrange1 = ((*address >> 12) >= (ppc->spr[SPR4XX_PBL1] >> 12) && (*address >> 12) < (ppc->spr[SPR4XX_PBU1] >> 12));
			int inrange2 = ((*address >> 12) >= (ppc->spr[SPR4XX_PBL2] >> 12) && (*address >> 12) < (ppc->spr[SPR4XX_PBU2] >> 12));

			/* if PX == 1, writes are only allowed OUTSIDE of the bounds */
			if (((ppc->msr & MSR4XX_PX) && (inrange1 || inrange2)) || (!(ppc->msr & MSR4XX_PX) && (!inrange1 && !inrange2)))
				return 0x002;
		}
		*address &= 0x7fffffff;
		return 0x001;
	}

	/* only applies if we support the OEA */
	if (!(ppc->cap & PPCCAP_OEA))
		return 0x001;

	/* also no translation necessary if translation is disabled */
	if ((transtype == TRANSLATE_FETCH && (ppc->msr & MSROEA_IR) == 0) || (transtype != TRANSLATE_FETCH && (ppc->msr & MSROEA_DR) == 0))
		return 0x001;

	/* first scan the appropriate BAT */
	if (ppc->cap & PPCCAP_601BAT)
	{
		for (batnum = 0; batnum < 4; batnum++)
		{
			UINT32 upper = ppc->spr[SPROEA_IBAT0U + 2*batnum + 0];
			UINT32 lower = ppc->spr[SPROEA_IBAT0U + 2*batnum + 1];
			int privbit = ((intention & TRANSLATE_USER_MASK) == 0) ? 3 : 2;

//          printf("bat %d upper = %08x privbit %d\n", batnum, upper, privbit);

			// is this pair valid?
			if (lower & 0x40)
			{
				UINT32 mask = (~lower & 0x3f) << 17;
				UINT32 addrout;
				UINT32 key = (upper >> privbit) & 1;

				/* check for a hit against this bucket */
				if ((*address & 0xfffe0000) == (upper & 0xfffe0000))
				{
					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, key, upper & 3))
					{
						return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
					}

					/* otherwise we're good */
					addrout = (lower & 0xff100000) | (*address & ~0xfffe0000);
					addrout |= ((*address & mask) | (lower & mask));
					*address = addrout; // top 9 bits from top 9 of PBN
					return 0x001;
				}
			}
		}
	}
	else
	{
		batbase = (transtype == TRANSLATE_FETCH) ? SPROEA_IBAT0U : SPROEA_DBAT0U;

		for (batnum = 0; batnum < 4; batnum++)
		{
			UINT32 upper = ppc->spr[batbase + 2*batnum + 0];

			/* check user/supervisor valid bit */
			if ((upper >> transpriv) & 0x01)
			{
				UINT32 mask = (~upper << 15) & 0xfffe0000;

				/* check for a hit against this bucket */
				if ((*address & mask) == (upper & mask))
				{
					UINT32 lower = ppc->spr[batbase + 2*batnum + 1];

					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, 1, lower & 3))
					{
						return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
					}

					/* otherwise we're good */
					*address = (lower & mask) | (*address & ~mask);
					return 0x001;
				}
			}
		}
	}

	/* look up the segment register */
	segreg = ppc->sr[*address >> 28];
	if (transtype == TRANSLATE_FETCH && (segreg & 0x10000000))
		return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);

	/* check for memory-forced I/O */
	if (ppc->cap & PPCCAP_MFIOC)
	{
		if ((transtype != TRANSLATE_FETCH) && ((segreg & 0x87f00000) == 0x87f00000))
		{
			*address = ((segreg & 0xf)<<28) | (*address & 0x0fffffff);
			return 1;
		}
		else if (segreg & 0x80000000)
		{
			fatalerror("PPC: Unhandled segment register %08x with T=1\n", segreg);
		}
	}

	/* get hash table information from SD1 */
	hashbase = ppc->spr[SPROEA_SDR1] & 0xffff0000;
	hashmask = ((ppc->spr[SPROEA_SDR1] & 0x1ff) << 16) | 0xffff;
	hash = (segreg & 0x7ffff) ^ ((*address >> 12) & 0xffff);

	/* if we're simulating the 603 MMU, fill in the data and stop here */
	if (ppc->cap & PPCCAP_603_MMU)
	{
		UINT32 entry = vtlb_table(ppc->vtlb)[*address >> 12];
		ppc->mmu603_cmp = 0x80000000 | ((segreg & 0xffffff) << 7) | (0 << 6) | ((*address >> 22) & 0x3f);
		ppc->mmu603_hash[0] = hashbase | ((hash << 6) & hashmask);
		ppc->mmu603_hash[1] = hashbase | ((~hash << 6) & hashmask);
		if ((entry & (VTLB_FLAG_FIXED | VTLB_FLAG_VALID)) == (VTLB_FLAG_FIXED | VTLB_FLAG_VALID))
		{
			*address = (entry & 0xfffff000) | (*address & 0x00000fff);
			return 0x001;
		}
		return DSISR_NOT_FOUND | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
	}

	/* loop twice over hashes */
	for (hashnum = 0; hashnum < 2; hashnum++)
	{
		offs_t ptegaddr = hashbase | ((hash << 6) & hashmask);
		UINT32 *ptegptr = (UINT32 *)ppc->program->get_read_ptr(ptegaddr);

		/* should only have valid memory here, but make sure */
		if (ptegptr != NULL)
		{
			UINT32 targetupper = 0x80000000 | ((segreg & 0xffffff) << 7) | (hashnum << 6) | ((*address >> 22) & 0x3f);
			int ptenum;

			/* scan PTEs */
			for (ptenum = 0; ptenum < 8; ptenum++)
				if (ptegptr[BYTE_XOR_BE(ptenum * 2)] == targetupper)
				{
					UINT32 pteglower = ptegptr[BYTE_XOR_BE(ptenum * 2 + 1)];

					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, (segreg >> (29 + transpriv)) & 1, pteglower & 3))
						return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);

					/* update page table bits */
					if (!(intention & TRANSLATE_DEBUG_MASK))
					{
						pteglower |= 0x100;
						if (transtype == TRANSLATE_WRITE)
							pteglower |= 0x080;
						ptegptr[BYTE_XOR_BE(ptenum * 2 + 1)] = pteglower;
					}

					/* otherwise we're good */
					*address = (pteglower & 0xfffff000) | (*address & 0x00000fff);
					return (pteglower >> 7) & 1;
				}
		}

		/* invert the hash after the first round */
		hash = ~hash;
	}

	/* we failed to find any match: not found */
	return DSISR_NOT_FOUND | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
}


/*-------------------------------------------------
    ppccom_translate_address - translate an address
    from logical to physical
-------------------------------------------------*/

int ppccom_translate_address(powerpc_state *ppc, address_spacenum space, int intention, offs_t *address)
{
	/* only applies to the program address space */
	if (space != AS_PROGRAM)
		return TRUE;

	/* translation is successful if the internal routine returns 0 or 1 */
	return (ppccom_translate_address_internal(ppc, intention, address) <= 1);
}


/*-------------------------------------------------
    ppccom_tlb_fill - handle a missing TLB entry
-------------------------------------------------*/

void ppccom_tlb_fill(powerpc_state *ppc)
{
	vtlb_fill(ppc->vtlb, ppc->param0, ppc->param1);
}


/*-------------------------------------------------
    ppccom_tlb_flush - flush the entire TLB,
    including fixed entries
-------------------------------------------------*/

void ppccom_tlb_flush(powerpc_state *ppc)
{
	vtlb_flush_dynamic(ppc->vtlb);
}



/***************************************************************************
    OPCODE HANDLING
***************************************************************************/

/*-------------------------------------------------
    ppccom_execute_tlbie - execute a TLBIE
    instruction
-------------------------------------------------*/

void ppccom_execute_tlbie(powerpc_state *ppc)
{
	vtlb_flush_address(ppc->vtlb, ppc->param0);
}


/*-------------------------------------------------
    ppccom_execute_tlbia - execute a TLBIA
    instruction
-------------------------------------------------*/

void ppccom_execute_tlbia(powerpc_state *ppc)
{
	vtlb_flush_dynamic(ppc->vtlb);
}


/*-------------------------------------------------
    ppccom_execute_tlbl - execute a TLBLD/TLBLI
    instruction
-------------------------------------------------*/

void ppccom_execute_tlbl(powerpc_state *ppc)
{
	UINT32 address = ppc->param0;
	int isitlb = ppc->param1;
	vtlb_entry flags = 0;
	int entrynum;

	/* determine entry number; we use rand() for associativity */
	entrynum = ((address >> 12) & 0x1f) | (ppc->device->machine().rand() & 0x20) | (isitlb ? 0x40 : 0);

	/* determine the flags */
	flags = VTLB_FLAG_VALID | VTLB_READ_ALLOWED | VTLB_FETCH_ALLOWED;
	if (ppc->spr[SPR603_RPA] & 0x80)
		flags |= VTLB_WRITE_ALLOWED;
	if (isitlb)
		flags |= VTLB_FETCH_ALLOWED;

	/* load the entry */
	vtlb_load(ppc->vtlb, entrynum, 1, address, (ppc->spr[SPR603_RPA] & 0xfffff000) | flags);
}


/*-------------------------------------------------
    ppccom_execute_mftb - execute an MFTB
    instruction
-------------------------------------------------*/

void ppccom_execute_mftb(powerpc_state *ppc)
{
	switch (ppc->param0)
	{
		/* user mode timebase read */
		case SPRVEA_TBL_R:
			ppc->param1 = get_timebase(ppc);
			break;
		case SPRVEA_TBU_R:
			ppc->param1 = get_timebase(ppc) >> 32;
			break;
	}
}


/*-------------------------------------------------
    ppccom_execute_mfspr - execute an MFSPR
    instruction
-------------------------------------------------*/

void ppccom_execute_mfspr(powerpc_state *ppc)
{
	/* handle OEA SPRs */
	if (ppc->cap & PPCCAP_OEA)
	{
		switch (ppc->param0)
		{
			/* read-through no-ops */
			case SPROEA_DSISR:
			case SPROEA_DAR:
			case SPROEA_SDR1:
			case SPROEA_SRR0:
			case SPROEA_SRR1:
			case SPROEA_EAR:
			case SPROEA_IBAT0L:
			case SPROEA_IBAT0U:
			case SPROEA_IBAT1L:
			case SPROEA_IBAT1U:
			case SPROEA_IBAT2L:
			case SPROEA_IBAT2U:
			case SPROEA_IBAT3L:
			case SPROEA_IBAT3U:
			case SPROEA_DBAT0L:
			case SPROEA_DBAT0U:
			case SPROEA_DBAT1L:
			case SPROEA_DBAT1U:
			case SPROEA_DBAT2L:
			case SPROEA_DBAT2U:
			case SPROEA_DBAT3L:
			case SPROEA_DBAT3U:
			case SPROEA_DABR:
				ppc->param1 = ppc->spr[ppc->param0];
				return;

			/* decrementer */
			case SPROEA_DEC:
				ppc->param1 = get_decrementer(ppc);
				return;
		}
	}

	/* handle 603 SPRs */
	if (ppc->cap & PPCCAP_603_MMU)
	{
		switch (ppc->param0)
		{
			/* read-through no-ops */
			case SPR603_DMISS:
			case SPR603_DCMP:
			case SPR603_HASH1:
			case SPR603_HASH2:
			case SPR603_IMISS:
			case SPR603_ICMP:
			case SPR603_RPA:
			case SPR603_HID0:
			case SPR603_HID1:
			case SPR603_IABR:
			case SPR603_HID2:
				ppc->param1 = ppc->spr[ppc->param0];
				return;

			/* timebase */
			case SPR603_TBL_R:
				ppc->param1 = get_timebase(ppc);
				return;
			case SPR603_TBU_R:
				ppc->param1 = (get_timebase(ppc) >> 32) & 0xffffff;
				return;
		}
	}

	/* handle 4XX SPRs */
	if (ppc->cap & PPCCAP_4XX)
	{
		switch (ppc->param0)
		{
			/* read-through no-ops */
			case SPR4XX_EVPR:
			case SPR4XX_ESR:
			case SPR4XX_SRR0:
			case SPR4XX_SRR1:
			case SPR4XX_SRR2:
			case SPR4XX_SRR3:
			case SPR4XX_TCR:
			case SPR4XX_TSR:
			case SPR4XX_IAC1:
			case SPR4XX_IAC2:
			case SPR4XX_DAC1:
			case SPR4XX_DAC2:
			case SPR4XX_DCCR:
			case SPR4XX_ICCR:
			case SPR4XX_PBL1:
			case SPR4XX_PBU1:
			case SPR4XX_PBL2:
			case SPR4XX_PBU2:
				ppc->param1 = ppc->spr[ppc->param0];
				return;

			/* timebase */
			case SPR4XX_TBLO:
			case SPR4XX_TBLU:
				ppc->param1 = get_timebase(ppc);
				return;
			case SPR4XX_TBHI:
			case SPR4XX_TBHU:
				ppc->param1 = (get_timebase(ppc) >> 32) & 0xffffff;
				return;
		}
	}

	/* default handling */
	mame_printf_debug("SPR %03X read\n", ppc->param0);
	ppc->param1 = ppc->spr[ppc->param0];
}


/*-------------------------------------------------
    ppccom_execute_mtspr - execute an MTSPR
    instruction
-------------------------------------------------*/

void ppccom_execute_mtspr(powerpc_state *ppc)
{
	/* handle OEA SPRs */
	if (ppc->cap & PPCCAP_OEA)
	{
		switch (ppc->param0)
		{
			/* write-through no-ops */
			case SPROEA_DSISR:
			case SPROEA_DAR:
			case SPROEA_SRR0:
			case SPROEA_SRR1:
			case SPROEA_EAR:
			case SPROEA_DABR:
				ppc->spr[ppc->param0] = ppc->param1;
				return;

			/* registers that affect the memory map */
			case SPROEA_SDR1:
			case SPROEA_IBAT0L:
			case SPROEA_IBAT0U:
			case SPROEA_IBAT1L:
			case SPROEA_IBAT1U:
			case SPROEA_IBAT2L:
			case SPROEA_IBAT2U:
			case SPROEA_IBAT3L:
			case SPROEA_IBAT3U:
			case SPROEA_DBAT0L:
			case SPROEA_DBAT0U:
			case SPROEA_DBAT1L:
			case SPROEA_DBAT1U:
			case SPROEA_DBAT2L:
			case SPROEA_DBAT2U:
			case SPROEA_DBAT3L:
			case SPROEA_DBAT3U:
				ppc->spr[ppc->param0] = ppc->param1;
				ppccom_tlb_flush(ppc);
				return;

			/* decrementer */
			case SPROEA_DEC:
				set_decrementer(ppc, ppc->param1);
				return;
		}
	}

	/* handle 603 SPRs */
	if (ppc->cap & PPCCAP_603_MMU)
	{
		switch (ppc->param0)
		{
			/* read-only */
			case SPR603_DMISS:
			case SPR603_DCMP:
			case SPR603_HASH1:
			case SPR603_HASH2:
			case SPR603_IMISS:
			case SPR603_ICMP:
				return;

			/* write-through no-ops */
			case SPR603_RPA:
			case SPR603_HID0:
			case SPR603_HID1:
			case SPR603_IABR:
			case SPR603_HID2:
				ppc->spr[ppc->param0] = ppc->param1;
				return;

			/* timebase */
			case SPR603_TBL_W:
				set_timebase(ppc, (get_timebase(ppc) & ~U64(0xffffffff00000000)) | ppc->param1);
				return;
			case SPR603_TBU_W:
				set_timebase(ppc, (get_timebase(ppc) & ~U64(0x00000000ffffffff)) | ((UINT64)ppc->param1 << 32));
				return;
		}
	}

	/* handle 4XX SPRs */
	if (ppc->cap & PPCCAP_4XX)
	{
		UINT32 oldval = ppc->spr[ppc->param0];
		switch (ppc->param0)
		{
			/* write-through no-ops */
			case SPR4XX_EVPR:
			case SPR4XX_ESR:
			case SPR4XX_DCCR:
			case SPR4XX_ICCR:
			case SPR4XX_SRR0:
			case SPR4XX_SRR1:
			case SPR4XX_SRR2:
			case SPR4XX_SRR3:
				ppc->spr[ppc->param0] = ppc->param1;
				return;

			/* registers that affect the memory map */
			case SPR4XX_PBL1:
			case SPR4XX_PBU1:
			case SPR4XX_PBL2:
			case SPR4XX_PBU2:
				ppc->spr[ppc->param0] = ppc->param1;
				ppccom_tlb_flush(ppc);
				return;

			/* timer control register */
			case SPR4XX_TCR:
				ppc->spr[SPR4XX_TCR] = ppc->param1 | (oldval & PPC4XX_TCR_WRC_MASK);
				if ((oldval ^ ppc->spr[SPR4XX_TCR]) & PPC4XX_TCR_FIE)
					ppc4xx_fit_callback(ppc->device->machine(), ppc, FALSE);
				if ((oldval ^ ppc->spr[SPR4XX_TCR]) & PPC4XX_TCR_PIE)
					ppc4xx_pit_callback(ppc->device->machine(), ppc, FALSE);
				return;

			/* timer status register */
			case SPR4XX_TSR:
				ppc->spr[SPR4XX_TSR] &= ~ppc->param1;
				ppc4xx_set_irq_line(ppc, 0, 0);
				return;

			/* PIT */
			case SPR4XX_PIT:
				ppc->spr[SPR4XX_PIT] = ppc->param1;
				ppc->pit_reload = ppc->param1;
				ppc4xx_pit_callback(ppc->device->machine(), ppc, FALSE);
				return;

			/* timebase */
			case SPR4XX_TBLO:
				set_timebase(ppc, (get_timebase(ppc) & ~U64(0x00ffffff00000000)) | ppc->param1);
				return;
			case SPR4XX_TBHI:
				set_timebase(ppc, (get_timebase(ppc) & ~U64(0x00000000ffffffff)) | ((UINT64)(ppc->param1 & 0x00ffffff) << 32));
				return;
		}
	}

	/* default handling */
	mame_printf_debug("SPR %03X write = %08X\n", ppc->param0, ppc->param1);
	ppc->spr[ppc->param0] = ppc->param1;
}


/*-------------------------------------------------
    ppccom_execute_mfdcr - execute an MFDCR
    instruction
-------------------------------------------------*/

void ppccom_execute_mfdcr(powerpc_state *ppc)
{
	/* handle various DCRs */
	switch (ppc->param0)
	{
		/* read-through no-ops */
		case DCR4XX_BR0:
		case DCR4XX_BR1:
		case DCR4XX_BR2:
		case DCR4XX_BR3:
		case DCR4XX_BR4:
		case DCR4XX_BR5:
		case DCR4XX_BR6:
		case DCR4XX_BR7:
		case DCR4XX_BESR:
		case DCR4XX_DMASR:
		case DCR4XX_DMACT0:
		case DCR4XX_DMADA0:
		case DCR4XX_DMASA0:
		case DCR4XX_DMACC0:
		case DCR4XX_DMACR0:
		case DCR4XX_DMACT1:
		case DCR4XX_DMADA1:
		case DCR4XX_DMASA1:
		case DCR4XX_DMACC1:
		case DCR4XX_DMACR1:
		case DCR4XX_DMACT2:
		case DCR4XX_DMADA2:
		case DCR4XX_DMASA2:
		case DCR4XX_DMACC2:
		case DCR4XX_DMACR2:
		case DCR4XX_DMACT3:
		case DCR4XX_DMADA3:
		case DCR4XX_DMASA3:
		case DCR4XX_DMACC3:
		case DCR4XX_DMACR3:
		case DCR4XX_EXIER:
		case DCR4XX_EXISR:
		case DCR4XX_IOCR:
			ppc->param1 = ppc->dcr[ppc->param0];
			return;
	}

	/* default handling */
	mame_printf_debug("DCR %03X read\n", ppc->param0);
	if (ppc->param0 < ARRAY_LENGTH(ppc->dcr))
		ppc->param1 = ppc->dcr[ppc->param0];
	else
		ppc->param1 = 0;
}


/*-------------------------------------------------
    ppccom_execute_mtdcr - execute an MTDCR
    instruction
-------------------------------------------------*/

void ppccom_execute_mtdcr(powerpc_state *ppc)
{
	UINT8 oldval;

	/* handle various DCRs */
	switch (ppc->param0)
	{
		/* write-through no-ops */
		case DCR4XX_BR0:
		case DCR4XX_BR1:
		case DCR4XX_BR2:
		case DCR4XX_BR3:
		case DCR4XX_BR4:
		case DCR4XX_BR5:
		case DCR4XX_BR6:
		case DCR4XX_BR7:
		case DCR4XX_BESR:
		case DCR4XX_DMACT0:
		case DCR4XX_DMADA0:
		case DCR4XX_DMASA0:
		case DCR4XX_DMACC0:
		case DCR4XX_DMACT1:
		case DCR4XX_DMADA1:
		case DCR4XX_DMASA1:
		case DCR4XX_DMACC1:
		case DCR4XX_DMACT2:
		case DCR4XX_DMADA2:
		case DCR4XX_DMASA2:
		case DCR4XX_DMACC2:
		case DCR4XX_DMACT3:
		case DCR4XX_DMADA3:
		case DCR4XX_DMASA3:
		case DCR4XX_DMACC3:
			ppc->dcr[ppc->param0] = ppc->param1;
			return;

		/* DMA status */
		case DCR4XX_DMASR:
			ppc->dcr[DCR4XX_DMASR] &= ~(ppc->param1 & 0xfff80070);
			ppc4xx_dma_update_irq_states(ppc);
			return;

		/* interrupt enables */
		case DCR4XX_EXIER:
			ppc->dcr[DCR4XX_EXIER] = ppc->param1;
			ppc4xx_set_irq_line(ppc, 0, 0);
			return;

		/* interrupt clear */
		case DCR4XX_EXISR:
			ppc->dcr[ppc->param0] &= ~ppc->param1;
			ppc4xx_set_irq_line(ppc, 0, 0);
			return;

		/* DMA controls */
		case DCR4XX_DMACR0:
		case DCR4XX_DMACR1:
		case DCR4XX_DMACR2:
		case DCR4XX_DMACR3:
			ppc->dcr[ppc->param0] = ppc->param1;
			if (ppc->param1 & PPC4XX_DMACR_CE)
				ppc4xx_dma_exec(ppc, (ppc->param0 - DCR4XX_DMACR0) / 8);
			ppc4xx_dma_update_irq_states(ppc);
			return;

		/* I/O control */
		case DCR4XX_IOCR:
			oldval = ppc->dcr[ppc->param0];
			ppc->dcr[ppc->param0] = ppc->param1;
			if ((oldval ^ ppc->param1) & 0x02)
				ppc4xx_spu_timer_reset(ppc);
			return;
	}

	/* default handling */
	mame_printf_debug("DCR %03X write = %08X\n", ppc->param0, ppc->param1);
	if (ppc->param0 < ARRAY_LENGTH(ppc->dcr))
		ppc->dcr[ppc->param0] = ppc->param1;
}



/***************************************************************************
    FLOATING POINT STATUS FLAGS HANDLING
***************************************************************************/

/*-------------------------------------------------
    ppccom_update_fprf - update the FPRF field
    of the FPSCR register
-------------------------------------------------*/

void ppccom_update_fprf(powerpc_state *ppc)
{
	UINT32 fprf;
	double f = ppc->f[ppc->param0];

	if (is_qnan_double(f))
	{
		fprf = 0x11;
	}
	else if (is_infinity_double(f))
	{
		if (sign_double(f))		/* -Infinity */
			fprf = 0x09;
		else					/* +Infinity */
			fprf = 0x05;
	}
	else if (is_normalized_double(f))
	{
		if (sign_double(f))		/* -Normalized */
			fprf = 0x08;
		else					/* +Normalized */
			fprf = 0x04;
	}
	else if (is_denormalized_double(f))
	{
		if (sign_double(f))		/* -Denormalized */
			fprf = 0x18;
		else					/* +Denormalized */
			fprf = 0x14;
	}
	else
	{
		if (sign_double(f))		/* -Zero */
			fprf = 0x12;
		else					/* +Zero */
			fprf = 0x02;
	}

	ppc->fpscr &= ~0x0001f000;
	ppc->fpscr |= fprf << 12;
}



/***************************************************************************
    COMMON GET/SET INFO
***************************************************************************/

/*-------------------------------------------------
    ppccom_set_info - set information about
    a PowerPC CPU
-------------------------------------------------*/

void ppccom_set_info(powerpc_state *ppc, UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ:			ppc->irq_pending = (ppc->irq_pending & ~1) | (info->i  != CLEAR_LINE); break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PPC_PC:				ppc->pc = info->i;						break;
		case CPUINFO_INT_REGISTER + PPC_MSR:			ppc->msr = info->i;						break;
		case CPUINFO_INT_REGISTER + PPC_CR:				set_cr(ppc, info->i);					break;
		case CPUINFO_INT_REGISTER + PPC_LR:				ppc->spr[SPR_LR] = info->i;				break;
		case CPUINFO_INT_REGISTER + PPC_CTR:			ppc->spr[SPR_CTR] = info->i;			break;
		case CPUINFO_INT_REGISTER + PPC_XER:			set_xer(ppc, info->i);					break;
		case CPUINFO_INT_REGISTER + PPC_SRR0:			ppc->spr[SPROEA_SRR0] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_SRR1:			ppc->spr[SPROEA_SRR1] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG0:			ppc->spr[SPROEA_SPRG0] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG1:			ppc->spr[SPROEA_SPRG1] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG2:			ppc->spr[SPROEA_SPRG2] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG3:			ppc->spr[SPROEA_SPRG3] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_SDR1:			ppc->spr[SPROEA_SDR1] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_EXIER:			ppc->dcr[DCR4XX_EXIER] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_EXISR:			ppc->dcr[DCR4XX_EXISR] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_EVPR:			ppc->spr[SPR4XX_EVPR] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_IOCR:			ppc->dcr[DCR4XX_IOCR] = info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_TBL:			set_timebase(ppc, (get_timebase(ppc) & ~U64(0x00ffffff00000000)) | info->i); break;
		case CPUINFO_INT_REGISTER + PPC_TBH:			set_timebase(ppc, (get_timebase(ppc) & ~U64(0x00000000ffffffff)) | ((UINT64)(ppc->param1 & 0x00ffffff) << 32)); break;
		case CPUINFO_INT_REGISTER + PPC_DEC:			set_decrementer(ppc, info->i);			break;

		case CPUINFO_INT_REGISTER + PPC_R0:				ppc->r[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R1:				ppc->r[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R2:				ppc->r[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R3:				ppc->r[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R4:				ppc->r[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R5:				ppc->r[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R6:				ppc->r[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R7:				ppc->r[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R8:				ppc->r[8] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R9:				ppc->r[9] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R10:			ppc->r[10] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R11:			ppc->r[11] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R12:			ppc->r[12] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R13:			ppc->r[13] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R14:			ppc->r[14] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R15:			ppc->r[15] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R16:			ppc->r[16] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R17:			ppc->r[17] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R18:			ppc->r[18] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R19:			ppc->r[19] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R20:			ppc->r[20] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R21:			ppc->r[21] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R22:			ppc->r[22] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R23:			ppc->r[23] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R24:			ppc->r[24] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R25:			ppc->r[25] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R26:			ppc->r[26] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R27:			ppc->r[27] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R28:			ppc->r[28] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R29:			ppc->r[29] = info->i;					break;
		case CPUINFO_INT_REGISTER + PPC_R30:			ppc->r[30] = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PPC_R31:			ppc->r[31] = info->i;					break;

		case CPUINFO_INT_REGISTER + PPC_F0:				ppc->f[0] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F1:				ppc->f[1] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F2:				ppc->f[2] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F3:				ppc->f[3] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F4:				ppc->f[4] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F5:				ppc->f[5] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F6:				ppc->f[6] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F7:				ppc->f[7] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F8:				ppc->f[8] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F9:				ppc->f[9] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F10:			ppc->f[10] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F11:			ppc->f[11] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F12:			ppc->f[12] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F13:			ppc->f[13] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F14:			ppc->f[14] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F15:			ppc->f[15] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F16:			ppc->f[16] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F17:			ppc->f[17] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F18:			ppc->f[18] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F19:			ppc->f[19] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F20:			ppc->f[20] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F21:			ppc->f[21] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F22:			ppc->f[22] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F23:			ppc->f[23] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F24:			ppc->f[24] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F25:			ppc->f[25] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F26:			ppc->f[26] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F27:			ppc->f[27] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F28:			ppc->f[28] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F29:			ppc->f[29] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F30:			ppc->f[30] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_F31:			ppc->f[31] = *(double *)&info->i;		break;
		case CPUINFO_INT_REGISTER + PPC_FPSCR:			ppc->fpscr = info->i;					break;
	}
}


/*-------------------------------------------------
    ppccom_get_info - get information about
    a PowerPC CPU
-------------------------------------------------*/

void ppccom_get_info(powerpc_state *ppc, UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					/* provided by core */					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 64;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_PAGE_SHIFT_PROGRAM:	info->i = POWERPC_MIN_PAGE_SHIFT;break;

		case CPUINFO_INT_INPUT_STATE + PPC_IRQ:			info->i = ppc->irq_pending ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					/* optionally implemented */			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PPC_PC:				info->i = ppc->pc;						break;
		case CPUINFO_INT_REGISTER + PPC_MSR:			info->i = ppc->msr;						break;
		case CPUINFO_INT_REGISTER + PPC_CR:				info->i = get_cr(ppc);					break;
		case CPUINFO_INT_REGISTER + PPC_LR:				info->i = ppc->spr[SPR_LR];				break;
		case CPUINFO_INT_REGISTER + PPC_CTR:			info->i = ppc->spr[SPR_CTR];			break;
		case CPUINFO_INT_REGISTER + PPC_XER:			info->i = get_xer(ppc);					break;
		case CPUINFO_INT_REGISTER + PPC_SRR0:			info->i = ppc->spr[SPROEA_SRR0];		break;
		case CPUINFO_INT_REGISTER + PPC_SRR1:			info->i = ppc->spr[SPROEA_SRR1];		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG0:			info->i = ppc->spr[SPROEA_SPRG0];		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG1:			info->i = ppc->spr[SPROEA_SPRG1];		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG2:			info->i = ppc->spr[SPROEA_SPRG2];		break;
		case CPUINFO_INT_REGISTER + PPC_SPRG3:			info->i = ppc->spr[SPROEA_SPRG3];		break;
		case CPUINFO_INT_REGISTER + PPC_SDR1:			info->i = ppc->spr[SPROEA_SDR1];		break;
		case CPUINFO_INT_REGISTER + PPC_EXIER:			info->i = ppc->dcr[DCR4XX_EXIER];		break;
		case CPUINFO_INT_REGISTER + PPC_EXISR:			info->i = ppc->dcr[DCR4XX_EXISR];		break;
		case CPUINFO_INT_REGISTER + PPC_EVPR:			info->i = ppc->spr[SPR4XX_EVPR];		break;
		case CPUINFO_INT_REGISTER + PPC_IOCR:			info->i = ppc->dcr[DCR4XX_IOCR];		break;
		case CPUINFO_INT_REGISTER + PPC_TBH:			info->i = get_timebase(ppc) >> 32;		break;
		case CPUINFO_INT_REGISTER + PPC_TBL:			info->i = (UINT32)get_timebase(ppc);	break;
		case CPUINFO_INT_REGISTER + PPC_DEC:			info->i = get_decrementer(ppc);			break;

		case CPUINFO_INT_REGISTER + PPC_R0:				info->i = ppc->r[0];					break;
		case CPUINFO_INT_REGISTER + PPC_R1:				info->i = ppc->r[1];					break;
		case CPUINFO_INT_REGISTER + PPC_R2:				info->i = ppc->r[2];					break;
		case CPUINFO_INT_REGISTER + PPC_R3:				info->i = ppc->r[3];					break;
		case CPUINFO_INT_REGISTER + PPC_R4:				info->i = ppc->r[4];					break;
		case CPUINFO_INT_REGISTER + PPC_R5:				info->i = ppc->r[5];					break;
		case CPUINFO_INT_REGISTER + PPC_R6:				info->i = ppc->r[6];					break;
		case CPUINFO_INT_REGISTER + PPC_R7:				info->i = ppc->r[7];					break;
		case CPUINFO_INT_REGISTER + PPC_R8:				info->i = ppc->r[8];					break;
		case CPUINFO_INT_REGISTER + PPC_R9:				info->i = ppc->r[9];					break;
		case CPUINFO_INT_REGISTER + PPC_R10:			info->i = ppc->r[10];					break;
		case CPUINFO_INT_REGISTER + PPC_R11:			info->i = ppc->r[11];					break;
		case CPUINFO_INT_REGISTER + PPC_R12:			info->i = ppc->r[12];					break;
		case CPUINFO_INT_REGISTER + PPC_R13:			info->i = ppc->r[13];					break;
		case CPUINFO_INT_REGISTER + PPC_R14:			info->i = ppc->r[14];					break;
		case CPUINFO_INT_REGISTER + PPC_R15:			info->i = ppc->r[15];					break;
		case CPUINFO_INT_REGISTER + PPC_R16:			info->i = ppc->r[16];					break;
		case CPUINFO_INT_REGISTER + PPC_R17:			info->i = ppc->r[17];					break;
		case CPUINFO_INT_REGISTER + PPC_R18:			info->i = ppc->r[18];					break;
		case CPUINFO_INT_REGISTER + PPC_R19:			info->i = ppc->r[19];					break;
		case CPUINFO_INT_REGISTER + PPC_R20:			info->i = ppc->r[20];					break;
		case CPUINFO_INT_REGISTER + PPC_R21:			info->i = ppc->r[21];					break;
		case CPUINFO_INT_REGISTER + PPC_R22:			info->i = ppc->r[22];					break;
		case CPUINFO_INT_REGISTER + PPC_R23:			info->i = ppc->r[23];					break;
		case CPUINFO_INT_REGISTER + PPC_R24:			info->i = ppc->r[24];					break;
		case CPUINFO_INT_REGISTER + PPC_R25:			info->i = ppc->r[25];					break;
		case CPUINFO_INT_REGISTER + PPC_R26:			info->i = ppc->r[26];					break;
		case CPUINFO_INT_REGISTER + PPC_R27:			info->i = ppc->r[27];					break;
		case CPUINFO_INT_REGISTER + PPC_R28:			info->i = ppc->r[28];					break;
		case CPUINFO_INT_REGISTER + PPC_R29:			info->i = ppc->r[29];					break;
		case CPUINFO_INT_REGISTER + PPC_R30:			info->i = ppc->r[30];					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PPC_R31:			info->i = ppc->r[31];					break;

		case CPUINFO_INT_REGISTER + PPC_F0:				info->i = *(UINT64 *)&ppc->f[0];		break;
		case CPUINFO_INT_REGISTER + PPC_F1:				info->i = *(UINT64 *)&ppc->f[1];		break;
		case CPUINFO_INT_REGISTER + PPC_F2:				info->i = *(UINT64 *)&ppc->f[2];		break;
		case CPUINFO_INT_REGISTER + PPC_F3:				info->i = *(UINT64 *)&ppc->f[3];		break;
		case CPUINFO_INT_REGISTER + PPC_F4:				info->i = *(UINT64 *)&ppc->f[4];		break;
		case CPUINFO_INT_REGISTER + PPC_F5:				info->i = *(UINT64 *)&ppc->f[5];		break;
		case CPUINFO_INT_REGISTER + PPC_F6:				info->i = *(UINT64 *)&ppc->f[6];		break;
		case CPUINFO_INT_REGISTER + PPC_F7:				info->i = *(UINT64 *)&ppc->f[7];		break;
		case CPUINFO_INT_REGISTER + PPC_F8:				info->i = *(UINT64 *)&ppc->f[8];		break;
		case CPUINFO_INT_REGISTER + PPC_F9:				info->i = *(UINT64 *)&ppc->f[9];		break;
		case CPUINFO_INT_REGISTER + PPC_F10:			info->i = *(UINT64 *)&ppc->f[10];		break;
		case CPUINFO_INT_REGISTER + PPC_F11:			info->i = *(UINT64 *)&ppc->f[11];		break;
		case CPUINFO_INT_REGISTER + PPC_F12:			info->i = *(UINT64 *)&ppc->f[12];		break;
		case CPUINFO_INT_REGISTER + PPC_F13:			info->i = *(UINT64 *)&ppc->f[13];		break;
		case CPUINFO_INT_REGISTER + PPC_F14:			info->i = *(UINT64 *)&ppc->f[14];		break;
		case CPUINFO_INT_REGISTER + PPC_F15:			info->i = *(UINT64 *)&ppc->f[15];		break;
		case CPUINFO_INT_REGISTER + PPC_F16:			info->i = *(UINT64 *)&ppc->f[16];		break;
		case CPUINFO_INT_REGISTER + PPC_F17:			info->i = *(UINT64 *)&ppc->f[17];		break;
		case CPUINFO_INT_REGISTER + PPC_F18:			info->i = *(UINT64 *)&ppc->f[18];		break;
		case CPUINFO_INT_REGISTER + PPC_F19:			info->i = *(UINT64 *)&ppc->f[19];		break;
		case CPUINFO_INT_REGISTER + PPC_F20:			info->i = *(UINT64 *)&ppc->f[20];		break;
		case CPUINFO_INT_REGISTER + PPC_F21:			info->i = *(UINT64 *)&ppc->f[21];		break;
		case CPUINFO_INT_REGISTER + PPC_F22:			info->i = *(UINT64 *)&ppc->f[22];		break;
		case CPUINFO_INT_REGISTER + PPC_F23:			info->i = *(UINT64 *)&ppc->f[23];		break;
		case CPUINFO_INT_REGISTER + PPC_F24:			info->i = *(UINT64 *)&ppc->f[24];		break;
		case CPUINFO_INT_REGISTER + PPC_F25:			info->i = *(UINT64 *)&ppc->f[25];		break;
		case CPUINFO_INT_REGISTER + PPC_F26:			info->i = *(UINT64 *)&ppc->f[26];		break;
		case CPUINFO_INT_REGISTER + PPC_F27:			info->i = *(UINT64 *)&ppc->f[27];		break;
		case CPUINFO_INT_REGISTER + PPC_F28:			info->i = *(UINT64 *)&ppc->f[28];		break;
		case CPUINFO_INT_REGISTER + PPC_F29:			info->i = *(UINT64 *)&ppc->f[29];		break;
		case CPUINFO_INT_REGISTER + PPC_F30:			info->i = *(UINT64 *)&ppc->f[30];		break;
		case CPUINFO_INT_REGISTER + PPC_F31:			info->i = *(UINT64 *)&ppc->f[31];		break;
		case CPUINFO_INT_REGISTER + PPC_FPSCR:			info->i = ppc->fpscr;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						/* provided by core */					break;
		case CPUINFO_FCT_INIT:							/* provided by core */					break;
		case CPUINFO_FCT_RESET:							/* provided by core */					break;
		case CPUINFO_FCT_EXIT:							/* provided by core */					break;
		case CPUINFO_FCT_EXECUTE:						/* provided by core */					break;
		case CPUINFO_FCT_TRANSLATE:						/* provided by core */					break;
		case CPUINFO_FCT_DISASSEMBLE:					/* provided by core */					break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &ppc->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "PowerPC");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "PowerPC");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "2.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						/* provided by core */					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Aaron Giles"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + PPC_PC:				sprintf(info->s, "PC: %08X", ppc->pc);				break;
		case CPUINFO_STR_REGISTER + PPC_MSR:			sprintf(info->s, "MSR:%08X", ppc->msr);				break;
		case CPUINFO_STR_REGISTER + PPC_CR:				sprintf(info->s, "CR: %08X", get_cr(ppc));			break;
		case CPUINFO_STR_REGISTER + PPC_LR:				sprintf(info->s, "LR: %08X", ppc->spr[SPR_LR]);		break;
		case CPUINFO_STR_REGISTER + PPC_CTR:			sprintf(info->s, "CTR:%08X", ppc->spr[SPR_CTR]);	break;
		case CPUINFO_STR_REGISTER + PPC_XER:			sprintf(info->s, "XER:%08X", get_xer(ppc));			break;
		case CPUINFO_STR_REGISTER + PPC_SRR0:			sprintf(info->s, "SRR0: %08X", ppc->spr[SPROEA_SRR0]);	break;
		case CPUINFO_STR_REGISTER + PPC_SRR1:			sprintf(info->s, "SRR1: %08X", ppc->spr[SPROEA_SRR1]);	break;
		case CPUINFO_STR_REGISTER + PPC_SPRG0:			sprintf(info->s, "SPRG0: %08X", ppc->spr[SPROEA_SPRG0]); break;
		case CPUINFO_STR_REGISTER + PPC_SPRG1:			sprintf(info->s, "SPRG1: %08X", ppc->spr[SPROEA_SPRG1]); break;
		case CPUINFO_STR_REGISTER + PPC_SPRG2:			sprintf(info->s, "SPRG2: %08X", ppc->spr[SPROEA_SPRG2]); break;
		case CPUINFO_STR_REGISTER + PPC_SPRG3:			sprintf(info->s, "SPRG3: %08X", ppc->spr[SPROEA_SPRG3]); break;
		case CPUINFO_STR_REGISTER + PPC_SDR1:			sprintf(info->s, "SDR1: %08X", ppc->spr[SPROEA_SDR1]); break;
		case CPUINFO_STR_REGISTER + PPC_EXIER:			sprintf(info->s, "EXIER: %08X", ppc->dcr[DCR4XX_EXIER]); break;
		case CPUINFO_STR_REGISTER + PPC_EXISR:			sprintf(info->s, "EXISR: %08X", ppc->dcr[DCR4XX_EXISR]); break;
		case CPUINFO_STR_REGISTER + PPC_EVPR:			sprintf(info->s, "EVPR: %08X", ppc->spr[SPR4XX_EVPR]); break;
		case CPUINFO_STR_REGISTER + PPC_IOCR:			sprintf(info->s, "IOCR: %08X", ppc->dcr[DCR4XX_EXISR]); break;
		case CPUINFO_STR_REGISTER + PPC_TBH:			sprintf(info->s, "TBH: %08X", (UINT32)(get_timebase(ppc) >> 32)); break;
		case CPUINFO_STR_REGISTER + PPC_TBL:			sprintf(info->s, "TBL: %08X", (UINT32)get_timebase(ppc)); break;
		case CPUINFO_STR_REGISTER + PPC_DEC:			sprintf(info->s, "DEC: %08X", get_decrementer(ppc)); break;

		case CPUINFO_STR_REGISTER + PPC_R0:				sprintf(info->s, "R0: %08X", ppc->r[0]); break;
		case CPUINFO_STR_REGISTER + PPC_R1:				sprintf(info->s, "R1: %08X", ppc->r[1]); break;
		case CPUINFO_STR_REGISTER + PPC_R2:				sprintf(info->s, "R2: %08X", ppc->r[2]); break;
		case CPUINFO_STR_REGISTER + PPC_R3:				sprintf(info->s, "R3: %08X", ppc->r[3]); break;
		case CPUINFO_STR_REGISTER + PPC_R4:				sprintf(info->s, "R4: %08X", ppc->r[4]); break;
		case CPUINFO_STR_REGISTER + PPC_R5:				sprintf(info->s, "R5: %08X", ppc->r[5]); break;
		case CPUINFO_STR_REGISTER + PPC_R6:				sprintf(info->s, "R6: %08X", ppc->r[6]); break;
		case CPUINFO_STR_REGISTER + PPC_R7:				sprintf(info->s, "R7: %08X", ppc->r[7]); break;
		case CPUINFO_STR_REGISTER + PPC_R8:				sprintf(info->s, "R8: %08X", ppc->r[8]); break;
		case CPUINFO_STR_REGISTER + PPC_R9:				sprintf(info->s, "R9: %08X", ppc->r[9]); break;
		case CPUINFO_STR_REGISTER + PPC_R10:			sprintf(info->s, "R10:%08X", ppc->r[10]); break;
		case CPUINFO_STR_REGISTER + PPC_R11:			sprintf(info->s, "R11:%08X", ppc->r[11]); break;
		case CPUINFO_STR_REGISTER + PPC_R12:			sprintf(info->s, "R12:%08X", ppc->r[12]); break;
		case CPUINFO_STR_REGISTER + PPC_R13:			sprintf(info->s, "R13:%08X", ppc->r[13]); break;
		case CPUINFO_STR_REGISTER + PPC_R14:			sprintf(info->s, "R14:%08X", ppc->r[14]); break;
		case CPUINFO_STR_REGISTER + PPC_R15:			sprintf(info->s, "R15:%08X", ppc->r[15]); break;
		case CPUINFO_STR_REGISTER + PPC_R16:			sprintf(info->s, "R16:%08X", ppc->r[16]); break;
		case CPUINFO_STR_REGISTER + PPC_R17:			sprintf(info->s, "R17:%08X", ppc->r[17]); break;
		case CPUINFO_STR_REGISTER + PPC_R18:			sprintf(info->s, "R18:%08X", ppc->r[18]); break;
		case CPUINFO_STR_REGISTER + PPC_R19:			sprintf(info->s, "R19:%08X", ppc->r[19]); break;
		case CPUINFO_STR_REGISTER + PPC_R20:			sprintf(info->s, "R20:%08X", ppc->r[20]); break;
		case CPUINFO_STR_REGISTER + PPC_R21:			sprintf(info->s, "R21:%08X", ppc->r[21]); break;
		case CPUINFO_STR_REGISTER + PPC_R22:			sprintf(info->s, "R22:%08X", ppc->r[22]); break;
		case CPUINFO_STR_REGISTER + PPC_R23:			sprintf(info->s, "R23:%08X", ppc->r[23]); break;
		case CPUINFO_STR_REGISTER + PPC_R24:			sprintf(info->s, "R24:%08X", ppc->r[24]); break;
		case CPUINFO_STR_REGISTER + PPC_R25:			sprintf(info->s, "R25:%08X", ppc->r[25]); break;
		case CPUINFO_STR_REGISTER + PPC_R26:			sprintf(info->s, "R26:%08X", ppc->r[26]); break;
		case CPUINFO_STR_REGISTER + PPC_R27:			sprintf(info->s, "R27:%08X", ppc->r[27]); break;
		case CPUINFO_STR_REGISTER + PPC_R28:			sprintf(info->s, "R28:%08X", ppc->r[28]); break;
		case CPUINFO_STR_REGISTER + PPC_R29:			sprintf(info->s, "R29:%08X", ppc->r[29]); break;
		case CPUINFO_STR_REGISTER + PPC_R30:			sprintf(info->s, "R30:%08X", ppc->r[30]); break;
		case CPUINFO_STR_REGISTER + PPC_R31:			sprintf(info->s, "R31:%08X", ppc->r[31]); break;

		case CPUINFO_STR_REGISTER + PPC_F0:				sprintf(info->s, "F0: %12f", ppc->f[0]); break;
		case CPUINFO_STR_REGISTER + PPC_F1:				sprintf(info->s, "F1: %12f", ppc->f[1]); break;
		case CPUINFO_STR_REGISTER + PPC_F2:				sprintf(info->s, "F2: %12f", ppc->f[2]); break;
		case CPUINFO_STR_REGISTER + PPC_F3:				sprintf(info->s, "F3: %12f", ppc->f[3]); break;
		case CPUINFO_STR_REGISTER + PPC_F4:				sprintf(info->s, "F4: %12f", ppc->f[4]); break;
		case CPUINFO_STR_REGISTER + PPC_F5:				sprintf(info->s, "F5: %12f", ppc->f[5]); break;
		case CPUINFO_STR_REGISTER + PPC_F6:				sprintf(info->s, "F6: %12f", ppc->f[6]); break;
		case CPUINFO_STR_REGISTER + PPC_F7:				sprintf(info->s, "F7: %12f", ppc->f[7]); break;
		case CPUINFO_STR_REGISTER + PPC_F8:				sprintf(info->s, "F8: %12f", ppc->f[8]); break;
		case CPUINFO_STR_REGISTER + PPC_F9:				sprintf(info->s, "F9: %12f", ppc->f[9]); break;
		case CPUINFO_STR_REGISTER + PPC_F10:			sprintf(info->s, "F10:%12f", ppc->f[10]); break;
		case CPUINFO_STR_REGISTER + PPC_F11:			sprintf(info->s, "F11:%12f", ppc->f[11]); break;
		case CPUINFO_STR_REGISTER + PPC_F12:			sprintf(info->s, "F12:%12f", ppc->f[12]); break;
		case CPUINFO_STR_REGISTER + PPC_F13:			sprintf(info->s, "F13:%12f", ppc->f[13]); break;
		case CPUINFO_STR_REGISTER + PPC_F14:			sprintf(info->s, "F14:%12f", ppc->f[14]); break;
		case CPUINFO_STR_REGISTER + PPC_F15:			sprintf(info->s, "F15:%12f", ppc->f[15]); break;
		case CPUINFO_STR_REGISTER + PPC_F16:			sprintf(info->s, "F16:%12f", ppc->f[16]); break;
		case CPUINFO_STR_REGISTER + PPC_F17:			sprintf(info->s, "F17:%12f", ppc->f[17]); break;
		case CPUINFO_STR_REGISTER + PPC_F18:			sprintf(info->s, "F18:%12f", ppc->f[18]); break;
		case CPUINFO_STR_REGISTER + PPC_F19:			sprintf(info->s, "F19:%12f", ppc->f[19]); break;
		case CPUINFO_STR_REGISTER + PPC_F20:			sprintf(info->s, "F20:%12f", ppc->f[20]); break;
		case CPUINFO_STR_REGISTER + PPC_F21:			sprintf(info->s, "F21:%12f", ppc->f[21]); break;
		case CPUINFO_STR_REGISTER + PPC_F22:			sprintf(info->s, "F22:%12f", ppc->f[22]); break;
		case CPUINFO_STR_REGISTER + PPC_F23:			sprintf(info->s, "F23:%12f", ppc->f[23]); break;
		case CPUINFO_STR_REGISTER + PPC_F24:			sprintf(info->s, "F24:%12f", ppc->f[24]); break;
		case CPUINFO_STR_REGISTER + PPC_F25:			sprintf(info->s, "F25:%12f", ppc->f[25]); break;
		case CPUINFO_STR_REGISTER + PPC_F26:			sprintf(info->s, "F26:%12f", ppc->f[26]); break;
		case CPUINFO_STR_REGISTER + PPC_F27:			sprintf(info->s, "F27:%12f", ppc->f[27]); break;
		case CPUINFO_STR_REGISTER + PPC_F28:			sprintf(info->s, "F28:%12f", ppc->f[28]); break;
		case CPUINFO_STR_REGISTER + PPC_F29:			sprintf(info->s, "F29:%12f", ppc->f[29]); break;
		case CPUINFO_STR_REGISTER + PPC_F30:			sprintf(info->s, "F30:%12f", ppc->f[30]); break;
		case CPUINFO_STR_REGISTER + PPC_F31:			sprintf(info->s, "F31:%12f", ppc->f[31]); break;
		case CPUINFO_STR_REGISTER + PPC_FPSCR:			sprintf(info->s, "FPSCR:%08X", ppc->fpscr); break;
	}
}



/***************************************************************************
    OEA HELPERS
***************************************************************************/

/*-------------------------------------------------
    decrementer_int_callback - callback that fires
    whenever a decrementer interrupt is generated
-------------------------------------------------*/

static TIMER_CALLBACK( decrementer_int_callback )
{
	powerpc_state *ppc = (powerpc_state *)ptr;
	UINT64 cycles_until_next;

	/* set the decrementer IRQ state */
	ppc->irq_pending |= 0x02;

	/* advance by another full rev */
	ppc->dec_zero_cycles += (UINT64)ppc->tb_divisor << 32;
	cycles_until_next = ppc->dec_zero_cycles - ppc->device->total_cycles();
	ppc->decrementer_int_timer->adjust(ppc->device->cycles_to_attotime(cycles_until_next));
}



/***************************************************************************
    EMBEDDED 4XX HELPERS
***************************************************************************/

/*-------------------------------------------------
    ppc4xx_set_irq_line - PowerPC 4XX-specific
    IRQ line management
-------------------------------------------------*/

static void ppc4xx_set_irq_line(powerpc_state *ppc, UINT32 bitmask, int state)
{
	UINT32 oldstate = ppc->irqstate;
	UINT32 levelmask;

	/* set or clear the appropriate bit */
	if (state != CLEAR_LINE)
		ppc->irqstate |= bitmask;
	else
		ppc->irqstate &= ~bitmask;

	/* if the state changed to on, edge trigger the interrupt */
	if (((ppc->irqstate ^ oldstate) & bitmask) && (ppc->irqstate & bitmask))
		ppc->dcr[DCR4XX_EXISR] |= bitmask;

	/* pass through all level-triggered interrupts */
	levelmask = PPC4XX_IRQ_BIT_CRITICAL | PPC4XX_IRQ_BIT_SPUR | PPC4XX_IRQ_BIT_SPUT;
	levelmask |= PPC4XX_IRQ_BIT_JTAGR | PPC4XX_IRQ_BIT_JTAGT;
	levelmask |= PPC4XX_IRQ_BIT_DMA0 | PPC4XX_IRQ_BIT_DMA1 | PPC4XX_IRQ_BIT_DMA2 | PPC4XX_IRQ_BIT_DMA3;
	if (!(ppc->dcr[DCR4XX_IOCR] & 0x80000000)) levelmask |= PPC4XX_IRQ_BIT_EXT0;
	if (!(ppc->dcr[DCR4XX_IOCR] & 0x20000000)) levelmask |= PPC4XX_IRQ_BIT_EXT1;
	if (!(ppc->dcr[DCR4XX_IOCR] & 0x08000000)) levelmask |= PPC4XX_IRQ_BIT_EXT2;
	if (!(ppc->dcr[DCR4XX_IOCR] & 0x02000000)) levelmask |= PPC4XX_IRQ_BIT_EXT3;
	if (!(ppc->dcr[DCR4XX_IOCR] & 0x00800000)) levelmask |= PPC4XX_IRQ_BIT_EXT4;
	ppc->dcr[DCR4XX_EXISR] = (ppc->dcr[DCR4XX_EXISR] & ~levelmask) | (ppc->irqstate & levelmask);

	/* update the IRQ status */
	ppc->irq_pending = ((ppc->dcr[DCR4XX_EXISR] & ppc->dcr[DCR4XX_EXIER]) != 0);
	if ((ppc->spr[SPR4XX_TCR] & PPC4XX_TCR_FIE) && (ppc->spr[SPR4XX_TSR] & PPC4XX_TSR_FIS))
		ppc->irq_pending = TRUE;
	if ((ppc->spr[SPR4XX_TCR] & PPC4XX_TCR_PIE) && (ppc->spr[SPR4XX_TSR] & PPC4XX_TSR_PIS))
		ppc->irq_pending = TRUE;
}


/*-------------------------------------------------
    ppc4xx_get_irq_line - PowerPC 4XX-specific
    IRQ line state getter
-------------------------------------------------*/

static int ppc4xx_get_irq_line(powerpc_state *ppc, UINT32 bitmask)
{
	return (ppc->irqstate & bitmask) ? ASSERT_LINE : CLEAR_LINE;
}


/*-------------------------------------------------
    ppc4xx_dma_update_irq_states - update the IRQ
    state for each DMA channel
-------------------------------------------------*/

static void ppc4xx_dma_update_irq_states(powerpc_state *ppc)
{
	int dmachan;

	/* update the IRQ state for each DMA channel */
	for (dmachan = 0; dmachan < 4; dmachan++)
		if ((ppc->dcr[DCR4XX_DMACR0 + 8 * dmachan] & PPC4XX_DMACR_CIE) && (ppc->dcr[DCR4XX_DMASR] & (0x11 << (27 - dmachan))))
			ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_DMA(dmachan), ASSERT_LINE);
		else
			ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_DMA(dmachan), CLEAR_LINE);
}


/*-------------------------------------------------
    ppc4xx_dma_decrement_count - decrement the
    count on a channel and interrupt if configured
    to do so
-------------------------------------------------*/

static int ppc4xx_dma_decrement_count(powerpc_state *ppc, int dmachan)
{
	UINT32 *dmaregs = &ppc->dcr[8 * dmachan];

	/* decrement the counter */
	dmaregs[DCR4XX_DMACT0]--;

	/* if non-zero, we keep going */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) != 0)
		return FALSE;

	/* set the complete bit and handle interrupts */
	ppc->dcr[DCR4XX_DMASR] |= 1 << (31 - dmachan);
//  ppc->dcr[DCR4XX_DMASR] |= 1 << (27 - dmachan);
	ppc4xx_dma_update_irq_states(ppc);
	return TRUE;
}


/*-------------------------------------------------
    ppc4xx_dma_fetch_transmit_byte - fetch a byte
    to send to a peripheral
-------------------------------------------------*/

static int ppc4xx_dma_fetch_transmit_byte(powerpc_state *ppc, int dmachan, UINT8 *byte)
{
	UINT32 *dmaregs = &ppc->dcr[8 * dmachan];

	/* if the channel is not enabled, fail */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return FALSE;

	/* if no transfers remaining, fail */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) == 0)
		return FALSE;

	/* fetch the data */
	*byte = ppc->program->read_byte(dmaregs[DCR4XX_DMADA0]++);
	ppc4xx_dma_decrement_count(ppc, dmachan);
	return TRUE;
}


/*-------------------------------------------------
    ppc4xx_dma_handle_receive_byte - receive a byte
    transmitted by a peripheral
-------------------------------------------------*/

static int ppc4xx_dma_handle_receive_byte(powerpc_state *ppc, int dmachan, UINT8 byte)
{
	UINT32 *dmaregs = &ppc->dcr[8 * dmachan];

	/* if the channel is not enabled, fail */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return FALSE;

	/* if no transfers remaining, fail */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) == 0)
		return FALSE;

	/* store the data */
	ppc->program->write_byte(dmaregs[DCR4XX_DMADA0]++, byte);
	ppc4xx_dma_decrement_count(ppc, dmachan);
	return TRUE;
}


/*-------------------------------------------------
    ppc4xx_dma_execute - execute a DMA operation
    if one is pending
-------------------------------------------------*/

static void ppc4xx_dma_exec(powerpc_state *ppc, int dmachan)
{
	static const UINT8 dma_transfer_width[4] = { 1, 2, 4, 16 };
	UINT32 *dmaregs = &ppc->dcr[8 * dmachan];
	INT32 destinc, srcinc;
	UINT8 width;

	/* skip if not enabled */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return;

	/* check for unsupported features */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_TCE))
		fatalerror("ppc4xx_dma_exec: DMA_TCE == 0");
	if (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CH)
		fatalerror("ppc4xx_dma_exec: DMA chaining not implemented");

	/* transfer mode */
	switch ((dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_TM_MASK) >> 21)
	{
		/* buffered mode DMA */
		case 0:
			/* nothing to do; this happens asynchronously and is driven by the SPU */
			break;

		/* fly-by mode DMA */
		case 1:
			fatalerror("ppc4xx_dma_exec: fly-by DMA not implemented");
			break;

		/* software initiated memory-to-memory mode DMA */
		case 2:
			width = dma_transfer_width[(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_PW_MASK) >> 26];
			srcinc = (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_SAI) ? width : 0;
			destinc = (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_DAI) ? width : 0;

			switch (width)
			{
				/* byte transfer */
				case 1:
					do
					{
						ppc->program->write_byte(dmaregs[DCR4XX_DMADA0], ppc->program->read_byte(dmaregs[DCR4XX_DMASA0]));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(ppc, dmachan));
					break;

				/* word transfer */
				case 2:
					do
					{
						ppc->program->write_word(dmaregs[DCR4XX_DMADA0], ppc->program->read_word(dmaregs[DCR4XX_DMASA0]));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(ppc, dmachan));
					break;

				/* dword transfer */
				case 4:
					do
					{
						ppc->program->write_dword(dmaregs[DCR4XX_DMADA0], ppc->program->read_dword(dmaregs[DCR4XX_DMASA0]));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(ppc, dmachan));
					break;

				/* 16-byte transfer */
				case 16:
					do
					{
						ppc->program->write_qword(dmaregs[DCR4XX_DMADA0], ppc->program->read_qword(dmaregs[DCR4XX_DMASA0]));
						ppc->program->write_qword(dmaregs[DCR4XX_DMADA0] + 8, ppc->program->read_qword(dmaregs[DCR4XX_DMASA0] + 8));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(ppc, dmachan));
					break;
			}
			break;

		/* hardware initiated memory-to-memory mode DMA */
		case 3:
			fatalerror("ppc4xx_dma_exec: HW mem-to-mem DMA not implemented");
			break;
	}
}


/*-------------------------------------------------
    ppc4xx_fit_callback - FIT timer callback
-------------------------------------------------*/

static TIMER_CALLBACK( ppc4xx_fit_callback )
{
	powerpc_state *ppc = (powerpc_state *)ptr;

	/* if this is a real callback and we are enabled, signal an interrupt */
	if (param)
	{
		ppc->spr[SPR4XX_TSR] |= PPC4XX_TSR_FIS;
		ppc4xx_set_irq_line(ppc, 0, 0);
	}

	/* update ourself for the next interval if we are enabled */
	if (ppc->spr[SPR4XX_TCR] & PPC4XX_TCR_FIE)
	{
		UINT32 timebase = get_timebase(ppc);
		UINT32 interval = 0x200 << (4 * ((ppc->spr[SPR4XX_TCR] & PPC4XX_TCR_FP_MASK) >> 24));
		UINT32 target = (timebase + interval) & ~(interval - 1);
		ppc->fit_timer->adjust(ppc->device->cycles_to_attotime((target + 1 - timebase) / ppc->tb_divisor), TRUE);
	}

	/* otherwise, turn ourself off */
	else
		ppc->fit_timer->adjust(attotime::never, FALSE);
}


/*-------------------------------------------------
    ppc4xx_pit_callback - PIT timer callback
-------------------------------------------------*/

static TIMER_CALLBACK( ppc4xx_pit_callback )
{
	powerpc_state *ppc = (powerpc_state *)ptr;

	/* if this is a real callback and we are enabled, signal an interrupt */
	if (param)
	{
		ppc->spr[SPR4XX_TSR] |= PPC4XX_TSR_PIS;
		ppc4xx_set_irq_line(ppc, 0, 0);
	}

	/* update ourself for the next interval if we are enabled and we are either being
       forced to update, or we are in auto-reload mode */
	if ((ppc->spr[SPR4XX_TCR] & PPC4XX_TCR_PIE) && ppc->pit_reload != 0 && (!param || (ppc->spr[SPR4XX_TCR] & PPC4XX_TCR_ARE)))
	{
		UINT32 timebase = get_timebase(ppc);
		UINT32 interval = ppc->pit_reload;
		UINT32 target = timebase + interval;
		ppc->pit_timer->adjust(ppc->device->cycles_to_attotime((target + 1 - timebase) / ppc->tb_divisor), TRUE);
	}

	/* otherwise, turn ourself off */
	else
		ppc->pit_timer->adjust(attotime::never, FALSE);
}


/*-------------------------------------------------
    ppc4xx_spu_update_irq_states - update the IRQ
    state for the SPU
-------------------------------------------------*/

static void ppc4xx_spu_update_irq_states(powerpc_state *ppc)
{
	/* check for receive buffer full interrupt */
	if ((ppc->spu.regs[SPU4XX_RX_COMMAND] & 0x60) == 0x20 && (ppc->spu.regs[SPU4XX_LINE_STATUS] & 0x80))
		ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_SPUR, ASSERT_LINE);

	/* check for receive error interrupt */
	else if ((ppc->spu.regs[SPU4XX_RX_COMMAND] & 0x10) && (ppc->spu.regs[SPU4XX_LINE_STATUS] & 0x78))
		ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_SPUR, ASSERT_LINE);

	/* clear otherwise */
	else
		ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_SPUR, CLEAR_LINE);

	/* check for transmit buffer empty interrupt */
	if ((ppc->spu.regs[SPU4XX_TX_COMMAND] & 0x60) == 0x20 && (ppc->spu.regs[SPU4XX_LINE_STATUS] & 0x04))
		ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_SPUT, ASSERT_LINE);

	/* check for shift register empty interrupt */
	else if ((ppc->spu.regs[SPU4XX_TX_COMMAND] & 0x10) && (ppc->spu.regs[SPU4XX_LINE_STATUS] & 0x02))
		ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_SPUT, ASSERT_LINE);

	/* clear otherwise */
	else
		ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_SPUT, CLEAR_LINE);
}


/*-------------------------------------------------
    ppc4xx_spu_rx_data - serial port data receive
-------------------------------------------------*/

static void ppc4xx_spu_rx_data(powerpc_state *ppc, UINT8 data)
{
	UINT32 new_rxin;

	/* fail if we are going to overflow */
	new_rxin = (ppc->spu.rxin + 1) % ARRAY_LENGTH(ppc->spu.rxbuffer);
	if (new_rxin == ppc->spu.rxout)
		fatalerror("ppc4xx_spu_rx_data: buffer overrun!");

	/* store the data and accept the new in index */
	ppc->spu.rxbuffer[ppc->spu.rxin] = data;
	ppc->spu.rxin = new_rxin;
}


/*-------------------------------------------------
    ppc4xx_spu_timer_reset - reset and recompute
    the transmit/receive timer
-------------------------------------------------*/

static void ppc4xx_spu_timer_reset(powerpc_state *ppc)
{
	UINT8 enabled = (ppc->spu.regs[SPU4XX_RX_COMMAND] | ppc->spu.regs[SPU4XX_TX_COMMAND]) & 0x80;

	/* if we're enabled, reset at the current baud rate */
	if (enabled)
	{
		attotime clockperiod = attotime::from_hz((ppc->dcr[DCR4XX_IOCR] & 0x02) ? 3686400 : 33333333);
		int divisor = ((ppc->spu.regs[SPU4XX_BAUD_DIVISOR_H] * 256 + ppc->spu.regs[SPU4XX_BAUD_DIVISOR_L]) & 0xfff) + 1;
		int bpc = 7 + ((ppc->spu.regs[SPU4XX_CONTROL] & 8) >> 3) + 1 + (ppc->spu.regs[SPU4XX_CONTROL] & 1);
		attotime charperiod = clockperiod * (divisor * 16 * bpc);
		ppc->spu.timer->adjust(charperiod, 0, charperiod);
		if (PRINTF_SPU)
			printf("ppc4xx_spu_timer_reset: baud rate = %.0f\n", ATTOSECONDS_TO_HZ(charperiod.attoseconds) * bpc);
	}

	/* otherwise, disable the timer */
	else
		ppc->spu.timer->adjust(attotime::never);
}


/*-------------------------------------------------
    ppc4xx_spu_callback - serial port send/receive
    timer
-------------------------------------------------*/

static TIMER_CALLBACK( ppc4xx_spu_callback )
{
	powerpc_state *ppc = (powerpc_state *)ptr;

	/* transmit enabled? */
	if (ppc->spu.regs[SPU4XX_TX_COMMAND] & 0x80)
	{
		int operation = (ppc->spu.regs[SPU4XX_TX_COMMAND] >> 5) & 3;

		/* if we have data to transmit, do it now */
		if (!(ppc->spu.regs[SPU4XX_LINE_STATUS] & 0x04))
		{
			/* if we have a transmit handler, send it that way */
			if (ppc->spu.tx_handler != NULL)
				(*ppc->spu.tx_handler)(ppc->device, ppc->spu.txbuf);

			/* indicate that we have moved it to the shift register */
			ppc->spu.regs[SPU4XX_LINE_STATUS] |= 0x04;
			ppc->spu.regs[SPU4XX_LINE_STATUS] &= ~0x02;
		}

		/* otherwise, clear the shift register */
		else if (!(ppc->spu.regs[SPU4XX_LINE_STATUS] & 0x02))
			ppc->spu.regs[SPU4XX_LINE_STATUS] |= 0x02;

		/* handle DMA */
		if (operation >= 2 && ppc4xx_dma_fetch_transmit_byte(ppc, operation, &ppc->spu.txbuf))
			ppc->spu.regs[SPU4XX_LINE_STATUS] &= ~0x04;
	}

	/* receive enabled? */
	if (ppc->spu.regs[SPU4XX_RX_COMMAND] & 0x80)
		if (ppc->spu.rxout != ppc->spu.rxin)
		{
			int operation = (ppc->spu.regs[SPU4XX_RX_COMMAND] >> 5) & 3;
			UINT8 rxbyte;

			/* consume the byte and advance the out pointer */
			rxbyte = ppc->spu.rxbuffer[ppc->spu.rxout];
			ppc->spu.rxout = (ppc->spu.rxout + 1) % ARRAY_LENGTH(ppc->spu.rxbuffer);

			/* if we're not full, copy data to the buffer and update the line status */
			if (!(ppc->spu.regs[SPU4XX_LINE_STATUS] & 0x80))
			{
				ppc->spu.rxbuf = rxbyte;
				ppc->spu.regs[SPU4XX_LINE_STATUS] |= 0x80;
			}

			/* otherwise signal an overrun */
			else
			{
				ppc->spu.regs[SPU4XX_LINE_STATUS] |= 0x20;
				goto updateirq;
			}

			/* handle DMA */
			if (operation >= 2 && ppc4xx_dma_handle_receive_byte(ppc, operation, ppc->spu.rxbuf))
				ppc->spu.regs[SPU4XX_LINE_STATUS] &= ~0x80;
		}

	/* update the final IRQ states */
updateirq:
	ppc4xx_spu_update_irq_states(ppc);
}


/*-------------------------------------------------
    ppc4xx_spu_r - serial port read handler
-------------------------------------------------*/

static READ8_HANDLER( ppc4xx_spu_r )
{
	powerpc_state *ppc = *(powerpc_state **)downcast<legacy_cpu_device *>(&space->device())->token();
	UINT8 result = 0xff;

	switch (offset)
	{
		case SPU4XX_BUFFER:
			result = ppc->spu.rxbuf;
			ppc->spu.regs[SPU4XX_LINE_STATUS] &= ~0x80;
			break;

		default:
			if (offset < ARRAY_LENGTH(ppc->spu.regs))
				result = ppc->spu.regs[offset];
			break;
	}
	if (PRINTF_SPU)
		printf("spu_r(%d) = %02X\n", offset, result);
	return result;
}


/*-------------------------------------------------
    ppc4xx_spu_w - serial port write handler
-------------------------------------------------*/

static WRITE8_HANDLER( ppc4xx_spu_w )
{
	powerpc_state *ppc = *(powerpc_state **)downcast<legacy_cpu_device *>(&space->device())->token();
	UINT8 oldstate, newstate;

	if (PRINTF_SPU)
		printf("spu_w(%d) = %02X\n", offset, data);
	switch (offset)
	{
		/* clear error bits */
		case SPU4XX_LINE_STATUS:
			ppc->spu.regs[SPU4XX_LINE_STATUS] &= ~(data & 0xf8);
			ppc4xx_spu_update_irq_states(ppc);
			break;

		/* enable/disable the timer if one of these is enabled */
		case SPU4XX_RX_COMMAND:
		case SPU4XX_TX_COMMAND:
			oldstate = ppc->spu.regs[SPU4XX_RX_COMMAND] | ppc->spu.regs[SPU4XX_TX_COMMAND];
			ppc->spu.regs[offset] = data;
			newstate = ppc->spu.regs[SPU4XX_RX_COMMAND] | ppc->spu.regs[SPU4XX_TX_COMMAND];
			if ((oldstate ^ newstate) & 0x80)
				ppc4xx_spu_timer_reset(ppc);
			ppc4xx_spu_update_irq_states(ppc);
			break;

		/* if the divisor changes, we need to update the timer */
		case SPU4XX_BAUD_DIVISOR_H:
		case SPU4XX_BAUD_DIVISOR_L:
			if (data != ppc->spu.regs[offset])
			{
				ppc->spu.regs[offset] = data;
				ppc4xx_spu_timer_reset(ppc);
			}
			break;

		/* if the number of data bits or stop bits changes, we need to update the timer */
		case SPU4XX_CONTROL:
			oldstate = ppc->spu.regs[offset];
			ppc->spu.regs[offset] = data;
			if ((oldstate ^ data) & 0x09)
				ppc4xx_spu_timer_reset(ppc);
			break;

			break;

		case SPU4XX_BUFFER:
			/* write to the transmit buffer and mark it full */
			ppc->spu.txbuf = data;
			ppc->spu.regs[SPU4XX_LINE_STATUS] &= ~0x04;
			break;

		default:
			if (offset < ARRAY_LENGTH(ppc->spu.regs))
				ppc->spu.regs[offset] = data;
			break;
	}
}



/*-------------------------------------------------
    internal_ppc4xx - internal address map for
    the 4XX
-------------------------------------------------*/

static ADDRESS_MAP_START( internal_ppc4xx, AS_PROGRAM, 32, legacy_cpu_device )
	AM_RANGE(0x40000000, 0x4000000f) AM_READWRITE8_LEGACY(ppc4xx_spu_r, ppc4xx_spu_w, 0xffffffff)
ADDRESS_MAP_END



/*-------------------------------------------------
    ppc4xx_spu_set_tx_handler - PowerPC 4XX-
    specific TX handler configuration
-------------------------------------------------*/

void ppc4xx_spu_set_tx_handler(device_t *device, ppc4xx_spu_tx_handler handler)
{
	powerpc_state *ppc = *(powerpc_state **)downcast<legacy_cpu_device *>(device)->token();
	ppc->spu.tx_handler = handler;
}


/*-------------------------------------------------
    ppc4xx_spu_receive_byte - PowerPC 4XX-
    specific serial byte receive
-------------------------------------------------*/

void ppc4xx_spu_receive_byte(device_t *device, UINT8 byteval)
{
	powerpc_state *ppc = *(powerpc_state **)downcast<legacy_cpu_device *>(device)->token();
	ppc4xx_spu_rx_data(ppc, byteval);
}


/*-------------------------------------------------
    ppc4xx_set_info - PowerPC 4XX-specific
    information setter
-------------------------------------------------*/

void ppc4xx_set_info(powerpc_state *ppc, UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_0:	ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_EXT0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_1:	ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_EXT1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_2:	ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_EXT2, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_3:	ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_EXT3, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_4:	ppc4xx_set_irq_line(ppc, PPC4XX_IRQ_BIT_EXT4, info->i);		break;

		/* --- everything else is handled generically --- */
		default:										ppccom_set_info(ppc, state, info);		break;
	}
}


/*-------------------------------------------------
    ppc4xx_get_info - PowerPC 4XX-specific
    information getter
-------------------------------------------------*/

void ppc4xx_get_info(powerpc_state *ppc, UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 5;							break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_0:	info->i = ppc4xx_get_irq_line(ppc, PPC4XX_IRQ_BIT_EXT0);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_1:	info->i = ppc4xx_get_irq_line(ppc, PPC4XX_IRQ_BIT_EXT1);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_2:	info->i = ppc4xx_get_irq_line(ppc, PPC4XX_IRQ_BIT_EXT2);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_3:	info->i = ppc4xx_get_irq_line(ppc, PPC4XX_IRQ_BIT_EXT3);		break;
		case CPUINFO_INT_INPUT_STATE + PPC_IRQ_LINE_4:	info->i = ppc4xx_get_irq_line(ppc, PPC4XX_IRQ_BIT_EXT4);		break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 31;					break;
		case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_PAGE_SHIFT_PROGRAM:	info->i = POWERPC_MIN_PAGE_SHIFT;break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							/* provided per-CPU */					break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(internal_ppc4xx); break;

		/* --- everything else is handled generically --- */
		default:										ppccom_get_info(ppc, state, info);		break;
	}
}

