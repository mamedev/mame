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

#define M_INR(R) ++R; I.AF.b.l=(I.AF.b.l&CF)|ZS[R]|((R==0x80)?VF:0)|((R&0x0F)?0:HF)
#define M_DCR(R) I.AF.b.l=(I.AF.b.l&CF)|NF|((R==0x80)?VF:0)|((R&0x0F)?0:HF); I.AF.b.l|=ZS[--R]
#define M_MVI(R) R=ARG()

#define M_ANA(R) I.AF.b.h&=R; I.AF.b.l=ZSP[I.AF.b.h]|HF
#define M_ORA(R) I.AF.b.h|=R; I.AF.b.l=ZSP[I.AF.b.h]
#define M_XRA(R) I.AF.b.h^=R; I.AF.b.l=ZSP[I.AF.b.h]

#define M_RLC { 												\
	I.AF.b.h = (I.AF.b.h << 1) | (I.AF.b.h >> 7);				\
	I.AF.b.l = (I.AF.b.l & ~(HF+NF+CF)) | (I.AF.b.h & CF);		\
}

#define M_RRC { 												\
	I.AF.b.l = (I.AF.b.l & ~(HF+NF+CF)) | (I.AF.b.h & CF);		\
	I.AF.b.h = (I.AF.b.h >> 1) | (I.AF.b.h << 7);				\
}

#define M_RAL { 												\
	int c = I.AF.b.l&CF;										\
	I.AF.b.l = (I.AF.b.l & ~(HF+NF+CF)) | (I.AF.b.h >> 7);		\
	I.AF.b.h = (I.AF.b.h << 1) | c; 							\
}

#define M_RAR { 												\
	int c = (I.AF.b.l&CF) << 7; 								\
	I.AF.b.l = (I.AF.b.l & ~(HF+NF+CF)) | (I.AF.b.h & CF);		\
	I.AF.b.h = (I.AF.b.h >> 1) | c; 							\
}

#define M_ADD(R) {							\
int q = I.AF.b.h+R; 							\
	I.AF.b.l=ZSP[q&255]|((q>>8)&CF)| 				\
		((I.AF.b.h^q^R)&HF)|					\
		(((R^I.AF.b.h^SF)&(R^q)&SF)>>5);			\
	I.AF.b.h=q; 							\
}

#define M_ADC(R) {						\
	int q = I.AF.b.h+R+(I.AF.b.l&CF);			\
	I.AF.b.l=ZSP[q&255]|((q>>8)&CF)| 			\
		((I.AF.b.h^q^R)&HF)|				\
		(((R^I.AF.b.h^SF)&(R^q)&SF)>>5);		\
	I.AF.b.h=q; 						\
}

#define M_SUB(R) {							\
	int q = I.AF.b.h-R; 						\
	I.AF.b.l=ZSP[q&255]|((q>>8)&CF)|NF|				\
		((I.AF.b.h^q^R)&HF)|					\
		(((R^I.AF.b.h)&(I.AF.b.h^q)&SF)>>5);			\
	I.AF.b.h=q; 							\
}

#define M_SBB(R) {                                              \
	int q = I.AF.b.h-R-(I.AF.b.l&CF);			\
	I.AF.b.l=ZSP[q&255]|((q>>8)&CF)|NF|			\
		((I.AF.b.h^q^R)&HF)|				\
		(((R^I.AF.b.h)&(I.AF.b.h^q)&SF)>>5);		\
	I.AF.b.h=q; 						\
}

#define M_CMP(R) {                                              	\
	int q = I.AF.b.h-R; 						\
	I.AF.b.l=ZSP[q&255]|((q>>8)&CF)|NF|				\
		((I.AF.b.h^q^R)&HF)|					\
		(((R^I.AF.b.h)&(I.AF.b.h^q)&SF)>>5);			\
}

#define M_IN													\
	I.XX.d=ARG();												\
	I.AF.b.h=io_read_byte_8(I.XX.d);

#define M_OUT													\
	I.XX.d=ARG();												\
	io_write_byte_8(I.XX.d,I.AF.b.h)

#define M_DAD(R) {                                              \
	int q = I.HL.d + I.R.d; 									\
	I.AF.b.l = ( I.AF.b.l & ~(HF+CF) ) |						\
		( ((I.HL.d^q^I.R.d) >> 8) & HF ) |						\
		( (q>>16) & CF );										\
	I.HL.w.l = q;												\
}

#define M_PUSH(R) {                                             \
	WM(--I.SP.w.l, I.R.b.h);									\
	WM(--I.SP.w.l, I.R.b.l);									\
}

#define M_POP(R) {												\
	I.R.b.l = RM(I.SP.w.l++);									\
	I.R.b.h = RM(I.SP.w.l++);									\
}

#define M_RET(cc)												\
{																\
	if (cc) 													\
	{															\
		i8085_ICount -= 6;										\
		M_POP(PC);												\
		change_pc(I.PC.d);									\
	}															\
}

#define M_JMP(cc) { 											\
	if (cc) {													\
		i8085_ICount -= 3;										\
		I.PC.w.l = ARG16(); 									\
		change_pc(I.PC.d);									\
	} else I.PC.w.l += 2;										\
}

#define M_CALL(cc)												\
{																\
	if (cc) 													\
	{															\
		UINT16 a = ARG16(); 									\
		i8085_ICount -= 6;										\
		M_PUSH(PC); 											\
		I.PC.d = a; 											\
		change_pc(I.PC.d);									\
	} else I.PC.w.l += 2;										\
}

#define M_RST(nn) { 											\
	M_PUSH(PC); 												\
	I.PC.d = 8 * nn;											\
	change_pc(I.PC.d);										\
}

#define M_DSUB() {												\
	int q = I.HL.b.l-I.BC.b.l;									\
	I.AF.b.l=ZS[q&255]|((q>>8)&CF)|NF|							\
		((I.HL.b.l^q^I.BC.b.l)&HF)|								\
		(((I.BC.b.l^I.HL.b.l)&(I.HL.b.l^q)&SF)>>5);				\
	I.HL.b.l=q; 												\
	q = I.HL.b.h-I.BC.b.h-(I.AF.b.l&CF);						\
	I.AF.b.l=ZS[q&255]|((q>>8)&CF)|NF|							\
		((I.HL.b.h^q^I.BC.b.h)&HF)|								\
		(((I.BC.b.h^I.HL.b.h)&(I.HL.b.h^q)&SF)>>5);				\
	if (I.HL.b.l!=0) I.AF.b.l&=~ZF;								\
}
