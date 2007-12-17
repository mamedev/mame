/*************************************************************************
 *
 *      Portable Signetics 2650 cpu emulation
 *
 *      Written by Juergen Buchmueller for use with MAME
 *
 *  Version 1.2
 *  - changed to clock cycle counts from machine cycles
 *  - replaced cycle table with inline code (M_RET conditional case)
 *  - removed wrong distinct add/sub CC and OVF handling
 *  - cosmetics, readability
 *
 *************************************************************************/

#include "debugger.h"
#include "s2650.h"
#include "s2650cpu.h"

/* define this to have some interrupt information logged */
#define VERBOSE 0

#if VERBOSE
#define LOG(x) logerror x
#else
#define LOG(x)
#endif

/* define this to expand all EA calculations inline */
#define INLINE_EA	1

int s2650_ICount = 0;

typedef struct {
	UINT16	ppc;	/* previous program counter (page + iar) */
    UINT16  page;   /* 8K page select register (A14..A13) */
    UINT16  iar;    /* instruction address register (A12..A0) */
    UINT16  ea;     /* effective address (A14..A0) */
    UINT8   psl;    /* processor status lower */
    UINT8   psu;    /* processor status upper */
    UINT8   r;      /* absolute addressing dst/src register */
    UINT8   reg[7]; /* 7 general purpose registers */
    UINT8   halt;   /* 1 if cpu is halted */
    UINT8   ir;     /* instruction register */
    UINT16  ras[8]; /* 8 return address stack entries */
	UINT8	irq_state;
    int     (*irq_callback)(int irqline);
}   s2650_Regs;

static s2650_Regs S;

/* condition code changes for a byte */
static const UINT8 ccc[0x200] = {
	0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x04,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84
};

static void s2650_set_sense(int state);


#define CHECK_IRQ_LINE											\
	if (S.irq_state != CLEAR_LINE)								\
	{															\
		if( (S.psu & II) == 0 ) 								\
		{														\
			int vector; 										\
			if (S.halt) 										\
			{													\
				S.halt = 0; 									\
				S.iar = (S.iar + 1) & PMSK; 					\
			}													\
			vector = (*S.irq_callback)(0) & 0xff;				\
			/* build effective address within first 8K page */	\
			S.ea = S2650_relative[vector] & PMSK;				\
			if (vector & 0x80)		/* indirect bit set ? */	\
			{													\
				int addr = S.ea;								\
				s2650_ICount -= 6;					\
				/* build indirect 32K address */				\
				S.ea = RDMEM(addr) << 8;						\
				if (!(++addr & PMSK)) addr -= PLEN; 			\
				S.ea = (S.ea + RDMEM(addr)) & AMSK; 			\
			}													\
			LOG(("S2650 interrupt to $%04x\n", S.ea));\
			S.psu  = (S.psu & ~SP) | ((S.psu + 1) & SP) | II;	\
			S.ras[S.psu & SP] = S.page + S.iar;					\
			S.page = S.ea & PAGE;								\
			S.iar  = S.ea & PMSK;								\
		}														\
	}

/***************************************************************
 *
 * set condition code (zero,plus,minus) from result
 ***************************************************************/
#define SET_CC(result)                                          \
	S.psl = (S.psl & ~CC) | ccc[result]

/***************************************************************
 *
 * set condition code (zero,plus,minus) and overflow
 ***************************************************************/
#define SET_CC_OVF(result,value)                                \
	S.psl = (S.psl & ~(OVF+CC)) |								\
		ccc[result + (((value) & 0x80) << 1)]

#define SET_CC_OVF_ADD(result,value1,value2) SET_CC_OVF(result,~((value1) ^ (value2)) & ((value1) ^ (result)))
#define SET_CC_OVF_SUB(result,value1,value2) SET_CC_OVF(result,~((value1) ^ (value2)) & ((value1) ^ (result)))

/***************************************************************
 * ROP
 * read next opcode
 ***************************************************************/
INLINE UINT8 ROP(void)
{
	UINT8 result = cpu_readop(S.page + S.iar);
	S.iar = (S.iar + 1) & PMSK;
	return result;
}

/***************************************************************
 * ARG
 * read next opcode argument
 ***************************************************************/
INLINE UINT8 ARG(void)
{
	UINT8 result = cpu_readop_arg(S.page + S.iar);
	S.iar = (S.iar + 1) & PMSK;
	return result;
}

/***************************************************************
 * RDMEM
 * read memory byte from addr
 ***************************************************************/
#define RDMEM(addr) program_read_byte_8(addr)

/***************************************************************
 * handy table to build PC relative offsets
 * from HR (holding register)
 ***************************************************************/
static	int 	S2650_relative[0x100] =
{
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-50,-49,
	-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,
	-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,
	-16,-15,-14,-13,-12,-11,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-50,-49,
	-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,
	-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,
	-16,-15,-14,-13,-12,-11,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
};

/***************************************************************
 * _REL_EA
 * build effective address with relative addressing
 ***************************************************************/
#define _REL_EA(page)											\
{																\
	UINT8 hr = ARG();	/* get 'holding register' */            \
	/* build effective address within current 8K page */		\
	S.ea = page + ((S.iar + S2650_relative[hr]) & PMSK);		\
	if (hr & 0x80) { /* indirect bit set ? */					\
		int addr = S.ea;										\
		s2650_ICount -= 6;										\
		/* build indirect 32K address */						\
		S.ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
		S.ea = (S.ea + RDMEM(addr)) & AMSK; 					\
	}															\
}

/***************************************************************
 * _REL_ZERO
 * build effective address with zero relative addressing
 ***************************************************************/
#define _REL_ZERO(page)											\
{																\
	UINT8 hr = ARG();	/* get 'holding register' */            \
	/* build effective address from 0 */						\
	S.ea = (S2650_relative[hr] & PMSK);							\
	if (hr & 0x80) { /* indirect bit set ? */					\
		int addr = S.ea;										\
		s2650_ICount -= 6;										\
		/* build indirect 32K address */						\
		S.ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
		S.ea = (S.ea + RDMEM(addr)) & AMSK; 					\
	}															\
}

/***************************************************************
 * _ABS_EA
 * build effective address with absolute addressing
 ***************************************************************/
#define _ABS_EA()												\
{																\
	UINT8 hr, dr;												\
	hr = ARG(); 	/* get 'holding register' */                \
	dr = ARG(); 	/* get 'data bus register' */               \
	/* build effective address within current 8K page */		\
	S.ea = S.page + (((hr << 8) + dr) & PMSK);					\
	/* indirect addressing ? */ 								\
	if (hr & 0x80) {											\
		int addr = S.ea;										\
		s2650_ICount -= 6;										\
		/* build indirect 32K address */						\
		/* build indirect 32K address */						\
		S.ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
        S.ea = (S.ea + RDMEM(addr)) & AMSK;                     \
	}															\
	/* check indexed addressing modes */						\
	switch (hr & 0x60) {										\
		case 0x00: /* not indexed */							\
			break;												\
		case 0x20: /* auto increment indexed */ 				\
			S.reg[S.r] += 1;									\
			S.ea = (S.ea & PAGE)+((S.ea+S.reg[S.r]) & PMSK);	\
			S.r = 0; /* absolute addressing reg is R0 */		\
			break;												\
		case 0x40: /* auto decrement indexed */ 				\
			S.reg[S.r] -= 1;									\
			S.ea = (S.ea & PAGE)+((S.ea+S.reg[S.r]) & PMSK);	\
			S.r = 0; /* absolute addressing reg is R0 */		\
			break;												\
		case 0x60: /* indexed */								\
			S.ea = (S.ea & PAGE)+((S.ea+S.reg[S.r]) & PMSK);	\
			S.r = 0; /* absolute addressing reg is R0 */		\
			break;												\
	}															\
}

/***************************************************************
 * _BRA_EA
 * build effective address with absolute addressing (branch)
 ***************************************************************/
#define _BRA_EA()												\
{																\
	UINT8 hr, dr;												\
	hr = ARG(); 	/* get 'holding register' */                \
	dr = ARG(); 	/* get 'data bus register' */               \
	/* build address in 32K address space */					\
	S.ea = ((hr << 8) + dr) & AMSK; 							\
	/* indirect addressing ? */ 								\
	if (hr & 0x80) {											\
		int addr = S.ea;										\
		s2650_ICount -= 6;										\
		/* build indirect 32K address */						\
		S.ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
        S.ea = (S.ea + RDMEM(addr)) & AMSK;                     \
	}															\
}

/***************************************************************
 * SWAP_REGS
 * Swap registers r1-r3 with r4-r6 (the second set)
 * This is done everytime the RS bit in PSL changes
 ***************************************************************/
#define SWAP_REGS												\
{																\
	UINT8 tmp;													\
	tmp = S.reg[1]; 											\
	S.reg[1] = S.reg[4];										\
	S.reg[4] = tmp; 											\
	tmp = S.reg[2]; 											\
	S.reg[2] = S.reg[5];										\
	S.reg[5] = tmp; 											\
	tmp = S.reg[3]; 											\
	S.reg[3] = S.reg[6];										\
	S.reg[6] = tmp; 											\
}

/***************************************************************
 * M_BRR
 * Branch relative if cond is true
 ***************************************************************/
#define M_BRR(cond) 											\
{																\
	if (cond)													\
	{															\
		REL_EA( S.page );										\
		S.page = S.ea & PAGE;									\
		S.iar  = S.ea & PMSK;									\
		change_pc(S.ea);										\
	} else S.iar = (S.iar + 1) & PMSK;							\
}

/***************************************************************
 * M_ZBRR
 * Branch relative to page zero
 ***************************************************************/
#define M_ZBRR()												\
{																\
	REL_ZERO( 0 );												\
	S.page = S.ea & PAGE;										\
	S.iar  = S.ea & PMSK;										\
	change_pc(S.ea);											\
}

/***************************************************************
 * M_BRA
 * Branch absolute if cond is true
 ***************************************************************/
#define M_BRA(cond) 											\
{																\
	if( cond )													\
	{															\
		BRA_EA();												\
		S.page = S.ea & PAGE;									\
		S.iar  = S.ea & PMSK;									\
		change_pc(S.ea);										\
	} else S.iar = (S.iar + 2) & PMSK;							\
}

/***************************************************************
 * M_BXA
 * Branch indexed absolute (EA + R3)
 ***************************************************************/
#define M_BXA() 												\
{																\
	BRA_EA();													\
	S.ea   = (S.ea + S.reg[3]) & AMSK;							\
	S.page = S.ea & PAGE;										\
	S.iar  = S.ea & PMSK;										\
	change_pc(S.ea);											\
}

/***************************************************************
 * M_BSR
 * Branch to subroutine relative if cond is true
 ***************************************************************/
#define M_BSR(cond) 											\
{																\
	if( cond )													\
	{															\
		REL_EA(S.page); 										\
		S.psu  = (S.psu & ~SP) | ((S.psu + 1) & SP);			\
		S.ras[S.psu & SP] = S.page + S.iar;						\
		S.page = S.ea & PAGE;									\
		S.iar  = S.ea & PMSK;									\
		change_pc(S.ea);										\
	} else	S.iar = (S.iar + 1) & PMSK; 						\
}

/***************************************************************
 * M_ZBSR
 * Branch to subroutine relative to page zero
 ***************************************************************/
#define M_ZBSR()												\
{																\
	REL_ZERO(0); 											    \
	S.psu  = (S.psu & ~SP) | ((S.psu + 1) & SP);				\
	S.ras[S.psu & SP] = S.page + S.iar;							\
	S.page = S.ea & PAGE;										\
	S.iar  = S.ea & PMSK;										\
	change_pc(S.ea);											\
}

/***************************************************************
 * M_BSA
 * Branch to subroutine absolute
 ***************************************************************/
#define M_BSA(cond) 											\
{																\
	if( cond )													\
	{															\
		BRA_EA();												\
		S.psu = (S.psu & ~SP) | ((S.psu + 1) & SP); 			\
		S.ras[S.psu & SP] = S.page + S.iar;						\
		S.page = S.ea & PAGE;									\
		S.iar  = S.ea & PMSK;									\
		change_pc(S.ea);										\
	} else S.iar = (S.iar + 2) & PMSK;							\
}

/***************************************************************
 * M_BSXA
 * Branch to subroutine indexed absolute (EA + R3)
 ***************************************************************/
#define M_BSXA()												\
{																\
	BRA_EA();													\
	S.ea  = (S.ea + S.reg[3]) & AMSK;							\
	S.psu = (S.psu & ~SP) | ((S.psu + 1) & SP); 				\
	S.ras[S.psu & SP] = S.page + S.iar;							\
	S.page = S.ea & PAGE;										\
	S.iar  = S.ea & PMSK;										\
	change_pc(S.ea);											\
}

/***************************************************************
 * M_RET
 * Return from subroutine if cond is true
 ***************************************************************/
#define M_RET(cond) 											\
{																\
	if( cond )													\
	{															\
		s2650_ICount -= 6;										\
		S.ea = S.ras[S.psu & SP];								\
		S.psu = (S.psu & ~SP) | ((S.psu - 1) & SP); 			\
		S.page = S.ea & PAGE;									\
		S.iar  = S.ea & PMSK;									\
		change_pc(S.ea);										\
	}															\
}

/***************************************************************
 * M_RETE
 * Return from subroutine if cond is true
 * and enable interrupts; afterwards check IRQ line
 * state and eventually take next interrupt
 ***************************************************************/
#define M_RETE(cond)											\
{																\
	if( cond )													\
	{															\
		S.ea = S.ras[S.psu & SP];								\
		S.psu = (S.psu & ~SP) | ((S.psu - 1) & SP); 			\
		S.page = S.ea & PAGE;									\
		S.iar  = S.ea & PMSK;									\
		change_pc(S.ea);										\
		S.psu &= ~II;											\
		CHECK_IRQ_LINE; 										\
	}															\
}

/***************************************************************
 * M_LOD
 * Load destination with source register
 ***************************************************************/
#define M_LOD(dest,source)										\
{																\
	dest = source;												\
	SET_CC(dest);												\
}

/***************************************************************
 * M_STR
 * Store source register to memory addr (CC unchanged)
 ***************************************************************/
#define M_STR(address,source)									\
	program_write_byte_8(address, source)

/***************************************************************
 * M_AND
 * Logical and destination with source
 ***************************************************************/
#define M_AND(dest,source)										\
{																\
	dest &= source; 											\
	SET_CC(dest);												\
}

/***************************************************************
 * M_IOR
 * Logical inclusive or destination with source
 ***************************************************************/
#define M_IOR(dest,source)										\
{																\
	dest |= source; 											\
	SET_CC(dest);												\
}

/***************************************************************
 * M_EOR
 * Logical exclusive or destination with source
 ***************************************************************/
#define M_EOR(dest,source)										\
{																\
	dest ^= source; 											\
	SET_CC(dest);												\
}

/***************************************************************
 * M_ADD
 * Add source to destination
 * Add with carry if WC flag of PSL is set
 ***************************************************************/
#define M_ADD(dest,_source)										\
{																\
	UINT8 source = _source;										\
	UINT8 before = dest;										\
	/* add source; carry only if WC is set */					\
	UINT16 res = dest + source + ((S.psl >> 3) & S.psl & C);	\
	S.psl &= ~(C | OVF | IDC);									\
	if(res & 0x100) S.psl |= C; 							    \
    dest = res & 0xff;                                          \
	if( (dest & 15) < (before & 15) ) S.psl |= IDC; 			\
	SET_CC_OVF_ADD(dest,before,source);							\
}

/***************************************************************
 * M_SUB
 * Subtract source from destination
 * Subtract with borrow if WC flag of PSL is set
 ***************************************************************/
#define M_SUB(dest,_source)										\
{																\
	UINT8 source = _source;										\
	UINT8 before = dest;										\
	/* subtract source; borrow only if WC is set */ 			\
	UINT16 res = dest - source - ((S.psl >> 3) & (S.psl ^ C) & C);	\
	S.psl &= ~(C | OVF | IDC);									\
	if((res & 0x100)==0) S.psl |= C; 							\
    dest = res & 0xff;                                          \
	if( (dest & 15) <= (before & 15) ) S.psl |= IDC; 			\
	SET_CC_OVF_SUB(dest,before,source);							\
}

/***************************************************************
 * M_COM
 * Compare register against value. If COM of PSL is set,
 * use unsigned, else signed comparison
 ***************************************************************/
#define M_COM(reg,val)											\
{																\
	int d;														\
	S.psl &= ~CC;												\
	if (S.psl & COM) d = (UINT8)reg - (UINT8)val;				\
				else d = (INT8)reg - (INT8)val; 				\
	if( d < 0 ) S.psl |= 0x80;									\
	else														\
	if( d > 0 ) S.psl |= 0x40;									\
}

/***************************************************************
 * M_DAR
 * Decimal adjust register
 ***************************************************************/
#define M_DAR(dest)												\
{																\
	if ((S.psl & C) == 0) dest += 0xA0;							\
	if ((S.psl & IDC) == 0) dest = (dest & 0xF0) | ((dest + 0x0A) & 0x0F);\
}

/***************************************************************
 * M_RRL
 * Rotate register left; If WC of PSL is set, rotate
 * through carry, else rotate circular
 ***************************************************************/
#define M_RRL(dest) 											\
{																\
	UINT8 before = dest;										\
	if( S.psl & WC )											\
	{															\
		UINT8 c = S.psl & C;									\
		S.psl &= ~(C + IDC);									\
		dest = (before << 1) | c;								\
		S.psl |= (before >> 7) + (dest & IDC);					\
	}															\
	else														\
	{															\
		dest = (before << 1) | (before >> 7);					\
	}															\
	SET_CC(dest);												\
	S.psl = (S.psl & ~OVF) | (((dest ^ before) >> 5) & OVF);	\
}

/***************************************************************
 * M_RRR
 * Rotate register right; If WC of PSL is set, rotate
 * through carry, else rotate circular
 ***************************************************************/
#define M_RRR(dest) 											\
{																\
	UINT8 before = dest;										\
	if (S.psl & WC) 											\
	{															\
		UINT8 c = S.psl & C;									\
		S.psl &= ~(C + IDC);									\
		dest = (before >> 1) | (c << 7);						\
		S.psl |= (before & C) + (dest & IDC);					\
	} else	dest = (before >> 1) | (before << 7);				\
	SET_CC(dest);												\
	S.psl = (S.psl & ~OVF) | (((dest ^ before) >> 5) & OVF);	\
}

// bxd() not necessary

/***************************************************************
 * M_SPSU
 * Store processor status upper (PSU) to register R0
 * Checks for External Sense IO port
 ***************************************************************/
#define M_SPSU()												\
{																\
	R0 = ((S.psu & ~PSU34) | (io_read_byte_8(S2650_SENSE_PORT) & SI)); \
	SET_CC(R0); 												\
}

/***************************************************************
 * M_SPSL
 * Store processor status lower (PSL) to register R0
 ***************************************************************/
#define M_SPSL()												\
{																\
	R0 = S.psl; 												\
	SET_CC(R0); 												\
}

/***************************************************************
 * M_CPSU
 * Clear processor status upper (PSU), selective
 ***************************************************************/
#define M_CPSU()												\
{																\
	UINT8 cpsu = ARG(); 										\
	S.psu = S.psu & ~cpsu;										\
	CHECK_IRQ_LINE; 											\
}

/***************************************************************
 * M_CPSL
 * Clear processor status lower (PSL), selective
 ***************************************************************/
#define M_CPSL()												\
{																\
	UINT8 cpsl = ARG(); 										\
	/* select other register set now ? */						\
	if( (cpsl & RS) && (S.psl & RS) )							\
		SWAP_REGS;												\
	S.psl = S.psl & ~cpsl;										\
	CHECK_IRQ_LINE; 											\
}

/***************************************************************
 * M_PPSU
 * Preset processor status upper (PSU), selective
 * Unused bits 3 and 4 can't be set
 ***************************************************************/
#define M_PPSU()												\
{																\
	UINT8 ppsu = (ARG() & ~PSU34) & ~SI;						\
	S.psu = S.psu | ppsu;										\
}

/***************************************************************
 * M_PPSL
 * Preset processor status lower (PSL), selective
 ***************************************************************/
#define M_PPSL()												\
{																\
	UINT8 ppsl = ARG(); 										\
	/* select 2nd register set now ? */ 						\
	if ((ppsl & RS) && !(S.psl & RS))							\
		SWAP_REGS;												\
	S.psl = S.psl | ppsl;										\
}

/***************************************************************
 * M_TPSU
 * Test processor status upper (PSU)
 ***************************************************************/
#define M_TPSU()												\
{																\
	UINT8 tpsu = ARG(); 										\
    UINT8 rpsu = (S.psu | (io_read_byte_8(S2650_SENSE_PORT) & SI)); \
	S.psl &= ~CC;												\
	if( (rpsu & tpsu) != tpsu )									\
		S.psl |= 0x80;											\
}

/***************************************************************
 * M_TPSL
 * Test processor status lower (PSL)
 ***************************************************************/
#define M_TPSL()												\
{																\
	UINT8 tpsl = ARG(); 										\
	if( (S.psl & tpsl) != tpsl )								\
		S.psl = (S.psl & ~CC) | 0x80;							\
	else														\
		S.psl &= ~CC;											\
}

/***************************************************************
 * M_TMI
 * Test under mask immediate
 ***************************************************************/
#define M_TMI(value)											\
{																\
	UINT8 tmi = ARG();											\
	S.psl &= ~CC;												\
	if( (value & tmi) != tmi )									\
		S.psl |= 0x80;											\
}

#if INLINE_EA
#define REL_EA(page) _REL_EA(page)
#define REL_ZERO(page) _REL_ZERO(page)
#define ABS_EA() _ABS_EA()
#define BRA_EA() _BRA_EA()
#else
static void REL_EA(unsigned short page) _REL_EA(page)
static void REL_ZERO(unsigned short page) _REL_ZERO(page)
static void ABS_EA(void) _ABS_EA()
static void BRA_EA(void) _BRA_EA()
#endif

static void s2650_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	S.irq_callback = irqcallback;

	state_save_register_item("s2650", index, S.ppc);
	state_save_register_item("s2650", index, S.page);
	state_save_register_item("s2650", index, S.iar);
	state_save_register_item("s2650", index, S.ea);
	state_save_register_item("s2650", index, S.psl);
	state_save_register_item("s2650", index, S.psu);
	state_save_register_item("s2650", index, S.r);
	state_save_register_item_array("s2650", index, S.reg);
	state_save_register_item("s2650", index, S.halt);
	state_save_register_item("s2650", index, S.ir);
	state_save_register_item_array("s2650", index, S.ras);
	state_save_register_item("s2650", index, S.irq_state);
}

static void s2650_reset(void)
{
	int (*save_irqcallback)(int) = S.irq_callback;
	memset(&S, 0, sizeof(S));
	S.irq_callback = save_irqcallback;
	S.psl = COM | WC;
	S.psu = 0;
}

static void s2650_exit(void)
{
	/* nothing to do */
}

static void s2650_get_context(void *dst)
{
	if( dst )
		*(s2650_Regs*)dst = S;
}

static void s2650_set_context(void *src)
{
	if( src )
	{
		S = *(s2650_Regs*)src;
		S.page = S.page & PAGE;
		S.iar = S.iar & PMSK;
        change_pc(S.page + S.iar);
	}
}

static void set_irq_line(int irqline, int state)
{
	if (irqline == 1)
	{
		if (state == CLEAR_LINE)
			s2650_set_sense(0);
		else
			s2650_set_sense(1);
		return;
	}

	S.irq_state = state;
	CHECK_IRQ_LINE;
}

static void s2650_set_flag(int state)
{
    if (state)
        S.psu |= FO;
    else
        S.psu &= ~FO;
}

static int s2650_get_flag(void)
{
    return (S.psu & FO) ? 1 : 0;
}

static void s2650_set_sense(int state)
{
    if (state)
        S.psu |= SI;
    else
        S.psu &= ~SI;
}

static int s2650_get_sense(void)
{
	/* OR'd with Input to allow for external connections */

    return (((S.psu & SI) ? 1 : 0) | ((io_read_byte_8(S2650_SENSE_PORT) & SI) ? 1 : 0));
}

static int s2650_execute(int cycles)
{
	s2650_ICount = cycles;
	do
	{
		S.ppc = S.page + S.iar;

		CALL_MAME_DEBUG;

		S.ir = ROP();
		S.r = S.ir & 3; 		/* register / value */
		switch (S.ir) {
			case 0x00:		/* LODZ,0 */
			case 0x01:		/* LODZ,1 */
			case 0x02:		/* LODZ,2 */
			case 0x03:		/* LODZ,3 */
				s2650_ICount -= 6;
				M_LOD( R0, S.reg[S.r] );
				break;

			case 0x04:		/* LODI,0 v */
			case 0x05:		/* LODI,1 v */
			case 0x06:		/* LODI,2 v */
			case 0x07:		/* LODI,3 v */
				s2650_ICount -= 6;
				M_LOD( S.reg[S.r], ARG() );
				break;

			case 0x08:		/* LODR,0 (*)a */
			case 0x09:		/* LODR,1 (*)a */
			case 0x0a:		/* LODR,2 (*)a */
			case 0x0b:		/* LODR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA( S.page );
				M_LOD( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x0c:		/* LODA,0 (*)a(,X) */
			case 0x0d:		/* LODA,1 (*)a(,X) */
			case 0x0e:		/* LODA,2 (*)a(,X) */
			case 0x0f:		/* LODA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_LOD( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x10:		/* illegal */
			case 0x11:		/* illegal */
				s2650_ICount -= 7;
				break;
			case 0x12:		/* SPSU */
				s2650_ICount -= 6;
				M_SPSU();
				break;
			case 0x13:		/* SPSL */
				s2650_ICount -= 6;
				M_SPSL();
				break;

			case 0x14:		/* RETC,0   (zero)  */
			case 0x15:		/* RETC,1   (plus)  */
			case 0x16:		/* RETC,2   (minus) */
				s2650_ICount -= 9;	/* +2 cycles if condition is true */
				M_RET( (S.psl >> 6) == S.r );
				break;
			case 0x17:		/* RETC,3   (always) */
				s2650_ICount -= 9;	/* +2 cycles if condition is true */
				M_RET( 1 );
				break;

			case 0x18:		/* BCTR,0  (*)a */
			case 0x19:		/* BCTR,1  (*)a */
			case 0x1a:		/* BCTR,2  (*)a */
				s2650_ICount -= 9;
				M_BRR( (S.psl >> 6) == S.r );
				break;
			case 0x1b:		/* BCTR,3  (*)a */
				s2650_ICount -= 9;
				M_BRR( 1 );
				break;

			case 0x1c:		/* BCTA,0  (*)a */
			case 0x1d:		/* BCTA,1  (*)a */
			case 0x1e:		/* BCTA,2  (*)a */
				s2650_ICount -= 9;
				M_BRA( (S.psl >> 6) == S.r );
				break;
			case 0x1f:		/* BCTA,3  (*)a */
				s2650_ICount -= 9;
				M_BRA( 1 );
				break;

			case 0x20:		/* EORZ,0 */
			case 0x21:		/* EORZ,1 */
			case 0x22:		/* EORZ,2 */
			case 0x23:		/* EORZ,3 */
				s2650_ICount -= 6;
				M_EOR( R0, S.reg[S.r] );
				break;

			case 0x24:		/* EORI,0 v */
			case 0x25:		/* EORI,1 v */
			case 0x26:		/* EORI,2 v */
			case 0x27:		/* EORI,3 v */
				s2650_ICount -= 6;
				M_EOR( S.reg[S.r], ARG() );
				break;

			case 0x28:		/* EORR,0 (*)a */
			case 0x29:		/* EORR,1 (*)a */
			case 0x2a:		/* EORR,2 (*)a */
			case 0x2b:		/* EORR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA( S.page );
				M_EOR( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x2c:		/* EORA,0 (*)a(,X) */
			case 0x2d:		/* EORA,1 (*)a(,X) */
			case 0x2e:		/* EORA,2 (*)a(,X) */
			case 0x2f:		/* EORA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_EOR( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x30:		/* REDC,0 */
			case 0x31:		/* REDC,1 */
			case 0x32:		/* REDC,2 */
			case 0x33:		/* REDC,3 */
				s2650_ICount -= 6;
				S.reg[S.r] = io_read_byte_8(S2650_CTRL_PORT);
				SET_CC( S.reg[S.r] );
				break;

			case 0x34:		/* RETE,0 */
			case 0x35:		/* RETE,1 */
			case 0x36:		/* RETE,2 */
				s2650_ICount -= 9;
				M_RETE( (S.psl >> 6) == S.r );
				break;
			case 0x37:		/* RETE,3 */
				s2650_ICount -= 9;
				M_RETE( 1 );
				break;

			case 0x38:		/* BSTR,0 (*)a */
			case 0x39:		/* BSTR,1 (*)a */
			case 0x3a:		/* BSTR,2 (*)a */
				s2650_ICount -= 9;
				M_BSR( (S.psl >> 6) == S.r );
				break;
			case 0x3b:		/* BSTR,R3 (*)a */
				s2650_ICount -= 9;
				M_BSR( 1 );
				break;

			case 0x3c:		/* BSTA,0 (*)a */
			case 0x3d:		/* BSTA,1 (*)a */
			case 0x3e:		/* BSTA,2 (*)a */
				s2650_ICount -= 9;
				M_BSA( (S.psl >> 6) == S.r );
				break;
			case 0x3f:		/* BSTA,3 (*)a */
				s2650_ICount -= 9;
				M_BSA( 1 );
				break;

			case 0x40:		/* HALT */
				s2650_ICount -= 6;
				S.iar = (S.iar - 1) & PMSK;
				S.halt = 1;
				if (s2650_ICount > 0)
					s2650_ICount = 0;
				break;
			case 0x41:		/* ANDZ,1 */
			case 0x42:		/* ANDZ,2 */
			case 0x43:		/* ANDZ,3 */
				s2650_ICount -= 6;
				M_AND( R0, S.reg[S.r] );
				break;

			case 0x44:		/* ANDI,0 v */
			case 0x45:		/* ANDI,1 v */
			case 0x46:		/* ANDI,2 v */
			case 0x47:		/* ANDI,3 v */
				s2650_ICount -= 6;
				M_AND( S.reg[S.r], ARG() );
				break;

			case 0x48:		/* ANDR,0 (*)a */
			case 0x49:		/* ANDR,1 (*)a */
			case 0x4a:		/* ANDR,2 (*)a */
			case 0x4b:		/* ANDR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA( S.page );
				M_AND( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x4c:		/* ANDA,0 (*)a(,X) */
			case 0x4d:		/* ANDA,1 (*)a(,X) */
			case 0x4e:		/* ANDA,2 (*)a(,X) */
			case 0x4f:		/* ANDA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_AND( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x50:		/* RRR,0 */
			case 0x51:		/* RRR,1 */
			case 0x52:		/* RRR,2 */
			case 0x53:		/* RRR,3 */
				s2650_ICount -= 6;
				M_RRR( S.reg[S.r] );
				break;

			case 0x54:		/* REDE,0 v */
			case 0x55:		/* REDE,1 v */
			case 0x56:		/* REDE,2 v */
			case 0x57:		/* REDE,3 v */
				s2650_ICount -= 9;
				S.reg[S.r] = io_read_byte_8( ARG() );
				SET_CC(S.reg[S.r]);
				break;

			case 0x58:		/* BRNR,0 (*)a */
			case 0x59:		/* BRNR,1 (*)a */
			case 0x5a:		/* BRNR,2 (*)a */
			case 0x5b:		/* BRNR,3 (*)a */
				s2650_ICount -= 9;
				M_BRR( S.reg[S.r] );
				break;

			case 0x5c:		/* BRNA,0 (*)a */
			case 0x5d:		/* BRNA,1 (*)a */
			case 0x5e:		/* BRNA,2 (*)a */
			case 0x5f:		/* BRNA,3 (*)a */
				s2650_ICount -= 9;
				M_BRA( S.reg[S.r] );
				break;

			case 0x60:		/* IORZ,0 */
			case 0x61:		/* IORZ,1 */
			case 0x62:		/* IORZ,2 */
			case 0x63:		/* IORZ,3 */
				s2650_ICount -= 6;
				M_IOR( R0, S.reg[S.r] );
				break;

			case 0x64:		/* IORI,0 v */
			case 0x65:		/* IORI,1 v */
			case 0x66:		/* IORI,2 v */
			case 0x67:		/* IORI,3 v */
				s2650_ICount -= 6;
				M_IOR( S.reg[S.r], ARG() );
				break;

			case 0x68:		/* IORR,0 (*)a */
			case 0x69:		/* IORR,1 (*)a */
			case 0x6a:		/* IORR,2 (*)a */
			case 0x6b:		/* IORR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA( S.page );
				M_IOR( S.reg[S. r],RDMEM(S.ea) );
				break;

			case 0x6c:		/* IORA,0 (*)a(,X) */
			case 0x6d:		/* IORA,1 (*)a(,X) */
			case 0x6e:		/* IORA,2 (*)a(,X) */
			case 0x6f:		/* IORA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_IOR( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x70:		/* REDD,0 */
			case 0x71:		/* REDD,1 */
			case 0x72:		/* REDD,2 */
			case 0x73:		/* REDD,3 */
				s2650_ICount -= 6;
				S.reg[S.r] = io_read_byte_8(S2650_DATA_PORT);
				SET_CC(S.reg[S.r]);
				break;

			case 0x74:		/* CPSU */
				s2650_ICount -= 9;
				M_CPSU();
				break;
			case 0x75:		/* CPSL */
				s2650_ICount -= 9;
				M_CPSL();
				break;
			case 0x76:		/* PPSU */
				s2650_ICount -= 9;
				M_PPSU();
				break;
			case 0x77:		/* PPSL */
				s2650_ICount -= 9;
				M_PPSL();
				break;

			case 0x78:		/* BSNR,0 (*)a */
			case 0x79:		/* BSNR,1 (*)a */
			case 0x7a:		/* BSNR,2 (*)a */
			case 0x7b:		/* BSNR,3 (*)a */
				s2650_ICount -= 9;
				M_BSR( S.reg[S.r] );
				break;

			case 0x7c:		/* BSNA,0 (*)a */
			case 0x7d:		/* BSNA,1 (*)a */
			case 0x7e:		/* BSNA,2 (*)a */
			case 0x7f:		/* BSNA,3 (*)a */
				s2650_ICount -= 9;
				M_BSA( S.reg[S.r] );
				break;

			case 0x80:		/* ADDZ,0 */
			case 0x81:		/* ADDZ,1 */
			case 0x82:		/* ADDZ,2 */
			case 0x83:		/* ADDZ,3 */
				s2650_ICount -= 6;
				M_ADD( R0,S.reg[S.r] );
				break;

			case 0x84:		/* ADDI,0 v */
			case 0x85:		/* ADDI,1 v */
			case 0x86:		/* ADDI,2 v */
			case 0x87:		/* ADDI,3 v */
				s2650_ICount -= 6;
				M_ADD( S.reg[S.r], ARG() );
				break;

			case 0x88:		/* ADDR,0 (*)a */
			case 0x89:		/* ADDR,1 (*)a */
			case 0x8a:		/* ADDR,2 (*)a */
			case 0x8b:		/* ADDR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA(S.page);
				M_ADD( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x8c:		/* ADDA,0 (*)a(,X) */
			case 0x8d:		/* ADDA,1 (*)a(,X) */
			case 0x8e:		/* ADDA,2 (*)a(,X) */
			case 0x8f:		/* ADDA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_ADD( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0x90:		/* illegal */
			case 0x91:		/* illegal */
				s2650_ICount -= 7;
				break;
			case 0x92:		/* LPSU */
				s2650_ICount -= 6;
				S.psu = (R0 & ~PSU34) & ~SI;
				break;
			case 0x93:		/* LPSL */
				s2650_ICount -= 6;
				/* change register set ? */
				if ((S.psl ^ R0) & RS)
					SWAP_REGS;
				S.psl = R0;
				break;

			case 0x94:		/* DAR,0 */
			case 0x95:		/* DAR,1 */
			case 0x96:		/* DAR,2 */
			case 0x97:		/* DAR,3 */
				s2650_ICount -= 9;
				M_DAR( S.reg[S.r] );
				break;

			case 0x98:		/* BCFR,0 (*)a */
			case 0x99:		/* BCFR,1 (*)a */
			case 0x9a:		/* BCFR,2 (*)a */
				s2650_ICount -= 9;
				M_BRR( (S.psl >> 6) != S.r );
				break;
			case 0x9b:		/* ZBRR    (*)a */
				s2650_ICount -= 9;
				M_ZBRR();
				break;

			case 0x9c:		/* BCFA,0 (*)a */
			case 0x9d:		/* BCFA,1 (*)a */
			case 0x9e:		/* BCFA,2 (*)a */
				s2650_ICount -= 9;
				M_BRA( (S.psl >> 6) != S.r );
				break;
			case 0x9f:		/* BXA     (*)a */
				s2650_ICount -= 9;
				M_BXA();
				break;

			case 0xa0:		/* SUBZ,0 */
			case 0xa1:		/* SUBZ,1 */
			case 0xa2:		/* SUBZ,2 */
			case 0xa3:		/* SUBZ,3 */
				s2650_ICount -= 6;
				M_SUB( R0, S.reg[S.r] );
				break;

			case 0xa4:		/* SUBI,0 v */
			case 0xa5:		/* SUBI,1 v */
			case 0xa6:		/* SUBI,2 v */
			case 0xa7:		/* SUBI,3 v */
				s2650_ICount -= 6;
				M_SUB( S.reg[S.r], ARG() );
				break;

			case 0xa8:		/* SUBR,0 (*)a */
			case 0xa9:		/* SUBR,1 (*)a */
			case 0xaa:		/* SUBR,2 (*)a */
			case 0xab:		/* SUBR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA(S.page);
				M_SUB( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0xac:		/* SUBA,0 (*)a(,X) */
			case 0xad:		/* SUBA,1 (*)a(,X) */
			case 0xae:		/* SUBA,2 (*)a(,X) */
			case 0xaf:		/* SUBA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_SUB( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0xb0:		/* WRTC,0 */
			case 0xb1:		/* WRTC,1 */
			case 0xb2:		/* WRTC,2 */
			case 0xb3:		/* WRTC,3 */
				s2650_ICount -= 6;
				io_write_byte_8(S2650_CTRL_PORT,S.reg[S.r]);
				break;

			case 0xb4:		/* TPSU */
				s2650_ICount -= 9;
				M_TPSU();
				break;
			case 0xb5:		/* TPSL */
				s2650_ICount -= 9;
				M_TPSL();
				break;
			case 0xb6:		/* illegal */
			case 0xb7:		/* illegal */
				s2650_ICount -= 7;
				break;

			case 0xb8:		/* BSFR,0 (*)a */
			case 0xb9:		/* BSFR,1 (*)a */
			case 0xba:		/* BSFR,2 (*)a */
				s2650_ICount -= 9;
				M_BSR( (S.psl >> 6) != S.r );
				break;
			case 0xbb:		/* ZBSR    (*)a */
				s2650_ICount -= 9;
				M_ZBSR();
				break;

			case 0xbc:		/* BSFA,0 (*)a */
			case 0xbd:		/* BSFA,1 (*)a */
			case 0xbe:		/* BSFA,2 (*)a */
				s2650_ICount -= 9;
				M_BSA( (S.psl >> 6) != S.r );
				break;
			case 0xbf:		/* BSXA    (*)a */
				s2650_ICount -= 9;
				M_BSXA();
				break;

			case 0xc0:		/* NOP */
				s2650_ICount -= 6;
				break;
			case 0xc1:		/* STRZ,1 */
			case 0xc2:		/* STRZ,2 */
			case 0xc3:		/* STRZ,3 */
				s2650_ICount -= 6;
				M_LOD( S.reg[S.r], R0 );
				break;

			case 0xc4:		/* illegal */
			case 0xc5:		/* illegal */
			case 0xc6:		/* illegal */
			case 0xc7:		/* illegal */
				s2650_ICount -= 7;
				break;

			case 0xc8:		/* STRR,0 (*)a */
			case 0xc9:		/* STRR,1 (*)a */
			case 0xca:		/* STRR,2 (*)a */
			case 0xcb:		/* STRR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA(S.page);
				M_STR( S.ea, S.reg[S.r] );
				break;

			case 0xcc:		/* STRA,0 (*)a(,X) */
			case 0xcd:		/* STRA,1 (*)a(,X) */
			case 0xce:		/* STRA,2 (*)a(,X) */
			case 0xcf:		/* STRA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_STR( S.ea, S.reg[S.r] );
				break;

			case 0xd0:		/* RRL,0 */
			case 0xd1:		/* RRL,1 */
			case 0xd2:		/* RRL,2 */
			case 0xd3:		/* RRL,3 */
				s2650_ICount -= 6;
				M_RRL( S.reg[S.r] );
				break;

			case 0xd4:		/* WRTE,0 v */
			case 0xd5:		/* WRTE,1 v */
			case 0xd6:		/* WRTE,2 v */
			case 0xd7:		/* WRTE,3 v */
				s2650_ICount -= 9;
				io_write_byte_8( ARG(), S.reg[S.r] );
				break;

			case 0xd8:		/* BIRR,0 (*)a */
			case 0xd9:		/* BIRR,1 (*)a */
			case 0xda:		/* BIRR,2 (*)a */
			case 0xdb:		/* BIRR,3 (*)a */
				s2650_ICount -= 9;
				M_BRR( ++S.reg[S.r] );
				break;

			case 0xdc:		/* BIRA,0 (*)a */
			case 0xdd:		/* BIRA,1 (*)a */
			case 0xde:		/* BIRA,2 (*)a */
			case 0xdf:		/* BIRA,3 (*)a */
				s2650_ICount -= 9;
				M_BRA( ++S.reg[S.r] );
				break;

			case 0xe0:		/* COMZ,0 */
			case 0xe1:		/* COMZ,1 */
			case 0xe2:		/* COMZ,2 */
			case 0xe3:		/* COMZ,3 */
				s2650_ICount -= 6;
				M_COM( R0, S.reg[S.r] );
				break;

			case 0xe4:		/* COMI,0 v */
			case 0xe5:		/* COMI,1 v */
			case 0xe6:		/* COMI,2 v */
			case 0xe7:		/* COMI,3 v */
				s2650_ICount -= 6;
				M_COM( S.reg[S.r], ARG() );
				break;

			case 0xe8:		/* COMR,0 (*)a */
			case 0xe9:		/* COMR,1 (*)a */
			case 0xea:		/* COMR,2 (*)a */
			case 0xeb:		/* COMR,3 (*)a */
				s2650_ICount -= 9;
				REL_EA(S.page);
				M_COM( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0xec:		/* COMA,0 (*)a(,X) */
			case 0xed:		/* COMA,1 (*)a(,X) */
			case 0xee:		/* COMA,2 (*)a(,X) */
			case 0xef:		/* COMA,3 (*)a(,X) */
				s2650_ICount -= 12;
				ABS_EA();
				M_COM( S.reg[S.r], RDMEM(S.ea) );
				break;

			case 0xf0:		/* WRTD,0 */
			case 0xf1:		/* WRTD,1 */
			case 0xf2:		/* WRTD,2 */
			case 0xf3:		/* WRTD,3 */
				s2650_ICount -= 6;
				io_write_byte_8(S2650_DATA_PORT, S.reg[S.r]);
				break;

			case 0xf4:		/* TMI,0  v */
			case 0xf5:		/* TMI,1  v */
			case 0xf6:		/* TMI,2  v */
			case 0xf7:		/* TMI,3  v */
				s2650_ICount -= 9;
				M_TMI( S.reg[S.r] );
				break;

			case 0xf8:		/* BDRR,0 (*)a */
			case 0xf9:		/* BDRR,1 (*)a */
			case 0xfa:		/* BDRR,2 (*)a */
			case 0xfb:		/* BDRR,3 (*)a */
				s2650_ICount -= 9;
				M_BRR( --S.reg[S.r] );
				break;

			case 0xfc:		/* BDRA,0 (*)a */
			case 0xfd:		/* BDRA,1 (*)a */
			case 0xfe:		/* BDRA,2 (*)a */
			case 0xff:		/* BDRA,3 (*)a */
				s2650_ICount -= 9;
				M_BRA( --S.reg[S.r] );
				break;
		}
	} while( s2650_ICount > 0 );

	return cycles - s2650_ICount;
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void s2650_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:				set_irq_line(1, info->i);				break;

		case CPUINFO_INT_PC:
			S.page = info->i & PAGE;
			S.iar = info->i & PMSK;
			change_pc(S.page + S.iar);
			break;

		case CPUINFO_INT_REGISTER + S2650_PC:			S.page = info->i & PAGE; S.iar = info->i & PMSK; break;
		case CPUINFO_INT_SP: 							S.psu = (S.psu & ~SP) | (info->i & SP); break;
		case CPUINFO_INT_REGISTER + S2650_PS:			S.psl = info->i & 0xff; S.psu = info->i >> 8; break;
		case CPUINFO_INT_REGISTER + S2650_R0:			S.reg[0] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R1:			S.reg[1] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R2:			S.reg[2] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R3:			S.reg[3] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R1A:			S.reg[4] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R2A:			S.reg[5] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R3A:			S.reg[6] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_HALT:			S.halt = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_SI:			s2650_set_sense(info->i);				break;
		case CPUINFO_INT_REGISTER + S2650_FO:			s2650_set_flag(info->i);				break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void s2650_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(S);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 5;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 13;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 15;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0;					break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 9;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = S.irq_state;					break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = s2650_get_sense() ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = S.ppc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + S2650_PC:			info->i = S.page + S.iar;				break;

		case CPUINFO_INT_SP:							info->i = S.psu & SP;					break;
		case CPUINFO_INT_REGISTER + S2650_PS:			info->i = (S.psu << 8) | S.psl;			break;
		case CPUINFO_INT_REGISTER + S2650_R0:			info->i = S.reg[0];						break;
		case CPUINFO_INT_REGISTER + S2650_R1:			info->i = S.reg[1];						break;
		case CPUINFO_INT_REGISTER + S2650_R2:			info->i = S.reg[2];						break;
		case CPUINFO_INT_REGISTER + S2650_R3:			info->i = S.reg[3];						break;
		case CPUINFO_INT_REGISTER + S2650_R1A:			info->i = S.reg[4];						break;
		case CPUINFO_INT_REGISTER + S2650_R2A:			info->i = S.reg[5];						break;
		case CPUINFO_INT_REGISTER + S2650_R3A:			info->i = S.reg[6];						break;
		case CPUINFO_INT_REGISTER + S2650_HALT:			info->i = S.halt;						break;
		case CPUINFO_INT_REGISTER + S2650_SI:			info->i = s2650_get_sense();			break;
		case CPUINFO_INT_REGISTER + S2650_FO:			info->i = s2650_get_flag();				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = s2650_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = s2650_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = s2650_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = s2650_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = s2650_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = s2650_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = s2650_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = s2650_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &s2650_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "S2650");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Signetics 2650");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.2");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Written by Juergen Buchmueller for use with MAME"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				S.psu & 0x80 ? 'S':'.',
				S.psu & 0x40 ? 'O':'.',
				S.psu & 0x20 ? 'I':'.',
				S.psu & 0x10 ? '?':'.',
				S.psu & 0x08 ? '?':'.',
				S.psu & 0x04 ? 's':'.',
				S.psu & 0x02 ? 's':'.',
				S.psu & 0x01 ? 's':'.',
                S.psl & 0x80 ? 'M':'.',
				S.psl & 0x40 ? 'P':'.',
				S.psl & 0x20 ? 'H':'.',
				S.psl & 0x10 ? 'R':'.',
				S.psl & 0x08 ? 'W':'.',
				S.psl & 0x04 ? 'V':'.',
				S.psl & 0x02 ? '2':'.',
				S.psl & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + S2650_PC:			sprintf(info->s, "PC:%04X", S.page + S.iar); break;
		case CPUINFO_STR_REGISTER + S2650_PS:			sprintf(info->s, "PS:%02X%02X", S.psu, S.psl); break;
		case CPUINFO_STR_REGISTER + S2650_R0:			sprintf(info->s, "R0:%02X", S.reg[0]);	break;
		case CPUINFO_STR_REGISTER + S2650_R1:			sprintf(info->s, "R1:%02X", S.reg[1]);	break;
		case CPUINFO_STR_REGISTER + S2650_R2:			sprintf(info->s, "R2:%02X", S.reg[2]);	break;
		case CPUINFO_STR_REGISTER + S2650_R3:			sprintf(info->s, "R3:%02X", S.reg[3]);	break;
		case CPUINFO_STR_REGISTER + S2650_R1A:			sprintf(info->s, "R1'%02X", S.reg[4]);	break;
		case CPUINFO_STR_REGISTER + S2650_R2A:			sprintf(info->s, "R2'%02X", S.reg[5]);	break;
		case CPUINFO_STR_REGISTER + S2650_R3A:			sprintf(info->s, "R3'%02X", S.reg[6]);	break;
		case CPUINFO_STR_REGISTER + S2650_HALT:			sprintf(info->s, "HALT:%X", S.halt);	break;
		case CPUINFO_STR_REGISTER + S2650_SI:			sprintf(info->s, "SI:%X", (S.psu & SI) ? 1 : 0); break;
		case CPUINFO_STR_REGISTER + S2650_FO:			sprintf(info->s, "FO:%X", (S.psu & FO) ? 1 : 0); break;
	}
}
