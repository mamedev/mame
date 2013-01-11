/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

// file will be included in all cpu variants
// function renaming will be added when necessary
// timing value should move to separate array

#undef ICOUNT

#define ICOUNT cpustate->icount


static void PREFIX186(_pusha)(i8086_state *cpustate)    /* Opcode 0x60 */
{
	unsigned tmp=cpustate->regs.w[SP];

#ifdef I80286
	if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP]-16, 16, I80286_WRITE);
#endif
	ICOUNT -= timing.pusha;
	PUSH(cpustate->regs.w[AX]);
	PUSH(cpustate->regs.w[CX]);
	PUSH(cpustate->regs.w[DX]);
	PUSH(cpustate->regs.w[BX]);
	PUSH(tmp);
	PUSH(cpustate->regs.w[BP]);
	PUSH(cpustate->regs.w[SI]);
	PUSH(cpustate->regs.w[DI]);
}

static unsigned i186_popa_tmp;  // hack around GCC 4.6 error because we need the side effects of POP
static void PREFIX186(_popa)(i8086_state *cpustate)    /* Opcode 0x61 */
{

#ifdef I80286
	if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP], 16, I80286_READ);
#endif
	ICOUNT -= timing.popa;
	POP(cpustate->regs.w[DI]);
	POP(cpustate->regs.w[SI]);
	POP(cpustate->regs.w[BP]);
	POP(i186_popa_tmp);
	POP(cpustate->regs.w[BX]);
	POP(cpustate->regs.w[DX]);
	POP(cpustate->regs.w[CX]);
	POP(cpustate->regs.w[AX]);
}

static void PREFIX186(_bound)(i8086_state *cpustate)    /* Opcode 0x62 */
{
	unsigned ModRM = FETCHOP;
	int low = (INT16)GetRMWord(ModRM);
	int high= (INT16)GetnextRMWord;
	int tmp= (INT16)RegWord(ModRM);
	if (tmp<low || tmp>high) {
		cpustate->pc-= ( cpustate->seg_prefix ? 3 : 2 );
		PREFIX86(_interrupt)(cpustate, 5);
	}
	ICOUNT -= timing.bound;
}

static void PREFIX186(_push_d16)(i8086_state *cpustate)    /* Opcode 0x68 */
{
	unsigned tmp = FETCH;

	ICOUNT -= timing.push_imm;
	tmp += FETCH << 8;
	PUSH(tmp);
}

static void PREFIX186(_imul_d16)(i8086_state *cpustate)    /* Opcode 0x69 */
{
	DEF_r16w(dst,src);
	unsigned src2=FETCH;
	src2+=(FETCH<<8);

	ICOUNT -= (ModRM >= 0xc0) ? timing.imul_rri16 : timing.imul_rmi16;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	cpustate->CarryVal = cpustate->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}


static void PREFIX186(_push_d8)(i8086_state *cpustate)    /* Opcode 0x6a */
{
	unsigned tmp = (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= timing.push_imm;
	PUSH(tmp);
}

static void PREFIX186(_imul_d8)(i8086_state *cpustate)    /* Opcode 0x6b */
{
	DEF_r16w(dst,src);
	unsigned src2= (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= (ModRM >= 0xc0) ? timing.imul_rri8 : timing.imul_rmi8;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	cpustate->CarryVal = cpustate->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}

static void PREFIX186(_insb)(i8086_state *cpustate)    /* Opcode 0x6c */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.ins8;
	PutMemB(ES,cpustate->regs.w[DI],read_port_byte(cpustate->regs.w[DX]));
	cpustate->regs.w[DI] += cpustate->DirVal;
}

static void PREFIX186(_insw)(i8086_state *cpustate)    /* Opcode 0x6d */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.ins16;
	PutMemW(ES,cpustate->regs.w[DI],read_port_word(cpustate->regs.w[DX]));
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
}

static void PREFIX186(_outsb)(i8086_state *cpustate)    /* Opcode 0x6e */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.outs8;
	write_port_byte(cpustate->regs.w[DX],GetMemB(DS,cpustate->regs.w[SI]));
	cpustate->regs.w[SI] += cpustate->DirVal; /* GOL 11/27/01 */
}

static void PREFIX186(_outsw)(i8086_state *cpustate)    /* Opcode 0x6f */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.outs16;
	write_port_word(cpustate->regs.w[DX],GetMemW(DS,cpustate->regs.w[SI]));
	cpustate->regs.w[SI] += 2 * cpustate->DirVal; /* GOL 11/27/01 */
}

static void PREFIX186(_rotshft_bd8)(i8086_state *cpustate)    /* Opcode 0xc0 */
{
	unsigned ModRM = FETCH;
	unsigned src = GetRMByte(ModRM);
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Byte)(cpustate,ModRM,count & 0x1f,src);
}

static void PREFIX186(_rotshft_wd8)(i8086_state *cpustate)    /* Opcode 0xc1 */
{
	unsigned ModRM = FETCH;
	unsigned src = GetRMWord(ModRM);
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Word)(cpustate,ModRM,count & 0x1f,src);
}

static void PREFIX186(_enter)(i8086_state *cpustate)    /* Opcode 0xc8 */
{
	unsigned nb = FETCH;
	unsigned i,level;
	UINT16 fp;

	nb += FETCH << 8;
#ifdef I80286
	level = FETCH & 0x1f;
	if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP]-2-(level*2), 2+(level*2), I80286_WRITE);
#else
	level = FETCH;
#endif
	ICOUNT -= (level == 0) ? timing.enter0 : (level == 1) ? timing.enter1 : timing.enter_base + level * timing.enter_count;
	PUSH(cpustate->regs.w[BP]);
	fp = cpustate->regs.w[SP];
	for (i=1;i<level;i++)
		PUSH(GetMemW(SS,cpustate->regs.w[BP]-i*2));
	if (level) PUSH(fp);
	cpustate->regs.w[BP] = fp;
	cpustate->regs.w[SP] -= nb;
}

static void PREFIX186(_leave)(i8086_state *cpustate)    /* Opcode 0xc9 */
{
	ICOUNT -= timing.leave;
	cpustate->regs.w[SP]=cpustate->regs.w[BP];
	POP(cpustate->regs.w[BP]);
}

static void PREFIX186(_rotshft_bcl)(i8086_state *cpustate)    /* Opcode 0xd2 */
{
	unsigned ModRM = FETCHOP;
	PREFIX86(_rotate_shift_Byte)(cpustate,ModRM,cpustate->regs.b[CL] & 0x1f,GetRMByte(ModRM));
}

static void PREFIX186(_rotshft_wcl)(i8086_state *cpustate)    /* Opcode 0xd3 */
{
	unsigned ModRM = FETCHOP;
	PREFIX86(_rotate_shift_Word)(cpustate,ModRM,cpustate->regs.b[CL] & 0x1f,GetRMWord(ModRM));
}
