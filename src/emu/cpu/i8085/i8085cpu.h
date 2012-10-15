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
#define X5F             0x20
#define HF              0x10
#define X3F             0x08
#define PF              0x04
#define VF              0x02
#define CF              0x01

#define IM_SID          0x80
#define IM_I75          0x40
#define IM_I65          0x20
#define IM_I55          0x10
#define IM_IE           0x08
#define IM_M75          0x04
#define IM_M65          0x02
#define IM_M55          0x01

#define ADDR_TRAP       0x0024
#define ADDR_RST55      0x002c
#define ADDR_RST65      0x0034
#define ADDR_RST75      0x003c
#define ADDR_INTR       0x0038


#define M_MVI(R) R=ARG(cpustate)

/* rotate */
#define M_RLC { 																	\
	cpustate->AF.b.h = (cpustate->AF.b.h << 1) | (cpustate->AF.b.h >> 7);			\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h & CF);			\
}

#define M_RRC { 																	\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h & CF);			\
	cpustate->AF.b.h = (cpustate->AF.b.h >> 1) | (cpustate->AF.b.h << 7);			\
}

#define M_RAL { 																	\
	int c = cpustate->AF.b.l&CF;													\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h >> 7);			\
	cpustate->AF.b.h = (cpustate->AF.b.h << 1) | c; 								\
}

#define M_RAR { 																	\
	int c = (cpustate->AF.b.l&CF) << 7; 											\
	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (cpustate->AF.b.h & CF);			\
	cpustate->AF.b.h = (cpustate->AF.b.h >> 1) | c; 								\
}

/* logical */
#define M_ORA(R) cpustate->AF.b.h|=R; cpustate->AF.b.l=ZSP[cpustate->AF.b.h]
#define M_XRA(R) cpustate->AF.b.h^=R; cpustate->AF.b.l=ZSP[cpustate->AF.b.h]
#define M_ANA(R) {UINT8 hc = ((cpustate->AF.b.h | R)<<1) & HF; cpustate->AF.b.h&=R; cpustate->AF.b.l=ZSP[cpustate->AF.b.h]; if(IS_8085(cpustate)) { cpustate->AF.b.l |= HF; } else {cpustate->AF.b.l |= hc; } }

/* increase / decrease */
#define M_INR(R) {UINT8 hc = ((R & 0x0f) == 0x0f) ? HF : 0; ++R; cpustate->AF.b.l= (cpustate->AF.b.l & CF ) | ZSP[R] | hc; }
#define M_DCR(R) {UINT8 hc = ((R & 0x0f) != 0x00) ? HF : 0; --R; cpustate->AF.b.l= (cpustate->AF.b.l & CF ) | ZSP[R] | hc | VF; }

/* arithmetic */
#define M_ADD(R) {																	\
	int q = cpustate->AF.b.h+R;														\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|((cpustate->AF.b.h^q^R)&HF);			\
	cpustate->AF.b.h=q;																\
}

#define M_ADC(R) {																	\
	int q = cpustate->AF.b.h+R+(cpustate->AF.b.l&CF);								\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|((cpustate->AF.b.h^q^R)&HF);			\
	cpustate->AF.b.h=q;																\
}

#define M_SUB(R) {																	\
	int q = cpustate->AF.b.h-R;														\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|(~(cpustate->AF.b.h^q^R)&HF)|VF;		\
	cpustate->AF.b.h=q;																\
}

#define M_SBB(R) {																	\
	int q = cpustate->AF.b.h-R-(cpustate->AF.b.l&CF);								\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|(~(cpustate->AF.b.h^q^R)&HF)|VF;		\
	cpustate->AF.b.h=q;																\
}

#define M_CMP(R) {																	\
	int q = cpustate->AF.b.h-R;														\
	cpustate->AF.b.l=ZSP[q&255]|((q>>8)&CF)|(~(cpustate->AF.b.h^q^R)&HF)|VF;		\
}

#define M_DAD(R) {																	\
	int q = cpustate->HL.d + cpustate->R.d;											\
	cpustate->AF.b.l = (cpustate->AF.b.l & ~CF) | (q>>16 & CF );					\
	cpustate->HL.w.l = q;															\
}

// DSUB is 8085-only, not sure if H flag handling is correct
#define M_DSUB(cpustate) {															\
	int q = cpustate->HL.b.l-cpustate->BC.b.l;										\
	cpustate->AF.b.l=ZS[q&255]|((q>>8)&CF)|VF|										\
		((cpustate->HL.b.l^q^cpustate->BC.b.l)&HF)|									\
		(((cpustate->BC.b.l^cpustate->HL.b.l)&(cpustate->HL.b.l^q)&SF)>>5);			\
	cpustate->HL.b.l=q; 															\
	q = cpustate->HL.b.h-cpustate->BC.b.h-(cpustate->AF.b.l&CF);					\
	cpustate->AF.b.l=ZS[q&255]|((q>>8)&CF)|VF|										\
		((cpustate->HL.b.h^q^cpustate->BC.b.h)&HF)|									\
		(((cpustate->BC.b.h^cpustate->HL.b.h)&(cpustate->HL.b.h^q)&SF)>>5);			\
	if (cpustate->HL.b.l!=0) cpustate->AF.b.l&=~ZF;									\
}

/* i/o */
#define M_IN																		\
	cpustate->STATUS = 0x42;														\
	cpustate->WZ.d=ARG(cpustate);													\
	cpustate->AF.b.h=cpustate->io->read_byte(cpustate->WZ.d);

#define M_OUT																		\
	cpustate->STATUS = 0x10;														\
	cpustate->WZ.d=ARG(cpustate);													\
	cpustate->io->write_byte(cpustate->WZ.d,cpustate->AF.b.h)

/* stack */
#define M_PUSH(R) {                                             					\
	cpustate->STATUS = 0x04;														\
	cpustate->program->write_byte(--cpustate->SP.w.l, cpustate->R.b.h);	\
	cpustate->program->write_byte(--cpustate->SP.w.l, cpustate->R.b.l);	\
}

#define M_POP(R) {																	\
	cpustate->STATUS = 0x86;														\
	cpustate->R.b.l = cpustate->program->read_byte(cpustate->SP.w.l++);	\
	cpustate->R.b.h = cpustate->program->read_byte(cpustate->SP.w.l++);	\
}

/* jumps */
// On 8085 jump if condition is not satisfied is shorter
#define M_JMP(cc) { 																\
	if (cc) {																		\
		cpustate->PC.w.l = ARG16(cpustate); 										\
	} else {																		\
		cpustate->PC.w.l += 2;														\
		cpustate->icount += (IS_8085(cpustate)) ? 3 : 0;							\
	}																				\
}

// On 8085 call if condition is not satisfied is 9 ticks
#define M_CALL(cc)																	\
{																					\
	if (cc) 																		\
	{																				\
		UINT16 a = ARG16(cpustate); 												\
		cpustate->icount -= (IS_8085(cpustate)) ? 7 : 6 ;							\
		M_PUSH(PC); 																\
		cpustate->PC.d = a; 														\
	} else {																		\
		cpustate->PC.w.l += 2;														\
		cpustate->icount += (IS_8085(cpustate)) ? 2 : 0;							\
	}																				\
}

// conditional RET only
#define M_RET(cc)																	\
{																					\
	if (cc) 																		\
	{																				\
		cpustate->icount -= 6;														\
		M_POP(PC);																	\
	}																				\
}

#define M_RST(nn) { 																\
	M_PUSH(PC); 																	\
	cpustate->PC.d = 8 * nn;														\
}

