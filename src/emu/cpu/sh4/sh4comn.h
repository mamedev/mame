// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh4comn.h
 *
 *   SH-4 non-specific components
 *
 *****************************************************************************/

#pragma once

#ifndef __SH4COMN_H__
#define __SH4COMN_H__

//#define USE_SH4DRC

/* speed up delay loops, bail out of tight loops */
#define BUSY_LOOP_HACKS     0

#define VERBOSE 0

#ifdef USE_SH4DRC
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

class sh4_frontend;
#endif

#define CPU_TYPE_SH3    (2)
#define CPU_TYPE_SH4    (3)

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define EXPPRI(pl,po,p,n)   (((4-(pl)) << 24) | ((15-(po)) << 16) | ((p) << 8) | (255-(n)))
#define NMIPRI()            EXPPRI(3,0,16,SH4_INTC_NMI)
#define INTPRI(p,n)         EXPPRI(4,2,p,n)

#define FP_RS(r) m_fr[(r)] // binary representation of single precision floating point register r
#define FP_RFS(r) *( (float  *)(m_fr+(r)) ) // single precision floating point register r
#define FP_RFD(r) *( (double *)(m_fr+(r)) ) // double precision floating point register r
#define FP_XS(r) m_xf[(r)] // binary representation of extended single precision floating point register r
#define FP_XFS(r) *( (float  *)(m_xf+(r)) ) // single precision extended floating point register r
#define FP_XFD(r) *( (double *)(m_xf+(r)) ) // double precision extended floating point register r
#ifdef LSB_FIRST
#define FP_RS2(r) m_fr[(r) ^ m_fpu_pr]
#define FP_RFS2(r) *( (float  *)(m_fr+((r) ^ m_fpu_pr)) )
#define FP_XS2(r) m_xf[(r) ^ m_fpu_pr]
#define FP_XFS2(r) *( (float  *)(m_xf+((r) ^ m_fpu_pr)) )
#endif


#ifdef USE_SH4DRC
struct sh4_state
{
	int icount;

	int pcfsel;                 // last pcflush entry set
	int maxpcfsel;              // highest valid pcflush entry
	UINT32 pcflushes[16];       // pcflush entries

	drc_cache *         cache;              /* pointer to the DRC code cache */
	drcuml_state *      drcuml;             /* DRC UML generator state */
	sh4_frontend *      drcfe;              /* pointer to the DRC front-end class */
	UINT32              drcoptions;         /* configurable DRC options */

	/* internal stuff */
	UINT8               cache_dirty;        /* true if we need to flush the cache */

	/* parameters for subroutines */
	UINT64              numcycles;          /* return value from gettotalcycles */
	UINT32              arg0;               /* print_debug argument 1 */
	UINT32              arg1;               /* print_debug argument 2 */
	UINT32              irq;                /* irq we're taking */

	/* register mappings */
	uml::parameter  regmap[16];                 /* parameter to register mappings for all 16 integer registers */

	uml::code_handle *  entry;                      /* entry point */
	uml::code_handle *  read8;                  /* read byte */
	uml::code_handle *  write8;                 /* write byte */
	uml::code_handle *  read16;                 /* read half */
	uml::code_handle *  write16;                    /* write half */
	uml::code_handle *  read32;                 /* read word */
	uml::code_handle *  write32;                    /* write word */

	uml::code_handle *  interrupt;              /* interrupt */
	uml::code_handle *  nocode;                 /* nocode */
	uml::code_handle *  out_of_cycles;              /* out of cycles exception handler */

	UINT32 prefadr;
	UINT32 target;
};
#endif

#ifdef USE_SH4DRC
class sh4_frontend : public drc_frontend
{
public:
	sh4_frontend(sh4_state &state, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);

protected:
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev);

private:
	bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_2(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_3(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_6(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_8(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_12(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);

	sh4_state &m_context;
};
#endif


enum
{
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

/* Bits in SR */
#define T   0x00000001
#define S   0x00000002
#define I   0x000000f0
#define Q   0x00000100
#define M   0x00000200
#define FD  0x00008000
#define BL  0x10000000
#define sRB 0x20000000
#define MD  0x40000000

/* 29 bits */
#define AM  0x1fffffff

#define FLAGS   (MD|sRB|BL|FD|M|Q|I|S|T)

/* Bits in FPSCR */
#define RM  0x00000003
#define DN  0x00040000
#define PR  0x00080000
#define SZ  0x00100000
#define FR  0x00200000

#define Rn  ((opcode>>8)&15)
#define Rm  ((opcode>>4)&15)

#define REGFLAG_R(n)                    (1 << (n))
#define REGFLAG_FR(n)                   (1 << (n))
#define REGFLAG_XR(n)                   (1 << (n))

/* register flags 1 */
#define REGFLAG_PR                      (1 << 0)
#define REGFLAG_MACL                    (1 << 1)
#define REGFLAG_MACH                    (1 << 2)
#define REGFLAG_GBR                     (1 << 3)
#define REGFLAG_VBR                     (1 << 4)
#define REGFLAG_SR                      (1 << 5)
#define REGFLAG_SGR                     (1 << 6)
#define REGFLAG_FPUL                    (1 << 7)
#define REGFLAG_FPSCR                   (1 << 8)
#define REGFLAG_DBR                     (1 << 9)
#define REGFLAG_SSR                     (1 << 10)
#define REGFLAG_SPC                     (1 << 11)


#endif /* __SH4COMN_H__ */
