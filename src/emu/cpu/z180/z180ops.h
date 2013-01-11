/***************************************************************
 * Enter HALT state; write 1 to fake port on first execution
 ***************************************************************/
#define ENTER_HALT(cs) {                                        \
	(cs)->_PC--;                                                \
	(cs)->HALT = 1;                                             \
}

/***************************************************************
 * Leave HALT state; write 0 to fake port
 ***************************************************************/
#define LEAVE_HALT(cs) {                                        \
	if( (cs)->HALT )                                            \
	{                                                           \
		(cs)->HALT = 0;                                         \
		(cs)->_PC++;                                            \
	}                                                           \
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
#define IN(cs,port)                                             \
	(((port ^ (cs)->IO_IOCR) & 0xffc0) == 0) ?                  \
		z180_readcontrol(cs, port) : (cs)->iospace->read_byte(port)

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
#define OUT(cs,port,value)                                      \
	if (((port ^ (cs)->IO_IOCR) & 0xffc0) == 0)                 \
		z180_writecontrol(cs,port,value);                       \
	else (cs)->iospace->write_byte(port,value)

/***************************************************************
 * MMU calculate the memory managemant lookup table
 * bb and cb specify a 4K page
 * If the 4 most significant bits of an 16 bit address are
 * greater or equal to the bank base, the bank base register
 * specifies the 4K offset into the 20 bit address space.
 * If the 4 bits are also greater or equal to the common base,
 * the common base register is used to specify the offset.
 ***************************************************************/
INLINE void z180_mmu(z180_state *cpustate)
{
	offs_t addr, page, bb, cb;
	bb = cpustate->IO_CBAR & 15;
	cb = cpustate->IO_CBAR >> 4;
	for( page = 0; page < 16; page++ )
	{
		addr = page << 12;
		if (page >= bb)
		{
			if (page >= cb)
				addr += (cpustate->IO_CBR << 12);
			else
				addr += (cpustate->IO_BBR << 12);
		}
		cpustate->mmu[page] = (addr & 0xfffff);
	}
}


#define MMU_REMAP_ADDR(cs,addr) ((cs)->mmu[((addr)>>12)&15]|((addr)&4095))

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
#define RM(cs,addr) (cs)->program->read_byte(MMU_REMAP_ADDR(cs,addr))
#ifdef UNUSED_FUNCTION
UINT8 z180_readmem(device_t *device, offs_t offset)
{
	z180_state *cpustate = get_safe_token(device);
	return RM(cpustate, offset);
}
#endif

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
#define WM(cs,addr,value) (cs)->program->write_byte(MMU_REMAP_ADDR(cs,addr),value)
#ifdef UNUSED_FUNCTION
void z180_writemem(device_t *device, offs_t offset, UINT8 data)
{
	z180_state *cpustate = get_safe_token(device);
	WM(cpustate, offset, data);
}
#endif

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
INLINE void RM16( z180_state *cpustate, offs_t addr, PAIR *r )
{
	r->b.l = RM(cpustate, addr);
	r->b.h = RM(cpustate, addr+1);
}

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
INLINE void WM16( z180_state *cpustate, offs_t addr, PAIR *r )
{
	WM(cpustate, addr, r->b.l);
	WM(cpustate, addr+1, r->b.h);
}

/***************************************************************
 * ROP(cpustate) is identical to RM() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
INLINE UINT8 ROP(z180_state *cpustate)
{
	offs_t addr = cpustate->_PCD;
	cpustate->_PC++;
	return cpustate->direct->read_decrypted_byte(MMU_REMAP_ADDR(cpustate, addr));
}

/****************************************************************
 * ARG(cpustate) is identical to ROP(cpustate) except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
INLINE UINT8 ARG(z180_state *cpustate)
{
	offs_t addr = cpustate->_PCD;
	cpustate->_PC++;
	return cpustate->direct->read_raw_byte(MMU_REMAP_ADDR(cpustate, addr));
}

INLINE UINT32 ARG16(z180_state *cpustate)
{
	offs_t addr = cpustate->_PCD;
	cpustate->_PC += 2;
	return cpustate->direct->read_raw_byte(MMU_REMAP_ADDR(cpustate, addr)) | (cpustate->direct->read_raw_byte(MMU_REMAP_ADDR(cpustate, addr+1)) << 8);
}

/***************************************************************
 * Calculate the effective addess cpustate->ea of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
#define EAX(cs) (cs)->ea = (UINT32)(UINT16)((cs)->_IX + (INT8)ARG(cs))
#define EAY(cs) (cs)->ea = (UINT32)(UINT16)((cs)->_IY + (INT8)ARG(cs))

/***************************************************************
 * POP
 ***************************************************************/
#define POP(cs,DR) { RM16(cs, (cs)->_SPD, &(cs)->DR ); (cs)->_SP += 2; }

/***************************************************************
 * PUSH
 ***************************************************************/
#define PUSH(cs,SR) { (cs)->_SP -= 2; WM16(cs, (cs)->_SPD, &(cs)->SR); }

/***************************************************************
 * JP
 ***************************************************************/
#define JP {                                                    \
	cpustate->_PCD = ARG16(cpustate);                                           \
}

/***************************************************************
 * JP_COND
 ***************************************************************/

#define JP_COND(cond)                                           \
	if( cond )                                                  \
	{                                                           \
		cpustate->_PCD = ARG16(cpustate);                                       \
	}                                                           \
	else                                                        \
	{                                                           \
		cpustate->_PC += 2;                                             \
	}

/***************************************************************
 * JR
 ***************************************************************/
#define JR()                                                    \
{                                                               \
	INT8 arg = (INT8)ARG(cpustate); /* ARG(cpustate) also increments cpustate->_PC */   \
	cpustate->_PC += arg;           /* so don't do cpustate->_PC += ARG(cpustate) */      \
}

/***************************************************************
 * JR_COND
 ***************************************************************/
#define JR_COND(cond,opcode)                                    \
	if( cond )                                                  \
	{                                                           \
		INT8 arg = (INT8)ARG(cpustate); /* ARG(cpustate) also increments cpustate->_PC */ \
		cpustate->_PC += arg;           /* so don't do cpustate->_PC += ARG(cpustate) */  \
		CC(ex,opcode);                                          \
	}                                                           \
	else cpustate->_PC++;
/***************************************************************
 * CALL
 ***************************************************************/
#define CALL()                                                  \
	cpustate->ea = ARG16(cpustate);                                             \
	PUSH(cpustate,  PC );                                               \
	cpustate->_PCD = cpustate->ea;

/***************************************************************
 * CALL_COND
 ***************************************************************/
#define CALL_COND(cond,opcode)                                  \
	if( cond )                                                  \
	{                                                           \
		cpustate->ea = ARG16(cpustate);                                         \
		PUSH(cpustate,  PC );                                           \
		cpustate->_PCD = cpustate->ea;                                              \
		CC(ex,opcode);                                          \
	}                                                           \
	else                                                        \
	{                                                           \
		cpustate->_PC+=2;                                               \
	}

/***************************************************************
 * RET_COND
 ***************************************************************/
#define RET_COND(cond,opcode)                                   \
	if( cond )                                                  \
	{                                                           \
		POP(cpustate, PC);                                              \
		CC(ex,opcode);                                          \
	}

/***************************************************************
 * RETN
 ***************************************************************/
#define RETN    {                                               \
	LOG(("Z180 '%s' RETN IFF1:%d IFF2:%d\n", cpustate->device->tag(), cpustate->IFF1, cpustate->IFF2)); \
	POP(cpustate, PC);                                                  \
	cpustate->IFF1 = cpustate->IFF2;                                                \
}

/***************************************************************
 * RETI
 ***************************************************************/
#define RETI    {                                               \
	POP(cpustate, PC);                                                  \
/* according to http://www.msxnet.org/tech/Z80/z80undoc.txt */  \
/*  cpustate->IFF1 = cpustate->IFF2;  */                                            \
	cpustate->daisy.call_reti_device();                 \
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
#define LD_R_A {                                                \
	cpustate->R = cpustate->_A;                                                 \
	cpustate->R2 = cpustate->_A & 0x80;             /* keep bit 7 of R */       \
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
#define LD_A_R {                                                \
	cpustate->_A = (cpustate->R & 0x7f) | cpustate->R2;                                     \
	cpustate->_F = (cpustate->_F & CF) | SZ[cpustate->_A] | ( cpustate->IFF2 << 2 );                    \
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
#define LD_I_A {                                                \
	cpustate->I = cpustate->_A;                                                 \
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
#define LD_A_I {                                                \
	cpustate->_A = cpustate->I;                                                 \
	cpustate->_F = (cpustate->_F & CF) | SZ[cpustate->_A] | ( cpustate->IFF2 << 2 );                    \
}

/***************************************************************
 * RST
 ***************************************************************/
#define RST(addr)                                               \
	PUSH(cpustate,  PC );                                               \
	cpustate->_PCD = addr;

/***************************************************************
 * INC  r8
 ***************************************************************/
INLINE UINT8 INC(z180_state *cpustate, UINT8 value)
{
	UINT8 res = value + 1;
	cpustate->_F = (cpustate->_F & CF) | SZHV_inc[res];
	return (UINT8)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
INLINE UINT8 DEC(z180_state *cpustate, UINT8 value)
{
	UINT8 res = value - 1;
	cpustate->_F = (cpustate->_F & CF) | SZHV_dec[res];
	return res;
}

/***************************************************************
 * RLCA
 ***************************************************************/
#define RLCA                                                    \
	cpustate->_A = (cpustate->_A << 1) | (cpustate->_A >> 7);                               \
	cpustate->_F = (cpustate->_F & (SF | ZF | PF)) | (cpustate->_A & (YF | XF | CF))

/***************************************************************
 * RRCA
 ***************************************************************/
#define RRCA                                                    \
	cpustate->_F = (cpustate->_F & (SF | ZF | PF)) | (cpustate->_A & (YF | XF | CF));       \
	cpustate->_A = (cpustate->_A >> 1) | (cpustate->_A << 7)

/***************************************************************
 * RLA
 ***************************************************************/
#define RLA {                                                   \
	UINT8 res = (cpustate->_A << 1) | (cpustate->_F & CF);                          \
	UINT8 c = (cpustate->_A & 0x80) ? CF : 0;                           \
	cpustate->_F = (cpustate->_F & (SF | ZF | PF)) | c | (res & (YF | XF));         \
	cpustate->_A = res;                                                 \
}

/***************************************************************
 * RRA
 ***************************************************************/
#define RRA {                                                   \
	UINT8 res = (cpustate->_A >> 1) | (cpustate->_F << 7);                          \
	UINT8 c = (cpustate->_A & 0x01) ? CF : 0;                           \
	cpustate->_F = (cpustate->_F & (SF | ZF | PF)) | c | (res & (YF | XF));         \
	cpustate->_A = res;                                                 \
}

/***************************************************************
 * RRD
 ***************************************************************/
#define RRD {                                                   \
	UINT8 n = RM(cpustate, cpustate->_HL);                                          \
	WM(cpustate,  cpustate->_HL, (n >> 4) | (cpustate->_A << 4) );                          \
	cpustate->_A = (cpustate->_A & 0xf0) | (n & 0x0f);                              \
	cpustate->_F = (cpustate->_F & CF) | SZP[cpustate->_A];                                 \
}

/***************************************************************
 * RLD
 ***************************************************************/
#define RLD {                                                   \
	UINT8 n = RM(cpustate, cpustate->_HL);                                          \
	WM(cpustate,  cpustate->_HL, (n << 4) | (cpustate->_A & 0x0f) );                            \
	cpustate->_A = (cpustate->_A & 0xf0) | (n >> 4);                                \
	cpustate->_F = (cpustate->_F & CF) | SZP[cpustate->_A];                                 \
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
#define ADD(value)                                              \
{                                                               \
	UINT32 ah = cpustate->_AFD & 0xff00;                                    \
	UINT32 res = (UINT8)((ah >> 8) + value);                    \
	cpustate->_F = SZHVC_add[ah | res];                                 \
	cpustate->_A = res;                                                 \
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
#define ADC(value)                                              \
{                                                               \
	UINT32 ah = cpustate->_AFD & 0xff00, c = cpustate->_AFD & 1;                    \
	UINT32 res = (UINT8)((ah >> 8) + value + c);                \
	cpustate->_F = SZHVC_add[(c << 16) | ah | res];                     \
	cpustate->_A = res;                                                 \
}

/***************************************************************
 * SUB  n
 ***************************************************************/
#define SUB(value)                                              \
{                                                               \
	UINT32 ah = cpustate->_AFD & 0xff00;                                    \
	UINT32 res = (UINT8)((ah >> 8) - value);                    \
	cpustate->_F = SZHVC_sub[ah | res];                                 \
	cpustate->_A = res;                                                 \
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
#define SBC(value)                                              \
{                                                               \
	UINT32 ah = cpustate->_AFD & 0xff00, c = cpustate->_AFD & 1;                    \
	UINT32 res = (UINT8)((ah >> 8) - value - c);                \
	cpustate->_F = SZHVC_sub[(c<<16) | ah | res];                       \
	cpustate->_A = res;                                                 \
}

/***************************************************************
 * NEG
 ***************************************************************/
#define NEG {                                                   \
	UINT8 value = cpustate->_A;                                         \
	cpustate->_A = 0;                                                   \
	SUB(value);                                                 \
}

/***************************************************************
 * DAA
 ***************************************************************/
#define DAA {                                                   \
	UINT8 r = cpustate->_A;                                         \
	if (cpustate->_F&NF) {                                          \
		if ((cpustate->_F&HF)|((cpustate->_A&0xf)>9)) r-=6;                     \
		if ((cpustate->_F&CF)|(cpustate->_A>0x99)) r-=0x60;                     \
	}                                                   \
	else {                                                  \
		if ((cpustate->_F&HF)|((cpustate->_A&0xf)>9)) r+=6;                     \
		if ((cpustate->_F&CF)|(cpustate->_A>0x99)) r+=0x60;                     \
	}                                                   \
	cpustate->_F=(cpustate->_F&3)|(cpustate->_A>0x99)|((cpustate->_A^r)&HF)|SZP[r];             \
	cpustate->_A=r;                                             \
}

/***************************************************************
 * AND  n
 ***************************************************************/
#define AND(value)                                              \
	cpustate->_A &= value;                                              \
	cpustate->_F = SZP[cpustate->_A] | HF

/***************************************************************
 * OR   n
 ***************************************************************/
#define OR(value)                                               \
	cpustate->_A |= value;                                              \
	cpustate->_F = SZP[cpustate->_A]

/***************************************************************
 * XOR  n
 ***************************************************************/
#define XOR(value)                                              \
	cpustate->_A ^= value;                                              \
	cpustate->_F = SZP[cpustate->_A]

/***************************************************************
 * CP   n
 ***************************************************************/
#define CP(value)                                               \
{                                                               \
	UINT32 ah = cpustate->_AFD & 0xff00;                                    \
	UINT32 res = (UINT8)((ah >> 8) - value);                    \
	cpustate->_F = SZHVC_sub[ah | res];                                 \
}

/***************************************************************
 * EX   AF,AF'
 ***************************************************************/
#define EX_AF {                                                 \
	PAIR tmp;                                                   \
	tmp = cpustate->AF; cpustate->AF = cpustate->AF2; cpustate->AF2 = tmp;          \
}

/***************************************************************
 * EX   DE,HL
 ***************************************************************/
#define EX_DE_HL {                                              \
	PAIR tmp;                                                   \
	tmp = cpustate->DE; cpustate->DE = cpustate->HL; cpustate->HL = tmp;            \
}

/***************************************************************
 * EXX
 ***************************************************************/
#define EXX {                                                   \
	PAIR tmp;                                                   \
	tmp = cpustate->BC; cpustate->BC = cpustate->BC2; cpustate->BC2 = tmp;          \
	tmp = cpustate->DE; cpustate->DE = cpustate->DE2; cpustate->DE2 = tmp;          \
	tmp = cpustate->HL; cpustate->HL = cpustate->HL2; cpustate->HL2 = tmp;          \
}

/***************************************************************
 * EX   (SP),r16
 ***************************************************************/
#define EXSP(DR)                                                \
{                                                               \
	PAIR tmp = { { 0, 0, 0, 0 } };                              \
	RM16(cpustate,  cpustate->_SPD, &tmp );                                         \
	WM16(cpustate,  cpustate->_SPD, &cpustate->DR );                                    \
	cpustate->DR = tmp;                                             \
}


/***************************************************************
 * ADD16
 ***************************************************************/
#define ADD16(DR,SR)                                            \
{                                                               \
	UINT32 res = cpustate->DR.d + cpustate->SR.d;                       \
	cpustate->_F = (cpustate->_F & (SF | ZF | VF)) |                                \
		(((cpustate->DR.d ^ res ^ cpustate->SR.d) >> 8) & HF) |         \
		((res >> 16) & CF);                                     \
	cpustate->DR.w.l = (UINT16)res;                                 \
}

/***************************************************************
 * ADC  r16,r16
 ***************************************************************/
#define ADC16(DR)                                               \
{                                                               \
	UINT32 res = cpustate->_HLD + cpustate->DR.d + (cpustate->_F & CF);                 \
	cpustate->_F = (((cpustate->_HLD ^ res ^ cpustate->DR.d) >> 8) & HF) |              \
		((res >> 16) & CF) |                                    \
		((res >> 8) & SF) |                                     \
		((res & 0xffff) ? 0 : ZF) |                             \
		(((cpustate->DR.d ^ cpustate->_HLD ^ 0x8000) & (cpustate->DR.d ^ res) & 0x8000) >> 13); \
	cpustate->_HL = (UINT16)res;                                            \
}

/***************************************************************
 * SBC  r16,r16
 ***************************************************************/
#define SBC16(DR)                                               \
{                                                               \
	UINT32 res = cpustate->_HLD - cpustate->DR.d - (cpustate->_F & CF);                 \
	cpustate->_F = (((cpustate->_HLD ^ res ^ cpustate->DR.d) >> 8) & HF) | NF |         \
		((res >> 16) & CF) |                                    \
		((res >> 8) & SF) |                                     \
		((res & 0xffff) ? 0 : ZF) |                             \
		(((cpustate->DR.d ^ cpustate->_HLD) & (cpustate->_HLD ^ res) &0x8000) >> 13);   \
	cpustate->_HL = (UINT16)res;                                            \
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
INLINE UINT8 RLC(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RRC  r8
 ***************************************************************/
INLINE UINT8 RRC(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res << 7)) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RL   r8
 ***************************************************************/
INLINE UINT8 RL(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (cpustate->_F & CF)) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RR   r8
 ***************************************************************/
INLINE UINT8 RR(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (cpustate->_F << 7)) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SLA  r8
 ***************************************************************/
INLINE UINT8 SLA(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SRA  r8
 ***************************************************************/
INLINE UINT8 SRA(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res & 0x80)) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SLL  r8
 ***************************************************************/
INLINE UINT8 SLL(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | 0x01) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SRL  r8
 ***************************************************************/
INLINE UINT8 SRL(z180_state *cpustate, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = (res >> 1) & 0xff;
	cpustate->_F = SZP[res] | c;
	return res;
}

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
#undef BIT
#define BIT(bit,reg)                                            \
	cpustate->_F = (cpustate->_F & CF) | HF | SZ_BIT[reg & (1<<bit)]

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
#define BIT_XY(bit,reg)                                         \
	cpustate->_F = (cpustate->_F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | ((cpustate->ea>>8) & (YF|XF))

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
#define LDI {                                                   \
	UINT8 io = RM(cpustate, cpustate->_HL);                                         \
	WM(cpustate,  cpustate->_DE, io );                                              \
	cpustate->_F &= SF | ZF | CF;                                       \
	if( (cpustate->_A + io) & 0x02 ) cpustate->_F |= YF; /* bit 1 -> flag 5 */      \
	if( (cpustate->_A + io) & 0x08 ) cpustate->_F |= XF; /* bit 3 -> flag 3 */      \
	cpustate->_HL++; cpustate->_DE++; cpustate->_BC--;                                      \
	if( cpustate->_BC ) cpustate->_F |= VF;                                         \
}

/***************************************************************
 * CPI
 ***************************************************************/
#define CPI {                                                   \
	UINT8 val = RM(cpustate, cpustate->_HL);                                        \
	UINT8 res = cpustate->_A - val;                                     \
	cpustate->_HL++; cpustate->_BC--;                                               \
	cpustate->_F = (cpustate->_F & CF) | (SZ[res] & ~(YF|XF)) | ((cpustate->_A ^ val ^ res) & HF) | NF;  \
	if( cpustate->_F & HF ) res -= 1;                                   \
	if( res & 0x02 ) cpustate->_F |= YF; /* bit 1 -> flag 5 */          \
	if( res & 0x08 ) cpustate->_F |= XF; /* bit 3 -> flag 3 */          \
	if( cpustate->_BC ) cpustate->_F |= VF;                                         \
}

/***************************************************************
 * INI
 ***************************************************************/
#define INI {                                                   \
	UINT8 io = IN(cpustate, cpustate->_BC);                                         \
	cpustate->_B--;                                                     \
	WM(cpustate,  cpustate->_HL, io );                                              \
	cpustate->_HL++;                                                        \
	cpustate->_F = SZ[cpustate->_B];                                                \
	if( io & SF ) cpustate->_F |= NF;                                   \
	if( (cpustate->_C + io + 1) & 0x100 ) cpustate->_F |= HF | CF;                  \
	if( (irep_tmp1[cpustate->_C & 3][io & 3] ^                          \
			breg_tmp2[cpustate->_B] ^                                       \
			(cpustate->_C >> 2) ^                                           \
			(io >> 2)) & 1 )                                        \
		cpustate->_F |= PF;                                             \
}

/***************************************************************
 * OUTI
 ***************************************************************/
#define OUTI {                                                  \
	UINT8 io = RM(cpustate, cpustate->_HL);                                         \
	cpustate->_B--;                                                     \
	OUT(cpustate,  cpustate->_BC, io );                                             \
	cpustate->_HL++;                                                        \
	cpustate->_F = SZ[cpustate->_B];                                                \
	if( io & SF ) cpustate->_F |= NF;                                   \
	if( (cpustate->_C + io + 1) & 0x100 ) cpustate->_F |= HF | CF;                  \
	if( (irep_tmp1[cpustate->_C & 3][io & 3] ^                          \
			breg_tmp2[cpustate->_B] ^                                       \
			(cpustate->_C >> 2) ^                                           \
			(io >> 2)) & 1 )                                        \
		cpustate->_F |= PF;                                             \
}

/***************************************************************
 * LDD
 ***************************************************************/
#define LDD {                                                   \
	UINT8 io = RM(cpustate, cpustate->_HL);                                         \
	WM(cpustate,  cpustate->_DE, io );                                              \
	cpustate->_F &= SF | ZF | CF;                                       \
	if( (cpustate->_A + io) & 0x02 ) cpustate->_F |= YF; /* bit 1 -> flag 5 */      \
	if( (cpustate->_A + io) & 0x08 ) cpustate->_F |= XF; /* bit 3 -> flag 3 */      \
	cpustate->_HL--; cpustate->_DE--; cpustate->_BC--;                                      \
	if( cpustate->_BC ) cpustate->_F |= VF;                                         \
}

/***************************************************************
 * CPD
 ***************************************************************/
#define CPD {                                                   \
	UINT8 val = RM(cpustate, cpustate->_HL);                                        \
	UINT8 res = cpustate->_A - val;                                     \
	cpustate->_HL--; cpustate->_BC--;                                               \
	cpustate->_F = (cpustate->_F & CF) | (SZ[res] & ~(YF|XF)) | ((cpustate->_A ^ val ^ res) & HF) | NF;  \
	if( cpustate->_F & HF ) res -= 1;                                   \
	if( res & 0x02 ) cpustate->_F |= YF; /* bit 1 -> flag 5 */          \
	if( res & 0x08 ) cpustate->_F |= XF; /* bit 3 -> flag 3 */          \
	if( cpustate->_BC ) cpustate->_F |= VF;                                         \
}

/***************************************************************
 * IND
 ***************************************************************/
#define IND {                                                   \
	UINT8 io = IN(cpustate, cpustate->_BC);                                         \
	cpustate->_B--;                                                     \
	WM(cpustate,  cpustate->_HL, io );                                              \
	cpustate->_HL--;                                                        \
	cpustate->_F = SZ[cpustate->_B];                                                \
	if( io & SF ) cpustate->_F |= NF;                                   \
	if( (cpustate->_C + io - 1) & 0x100 ) cpustate->_F |= HF | CF;                  \
	if( (drep_tmp1[cpustate->_C & 3][io & 3] ^                          \
			breg_tmp2[cpustate->_B] ^                                       \
			(cpustate->_C >> 2) ^                                           \
			(io >> 2)) & 1 )                                        \
		cpustate->_F |= PF;                                             \
}

/***************************************************************
 * OUTD
 ***************************************************************/
#define OUTD {                                                  \
	UINT8 io = RM(cpustate, cpustate->_HL);                                         \
	cpustate->_B--;                                                     \
	OUT(cpustate,  cpustate->_BC, io );                                             \
	cpustate->_HL--;                                                        \
	cpustate->_F = SZ[cpustate->_B];                                                \
	if( io & SF ) cpustate->_F |= NF;                                   \
	if( (cpustate->_C + io - 1) & 0x100 ) cpustate->_F |= HF | CF;                  \
	if( (drep_tmp1[cpustate->_C & 3][io & 3] ^                          \
			breg_tmp2[cpustate->_B] ^                                       \
			(cpustate->_C >> 2) ^                                           \
			(io >> 2)) & 1 )                                        \
		cpustate->_F |= PF;                                             \
}

/***************************************************************
 * LDIR
 ***************************************************************/
#define LDIR                                                    \
	LDI;                                                        \
	if( cpustate->_BC )                                                 \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb0);                                            \
	}

/***************************************************************
 * CPIR
 ***************************************************************/
#define CPIR                                                    \
	CPI;                                                        \
	if( cpustate->_BC && !(cpustate->_F & ZF) )                                     \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb1);                                            \
	}

/***************************************************************
 * INIR
 ***************************************************************/
#define INIR                                                    \
	INI;                                                        \
	if( cpustate->_B )                                                  \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb2);                                            \
	}

/***************************************************************
 * OTIR
 ***************************************************************/
#define OTIR                                                    \
	OUTI;                                                       \
	if( cpustate->_B )                                                  \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb3);                                            \
	}

/***************************************************************
 * LDDR
 ***************************************************************/
#define LDDR                                                    \
	LDD;                                                        \
	if( cpustate->_BC )                                                 \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb8);                                            \
	}

/***************************************************************
 * CPDR
 ***************************************************************/
#define CPDR                                                    \
	CPD;                                                        \
	if( cpustate->_BC && !(cpustate->_F & ZF) )                                     \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb9);                                            \
	}

/***************************************************************
 * INDR
 ***************************************************************/
#define INDR                                                    \
	IND;                                                        \
	if( cpustate->_B )                                                  \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xba);                                            \
	}

/***************************************************************
 * OTDR
 ***************************************************************/
#define OTDR                                                    \
	OUTD;                                                       \
	if( cpustate->_B )                                                  \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xbb);                                            \
	}

/***************************************************************
 * EI
 ***************************************************************/
#define EI {                                                    \
	cpustate->IFF1 = cpustate->IFF2 = 1;                                            \
	cpustate->after_EI = 1;                                         \
}

/***************************************************************
 * TST  n
 ***************************************************************/
#define TST(value)                                              \
	cpustate->_F = SZP[cpustate->_A & value] | HF

/***************************************************************
 * MLT  rr
 ***************************************************************/
#define MLT(DR) {                                               \
	cpustate->DR.w.l = cpustate->DR.b.l * cpustate->DR.b.h;                 \
}

/***************************************************************
 * OTIM
 ***************************************************************/
#define OTIM {                                                  \
	cpustate->_B--;                                                     \
	OUT(cpustate,  cpustate->_C, RM(cpustate, cpustate->_HL) );                                         \
	cpustate->_HL++;                                                        \
	cpustate->_C++;                                                     \
	cpustate->_F = (cpustate->_B) ? NF : NF | ZF;                                   \
}

/***************************************************************
 * OTDM
 ***************************************************************/
#define OTDM {                                                  \
	cpustate->_B--;                                                     \
	OUT(cpustate,  cpustate->_C, RM(cpustate, cpustate->_HL) );                                         \
	cpustate->_HL--;                                                        \
	cpustate->_C--;                                                     \
	cpustate->_F = (cpustate->_B) ? NF : NF | ZF;                                   \
}

/***************************************************************
 * OTIMR
 ***************************************************************/
#define OTIMR                                                   \
	OTIM;                                                       \
	if( cpustate->_B )                                                  \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb3);                                            \
	}

/***************************************************************
 * OTDMR
 ***************************************************************/
#define OTDMR                                                   \
	OTDM;                                                       \
	if( cpustate->_B )                                                  \
	{                                                           \
		cpustate->_PC -= 2;                                             \
		CC(ex,0xb3);                                            \
	}

/***************************************************************
 * OTDMR
 ***************************************************************/
#define SLP {                                                   \
	cpustate->icount = 0;                                           \
	cpustate->HALT = 2;                                                 \
}
