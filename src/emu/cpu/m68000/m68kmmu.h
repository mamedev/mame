/*
    m68kmmu.h - PMMU implementation for 68851/68030/68040
            HMMU implementation for 68020 (II and LC variants)

    By R. Belmont

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.
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
static UINT32 DECODE_EA_32(m68ki_cpu_core *m68k, int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			return REG_A[reg];
		}
		case 3:		// (An)+
		{
			UINT32 ea = EA_AY_PI_32(m68k);
			return ea;
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_32(m68k);
			return ea;
		}
		case 6:		// (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_32(m68k);
			return ea;
		}
		case 7:
		{
			switch (reg)
			{
				case 0:		// (xxx).W
				{
					UINT32 ea = (UINT32)OPER_I_16(m68k);
					return ea;
				}
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					return ea;
				}
				case 2:		// (d16, PC)
				{
					UINT32 ea = EA_PCDI_32(m68k);
					return ea;
				}
				default:	fatalerror("m68k: DECODE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("m68k: DECODE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
	return 0;
}

/*
    pmmu_atc_add: adds this address to the ATC
*/
void pmmu_atc_add(m68ki_cpu_core *m68k, UINT32 logical, UINT32 physical, int fc)
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
    // logerror("ATC[%2d] add: log %08x -> phys %08x (fc=%d)\n", found, (logical>>ps) << ps, (physical >> ps) << ps, fc);
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
void pmmu_atc_flush(m68ki_cpu_core *m68k)
{
	int i;
    // logerror("ATC flush: pc=%08x\n", REG_PPC);

	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		m68k->mmu_atc_tag[i] = 0;
	}

	m68k->mmu_atc_rr = 0;
}


INLINE UINT32 get_dt2_table_entry(m68ki_cpu_core *m68k, UINT32 tptr, UINT8 ptest)
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

INLINE UINT32 get_dt3_table_entry(m68ki_cpu_core *m68k, UINT32 tptr, UINT8 ptest)
{
	UINT32 tbl_entry2 = m68k->program->read_dword(tptr);
	UINT32 tbl_entry = m68k->program->read_dword(tptr + 4);
	UINT32 dt = tbl_entry2 & M68K_MMU_DF_DT;

	m68k->mmu_tmp_sr |= tbl_entry2 & 0x0100 ? M68K_MMU_SR_SUPERVISOR_ONLY : 0;
	m68k->mmu_tmp_sr |= tbl_entry2 & 0x0004 ? M68K_MMU_SR_WRITE_PROTECT : 0;

	if (!ptest)
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
/*INLINE*/ static UINT32 pmmu_translate_addr_with_fc(m68ki_cpu_core *m68k, UINT32 addr_in, UINT8 fc, UINT8 ptest)
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
		if ((addr_in & address_mask) == address_base)
		{
//          logerror("PMMU: pc=%x TT0 fc=%x addr_in=%08x address_mask=%08x address_base=%08x\n", m68k->ppc, fc, addr_in, address_mask, address_base);
			return addr_in;
		}
	}

//  if ((++pmmu_access_count % 10000000) == 0) {
//      logerror("pmmu_translate_addr_with_fc: atc usage = %d%%\n", pmmu_atc_count*100/pmmu_access_count);
//      pmmu_atc_count = pmmu_access_count = 0;
//  }

	// get page size (i.e. # of bits to ignore); ps is 10 or 12 for Apollo, 8 otherwise
	ps = (m68k->mmu_tc >> 20) & 0xf;
	atc_tag = M68K_MMU_ATC_VALID | ((fc &7) << 24) | addr_in >> ps;

	// first see if this is already in the ATC
	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		// if tag bits and function code match, we've got it
		if (m68k->mmu_atc_tag[i] == atc_tag)
		{
			 if (m68k->mmu_tmp_rw || !(m68k->mmu_atc_data[i] & M68K_MMU_ATC_WRITE_PR))
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
				addr_out = (m68k->mmu_atc_data[i]<<8) | (addr_in & ~(~0 << ps));
//              logerror("ATC[%2d] hit: log %08x -> phys %08x  pc=%08x fc=%d\n", i, addr_in, addr_out, REG_PPC, fc);
//              pmmu_atc_count++;
				return addr_out;
			 }
		}
	}

	// if SRP is enabled and we're in supervisor mode, use it
	if ((m68k->mmu_tc & M68K_MMU_TC_SRE) && (fc & 4))
	{
		root_aptr = m68k->mmu_srp_aptr;
		root_limit = m68k->mmu_srp_limit;
	}
	else	// else use the CRP
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

//  logerror("PMMU: tcr %08x limit %08x aptr %08x is %x abits %d bbits %d cbits %d\n", m68k->mmu_tc, root_limit, root_aptr, is, abits, bbits, cbits);

	// get table A offset
	tofs = (addr_in<<is)>>(32-abits);
	tptr = root_aptr & 0xfffffff0;

	// find out what format table A is
	switch (root_limit & M68K_MMU_DF_DT)
	{
		case M68K_MMU_DF_DT0:	// invalid, will cause MMU exception
			m68k->mmu_tmp_sr |= M68K_MMU_SR_INVALID;
			return root_aptr;

		case M68K_MMU_DF_DT1:	// page descriptor, will cause direct mapping
			addr_out = tptr + addr_in;
//          logerror("PMMU: PC=%x root mode %d (addr_in %08x -> %08x)\n", m68k->ppc, M68K_MMU_DF_DT1, addr_in, addr_out);
			return addr_out;

		case M68K_MMU_DF_DT2:	// valid 4 byte descriptors
			tofs *= 4;
//          if (verbose) logerror("PMMU: reading table A entry at %08x\n", tofs + tptr);
			tbl_entry = get_dt2_table_entry(m68k,  tptr + tofs,  ptest);
			tamode = tbl_entry & M68K_MMU_DF_DT;
//          if (verbose) logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tamode, tofs);
			break;

		case M68K_MMU_DF_DT3: // valid 8 byte descriptors
			tofs *= 8;
//          if (verbose) logerror("PMMU: reading table A entries at %08x\n", tofs + tptr);
			tbl_entry = get_dt3_table_entry(m68k,  tofs + tptr,  ptest);
			tamode = tbl_entry & M68K_MMU_DF_DT;
//          if (verbose) logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tamode, tofs);
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
//          if (verbose) logerror("PMMU: reading table B entry at %08x\n", tofs + tptr);
			tbl_entry = get_dt2_table_entry(m68k, tptr + tofs,  ptest);
			tbmode = tbl_entry & M68K_MMU_DF_DT;
//          if (verbose) logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tbmode, tofs);
			break;

		case M68K_MMU_DF_DT3: // 8-byte table B descriptor
			tofs *= 8;
//          if (verbose) logerror("PMMU: reading table B entries at %08x\n", tofs + tptr);
			tbl_entry = get_dt3_table_entry(m68k, tptr + tofs,  ptest);
			tbmode = tbl_entry & M68K_MMU_DF_DT;
			tbl_entry &= ~M68K_MMU_DF_DT;
//          if (verbose) logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tbmode, tofs);
			break;

		case M68K_MMU_DF_DT1:	// early termination descriptor
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
			case M68K_MMU_DF_DT0:	// invalid, will cause MMU exception (but not for ptest)
				m68k->mmu_tmp_sr |= (M68K_MMU_SR_INVALID | M68K_MMU_SR_LEVEL_2);
				// last valid pointer (for ptest)
				addr_out = last_entry_ptr;
				resolved = 1;
				break;

			case M68K_MMU_DF_DT2: // 4-byte table C descriptor
				tofs *= 4;
//              if (verbose) logerror("PMMU: reading table C entry at %08x\n", tofs + tptr);
				tbl_entry = get_dt2_table_entry(m68k, tptr + tofs, ptest);
				tcmode = tbl_entry & M68K_MMU_DF_DT;
//              if (verbose) logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tbmode, tofs);
				break;

			case M68K_MMU_DF_DT3: // 8-byte table C descriptor
				tofs *= 8;
//              if (verbose) logerror("PMMU: reading table C entries at %08x\n", tofs + tptr);
				tbl_entry = get_dt3_table_entry(m68k,  tptr+ tofs,  ptest);
				tcmode = tbl_entry & M68K_MMU_DF_DT;
//              if (verbose) logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tcmode, tofs);
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
			case M68K_MMU_DF_DT0:	// invalid, will cause MMU exception (unless ptest)
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
			}
		}
		else if ((m68k->mmu_tmp_sr & M68K_MMU_SR_SUPERVISOR_ONLY) && !(fc & 4))
		{
			if (++m68k->mmu_tmp_buserror_occurred == 1)
			{
				m68k->mmu_tmp_buserror_address = addr_in;
			}
		}
		else if ((m68k->mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT) && !m68k->mmu_tmp_rw)
		{
			if (++m68k->mmu_tmp_buserror_occurred == 1)
			{
				m68k->mmu_tmp_buserror_address = addr_in;
			}
		}

		if (!m68k->mmu_tmp_buserror_occurred)
		{
			// we add only valid entries
			pmmu_atc_add(m68k, addr_in, addr_out, fc);
		}
	}

	//logerror("PMMU: [%08x] => [%08x]\n", addr_in, addr_out);

	return addr_out;
}

/*
    pmmu_translate_addr: perform 68851/68030-style PMMU address translation
*/
/*INLINE*/ static UINT32 pmmu_translate_addr(m68ki_cpu_core *m68k, UINT32 addr_in)
{
	UINT32 addr_out = pmmu_translate_addr_with_fc(m68k, addr_in, m68k->mmu_tmp_fc, 0);

//  if (m68k->mmu_tmp_buserror_occurred > 0) {
//      logerror("PMMU: pc=%08x sp=%08x va=%08x pa=%08x - invalid Table mode for level=%d (buserror %d)\n",
//              REG_PPC, REG_A[7], addr_in, addr_out, m68k->mmu_tmp_sr & M68K_MMU_SR_LEVEL_3,
//              m68k->mmu_tmp_buserror_occurred);
//  }

	return addr_out;
}

/*
    m68881_mmu_ops: COP 0 MMU opcode handling

*/

void m68881_mmu_ops(m68ki_cpu_core *m68k)
{
	UINT16 modes;
	UINT32 ea = m68k->ir & 0x3f;
	UINT64 temp64;


	// catch the 2 "weird" encodings up front (PBcc)
	if ((m68k->ir & 0xffc0) == 0xf0c0)
	{
		logerror("680x0: unhandled PBcc\n");
		return;
	}
	else if ((m68k->ir & 0xffc0) == 0xf080)
	{
		logerror("680x0: unhandled PBcc\n");
		return;
	}
	else	// the rest are 1111000xxxXXXXXX where xxx is the instruction family
	{
		switch ((m68k->ir>>9) & 0x7)
		{
			case 0:
				modes = OPER_I_16(m68k);

				if ((modes & 0xfde0) == 0x2000)	// PLOAD
				{
					UINT32 ltmp = DECODE_EA_32(m68k, ea);
					UINT32 ptmp;

					ptmp = ltmp;
					if (m68k->pmmu_enabled)
					{
						ptmp = pmmu_translate_addr_with_fc(m68k, ltmp, modes & 0x07, 0);
					}

                    logerror("680x0: PLOADing ATC with logical %08x => phys %08x\n", ltmp, ptmp);
					// FIXME: rw bit?
					pmmu_atc_add(m68k, ltmp, ptmp,  modes & 0x07);
					return;
				}
				else if ((modes & 0xe200) == 0x2000)	// PFLUSH
				{
					pmmu_atc_flush(m68k);
					return;
				}
				else if (modes == 0xa000)	// PFLUSHR
				{
					pmmu_atc_flush(m68k);
					return;
				}
				else if (modes == 0x2800)	// PVALID (FORMAT 1)
				{
					logerror("680x0: unhandled PVALID1\n");
					return;
				}
				else if ((modes & 0xfff8) == 0x2c00)	// PVALID (FORMAT 2)
				{
					logerror("680x0: unhandled PVALID2\n");
					return;
				}
				else if ((modes & 0xe000) == 0x8000)	// PTEST
				{
					UINT32 v_addr = DECODE_EA_32(m68k,  ea);
					UINT32 p_addr;
					UINT32 fc = modes & 0x1f;
					switch (fc >> 3) {
					case 0:
						fc = fc == 0 ? m68k->sfc :  m68k->dfc;
						break;
					case 1:
						fc = REG_D[fc &7] &7;
						break;
					case 2:
						fc &=7;
						break;
					}

					p_addr = pmmu_translate_addr_with_fc(m68k, v_addr, fc, 1);
					m68k->mmu_sr = m68k->mmu_tmp_sr;

//                  logerror("PMMU: pc=%08x sp=%08x va=%08x pa=%08x PTEST fc=%x level=%x mmu_sr=%04x\n",
//                          m68k->ppc, REG_A[7], v_addr, p_addr, fc, (modes >> 10) & 0x07, m68k->mmu_sr);

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
						case 0:	// MC68030/040 form with FD bit
						case 2:	// MC68881 form, FD never set
							if (modes & 0x200)
							{
									switch ((modes>>10) & 0x3f)
									{
										case 0x02: // transparent translation register 0
											WRITE_EA_32(m68k, ea, m68k->mmu_tt0);
//                                          logerror("PMMU: pc=%x PMOVE from mmu_tt0=%08x\n", m68k->ppc, m68k->mmu_tt0);
											break;
										case 0x03: // transparent translation register 1
											WRITE_EA_32(m68k, ea, m68k->mmu_tt1);
//                                          logerror("PMMU: pc=%x PMOVE from mmu_tt1=%08x\n", m68k->ppc, m68k->mmu_tt1);
											break;
										case 0x10:	// translation control register
											WRITE_EA_32(m68k, ea, m68k->mmu_tc);
//                                          logerror("PMMU: pc=%x PMOVE from mmu_tc=%08x\n", m68k->ppc, m68k->mmu_tc);
											break;

										case 0x12: // supervisor root pointer
											WRITE_EA_64(m68k, ea, (UINT64)m68k->mmu_srp_limit<<32 | (UINT64)m68k->mmu_srp_aptr);
//                                          logerror("PMMU: pc=%x PMOVE from SRP limit = %08x, aptr = %08x\n", REG_PPC, m68k->mmu_srp_limit, m68k->mmu_srp_aptr);
											break;

										case 0x13: // CPU root pointer
											WRITE_EA_64(m68k, ea, (UINT64)m68k->mmu_crp_limit<<32 | (UINT64)m68k->mmu_crp_aptr);
//                                          logerror("PMMU: pc=%x PMOVE from CRP limit = %08x, aptr = %08x\n", REG_PPC, m68k->mmu_crp_limit, m68k->mmu_crp_aptr);
											break;

										default:
											logerror("680x0: PMOVE from unknown MMU register %x, PC %x\n", (modes>>10) & 7, m68k->pc);
											break;
								}

							}
							else	// top 3 bits of modes: 010 for this, 011 for status, 000 for transparent translation regs
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
										logerror("680x0: unknown PMOVE case 1, PC %x\n", m68k->pc);
										break;

									case 2:
										switch ((modes>>10) & 7)
										{
											case 0:	// translation control register
												m68k->mmu_tc = READ_EA_32(m68k, ea);
//                                              logerror("PMMU: TC = %08x\n", m68k->mmu_tc);

												if (m68k->mmu_tc & 0x80000000)
												{
													m68k->pmmu_enabled = 1;
//                                                  logerror("PMMU enabled\n");
												}
												else
												{
													m68k->pmmu_enabled = 0;
//                                                  logerror("PMMU disabled\n");
												}

												if (!(modes & 0x100))	// flush ATC on moves to TC, SRP, CRP with FD bit clear
												{
													pmmu_atc_flush(m68k);
												}
												break;

											case 2:	// supervisor root pointer
												temp64 = READ_EA_64(m68k, ea);
												m68k->mmu_srp_limit = (temp64>>32) & 0xffffffff;
												m68k->mmu_srp_aptr = temp64 & 0xffffffff;
//                                              logerror("PMMU: SRP limit = %08x aptr = %08x\n", m68k->mmu_srp_limit, m68k->mmu_srp_aptr);
												if (!(modes & 0x100))
												{
													pmmu_atc_flush(m68k);
												}
												break;

											case 3:	// CPU root pointer
												temp64 = READ_EA_64(m68k, ea);
												m68k->mmu_crp_limit = (temp64>>32) & 0xffffffff;
												m68k->mmu_crp_aptr = temp64 & 0xffffffff;
//                                              logerror("PMMU: CRP limit = %08x aptr = %08x\n", m68k->mmu_crp_limit, m68k->mmu_crp_aptr);
												if (!(modes & 0x100))
												{
													pmmu_atc_flush(m68k);
												}
												break;

											default:
												logerror("680x0: PMOVE to unknown MMU register %x, PC %x\n", (modes>>10) & 7, m68k->pc);
												break;
										}
										break;

									case 3:	// MMU status
										{
											UINT32 temp = READ_EA_32(m68k, ea);
											logerror("680x0: unsupported PMOVE %x to MMU status, PC %x\n", temp, m68k->pc);
										}
										break;
								}
							}
							break;

						case 3:	// MC68030 to/from status reg
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
							logerror("680x0: unknown PMOVE mode %x (modes %04x) (PC %x)\n", (modes>>13) & 0x7, modes, m68k->pc);
							break;

					}
				}
				break;

			default:
				logerror("680x0: unknown PMMU instruction group %d\n", (m68k->ir>>9) & 0x7);
				break;
		}
	}
}


/* Apple HMMU translation is much simpler */
INLINE UINT32 hmmu_translate_addr(m68ki_cpu_core *m68k, UINT32 addr_in)
{
	UINT32 addr_out;

	addr_out = addr_in;

	// check if LC 24-bit mode is enabled - this simply blanks out A31, the V8 ignores A30-24 always
	if (m68k->hmmu_enabled == M68K_HMMU_ENABLE_LC)
	{
		addr_out = addr_in & 0xffffff;
	}
	else if (m68k->hmmu_enabled == M68K_HMMU_ENABLE_II)	// the original II does a more complex translation
	{
		addr_out = addr_in & 0xffffff;

		if ((addr_in >= 0x800000) && (addr_in <= 0x8fffff))
		{
			addr_out |= 0x40000000;	// ROM
		}
		else if ((addr_in >= 0x900000) && (addr_in <= 0xefffff))
		{
			addr_out |= 0xf0ff0000;	// NuBus
			addr_out |= ((addr_in & 0xf00000)<<4);
		}
		else if (addr_in >= 0xf00000)
		{
			addr_out |= 0x50000000;	// I/O
		}

		// (RAM is at 0 and doesn't need special massaging)
	}

	return addr_out;
}
