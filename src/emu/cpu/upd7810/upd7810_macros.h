// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    uPD7810/11, 7810H/11H, 78C10/C11/C14 private use macros

***************************************************************************/

//**************************************************************************
//  REGISTER
//**************************************************************************

#define PPC     m_ppc.w.l
#define PC      m_pc.w.l
#define PCL     m_pc.b.l
#define PCH     m_pc.b.h
#define PCD     m_pc.d
#define SP      m_sp.w.l
#define SPL     m_sp.b.l
#define SPH     m_sp.b.h
#define SPD     m_sp.d
#define PSW     m_psw
#define OP      m_op
#define OP2     m_op2
#define IFF     m_iff
#define EA      m_ea.w.l
#define EAL     m_ea.b.l
#define EAH     m_ea.b.h
#define VA      m_va.w.l
#define V       m_va.b.h
#define A       m_va.b.l
#define VAD     m_va.d
#define BC      m_bc.w.l
#define B       m_bc.b.h
#define C       m_bc.b.l
#define DE      m_de.w.l
#define D       m_de.b.h
#define E       m_de.b.l
#define HL      m_hl.w.l
#define H       m_hl.b.h
#define L       m_hl.b.l
#define EA2     m_ea2.w.l
#define VA2     m_va2.w.l
#define BC2     m_bc2.w.l
#define DE2     m_de2.w.l
#define HL2     m_hl2.w.l

#define OVC0    m_ovc0
#define OVC1    m_ovc1
#define OVCE    m_ovce
#define OVCF    m_ovcf
#define OVCS    m_ovcs
#define EDGES   m_edges

#define CNT0    m_cnt.b.l
#define CNT1    m_cnt.b.h
#define TM0     m_tm.b.l
#define TM1     m_tm.b.h
#define ECNT    m_ecnt.w.l
#define ECPT    m_ecnt.w.h
#define ETM0    m_etm.w.l
#define ETM1    m_etm.w.h

#define MA      m_ma
#define MB      m_mb
#define MCC     m_mcc
#define MC      m_mc
#define MM      m_mm
#define MF      m_mf
#define TMM     m_tmm
#define ETMM    m_etmm
#define EOM     m_eom
#define SML     m_sml
#define SMH     m_smh
#define PANM    m_panm
#define ANM     m_anm
#define MKL     m_mkl
#define MKH     m_mkh
#define ZCM     m_zcm

#define CR0     m_cr0
#define CR1     m_cr1
#define CR2     m_cr2
#define CR3     m_cr3
#define RXB     m_rxb
#define TXB     m_txb

#define RXD     m_rxd
#define TXD     m_txd
#define SCK     m_sck
#define TI      m_ti
#define TO      m_to
#define CI      m_ci
#define LV0     m_lv0
#define LV1     m_lv1
#define CO0     m_co0
#define CO1     m_co1

#define IRR     m_irr
#define ITF     m_itf


//**************************************************************************
//  MEMORY/OPCODE READ/WRITE
//**************************************************************************

#define RDOP(O)     O = m_direct->read_byte(PCD); PC++
#define RDOPARG(A)  A = m_direct->read_byte(PCD); PC++
#define RM(A)       m_program->read_byte(A)
#define WM(A,V)     m_program->write_byte(A,V)


//**************************************************************************
//  PSW OPERATIONS
//**************************************************************************

#define ZHC_ADD(after,before,carry)     \
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (after == before) \
		PSW = (PSW&~CY) | (carry); \
	else if (after < before)            \
		PSW |= CY;          \
	else                                \
		PSW &= ~CY;             \
	if ((after & 15) < (before & 15))   \
		PSW |= HC;                      \
	else                                \
		PSW &= ~HC;

#define ZHC_SUB(after,before,carry)     \
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (before == after)                    \
		PSW = (PSW & ~CY) | (carry);    \
	else if (after > before)            \
		PSW |= CY;          \
	else                                \
		PSW &= ~CY;             \
	if ((after & 15) > (before & 15))   \
		PSW |= HC;                      \
	else                                \
		PSW &= ~HC;

#define SKIP_CY     if (CY == (PSW & CY)) PSW |= SK
#define SKIP_NC     if (0 == (PSW & CY)) PSW |= SK
#define SKIP_Z      if (Z == (PSW & Z)) PSW |= SK
#define SKIP_NZ     if (0 == (PSW & Z)) PSW |= SK

#define SET_Z(n)    if (n) PSW &= ~Z; else PSW |= Z
