/****************************************************************************

    NEC V20/V30/V33 emulator modified to a v30mz emulator

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
        Fix memory interface (should be 16 bit).

****************************************************************************/

#include "emu.h"
#include "debugger.h"

#define PC(n)		(((n)->sregs[CS]<<4)+(n)->ip)

typedef UINT8 BOOLEAN;
typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef UINT32 DWORD;

#include "v30mz.h"
#include "nec.h"

/* NEC registers */
typedef union
{                   /* eight general registers */
    UINT16 w[8];    /* viewed as 16 bits registers */
    UINT8  b[16];   /* or as 8 bit registers */
} necbasicregs;

/* default configuration */
static const nec_config default_config =
{
	NULL		 // no internal decryption table
};

typedef struct _v30mz_state v30mz_state;
struct _v30mz_state
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
	UINT8	no_interrupt;

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int icount;

	UINT32 prefix_base;	/* base address of the latest prefix segment */
	char seg_prefix;		/* prefix segment indicator */

	UINT32 ea;
	UINT16 eo;
	UINT16 e16;

	const nec_config *config;
};

extern int necv_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, const nec_config *config);

INLINE v30mz_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == V30MZ);
	return (v30mz_state *)downcast<legacy_cpu_device *>(device)->token();
}

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/


/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#include "necinstr.h"
#include "necea.h"
#include "necmodrm.h"

static UINT8 parity_table[256];

/***************************************************************************/

static CPU_RESET( nec )
{
	v30mz_state *cpustate = get_safe_token(device);
    unsigned int i,j,c;
    static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };
	device_irq_acknowledge_callback save_irqcallback;

	save_irqcallback = cpustate->irq_callback;
	memset( cpustate, 0, sizeof(*cpustate) );
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	cpustate->sregs[CS] = 0xffff;

    for (i = 0;i < 256; i++)
    {
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;
		parity_table[i] = !(c & 1);
    }

	cpustate->ZeroVal = cpustate->ParityVal = 1;
	SetMD(1);						/* set the mode-flag = native mode */

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
}

static CPU_EXIT( nec )
{

}

static void nec_interrupt(v30mz_state *cpustate,unsigned int_num,BOOLEAN md_flag)
{
    UINT32 dest_seg, dest_off;

    i_pushf(cpustate);
	cpustate->TF = cpustate->IF = 0;
	if (md_flag) SetMD(0);	/* clear Mode-flag = start 8080 emulation mode */

	if (int_num == -1)
	{
		int_num = (*cpustate->irq_callback)(cpustate->device, 0);

		cpustate->irq_state = CLEAR_LINE;
		cpustate->pending_irq &= ~INT_IRQ;
	}

    dest_off = ReadWord(int_num*4);
    dest_seg = ReadWord(int_num*4+2);

	PUSH(cpustate->sregs[CS]);
	PUSH(cpustate->ip);
	cpustate->ip = (WORD)dest_off;
	cpustate->sregs[CS] = (WORD)dest_seg;
}

static void nec_trap(v30mz_state *cpustate)
{
	nec_instruction[FETCHOP](cpustate);
	nec_interrupt(cpustate,1,0);
}

static void external_int(v30mz_state *cpustate)
{
	if( cpustate->pending_irq & NMI_IRQ )
	{
		nec_interrupt(cpustate,NEC_NMI_INT_VECTOR,0);
		cpustate->pending_irq &= ~NMI_IRQ;
	}
	else if( cpustate->pending_irq )
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt(cpustate,(UINT32)-1,0);
	}
}

/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

#define OP(num,func_name) static void func_name(v30mz_state *cpustate)

OP( 0x00, i_add_br8  ) { DEF_br8;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(1,3);		}
OP( 0x01, i_add_wr16 ) { DEF_wr16;	ADDW;	PutbackRMWord(ModRM,dst);	CLKM(1,3);}
OP( 0x02, i_add_r8b  ) { DEF_r8b;	ADDB;	RegByte(ModRM)=dst;			CLKM(1,2);		}
OP( 0x03, i_add_r16w ) { DEF_r16w;	ADDW;	RegWord(ModRM)=dst;			CLKM(1,2);	}
OP( 0x04, i_add_ald8 ) { DEF_ald8;	ADDB;	cpustate->regs.b[AL]=dst;			CLK(1);				}
OP( 0x05, i_add_axd16) { DEF_axd16;	ADDW;	cpustate->regs.w[AW]=dst;			CLK(1);				}
OP( 0x06, i_push_es  ) { PUSH(cpustate->sregs[ES]);	CLK(2); 	}
OP( 0x07, i_pop_es   ) { POP(cpustate->sregs[ES]);	CLK(3);	}

OP( 0x08, i_or_br8   ) { DEF_br8;	ORB;	PutbackRMByte(ModRM,dst);	CLKM(1,3);		}
OP( 0x09, i_or_wr16  ) { DEF_wr16;	ORW;	PutbackRMWord(ModRM,dst);	CLKM(1,3);}
OP( 0x0a, i_or_r8b   ) { DEF_r8b;	ORB;	RegByte(ModRM)=dst;			CLKM(1,2);		}
OP( 0x0b, i_or_r16w  ) { DEF_r16w;	ORW;	RegWord(ModRM)=dst;			CLKM(1,2);	}
OP( 0x0c, i_or_ald8  ) { DEF_ald8;	ORB;	cpustate->regs.b[AL]=dst;			CLK(1);				}
OP( 0x0d, i_or_axd16 ) { DEF_axd16;	ORW;	cpustate->regs.w[AW]=dst;			CLK(1);				}
OP( 0x0e, i_push_cs  ) { PUSH(cpustate->sregs[CS]);	CLK(2);	}
OP( 0x0f, i_pre_nec  ) { UINT32 ModRM, tmp, tmp2;
	switch (FETCH) {
		case 0x10 : BITOP_BYTE;	tmp2 = cpustate->regs.b[CL] & 0x7;	cpustate->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	cpustate->CarryVal=cpustate->OverVal=0; break; /* Test */
		case 0x11 : BITOP_WORD;	tmp2 = cpustate->regs.b[CL] & 0xf;	cpustate->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	cpustate->CarryVal=cpustate->OverVal=0; break; /* Test */
		case 0x12 : BITOP_BYTE;	tmp2 = cpustate->regs.b[CL] & 0x7;	tmp &= ~(1<<tmp2);	PutbackRMByte(ModRM,tmp);	break; /* Clr */
		case 0x13 : BITOP_WORD;	tmp2 = cpustate->regs.b[CL] & 0xf;	tmp &= ~(1<<tmp2);	PutbackRMWord(ModRM,tmp);	break; /* Clr */
		case 0x14 : BITOP_BYTE;	tmp2 = cpustate->regs.b[CL] & 0x7;	tmp |= (1<<tmp2);	PutbackRMByte(ModRM,tmp);	break; /* Set */
		case 0x15 : BITOP_WORD;	tmp2 = cpustate->regs.b[CL] & 0xf;	tmp |= (1<<tmp2);	PutbackRMWord(ModRM,tmp);	break; /* Set */
		case 0x16 : BITOP_BYTE;	tmp2 = cpustate->regs.b[CL] & 0x7;	BIT_NOT;			PutbackRMByte(ModRM,tmp);	break; /* Not */
		case 0x17 : BITOP_WORD;	tmp2 = cpustate->regs.b[CL] & 0xf;	BIT_NOT;			PutbackRMWord(ModRM,tmp);	break; /* Not */

		case 0x18 : BITOP_BYTE;	tmp2 = (FETCH) & 0x7;	cpustate->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	cpustate->CarryVal=cpustate->OverVal=0; break; /* Test */
		case 0x19 : BITOP_WORD;	tmp2 = (FETCH) & 0xf;	cpustate->ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;	cpustate->CarryVal=cpustate->OverVal=0; break; /* Test */
		case 0x1a : BITOP_BYTE;	tmp2 = (FETCH) & 0x7;	tmp &= ~(1<<tmp2);		PutbackRMByte(ModRM,tmp);	break; /* Clr */
		case 0x1b : BITOP_WORD;	tmp2 = (FETCH) & 0xf;	tmp &= ~(1<<tmp2);		PutbackRMWord(ModRM,tmp);	break; /* Clr */
		case 0x1c : BITOP_BYTE;	tmp2 = (FETCH) & 0x7;	tmp |= (1<<tmp2);		PutbackRMByte(ModRM,tmp);	break; /* Set */
		case 0x1d : BITOP_WORD;	tmp2 = (FETCH) & 0xf;	tmp |= (1<<tmp2);		PutbackRMWord(ModRM,tmp);	break; /* Set */
		case 0x1e : BITOP_BYTE;	tmp2 = (FETCH) & 0x7;	BIT_NOT;				PutbackRMByte(ModRM,tmp);	break; /* Not */
		case 0x1f : BITOP_WORD;	tmp2 = (FETCH) & 0xf;	BIT_NOT;				PutbackRMWord(ModRM,tmp);	break; /* Not */

		case 0x20 :	ADD4S; break;
		case 0x22 :	SUB4S; break;
		case 0x26 :	CMP4S; break;
		case 0x28 : ModRM = FETCH; tmp = GetRMByte(ModRM); tmp <<= 4; tmp |= cpustate->regs.b[AL] & 0xf; cpustate->regs.b[AL] = (cpustate->regs.b[AL] & 0xf0) | ((tmp>>8)&0xf); tmp &= 0xff; PutbackRMByte(ModRM,tmp); CLKM(9,15); break;
		case 0x2a : ModRM = FETCH; tmp = GetRMByte(ModRM); tmp2 = (cpustate->regs.b[AL] & 0xf)<<4; cpustate->regs.b[AL] = (cpustate->regs.b[AL] & 0xf0) | (tmp&0xf); tmp = tmp2 | (tmp>>4);	PutbackRMByte(ModRM,tmp); CLKM(13,19); break;
		case 0x31 : ModRM = FETCH; ModRM=0; logerror("%06x: Unimplemented bitfield INS\n",PC(cpustate)); break;
		case 0x33 : ModRM = FETCH; ModRM=0; logerror("%06x: Unimplemented bitfield EXT\n",PC(cpustate)); break;
		case 0x92 : CLK(2); break; /* V25/35 FINT */
		case 0xe0 : ModRM = FETCH; ModRM=0; logerror("%06x: V33 unimplemented BRKXA (break to expansion address)\n",PC(cpustate)); break;
		case 0xf0 : ModRM = FETCH; ModRM=0; logerror("%06x: V33 unimplemented RETXA (return from expansion address)\n",PC(cpustate)); break;
		case 0xff : ModRM = FETCH; ModRM=0; logerror("%06x: unimplemented BRKEM (break to 8080 emulation mode)\n",PC(cpustate)); break;
		default:    logerror("%06x: Unknown V20 instruction\n",PC(cpustate)); break;
	}
}

OP( 0x10, i_adc_br8  ) { DEF_br8;	src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(1,3);		}
OP( 0x11, i_adc_wr16 ) { DEF_wr16;	src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	CLKM(1,3);}
OP( 0x12, i_adc_r8b  ) { DEF_r8b;	src+=CF;	ADDB;	RegByte(ModRM)=dst;			CLKM(1,2);		}
OP( 0x13, i_adc_r16w ) { DEF_r16w;	src+=CF;	ADDW;	RegWord(ModRM)=dst;			CLKM(1,2);	}
OP( 0x14, i_adc_ald8 ) { DEF_ald8;	src+=CF;	ADDB;	cpustate->regs.b[AL]=dst;			CLK(1);				}
OP( 0x15, i_adc_axd16) { DEF_axd16;	src+=CF;	ADDW;	cpustate->regs.w[AW]=dst;			CLK(1);				}
OP( 0x16, i_push_ss  ) { PUSH(cpustate->sregs[SS]);		CLK(2);	}
OP( 0x17, i_pop_ss   ) { POP(cpustate->sregs[SS]);		CLK(3);	cpustate->no_interrupt=1; }

OP( 0x18, i_sbb_br8  ) { DEF_br8;	src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(1,3);		}
OP( 0x19, i_sbb_wr16 ) { DEF_wr16;	src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	CLKM(1,3);}
OP( 0x1a, i_sbb_r8b  ) { DEF_r8b;	src+=CF;	SUBB;	RegByte(ModRM)=dst;			CLKM(1,2);		}
OP( 0x1b, i_sbb_r16w ) { DEF_r16w;	src+=CF;	SUBW;	RegWord(ModRM)=dst;			CLKM(1,2);	}
OP( 0x1c, i_sbb_ald8 ) { DEF_ald8;	src+=CF;	SUBB;	cpustate->regs.b[AL]=dst;			CLK(1); 				}
OP( 0x1d, i_sbb_axd16) { DEF_axd16;	src+=CF;	SUBW;	cpustate->regs.w[AW]=dst;			CLK(1);	}
OP( 0x1e, i_push_ds  ) { PUSH(cpustate->sregs[DS]);		CLK(2);	}
OP( 0x1f, i_pop_ds   ) { POP(cpustate->sregs[DS]);		CLK(3);	}

OP( 0x20, i_and_br8  ) { DEF_br8;	ANDB;	PutbackRMByte(ModRM,dst);	CLKM(1,3);		}
OP( 0x21, i_and_wr16 ) { DEF_wr16;	ANDW;	PutbackRMWord(ModRM,dst);	CLKM(1,3);}
OP( 0x22, i_and_r8b  ) { DEF_r8b;	ANDB;	RegByte(ModRM)=dst;			CLKM(1,2);		}
OP( 0x23, i_and_r16w ) { DEF_r16w;	ANDW;	RegWord(ModRM)=dst;			CLKM(1,2);	}
OP( 0x24, i_and_ald8 ) { DEF_ald8;	ANDB;	cpustate->regs.b[AL]=dst;			CLK(1);				}
OP( 0x25, i_and_axd16) { DEF_axd16;	ANDW;	cpustate->regs.w[AW]=dst;			CLK(1);	}
OP( 0x26, i_es       ) { cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[ES]<<4;	CLK(1);		nec_instruction[FETCHOP](cpustate);	cpustate->seg_prefix=FALSE; }
OP( 0x27, i_daa      ) { ADJ4(6,0x60);									CLK(10);	}

OP( 0x28, i_sub_br8  ) { DEF_br8;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(1,3);		}
OP( 0x29, i_sub_wr16 ) { DEF_wr16;	SUBW;	PutbackRMWord(ModRM,dst);	CLKM(1,3);}
OP( 0x2a, i_sub_r8b  ) { DEF_r8b;	SUBB;	RegByte(ModRM)=dst;			CLKM(1,2);		}
OP( 0x2b, i_sub_r16w ) { DEF_r16w;	SUBW;	RegWord(ModRM)=dst;			CLKM(1,2);	}
OP( 0x2c, i_sub_ald8 ) { DEF_ald8;	SUBB;	cpustate->regs.b[AL]=dst;			CLK(1); 				}
OP( 0x2d, i_sub_axd16) { DEF_axd16;	SUBW;	cpustate->regs.w[AW]=dst;			CLK(1);	}
OP( 0x2e, i_cs       ) { cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[CS]<<4;	CLK(1);		nec_instruction[FETCHOP](cpustate);	cpustate->seg_prefix=FALSE; }
OP( 0x2f, i_das      ) { ADJ4(-6,-0x60);								CLK(10);	}

OP( 0x30, i_xor_br8  ) { DEF_br8;	XORB;	PutbackRMByte(ModRM,dst);	CLKM(1,3);		}
OP( 0x31, i_xor_wr16 ) { DEF_wr16;	XORW;	PutbackRMWord(ModRM,dst);	CLKM(1,3);}
OP( 0x32, i_xor_r8b  ) { DEF_r8b;	XORB;	RegByte(ModRM)=dst;			CLKM(1,2);		}
OP( 0x33, i_xor_r16w ) { DEF_r16w;	XORW;	RegWord(ModRM)=dst;			CLKM(1,2);	}
OP( 0x34, i_xor_ald8 ) { DEF_ald8;	XORB;	cpustate->regs.b[AL]=dst;			CLK(1); 				}
OP( 0x35, i_xor_axd16) { DEF_axd16;	XORW;	cpustate->regs.w[AW]=dst;			CLK(1);	}
OP( 0x36, i_ss       ) { cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[SS]<<4;	CLK(1);		nec_instruction[FETCHOP](cpustate);	cpustate->seg_prefix=FALSE; }
OP( 0x37, i_aaa      ) { ADJB(6, (cpustate->regs.b[AL] > 0xf9) ? 2 : 1);		CLK(9); 	}

OP( 0x38, i_cmp_br8  ) { DEF_br8;	SUBB;					CLKM(1,2); }
OP( 0x39, i_cmp_wr16 ) { DEF_wr16;	SUBW;					CLKM(1,2);}
OP( 0x3a, i_cmp_r8b  ) { DEF_r8b;	SUBB;					CLKM(1,2); }
OP( 0x3b, i_cmp_r16w ) { DEF_r16w;	SUBW;					CLKM(1,2);	}
OP( 0x3c, i_cmp_ald8 ) { DEF_ald8;	SUBB;					CLK(1); }
OP( 0x3d, i_cmp_axd16) { DEF_axd16;	SUBW;					CLK(1);	}
OP( 0x3e, i_ds       ) { cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[DS]<<4;	CLK(1);		nec_instruction[FETCHOP](cpustate);	cpustate->seg_prefix=FALSE; }
OP( 0x3f, i_aas      ) { ADJB(-6, (cpustate->regs.b[AL] < 6) ? -2 : -1);		CLK(9);	}

OP( 0x40, i_inc_ax  ) { IncWordReg(AW);						CLK(1);	}
OP( 0x41, i_inc_cx  ) { IncWordReg(CW);						CLK(1);	}
OP( 0x42, i_inc_dx  ) { IncWordReg(DW);						CLK(1);	}
OP( 0x43, i_inc_bx  ) { IncWordReg(BW);						CLK(1);	}
OP( 0x44, i_inc_sp  ) { IncWordReg(SP);						CLK(1);	}
OP( 0x45, i_inc_bp  ) { IncWordReg(BP);						CLK(1);	}
OP( 0x46, i_inc_si  ) { IncWordReg(IX);						CLK(1);	}
OP( 0x47, i_inc_di  ) { IncWordReg(IY);						CLK(1);	}

OP( 0x48, i_dec_ax  ) { DecWordReg(AW);						CLK(1);	}
OP( 0x49, i_dec_cx  ) { DecWordReg(CW);						CLK(1);	}
OP( 0x4a, i_dec_dx  ) { DecWordReg(DW);						CLK(1);	}
OP( 0x4b, i_dec_bx  ) { DecWordReg(BW);						CLK(1);	}
OP( 0x4c, i_dec_sp  ) { DecWordReg(SP);						CLK(1);	}
OP( 0x4d, i_dec_bp  ) { DecWordReg(BP);						CLK(1);	}
OP( 0x4e, i_dec_si  ) { DecWordReg(IX);						CLK(1);	}
OP( 0x4f, i_dec_di  ) { DecWordReg(IY);						CLK(1);	}

OP( 0x50, i_push_ax ) { PUSH(cpustate->regs.w[AW]);					CLK(1); }
OP( 0x51, i_push_cx ) { PUSH(cpustate->regs.w[CW]);					CLK(1); }
OP( 0x52, i_push_dx ) { PUSH(cpustate->regs.w[DW]);					CLK(1); }
OP( 0x53, i_push_bx ) { PUSH(cpustate->regs.w[BW]);					CLK(1); }
OP( 0x54, i_push_sp ) { PUSH(cpustate->regs.w[SP]);					CLK(1); }
OP( 0x55, i_push_bp ) { PUSH(cpustate->regs.w[BP]);					CLK(1); }
OP( 0x56, i_push_si ) { PUSH(cpustate->regs.w[IX]);					CLK(1); }
OP( 0x57, i_push_di ) { PUSH(cpustate->regs.w[IY]);					CLK(1); }

OP( 0x58, i_pop_ax  ) { POP(cpustate->regs.w[AW]);					CLK(1); }
OP( 0x59, i_pop_cx  ) { POP(cpustate->regs.w[CW]);					CLK(1); }
OP( 0x5a, i_pop_dx  ) { POP(cpustate->regs.w[DW]);					CLK(1); }
OP( 0x5b, i_pop_bx  ) { POP(cpustate->regs.w[BW]);					CLK(1); }
OP( 0x5c, i_pop_sp  ) { POP(cpustate->regs.w[SP]);					CLK(1); }
OP( 0x5d, i_pop_bp  ) { POP(cpustate->regs.w[BP]);					CLK(1); }
OP( 0x5e, i_pop_si  ) { POP(cpustate->regs.w[IX]);					CLK(1); }
OP( 0x5f, i_pop_di  ) { POP(cpustate->regs.w[IY]);					CLK(1); }

OP( 0x60, i_pusha  ) {
	unsigned tmp=cpustate->regs.w[SP];
	PUSH(cpustate->regs.w[AW]);
	PUSH(cpustate->regs.w[CW]);
	PUSH(cpustate->regs.w[DW]);
	PUSH(cpustate->regs.w[BW]);
    PUSH(tmp);
	PUSH(cpustate->regs.w[BP]);
	PUSH(cpustate->regs.w[IX]);
	PUSH(cpustate->regs.w[IY]);
	CLK(9);
}
static unsigned nec_v30mz_popa_tmp;
OP( 0x61, i_popa  ) {
	POP(cpustate->regs.w[IY]);
	POP(cpustate->regs.w[IX]);
	POP(cpustate->regs.w[BP]);
    POP(nec_v30mz_popa_tmp);
	POP(cpustate->regs.w[BW]);
	POP(cpustate->regs.w[DW]);
	POP(cpustate->regs.w[CW]);
	POP(cpustate->regs.w[AW]);
	CLK(8);
}
OP( 0x62, i_chkind  ) {
	UINT32 low,high,tmp;
	GetModRM;
	low = GetRMWord(ModRM);
	high= GetnextRMWord;
	tmp= RegWord(ModRM);
	if (tmp<low || tmp>high) {
		nec_interrupt(cpustate,5,0);
		CLK(20);
	} else {
		CLK(13);
	}
	logerror("%06x: bound %04x high %04x low %04x tmp\n",PC(cpustate),high,low,tmp);
}
OP( 0x64, i_repnc  ) {	UINT32 next = FETCHOP;	UINT16 c = cpustate->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[ES]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x2e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[CS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x36:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[SS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x3e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[DS]<<4;	next = FETCHOP;	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(2); if (c) do { i_insb(cpustate);  c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0x6d:  CLK(2); if (c) do { i_insw(cpustate);  c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0x6e:	CLK(2); if (c) do { i_outsb(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0x6f:  CLK(2); if (c) do { i_outsw(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xa4:	CLK(2); if (c) do { i_movsb(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xa5:  CLK(2); if (c) do { i_movsw(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xa6:	CLK(2); if (c) do { i_cmpsb(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xa7:	CLK(2); if (c) do { i_cmpsw(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xaa:	CLK(2); if (c) do { i_stosb(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xab:  CLK(2); if (c) do { i_stosw(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xac:	CLK(2); if (c) do { i_lodsb(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xad:  CLK(2); if (c) do { i_lodsw(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xae:	CLK(2); if (c) do { i_scasb(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
	    case 0xaf:	CLK(2); if (c) do { i_scasw(cpustate); c--; } while (c>0 && !CF); cpustate->regs.w[CW]=c; break;
		default:	logerror("%06x: REPNC invalid\n",PC(cpustate));	nec_instruction[next](cpustate);
    }
	cpustate->seg_prefix=FALSE;
}

OP( 0x65, i_repc  ) {	UINT32 next = FETCHOP;	UINT16 c = cpustate->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[ES]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x2e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[CS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x36:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[SS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x3e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[DS]<<4;	next = FETCHOP;	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(2); if (c) do { i_insb(cpustate);  c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0x6d:  CLK(2); if (c) do { i_insw(cpustate);  c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0x6e:	CLK(2); if (c) do { i_outsb(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0x6f:  CLK(2); if (c) do { i_outsw(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0xa4:	CLK(2); if (c) do { i_movsb(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0xa5:  CLK(2); if (c) do { i_movsw(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0xa6:	CLK(2); if (c) do { i_cmpsb(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c; break;
	    case 0xa7:	CLK(2); if (c) do { i_cmpsw(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c; break;
	    case 0xaa:	CLK(2); if (c) do { i_stosb(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0xab:  CLK(2); if (c) do { i_stosw(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0xac:	CLK(2); if (c) do { i_lodsb(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0xad:  CLK(2); if (c) do { i_lodsw(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c;	break;
	    case 0xae:	CLK(2); if (c) do { i_scasb(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c; break;
	    case 0xaf:	CLK(2); if (c) do { i_scasw(cpustate); c--; } while (c>0 && CF);	cpustate->regs.w[CW]=c; break;
		default:	logerror("%06x: REPC invalid\n",PC(cpustate));	nec_instruction[next](cpustate);
    }
	cpustate->seg_prefix=FALSE;
}

OP( 0x68, i_push_d16 ) { UINT32 tmp;	FETCHWORD(tmp); PUSH(tmp);	CLK(1);	}
OP( 0x69, i_imul_d16 ) { UINT32 tmp;	DEF_r16w;	FETCHWORD(tmp); dst = (INT32)((INT16)src)*(INT32)((INT16)tmp); cpustate->CarryVal = cpustate->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);     RegWord(ModRM)=(WORD)dst;     if (ModRM >= 0xc0) CLK(3) else CLK(4) }
OP( 0x6a, i_push_d8  ) { UINT32 tmp = (WORD)((INT16)((INT8)FETCH)); 	PUSH(tmp);	CLK(1);	}
OP( 0x6b, i_imul_d8  ) { UINT32 src2; DEF_r16w; src2= (WORD)((INT16)((INT8)FETCH)); dst = (INT32)((INT16)src)*(INT32)((INT16)src2); cpustate->CarryVal = cpustate->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1); RegWord(ModRM)=(WORD)dst; if (ModRM >= 0xc0) CLK(3) else CLK(4) }
OP( 0x6c, i_insb     ) { PutMemB(ES,cpustate->regs.w[IY],read_port(cpustate->regs.w[DW])); cpustate->regs.w[IY]+= -2 * cpustate->DF + 1; CLK(6); }
OP( 0x6d, i_insw     ) { PutMemB(ES,cpustate->regs.w[IY],read_port(cpustate->regs.w[DW])); PutMemB(ES,(cpustate->regs.w[IY]+1)&0xffff,read_port((cpustate->regs.w[DW]+1)&0xffff)); cpustate->regs.w[IY]+= -4 * cpustate->DF + 2; CLK(6); }
OP( 0x6e, i_outsb    ) { write_port(cpustate->regs.w[DW],GetMemB(DS,cpustate->regs.w[IX])); cpustate->regs.w[IX]+= -2 * cpustate->DF + 1; CLK(7); }
OP( 0x6f, i_outsw    ) { write_port(cpustate->regs.w[DW],GetMemB(DS,cpustate->regs.w[IX])); write_port((cpustate->regs.w[DW]+1)&0xffff,GetMemB(DS,(cpustate->regs.w[IX]+1)&0xffff)); cpustate->regs.w[IX]+= -4 * cpustate->DF + 2; CLK(7); }

OP( 0x70, i_jo      ) { JMP( OF); if ( OF) CLK(4) else CLK(1) }
OP( 0x71, i_jno     ) { JMP(!OF); if (!OF) CLK(4) else CLK(1) }
OP( 0x72, i_jc      ) { JMP( CF); if ( CF) CLK(4) else CLK(1) }
OP( 0x73, i_jnc     ) { JMP(!CF); if (!CF) CLK(4) else CLK(1) }
OP( 0x74, i_jz      ) { JMP( ZF); if ( ZF) CLK(4) else CLK(1) }
OP( 0x75, i_jnz     ) { JMP(!ZF); if (!ZF) CLK(4) else CLK(1) }
OP( 0x76, i_jce     ) { JMP(CF || ZF); if (CF || ZF) CLK(4) else CLK(1) }
OP( 0x77, i_jnce    ) { JMP(!(CF || ZF)); if (!(CF || ZF)) CLK(4) else CLK(1) }
OP( 0x78, i_js      ) { JMP( SF); if ( SF) CLK(4) else CLK(1) }
OP( 0x79, i_jns     ) { JMP(!SF); if (!SF) CLK(4) else CLK(1) }
OP( 0x7a, i_jp      ) { JMP( PF); if ( PF) CLK(4) else CLK(1) }
OP( 0x7b, i_jnp     ) { JMP(!PF); if (!PF) CLK(4) else CLK(1) }
OP( 0x7c, i_jl      ) { JMP((SF!=OF)&&(!ZF)); if ((SF!=OF)&&(!ZF)) CLK(4) else CLK(1) }
OP( 0x7d, i_jnl     ) { JMP((ZF)||(SF==OF)); if ((ZF)||(SF==OF)) CLK(4) else CLK(1) }
OP( 0x7e, i_jle     ) { JMP((ZF)||(SF!=OF)); if ((ZF)||(SF!=OF)) CLK(4) else CLK(1) }
OP( 0x7f, i_jnle    ) { JMP((SF==OF)&&(!ZF)); if ((SF==OF)&&(!ZF)) CLK(4) else CLK(1) }

OP( 0x80, i_80pre   ) { UINT32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = FETCH;
	if (ModRM >=0xc0 ) CLK(1) else if ((ModRM & 0x38)==0x38) CLK(2) else CLK(3)
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

OP( 0x81, i_81pre   ) { UINT32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = FETCH; src+= (FETCH << 8);
	if (ModRM >=0xc0 ) CLK(1) else if ((ModRM & 0x38)==0x38) CLK(2) else CLK(3)
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

OP( 0x82, i_82pre   ) { UINT32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = (BYTE)((INT8)FETCH);
	if (ModRM >=0xc0 ) CLK(1) else if ((ModRM & 0x38)==0x38) CLK(2) else CLK(3)
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

OP( 0x83, i_83pre   ) { UINT32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = (WORD)((INT16)((INT8)FETCH));
	if (ModRM >=0xc0 ) CLK(1) else if ((ModRM & 0x38)==0x38) CLK(2) else CLK(3)
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

OP( 0x84, i_test_br8  ) { DEF_br8;	ANDB;	CLKM(1,2);		}
OP( 0x85, i_test_wr16 ) { DEF_wr16;	ANDW;	CLKM(1,2);	}
OP( 0x86, i_xchg_br8  ) { DEF_br8;	RegByte(ModRM)=dst; PutbackRMByte(ModRM,src); CLKM(3,5); }
OP( 0x87, i_xchg_wr16 ) { DEF_wr16;	RegWord(ModRM)=dst; PutbackRMWord(ModRM,src); CLKM(3,5); }

OP( 0x88, i_mov_br8   ) { UINT8  src; GetModRM; src = RegByte(ModRM);	PutRMByte(ModRM,src);	CLK(1); 			}
OP( 0x89, i_mov_wr16  ) { UINT16 src; GetModRM; src = RegWord(ModRM);	PutRMWord(ModRM,src);	CLK(1); 	}
OP( 0x8a, i_mov_r8b   ) { UINT8  src; GetModRM; src = GetRMByte(ModRM);	RegByte(ModRM)=src;		CLK(1); 		}
OP( 0x8b, i_mov_r16w  ) { UINT16 src; GetModRM; src = GetRMWord(ModRM);	RegWord(ModRM)=src; 	CLK(1); 	}
OP( 0x8c, i_mov_wsreg ) { GetModRM; PutRMWord(ModRM,cpustate->sregs[(ModRM & 0x38) >> 3]);				CLKM(1,3); }
OP( 0x8d, i_lea       ) { UINT16 ModRM = FETCH; (void)(*GetEA[ModRM])(cpustate); RegWord(ModRM)=cpustate->eo;	CLK(1); }
OP( 0x8e, i_mov_sregw ) { UINT16 src; GetModRM; src = GetRMWord(ModRM); CLKM(2,3);
    switch (ModRM & 0x38) {
	    case 0x00: cpustate->sregs[ES] = src; break; /* mov es,ew */
		case 0x08: cpustate->sregs[CS] = src; break; /* mov cs,ew */
	    case 0x10: cpustate->sregs[SS] = src; break; /* mov ss,ew */
	    case 0x18: cpustate->sregs[DS] = src; break; /* mov ds,ew */
		default:   logerror("%06x: Mov Sreg - Invalid register\n",PC(cpustate));
    }
	cpustate->no_interrupt=1;
}
OP( 0x8f, i_popw ) { UINT16 tmp; GetModRM; POP(tmp); PutRMWord(ModRM,tmp); CLKM(1,3); }
OP( 0x90, i_nop  ) { CLK(1);
	/* Cycle skip for idle loops (0: NOP  1:  JMP 0) */
	if (cpustate->no_interrupt==0 && cpustate->icount>0 && (cpustate->pending_irq==0) && (PEEKOP((cpustate->sregs[CS]<<4)+cpustate->ip))==0xeb && (PEEK((cpustate->sregs[CS]<<4)+cpustate->ip+1))==0xfd)
		cpustate->icount%=15;
}
OP( 0x91, i_xchg_axcx ) { XchgAWReg(CW); CLK(3); }
OP( 0x92, i_xchg_axdx ) { XchgAWReg(DW); CLK(3); }
OP( 0x93, i_xchg_axbx ) { XchgAWReg(BW); CLK(3); }
OP( 0x94, i_xchg_axsp ) { XchgAWReg(SP); CLK(3); }
OP( 0x95, i_xchg_axbp ) { XchgAWReg(BP); CLK(3); }
OP( 0x96, i_xchg_axsi ) { XchgAWReg(IX); CLK(3); }
OP( 0x97, i_xchg_axdi ) { XchgAWReg(IY); CLK(3); }

OP( 0x98, i_cbw       ) { cpustate->regs.b[AH] = (cpustate->regs.b[AL] & 0x80) ? 0xff : 0;		CLK(1);	}
OP( 0x99, i_cwd       ) { cpustate->regs.w[DW] = (cpustate->regs.b[AH] & 0x80) ? 0xffff : 0;	CLK(1);	}
OP( 0x9a, i_call_far  ) { UINT32 tmp, tmp2;	FETCHWORD(tmp); FETCHWORD(tmp2); PUSH(cpustate->sregs[CS]); PUSH(cpustate->ip); cpustate->ip = (WORD)tmp; cpustate->sregs[CS] = (WORD)tmp2; CLK(10); }
OP( 0x9b, i_wait      ) { logerror("%06x: Hardware POLL\n",PC(cpustate)); }
OP( 0x9c, i_pushf     ) { PUSH( CompressFlags() ); CLK(2); }
OP( 0x9d, i_popf      ) { UINT32 tmp; POP(tmp); ExpandFlags(tmp); CLK(3); if (cpustate->TF) nec_trap(cpustate); }
OP( 0x9e, i_sahf      ) { UINT32 tmp = (CompressFlags() & 0xff00) | (cpustate->regs.b[AH] & 0xd5); ExpandFlags(tmp); CLK(4); }
OP( 0x9f, i_lahf      ) { cpustate->regs.b[AH] = CompressFlags() & 0xff; CLK(2); }

OP( 0xa0, i_mov_aldisp ) { UINT32 addr; FETCHWORD(addr); cpustate->regs.b[AL] = GetMemB(DS, addr); CLK(1); }
OP( 0xa1, i_mov_axdisp ) { UINT32 addr; FETCHWORD(addr); cpustate->regs.b[AL] = GetMemB(DS, addr); cpustate->regs.b[AH] = GetMemB(DS, (addr+1)&0xffff); CLK(1); }
OP( 0xa2, i_mov_dispal ) { UINT32 addr; FETCHWORD(addr); PutMemB(DS, addr, cpustate->regs.b[AL]);  CLK(1); }
OP( 0xa3, i_mov_dispax ) { UINT32 addr; FETCHWORD(addr); PutMemB(DS, addr, cpustate->regs.b[AL]);  PutMemB(DS, (addr+1)&0xffff, cpustate->regs.b[AH]); CLK(1); }
OP( 0xa4, i_movsb      ) { UINT32 tmp = GetMemB(DS,cpustate->regs.w[IX]); PutMemB(ES,cpustate->regs.w[IY], tmp); cpustate->regs.w[IY] += -2 * cpustate->DF + 1; cpustate->regs.w[IX] += -2 * cpustate->DF + 1; CLK(5); }
OP( 0xa5, i_movsw      ) { UINT32 tmp = GetMemW(DS,cpustate->regs.w[IX]); PutMemW(ES,cpustate->regs.w[IY], tmp); cpustate->regs.w[IY] += -4 * cpustate->DF + 2; cpustate->regs.w[IX] += -4 * cpustate->DF + 2; CLK(5); }
OP( 0xa6, i_cmpsb      ) { UINT32 src = GetMemB(ES, cpustate->regs.w[IY]); UINT32 dst = GetMemB(DS, cpustate->regs.w[IX]); SUBB; cpustate->regs.w[IY] += -2 * cpustate->DF + 1; cpustate->regs.w[IX] += -2 * cpustate->DF + 1; CLK(6); }
OP( 0xa7, i_cmpsw      ) { UINT32 src = GetMemW(ES, cpustate->regs.w[IY]); UINT32 dst = GetMemW(DS, cpustate->regs.w[IX]); SUBW; cpustate->regs.w[IY] += -4 * cpustate->DF + 2; cpustate->regs.w[IX] += -4 * cpustate->DF + 2; CLK(6); }

OP( 0xa8, i_test_ald8  ) { DEF_ald8;  ANDB; CLK(1); }
OP( 0xa9, i_test_axd16 ) { DEF_axd16; ANDW; CLK(1); }
OP( 0xaa, i_stosb      ) { PutMemB(ES,cpustate->regs.w[IY],cpustate->regs.b[AL]);	cpustate->regs.w[IY] += -2 * cpustate->DF + 1; CLK(3);  }
OP( 0xab, i_stosw      ) { PutMemW(ES,cpustate->regs.w[IY],cpustate->regs.w[AW]);	cpustate->regs.w[IY] += -4 * cpustate->DF + 2; CLK(3); }
OP( 0xac, i_lodsb      ) { cpustate->regs.b[AL] = GetMemB(DS,cpustate->regs.w[IX]); cpustate->regs.w[IX] += -2 * cpustate->DF + 1; CLK(3);  }
OP( 0xad, i_lodsw      ) { cpustate->regs.w[AW] = GetMemW(DS,cpustate->regs.w[IX]); cpustate->regs.w[IX] += -4 * cpustate->DF + 2; CLK(3); }
OP( 0xae, i_scasb      ) { UINT32 src = GetMemB(ES, cpustate->regs.w[IY]);	UINT32 dst = cpustate->regs.b[AL]; SUBB; cpustate->regs.w[IY] += -2 * cpustate->DF + 1; CLK(4);  }
OP( 0xaf, i_scasw      ) { UINT32 src = GetMemW(ES, cpustate->regs.w[IY]);	UINT32 dst = cpustate->regs.w[AW]; SUBW; cpustate->regs.w[IY] += -4 * cpustate->DF + 2; CLK(4); }

OP( 0xb0, i_mov_ald8  ) { cpustate->regs.b[AL] = FETCH;	CLK(1); }
OP( 0xb1, i_mov_cld8  ) { cpustate->regs.b[CL] = FETCH; CLK(1); }
OP( 0xb2, i_mov_dld8  ) { cpustate->regs.b[DL] = FETCH; CLK(1); }
OP( 0xb3, i_mov_bld8  ) { cpustate->regs.b[BL] = FETCH; CLK(1); }
OP( 0xb4, i_mov_ahd8  ) { cpustate->regs.b[AH] = FETCH; CLK(1); }
OP( 0xb5, i_mov_chd8  ) { cpustate->regs.b[CH] = FETCH; CLK(1); }
OP( 0xb6, i_mov_dhd8  ) { cpustate->regs.b[DH] = FETCH; CLK(1); }
OP( 0xb7, i_mov_bhd8  ) { cpustate->regs.b[BH] = FETCH;	CLK(1); }

OP( 0xb8, i_mov_axd16 ) { cpustate->regs.b[AL] = FETCH;	 cpustate->regs.b[AH] = FETCH;	CLK(1); }
OP( 0xb9, i_mov_cxd16 ) { cpustate->regs.b[CL] = FETCH;	 cpustate->regs.b[CH] = FETCH;	CLK(1); }
OP( 0xba, i_mov_dxd16 ) { cpustate->regs.b[DL] = FETCH;	 cpustate->regs.b[DH] = FETCH;	CLK(1); }
OP( 0xbb, i_mov_bxd16 ) { cpustate->regs.b[BL] = FETCH;	 cpustate->regs.b[BH] = FETCH;	CLK(1); }
OP( 0xbc, i_mov_spd16 ) { cpustate->regs.b[SPL] = FETCH; cpustate->regs.b[SPH] = FETCH;	CLK(1); }
OP( 0xbd, i_mov_bpd16 ) { cpustate->regs.b[BPL] = FETCH; cpustate->regs.b[BPH] = FETCH; CLK(1); }
OP( 0xbe, i_mov_sid16 ) { cpustate->regs.b[IXL] = FETCH; cpustate->regs.b[IXH] = FETCH;	CLK(1); }
OP( 0xbf, i_mov_did16 ) { cpustate->regs.b[IYL] = FETCH; cpustate->regs.b[IYH] = FETCH;	CLK(1); }

OP( 0xc0, i_rotshft_bd8 ) {
	UINT32 src, dst; UINT8 c;
	GetModRM; src = (unsigned)GetRMByte(ModRM); dst=src;
	c=FETCH;
	CLKM(3,5);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_BYTE;  c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x08: do { ROR_BYTE;  c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x10: do { ROLC_BYTE; c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x18: do { RORC_BYTE; c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x20: SHL_BYTE(c); break;
		case 0x28: SHR_BYTE(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xc0 0x30 (SHLA)\n",PC(cpustate)); break;
		case 0x38: SHRA_BYTE(c); break;
	}
}

OP( 0xc1, i_rotshft_wd8 ) {
	UINT32 src, dst;  UINT8 c;
	GetModRM; src = (unsigned)GetRMWord(ModRM); dst=src;
	c=FETCH;
	CLKM(3,5);
    if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_WORD;  c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x08: do { ROR_WORD;  c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x10: do { ROLC_WORD; c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x18: do { RORC_WORD; c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x20: SHL_WORD(c); break;
		case 0x28: SHR_WORD(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xc1 0x30 (SHLA)\n",PC(cpustate)); break;
		case 0x38: SHRA_WORD(c); break;
	}
}

OP( 0xc2, i_ret_d16  ) { UINT32 count = FETCH; count += FETCH << 8; POP(cpustate->ip); cpustate->regs.w[SP]+=count; CLK(6); }
OP( 0xc3, i_ret      ) { POP(cpustate->ip); CLK(6); }
OP( 0xc4, i_les_dw   ) { GetModRM; WORD tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; cpustate->sregs[ES] = GetnextRMWord; CLK(6); }
OP( 0xc5, i_lds_dw   ) { GetModRM; WORD tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; cpustate->sregs[DS] = GetnextRMWord; CLK(6); }
OP( 0xc6, i_mov_bd8  ) { GetModRM; PutImmRMByte(ModRM); CLK(1); }
OP( 0xc7, i_mov_wd16 ) { GetModRM; PutImmRMWord(ModRM); CLK(1); }

OP( 0xc8, i_enter ) {
	UINT32 nb = FETCH;
	UINT32 i,level;

	CLK(8);
	nb += FETCH << 8;
	level = FETCH;
	PUSH(cpustate->regs.w[BP]);
	cpustate->regs.w[BP]=cpustate->regs.w[SP];
	cpustate->regs.w[SP] -= nb;
	for (i=1;i<level;i++) {
		PUSH(GetMemW(SS,cpustate->regs.w[BP]-i*2));
		CLK(4);
	}
	if (level) { PUSH(cpustate->regs.w[BP]); if (level==1) CLK(2) else CLK(3) }
}
OP( 0xc9, i_leave ) {
	cpustate->regs.w[SP]=cpustate->regs.w[BP];
	POP(cpustate->regs.w[BP]);
	CLK(2);
}
OP( 0xca, i_retf_d16  ) { UINT32 count = FETCH; count += FETCH << 8; POP(cpustate->ip); POP(cpustate->sregs[CS]); cpustate->regs.w[SP]+=count; CLK(9); }
OP( 0xcb, i_retf      ) { POP(cpustate->ip); POP(cpustate->sregs[CS]); CLK(8); }
OP( 0xcc, i_int3      ) { nec_interrupt(cpustate,3,0); CLK(9); }
OP( 0xcd, i_int       ) { nec_interrupt(cpustate,FETCH,0); CLK(10); }
OP( 0xce, i_into      ) { if (OF) { nec_interrupt(cpustate,4,0); CLK(13); } else CLK(6); }
OP( 0xcf, i_iret      ) { POP(cpustate->ip); POP(cpustate->sregs[CS]); i_popf(cpustate); CLK(10); }

OP( 0xd0, i_rotshft_b ) {
	UINT32 src, dst; GetModRM; src = (UINT32)GetRMByte(ModRM); dst=src;
	CLKM(1,3);
    switch (ModRM & 0x38) {
		case 0x00: ROL_BYTE;  PutbackRMByte(ModRM,(BYTE)dst); cpustate->OverVal = (src^dst)&0x80; break;
		case 0x08: ROR_BYTE;  PutbackRMByte(ModRM,(BYTE)dst); cpustate->OverVal = (src^dst)&0x80; break;
		case 0x10: ROLC_BYTE; PutbackRMByte(ModRM,(BYTE)dst); cpustate->OverVal = (src^dst)&0x80; break;
		case 0x18: RORC_BYTE; PutbackRMByte(ModRM,(BYTE)dst); cpustate->OverVal = (src^dst)&0x80; break;
		case 0x20: SHL_BYTE(1); cpustate->OverVal = (src^dst)&0x80; break;
		case 0x28: SHR_BYTE(1); cpustate->OverVal = (src^dst)&0x80; break;
		case 0x30: logerror("%06x: Undefined opcode 0xd0 0x30 (SHLA)\n",PC(cpustate)); break;
		case 0x38: SHRA_BYTE(1); cpustate->OverVal = 0; break;
	}
}

OP( 0xd1, i_rotshft_w ) {
	UINT32 src, dst; GetModRM; src = (UINT32)GetRMWord(ModRM); dst=src;
	CLKM(1,3);
	switch (ModRM & 0x38) {
		case 0x00: ROL_WORD;  PutbackRMWord(ModRM,(WORD)dst); cpustate->OverVal = (src^dst)&0x8000; break;
		case 0x08: ROR_WORD;  PutbackRMWord(ModRM,(WORD)dst); cpustate->OverVal = (src^dst)&0x8000; break;
		case 0x10: ROLC_WORD; PutbackRMWord(ModRM,(WORD)dst); cpustate->OverVal = (src^dst)&0x8000; break;
		case 0x18: RORC_WORD; PutbackRMWord(ModRM,(WORD)dst); cpustate->OverVal = (src^dst)&0x8000; break;
		case 0x20: SHL_WORD(1); cpustate->OverVal = (src^dst)&0x8000;  break;
		case 0x28: SHR_WORD(1); cpustate->OverVal = (src^dst)&0x8000;  break;
		case 0x30: logerror("%06x: Undefined opcode 0xd1 0x30 (SHLA)\n",PC(cpustate)); break;
		case 0x38: SHRA_WORD(1); cpustate->OverVal = 0; break;
	}
}

OP( 0xd2, i_rotshft_bcl ) {
	UINT32 src, dst; UINT8 c; GetModRM; src = (UINT32)GetRMByte(ModRM); dst=src;
	c=cpustate->regs.b[CL];
	CLKM(3,5);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_BYTE;  c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x08: do { ROR_BYTE;  c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x10: do { ROLC_BYTE; c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x18: do { RORC_BYTE; c--; } while (c>0); PutbackRMByte(ModRM,(BYTE)dst); break;
		case 0x20: SHL_BYTE(c); break;
		case 0x28: SHR_BYTE(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xd2 0x30 (SHLA)\n",PC(cpustate)); break;
		case 0x38: SHRA_BYTE(c); break;
	}
}

OP( 0xd3, i_rotshft_wcl ) {
	UINT32 src, dst; UINT8 c; GetModRM; src = (UINT32)GetRMWord(ModRM); dst=src;
	c=cpustate->regs.b[CL];
	CLKM(3,5);
    if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_WORD;  c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x08: do { ROR_WORD;  c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x10: do { ROLC_WORD; c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x18: do { RORC_WORD; c--; } while (c>0); PutbackRMWord(ModRM,(WORD)dst); break;
		case 0x20: SHL_WORD(c); break;
		case 0x28: SHR_WORD(c); break;
		case 0x30: logerror("%06x: Undefined opcode 0xd3 0x30 (SHLA)\n",PC(cpustate)); break;
		case 0x38: SHRA_WORD(c); break;
	}
}

OP( 0xd4, i_aam    ) { FETCH; cpustate->regs.b[AH] = cpustate->regs.b[AL] / 10; cpustate->regs.b[AL] %= 10; SetSZPF_Word(cpustate->regs.w[AW]); CLK(17); }
OP( 0xd5, i_aad    ) { FETCH; cpustate->regs.b[AL] = cpustate->regs.b[AH] * 10 + cpustate->regs.b[AL]; cpustate->regs.b[AH] = 0; SetSZPF_Byte(cpustate->regs.b[AL]); CLK(5); }
OP( 0xd6, i_setalc ) { cpustate->regs.b[AL] = (CF)?0xff:0x00; CLK(3); logerror("%06x: Undefined opcode (SETALC)\n",PC(cpustate)); }
OP( 0xd7, i_trans  ) { UINT32 dest = (cpustate->regs.w[BW]+cpustate->regs.b[AL])&0xffff; cpustate->regs.b[AL] = GetMemB(DS, dest); CLK(5); }
OP( 0xd8, i_fpo    ) { GetModRM; CLK(1);	logerror("%06x: Unimplemented floating point control %04x\n",PC(cpustate),ModRM); }

OP( 0xe0, i_loopne ) { INT8 disp = (INT8)FETCH; cpustate->regs.w[CW]--; if (!ZF && cpustate->regs.w[CW]) { cpustate->ip = (WORD)(cpustate->ip+disp); CLK(6); } else CLK(3); }
OP( 0xe1, i_loope  ) { INT8 disp = (INT8)FETCH; cpustate->regs.w[CW]--; if ( ZF && cpustate->regs.w[CW]) { cpustate->ip = (WORD)(cpustate->ip+disp); CLK(6); } else CLK(3); }
OP( 0xe2, i_loop   ) { INT8 disp = (INT8)FETCH; cpustate->regs.w[CW]--; if (cpustate->regs.w[CW]) { cpustate->ip = (WORD)(cpustate->ip+disp); CLK(5); } else CLK(2); }
OP( 0xe3, i_jcxz   ) { INT8 disp = (INT8)FETCH; if (cpustate->regs.w[CW] == 0) { cpustate->ip = (WORD)(cpustate->ip+disp); CLK(4); } else CLK(1); }
OP( 0xe4, i_inal   ) { UINT8 port = FETCH; cpustate->regs.b[AL] = read_port(port); CLK(6);	}
OP( 0xe5, i_inax   ) { UINT8 port = FETCH; cpustate->regs.b[AL] = read_port(port); cpustate->regs.b[AH] = read_port(port+1); CLK(6); }
OP( 0xe6, i_outal  ) { UINT8 port = FETCH; write_port(port, cpustate->regs.b[AL]); CLK(6);	}
OP( 0xe7, i_outax  ) { UINT8 port = FETCH; write_port(port, cpustate->regs.b[AL]); write_port(port+1, cpustate->regs.b[AH]); CLK(6);	}

OP( 0xe8, i_call_d16 ) { UINT32 tmp; FETCHWORD(tmp); PUSH(cpustate->ip); cpustate->ip = (WORD)(cpustate->ip+(INT16)tmp); CLK(5); }
OP( 0xe9, i_jmp_d16  ) { UINT32 tmp; FETCHWORD(tmp); cpustate->ip = (WORD)(cpustate->ip+(INT16)tmp); CLK(4); }
OP( 0xea, i_jmp_far  ) { UINT32 tmp,tmp1; FETCHWORD(tmp); FETCHWORD(tmp1); cpustate->sregs[CS] = (WORD)tmp1;	cpustate->ip = (WORD)tmp; CLK(7);  }
OP( 0xeb, i_jmp_d8   ) { int tmp = (int)((INT8)FETCH); CLK(4);
	if (tmp==-2 && cpustate->no_interrupt==0 && (cpustate->pending_irq==0) && cpustate->icount>0) cpustate->icount%=12; /* cycle skip */
	cpustate->ip = (WORD)(cpustate->ip+tmp);
}
OP( 0xec, i_inaldx   ) { cpustate->regs.b[AL] = read_port(cpustate->regs.w[DW]); CLK(6);}
OP( 0xed, i_inaxdx   ) { UINT32 port = cpustate->regs.w[DW];	cpustate->regs.b[AL] = read_port(port);	cpustate->regs.b[AH] = read_port(port+1); CLK(6); }
OP( 0xee, i_outdxal  ) { write_port(cpustate->regs.w[DW], cpustate->regs.b[AL]); CLK(6);	}
OP( 0xef, i_outdxax  ) { UINT32 port = cpustate->regs.w[DW];	write_port(port, cpustate->regs.b[AL]);	write_port(port+1, cpustate->regs.b[AH]); CLK(6); }

OP( 0xf0, i_lock     ) { logerror("%06x: Warning - BUSLOCK\n",PC(cpustate)); cpustate->no_interrupt=1; CLK(1); }
OP( 0xf2, i_repne    ) { UINT32 next = FETCHOP; UINT16 c = cpustate->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[ES]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x2e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[CS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x36:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[SS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x3e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[DS]<<4;	next = FETCHOP;	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(3); if (c) do { i_insb(cpustate);  c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0x6d:  CLK(3); if (c) do { i_insw(cpustate);  c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0x6e:	CLK(3); if (c) do { i_outsb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0x6f:  CLK(3); if (c) do { i_outsw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xa4:	CLK(3); if (c) do { i_movsb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xa5:  CLK(3); if (c) do { i_movsw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xa6:	CLK(3); if (c) do { i_cmpsb(cpustate); c--; } while (c>0 && ZF==0);	cpustate->regs.w[CW]=c; break;
	    case 0xa7:	CLK(3); if (c) do { i_cmpsw(cpustate); c--; } while (c>0 && ZF==0);	cpustate->regs.w[CW]=c; break;
	    case 0xaa:	CLK(3); if (c) do { i_stosb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xab:  CLK(3); if (c) do { i_stosw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xac:	CLK(3); if (c) do { i_lodsb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xad:  CLK(3); if (c) do { i_lodsw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xae:	CLK(3); if (c) do { i_scasb(cpustate); c--; } while (c>0 && ZF==0);	cpustate->regs.w[CW]=c; break;
	    case 0xaf:	CLK(3); if (c) do { i_scasw(cpustate); c--; } while (c>0 && ZF==0);	cpustate->regs.w[CW]=c; break;
		default:	logerror("%06x: REPNE invalid\n",PC(cpustate));	nec_instruction[next](cpustate);
    }
	cpustate->seg_prefix=FALSE;
}
OP( 0xf3, i_repe     ) { UINT32 next = FETCHOP; UINT16 c = cpustate->regs.w[CW];
    switch(next) { /* Segments */
	    case 0x26:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[ES]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x2e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[CS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x36:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[SS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x3e:	cpustate->seg_prefix=TRUE;	cpustate->prefix_base=cpustate->sregs[DS]<<4;	next = FETCHOP;	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(3); if (c) do { i_insb(cpustate);  c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0x6d:  CLK(3); if (c) do { i_insw(cpustate);  c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0x6e:	CLK(3); if (c) do { i_outsb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0x6f:  CLK(3); if (c) do { i_outsw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xa4:	CLK(3); if (c) do { i_movsb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xa5:  CLK(3); if (c) do { i_movsw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xa6:	CLK(3); if (c) do { i_cmpsb(cpustate); c--; } while (c>0 && ZF==1);	cpustate->regs.w[CW]=c; break;
	    case 0xa7:	CLK(3); if (c) do { i_cmpsw(cpustate); c--; } while (c>0 && ZF==1);	cpustate->regs.w[CW]=c; break;
	    case 0xaa:	CLK(3); if (c) do { i_stosb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xab:  CLK(3); if (c) do { i_stosw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xac:	CLK(3); if (c) do { i_lodsb(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xad:  CLK(3); if (c) do { i_lodsw(cpustate); c--; } while (c>0);	cpustate->regs.w[CW]=c;	break;
	    case 0xae:	CLK(3); if (c) do { i_scasb(cpustate); c--; } while (c>0 && ZF==1);	cpustate->regs.w[CW]=c; break;
	    case 0xaf:	CLK(3); if (c) do { i_scasw(cpustate); c--; } while (c>0 && ZF==1);	cpustate->regs.w[CW]=c; break;
		default:	logerror("%06x: REPE invalid\n",PC(cpustate)); nec_instruction[next](cpustate);
    }
	cpustate->seg_prefix=FALSE;
}
OP( 0xf4, i_hlt ) { logerror("%06x: HALT\n",PC(cpustate)); cpustate->icount=0; }
OP( 0xf5, i_cmc ) { cpustate->CarryVal = !CF; CLK(4); }
OP( 0xf6, i_f6pre ) { UINT32 tmp; UINT32 uresult,uresult2; INT32 result,result2;
	GetModRM; tmp = GetRMByte(ModRM);
    switch (ModRM & 0x38) {
		case 0x00: tmp &= FETCH; cpustate->CarryVal = cpustate->OverVal = 0; SetSZPF_Byte(tmp); CLKM(1,2); break; /* TEST */
		case 0x08: logerror("%06x: Undefined opcode 0xf6 0x08\n",PC(cpustate)); break;
		case 0x10: PutbackRMByte(ModRM,~tmp); CLKM(1,3); break; /* NOT */
		case 0x18: cpustate->CarryVal=(tmp!=0); tmp=(~tmp)+1; SetSZPF_Byte(tmp); PutbackRMByte(ModRM,tmp&0xff); CLKM(1,3); break; /* NEG */
		case 0x20: uresult = cpustate->regs.b[AL]*tmp; cpustate->regs.w[AW]=(WORD)uresult; cpustate->CarryVal=cpustate->OverVal=(cpustate->regs.b[AH]!=0); CLKM(3,4); break; /* MULU */
		case 0x28: result = (INT16)((INT8)cpustate->regs.b[AL])*(INT16)((INT8)tmp); cpustate->regs.w[AW]=(WORD)result; cpustate->CarryVal=cpustate->OverVal=(cpustate->regs.b[AH]!=0); CLKM(3,4); break; /* MUL */
		case 0x30: if (tmp) { DIVUB; } else nec_interrupt(cpustate,0,0); CLKM(15,16); break;
		case 0x38: if (tmp) { DIVB;  } else nec_interrupt(cpustate,0,0); CLKM(17,18); break;
	}
}

OP( 0xf7, i_f7pre   ) { UINT32 tmp,tmp2; UINT32 uresult,uresult2; INT32 result,result2;
	GetModRM; tmp = GetRMWord(ModRM);
    switch (ModRM & 0x38) {
		case 0x00: FETCHWORD(tmp2); tmp &= tmp2; cpustate->CarryVal = cpustate->OverVal = 0; SetSZPF_Word(tmp); CLKM(1,2); break; /* TEST */
		case 0x08: logerror("%06x: Undefined opcode 0xf7 0x08\n",PC(cpustate)); break;
		case 0x10: PutbackRMWord(ModRM,~tmp); CLKM(1,3); break; /* NOT */
		case 0x18: cpustate->CarryVal=(tmp!=0); tmp=(~tmp)+1; SetSZPF_Word(tmp); PutbackRMWord(ModRM,tmp&0xffff); CLKM(1,3); break; /* NEG */
		case 0x20: uresult = cpustate->regs.w[AW]*tmp; cpustate->regs.w[AW]=uresult&0xffff; cpustate->regs.w[DW]=((UINT32)uresult)>>16; cpustate->CarryVal=cpustate->OverVal=(cpustate->regs.w[DW]!=0); CLKM(3,4); break; /* MULU */
		case 0x28: result = (INT32)((INT16)cpustate->regs.w[AW])*(INT32)((INT16)tmp); cpustate->regs.w[AW]=result&0xffff; cpustate->regs.w[DW]=result>>16; cpustate->CarryVal=cpustate->OverVal=(cpustate->regs.w[DW]!=0); CLKM(3,4); break; /* MUL */
		case 0x30: if (tmp) { DIVUW; } else nec_interrupt(cpustate,0,0); CLKM(23,24); break;
		case 0x38: if (tmp) { DIVW;  } else nec_interrupt(cpustate,0,0); CLKM(24,25); break;
	}
}

OP( 0xf8, i_clc   ) { cpustate->CarryVal = 0;	CLK(4);	}
OP( 0xf9, i_stc   ) { cpustate->CarryVal = 1;	CLK(4);	}
OP( 0xfa, i_di    ) { SetIF(0);			CLK(4);	}
OP( 0xfb, i_ei    ) { SetIF(1);			CLK(4);	}
OP( 0xfc, i_cld   ) { SetDF(0);			CLK(4);	}
OP( 0xfd, i_std   ) { SetDF(1);			CLK(4);	}
OP( 0xfe, i_fepre ) { UINT32 tmp, tmp1; GetModRM; tmp=GetRMByte(ModRM);
    switch(ModRM & 0x38) {
    	case 0x00: tmp1 = tmp+1; cpustate->OverVal = (tmp==0x7f); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(BYTE)tmp1); CLKM(1,3); break; /* INC */
		case 0x08: tmp1 = tmp-1; cpustate->OverVal = (tmp==0x80); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(BYTE)tmp1); CLKM(1,3); break; /* DEC */
		default:   logerror("%06x: FE Pre with unimplemented mod\n",PC(cpustate));
	}
}
OP( 0xff, i_ffpre ) { UINT32 tmp, tmp1; GetModRM; tmp=GetRMWord(ModRM);
    switch(ModRM & 0x38) {
    	case 0x00: tmp1 = tmp+1; cpustate->OverVal = (tmp==0x7fff); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(WORD)tmp1); CLKM(1,3); break; /* INC */
		case 0x08: tmp1 = tmp-1; cpustate->OverVal = (tmp==0x8000); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(WORD)tmp1); CLKM(1,3); break; /* DEC */
		case 0x10: PUSH(cpustate->ip);	cpustate->ip = (WORD)tmp; CLKM(5,6); break; /* CALL */
		case 0x18: tmp1 = cpustate->sregs[CS]; cpustate->sregs[CS] = GetnextRMWord; PUSH(tmp1); PUSH(cpustate->ip); cpustate->ip = tmp; CLKM(5,12); break; /* CALL FAR */
		case 0x20: cpustate->ip = tmp; CLKM(4,5); break; /* JMP */
		case 0x28: cpustate->ip = tmp; cpustate->sregs[CS] = GetnextRMWord; CLK(10); break; /* JMP FAR */
		case 0x30: PUSH(tmp); CLK(1); break;
		default:   logerror("%06x: FF Pre with unimplemented mod\n",PC(cpustate));
	}
}

static void i_invalid(v30mz_state *cpustate)
{
	cpustate->icount-=10;
	logerror("%06x: Invalid Opcode\n",PC(cpustate));
}

/*****************************************************************************/

static void set_irq_line(v30mz_state *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if( cpustate->nmi_state == state ) return;
	    cpustate->nmi_state = state;
		if (state != CLEAR_LINE)
		{
			cpustate->pending_irq |= NMI_IRQ;
		}
	}
	else
	{
		cpustate->irq_state = state;
		if (state == CLEAR_LINE)
		{
	//      if (!cpustate->IF)  NS010718 fix interrupt request loss
				cpustate->pending_irq &= ~INT_IRQ;
		}
		else
		{
	//      if (cpustate->IF)   NS010718 fix interrupt request loss
				cpustate->pending_irq |= INT_IRQ;
		}
	}
}

static CPU_DISASSEMBLE( nec )
{
	v30mz_state *cpustate = get_safe_token(device);

	return necv_dasm_one(buffer, pc, oprom, cpustate->config);
}

static void nec_init(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback, int type)
{
	v30mz_state *cpustate = get_safe_token(device);

	const nec_config *config = &default_config;

	device->save_item(NAME(cpustate->regs.w));
	device->save_item(NAME(cpustate->sregs));

	device->save_item(NAME(cpustate->ip));
	device->save_item(NAME(cpustate->TF));
	device->save_item(NAME(cpustate->IF));
	device->save_item(NAME(cpustate->DF));
	device->save_item(NAME(cpustate->MF));
	device->save_item(NAME(cpustate->SignVal));
	device->save_item(NAME(cpustate->int_vector));
	device->save_item(NAME(cpustate->pending_irq));
	device->save_item(NAME(cpustate->nmi_state));
	device->save_item(NAME(cpustate->irq_state));
	device->save_item(NAME(cpustate->AuxVal));
	device->save_item(NAME(cpustate->OverVal));
	device->save_item(NAME(cpustate->ZeroVal));
	device->save_item(NAME(cpustate->CarryVal));
	device->save_item(NAME(cpustate->ParityVal));

	cpustate->config = config;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);
}

static CPU_INIT( v30mz ) { nec_init(device, irqcallback, 3); }
static CPU_EXECUTE( v30mz )
{
	v30mz_state *cpustate = get_safe_token(device);

	while(cpustate->icount>0) {
		/* Dispatch IRQ */
		if (cpustate->pending_irq && cpustate->no_interrupt==0)
		{
			if (cpustate->pending_irq & NMI_IRQ)
				external_int(cpustate);
			else if (cpustate->IF)
				external_int(cpustate);
		}

		/* No interrupt allowed between last instruction and this one */
		if (cpustate->no_interrupt)
			cpustate->no_interrupt--;

		debugger_instruction_hook(device, (cpustate->sregs[CS]<<4) + cpustate->ip);
		nec_instruction[FETCHOP](cpustate);
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( nec )
{
	v30mz_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:
			if( info->i - (cpustate->sregs[CS]<<4) < 0x10000 )
			{
				cpustate->ip = info->i - (cpustate->sregs[CS]<<4);
			}
			else
			{
				cpustate->sregs[CS] = info->i >> 4;
				cpustate->ip = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_IP:				cpustate->ip = info->i;							break;
		case CPUINFO_INT_SP:
			if( info->i - (cpustate->sregs[SS]<<4) < 0x10000 )
			{
				cpustate->regs.w[SP] = info->i - (cpustate->sregs[SS]<<4);
			}
			else
			{
				cpustate->sregs[SS] = info->i >> 4;
				cpustate->regs.w[SP] = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_SP:				cpustate->regs.w[SP] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			ExpandFlags(info->i);					break;
        case CPUINFO_INT_REGISTER + NEC_AW:				cpustate->regs.w[AW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				cpustate->regs.w[CW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				cpustate->regs.w[DW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				cpustate->regs.w[BW] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				cpustate->regs.w[BP] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				cpustate->regs.w[IX] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				cpustate->regs.w[IY] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				cpustate->sregs[ES] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				cpustate->sregs[CS] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				cpustate->sregs[SS] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				cpustate->sregs[DS] = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_VECTOR:			cpustate->int_vector = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( v30mz )
{
	v30mz_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	int flags;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(v30mz_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 80;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 20;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = (cpustate->pending_irq & INT_IRQ) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;					break;

		case CPUINFO_INT_PREVIOUSPC:					/* not supported */						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:				info->i = ((cpustate->sregs[CS]<<4) + cpustate->ip);	break;
		case CPUINFO_INT_REGISTER + NEC_IP:				info->i = cpustate->ip;							break;
		case CPUINFO_INT_SP:							info->i = (cpustate->sregs[SS]<<4) + cpustate->regs.w[SP]; break;
		case CPUINFO_INT_REGISTER + NEC_SP:				info->i = cpustate->regs.w[SP];					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			info->i = CompressFlags();				break;
        case CPUINFO_INT_REGISTER + NEC_AW:				info->i = cpustate->regs.w[AW];					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				info->i = cpustate->regs.w[CW];					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				info->i = cpustate->regs.w[DW];					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				info->i = cpustate->regs.w[BW];					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				info->i = cpustate->regs.w[BP];					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				info->i = cpustate->regs.w[IX];					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				info->i = cpustate->regs.w[IY];					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				info->i = cpustate->sregs[ES];					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				info->i = cpustate->sregs[CS];					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				info->i = cpustate->sregs[SS];					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				info->i = cpustate->sregs[DS];					break;
		case CPUINFO_INT_REGISTER + NEC_VECTOR:			info->i = cpustate->int_vector;					break;
		case CPUINFO_INT_REGISTER + NEC_PENDING:		info->i = cpustate->pending_irq;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(nec);			break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(nec);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(nec);					break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(nec);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "NEC V-Series"); break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.5"); break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__); break;
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

        case CPUINFO_STR_REGISTER + NEC_PC:				sprintf(info->s, "PC:%04X", (cpustate->sregs[CS]<<4) + cpustate->ip); break;
        case CPUINFO_STR_REGISTER + NEC_IP:				sprintf(info->s, "IP:%04X", cpustate->ip); break;
        case CPUINFO_STR_REGISTER + NEC_SP:				sprintf(info->s, "SP:%04X", cpustate->regs.w[SP]); break;
        case CPUINFO_STR_REGISTER + NEC_FLAGS:			sprintf(info->s, "F:%04X", CompressFlags()); break;
        case CPUINFO_STR_REGISTER + NEC_AW:				sprintf(info->s, "AW:%04X", cpustate->regs.w[AW]); break;
        case CPUINFO_STR_REGISTER + NEC_CW:				sprintf(info->s, "CW:%04X", cpustate->regs.w[CW]); break;
        case CPUINFO_STR_REGISTER + NEC_DW:				sprintf(info->s, "DW:%04X", cpustate->regs.w[DW]); break;
        case CPUINFO_STR_REGISTER + NEC_BW:				sprintf(info->s, "BW:%04X", cpustate->regs.w[BW]); break;
        case CPUINFO_STR_REGISTER + NEC_BP:				sprintf(info->s, "BP:%04X", cpustate->regs.w[BP]); break;
        case CPUINFO_STR_REGISTER + NEC_IX:				sprintf(info->s, "IX:%04X", cpustate->regs.w[IX]); break;
        case CPUINFO_STR_REGISTER + NEC_IY:				sprintf(info->s, "IY:%04X", cpustate->regs.w[IY]); break;
        case CPUINFO_STR_REGISTER + NEC_ES:				sprintf(info->s, "ES:%04X", cpustate->sregs[ES]); break;
        case CPUINFO_STR_REGISTER + NEC_CS:				sprintf(info->s, "CS:%04X", cpustate->sregs[CS]); break;
        case CPUINFO_STR_REGISTER + NEC_SS:				sprintf(info->s, "SS:%04X", cpustate->sregs[SS]); break;
        case CPUINFO_STR_REGISTER + NEC_DS:				sprintf(info->s, "DS:%04X", cpustate->sregs[DS]); break;
        case CPUINFO_STR_REGISTER + NEC_VECTOR:			sprintf(info->s, "V:%02X", cpustate->int_vector); break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_INIT:		info->init = CPU_INIT_NAME(v30mz);
					break;
	case CPUINFO_FCT_EXECUTE:	info->execute = CPU_EXECUTE_NAME(v30mz);
					break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case DEVINFO_STR_NAME:		strcpy(info->s, "V30MZ");
					break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(V30MZ, v30mz);
