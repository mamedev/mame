/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

// file will be included in all cpu variants
// function renaming will be added when neccessary
// timing value should move to separate array

#undef ICOUNT

#define ICOUNT i8086_ICount


static void PREFIX186(_pusha)(void)    /* Opcode 0x60 */
{
	unsigned tmp=I.regs.w[SP];

	ICOUNT -= timing.pusha;
	PUSH(I.regs.w[AX]);
	PUSH(I.regs.w[CX]);
	PUSH(I.regs.w[DX]);
	PUSH(I.regs.w[BX]);
    PUSH(tmp);
	PUSH(I.regs.w[BP]);
	PUSH(I.regs.w[SI]);
	PUSH(I.regs.w[DI]);
}

static void PREFIX186(_popa)(void)    /* Opcode 0x61 */
{
	 unsigned tmp;

	ICOUNT -= timing.popa;
	POP(I.regs.w[DI]);
	POP(I.regs.w[SI]);
	POP(I.regs.w[BP]);
	POP(tmp);
	POP(I.regs.w[BX]);
	POP(I.regs.w[DX]);
	POP(I.regs.w[CX]);
	POP(I.regs.w[AX]);
}

static void PREFIX186(_bound)(void)    /* Opcode 0x62 */
{
	unsigned ModRM = FETCHOP;
	int low = (INT16)GetRMWord(ModRM);
    int high= (INT16)GetnextRMWord;
	int tmp= (INT16)RegWord(ModRM);
	if (tmp<low || tmp>high) {
		I.pc-=2;
		PREFIX86(_interrupt)(5);
	}
	ICOUNT -= timing.bound;
}

static void PREFIX186(_push_d16)(void)    /* Opcode 0x68 */
{
	unsigned tmp = FETCH;

	ICOUNT -= timing.push_imm;
	tmp += FETCH << 8;
	PUSH(tmp);
}

static void PREFIX186(_imul_d16)(void)    /* Opcode 0x69 */
{
	DEF_r16w(dst,src);
	unsigned src2=FETCH;
	src+=(FETCH<<8);

	ICOUNT -= (ModRM >= 0xc0) ? timing.imul_rri16 : timing.imul_rmi16;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	I.CarryVal = I.OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}


static void PREFIX186(_push_d8)(void)    /* Opcode 0x6a */
{
	unsigned tmp = (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= timing.push_imm;
	PUSH(tmp);
}

static void PREFIX186(_imul_d8)(void)    /* Opcode 0x6b */
{
	DEF_r16w(dst,src);
	unsigned src2= (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= (ModRM >= 0xc0) ? timing.imul_rri8 : timing.imul_rmi8;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	I.CarryVal = I.OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}

static void PREFIX186(_insb)(void)    /* Opcode 0x6c */
{
	ICOUNT -= timing.ins8;
	PutMemB(ES,I.regs.w[DI],read_port_byte(I.regs.w[DX]));
	I.regs.w[DI] += I.DirVal;
}

static void PREFIX186(_insw)(void)    /* Opcode 0x6d */
{
	ICOUNT -= timing.ins16;
	PutMemW(ES,I.regs.w[DI],read_port_word(I.regs.w[DX]));
	I.regs.w[DI] += 2 * I.DirVal;
}

static void PREFIX186(_outsb)(void)    /* Opcode 0x6e */
{
	ICOUNT -= timing.outs8;
	write_port_byte(I.regs.w[DX],GetMemB(DS,I.regs.w[SI]));
	I.regs.w[SI] += I.DirVal; /* GOL 11/27/01 */
}

static void PREFIX186(_outsw)(void)    /* Opcode 0x6f */
{
	ICOUNT -= timing.outs16;
	write_port_word(I.regs.w[DX],GetMemW(DS,I.regs.w[SI]));
	I.regs.w[SI] += 2 * I.DirVal; /* GOL 11/27/01 */
}

static void PREFIX186(_rotshft_bd8)(void)    /* Opcode 0xc0 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Byte)(ModRM,count);
}

static void PREFIX186(_rotshft_wd8)(void)    /* Opcode 0xc1 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Word)(ModRM,count);
}

static void PREFIX186(_enter)(void)    /* Opcode 0xc8 */
{
	unsigned nb = FETCH;	 unsigned i,level;

	nb += FETCH << 8;
	level = FETCH;
	ICOUNT -= (level == 0) ? timing.enter0 : (level == 1) ? timing.enter1 : timing.enter_base + level * timing.enter_count;
	PUSH(I.regs.w[BP]);
	I.regs.w[BP]=I.regs.w[SP];
	I.regs.w[SP] -= nb;
	for (i=1;i<level;i++)
		PUSH(GetMemW(SS,I.regs.w[BP]-i*2));
	if (level) PUSH(I.regs.w[BP]);
}

static void PREFIX186(_leave)(void)    /* Opcode 0xc9 */
{
	ICOUNT -= timing.leave;
	I.regs.w[SP]=I.regs.w[BP];
	POP(I.regs.w[BP]);
}
