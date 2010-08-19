/****************************************************************************

    NEC V20/V30/V33 emulator

    ---------------------------------------------

    V20 = uPD70108 = 8-bit data bus @ 5MHz or 8MHz
    V20HL = uPD70108H = V20 with EMS support (24-bit address bus)

    V25 = uPD70320 = V20 with on-chip features:
            - 256 bytes on-chip RAM
            - 8 register banks
            - 4-bit input port
            - 20-bit I/O port
            - 2 channel serial interface
            - interrupt controller
            - 2 channel DMA controller
            - 2 channel 16-bit timer
            - new instructions: BTCLR, RETRBI, STOP, BRKCS, TSKSW,
                                MOVSPA, MOVSPB

    V25+ = uPD70325 = V25 @ 8MHz or 10MHz plus changes:
            - faster DMA
            - improved serial interface

    ---------------------------------------------

    V30 = uPD70116 = 16-bit data bus version of V20
    V30HL = uPD70116H = 16-bit data bus version of V20HL
    V30MX = V30HL with separate address and data busses

    V35 = uPD70330 = 16-bit data bus version of V25

    V35+ = uPD70335 = 16-bit data bus version of V25+

    ---------------------------------------------

    V40 = uPD70208 = 8-bit data bus @ 10MHz
    V40HL = uPD70208H = V40 with support up to 20Mhz

    ---------------------------------------------

    V50 = uPD70216 = 16-bit data bus version of V40
    V50HL = uPD70216H = 16-bit data bus version of V40HL

    ---------------------------------------------

    V41 = uPD70270

    V51 = uPD70280



    V33A = uPD70136A

    V53A = uPD70236A



    Instruction differences:
        V20, V30, V40, V50 have dedicated emulation instructions
            (BRKEM, RETEM, CALLN)

        V33A, V53A has dedicated address mode instructions
            (BRKXA, RETXA)



    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
    on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
    on the i286 emulator by Fabrice Frances which had initial work based on
    David Hedley's pcemu(!).

    This new core features 99% accurate cycle counts for each processor,
    there are still some complex situations where cycle counts are wrong,
    typically where a few instructions have differing counts for odd/even
    source and odd/even destination memory operands.

    Flag settings are also correct for the NEC processors rather than the
    I86 versions.

    Changelist:

    22/02/2003:
        Removed cycle counts from memory accesses - they are certainly wrong,
        and there is already a memory access cycle penalty in the opcodes
        using them.

        Fixed save states.

        Fixed ADJBA/ADJBS/ADJ4A/ADJ4S flags/return values for all situations.
        (Fixes bugs in Geostorm and Thunderblaster)

        Fixed carry flag on NEG (I thought this had been fixed circa Mame 0.58,
        but it seems I never actually submitted the fix).

        Fixed many cycle counts in instructions and bug in cycle count
        macros (odd word cases were testing for odd instruction word address
        not data address).

    Todo!
        Double check cycle timing is 100%.

****************************************************************************/

#include "emu.h"
#include "debugger.h"

typedef UINT8 BOOLEAN;
typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef UINT32 DWORD;

#include "necpriv.h"
#include "nec.h"

#define PC(n)		(((n)->sregs[PS]<<4)+(n)->ip)

/* default configuration */
static const nec_config default_config =
{
	NULL
};

extern int necv_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, const nec_config *config);

/* NEC registers */
typedef union
{                   /* eight general registers */
    UINT16 w[8];    /* viewed as 16 bits registers */
    UINT8  b[16];   /* or as 8 bit registers */
} necbasicregs;

typedef struct
{
	offs_t	fetch_xor;

	UINT8	(*rbyte)(address_space *, offs_t);
	UINT16	(*rword)(address_space *, offs_t);
	void	(*wbyte)(address_space *, offs_t, UINT8);
	void	(*wword)(address_space *, offs_t, UINT16);
} memory_interface;


typedef struct _nec_state_t nec_state_t;
struct _nec_state_t
{
	necbasicregs regs;
	UINT16	sregs[4];

	UINT16	ip;

	INT32	SignVal;
    UINT32  AuxVal, OverVal, ZeroVal, CarryVal, ParityVal; /* 0 or non-0 valued flags */
	UINT8	TF, IF, DF, MF; 	/* 0 or 1 valued flags */	/* OB[19.07.99] added Mode Flag V30 */
	UINT32	int_vector;
	UINT32	pending_irq;
	UINT32	nmi_state;
	UINT32	irq_state;
	UINT32	poll_state;
	UINT8	no_interrupt;

	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	address_space *io;
	int		icount;

	memory_interface	mem;

	const nec_config *config;

	UINT8	prefetch_size;
	UINT8	prefetch_cycles;
	INT8	prefetch_count;
	UINT8	prefetch_reset;
	UINT32	chip_type;

	UINT32	prefix_base;	/* base address of the latest prefix segment */
	UINT8	seg_prefix;		/* prefix segment indicator */

};

INLINE nec_state_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == V20 ||
		   device->type() == V25 ||
		   device->type() == V30 ||
		   device->type() == V33 ||
		   device->type() == V35);
	return (nec_state_t *)downcast<legacy_cpu_device *>(device)->token();
}

/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

INLINE void prefetch(nec_state_t *nec_state)
{
	nec_state->prefetch_count--;
}

static void do_prefetch(nec_state_t *nec_state, int previous_ICount)
{
	int diff = previous_ICount - (int) nec_state->icount;

	/* The implementation is not accurate, but comes close.
     * It does not respect that the V30 will fetch two bytes
     * at once directly, but instead uses only 2 cycles instead
     * of 4. There are however only very few sources publicy
     * available and they are vague.
     */
	while (nec_state->prefetch_count<0)
	{
		nec_state->prefetch_count++;
		if (diff>nec_state->prefetch_cycles)
			diff -= nec_state->prefetch_cycles;
		else
			nec_state->icount -= nec_state->prefetch_cycles;
	}

	if (nec_state->prefetch_reset)
	{
		nec_state->prefetch_count = 0;
		nec_state->prefetch_reset = 0;
		return;
	}

	while (diff>=nec_state->prefetch_cycles && nec_state->prefetch_count < nec_state->prefetch_size)
	{
		diff -= nec_state->prefetch_cycles;
		nec_state->prefetch_count++;
	}

}

INLINE UINT8 fetch(nec_state_t *nec_state)
{
	prefetch(nec_state);
	return memory_raw_read_byte(nec_state->program, FETCH_XOR((nec_state->sregs[PS]<<4)+nec_state->ip++));
}

INLINE UINT16 fetchword(nec_state_t *nec_state)
{
	UINT16 r = FETCH();
	r |= (FETCH()<<8);
	return r;
}

#include "necinstr.h"
#include "necea.h"
#include "necmodrm.h"

static UINT8 parity_table[256];

#include "emu.h"

static UINT8 fetchop(nec_state_t *nec_state)
{
	UINT8 ret;

	prefetch(nec_state);
	ret = memory_decrypted_read_byte(nec_state->program, FETCH_XOR( ( nec_state->sregs[PS]<<4)+nec_state->ip++));

	if (nec_state->MF == 1)
		if (nec_state->config->v25v35_decryptiontable)
		{
			ret = nec_state->config->v25v35_decryptiontable[ret];
		}
	return ret;
}



/***************************************************************************/

static CPU_RESET( nec )
{
	nec_state_t *nec_state = get_safe_token(device);
	unsigned int i,j,c;
    static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	memset( &nec_state->regs.w, 0, sizeof(nec_state->regs.w));
	memset( &nec_state->sregs, 0, sizeof(nec_state->sregs));

	nec_state->ip = 0;
	nec_state->TF = 0;
	nec_state->IF = 0;
	nec_state->DF = 0;
	nec_state->MF = 0;
	nec_state->SignVal = 0;
	nec_state->int_vector = 0;
	nec_state->pending_irq = 0;
	nec_state->nmi_state = 0;
	nec_state->irq_state = 0;
	nec_state->poll_state = 0;
	nec_state->AuxVal = 0;
	nec_state->OverVal = 0;
	nec_state->ZeroVal = 0;
	nec_state->CarryVal = 0;
	nec_state->ParityVal = 0;


	nec_state->sregs[PS] = 0xffff;

	CHANGE_PC;

    for (i = 0;i < 256; i++)
    {
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;
		parity_table[i] = !(c & 1);
    }

	nec_state->ZeroVal = nec_state->ParityVal = 1;
	SetMD(1);						/* set the mode-flag = native mode */

	/* for v25+ / v35+ there is an internal decryption table */
	//nec_state->config->v25v35_decryptiontable = gussun_test_decryption_table;

    for (i = 0; i < 256; i++)
    {
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
    }

    for (i = 0xc0; i < 0x100; i++)
    {
		Mod_RM.RM.w[i] = (WREGS)( i & 7 );
		Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
    }

	nec_state->poll_state = 1;
}

static CPU_EXIT( nec )
{

}

static void nec_interrupt(nec_state_t *nec_state, unsigned int_num,BOOLEAN md_flag)
{
    UINT32 dest_seg, dest_off;

    i_pushf(nec_state);
	nec_state->TF = nec_state->IF = 0;
	if (md_flag) SetMD(0);	/* clear Mode-flag = start 8080 emulation mode */

	if (int_num == -1)
	{
		int_num = (*nec_state->irq_callback)(nec_state->device, 0);

		nec_state->irq_state = CLEAR_LINE;
		nec_state->pending_irq &= ~INT_IRQ;
	}

    dest_off = read_mem_word(int_num*4);
    dest_seg = read_mem_word(int_num*4+2);

	PUSH(nec_state->sregs[PS]);
	PUSH(nec_state->ip);
	nec_state->ip = (WORD)dest_off;
	nec_state->sregs[PS] = (WORD)dest_seg;
	CHANGE_PC;
}

static void nec_trap(nec_state_t *nec_state)
{
	nec_instruction[fetchop(nec_state)](nec_state);
	nec_interrupt(nec_state, 1,0);
}

static void external_int(nec_state_t *nec_state)
{
	if( nec_state->pending_irq & NMI_IRQ )
	{
		nec_interrupt(nec_state, NEC_NMI_INT_VECTOR,0);
		nec_state->pending_irq &= ~NMI_IRQ;
	}
	else if( nec_state->pending_irq )
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt(nec_state, (UINT32)-1,0);
	}
}

/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

#define OP(num,func_name) static void func_name(nec_state_t *nec_state)

OP( 0x00, i_add_br8  ) { DEF_br8;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x01, i_add_wr16 ) { DEF_wr16;	ADDW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x02, i_add_r8b  ) { DEF_r8b;	ADDB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x03, i_add_r16w ) { DEF_r16w;	ADDW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x04, i_add_ald8 ) { DEF_ald8;	ADDB;	nec_state->regs.b[AL]=dst;			CLKS(4,4,2);				}
OP( 0x05, i_add_axd16) { DEF_axd16;	ADDW;	nec_state->regs.w[AW]=dst;			CLKS(4,4,2);				}
OP( 0x06, i_push_es  ) { PUSH(nec_state->sregs[DS1]);	CLKS(12,8,3);	}
OP( 0x07, i_pop_es   ) { POP(nec_state->sregs[DS1]);	CLKS(12,8,5);	}

OP( 0x08, i_or_br8   ) { DEF_br8;	ORB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x09, i_or_wr16  ) { DEF_wr16;	ORW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x0a, i_or_r8b   ) { DEF_r8b;	ORB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x0b, i_or_r16w  ) { DEF_r16w;	ORW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x0c, i_or_ald8  ) { DEF_ald8;	ORB;	nec_state->regs.b[AL]=dst;			CLKS(4,4,2);				}
OP( 0x0d, i_or_axd16 ) { DEF_axd16;	ORW;	nec_state->regs.w[AW]=dst;			CLKS(4,4,2);				}
OP( 0x0e, i_push_cs  ) { PUSH(nec_state->sregs[PS]);	CLKS(12,8,3);	}
OP( 0x0f, i_pre_nec  ) { UINT32 ModRM, tmp, tmp2;
	switch (FETCH()) {
		case 0x10 : BITOP_BYTE;	CLKS(3,3,4); tmp2 = nec_state->regs.b[CL] & 0x7;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x11 : BITOP_WORD;	CLKS(3,3,4); tmp2 = nec_state->regs.b[CL] & 0xf;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x12 : BITOP_BYTE;	CLKS(5,5,4); tmp2 = nec_state->regs.b[CL] & 0x7;	tmp &= ~(1<<tmp2);	PutbackRMByte(ModRM,tmp);	break; /* Clr */
		case 0x13 : BITOP_WORD;	CLKS(5,5,4); tmp2 = nec_state->regs.b[CL] & 0xf;	tmp &= ~(1<<tmp2);	PutbackRMWord(ModRM,tmp);	break; /* Clr */
		case 0x14 : BITOP_BYTE;	CLKS(4,4,4); tmp2 = nec_state->regs.b[CL] & 0x7;	tmp |= (1<<tmp2);	PutbackRMByte(ModRM,tmp);	break; /* Set */
		case 0x15 : BITOP_WORD;	CLKS(4,4,4); tmp2 = nec_state->regs.b[CL] & 0xf;	tmp |= (1<<tmp2);	PutbackRMWord(ModRM,tmp);	break; /* Set */
		case 0x16 : BITOP_BYTE;	CLKS(4,4,4); tmp2 = nec_state->regs.b[CL] & 0x7;	BIT_NOT;			PutbackRMByte(ModRM,tmp);	break; /* Not */
		case 0x17 : BITOP_WORD;	CLKS(4,4,4); tmp2 = nec_state->regs.b[CL] & 0xf;	BIT_NOT;			PutbackRMWord(ModRM,tmp);	break; /* Not */

		case 0x18 : BITOP_BYTE;	CLKS(4,4,4); tmp2 = (FETCH()) & 0x7;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x19 : BITOP_WORD;	CLKS(4,4,4); tmp2 = (FETCH()) & 0xf;	nec_state->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	nec_state->CarryVal=nec_state->OverVal=0; break; /* Test */
		case 0x1a : BITOP_BYTE;	CLKS(6,6,4); tmp2 = (FETCH()) & 0x7;	tmp &= ~(1<<tmp2);		PutbackRMByte(ModRM,tmp);	break; /* Clr */
		case 0x1b : BITOP_WORD;	CLKS(6,6,4); tmp2 = (FETCH()) & 0xf;	tmp &= ~(1<<tmp2);		PutbackRMWord(ModRM,tmp);	break; /* Clr */
		case 0x1c : BITOP_BYTE;	CLKS(5,5,4); tmp2 = (FETCH()) & 0x7;	tmp |= (1<<tmp2);		PutbackRMByte(ModRM,tmp);	break; /* Set */
		case 0x1d : BITOP_WORD;	CLKS(5,5,4); tmp2 = (FETCH()) & 0xf;	tmp |= (1<<tmp2);		PutbackRMWord(ModRM,tmp);	break; /* Set */
		case 0x1e : BITOP_BYTE;	CLKS(5,5,4); tmp2 = (FETCH()) & 0x7;	BIT_NOT;				PutbackRMByte(ModRM,tmp);	break; /* Not */
		case 0x1f : BITOP_WORD;	CLKS(5,5,4); tmp2 = (FETCH()) & 0xf;	BIT_NOT;				PutbackRMWord(ModRM,tmp);	break; /* Not */

		case 0x20 :	ADD4S; CLKS(7,7,2); break;
		case 0x22 :	SUB4S; CLKS(7,7,2); break;
		case 0x26 :	CMP4S; CLKS(7,7,2); break;
		case 0x28 : ModRM = FETCH(); tmp = GetRMByte(ModRM); tmp <<= 4; tmp |= nec_state->regs.b[AL] & 0xf; nec_state->regs.b[AL] = (nec_state->regs.b[AL] & 0xf0) | ((tmp>>8)&0xf); tmp &= 0xff; PutbackRMByte(ModRM,tmp); CLKM(13,13,9,28,28,15); break;
		case 0x2a : ModRM = FETCH(); tmp = GetRMByte(ModRM); tmp2 = (nec_state->regs.b[AL] & 0xf)<<4; nec_state->regs.b[AL] = (nec_state->regs.b[AL] & 0xf0) | (tmp&0xf); tmp = tmp2 | (tmp>>4);	PutbackRMByte(ModRM,tmp); CLKM(17,17,13,32,32,19); break;
		case 0x31 : ModRM = FETCH(); ModRM=0; logerror("%06x: Unimplemented bitfield INS\n",PC(nec_state)); break;
		case 0x33 : ModRM = FETCH(); ModRM=0; logerror("%06x: Unimplemented bitfield EXT\n",PC(nec_state)); break;
		case 0x92 : CLK(2); break; /* V25/35 FINT */
		case 0xe0 : ModRM = FETCH(); ModRM=0; logerror("%06x: V33 unimplemented BRKXA (break to expansion address)\n",PC(nec_state)); break;
		case 0xf0 : ModRM = FETCH(); ModRM=0; logerror("%06x: V33 unimplemented RETXA (return from expansion address)\n",PC(nec_state)); break;
		case 0xff : ModRM = FETCH(); ModRM=0; logerror("%06x: unimplemented BRKEM (break to 8080 emulation mode)\n",PC(nec_state)); break;
		default:    logerror("%06x: Unknown V20 instruction\n",PC(nec_state)); break;
	}
}

OP( 0x10, i_adc_br8  ) { DEF_br8;	src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x11, i_adc_wr16 ) { DEF_wr16;	src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x12, i_adc_r8b  ) { DEF_r8b;	src+=CF;	ADDB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x13, i_adc_r16w ) { DEF_r16w;	src+=CF;	ADDW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x14, i_adc_ald8 ) { DEF_ald8;	src+=CF;	ADDB;	nec_state->regs.b[AL]=dst;			CLKS(4,4,2);				}
OP( 0x15, i_adc_axd16) { DEF_axd16;	src+=CF;	ADDW;	nec_state->regs.w[AW]=dst;			CLKS(4,4,2);				}
OP( 0x16, i_push_ss  ) { PUSH(nec_state->sregs[SS]);		CLKS(12,8,3);	}
OP( 0x17, i_pop_ss   ) { POP(nec_state->sregs[SS]);		CLKS(12,8,5);	nec_state->no_interrupt=1; }

OP( 0x18, i_sbb_br8  ) { DEF_br8;	src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x19, i_sbb_wr16 ) { DEF_wr16;	src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x1a, i_sbb_r8b  ) { DEF_r8b;	src+=CF;	SUBB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x1b, i_sbb_r16w ) { DEF_r16w;	src+=CF;	SUBW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x1c, i_sbb_ald8 ) { DEF_ald8;	src+=CF;	SUBB;	nec_state->regs.b[AL]=dst;			CLKS(4,4,2);				}
OP( 0x1d, i_sbb_axd16) { DEF_axd16;	src+=CF;	SUBW;	nec_state->regs.w[AW]=dst;			CLKS(4,4,2);	}
OP( 0x1e, i_push_ds  ) { PUSH(nec_state->sregs[DS0]);		CLKS(12,8,3);	}
OP( 0x1f, i_pop_ds   ) { POP(nec_state->sregs[DS0]);		CLKS(12,8,5);	}

OP( 0x20, i_and_br8  ) { DEF_br8;	ANDB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x21, i_and_wr16 ) { DEF_wr16;	ANDW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x22, i_and_r8b  ) { DEF_r8b;	ANDB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x23, i_and_r16w ) { DEF_r16w;	ANDW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x24, i_and_ald8 ) { DEF_ald8;	ANDB;	nec_state->regs.b[AL]=dst;			CLKS(4,4,2);				}
OP( 0x25, i_and_axd16) { DEF_axd16;	ANDW;	nec_state->regs.w[AW]=dst;			CLKS(4,4,2);	}
OP( 0x26, i_es       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS1]<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x27, i_daa      ) { ADJ4(6,0x60);									CLKS(3,3,2);	}

OP( 0x28, i_sub_br8  ) { DEF_br8;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x29, i_sub_wr16 ) { DEF_wr16;	SUBW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x2a, i_sub_r8b  ) { DEF_r8b;	SUBB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x2b, i_sub_r16w ) { DEF_r16w;	SUBW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x2c, i_sub_ald8 ) { DEF_ald8;	SUBB;	nec_state->regs.b[AL]=dst;			CLKS(4,4,2);				}
OP( 0x2d, i_sub_axd16) { DEF_axd16;	SUBW;	nec_state->regs.w[AW]=dst;			CLKS(4,4,2);	}
OP( 0x2e, i_cs       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[PS]<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x2f, i_das      ) { ADJ4(-6,-0x60);								CLKS(3,3,2);	}

OP( 0x30, i_xor_br8  ) { DEF_br8;	XORB;	PutbackRMByte(ModRM,dst);	CLKM(2,2,2,16,16,7);		}
OP( 0x31, i_xor_wr16 ) { DEF_wr16;	XORW;	PutbackRMWord(ModRM,dst);	CLKR(24,24,11,24,16,7,2,EA);}
OP( 0x32, i_xor_r8b  ) { DEF_r8b;	XORB;	RegByte(ModRM)=dst;			CLKM(2,2,2,11,11,6);		}
OP( 0x33, i_xor_r16w ) { DEF_r16w;	XORW;	RegWord(ModRM)=dst;			CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x34, i_xor_ald8 ) { DEF_ald8;	XORB;	nec_state->regs.b[AL]=dst;			CLKS(4,4,2);				}
OP( 0x35, i_xor_axd16) { DEF_axd16;	XORW;	nec_state->regs.w[AW]=dst;			CLKS(4,4,2);	}
OP( 0x36, i_ss       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[SS]<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x37, i_aaa      ) { ADJB(6, (nec_state->regs.b[AL] > 0xf9) ? 2 : 1);		CLKS(7,7,4);	}

OP( 0x38, i_cmp_br8  ) { DEF_br8;	SUBB;					CLKM(2,2,2,11,11,6); }
OP( 0x39, i_cmp_wr16 ) { DEF_wr16;	SUBW;					CLKR(15,15,8,15,11,6,2,EA);}
OP( 0x3a, i_cmp_r8b  ) { DEF_r8b;	SUBB;					CLKM(2,2,2,11,11,6); }
OP( 0x3b, i_cmp_r16w ) { DEF_r16w;	SUBW;					CLKR(15,15,8,15,11,6,2,EA);	}
OP( 0x3c, i_cmp_ald8 ) { DEF_ald8;	SUBB;					CLKS(4,4,2); }
OP( 0x3d, i_cmp_axd16) { DEF_axd16;	SUBW;					CLKS(4,4,2);	}
OP( 0x3e, i_ds       ) { nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS0]<<4;	CLK(2);		nec_instruction[fetchop(nec_state)](nec_state);	nec_state->seg_prefix=FALSE; }
OP( 0x3f, i_aas      ) { ADJB(-6, (nec_state->regs.b[AL] < 6) ? -2 : -1);		CLKS(7,7,4);	}

OP( 0x40, i_inc_ax  ) { IncWordReg(AW);						CLK(2);	}
OP( 0x41, i_inc_cx  ) { IncWordReg(CW);						CLK(2);	}
OP( 0x42, i_inc_dx  ) { IncWordReg(DW);						CLK(2);	}
OP( 0x43, i_inc_bx  ) { IncWordReg(BW);						CLK(2);	}
OP( 0x44, i_inc_sp  ) { IncWordReg(SP);						CLK(2);	}
OP( 0x45, i_inc_bp  ) { IncWordReg(BP);						CLK(2);	}
OP( 0x46, i_inc_si  ) { IncWordReg(IX);						CLK(2);	}
OP( 0x47, i_inc_di  ) { IncWordReg(IY);						CLK(2);	}

OP( 0x48, i_dec_ax  ) { DecWordReg(AW);						CLK(2);	}
OP( 0x49, i_dec_cx  ) { DecWordReg(CW);						CLK(2);	}
OP( 0x4a, i_dec_dx  ) { DecWordReg(DW);						CLK(2);	}
OP( 0x4b, i_dec_bx  ) { DecWordReg(BW);						CLK(2);	}
OP( 0x4c, i_dec_sp  ) { DecWordReg(SP);						CLK(2);	}
OP( 0x4d, i_dec_bp  ) { DecWordReg(BP);						CLK(2);	}
OP( 0x4e, i_dec_si  ) { DecWordReg(IX);						CLK(2);	}
OP( 0x4f, i_dec_di  ) { DecWordReg(IY);						CLK(2);	}

OP( 0x50, i_push_ax ) { PUSH(nec_state->regs.w[AW]);					CLKS(12,8,3); }
OP( 0x51, i_push_cx ) { PUSH(nec_state->regs.w[CW]);					CLKS(12,8,3); }
OP( 0x52, i_push_dx ) { PUSH(nec_state->regs.w[DW]);					CLKS(12,8,3); }
OP( 0x53, i_push_bx ) { PUSH(nec_state->regs.w[BW]);					CLKS(12,8,3); }
OP( 0x54, i_push_sp ) { PUSH(nec_state->regs.w[SP]);					CLKS(12,8,3); }
OP( 0x55, i_push_bp ) { PUSH(nec_state->regs.w[BP]);					CLKS(12,8,3); }
OP( 0x56, i_push_si ) { PUSH(nec_state->regs.w[IX]);					CLKS(12,8,3); }
OP( 0x57, i_push_di ) { PUSH(nec_state->regs.w[IY]);					CLKS(12,8,3); }

OP( 0x58, i_pop_ax  ) { POP(nec_state->regs.w[AW]);					CLKS(12,8,5); }
OP( 0x59, i_pop_cx  ) { POP(nec_state->regs.w[CW]);					CLKS(12,8,5); }
OP( 0x5a, i_pop_dx  ) { POP(nec_state->regs.w[DW]);					CLKS(12,8,5); }
OP( 0x5b, i_pop_bx  ) { POP(nec_state->regs.w[BW]);					CLKS(12,8,5); }
OP( 0x5c, i_pop_sp  ) { POP(nec_state->regs.w[SP]);					CLKS(12,8,5); }
OP( 0x5d, i_pop_bp  ) { POP(nec_state->regs.w[BP]);					CLKS(12,8,5); }
OP( 0x5e, i_pop_si  ) { POP(nec_state->regs.w[IX]);					CLKS(12,8,5); }
OP( 0x5f, i_pop_di  ) { POP(nec_state->regs.w[IY]);					CLKS(12,8,5); }

OP( 0x60, i_pusha  ) {
	unsigned tmp=nec_state->regs.w[SP];
	PUSH(nec_state->regs.w[AW]);
	PUSH(nec_state->regs.w[CW]);
	PUSH(nec_state->regs.w[DW]);
	PUSH(nec_state->regs.w[BW]);
    PUSH(tmp);
	PUSH(nec_state->regs.w[BP]);
	PUSH(nec_state->regs.w[IX]);
	PUSH(nec_state->regs.w[IY]);
	CLKS(67,35,20);
}
OP( 0x61, i_popa  ) {
    unsigned tmp;
	POP(nec_state->regs.w[IY]);
	POP(nec_state->regs.w[IX]);
	POP(nec_state->regs.w[BP]);
    POP(tmp);
	POP(nec_state->regs.w[BW]);
	POP(nec_state->regs.w[DW]);
	POP(nec_state->regs.w[CW]);
	POP(nec_state->regs.w[AW]);
	CLKS(75,43,22);
}
OP( 0x62, i_chkind  ) {
	UINT32 low,high,tmp;
	GetModRM;
    low = GetRMWord(ModRM);
    high= GetnextRMWord;
    tmp= RegWord(ModRM);
    if (tmp<low || tmp>high) {
		nec_interrupt(nec_state, 5,0);
    }
	nec_state->icount-=20;
	logerror("%06x: bound %04x high %04x low %04x tmp\n",PC(nec_state),high,low,tmp);
}
OP( 0x63, i_brkn   ) { nec_interrupt(nec_state, FETCH(),1); CLKS(50,50,24); } // timing not verified, used by riskchal / gussun
OP( 0x64, i_repnc  ) {	UINT32 next = fetchop(nec_state);	UINT16 c = nec_state->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS1]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[PS]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[SS]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS0]<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
	    case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && !CF); nec_state->regs.w[CW]=c; break;
		default:	logerror("%06x: REPNC invalid\n",PC(nec_state));	nec_instruction[next](nec_state);
    }
	nec_state->seg_prefix=FALSE;
}

OP( 0x65, i_repc  ) {	UINT32 next = fetchop(nec_state);	UINT16 c = nec_state->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS1]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[PS]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[SS]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS0]<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c; break;
	    case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c; break;
	    case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c;	break;
	    case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c; break;
	    case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && CF);	nec_state->regs.w[CW]=c; break;
		default:	logerror("%06x: REPC invalid\n",PC(nec_state));	nec_instruction[next](nec_state);
    }
	nec_state->seg_prefix=FALSE;
}

OP( 0x68, i_push_d16 ) { UINT32 tmp;	tmp = FETCHWORD(); PUSH(tmp);	CLKW(12,12,5,12,8,5,nec_state->regs.w[SP]);	}
OP( 0x69, i_imul_d16 ) { UINT32 tmp;	DEF_r16w;	tmp = FETCHWORD(); dst = (INT32)((INT16)src)*(INT32)((INT16)tmp); nec_state->CarryVal = nec_state->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);     RegWord(ModRM)=(WORD)dst;     nec_state->icount-=(ModRM >=0xc0 )?38:47;}
OP( 0x6a, i_push_d8  ) { UINT32 tmp = (WORD)((INT16)((INT8)FETCH()));	PUSH(tmp);	CLKW(11,11,5,11,7,3,nec_state->regs.w[SP]);	}
OP( 0x6b, i_imul_d8  ) { UINT32 src2; DEF_r16w; src2= (WORD)((INT16)((INT8)FETCH())); dst = (INT32)((INT16)src)*(INT32)((INT16)src2); nec_state->CarryVal = nec_state->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1); RegWord(ModRM)=(WORD)dst; nec_state->icount-=(ModRM >=0xc0 )?31:39; }
OP( 0x6c, i_insb     ) { PutMemB(DS1,nec_state->regs.w[IY],read_port_byte(nec_state->regs.w[DW])); nec_state->regs.w[IY]+= -2 * nec_state->DF + 1; CLK(8); }
OP( 0x6d, i_insw     ) { PutMemW(DS1,nec_state->regs.w[IY],read_port_word(nec_state->regs.w[DW])); nec_state->regs.w[IY]+= -4 * nec_state->DF + 2; CLKS(18,10,8); }
OP( 0x6e, i_outsb    ) { write_port_byte(nec_state->regs.w[DW],GetMemB(DS0,nec_state->regs.w[IX])); nec_state->regs.w[IX]+= -2 * nec_state->DF + 1; CLK(8); }
OP( 0x6f, i_outsw    ) { write_port_word(nec_state->regs.w[DW],GetMemW(DS0,nec_state->regs.w[IX])); nec_state->regs.w[IX]+= -4 * nec_state->DF + 2; CLKS(18,10,8); }

OP( 0x70, i_jo      ) { JMP( OF);				CLKS(4,4,3); }
OP( 0x71, i_jno     ) { JMP(!OF);				CLKS(4,4,3); }
OP( 0x72, i_jc      ) { JMP( CF);				CLKS(4,4,3); }
OP( 0x73, i_jnc     ) { JMP(!CF);				CLKS(4,4,3); }
OP( 0x74, i_jz      ) { JMP( ZF);				CLKS(4,4,3); }
OP( 0x75, i_jnz     ) { JMP(!ZF);				CLKS(4,4,3); }
OP( 0x76, i_jce     ) { JMP(CF || ZF);			CLKS(4,4,3); }
OP( 0x77, i_jnce    ) { JMP(!(CF || ZF));		CLKS(4,4,3); }
OP( 0x78, i_js      ) { JMP( SF);				CLKS(4,4,3); }
OP( 0x79, i_jns     ) { JMP(!SF);				CLKS(4,4,3); }
OP( 0x7a, i_jp      ) { JMP( PF);				CLKS(4,4,3); }
OP( 0x7b, i_jnp     ) { JMP(!PF);				CLKS(4,4,3); }
OP( 0x7c, i_jl      ) { JMP((SF!=OF)&&(!ZF));	CLKS(4,4,3); }
OP( 0x7d, i_jnl     ) { JMP((ZF)||(SF==OF));	CLKS(4,4,3); }
OP( 0x7e, i_jle     ) { JMP((ZF)||(SF!=OF));	CLKS(4,4,3); }
OP( 0x7f, i_jnle    ) { JMP((SF==OF)&&(!ZF));	CLKS(4,4,3); }

OP( 0x80, i_80pre   ) { UINT32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = FETCH();
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKS(13,13,6) else CLKS(18,18,7)
	switch (ModRM & 0x38) {
	    case 0x00: ADDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x08: ORB;				PutbackRMByte(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	break;
		case 0x20: ANDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x28: SUBB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x30: XORB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x38: SUBB;			break;	/* CMP */
    }
}

OP( 0x81, i_81pre   ) { UINT32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = FETCH(); src+= (FETCH() << 8);
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKW(17,17,8,17,13,6,EA) else CLKW(26,26,11,26,18,7,EA)
    switch (ModRM & 0x38) {
	    case 0x00: ADDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x08: ORW;				PutbackRMWord(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	break;
		case 0x20: ANDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x28: SUBW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x30: XORW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x38: SUBW;			break;	/* CMP */
    }
}

OP( 0x82, i_82pre   ) { UINT32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = (BYTE)((INT8)FETCH());
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKS(13,13,6) else CLKS(18,18,7)
	switch (ModRM & 0x38) {
	    case 0x00: ADDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x08: ORB;				PutbackRMByte(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	break;
		case 0x20: ANDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x28: SUBB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x30: XORB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x38: SUBB;			break;	/* CMP */
    }
}

OP( 0x83, i_83pre   ) { UINT32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = (WORD)((INT16)((INT8)FETCH()));
	if (ModRM >=0xc0 ) CLKS(4,4,2) else if ((ModRM & 0x38)==0x38) CLKW(17,17,8,17,13,6,EA) else CLKW(26,26,11,26,18,7,EA)
    switch (ModRM & 0x38) {
	    case 0x00: ADDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x08: ORW;				PutbackRMWord(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	break;
		case 0x20: ANDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x28: SUBW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x30: XORW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x38: SUBW;			break;	/* CMP */
    }
}

OP( 0x84, i_test_br8  ) { DEF_br8;	ANDB;	CLKM(2,2,2,10,10,6);		}
OP( 0x85, i_test_wr16 ) { DEF_wr16;	ANDW;	CLKR(14,14,8,14,10,6,2,EA);	}
OP( 0x86, i_xchg_br8  ) { DEF_br8;	RegByte(ModRM)=dst; PutbackRMByte(ModRM,src); CLKM(3,3,3,16,18,8); }
OP( 0x87, i_xchg_wr16 ) { DEF_wr16;	RegWord(ModRM)=dst; PutbackRMWord(ModRM,src); CLKR(24,24,12,24,16,8,3,EA); }

OP( 0x88, i_mov_br8   ) { UINT8  src; GetModRM; src = RegByte(ModRM);	PutRMByte(ModRM,src);	CLKM(2,2,2,9,9,3);			}
OP( 0x89, i_mov_wr16  ) { UINT16 src; GetModRM; src = RegWord(ModRM);	PutRMWord(ModRM,src);	CLKR(13,13,5,13,9,3,2,EA);	}
OP( 0x8a, i_mov_r8b   ) { UINT8  src; GetModRM; src = GetRMByte(ModRM);	RegByte(ModRM)=src;		CLKM(2,2,2,11,11,5);		}
OP( 0x8b, i_mov_r16w  ) { UINT16 src; GetModRM; src = GetRMWord(ModRM);	RegWord(ModRM)=src; 	CLKR(15,15,7,15,11,5,2,EA); 	}
OP( 0x8c, i_mov_wsreg ) { GetModRM; PutRMWord(ModRM,nec_state->sregs[(ModRM & 0x38) >> 3]);				CLKR(14,14,5,14,10,3,2,EA); }
OP( 0x8d, i_lea       ) { UINT16 ModRM = FETCH(); (void)(*GetEA[ModRM])(nec_state); RegWord(ModRM)=EO;	CLKS(4,4,2); }
OP( 0x8e, i_mov_sregw ) { UINT16 src; GetModRM; src = GetRMWord(ModRM); CLKR(15,15,7,15,11,5,2,EA);
    switch (ModRM & 0x38) {
	    case 0x00: nec_state->sregs[DS1] = src; break; /* mov es,ew */
		case 0x08: nec_state->sregs[PS] = src; break; /* mov cs,ew */
	    case 0x10: nec_state->sregs[SS] = src; break; /* mov ss,ew */
	    case 0x18: nec_state->sregs[DS0] = src; break; /* mov ds,ew */
		default:   logerror("%06x: Mov Sreg - Invalid register\n",PC(nec_state));
    }
	nec_state->no_interrupt=1;
}
OP( 0x8f, i_popw ) { UINT16 tmp; GetModRM; POP(tmp); PutRMWord(ModRM,tmp); nec_state->icount-=21; }
OP( 0x90, i_nop  ) { CLK(3); }
OP( 0x91, i_xchg_axcx ) { XchgAWReg(CW); CLK(3); }
OP( 0x92, i_xchg_axdx ) { XchgAWReg(DW); CLK(3); }
OP( 0x93, i_xchg_axbx ) { XchgAWReg(BW); CLK(3); }
OP( 0x94, i_xchg_axsp ) { XchgAWReg(SP); CLK(3); }
OP( 0x95, i_xchg_axbp ) { XchgAWReg(BP); CLK(3); }
OP( 0x96, i_xchg_axsi ) { XchgAWReg(IX); CLK(3); }
OP( 0x97, i_xchg_axdi ) { XchgAWReg(IY); CLK(3); }

OP( 0x98, i_cbw       ) { nec_state->regs.b[AH] = (nec_state->regs.b[AL] & 0x80) ? 0xff : 0;		CLK(2);	}
OP( 0x99, i_cwd       ) { nec_state->regs.w[DW] = (nec_state->regs.b[AH] & 0x80) ? 0xffff : 0;	CLK(4);	}
OP( 0x9a, i_call_far  ) { UINT32 tmp, tmp2;	tmp = FETCHWORD(); tmp2 = FETCHWORD(); PUSH(nec_state->sregs[PS]); PUSH(nec_state->ip); nec_state->ip = (WORD)tmp; nec_state->sregs[PS] = (WORD)tmp2; CHANGE_PC; CLKW(29,29,13,29,21,9,nec_state->regs.w[SP]); }
OP( 0x9b, i_wait      ) { if (!nec_state->poll_state) nec_state->ip--; CLK(5); }
OP( 0x9c, i_pushf     ) { UINT16 tmp = CompressFlags(); PUSH( tmp ); CLKS(12,8,3); }
OP( 0x9d, i_popf      ) { UINT32 tmp; POP(tmp); ExpandFlags(tmp); CLKS(12,8,5); if (nec_state->TF) nec_trap(nec_state); }
OP( 0x9e, i_sahf      ) { UINT32 tmp = (CompressFlags() & 0xff00) | (nec_state->regs.b[AH] & 0xd5); ExpandFlags(tmp); CLKS(3,3,2); }
OP( 0x9f, i_lahf      ) { nec_state->regs.b[AH] = CompressFlags() & 0xff; CLKS(3,3,2); }

OP( 0xa0, i_mov_aldisp ) { UINT32 addr; addr = FETCHWORD(); nec_state->regs.b[AL] = GetMemB(DS0, addr); CLKS(10,10,5); }
OP( 0xa1, i_mov_axdisp ) { UINT32 addr; addr = FETCHWORD(); nec_state->regs.w[AW] = GetMemW(DS0, addr); CLKW(14,14,7,14,10,5,addr); }
OP( 0xa2, i_mov_dispal ) { UINT32 addr; addr = FETCHWORD(); PutMemB(DS0, addr, nec_state->regs.b[AL]);  CLKS(9,9,3); }
OP( 0xa3, i_mov_dispax ) { UINT32 addr; addr = FETCHWORD(); PutMemW(DS0, addr, nec_state->regs.w[AW]);  CLKW(13,13,5,13,9,3,addr); }
OP( 0xa4, i_movsb      ) { UINT32 tmp = GetMemB(DS0,nec_state->regs.w[IX]); PutMemB(DS1,nec_state->regs.w[IY], tmp); nec_state->regs.w[IY] += -2 * nec_state->DF + 1; nec_state->regs.w[IX] += -2 * nec_state->DF + 1; CLKS(8,8,6); }
OP( 0xa5, i_movsw      ) { UINT32 tmp = GetMemW(DS0,nec_state->regs.w[IX]); PutMemW(DS1,nec_state->regs.w[IY], tmp); nec_state->regs.w[IY] += -4 * nec_state->DF + 2; nec_state->regs.w[IX] += -4 * nec_state->DF + 2; CLKS(16,16,10); }
OP( 0xa6, i_cmpsb      ) { UINT32 src = GetMemB(DS1, nec_state->regs.w[IY]); UINT32 dst = GetMemB(DS0, nec_state->regs.w[IX]); SUBB; nec_state->regs.w[IY] += -2 * nec_state->DF + 1; nec_state->regs.w[IX] += -2 * nec_state->DF + 1; CLKS(14,14,14); }
OP( 0xa7, i_cmpsw      ) { UINT32 src = GetMemW(DS1, nec_state->regs.w[IY]); UINT32 dst = GetMemW(DS0, nec_state->regs.w[IX]); SUBW; nec_state->regs.w[IY] += -4 * nec_state->DF + 2; nec_state->regs.w[IX] += -4 * nec_state->DF + 2; CLKS(14,14,14); }

OP( 0xa8, i_test_ald8  ) { DEF_ald8;  ANDB; CLKS(4,4,2); }
OP( 0xa9, i_test_axd16 ) { DEF_axd16; ANDW; CLKS(4,4,2); }
OP( 0xaa, i_stosb      ) { PutMemB(DS1,nec_state->regs.w[IY],nec_state->regs.b[AL]);	nec_state->regs.w[IY] += -2 * nec_state->DF + 1; CLKS(4,4,3);  }
OP( 0xab, i_stosw      ) { PutMemW(DS1,nec_state->regs.w[IY],nec_state->regs.w[AW]);	nec_state->regs.w[IY] += -4 * nec_state->DF + 2; CLKW(8,8,5,8,4,3,nec_state->regs.w[IY]); }
OP( 0xac, i_lodsb      ) { nec_state->regs.b[AL] = GetMemB(DS0,nec_state->regs.w[IX]); nec_state->regs.w[IX] += -2 * nec_state->DF + 1; CLKS(4,4,3);  }
OP( 0xad, i_lodsw      ) { nec_state->regs.w[AW] = GetMemW(DS0,nec_state->regs.w[IX]); nec_state->regs.w[IX] += -4 * nec_state->DF + 2; CLKW(8,8,5,8,4,3,nec_state->regs.w[IX]); }
OP( 0xae, i_scasb      ) { UINT32 src = GetMemB(DS1, nec_state->regs.w[IY]);	UINT32 dst = nec_state->regs.b[AL]; SUBB; nec_state->regs.w[IY] += -2 * nec_state->DF + 1; CLKS(4,4,3);  }
OP( 0xaf, i_scasw      ) { UINT32 src = GetMemW(DS1, nec_state->regs.w[IY]);	UINT32 dst = nec_state->regs.w[AW]; SUBW; nec_state->regs.w[IY] += -4 * nec_state->DF + 2; CLKW(8,8,5,8,4,3,nec_state->regs.w[IY]); }

OP( 0xb0, i_mov_ald8  ) { nec_state->regs.b[AL] = FETCH();	CLKS(4,4,2); }
OP( 0xb1, i_mov_cld8  ) { nec_state->regs.b[CL] = FETCH(); CLKS(4,4,2); }
OP( 0xb2, i_mov_dld8  ) { nec_state->regs.b[DL] = FETCH(); CLKS(4,4,2); }
OP( 0xb3, i_mov_bld8  ) { nec_state->regs.b[BL] = FETCH(); CLKS(4,4,2); }
OP( 0xb4, i_mov_ahd8  ) { nec_state->regs.b[AH] = FETCH(); CLKS(4,4,2); }
OP( 0xb5, i_mov_chd8  ) { nec_state->regs.b[CH] = FETCH(); CLKS(4,4,2); }
OP( 0xb6, i_mov_dhd8  ) { nec_state->regs.b[DH] = FETCH(); CLKS(4,4,2); }
OP( 0xb7, i_mov_bhd8  ) { nec_state->regs.b[BH] = FETCH();	CLKS(4,4,2); }

OP( 0xb8, i_mov_axd16 ) { nec_state->regs.b[AL] = FETCH();	 nec_state->regs.b[AH] = FETCH();	CLKS(4,4,2); }
OP( 0xb9, i_mov_cxd16 ) { nec_state->regs.b[CL] = FETCH();	 nec_state->regs.b[CH] = FETCH();	CLKS(4,4,2); }
OP( 0xba, i_mov_dxd16 ) { nec_state->regs.b[DL] = FETCH();	 nec_state->regs.b[DH] = FETCH();	CLKS(4,4,2); }
OP( 0xbb, i_mov_bxd16 ) { nec_state->regs.b[BL] = FETCH();	 nec_state->regs.b[BH] = FETCH();	CLKS(4,4,2); }
OP( 0xbc, i_mov_spd16 ) { nec_state->regs.b[SPL] = FETCH(); nec_state->regs.b[SPH] = FETCH();	CLKS(4,4,2); }
OP( 0xbd, i_mov_bpd16 ) { nec_state->regs.b[BPL] = FETCH(); nec_state->regs.b[BPH] = FETCH(); CLKS(4,4,2); }
OP( 0xbe, i_mov_sid16 ) { nec_state->regs.b[IXL] = FETCH(); nec_state->regs.b[IXH] = FETCH();	CLKS(4,4,2); }
OP( 0xbf, i_mov_did16 ) { nec_state->regs.b[IYL] = FETCH(); nec_state->regs.b[IYH] = FETCH();	CLKS(4,4,2); }

OP( 0xc0, i_rotshft_bd8 ) {
	UINT32 src, dst; UINT8 c;
	GetModRM; src = (unsigned)GetRMByte(ModRM); dst=src;
	c=FETCH();
	CLKM(7,7,2,19,19,6);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x08: do { ROR_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x10: do { ROLC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x18: do { RORC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x20: SHL_BYTE(c); break;
		case 0x28: SHR_BYTE(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xc0 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_BYTE(c); break;
	}
}

OP( 0xc1, i_rotshft_wd8 ) {
	UINT32 src, dst;  UINT8 c;
	GetModRM; src = (unsigned)GetRMWord(ModRM); dst=src;
	c=FETCH();
	CLKM(7,7,2,27,19,6);
    if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x08: do { ROR_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x10: do { ROLC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x18: do { RORC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x20: SHL_WORD(c); break;
		case 0x28: SHR_WORD(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xc1 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_WORD(c); break;
	}
}

OP( 0xc2, i_ret_d16  ) { UINT32 count = FETCH(); count += FETCH() << 8; POP(nec_state->ip); nec_state->regs.w[SP]+=count; CHANGE_PC; CLKS(24,24,10); }
OP( 0xc3, i_ret      ) { POP(nec_state->ip); CHANGE_PC; CLKS(19,19,10); }
OP( 0xc4, i_les_dw   ) { GetModRM; WORD tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; nec_state->sregs[DS1] = GetnextRMWord; CLKW(26,26,14,26,18,10,EA); }
OP( 0xc5, i_lds_dw   ) { GetModRM; WORD tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; nec_state->sregs[DS0] = GetnextRMWord; CLKW(26,26,14,26,18,10,EA); }
OP( 0xc6, i_mov_bd8  ) { GetModRM; PutImmRMByte(ModRM); nec_state->icount-=(ModRM >=0xc0 )?4:11; }
OP( 0xc7, i_mov_wd16 ) { GetModRM; PutImmRMWord(ModRM); nec_state->icount-=(ModRM >=0xc0 )?4:15; }

OP( 0xc8, i_enter ) {
    UINT32 nb = FETCH();
    UINT32 i,level;

    nec_state->icount-=23;
    nb += FETCH() << 8;
    level = FETCH();
    PUSH(nec_state->regs.w[BP]);
    nec_state->regs.w[BP]=nec_state->regs.w[SP];
    nec_state->regs.w[SP] -= nb;
    for (i=1;i<level;i++) {
	PUSH(GetMemW(SS,nec_state->regs.w[BP]-i*2));
	nec_state->icount-=16;
    }
    if (level) PUSH(nec_state->regs.w[BP]);
}
OP( 0xc9, i_leave ) {
	nec_state->regs.w[SP]=nec_state->regs.w[BP];
	POP(nec_state->regs.w[BP]);
	nec_state->icount-=8;
}
OP( 0xca, i_retf_d16  ) { UINT32 count = FETCH(); count += FETCH() << 8; POP(nec_state->ip); POP(nec_state->sregs[PS]); nec_state->regs.w[SP]+=count; CHANGE_PC; CLKS(32,32,16); }
OP( 0xcb, i_retf      ) { POP(nec_state->ip); POP(nec_state->sregs[PS]); CHANGE_PC; CLKS(29,29,16); }
OP( 0xcc, i_int3      ) { nec_interrupt(nec_state, 3,0); CLKS(50,50,24); }
OP( 0xcd, i_int       ) { nec_interrupt(nec_state, FETCH(),0); CLKS(50,50,24); }
OP( 0xce, i_into      ) { if (OF) { nec_interrupt(nec_state, 4,0); CLKS(52,52,26); } else CLK(3); }
OP( 0xcf, i_iret      ) { POP(nec_state->ip); POP(nec_state->sregs[PS]); i_popf(nec_state); SetMD(1); CHANGE_PC; CLKS(39,39,19); }

OP( 0xd0, i_rotshft_b ) {
	UINT32 src, dst; GetModRM; src = (UINT32)GetRMByte(ModRM); dst=src;
	CLKM(6,6,2,16,16,7);
    switch (ModRM & 0x38) {
		case 0x00: ROL_BYTE;  PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x08: ROR_BYTE;  PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x10: ROLC_BYTE; PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x18: RORC_BYTE; PutbackRMByte(ModRM,(BYTE)dst); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x20: SHL_BYTE(1); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x28: SHR_BYTE(1); nec_state->OverVal = (src^dst)&0x80; break;
		case 0x30: logerror("%06x: Undefined opcode 0xd0 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_BYTE(1); nec_state->OverVal = 0; break;
	}
}

OP( 0xd1, i_rotshft_w ) {
	UINT32 src, dst; GetModRM; src = (UINT32)GetRMWord(ModRM); dst=src;
	CLKM(6,6,2,24,16,7);
	switch (ModRM & 0x38) {
		case 0x00: ROL_WORD;  PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x08: ROR_WORD;  PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x10: ROLC_WORD; PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x18: RORC_WORD; PutbackRMWord(ModRM,(WORD)dst); nec_state->OverVal = (src^dst)&0x8000; break;
		case 0x20: SHL_WORD(1); nec_state->OverVal = (src^dst)&0x8000;  break;
		case 0x28: SHR_WORD(1); nec_state->OverVal = (src^dst)&0x8000;  break;
		case 0x30: logerror("%06x: Undefined opcode 0xd1 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_WORD(1); nec_state->OverVal = 0; break;
	}
}

OP( 0xd2, i_rotshft_bcl ) {
	UINT32 src, dst; UINT8 c; GetModRM; src = (UINT32)GetRMByte(ModRM); dst=src;
	c=nec_state->regs.b[CL];
	CLKM(7,7,2,19,19,6);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x08: do { ROR_BYTE;  c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x10: do { ROLC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x18: do { RORC_BYTE; c--; CLK(1); } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x20: SHL_BYTE(c); break;
		case 0x28: SHR_BYTE(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xd2 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_BYTE(c); break;
	}
}

OP( 0xd3, i_rotshft_wcl ) {
	UINT32 src, dst; UINT8 c; GetModRM; src = (UINT32)GetRMWord(ModRM); dst=src;
	c=nec_state->regs.b[CL];
	CLKM(7,7,2,27,19,6);
    if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x08: do { ROR_WORD;  c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x10: do { ROLC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x18: do { RORC_WORD; c--; CLK(1); } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x20: SHL_WORD(c); break;
		case 0x28: SHR_WORD(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xd3 0x30 (SHLA)\n",PC(nec_state)); break;
		case 0x38: SHRA_WORD(c); break;
	}
}

OP( 0xd4, i_aam    ) { UINT32 mult=FETCH(); mult=0; nec_state->regs.b[AH] = nec_state->regs.b[AL] / 10; nec_state->regs.b[AL] %= 10; SetSZPF_Word(nec_state->regs.w[AW]); CLKS(15,15,12); }
OP( 0xd5, i_aad    ) { UINT32 mult=FETCH(); mult=0; nec_state->regs.b[AL] = nec_state->regs.b[AH] * 10 + nec_state->regs.b[AL]; nec_state->regs.b[AH] = 0; SetSZPF_Byte(nec_state->regs.b[AL]); CLKS(7,7,8); }
OP( 0xd6, i_setalc ) { nec_state->regs.b[AL] = (CF)?0xff:0x00; nec_state->icount-=3; logerror("%06x: Undefined opcode (SETALC)\n",PC(nec_state)); }
OP( 0xd7, i_trans  ) { UINT32 dest = (nec_state->regs.w[BW]+nec_state->regs.b[AL])&0xffff; nec_state->regs.b[AL] = GetMemB(DS0, dest); CLKS(9,9,5); }
OP( 0xd8, i_fpo    ) { GetModRM; nec_state->icount-=2;	logerror("%06x: Unimplemented floating point control %04x\n",PC(nec_state),ModRM); }

OP( 0xe0, i_loopne ) { INT8 disp = (INT8)FETCH(); nec_state->regs.w[CW]--; if (!ZF && nec_state->regs.w[CW]) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(14,14,6); } else CLKS(5,5,3); }
OP( 0xe1, i_loope  ) { INT8 disp = (INT8)FETCH(); nec_state->regs.w[CW]--; if ( ZF && nec_state->regs.w[CW]) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(14,14,6); } else CLKS(5,5,3); }
OP( 0xe2, i_loop   ) { INT8 disp = (INT8)FETCH(); nec_state->regs.w[CW]--; if (nec_state->regs.w[CW]) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(13,13,6); } else CLKS(5,5,3); }
OP( 0xe3, i_jcxz   ) { INT8 disp = (INT8)FETCH(); if (nec_state->regs.w[CW] == 0) { nec_state->ip = (WORD)(nec_state->ip+disp); /*CHANGE_PC;*/ CLKS(13,13,6); } else CLKS(5,5,3); }
OP( 0xe4, i_inal   ) { UINT8 port = FETCH(); nec_state->regs.b[AL] = read_port_byte(port); CLKS(9,9,5);	}
OP( 0xe5, i_inax   ) { UINT8 port = FETCH(); nec_state->regs.w[AW] = read_port_word(port); CLKW(13,13,7,13,9,5,port); }
OP( 0xe6, i_outal  ) { UINT8 port = FETCH(); write_port_byte(port, nec_state->regs.b[AL]); CLKS(8,8,3);	}
OP( 0xe7, i_outax  ) { UINT8 port = FETCH(); write_port_word(port, nec_state->regs.w[AW]); CLKW(12,12,5,12,8,3,port);	}

OP( 0xe8, i_call_d16 ) { UINT32 tmp; tmp = FETCHWORD(); PUSH(nec_state->ip); nec_state->ip = (WORD)(nec_state->ip+(INT16)tmp); CHANGE_PC; nec_state->icount-=24; }
OP( 0xe9, i_jmp_d16  ) { UINT32 tmp; tmp = FETCHWORD(); nec_state->ip = (WORD)(nec_state->ip+(INT16)tmp); CHANGE_PC; nec_state->icount-=15; }
OP( 0xea, i_jmp_far  ) { UINT32 tmp,tmp1; tmp = FETCHWORD(); tmp1 = FETCHWORD(); nec_state->sregs[PS] = (WORD)tmp1; 	nec_state->ip = (WORD)tmp; CHANGE_PC; nec_state->icount-=27;  }
OP( 0xeb, i_jmp_d8   ) { int tmp = (int)((INT8)FETCH()); nec_state->icount-=12; nec_state->ip = (WORD)(nec_state->ip+tmp); }
OP( 0xec, i_inaldx   ) { nec_state->regs.b[AL] = read_port_byte(nec_state->regs.w[DW]); CLKS(8,8,5);}
OP( 0xed, i_inaxdx   ) { nec_state->regs.w[AW] = read_port_word(nec_state->regs.w[DW]); CLKW(12,12,7,12,8,5,nec_state->regs.w[DW]); }
OP( 0xee, i_outdxal  ) { write_port_byte(nec_state->regs.w[DW], nec_state->regs.b[AL]); CLKS(8,8,3);	}
OP( 0xef, i_outdxax  ) { write_port_word(nec_state->regs.w[DW], nec_state->regs.w[AW]); CLKW(12,12,5,12,8,3,nec_state->regs.w[DW]); }

OP( 0xf0, i_lock     ) { logerror("%06x: Warning - BUSLOCK\n",PC(nec_state)); nec_state->no_interrupt=1; CLK(2); }
OP( 0xf2, i_repne    ) { UINT32 next = fetchop(nec_state); UINT16 c = nec_state->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS1]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[PS]<<4;		next = fetchop(nec_state);	CLK(2); break;
	    case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[SS]<<4;		next = fetchop(nec_state);	CLK(2); break;
	    case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS0]<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && ZF==0);	nec_state->regs.w[CW]=c; break;
	    case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && ZF==0);	nec_state->regs.w[CW]=c; break;
	    case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && ZF==0);	nec_state->regs.w[CW]=c; break;
	    case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && ZF==0);	nec_state->regs.w[CW]=c; break;
		default:	logerror("%06x: REPNE invalid\n",PC(nec_state));	nec_instruction[next](nec_state);
    }
	nec_state->seg_prefix=FALSE;
}
OP( 0xf3, i_repe     ) { UINT32 next = fetchop(nec_state); UINT16 c = nec_state->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS1]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x2e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[PS]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x36:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[SS]<<4;	next = fetchop(nec_state);	CLK(2); break;
	    case 0x3e:	nec_state->seg_prefix=TRUE;	nec_state->prefix_base=nec_state->sregs[DS0]<<4;	next = fetchop(nec_state);	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(2); if (c) do { i_insb(nec_state);  c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0x6d:  CLK(2); if (c) do { i_insw(nec_state);  c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0x6e:	CLK(2); if (c) do { i_outsb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0x6f:  CLK(2); if (c) do { i_outsw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xa4:	CLK(2); if (c) do { i_movsb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xa5:  CLK(2); if (c) do { i_movsw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xa6:	CLK(2); if (c) do { i_cmpsb(nec_state); c--; } while (c>0 && ZF==1);	nec_state->regs.w[CW]=c; break;
	    case 0xa7:	CLK(2); if (c) do { i_cmpsw(nec_state); c--; } while (c>0 && ZF==1);	nec_state->regs.w[CW]=c; break;
	    case 0xaa:	CLK(2); if (c) do { i_stosb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xab:  CLK(2); if (c) do { i_stosw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xac:	CLK(2); if (c) do { i_lodsb(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xad:  CLK(2); if (c) do { i_lodsw(nec_state); c--; } while (c>0);	nec_state->regs.w[CW]=c;	break;
	    case 0xae:	CLK(2); if (c) do { i_scasb(nec_state); c--; } while (c>0 && ZF==1);	nec_state->regs.w[CW]=c; break;
	    case 0xaf:	CLK(2); if (c) do { i_scasw(nec_state); c--; } while (c>0 && ZF==1);	nec_state->regs.w[CW]=c; break;
		default:	logerror("%06x: REPE invalid\n",PC(nec_state)); nec_instruction[next](nec_state);
    }
	nec_state->seg_prefix=FALSE;
}
OP( 0xf4, i_hlt ) { logerror("%06x: HALT\n",PC(nec_state)); nec_state->icount=0; }
OP( 0xf5, i_cmc ) { nec_state->CarryVal = !CF; CLK(2); }
OP( 0xf6, i_f6pre ) { UINT32 tmp; UINT32 uresult,uresult2; INT32 result,result2;
	GetModRM; tmp = GetRMByte(ModRM);
    switch (ModRM & 0x38) {
		case 0x00: tmp &= FETCH(); nec_state->CarryVal = nec_state->OverVal = 0; SetSZPF_Byte(tmp); nec_state->icount-=(ModRM >=0xc0 )?4:11; break; /* TEST */
		case 0x08: logerror("%06x: Undefined opcode 0xf6 0x08\n",PC(nec_state)); break;
		case 0x10: PutbackRMByte(ModRM,~tmp); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NOT */
		case 0x18: nec_state->CarryVal=(tmp!=0); tmp=(~tmp)+1; SetSZPF_Byte(tmp); PutbackRMByte(ModRM,tmp&0xff); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NEG */
		case 0x20: uresult = nec_state->regs.b[AL]*tmp; nec_state->regs.w[AW]=(WORD)uresult; nec_state->CarryVal=nec_state->OverVal=(nec_state->regs.b[AH]!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MULU */
		case 0x28: result = (INT16)((INT8)nec_state->regs.b[AL])*(INT16)((INT8)tmp); nec_state->regs.w[AW]=(WORD)result; nec_state->CarryVal=nec_state->OverVal=(nec_state->regs.b[AH]!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MUL */
		case 0x30: if (tmp) { DIVUB; } else nec_interrupt(nec_state, 0,0); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
		case 0x38: if (tmp) { DIVB;  } else nec_interrupt(nec_state, 0,0); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
    }
}

OP( 0xf7, i_f7pre   ) { UINT32 tmp,tmp2; UINT32 uresult,uresult2; INT32 result,result2;
	GetModRM; tmp = GetRMWord(ModRM);
    switch (ModRM & 0x38) {
		case 0x00: tmp2 = FETCHWORD(); tmp &= tmp2; nec_state->CarryVal = nec_state->OverVal = 0; SetSZPF_Word(tmp); nec_state->icount-=(ModRM >=0xc0 )?4:11; break; /* TEST */
		case 0x08: logerror("%06x: Undefined opcode 0xf7 0x08\n",PC(nec_state)); break;
		case 0x10: PutbackRMWord(ModRM,~tmp); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NOT */
		case 0x18: nec_state->CarryVal=(tmp!=0); tmp=(~tmp)+1; SetSZPF_Word(tmp); PutbackRMWord(ModRM,tmp&0xffff); nec_state->icount-=(ModRM >=0xc0 )?2:16; break; /* NEG */
		case 0x20: uresult = nec_state->regs.w[AW]*tmp; nec_state->regs.w[AW]=uresult&0xffff; nec_state->regs.w[DW]=((UINT32)uresult)>>16; nec_state->CarryVal=nec_state->OverVal=(nec_state->regs.w[DW]!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MULU */
		case 0x28: result = (INT32)((INT16)nec_state->regs.w[AW])*(INT32)((INT16)tmp); nec_state->regs.w[AW]=result&0xffff; nec_state->regs.w[DW]=result>>16; nec_state->CarryVal=nec_state->OverVal=(nec_state->regs.w[DW]!=0); nec_state->icount-=(ModRM >=0xc0 )?30:36; break; /* MUL */
		case 0x30: if (tmp) { DIVUW; } else nec_interrupt(nec_state, 0,0); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
		case 0x38: if (tmp) { DIVW;  } else nec_interrupt(nec_state, 0,0); nec_state->icount-=(ModRM >=0xc0 )?43:53; break;
	}
}

OP( 0xf8, i_clc   ) { nec_state->CarryVal = 0;	CLK(2);	}
OP( 0xf9, i_stc   ) { nec_state->CarryVal = 1;	CLK(2);	}
OP( 0xfa, i_di    ) { SetIF(0);			CLK(2);	}
OP( 0xfb, i_ei    ) { SetIF(1);			CLK(2);	}
OP( 0xfc, i_cld   ) { SetDF(0);			CLK(2);	}
OP( 0xfd, i_std   ) { SetDF(1);			CLK(2);	}
OP( 0xfe, i_fepre ) { UINT32 tmp, tmp1; GetModRM; tmp=GetRMByte(ModRM);
    switch(ModRM & 0x38) {
    	case 0x00: tmp1 = tmp+1; nec_state->OverVal = (tmp==0x7f); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(BYTE)tmp1); CLKM(2,2,2,16,16,7); break; /* INC */
		case 0x08: tmp1 = tmp-1; nec_state->OverVal = (tmp==0x80); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(BYTE)tmp1); CLKM(2,2,2,16,16,7); break; /* DEC */
		default:   logerror("%06x: FE Pre with unimplemented mod\n",PC(nec_state));
	}
}
OP( 0xff, i_ffpre ) { UINT32 tmp, tmp1; GetModRM; tmp=GetRMWord(ModRM);
    switch(ModRM & 0x38) {
    	case 0x00: tmp1 = tmp+1; nec_state->OverVal = (tmp==0x7fff); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(WORD)tmp1); CLKM(2,2,2,24,16,7); break; /* INC */
		case 0x08: tmp1 = tmp-1; nec_state->OverVal = (tmp==0x8000); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(WORD)tmp1); CLKM(2,2,2,24,16,7); break; /* DEC */
		case 0x10: PUSH(nec_state->ip);	nec_state->ip = (WORD)tmp; CHANGE_PC; nec_state->icount-=(ModRM >=0xc0 )?16:20; break; /* CALL */
		case 0x18: tmp1 = nec_state->sregs[PS]; nec_state->sregs[PS] = GetnextRMWord; PUSH(tmp1); PUSH(nec_state->ip); nec_state->ip = tmp; CHANGE_PC; nec_state->icount-=(ModRM >=0xc0 )?16:26; break; /* CALL FAR */
		case 0x20: nec_state->ip = tmp; CHANGE_PC; nec_state->icount-=13; break; /* JMP */
		case 0x28: nec_state->ip = tmp; nec_state->sregs[PS] = GetnextRMWord; CHANGE_PC; nec_state->icount-=15; break; /* JMP FAR */
		case 0x30: PUSH(tmp); nec_state->icount-=4; break;
		default:   logerror("%06x: FF Pre with unimplemented mod\n",PC(nec_state));
	}
}

static void i_invalid(nec_state_t *nec_state)
{
	nec_state->icount-=10;
	logerror("%06x: Invalid Opcode\n",PC(nec_state));
}

/*****************************************************************************/

static void set_irq_line(nec_state_t *nec_state, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if( nec_state->nmi_state == state ) return;
	    nec_state->nmi_state = state;
		if (state != CLEAR_LINE)
		{
			nec_state->pending_irq |= NMI_IRQ;
		}
	}
	else
	{
		nec_state->irq_state = state;
		if (state == CLEAR_LINE)
		{
	//      if (!nec_state->IF)  NS010718 fix interrupt request loss
				nec_state->pending_irq &= ~INT_IRQ;
		}
		else
		{
	//      if (nec_state->IF)   NS010718 fix interrupt request loss
				nec_state->pending_irq |= INT_IRQ;
		}
	}
}

static void set_poll_line(nec_state_t *nec_state, int state)
{
	nec_state->poll_state = state;
}

static CPU_DISASSEMBLE( nec )
{
	nec_state_t *nec_state = get_safe_token(device);

	return necv_dasm_one(buffer, pc, oprom, nec_state->config);
}

static void nec_init(legacy_cpu_device *device, device_irq_callback irqcallback, int type)
{
	const nec_config *config = device->baseconfig().static_config() ? (const nec_config *)device->baseconfig().static_config() : &default_config;
	nec_state_t *nec_state = get_safe_token(device);

	nec_state->config = config;


	state_save_register_device_item_array(device, 0, nec_state->regs.w);
	state_save_register_device_item_array(device, 0, nec_state->sregs);

	state_save_register_device_item(device, 0, nec_state->ip);
	state_save_register_device_item(device, 0, nec_state->TF);
	state_save_register_device_item(device, 0, nec_state->IF);
	state_save_register_device_item(device, 0, nec_state->DF);
	state_save_register_device_item(device, 0, nec_state->MF);
	state_save_register_device_item(device, 0, nec_state->SignVal);
	state_save_register_device_item(device, 0, nec_state->int_vector);
	state_save_register_device_item(device, 0, nec_state->pending_irq);
	state_save_register_device_item(device, 0, nec_state->nmi_state);
	state_save_register_device_item(device, 0, nec_state->irq_state);
	state_save_register_device_item(device, 0, nec_state->poll_state);
	state_save_register_device_item(device, 0, nec_state->AuxVal);
	state_save_register_device_item(device, 0, nec_state->OverVal);
	state_save_register_device_item(device, 0, nec_state->ZeroVal);
	state_save_register_device_item(device, 0, nec_state->CarryVal);
	state_save_register_device_item(device, 0, nec_state->ParityVal);

	nec_state->irq_callback = irqcallback;
	nec_state->device = device;
	nec_state->program = device->space(AS_PROGRAM);
	nec_state->io = device->space(AS_IO);
}


/*****************************************************************************
    8-bit memory accessors
 *****************************************************************************/

static UINT8 memory_read_byte(address_space *space, offs_t address) { return space->read_byte(address); }
static void memory_write_byte(address_space *space, offs_t address, UINT8 data) { space->write_byte(address, data); }
static UINT16 memory_read_word(address_space *space, offs_t address) { return space->read_word(address); }
static void memory_write_word(address_space *space, offs_t address, UINT16 data) { space->write_word(address, data); }


static void configure_memory_8bit(nec_state_t *nec_state)
{
	nec_state->mem.fetch_xor = 0;

	nec_state->mem.rbyte = memory_read_byte;
	nec_state->mem.rword = memory_read_word;
	nec_state->mem.wbyte = memory_write_byte;
	nec_state->mem.wword = memory_write_word;
}


/*****************************************************************************
    16-bit memory accessors
 *****************************************************************************/

static UINT16 read_word_16le(address_space *space, offs_t addr)
{
	if (!(addr & 1))
		return space->read_word(addr);
	else
	{
		UINT16 result = space->read_byte(addr);
		return result | (space->read_byte(addr + 1) << 8);
	}
}

static void write_word_16le(address_space *space, offs_t addr, UINT16 data)
{
	if (!(addr & 1))
		space->write_word(addr, data);
	else
	{
		space->write_byte(addr, data);
		space->write_byte(addr + 1, data >> 8);
	}
}

static void configure_memory_16bit(nec_state_t *nec_state)
{
	nec_state->mem.fetch_xor = BYTE_XOR_LE(0);

	nec_state->mem.rbyte = memory_read_byte;
	nec_state->mem.rword = read_word_16le;
	nec_state->mem.wbyte = memory_write_byte;
	nec_state->mem.wword = write_word_16le;
}

static CPU_EXECUTE( necv )
{
	nec_state_t *nec_state = get_safe_token(device);
	int prev_ICount;

	while(nec_state->icount>0) {
		/* Dispatch IRQ */
		if (nec_state->pending_irq && nec_state->no_interrupt==0)
		{
			if (nec_state->pending_irq & NMI_IRQ)
				external_int(nec_state);
			else if (nec_state->IF)
				external_int(nec_state);
		}

		/* No interrupt allowed between last instruction and this one */
		if (nec_state->no_interrupt)
			nec_state->no_interrupt--;

		debugger_instruction_hook(device, (nec_state->sregs[PS]<<4) + nec_state->ip);
		prev_ICount = nec_state->icount;
		nec_instruction[fetchop(nec_state)](nec_state);
		do_prefetch(nec_state, prev_ICount);
    }
}

/* Wrappers for the different CPU types */
static CPU_INIT( v20 )
{
	nec_state_t *nec_state = get_safe_token(device);

	nec_init(device, irqcallback, 0);
	configure_memory_8bit(nec_state);
	nec_state->chip_type=V20_TYPE;
	nec_state->prefetch_size = 4;		/* 3 words */
	nec_state->prefetch_cycles = 4;		/* four cycles per byte */
}

static CPU_INIT( v30 )
{
	nec_state_t *nec_state = get_safe_token(device);

	nec_init(device, irqcallback, 1);
	configure_memory_16bit(nec_state);
	nec_state->chip_type=V30_TYPE;
	nec_state->prefetch_size = 6;		/* 3 words */
	nec_state->prefetch_cycles = 2;		/* two cycles per byte / four per word */

}

static CPU_INIT( v33 )
{
	nec_state_t *nec_state = get_safe_token(device);

	nec_init(device, irqcallback, 2);
	nec_state->chip_type=V33_TYPE;
	nec_state->prefetch_size = 6;
	/* FIXME: Need information about prefetch size and cycles for V33.
     * complete guess below, nbbatman will not work
     * properly without. */
	nec_state->prefetch_cycles = 1;		/* two cycles per byte / four per word */

	configure_memory_16bit(nec_state);
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( nec )
{
	nec_state_t *nec_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:					set_irq_line(nec_state, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		set_irq_line(nec_state, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:	set_poll_line(nec_state, info->i);					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:
			if( info->i - (nec_state->sregs[PS]<<4) < 0x10000 )
			{
				nec_state->ip = info->i - (nec_state->sregs[PS]<<4);
			}
			else
			{
				nec_state->sregs[PS] = info->i >> 4;
				nec_state->ip = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_IP:				nec_state->ip = info->i;							break;
		case CPUINFO_INT_SP:
			if( info->i - (nec_state->sregs[SS]<<4) < 0x10000 )
			{
				nec_state->regs.w[SP] = info->i - (nec_state->sregs[SS]<<4);
			}
			else
			{
				nec_state->sregs[SS] = info->i >> 4;
				nec_state->regs.w[SP] = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_SP:				nec_state->regs.w[SP] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			ExpandFlags(info->i);					break;
        case CPUINFO_INT_REGISTER + NEC_AW:				nec_state->regs.w[AW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				nec_state->regs.w[CW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				nec_state->regs.w[DW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				nec_state->regs.w[BW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				nec_state->regs.w[BP] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				nec_state->regs.w[IX] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				nec_state->regs.w[IY] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				nec_state->sregs[DS1] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				nec_state->sregs[PS] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				nec_state->sregs[SS] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				nec_state->sregs[DS0] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_VECTOR:			nec_state->int_vector = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static CPU_GET_INFO( nec )
{
	nec_state_t *nec_state = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	int flags;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(nec_state_t);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 80;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					info->i = (nec_state->pending_irq & INT_IRQ) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		info->i = nec_state->nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:	info->i = nec_state->poll_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					/* not supported */						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:				info->i = ((nec_state->sregs[PS]<<4) + nec_state->ip);	break;
		case CPUINFO_INT_REGISTER + NEC_IP:				info->i = nec_state->ip;							break;
		case CPUINFO_INT_SP:							info->i = (nec_state->sregs[SS]<<4) + nec_state->regs.w[SP]; break;
		case CPUINFO_INT_REGISTER + NEC_SP:				info->i = nec_state->regs.w[SP];					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			info->i = CompressFlags();				break;
        case CPUINFO_INT_REGISTER + NEC_AW:				info->i = nec_state->regs.w[AW];					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				info->i = nec_state->regs.w[CW];					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				info->i = nec_state->regs.w[DW];					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				info->i = nec_state->regs.w[BW];					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				info->i = nec_state->regs.w[BP];					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				info->i = nec_state->regs.w[IX];					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				info->i = nec_state->regs.w[IY];					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				info->i = nec_state->sregs[DS1];					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				info->i = nec_state->sregs[PS];					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				info->i = nec_state->sregs[SS];					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				info->i = nec_state->sregs[DS0];					break;
		case CPUINFO_INT_REGISTER + NEC_VECTOR:			info->i = nec_state->int_vector;					break;
		case CPUINFO_INT_REGISTER + NEC_PENDING:		info->i = nec_state->pending_irq;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(nec);			break;
		case CPUINFO_FCT_INIT:							/* set per-CPU */						break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(nec);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(nec);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(necv);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(nec);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &nec_state->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "NEC");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "NEC V-Series");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.5");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "NEC emulator v1.5 by Bryan McPhail"); break;

		case CPUINFO_STR_FLAGS:
            flags = CompressFlags();
            sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                flags & 0x8000 ? 'M':'.',
                flags & 0x4000 ? '?':'.',
                flags & 0x2000 ? '?':'.',
                flags & 0x1000 ? '?':'.',
                flags & 0x0800 ? 'O':'.',
                flags & 0x0400 ? 'D':'.',
                flags & 0x0200 ? 'I':'.',
                flags & 0x0100 ? 'T':'.',
                flags & 0x0080 ? 'S':'.',
                flags & 0x0040 ? 'Z':'.',
                flags & 0x0020 ? '?':'.',
                flags & 0x0010 ? 'A':'.',
                flags & 0x0008 ? '?':'.',
                flags & 0x0004 ? 'P':'.',
                flags & 0x0002 ? 'N':'.',
                flags & 0x0001 ? 'C':'.');
            break;

        case CPUINFO_STR_REGISTER + NEC_PC:				sprintf(info->s, "PC:%04X", (nec_state->sregs[PS]<<4) + nec_state->ip); break;
        case CPUINFO_STR_REGISTER + NEC_IP:				sprintf(info->s, "IP:%04X", nec_state->ip); break;
        case CPUINFO_STR_REGISTER + NEC_SP:				sprintf(info->s, "SP:%04X", nec_state->regs.w[SP]); break;
        case CPUINFO_STR_REGISTER + NEC_FLAGS:			sprintf(info->s, "F:%04X", CompressFlags()); break;
        case CPUINFO_STR_REGISTER + NEC_AW:				sprintf(info->s, "AW:%04X", nec_state->regs.w[AW]); break;
        case CPUINFO_STR_REGISTER + NEC_CW:				sprintf(info->s, "CW:%04X", nec_state->regs.w[CW]); break;
        case CPUINFO_STR_REGISTER + NEC_DW:				sprintf(info->s, "DW:%04X", nec_state->regs.w[DW]); break;
        case CPUINFO_STR_REGISTER + NEC_BW:				sprintf(info->s, "BW:%04X", nec_state->regs.w[BW]); break;
        case CPUINFO_STR_REGISTER + NEC_BP:				sprintf(info->s, "BP:%04X", nec_state->regs.w[BP]); break;
        case CPUINFO_STR_REGISTER + NEC_IX:				sprintf(info->s, "IX:%04X", nec_state->regs.w[IX]); break;
        case CPUINFO_STR_REGISTER + NEC_IY:				sprintf(info->s, "IY:%04X", nec_state->regs.w[IY]); break;
        case CPUINFO_STR_REGISTER + NEC_ES:				sprintf(info->s, "DS1:%04X", nec_state->sregs[DS1]); break;
        case CPUINFO_STR_REGISTER + NEC_CS:				sprintf(info->s, "PS:%04X", nec_state->sregs[PS]); break;
        case CPUINFO_STR_REGISTER + NEC_SS:				sprintf(info->s, "SS:%04X", nec_state->sregs[SS]); break;
        case CPUINFO_STR_REGISTER + NEC_DS:				sprintf(info->s, "DS0:%04X", nec_state->sregs[DS0]); break;
        case CPUINFO_STR_REGISTER + NEC_VECTOR:			sprintf(info->s, "V:%02X", nec_state->int_vector); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v20 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v20);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V20");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v25 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v20);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V25");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v30 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v30);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V30");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v33 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v33);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V33");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v35 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v30);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V35");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(V20, v20);
DEFINE_LEGACY_CPU_DEVICE(V25, v25);
DEFINE_LEGACY_CPU_DEVICE(V30, v30);
DEFINE_LEGACY_CPU_DEVICE(V33, v33);
DEFINE_LEGACY_CPU_DEVICE(V35, v35);
