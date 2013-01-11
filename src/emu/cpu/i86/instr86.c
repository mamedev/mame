/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

/*
 * file will be included in all cpu variants
 * put non i86 instructions in own files (i286, i386, nec)
 * function renaming will be added when necessary
 * timing value should move to separate array
 */

/*
    PHS - 2010-12-29

    Moved several instruction stubs so that they are compiled separately for
    the 8086 and 80186. The instructions affected are :

    _pop_ss, _es, _cs, _ss, _ds, _mov_sregw and _sti

    This is because they call the next instruction directly as it cannot be
    interrupted. If they are not compiled separately when executing on an
    80186, the wrong set of instructions are used (the 8086 set). This has
    the serious effect of ignoring the next instruction, as invalid, *IF*
    it is an 80186 specific instruction.

*/

#undef ICOUNT

#define ICOUNT cpustate->icount


#if !defined(I80186)
static void PREFIX86(_interrupt)(i8086_state *cpustate, unsigned int_num)
{
	unsigned dest_seg, dest_off;
	WORD ip = cpustate->pc - cpustate->base[CS];

	if (int_num == -1)
		int_num = (*cpustate->irq_callback)(cpustate->device, 0);

#ifdef I80286
	if (PM) {
		i80286_interrupt_descriptor(cpustate, int_num, 0, 0);
	} else {
#endif
		dest_off = ReadWord(int_num*4);
		dest_seg = ReadWord(int_num*4+2);

		PREFIX(_pushf(cpustate));
		cpustate->TF = cpustate->IF = 0;
		PUSH(cpustate->sregs[CS]);
		PUSH(ip);
		cpustate->sregs[CS] = (WORD)dest_seg;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->base[CS] + dest_off) & AMASK;
		CHANGE_PC(cpustate->pc);
#ifdef I80286
	}
#endif
	cpustate->extra_cycles += timing.exception;
}

static void PREFIX86(_trap)(i8086_state *cpustate)
{
	PREFIX(_instruction)[FETCHOP](cpustate);
	PREFIX(_interrupt)(cpustate, 1);
}
#endif

#ifndef I80186
static void PREFIX86(_rotate_shift_Byte)(i8086_state *cpustate, unsigned ModRM, unsigned count, unsigned src)
{
//  unsigned src = (unsigned)GetRMByte(ModRM);
	unsigned dst=src;

	if (count==0)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m8_base;
	}
	else if (count==1)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m8_1;

		switch (ModRM & 0x38)
		{
		case 0x00:  /* ROL eb,1 */
			cpustate->CarryVal = src & 0x80;
			dst=(src<<1)+CF;
			PutbackRMByte(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x08:  /* ROR eb,1 */
			cpustate->CarryVal = src & 0x01;
			dst = ((CF<<8)+src) >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x10:  /* RCL eb,1 */
			dst=(src<<1)+CF;
			PutbackRMByte(ModRM,dst);
			SetCFB(dst);
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x18:  /* RCR eb,1 */
			dst = ((CF<<8)+src) >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x20:  /* SHL eb,1 */
		case 0x30:
			dst = src << 1;
			PutbackRMByte(ModRM,dst);
			SetCFB(dst);
			cpustate->OverVal = (src^dst)&0x80;
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 0x28:  /* SHR eb,1 */
			dst = src >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = src & 0x80;
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 0x38:  /* SAR eb,1 */
			dst = ((INT8)src) >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = 0;
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		}
	}
	else
	{
		int tmpcf = CF;
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base + (timing.rot_reg_bit * count) : timing.rot_m8_base + (timing.rot_m8_bit * count);

		switch (ModRM & 0x38)
		{
		case 0x00:  /* ROL eb,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x80;
				dst = (dst << 1) + CF;
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x08:  /* ROR eb,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 7);
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x10:  /* RCL eb,count */
			for (; count > 0; count--)
			{
				dst = (dst << 1) + tmpcf;
				tmpcf = (int)((dst & 0x100) != 0);
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x18:  /* RCR eb,count */
			for (; count > 0; count--)
			{
				dst = (tmpcf<<8)+dst;
				tmpcf = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x20:
		case 0x30:  /* SHL eb,count */
			for(int i=0;i<count;i++) dst<<= 1;
			SetCFB(dst);
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x28:  /* SHR eb,count */
			for(int i=0;i<count-1;i++) dst>>= 1;
			cpustate->CarryVal = dst & 0x1;
			dst >>= 1;
			SetSZPF_Byte(dst);
			cpustate->AuxVal = 1;
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x38:  /* SAR eb,count */
			for(int i=0;i<count-1;i++) dst = ((INT8)dst) >> 1;
			cpustate->CarryVal = dst & 0x1;
			dst = ((INT8)((BYTE)dst)) >> 1;
			SetSZPF_Byte(dst);
			cpustate->AuxVal = 1;
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		}
	}
}

static void PREFIX86(_rotate_shift_Word)(i8086_state *cpustate, unsigned ModRM, unsigned count, unsigned src)
{
//  unsigned src = GetRMWord(ModRM);
	unsigned dst=src;

	if (count==0)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m16_base;
	}
	else if (count==1)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m16_1;

		switch (ModRM & 0x38)
		{
#if 0
		case 0x00:  /* ROL ew,1 */
			tmp2 = (tmp << 1) + CF;
			SetCFW(tmp2);
			cpustate->OverVal = !(!(tmp & 0x4000)) != CF;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x08:  /* ROR ew,1 */
			cpustate->CarryVal = tmp & 0x01;
			tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
			cpustate->OverVal = !(!(tmp & 0x8000)) != CF;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x10:  /* RCL ew,1 */
			tmp2 = (tmp << 1) + CF;
			SetCFW(tmp2);
			cpustate->OverVal = (tmp ^ (tmp << 1)) & 0x8000;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x18:  /* RCR ew,1 */
			tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
			cpustate->OverVal = !(!(tmp & 0x8000)) != CF;
			cpustate->CarryVal = tmp & 0x01;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x20:  /* SHL ew,1 */
		case 0x30:
			tmp <<= 1;

			SetCFW(tmp);
			SetOFW_Add(tmp,tmp2,tmp2);
			cpustate->AuxVal = 1;
			SetSZPF_Word(tmp);

			PutbackRMWord(ModRM,tmp);
			break;
		case 0x28:  /* SHR ew,1 */
			cpustate->CarryVal = tmp & 0x01;
			cpustate->OverVal = tmp & 0x8000;

			tmp2 = tmp >> 1;

			SetSZPF_Word(tmp2);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,tmp2);
			break;
			case 0x38:  /* SAR ew,1 */
			cpustate->CarryVal = tmp & 0x01;
			cpustate->OverVal = 0;

			tmp2 = (tmp >> 1) | (tmp & 0x8000);

			SetSZPF_Word(tmp2);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,tmp2);
			break;
#else
		case 0x00:  /* ROL ew,1 */
			cpustate->CarryVal = src & 0x8000;
			dst=(src<<1)+CF;
			PutbackRMWord(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x08:  /* ROR ew,1 */
			cpustate->CarryVal = src & 0x01;
			dst = ((CF<<16)+src) >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x10:  /* RCL ew,1 */
			dst=(src<<1)+CF;
			PutbackRMWord(ModRM,dst);
			SetCFW(dst);
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x18:  /* RCR ew,1 */
			dst = ((CF<<16)+src) >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x20:  /* SHL ew,1 */
		case 0x30:
			dst = src << 1;
			PutbackRMWord(ModRM,dst);
			SetCFW(dst);
			cpustate->OverVal = (src^dst)&0x8000;
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 0x28:  /* SHR ew,1 */
			dst = src >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = src & 0x8000;
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 0x38:  /* SAR ew,1 */
			dst = ((INT16)src) >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = 0;
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			break;
#endif
		}
	}
	else
	{
		int tmpcf = CF;
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base + (timing.rot_reg_bit * count) : timing.rot_m8_base + (timing.rot_m16_bit * count);

		switch (ModRM & 0x38)
		{
		case 0x00:  /* ROL ew,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x8000;
				dst = (dst << 1) + CF;
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x08:  /* ROR ew,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 15);
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x10:  /* RCL ew,count */
			for (; count > 0; count--)
			{
				dst = (dst << 1) + tmpcf;
				tmpcf = (int)((dst & 0x10000) != 0);
			}
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x18:  /* RCR ew,count */
			for (; count > 0; count--)
			{
				dst = dst + (tmpcf << 16);
				tmpcf = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x20:
		case 0x30:  /* SHL ew,count */
			for(int i=0;i<count;i++) dst<<= 1;
			SetCFW(dst);
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			PutbackRMWord(ModRM,dst);
			break;
		case 0x28:  /* SHR ew,count */
			for(int i=0;i<count-1;i++) dst>>= 1;
			cpustate->CarryVal = dst & 0x1;
			dst >>= 1;
			SetSZPF_Word(dst);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,dst);
			break;
		case 0x38:  /* SAR ew,count */
			for(int i=0;i<count-1;i++) dst = ((INT16)dst) >> 1;
			cpustate->CarryVal = dst & 0x01;
			dst = ((INT16)((WORD)dst)) >> 1;
			SetSZPF_Word(dst);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,dst);
			break;
		}
	}
}
#endif

static void PREFIX(rep)(i8086_state *cpustate,int flagval)
{
	/* Handles rep- and repnz- prefixes. flagval is the value of ZF for the
	     loop  to continue for CMPS and SCAS instructions. */

	unsigned next = FETCHOP;

	switch(next)
	{
	case 0x26:  /* ES: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = ES;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
	case 0x2e:  /* CS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = CS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
	case 0x36:  /* SS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = SS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
	case 0x3e:  /* DS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = DS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
#ifndef I8086
	case 0x6c:  /* REP INSB */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_ins8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemB(ES,cpustate->regs.w[DI],read_port_byte(cpustate->regs.w[DX]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			ICOUNT -= timing.rep_ins8_count;
		}
		break;
	case 0x6d:  /* REP INSW */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_ins16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemW(ES,cpustate->regs.w[DI],read_port_word(cpustate->regs.w[DX]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_ins16_count;
		}
		break;
	case 0x6e:  /* REP OUTSB */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_outs8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			write_port_byte(cpustate->regs.w[DX],GetMemB(DS,cpustate->regs.w[SI]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += cpustate->DirVal; /* GOL 11/27/01 */
			ICOUNT -= timing.rep_outs8_count;
		}
		break;
	case 0x6f:  /* REP OUTSW */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_outs16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			write_port_word(cpustate->regs.w[DX],GetMemW(DS,cpustate->regs.w[SI]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal; /* GOL 11/27/01 */
			ICOUNT -= timing.rep_outs16_count;
		}
		break;
#endif
	case 0xa4:  /* REP MOVSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_movs8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			BYTE tmp;

			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			tmp = GetMemB(DS,cpustate->regs.w[SI]);
			PutMemB(ES,cpustate->regs.w[DI], tmp);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			cpustate->regs.w[SI] += cpustate->DirVal;
			ICOUNT -= timing.rep_movs8_count;
		}
		break;
	case 0xa5:  /* REP MOVSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_movs16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			WORD tmp;

			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			tmp = GetMemW(DS,cpustate->regs.w[SI]);
			PutMemW(ES,cpustate->regs.w[DI], tmp);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_movs16_count;
		}
		break;
	case 0xa6:  /* REP(N)E CMPSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_cmps8_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned dst, src;

			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			dst = GetMemB(ES, cpustate->regs.w[DI]);
			src = GetMemB(DS, cpustate->regs.w[SI]);
			SUBB(src,dst); /* opposite of the usual convention */
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			cpustate->regs.w[SI] += cpustate->DirVal;
			ICOUNT -= timing.rep_cmps8_count;
		}
		break;
	case 0xa7:  /* REP(N)E CMPSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_cmps16_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned dst, src;

			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			dst = GetMemW(ES, cpustate->regs.w[DI]);
			src = GetMemW(DS, cpustate->regs.w[SI]);
			SUBW(src,dst); /* opposite of the usual convention */
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_cmps16_count;
		}
		break;
	case 0xaa:  /* REP STOSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_stos8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemB(ES,cpustate->regs.w[DI],cpustate->regs.b[AL]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			ICOUNT -= timing.rep_stos8_count;
		}
		break;
	case 0xab:  /* REP STOSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_stos16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemW(ES,cpustate->regs.w[DI],cpustate->regs.w[AX]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_stos16_count;
		}
		break;
	case 0xac:  /* REP LODSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_lods8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			cpustate->regs.b[AL] = GetMemB(DS,cpustate->regs.w[SI]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += cpustate->DirVal;
			ICOUNT -= timing.rep_lods8_count;
		}
		break;
	case 0xad:  /* REP LODSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_lods16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			cpustate->regs.w[AX] = GetMemW(DS,cpustate->regs.w[SI]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_lods16_count;
		}
		break;
	case 0xae:  /* REP(N)E SCASB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_scas8_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned src, dst;

			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			src = GetMemB(ES, cpustate->regs.w[DI]);
			dst = cpustate->regs.b[AL];
			SUBB(dst,src);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			ICOUNT -= timing.rep_scas8_count;
		}
		break;
	case 0xaf:  /* REP(N)E SCASW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_scas16_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned src, dst;

			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			src = GetMemW(ES, cpustate->regs.w[DI]);
			dst = cpustate->regs.w[AX];
			SUBW(dst,src);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_scas16_count;
		}
		break;
	default:
		PREFIX(_instruction)[next](cpustate);
	}
}

#ifndef I80186
static void PREFIX86(_add_br8)(i8086_state *cpustate)    /* Opcode 0x00 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ADDB(dst,src);
	PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_add_wr16)(i8086_state *cpustate)    /* Opcode 0x01 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ADDW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_add_r8b)(i8086_state *cpustate)    /* Opcode 0x02 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ADDB(dst,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_add_r16w)(i8086_state *cpustate)    /* Opcode 0x03 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ADDW(dst,src);
	RegWord(ModRM)=dst;
}


static void PREFIX86(_add_ald8)(i8086_state *cpustate)    /* Opcode 0x04 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ADDB(dst,src);
	cpustate->regs.b[AL]=dst;
}


static void PREFIX86(_add_axd16)(i8086_state *cpustate)    /* Opcode 0x05 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ADDW(dst,src);
	cpustate->regs.w[AX]=dst;
}


static void PREFIX86(_push_es)(i8086_state *cpustate)    /* Opcode 0x06 */
{
	ICOUNT -= timing.push_seg;
	PUSH(cpustate->sregs[ES]);
}


static void PREFIX86(_pop_es)(i8086_state *cpustate)    /* Opcode 0x07 */
{
#ifdef I80286
	i80286_pop_seg(cpustate,ES);
#else
	POP(cpustate->sregs[ES]);
	cpustate->base[ES] = SegBase(ES);
#endif
	ICOUNT -= timing.pop_seg;
}

static void PREFIX86(_or_br8)(i8086_state *cpustate)    /* Opcode 0x08 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ORB(dst,src);
	PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_or_wr16)(i8086_state *cpustate)    /* Opcode 0x09 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ORW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_or_r8b)(i8086_state *cpustate)    /* Opcode 0x0a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ORB(dst,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_or_r16w)(i8086_state *cpustate)    /* Opcode 0x0b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ORW(dst,src);
	RegWord(ModRM)=dst;
}

static void PREFIX86(_or_ald8)(i8086_state *cpustate)    /* Opcode 0x0c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ORB(dst,src);
	cpustate->regs.b[AL]=dst;
}

static void PREFIX86(_or_axd16)(i8086_state *cpustate)    /* Opcode 0x0d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ORW(dst,src);
	cpustate->regs.w[AX]=dst;
}

static void PREFIX86(_push_cs)(i8086_state *cpustate)    /* Opcode 0x0e */
{
	ICOUNT -= timing.push_seg;
	PUSH(cpustate->sregs[CS]);
}

#ifndef I80286
static void PREFIX86(_pop_cs)(i8086_state *cpustate)    /* Opcode 0x0f */
{
	int ip = cpustate->pc - cpustate->base[CS];
	ICOUNT -= timing.push_seg;
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	CHANGE_PC(cpustate->pc);
}
#endif

static void PREFIX86(_adc_br8)(i8086_state *cpustate)    /* Opcode 0x10 */
{
	int tmpcf;
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src+=CF;
	ADCB(dst,src,tmpcf);
	PutbackRMByte(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

static void PREFIX86(_adc_wr16)(i8086_state *cpustate)    /* Opcode 0x11 */
{
	int tmpcf;
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src+=CF;
	ADCW(dst,src,tmpcf);
	PutbackRMWord(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

static void PREFIX86(_adc_r8b)(i8086_state *cpustate)    /* Opcode 0x12 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src+=CF;
	ADDB(dst,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_adc_r16w)(i8086_state *cpustate)    /* Opcode 0x13 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src+=CF;
	ADDW(dst,src);
	RegWord(ModRM)=dst;
}

static void PREFIX86(_adc_ald8)(i8086_state *cpustate)    /* Opcode 0x14 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	src+=CF;
	ADDB(dst,src);
	cpustate->regs.b[AL] = dst;
}

static void PREFIX86(_adc_axd16)(i8086_state *cpustate)    /* Opcode 0x15 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	src+=CF;
	ADDW(dst,src);
	cpustate->regs.w[AX]=dst;
}

static void PREFIX86(_push_ss)(i8086_state *cpustate)    /* Opcode 0x16 */
{
	PUSH(cpustate->sregs[SS]);
	ICOUNT -= timing.push_seg;
}

static void PREFIX86(_sbb_br8)(i8086_state *cpustate)    /* Opcode 0x18 */
{
	int tmpcf;
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src+=CF;
	SBBB(dst,src,tmpcf);
	PutbackRMByte(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

static void PREFIX86(_sbb_wr16)(i8086_state *cpustate)    /* Opcode 0x19 */
{
	int tmpcf;
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src+=CF;
	SBBW(dst,src,tmpcf);
	PutbackRMWord(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

static void PREFIX86(_sbb_r8b)(i8086_state *cpustate)    /* Opcode 0x1a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src+=CF;
	SUBB(dst,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_sbb_r16w)(i8086_state *cpustate)    /* Opcode 0x1b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src+=CF;
	SUBW(dst,src);
	RegWord(ModRM)= dst;
}

static void PREFIX86(_sbb_ald8)(i8086_state *cpustate)    /* Opcode 0x1c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	src+=CF;
	SUBB(dst,src);
	cpustate->regs.b[AL] = dst;
}

static void PREFIX86(_sbb_axd16)(i8086_state *cpustate)    /* Opcode 0x1d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	src+=CF;
	SUBW(dst,src);
	cpustate->regs.w[AX]=dst;
}

static void PREFIX86(_push_ds)(i8086_state *cpustate)    /* Opcode 0x1e */
{
	PUSH(cpustate->sregs[DS]);
	ICOUNT -= timing.push_seg;
}

static void PREFIX86(_pop_ds)(i8086_state *cpustate)    /* Opcode 0x1f */
{
#ifdef I80286
	i80286_pop_seg(cpustate,DS);
#else
	POP(cpustate->sregs[DS]);
	cpustate->base[DS] = SegBase(DS);
#endif
	ICOUNT -= timing.push_seg;
}

static void PREFIX86(_and_br8)(i8086_state *cpustate)    /* Opcode 0x20 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ANDB(dst,src);
	PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_and_wr16)(i8086_state *cpustate)    /* Opcode 0x21 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ANDW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_and_r8b)(i8086_state *cpustate)    /* Opcode 0x22 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_and_r16w)(i8086_state *cpustate)    /* Opcode 0x23 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst,src);
	RegWord(ModRM)=dst;
}

static void PREFIX86(_and_ald8)(i8086_state *cpustate)    /* Opcode 0x24 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ANDB(dst,src);
	cpustate->regs.b[AL] = dst;
}

static void PREFIX86(_and_axd16)(i8086_state *cpustate)    /* Opcode 0x25 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ANDW(dst,src);
	cpustate->regs.w[AX]=dst;
}

static void PREFIX86(_daa)(i8086_state *cpustate)    /* Opcode 0x27 */
{
	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		cpustate->regs.b[AL] = tmp = cpustate->regs.b[AL] + 6;
		cpustate->AuxVal = 1;
		cpustate->CarryVal |= tmp & 0x100;
	}

	if (CF || (cpustate->regs.b[AL] > 0x9f))
	{
		cpustate->regs.b[AL] += 0x60;
		cpustate->CarryVal = 1;
	}

	SetSZPF_Byte(cpustate->regs.b[AL]);
	ICOUNT -= timing.daa;
}

static void PREFIX86(_sub_br8)(i8086_state *cpustate)    /* Opcode 0x28 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	SUBB(dst,src);
	PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_sub_wr16)(i8086_state *cpustate)    /* Opcode 0x29 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	SUBW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_sub_r8b)(i8086_state *cpustate)    /* Opcode 0x2a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_sub_r16w)(i8086_state *cpustate)    /* Opcode 0x2b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst,src);
	RegWord(ModRM)=dst;
}

static void PREFIX86(_sub_ald8)(i8086_state *cpustate)    /* Opcode 0x2c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	SUBB(dst,src);
	cpustate->regs.b[AL] = dst;
}

static void PREFIX86(_sub_axd16)(i8086_state *cpustate)    /* Opcode 0x2d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	SUBW(dst,src);
	cpustate->regs.w[AX]=dst;
}

static void PREFIX86(_das)(i8086_state *cpustate)    /* Opcode 0x2f */
{
	UINT8 tmpAL=cpustate->regs.b[AL];
	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		cpustate->regs.b[AL] = tmp = cpustate->regs.b[AL] - 6;
		cpustate->AuxVal = 1;
		cpustate->CarryVal |= tmp & 0x100;
	}

	if (CF || (tmpAL > 0x9f))
	{
		cpustate->regs.b[AL] -= 0x60;
		cpustate->CarryVal = 1;
	}

	SetSZPF_Byte(cpustate->regs.b[AL]);
	ICOUNT -= timing.das;
}

static void PREFIX86(_xor_br8)(i8086_state *cpustate)    /* Opcode 0x30 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	XORB(dst,src);
	PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_xor_wr16)(i8086_state *cpustate)    /* Opcode 0x31 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	XORW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_xor_r8b)(i8086_state *cpustate)    /* Opcode 0x32 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	XORB(dst,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_xor_r16w)(i8086_state *cpustate)    /* Opcode 0x33 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	XORW(dst,src);
	RegWord(ModRM)=dst;
}

static void PREFIX86(_xor_ald8)(i8086_state *cpustate)    /* Opcode 0x34 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	XORB(dst,src);
	cpustate->regs.b[AL] = dst;
}

static void PREFIX86(_xor_axd16)(i8086_state *cpustate)    /* Opcode 0x35 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	XORW(dst,src);
	cpustate->regs.w[AX]=dst;
}

static void PREFIX86(_aaa)(i8086_state *cpustate)    /* Opcode 0x37 */
{
	UINT8 ALcarry=1;
	if (cpustate->regs.b[AL]>0xf9) ALcarry=2;

	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		cpustate->regs.b[AL] += 6;
		cpustate->regs.b[AH] += ALcarry;
		cpustate->AuxVal = 1;
		cpustate->CarryVal = 1;
	}
	else
	{
		cpustate->AuxVal = 0;
		cpustate->CarryVal = 0;
	}
	cpustate->regs.b[AL] &= 0x0F;
	ICOUNT -= timing.aaa;
}

static void PREFIX86(_cmp_br8)(i8086_state *cpustate)    /* Opcode 0x38 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst,src);
}

static void PREFIX86(_cmp_wr16)(i8086_state *cpustate)    /* Opcode 0x39 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst,src);
}

static void PREFIX86(_cmp_r8b)(i8086_state *cpustate)    /* Opcode 0x3a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst,src);
}

static void PREFIX86(_cmp_r16w)(i8086_state *cpustate)    /* Opcode 0x3b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst,src);
}

static void PREFIX86(_cmp_ald8)(i8086_state *cpustate)    /* Opcode 0x3c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	SUBB(dst,src);
}

static void PREFIX86(_cmp_axd16)(i8086_state *cpustate)    /* Opcode 0x3d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	SUBW(dst,src);
}

static void PREFIX86(_aas)(i8086_state *cpustate)    /* Opcode 0x3f */
{
//  UINT8 ALcarry=1;
//  if (cpustate->regs.b[AL]>0xf9) ALcarry=2;

	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		cpustate->regs.b[AL] -= 6;
		cpustate->regs.b[AH] -= 1;
		cpustate->AuxVal = 1;
		cpustate->CarryVal = 1;
	}
	else
	{
		cpustate->AuxVal = 0;
		cpustate->CarryVal = 0;
	}
	cpustate->regs.b[AL] &= 0x0F;
	ICOUNT -= timing.aas;
}

#define IncWordReg(Reg)                     \
{                                           \
	unsigned tmp = (unsigned)cpustate->regs.w[Reg]; \
	unsigned tmp1 = tmp+1;                  \
	SetOFW_Add(tmp1,tmp,1);                 \
	SetAF(tmp1,tmp,1);                      \
	SetSZPF_Word(tmp1);                     \
	cpustate->regs.w[Reg]=tmp1;                     \
	ICOUNT -= timing.incdec_r16;            \
}

static void PREFIX86(_inc_ax)(i8086_state *cpustate)    /* Opcode 0x40 */
{
	IncWordReg(AX);
}

static void PREFIX86(_inc_cx)(i8086_state *cpustate)    /* Opcode 0x41 */
{
	IncWordReg(CX);
}

static void PREFIX86(_inc_dx)(i8086_state *cpustate)    /* Opcode 0x42 */
{
	IncWordReg(DX);
}

static void PREFIX(_inc_bx)(i8086_state *cpustate)    /* Opcode 0x43 */
{
	IncWordReg(BX);
}

static void PREFIX86(_inc_sp)(i8086_state *cpustate)    /* Opcode 0x44 */
{
	IncWordReg(SP);
}

static void PREFIX86(_inc_bp)(i8086_state *cpustate)    /* Opcode 0x45 */
{
	IncWordReg(BP);
}

static void PREFIX86(_inc_si)(i8086_state *cpustate)    /* Opcode 0x46 */
{
	IncWordReg(SI);
}

static void PREFIX86(_inc_di)(i8086_state *cpustate)    /* Opcode 0x47 */
{
	IncWordReg(DI);
}

#define DecWordReg(Reg)                     \
{                                           \
	unsigned tmp = (unsigned)cpustate->regs.w[Reg]; \
	unsigned tmp1 = tmp-1;                  \
	SetOFW_Sub(tmp1,1,tmp);                 \
	SetAF(tmp1,tmp,1);                      \
	SetSZPF_Word(tmp1);                     \
	cpustate->regs.w[Reg]=tmp1;                     \
	ICOUNT -= timing.incdec_r16;            \
}

static void PREFIX86(_dec_ax)(i8086_state *cpustate)    /* Opcode 0x48 */
{
	DecWordReg(AX);
}

static void PREFIX86(_dec_cx)(i8086_state *cpustate)    /* Opcode 0x49 */
{
	DecWordReg(CX);
}

static void PREFIX86(_dec_dx)(i8086_state *cpustate)    /* Opcode 0x4a */
{
	DecWordReg(DX);
}

static void PREFIX86(_dec_bx)(i8086_state *cpustate)    /* Opcode 0x4b */
{
	DecWordReg(BX);
}

static void PREFIX86(_dec_sp)(i8086_state *cpustate)    /* Opcode 0x4c */
{
	DecWordReg(SP);
}

static void PREFIX86(_dec_bp)(i8086_state *cpustate)    /* Opcode 0x4d */
{
	DecWordReg(BP);
}

static void PREFIX86(_dec_si)(i8086_state *cpustate)    /* Opcode 0x4e */
{
	DecWordReg(SI);
}

static void PREFIX86(_dec_di)(i8086_state *cpustate)    /* Opcode 0x4f */
{
	DecWordReg(DI);
}

static void PREFIX86(_push_ax)(i8086_state *cpustate)    /* Opcode 0x50 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[AX]);
}

static void PREFIX86(_push_cx)(i8086_state *cpustate)    /* Opcode 0x51 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[CX]);
}

static void PREFIX86(_push_dx)(i8086_state *cpustate)    /* Opcode 0x52 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[DX]);
}

static void PREFIX86(_push_bx)(i8086_state *cpustate)    /* Opcode 0x53 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[BX]);
}

static void PREFIX86(_push_sp)(i8086_state *cpustate)    /* Opcode 0x54 */
{
	ICOUNT -= timing.push_r16;
#ifdef I80286
	PUSH(cpustate->regs.w[SP]+2);
#else
	PUSH(cpustate->regs.w[SP]);
#endif
}

static void PREFIX86(_push_bp)(i8086_state *cpustate)    /* Opcode 0x55 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[BP]);
}


static void PREFIX86(_push_si)(i8086_state *cpustate)    /* Opcode 0x56 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[SI]);
}

static void PREFIX86(_push_di)(i8086_state *cpustate)    /* Opcode 0x57 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[DI]);
}

static void PREFIX86(_pop_ax)(i8086_state *cpustate)    /* Opcode 0x58 */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[AX]);
}

static void PREFIX86(_pop_cx)(i8086_state *cpustate)    /* Opcode 0x59 */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[CX]);
}

static void PREFIX86(_pop_dx)(i8086_state *cpustate)    /* Opcode 0x5a */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[DX]);
}

static void PREFIX86(_pop_bx)(i8086_state *cpustate)    /* Opcode 0x5b */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[BX]);
}

static void PREFIX86(_pop_sp)(i8086_state *cpustate)    /* Opcode 0x5c */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[SP]);
}

static void PREFIX86(_pop_bp)(i8086_state *cpustate)    /* Opcode 0x5d */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[BP]);
}

static void PREFIX86(_pop_si)(i8086_state *cpustate)    /* Opcode 0x5e */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[SI]);
}

static void PREFIX86(_pop_di)(i8086_state *cpustate)    /* Opcode 0x5f */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[DI]);
}

static void PREFIX86(_jo)(i8086_state *cpustate)    /* Opcode 0x70 */
{
	int tmp = (int)((INT8)FETCH);
	if (OF)
	{
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jno)(i8086_state *cpustate)    /* Opcode 0x71 */
{
	int tmp = (int)((INT8)FETCH);
	if (!OF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jb)(i8086_state *cpustate)    /* Opcode 0x72 */
{
	int tmp = (int)((INT8)FETCH);
	if (CF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jnb)(i8086_state *cpustate)    /* Opcode 0x73 */
{
	int tmp = (int)((INT8)FETCH);
	if (!CF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jz)(i8086_state *cpustate)    /* Opcode 0x74 */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jnz)(i8086_state *cpustate)    /* Opcode 0x75 */
{
	int tmp = (int)((INT8)FETCH);
	if (!ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jbe)(i8086_state *cpustate)    /* Opcode 0x76 */
{
	int tmp = (int)((INT8)FETCH);
	if (CF || ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jnbe)(i8086_state *cpustate)    /* Opcode 0x77 */
{
	int tmp = (int)((INT8)FETCH);
	if (!(CF || ZF)) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_js)(i8086_state *cpustate)    /* Opcode 0x78 */
{
	int tmp = (int)((INT8)FETCH);
	if (SF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jns)(i8086_state *cpustate)    /* Opcode 0x79 */
{
	int tmp = (int)((INT8)FETCH);
	if (!SF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jp)(i8086_state *cpustate)    /* Opcode 0x7a */
{
	int tmp = (int)((INT8)FETCH);
	if (PF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jnp)(i8086_state *cpustate)    /* Opcode 0x7b */
{
	int tmp = (int)((INT8)FETCH);
	if (!PF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jl)(i8086_state *cpustate)    /* Opcode 0x7c */
{
	int tmp = (int)((INT8)FETCH);
	if ((SF!=OF)&&!ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jnl)(i8086_state *cpustate)    /* Opcode 0x7d */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF||(SF==OF)) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jle)(i8086_state *cpustate)    /* Opcode 0x7e */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF||(SF!=OF)) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_jnle)(i8086_state *cpustate)    /* Opcode 0x7f */
{
	int tmp = (int)((INT8)FETCH);
	if ((SF==OF)&&!ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

static void PREFIX86(_80pre)(i8086_state *cpustate)    /* Opcode 0x80 */
{
	unsigned ModRM = FETCHOP;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	int tmpcf;

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD eb,d8 */
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x08:  /* OR eb,d8 */
		ORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x10:  /* ADC eb,d8 */
		src+=CF;
		ADCB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x18:  /* SBB eb,b8 */
		src+=CF;
		SBBB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x20:  /* AND eb,d8 */
		ANDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x28:  /* SUB eb,d8 */
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x30:  /* XOR eb,d8 */
		XORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x38:  /* CMP eb,d8 */
		SUBB(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
	}
}


static void PREFIX86(_81pre)(i8086_state *cpustate)    /* Opcode 0x81 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = FETCH;
	int tmpcf;
	src+= (FETCH << 8);

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD ew,d16 */
		ADDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x08:  /* OR ew,d16 */
		ORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x10:  /* ADC ew,d16 */
		src+=CF;
		ADCW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x18:  /* SBB ew,d16 */
		src+=CF;
		SBBW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x20:  /* AND ew,d16 */
		ANDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x28:  /* SUB ew,d16 */
		SUBW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x30:  /* XOR ew,d16 */
		XORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x38:  /* CMP ew,d16 */
		SUBW(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		break;
	}
}

static void PREFIX86(_82pre)(i8086_state *cpustate)  /* Opcode 0x82 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	int tmpcf;

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD eb,d8 */
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x08:  /* OR eb,d8 */
		ORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x10:  /* ADC eb,d8 */
		src+=CF;
		ADCB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x18:  /* SBB eb,d8 */
		src+=CF;
		SBBB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x20:  /* AND eb,d8 */
		ANDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x28:  /* SUB eb,d8 */
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x30:  /* XOR eb,d8 */
		XORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x38:  /* CMP eb,d8 */
		SUBB(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
	}
}

static void PREFIX86(_83pre)(i8086_state *cpustate)    /* Opcode 0x83 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = (WORD)((INT16)((INT8)FETCH));
	int tmpcf;

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD ew,d16 */
		ADDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x08:  /* OR ew,d16 */
		ORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x10:  /* ADC ew,d16 */
		src+=CF;
		ADCW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x18:  /* SBB ew,d16 */
		src+=CF;
		SBBW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x20:  /* AND ew,d16 */
		ANDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x28:  /* SUB ew,d16 */
		SUBW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x30:  /* XOR ew,d16 */
		XORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x38:  /* CMP ew,d16 */
		SUBW(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8_ro;
		break;
	}
}

static void PREFIX86(_test_br8)(i8086_state *cpustate)    /* Opcode 0x84 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst,src);
}

static void PREFIX86(_test_wr16)(i8086_state *cpustate)    /* Opcode 0x85 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst,src);
}

static void PREFIX86(_xchg_br8)(i8086_state *cpustate)    /* Opcode 0x86 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.xchg_rr8 : timing.xchg_rm8;
	PutbackRMByte(ModRM,src);
	RegByte(ModRM)=dst;
}

static void PREFIX86(_xchg_wr16)(i8086_state *cpustate)    /* Opcode 0x87 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.xchg_rr16 : timing.xchg_rm16;
	PutbackRMWord(ModRM,src);
	RegWord(ModRM)=dst;
}

static void PREFIX86(_mov_br8)(i8086_state *cpustate)    /* Opcode 0x88 */
{
	unsigned ModRM = FETCH;
	BYTE src = RegByte(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_mr8;
	PutRMByte(ModRM,src);
}

static void PREFIX86(_mov_wr16)(i8086_state *cpustate)    /* Opcode 0x89 */
{
	unsigned ModRM = FETCH;
	WORD src = RegWord(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr16 : timing.mov_mr16;
	PutRMWord(ModRM,src);
}

static void PREFIX86(_mov_r8b)(i8086_state *cpustate)    /* Opcode 0x8a */
{
	unsigned ModRM = FETCH;
	BYTE src = GetRMByte(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm8;
	RegByte(ModRM)=src;
}

static void PREFIX86(_mov_r16w)(i8086_state *cpustate)    /* Opcode 0x8b */
{
	unsigned ModRM = FETCH;
	WORD src = GetRMWord(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm16;
	RegWord(ModRM)=src;
}

static void PREFIX86(_mov_wsreg)(i8086_state *cpustate)    /* Opcode 0x8c */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rs : timing.mov_ms;
	if (ModRM & 0x20) { /* HJB 12/13/98 1xx is invalid */
		cpustate->pc = cpustate->prevpc;
		return PREFIX86(_invalid)(cpustate);
	}

	PutRMWord(ModRM,cpustate->sregs[(ModRM & 0x38) >> 3]);
}

static void PREFIX86(_lea)(i8086_state *cpustate)    /* Opcode 0x8d */
{
	unsigned ModRM = FETCH;
	ICOUNT -= timing.lea;
	(void)(*GetEA[ModRM])(cpustate);
	RegWord(ModRM)=cpustate->eo;    /* HJB 12/13/98 effective offset (no segment part) */
}

static void PREFIX86(_popw)(i8086_state *cpustate)    /* Opcode 0x8f */
{
	unsigned ModRM = FETCH;
		WORD tmp;
	tmp = ReadWord(cpustate->base[SS] + cpustate->regs.w[SP]);
	ICOUNT -= (ModRM >= 0xc0) ? timing.pop_r16 : timing.pop_m16;
	PutRMWord(ModRM,tmp);
	cpustate->regs.w[SP] += 2;
}


#define XchgAXReg(Reg)              \
{                                   \
	WORD tmp;                       \
	tmp = cpustate->regs.w[Reg];            \
	cpustate->regs.w[Reg] = cpustate->regs.w[AX];   \
	cpustate->regs.w[AX] = tmp;             \
	ICOUNT -= timing.xchg_ar16;     \
}


static void PREFIX86(_nop)(i8086_state *cpustate)    /* Opcode 0x90 */
{
	/* this is XchgAXReg(AX); */
	ICOUNT -= timing.nop;
}

static void PREFIX86(_xchg_axcx)(i8086_state *cpustate)    /* Opcode 0x91 */
{
	XchgAXReg(CX);
}

static void PREFIX86(_xchg_axdx)(i8086_state *cpustate)    /* Opcode 0x92 */
{
	XchgAXReg(DX);
}

static void PREFIX86(_xchg_axbx)(i8086_state *cpustate)    /* Opcode 0x93 */
{
	XchgAXReg(BX);
}

static void PREFIX86(_xchg_axsp)(i8086_state *cpustate)    /* Opcode 0x94 */
{
	XchgAXReg(SP);
}

static void PREFIX86(_xchg_axbp)(i8086_state *cpustate)    /* Opcode 0x95 */
{
	XchgAXReg(BP);
}

static void PREFIX86(_xchg_axsi)(i8086_state *cpustate)    /* Opcode 0x96 */
{
	XchgAXReg(SI);
}

static void PREFIX86(_xchg_axdi)(i8086_state *cpustate)    /* Opcode 0x97 */
{
	XchgAXReg(DI);
}

static void PREFIX86(_cbw)(i8086_state *cpustate)    /* Opcode 0x98 */
{
	ICOUNT -= timing.cbw;
	cpustate->regs.b[AH] = (cpustate->regs.b[AL] & 0x80) ? 0xff : 0;
}

static void PREFIX86(_cwd)(i8086_state *cpustate)    /* Opcode 0x99 */
{
	ICOUNT -= timing.cwd;
	cpustate->regs.w[DX] = (cpustate->regs.b[AH] & 0x80) ? 0xffff : 0;
}

static void PREFIX86(_call_far)(i8086_state *cpustate)
{
	unsigned int tmp, tmp2;
	WORD cs, ip;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp2 = FETCH;
	tmp2 += FETCH << 8;

	ip = cpustate->pc - cpustate->base[CS];
	cs = cpustate->sregs[CS];

#ifdef I80286
	i80286_code_descriptor(cpustate, tmp2, tmp, 2);
#else
	cpustate->sregs[CS] = (WORD)tmp2;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->base[CS] + (WORD)tmp) & AMASK;
#endif
	PUSH(cs);
	PUSH(ip);
	ICOUNT -= timing.call_far;
	CHANGE_PC(cpustate->pc);
}

static void PREFIX86(_wait)(i8086_state *cpustate)    /* Opcode 0x9b */
{
#ifdef I80286
	if ((cpustate->msw&0x0a) == 0x0a) throw TRAP(FPU_UNAVAILABLE,-1);
#endif
	if (cpustate->test_state)
	{
		ICOUNT = 0;
		cpustate->pc--;
	}
	else
		ICOUNT -= timing.wait;
}

static void PREFIX86(_pushf)(i8086_state *cpustate)    /* Opcode 0x9c */
{
	unsigned tmp;
	ICOUNT -= timing.pushf;

	tmp = CompressFlags();
#ifdef I80286
	if(!PM) ( tmp &= ~0xf000 );
#endif
	PUSH( tmp );
}

#ifndef I80286
static void PREFIX86(_popf)(i8086_state *cpustate)    /* Opcode 0x9d */
{
	unsigned tmp;
	POP(tmp);
	ICOUNT -= timing.popf;

	ExpandFlags(tmp);
	cpustate->flags = tmp;
	cpustate->flags = CompressFlags();

	if (cpustate->TF) PREFIX(_trap)(cpustate);

	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (cpustate->IF && cpustate->irq_state)
		PREFIX(_interrupt)(cpustate, (UINT32)-1);
}
#endif

static void PREFIX86(_sahf)(i8086_state *cpustate)    /* Opcode 0x9e */
{
	unsigned tmp = (CompressFlags() & 0xff00) | (cpustate->regs.b[AH] & 0xd5);
	ICOUNT -= timing.sahf;
	ExpandFlags(tmp);
}

static void PREFIX86(_lahf)(i8086_state *cpustate)    /* Opcode 0x9f */
{
	cpustate->regs.b[AH] = CompressFlags() & 0xff;
	ICOUNT -= timing.lahf;
}


static void PREFIX86(_mov_aldisp)(i8086_state *cpustate)    /* Opcode 0xa0 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_am8;
	cpustate->regs.b[AL] = GetMemB(DS, addr);
}

static void PREFIX86(_mov_axdisp)(i8086_state *cpustate)    /* Opcode 0xa1 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_am16;
	cpustate->regs.w[AX] = GetMemW(DS, addr);
}

static void PREFIX86(_mov_dispal)(i8086_state *cpustate)    /* Opcode 0xa2 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_ma8;
	PutMemB(DS, addr, cpustate->regs.b[AL]);
}

static void PREFIX86(_mov_dispax)(i8086_state *cpustate)    /* Opcode 0xa3 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_ma16;
	PutMemW(DS, addr, cpustate->regs.w[AX]);
}

static void PREFIX86(_movsb)(i8086_state *cpustate)    /* Opcode 0xa4 */
{
	BYTE tmp = GetMemB(DS,cpustate->regs.w[SI]);
	PutMemB(ES,cpustate->regs.w[DI], tmp);
	cpustate->regs.w[DI] += cpustate->DirVal;
	cpustate->regs.w[SI] += cpustate->DirVal;
	ICOUNT -= timing.movs8;
}

static void PREFIX86(_movsw)(i8086_state *cpustate)    /* Opcode 0xa5 */
{
	WORD tmp = GetMemW(DS,cpustate->regs.w[SI]);
	PutMemW(ES,cpustate->regs.w[DI], tmp);
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	cpustate->regs.w[SI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.movs16;
}

static void PREFIX86(_cmpsb)(i8086_state *cpustate)    /* Opcode 0xa6 */
{
	unsigned dst = GetMemB(ES, cpustate->regs.w[DI]);
	unsigned src = GetMemB(DS, cpustate->regs.w[SI]);
	SUBB(src,dst); /* opposite of the usual convention */
	cpustate->regs.w[DI] += cpustate->DirVal;
	cpustate->regs.w[SI] += cpustate->DirVal;
	ICOUNT -= timing.cmps8;
}

static void PREFIX86(_cmpsw)(i8086_state *cpustate)    /* Opcode 0xa7 */
{
	unsigned dst = GetMemW(ES, cpustate->regs.w[DI]);
	unsigned src = GetMemW(DS, cpustate->regs.w[SI]);
	SUBW(src,dst); /* opposite of the usual convention */
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	cpustate->regs.w[SI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.cmps16;
}

static void PREFIX86(_test_ald8)(i8086_state *cpustate)    /* Opcode 0xa8 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ANDB(dst,src);
}

static void PREFIX86(_test_axd16)(i8086_state *cpustate)    /* Opcode 0xa9 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ANDW(dst,src);
}

static void PREFIX86(_stosb)(i8086_state *cpustate)    /* Opcode 0xaa */
{
	PutMemB(ES,cpustate->regs.w[DI],cpustate->regs.b[AL]);
	cpustate->regs.w[DI] += cpustate->DirVal;
	ICOUNT -= timing.stos8;
}

static void PREFIX86(_stosw)(i8086_state *cpustate)    /* Opcode 0xab */
{
	PutMemW(ES,cpustate->regs.w[DI],cpustate->regs.w[AX]);
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.stos16;
}

static void PREFIX86(_lodsb)(i8086_state *cpustate)    /* Opcode 0xac */
{
	cpustate->regs.b[AL] = GetMemB(DS,cpustate->regs.w[SI]);
	cpustate->regs.w[SI] += cpustate->DirVal;
	ICOUNT -= timing.lods8;
}

static void PREFIX86(_lodsw)(i8086_state *cpustate)    /* Opcode 0xad */
{
	cpustate->regs.w[AX] = GetMemW(DS,cpustate->regs.w[SI]);
	cpustate->regs.w[SI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.lods16;
}

static void PREFIX86(_scasb)(i8086_state *cpustate)    /* Opcode 0xae */
{
	unsigned src = GetMemB(ES, cpustate->regs.w[DI]);
	unsigned dst = cpustate->regs.b[AL];
	SUBB(dst,src);
	cpustate->regs.w[DI] += cpustate->DirVal;
	ICOUNT -= timing.scas8;
}

static void PREFIX86(_scasw)(i8086_state *cpustate)    /* Opcode 0xaf */
{
	unsigned src = GetMemW(ES, cpustate->regs.w[DI]);
	unsigned dst = cpustate->regs.w[AX];
	SUBW(dst,src);
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.scas16;
}

static void PREFIX86(_mov_ald8)(i8086_state *cpustate)    /* Opcode 0xb0 */
{
	cpustate->regs.b[AL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_cld8)(i8086_state *cpustate)    /* Opcode 0xb1 */
{
	cpustate->regs.b[CL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_dld8)(i8086_state *cpustate)    /* Opcode 0xb2 */
{
	cpustate->regs.b[DL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_bld8)(i8086_state *cpustate)    /* Opcode 0xb3 */
{
	cpustate->regs.b[BL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_ahd8)(i8086_state *cpustate)    /* Opcode 0xb4 */
{
	cpustate->regs.b[AH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_chd8)(i8086_state *cpustate)    /* Opcode 0xb5 */
{
	cpustate->regs.b[CH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_dhd8)(i8086_state *cpustate)    /* Opcode 0xb6 */
{
	cpustate->regs.b[DH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_bhd8)(i8086_state *cpustate)    /* Opcode 0xb7 */
{
	cpustate->regs.b[BH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

static void PREFIX86(_mov_axd16)(i8086_state *cpustate)    /* Opcode 0xb8 */
{
	cpustate->regs.b[AL] = FETCH;
	cpustate->regs.b[AH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_mov_cxd16)(i8086_state *cpustate)    /* Opcode 0xb9 */
{
	cpustate->regs.b[CL] = FETCH;
	cpustate->regs.b[CH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_mov_dxd16)(i8086_state *cpustate)    /* Opcode 0xba */
{
	cpustate->regs.b[DL] = FETCH;
	cpustate->regs.b[DH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_mov_bxd16)(i8086_state *cpustate)    /* Opcode 0xbb */
{
	cpustate->regs.b[BL] = FETCH;
	cpustate->regs.b[BH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_mov_spd16)(i8086_state *cpustate)    /* Opcode 0xbc */
{
	cpustate->regs.b[SPL] = FETCH;
	cpustate->regs.b[SPH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_mov_bpd16)(i8086_state *cpustate)    /* Opcode 0xbd */
{
	cpustate->regs.b[BPL] = FETCH;
	cpustate->regs.b[BPH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_mov_sid16)(i8086_state *cpustate)    /* Opcode 0xbe */
{
	cpustate->regs.b[SIL] = FETCH;
	cpustate->regs.b[SIH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_mov_did16)(i8086_state *cpustate)    /* Opcode 0xbf */
{
	cpustate->regs.b[DIL] = FETCH;
	cpustate->regs.b[DIH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

static void PREFIX86(_ret_d16)(i8086_state *cpustate)    /* Opcode 0xc2 */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(cpustate->pc);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	cpustate->regs.w[SP]+=count;
	ICOUNT -= timing.ret_near_imm;
	CHANGE_PC(cpustate->pc);
}

static void PREFIX86(_ret)(i8086_state *cpustate)    /* Opcode 0xc3 */
{
	POP(cpustate->pc);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.ret_near;
	CHANGE_PC(cpustate->pc);
}

static void PREFIX86(_les_dw)(i8086_state *cpustate)    /* Opcode 0xc4 */
{
	unsigned ModRM = FETCH;
	WORD tmp = GetRMWord(ModRM);

#ifdef I80286
	i80286_data_descriptor(cpustate,ES,GetnextRMWord);
#else
	cpustate->sregs[ES] = GetnextRMWord;
	cpustate->base[ES] = SegBase(ES);
#endif
	RegWord(ModRM)= tmp;
	ICOUNT -= timing.load_ptr;
}

static void PREFIX86(_lds_dw)(i8086_state *cpustate)    /* Opcode 0xc5 */
{
	unsigned ModRM = FETCH;
	WORD tmp = GetRMWord(ModRM);

#ifdef I80286
	i80286_data_descriptor(cpustate,DS,GetnextRMWord);
#else
	cpustate->sregs[DS] = GetnextRMWord;
	cpustate->base[DS] = SegBase(DS);
#endif
	RegWord(ModRM)=tmp;
	ICOUNT -= timing.load_ptr;
}

static void PREFIX86(_mov_bd8)(i8086_state *cpustate)    /* Opcode 0xc6 */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_ri8 : timing.mov_mi8;
	PutImmRMByte(ModRM);
}

static void PREFIX86(_mov_wd16)(i8086_state *cpustate)    /* Opcode 0xc7 */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_ri16 : timing.mov_mi16;
	PutImmRMWord(ModRM);
}

#ifndef I80286
static void PREFIX86(_retf_d16)(i8086_state *cpustate)    /* Opcode 0xca */
{
	unsigned count = FETCH;
	count += FETCH << 8;

	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	cpustate->regs.w[SP]+=count;
	ICOUNT -= timing.ret_far_imm;
	CHANGE_PC(cpustate->pc);
}

static void PREFIX86(_retf)(i8086_state *cpustate)    /* Opcode 0xcb */
{
	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.ret_far;
	CHANGE_PC(cpustate->pc);
}
#endif

static void PREFIX86(_int3)(i8086_state *cpustate)    /* Opcode 0xcc */
{
	ICOUNT -= timing.int3;
	PREFIX(_interrupt)(cpustate, 3);
}

static void PREFIX86(_int)(i8086_state *cpustate)    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	ICOUNT -= timing.int_imm;
	PREFIX(_interrupt)(cpustate, int_num);
}

static void PREFIX86(_into)(i8086_state *cpustate)    /* Opcode 0xce */
{
	if (OF) {
		ICOUNT -= timing.into_t;
		PREFIX(_interrupt)(cpustate, 4);
	} else ICOUNT -= timing.into_nt;
}

#ifndef I80286
static void PREFIX86(_iret)(i8086_state *cpustate)    /* Opcode 0xcf */
{
	ICOUNT -= timing.iret;
	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
		PREFIX(_popf)(cpustate);
	CHANGE_PC(cpustate->pc);

	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (cpustate->IF && cpustate->irq_state)
		PREFIX(_interrupt)(cpustate, (UINT32)-1);
}
#endif

static void PREFIX86(_rotshft_b)(i8086_state *cpustate)    /* Opcode 0xd0 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Byte)(cpustate,ModRM,1,GetRMByte(ModRM));
}


static void PREFIX86(_rotshft_w)(i8086_state *cpustate)    /* Opcode 0xd1 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Word)(cpustate,ModRM,1,GetRMWord(ModRM));
}


#ifdef I8086
static void PREFIX86(_rotshft_bcl)(i8086_state *cpustate)    /* Opcode 0xd2 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Byte)(cpustate,ModRM,cpustate->regs.b[CL],GetRMByte(ModRM));
}

static void PREFIX86(_rotshft_wcl)(i8086_state *cpustate)    /* Opcode 0xd3 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Word)(cpustate,ModRM,cpustate->regs.b[CL],GetRMWord(ModRM));
}
#endif

/* OB: Opcode works on NEC V-Series but not the Variants              */
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.              */
static void PREFIX86(_aam)(i8086_state *cpustate)    /* Opcode 0xd4 */
{
	unsigned mult = FETCH;

	ICOUNT -= timing.aam;
	if (mult == 0)
		PREFIX(_interrupt)(cpustate, 0);
	else
	{
		cpustate->regs.b[AH] = cpustate->regs.b[AL] / mult;
		cpustate->regs.b[AL] %= mult;

		SetSZPF_Word(cpustate->regs.w[AX]);
	}
}

static void PREFIX86(_aad)(i8086_state *cpustate)    /* Opcode 0xd5 */
{
	unsigned mult = FETCH;

	ICOUNT -= timing.aad;

	cpustate->regs.b[AL] = cpustate->regs.b[AH] * mult + cpustate->regs.b[AL];
	cpustate->regs.b[AH] = 0;

	SetZF(cpustate->regs.b[AL]);
	SetPF(cpustate->regs.b[AL]);
	cpustate->SignVal = 0;
}


static void PREFIX86(_xlat)(i8086_state *cpustate)    /* Opcode 0xd7 */
{
	unsigned dest = cpustate->regs.w[BX]+cpustate->regs.b[AL];

	ICOUNT -= timing.xlat;
	cpustate->regs.b[AL] = GetMemB(DS, dest);
}

#ifndef I80286
static void PREFIX86(_escape)(i8086_state *cpustate)    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
{
	unsigned ModRM = FETCH;
	ICOUNT -= timing.nop;
	GetRMByte(ModRM);
}
#endif

static void PREFIX86(_loopne)(i8086_state *cpustate)    /* Opcode 0xe0 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = cpustate->regs.w[CX]-1;

	cpustate->regs.w[CX]=tmp;

	if (!ZF && tmp) {
		ICOUNT -= timing.loop_t;
		cpustate->pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.loop_nt;
}

static void PREFIX86(_loope)(i8086_state *cpustate)    /* Opcode 0xe1 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = cpustate->regs.w[CX]-1;

	cpustate->regs.w[CX]=tmp;

	if (ZF && tmp) {
		ICOUNT -= timing.loope_t;
			cpustate->pc += disp;
/* ASG - can probably assume this is safe
         CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.loope_nt;
}

static void PREFIX86(_loop)(i8086_state *cpustate)    /* Opcode 0xe2 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = cpustate->regs.w[CX]-1;

	cpustate->regs.w[CX]=tmp;

	if (tmp) {
		ICOUNT -= timing.loop_t;
		cpustate->pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.loop_nt;
}

static void PREFIX86(_jcxz)(i8086_state *cpustate)    /* Opcode 0xe3 */
{
	int disp = (int)((INT8)FETCH);

	if (cpustate->regs.w[CX] == 0) {
		ICOUNT -= timing.jcxz_t;
		cpustate->pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else
		ICOUNT -= timing.jcxz_nt;
}

static void PREFIX86(_inal)(i8086_state *cpustate)    /* Opcode 0xe4 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.in_imm8;
	cpustate->regs.b[AL] = read_port_byte(port);
}

static void PREFIX86(_inax)(i8086_state *cpustate)    /* Opcode 0xe5 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.in_imm16;
	cpustate->regs.w[AX] = read_port_word(port);
}

static void PREFIX86(_outal)(i8086_state *cpustate)    /* Opcode 0xe6 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.out_imm8;
	write_port_byte(port, cpustate->regs.b[AL]);
}

static void PREFIX86(_outax)(i8086_state *cpustate)    /* Opcode 0xe7 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.out_imm16;
	write_port_word(port, cpustate->regs.w[AX]);
}

static void PREFIX86(_call_d16)(i8086_state *cpustate)    /* Opcode 0xe8 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
	ip = cpustate->pc - cpustate->base[CS];
	PUSH(ip);
	ip += tmp;
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.call_near;
	CHANGE_PC(cpustate->pc);
}

static void PREFIX86(_jmp_d16)(i8086_state *cpustate)    /* Opcode 0xe9 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
	ip = cpustate->pc - cpustate->base[CS] + tmp;
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.jmp_near;
	CHANGE_PC(cpustate->pc);
}

static void PREFIX86(_jmp_far)(i8086_state *cpustate)    /* Opcode 0xea */
{
	unsigned tmp,tmp1;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp1 = FETCH;
	tmp1 += FETCH << 8;

#ifdef I80286
	i80286_code_descriptor(cpustate, tmp1,tmp, 1);
#else
	cpustate->sregs[CS] = (WORD)tmp1;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->base[CS] + tmp) & AMASK;
#endif
	ICOUNT -= timing.jmp_far;
	CHANGE_PC(cpustate->pc);
}

static void PREFIX86(_jmp_d8)(i8086_state *cpustate)    /* Opcode 0xeb */
{
	int tmp = (int)((INT8)FETCH);
	cpustate->pc += tmp;
/* ASG - can probably assume this is safe
    CHANGE_PC(cpustate->pc);*/
	ICOUNT -= timing.jmp_short;
}

static void PREFIX86(_inaldx)(i8086_state *cpustate)    /* Opcode 0xec */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.in_dx8;
	cpustate->regs.b[AL] = read_port_byte(cpustate->regs.w[DX]);
}

static void PREFIX86(_inaxdx)(i8086_state *cpustate)    /* Opcode 0xed */
{
	unsigned port = cpustate->regs.w[DX];
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.in_dx16;
	cpustate->regs.w[AX] = read_port_word(port);
}

static void PREFIX86(_outdxal)(i8086_state *cpustate)    /* Opcode 0xee */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.out_dx8;
	write_port_byte(cpustate->regs.w[DX], cpustate->regs.b[AL]);
}

static void PREFIX86(_outdxax)(i8086_state *cpustate)    /* Opcode 0xef */
{
	unsigned port = cpustate->regs.w[DX];
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.out_dx16;
	write_port_word(port, cpustate->regs.w[AX]);
}

/* I think thats not a V20 instruction...*/
static void PREFIX86(_lock)(i8086_state *cpustate)    /* Opcode 0xf0 */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.nop;
	PREFIX(_instruction)[FETCHOP](cpustate);  /* un-interruptible */
}
#endif

static void PREFIX(_pop_ss)(i8086_state *cpustate)    /* Opcode 0x17 */
{
#ifdef I80286
	i80286_pop_seg(cpustate, SS);
#else
	POP(cpustate->sregs[SS]);
	cpustate->base[SS] = SegBase(SS);
#endif
	ICOUNT -= timing.pop_seg;
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */
}

static void PREFIX(_es)(i8086_state *cpustate)    /* Opcode 0x26 */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = ES;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

static void PREFIX(_cs)(i8086_state *cpustate)    /* Opcode 0x2e */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = CS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

static void PREFIX(_ss)(i8086_state *cpustate)    /* Opcode 0x36 */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = SS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

static void PREFIX(_ds)(i8086_state *cpustate)    /* Opcode 0x3e */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = DS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

static void PREFIX(_mov_sregw)(i8086_state *cpustate)    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
	WORD src = GetRMWord(ModRM);

	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_sr : timing.mov_sm;
#ifdef I80286
	switch (ModRM & 0x38)
	{
	case 0x00:  /* mov es,ew */
		i80286_data_descriptor(cpustate,ES,src);
		break;
	case 0x18:  /* mov ds,ew */
		i80286_data_descriptor(cpustate,DS,src);
		break;
	case 0x10:  /* mov ss,ew */
		i80286_data_descriptor(cpustate,SS,src);
		cpustate->seg_prefix = FALSE;
		PREFIX(_instruction)[FETCHOP](cpustate);
		break;
	case 0x08:  /* mov cs,ew */
		PREFIX(_invalid)(cpustate);
		break;  /* doesn't do a jump far */
	}
#else
	switch (ModRM & 0x38)
	{
	case 0x00:  /* mov es,ew */
		cpustate->sregs[ES] = src;
		cpustate->base[ES] = SegBase(ES);
		break;
	case 0x18:  /* mov ds,ew */
		cpustate->sregs[DS] = src;
		cpustate->base[DS] = SegBase(DS);
		break;
	case 0x10:  /* mov ss,ew */
		cpustate->sregs[SS] = src;
		cpustate->base[SS] = SegBase(SS); /* no interrupt allowed before next instr */
		cpustate->seg_prefix = FALSE;
		PREFIX(_instruction)[FETCHOP](cpustate);
		break;
	case 0x08:  /* mov cs,ew */
#ifndef I80186
		int ip = cpustate->pc - cpustate->base[CS];
		cpustate->sregs[CS] = src;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
		CHANGE_PC(cpustate->pc);
#endif
		break;
	}
#endif
}

static void PREFIX(_repne)(i8086_state *cpustate)    /* Opcode 0xf2 */
{
		PREFIX(rep)(cpustate, 0);
}

static void PREFIX(_repe)(i8086_state *cpustate)    /* Opcode 0xf3 */
{
	PREFIX(rep)(cpustate, 1);
}

static void PREFIX(_sti)(i8086_state *cpustate)    /* Opcode 0xfb */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.flag_ops;
	SetIF(1);
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */

	/* if an interrupt is pending, signal an interrupt */
	if (cpustate->irq_state)
#ifdef I80286
		i80286_interrupt_descriptor(cpustate, (*cpustate->irq_callback)(cpustate->device, 0), 2, -1);
#else
		PREFIX86(_interrupt)(cpustate, (UINT32)-1);
#endif
}

#ifndef I80186
static void PREFIX86(_hlt)(i8086_state *cpustate)    /* Opcode 0xf4 */
{
#ifdef I80286
	if(PM && (CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	cpustate->halted=1;
	ICOUNT = 0;
}

static void PREFIX86(_cmc)(i8086_state *cpustate)    /* Opcode 0xf5 */
{
	ICOUNT -= timing.flag_ops;
	cpustate->CarryVal = !CF;
}

static void PREFIX86(_f6pre)(i8086_state *cpustate)
{
	/* Opcode 0xf6 */
	unsigned ModRM = FETCH;
	unsigned tmp = (unsigned)GetRMByte(ModRM);
	unsigned tmp2;


	switch (ModRM & 0x38)
	{
	case 0x00:  /* TEST Eb, data8 */
	case 0x08:  /* ??? */
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		tmp &= FETCH;

		cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0;
		SetSZPF_Byte(tmp);
		break;

	case 0x10:  /* NOT Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		PutbackRMByte(ModRM,~tmp);
		break;

		case 0x18:  /* NEG Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		tmp2=0;
		SUBB(tmp2,tmp);
		PutbackRMByte(ModRM,tmp2);
		break;
	case 0x20:  /* MUL AL, Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.mul_r8 : timing.mul_m8;
		{
			UINT16 result;
			tmp2 = cpustate->regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (UINT16)tmp2*tmp;
			cpustate->regs.w[AX]=(WORD)result;

			SetZF(cpustate->regs.w[AX]);
			cpustate->CarryVal = cpustate->OverVal = (cpustate->regs.b[AH] != 0);
		}
		break;
		case 0x28:  /* IMUL AL, Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.imul_r8 : timing.imul_m8;
		{
			INT16 result;

			tmp2 = (unsigned)cpustate->regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (INT16)((INT8)tmp2)*(INT16)((INT8)tmp);
			cpustate->regs.w[AX]=(WORD)result;

			SetZF(cpustate->regs.w[AX]);

			cpustate->CarryVal = cpustate->OverVal = (result >> 7 != 0) && (result >> 7 != -1);
		}
		break;
	case 0x30:  /* DIV AL, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.div_r8 : timing.div_m8;
		{
			UINT16 result;

			result = cpustate->regs.w[AX];

			if (tmp)
			{
				if ((result / tmp) > 0xff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.b[AH] = result % tmp;
					cpustate->regs.b[AL] = result / tmp;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	case 0x38:  /* IDIV AL, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.idiv_r8 : timing.idiv_m8;
		{

			INT16 result;

			result = cpustate->regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT16)((INT8)tmp);

				if ((result /= (INT16)((INT8)tmp)) > 0xff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.b[AL] = result;
					cpustate->regs.b[AH] = tmp2;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	}
}


static void PREFIX86(_f7pre)(i8086_state *cpustate)
{
	/* Opcode 0xf7 */
	unsigned ModRM = FETCH;
		unsigned tmp = GetRMWord(ModRM);
	unsigned tmp2;


	switch (ModRM & 0x38)
	{
	case 0x00:  /* TEST Ew, data16 */
	case 0x08:  /* ??? */
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		tmp2 = FETCH;
		tmp2 += FETCH << 8;

		tmp &= tmp2;

		cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0;
		SetSZPF_Word(tmp);
		break;

	case 0x10:  /* NOT Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp = ~tmp;
		PutbackRMWord(ModRM,tmp);
		break;

	case 0x18:  /* NEG Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp2 = 0;
		SUBW(tmp2,tmp);
		PutbackRMWord(ModRM,tmp2);
		break;
	case 0x20:  /* MUL AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.mul_r16 : timing.mul_m16;
		{
			UINT32 result;
			tmp2 = cpustate->regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (UINT32)tmp2*tmp;
			cpustate->regs.w[AX]=(WORD)result;
			result >>= 16;
			cpustate->regs.w[DX]=result;

			SetZF(cpustate->regs.w[AX] | cpustate->regs.w[DX]);
			cpustate->CarryVal = cpustate->OverVal = (cpustate->regs.w[DX] != 0);
		}
		break;

	case 0x28:  /* IMUL AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.imul_r16 : timing.imul_m16;
		{
			INT32 result;

			tmp2 = cpustate->regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (INT32)((INT16)tmp2)*(INT32)((INT16)tmp);
			cpustate->CarryVal = cpustate->OverVal = (result >> 15 != 0) && (result >> 15 != -1);

			cpustate->regs.w[AX]=(WORD)result;
			result = (WORD)(result >> 16);
			cpustate->regs.w[DX]=result;

			SetZF(cpustate->regs.w[AX] | cpustate->regs.w[DX]);
		}
		break;
		case 0x30:  /* DIV AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.div_r16 : timing.div_m16;
		{
			UINT32 result;

			result = (cpustate->regs.w[DX] << 16) + cpustate->regs.w[AX];

			if (tmp)
			{
				tmp2 = result % tmp;
				if ((result / tmp) > 0xffff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.w[DX]=tmp2;
					result /= tmp;
					cpustate->regs.w[AX]=result;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	case 0x38:  /* IDIV AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.idiv_r16 : timing.idiv_m16;
		{
			INT32 result;

			result = (cpustate->regs.w[DX] << 16) + cpustate->regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT32)((INT16)tmp);
				if ((result /= (INT32)((INT16)tmp)) > 0xffff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.w[AX]=result;
					cpustate->regs.w[DX]=tmp2;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	}
}


static void PREFIX86(_clc)(i8086_state *cpustate)    /* Opcode 0xf8 */
{
	ICOUNT -= timing.flag_ops;
	cpustate->CarryVal = 0;
}

static void PREFIX86(_stc)(i8086_state *cpustate)    /* Opcode 0xf9 */
{
	ICOUNT -= timing.flag_ops;
	cpustate->CarryVal = 1;
}

static void PREFIX86(_cli)(i8086_state *cpustate)    /* Opcode 0xfa */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.flag_ops;
	SetIF(0);
}

static void PREFIX86(_cld)(i8086_state *cpustate)    /* Opcode 0xfc */
{
	ICOUNT -= timing.flag_ops;
	SetDF(0);
}

static void PREFIX86(_std)(i8086_state *cpustate)    /* Opcode 0xfd */
{
	ICOUNT -= timing.flag_ops;
	SetDF(1);
}

static void PREFIX86(_fepre)(i8086_state *cpustate)    /* Opcode 0xfe */
{
	unsigned ModRM = FETCH;
	unsigned tmp = GetRMByte(ModRM);
	unsigned tmp1;

	ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r8 : timing.incdec_m8;
	if ((ModRM & 0x38) == 0)  /* INC eb */
	{
		tmp1 = tmp+1;
		SetOFB_Add(tmp1,tmp,1);
	}
	else  /* DEC eb */
	{
		tmp1 = tmp-1;
		SetOFB_Sub(tmp1,1,tmp);
	}

	SetAF(tmp1,tmp,1);
	SetSZPF_Byte(tmp1);

	PutbackRMByte(ModRM,(BYTE)tmp1);
}


static void PREFIX86(_ffpre)(i8086_state *cpustate)    /* Opcode 0xff */
{
	unsigned ModRM = FETCHOP;
	unsigned tmp;
	unsigned tmp1;
	WORD ip;

	switch(ModRM & 0x38)
	{
	case 0x00:  /* INC ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp+1;

		SetOFW_Add(tmp1,tmp,1);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

	case 0x08:  /* DEC ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp-1;

		SetOFW_Sub(tmp1,1,tmp);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

	case 0x10:  /* CALL ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.call_r16 : timing.call_m16;
		tmp = GetRMWord(ModRM);
		ip = cpustate->pc - cpustate->base[CS];
		PUSH(ip);
		cpustate->pc = (cpustate->base[CS] + (WORD)tmp) & AMASK;
		CHANGE_PC(cpustate->pc);
		break;

	case 0x18:  /* CALL FAR ea */
		ICOUNT -= timing.call_m32;
		tmp = cpustate->sregs[CS];  /* HJB 12/13/98 need to skip displacements of cpustate->ea */
		tmp1 = GetRMWord(ModRM);
		ip = cpustate->pc - cpustate->base[CS];
#ifdef I80286
		i80286_code_descriptor(cpustate, GetnextRMWord, tmp1, 2);
#else
		cpustate->sregs[CS] = GetnextRMWord;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->base[CS] + tmp1) & AMASK;
#endif
		PUSH(tmp);
		PUSH(ip);
		CHANGE_PC(cpustate->pc);
		break;

	case 0x20:  /* JMP ea */
		ICOUNT -= (ModRM >= 0xc0) ? timing.jmp_r16 : timing.jmp_m16;
		ip = GetRMWord(ModRM);
		cpustate->pc = (cpustate->base[CS] + ip) & AMASK;
		CHANGE_PC(cpustate->pc);
		break;

	case 0x28:  /* JMP FAR ea */
		ICOUNT -= timing.jmp_m32;

#ifdef I80286
		tmp = GetRMWord(ModRM);
		i80286_code_descriptor(cpustate, GetnextRMWord, tmp, 1);
#else
		cpustate->pc = GetRMWord(ModRM);
		cpustate->sregs[CS] = GetnextRMWord;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
#endif
		CHANGE_PC(cpustate->pc);
		break;

	case 0x30:  /* PUSH ea */
		ICOUNT -= (ModRM >= 0xc0) ? timing.push_r16 : timing.push_m16;
		tmp = GetRMWord(ModRM);
		PUSH(tmp);
		break;
	default:
		tmp = GetRMWord(ModRM);  // 286 doesn't matter but 8086?
		return PREFIX(_invalid)(cpustate);
	}
}


static void PREFIX86(_invalid)(i8086_state *cpustate)
{
#ifdef I80286
	throw TRAP(ILLEGAL_INSTRUCTION,-1);
#else
	/* i8086/i8088 ignore an invalid opcode. */
	/* i80186/i80188 probably also ignore an invalid opcode. */
	logerror("illegal instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-1), cpustate->pc);
	ICOUNT -= 10;
#endif
}

#ifndef I80286
static void PREFIX86(_invalid_2b)(i8086_state *cpustate)
{
	unsigned ModRM = FETCH;
	GetRMByte(ModRM);
	logerror("illegal 2 byte instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-2), cpustate->pc-2);
	ICOUNT -= 10;
}
#endif
#endif
