/*******************************************************
 *
 *      Portable (hopefully ;-) 8085A emulator
 *
 *      Written by J. Buchmueller for use with MAME
 *
 *      Partially based on Z80Em by Marcel De Kogel
 *
 *      CPU related macros
 *
 *******************************************************/


#define SF              0x80
#define ZF              0x40
#define YF              0x20
#define HF              0x10
#define XF              0x08
#define VF              0x04
#define NF              0x02
#define CF              0x01

#define IM_SID          0x80
#define IM_SOD          0x40
//#define IM_IEN          0x20
#define IM_INTR         0x20 //AT: the 8085 ignores bit 0x20. we move IM_INTR here for compatibility.
#define IM_TRAP         0x10
//#define IM_INTR         0x08
#define IM_IEN          0x08 //AT: RIM returns IEN status on this bit. SIM checks this bit to allow masking RST55-75
#define IM_RST75        0x04
#define IM_RST65        0x02
#define IM_RST55        0x01

#define ADDR_TRAP       0x0024
#define ADDR_RST55      0x002c
#define ADDR_RST65      0x0034
#define ADDR_RST75      0x003c
#define ADDR_INTR       0x0038

#define M_INR(R) {UINT8 hc = ((R & 0x0f) == 0x0f) ? HF : 0; ++R; cpustate->AF.b.l= (cpustate->AF.b.l & CF ) | ZSP[R] | hc; }
#define M_DCR(R) {UINT8 hc = ((R & 0x0f) == 0x00) ? HF : 0; --R; cpustate->AF.b.l= (cpustate->AF.b.l & CF ) | ZSP[R] | hc | NF; }
#define M_MVI(R) R=ARG(cpustate)

#define M_ANA(R) { int i = (((cpustate->AF.b.h | R)>>3) & 1)*HF; cpustate->AF.b.h&=R; cpustate->AF.b.l=ZSP[cpustate->AF.b.h]; if( cpustate->cputype ) { cpustate->AF.b.l |= HF; } else {cpustate->AF.b.l |= i; } }
#define M_ORA(R) cpustate->AF.b.h|=R; cpustate->AF.b.l=ZSP[cpustate->AF.b.h]
#define M_XRA(R) cpustate->AF.b.h^=R; cpustate->AF.b.l=ZSP[cpustate->AF.b.h]

#define M_RLC { 												\
	cpustate->AF.b.h = (cpustate->AF.b.h << 1) | (cpustate->AF.b.h >> 7);				\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h & CF);				\
}

#define M_RRC { 												\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h & CF);				\
	cpustate->AF.b.h = (cpustate->AF.b.h >> 1) | (cpustate->AF.b.h << 7);				\
}

#define M_RAL { 												\
	int c = cpustate->AF.b.l&CF;										\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h >> 7);				\
	cpustate->AF.b.h = (cpustate->AF.b.h << 1) | c; 							\
}

#define M_RAR { 												\
	int c = (cpustate->AF.b.l&CF) << 7; 								\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h & CF);				\
	cpustate->AF.b.h = (cpustate->AF.b.h >> 1) | c; 							\
}

#define M_ADD(R) {							\
int q = cpustate->AF.b.h+R; 							\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)| 				\
		((cpustate->AF.b.h^q^R)&HF)|					\
		(((R^cpustate->AF.b.h^SF)&(R^q)&SF)>>5);			\
	cpustate->AF.b.h=q; 							\
}

#define M_ADC(R) {						\
	int q = cpustate->AF.b.h+R+(cpustate->AF.b.l&CF);			\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)| 			\
		((cpustate->AF.b.h^q^R)&HF)|				\
		(((R^cpustate->AF.b.h^SF)&(R^q)&SF)>>5);		\
	cpustate->AF.b.h=q; 						\
}

#define M_SUB(R) {							\
	int q = cpustate->AF.b.h-R; 						\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|NF|				\
		((cpustate->AF.b.h^q^R)&HF)|					\
		(((R^cpustate->AF.b.h)&(cpustate->AF.b.h^q)&SF)>>5);			\
	cpustate->AF.b.h=q; 							\
}

#define M_SBB(R) {                                              \
	int q = cpustate->AF.b.h-R-(cpustate->AF.b.l&CF);			\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|NF|			\
		((cpustate->AF.b.h^q^R)&HF)|				\
		(((R^cpustate->AF.b.h)&(cpustate->AF.b.h^q)&SF)>>5);		\
	cpustate->AF.b.h=q; 						\
}

#define M_CMP(R) {                                              	\
	int q = cpustate->AF.b.h-R; 						\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|NF|				\
		((cpustate->AF.b.h^q^R)&HF)|					\
		(((R^cpustate->AF.b.h)&(cpustate->AF.b.h^q)&SF)>>5);			\
}

#define M_IN													\
	cpustate->STATUS = 0x42; 											\
	cpustate->XX.d=ARG(cpustate);												\
	cpustate->AF.b.h=memory_read_byte_8le(cpustate->io, cpustate->XX.d);

#define M_OUT													\
	cpustate->STATUS = 0x10; 											\
	cpustate->XX.d=ARG(cpustate);												\
	memory_write_byte_8le(cpustate->io, cpustate->XX.d,cpustate->AF.b.h)

#define M_DAD(R) {                                              \
	int q = cpustate->HL.d + cpustate->R.d; 									\
	cpustate->AF.b.l = ( cpustate->AF.b.l & ~(HF+CF) ) |						\
		( ((cpustate->HL.d^q^cpustate->R.d) >> 8) & HF ) |						\
		( (q>>16) & CF );										\
	cpustate->HL.w.l = q;												\
}

#define M_PUSH(R) {                                             \
	cpustate->STATUS = 0x04; 											\
	memory_write_byte_8le(cpustate->program, --cpustate->SP.w.l, cpustate->R.b.h);									\
	memory_write_byte_8le(cpustate->program, --cpustate->SP.w.l, cpustate->R.b.l);									\
}

#define M_POP(R) {												\
	cpustate->STATUS = 0x86;											\
	cpustate->R.b.l = memory_read_byte_8le(cpustate->program, cpustate->SP.w.l++);									\
	cpustate->R.b.h = memory_read_byte_8le(cpustate->program, cpustate->SP.w.l++);									\
}

#define M_RET(cc)												\
{																\
	if (cc) 													\
	{															\
		cpustate->icount -= 6;										\
		M_POP(PC);												\
	}															\
}

// On 8085 jump if condition is not satisfied is shorter
#define M_JMP(cc) { 											\
	if (cc) {													\
		cpustate->PC.w.l = ARG16(cpustate); 									\
	} else {													\
		cpustate->PC.w.l += 2;											\
		cpustate->icount += (cpustate->cputype) ? 3 : 0;					\
	}															\
}

// On 8085 call if condition is not satisfied is 9 ticks
#define M_CALL(cc)												\
{																\
	if (cc) 													\
	{															\
		UINT16 a = ARG16(cpustate); 									\
		cpustate->icount -= (cpustate->cputype) ? 7 : 6 ;					\
		M_PUSH(PC); 											\
		cpustate->PC.d = a; 											\
	} else {													\
		cpustate->PC.w.l += 2;											\
		cpustate->icount += (cpustate->cputype) ? 2 : 0;					\
	}															\
}

#define M_RST(nn) { 											\
	M_PUSH(PC); 												\
	cpustate->PC.d = 8 * nn;											\
}

#define M_DSUB(cpustate) {												\
	int q = cpustate->HL.b.l-cpustate->BC.b.l;									\
	cpustate->AF.b.l=ZS[q&255]|((q>>8)&CF)|NF|							\
		((cpustate->HL.b.l^q^cpustate->BC.b.l)&HF)|								\
		(((cpustate->BC.b.l^cpustate->HL.b.l)&(cpustate->HL.b.l^q)&SF)>>5);				\
	cpustate->HL.b.l=q; 												\
	q = cpustate->HL.b.h-cpustate->BC.b.h-(cpustate->AF.b.l&CF);						\
	cpustate->AF.b.l=ZS[q&255]|((q>>8)&CF)|NF|							\
		((cpustate->HL.b.h^q^cpustate->BC.b.h)&HF)|								\
		(((cpustate->BC.b.h^cpustate->HL.b.h)&(cpustate->HL.b.h^q)&SF)>>5);				\
	if (cpustate->HL.b.l!=0) cpustate->AF.b.l&=~ZF;								\
}
