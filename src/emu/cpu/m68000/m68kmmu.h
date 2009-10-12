/*
    m68kmmu.h - PMMU implementation for 68851/68030/68040

    By R. Belmont

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.
*/

/*
	pmmu_translate_addr: perform 68851/68030-style PMMU address translation
*/
INLINE UINT32 pmmu_translate_addr(m68ki_cpu_core *m68k, UINT32 addr_in)
{
	UINT32 addr_out, tbl_entry, tbl_entry2, tamode, tbmode;
	UINT32 root_aptr, root_limit, tofs, is, abits, bbits, cbits;
	UINT32 resolved, tptr, shift;

	resolved = 0;
	addr_out = addr_in;

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

//	logerror("PMMU: tcr %08x limit %08x aptr %08x is %x abits %d bbits %d cbits %d\n", m68k->mmu_tc, root_limit, root_aptr, is, abits, bbits, cbits);

	// get table A offset
	tofs = (addr_in<<is)>>(32-abits);
	
	// find out what format table A is	
	switch (root_limit & 3)
	{
		case 0:	// invalid, should cause MMU exception
		case 1:	// page descriptor, should cause direct mapping
			fatalerror("680x0 PMMU: Unhandled root mode\n");
			break;

		case 2:	// valid 4 byte descriptors
			tofs *= 4;
//			logerror("PMMU: reading table A entry at %08x\n", tofs + (root_aptr & 0xfffffffc));
			tbl_entry = memory_read_dword_32be(m68k->program, tofs + (root_aptr & 0xfffffffc));
			tamode = tbl_entry & 3;
//			logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tamode, tofs);
			break;

		case 3: // valid 8 byte descriptors
			tofs *= 8;
//			logerror("PMMU: reading table A entries at %08x\n", tofs + (root_aptr & 0xfffffffc));
			tbl_entry2 = memory_read_dword_32be(m68k->program, tofs + (root_aptr & 0xfffffffc));
			tbl_entry = memory_read_dword_32be(m68k->program, tofs + (root_aptr & 0xfffffffc)+4);
			tamode = tbl_entry2 & 3;
//			logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tamode, tofs);
			break;
	}

	// get table B offset and pointer
	tofs = (addr_in<<(is+abits))>>(32-bbits);
	tptr = tbl_entry & 0xfffffff0;

	// find out what format table B is, if any
	switch (tamode)
	{
		case 0: // invalid, should cause MMU exception
			fatalerror("680x0 PMMU: Unhandled Table A mode %d (addr_in %08x)\n", tamode, addr_in);
			break;
		
		case 2: // 4-byte table B descriptor
			tofs *= 4;
//			logerror("PMMU: reading table B entry at %08x\n", tofs + tptr);
			tbl_entry = memory_read_dword_32be(m68k->program, tofs + tptr);
			tbmode = tbl_entry & 3;
//			logerror("PMMU: addr %08x entry %08x mode %x tofs %x\n", addr_in, tbl_entry, tbmode, tofs);
			break;

		case 3: // 8-byte table B descriptor
			tofs *= 8;
//			logerror("PMMU: reading table B entries at %08x\n", tofs + tptr);
			tbl_entry2 = memory_read_dword_32be(m68k->program, tofs + tptr);
			tbl_entry = memory_read_dword_32be(m68k->program, tofs + tptr + 4);
			tbmode = tbl_entry2 & 3;
//			logerror("PMMU: addr %08x entry %08x entry2 %08x mode %x tofs %x\n", addr_in, tbl_entry, tbl_entry2, tbmode, tofs);
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
		switch (tbmode)
		{
			case 0:	// invalid, should cause MMU exception
			case 2: // 4-byte table C descriptor
			case 3: // 8-byte table C descriptor
				fatalerror("680x0 PMMU: Unhandled Table B mode %d (addr_in %08x)\n", tbmode, addr_in);
				break;

			case 1: // early termination descriptor
				tbl_entry &= 0xffffff00;

				shift = is+abits+bbits;
				addr_out = ((addr_in<<shift)>>shift) + tbl_entry;
				resolved = 1;
				break;
		}
	}

//	if ((addr_in < 0x40000000) || (addr_in > 0x4fffffff)) printf("PMMU: [%08x] => [%08x]\n", addr_in, addr_out);

	return addr_out;
}

/*

maincpu at 40804366: called unimplemented instruction f000 (cpgen)
PMMU: tcr 80f05570 limit 7fff0003 aptr 043ffcc0 is 0
PMMU: reading table A entries at 043ffce0
PMMU: addr 4080438a entry 00000000 entry2 7ffffc18 mode 0 aofs 20
680x0 PMMU: Unhandled Table A mode 0

enable, PS = f

tblA @ 043ffcc0:

043ffcc0 0001fc0a 043ffcb0  =>  00000019 04000019
043ffcc8 7ffffc18 00000000     
043ffcd0 7ffffc18 00000000     
043ffcd8 7ffffc18 00000000     
043ffce0 7ffffc18 00000000     
043ffce8 7ffffc18 00000000     
043ffcf0 7ffffc18 00000000     
043ffcf8 7ffffc18 00000000     
043ffd00 7ffffc19 40000000
043ffd08 7ffffc19 48000000
043ffd10 7ffffc59 50000000
043ffd18 7ffffc59 58000000
043ffd20 7ffffc59 60000000
043ffd28 7ffffc59 68000000
043ffd30 7ffffc59 70000000
043ffd38 7ffffc59 78000000
043ffd40 7ffffc59 80000000
043ffd48 7ffffc59 88000000
043ffd50 7ffffc59 90000000
043ffd58 7ffffc59 98000000
043ffd60 7ffffc59 a0000000
043ffd68 7ffffc59 a8000000
043ffd70 7ffffc59 b0000000
043ffd78 7ffffc59 b8000000
043ffd80 7ffffc59 c0000000
043ffd88 7ffffc59 c8000000
043ffd90 7ffffc59 d0000000
043ffd98 7ffffc59 d8000000
043ffda0 7ffffc59 e0000000
043ffda8 7ffffc59 e8000000
043ffdb0 7ffffc59 f0000000
043ffdb8 7ffffc59 f8000000

*/

