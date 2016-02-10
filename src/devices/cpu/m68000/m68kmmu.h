// license:BSD-3-Clause
// copyright-holders:R. Belmont, Hans Ostermeyer
/*
    m68kmmu.h - PMMU implementation for 68851/68030/68040
            HMMU implementation for 68020 (II and LC variants)

    By R. Belmont and Hans Ostermeyer
*/

// MMU status register bit definitions

#define M68K_MMU_SR_BUS_ERROR 0x8000
#define M68K_MMU_SR_SUPERVISOR_ONLY 0x2000
#define M68K_MMU_SR_WRITE_PROTECT 0x0800
#define M68K_MMU_SR_INVALID 0x0400
#define M68K_MMU_SR_MODIFIED 0x0200
#define M68K_MMU_SR_LEVEL_0 0x0000
#define M68K_MMU_SR_LEVEL_1 0x0001
#define M68K_MMU_SR_LEVEL_2 0x0002
#define M68K_MMU_SR_LEVEL_3 0x0003

// MMU translation table descriptor field definitions

#define M68K_MMU_DF_DT         0x0003
#define M68K_MMU_DF_DT0        0x0000
#define M68K_MMU_DF_DT1        0x0001
#define M68K_MMU_DF_DT2        0x0002
#define M68K_MMU_DF_DT3        0x0003
#define M68K_MMU_DF_WP         0x0004
#define M68K_MMU_DF_USED       0x0008
#define M68K_MMU_DF_MODIFIED   0x0010
#define M68K_MMU_DF_CI         0x0040
#define M68K_MMU_DF_SUPERVISOR 0x0100

// MMU ATC Fields

#define M68K_MMU_ATC_BUSERROR  0x08000000
#define M68K_MMU_ATC_CACHE_IN  0x04000000
#define M68K_MMU_ATC_WRITE_PR  0x02000000
#define M68K_MMU_ATC_MODIFIED  0x01000000
#define M68K_MMU_ATC_MASK      0x00ffffff
#define M68K_MMU_ATC_SHIFT     8
#define M68K_MMU_ATC_VALID     0x08000000

// MMU Translation Control register
#define M68K_MMU_TC_SRE        0x02000000

/* decodes the effective address */
static UINT32 DECODE_EA_32(m68000_base_device *m68k, int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			return REG_A(m68k)[reg];
		}
		case 3:     // (An)+
		{
			UINT32 ea = EA_AY_PI_32(m68k);
			return ea;
		}
		case 5:     // (d16, An)
		{
			UINT32 ea = EA_AY_DI_32(m68k);
			return ea;
		}
		case 6:     // (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_32(m68k);
			return ea;
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					UINT32 ea = (UINT32)OPER_I_16(m68k);
					return ea;
				}
				case 1:     // (xxx).L
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					return ea;
				}
				case 2:     // (d16, PC)
				{
					UINT32 ea = EA_PCDI_32(m68k);
					return ea;
				}
				default:    fatalerror("m68k: DECODE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC(m68k));
			}
			break;
		}
		default:    fatalerror("m68k: DECODE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC(m68k));
	}
	return 0;
}

/*
    pmmu_atc_add: adds this address to the ATC
*/
void pmmu_atc_add(m68000_base_device *m68k, UINT32 logical, UINT32 physical, int fc)
{
	int i, found;

	// get page size (i.e. # of bits to ignore); is 10 for Apollo
	int ps = (m68k->mmu_tc >> 20) & 0xf;
	// Note: exact emulation would use (logical >> ps) << (ps-8)
	UINT32 atc_tag = M68K_MMU_ATC_VALID | ((fc &7) << 24)| logical >> ps;

	// first see if this is already in the cache
	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		// if tag bits and function code match, don't add
		if (m68k->mmu_atc_tag[i] == atc_tag)
		{
			return;
		}
	}

	// find an open entry
	found = -1;
	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		if (!(m68k->mmu_atc_tag[i] & M68K_MMU_ATC_VALID))
		{
			found = i;
			break;
		}
	}

	// did we find an entry?  steal one by round-robin then
	if (found == -1)
	{
		found = m68k->mmu_atc_rr++;

		if (m68k->mmu_atc_rr >= MMU_ATC_ENTRIES)
		{
			m68k->mmu_atc_rr = 0;
		}
	}

	// add the entry
	// printf("ATC[%2d] add: log %08x -> phys %08x (fc=%d)\n", found, (logical>>ps) << ps, (physical >> ps) << ps, fc);
	m68k->mmu_atc_tag[found] = atc_tag;
	m68k->mmu_atc_data[found] = (physical >> ps) << (ps-8);

	if (m68k->mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT)
	{
		m68k->mmu_atc_data[found] |= M68K_MMU_ATC_WRITE_PR;
	}
}

/*
    pmmu_atc_flush: flush entire ATC

    7fff0003 001ffd10 80f05750 is what should load
*/
void pmmu_atc_flush(m68000_base_device *m68k)
{
	int i;
	// printf("ATC flush: pc=%08x\n", REG_PPC(m68k));

	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		m68k->mmu_atc_tag[i] = 0;
	}

	m68k->mmu_atc_rr = 0;
}


static inline UINT32 get_dt2_table_entry(m68000_base_device *m68k, UINT32 tptr, UINT8 ptest)
{
	UINT32 tbl_entry = m68k->program->read_dword(tptr);
	UINT32 dt = tbl_entry & M68K_MMU_DF_DT;

	m68k->mmu_tmp_sr |= tbl_entry & 0x0004 ? M68K_MMU_SR_WRITE_PROTECT : 0;

	if (!ptest && dt != M68K_MMU_DF_DT0)
	{
		if (dt == M68K_MMU_DF_DT1 && !m68k->mmu_tmp_rw && !(m68k->mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT))
		{
			// set used and modified
			m68k->program->write_dword( tptr, tbl_entry | M68K_MMU_DF_USED | M68K_MMU_DF_MODIFIED);
		}
		else if (!(tbl_entry & M68K_MMU_DF_USED))
		{
			m68k->program->write_dword( tptr, tbl_entry | M68K_MMU_DF_USED);
		}
	}
	return tbl_entry;
}

static inline UINT32 get_dt3_table_entry(m68000_base_device *m68k, UINT32 tptr, UINT8 fc, UINT8 ptest)
{
	UINT32 tbl_entry2 = m68k->program->read_dword(tptr);
	UINT32 tbl_entry = m68k->program->read_dword(tptr + 4);
	UINT32 dt = tbl_entry2 & M68K_MMU_DF_DT;

	m68k->mmu_tmp_sr |= ((tbl_entry2 & 0x0100) && !(fc & 4)) ? M68K_MMU_SR_SUPERVISOR_ONLY : 0;
	m68k->mmu_tmp_sr |= tbl_entry2 & 0x0004 ? M68K_MMU_SR_WRITE_PROTECT : 0;

	if (!ptest && dt != M68K_MMU_DF_DT0)
	{
		if (dt == M68K_MMU_DF_DT1 && !m68k->mmu_tmp_rw && !(m68k->mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT))
		{
			// set modified
			m68k->program->write_dword( tptr, tbl_entry2 | M68K_MMU_DF_USED | M68K_MMU_DF_MODIFIED);
		}
		else if (!(tbl_entry2 & M68K_MMU_DF_USED))
		{
			m68k->program->write_dword( tptr, tbl_entry2 | M68K_MMU_DF_USED);
		}
	}

	return (tbl_entry & ~M68K_MMU_DF_DT) | dt;
}

/*
    pmmu_translate_addr_with_fc: perform 68851/68030-style PMMU address translation
*/
/*static inline*/ static UINT32 pmmu_translate_addr_with_fc(m68000_base_device *m68k, UINT32 addr_in, UINT8 fc, UINT8 ptest)
{
	UINT32 addr_out, tbl_entry = 0, tamode = 0, tbmode = 0, tcmode = 0;
	UINT32 root_aptr, root_limit, tofs, ps, is, abits, bbits, cbits;
	UINT32 resolved, tptr, shift, last_entry_ptr;
	int i;
	UINT32 atc_tag;
//  int verbose = 0;

//  static UINT32 pmmu_access_count = 0;
//  static UINT32 pmmu_atc_count = 0;

	resolved = 0;
	addr_out = addr_in;
	m68k->mmu_tmp_sr = 0;

	if (fc == 7)
	{
		return addr_in;
	}

	if (m68k->mmu_tt0 & 0x8000)
	{
		// transparent translation register 0 enabled
		UINT32 address_base = m68k->mmu_tt0 & 0xff000000;
		UINT32 address_mask = ((m68k->mmu_tt0 << 8) & 0xff000000) ^ 0xff000000;
		if ((addr_in & address_mask) == address_base && (fc & ~m68k->mmu_tt0) == ((m68k->mmu_tt0 >> 4) & 7))
		{
//          printf("PMMU: pc=%x TT0 fc=%x addr_in=%08x address_mask=%08x address_base=%08x\n", m68k->ppc, fc, addr_in, address_mask, address_base);
			return addr_in;
		}
	}

	if (m68k->mmu_tt1 & 0x8000)
	{
		// transparent translation register 1 enabled
		UINT32 address_base = m68k->mmu_tt1 & 0xff000000;
		UINT32 address_mask = ((m68k->mmu_tt1 << 8) & 0xff000000) ^ 0xff000000;
		if ((addr_in & address_mask) == address_base && (fc & ~m68k->mmu_tt1) == ((m68k->mmu_tt1 >> 4) & 7))
		{
//          printf("PMMU: pc=%x TT1 fc=%x addr_in=%08x address_mask=%08x address_base=%08x\n", m68k->ppc, fc, addr_in, address_mask, address_base);
			return addr_in;
		}
	}

//  if ((++pmmu_access_count % 10000000) == 0) {
//      printf("pmmu_translate_addr_with_fc: atc usage = %d%%\n", pmmu_atc_count*100/pmmu_access_count);
//      pmmu_atc_count = pmmu_access_count = 0;
//  }

	// get page size (i.e. # of bits to ignore); ps is 10 or 12 for Apollo, 8 otherwise
	ps = (m68k->mmu_tc >> 20) & 0xf;
	atc_tag = M68K_MMU_ATC_VALID | ((fc &7) << 24) | addr_in >> ps;

	// first see if this is already in the ATC
	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		if (m68k->mmu_atc_tag[i] != atc_tag)
		{
			// tag bits and function code don't match
		}
		else if (!m68k->mmu_tmp_rw && (m68k->mmu_atc_data[i] & M68K_MMU_ATC_WRITE_PR))
		{
			// write mode, but write protected
		}
		else if (!m68k->mmu_tmp_rw && !(m68k->mmu_atc_data[i] & M68K_MMU_ATC_MODIFIED))
		{
			// first write; must set modified in PMMU tables as well
		}
		else
		{
			// read access or write access and not write protected
			if (!m68k->mmu_tmp_rw && !ptest)
			{
				// FIXME: must set modified in PMMU tables as well
				m68k->mmu_atc_data[i] |= M68K_MMU_ATC_MODIFIED;
			}
			else
			{
				// FIXME: supervisor mode?
				m68k->mmu_tmp_sr = M68K_MMU_SR_MODIFIED;
			}
			addr_out = (m68k->mmu_atc_data[i] << 8) | (addr_in & ~(~0 << ps));
//          printf("ATC[%2d] hit: log %08x -> phys %08x  pc=%08x fc=%d\n", i, addr_in, addr_out, REG_PPC(m68k), fc);
//          pmmu_atc_count++;
			return addr_out;
		}
	}

	// if SRP is enabled and we're in supervisor mode, use it
	if ((m68k->mmu_tc & M68K_MMU_TC_SRE) && (fc & 4))
	{
		root_aptr = m68k->mmu_srp_aptr;
		root_limit = m68k->mmu_srp_limit;
	}
	else    // else use the CRP
	{
		root_aptr = m68k->mmu_crp_aptr;
		root_limit = m68k->mmu_crp_limit;
	}

	// get initial shift (# of top bits to ignore)
	is = (m68k->mmu_tc >> 16) & 0xf;
	ps = (m68k->mmu_tc >> 20) & 0xf;
	abits = (m68k->mmu_tc >> 12) & 0xf;
	bbits = (m68k->mmu_tc >> 8) & 0xf;
	cbits = (m68k->mmu_tc >> 4) & 0xf;

	//  printf("PMMU: tcr %08x limit %08x aptr %08x is %x abits %d bbits %d cbits %d\n", m68k->mmu_tc, root_limit, root_aptr, is, abits, bbits, cbits);

	// get table A offset
	tofs = (addr_in<<is)>>(32-abits);
	tptr = root_aptr & 0xfffffff0;

	// find out what format table A is
	switch (root_limit & M68K_MMU_DF_DT)
	{
		case M68K_MMU_DF_DT0:   // invalid, will cause MMU exception
			m68k->mmu_tmp_sr |= M68K_MMU_SR_INVALID;
			return root_aptr;

		case M68K_MMU_DF_DT1:   // page descriptor, will cause direct mapping
			addr_out = tptr + addr_in;
//          printf("PMMU: PC=%x root mode %d (addr_in %08x -> %08x)\n", m68k->ppc, M68K_MMU_DF_DT1, addr_in, addr_out);
			return addr_out;

		case M68K_MMU_DF_DT2:   // valid 4 byte descriptors
			tofs *= 4;
//          if (verbose) printf("PMMU: reading table A entry at %08x\n", tofs + tptr);
			tbl_entry = get_dt2_table_entry(m68k,  tptr + tofs,  ptest);
			tamode = tbl_entry & M68K_MMU_DF_DT;
//          if (verbose) printf("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tamode, tofs);
			break;

		case M68K_MMU_DF_DT3: // valid 8 byte descriptors
			tofs *= 8;
//          if (verbose) printf("PMMU: reading table A entries at %08x\n", tofs + tptr);
			tbl_entry = get_dt3_table_entry(m68k,  tofs + tptr, fc,  ptest);
			tamode = tbl_entry & M68K_MMU_DF_DT;
//          if (verbose) printf("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tamode, tofs);
			break;
	}

	last_entry_ptr = tptr + tofs;

	// get table B offset and pointer
	tofs = (addr_in<<(is+abits))>>(32-bbits);
	tptr = tbl_entry & 0xfffffff0;

	// find out what format table B is, if any
	switch (tamode)
	{
		case M68K_MMU_DF_DT0: // invalid, will cause MMU exception (but not for ptest)
			m68k->mmu_tmp_sr |= (M68K_MMU_SR_INVALID | M68K_MMU_SR_LEVEL_1);
			// last valid pointer (for ptest)
			addr_out = last_entry_ptr;
			resolved = 1;
			break;

		case M68K_MMU_DF_DT2: // 4-byte table B descriptor
			tofs *= 4;
//          if (verbose) printf("PMMU: reading table B entry at %08x\n", tofs + tptr);
			tbl_entry = get_dt2_table_entry(m68k, tptr + tofs,  ptest);
			tbmode = tbl_entry & M68K_MMU_DF_DT;
//          if (verbose) printf("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tbmode, tofs);
			break;

		case M68K_MMU_DF_DT3: // 8-byte table B descriptor
			tofs *= 8;
//          if (verbose) printf("PMMU: reading table B entries at %08x\n", tofs + tptr);
			tbl_entry = get_dt3_table_entry(m68k, tptr + tofs, fc,  ptest);
			tbmode = tbl_entry & M68K_MMU_DF_DT;
			tbl_entry &= ~M68K_MMU_DF_DT;
//          if (verbose) printf("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tbmode, tofs);
			break;

		case M68K_MMU_DF_DT1:   // early termination descriptor
			tbl_entry &= (~0 << ps);

			shift = is+abits;
			addr_out = ((addr_in<<shift)>>shift) + tbl_entry;
			resolved = 1;
			break;
	}

	// if table A wasn't early-out, continue to process table B
	if (!resolved)
	{
		last_entry_ptr =  tptr + tofs;

		// get table C offset and pointer
		tofs = (addr_in<<(is+abits+bbits))>>(32-cbits);
		tptr = tbl_entry & 0xfffffff0;

		switch (tbmode)
		{
			case M68K_MMU_DF_DT0:   // invalid, will cause MMU exception (but not for ptest)
				m68k->mmu_tmp_sr |= (M68K_MMU_SR_INVALID | M68K_MMU_SR_LEVEL_2);
				// last valid pointer (for ptest)
				addr_out = last_entry_ptr;
				resolved = 1;
				break;

			case M68K_MMU_DF_DT2: // 4-byte table C descriptor
				tofs *= 4;
//              if (verbose) printf("PMMU: reading table C entry at %08x\n", tofs + tptr);
				tbl_entry = get_dt2_table_entry(m68k, tptr + tofs, ptest);
				tcmode = tbl_entry & M68K_MMU_DF_DT;
//              if (verbose) printf("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tbmode, tofs);
				break;

			case M68K_MMU_DF_DT3: // 8-byte table C descriptor
				tofs *= 8;
//              if (verbose) printf("PMMU: reading table C entries at %08x\n", tofs + tptr);
				tbl_entry = get_dt3_table_entry(m68k,  tptr+ tofs, fc,  ptest);
				tcmode = tbl_entry & M68K_MMU_DF_DT;
//              if (verbose) printf("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tcmode, tofs);
				break;

			case M68K_MMU_DF_DT1: // termination descriptor
				tbl_entry &= (~0 << ps);

				shift = is+abits+bbits;
				addr_out = ((addr_in<<shift)>>shift) + tbl_entry;
				resolved = 1;
				break;
		}
	}

	if (!resolved)
	{
		switch (tcmode)
		{
			case M68K_MMU_DF_DT0:   // invalid, will cause MMU exception (unless ptest)
				m68k->mmu_tmp_sr |= (M68K_MMU_SR_INVALID | M68K_MMU_SR_LEVEL_3);
				addr_out = tptr + tofs;
				resolved = 1;
				break;

			case M68K_MMU_DF_DT2: // 4-byte (short-form) indirect descriptor
			case M68K_MMU_DF_DT3: // 8-byte (long-form) indirect descriptor
				fatalerror("PMMU: pc=%08x Unhandled Table C mode %d (addr_in %08x)\n", m68k->ppc, tcmode, addr_in);
				break;

			case M68K_MMU_DF_DT1: // termination descriptor
				tbl_entry &= (~0 << ps);

				shift = is+abits+bbits+cbits;
				addr_out = ((addr_in<<shift)>>shift) + tbl_entry;
				resolved = 1;
				break;
		}
	}

	if (!ptest)
	{
		if (m68k->mmu_tmp_sr & M68K_MMU_SR_INVALID)
		{
			if (++m68k->mmu_tmp_buserror_occurred == 1)
			{
				m68k->mmu_tmp_buserror_address = addr_in;
				m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
				m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
			}
		}
		else if (m68k->mmu_tmp_sr & M68K_MMU_SR_SUPERVISOR_ONLY)
		{
			if (++m68k->mmu_tmp_buserror_occurred == 1)
			{
				m68k->mmu_tmp_buserror_address = addr_in;
				m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
				m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
			}
		}
		else if ((m68k->mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT) && !m68k->mmu_tmp_rw)
		{
			if (++m68k->mmu_tmp_buserror_occurred == 1)
			{
				m68k->mmu_tmp_buserror_address = addr_in;
				m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
				m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
			}
		}

		if (!m68k->mmu_tmp_buserror_occurred)
		{
			// we add only valid entries
			pmmu_atc_add(m68k, addr_in, addr_out, fc);
		}
	}

	//printf("PMMU: [%08x] => [%08x]\n", addr_in, addr_out);

	return addr_out;
}


// FC bits: 2 = supervisor, 1 = program, 0 = data
// the 68040 is a subset of the 68851 and 68030 PMMUs - the page table sizes are fixed, there is no early termination, etc, etc.
/*static inline*/ static UINT32 pmmu_translate_addr_with_fc_040(m68000_base_device *m68k, UINT32 addr_in, UINT8 fc, UINT8 ptest)
{
	UINT32 addr_out, tt0, tt1;

	addr_out = addr_in;
	m68k->mmu_tmp_sr = 0;

	// transparent translation registers are always in force even if the PMMU itself is disabled
	// they don't do much in emulation because we never write out of order, but the write-protect and cache control features
	// are emulatable, and apparently transparent translation regions skip the page table lookup.
	if (fc & 1) // data, use DTT0/DTT1
	{
		tt0 = m68k->mmu_dtt0;
		tt1 = m68k->mmu_dtt1;
	}
	else if (fc & 2)    // program, use ITT0/ITT1
	{
		tt0 = m68k->mmu_itt0;
		tt1 = m68k->mmu_itt1;
	}
	else
	{
		fatalerror("68040: function code %d is neither data nor program!\n", fc&7);
	}

	if (tt0 & 0x8000)
	{
		static int fcmask[4] = { 4, 4, 0, 0 };
		static int fcmatch[4] = { 0, 4, 0, 0 };
		UINT32 mask = (tt0>>16) & 0xff;
		mask ^= 0xff;
		mask <<= 24;

		if ((addr_in & mask) == (tt0 & mask) && (fc & fcmask[(tt0 >> 13) & 3]) == fcmatch[(tt0 >> 13) & 3])
		{
			//          fprintf(stderr, "TT0 match on address %08x (TT0 = %08x, mask = %08x)\n", addr_in, tt0, mask);
			if ((tt0 & 4) && !m68k->mmu_tmp_rw && !ptest)   // write protect?
			{
				if (++m68k->mmu_tmp_buserror_occurred == 1)
				{
					m68k->mmu_tmp_buserror_address = addr_in;
					m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
					m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
				}
			}

			return addr_in;
		}
	}

	if (tt1 & 0x8000)
	{
		static int fcmask[4] = { 4, 4, 0, 0 };
		static int fcmatch[4] = { 0, 4, 0, 0 };
		UINT32 mask = (tt1>>16) & 0xff;
		mask ^= 0xff;
		mask <<= 24;

		if ((addr_in & mask) == (tt1 & mask) && (fc & fcmask[(tt1 >> 13) & 3]) == fcmatch[(tt1 >> 13) & 3])
		{
			//          fprintf(stderr, "TT1 match on address %08x (TT0 = %08x, mask = %08x)\n", addr_in, tt1, mask);
			if ((tt1 & 4) && !m68k->mmu_tmp_rw && !ptest)   // write protect?
			{
				if (++m68k->mmu_tmp_buserror_occurred == 1)
				{
					m68k->mmu_tmp_buserror_address = addr_in;
					m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
					m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
				}
			}

			return addr_in;
		}
	}

	if (m68k->pmmu_enabled)
	{
		UINT32 root_idx = (addr_in>>25) & 0x7f;
		UINT32 ptr_idx = (addr_in>>18) & 0x7f;
		UINT32 page_idx, page;
		UINT32 root_ptr, pointer_ptr, page_ptr;
		UINT32 root_entry, pointer_entry, page_entry;

		// select supervisor or user root pointer
		if (fc & 4)
		{
			root_ptr = m68k->mmu_srp_aptr + (root_idx<<2);
		}
		else
		{
			root_ptr = m68k->mmu_urp_aptr + (root_idx<<2);
		}

		// get the root entry
		root_entry = m68k->program->read_dword(root_ptr);

		// is UDT marked valid?
		if (root_entry & 2)
		{
			// we're accessing through this root entry, so set the U bit
			if ((!(root_entry & 0x8)) && (!ptest))
			{
				root_entry |= 0x8;
				m68k->program->write_dword(root_ptr, root_entry);
			}

			// PTEST: any write protect bits set in the search tree will set W in SR
			if ((ptest) && (root_entry & 4))
			{
				m68k->mmu_tmp_sr |= 4;
			}

			pointer_ptr = (root_entry & ~0x1ff) + (ptr_idx<<2);
			pointer_entry = m68k->program->read_dword(pointer_ptr);

			// PTEST: any write protect bits set in the search tree will set W in SR
			if ((ptest) && (pointer_entry & 4))
			{
				m68k->mmu_tmp_sr |= 4;
			}

			// update U bit on this pointer entry too
			if ((!(pointer_entry & 0x8)) && (!ptest))
			{
				pointer_entry |= 0x8;
				m68k->program->write_dword(pointer_ptr, pointer_entry);
			}

			//          logerror("pointer entry = %08x\n", pointer_entry);

			// write protected by the root or pointer entries?
			if ((((root_entry & 4) && !m68k->mmu_tmp_rw) || ((pointer_entry & 4) && !m68k->mmu_tmp_rw)) && !ptest)
			{
				if (++m68k->mmu_tmp_buserror_occurred == 1)
				{
					m68k->mmu_tmp_buserror_address = addr_in;
					m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
					m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
				}

				return addr_in;
			}

			// is UDT valid on the pointer entry?
			if (!(pointer_entry & 2) && !ptest)
			{
//              fprintf(stderr, "Invalid pointer entry!  PC=%x, addr=%x\n", m68k->ppc, addr_in);
				if (++m68k->mmu_tmp_buserror_occurred == 1)
				{
					m68k->mmu_tmp_buserror_address = addr_in;
					m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
					m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
				}

				return addr_in;
			}

			// (fall out of these ifs into the page lookup below)
		}
		else // throw an error
		{
//          fprintf(stderr, "Invalid root entry!  PC=%x, addr=%x\n", m68k->ppc, addr_in);

			if (!ptest)
			{
				if (++m68k->mmu_tmp_buserror_occurred == 1)
				{
					m68k->mmu_tmp_buserror_address = addr_in;
					m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
					m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
				}
			}

			return addr_in;
		}

		// now do the page lookup
		if (m68k->mmu_tc & 0x4000)  // 8k pages?
		{
			page_idx = (addr_in >> 13) & 0x1f;
			page = addr_in & 0x1fff;
			pointer_entry &= ~0x7f;
			//          logerror("8k pages: index %x page %x\n", page_idx, page);
		}
		else    // 4k pages
		{
			page_idx = (addr_in >> 12) & 0x3f;
			page = addr_in & 0xfff;
			pointer_entry &= ~0xff;
			//          logerror("4k pages: index %x page %x\n", page_idx, page);
		}

		page_ptr = pointer_entry + (page_idx<<2);
		page_entry = m68k->program->read_dword(page_ptr);
		m68k->mmu_last_page_entry_addr = page_ptr;

		//      logerror("page_entry = %08x\n", page_entry);

		// resolve indirect page pointers
		while ((page_entry & 3) == 2)
		{
			page_entry = m68k->program->read_dword(page_entry & ~0x3);
			m68k->mmu_last_page_entry_addr = (page_entry & ~0x3);
		}
		m68k->mmu_last_page_entry = page_entry;

		// is the page write protected or supervisor protected?
		if ((((page_entry & 4) && !m68k->mmu_tmp_rw) || ((page_entry & 0x80) && !(fc&4))) && !ptest)
		{
			if (++m68k->mmu_tmp_buserror_occurred == 1)
			{
				m68k->mmu_tmp_buserror_address = addr_in;
				m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
				m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
			}

			return addr_in;
		}

		switch (page_entry & 3)
		{
			case 0: // invalid
//              fprintf(stderr, "Invalid page entry!  PC=%x, addr=%x\n", m68k->ppc, addr_in);
				if (!ptest)
				{
					if (++m68k->mmu_tmp_buserror_occurred == 1)
					{
						m68k->mmu_tmp_buserror_address = addr_in;
						m68k->mmu_tmp_buserror_rw = m68k->mmu_tmp_rw;
						m68k->mmu_tmp_buserror_fc = m68k->mmu_tmp_fc;
					}
				}

				return addr_in;

			case 1:
			case 3: // normal
				if (m68k->mmu_tc & 0x4000)  // 8k pages?
				{
					addr_out = (page_entry & ~0x1fff) | page;
				}
				else
				{
					addr_out = (page_entry & ~0xfff) | page;
				}

				if (!(ptest))
				{
					page_entry |= 0x8;  // always set the U bit

					// if we're writing, the M bit comes into play
					if (!m68k->mmu_tmp_rw)
					{
						page_entry |= 0x10; // set Modified
					}

					// if these updates resulted in a change, write the entry back where we found it
					if (page_entry != m68k->mmu_last_page_entry)
					{
						m68k->mmu_last_page_entry = page_entry;
						m68k->program->write_dword(m68k->mmu_last_page_entry_addr, m68k->mmu_last_page_entry);
					}
				}
				else
				{
					// page entry: UR G U1 U0 S CM CM M U W PDT
					// SR:         B  G U1 U0 S CM CM M 0 W T R
					m68k->mmu_tmp_sr |= ((addr_out & ~0xfff) || (page_entry & 0x7f4));
				}
				break;

			case 2: // shouldn't happen
				fatalerror("68040: got indirect final page pointer, shouldn't be possible\n");
				break;
		}
		//      if (addr_in != addr_out) fprintf(stderr, "040MMU: [%08x] => [%08x]\n", addr_in, addr_out);
	}

	return addr_out;
}

/*
    pmmu_translate_addr: perform 68851/68030-style PMMU address translation
*/
/*static inline*/ static UINT32 pmmu_translate_addr(m68000_base_device *m68k, UINT32 addr_in)
{
	UINT32 addr_out;

	if (CPU_TYPE_IS_040_PLUS(m68k->cpu_type))
	{
		addr_out = pmmu_translate_addr_with_fc_040(m68k, addr_in, m68k->mmu_tmp_fc, 0);
	}
	else
	{
		addr_out = pmmu_translate_addr_with_fc(m68k, addr_in, m68k->mmu_tmp_fc, 0);
	}

//  if (m68k->mmu_tmp_buserror_occurred > 0) {
//      printf("PMMU: pc=%08x sp=%08x va=%08x pa=%08x - invalid Table mode for level=%d (buserror %d)\n",
//              REG_PPC(m68k), REG_A(m68k)[7], addr_in, addr_out, m68k->mmu_tmp_sr & M68K_MMU_SR_LEVEL_3,
//              m68k->mmu_tmp_buserror_occurred);
//  }

	return addr_out;
}

/*
    m68851_mmu_ops: COP 0 MMU opcode handling
*/

void m68881_mmu_ops(m68000_base_device *m68k)
{
	UINT16 modes;
	UINT32 ea = m68k->ir & 0x3f;
	UINT64 temp64;


	// catch the 2 "weird" encodings up front (PBcc)
	if ((m68k->ir & 0xffc0) == 0xf0c0)
	{
		printf("680x0: unhandled PBcc\n");
		return;
	}
	else if ((m68k->ir & 0xffc0) == 0xf080)
	{
		printf("680x0: unhandled PBcc\n");
		return;
	}
	else if ((m68k->ir & 0xffe0) == 0xf500)
	{
//      logerror("68040 pflush: pc=%08x ir=%04x opmode=%d register=%d\n", REG_PPC(m68k), m68k->ir, (m68k->ir >> 3) & 3, m68k->ir & 7);
		pmmu_atc_flush(m68k);
	}
	else    // the rest are 1111000xxxXXXXXX where xxx is the instruction family
	{
		switch ((m68k->ir>>9) & 0x7)
		{
			case 0:
				modes = OPER_I_16(m68k);

				if ((modes & 0xfde0) == 0x2000) // PLOAD
				{
					UINT32 ltmp = DECODE_EA_32(m68k, ea);
					UINT32 ptmp;

					ptmp = ltmp;
					if (m68k->pmmu_enabled)
					{
						if (CPU_TYPE_IS_040_PLUS(m68k->cpu_type))
						{
							ptmp = pmmu_translate_addr_with_fc_040(m68k, ltmp, modes & 0x07, 0);
						}
						else
						{
							ptmp = pmmu_translate_addr_with_fc(m68k, ltmp, modes & 0x07, 0);
						}
					}

//                    printf("680x0: PLOADing ATC with logical %08x => phys %08x\n", ltmp, ptmp);
					// FIXME: rw bit?
					pmmu_atc_add(m68k, ltmp, ptmp,  modes & 0x07);
					return;
				}
				else if ((modes & 0xe200) == 0x2000)    // PFLUSH
				{
					pmmu_atc_flush(m68k);
					return;
				}
				else if (modes == 0xa000)   // PFLUSHR
				{
					pmmu_atc_flush(m68k);
					return;
				}
				else if (modes == 0x2800)   // PVALID (FORMAT 1)
				{
					printf("680x0: unhandled PVALID1\n");
					return;
				}
				else if ((modes & 0xfff8) == 0x2c00)    // PVALID (FORMAT 2)
				{
					printf("680x0: unhandled PVALID2\n");
					return;
				}
				else if ((modes & 0xe000) == 0x8000)    // PTEST
				{
					UINT32 v_addr = DECODE_EA_32(m68k,  ea);
					UINT32 p_addr;
					UINT32 fc = modes & 0x1f;
					switch (fc >> 3) {
					case 0:
						fc = fc == 0 ? m68k->sfc :  m68k->dfc;
						break;
					case 1:
						fc = REG_D(m68k)[fc &7] &7;
						break;
					case 2:
						fc &=7;
						break;
					}

					if (CPU_TYPE_IS_040_PLUS(m68k->cpu_type))
					{
						p_addr = pmmu_translate_addr_with_fc_040(m68k, v_addr, fc, 1);
					}
					else
					{
						p_addr = pmmu_translate_addr_with_fc(m68k, v_addr, fc, 1);
					}
					m68k->mmu_sr = m68k->mmu_tmp_sr;

//                  printf("PMMU: pc=%08x sp=%08x va=%08x pa=%08x PTEST fc=%x level=%x mmu_sr=%04x\n",
//                          m68k->ppc, REG_A(m68k)[7], v_addr, p_addr, fc, (modes >> 10) & 0x07, m68k->mmu_sr);

					if (modes & 0x100)
					{
						int areg = (modes >> 5) & 7;
						WRITE_EA_32(m68k, 0x08 | areg, p_addr);
					}
					return;
				}
				else
				{
					switch ((modes>>13) & 0x7)
					{
						case 0: // MC68030/040 form with FD bit
						case 2: // MC68881 form, FD never set
							if (modes & 0x200)
							{
									switch ((modes>>10) & 0x3f)
									{
										case 0x02: // transparent translation register 0
											WRITE_EA_32(m68k, ea, m68k->mmu_tt0);
//                                          printf("PMMU: pc=%x PMOVE from mmu_tt0=%08x\n", m68k->ppc, m68k->mmu_tt0);
											break;
										case 0x03: // transparent translation register 1
											WRITE_EA_32(m68k, ea, m68k->mmu_tt1);
//                                          printf("PMMU: pc=%x PMOVE from mmu_tt1=%08x\n", m68k->ppc, m68k->mmu_tt1);
											break;
										case 0x10:  // translation control register
											WRITE_EA_32(m68k, ea, m68k->mmu_tc);
//                                          printf("PMMU: pc=%x PMOVE from mmu_tc=%08x\n", m68k->ppc, m68k->mmu_tc);
											break;

										case 0x12: // supervisor root pointer
											WRITE_EA_64(m68k, ea, (UINT64)m68k->mmu_srp_limit<<32 | (UINT64)m68k->mmu_srp_aptr);
//                                          printf("PMMU: pc=%x PMOVE from SRP limit = %08x, aptr = %08x\n", REG_PPC(m68k), m68k->mmu_srp_limit, m68k->mmu_srp_aptr);
											break;

										case 0x13: // CPU root pointer
											WRITE_EA_64(m68k, ea, (UINT64)m68k->mmu_crp_limit<<32 | (UINT64)m68k->mmu_crp_aptr);
//                                          printf("PMMU: pc=%x PMOVE from CRP limit = %08x, aptr = %08x\n", REG_PPC(m68k), m68k->mmu_crp_limit, m68k->mmu_crp_aptr);
											break;

										default:
											printf("680x0: PMOVE from unknown MMU register %x, PC %x\n", (modes>>10) & 7, m68k->pc);
											break;
								}

							}
							else    // top 3 bits of modes: 010 for this, 011 for status, 000 for transparent translation regs
							{
								switch ((modes>>13) & 7)
								{
									case 0:
										{
											UINT32 temp = READ_EA_32(m68k, ea);

											if (((modes>>10) & 7) == 2)
											{
												m68k->mmu_tt0 = temp;
											}
											else if (((modes>>10) & 7) == 3)
											{
												m68k->mmu_tt1 = temp;
											}
										}
										break;

									case 1:
										printf("680x0: unknown PMOVE case 1, PC %x\n", m68k->pc);
										break;

									case 2:
										switch ((modes>>10) & 7)
										{
											case 0: // translation control register
												m68k->mmu_tc = READ_EA_32(m68k, ea);
//                                              printf("PMMU: TC = %08x\n", m68k->mmu_tc);

												if (m68k->mmu_tc & 0x80000000)
												{
													m68k->pmmu_enabled = 1;
//                                                  printf("PMMU enabled\n");
												}
												else
												{
													m68k->pmmu_enabled = 0;
//                                                  printf("PMMU disabled\n");
												}

												if (!(modes & 0x100))   // flush ATC on moves to TC, SRP, CRP with FD bit clear
												{
													pmmu_atc_flush(m68k);
												}
												break;

											case 2: // supervisor root pointer
												temp64 = READ_EA_64(m68k, ea);
												m68k->mmu_srp_limit = (temp64>>32) & 0xffffffff;
												m68k->mmu_srp_aptr = temp64 & 0xffffffff;
//                                              printf("PMMU: SRP limit = %08x aptr = %08x\n", m68k->mmu_srp_limit, m68k->mmu_srp_aptr);
												if (!(modes & 0x100))
												{
													pmmu_atc_flush(m68k);
												}
												break;

											case 3: // CPU root pointer
												temp64 = READ_EA_64(m68k, ea);
												m68k->mmu_crp_limit = (temp64>>32) & 0xffffffff;
												m68k->mmu_crp_aptr = temp64 & 0xffffffff;
//                                              printf("PMMU: CRP limit = %08x aptr = %08x\n", m68k->mmu_crp_limit, m68k->mmu_crp_aptr);
												if (!(modes & 0x100))
												{
													pmmu_atc_flush(m68k);
												}
												break;

												case 7: // MC68851 Access Control Register
													if (m68k->cpu_type == CPU_TYPE_020)
													{
														// DomainOS on Apollo DN3000 will only reset this to 0
														UINT16 mmu_ac = READ_EA_16(m68k, ea);
														if (mmu_ac != 0)
														{
															printf("680x0 PMMU: pc=%x PMOVE to mmu_ac=%08x\n",
																	m68k->ppc, mmu_ac);
														}
														break;
													}
													// fall through; unknown PMOVE mode unless MC68020 with MC68851

											default:
												printf("680x0: PMOVE to unknown MMU register %x, PC %x\n", (modes>>10) & 7, m68k->pc);
												break;
										}
										break;

									case 3: // MMU status
										{
											UINT32 temp = READ_EA_32(m68k, ea);
											printf("680x0: unsupported PMOVE %x to MMU status, PC %x\n", temp, m68k->pc);
										}
										break;
								}
							}
							break;

						case 3: // MC68030 to/from status reg
							if (modes & 0x200)
							{
								WRITE_EA_16(m68k, ea, m68k->mmu_sr);
							}
							else
							{
								m68k->mmu_sr = READ_EA_16(m68k, ea);
							}
							break;

						default:
							printf("680x0: unknown PMOVE mode %x (modes %04x) (PC %x)\n", (modes>>13) & 0x7, modes, m68k->pc);
							break;

					}
				}
				break;

			default:
				printf("680x0: unknown PMMU instruction group %d\n", (m68k->ir>>9) & 0x7);
				break;
		}
	}
}


/* Apple HMMU translation is much simpler */
static inline UINT32 hmmu_translate_addr(m68000_base_device *m68k, UINT32 addr_in)
{
	UINT32 addr_out;

	addr_out = addr_in;

	// check if LC 24-bit mode is enabled - this simply blanks out A31, the V8 ignores A30-24 always
	if (m68k->hmmu_enabled == M68K_HMMU_ENABLE_LC)
	{
		addr_out = addr_in & 0xffffff;
	}
	else if (m68k->hmmu_enabled == M68K_HMMU_ENABLE_II) // the original II does a more complex translation
	{
		addr_out = addr_in & 0xffffff;

		if ((addr_out >= 0x800000) && (addr_out <= 0x8fffff))
		{
			addr_out |= 0x40000000; // ROM
		}
		else if ((addr_out >= 0x900000) && (addr_out <= 0xefffff))
		{
			addr_out = 0xf0000000;  // NuBus
			addr_out |= ((addr_in & 0xf00000)<<4);
			addr_out |= (addr_in & 0xfffff);
		}
		else if (addr_out >= 0xf00000)
		{
			addr_out |= 0x50000000; // I/O
		}

		// (RAM is at 0 and doesn't need special massaging)
	}

	return addr_out;
}
