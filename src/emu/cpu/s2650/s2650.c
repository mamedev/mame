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

#include "emu.h"
#include "debugger.h"
#include "s2650.h"
#include "s2650cpu.h"

/* define this to have some interrupt information logged */
#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* define this to expand all EA calculations inline */
#define INLINE_EA	1

struct s2650_regs {
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

	int		icount;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
};

INLINE s2650_regs *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == S2650);
	return (s2650_regs *)downcast<legacy_cpu_device *>(device)->token();
}

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

/***************************************************************
 * handy table to build PC relative offsets
 * from HR (holding register)
 ***************************************************************/
static const int S2650_relative[0x100] =
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
 * RDMEM
 * read memory byte from addr
 ***************************************************************/
#define RDMEM(addr) s2650c->program->read_byte(addr)

static void s2650_set_sense(s2650_regs *s2650c, int state);

INLINE void set_psu(s2650_regs *s2650c, UINT8 new_val)
{
	UINT8 old = s2650c->psu;

    s2650c->psu = new_val;
    if ((new_val ^ old) & FO)
    	s2650c->io->write_byte(S2650_FO_PORT, (new_val & FO) ? 1 : 0);
}

INLINE UINT8 get_sp(s2650_regs *s2650c)
{
	return (s2650c->psu & SP);
}

INLINE void set_sp(s2650_regs *s2650c, UINT8 new_sp)
{
	s2650c->psu = (s2650c->psu & ~SP) | (new_sp & SP);
}

INLINE int check_irq_line(s2650_regs *s2650c)
{
	int cycles = 0;

	if (s2650c->irq_state != CLEAR_LINE)
	{
		if( (s2650c->psu & II) == 0 )
		{
			int vector;
			if (s2650c->halt)
			{
				s2650c->halt = 0;
				s2650c->iar = (s2650c->iar + 1) & PMSK;
			}
			vector = (*s2650c->irq_callback)(s2650c->device, 0) & 0xff;
			/* build effective address within first 8K page */
			s2650c->ea = S2650_relative[vector] & PMSK;
			if (vector & 0x80)		/* indirect bit set ? */
			{
				int addr = s2650c->ea;
				cycles += 6;
				/* build indirect 32K address */
				s2650c->ea = RDMEM(addr) << 8;
				if (!(++addr & PMSK)) addr -= PLEN;
				s2650c->ea = (s2650c->ea + RDMEM(addr)) & AMSK;
			}
			LOG(("S2650 interrupt to $%04x\n", s2650c->ea));
			set_sp(s2650c, get_sp(s2650c) + 1);
			set_psu(s2650c, s2650c->psu | II);
			s2650c->ras[get_sp(s2650c)] = s2650c->page + s2650c->iar;
			s2650c->page = s2650c->ea & PAGE;
			s2650c->iar  = s2650c->ea & PMSK;
		}
	}
	return cycles;
}

/***************************************************************
 *
 * set condition code (zero,plus,minus) from result
 ***************************************************************/
#define SET_CC(result)                                          \
	s2650c->psl = (s2650c->psl & ~CC) | ccc[result]

/***************************************************************
 *
 * set condition code (zero,plus,minus) and overflow
 ***************************************************************/
#define SET_CC_OVF(result,value)                                \
	s2650c->psl = (s2650c->psl & ~(OVF+CC)) |								\
		ccc[result + (((value) & 0x80) << 1)]

#define SET_CC_OVF_ADD(result,value1,value2) SET_CC_OVF(result,~((value1) ^ (value2)) & ((value1) ^ (result)))
#define SET_CC_OVF_SUB(result,value1,value2) SET_CC_OVF(result,~((value1) ^ (value2)) & ((value1) ^ (result)))

/***************************************************************
 * ROP
 * read next opcode
 ***************************************************************/
INLINE UINT8 ROP(s2650_regs *s2650c)
{
	UINT8 result = s2650c->direct->read_decrypted_byte(s2650c->page + s2650c->iar);
	s2650c->iar = (s2650c->iar + 1) & PMSK;
	return result;
}

/***************************************************************
 * ARG
 * read next opcode argument
 ***************************************************************/
INLINE UINT8 ARG(s2650_regs *s2650c)
{
	UINT8 result = s2650c->direct->read_raw_byte(s2650c->page + s2650c->iar);
	s2650c->iar = (s2650c->iar + 1) & PMSK;
	return result;
}

/***************************************************************
 * _REL_EA
 * build effective address with relative addressing
 ***************************************************************/
#define _REL_EA(page)											\
{																\
	UINT8 hr = ARG(s2650c);	/* get 'holding register' */            \
	/* build effective address within current 8K page */		\
	s2650c->ea = page + ((s2650c->iar + S2650_relative[hr]) & PMSK);		\
	if (hr & 0x80) { /* indirect bit set ? */					\
		int addr = s2650c->ea;										\
		s2650c->icount -= 6;										\
		/* build indirect 32K address */						\
		s2650c->ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
		s2650c->ea = (s2650c->ea + RDMEM(addr)) & AMSK; 					\
	}															\
}

/***************************************************************
 * _REL_ZERO
 * build effective address with zero relative addressing
 ***************************************************************/
#define _REL_ZERO(page)											\
{																\
	UINT8 hr = ARG(s2650c);	/* get 'holding register' */            \
	/* build effective address from 0 */						\
	s2650c->ea = (S2650_relative[hr] & PMSK);							\
	if (hr & 0x80) { /* indirect bit set ? */					\
		int addr = s2650c->ea;										\
		s2650c->icount -= 6;										\
		/* build indirect 32K address */						\
		s2650c->ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
		s2650c->ea = (s2650c->ea + RDMEM(addr)) & AMSK; 					\
	}															\
}

/***************************************************************
 * _ABS_EA
 * build effective address with absolute addressing
 ***************************************************************/
#define _ABS_EA()												\
{																\
	UINT8 hr, dr;												\
	hr = ARG(s2650c);	/* get 'holding register' */                \
	dr = ARG(s2650c);	/* get 'data bus register' */               \
	/* build effective address within current 8K page */		\
	s2650c->ea = s2650c->page + (((hr << 8) + dr) & PMSK);					\
	/* indirect addressing ? */ 								\
	if (hr & 0x80) {											\
		int addr = s2650c->ea;										\
		s2650c->icount -= 6;										\
		/* build indirect 32K address */						\
		/* build indirect 32K address */						\
		s2650c->ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
        s2650c->ea = (s2650c->ea + RDMEM(addr)) & AMSK;                     \
	}															\
	/* check indexed addressing modes */						\
	switch (hr & 0x60) {										\
		case 0x00: /* not indexed */							\
			break;												\
		case 0x20: /* auto increment indexed */ 				\
			s2650c->reg[s2650c->r] += 1;									\
			s2650c->ea = (s2650c->ea & PAGE)+((s2650c->ea+s2650c->reg[s2650c->r]) & PMSK);	\
			s2650c->r = 0; /* absolute addressing reg is R0 */		\
			break;												\
		case 0x40: /* auto decrement indexed */ 				\
			s2650c->reg[s2650c->r] -= 1;									\
			s2650c->ea = (s2650c->ea & PAGE)+((s2650c->ea+s2650c->reg[s2650c->r]) & PMSK);	\
			s2650c->r = 0; /* absolute addressing reg is R0 */		\
			break;												\
		case 0x60: /* indexed */								\
			s2650c->ea = (s2650c->ea & PAGE)+((s2650c->ea+s2650c->reg[s2650c->r]) & PMSK);	\
			s2650c->r = 0; /* absolute addressing reg is R0 */		\
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
	hr = ARG(s2650c);	/* get 'holding register' */                \
	dr = ARG(s2650c);	/* get 'data bus register' */               \
	/* build address in 32K address space */					\
	s2650c->ea = ((hr << 8) + dr) & AMSK;							\
	/* indirect addressing ? */ 								\
	if (hr & 0x80) {											\
		int addr = s2650c->ea;										\
		s2650c->icount -= 6;										\
		/* build indirect 32K address */						\
		s2650c->ea = RDMEM(addr) << 8;								\
		if( (++addr & PMSK) == 0 ) addr -= PLEN; /* page wrap */\
        s2650c->ea = (s2650c->ea + RDMEM(addr)) & AMSK;                     \
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
	tmp = s2650c->reg[1];											\
	s2650c->reg[1] = s2650c->reg[4];										\
	s2650c->reg[4] = tmp;											\
	tmp = s2650c->reg[2];											\
	s2650c->reg[2] = s2650c->reg[5];										\
	s2650c->reg[5] = tmp;											\
	tmp = s2650c->reg[3];											\
	s2650c->reg[3] = s2650c->reg[6];										\
	s2650c->reg[6] = tmp;											\
}

/***************************************************************
 * M_BRR
 * Branch relative if cond is true
 ***************************************************************/
#define M_BRR(cond) 											\
{																\
	if (cond)													\
	{															\
		REL_EA( s2650c->page );										\
		s2650c->page = s2650c->ea & PAGE;									\
		s2650c->iar  = s2650c->ea & PMSK;									\
	} else s2650c->iar = (s2650c->iar + 1) & PMSK;							\
}

/***************************************************************
 * M_ZBRR
 * Branch relative to page zero
 ***************************************************************/
#define M_ZBRR()												\
{																\
	REL_ZERO( 0 );												\
	s2650c->page = s2650c->ea & PAGE;										\
	s2650c->iar  = s2650c->ea & PMSK;										\
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
		s2650c->page = s2650c->ea & PAGE;									\
		s2650c->iar  = s2650c->ea & PMSK;									\
	} else s2650c->iar = (s2650c->iar + 2) & PMSK;							\
}

/***************************************************************
 * M_BXA
 * Branch indexed absolute (EA + R3)
 ***************************************************************/
#define M_BXA() 												\
{																\
	BRA_EA();													\
	s2650c->ea   = (s2650c->ea + s2650c->reg[3]) & AMSK;							\
	s2650c->page = s2650c->ea & PAGE;										\
	s2650c->iar  = s2650c->ea & PMSK;										\
}

/***************************************************************
 * M_BSR
 * Branch to subroutine relative if cond is true
 ***************************************************************/
#define M_BSR(cond) 											\
{																\
	if( cond )													\
	{															\
		REL_EA(s2650c->page);									\
		set_sp(s2650c, get_sp(s2650c) + 1);						\
		s2650c->ras[get_sp(s2650c)] = s2650c->page + s2650c->iar;						\
		s2650c->page = s2650c->ea & PAGE;						\
		s2650c->iar  = s2650c->ea & PMSK;						\
	} else	s2650c->iar = (s2650c->iar + 1) & PMSK; 			\
}

/***************************************************************
 * M_ZBSR
 * Branch to subroutine relative to page zero
 ***************************************************************/
#define M_ZBSR()												\
{																\
	REL_ZERO(0);											    \
	set_sp(s2650c, get_sp(s2650c) + 1);							\
	s2650c->ras[get_sp(s2650c)] = s2650c->page + s2650c->iar;							\
	s2650c->page = s2650c->ea & PAGE;							\
	s2650c->iar  = s2650c->ea & PMSK;							\
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
		set_sp(s2650c, get_sp(s2650c) + 1); 					\
		s2650c->ras[get_sp(s2650c)] = s2650c->page + s2650c->iar;						\
		s2650c->page = s2650c->ea & PAGE;									\
		s2650c->iar  = s2650c->ea & PMSK;									\
	} else s2650c->iar = (s2650c->iar + 2) & PMSK;							\
}

/***************************************************************
 * M_BSXA
 * Branch to subroutine indexed absolute (EA + R3)
 ***************************************************************/
#define M_BSXA()												\
{																\
	BRA_EA();													\
	s2650c->ea  = (s2650c->ea + s2650c->reg[3]) & AMSK;							\
	set_sp(s2650c, get_sp(s2650c) + 1); 				\
	s2650c->ras[get_sp(s2650c)] = s2650c->page + s2650c->iar;							\
	s2650c->page = s2650c->ea & PAGE;										\
	s2650c->iar  = s2650c->ea & PMSK;										\
}

/***************************************************************
 * M_RET
 * Return from subroutine if cond is true
 ***************************************************************/
#define M_RET(cond) 											\
{																\
	if( cond )													\
	{															\
		s2650c->icount -= 6;									\
		s2650c->ea = s2650c->ras[get_sp(s2650c)];				\
		set_sp(s2650c, get_sp(s2650c) - 1); 					\
		s2650c->page = s2650c->ea & PAGE;						\
		s2650c->iar  = s2650c->ea & PMSK;						\
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
		s2650c->ea = s2650c->ras[get_sp(s2650c)];				\
		set_sp(s2650c, get_sp(s2650c) - 1); 					\
		s2650c->page = s2650c->ea & PAGE;						\
		s2650c->iar  = s2650c->ea & PMSK;						\
		set_psu(s2650c, s2650c->psu & ~II);						\
		s2650c->icount -= check_irq_line(s2650c);				\
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
	s2650c->program->write_byte(address, source)

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
	UINT16 res = dest + source + ((s2650c->psl >> 3) & s2650c->psl & C);	\
	s2650c->psl &= ~(C | OVF | IDC);									\
	if(res & 0x100) s2650c->psl |= C;							    \
    dest = res & 0xff;                                          \
	if( (dest & 15) < (before & 15) ) s2650c->psl |= IDC;			\
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
	UINT16 res = dest - source - ((s2650c->psl >> 3) & (s2650c->psl ^ C) & C);	\
	s2650c->psl &= ~(C | OVF | IDC);									\
	if((res & 0x100)==0) s2650c->psl |= C;							\
    dest = res & 0xff;                                          \
	if( (dest & 15) <= (before & 15) ) s2650c->psl |= IDC;			\
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
	s2650c->psl &= ~CC;												\
	if (s2650c->psl & COM) d = (UINT8)reg - (UINT8)val;				\
				else d = (INT8)reg - (INT8)val; 				\
	if( d < 0 ) s2650c->psl |= 0x80;									\
	else														\
	if( d > 0 ) s2650c->psl |= 0x40;									\
}

/***************************************************************
 * M_DAR
 * Decimal adjust register
 ***************************************************************/
#define M_DAR(dest)												\
{																\
	if ((s2650c->psl & C) == 0) dest += 0xA0;							\
	if ((s2650c->psl & IDC) == 0) dest = (dest & 0xF0) | ((dest + 0x0A) & 0x0F);\
}

/***************************************************************
 * M_RRL
 * Rotate register left; If WC of PSL is set, rotate
 * through carry, else rotate circular
 ***************************************************************/
#define M_RRL(dest) 											\
{																\
	UINT8 before = dest;										\
	if( s2650c->psl & WC )											\
	{															\
		UINT8 c = s2650c->psl & C;									\
		s2650c->psl &= ~(C + IDC);									\
		dest = (before << 1) | c;								\
		s2650c->psl |= (before >> 7) + (dest & IDC);					\
	}															\
	else														\
	{															\
		dest = (before << 1) | (before >> 7);					\
	}															\
	SET_CC(dest);												\
	s2650c->psl = (s2650c->psl & ~OVF) | (((dest ^ before) >> 5) & OVF);	\
}

/***************************************************************
 * M_RRR
 * Rotate register right; If WC of PSL is set, rotate
 * through carry, else rotate circular
 ***************************************************************/
#define M_RRR(dest) 											\
{																\
	UINT8 before = dest;										\
	if (s2650c->psl & WC)											\
	{															\
		UINT8 c = s2650c->psl & C;									\
		s2650c->psl &= ~(C + IDC);									\
		dest = (before >> 1) | (c << 7);						\
		s2650c->psl |= (before & C) + (dest & IDC);					\
	} else	dest = (before >> 1) | (before << 7);				\
	SET_CC(dest);												\
	s2650c->psl = (s2650c->psl & ~OVF) | (((dest ^ before) >> 5) & OVF);	\
}

// bxd() not necessary

/***************************************************************
 * M_SPSU
 * Store processor status upper (PSU) to register R0
 * Checks for External Sense IO port
 ***************************************************************/
#define M_SPSU()												\
{																\
	R0 = ((s2650c->psu & ~PSU34) | (s2650c->io->read_byte(S2650_SENSE_PORT) ? SI : 0)); \
	SET_CC(R0); 												\
}

/***************************************************************
 * M_SPSL
 * Store processor status lower (PSL) to register R0
 ***************************************************************/
#define M_SPSL()												\
{																\
	R0 = s2650c->psl;												\
	SET_CC(R0); 												\
}

/***************************************************************
 * M_CPSU
 * Clear processor status upper (PSU), selective
 ***************************************************************/
#define M_CPSU()												\
{																\
	UINT8 cpsu = ARG(s2650c);									\
	set_psu(s2650c, s2650c->psu & ~cpsu);						\
	s2650c->icount -= check_irq_line(s2650c);					\
}

/***************************************************************
 * M_CPSL
 * Clear processor status lower (PSL), selective
 ***************************************************************/
#define M_CPSL()												\
{																\
	UINT8 cpsl = ARG(s2650c);									\
	/* select other register set now ? */						\
	if( (cpsl & RS) && (s2650c->psl & RS) )						\
		SWAP_REGS;												\
	s2650c->psl = s2650c->psl & ~cpsl;							\
}

/***************************************************************
 * M_PPSU
 * Preset processor status upper (PSU), selective
 * Unused bits 3 and 4 can't be set
 ***************************************************************/
#define M_PPSU()												\
{																\
	UINT8 ppsu = (ARG(s2650c) & ~PSU34) & ~SI;					\
	set_psu(s2650c, s2650c->psu | ppsu);						\
}

/***************************************************************
 * M_PPSL
 * Preset processor status lower (PSL), selective
 ***************************************************************/
#define M_PPSL()												\
{																\
	UINT8 ppsl = ARG(s2650c);										\
	/* select 2nd register set now ? */ 						\
	if ((ppsl & RS) && !(s2650c->psl & RS))							\
		SWAP_REGS;												\
	s2650c->psl = s2650c->psl | ppsl;										\
}

/***************************************************************
 * M_TPSU
 * Test processor status upper (PSU)
 ***************************************************************/
#define M_TPSU()												\
{																\
	UINT8 tpsu = ARG(s2650c);										\
    UINT8 rpsu = (s2650c->psu | (s2650c->io->read_byte(S2650_SENSE_PORT) ? SI : 0)); \
	s2650c->psl &= ~CC;												\
	if( (rpsu & tpsu) != tpsu )									\
		s2650c->psl |= 0x80;											\
}

/***************************************************************
 * M_TPSL
 * Test processor status lower (PSL)
 ***************************************************************/
#define M_TPSL()												\
{																\
	UINT8 tpsl = ARG(s2650c);										\
	if( (s2650c->psl & tpsl) != tpsl )								\
		s2650c->psl = (s2650c->psl & ~CC) | 0x80;							\
	else														\
		s2650c->psl &= ~CC;											\
}

/***************************************************************
 * M_TMI
 * Test under mask immediate
 ***************************************************************/
#define M_TMI(value)											\
{																\
	UINT8 tmi = ARG(s2650c);											\
	s2650c->psl &= ~CC;												\
	if( (value & tmi) != tmi )									\
		s2650c->psl |= 0x80;											\
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

static CPU_INIT( s2650 )
{
	s2650_regs *s2650c = get_safe_token(device);

	s2650c->irq_callback = irqcallback;
	s2650c->device = device;
	s2650c->program = device->space(AS_PROGRAM);
	s2650c->direct = &s2650c->program->direct();
	s2650c->io = device->space(AS_IO);

	device->save_item(NAME(s2650c->ppc));
	device->save_item(NAME(s2650c->page));
	device->save_item(NAME(s2650c->iar));
	device->save_item(NAME(s2650c->ea));
	device->save_item(NAME(s2650c->psl));
	device->save_item(NAME(s2650c->psu));
	device->save_item(NAME(s2650c->r));
	device->save_item(NAME(s2650c->reg));
	device->save_item(NAME(s2650c->halt));
	device->save_item(NAME(s2650c->ir));
	device->save_item(NAME(s2650c->ras));
	device->save_item(NAME(s2650c->irq_state));
}

static CPU_RESET( s2650 )
{
	s2650_regs *s2650c = get_safe_token(device);

	s2650c->ppc = 0;
	s2650c->page = 0,
	s2650c->iar = 0;
	s2650c->ea = 0;
	s2650c->r = 0;
	s2650c->halt = 0;
	s2650c->ir = 0;
	memset(s2650c->reg, 0, sizeof(s2650c->reg));
	memset(s2650c->ras, 0, sizeof(s2650c->ras));

	s2650c->device = device;
	s2650c->program = device->space(AS_PROGRAM);
	s2650c->direct = &s2650c->program->direct();
	s2650c->io = device->space(AS_IO);
	s2650c->psl = COM | WC;
	/* force write */
	s2650c->psu = 0xff;
	set_psu(s2650c, 0);
}

static CPU_EXIT( s2650 )
{
	/* nothing to do */
}

static void set_irq_line(s2650_regs *s2650c, int irqline, int state)
{
	if (irqline == 1)
	{
		if (state == CLEAR_LINE)
			s2650_set_sense(s2650c, 0);
		else
			s2650_set_sense(s2650c, 1);
		return;
	}

	s2650c->irq_state = state;
}

static void s2650_set_flag(s2650_regs *s2650c, int state)
{
    if (state)
        set_psu(s2650c, s2650c->psu | FO);
    else
    	set_psu(s2650c, s2650c->psu & ~FO);
}

static int s2650_get_flag(s2650_regs *s2650c)
{
    return (s2650c->psu & FO) ? 1 : 0;
}

static void s2650_set_sense(s2650_regs *s2650c, int state)
{
    if (state)
    	set_psu(s2650c, s2650c->psu | SI);
    else
    	set_psu(s2650c, s2650c->psu & ~SI);
}

static int s2650_get_sense(s2650_regs *s2650c)
{
	/* OR'd with Input to allow for external connections */

    return (((s2650c->psu & SI) ? 1 : 0) | ((s2650c->io->read_byte(S2650_SENSE_PORT) & SI) ? 1 : 0));
}

static CPU_EXECUTE( s2650 )
{
	s2650_regs *s2650c = get_safe_token(device);

	/* check for external irqs */
	int cycles = check_irq_line(s2650c);
	s2650c->icount -= cycles;

	do
	{
		s2650c->ppc = s2650c->page + s2650c->iar;

		debugger_instruction_hook(device, s2650c->page + s2650c->iar);

		s2650c->ir = ROP(s2650c);
		s2650c->r = s2650c->ir & 3; 		/* register / value */
		switch (s2650c->ir) {
			case 0x00:		/* LODZ,0 */
			case 0x01:		/* LODZ,1 */
			case 0x02:		/* LODZ,2 */
			case 0x03:		/* LODZ,3 */
				s2650c->icount -= 6;
				M_LOD( R0, s2650c->reg[s2650c->r] );
				break;

			case 0x04:		/* LODI,0 v */
			case 0x05:		/* LODI,1 v */
			case 0x06:		/* LODI,2 v */
			case 0x07:		/* LODI,3 v */
				s2650c->icount -= 6;
				M_LOD( s2650c->reg[s2650c->r], ARG(s2650c) );
				break;

			case 0x08:		/* LODR,0 (*)a */
			case 0x09:		/* LODR,1 (*)a */
			case 0x0a:		/* LODR,2 (*)a */
			case 0x0b:		/* LODR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA( s2650c->page );
				M_LOD( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x0c:		/* LODA,0 (*)a(,X) */
			case 0x0d:		/* LODA,1 (*)a(,X) */
			case 0x0e:		/* LODA,2 (*)a(,X) */
			case 0x0f:		/* LODA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_LOD( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x10:		/* illegal */
			case 0x11:		/* illegal */
				s2650c->icount -= 7;
				break;
			case 0x12:		/* SPSU */
				s2650c->icount -= 6;
				M_SPSU();
				break;
			case 0x13:		/* SPSL */
				s2650c->icount -= 6;
				M_SPSL();
				break;

			case 0x14:		/* RETC,0   (zero)  */
			case 0x15:		/* RETC,1   (plus)  */
			case 0x16:		/* RETC,2   (minus) */
				s2650c->icount -= 9;	/* +2 cycles if condition is true */
				M_RET( (s2650c->psl >> 6) == s2650c->r );
				break;
			case 0x17:		/* RETC,3   (always) */
				s2650c->icount -= 9;	/* +2 cycles if condition is true */
				M_RET( 1 );
				break;

			case 0x18:		/* BCTR,0  (*)a */
			case 0x19:		/* BCTR,1  (*)a */
			case 0x1a:		/* BCTR,2  (*)a */
				s2650c->icount -= 9;
				M_BRR( (s2650c->psl >> 6) == s2650c->r );
				break;
			case 0x1b:		/* BCTR,3  (*)a */
				s2650c->icount -= 9;
				M_BRR( 1 );
				break;

			case 0x1c:		/* BCTA,0  (*)a */
			case 0x1d:		/* BCTA,1  (*)a */
			case 0x1e:		/* BCTA,2  (*)a */
				s2650c->icount -= 9;
				M_BRA( (s2650c->psl >> 6) == s2650c->r );
				break;
			case 0x1f:		/* BCTA,3  (*)a */
				s2650c->icount -= 9;
				M_BRA( 1 );
				break;

			case 0x20:		/* EORZ,0 */
			case 0x21:		/* EORZ,1 */
			case 0x22:		/* EORZ,2 */
			case 0x23:		/* EORZ,3 */
				s2650c->icount -= 6;
				M_EOR( R0, s2650c->reg[s2650c->r] );
				break;

			case 0x24:		/* EORI,0 v */
			case 0x25:		/* EORI,1 v */
			case 0x26:		/* EORI,2 v */
			case 0x27:		/* EORI,3 v */
				s2650c->icount -= 6;
				M_EOR( s2650c->reg[s2650c->r], ARG(s2650c) );
				break;

			case 0x28:		/* EORR,0 (*)a */
			case 0x29:		/* EORR,1 (*)a */
			case 0x2a:		/* EORR,2 (*)a */
			case 0x2b:		/* EORR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA( s2650c->page );
				M_EOR( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x2c:		/* EORA,0 (*)a(,X) */
			case 0x2d:		/* EORA,1 (*)a(,X) */
			case 0x2e:		/* EORA,2 (*)a(,X) */
			case 0x2f:		/* EORA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_EOR( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x30:		/* REDC,0 */
			case 0x31:		/* REDC,1 */
			case 0x32:		/* REDC,2 */
			case 0x33:		/* REDC,3 */
				s2650c->icount -= 6;
				s2650c->reg[s2650c->r] = s2650c->io->read_byte(S2650_CTRL_PORT);
				SET_CC( s2650c->reg[s2650c->r] );
				break;

			case 0x34:		/* RETE,0 */
			case 0x35:		/* RETE,1 */
			case 0x36:		/* RETE,2 */
				s2650c->icount -= 9;
				M_RETE( (s2650c->psl >> 6) == s2650c->r );
				break;
			case 0x37:		/* RETE,3 */
				s2650c->icount -= 9;
				M_RETE( 1 );
				break;

			case 0x38:		/* BSTR,0 (*)a */
			case 0x39:		/* BSTR,1 (*)a */
			case 0x3a:		/* BSTR,2 (*)a */
				s2650c->icount -= 9;
				M_BSR( (s2650c->psl >> 6) == s2650c->r );
				break;
			case 0x3b:		/* BSTR,R3 (*)a */
				s2650c->icount -= 9;
				M_BSR( 1 );
				break;

			case 0x3c:		/* BSTA,0 (*)a */
			case 0x3d:		/* BSTA,1 (*)a */
			case 0x3e:		/* BSTA,2 (*)a */
				s2650c->icount -= 9;
				M_BSA( (s2650c->psl >> 6) == s2650c->r );
				break;
			case 0x3f:		/* BSTA,3 (*)a */
				s2650c->icount -= 9;
				M_BSA( 1 );
				break;

			case 0x40:		/* HALT */
				s2650c->icount -= 6;
				s2650c->iar = (s2650c->iar - 1) & PMSK;
				s2650c->halt = 1;
				if (s2650c->icount > 0)
					s2650c->icount = 0;
				break;
			case 0x41:		/* ANDZ,1 */
			case 0x42:		/* ANDZ,2 */
			case 0x43:		/* ANDZ,3 */
				s2650c->icount -= 6;
				M_AND( R0, s2650c->reg[s2650c->r] );
				break;

			case 0x44:		/* ANDI,0 v */
			case 0x45:		/* ANDI,1 v */
			case 0x46:		/* ANDI,2 v */
			case 0x47:		/* ANDI,3 v */
				s2650c->icount -= 6;
				M_AND( s2650c->reg[s2650c->r], ARG(s2650c) );
				break;

			case 0x48:		/* ANDR,0 (*)a */
			case 0x49:		/* ANDR,1 (*)a */
			case 0x4a:		/* ANDR,2 (*)a */
			case 0x4b:		/* ANDR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA( s2650c->page );
				M_AND( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x4c:		/* ANDA,0 (*)a(,X) */
			case 0x4d:		/* ANDA,1 (*)a(,X) */
			case 0x4e:		/* ANDA,2 (*)a(,X) */
			case 0x4f:		/* ANDA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_AND( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x50:		/* RRR,0 */
			case 0x51:		/* RRR,1 */
			case 0x52:		/* RRR,2 */
			case 0x53:		/* RRR,3 */
				s2650c->icount -= 6;
				M_RRR( s2650c->reg[s2650c->r] );
				break;

			case 0x54:		/* REDE,0 v */
			case 0x55:		/* REDE,1 v */
			case 0x56:		/* REDE,2 v */
			case 0x57:		/* REDE,3 v */
				s2650c->icount -= 9;
				s2650c->reg[s2650c->r] = s2650c->io->read_byte( ARG(s2650c) );
				SET_CC(s2650c->reg[s2650c->r]);
				break;

			case 0x58:		/* BRNR,0 (*)a */
			case 0x59:		/* BRNR,1 (*)a */
			case 0x5a:		/* BRNR,2 (*)a */
			case 0x5b:		/* BRNR,3 (*)a */
				s2650c->icount -= 9;
				M_BRR( s2650c->reg[s2650c->r] );
				break;

			case 0x5c:		/* BRNA,0 (*)a */
			case 0x5d:		/* BRNA,1 (*)a */
			case 0x5e:		/* BRNA,2 (*)a */
			case 0x5f:		/* BRNA,3 (*)a */
				s2650c->icount -= 9;
				M_BRA( s2650c->reg[s2650c->r] );
				break;

			case 0x60:		/* IORZ,0 */
			case 0x61:		/* IORZ,1 */
			case 0x62:		/* IORZ,2 */
			case 0x63:		/* IORZ,3 */
				s2650c->icount -= 6;
				M_IOR( R0, s2650c->reg[s2650c->r] );
				break;

			case 0x64:		/* IORI,0 v */
			case 0x65:		/* IORI,1 v */
			case 0x66:		/* IORI,2 v */
			case 0x67:		/* IORI,3 v */
				s2650c->icount -= 6;
				M_IOR( s2650c->reg[s2650c->r], ARG(s2650c) );
				break;

			case 0x68:		/* IORR,0 (*)a */
			case 0x69:		/* IORR,1 (*)a */
			case 0x6a:		/* IORR,2 (*)a */
			case 0x6b:		/* IORR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA( s2650c->page );
				M_IOR( s2650c->reg[s2650c-> r],RDMEM(s2650c->ea) );
				break;

			case 0x6c:		/* IORA,0 (*)a(,X) */
			case 0x6d:		/* IORA,1 (*)a(,X) */
			case 0x6e:		/* IORA,2 (*)a(,X) */
			case 0x6f:		/* IORA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_IOR( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x70:		/* REDD,0 */
			case 0x71:		/* REDD,1 */
			case 0x72:		/* REDD,2 */
			case 0x73:		/* REDD,3 */
				s2650c->icount -= 6;
				s2650c->reg[s2650c->r] = s2650c->io->read_byte(S2650_DATA_PORT);
				SET_CC(s2650c->reg[s2650c->r]);
				break;

			case 0x74:		/* CPSU */
				s2650c->icount -= 9;
				M_CPSU();
				break;
			case 0x75:		/* CPSL */
				s2650c->icount -= 9;
				M_CPSL();
				break;
			case 0x76:		/* PPSU */
				s2650c->icount -= 9;
				M_PPSU();
				break;
			case 0x77:		/* PPSL */
				s2650c->icount -= 9;
				M_PPSL();
				break;

			case 0x78:		/* BSNR,0 (*)a */
			case 0x79:		/* BSNR,1 (*)a */
			case 0x7a:		/* BSNR,2 (*)a */
			case 0x7b:		/* BSNR,3 (*)a */
				s2650c->icount -= 9;
				M_BSR( s2650c->reg[s2650c->r] );
				break;

			case 0x7c:		/* BSNA,0 (*)a */
			case 0x7d:		/* BSNA,1 (*)a */
			case 0x7e:		/* BSNA,2 (*)a */
			case 0x7f:		/* BSNA,3 (*)a */
				s2650c->icount -= 9;
				M_BSA( s2650c->reg[s2650c->r] );
				break;

			case 0x80:		/* ADDZ,0 */
			case 0x81:		/* ADDZ,1 */
			case 0x82:		/* ADDZ,2 */
			case 0x83:		/* ADDZ,3 */
				s2650c->icount -= 6;
				M_ADD( R0,s2650c->reg[s2650c->r] );
				break;

			case 0x84:		/* ADDI,0 v */
			case 0x85:		/* ADDI,1 v */
			case 0x86:		/* ADDI,2 v */
			case 0x87:		/* ADDI,3 v */
				s2650c->icount -= 6;
				M_ADD( s2650c->reg[s2650c->r], ARG(s2650c) );
				break;

			case 0x88:		/* ADDR,0 (*)a */
			case 0x89:		/* ADDR,1 (*)a */
			case 0x8a:		/* ADDR,2 (*)a */
			case 0x8b:		/* ADDR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA(s2650c->page);
				M_ADD( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x8c:		/* ADDA,0 (*)a(,X) */
			case 0x8d:		/* ADDA,1 (*)a(,X) */
			case 0x8e:		/* ADDA,2 (*)a(,X) */
			case 0x8f:		/* ADDA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_ADD( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0x90:		/* illegal */
			case 0x91:		/* illegal */
				s2650c->icount -= 7;
				break;
			case 0x92:		/* LPSU */
				s2650c->icount -= 6;
				set_psu(s2650c, (R0 & ~PSU34) & ~SI);
				break;
			case 0x93:		/* LPSL */
				s2650c->icount -= 6;
				/* change register set ? */
				if ((s2650c->psl ^ R0) & RS)
					SWAP_REGS;
				s2650c->psl = R0;
				break;

			case 0x94:		/* DAR,0 */
			case 0x95:		/* DAR,1 */
			case 0x96:		/* DAR,2 */
			case 0x97:		/* DAR,3 */
				s2650c->icount -= 9;
				M_DAR( s2650c->reg[s2650c->r] );
				break;

			case 0x98:		/* BCFR,0 (*)a */
			case 0x99:		/* BCFR,1 (*)a */
			case 0x9a:		/* BCFR,2 (*)a */
				s2650c->icount -= 9;
				M_BRR( (s2650c->psl >> 6) != s2650c->r );
				break;
			case 0x9b:		/* ZBRR    (*)a */
				s2650c->icount -= 9;
				M_ZBRR();
				break;

			case 0x9c:		/* BCFA,0 (*)a */
			case 0x9d:		/* BCFA,1 (*)a */
			case 0x9e:		/* BCFA,2 (*)a */
				s2650c->icount -= 9;
				M_BRA( (s2650c->psl >> 6) != s2650c->r );
				break;
			case 0x9f:		/* BXA     (*)a */
				s2650c->icount -= 9;
				M_BXA();
				break;

			case 0xa0:		/* SUBZ,0 */
			case 0xa1:		/* SUBZ,1 */
			case 0xa2:		/* SUBZ,2 */
			case 0xa3:		/* SUBZ,3 */
				s2650c->icount -= 6;
				M_SUB( R0, s2650c->reg[s2650c->r] );
				break;

			case 0xa4:		/* SUBI,0 v */
			case 0xa5:		/* SUBI,1 v */
			case 0xa6:		/* SUBI,2 v */
			case 0xa7:		/* SUBI,3 v */
				s2650c->icount -= 6;
				M_SUB( s2650c->reg[s2650c->r], ARG(s2650c) );
				break;

			case 0xa8:		/* SUBR,0 (*)a */
			case 0xa9:		/* SUBR,1 (*)a */
			case 0xaa:		/* SUBR,2 (*)a */
			case 0xab:		/* SUBR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA(s2650c->page);
				M_SUB( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0xac:		/* SUBA,0 (*)a(,X) */
			case 0xad:		/* SUBA,1 (*)a(,X) */
			case 0xae:		/* SUBA,2 (*)a(,X) */
			case 0xaf:		/* SUBA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_SUB( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0xb0:		/* WRTC,0 */
			case 0xb1:		/* WRTC,1 */
			case 0xb2:		/* WRTC,2 */
			case 0xb3:		/* WRTC,3 */
				s2650c->icount -= 6;
				s2650c->io->write_byte(S2650_CTRL_PORT,s2650c->reg[s2650c->r]);
				break;

			case 0xb4:		/* TPSU */
				s2650c->icount -= 9;
				M_TPSU();
				break;
			case 0xb5:		/* TPSL */
				s2650c->icount -= 9;
				M_TPSL();
				break;
			case 0xb6:		/* illegal */
			case 0xb7:		/* illegal */
				s2650c->icount -= 7;
				break;

			case 0xb8:		/* BSFR,0 (*)a */
			case 0xb9:		/* BSFR,1 (*)a */
			case 0xba:		/* BSFR,2 (*)a */
				s2650c->icount -= 9;
				M_BSR( (s2650c->psl >> 6) != s2650c->r );
				break;
			case 0xbb:		/* ZBSR    (*)a */
				s2650c->icount -= 9;
				M_ZBSR();
				break;

			case 0xbc:		/* BSFA,0 (*)a */
			case 0xbd:		/* BSFA,1 (*)a */
			case 0xbe:		/* BSFA,2 (*)a */
				s2650c->icount -= 9;
				M_BSA( (s2650c->psl >> 6) != s2650c->r );
				break;
			case 0xbf:		/* BSXA    (*)a */
				s2650c->icount -= 9;
				M_BSXA();
				break;

			case 0xc0:		/* NOP */
				s2650c->icount -= 6;
				break;
			case 0xc1:		/* STRZ,1 */
			case 0xc2:		/* STRZ,2 */
			case 0xc3:		/* STRZ,3 */
				s2650c->icount -= 6;
				M_LOD( s2650c->reg[s2650c->r], R0 );
				break;

			case 0xc4:		/* illegal */
			case 0xc5:		/* illegal */
			case 0xc6:		/* illegal */
			case 0xc7:		/* illegal */
				s2650c->icount -= 7;
				break;

			case 0xc8:		/* STRR,0 (*)a */
			case 0xc9:		/* STRR,1 (*)a */
			case 0xca:		/* STRR,2 (*)a */
			case 0xcb:		/* STRR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA(s2650c->page);
				M_STR( s2650c->ea, s2650c->reg[s2650c->r] );
				break;

			case 0xcc:		/* STRA,0 (*)a(,X) */
			case 0xcd:		/* STRA,1 (*)a(,X) */
			case 0xce:		/* STRA,2 (*)a(,X) */
			case 0xcf:		/* STRA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_STR( s2650c->ea, s2650c->reg[s2650c->r] );
				break;

			case 0xd0:		/* RRL,0 */
			case 0xd1:		/* RRL,1 */
			case 0xd2:		/* RRL,2 */
			case 0xd3:		/* RRL,3 */
				s2650c->icount -= 6;
				M_RRL( s2650c->reg[s2650c->r] );
				break;

			case 0xd4:		/* WRTE,0 v */
			case 0xd5:		/* WRTE,1 v */
			case 0xd6:		/* WRTE,2 v */
			case 0xd7:		/* WRTE,3 v */
				s2650c->icount -= 9;
				s2650c->io->write_byte( ARG(s2650c), s2650c->reg[s2650c->r] );
				break;

			case 0xd8:		/* BIRR,0 (*)a */
			case 0xd9:		/* BIRR,1 (*)a */
			case 0xda:		/* BIRR,2 (*)a */
			case 0xdb:		/* BIRR,3 (*)a */
				s2650c->icount -= 9;
				M_BRR( ++s2650c->reg[s2650c->r] );
				break;

			case 0xdc:		/* BIRA,0 (*)a */
			case 0xdd:		/* BIRA,1 (*)a */
			case 0xde:		/* BIRA,2 (*)a */
			case 0xdf:		/* BIRA,3 (*)a */
				s2650c->icount -= 9;
				M_BRA( ++s2650c->reg[s2650c->r] );
				break;

			case 0xe0:		/* COMZ,0 */
			case 0xe1:		/* COMZ,1 */
			case 0xe2:		/* COMZ,2 */
			case 0xe3:		/* COMZ,3 */
				s2650c->icount -= 6;
				M_COM( R0, s2650c->reg[s2650c->r] );
				break;

			case 0xe4:		/* COMI,0 v */
			case 0xe5:		/* COMI,1 v */
			case 0xe6:		/* COMI,2 v */
			case 0xe7:		/* COMI,3 v */
				s2650c->icount -= 6;
				M_COM( s2650c->reg[s2650c->r], ARG(s2650c) );
				break;

			case 0xe8:		/* COMR,0 (*)a */
			case 0xe9:		/* COMR,1 (*)a */
			case 0xea:		/* COMR,2 (*)a */
			case 0xeb:		/* COMR,3 (*)a */
				s2650c->icount -= 9;
				REL_EA(s2650c->page);
				M_COM( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0xec:		/* COMA,0 (*)a(,X) */
			case 0xed:		/* COMA,1 (*)a(,X) */
			case 0xee:		/* COMA,2 (*)a(,X) */
			case 0xef:		/* COMA,3 (*)a(,X) */
				s2650c->icount -= 12;
				ABS_EA();
				M_COM( s2650c->reg[s2650c->r], RDMEM(s2650c->ea) );
				break;

			case 0xf0:		/* WRTD,0 */
			case 0xf1:		/* WRTD,1 */
			case 0xf2:		/* WRTD,2 */
			case 0xf3:		/* WRTD,3 */
				s2650c->icount -= 6;
				s2650c->io->write_byte(S2650_DATA_PORT, s2650c->reg[s2650c->r]);
				break;

			case 0xf4:		/* TMI,0  v */
			case 0xf5:		/* TMI,1  v */
			case 0xf6:		/* TMI,2  v */
			case 0xf7:		/* TMI,3  v */
				s2650c->icount -= 9;
				M_TMI( s2650c->reg[s2650c->r] );
				break;

			case 0xf8:		/* BDRR,0 (*)a */
			case 0xf9:		/* BDRR,1 (*)a */
			case 0xfa:		/* BDRR,2 (*)a */
			case 0xfb:		/* BDRR,3 (*)a */
				s2650c->icount -= 9;
				M_BRR( --s2650c->reg[s2650c->r] );
				break;

			case 0xfc:		/* BDRA,0 (*)a */
			case 0xfd:		/* BDRA,1 (*)a */
			case 0xfe:		/* BDRA,2 (*)a */
			case 0xff:		/* BDRA,3 (*)a */
				s2650c->icount -= 9;
				M_BRA( --s2650c->reg[s2650c->r] );
				break;
		}
	} while( s2650c->icount > 0 );
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( s2650 )
{
	s2650_regs *s2650c = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(s2650c, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:				set_irq_line(s2650c, 1, info->i);				break;

		case CPUINFO_INT_PC:
			s2650c->page = info->i & PAGE;
			s2650c->iar = info->i & PMSK;
			break;

		case CPUINFO_INT_REGISTER + S2650_PC:			s2650c->page = info->i & PAGE; s2650c->iar = info->i & PMSK; break;
		case CPUINFO_INT_SP:							set_sp(s2650c, info->i);						break;
		case CPUINFO_INT_REGISTER + S2650_PS:			s2650c->psl = info->i & 0xff; set_psu(s2650c, info->i >> 8); break;
		case CPUINFO_INT_REGISTER + S2650_R0:			s2650c->reg[0] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R1:			s2650c->reg[1] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R2:			s2650c->reg[2] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R3:			s2650c->reg[3] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R1A:			s2650c->reg[4] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R2A:			s2650c->reg[5] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_R3A:			s2650c->reg[6] = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_HALT:			s2650c->halt = info->i;						break;
		case CPUINFO_INT_REGISTER + S2650_SI:			s2650_set_sense(s2650c, info->i);				break;
		case CPUINFO_INT_REGISTER + S2650_FO:			s2650_set_flag(s2650c, info->i);				break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( s2650 )
{
	s2650_regs *s2650c = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(s2650_regs);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 5;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 13;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 15;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;					break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 9;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = s2650c->irq_state;					break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = s2650_get_sense(s2650c) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = s2650c->ppc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + S2650_PC:			info->i = s2650c->page + s2650c->iar;				break;

		case CPUINFO_INT_SP:							info->i = get_sp(s2650c);					break;
		case CPUINFO_INT_REGISTER + S2650_PS:			info->i = (s2650c->psu << 8) | s2650c->psl;			break;
		case CPUINFO_INT_REGISTER + S2650_R0:			info->i = s2650c->reg[0];						break;
		case CPUINFO_INT_REGISTER + S2650_R1:			info->i = s2650c->reg[1];						break;
		case CPUINFO_INT_REGISTER + S2650_R2:			info->i = s2650c->reg[2];						break;
		case CPUINFO_INT_REGISTER + S2650_R3:			info->i = s2650c->reg[3];						break;
		case CPUINFO_INT_REGISTER + S2650_R1A:			info->i = s2650c->reg[4];						break;
		case CPUINFO_INT_REGISTER + S2650_R2A:			info->i = s2650c->reg[5];						break;
		case CPUINFO_INT_REGISTER + S2650_R3A:			info->i = s2650c->reg[6];						break;
		case CPUINFO_INT_REGISTER + S2650_HALT:			info->i = s2650c->halt;						break;
		case CPUINFO_INT_REGISTER + S2650_SI:			info->i = s2650_get_sense(s2650c);			break;
		case CPUINFO_INT_REGISTER + S2650_FO:			info->i = s2650_get_flag(s2650c);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(s2650);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(s2650);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(s2650);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(s2650);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(s2650);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(s2650);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &s2650c->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "S2650");				break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Signetics 2650");		break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.2");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Written by Juergen Buchmueller for use with MAME"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				s2650c->psu & 0x80 ? 'S':'.',
				s2650c->psu & 0x40 ? 'O':'.',
				s2650c->psu & 0x20 ? 'I':'.',
				s2650c->psu & 0x10 ? '?':'.',
				s2650c->psu & 0x08 ? '?':'.',
				s2650c->psu & 0x04 ? 's':'.',
				s2650c->psu & 0x02 ? 's':'.',
				s2650c->psu & 0x01 ? 's':'.',
                s2650c->psl & 0x80 ? 'M':'.',
				s2650c->psl & 0x40 ? 'P':'.',
				s2650c->psl & 0x20 ? 'H':'.',
				s2650c->psl & 0x10 ? 'R':'.',
				s2650c->psl & 0x08 ? 'W':'.',
				s2650c->psl & 0x04 ? 'V':'.',
				s2650c->psl & 0x02 ? '2':'.',
				s2650c->psl & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + S2650_PC:			sprintf(info->s, "PC:%04X", s2650c->page + s2650c->iar); break;
		case CPUINFO_STR_REGISTER + S2650_PS:			sprintf(info->s, "PS:%02X%02X", s2650c->psu, s2650c->psl); break;
		case CPUINFO_STR_REGISTER + S2650_R0:			sprintf(info->s, "R0:%02X", s2650c->reg[0]);	break;
		case CPUINFO_STR_REGISTER + S2650_R1:			sprintf(info->s, "R1:%02X", s2650c->reg[1]);	break;
		case CPUINFO_STR_REGISTER + S2650_R2:			sprintf(info->s, "R2:%02X", s2650c->reg[2]);	break;
		case CPUINFO_STR_REGISTER + S2650_R3:			sprintf(info->s, "R3:%02X", s2650c->reg[3]);	break;
		case CPUINFO_STR_REGISTER + S2650_R1A:			sprintf(info->s, "R1':%02X", s2650c->reg[4]);	break;
		case CPUINFO_STR_REGISTER + S2650_R2A:			sprintf(info->s, "R2':%02X", s2650c->reg[5]);	break;
		case CPUINFO_STR_REGISTER + S2650_R3A:			sprintf(info->s, "R3':%02X", s2650c->reg[6]);	break;
		case CPUINFO_STR_REGISTER + S2650_HALT:			sprintf(info->s, "HALT:%X", s2650c->halt);	break;
		case CPUINFO_STR_REGISTER + S2650_SI:			sprintf(info->s, "SI:%X", (s2650c->psu & SI) ? 1 : 0); break;
		case CPUINFO_STR_REGISTER + S2650_FO:			sprintf(info->s, "FO:%X", (s2650c->psu & FO) ? 1 : 0); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(S2650, s2650);
