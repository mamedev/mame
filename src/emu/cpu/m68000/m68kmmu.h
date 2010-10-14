/*
    m68kmmu.h - PMMU implementation for 68851/68030/68040
            HMMU implementation for 68020 (II and LC variants)

    By R. Belmont

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.
*/

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
void pmmu_atc_add(m68ki_cpu_core *m68k, UINT32 logical, UINT32 physical)
{
	int i, found, curfc;

	curfc = (m68ki_get_sr(m68k) & 0x2000) ? 4 : 0;
	curfc |= 1;	// program vs. data space is not implemented

	// first see if this is already in the cache
	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		// if tag bits and function code match, don't add
		if (((m68k->mmu_atc_tag[i] & 0xffffff) == (logical>>8)) && (((m68k->mmu_atc_tag[i]>>24) & 7) == curfc))
		{
			return;
		}
	}

	// find an open entry
	found = -1;
	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		if (!(m68k->mmu_atc_tag[i] & 0x80000000))
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
//  logerror("ATC[%d] add: log %08x -> phys %08x\n", found, logical, physical);
	m68k->mmu_atc_tag[found] = (logical>>8) | (curfc<<24) | 0x80000000;
	m68k->mmu_atc_data[found] = (physical>>8);
}

/*
    pmmu_atc_flush: flush entire ATC

    7fff0003 001ffd10 80f05750 is what should load
*/
void pmmu_atc_flush(m68ki_cpu_core *m68k)
{
	int i;

	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		m68k->mmu_atc_tag[i] = 0;
	}

	m68k->mmu_atc_rr = 0;
}

/*
    pmmu_translate_addr: perform 68851/68030-style PMMU address translation
*/
INLINE UINT32 pmmu_translate_addr(m68ki_cpu_core *m68k, UINT32 addr_in)
{
	UINT32 addr_out, tbl_entry = 0, tbl_entry2, tamode = 0, tbmode = 0, tcmode = 0;
	UINT32 root_aptr, root_limit, tofs, is, abits, bbits, cbits;
	UINT32 resolved, tptr, shift;
	int curfc, i;
//  int verbose = 0;

	resolved = 0;
	addr_out = addr_in;

	curfc = (m68ki_get_sr(m68k) & 0x2000) ? 4 : 0;
	curfc |= 1;	// program vs. data space is not implemented

	// first see if this is already in the ATC
	for (i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		// if tag bits and function code match, we've got it
		if (((m68k->mmu_atc_tag[i] & 0xffffff) == (addr_in>>8)) && (((m68k->mmu_atc_tag[i]>>24) & 7) == curfc))
		{
			addr_out = (m68k->mmu_atc_data[i]<<8) | (addr_in & 0xff);
//          logerror("ATC[%d] hit: log %08x -> phys %08x\n", i, addr_in, addr_out);
			return addr_out;
		}
	}

//  if (addr_in == 0x5fa04c) verbose = 1;

	// if SRP is enabled and we're in supervisor mode, use it
	if ((m68k->mmu_tc & 0x02000000) && (m68ki_get_sr(m68k) & 0x2000))
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
	is = (m68k->mmu_tc>>16) & 0xf;
	abits = (m68k->mmu_tc>>12)&0xf;
	bbits = (m68k->mmu_tc>>8)&0xf;
	cbits = (m68k->mmu_tc>>4)&0xf;

	//logerror("PMMU: tcr %08x limit %08x aptr %08x is %x abits %d bbits %d cbits %d\n", m68k->mmu_tc, root_limit, root_aptr, is, abits, bbits, cbits);

	// get table A offset
	tofs = (addr_in<<is)>>(32-abits);

	// find out what format table A is
	switch (root_limit & 3)
	{
		case 0:	// invalid, should cause MMU exception
		case 1:	// page descriptor, should cause direct mapping
			logerror("680x0 PMMU: Unhandled root mode %d, not translating (addr_in %x)\n", root_limit & 3, addr_in);
			logerror("          : aptr %08x limit %08x\n", root_aptr, root_limit);
			return addr_in;
			break;

		case 2:	// valid 4 byte descriptors
			tofs *= 4;
//          if (verbose) logerror("PMMU: reading table A entry at %08x\n", tofs + (root_aptr & 0xfffffffc));
			tbl_entry = m68k->program->read_dword(tofs + (root_aptr & 0xfffffffc));
			tamode = tbl_entry & 3;
//          if (verbose) logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tamode, tofs);
			break;

		case 3: // valid 8 byte descriptors
			tofs *= 8;
//          if (verbose) logerror("PMMU: reading table A entries at %08x\n", tofs + (root_aptr & 0xfffffffc));
			tbl_entry2 = m68k->program->read_dword(tofs + (root_aptr & 0xfffffffc));
			tbl_entry = m68k->program->read_dword(tofs + (root_aptr & 0xfffffffc)+4);
			tamode = tbl_entry2 & 3;
//          if (verbose) logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tamode, tofs);
			break;
	}

	// get table B offset and pointer
	tofs = (addr_in<<(is+abits))>>(32-bbits);
	tptr = tbl_entry & 0xfffffff0;

	// find out what format table B is, if any
	switch (tamode)
	{
		case 0: // invalid, should cause MMU exception
			fatalerror("680x0 PMMU: Unhandled Table A mode %d (addr_in %08x PC %x)\n", tamode, addr_in, m68k->pc);
			break;

		case 2: // 4-byte table B descriptor
			tofs *= 4;
//          if (verbose) logerror("PMMU: reading table B entry at %08x\n", tofs + tptr);
			tbl_entry = m68k->program->read_dword(tofs + tptr);
			tbmode = tbl_entry & 3;
//          if (verbose) logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tbmode, tofs);
			break;

		case 3: // 8-byte table B descriptor
			tofs *= 8;
//          if (verbose) logerror("PMMU: reading table B entries at %08x\n", tofs + tptr);
			tbl_entry2 = m68k->program->read_dword(tofs + tptr);
			tbl_entry = m68k->program->read_dword(tofs + tptr + 4);
			tbmode = tbl_entry2 & 3;
//          if (verbose) logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tbmode, tofs);
			break;

		case 1:	// early termination descriptor
			tbl_entry &= 0xffffff00;

			shift = is+abits;
			addr_out = ((addr_in<<shift)>>shift) + tbl_entry;
			resolved = 1;
			break;
	}

	// if table A wasn't early-out, continue to process table B
	if (!resolved)
	{
		// get table C offset and pointer
		tofs = (addr_in<<(is+abits+bbits))>>(32-cbits);
		tptr = tbl_entry & 0xfffffff0;

		switch (tbmode)
		{
			case 0:	// invalid, should cause MMU exception
				fatalerror("680x0 PMMU: Unhandled Table B mode %d (addr_in %08x PC %x)\n", tbmode, addr_in, m68k->pc);
				break;

			case 2: // 4-byte table C descriptor
				tofs *= 4;
//              if (verbose) logerror("PMMU: reading table C entry at %08x\n", tofs + tptr);
				tbl_entry = m68k->program->read_dword(tofs + tptr);
				tcmode = tbl_entry & 3;
//              if (verbose) logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tbmode, tofs);
				break;

			case 3: // 8-byte table C descriptor
				tofs *= 8;
//              if (verbose) logerror("PMMU: reading table C entries at %08x\n", tofs + tptr);
				tbl_entry2 = m68k->program->read_dword(tofs + tptr);
				tbl_entry = m68k->program->read_dword(tofs + tptr + 4);
				tcmode = tbl_entry2 & 3;
//              if (verbose) logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tbmode, tofs);
				break;

			case 1: // termination descriptor
				tbl_entry &= 0xffffff00;

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
			case 0:	// invalid, should cause MMU exception
			case 2: // 4-byte table D descriptor
			case 3: // 8-byte table D descriptor
				fatalerror("680x0 PMMU: Unhandled Table C mode %d (addr_in %08x PC %x)\n", tbmode, addr_in, m68k->pc);
				break;

			case 1: // termination descriptor
				tbl_entry &= 0xffffff00;

				shift = is+abits+bbits+cbits;
				addr_out = ((addr_in<<shift)>>shift) + tbl_entry;
				resolved = 1;
				break;
		}
	}

	pmmu_atc_add(m68k, addr_in, addr_out);

	//logerror("PMMU: [%08x] => [%08x]\n", addr_in, addr_out);

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
						ptmp = pmmu_translate_addr(m68k, ltmp);
					}

//                  logerror("680x0: PLOADing ATC with logical %08x => phys %08x\n", ltmp, ptmp);

					pmmu_atc_add(m68k, ltmp, ptmp);
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
					logerror("680x0: unhandled PTEST\n");
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
								switch ((modes>>10) & 7)
								{
									case 0:	// translation control register
										WRITE_EA_32(m68k, ea, m68k->mmu_tc);
										break;

									case 2: // supervisor root pointer
										WRITE_EA_64(m68k, ea, (UINT64)m68k->mmu_srp_limit<<32 | (UINT64)m68k->mmu_srp_aptr);
										break;

									case 3: // CPU root pointer
										WRITE_EA_64(m68k, ea, (UINT64)m68k->mmu_crp_limit<<32 | (UINT64)m68k->mmu_crp_aptr);
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
								WRITE_EA_32(m68k, ea, m68k->mmu_sr);
							}
							else
							{
								m68k->mmu_sr = READ_EA_32(m68k, ea);
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

	// check if LC mode is enabled
	if (m68k->hmmu_enabled == M68K_HMMU_ENABLE_LC)
	{
		addr_in &= 0xffffff;
		addr_out &= 0xffffff;

		if ((addr_in >= 0xa00000) && (addr_in <= 0xdfffff))
		{
			addr_out |= 0x40000000;
		}
		if ((addr_in >= 0xe00000) && (addr_in <= 0xefffff))
		{
			addr_out &= 0xfffff;
			addr_out |= 0xfe000000;
		}
		else if (addr_in >= 0xf00000)
		{
			addr_out |= 0x50000000;
		}
	}

	return addr_out;
}
