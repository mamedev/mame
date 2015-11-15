// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#define DUMP_PAGEFAULTS 0

INLINE UINT8 READ8(UINT32 a)
{
	return ppc.read8(ppc.program, a);
}

INLINE UINT16 READ16(UINT32 a)
{
	if( a & 0x1 )
		return ppc.read16_unaligned(ppc.program, a);
	else
		return ppc.read16(ppc.program, a);
}

INLINE UINT32 READ32(UINT32 a)
{
	if( a & 0x3 )
		return ppc.read32_unaligned(ppc.program, a);
	else
		return ppc.read32(ppc.program, a);
}

INLINE UINT64 READ64(UINT32 a)
{
	if( a & 0x7 )
		return ppc.read64_unaligned(ppc.program, a);
	else
		return ppc.read64(ppc.program, a);
}

INLINE void WRITE8(UINT32 a, UINT8 d)
{
	ppc.write8(ppc.program, a, d);
}

INLINE void WRITE16(UINT32 a, UINT16 d)
{
	if( a & 0x1 )
		ppc.write16_unaligned(ppc.program, a, d);
	else
		ppc.write16(ppc.program, a, d);
}

INLINE void WRITE32(UINT32 a, UINT32 d)
{
	if( ppc.reserved ) {
		if( a == ppc.reserved_address ) {
			ppc.reserved = 0;
		}
	}

	if( a & 0x3 )
		ppc.write32_unaligned(ppc.program, a, d);
	else
		ppc.write32(ppc.program, a, d);
}

INLINE void WRITE64(UINT32 a, UINT64 d)
{
	if( a & 0x7 )
		ppc.write64_unaligned(ppc.program, a, d);
	else
		ppc.write64(ppc.program, a, d);
}

/***********************************************************************/

static UINT16 ppc_read16_unaligned(address_space &space, UINT32 a)
{
	return ((UINT16)ppc.read8(space, a+0) << 8) | ((UINT16)ppc.read8(space, a+1) << 0);
}

static UINT32 ppc_read32_unaligned(address_space &space, UINT32 a)
{
	return ((UINT32)ppc.read8(space, a+0) << 24) | ((UINT32)ppc.read8(space, a+1) << 16) |
					((UINT32)ppc.read8(space, a+2) << 8) | ((UINT32)ppc.read8(space, a+3) << 0);
}

static UINT64 ppc_read64_unaligned(address_space &space, UINT32 a)
{
	return ((UINT64)READ32(space, a+0) << 32) | (UINT64)(READ32(space, a+4));
}

static void ppc_write16_unaligned(address_space &space, UINT32 a, UINT16 d)
{
	ppc.write8(space, a+0, (UINT8)(d >> 8));
	ppc.write8(space, a+1, (UINT8)(d));
}

static void ppc_write32_unaligned(address_space &space, UINT32 a, UINT32 d)
{
	ppc.write8(space, a+0, (UINT8)(d >> 24));
	ppc.write8(space, a+1, (UINT8)(d >> 16));
	ppc.write8(space, a+2, (UINT8)(d >> 8));
	ppc.write8(space, a+3, (UINT8)(d >> 0));
}

static void ppc_write64_unaligned(address_space &space, UINT32 a, UINT64 d)
{
	ppc.write32(space, a+0, (UINT32)(d >> 32));
	ppc.write32(space, a+4, (UINT32)(d));
}

/***********************************************************************/

#define DSISR_PAGE      0x40000000
#define DSISR_PROT      0x08000000
#define DSISR_STORE     0x02000000

enum
{
	PPC_TRANSLATE_DATA  = 0x0000,
	PPC_TRANSLATE_CODE  = 0x0001,

	PPC_TRANSLATE_READ  = 0x0000,
	PPC_TRANSLATE_WRITE = 0x0002,

	PPC_TRANSLATE_NOEXCEPTION = 0x0004
};

static int ppc_is_protected(UINT32 pp, int flags)
{
	if (flags & PPC_TRANSLATE_WRITE)
	{
		if ((pp & 0x00000003) != 0x00000002)
			return TRUE;
	}
	else
	{
		if ((pp & 0x00000003) == 0x00000000)
			return TRUE;
	}
	return FALSE;
}

static int ppc_translate_address(offs_t *addr_ptr, int flags)
{
	const BATENT *bat;
	UINT32 address;
	UINT32 sr, vsid, hash;
	UINT32 pteg_address;
	UINT32 target_pte, bl, mask;
	UINT64 pte;
	UINT64 *pteg_ptr[2];
	int i, hash_type;
	UINT32 dsisr = DSISR_PROT;

	bat = (flags & PPC_TRANSLATE_CODE) ? ppc.ibat : ppc.dbat;

	address = *addr_ptr;

	/* first check the block address translation table */
	for (i = 0; i < 4; i++)
	{
		if (bat[i].u & ((MSR & MSR_PR) ? 0x00000001 : 0x00000002))
		{
			bl = bat[i].u & 0x00001FFC;
			mask = (~bl << 15) & 0xFFFE0000;

			if ((address & mask) == (bat[i].u & 0xFFFE0000))
			{
				if (ppc_is_protected(bat[i].l, flags))
					goto exception;

				*addr_ptr = (bat[i].l & 0xFFFE0000)
					| (address & ((bl << 15) | 0x0001FFFF));
				return 1;
			}
		}
	}

	/* now try page address translation */
	sr = ppc.sr[(address >> 28) & 0x0F];
	if (sr & 0x80000000)
	{
		/* direct store translation */
		if ((flags & PPC_TRANSLATE_NOEXCEPTION) == 0)
			fatalerror("ppc: direct store translation not yet implemented\n");
		return 0;
	}
	else
	{
		/* is no execute is set? */
		if ((flags & PPC_TRANSLATE_CODE) && (sr & 0x10000000))
			goto exception;

		vsid = sr & 0x00FFFFFF;
		hash = (vsid & 0x0007FFFF) ^ ((address >> 12) & 0xFFFF);
		target_pte = (vsid << 7) | ((address >> 22) & 0x3F) | 0x80000000;

		/* we have to try both types of hashes */
		for (hash_type = 0; hash_type <= 1; hash_type++)
		{
			pteg_address = (ppc.sdr1 & 0xFFFF0000)
				| (((ppc.sdr1 & 0x01FF) & (hash >> 10)) << 16)
				| ((hash & 0x03FF) << 6);

			pteg_ptr[hash_type] = ppc->program->get_read_ptr(pteg_address);
			if (pteg_ptr[hash_type])
			{
				for (i = 0; i < 8; i++)
				{
					pte = pteg_ptr[hash_type][i];

					/* is valid? */
					if (((pte >> 32) & 0xFFFFFFFF) == target_pte)
					{
						if (ppc_is_protected((UINT32) pte, flags))
							goto exception;

						*addr_ptr = ((UINT32) (pte & 0xFFFFF000))
							| (address & 0x0FFF);
						return 1;
					}
				}
			}

			hash ^= 0x7FFFF;
			target_pte ^= 0x40;
		}

		if (DUMP_PAGEFAULTS)
		{
			osd_printf_debug("PAGE FAULT: address=%08X PC=%08X SDR1=%08X MSR=%08X\n", address, ppc.pc, ppc.sdr1, ppc.msr);
			osd_printf_debug("\n");

			for (i = 0; i < 4; i++)
			{
				bl = bat[i].u & 0x00001FFC;
				mask = (~bl << 15) & 0xFFFE0000;
				osd_printf_debug("    BAT[%d]=%08X%08X    (A & %08X = %08X)\n", i, bat[i].u, bat[i].l,
					mask, bat[i].u & 0xFFFE0000);
			}
			osd_printf_debug("\n");
			osd_printf_debug("    VSID=%06X HASH=%05X HASH\'=%05X\n", vsid, hash, hash ^ 0x7FFFF);

			for (hash_type = 0; hash_type <= 1; hash_type++)
			{
				if (pteg_ptr[hash_type])
				{
					for (i = 0; i < 8; i++)
					{
						pte = pteg_ptr[hash_type][i];
						osd_printf_debug("    PTE[%i%c]=%08X%08X\n",
							i,
							hash_type ? '\'' : ' ',
							(unsigned) (pte >> 32),
							(unsigned) (pte >> 0));
					}
				}
			}
		}
	}

	dsisr = DSISR_PAGE;

exception:
	/* lookup failure - exception */
	if ((flags & PPC_TRANSLATE_NOEXCEPTION) == 0)
	{
		if (flags & PPC_TRANSLATE_CODE)
		{
			ppc_exception(EXCEPTION_ISI);
		}
		else
		{
			ppc.dar = address;
			if (flags & PPC_TRANSLATE_WRITE)
				ppc.dsisr = dsisr | DSISR_STORE;
			else
				ppc.dsisr = dsisr;

			ppc_exception(EXCEPTION_DSI);
		}
	}
	return 0;
}

static int ppc_translate_address_cb(address_spacenum space, offs_t *addr)
{
	int success = 1;

	if (space == AS_PROGRAM)
	{
		if (MSR & MSR_DR)
			success = ppc_translate_address(addr, PPC_TRANSLATE_CODE | PPC_TRANSLATE_READ | PPC_TRANSLATE_NOEXCEPTION);
	}
	return success;
}

static UINT8 ppc_read8_translated(address_space &space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return space.read_byte(address);
}

static UINT16 ppc_read16_translated(address_space &space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return space.read_word(address);
}

static UINT32 ppc_read32_translated(address_space &space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return space.read_dword(address);
}

static UINT64 ppc_read64_translated(address_space &space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return space.read_qword(address);
}

static void ppc_write8_translated(address_space &space, offs_t address, UINT8 data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	space.write_byte(address, data);
}

static void ppc_write16_translated(address_space &space, offs_t address, UINT16 data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	space.write_word(address, data);
}

static void ppc_write32_translated(address_space &space, offs_t address, UINT32 data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	space.write_dword(address, data);
}

static void ppc_write64_translated(address_space &space, offs_t address, UINT64 data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	space.write_qword(address, data);
}

#ifndef PPC_DRC
static UINT32 ppc_readop_translated(address_space &space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_CODE | PPC_TRANSLATE_READ);
	return space.read_dword(address);
}
#endif

/***********************************************************************/


static CPU_DISASSEMBLE( ppc )
{
	UINT32 op;
	op = BIG_ENDIANIZE_INT32(*((UINT32 *) oprom));
	return ppc_dasm_one(buffer, pc, op);
}

/***********************************************************************/

static CPU_READOP( ppc )
{
	if (!(ppc.msr & MSR_IR))
		return 0;

	*value = 0;

	if (ppc_translate_address(&offset, PPC_TRANSLATE_CODE | PPC_TRANSLATE_READ | PPC_TRANSLATE_NOEXCEPTION))
	{
		switch(size)
		{
			case 1: *value = ppc.program->read_byte(offset);    break;
			case 2: *value = ppc.program->read_word(offset);    break;
			case 4: *value = ppc.program->read_dword(offset);   break;
			case 8: *value = ppc.program->read_qword(offset);   break;
		}
	}

	return 1;
}

static CPU_READ( ppc )
{
	if (!(ppc.msr & MSR_DR))
		return 0;

	*value = 0;

	if (ppc_translate_address(&offset, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ | PPC_TRANSLATE_NOEXCEPTION))
	{
		switch(size)
		{
			case 1: *value = ppc.program->read_byte(offset);    break;
			case 2: *value = ppc.program->read_word(offset);    break;
			case 4: *value = ppc.program->read_dword(offset);   break;
			case 8: *value = ppc.program->read_qword(offset);   break;
		}
	}

	return 1;
}

static CPU_WRITE( ppc )
{
	if (!(ppc.msr & MSR_DR))
		return 0;

	if (ppc_translate_address(&offset, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE | PPC_TRANSLATE_NOEXCEPTION))
	{
		switch(size)
		{
			case 1: ppc.program->write_byte(offset, value); break;
			case 2: ppc.program->write_word(offset, value); break;
			case 4: ppc.program->write_dword(offset, value);    break;
			case 8: ppc.program->write_qword(offset, value);    break;
		}
	}

	return 1;
}
