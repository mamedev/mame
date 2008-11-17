/***************************************************************
 * Enter HALT state; write 1 to fake port on first execution
 ***************************************************************/
#define ENTER_HALT {											\
	_PC--;														\
	_HALT = 1;													\
	if( !Z180.after_EI ) 										\
		CPU_BURN_NAME(z180)( NULL, z180_icount );				\
}

/***************************************************************
 * Leave HALT state; write 0 to fake port
 ***************************************************************/
#define LEAVE_HALT {											\
	if( _HALT ) 												\
	{															\
		_HALT = 0;												\
		_PC++;													\
	}															\
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
#define IN(port)												\
	(((port ^ IO_IOCR) & 0xffc0) == 0) ?						\
		z180_readcontrol(port) : io_read_byte_8le(port)

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
#define OUT(port,value) 										\
	if (((port ^ IO_IOCR) & 0xffc0) == 0)						\
		z180_writecontrol(port,value);							\
	else io_write_byte_8le(port,value)

/***************************************************************
 * MMU calculate the memory managemant lookup table
 * bb and cb specify a 4K page
 * If the 4 most significant bits of an 16 bit address are
 * greater or equal to the bank base, the bank base register
 * specifies the 4K offset into the 20 bit address space.
 * If the 4 bits are also greater or equal to the common base,
 * the common base register is used to specify the offset.
 ***************************************************************/
INLINE void z180_mmu( void )
{
	offs_t addr, page, bb, cb;
	bb = IO_CBAR & 15;
	cb = IO_CBAR >> 4;
	for( page = 0; page < 16; page++ )
	{
		addr = page << 12;
		if (page >= bb)
		{
			if (page >= cb)
				addr += IO_CBR << 12;
			else
				addr += IO_BBR << 12;
		}
		Z180.mmu[page] = addr;
	}
}


#define MMU_REMAP_ADDR(addr) (Z180.mmu[((addr)>>12)&15]|((addr)&4095))

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
#define RM(addr)	program_read_byte_8le(MMU_REMAP_ADDR(addr))
UINT8 z180_readmem(offs_t offset)
{
	return RM(offset);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
#define WM(addr,value) program_write_byte_8le(MMU_REMAP_ADDR(addr),value)
void z180_writemem(offs_t offset, UINT8 data)
{
	WM(offset, data);
}


/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
INLINE void RM16( offs_t addr, PAIR *r )
{
	r->b.l = RM(addr);
	r->b.h = RM(addr+1);
}

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
INLINE void WM16( offs_t addr, PAIR *r )
{
	WM(addr,r->b.l);
	WM(addr+1,r->b.h);
}

/***************************************************************
 * ROP() is identical to RM() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
INLINE UINT8 ROP(void)
{
	offs_t addr = _PCD;
	_PC++;
	return program_decrypted_read_byte(MMU_REMAP_ADDR(addr));
}

/****************************************************************
 * ARG() is identical to ROP() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
INLINE UINT8 ARG(void)
{
	offs_t addr = _PCD;
	_PC++;
	return program_raw_read_byte(MMU_REMAP_ADDR(addr));
}

INLINE UINT32 ARG16(void)
{
	offs_t addr = _PCD;
	_PC += 2;
	return program_raw_read_byte(MMU_REMAP_ADDR(addr)) | (program_raw_read_byte(MMU_REMAP_ADDR(addr+1)) << 8);
}

/****************************************************************************
 * Change program counter - MMU lookup
 ****************************************************************************/
#define z180_change_pc(addr) change_pc(MMU_REMAP_ADDR(addr))
void z180_setOPbase(int pc)
{
	z180_change_pc(pc);
}

/***************************************************************
 * Calculate the effective addess EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
#define EAX EA = (UINT32)(UINT16)(_IX+(INT8)ARG())
#define EAY EA = (UINT32)(UINT16)(_IY+(INT8)ARG())

/***************************************************************
 * POP
 ***************************************************************/
#define POP(DR) { RM16( _SPD, &Z180.DR ); _SP += 2; }

/***************************************************************
 * PUSH
 ***************************************************************/
#define PUSH(SR) { _SP -= 2; WM16( _SPD, &Z180.SR ); }

/***************************************************************
 * JP
 ***************************************************************/
#define JP {													\
	_PCD = ARG16(); 											\
	z180_change_pc(_PCD);										\
}

/***************************************************************
 * JP_COND
 ***************************************************************/

#define JP_COND(cond)											\
	if( cond )													\
	{															\
		_PCD = ARG16(); 										\
		z180_change_pc(_PCD);									\
	}															\
	else														\
	{															\
		_PC += 2;												\
	}

/***************************************************************
 * JR
 ***************************************************************/
#define JR()													\
{																\
	unsigned oldpc = _PCD-1;									\
	INT8 arg = (INT8)ARG(); /* ARG() also increments _PC */ 	\
	_PC += arg; 			/* so don't do _PC += ARG() */      \
	z180_change_pc(_PCD);										\
	/* speed up busy loop */									\
	if( _PCD == oldpc ) 										\
	{															\
		if( !Z180.after_EI ) 									\
			BURNODD( z180_icount, 1, cc[Z180_TABLE_op][0x18] ); \
	}															\
	else														\
	{															\
		UINT8 op = program_decrypted_read_byte(_PCD);							\
		if( _PCD == oldpc-1 )									\
		{														\
			/* NOP - JR $-1 or EI - JR $-1 */					\
			if ( op == 0x00 || op == 0xfb ) 					\
			{													\
				if( !Z180.after_EI ) 							\
				   BURNODD( z180_icount-cc[Z180_TABLE_op][0x00],\
					   2, cc[Z180_TABLE_op][0x00]+cc[Z180_TABLE_op][0x18]); \
			}													\
		}														\
		else													\
		/* LD SP,#xxxx - JR $-3 */								\
		if( _PCD == oldpc-3 && op == 0x31 ) 					\
		{														\
			if( !Z180.after_EI ) 								\
			   BURNODD( z180_icount-cc[Z180_TABLE_op][0x31],	\
				   2, cc[Z180_TABLE_op][0x31]+cc[Z180_TABLE_op][0x18]); \
		}														\
	}															\
}

/***************************************************************
 * JR_COND
 ***************************************************************/
#define JR_COND(cond,opcode)									\
	if( cond )													\
	{															\
		INT8 arg = (INT8)ARG(); /* ARG() also increments _PC */ \
		_PC += arg; 			/* so don't do _PC += ARG() */  \
		CC(ex,opcode);											\
		z180_change_pc(_PCD);									\
	}															\
	else _PC++; 												\

/***************************************************************
 * CALL
 ***************************************************************/
#define CALL()													\
	EA = ARG16();												\
	PUSH( PC ); 												\
	_PCD = EA;													\
	z180_change_pc(_PCD)

/***************************************************************
 * CALL_COND
 ***************************************************************/
#define CALL_COND(cond,opcode)									\
	if( cond )													\
	{															\
		EA = ARG16();											\
		PUSH( PC ); 											\
		_PCD = EA;												\
		CC(ex,opcode);											\
		z180_change_pc(_PCD);									\
	}															\
	else														\
	{															\
		_PC+=2; 												\
	}

/***************************************************************
 * RET_COND
 ***************************************************************/
#define RET_COND(cond,opcode)									\
	if( cond )													\
	{															\
		POP(PC);												\
		z180_change_pc(_PCD);									\
		CC(ex,opcode);											\
	}

/***************************************************************
 * RETN
 ***************************************************************/
#define RETN	{												\
	LOG(("Z180 #%d RETN IFF1:%d IFF2:%d\n", cpunum_get_active(), _IFF1, _IFF2)); \
	POP(PC);													\
	z180_change_pc(_PCD);										\
	_IFF1 = _IFF2;												\
}

/***************************************************************
 * RETI
 ***************************************************************/
#define RETI	{												\
	POP(PC);													\
	z180_change_pc(_PCD);										\
/* according to http://www.msxnet.org/tech/Z80/z80undoc.txt */	\
/*  _IFF1 = _IFF2;  */											\
	if (Z180.daisy)												\
		z80daisy_call_reti_device(Z180.daisy);					\
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
#define LD_R_A {												\
	_R = _A;													\
	_R2 = _A & 0x80;				/* keep bit 7 of R */		\
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
#define LD_A_R {												\
	_A = (_R & 0x7f) | _R2; 									\
	_F = (_F & CF) | SZ[_A] | ( _IFF2 << 2 );					\
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
#define LD_I_A {												\
	_I = _A;													\
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
#define LD_A_I {												\
	_A = _I;													\
	_F = (_F & CF) | SZ[_A] | ( _IFF2 << 2 );					\
}

/***************************************************************
 * RST
 ***************************************************************/
#define RST(addr)												\
	PUSH( PC ); 												\
	_PCD = addr;												\
	z180_change_pc(_PCD)

/***************************************************************
 * INC  r8
 ***************************************************************/
INLINE UINT8 INC(UINT8 value)
{
	UINT8 res = value + 1;
	_F = (_F & CF) | SZHV_inc[res];
	return (UINT8)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
INLINE UINT8 DEC(UINT8 value)
{
	UINT8 res = value - 1;
	_F = (_F & CF) | SZHV_dec[res];
	return res;
}

/***************************************************************
 * RLCA
 ***************************************************************/
#define RLCA													\
	_A = (_A << 1) | (_A >> 7); 								\
	_F = (_F & (SF | ZF | PF)) | (_A & (YF | XF | CF))

/***************************************************************
 * RRCA
 ***************************************************************/
#define RRCA													\
	_F = (_F & (SF | ZF | PF)) | (_A & (YF | XF | CF)); 		\
	_A = (_A >> 1) | (_A << 7)

/***************************************************************
 * RLA
 ***************************************************************/
#define RLA {													\
	UINT8 res = (_A << 1) | (_F & CF);							\
	UINT8 c = (_A & 0x80) ? CF : 0; 							\
	_F = (_F & (SF | ZF | PF)) | c | (res & (YF | XF)); 		\
	_A = res;													\
}

/***************************************************************
 * RRA
 ***************************************************************/
#define RRA {													\
	UINT8 res = (_A >> 1) | (_F << 7);							\
	UINT8 c = (_A & 0x01) ? CF : 0; 							\
	_F = (_F & (SF | ZF | PF)) | c | (res & (YF | XF)); 		\
	_A = res;													\
}

/***************************************************************
 * RRD
 ***************************************************************/
#define RRD {													\
	UINT8 n = RM(_HL);											\
	WM( _HL, (n >> 4) | (_A << 4) );							\
	_A = (_A & 0xf0) | (n & 0x0f);								\
	_F = (_F & CF) | SZP[_A];									\
}

/***************************************************************
 * RLD
 ***************************************************************/
#define RLD {													\
	UINT8 n = RM(_HL);											\
	WM( _HL, (n << 4) | (_A & 0x0f) );							\
	_A = (_A & 0xf0) | (n >> 4);								\
	_F = (_F & CF) | SZP[_A];									\
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define ADD(value)												\
{																\
	UINT32 ah = _AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) + value);					\
	_F = SZHVC_add[ah | res];									\
	_A = res;													\
}
#else
#define ADD(value)												\
{																\
	unsigned val = value;										\
	unsigned res = _A + val;									\
	_F = SZ[(UINT8)res] | ((res >> 8) & CF) |					\
		((_A ^ res ^ val) & HF) |								\
		(((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5);		\
	_A = (UINT8)res;											\
}
#endif

/***************************************************************
 * ADC  A,n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define ADC(value)												\
{																\
	UINT32 ah = _AFD & 0xff00, c = _AFD & 1;					\
	UINT32 res = (UINT8)((ah >> 8) + value + c);				\
	_F = SZHVC_add[(c << 16) | ah | res];						\
	_A = res;													\
}
#else
#define ADC(value)												\
{																\
	unsigned val = value;										\
	unsigned res = _A + val + (_F & CF);						\
	_F = SZ[res & 0xff] | ((res >> 8) & CF) |					\
		((_A ^ res ^ val) & HF) |								\
		(((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5);		\
	_A = res;													\
}
#endif

/***************************************************************
 * SUB  n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define SUB(value)												\
{																\
	UINT32 ah = _AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) - value);					\
	_F = SZHVC_sub[ah | res];									\
	_A = res;													\
}
#else
#define SUB(value)												\
{																\
	unsigned val = value;										\
	unsigned res = _A - val;									\
	_F = SZ[res & 0xff] | ((res >> 8) & CF) | NF |				\
		((_A ^ res ^ val) & HF) |								\
		(((val ^ _A) & (_A ^ res) & 0x80) >> 5);				\
	_A = res;													\
}
#endif

/***************************************************************
 * SBC  A,n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define SBC(value)												\
{																\
	UINT32 ah = _AFD & 0xff00, c = _AFD & 1;					\
	UINT32 res = (UINT8)((ah >> 8) - value - c);				\
	_F = SZHVC_sub[(c<<16) | ah | res]; 						\
	_A = res;													\
}
#else
#define SBC(value)												\
{																\
	unsigned val = value;										\
	unsigned res = _A - val - (_F & CF);						\
	_F = SZ[res & 0xff] | ((res >> 8) & CF) | NF |				\
		((_A ^ res ^ val) & HF) |								\
		(((val ^ _A) & (_A ^ res) & 0x80) >> 5);				\
	_A = res;													\
}
#endif

/***************************************************************
 * NEG
 ***************************************************************/
#define NEG {													\
	UINT8 value = _A;											\
	_A = 0; 													\
	SUB(value); 												\
}

/***************************************************************
 * DAA
 ***************************************************************/
#define DAA {													\
	int idx = _A;												\
	if( _F & CF ) idx |= 0x100; 								\
	if( _F & HF ) idx |= 0x200; 								\
	if( _F & NF ) idx |= 0x400; 								\
	_AF = DAATable[idx];										\
}

/***************************************************************
 * AND  n
 ***************************************************************/
#define AND(value)												\
	_A &= value;												\
	_F = SZP[_A] | HF

/***************************************************************
 * OR   n
 ***************************************************************/
#define OR(value)												\
	_A |= value;												\
	_F = SZP[_A]

/***************************************************************
 * XOR  n
 ***************************************************************/
#define XOR(value)												\
	_A ^= value;												\
	_F = SZP[_A]

/***************************************************************
 * CP   n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define CP(value)												\
{																\
	UINT32 ah = _AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) - value);					\
	_F = SZHVC_sub[ah | res];									\
}
#else
#define CP(value)												\
{																\
	unsigned val = value;										\
	unsigned res = _A - val;									\
	_F = SZ[res & 0xff] | ((res >> 8) & CF) | NF |				\
		((_A ^ res ^ val) & HF) |								\
		((((val ^ _A) & (_A ^ res)) >> 5) & VF);				\
}
#endif

/***************************************************************
 * EX   AF,AF'
 ***************************************************************/
#define EX_AF { 												\
	PAIR tmp;													\
	tmp = Z180.AF; Z180.AF = Z180.AF2; Z180.AF2 = tmp;			\
}

/***************************************************************
 * EX   DE,HL
 ***************************************************************/
#define EX_DE_HL {												\
	PAIR tmp;													\
	tmp = Z180.DE; Z180.DE = Z180.HL; Z180.HL = tmp;			\
}

/***************************************************************
 * EXX
 ***************************************************************/
#define EXX {													\
	PAIR tmp;													\
	tmp = Z180.BC; Z180.BC = Z180.BC2; Z180.BC2 = tmp;			\
	tmp = Z180.DE; Z180.DE = Z180.DE2; Z180.DE2 = tmp;			\
	tmp = Z180.HL; Z180.HL = Z180.HL2; Z180.HL2 = tmp;			\
}

/***************************************************************
 * EX   (SP),r16
 ***************************************************************/
#define EXSP(DR)												\
{																\
	PAIR tmp = { { 0, 0, 0, 0 } };								\
	RM16( _SPD, &tmp ); 										\
	WM16( _SPD, &Z180.DR ); 									\
	Z180.DR = tmp;												\
}


/***************************************************************
 * ADD16
 ***************************************************************/
#define ADD16(DR,SR)											\
{																\
	UINT32 res = Z180.DR.d + Z180.SR.d; 						\
	_F = (_F & (SF | ZF | VF)) |								\
		(((Z180.DR.d ^ res ^ Z180.SR.d) >> 8) & HF) |			\
		((res >> 16) & CF); 									\
	Z180.DR.w.l = (UINT16)res;									\
}

/***************************************************************
 * ADC  r16,r16
 ***************************************************************/
#define ADC16(DR)												\
{																\
	UINT32 res = _HLD + Z180.DR.d + (_F & CF);					\
	_F = (((_HLD ^ res ^ Z180.DR.d) >> 8) & HF) |				\
		((res >> 16) & CF) |									\
		((res >> 8) & SF) | 									\
		((res & 0xffff) ? 0 : ZF) | 							\
		(((Z180.DR.d ^ _HLD ^ 0x8000) & (Z180.DR.d ^ res) & 0x8000) >> 13); \
	_HL = (UINT16)res;											\
}

/***************************************************************
 * SBC  r16,r16
 ***************************************************************/
#define SBC16(DR)												\
{																\
	UINT32 res = _HLD - Z180.DR.d - (_F & CF);					\
	_F = (((_HLD ^ res ^ Z180.DR.d) >> 8) & HF) | NF |			\
		((res >> 16) & CF) |									\
		((res >> 8) & SF) | 									\
		((res & 0xffff) ? 0 : ZF) | 							\
		(((Z180.DR.d ^ _HLD) & (_HLD ^ res) &0x8000) >> 13);	\
	_HL = (UINT16)res;											\
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
INLINE UINT8 RLC(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RRC  r8
 ***************************************************************/
INLINE UINT8 RRC(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res << 7)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RL   r8
 ***************************************************************/
INLINE UINT8 RL(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (_F & CF)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RR   r8
 ***************************************************************/
INLINE UINT8 RR(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (_F << 7)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SLA  r8
 ***************************************************************/
INLINE UINT8 SLA(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SRA  r8
 ***************************************************************/
INLINE UINT8 SRA(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res & 0x80)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SLL  r8
 ***************************************************************/
INLINE UINT8 SLL(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | 0x01) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SRL  r8
 ***************************************************************/
INLINE UINT8 SRL(UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = (res >> 1) & 0xff;
	_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
#undef BIT
#define BIT(bit,reg)											\
	_F = (_F & CF) | HF | SZ_BIT[reg & (1<<bit)]

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
#define BIT_XY(bit,reg) 										\
	_F = (_F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | ((EA>>8) & (YF|XF))

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
INLINE UINT8 RES(UINT8 bit, UINT8 value)
{
	return value & ~(1<<bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
INLINE UINT8 SET(UINT8 bit, UINT8 value)
{
	return value | (1<<bit);
}

/***************************************************************
 * LDI
 ***************************************************************/
#define LDI {													\
	UINT8 io = RM(_HL); 										\
	WM( _DE, io );												\
	_F &= SF | ZF | CF; 										\
	if( (_A + io) & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */		\
	if( (_A + io) & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */		\
	_HL++; _DE++; _BC--;										\
	if( _BC ) _F |= VF; 										\
}

/***************************************************************
 * CPI
 ***************************************************************/
#define CPI {													\
	UINT8 val = RM(_HL);										\
	UINT8 res = _A - val;										\
	_HL++; _BC--;												\
	_F = (_F & CF) | (SZ[res] & ~(YF|XF)) | ((_A ^ val ^ res) & HF) | NF;  \
	if( _F & HF ) res -= 1; 									\
	if( res & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */			\
	if( res & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */			\
	if( _BC ) _F |= VF; 										\
}

/***************************************************************
 * INI
 ***************************************************************/
#define INI {													\
	UINT8 io = IN(_BC); 										\
	_B--;														\
	WM( _HL, io );												\
	_HL++;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( (_C + io + 1) & 0x100 ) _F |= HF | CF;					\
	if( (irep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * OUTI
 ***************************************************************/
#define OUTI {													\
	UINT8 io = RM(_HL); 										\
	_B--;														\
	OUT( _BC, io ); 											\
	_HL++;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( (_C + io + 1) & 0x100 ) _F |= HF | CF;					\
	if( (irep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * LDD
 ***************************************************************/
#define LDD {													\
	UINT8 io = RM(_HL); 										\
	WM( _DE, io );												\
	_F &= SF | ZF | CF; 										\
	if( (_A + io) & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */		\
	if( (_A + io) & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */		\
	_HL--; _DE--; _BC--;										\
	if( _BC ) _F |= VF; 										\
}

/***************************************************************
 * CPD
 ***************************************************************/
#define CPD {													\
	UINT8 val = RM(_HL);										\
	UINT8 res = _A - val;										\
	_HL--; _BC--;												\
	_F = (_F & CF) | (SZ[res] & ~(YF|XF)) | ((_A ^ val ^ res) & HF) | NF;  \
	if( _F & HF ) res -= 1; 									\
	if( res & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */			\
	if( res & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */			\
	if( _BC ) _F |= VF; 										\
}

/***************************************************************
 * IND
 ***************************************************************/
#define IND {													\
	UINT8 io = IN(_BC); 										\
	_B--;														\
	WM( _HL, io );												\
	_HL--;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( (_C + io - 1) & 0x100 ) _F |= HF | CF;					\
	if( (drep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * OUTD
 ***************************************************************/
#define OUTD {													\
	UINT8 io = RM(_HL); 										\
	_B--;														\
	OUT( _BC, io ); 											\
	_HL--;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( (_C + io - 1) & 0x100 ) _F |= HF | CF;					\
	if( (drep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * LDIR
 ***************************************************************/
#define LDIR													\
	LDI;														\
	if( _BC )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb0);											\
	}

/***************************************************************
 * CPIR
 ***************************************************************/
#define CPIR													\
	CPI;														\
	if( _BC && !(_F & ZF) ) 									\
	{															\
		_PC -= 2;												\
		CC(ex,0xb1);											\
	}

/***************************************************************
 * INIR
 ***************************************************************/
#define INIR													\
	INI;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb2);											\
	}

/***************************************************************
 * OTIR
 ***************************************************************/
#define OTIR													\
	OUTI;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb3);											\
	}

/***************************************************************
 * LDDR
 ***************************************************************/
#define LDDR													\
	LDD;														\
	if( _BC )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb8);											\
	}

/***************************************************************
 * CPDR
 ***************************************************************/
#define CPDR													\
	CPD;														\
	if( _BC && !(_F & ZF) ) 									\
	{															\
		_PC -= 2;												\
		CC(ex,0xb9);											\
	}

/***************************************************************
 * INDR
 ***************************************************************/
#define INDR													\
	IND;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xba);											\
	}

/***************************************************************
 * OTDR
 ***************************************************************/
#define OTDR													\
	OUTD;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xbb);											\
	}

/***************************************************************
 * EI
 ***************************************************************/
#define EI {													\
	_IFF1 = _IFF2 = 1;											\
	Z180.after_EI = 1;											\
}

/***************************************************************
 * TST  n
 ***************************************************************/
#define TST(value)												\
	_F = SZP[_A & value] | HF

/***************************************************************
 * MLT  rr
 ***************************************************************/
#define MLT(DR) {												\
	Z180.DR.w.l = Z180.DR.b.l * Z180.DR.b.h;					\
}

/***************************************************************
 * OTIM
 ***************************************************************/
#define OTIM {													\
	_B--;														\
	OUT( _C, RM(_HL) ); 										\
	_HL++;														\
	_C++;														\
	_F = (_B) ? NF : NF | ZF;									\
}

/***************************************************************
 * OTDM
 ***************************************************************/
#define OTDM {													\
	_B--;														\
	OUT( _C, RM(_HL) ); 										\
	_HL--;														\
	_C--;														\
	_F = (_B) ? NF : NF | ZF;									\
}

/***************************************************************
 * OTIMR
 ***************************************************************/
#define OTIMR													\
	OTIM;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb3);											\
	}

/***************************************************************
 * OTDMR
 ***************************************************************/
#define OTDMR													\
	OTDM;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb3);											\
	}

/***************************************************************
 * OTDMR
 ***************************************************************/
#define SLP {													\
	z180_icount = 0;											\
	_HALT = 2;													\
}

